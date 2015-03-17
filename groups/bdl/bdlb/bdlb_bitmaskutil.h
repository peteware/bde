// bdlb_bitmaskutil.h                                                 -*-C++-*-
#ifndef INCLUDED_BDLB_BITMASKUTIL
#define INCLUDED_BDLB_BITMASKUTIL

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide simple mask values of 'uint32_t' and 'uint64_t' types.
//
//@CLASSES:
//  bdlb::BitMaskUtil: namespace for bit-level mask operations
//
//@DESCRIPTION: This component provides a utility 'struct',
// 'bdlb::BitMaskUtil', that serves as a namespace for a collection of
// functions that provide simple binary masks.
//
///Usage
///-----
// Note that, in all of these examples, the low-order bit is considered bit '0'
// and resides on the right edge of the bit string.
//
// 'ge' will return a bit mask with all bits below the specified 'index'
// cleared, and all bits at or above the 'index' set:
//..
//  assert((uint32_t) 0xffff0000 == bdlb::BitMaskUtil::ge(16));
//
//  +-------------------------------------------------------------------------+
//  | 'bdlb::BitMaskUtil::ge(16)' in binary:                                  |
//  |                                                                         |
//  | bit 16:                                               *                 |
//  | All bits at and above bit 16 are set:  11111111111111110000000000000000 |
//  +-------------------------------------------------------------------------+
//..
// Similarly, 'ltMask' return a bit mask with all bits above the specified
// 'index' cleared, and all bits at or below 'index' set:
//..
//  assert((uint32_t) 0x0000ffff == bdlb::BitMaskUtil::lt(16));
//
//  +-------------------------------------------------------------------------+
//  | 'bdlb::BitMaskUtil::lt(16)' in binary:                                  |
//  |                                                                         |
//  | bit 16:                                               *                 |
//  | All bits at and below bit 16 are set:  00000000000000001111111111111111 |
//  +-------------------------------------------------------------------------+
//..
// 'eqMask' return a bit mask with only the bit at the specified 'index' set:
//..
//  assert((uint32_t) 0x00800000 == bdlb::BitMaskUtil::eq(23));
//
//  +-------------------------------------------------------------------------+
//  | 'bdlb::BitMaskUtil::eq(23)' in binary:                                  |
//  |                                                                         |
//  | bit 23:                                        *                        |
//  | Only bit 23 is set:                    00000000100000000000000000000000 |
//  +-------------------------------------------------------------------------+
//..
// Similarly, 'neMask' return a bit mask with only the bit at the specified
// 'index' cleared:
//..
//  assert((uint32_t) 0xfffeffff == bdlb::BitMaskUtil::ne(16));
//
//  +-------------------------------------------------------------------------+
//  | 'bdlb::BitMaskUtil::ne(16)' in binary:                                  |
//  |                                                                         |
//  | bit 16:                                               *                 |
//  | All bits other than bit 16 are set:    11111111111111101111111111111111 |
//  +-------------------------------------------------------------------------+
//..
// Finally, 'one' and 'zero' return a bit mask with either all bits within a
// specified 'range' starting from a specified 'index' set, or cleared,
// respectively:
//..
//  assert((uint32_t) 0x000f0000 == bdlb::BitMaskUtil::one(16, 4));
//
//  +-------------------------------------------------------------------------+
//  | 'bdlb::BitMaskUtil::one(16, 4)' in binary:                              |
//  |                                                                         |
//  | bit 16:                                               *                 |
//  | 4 bits starting at bit 16:                         ****                 |
//  | Result: only those bits set:           00000000000011110000000000000000 |
//  +-------------------------------------------------------------------------+
//
//  assert((uint32_t) 0xfff0ffff == bdlb::BitMaskUtil::zero(16, 4));
//
//  +-------------------------------------------------------------------------+
//  | 'bdlb::BitMaskUtil::zero(16, 4)' in binary:                             |
//  |                                                                         |
//  | bit 16:                                               *                 |
//  | 4 bits starting at bit 16:                         ****                 |
//  | Result: only those bits cleared:       11111111111100001111111111111111 |
//  +-------------------------------------------------------------------------+
//..

#ifndef INCLUDED_BDLSCM_VERSION
#include <bdlscm_version.h>
#endif

#ifndef INCLUDED_BDLB_BITUTIL
#include <bdlb_bitutil.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

namespace BloombergLP {
namespace bdlb {

