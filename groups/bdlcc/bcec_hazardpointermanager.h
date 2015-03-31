// bcec_hazardpointermanager.h                                         -*-C++-*-
#ifndef INCLUDED_BCEC_HAZARDPOINTERMANAGER
#define INCLUDED_BCEC_HAZARDPOINTERMANAGER

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: 
//
//@CLASSES:
//  bcec_HazardPointerManager:
//
//@AUTHOR: Henry Verschell (hverschell)
//
//@SEE_ALSO:
//
//@DESCRIPTION:
//
///Usage
///-----
//..

#ifndef INCLUDED_BCESCM_VERSION
#include <bcescm_version.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

#ifndef INCLUDED_BSLS_ATOMIC
#include <bsls_atomic.h>
#endif

#ifndef INCLUDED_BSLS_TYPES
#include <bsls_types.h>
#endif

#ifndef INCLUDED_BSL_UNORDEREDSET
#include <bsl_unordered_set.h>
#endif



namespace BloombergLP {

                     // ===============================
                     // class bcec_HazardPointerManager
                     // ===============================

class bcec_HazardPointerManager {


  public:    

    static bsls::AtomicOperations::AtomicTypes::Pointer *acquireHazardPointer();

    static void releaseHazardPointer(
                  bsls::AtomicOperations::AtomicTypes::Pointer *hazardPointer);

    static void loadHazardPointers(bsl::unordered_set<void *> *result);
};


// ============================================================================
//                      INLINE FUNCTION DEFINITIONS
// ============================================================================



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
