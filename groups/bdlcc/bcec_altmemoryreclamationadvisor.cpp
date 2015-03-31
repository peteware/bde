// bcec_altmemoryreclamationadvisor.h                                     -*-C++-*-
#include <bcec_altmemoryreclamationadvisor.h>

#include <bdes_ident.h>
BDES_IDENT_RCSID(bcec_altmemoryreclamationadvisor_cpp,"$Id$ $CSID$")

#include <bslmf_assert.h>
#include <bslma_newdeleteallocator.h>
#include <bsls_assert.h>
#include <bsls_atomic.h>

#include <bsl_iostream.h>

namespace BloombergLP {

                      // -----------------------------------
                      // class bcec_AltMemoryReclamationAdvisor
                      // -----------------------------------

static const bsls::Types::Uint64 NON_ACTIVE = -1;

struct StateNode {
    bsls::AtomicInt64               d_activeState;
    bsls::AtomicPointer<StateNode>  d_nextState_p;

    StateNode() : d_activeState(NON_ACTIVE) , d_nextState_p(0) {}
};

static bsls::AtomicInt64       s_systemStateState(1);  // tasks last state

static bsls::AtomicPointer<StateNode> 
                               s_threadStateList(0);   // list of each threads
                                                       // active state 



// CLASS METHODS
void *bcec_AltMemoryReclamationAdvisor::acquireState()
{

    bsls::Types::Uint64 currentState = s_systemStateState.loadAcquire();

    StateNode *stateNode = s_threadStateList.loadAcquire();
    
    while (stateNode) {
        if (NON_ACTIVE == stateNode->d_activeState.testAndSwap(NON_ACTIVE, 
                                                               currentState)) {
            return stateNode;
        }
        stateNode = stateNode->d_nextState_p.loadAcquire();
    }

    stateNode = new (bslma::NewDeleteAllocator::singleton()) StateNode;
    stateNode->d_activeState.storeRelaxed(currentState);

    StateNode *prevHead = 0;        
    StateNode *head     = s_threadStateList.loadRelaxed();
    
    do {
        stateNode->d_nextState_p = head;
        prevHead                 = head;
        head = s_threadStateList.testAndSwap(head, stateNode);
    } while (head != prevHead);

    return stateNode;
}

void bcec_AltMemoryReclamationAdvisor::releaseState(void *node)
{
    StateNode *stateNode = reinterpret_cast<StateNode *>(node);

    stateNode->d_activeState.storeRelaxed(NON_ACTIVE);

}

bsls::Types::Uint64 bcec_AltMemoryReclamationAdvisor::updateState()
{
    return s_systemStateState.addAcqRel(1);
}

bsls::Types::Uint64 bcec_AltMemoryReclamationAdvisor::oldestActiveState()
{
    bsls::Types::Uint64 oldestActiveState = -1;
    
    StateNode *node = s_threadStateList.loadRelaxed();
    while (node) {
        bsls::Types::Uint64 activeState = node->d_activeState.loadRelaxed();
        if (activeState < oldestActiveState) {
            oldestActiveState = activeState;
        }
        node = node->d_nextState_p;
    }
    BSLS_ASSERT(oldestActiveState != static_cast<bsls::Types::Uint64>(-1));

    return oldestActiveState;
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

