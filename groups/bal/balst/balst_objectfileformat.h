// balst_objectfileformat.h                                           -*-C++-*-

// ----------------------------------------------------------------------------
//                                   NOTICE
//
// This component is not up to date with current BDE coding standards, and
// should not be used as an example for new development.
// ----------------------------------------------------------------------------

#ifndef INCLUDED_BALST_OBJECTFILEFORMAT
#define INCLUDED_BALST_OBJECTFILEFORMAT

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide platform-dependent object file format trait definitions.
//
//@CLASSES:
//   balst::ObjectFileFormat: namespace for object file format traits
//
//@SEE_ALSO:
//
//@DESCRIPTION: This component defines a set of traits that identify and
// describe a platform's object file format properties.  For example, the
// 'balst::ObjectFileFormat::ResolverPolicy' trait is ascribed a "value" (i.e.,
// 'Elf' or 'Xcoff') appropriate for each supported platform.  The various
// stack trace traits are actually types declared in the
// 'bdescu_ObjectFileFormat' 'struct'.  These types are intended to be used in
// specializing template implementations or to enable function overloading
// based on the prevalent system's characteristics.  #defines are also
// provided by this component to facilitate conditional compilation depending
// upon object file formats.
//
///DWARF Information
///-----------------
// DWARF is a format for detailed debugging information.  It is not a complete
// format, but is used within other formats.  It is used within ELF on Linux,
// but not (yet) on Solaris at Bloomberg (currently the ELF format on Solaris
// still uses STABS).  It is used within the Mach-O format (also known as the
// 'Dladdr' format in this file) used on Darwin.  It is also used by the Clang
// compiler (which uses ELF).
//
// For all these platforms, parsing the DWARF information is necessary for the
// stack trace to get source file names and line numbers (the ELF format gives
// source file names, but only in the case of file-scope static functions).
//
// DWARF is implemented for g++ versions earlier than 7.1.0 on Linux.
//
///Implementation Note
///- - - - - - - - - -
// Linux g++ 7.1.0 uses DWARF version 4, while g++ 5.4.0 and before use DWARF
// version 3.  At the moment the required system header, 'dwarf.h', is not
// available in the Bloomberg production build 'chroot' environment, so
// support for dwarf formats is disabled.
//
// DWARF support on Clang is problematic and not currrently implemented, see
// the long comment in balst_stacktraceresolverimpl_elf.cpp, which explains
// exactly how it could be implemented when that becomes a priority.
//
// We have not yet investigated implementing DWARF for Dladdr (Darwin).
//
///Usage
///-----
// In this section we show the intended usage of this component.
//
///Example 1: Accessing 'balst::ObjectFileFormat' Information at Run Time
/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// The templated (specialized) 'typeTest' function returns a unique, non-zero
// value when passed an object of types
// 'balst::ObjectFileFormat::{Elf,Xcoff,Windows}', and 0 otherwise.
//..
//  template <typename TYPE>
//  int typeTest(const TYPE &)
//  {
//      return 0;
//  }
//
//  int typeTest(const balst::ObjectFileFormat::Elf &)
//  {
//      return 1;
//  }
//
//  int typeTest(const balst::ObjectFileFormat::Xcoff &)
//  {
//      return 2;
//  }
//
//  int typeTest(const balst::ObjectFileFormat::Windows &)
//  {
//      return 3;
//  }
//
//  int main() ...
//..
// We define an object 'policy' of type 'balst::ObjectFileFormat::Policy',
// which will be of type '...::Elf', '...::Xcoff', or '...::Windows'
// appropriate for the platform.
//..
//      balst::ObjectFileFormat::Policy policy;
//..
// We now test it using 'typeTest':
//..
//      assert(typeTest(policy) > 0);
//
//  #if defined(BALST_OBJECTFILEFORMAT_RESOLVER_ELF)
//      assert(1 == typeTest(policy));
//  #endif
//
//  #if defined(BALST_OBJECTFILEFORMAT_RESOLVER_XCOFF)
//      assert(2 == typeTest(policy));
//  #endif
//
//  #if defined(BALST_OBJECTFILEFORMAT_RESOLVER_WINDOWS)
//      assert(3 == typeTest(policy));
//  #endif
//  }
//..

#ifndef INCLUDED_BALSCM_VERSION
#include <balscm_version.h>
#endif

#ifndef INCLUDED_BSLS_PLATFORM
#include <bsls_platform.h>
#endif

namespace BloombergLP {

namespace balst {
                           // ======================
                           // class ObjectFileFormat
                           // ======================

struct ObjectFileFormat {
    // This 'struct' is named 'ObjectFileFormat' for historical reasons, what
    // it really determines is resolving strategy.  Linux, for example, can be
    // resolved using either the 'Elf' or 'Dladdr' policies.  We choose 'Elf'
    // for linux because that mode of resolving yields more information.

    struct Elf {};        // resolve as elf object

    struct Xcoff {};      // resolve as xcoff object

    struct Windows {};    // format used on Microsoft Windows platform

    struct Dladdr {};     // resolve using the 'dladdr' call

    struct Dummy {};

#if defined(BSLS_PLATFORM_OS_SOLARIS) || \
    defined(BSLS_PLATFORM_OS_LINUX)   || \
    defined(BSLS_PLATFORM_OS_HPUX)

    typedef Elf Policy;
#   define BALST_OBJECTFILEFORMAT_RESOLVER_ELF 1

# if defined(BSLS_PLATFORM_OS_LINUX) && defined(BSLS_PLATFORM_CMP_GNU)        \
    && BSLS_PLATFORM_CMP_VERSION < 70100
    // DWARF support is implemented only for Linux g++ < 7.1.0.

#   define BALST_OBJECTFILEFORMAT_RESOLVER_DWARF 1
# endif

#elif defined(BSLS_PLATFORM_OS_AIX)

    typedef Xcoff Policy;
#   define BALST_OBJECTFILEFORMAT_RESOLVER_XCOFF 1

#elif defined(BSLS_PLATFORM_OS_WINDOWS)

    typedef Windows Policy;
#   define BALST_OBJECTFILEFORMAT_RESOLVER_WINDOWS 1

#elif defined(BSLS_PLATFORM_OS_DARWIN)

    typedef Dladdr Policy;
#   define BALST_OBJECTFILEFORMAT_RESOLVER_DLADDR 1

#else

    typedef Dummy Policy;
#   error unrecognized platform
#   define BALST_OBJECTFILEFORMAT_RESOLVER_UNIMPLEMENTED 1

#endif

};
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
