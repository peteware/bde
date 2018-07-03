// bslmt_entrypointfunctoradapter.h                                   -*-C++-*-
#ifndef INCLUDED_BSLMT_ENTRYPOINTFUNCTORADAPTER
#define INCLUDED_BSLMT_ENTRYPOINTFUNCTORADAPTER

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$: $")

//@PURPOSE: Provide types and utilities to simplify thread creation.
//
//@CLASSES:
//      EntryPointFunctorAdapter: Encapsulate invokable object with allocator.
//  EntryPointFunctorAdapterUtil: Dynamic allocation of adapter objects.
//
//@DESCRIPTION: This component defines a type, 'EntryPointFunctorAdapter', that
// contains a single instance of a parameterized invokable type along with an
// allocator to manage it.  The parameterized type must provide a copy
// constructor and 'void operator()()'.
//
// This component also provides a C-linkage function
// 'bslmt_EntryPointFunctorAdapter_invoker' that operates on a pointer to
// 'EntryPointFunctorAdapter', invoking the invokable object contained within
// it and then deallocating the adapter object along with the contained
// invokable object.  Together, 'EntryPointFunctorAdapter' and
// 'bslmt_EntryPointFunctorAdapter_invoker' simplify the process of invoking a
// generic functor as a C-style callback, such as a thread entry point.
//
// Finally, this component provides 'EntryPointFunctorAdapterUtil', a namespace
// for a utility function that dynamically allocates instances of
// 'EntryPointFunctorAdapter'.
//
///Usage
///-----
// This section illustrates the intended use of this component.
//
///Example 1: Wrapping a C++ Invokable Type
/// - - - - - - - - - - - - - - - - - - - -
// Suppose we have an existing interface for invoking a C-linkage function and
// passing a void* argument to it.  This situation may arise when starting
// threads or in general when registering a C-style callback.  A simplistic
// example of such a function is:
//..
//  extern "C" {
//     typedef void *(*CallbackFunction)(void*);
//  }
//
//  void *executeWithArgument(CallbackFunction funcPtr, void *argument)
//  {
//     return funcPtr(argument);
//  }
//..
// In this example, we want to use this interface to invoke a C++-style
// functor.  Our approach will be to use
// 'bslmt_EntryPointFunctorAdapter_invoker' as the C-linkage callback function,
// and a dynamically allocated value of 'EntryPointFunctorAdapter' as the
// 'void*' argument.
//
// First, we define a C++ functor type.  This type implements the job of
// counting the number of words in a string held by value.
//..
//  class WordCountJob {
//      // DATA
//      bsl::string  d_message;
//      int         *d_result_p; // held, not owned
//
//    public:
//      // TRAITS
//      BSLMF_NESTED_TRAIT_DECLARATION(WordCountJob,
//                                     bslma::UsesBslmaAllocator);
//
//      // CREATORS
//      WordCountJob(const bslstl::StringRef&  message,
//                   int                      *result,
//                   bslma::Allocator         *basicAllocator = 0);
//          // Create a new functor that, upon execution, counts the number of
//          // words (contiguous sequences of non-space characters) in the
//          // specified 'message' and stores the count in the specified
//          // 'result' address.  Optionally specify a 'basicAllocator' used to
//          // supply memory.  If 'basicAllocator' is 0, the currently
//          // installed default allocator is used.
//
//      WordCountJob(const WordCountJob&  original,
//                   bslma::Allocator    *basicAllocator = 0);
//          // Create a new functor that performs the same calculation as the
//          // specified 'other' functor.  Optionally specify a
//          // 'basicAllocator' used to supply memory.  If 'basicAllocator' is
//          // 0, the currently installed default allocator is used.
//
//      // MANIPULATORS
//      void operator()();
//          // Count the number of words in the message and store the count in
//          // the address specified on construction.
//  };
//
//  inline
//  WordCountJob::WordCountJob(const bslstl::StringRef&  message,
//                             int                      *result,
//                             bslma::Allocator         *basicAllocator)
//  : d_message(message, basicAllocator)
//  , d_result_p(result)
//  {}
//
//  inline
//  WordCountJob::WordCountJob(const WordCountJob&  original,
//                             bslma::Allocator    *basicAllocator)
//  : d_message(original.d_message, basicAllocator)
//  , d_result_p(original.d_result_p)
//  {}
//
//  void WordCountJob::operator()()
//  {
//      bool inWord = false;
//      *d_result_p = 0;
//      for (unsigned i = 0; i < d_message.length(); ++i) {
//          if (isspace(d_message[i])) {
//              inWord = false;
//          } else if (!inWord) {
//              inWord = true;
//              ++(*d_result_p);
//          }
//      }
//  }
//..
// Next, we dynamically allocate an 'EntryPointFunctorAdapter' wrapping an
// instance of this functor:
//..
//  int result = 0;
//  WordCountJob job("The quick brown fox jumped over the lazy dog.",
//                   &result);
//
//  bslma::ManagedPtr<
//      bslmt::EntryPointFunctorAdapter<WordCountJob> > threadData;
//  bslmt::EntryPointFunctorAdapterUtil::allocateAdapter(&threadData,
//                                                       job,
//                                                       "");
//..
// Finally, we use 'bslmt_EntryPointFunctorAdapter_invoker' to invoke the job
// in the context of a C-linkage function.  Note that
// 'bslmt_EntryPointFunctorAdapter_invoker' will deallocate the adapter object
// and the contained invokable job after executing it, so we must release the
// adapter from memory management via 'ManagedPtr'.  (In general, system APIs
// that register callbacks may fail; newly allocated adapters are loaded into
// 'ManagedPtr' to aid in proper error and exception handling, outside the
// scope of this example.)
//..
//  executeWithArgument(bslmt_EntryPointFunctorAdapter_invoker,
//                      threadData.ptr());
//  threadData.release();
//  assert(9 == result);
//..

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLMT_PLATFORM
#include <bslmt_platform.h>
#endif

