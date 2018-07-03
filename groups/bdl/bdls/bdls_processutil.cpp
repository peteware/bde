// bdls_processutil.cpp                                               -*-C++-*-

// ----------------------------------------------------------------------------
//                                   NOTICE
//
// This component is not up to date with current BDE coding standards, and
// should not be used as an example for new development.
// ----------------------------------------------------------------------------

#include <bdls_processutil.h>

#include <bsls_ident.h>
BSLS_IDENT_RCSID(bdls_processutil_cpp,"$Id$ $CSID$")

#include <bdlsb_memoutstreambuf.h>

#include <bsls_assert.h>
#include <bsls_platform.h>

#include <bsl_iostream.h>
#include <bsl_fstream.h>

#ifdef BSLS_PLATFORM_OS_WINDOWS
#include <windows.h>
#include <bdlde_charconvertutf16.h>
#include <bdlma_localsequentialallocator.h>
#else
#include <unistd.h>
#endif

#if defined(BSLS_PLATFORM_OS_SOLARIS)
#include <procfs.h>
#include <fcntl.h>
#ifndef _REENTRANT
#define _REENTRANT
#endif
#elif defined(BSLS_PLATFORM_OS_AIX)
#include <sys/procfs.h>
#include <fcntl.h>
#include <procinfo.h>
#include <bslma_allocator.h>
#include <bslma_deallocatorproctor.h>
#include <bslma_default.h>
#elif defined(BSLS_PLATFORM_OS_LINUX) || defined(BSLS_PLATFORM_OS_CYGWIN)
#include <fcntl.h>
#elif defined(BSLS_PLATFORM_OS_HPUX)
#include <sys/pstat.h>
#elif defined(BSLS_PLATFORM_OS_DARWIN)
#include <libproc.h>
#endif

#if defined BSLS_PLATFORM_OS_AIX
// DRQS 30045663 - AIX is missing this prototype
extern int getargs(void *processBuffer,
                   int   bufferLen,
                   char *argsBuffer,
                   int   argsLen);
#endif


namespace BloombergLP {

namespace bdls {
                             // ------------------
                             // struct ProcessUtil
                             // ------------------

// CLASS METHODS
int ProcessUtil::getProcessId() {
#ifdef BSLS_PLATFORM_OS_WINDOWS
    return static_cast<int>(GetCurrentProcessId());
#else
    return static_cast<int>(getpid());
#endif
}

int ProcessUtil::getProcessName(bsl::string *result)
{
    BSLS_ASSERT(result);

    result->clear();

#if defined BSLS_PLATFORM_OS_WINDOWS
    bdlma::LocalSequentialAllocator<MAX_PATH + 1> la;
    bsl::wstring wResult(MAX_PATH, 0, &la);
    while (wResult.length() <= 4 * MAX_PATH) {
        DWORD length = GetModuleFileNameW(0, &wResult[0], wResult.length());
        if (length <= 0) {  // Error
            return 1;                                                 // RETURN
        }
        else if (length < wResult.length()) {  // Success
            wResult.resize(length);
            return bdlde::CharConvertUtf16::utf16ToUtf8(result, wResult);
                                                                      // RETURN
        }
        else {  // Not enough space for the process name in 'wResult'
            wResult.resize(wResult.length() * 2); // Make more space
        }
    }

    return -1;
#else
# if defined BSLS_PLATFORM_OS_HPUX
    result->resize(256);
    int rc = pstat_getcommandline(&(*result->begin()),
                                  result->size(), 1,
                                  getpid());
    if (rc < 0)
    {
        return -1;
    }

    bsl::string::size_type pos = result->find_first_of(' ');
    if (bsl::string::npos != pos) {
        result->resize(pos);
    }
# elif defined BSLS_PLATFORM_OS_LINUX || defined BSLS_PLATFORM_OS_CYGWIN ||   \
       defined BSLS_PLATFORM_OS_SOLARIS
    enum { NUM_ELEMENTS = 14 + 16 };  // "/proc/<pid>/cmdline"

    bdlsb::MemOutStreamBuf osb(NUM_ELEMENTS);
    bsl::ostream          os(&osb);
    os << "/proc/" << getpid() << "/cmdline" << bsl::ends;
    const char *procfs = osb.data();

    bsl::ifstream ifs;
    ifs.open(procfs, bsl::ios_base::in | bsl::ios_base::binary);

    if (ifs.fail()) {
        return -1;                                                    // RETURN
    }

    ifs >> *result;

    bsl::string::size_type pos = result->find_first_of('\0');
    if (bsl::string::npos != pos) {
        result->resize(pos);
    }
# elif defined BSLS_PLATFORM_OS_DARWIN
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
    if (proc_pidpath (getpid(), pathbuf, sizeof(pathbuf)) <= 0) {
        return -1;
    }
    result->assign(pathbuf);
# else
#  if defined BSLS_PLATFORM_OS_AIX
    struct procentry64  procbuffer;
    static const int    argslen = 65535;
    bslma::Allocator   *allocator = bslma::Default::allocator(0);

    char *argsbuffer = static_cast<char *>(allocator->allocate(argslen));
    bslma::DeallocatorProctor<bslma::Allocator> proctor(argsbuffer, allocator);

    procbuffer.pi_pid = getpid();

    if (getargs(&procbuffer, sizeof(procbuffer), argsbuffer, argslen) < 0 ) {
        return -1;
    }

    if (bsl::find(argsbuffer, argsbuffer + argslen, '\0')
                                                     == argsbuffer + argslen) {
        // Process name is longer than 65K, and it was truncated.
        return -2;                                                    // RETURN
    }

    result->assign(argsbuffer);
    return 0;
#  endif
# endif
#endif
    return result->empty();
}
}  // close package namespace

}  // close enterprise namespace

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
