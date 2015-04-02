// bcec_hazardpointermanager.h                                        -*-C++-*-
#include <bcec_hazardpointermanager.h>

#include <bdes_ident.h>
BDES_IDENT_RCSID(bcec_hazardpointermanager_cpp,"$Id$ $CSID$")

#include <bslmf_assert.h>
#include <bslma_newdeleteallocator.h>
#include <bsls_assert.h>
#include <bsls_atomic.h>
#include <bsls_atomicoperations.h>

#include <bsl_iostream.h>

namespace BloombergLP {


struct HazardPointerRecord {
    bsls::AtomicOperations::AtomicTypes::Pointer d_ptr;
    bsls::AtomicPointer<HazardPointerRecord>     d_next;
    bsls::AtomicInt                              d_active;            
};

static bsls::AtomicPointer<HazardPointerRecord> g_hazardPtrListHead(0);

                      // -------------------------------
                      // class bcec_HazardPointerManager
                      // -------------------------------

// CLASS METHODS
bsls::AtomicOperations::AtomicTypes::Pointer *
bcec_HazardPointerManager::acquireHazardPointer()
{
    HazardPointerRecord *record = g_hazardPtrListHead.loadAcquire();
    
    while (record) {
        if (0 == record->d_active.testAndSwap(0, 1)) {
            BSLS_ASSERT_OPT(reinterpret_cast<void *>(&record->d_ptr) == 
                            reinterpret_cast<void *>(record));
            return &record->d_ptr;
        }
        record = record->d_next.loadAcquire();
    }

    record = new (bslma::NewDeleteAllocator::singleton()) HazardPointerRecord;
    record->d_active.storeRelaxed(1);
    bsls::AtomicOperations::setPtrRelaxed(&record->d_ptr, 0);
    
    HazardPointerRecord *prevHead; 
    HazardPointerRecord *head = g_hazardPtrListHead.loadRelaxed();
    do {
        record->d_next.storeRelaxed(head);
        prevHead = head;
        head     = g_hazardPtrListHead.testAndSwapAcqRel(head, record);
    } while (prevHead != head);

    BSLS_ASSERT_OPT(reinterpret_cast<void *>(&record->d_ptr) == 
                    reinterpret_cast<void *>(record));
    return &record->d_ptr;
}


void bcec_HazardPointerManager::releaseHazardPointer(
                  bsls::AtomicOperations::AtomicTypes::Pointer *hazardPointer)
{
    HazardPointerRecord *record = 
        reinterpret_cast<HazardPointerRecord *>(hazardPointer);
    bsls::AtomicOperations::setPtrRelaxed(&record->d_ptr, 0);
    record->d_active.storeRelaxed(0);

}

void bcec_HazardPointerManager::loadHazardPointers(
                                             bsl::unordered_set<void *> *result)
{
    HazardPointerRecord *record = g_hazardPtrListHead.loadAcquire();
    while (record) {
        void *hazardPointer =
                       bsls::AtomicOperations::getPtrRelaxed(&record->d_ptr);

        if (0 != hazardPointer) {
            result->insert(hazardPointer);
        }
        record = record->d_next.loadAcquire();
    }
}


}  // close enterprise namespace


// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2013
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------