#ifndef INCLUDED_BSLMT_THREADUTILIMPL_PTHREAD
#include <bslmt_threadutilimpl_pthread.h>
#endif

#ifndef INCLUDED_BSLMT_THREADUTILIMPL_WIN32
#include <bslmt_threadutilimpl_win32.h>
#endif

#ifndef INCLUDED_BSLALG_CONSTRUCTORPROXY
#include <bslalg_constructorproxy.h>
#endif

#ifndef INCLUDED_BSLMA_ALLOCATOR
#include <bslma_allocator.h>
#endif

#ifndef INCLUDED_BSLMA_DEFAULT
#include <bslma_default.h>
#endif

#ifndef INCLUDED_BSLMA_MANAGEDPTR
#include <bslma_managedptr.h>
#endif

#ifndef INCLUDED_BSLMA_RAWDELETERGUARD
#include <bslma_rawdeleterguard.h>
#endif

#ifndef INCLUDED_BSL_STRING
#include <bsl_string.h>
#endif

namespace BloombergLP {

extern "C" {

void *bslmt_EntryPointFunctorAdapter_invoker(void* argument);
    // Interpreting 'argument' as an 'EntryPointFunctorAdapter_Base*', invoke
    // 'argument->function(argument)'.  Do not use outside this component.

}

namespace bslmt {

template <class THREAD_POLICY>
struct ThreadUtilImpl;

struct EntryPointFunctorAdapterUtil;

class EntryPointFunctorAdapter_Base {
    // This component-private type provides a non-templated view of
    // 'EntryPointFunctorAdapter' for accessing the invoker function.  Do not
    // use outside this component.

  public:
    // PUBLIC TYPES
    typedef void (*InvokerFunction)(void*);
        // 'InvokerFunction' is an alias for the type of function pointer held
        // in an EntryPointFunctorAdapter_Base.  Instances of the function are
        // intended to interpret their argument as an
        // 'EntryPointFunctorAdapter<TYPE>*'.

  private:
    InvokerFunction d_function; // Function to operate on template object

  protected:
    // PROTECTED CREATORS
    explicit EntryPointFunctorAdapter_Base(InvokerFunction function);
        // Create a new object holding the specified 'function'.

  public:
    // PUBLIC ACCESSORS
    InvokerFunction function() const;
        // Return the function supplied at construction.
};

template <typename TYPE>
class EntryPointFunctorAdapter : private EntryPointFunctorAdapter_Base {
    // Hold a copy of an instance of parameterized type, along with the
    // allocator used to manage the copy.  'TYPE' shall have a copy
    // constructor, and declare the 'bslma::UsesBslmaAllocator' trait if it
    // uses 'bslma::Allocator' as an argument to its copy constructor.

    // DATA
    bslalg::ConstructorProxy<TYPE> d_functor;
    bsl::string                    d_threadName;

    // FRIENDS
    friend struct EntryPointFunctorAdapterUtil;

