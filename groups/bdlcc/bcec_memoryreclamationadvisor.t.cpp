// bcec_memoryreclamationadvisor.t.cpp                                 -*-C++-*-
#include <bcec_memoryreclamationadvisor.h>

#include <bcec_altmemoryreclamationadvisor.h>
#include <bcec_hazardpointermanager.h>

#include <bcep_fixedthreadpool.h>
#include <bcemt_barrier.h>
#include <bcemt_lockguard.h>
#include <bcemt_threadutil.h>
#include <bcema_pool.h>

#include <bdef_bind.h>

#include <bslma_allocator.h>
#include <bslma_default.h>
#include <bslma_defaultallocatorguard.h>
#include <bslma_testallocator.h>

#include <bsls_assert.h>
#include <bsls_asserttest.h>
#include <bsls_stopwatch.h>
#include <bsls_types.h>


#include <bsl_algorithm.h>
#include <bsl_iostream.h>
#include <bsl_vector.h>
#include <bsls_atomic.h>

using namespace BloombergLP;
using namespace std;

//=============================================================================
//                              TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
//-----------------------------------------------------------------------------
// CLASS METHODS
// ----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [ 5] USAGE EXAMPLE
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
//                      STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------
static int testStatus = 0;

namespace {

void aSsErT(int c, const char *s, int i)
{
    if (c) {
        bsl::cout << "Error " << __FILE__ << "(" << i << "): " << s
                  << "    (failed)" << bsl::endl;
        if (0 <= testStatus && testStatus <= 100) ++testStatus;
    }
}

}  // close anonymous namespace

#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }

