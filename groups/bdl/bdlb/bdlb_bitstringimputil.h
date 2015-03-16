// bdlb_bitstringimputil.h                                            -*-C++-*-
#ifndef INCLUDED_BDLB_BITSTRINGIMPUTIL
#define INCLUDED_BDLB_BITSTRINGIMPUTIL

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide functional bit-manipulation of 'uint64_t' values.
//
//@CLASSES:
//  bdlb::BitStringImpUtil: namespace for 'uint64_t' utilities.
//
//@DESCRIPTION: This component provides a utility 'struct',
// 'bdlb::BitStringImpUtil', that serves as a namespace for a collection of
// functions that provide, bit-level operations on 'uint64_t'.  Some of these
// functions consist of a single bit-wise C scalar bit operation, the point of
// implmenting them as function is facilitate providing them as template
// arguments to templates in 'bdlb::BitStringUtil'.
//
// Note that no functions supporting 'uint32_t' are provided here.  This
// component exists solely to support 'bslb::BitStringUtil', that deals
// solely in 'uint64_t's.
//
///Usage
///-----
//
///Manipulators
/// - - - - - -
//..
//  uint64_t myInt;
//
//  myInt = 0x3333;
//  bdlb::BitStringImpUtil::andEqBits(&myInt, 8,  0, 8);
//  ASSERT(  0x33 == myInt);
//..
//  +-------------------------------------------------------------------------+
//  | 'bdlb::BitStringImpUtil::andEqBits(&myInt, 8, 0, 8)' in binary:         |
//  |                                                                         |
//  | 'myInt' before in binary:              00000000000000000011001100110011 |
//  | 'srcInteger == -1' in binary:          00000000000000000000000000000000 |
//  | logical or the 8 bits of 'srcInteger'                                   |
//  | at index 16:                                           00000000         |
//  | 'myInt' after in binary:               00000000000000000000000000110011 |
//  +-------------------------------------------------------------------------+
//..
//  myInt = 0x3333;
//  bdlb::BitStringImpUtil::andEqBits(&myInt, 8, -1, 8);    // Note '-1' is all
//                                                         // '1's.
//  ASSERT(0x3333 == myInt);
//..
// 'orEqBits' will take a slice of a second integer 'srcInteger' and bitwise
// OR it with another integer:
//..
//  myInt = 0x33333333;
//  bdlb::BitStringImpUtil::orEqBits(&myInt, 16, -1, 8);
//  ASSERT((int) 0x33ff3333 == myInt);
//..
//  +-------------------------------------------------------------------------+
//  | 'bdlb::BitStringImpUtil::orEqBits(&myInt, 16, -1, 8)' in binary:        |
//  |                                                                         |
//  | 'myInt' before in binary:              00110011001100110011001100110011 |
//  | 'srcInteger == -1' in binary:          11111111111111111111111111111111 |
//  | logical or the 8 bits of 'srcInteger'                                   |
//  | at index 16:                                   11111111                 |
//  | 'myInt' after in binary:               00110011111111110011001100110011 |
//  +-------------------------------------------------------------------------+
//..
//  myInt = 0;
//  bdlb::BitStringImpUtil::orEqBits(&myInt, 16, -1, 8);
//  ASSERT((int) 0x00ff0000 == myInt);
//..
// Another interface with 'xorEqBits' will take a section of a second scalar
// and bitwise XOR it with a first scalar:
//..
//  myInt = 0x77777777;
//  bdlb::BitStringImpUtil::xorEqBits(&myInt, 16, 0xff, 8);
//  ASSERT((int) 0x77887777 == myInt);
//..
//  +-------------------------------------------------------------------------+
//  | 'bdlb::BitStringImpUtil::xorEqBits(&myInt, 16, 0xff, 8)' in binary:     |
//  |                                                                         |
//  | 'myInt' before in binary:              01110111011101110111011101110111 |
//  | bits to affect: 8 bits at offset 16:           ********                 |
//  | value to put in those 8 bits is 0xff:          11111111                 |
//  | xor those 8 bits, leaving other bits unaffected                         |
//  | 'myInt' after in binary:               01110111100010000111011101110111 |
//  +-------------------------------------------------------------------------+
//..
//  myInt = 0x77777777;
//  bdlb::BitStringImpUtil::xorEqBits(&myInt, 16, 0x55, 8);
//  ASSERT((int) 0x77227777 == myInt);
//..
///Accessors
///- - - - -
// The 'find[01]At(Max,Min)IndexRaw' routines are used for finding the highest
// (or lowest) set (or clear) bit in a scalar, possibly within a subset of the
// integer:
//..
//  ASSERT( 8 == bdlb::BitStringImpUtil::find1AtMaxIndexRaw(0x00000101));
//..
//  +-------------------------------------------------------------------------+
//  | 'bdlb::BitStringImpUtil::find1AtMaxIndexRaw(0x000101)' in binary:       |
//  |                                                                         |
//  | input:                                 00000000000000000000000100000001 |
//  | bit 8:                                                        *         |
//  | range to look for highest bit in:                      **************** |
//  | highest set bit in that range:                                1         |
//  | index of that bit is 8                                                  |
//  +-------------------------------------------------------------------------+
//..
//  ASSERT( 8 == bdlb::BitStringImpUtil::find1AtMinIndexRaw(0xffff0100));
//..


