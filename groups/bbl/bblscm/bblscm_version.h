// bblscm_version.h                                                   -*-C++-*-
#ifndef INCLUDED_BBLSCM_VERSION
#define INCLUDED_BBLSCM_VERSION

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide source control management (versioning) information.
//
//@CLASSES:
// bblscm::Version: namespace for RCS and SCCS versioning information for 'bbl'
//
//@DESCRIPTION: This component provides source control management (versioning)
// information for the 'bbl' package group.  In particular, this component
// embeds RCS-style and SCCS-style version strings in binary executable files
// that use one or more components from the 'bbl' package group.  This version
// information may be extracted from binary files using common UNIX utilities
// (e.g., 'ident' and 'what').  In addition, the 'version' 'static' member
// function in the 'bblscm::Version' 'struct' can be used to query version
// information for the 'bbl' package group at runtime.  The following usage
// examples illustrate these two basic capabilities.
//
// Note that unless the 'version' method will be called, it is not necessary to
// '#include' this component header file to get 'bbl' version information
// embedded in an executable.  It is only necessary to use one or more 'bbl'
// components (and, hence, link in the 'bbl' library).
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Embedding Version Information
/// - - - - - - - - - - - - - - - - - - - -
// The version of the 'bbl' package group linked into a program can be
// obtained at runtime using the 'version' 'static' member function as follows:
//..
//        #include <bblscm_version.h>
//
//        assert(0 != bblscm::Version::version());
//
//        bsl::cout << "BBL version: " << bblscm::Version::version()
//                  << bsl::endl;
//..
// Output similar to the following will be printed to 'stdout':
//..
//        BBL version: BLP_LIB_BDE_BBL_0.01.0
//..
// The "0.01.0" portion of the string distinguishes different versions of the
// 'bbl' package group.
//
///Example 2: Accessing the Embedded Version information
///- - - - - - - - - - - - - - - - - - - - - - - - - - -
// The versioning information embedded into a binary file by this component can
// be examined under UNIX using several well-known utilities.  For example:
//..
//        $ ident a.out
//        a.out:
//             $Id: BLP_LIB_BDE_BBL_0.01.0 $
//
//        $ what a.out | grep BBL
//                BLP_LIB_BDE_BBL_0.01.0
//
//        $ strings a.out | grep BBL
//        $Id: BLP_LIB_BDE_BBL_0.01.0 $
//        @(#)BLP_LIB_BDE_BBL_0.01.0
//        BLP_LIB_BDE_BBL_0.01.0
//..
// Note that 'ident' and 'what' typically will display many version strings
// unrelated to 'bbl' depending on the libraries used by 'a.out'.

#ifndef INCLUDED_BBLSCM_VERSIONTAG
#include <bblscm_versiontag.h>     // 'BBL_VERSION_MAJOR', 'BBL_VERSION_MINOR'
#endif

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLS_LINKCOERCION
#include <bsls_linkcoercion.h>
#endif

namespace BloombergLP {

namespace bblscm {

                         // ==============
                         // struct Version
                         // ==============

struct Version {
    // This struct provides a namespace for (1) source control management
    // (versioning) information that is embedded in binary executable files,
    // and (2) a facility to query that information at runtime.

    // CLASS DATA
    static const char *s_ident;              // RCS-style version string
    static const char *s_what;               // SCCS-style version string

#define BBLSCM_CONCAT2(a,b,c,d,e,f) a ## b ## c ## d ## e ## f
#define BBLSCM_CONCAT(a,b,c,d,e,f)  BBLSCM_CONCAT2(a,b,c,d,e,f)

// 'BBLSCM_S_VERSION' is a symbol whose name warns users of version mismatch
// linking errors.  Note that the exact string "compiled_this_object" must be
// present in this version coercion symbol.  Tools may look for this pattern to
// warn users of mismatches.
#define BBLSCM_D_VERSION BBLSCM_CONCAT(s_version_BBL_,       \
                                       BBL_VERSION_MAJOR, _, \
                                       BBL_VERSION_MINOR, _, \
                                       compiled_this_object)

    static const char *BBLSCM_S_VERSION;     // BDE-style version string

    static const char *s_dependencies;       // package-group dependency string
    static const char *s_buildInfo;          // available for future use
    static const char *s_timestamp;          // available for future use
    static const char *s_sourceControlInfo;  // available for future use

    // CLASS METHODS
    static const char *version();
        // Return the address of a character string that identifies the version
        // of the 'bbl' package group in use.
};

// ============================================================================
//                            INLINE DEFINITIONS
// ============================================================================

                         // --------------
                         // struct Version
                         // --------------

// CLASS METHODS
inline
const char *Version::version()
{
    return BBLSCM_S_VERSION;
}

}  // close package namespace

BSLS_LINKCOERCION_FORCE_SYMBOL_DEPENDENCY(const char *,
                                          bblscm_version_assertion,
                                          bblscm::Version::BBLSCM_S_VERSION);

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