//=============================================================================
//                  STANDARD BDE LOOP-ASSERT TEST MACROS
//-----------------------------------------------------------------------------
#define LOOP_ASSERT(I,X) { \
    if (!(X)) { bsl::cout << #I << ": " << I << "\n"; \
                aSsErT(1, #X, __LINE__); }}

#define LOOP2_ASSERT(I,J,X) { \
    if (!(X)) { bsl::cout << #I << ": " << I << "\t"  \
                          << #J << ": " << J << "\n"; \
                aSsErT(1, #X, __LINE__); } }

#define LOOP3_ASSERT(I,J,K,X) { \
   if (!(X)) { bsl::cout << #I << ": " << I << "\t" \
                         << #J << ": " << J << "\t" \
                         << #K << ": " << K << "\n";\
               aSsErT(1, #X, __LINE__); } }

#define LOOP0_ASSERT ASSERT
#define LOOP1_ASSERT LOOP_ASSERT

//=============================================================================
//                  STANDARD BDE VARIADIC ASSERT TEST MACROS
//-----------------------------------------------------------------------------

#define NUM_ARGS_IMPL(X5, X4, X3, X2, X1, X0, N, ...)   N
#define NUM_ARGS(...) NUM_ARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1, 0, "")

#define LOOPN_ASSERT_IMPL(N, ...) LOOP ## N ## _ASSERT(__VA_ARGS__)
#define LOOPN_ASSERT(N, ...)      LOOPN_ASSERT_IMPL(N, __VA_ARGS__)

#define ASSERTV(...) LOOPN_ASSERT(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

//=============================================================================
//                  SEMI-STANDARD TEST OUTPUT MACROS
//-----------------------------------------------------------------------------
#define P(X)  cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X)  cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define P_(X) cout << #X " = " << (X) << ", "<< flush; // P(X) without '\n'
#define T_    cout << "\t" << flush;          // Print a tab (w/o newline)
#define L_ __LINE__                           // current Line number


// ============================================================================
//                  NEGATIVE-TEST MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT_SAFE_PASS(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_PASS(EXPR)
#define ASSERT_SAFE_FAIL(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_FAIL(EXPR)
#define ASSERT_PASS(EXPR)      BSLS_ASSERTTEST_ASSERT_PASS(EXPR)
#define ASSERT_FAIL(EXPR)      BSLS_ASSERTTEST_ASSERT_FAIL(EXPR)
#define ASSERT_OPT_PASS(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_PASS(EXPR)
#define ASSERT_OPT_FAIL(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_FAIL(EXPR)

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------
typedef bcec_MemoryReclamationAdvisor          Obj;

static int verbose = 0;
static int veryVerbose = 0;
static int veryVeryVerbose = 0;
static int veryVeryVeryVerbose = 0;

//=============================================================================
//                  TEST FUNCTIONS
//-----------------------------------------------------------------------------


// ============================================================================
//                         GLOBAL CLASSES FOR TESTING
// ----------------------------------------------------------------------------

struct MapNode {
   unsigned int         d_value;
   bsls::Types::Uint64  d_state;
   MapNode             *d_next;

   MapNode(unsigned int value) : d_value(value), d_state(0), d_next(0) {}
};

int g_sleepMs = 0;

inline
void testingDelay()
{
    if (g_sleepMs) {
        int seconds = g_sleepMs / 1000;
        int microSeconds = (g_sleepMs % 1000) * 1000;
        bcemt_ThreadUtil::microSleep(microSeconds, seconds);
    }
}

class MarkingTestAllocator : public bslma::Allocator {
        
   // DATA
   bcema_Pool      d_pool;
   const int       d_size; 
   bsls::AtomicInt d_numInUse;
  public:    

    enum {
        DELETED_VALUE = -2
    };
            
    MarkingTestAllocator(bslma::Allocator *allocator)
    : d_pool(sizeof(MapNode), allocator)
    , d_size(sizeof(MapNode))
    {
        BSLS_ASSERT(d_size >= 2);
    }
    

    virtual void *allocate(size_type size)
    {
        d_numInUse.addRelaxed(1);
        BSLS_ASSERT(d_size == size);
        return d_pool.allocate();
    }

    void deallocate(void *address) 
    {
        d_numInUse.addRelaxed(-1);
        MapNode *value = static_cast<MapNode *>(address);
        ASSERT(DELETED_VALUE != value->d_value);
        value->d_value = DELETED_VALUE;
    }
        
    int numInUse() const
    {
        return d_numInUse.loadRelaxed();
    }
};

class HazardPtrStaticIntMapWithMutex {

    bsls::AtomicPointer<MapNode>  d_value_p;

    bsls::AtomicPointer<MapNode>  d_freeList_p;

    bsls::AtomicInt               d_freeListSize;

    bslma::Allocator             *d_allocator_p;

    bcemt_Mutex                   d_mutex;
       
  public:
    enum { 
        NO_VALUE  = -1
    };
    
    explicit HazardPtrStaticIntMapWithMutex(bslma::Allocator *allocator) 
    : d_value_p(0)
    , d_allocator_p(allocator)
    { 
    }

    unsigned int setValue(unsigned int value) 
        // Set the mapped integer to the specified 'value' and return the
        // previously mapped value, or 'NOVALUE' if no value was previously
        // mapped. 
    {
        MapNode *valuePtr = new (*d_allocator_p) MapNode(value);
        
        MapNode *prevPtr = d_value_p.swapAcqRel(valuePtr);


        if (!prevPtr) {
            return NO_VALUE;
        }

        unsigned int prevValue = prevPtr->d_value;

        releaseNode(prevPtr);

        return prevValue;
    }

    unsigned int getValue() const {

        typedef bsls::AtomicOperations::AtomicTypes::Pointer AtomicPtr;


        AtomicPtr *hazardPointer =
                             bcec_HazardPointerManager::acquireHazardPointer();

        MapNode *valuePtr;
        do {
            valuePtr = d_value_p.loadAcquire();

            if (0 == valuePtr) {
                return NO_VALUE;
            }

            bsls::AtomicOperations::setPtrRelaxed(hazardPointer, valuePtr);
                        
        } while (valuePtr != d_value_p.loadRelaxed());
        

        testingDelay();

        unsigned int result = valuePtr->d_value;

        bcec_HazardPointerManager::releaseHazardPointer(hazardPointer);

        return result;
    }

    void releaseNode(MapNode *node)
    {
        MapNode *prevHead     = 0;
        MapNode *freeListHead = d_freeList_p.loadRelaxed();
        do {
            prevHead     = freeListHead;
            node->d_next = freeListHead;
            freeListHead = d_freeList_p.testAndSwapAcqRel(freeListHead, node);
        } while (prevHead != freeListHead);
        
        int size = d_freeListSize.addRelaxed(1);

        if (size > 100 && 0 == d_mutex.tryLock()) {
            d_freeListSize.addRelaxed(-100);

            bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex, 1);
            bsl::unordered_set<void *> hazardPointers;


            MapNode *head = d_freeList_p.swapAcqRel(0);
            bcec_HazardPointerManager::loadHazardPointers(&hazardPointers);

            int numElementsActuallyRemoved = 0;

            MapNode **prevNodeNextPtr = &head;
            MapNode *node             = head;
            while (node) {
                if (0 == hazardPointers.count(node)) {
                    *prevNodeNextPtr = node->d_next;
                    d_allocator_p->deallocate(node);
                    ++numElementsActuallyRemoved;
                }
                else {
                    prevNodeNextPtr = &node->d_next;
                }
                node = *prevNodeNextPtr;
            }

            if (0 == head) {
                return;
            }

            MapNode *prevHead     = 0;
            MapNode *freeListHead = d_freeList_p.loadRelaxed();
            do {
                prevHead         = freeListHead;
                *prevNodeNextPtr = freeListHead;
                freeListHead = 
                           d_freeList_p.testAndSwapAcqRel(freeListHead, head);
            } while (prevHead != freeListHead);
            d_freeListSize.addRelaxed(100 - numElementsActuallyRemoved);
        }
    }

};

class AltSafeStaticIntMapWithMutex {

    bsls::AtomicPointer<MapNode>  d_value_p;

    bsls::AtomicPointer<MapNode>  d_freeList_p;

    bsls::AtomicInt               d_freeListSize;

    bslma::Allocator             *d_allocator_p;

    bcemt_Mutex                   d_mutex;
  public:
    enum { 
        NO_VALUE  = -1
    };
    
    explicit AltSafeStaticIntMapWithMutex(bslma::Allocator *allocator) 
    : d_value_p(0)
    , d_freeListSize(0)
    , d_allocator_p(allocator)
    { 
    }

    unsigned int setValue(unsigned int value) 
        // Set the mapped integer to the specified 'value' and return the
        // previously mapped value, or 'NOVALUE' if no value was previously
        // mapped. 
    {
        bcec_AltMemoryReclamationGuard guard;

        MapNode *valuePtr = new (*d_allocator_p) MapNode(value);
        
        MapNode *prevPtr = d_value_p.swapAcqRel(valuePtr);

        bsls::Types::Uint64 state = 
                            bcec_AltMemoryReclamationAdvisor::updateState();

        if (!prevPtr) {
            return NO_VALUE;
        }

        unsigned int prevValue = prevPtr->d_value;

        releaseNode(prevPtr, state);

        return prevValue;
    }

    unsigned int getValue() const {
        bcec_AltMemoryReclamationGuard guard;

        MapNode *valuePtr = d_value_p.loadAcquire();


        testingDelay();

        return valuePtr ? valuePtr->d_value : NO_VALUE;
    }


    void releaseNode(MapNode *node, bsls::Types::Uint64 state)
    {
        node->d_state = state;

        MapNode *prevHead     = 0;
        MapNode *freeListHead = d_freeList_p.loadRelaxed();
        do {
            prevHead     = freeListHead;
            node->d_next = freeListHead;
            freeListHead = d_freeList_p.testAndSwapAcqRel(freeListHead, node);
        } while (prevHead != freeListHead);
        
        int size = d_freeListSize.addRelaxed(1);

        if (size > 100 && 0 == d_mutex.tryLock()) {
            d_freeListSize.addRelaxed(-100);

            bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex, 1);
            bsls::Types::Uint64 oldestActiveState = 
                      bcec_AltMemoryReclamationAdvisor::oldestActiveState();

            MapNode *head             = d_freeList_p.swapAcqRel(0);

            int numElementsActuallyRemoved = 0;

            MapNode **prevNodeNextPtr = &head;
            MapNode *node             = head;
            while (node) {
                if (node->d_state < oldestActiveState) {
                    *prevNodeNextPtr = node->d_next;
                    d_allocator_p->deallocate(node);
                    ++numElementsActuallyRemoved;
                }
                else {
                    prevNodeNextPtr = &node->d_next;
                }
                node = *prevNodeNextPtr;
            }

            if (0 == head) {
                return;
            }

            MapNode *prevHead     = 0;
            MapNode *freeListHead = d_freeList_p.loadRelaxed();
            do {
                prevHead         = freeListHead;
                *prevNodeNextPtr = freeListHead;
                freeListHead = 
                           d_freeList_p.testAndSwapAcqRel(freeListHead, head);
            } while (prevHead != freeListHead);
            d_freeListSize.addRelaxed(100 - numElementsActuallyRemoved);
        }
    }

};

class StateSafeStaticIntMapWithMutex {

    bsls::AtomicPointer<MapNode>  d_value_p;

    bsls::AtomicPointer<MapNode>  d_freeList_p;

    bsls::AtomicInt               d_freeListSize;

    bslma::Allocator             *d_allocator_p;

    bcemt_Mutex                   d_mutex;
  public:
    enum { 
        NO_VALUE  = -1
    };
    
    explicit StateSafeStaticIntMapWithMutex(bslma::Allocator *allocator) 
    : d_value_p(0)
    , d_freeListSize(0)
    , d_allocator_p(allocator)
    { 
    }

    unsigned int setValue(unsigned int value) 
        // Set the mapped integer to the specified 'value' and return the
        // previously mapped value, or 'NOVALUE' if no value was previously
        // mapped. 
    {
        bcec_MemoryReclamationGuard guard;

        MapNode *valuePtr = new (*d_allocator_p) MapNode(value);
        
        MapNode *prevPtr = d_value_p.swapAcqRel(valuePtr);

        bsls::Types::Uint64 state = 
                                  bcec_MemoryReclamationAdvisor::updateState();

        if (!prevPtr) {
            return NO_VALUE;
        }

        unsigned int prevValue = prevPtr->d_value;

        releaseNode(prevPtr, state);

        return prevValue;
    }

    unsigned int getValue() const {
        bcec_MemoryReclamationGuard guard;

        MapNode *valuePtr = d_value_p.loadAcquire();


        testingDelay();

        return valuePtr ? valuePtr->d_value : NO_VALUE;
    }


    void releaseNode(MapNode *node, bsls::Types::Uint64 state)
    {
        node->d_state = state;

        MapNode *prevHead     = 0;
        MapNode *freeListHead = d_freeList_p.loadRelaxed();
        do {
            prevHead     = freeListHead;
            node->d_next = freeListHead;
            freeListHead = d_freeList_p.testAndSwapAcqRel(freeListHead, node);
        } while (prevHead != freeListHead);
        
        int size = d_freeListSize.addRelaxed(1);

        if (size > 100 && 0 == d_mutex.tryLock()) {
            d_freeListSize.addRelaxed(-100);

            bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex, 1);
            bsls::Types::Uint64 oldestActiveState = 
                           bcec_MemoryReclamationAdvisor::oldestActiveState();

            MapNode *head             = d_freeList_p.swapAcqRel(0);

            int numElementsActuallyRemoved = 0;

            MapNode **prevNodeNextPtr = &head;
            MapNode *node             = head;
            while (node) {
                if (node->d_state < oldestActiveState) {
                    *prevNodeNextPtr = node->d_next;
                    d_allocator_p->deallocate(node);
                    ++numElementsActuallyRemoved;
                }
                else {
                    prevNodeNextPtr = &node->d_next;
                }
                node = *prevNodeNextPtr;
            }

            if (0 == head) {
                return;
            }

            MapNode *prevHead     = 0;
            MapNode *freeListHead = d_freeList_p.loadRelaxed();
            do {
                prevHead         = freeListHead;
                *prevNodeNextPtr = freeListHead;
                freeListHead = 
                           d_freeList_p.testAndSwapAcqRel(freeListHead, head);
            } while (prevHead != freeListHead);
            d_freeListSize.addRelaxed(100 - numElementsActuallyRemoved);
        }
    }

};

class StateSafeStaticIntMapNoMutex {

    bsls::AtomicPointer<MapNode>  d_value_p;

    bsls::AtomicPointer<MapNode>  d_freeList_p;

    bsls::AtomicInt               d_freeListSize;

    bslma::Allocator             *d_allocator_p;

  public:
    enum { 
        NO_VALUE  = -1
    };
    
    explicit StateSafeStaticIntMapNoMutex(bslma::Allocator *allocator) 
    : d_value_p(0)
    , d_freeListSize(0)
    , d_allocator_p(allocator)
    { 
    }

    unsigned int setValue(unsigned int value) 
        // Set the mapped integer to the specified 'value' and return the
        // previously mapped value, or 'NOVALUE' if no value was previously
        // mapped. 
    {
        bcec_MemoryReclamationGuard guard;

        MapNode *valuePtr = new (*d_allocator_p) MapNode(value);
        
        MapNode *prevPtr = d_value_p.swapAcqRel(valuePtr);

        bsls::Types::Uint64 state = 
                                  bcec_MemoryReclamationAdvisor::updateState();

        if (!prevPtr) {
            return NO_VALUE;
        }

        unsigned int prevValue = prevPtr->d_value;

        releaseNode(prevPtr, state);

        return prevValue;
    }

    unsigned int getValue() const {
        bcec_MemoryReclamationGuard guard;

        MapNode *valuePtr = d_value_p.loadAcquire();
        
        testingDelay();

        return valuePtr ? valuePtr->d_value : NO_VALUE;
    }

    void releaseNode(MapNode *node, bsls::Types::Uint64 state)
    {
        node->d_state = state;

        MapNode *prevHead     = 0;
        MapNode *freeListHead = d_freeList_p.loadRelaxed();
        do {
            prevHead     = freeListHead;
            node->d_next = freeListHead;
            freeListHead = d_freeList_p.testAndSwapAcqRel(freeListHead, node);
        } while (prevHead != freeListHead);
        
        int size = d_freeListSize.addRelaxed(1);

        if (size > 100) {
            d_freeListSize.addRelaxed(-100);

            bsls::Types::Uint64 oldestActiveState = 
                           bcec_MemoryReclamationAdvisor::oldestActiveState();

            MapNode *head             = d_freeList_p.swapAcqRel(0);

            MapNode **prevNodeNextPtr = &head;
            MapNode *node             = head;
            int numElementsActuallyRemoved = 0;
            while (node) {
                if (node->d_state < oldestActiveState) {
                    *prevNodeNextPtr = node->d_next;
                    d_allocator_p->deallocate(node);
                    ++numElementsActuallyRemoved;
                }
                else {
                    prevNodeNextPtr = &node->d_next;
                }
                node = *prevNodeNextPtr;
            }

            if (0 == head) {
                return;
            }

            MapNode *prevHead     = 0;
            MapNode *freeListHead = d_freeList_p.loadRelaxed();
            do {
                prevHead         = freeListHead;
                *prevNodeNextPtr = freeListHead;
                freeListHead = 
                           d_freeList_p.testAndSwapAcqRel(freeListHead, head);
            } while (prevHead != freeListHead);
            d_freeListSize.addRelaxed(100 - numElementsActuallyRemoved);
        }
    }


};

class HazardPtrMapNoMutex {

    bsls::AtomicPointer<MapNode>  d_value_p;

    bsls::AtomicPointer<MapNode>  d_freeList_p;

    bsls::AtomicInt               d_freeListSize;

    bslma::Allocator             *d_allocator_p;

  public:
    enum { 
        NO_VALUE  = -1
    };
    
    explicit HazardPtrMapNoMutex(bslma::Allocator *allocator) 
    : d_value_p(0)
    , d_freeListSize(0)
    , d_allocator_p(allocator)
    { 
    }

    unsigned int setValue(unsigned int value) 
        // Set the mapped integer to the specified 'value' and return the
        // previously mapped value, or 'NOVALUE' if no value was previously
        // mapped. 
    {
        bcec_MemoryReclamationGuard guard;

        MapNode *valuePtr = new (*d_allocator_p) MapNode(value);
        
        MapNode *prevPtr = d_value_p.swapAcqRel(valuePtr);

        bsls::Types::Uint64 state = 
                                  bcec_MemoryReclamationAdvisor::updateState();

        if (!prevPtr) {
            return NO_VALUE;
        }

        unsigned int prevValue = prevPtr->d_value;

        releaseNode(prevPtr, state);

        return prevValue;
    }

    unsigned int getValue() const {
        bcec_MemoryReclamationGuard guard;

        MapNode *valuePtr = d_value_p.loadAcquire();

        testingDelay();

        return valuePtr ? valuePtr->d_value : NO_VALUE;
    }

    void releaseNode(MapNode *node, bsls::Types::Uint64 state)
    {
        node->d_state = state;

        MapNode *prevHead     = 0;
        MapNode *freeListHead = d_freeList_p.loadRelaxed();
        do {
            prevHead     = freeListHead;
            node->d_next = freeListHead;
            freeListHead = d_freeList_p.testAndSwapAcqRel(freeListHead, node);
        } while (prevHead != freeListHead);
        
        int size = d_freeListSize.addRelaxed(1);

        if (size > 100) {
            d_freeListSize.addRelaxed(-100);

            bsls::Types::Uint64 oldestActiveState = 
                           bcec_MemoryReclamationAdvisor::oldestActiveState();

            MapNode *head             = d_freeList_p.swapAcqRel(0);

            MapNode **prevNodeNextPtr = &head;
            MapNode *node             = head;
            int numElementsActuallyRemoved = 0;
            while (node) {
                if (node->d_state < oldestActiveState) {
                    *prevNodeNextPtr = node->d_next;
                    d_allocator_p->deallocate(node);
                    ++numElementsActuallyRemoved;
                }
                else {
                    prevNodeNextPtr = &node->d_next;
                }
                node = *prevNodeNextPtr;
            }

            if (0 == head) {
                return;
            }

            MapNode *prevHead     = 0;
            MapNode *freeListHead = d_freeList_p.loadRelaxed();
            do {
                prevHead         = freeListHead;
                *prevNodeNextPtr = freeListHead;
                freeListHead = 
                           d_freeList_p.testAndSwapAcqRel(freeListHead, head);
            } while (prevHead != freeListHead);
            d_freeListSize.addRelaxed(100 - numElementsActuallyRemoved);
        }
    }

};


class MutexStaticIntMap {

    bsls::AtomicPointer<MapNode>  d_value_p;
    bslma::Allocator             *d_allocator_p;
    mutable bcemt_Mutex           d_mutex;

  public:
    enum { 
        NO_VALUE  = -1
    };
    
    explicit MutexStaticIntMap(bslma::Allocator *allocator) 
    : d_value_p(0)
    , d_allocator_p(allocator)
    { 
    }

    unsigned int setValue(unsigned int value) 
        // Set the mapped integer to the specified 'value' and return the
        // previously mapped value, or 'NOVALUE' if no value was previously
        // mapped. 
    {
        bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);

        MapNode *valuePtr = new (*d_allocator_p) MapNode(value);
        
        MapNode *prevPtr = d_value_p.swapAcqRel(valuePtr);


        if (!prevPtr) {
            return NO_VALUE;
        }

        unsigned int prevValue = prevPtr->d_value;

        d_allocator_p->deallocate(prevPtr);

        return prevValue;
    }

    unsigned int getValue() const {

        bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);

        MapNode *valuePtr = d_value_p.loadAcquire();

        testingDelay();

        return valuePtr ? valuePtr->d_value : NO_VALUE;
    }
};


class BrokenStaticIntMap {

    bsls::AtomicPointer<MapNode>  d_value_p;
    bslma::Allocator             *d_allocator_p;
       
  public:
    enum { 
        NO_VALUE  = -1
    };
    
    explicit BrokenStaticIntMap(bslma::Allocator *allocator) 
    : d_value_p(0)
    , d_allocator_p(allocator)
    { 
    }

    unsigned int setValue(unsigned int value) 
        // Set the mapped integer to the specified 'value' and return the
        // previously mapped value, or 'NOVALUE' if no value was previously
        // mapped. 
    {
        MapNode *valuePtr = new (*d_allocator_p) MapNode(value);
        
        MapNode *prevPtr = d_value_p.swapAcqRel(valuePtr);


        if (!prevPtr) {
            return NO_VALUE;
        }

        unsigned int prevValue = prevPtr->d_value;

        d_allocator_p->deallocate(prevPtr);

        return prevValue;
    }

    unsigned int getValue() const {

        MapNode *valuePtr = d_value_p.loadAcquire();

        testingDelay();

        return valuePtr ? valuePtr->d_value : NO_VALUE;
    }
};

enum {
    CHUNK_SIZE = 100
};

template <class TYPE>
void writeThread(bsls::AtomicInt64 *numOperations,
                 TYPE              *unsignedIntMap, 
                 bcemt_Barrier     *barrier,
                 bsls::AtomicInt   *doneFlag)
{
    bsls::Types::Int64 localNumOperations = 0;

    barrier->wait();

    do {
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            unsigned int prevValue = 
                               unsignedIntMap->setValue(localNumOperations);
            ASSERT(MarkingTestAllocator::DELETED_VALUE != prevValue);
        }
        localNumOperations += CHUNK_SIZE;
    } while (0 == *doneFlag);

    barrier->wait();

    *numOperations += localNumOperations;
}

