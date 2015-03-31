// bcec_memoryreclamationadvisor.h                                     -*-C++-*-
#include <bcec_memoryreclamationadvisor.h>

#include <bdes_ident.h>
BDES_IDENT_RCSID(bcec_memoryreclamationadvisor_cpp,"$Id$ $CSID$")

#include <bslmf_assert.h>
#include <bslma_newdeleteallocator.h>
#include <bsls_assert.h>
#include <bsls_atomic.h>

#include <bsl_iostream.h>

namespace BloombergLP {

                      // -----------------------------------
                      // class bcec_MemoryReclamationAdvisor
                      // -----------------------------------

struct StateNode {
    volatile bsls::Types::Uint64   *d_activeState_p;
    bsls::AtomicPointer<StateNode>  d_nextState_p;
};



__thread volatile bsls::Types::Uint64 t_activeState = 0;    
                                                       // current threads active
                                                       // state; 0 is an
                                                       // uninitialzed state,
                                                       // (Uint64)-1 indicates a
                                                       // non active state

bsls::AtomicInt64              s_systemStateState(1);  // tasks last state

bsls::AtomicPointer<StateNode> s_threadStateList(0);   // list of each threads
                                                       // active state 


static const bsls::Types::Uint64 NON_ACTIVE = -1;

void setUint64Release(volatile bsls::Types::Uint64 *result, 
                      bsls::Types::Uint64           value)
{

    BSLMF_ASSERT(sizeof(bsls::AtomicOperations::AtomicTypes::Int64) == 
                 sizeof(bsls::Types::Uint64));

    bsls::AtomicOperations::setInt64Release(
                      (bsls::AtomicOperations::AtomicTypes::Int64 *)(result),
                      static_cast<bsls::Types::Int64>(value));
}

bsls::Types::Uint64 getUint64Acquire(const volatile bsls::Types::Uint64 *value)
{
    BSLMF_ASSERT(sizeof(bsls::AtomicOperations::AtomicTypes::Int64) == 
                 sizeof(bsls::Types::Uint64));

    return static_cast<bsls::Types::Uint64>(
        bsls::AtomicOperations::getInt64Acquire(
                 (bsls::AtomicOperations::AtomicTypes::Int64 const *)(value)));

}



// CLASS METHODS
bsls::Types::Uint64 bcec_MemoryReclamationAdvisor::acquireState()
{
    BSLS_ASSERT(0 == t_activeState || NON_ACTIVE == t_activeState);

    if (0 == t_activeState) {
        StateNode *stateNode = 
                         new (bslma::NewDeleteAllocator::singleton()) StateNode;
        stateNode->d_activeState_p = &t_activeState;

        StateNode *prevHead = 0;        
        StateNode *head     = s_threadStateList.loadRelaxed();

        do {
            stateNode->d_nextState_p = head;
            prevHead                 = head;
            head = s_threadStateList.testAndSwap(head, stateNode);
        } while (head != prevHead);
    }

    bsls::Types::Uint64 currentState = s_systemStateState.loadAcquire();
    setUint64Release(&t_activeState, currentState);
    return currentState;
}

bsls::Types::Uint64 bcec_MemoryReclamationAdvisor::releaseState()
{
    BSLS_ASSERT(0 != t_activeState && NON_ACTIVE != t_activeState);

    setUint64Release(&t_activeState, NON_ACTIVE);

    return t_activeState;
}

bsls::Types::Uint64 bcec_MemoryReclamationAdvisor::updateState()
{
    BSLS_ASSERT(0 != s_threadStateList.loadRelaxed());
    BSLS_ASSERT(0 != t_activeState && NON_ACTIVE != t_activeState);

    return s_systemStateState.addAcqRel(1);
}

bsls::Types::Uint64 bcec_MemoryReclamationAdvisor::oldestActiveState()
{
    BSLS_ASSERT(0 != s_threadStateList.loadRelaxed());
    BSLS_ASSERT(0 != t_activeState && NON_ACTIVE != t_activeState);

    bsls::Types::Uint64 oldestActiveState = -1;
    
    StateNode *node = s_threadStateList.loadRelaxed();
    while (node) {
        bsls::Types::Uint64 activeState = 
                                      getUint64Acquire(node->d_activeState_p);
        if (activeState < oldestActiveState) {
            oldestActiveState = activeState;
        }
        node = node->d_nextState_p;
    }
    BSLS_ASSERT(oldestActiveState != static_cast<bsls::Types::Uint64>(-1));

    return oldestActiveState;
}

bsls::Types::Uint64 
bcec_MemoryReclamationAdvisor::currentThreadActiveState()
{
    return getUint64Acquire(&t_activeState);
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

