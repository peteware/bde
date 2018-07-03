// bsl_locale.h                                                       -*-C++-*-
#ifndef INCLUDED_BSL_LOCALE
#define INCLUDED_BSL_LOCALE

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide functionality of the corresponding C++ Standard header.
//
//@SEE_ALSO: package bsl+stdhdrs
//
//@DESCRIPTION: Provide types, in the 'bsl' namespace, equivalent to those
// defined in the corresponding C++ standard header.  Include the native
// compiler-provided standard header, and also directly include Bloomberg's
// implementation of the C++ standard type (if one exists).  Finally, place the
// included symbols from the 'std' namespace (if any) into the 'bsl' namespace.

#ifndef INCLUDED_BSLS_LIBRARYFEATURES
#include <bsls_libraryfeatures.h>
#endif

#ifndef INCLUDED_BSLS_NATIVESTD
#include <bsls_nativestd.h>
#endif

#include <locale>

namespace bsl {
    // Import selected symbols into bsl namespace

    using native_std::codecvt;
    using native_std::codecvt_base;
    using native_std::codecvt_byname;
    using native_std::collate;
    using native_std::collate_byname;
    using native_std::ctype;
    using native_std::ctype_base;
    using native_std::ctype_byname;
    using native_std::has_facet;
    using native_std::isalnum;
    using native_std::isalpha;
    using native_std::iscntrl;
    using native_std::isdigit;
    using native_std::isgraph;
    using native_std::islower;
    using native_std::isprint;
    using native_std::ispunct;
    using native_std::isspace;
    using native_std::isupper;
    using native_std::isxdigit;
    using native_std::locale;
    using native_std::messages;
    using native_std::messages_base;
    using native_std::messages_byname;
    using native_std::money_base;
    using native_std::money_get;
    using native_std::money_put;
    using native_std::moneypunct;
    using native_std::moneypunct_byname;
    using native_std::num_get;
    using native_std::num_put;
    using native_std::numpunct;
    using native_std::numpunct_byname;
    using native_std::time_base;
    using native_std::time_get;
    using native_std::time_get_byname;
    using native_std::time_put;
    using native_std::time_put_byname;
    using native_std::tolower;
    using native_std::toupper;
    using native_std::use_facet;

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP11_MISCELLANEOUS_UTILITIES
    using native_std::isblank;
#endif  // BSLS_LIBRARYFEATURES_HAS_CPP11_MISCELLANEOUS_UTILITIES

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP11_MISCELLANEOUS_UTILITIES
    using native_std::wstring_convert;
    using native_std::wbuffer_convert;
#endif  // BSLS_LIBRARYFEATURES_HAS_CPP11_MISCELLANEOUS_UTILITIES

}  // close package namespace

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