template <class TYPE>
void readThread(bsls::AtomicInt64 *numOperations,
                TYPE              *unsignedIntMap,                 
                bcemt_Barrier     *barrier,
                bsls::AtomicInt   *doneFlag)
{
    bsls::Types::Int64 localNumOperations = 0;

    barrier->wait();

    do {
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            unsigned int prevValue = unsignedIntMap->getValue();
            ASSERT(MarkingTestAllocator::DELETED_VALUE != prevValue);
        }
        localNumOperations += CHUNK_SIZE;
    } while (0 == *doneFlag);

    barrier->wait();

    *numOperations += localNumOperations;
}


template <class MapType>
void performanceTest(int numThreads, int numSeconds)
{
    const int NUM_READERS = numThreads - numThreads/2;
    const int NUM_WRITERS = numThreads / 2;
    const int TOTAL_THREADS = NUM_READERS + NUM_WRITERS;

    bsls::AtomicInt64 numOperations;
    bcemt_Barrier     barrier(TOTAL_THREADS + 1);
    bsls::AtomicInt   doneFlag(0);

    bcep_FixedThreadPool threadPool(TOTAL_THREADS, TOTAL_THREADS * 2);

    MarkingTestAllocator testAllocator(
                                 &bslma::NewDeleteAllocator::singleton());

    MapType map(&testAllocator);

    threadPool.enable();
    for (int i = 0; i < NUM_READERS; ++i) {
        threadPool.enqueueJob(bdef_BindUtil::bind(&readThread<MapType>, 
                                                  &numOperations,
                                                  &map,
                                                  &barrier,
                                                  &doneFlag));
    }
    for (int i = 0; i < NUM_WRITERS; ++i) {
        threadPool.enqueueJob(bdef_BindUtil::bind(&writeThread<MapType>,
                                                  &numOperations,
                                                  &map,
                                                  &barrier,
                                                  &doneFlag));
    }
    threadPool.start();


    barrier.wait();

    bsls::Stopwatch stopwatch; 
    stopwatch.start();       
    bcemt_ThreadUtil::sleep(bdet_TimeInterval(numSeconds));
    
    doneFlag = 1;
    
    stopwatch.stop();
    barrier.wait();
    
    threadPool.stop();


//    ASSERTV(testAllocator.numInUse(), 
//            testAllocator.numInUse() <= TOTAL_THREADS + 2);

    bsl::cout << "numReaders: " << NUM_READERS << bsl::endl
              << "numWriters: " << NUM_WRITERS << bsl::endl
              << "numSeconds: " << stopwatch.elapsedTime() << bsl::endl
              << "numOperations:" << numOperations << bsl::endl
              << "Operations/Sec:" << (double)numOperations /
                                      stopwatch.elapsedTime()
              << bsl::endl
              << bsl::endl;
    
}