#ifndef INCLUDED_BDLSCM_VERSION
#include <bdlscm_version.h>
#endif

#ifndef INCLUDED_BDLB_BITMASKUTIL
#include <bdlb_bitmaskutil.h>
#endif

#ifndef INCLUDED_BDLB_BITUTIL
#include <bdlb_bitutil.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

#ifndef INCLUDED_BSLS_PERFORMANCEHINT
#include <bsls_performancehint.h>
#endif

namespace BloombergLP {
namespace bdlb {

                           // =======================
                           // struct BitStringImpUtil
                           // =======================

struct BitStringImpUtil {


    // PUBLIC TYPES
    enum { k_BITS_PER_UINT64 = 64 };  // bits used to represent an 'uint64_t'

    typedef BitUtil::uint64_t    uint64_t;

    // CLASS METHODS

                                // Manipulators

    static void andEqBits(uint64_t *dstScalar,
                          int       dstIndex,
                          uint64_t  srcScalar,
                          int       numBits);
        // Bitwise AND the specified least-significant 'numBits' in the
        // specified 'srcScalar' to those in the specified 'dstScalar' starting
        // at the specified 'dstIndex'.  The behavior is undefined unless
        // '0 <= dstIndex', '0 <= numBits', and
        // 'dstIndex + numBits <= k_BITS_PER_UINT64'.

    static void andEqWord(uint64_t *dstScalar, uint64_t srcScalar);
        // Assign to the specified '*dstScalar' the value of '*dstScalar'
        // bitwise AND-ed with the specified 'srcScalar'.

    static void minusEqBits(uint64_t *dstScalar,
                            int       dstIndex,
                            uint64_t  srcScalar,
                            int       numBits);
        // Bitwise MINUS the specified least-significant 'numBits' in the
        // specified 'srcScalar' from those in the specified 'dstScalar'
        // starting at the specified 'dstIndex'.  The behavior is undefined
        // unless '0 <= dstIndex', '0 <= numBits', and
        // 'dstIndex + numBits <= k_BITS_PER_UINT64'.  Note that the bitwise
        // difference, 'a - b', is defined in C++ code as 'a & ~b'.

    static void minusEqWord(uint64_t *dstScalar, uint64_t srcScalar);
        // Assign to the specified '*dstScalar' the value of '*dstScalar'
        // bitwise MINUS-ed with the specified 'srcScalar'.

    static void orEqBits(uint64_t *dstScalar,
                         int       dstIndex,
                         uint64_t  srcScalar,
                         int       numBits);
        // Bitwise OR the specified least-significant 'numBits' in the
        // specified 'srcScalar' to those in the specified 'dstScalar' starting
        // at the specified 'dstIndex'.  The behavior is undefined unless
        // '0 <= dstIndex', '0 <= numBits', and
        // 'dstIndex + numBits <= k_BITS_PER_UINT64'.

    static void orEqWord(uint64_t *dstScalar, uint64_t srcScalar);
        // Assign to the specified '*dstScalar' the value of '*dstScalar'
        // bitwise OR-ed with the specified 'srcScalar'.

    static void setEqBits(uint64_t *dstScalar,
                          int       dstIndex,
                          uint64_t  srcScalar,
                          int       numBits);
        // Replace the specified 'numBits' in the specified 'dstScalar'
        // starting at the specified 'dstIndex' with the least-significant
        // 'numBits' of the specified 'srcScalar'.  The behavior is undefined
        // unless '0 <= dstIndex', '0 <= numBits', and
        // 'dstIndex + numBits <= k_BITS_PER_UINT64'.

    static void setEqWord(uint64_t *dstScalar, uint64_t srcScalar);
        // Assign to the specified '*dstScalar' the value of the specified
        // 'srcScalar'.

    static void xorEqBits(uint64_t *dstScalar,
                          int       dstIndex,
                          uint64_t  srcScalar,
                          int       numBits);
        // Bitwise XOR the specified least-significant 'numBits' in the
        // specified 'srcScalar' to those in the specified 'dstScalar' starting
        // at the specified 'dstIndex'.  The behavior is undefined unless
        // '0 <= dstIndex', '0 <= numBits', and
        // 'dstIndex + numBits <= k_BITS_PER_UINT64'.

    static void xorEqWord(uint64_t *dstScalar, uint64_t srcScalar);
        // Assign to the specified '*dstScalar' the value of '*dstScalar'
        // bitwise XOR-ed with the specified 'srcScalar'.

                                // Accessors