                               // ==================
                               // struct BitMaskUtil
                               // ==================

struct BitMaskUtil {
    // This utility 'struct' provides a namespace for a set of bit-level,
    // stateless functions that operate on the built-in 32- and 64-bit integer
    // types 'uint32_t' and 'uint64_t', respectively.

    // PUBLIC TYPES
    enum {
        k_BITS_PER_UINT32 = 32,  // bits used to represent an 'uint32_t'
        k_BITS_PER_UINT64 = 64   // bits used to represent an 'uint64_t'
    };

    typedef BitUtil::uint32_t    uint32_t;
    typedef BitUtil::uint64_t    uint64_t;

    // CLASS METHODS
    static uint32_t eq(int   index);
    static uint64_t eq64(int index);
        // Return an unsigned integral value having the bit at the specified
        // 'index' position set to 1, and all other bits set to 0.  The
        // behavior is undefined unless 'index >= 0'.  Note that supplying a
        // value of 'index >= sizeInBits(ReturnType)', where 'ReturnType' is
        // the particular C++ type of the integral value returned by this
        // function, results in a mask having all bits set to 0.

    static uint32_t ge(int   index);
    static uint64_t ge64(int index);
        // Return an unsigned integral value having bits at positions greater
        // than or equal to the specified 'index' set to 1, and all other bits
        // set to 0.  The behavior is undefined unless 'index >= 0'.  Note that
        // supplying an 'index >= sizeInBits(ReturnType)', where 'ReturnType'
        // is the particular C++ type of the integral value returned by this
        // function, results in a mask having all bits set to 0.

    static uint32_t gt(int   index);
    static uint64_t gt64(int index);
        // Return an unsigned integral value having bits at positions greater
        // than the specified 'index' set to 1, and all other bits set to 0.
        // The behavior is undefined unless 'index >= 0'.  Note that supplying
        // an 'index >= sizeInBits(ReturnType)', where 'ReturnType' is the
        // particular C++ type of the integral value returned by this function,
        // results in a mask having all bits set to 0.

    static uint32_t le(int   index);
    static uint64_t le64(int index);
        // Return an unsigned integral value having bits at positions less than
        // or equal to the specified 'index' set to 1, and all other bits set
        // to 0.  The behavior is undefined unless 'index >= 0'.  Note that
        // supplying an 'index >= sizeInBits(ReturnType)', where 'ReturnType'
        // is the particular C++ type of the integral value returned by this
        // function, results in a mask having all bits set to 1.

    static uint32_t lt(int   index);
    static uint64_t lt64(int index);
        // Return an unsigned integral value having bits at positions less than
        // the specified 'index' set to 1, and all other bits set to 0.  The
        // behavior is undefined unless 'idex >= 0'.  Note that supplying an
        // 'index >= sizeInBits(ReturnType)', where 'ReturnType' is the
        // particular C++ type of the integral value returned by this function,
        // results in a mask having all bits set to 1.

    static uint32_t ne(int   index);
    static uint64_t ne64(int index);
        // Return an unsigned integral value having the bit at the specified
        // 'index' position set to 0, and all other bits set to 1.  The
        // behavior is undefined unless 'index >= 0'.  Note that supplying an
        // 'index >= sizeInBits(ReturnType)', where 'ReturnType' is the
        // particular C++ type of the integral value returned by this function
        // results in a mask having all bits set to 1.

    static uint32_t one(int   index, int numBits);
    static uint64_t one64(int index, int numBits);
        // Return an unsigned integral value having the specified 'numBits'
        // starting at the specified 'index' set to 1, and all other bits set
        // to 0.  The behavior is undefined unless 'index >= 0' and
        // 'numBits >= 0'.  Note that supplying an
        // 'index >= sizeInBits(ReturnType)', where 'ReturnType' is the
        // particular C++ type of integral value returned by this function,
        // results in a mask having all bits set to 0.  Also note that there is
        // no upper limit on 'numBits' or 'index + numBits'.  Also note that
        // supplying an 'index < sizeInBits(ReturnType)' and a 'numBits' such
        // that 'index + numBits > sizeInBits(ReturnType)' results in the same
        // value being returned as if
        // 'function(index, sizeInBits(ReturnType) - index)' had been called.