//=============================================================================
//                                 MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? std::atoi(argv[1]) : 0;

    verbose = (argc > 2);
    veryVerbose = (argc >  3);
    veryVeryVerbose = (argc > 4);
    veryVeryVeryVerbose = (argc > 5);

    cout << "TEST " << __FILE__ << " CASE " << test << endl;


    switch (test) { case 0:
      case 6: {
        // --------------------------------------------------------------------
        // TESTING USAGE EXAMPLE
        //
        // Concerns:
        //   The usage example provided in the component header file must
        //   compile, link, and run on all platforms as shown.
        //
        // Plan:
        //   Incorporate usage example from header into driver, remove leading
        //   comment characters, and replace 'BSLS_ASSERT' with 'ASSERT'.
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Usage Example"
                          << "\n=====================" << endl;


      } break;
      case 3: {
        // --------------------------------------------------------------------
        // BREATHING TEST: bcec_AltMemoryReclamationAdvisor
        //   Developers' Sandbox.
        //
        // Plan:
        //   Perform and ad-hoc test of the primary modifiers and accessors.
        //
        // Testing:
        //   This "test" *exercises* basic functionality, but *tests* nothing.
        // --------------------------------------------------------------------

        if (verbose) cout << endl << "BREATHING TEST" << endl
                                  << "==============" << endl;

        ASSERT(-1 == bcec_AltMemoryReclamationAdvisor::oldestActiveState());
        
        void *stateA = bcec_AltMemoryReclamationAdvisor::acquireState();

        ASSERT(0 != stateA);
        ASSERT(1 == bcec_AltMemoryReclamationAdvisor::oldestActiveState());

        void *stateB = bcec_AltMemoryReclamationAdvisor::acquireState();

        ASSERT(0 != stateB);
        ASSERT(1 == bcec_AltMemoryReclamationAdvisor::oldestActiveState());

        ASSERT(stateA != stateB);

        ASSERT(2 == bcec_AltMemoryReclamationAdvisor::updateState());
        ASSERT(1 == bcec_AltMemoryReclamationAdvisor::oldestActiveState());

        bcec_AltMemoryReclamationAdvisor::releaseState(stateA);

        ASSERT(1 == bcec_AltMemoryReclamationAdvisor::oldestActiveState());

        void *stateC = bcec_AltMemoryReclamationAdvisor::acquireState();

        ASSERT(0      != stateC);
        ASSERT(stateA == stateC);

        ASSERT(1 == bcec_AltMemoryReclamationAdvisor::oldestActiveState());

        bcec_AltMemoryReclamationAdvisor::releaseState(stateB);

        ASSERT(2 == bcec_AltMemoryReclamationAdvisor::oldestActiveState());
        
        
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // BREATHING TEST: bcec_HazardPointerManager
        //   Developers' Sandbox.
        //
        // Plan:
        //   Perform and ad-hoc test of the primary modifiers and accessors.
        //
        // Testing:
        //   This "test" *exercises* basic functionality, but *tests* nothing.
        // --------------------------------------------------------------------

        if (verbose) cout << endl << "BREATHING TEST" << endl
                                  << "==============" << endl;

        char a, b, c;

        typedef bsls::AtomicOperations::AtomicTypes::Pointer AtomicPtr;

        AtomicPtr *ptr1 = bcec_HazardPointerManager::acquireHazardPointer();
        ASSERT(0 != ptr1);
        bsls::AtomicOperations::setPtrRelaxed(ptr1, &a);

        AtomicPtr *ptr2 = bcec_HazardPointerManager::acquireHazardPointer();
        ASSERT(0 != ptr2);
        bsls::AtomicOperations::setPtrRelaxed(ptr2, &b);

        AtomicPtr *ptr3 = bcec_HazardPointerManager::acquireHazardPointer();
        ASSERT(0 != ptr3);
        bsls::AtomicOperations::setPtrRelaxed(ptr3, &c);


        ASSERT(ptr1 != ptr2);
        ASSERT(ptr2 != ptr3);
        ASSERT(ptr1 != ptr3);

        
        bsl::unordered_set<void *> hazardPointers;
        bcec_HazardPointerManager::loadHazardPointers(&hazardPointers);
        ASSERT(3 == hazardPointers.size());
        ASSERT(1 == hazardPointers.count(&a));
        ASSERT(1 == hazardPointers.count(&b));
        ASSERT(1 == hazardPointers.count(&c));
        ASSERT(0 == hazardPointers.count(&hazardPointers));
        hazardPointers.clear();
        

        bcec_HazardPointerManager::releaseHazardPointer(ptr1);
        bcec_HazardPointerManager::loadHazardPointers(&hazardPointers);
        ASSERT(2 == hazardPointers.size());
        ASSERT(0 == hazardPointers.count(&a));
        ASSERT(1 == hazardPointers.count(&b));
        ASSERT(1 == hazardPointers.count(&c));
        ASSERT(0 == hazardPointers.count(&hazardPointers));
        hazardPointers.clear();

        AtomicPtr *ptr1A = bcec_HazardPointerManager::acquireHazardPointer();
        ASSERT(0 != ptr1A);
        ASSERT(ptr1 == ptr1A);


        
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST:
        //   Developers' Sandbox.
        //
        // Plan:
        //   Perform and ad-hoc test of the primary modifiers and accessors.
        //
        // Testing:
        //   This "test" *exercises* basic functionality, but *tests* nothing.
        // --------------------------------------------------------------------

        if (verbose) cout << endl << "BREATHING TEST" << endl
                                  << "==============" << endl;


        enum {
            NUM_THREADS = 50,
            NUM_SECS    = 2
        };

        g_sleepMs = 0;

        bsl::cout << "MutexStaticIntMap:" << bsl::endl;
        performanceTest<MutexStaticIntMap>(NUM_THREADS, NUM_SECS);

        bsl::cout << "HazardPtrStaticIntMapWithMutex:" << bsl::endl;
        performanceTest<HazardPtrStaticIntMapWithMutex>(NUM_THREADS, NUM_SECS);

        bsl::cout << "StateSafeStaticIntMapNoMutex:" << bsl::endl;
        performanceTest<StateSafeStaticIntMapNoMutex>(NUM_THREADS, NUM_SECS);

        bsl::cout << "StateSafeStaticIntMapWithMutex:" << bsl::endl;
        performanceTest<StateSafeStaticIntMapWithMutex>(NUM_THREADS, NUM_SECS);

        bsl::cout << "AltSafeStaticIntMapWithMutex:" << bsl::endl;
        performanceTest<AltSafeStaticIntMapWithMutex>(NUM_THREADS, NUM_SECS);

        bsl::cout << "HazardPtrStaticIntMapWithMutex:" << bsl::endl;
        performanceTest<HazardPtrStaticIntMapWithMutex>(NUM_THREADS, NUM_SECS);



       } break;
      default: {
        cerr << "WARNING: CASE `" << test << "' NOT FOUND." << endl;
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        cerr << "Error, non-zero test status = " << testStatus << "." << endl;
    }
    return testStatus;
}

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2004
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
