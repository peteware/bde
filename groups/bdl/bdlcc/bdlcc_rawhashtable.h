// bdlt_hashtable.h                                                    -*-C++-*-
#ifndef INCLUDED_BDLT_HASHTABLE
#define INCLUDED_BDLT_HASHTABLE

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE:
//
//@CLASSES:
//  bdlt::Hashtable: combined date and time value (millisecond resolution)
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

#ifndef INCLUDED_BSLMA_SHAREDPTRINPLACEREP
#include <bslma_sharedptrinplacerep.h>
#endif

#ifndef INCLUDED_BSL_CSTDDEF
#include <bsl_cstddef.h>
#endif

#ifndef INCLUDED_BSL_MEMORY
#include <bsl_memory.h>
#endif

#ifndef INCLUDED_BSL_UTILITY
#include <bsl_utility.h>
#endif

namespace BloombergLP {
namespace bdlt {

                        // ==============
                        // class Hashtable
                        // ==============

template <class KEY, class VALUE, class HASHER, class EQUALITY>
class Hashtable {


    typedef bsl::shared_ptr<KEY>              Key;
    typedef bsl::shared_ptr<VALUE>            Value;
    typedef bslma::SharedPtrInplaceRep<KEY>   KeyRep;
    typedef bslma::SharedPtrInplaceRep<VALUE> ValueRep;
    typedef bsls::AtomicPointer<Key>          KeyPtr;
    typedef bsls::AtomicPointer<Value>        ValuePtr;

    struct Bucket {
        KeyPtr d_key;
        ValuePtr d_value;
    };

    const void *k_DELETED = -1;

    Bucket           *d_buckets_p;
    bsl::size_t       d_numBuckets;
    bsl::size_t       d_numElements;
    bslma::Allocator *d_allocator_p;


  public:

    Hashtable(int maxNumElements)
    : d_buckets_p(0)
    , d_numElements(0)
    , d_allocator_p(bslma::Default(0))
    , d_numBuckets(maxNumElements)
    {
        d_buckets_p = new (*d_allocator_p) Bucket[d_numBuckets];
    }

    void insert(const KEY& key, const VALUE& value)
    {
        HASHER   hasher;
        EQUALITY equals;

        bsl::size_t hash   = hasher(key);
        bsl::size_t bucket = hash % d_numBuckets;
        bsl::size_t originalBucket = bucket;
        while (true) {
            Key *keySPtrPtr = d_buckets_p[bucket].d_key.loadRelaxed();
            if (0 == keySPtrPtr) {
                Key *key = new (*d_allocator_p) Key;
                bslma::RawDeleterProctor<Key> guard(key, d_allocator_p);
                key->createInplace(d_allocator_p, key);
                
                
                Value *value = new (*d_allocator_p) Value;
                
                return bsl::shared_ptr<VALUE>();
            }
            if (equals(**keySPtrPtr, key)) {
                Value *valueSPtrPtr = d_buckets_p[bucket].d_value.loadRelaxed();
                while (true) {
                    
                }

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

    // ACCESSORS
    bsl::shared_ptr<VALUE> find(const KEY& key) const
    {
        HASHER   hasher;
        EQUALITY equals;

        bsl::size_t hash   = hasher(key);
        bsl::size_t bucket = hash % d_numBuckets;

        bsl::size_t originalBucket = bucket;
        while (true) {
            Key *keySPtrPtr = d_buckets_p[bucket].d_key.loadRelaxed();
            if (0 == keySPtrPtr) {
                return bsl::shared_ptr<VALUE>();
            }
            if (equals(**keySPtrPtr, key)) {
                Value *valueSPtrPtr = d_buckets_p[bucket].d_value.loadRelaxed();
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
}  // close namespace bdlt
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