    static uint32_t zero(int   index, int numBits);
    static uint64_t zero64(int index, int numBits);
        // Return an unsigned integral value having the specified 'numBits'
        // starting at the specified 'index' set to 0, and all other bits set
        // to 1.  The behavior is undefined unless 'index >= 0' and
        // 'numBits >= 0'.  Note that supplying an
        // 'index >= sizeInBits(ReturnType)', where 'ReturnType' is the
        // particular C++ type of integral value returned by this function,
        // results in a mask having all bits set to 1.  Also note that there is
        // no upper limit on 'numBits' or 'index + numBits'.  Also note that
        // supplying an 'index < sizeInBits(ReturnType)' and a 'numBits' such
        // that 'index + numBits > sizeInBits(ReturnType)' results in the same
        // value being returned as if
        // 'function(index, sizeInBits(ReturnType) - index)' had been called.
};

// ============================================================================
//                              INLINE DEFINITIONS
// ============================================================================

                               // -----------------
                               // Level-0 Functions
                               // -----------------

// CLASS METHODS
inline
BitMaskUtil::uint32_t BitMaskUtil::eq(int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    return BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                   index < static_cast<int>(k_BITS_PER_UINT32))
           ? 1 << index
           : 0;
}

inline
BitMaskUtil::uint64_t BitMaskUtil::eq64(int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    return BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                   index < static_cast<int>(k_BITS_PER_UINT64))
           ? 1LL << index
           : 0;
}

inline
BitMaskUtil::uint32_t BitMaskUtil::ge(int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    return BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                   index < static_cast<int>(k_BITS_PER_UINT32))
           ? (~0 << index)
           : 0;
}

inline
BitMaskUtil::uint64_t BitMaskUtil::ge64(int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    return BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                   index < static_cast<int>(k_BITS_PER_UINT64))
           ? (~0ULL << index)
           : 0;
}

inline
BitMaskUtil::uint32_t BitMaskUtil::gt(int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    ++index;
    return BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                   index < static_cast<int>(k_BITS_PER_UINT32))
           ? ~((1 << index) - 1)
           : 0;
}

inline
BitMaskUtil::uint64_t BitMaskUtil::gt64(int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    ++index;
    return BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                   index < static_cast<int>(k_BITS_PER_UINT64))
           ? ~((1LL << index) - 1)
           : 0;
}

inline
BitMaskUtil::uint32_t BitMaskUtil::le(int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    ++index;
    return BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                   index < static_cast<int>(k_BITS_PER_UINT32))
           ? (1 << index) - 1
           : -1;
}

inline
BitMaskUtil::uint64_t BitMaskUtil::le64(int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    ++index;
    return BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                   index < static_cast<int>(k_BITS_PER_UINT64))
           ? (1LL << index) - 1
           : -1LL;
}

inline
BitMaskUtil::uint32_t BitMaskUtil::lt(int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    return BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                  index >= static_cast<int>(k_BITS_PER_UINT32))
           ? -1
           : (1 << index) - 1;
}

inline
BitMaskUtil::uint64_t BitMaskUtil::lt64(int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    return BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                  index >= static_cast<int>(k_BITS_PER_UINT64))
           ? -1LL
           : (1LL << index) - 1;
}

inline
BitMaskUtil::uint32_t BitMaskUtil::ne(int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    return BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                  index >= static_cast<int>(k_BITS_PER_UINT32))
           ? -1
           : ~(1 << index);
}

inline
BitMaskUtil::uint64_t BitMaskUtil::ne64(int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    return BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                  index >= static_cast<int>(k_BITS_PER_UINT64))
           ? -1LL
           : ~(1LL << index);
}

                               // -----------------
                               // Level-1 Functions
                               // -----------------

inline
BitMaskUtil::uint32_t BitMaskUtil::one(int index, int numBits)
{
    BSLS_ASSERT_SAFE(0 <= index);
    BSLS_ASSERT_SAFE(0 <= numBits);

    return lt(index + numBits) & ge(index);
}

inline
BitMaskUtil::uint64_t BitMaskUtil::one64(int index, int numBits)
{
    BSLS_ASSERT_SAFE(0 <= index);
    BSLS_ASSERT_SAFE(0 <= numBits);

    return lt64(index + numBits) & ge64(index);
}

inline
BitMaskUtil::uint32_t BitMaskUtil::zero(int index, int numBits)
{
    BSLS_ASSERT_SAFE(0 <= index);
    BSLS_ASSERT_SAFE(0 <= numBits);

    return lt(index) | ge(index + numBits);
}

inline
BitMaskUtil::uint64_t BitMaskUtil::zero64(int index, int numBits)
{
    BSLS_ASSERT_SAFE(0 <= index);
    BSLS_ASSERT_SAFE(0 <= numBits);

    return lt64(index) | ge64(index + numBits);
}

}  // close package namespace
}  // close enterprise namespace

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
