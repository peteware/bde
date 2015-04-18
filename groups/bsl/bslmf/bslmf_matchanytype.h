// bslmf_matchanytype.h                                               -*-C++-*-
#ifndef INCLUDED_BSLMF_MATCHANYTYPE
#define INCLUDED_BSLMF_MATCHANYTYPE

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a generic type to which any type can be converted.
//
//@CLASSES:
//  bslmf::MatchAnyType: generic type to which any type can be converted
//  bslmf::TypeRep: meta-function for providing a reference to 'TYPE'
//
//@DESCRIPTION: 'bslmf::MatchAnyType' is a type to which any type can be
// implicitly converted.  This is useful for creating an overloaded function
// that is a catch-all for all types not explicitly provided for in other
// overloaded functions with the same name.
//
// 'bslmf::TypeRep' allows one to create a reference to a type.  In complex
// template programming, one is often dealing with unknown types, about the
// constructors of which one knows nothing.  One often needs an object of the
// given type, but since nothing is known about the constructors, one can't
// just construct and object of the type.  'bslmf::TypeRep' allows one to
// create a reference to the type.  Note that the 'rep' function in
// 'bslmf::TypeRep' is not implemented, it must never be called at run time.
//
///Usage Example 1: 'bslmf::MatchAnyType'
///--------------------------------------
//..
//  struct X { };
//  struct Y { };
//  struct Z : public Y { };
//
//  inline bool isY(const bslmf::MatchAnyType&) { return false; }
//  inline bool isY(const Y&)              { return true;  }
//
//  assert(! isY(X()));
//  assert(  isY(Y()));
//  assert(  isY(Z()));
//  assert(! isY(int()));
//  assert(! isY(4));
//  assert(! isY(4.0));
//  assert(! isY("The king is a fink!"));
//
//  X x;
//  Y y;
//  Z z;
//  assert(! isY(x));
//  assert(  isY(y));
//  assert(  isY(z));
//..
///Usage Example 2: 'bslmf::TypeRep'
///--------------------------------
//..
//  struct X {};
//  struct Y {};
//
//  struct HasHorridCtorX : public X {
//      HasHorridCtorX(int, double, X, char, char *, void *) {}
//          // It's inconvenient to actually create an object of this type
//          // because the constructor takes so many arguments.  It's also
//          // impossible because the c'tor is undefined.
//  };
//  struct HasHorridCtorY : public Y {
//      HasHorridCtorY(int, double, X, char, char *, void *) {}
//          // It's inconvenient to actually create an object of this type
//          // because the constructor takes so many arguments.  It's also
//          // impossible because the c'tor is undefined.
//  };
//
//  template <int i>
//  struct MetaInt { char d_array[i + 1]; };
//
//  #define METAINT_TO_UINT(metaint)   (sizeof(metaint) - 1)
//
//  MetaInt<1> isX(const X&);
//  MetaInt<0> isX(const bslmf::MatchAnyType&);
//
//  assert(1 == METAINT_TO_UINT(isX(X())));
//  assert(0 == METAINT_TO_UINT(isX(Y())));
//  assert(1 == METAINT_TO_UINT(isX(bslmf::TypeRep<HasHorridCtorX>::rep())));
//  assert(0 == METAINT_TO_UINT(isX(bslmf::TypeRep<HasHorridCtorY>::rep())));
//  assert(0 == METAINT_TO_UINT(isX(3)));
//  assert(0 == METAINT_TO_UINT(isX(3.0)));
//  assert(0 == METAINT_TO_UINT(isX("The king is a fink!")));
//..

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLMF_ENABLEIF
#include <bslmf_enableif.h>
#endif

#ifndef INCLUDED_BSLMF_INTEGRALCONSTANT
#include <bslmf_integralconstant.h>
#endif

#ifndef INCLUDED_BSLMF_ISFUNCTION
#include <bslmf_isfunction.h>
#endif

namespace BloombergLP {

namespace bslmf {

                        // ==================
                        // class MatchAnyType
                        // ==================

struct MatchAnyType {
    // Any type can be converted into this type.

    template <class TYPE> MatchAnyType(const TYPE&) { }             // IMPLICIT
        // This constructor will match any rvalue or any non-volatile lvalue.
        // A non-const version of this constructor is not necessary and will
        // cause some compilers to complain of ambiguities.

    template <class TYPE> MatchAnyType(const volatile TYPE&) { }    // IMPLICIT
        // This constructor will match any volatile lvalue.  According to the
        // standard, it should NOT match an rvalue.  A non-const version of
        // this constructor is not necessary and will cause some compilers to
        // complain of ambiguities.

    template <class TYPE>
    MatchAnyType(TYPE&,
            typename bsl::enable_if<bsl::is_function<TYPE>::value>::type * = 0)
    { }    // IMPLICIT
        // This overload is required for some compilers that will not bind
        // function types to cv-qualified references, but must be excluded from
        // the overload set unless the parameterised 'TYPE' is a function type.
        // The list of "some compilers" is currently known to include IBM xlC
        // and Sun CC, last tested with version xlC 12.1 and Sun CC 12.3.
};

                        // =============
                        // class TypeRep
                        // =============

template <class TYPE>
struct TypeRep {
    // Generate a reference to 'TYPE' for use in meta-functions.

    static TYPE& rep();
        // Provide a reference to a 'TYPE' object.  This function has no body
        // and must never be called at run time.  Thus, it does not matter if
        // 'TYPE' has a default constructor or not.
};

template <class TYPE>
struct TypeRep<TYPE&> {
    // Generate a reference to 'TYPE' for use in meta-functions.  This is a
    // partial specialization of 'TypeRep' instantiated with a reference.

    static TYPE& rep();
        // Provide a reference to a 'TYPE' object.  This function has no body
        // and must never be called at run time.  Thus, it does not matter if
        // 'TYPE' has a default constructor or not.
};

}  // close package namespace


}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2013 Bloomberg Finance L.P.
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
