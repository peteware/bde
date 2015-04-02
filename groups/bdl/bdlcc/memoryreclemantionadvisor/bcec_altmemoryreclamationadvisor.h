// bcec_altmemoryreclamationadvisor.h                                  -*-C++-*-
#ifndef INCLUDED_BCEC_ALTMEMORYRECLAMATIONADVISOR
#define INCLUDED_BCEC_ALTMEMORYRECLAMATIONADVISOR

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide a mechanism to determine if memory can be safely reclaimed.
//
//@CLASSES:
//  bcec_AltMemoryReclamationAdvisor: determines if memory can be safely reclaimed
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
                     // class bcec_AltMemoryReclamation
                     // ============================

class bcec_AltMemoryReclamationGuard {
     void *d_state;

  public:
     bcec_AltMemoryReclamationGuard();
    ~bcec_AltMemoryReclamationGuard();
        
};

                     // ===================================
                     // class bcec_AltMemoryReclamationAdvisor
                     // ===================================

class bcec_AltMemoryReclamationAdvisor {


  public:
    static void *acquireState();

    static void releaseState(void *state);

    static bsls::Types::Uint64 updateState();

    static bsls::Types::Uint64 oldestActiveState();
};


// ============================================================================
//                      INLINE FUNCTION DEFINITIONS
// ============================================================================

inline
bcec_AltMemoryReclamationGuard::bcec_AltMemoryReclamationGuard()
: d_state(bcec_AltMemoryReclamationAdvisor::acquireState())
{
    
}

inline
bcec_AltMemoryReclamationGuard::~bcec_AltMemoryReclamationGuard()
{
    bcec_AltMemoryReclamationAdvisor::releaseState(d_state);
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
