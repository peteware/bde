// bdlcc_rawhashtable.h                                                -*-C++-*-
#ifndef INCLUDED_BDLCC_RAWHASHTABLE
#define INCLUDED_BDLCC_RAWHASHTABLE

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE:
//
//@CLASSES:
//  bdlcc::RawHashtable: combined date and time value (millisecond resolution)
//
//@SEE_ALSO:
//
//@DESCRIPTION:

///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1:
///- - - - - - - - - - - -

#ifndef INCLUDED_BSLS_ATOMIC
#include <bsls_atomic.h>
#endif

#ifndef INCLUDED_BSLMA_DEFAULT
#include <bslma_default.h>
#endif

#ifndef INCLUDED_BSLMA_RAWDELETERPROCTOR
#include <bslma_rawdeleterproctor.h>
#endif

#ifndef INCLUDED_BSL_CSTDDEF
#include <bsl_cstddef.h>
#endif

#ifndef INCLUDED_BSL_CSTRING
#include <bsl_cstring.h>
#endif

#ifndef INCLUDED_BSL_FUNCTIONAL
#include <bsl_functional.h>
#endif

#ifndef INCLUDED_BSL_MEMORY
#include <bsl_memory.h>
#endif

#ifndef INCLUDED_BSL_UTILITY
#include <bsl_utility.h>
#endif


namespace BloombergLP {
namespace bdlcc {

                        // ==============
                        // class Hashtable
                        // ==============

template <class KEY, 
          class VALUE, 
          class HASHER   = bsl::hash<KEY>,
          class EQUALITY = bsl::equal_to<KEY> >
class Hashtable {


    typedef bsl::shared_ptr<KEY>              Key;
    typedef bsl::shared_ptr<VALUE>            Value;
    typedef bslma::SharedPtrInplaceRep<KEY>   KeyRep;
    typedef bslma::SharedPtrInplaceRep<VALUE> ValueRep;
    typedef bsls::AtomicPointer<Key>          KeyPtr;
    typedef bsls::AtomicPointer<Value>        ValuePtr;

    struct Bucket {
        KeyPtr   d_key;
        ValuePtr d_value;
    };

    const void *k_DELETED = reinterpret_cast<void *>( -1);

    Bucket           *d_buckets_p;
    bsl::size_t       d_numBuckets;
    bsl::size_t       d_numElements;
    bslma::Allocator *d_allocator_p;


  public:

    Hashtable(int maxNumElements, bslma::Allocator *basicAllocator = 0)
    : d_buckets_p(0)
    , d_numElements(0)
    , d_allocator_p(bslma::Default::allocator(basicAllocator))
    , d_numBuckets(maxNumElements)
    {
      d_buckets_p = reinterpret_cast<Bucket *>(
               d_allocator_p->allocate(sizeof(Bucket) * d_numBuckets));
      bsl::memset(d_buckets_p, 0, sizeof(Bucket) * d_numBuckets);
    }

    void insert(const KEY& key, const VALUE& value)
    {
        typedef bslma::RawDeleterProctor<Value, bslma::Allocator> ValueProctor;

        Value *newValue = new (*d_allocator_p) Value;
        ValueProctor valueGuard(newValue, d_allocator_p);
        newValue->createInplace(d_allocator_p, value, d_allocator_p);

        insert(key, newValue);
        valueGuard.release();
    }

    void remove(const KEY& key)
    {
        insert(key, reinterpret_cast<Value *>(const_cast<void *>(k_DELETED)));
    }

    void insert(const KEY& key, Value *newValue)
    {
        typedef bslma::RawDeleterProctor<Key, bslma::Allocator>   KeyProctor;

        HASHER   hasher;
        EQUALITY equals;

        bsl::size_t hash   = hasher(key);
        bsl::size_t bucket = hash % d_numBuckets;
        bsl::size_t originalBucket = bucket;

        while (true) {
            Key *keySPtrPtr = d_buckets_p[bucket].d_key.loadAcquire();
            if (0 == keySPtrPtr) {
                Key *newKey = new (*d_allocator_p) Key;
                KeyProctor keyGuard(newKey, d_allocator_p);
                newKey->createInplace(d_allocator_p, key, d_allocator_p);
                if (0 == 
                    d_buckets_p[bucket].d_key.testAndSwapAcqRel(0, newKey)) {
                    keyGuard.release();
                    break;                
                }
                continue;
            }
            else if (equals(**keySPtrPtr, key)) {
                break;
            }
            ++bucket;

            if (bucket == originalBucket) {
                BSLS_ASSERT_OPT(false); // TBD
            }
            if (bucket == d_numBuckets) {
                bucket = 0;
            }
        }
        while (true) {
            Value *valueSPtrPtr = d_buckets_p[bucket].d_value.loadRelaxed();
            if (valueSPtrPtr == 
                d_buckets_p[bucket].d_value.testAndSwapAcqRel(valueSPtrPtr,
                                                              newValue)) {
                break;                
            }
        }
    }

    // ACCESSORS
    bsl::shared_ptr<VALUE> find(const KEY& key) const
    {
        HASHER   hasher;
        EQUALITY equals;

        bsl::size_t hash   = hasher(key);
        bsl::size_t bucket = hash % d_numBuckets;

        bsl::size_t originalBucket = bucket;
        while (true) {
            Key *keySPtrPtr = d_buckets_p[bucket].d_key.loadAcquire();
            if (0 == keySPtrPtr) {
                return bsl::shared_ptr<VALUE>();
            }
            if (equals(**keySPtrPtr, key)) {
                Value *valueSPtrPtr = d_buckets_p[bucket].d_value.loadAcquire();
                if (0 == valueSPtrPtr || k_DELETED == valueSPtrPtr) {
                    return bsl::shared_ptr<VALUE>();
                }
                return *valueSPtrPtr;
            }

            ++bucket;

            if (bucket == originalBucket) {
                return bsl::shared_ptr<VALUE>();
            }
            if (bucket == d_numBuckets) {
                bucket = 0;
            }

        }
        BSLS_ASSERT_OPT(false);        // unreachable by design
        return bsl::shared_ptr<VALUE>();
    }
};

// ============================================================================
//                            INLINE DEFINITIONS
// ============================================================================

                               // --------------
                               // class Hashtable
                               // --------------
}  // close namespace bdlcc
}  // close namespace BloombergLP

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
