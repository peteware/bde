// bdlt_hashtable.h                                                    -*-C++-*-
#ifndef INCLUDED_BDLT_HASHTABLE
#define INCLUDED_BDLT_HASHTABLE

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: 
//
//@CLASSES:
//  bdlt::Hashtable: combined date and time value (millisecond resolution)
//
//@SEE_ALSO: 
//
//@DESCRIPTION: 

///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: 
///- - - - - - - - - - - -

#ifndef INCLUDED_BSLS_ATOMIC
#include <bsls_atomic.h>
#endif

#ifndef INCLUDED_BSLMA_DEFAULT
#include <bslma_default.h>
#endif

#ifndef INCLUDED_BSL_CSTDDEF
#include <bsl_cstddef.h>
#endif

#ifndef INCLUDED_BSL_UTILITY
#include <bsl_utility.h>
#endif

namespace BloombergLP {
namespace bdlt {

                        // ==============
                        // class Hashtable
                        // ==============

template <class KEY, class VALUE, class HASHER, class EQUALITY>
class Hashtable {

    typedef bsl::pair<KEY, VALUE>        Element;
    typedef bsls::AtomicPointer<Element> ElementPtr;        


    
    ElementPtr       *d_buckets_p;
    bsl::size_t       d_numBuckets;
    bsl::size_t       d_numElements;
    bslma::Allocator *d_allocator_p;


  public:

    Hashtable(int maxNumElements) 
    : d_buckets_p(0)
    , d_numElements(0)
    , d_allocator_p(bslma::Default(0))
    , d_numBuckets(maxNumElements)
    {
        d_buckets_p = new (*d_allocator_p) ElementPtr[d_numBuckets];
    }

};

// ============================================================================
//                            INLINE DEFINITIONS
// ============================================================================

                               // --------------
                               // class Hashtable
                               // --------------
}  // close namespace bdlt
}  // close namespace BloombergLP

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
