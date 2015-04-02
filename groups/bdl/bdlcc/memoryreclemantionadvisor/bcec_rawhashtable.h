//@PURPOSE: Provide a facility to force a link-time reference into an object.
#include <bcemt_mutex.h>
#include <bcemt_lockguard.h>

#include <bdes_bitutil.h>
#include <bslma_allocator.h>
#include <bslma_deallocatorproctor.h>
#include <bsls_atomic.h>

#ifndef INCLUDED_BSL_FUNCTIONAL
#include <bsl_functional.h>
#endif

#ifndef INCLUDED_BSL_CSTDDEF
#include <bsl_cstddef.h>
#endif

#include <bsl_iostream.h>

namespace BloombergLP {


struct RawHashTableConstants {
    static const void *TOMBSTONE;
    static const void *SENTINEL;
};

template <class T>
T *toPrime(const T *ptr)
{
    return ptr | 1;    
}

template <class T>
T *stripPrime(const T *ptr)
{
    static bsls::Types::UintPtr mask = ~1;
    bsls::Types::UintPtr value = reinterpret_cast<bsls::Types::UintPtr>(ptr);
    return reinterpret_cast<T *>(value & mask);
}

template <class T>
bool isPrime(const T *ptr)
{
    return ptr & 1;
}


template <
        class KEY,
        class VALUE,
        class HASH  = bsl::hash<KEY>,
        class EQUAL = bsl::equal_to<KEY> >
class RawHashTable {

   typedef bsls::AtomicOperations   Op;
   typedef Op::AtomicTypes::Pointer AtomicPtr;
   typedef RawHashTableConstants    K;

   struct Entry {
       AtomicPtr d_key;
       AtomicPtr d_value;
   };
        

   struct Table {
       Entry * const      d_entries;    
       AtomicPtr          d_nextTable;
       bsls::AtomicInt    d_numUsedBuckets;
       const unsigned int d_numBuckets;
       const unsigned int d_tableMask;

       Table(Entry *entries, int numUsedBuckets, int numBuckets) 
       : d_entries(entries)
       , d_numUsedBuckets(numUsedBuckets)
       , d_numBuckets(numBuckets)
       , d_tableMask(numBuckets - 1)
       {
           Op::initPointer(&d_nextTable, 0);

           BSLS_ASSERT(numBuckets == 
                       bdes_BitUtil::roundUpToBinaryPower(numBuckets));
       }
             
   };

   enum { 
       INITIAL_SIZE = 256,
   };

   enum ExpectedState {
       ANY_STATE    = 1,
       UNITITALIZED = 2,
       NO_VALUE     = 3,
       ANY_VALUE    = 4
   };


   bsls::AtomicInt   d_size;
   AtomicPtr         d_readTable;
   AtomicPtr         d_writeTable;
   bcemt_Mutex       d_resizeMutex;
   float             d_resizeLoadFactorLimit;
   bslma::Allocator *d_allocator_p;

  private:

   VALUE *getItem(Table *table, bsl::size_t hashValue, KEY *key)
   {
       EQUAL equal;       
       Entry *entries = table->d_entries;
       
       bsl::size_t hashBucket = hashValue & table->d_tableMask;
       bsl::size_t idx        = hashBucket;

       do {
           const Entry& entry = entries[idx];
           const KEY *entryKey = (const KEY *)Op::getPtrAcquire(&entry.d_key);

           if (!entryKey) {
               return 0;
           }

           else if (equal(*key, *entryKey)) {
               VALUE *value = (VALUE *)Op::getPtrAcquire(&entry.d_value);
               if (value != K::TOMBSTONE) {
                   return stripPrime(value);
               }
               return 0;
           }
           
           ++idx;
           idx &= table->d_tableMask;           

       } while (idx != hashBucket);
   }

   VALUE *setItem(Table         *table, 
                  bsl::size_t    hashValue, 
                  KEY           *key,
                  VALUE         *value,
                  ExpectedState  expectedState)
   {
       EQUAL equal;
       Entry *entries = table->d_entries;
       
       bsl::size_t hashBucket = hashValue & table->d_tableMask;
       bsl::size_t idx        = hashBucket;

       
       do {
           Entry& entry = entries[idx];
           const KEY *entryKey = (const KEY *)Op::getPtrAcquire(&entry.d_key);
           if (!entryKey) {

               entryKey = (KEY *)Op::testAndSwapPtrAcqRel(&entry.d_key, 0, key);

               if (0 == entryKey) {
                   break;
               }
               // Someone else grabbed this bucket before we could.
           }

           if (equal(*key, *entryKey)) {
               if (expectedState == UNITITALIZED) {
                   return value;
               }
               break;
           }

           ++idx;
           idx &= table->d_tableMask;           
       } while (idx != hashBucket);

       // TBD: Handle full table

       Entry& entry = entries[idx];
       VALUE *previous = (VALUE *)Op::getPtrRelaxed(&entry.d_value);          
       while (true) {
           if ((expectedState == UNITITALIZED && previous != 0) ||
               (expectedState == NO_VALUE && !(previous == 0 && 
                                               previous == K::TOMBSTONE)) ||
               (expectedState == ANY_VALUE && (previous == 0 ||
                                               previous == K::TOMBSTONE))) {
               // The previous value is not correct.
               return value;
           }
           VALUE *result = (VALUE *)Op::testAndSwapPtrAcqRel(&entry.d_value,
                                                             previous,
                                                             value);
           if (result == previous) {
               if (value != K::TOMBSTONE) {
                   if (result == 0 || result == K::TOMBSTONE) {
                       d_size.addRelaxed(1);
                       table->d_numUsedBuckets.addRelaxed(1);
                   }
               }
               else {
                   if (result != 0 && result != K::TOMBSTONE) {
                       d_size.addRelaxed(-1);
                       table->d_numUsedBuckets.addRelaxed(-1);
                   }
               }
               return result;
           }
           previous = result;
       }

       BSLS_ASSERT_OPT(false);
       return 0;
   }