  private:
    // NOT IMPLEMENTED
    EntryPointFunctorAdapter(const EntryPointFunctorAdapter&);
    EntryPointFunctorAdapter& operator=(const EntryPointFunctorAdapter&);

    // PRIVATE CREATORS
    EntryPointFunctorAdapter(const TYPE&               functor,
                             const bslstl::StringRef&  threadName,
                             bslma::Allocator         *allocator);
        // Create a new managed object holding copies of the specified
        // 'functor' and 'threadName' values and using the specified
        // 'allocator' to supply memory.

    // PRIVATE CLASS METHODS
    static void invokerFunction(void *adapter);
        // Interpreting the specified 'adapter' as an
        // 'EntryPointFunctorAdapter<TYPE>*', invoke 'd_object' and then
        // deallocate 'adapter' using the allocator used by '*adapter'.

    // PRIVATE MANIPULATORS
    TYPE& functor();
        // Return a reference to the functor.

  public:
    // ~EntryPointFunctorAdapter() = default;
        // Destroy this object and the underlying managed object.  Note that
        // this public destructor is generated by the compiler.
};

struct EntryPointFunctorAdapterUtil {

    // CLASS METHODS
    template<typename TYPE>
    static void allocateAdapter(
       bslma::ManagedPtr<EntryPointFunctorAdapter<TYPE> > *adapter,
       const TYPE&                                         invokable,
       const bslstl::StringRef&                            threadName,
       bslma::Allocator                                   *basicAllocator = 0);
        // Allocate a new 'EntryPointFunctorAdapter' holding copies of the
        // specified 'invokable' object and 'threadName', and load the result
        // into the specified 'adapter'.  Optionally specify a 'basicAllocator'
        // used to supply memory.  If 'basicAllocator' is 0, the currently
        // installed default allocator is used.
};

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

                    // -----------------------------------
                    // class EntryPointFunctorAdapter_Base
                    // -----------------------------------

// PROTECTED CREATORS
inline
EntryPointFunctorAdapter_Base::EntryPointFunctorAdapter_Base(
                                                      InvokerFunction function)
: d_function(function)
{}

// PUBLIC ACCESSORS
inline
EntryPointFunctorAdapter_Base::InvokerFunction
EntryPointFunctorAdapter_Base::function() const
{
    return d_function;
}

                      // ------------------------------
                      // class EntryPointFunctorAdapter
                      // ------------------------------

// PRIVATE CLASS METHODS
template <typename TYPE>
inline
void EntryPointFunctorAdapter<TYPE>::invokerFunction(void *adapterRaw)
{
    EntryPointFunctorAdapter<TYPE> *adapter =
                      static_cast<EntryPointFunctorAdapter<TYPE>*>(adapterRaw);

    bslma::Allocator *a = adapter->d_threadName.get_allocator().mechanism();
    bslma::RawDeleterGuard<EntryPointFunctorAdapter<TYPE>,
                           bslma::Allocator> adapterGuard(adapter, a);

    if (false == adapter->d_threadName.empty()) {
        ThreadUtilImpl<Platform::ThreadPolicy>::setThreadName(
                                                        adapter->d_threadName);
    }

    adapter->functor()();
}

// PRIVATE CREATORS
template <typename TYPE>
inline
EntryPointFunctorAdapter<TYPE>::EntryPointFunctorAdapter(
                                          const TYPE&               functor,
                                          const bslstl::StringRef&  threadName,
                                          bslma::Allocator         *allocator)
: EntryPointFunctorAdapter_Base(&invokerFunction)
, d_functor(functor, allocator)
, d_threadName(threadName, allocator)
{}

// PRIVATE MANIPULATORS
template <typename TYPE>
inline
TYPE& EntryPointFunctorAdapter<TYPE>::functor()
{
    return d_functor.object();
}

                    // ----------------------------------
                    // class EntryPointFunctorAdapterUtil
                    // ----------------------------------

template <typename TYPE>
inline
void EntryPointFunctorAdapterUtil::allocateAdapter(
          bslma::ManagedPtr<EntryPointFunctorAdapter<TYPE> > *adapter,
          const TYPE&                                         invokable,
          const bslstl::StringRef&                            threadName,
          bslma::Allocator                                   *basicAllocator)
{
    bslma::Allocator *allocator = bslma::Default::allocator(basicAllocator);
    adapter->load(new (*allocator) EntryPointFunctorAdapter<TYPE>(invokable,
                                                                  threadName,
                                                                  allocator),
                  allocator);
}

}  // close package namespace
}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2015 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