    static int find1AtMaxIndexRaw(uint64_t value);
        // Return the index of the highest-order set bit in the specified
        // 'value'.  The behavior is undefined if '0 == value'.

    static int find1AtMinIndexRaw(uint64_t value);
        // Return the index of the lowest-order set bit in the specified
        // 'value'.  The behavior is undefined if '0 == value'.
};

// ============================================================================
//                              INLINE DEFINITIONS
// ============================================================================

                                // Manipulators

inline
void BitStringImpUtil::andEqBits(uint64_t *dstScalar,
                                 int       dstIndex,
                                 uint64_t  srcScalar,
                                 int       numBits)
{
    BSLS_ASSERT_SAFE(                 0 <= numBits);
    BSLS_ASSERT_SAFE(                 0 <= dstIndex);
    BSLS_ASSERT_SAFE(dstIndex + numBits <= k_BITS_PER_UINT64);

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(dstIndex < k_BITS_PER_UINT64)) {
        *dstScalar &= BitMaskUtil::zero64(dstIndex, numBits) |
                                                       (srcScalar << dstIndex);
    }
}

inline
void BitStringImpUtil::andEqWord(uint64_t *dstScalar, uint64_t srcScalar)
{
    *dstScalar &= srcScalar;
}

inline
void BitStringImpUtil::minusEqBits(uint64_t *dstScalar,
                                   int       dstIndex,
                                   uint64_t  srcScalar,
                                   int       numBits)
{
    BSLS_ASSERT_SAFE(                 0 <= dstIndex);
    BSLS_ASSERT_SAFE(                 0 <= numBits);
    BSLS_ASSERT_SAFE(dstIndex + numBits <= k_BITS_PER_UINT64);

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(dstIndex < k_BITS_PER_UINT64)) {
        *dstScalar &= BitMaskUtil::zero64(dstIndex, numBits) |
                                                      (~srcScalar << dstIndex);
    }
}

inline
void BitStringImpUtil::minusEqWord(uint64_t *dstScalar, uint64_t srcScalar)
{
    *dstScalar &= ~srcScalar;
}

inline
void BitStringImpUtil::orEqBits(uint64_t *dstScalar,
                                int       dstIndex,
                                uint64_t  srcScalar,
                                int       numBits)
{
    BSLS_ASSERT_SAFE(                 0 <= dstIndex);
    BSLS_ASSERT_SAFE(                 0 <= numBits);
    BSLS_ASSERT_SAFE(dstIndex + numBits <= k_BITS_PER_UINT64);

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(dstIndex < k_BITS_PER_UINT64)) {
        *dstScalar |= (srcScalar & BitMaskUtil::lt64(numBits)) << dstIndex;
    }
}

inline
void BitStringImpUtil::orEqWord(uint64_t *dstScalar, uint64_t srcScalar)
{
    *dstScalar |= srcScalar;
}

inline
void BitStringImpUtil::setEqBits(uint64_t *dstScalar,
                                 int       dstIndex,
                                 uint64_t  srcScalar,
                                 int       numBits)
{
    BSLS_ASSERT_SAFE(0                  <= dstIndex);
    BSLS_ASSERT_SAFE(0                  <= numBits);
    BSLS_ASSERT_SAFE(dstIndex + numBits <= k_BITS_PER_UINT64);

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(dstIndex < k_BITS_PER_UINT64)) {
        uint64_t mask = BitMaskUtil::lt64(numBits);

        *dstScalar &= ~(mask << dstIndex);
        *dstScalar |= (srcScalar & mask) << dstIndex;
    }
}

inline
void BitStringImpUtil::setEqWord(uint64_t *dstScalar, uint64_t srcScalar)
{
    *dstScalar = srcScalar;
}

inline
void BitStringImpUtil::xorEqBits(uint64_t *dstScalar,
                                 int       dstIndex,
                                 uint64_t  srcScalar,
                                 int       numBits)
{
    BSLS_ASSERT_SAFE(                 0 <= dstIndex);
    BSLS_ASSERT_SAFE(                 0 <= numBits);
    BSLS_ASSERT_SAFE(dstIndex + numBits <= k_BITS_PER_UINT64);

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(dstIndex < k_BITS_PER_UINT64)) {
        *dstScalar ^= (srcScalar & BitMaskUtil::lt64(numBits)) << dstIndex;
    }
}

inline
void BitStringImpUtil::xorEqWord(uint64_t *dstScalar, uint64_t srcScalar)
{
    *dstScalar ^= srcScalar;
}

                                // Accessors

inline
int BitStringImpUtil::find1AtMaxIndexRaw(uint64_t value)
{
    BSLS_ASSERT_SAFE(0 != value);

    return k_BITS_PER_UINT64 - 1 - BitUtil::numLeadingUnsetBits(value);
}

inline
int BitStringImpUtil::find1AtMinIndexRaw(uint64_t value)
{
    BSLS_ASSERT_SAFE(0 != value);

    return BitUtil::numTrailingUnsetBits(value);
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
