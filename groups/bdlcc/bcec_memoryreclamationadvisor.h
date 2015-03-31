// bcec_memoryreclamationadvisor.h                                     -*-C++-*-
#ifndef INCLUDED_BCEC_MEMORYRECLAMATIONADVISOR
#define INCLUDED_BCEC_MEMORYRECLAMATIONADVISOR

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide a mechanism to determine if memory can be safely reclaimed.
//
//@CLASSES:
//  bcec_MemoryReclamationAdvisor: determines if memory can be safely reclaimed
//
//@AUTHOR: Henry Verschell (hverschell)
//
//@SEE_ALSO:
//
//@DESCRIPTION: This component provides a wait-free mechanism for ensuring the
// safe reclamation of resources used by a non-blocking (lock-free) algorithm.
// The reclamation of resources of resources is a practical issue for many
// lock-free algorithms 
//
///Usage
///-----
//..

#ifndef INCLUDED_BCESCM_VERSION
#include <bcescm_version.h>
#endif

#ifndef INCLUDED_BSLS_TYPES
#include <bsls_types.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

namespace BloombergLP {

                     // ============================
                     // class bcec_MemoryReclamation
                     // ============================

class bcec_MemoryReclamationGuard {
  public:
     bcec_MemoryReclamationGuard();
    ~bcec_MemoryReclamationGuard();
        
};

                     // ===================================
                     // class bcec_MemoryReclamationAdvisor
                     // ===================================

class bcec_MemoryReclamationAdvisor {


  public:
    static bsls::Types::Uint64 acquireState();

    static bsls::Types::Uint64 releaseState();

    static bsls::Types::Uint64 updateState();

    static bsls::Types::Uint64 oldestActiveState();

    static bsls::Types::Uint64 currentThreadActiveState();
};


// ============================================================================
//                      INLINE FUNCTION DEFINITIONS
// ============================================================================

inline
bcec_MemoryReclamationGuard::bcec_MemoryReclamationGuard()
{
    bcec_MemoryReclamationAdvisor::acquireState();
}

inline
bcec_MemoryReclamationGuard::~bcec_MemoryReclamationGuard()
{
    bcec_MemoryReclamationAdvisor::releaseState();
}


}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2013
//      All Rights Reserved.
//      Property of Bloomberg L.P.  (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ----------------------------------