  public:

   RawHashTable(bslma::Allocator *allocator) 
   : d_size(0)
   , d_allocator_p(allocator)
   , d_resizeLoadFactorLimit(.5)
   {
       
       Entry *entries = 
                 (Entry *)d_allocator_p->allocate(sizeof(Entry) * INITIAL_SIZE);
       Table *table   = new (*d_allocator_p) Table(entries, 0, INITIAL_SIZE);
       Op::initPointer(&d_readTable, table);
       Op::initPointer(&d_writeTable, table);
   }

   ~RawHashTable()
   {
       Table *table = (Table *)Op::getPtrRelaxed(&d_readTable);
       while (table) {
           Entry *entries = table->d_entries;
           d_allocator_p->deallocate(entries);

           Table *oldTable = table;
           table = (Table *)Op::getPtrRelaxed(&table->d_nextTable);
           d_allocator_p->deleteObject(oldTable);
       }

   }

   VALUE *getItem(KEY *key)
   {
       HASH  hash;
       bsl::size_t hashValue = hash(*key);      
       Table *table = (Table *)Op::getPtrRelaxed(&d_readTable);
       return getItem(table, hashValue, key);
   }

   VALUE *setItem(KEY *key, VALUE *value)
      // Associate the specified 'value' with the specified 'key' in this hash
      // table.  Return the previous value associated with 'key' or 0, if no
      // value was previously associated with key.
   {
       HASH  hash;
       bsl::size_t hashValue = hash(*key);
       Table *table = (Table *)Op::getPtrRelaxed(&d_writeTable);
       return setItem(table, hashValue, key, value, ANY_STATE);
   }

   VALUE *deleteItem(KEY *key)
   {
       HASH  hash;
       bsl::size_t hashValue = hash(*key);      
       Table *table = (Table *)Op::getPtrRelaxed(&d_writeTable);
       return setItem(table, hashValue, key, (KEY *)K::TOMBSTONE, ANY_VALUE); 
   }

   void transferBucket(Table *originalTable, int bucket, Table *nextTable)
   {
   }

   int rahash(Table *table)
   {
       typedef bslma::DeallocatorProctor<bslma::Allocator> DeallocProctor;
       enum { ALREADY_BEING_RESIZED = -1 };

       Table *nextTable = (Table *)Op::getPtrRelaxed(&table->d_nextTable);
       if (0 != nextTable) {
           return ALREADY_BEING_RESIZED;
       }
      
       int   usedBuckets = table->d_numUsedBuckets.loadRelaxed();
       float loadFactor  = (float)usedBuckets / table->d_numUsedBuckets;

       int newNumBuckets = table->numBuckets;
       if (loadFactor >= d_resizeLoadFactorLimit) {
           newNumBuckets *= 2;
       }

       Entry *entries = 
                (Entry *)d_allocator_p->allocate(sizeof(Entry) * newNumBuckets);
       DeallocProctor entryGuard(entries, d_allocator_p);

       table  = new (*d_allocator_p) Table(entries, 0, newNumBuckets);
       DeallocProctor tableGuard(table, d_allocator_p);
       
       if (0 != Op::testAndSwapPtrAcqRel(&table->d_nextTable, 0, table)) {
           return ALREADY_BEING_RESIZED;
       }
       entryGuard.release();
       tableGuard.release();
       
       for (int i = 0; i < newNumBuckets; ++i) {
           transferBucket(table, i, nextTable);
       }
   }

   VALUE *transferBucket(Table *srcTable, int bucket, Table *nextTable)
   {  
       Entry& entry = srcTable->d_entries[bucket];

       const KEY *key = (const KEY *)Op::getPtrRelaxed(&entry.d_key);
       if (0 == key) {
           key = (KEY *)Op::testAndSwapPtrAcqRel(&entry.d_key, 0, SENTINEL);
           if (0 == key) {
               return 0;
           }
       }
       key = (const KEY *)Op::getPtrAcquire(&entry.d_key);

       const VALUE *value = (const VALUE *)Op::getPtrRelaxed(&entry.d_value);
       if (0 == value) {
           value = 
               (VALUE *)Op::testAndSwapPtrAcqRel(&entry.d_value, 0, SENTINEL);
           if (0 == value) {
               return 0;
           }
       }

       HASH hash;
       bsl::size_t hashValue = hash(*key);      
       int newHashBucket = hashValue & nextTable->d_tableMask;
       int idx           = newHashBucket;
       do {
           Entry& newEntry = nextTable->d_entries[idx];
           const KEY *newKey = (const KEY *)Op::getPtrAcquire(&entry.d_key);
           if (!newKey) {
               newKey = (KEY *)Op::testAndSwapPtrAcqRel(&entry.d_key, 0, key);
               if (0 == entryKey) {
                   break;
               }
               // Someone else grabbed this bucket before we could.
           }
           if (equal(*newKey, *key)) {
               if (expectedState == UNITITALIZED) {
                   return value;
               }
               break;
           }

           ++idx;
           idx &= table->d_tableMask;           
       } while (idx != newHashBucket);


       while (1) {
           const VALUE *valuePrime = setPrime(value);
           
       }
       
   }


   int numElements() const
   {
       return d_size.loadRelaxed();
   }
};


} // close namespace BloombergLP;
