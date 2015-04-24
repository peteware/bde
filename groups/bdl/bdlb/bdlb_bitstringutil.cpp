// bdlb_bitstringutil.cpp                                             -*-C++-*-
#include <bdlb_bitstringutil.h>

#include <bsls_ident.h>
BSLS_IDENT_RCSID(bdlb_bitstringutil_cpp,"$Id$ $CSID$")

#include <bdlb_bitmaskutil.h>
#include <bdlb_bitstringimputil.h>
#include <bdlb_bitutil.h>

#include <bsls_assert.h>
#include <bsls_platform.h>

#include <bsl_algorithm.h>
#include <bsl_ios.h>
#include <bsl_iomanip.h>
#include <bsl_ostream.h>

using namespace BloombergLP;

namespace {

typedef bdlb::BitUtil                 BitUtil;
typedef bdlb::BitMaskUtil             BitMaskUtil;
typedef bdlb::BitStringImpUtil        Imp;
typedef bdlb::BitStringUtil::uint64_t Uint64;

enum { k_BITS_PER_UINT64 = bdlb::BitStringUtil::k_BITS_PER_UINT64 };

                              // -----------
                              // class Mover
                              // -----------

template <void OPER_DO_BITS(        Uint64 *, int, Uint64, int),
          void OPER_DO_ALIGNED_WORD(Uint64 *,      Uint64     )>
class Mover {
    // This template 'class' is a namespace for static functions that will
    // maniuplate bit string.  The client will use 'move', 'left' and 'right'
    // to apply bitwise-logical operations over a range of bitStirngs.
    //
    // The two template arguments are functions:
    //..
    // void OPER_DO_BITS(Uint64 *dstWord,
    //                   int     dstIndex,
    //                   Uint64  srcValue,
    //                   int     numBits);
    //..
    // where 'OPER_DO_BITS' will apply some bitwise-logical operation between
    // 'numBits' bits in 'dstBitWord' and the low-order 'numBits' of
    // 'srcValue', assigning the result to the corresponding bits of 'dstWord'.
    // Note that the behavior is undefined unless
    // 'dstIndex + numBits <= k_BITS_PER_UINT64', so the operation never
    // affects more than a single word.
    //
    // And:
    //..
    // void OPER_DO_ALIGNED_WORD(Uint64 *dstWord,
    //                           Uint64  srcVallue);
    //..
    // where 'OPER_DO_ALIGGNED_WORD' will apply the same bitwise-logical
    // operation between all' bits of '*dstWord' and all bits of 'srcValue',
    // assigning the result to '*dstWord'.  Note that a call do
    // 'OPER_DO_ALIGNED_WORD(dstWord, srcValue)' would have exactly the same
    // effect as 'OPER_DO_BITS(dstWord, 0, srcValue, k_BITS_PER_UINT64)', but
    // 'OPER_DO_ALIGNED_WORD' is much more efficient in that case.

    // PRIVATE CLASS METHODS
    static void doPartialWord(Uint64 *dstBitString,
                              int     dstIndex,
                              Uint64  srcValue,
                              int     numBits);
        // Set the specified 'numBits' contiguous bits starting at the
        // specified 'dstIndex' in the specified 'dstBitString' to the result
        // of the templatized operation 'OPER_DO_BITS' of those bits and the
        // low-order 'numBits' bits in the specified 'srcValue'.  All other
        // bits are unaffected.  The behavior is undefined unless
        // '0 <= dstIndex < k_BITS_PER_UINT64', and
        // '0 <= numBits < k_BITS_PER_UINT64'.  Note that this operation may
        // affect up to two words of 'dstBitString'.

    static void doFullNonAlignedWord(Uint64 *dstBitString,
                                     int     dstIndex,
                                     Uint64  srcValue);
        // Set the 'k_BITS_PER_UINT64' contiguous bits starting at the
        // specified 'dstIndex' in the specified 'dstBitString' to the result
        // of the templatized operation 'OPER_DO_BITS' of those bits and bits
        // in the specified 'srcValue'.  All other bits are unaffected.  The
        // operation 'OPER_DO_BITS' has arguments: pointer to destination
        // array, index within destination array, source value, and number of
        // bits to apply the operation upon.  The behavior is undefined unless
        // '0 < dstIndex < k_BITS_PER_UINT64'.

    static bool requiresRightMove(const Uint64 *dstBitString,
                                  int           dstIndex,
                                  const Uint64 *srcBitString,
                                  int           srcIndex,
                                  int           numBits);
        // Return 'true' if the destination bit string specified by
        // 'dstBitString', 'dstIndex', and 'numBits' overlaps with the bit
        // string specified by 'srcBitString', 'srcIndex', and 'numBits' in
        // such a way that a right ('right' meaning 'right-to-left') operation
        // is required rather than a left-to-right operation is required.

  public:
    // PUBLIC CLASS METHODS

                                // directional moves

    static void left(Uint64       *dstBitString,
                     int           dstIndex,
                     const Uint64 *srcBitString,
                     int           srcIndex,
                     int           numBits);
        // Apply the bitwise-logical operation indicated by 'OPER_DO_BITS' and
        // 'OPER_DO_ALIGNED_WORD' between the specified 'numBits' of the
        // specified 'dstBitString' and the specified 'srcBitString', beginning
        // at the specified 'dstIndex' and the speecified 'srcIndex',
        // respectively.  Use 'doPartialWord' and 'doFullNonAlignedWord', and
        // 'OPER_DO_ALIGNED_WORD' to apply the operation.  The operation
        // proceeds from the low-order bits to the high-order bits (e.g.,
        // 'memcpy').  All other bits are unaffected.  The behavior is
        // undefined unless '0 <= dstIndex', '0 <= srcIndex', 'srcBitString'
        // contains at least 'srcIndex + numBits' bit values, '0 <= numBits',
        // and 'dstBitString' contains at least 'dstIndex + numBits'.  Note
        // that this method is alias-safe if 'dstIndex <= srcIndex' or
        // 'srcIndex + numBits <= dstIndex'.

    static void right(Uint64       *dstBitString,
                      int           dstIndex,
                      const Uint64 *srcBitString,
                      int           srcIndex,
                      int           numBits);
        // Apply the bitwise-logical operation indicated by 'OPER_DO_BITS' and
        // 'OPER_DO_ALIGNED_WORD' between the specified 'numBits' of the
        // specified 'dstBitString' and the specified 'srcBitString', beginning
        // at the specified 'dstIndex' and the specified 'srcIndex',
        // respectively.  Use 'doPartialWord' and 'doFullNonAlignedWord', and
        // 'OPER_DO_ALIGNED_WORD' to apply the operation.  The operation
        // proceeds from the high-order bits to the low-order bits (e.g., the
        // opposite of 'memcpy').  All other bits are unaffected.  The behavior
        // is undefined unless '0 <= dstIndex', '0 <= srcIndex', 'srcBitString'
        // contains at least 'srcIndex + numBits' bit values, '0 <= numBits',
        // and 'dstBitString' contains at least 'dstIndex + numBits'.  Note
        // that this method is alias-safe if 'dstIndex <= srcIndex' or
        // 'srcIndex + numBits <= dstIndex'.

                                // ambidextrous move

    static void move(Uint64       *dstBitString,
                     int           dstIndex,
                     const Uint64 *srcBitString,
                     int           srcIndex,
                     int           numBits);
        // Apply the bitwise-logical operation indicated by 'OPER_DO_BITS' and
        // 'OPER_DO_ALIGNED_WORD' between the specified 'numBits' of the
        // specified 'dstBitString' and the specified 'srcBitString', beginning
        // at the specified 'dstIndex' and 'srcIndex', respectively.  This
        // function is implemented by simply calling 'left' or 'right' as
        // appropriate, depending upon whether, and how, 'dstBitString' and
        // 'srcBitString' overlap.
};

template <void OPER_DO_BITS(        Uint64 *, int, Uint64, int),
          void OPER_DO_ALIGNED_WORD(Uint64 *,      Uint64)>
inline
void Mover<OPER_DO_BITS, OPER_DO_ALIGNED_WORD>::doPartialWord(
                                                          Uint64 *dstBitString,
                                                          int     dstIndex,
                                                          Uint64  srcValue,
                                                          int     numBits)
{
    BSLS_ASSERT_SAFE(0 <= dstIndex);
    BSLS_ASSERT_SAFE(     dstIndex < k_BITS_PER_UINT64);
    BSLS_ASSERT_SAFE(     numBits  < k_BITS_PER_UINT64);

    if (numBits <= 0) {
        BSLS_ASSERT_SAFE(0 == numBits);

        return;                                                       // RETURN
    }

    const int dstLen = k_BITS_PER_UINT64 - dstIndex;
    if (numBits <= dstLen) {
        // Fits in the 'dstIndex' element.

        OPER_DO_BITS(dstBitString, dstIndex, srcValue, numBits);
    }
    else {
        // Destination bits span two 'int' array elements.

        OPER_DO_BITS(dstBitString,   dstIndex,           srcValue,     dstLen);
        OPER_DO_BITS(dstBitString+1,        0, srcValue >> dstLen,
                                                             numBits - dstLen);
    }
}

template <void OPER_DO_BITS(        Uint64 *, int, Uint64, int),
          void OPER_DO_ALIGNED_WORD(Uint64 *,      Uint64)>
inline
void Mover<OPER_DO_BITS, OPER_DO_ALIGNED_WORD>::doFullNonAlignedWord(
                                                          Uint64 *dstBitString,
                                                          int     dstIndex,
                                                          Uint64  srcValue)
{
    BSLS_ASSERT_SAFE(0 < dstIndex);
    BSLS_ASSERT_SAFE(    dstIndex < k_BITS_PER_UINT64);

    // Since 'dstIndex > 0', destination bits always span two 'int' array
    // elements.

    const int dstLen = k_BITS_PER_UINT64 - dstIndex;
    OPER_DO_BITS(dstBitString,     dstIndex,           srcValue,   dstLen);
    OPER_DO_BITS(dstBitString + 1,        0, srcValue >> dstLen, dstIndex);
}

template <void OPER_DO_BITS(        Uint64 *, int, Uint64, int),
          void OPER_DO_ALIGNED_WORD(Uint64 *,      Uint64)>
bool Mover<OPER_DO_BITS, OPER_DO_ALIGNED_WORD>::requiresRightMove(
                                                    const Uint64 *dstBitString,
                                                    int           dstIndex,
                                                    const Uint64 *srcBitString,
                                                    int           srcIndex,
                                                    int           numBits)
    // Return 'true' if the destination bit string specified by 'dstBitString',
    // 'dstIndex', and 'numBits' overlaps with the bit string specified by
    // 'srcBitString', 'srcIndex', and 'numBits' in such a way that a right
    // ('right' meaning 'right-to-left') operation is required rather than a
    // left-to-right operation is required.
{
    BSLS_ASSERT(0 <= dstIndex);
    BSLS_ASSERT(0 <= srcIndex);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return false;                                                 // RETURN
    }

    int       dstIdx = dstIndex / k_BITS_PER_UINT64;
    const int dstPos = dstIndex % k_BITS_PER_UINT64;
    int       srcIdx = srcIndex / k_BITS_PER_UINT64;
    const int srcPos = srcIndex % k_BITS_PER_UINT64;

    dstBitString += dstIdx;
    dstIdx       =  0;
    srcBitString += srcIdx;
    srcIdx       =  0;

    if    (dstBitString <  srcBitString
       || (dstBitString == srcBitString && dstPos <= srcPos)) {
        // Either dst is non-overlapping with src, or they're overlapping
        // with dst to the left of src, or src == dst.

        return false;                                                 // RETURN
    }

    const int     srcTop    = srcPos + numBits;
    const int     srcEndIdx = srcTop / k_BITS_PER_UINT64;
    const int     srcEndPos = srcTop % k_BITS_PER_UINT64;
    const Uint64 *srcEndPtr = srcBitString + srcEndIdx;

    if    (srcEndPtr <  dstBitString
       || (srcEndPtr == dstBitString && srcEndPos <= dstPos)) {
        // Non-overlapping, src left of dst.  Right move not required.

        return false;                                                 // RETURN
    }

    BSLS_ASSERT_SAFE((dstBitString >  srcBitString ||
                     (dstBitString == srcBitString && dstPos > srcPos))
                  &&  dstBitString <=  srcEndPtr);

    return true;
}

template <void OPER_DO_BITS(        Uint64 *, int, Uint64, int),
          void OPER_DO_ALIGNED_WORD(Uint64 *,      Uint64)>
void Mover<OPER_DO_BITS, OPER_DO_ALIGNED_WORD>::left(
                                                    Uint64       *dstBitString,
                                                    int           dstIndex,
                                                    const Uint64 *srcBitString,
                                                    int           srcIndex,
                                                    int           numBits)
{
    BSLS_ASSERT(0 <= dstIndex);
    BSLS_ASSERT(0 <= srcIndex);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return;                                                       // RETURN
    }

    int dstIdx = dstIndex / k_BITS_PER_UINT64;
    int dstPos = dstIndex % k_BITS_PER_UINT64;

    int srcIdx = srcIndex / k_BITS_PER_UINT64;
    int srcPos = srcIndex % k_BITS_PER_UINT64;

    dstBitString += dstIdx;
    dstIdx       =  0;
    srcBitString += srcIdx;
    srcIdx       =  0;

#if defined(BSLS_ASSERT_SAFE_IS_ACTIVE)
    {
        const int srcEndIdx       = (srcPos + numBits) / k_BITS_PER_UINT64;
        const int srcEndPos       = (srcPos + numBits) % k_BITS_PER_UINT64;
        const Uint64 *srcEndPtr = srcBitString + srcEndIdx;

        BSLS_ASSERT_SAFE(dstBitString <  srcBitString
                     || (dstBitString == srcBitString && dstPos    <= srcPos)
                     ||  srcEndPtr    <  dstBitString
                     || (srcEndPtr    == dstBitString && srcEndPos <= dstPos));
    }
#endif

    // Copy bits to align residual of src on an 'int' boundary.

    if (srcPos) {
        int srcLen = k_BITS_PER_UINT64 - srcPos;
        if (srcLen >= numBits) {
            doPartialWord(&dstBitString[dstIdx],
                          dstPos,
                          srcBitString[ srcIdx] >> srcPos,
                          numBits);
            return;                                                   // RETURN
        }

        doPartialWord(&dstBitString[dstIdx],
                      dstPos,
                      srcBitString[ srcIdx] >> srcPos,
                      srcLen);
        dstPos += srcLen;
        if (dstPos >= k_BITS_PER_UINT64) {
            dstPos -= k_BITS_PER_UINT64;
            ++dstIdx;
        }
        numBits -= srcLen;

        // srcPos = (srcPos + srcLen) % K_BITS_PER_UINT64; (that is the same as
        // 'srcPos = 0;').

        srcPos = 0;
        ++srcIdx;
    }

    BSLS_ASSERT_SAFE(0 == srcPos);
    BSLS_ASSERT_SAFE(numBits > 0);

    // Copy full source ints.

    if (dstPos) {
        // Normal case of the destination location being unaligned.

        for (; numBits >= k_BITS_PER_UINT64; numBits -= k_BITS_PER_UINT64) {
            doFullNonAlignedWord(&dstBitString[dstIdx++],
                                 dstPos,
                                 srcBitString[ srcIdx++]);
        }
    }
    else {
        // The source and destination locations are both aligned.

        for (; numBits >= k_BITS_PER_UINT64; numBits -= k_BITS_PER_UINT64) {
            OPER_DO_ALIGNED_WORD(&dstBitString[dstIdx++],
                                 srcBitString[ srcIdx++]);
        }
    }
    BSLS_ASSERT_SAFE(numBits < k_BITS_PER_UINT64);

    // Move residual bits, if any.

    if (! numBits) {
        return;                                                       // RETURN
    }
    BSLS_ASSERT_SAFE(numBits > 0);

    doPartialWord(&dstBitString[dstIdx],
                  dstPos,
                  srcBitString[ srcIdx],
                  numBits);
}

template <void OPER_DO_BITS(        Uint64 *, int, Uint64, int),
          void OPER_DO_ALIGNED_WORD(Uint64 *,      Uint64)>
void Mover<OPER_DO_BITS, OPER_DO_ALIGNED_WORD>::right(
                                                    Uint64       *dstBitString,
                                                    int           dstIndex,
                                                    const Uint64 *srcBitString,
                                                    int           srcIndex,
                                                    int           numBits)
{
    // Precondtions can be checked with safe asserts since they were always
    // checked prior to this function being called.

    BSLS_ASSERT_SAFE(0 <= dstIndex);
    BSLS_ASSERT_SAFE(0 <= srcIndex);

    if (numBits <= 0) {
        BSLS_ASSERT_SAFE(0 == numBits);

        return;                                                       // RETURN
    }

    // Copy bits to align residual of src on an 'int' boundary.

    int endDstBit = dstIndex + numBits;
    int dstIdx    = endDstBit / k_BITS_PER_UINT64;
    int dstPos    = endDstBit % k_BITS_PER_UINT64;

    int endSrcBit = srcIndex + numBits;
    int srcIdx    = endSrcBit / k_BITS_PER_UINT64;
    int srcPos    = endSrcBit % k_BITS_PER_UINT64;

    dstBitString += dstIdx;
    dstIdx        = 0;
    srcBitString += srcIdx;
    srcIdx        = 0;

    // Either we're non-overlapping right, or overlapping such that a right
    // copy is necessay.

    BSLS_ASSERT_SAFE(dstBitString >  srcBitString
                 || (dstBitString == srcBitString && dstPos >= srcPos));

    // Note that this function should never be called in the caes of
    // non-overlapping right.

    if (srcPos) {
        if (srcPos >= numBits) {
            srcPos -= numBits;
            dstPos -= numBits;
            if (dstPos < 0) {
                dstPos += k_BITS_PER_UINT64;
                --dstIdx;
            }

            doPartialWord(&dstBitString[dstIdx],
                          dstPos,
                          srcBitString[      0] >> srcPos,
                          numBits);
            return;                                                   // RETURN
        }

        dstPos -= srcPos;
        if (dstPos < 0) {
            dstPos += k_BITS_PER_UINT64;
            --dstIdx;
        }

        doPartialWord(&dstBitString[dstIdx],
                      dstPos,
                      srcBitString[      0],
                      srcPos);
        numBits -= srcPos;

        // srcPos -= srcPos, but srcPos is not used after this
    }

    // src is now aligned.  Copy words at a time.

    if (dstPos) {
        // Normal case of the destination location being unaligned.

        for (; numBits >= k_BITS_PER_UINT64; numBits -= k_BITS_PER_UINT64) {
            doFullNonAlignedWord(&dstBitString[--dstIdx],
                                 dstPos,
                                 srcBitString[ --srcIdx]);
        }
    }
    else {
        // The source and destination locations are both aligned.

        for (; numBits >= k_BITS_PER_UINT64; numBits -= k_BITS_PER_UINT64) {
            OPER_DO_ALIGNED_WORD(&dstBitString[--dstIdx],
                                 srcBitString[ --srcIdx]);
        }
    }
    BSLS_ASSERT_SAFE(numBits < k_BITS_PER_UINT64);

    if (numBits <= 0) {
        BSLS_ASSERT_SAFE(0 == numBits);

        return;                                                       // RETURN
    }

    // Move the residual high-order 'numBits' bits of 'srcBitString[srcIdx-1]'

    dstPos -= numBits;
    if (dstPos < 0) {
        dstPos += k_BITS_PER_UINT64;
        --dstIdx;
    }
    doPartialWord(&dstBitString[dstIdx],
                  dstPos,
                  srcBitString[srcIdx - 1] >> (k_BITS_PER_UINT64 - numBits),
                  numBits);
}

template <void OPER_DO_BITS(        Uint64 *, int, Uint64, int),
          void OPER_DO_ALIGNED_WORD(Uint64 *,      Uint64)>
inline
void Mover<OPER_DO_BITS, OPER_DO_ALIGNED_WORD>::move(
                                                    Uint64       *dstBitString,
                                                    int           dstIndex,
                                                    const Uint64 *srcBitString,
                                                    int           srcIndex,
                                                    int           numBits)
{
    // preconditions verified in 'requiresRightMove'

    if (requiresRightMove(dstBitString,
                          dstIndex,
                          srcBitString,
                          srcIndex,
                          numBits)) {
        right(dstBitString,
              dstIndex,
              srcBitString,
              srcIndex,
              numBits);
    }
    else {
        left(dstBitString,
             dstIndex,
             srcBitString,
             srcIndex,
             numBits);
    }
}

}  // close unnamed namespace


                                // widely used

static inline
Uint64 rawLt64(int numBits)
    // Return a 'Uint64' value with the low order specified 'numBits' set and
    // the rest clear.  The behavior is undefined unless 'numBits'
    // '0 <= numBits < k_BITS_PER_UINT64'.  This function performs the same
    // calculation as 'BitMaskUtil::lt64', excpet that it doesn't waste time
    // handling the case of 'k_BITS_PER_UINT64 == numBits'.
{
    BSLS_ASSERT_SAFE(0 <= numBits);
    BSLS_ASSERT_SAFE(     numBits < k_BITS_PER_UINT64);

    return (1ULL << numBits) - 1;
}

static inline
Uint64 rawGe64(int numBits)
    // Return a 'Uint64' value with the low order specified 'numBits' clear and
    // the rest set.  The behavior is undefined unless 'numBits'
    // '0 <= numBits < k_BITS_PER_UINT64'.  This function performs the same
    // calculation as 'BitMaskUtil::ge64', excpet that it doesn't waste time
    // handling the case of 'k_BITS_PER_UINT64 == numBits'.
{
    BSLS_ASSERT_SAFE(0 <= numBits);
    BSLS_ASSERT_SAFE(     numBits < k_BITS_PER_UINT64);

    return ~0ULL << numBits;
}

                        // for 'areEqual'

static inline
bool bitsInWordsDiffer(Uint64 word1,
                       int    pos1,
                       Uint64 word2,
                       int    pos2,
                       int    numBits)
    // Compare the specified 'numBits' sequence of bits starting at the
    // specified 'pos1' in the specified 'word1' with the 'numBits' starting at
    // the specified 'pos2' in the specified 'word2', returning 'false' if they
    // match and 'true' if they differ.  The behavior is undefined unless
    // '0 <= pos1 < k_BITS_PER_UINT64', '0 <= pos2 < k_BITS_PER_UINT64',
    // '0 < numBits <= k_BITS_PER_UINT64',
    // 'pos1 + numBits <= k_BITS_PER_UINT64', and
    // 'pos2 + numBits <= k_BITS_PER_UINT64'.  Note that this function does not
    // handle the case of '0 == numBits'.
{
    BSLS_ASSERT_SAFE(numBits > 0);

    // 'numBits' is positive, so the asserts below of 'numBits + pos*' enforce
    // 'pos* < k_bits-pER_INT64'.  Also, 'pos* >= 0' so those same asserts
    // enforce 'numBits <= k_BITS_PER_UINT64'.

    BSLS_ASSERT_SAFE(0 <=      pos1);
    BSLS_ASSERT_SAFE(numBits + pos1 <= k_BITS_PER_UINT64);
    BSLS_ASSERT_SAFE(0 <=      pos2);
    BSLS_ASSERT_SAFE(numBits + pos2 <= k_BITS_PER_UINT64);

    // If two down-shifted bits don't match, the xor of them will be 1.  We
    // mask out the low-order 'numBits' (the bits we are interested in), and
    // if they're all 0, then the lower order 'numBits' of the two down-shifted
    // words must have matched and we should return 'false'.

    return ((word1 >> pos1) ^ (word2 >> pos2)) & BitMaskUtil::lt64(numBits);
}

                        // for 'swapRaw'

static
void swapBitsInWords(Uint64 *word1,
                     int     index1,
                     Uint64 *word2,
                     int     index2,
                     int     numBits)
    // Swap the specified 'numBits' sequence of bits starting at the
    // specified 'index1' in the specified 'word1' with the 'numBits'
    // starting at the specified 'index2' in the specified 'word2'.  The
    // behavior is undefined unless '0 <= index1 < k_BITS_PER_UINT64',
    // '0 <= index2 < k_BITS_PER_UINT64', '0 < numBits <= k_BITS_PER_UINT64',
    // 'word1 + numBits <= k_BITS_PER_UINT64', and
    // 'word2 + numBits <= k_BITS_PER_UINT64'.  The behavior is also undefined
    // if the two sets of bits specified are overlapping regions of the same
    // word, which is unchecked and the responsibility of the caller to ensure
    // doesn't happen.  Note that it is permissible for 'word1' and 'word1' to
    // refer to the same word, provided the bits being swapped don't overlap.
    // Note this function doesn't handle the case of '0 == numBits'.
{
    BSLS_ASSERT_SAFE(0 < numBits);
    BSLS_ASSERT_SAFE(0 <= index1);
    BSLS_ASSERT_SAFE(0 <= index2);

    // The following two asserts enforce 'numBits <= k_BITS_PER_UINT64', and
    // since 'numBits > 0', they enforce both indexes are less than
    // 'k_BITS_PER_UINT64'.

    BSLS_ASSERT_SAFE(index1 + numBits <= k_BITS_PER_UINT64);
    BSLS_ASSERT_SAFE(index2 + numBits <= k_BITS_PER_UINT64);

    const Uint64 mask  = BitMaskUtil::lt64(numBits);
    const Uint64 bits1 = (*word1 >> index1) & mask;
    const Uint64 bits2 = (*word2 >> index2) & mask;

    // Zero out the footprint we will write to.

    *word1 &= ~(mask << index1);
    *word2 &= ~(mask << index2);

    // Now or in the bits from the other word.

    *word1 |= bits2 << index1;
    *word2 |= bits1 << index2;
}

                        // for 'print'

static
void putSpaces(bsl::ostream& stream, int numSpaces)
    // Efficiently insert the specified 'numSpaces' spaces into the specified
    // 'stream'.  This function has no effect on 'stream' if 'numSpaces < 0'.
{
    // Algorithm: Write spaces in chunks.  The chunk size is large enough so
    // that most times only a single call to the 'write' method is needed.

    // Define the largest chunk of spaces:

    static const char spaces[] = "                                        ";

    enum { k_spaces_SIZE = sizeof(spaces) - 1 };

    while (numSpaces >= k_spaces_SIZE) {
        stream.write(spaces, k_spaces_SIZE);
        numSpaces -= k_spaces_SIZE;
    }

    if (0 < numSpaces) {
        stream.write(spaces, numSpaces);
    }
}

static
bsl::ostream& indent(bsl::ostream& stream,
                     int           level,
                     int           spacesPerLevel)
    // Output indentation to the specified 'stream' that is appropriate
    // according to BDE printing conventions for the specified 'level' and
    // the specified 'spacesPerLevel'.
{
    if (spacesPerLevel < 0) {
        spacesPerLevel = -spacesPerLevel;
    }

    putSpaces(stream, level * spacesPerLevel);
    return stream;
}

static
bsl::ostream& newlineAndIndent(bsl::ostream& stream,
                               int           level,
                               int           spacesPerLevel)
    // Output a newline and indentation to the specified 'stream' appropriate
    // for the specified 'level' and the specified 'spacesPerLevel'.
{
    if (spacesPerLevel < 0) {
        return stream << ' ';                                         // RETURN
    }

    if (level < 0) {
        level = -level;
    }

    stream << '\n';
    putSpaces(stream, level * spacesPerLevel);
    return stream;
}

namespace BloombergLP {
namespace bdlb {

                        // ====================
                        // struct BitStringUtil
                        // ====================

// CLASS METHODS

                            // Manipulators

                            // Assign

void BitStringUtil::assign(uint64_t *bitString,
                           int       index,
                           bool      value,
                           int       numBits)
{
    BSLS_ASSERT(0 <= index);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return;                                                       // RETURN
    }

    int idx       = index / k_BITS_PER_UINT64;
    int pos       = index % k_BITS_PER_UINT64;
    int numOfBits = bsl::min(k_BITS_PER_UINT64 - pos, numBits);

    // Set the partial leading bits

    if (value) {
        bitString[idx] |= BitMaskUtil::one64( pos, numOfBits);
    }
    else {
        bitString[idx] &= BitMaskUtil::zero64(pos, numOfBits);
    }

    numBits -= numOfBits;

    // pos += numOfBits; pos %= k_BITS_PER_UINT64; assert(0 == pos);

    // Set the integers with whole bits

    const uint64_t valueWord = value ? ~0ULL : 0ULL;
    for (; numBits >= k_BITS_PER_UINT64; numBits -= k_BITS_PER_UINT64) {
        bitString[++idx] = valueWord;
    }

    if (numBits <= 0) {
        BSLS_ASSERT_SAFE(0 == numBits);

        return;                                                       // RETURN
    }

    // Set the partial trailing bits

    if (value) {
        bitString[++idx] |= rawLt64(numBits);
    }
    else {
        bitString[++idx] &= rawGe64(numBits);
    }
}

void BitStringUtil::assign0(uint64_t  *bitString,
                            int        index,
                            int        numBits)
{
    BSLS_ASSERT(0 <= index);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return;                                                       // RETURN
    }

    int idx       = index / k_BITS_PER_UINT64;
    int pos       = index % k_BITS_PER_UINT64;
    int numOfBits = bsl::min(k_BITS_PER_UINT64 - pos, numBits);

    // Set the partial leading bits

    bitString[idx] &= BitMaskUtil::zero64(pos, numOfBits);

    numBits -= numOfBits;

    // pos += numOfBits; pos %= k_BITS_PER_UINT64; assert(0 == pos);

    for (; numBits >= k_BITS_PER_UINT64; numBits -= k_BITS_PER_UINT64) {
        bitString[++idx] = 0ULL;
    }

    if (numBits <= 0) {
        BSLS_ASSERT_SAFE(0 == numBits);

        return;                                                       // RETURN
    }

    // Set the partial trailing bits

    bitString[++idx] &= rawGe64(numBits);
}

void BitStringUtil::assign1(uint64_t  *bitString,
                            int        index,
                            int        numBits)
{
    BSLS_ASSERT(0 <= index);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return;                                                       // RETURN
    }

    int idx       = index / k_BITS_PER_UINT64;
    int pos       = index % k_BITS_PER_UINT64;
    int numOfBits = bsl::min(k_BITS_PER_UINT64 - pos, numBits);

    // Set the partial leading bits

    bitString[idx] |= BitMaskUtil::one64(pos, numOfBits);

    numBits -= numOfBits;

    // Set the integers with whole bits

    const uint64_t valueWord = ~0LL;
    for (; numBits >= k_BITS_PER_UINT64; numBits -= k_BITS_PER_UINT64) {
        bitString[++idx] = valueWord;
    }

    if (numBits <= 0) {
        BSLS_ASSERT_SAFE(0 == numBits);

        return;                                                       // RETURN
    }

    // Set the partial trailing bits

    bitString[++idx] |= rawLt64(numBits);
}

void BitStringUtil::assignBits(uint64_t *bitString,
                               int       index,
                               uint64_t  srcBits,
                               int       numBits)
{
    BSLS_ASSERT(index >= 0);
    BSLS_ASSERT(numBits <= k_BITS_PER_UINT64);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return;                                                       // RETURN
    }

    bitString += index / k_BITS_PER_UINT64;
    int pos    = index % k_BITS_PER_UINT64;
    int dstLen = k_BITS_PER_UINT64 - pos;

    if (dstLen >= numBits) {
        const uint64_t mask = BitMaskUtil::lt64(numBits);
        srcBits &= mask;
        *bitString &= ~(mask  << pos);
        *bitString |= srcBits << pos;
    }
    else {
        uint64_t mask = rawLt64(dstLen);
        uint64_t bits = srcBits & mask;
        *bitString &= ~(mask << pos);
        *bitString |=   bits << pos;

        mask = rawLt64(numBits - dstLen);
        bits = (srcBits >> dstLen) & mask;
        *++bitString &= ~mask;
        *  bitString |=  bits;
    }
}

                             // Bitwise-Logical

void BitStringUtil::andEqual(uint64_t       *dstBitString,
                             int             dstIndex,
                             const uint64_t *srcBitString,
                             int             srcIndex,
                             int             numBits)
{
    // preconditions checked in 'move'.

    Mover<Imp::andEqBits, Imp::andEqWord>::move(dstBitString,
                                                dstIndex,
                                                srcBitString,
                                                srcIndex,
                                                numBits);
}

void BitStringUtil::minusEqual(uint64_t       *dstBitString,
                               int             dstIndex,
                               const uint64_t *srcBitString,
                               int             srcIndex,
                               int             numBits)
{
    // preconditions checked in 'move'.

    Mover<Imp::minusEqBits, Imp::minusEqWord>::move(dstBitString,
                                                    dstIndex,
                                                    srcBitString,
                                                    srcIndex,
                                                    numBits);
}

void BitStringUtil::orEqual(uint64_t       *dstBitString,
                            int             dstIndex,
                            const uint64_t *srcBitString,
                            int             srcIndex,
                            int             numBits)
{
    // preconditions checked in 'move'.

    Mover<Imp::orEqBits, Imp::orEqWord>::move(dstBitString,
                                              dstIndex,
                                              srcBitString,
                                              srcIndex,
                                              numBits);
}

void BitStringUtil::xorEqual(uint64_t       *dstBitString,
                             int             dstIndex,
                             const uint64_t *srcBitString,
                             int             srcIndex,
                             int             numBits)
{
    // preconditions checked in 'move'.

    Mover<Imp::xorEqBits, Imp::xorEqWord>::move(dstBitString,
                                                dstIndex,
                                                srcBitString,
                                                srcIndex,
                                                numBits);
}

                            // Copy

void BitStringUtil::copy(uint64_t       *dstBitString,
                         int             dstIndex,
                         const uint64_t *srcBitString,
                         int             srcIndex,
                         int             numBits)
{
    // preconditions checked in 'move'.

    Mover<Imp::setEqBits, Imp::setEqWord>::move(dstBitString,
                                                dstIndex,
                                                srcBitString,
                                                srcIndex,
                                                numBits);
}

void BitStringUtil::copyRaw(uint64_t       *dstBitString,
                            int             dstIndex,
                            const uint64_t *srcBitString,
                            int             srcIndex,
                            int             numBits)
{
    // preconditions checked in 'left'

    Mover<Imp::setEqBits, Imp::setEqWord>::left(dstBitString,
                                                dstIndex,
                                                srcBitString,
                                                srcIndex,
                                                numBits);
}
                            // Insert / Remove

void BitStringUtil::insertRaw(uint64_t *bitString,
                              int       initialLength,
                              int       dstIndex,
                              int       numBits)
{
    BSLS_ASSERT(0 <= initialLength);
    BSLS_ASSERT(0 <= dstIndex);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return;                                                       // RETURN
    }

    if (dstIndex >= initialLength) {
        BSLS_ASSERT(dstIndex == initialLength);

        return;                                                       // RETURN
    }

    Mover<Imp::setEqBits, Imp::setEqWord>::right(bitString,
                                                 dstIndex + numBits,
                                                 bitString,
                                                 dstIndex,
                                                 initialLength - dstIndex);
}

void BitStringUtil::remove(uint64_t *bitString,
                           int       length,
                           int       index,
                           int       numBits)
{
    BSLS_ASSERT(0 <= length);
    BSLS_ASSERT(0 <= index);
    BSLS_ASSERT(index + numBits <= length);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return;                                                       // RETURN
    }

    const int remBits = length - index - numBits;
    if (0 == remBits) {
        return;                                                       // RETURN
    }

    // copy numBits starting index + numBits to index

    Mover<Imp::setEqBits, Imp::setEqWord>::left(bitString,
                                                index,
                                                bitString,
                                                index + numBits,
                                                remBits);
}

                            // Other Manipulators

void BitStringUtil::swapRaw(uint64_t *bitString1,
                            int       index1,
                            uint64_t *bitString2,
                            int       index2,
                            int       numBits)
{
    BSLS_ASSERT(0 <= index1);
    BSLS_ASSERT(0 <= index2);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return;                                                       // RETURN
    }

    // Normalize pointers and indices

    bitString1 += index1 / k_BITS_PER_UINT64;
    int pos1   =  index1 % k_BITS_PER_UINT64;
    int rem1   =  k_BITS_PER_UINT64 - pos1;    // bits from pos1 to end of word

    bitString2 += index2 / k_BITS_PER_UINT64;
    int pos2   =  index2 % k_BITS_PER_UINT64;
    int rem2   =  k_BITS_PER_UINT64 - pos2;    // bits from pos2 to end of word

    {
        // Verify non-overlapping.  It will be really disastrous if someone
        // tries to swap overlapping areas, so we check this with an
        // assert_opt.

        const uint64_t *lastBs1 = bitString1 + ((pos1 + numBits - 1) /
                                                            k_BITS_PER_UINT64);
        const uint64_t *lastBs2 = bitString2 + ((pos2 + numBits - 1) /
                                                            k_BITS_PER_UINT64);

        if (bitString2 <= lastBs1 && bitString1 <= lastBs2) {
            int endPos1 = (pos1 + numBits - 1) % k_BITS_PER_UINT64 + 1;
            int endPos2 = (pos2 + numBits - 1) % k_BITS_PER_UINT64 + 1;

            BSLS_ASSERT_OPT((bitString1 == lastBs2 && pos1 >= endPos2) ||
                            (bitString2 == lastBs1 && pos2 >= endPos1));
        }
    }

    if (pos1 == pos2) {
        int nb = bsl::min(rem1, numBits);
        BSLS_ASSERT_SAFE(nb > 0);

        // Swap patial partial bits

        swapBitsInWords(bitString1, pos1, bitString2, pos2, nb);

        numBits -= nb;
        if (numBits <= 0) {
            BSLS_ASSERT_SAFE(0 == numBits);

            return;                                                   // RETURN
        }

        for (; numBits >= k_BITS_PER_UINT64; numBits -= k_BITS_PER_UINT64) {
            using bsl::swap;

            swap(*++bitString1, *++bitString2);
        }

        if (numBits <= 0) {
            BSLS_ASSERT_SAFE(0 == numBits);

            return;                                                   // RETURN
        }

        swapBitsInWords(++bitString1, 0, ++bitString2, 0, numBits);
    }
    else {
        if (rem1 > rem2) {
            using bsl::swap;

            swap(bitString1, bitString2);
            swap(pos1, pos2);
            swap(rem1, rem2);
        }

        while (true) {
            {
                const int numBitsA = bsl::min(rem1, numBits);
                BSLS_ASSERT_SAFE(numBitsA > 0);

                swapBitsInWords(bitString1, pos1, bitString2, pos2, numBitsA);

                numBits -= numBitsA;
                if (numBits <= 0) {
                    BSLS_ASSERT_SAFE(0 == numBits);

                    return;                                           // RETURN
                }

                // pos1 += numBitsA; pos1 %= k_BITS_PER_UINT64;
                //
                // Note 'numBitsA == rem1 == k_BITS_PER_UINT64 - pos1'
                // so this brings pos1 flush to word boundary, pos1 = 0;
                // rem1 -= numBitsA; that means 'rem1 = 0;' but it's never
                // used after this.

                pos1 =  0;
                ++bitString1;

                pos2 += numBitsA;
                rem2 -= numBitsA;
            }

            {
                const int numBitsB = bsl::min(rem2, numBits);
                BSLS_ASSERT_SAFE(numBitsB > 0);

                swapBitsInWords(bitString1, pos1, bitString2, pos2, numBitsB);

                numBits -= numBitsB;
                if (numBits <= 0) {
                    BSLS_ASSERT_SAFE(0 == numBits);

                    return;                                           // RETURN
                }

                // pos2 += numBitsB; that brings pos2 up to the word boundary

                pos2 = 0;
                ++bitString2;
                rem2 = k_BITS_PER_UINT64;

                // pos1 += numBitsB; but pos1 was 0

                pos1 = numBitsB;
                rem1 = k_BITS_PER_UINT64 - pos1;
            }
        }
    }
}

void BitStringUtil::toggle(uint64_t *bitString, int index, int numBits)
{
    BSLS_ASSERT(0 <= index);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return;                                                       // RETURN
    }

    int idx = index / k_BITS_PER_UINT64;
    int pos = index % k_BITS_PER_UINT64;

    // Toggle unaligned initial bits.

    if (pos) {
        const int dstLen = k_BITS_PER_UINT64 - pos;
        if (dstLen > numBits) {
            bitString[idx] ^= rawGe64(pos) & rawLt64(pos + numBits);
            return;                                                   // RETURN
        }

        bitString[idx++]   ^= rawGe64(pos);
        numBits -= dstLen;
    }

    // Toggle full ints.

    for (; numBits >= k_BITS_PER_UINT64; numBits -= k_BITS_PER_UINT64) {
        bitString[idx++] ^= ~0ULL;
    }
    BSLS_ASSERT_SAFE(numBits < k_BITS_PER_UINT64);

    // Toggle trailing bits.

    if (numBits <= 0) {
        BSLS_ASSERT_SAFE(0 == numBits);

        return;                                                       // RETURN
    }
    BSLS_ASSERT_SAFE(numBits > 0);

    bitString[idx] ^= rawLt64(numBits);
}

                            // Accessors

bool BitStringUtil::areEqual(const uint64_t *bitString1,
                             int             index1,
                             const uint64_t *bitString2,
                             int             index2,
                             int             numBits)
{
    BSLS_ASSERT(bitString1);
    BSLS_ASSERT(0 <= index1);
    BSLS_ASSERT(bitString2);
    BSLS_ASSERT(0 <= index2);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return true;                                                  // RETURN
    }

    bitString1 += index1 / k_BITS_PER_UINT64;
    int pos1   =  index1 % k_BITS_PER_UINT64;
    int rem1   =  k_BITS_PER_UINT64 - pos1;

    bitString2 += index2 / k_BITS_PER_UINT64;
    int pos2   =  index2 % k_BITS_PER_UINT64;
    int rem2   =  k_BITS_PER_UINT64 - pos2;

    BSLS_ASSERT_SAFE(rem1 > 0 && rem2 > 0);

    if (pos1 == pos2) {
        int nb = bsl::min(rem1, numBits);

        if (bitsInWordsDiffer(*bitString1, pos1, *bitString2, pos2, nb)) {
            return false;                                             // RETURN
        }

        numBits -= nb;
        if (numBits <= 0) {
            BSLS_ASSERT_SAFE(0 == numBits);

            return true;                                              // RETURN
        }
        BSLS_ASSERT_SAFE(rem1 == nb);

        // We're never using 'pos1', 'pos2', 'rem1', or 'rem2' again, but just
        // to illustratie:
        //..
        // pos1 += nb; pos1 %= k_BITS_PER_UINT64; rem1 -= nb;
        // pos2 += nb; pos2 %= k_BITS_PER_UINT64; rem2 -= nb;
        // assert(0 == pos1);    assert(0 == rem1);
        // assert(0 == pos2);    assert(0 == rem2);
        //..

        for (; numBits >= k_BITS_PER_UINT64; numBits -= k_BITS_PER_UINT64) {
            if (*++bitString1 != *++bitString2) {
                return false;                                         // RETURN
            }
        }

        if (numBits <= 0) {
            BSLS_ASSERT_SAFE(0 == numBits);

            return true;                                              // RETURN
        }

        return !bitsInWordsDiffer(*++bitString1, 0, *++bitString2, 0, numBits);
                                                                      // RETURN
    }

    if (rem1 > rem2) {
        using bsl::swap;

        swap(bitString1, bitString2);
        swap(pos1, pos2);
        swap(rem1, rem2);
    }

    while (true) {
        // One iteration through the loop will either get us to the end, or
        // advance us 'rem2' bits, which will get 'pos2' word-aligned for the
        // next iteration, so each iteration after the first and before the
        // last will cover a full word.

        // The first part of the loop will compare 'numBitsA' bits, which will
        // either get pos1 to the end of the data, or to a word boundard of
        // 'bitString1'.

        {
            BSLS_ASSERT_SAFE(rem1 < rem2);
            BSLS_ASSERT_SAFE(rem1 > 0 && rem2 > 0);
            BSLS_ASSERT_SAFE(rem1 <  k_BITS_PER_UINT64);
            BSLS_ASSERT_SAFE(rem2 <= k_BITS_PER_UINT64);

            const int numBitsA = bsl::min(rem1, numBits);
            BSLS_ASSERT_SAFE(numBitsA > 0);

            if (bitsInWordsDiffer(*bitString1,
                                  pos1,
                                  *bitString2,
                                  pos2,
                                  numBitsA)) {
                return false;                                         // RETURN
            }

            numBits -= numBitsA;
            if (numBits <= 0) {
                BSLS_ASSERT_SAFE(0 == numBits);

                return true;                                          // RETURN
            }

            BSLS_ASSERT_SAFE(numBitsA == rem1);

            // pos1 = (pos1 + rem1) % k_BITS_PER_UINT64;  // (which is the same
            // as 'pos1 = 0;').

            ++bitString1;
            pos1 = 0;
            rem1 = k_BITS_PER_UINT64;

            pos2 += numBitsA;
            BSLS_ASSERT_SAFE(pos2 < k_BITS_PER_UINT64);
            rem2 -= numBitsA;
        }

        // The second part of the loop we will compare 'numBitsB bits, which
        // when we advance will get 'pos2' either to the end of the data, or to
        // a word boundary of 'bitString2'.

        {
            const int numBitsB = bsl::min(rem2, numBits);
            BSLS_ASSERT_SAFE(numBitsB > 0);

            if (bitsInWordsDiffer(*bitString1,
                                  pos1,
                                  *bitString2,
                                  pos2,
                                  numBitsB)) {
                return false;                                         // RETURN
            }

            numBits -= numBitsB;
            if (numBits <= 0) {
                BSLS_ASSERT_SAFE(0 == numBits);

                return true;                                          // RETURN
            }

            // Since 'rem2' was the original distance to the bitString2 word
            // boundary, if we advance 'pos2' by 'numBitsB' it will exactly
            // reach the word boundary.

            ++bitString2;
            pos2 = 0;
            rem2 = k_BITS_PER_UINT64;

            // Advance 'pos1' by 'numBitsB', but remember 'pos1' was 0.

            pos1 = numBitsB;
            rem1 -= numBitsB;
        }
    }

    return true;
}

bool BitStringUtil::areEqual(const uint64_t *bitString1,
                             const uint64_t *bitString2,
                             int             numBits)
{
    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return true;                                                  // RETURN
    }

    const int lastWord = (numBits - 1) / k_BITS_PER_UINT64;

    for (int ii = 0; ii < lastWord; ++ii) {
        if (bitString1[ii] != bitString2[ii]) {
            return false;                                             // RETURN
        }
    }

    const int endPos = (numBits - 1) % k_BITS_PER_UINT64 + 1;

    return 0 == ((bitString1[lastWord] ^ bitString2[lastWord]) &
                                                    BitMaskUtil::lt64(endPos));
}

// Implementation note for all the 'find[01]At{Max,Min}Index' functions:
// 'Imp::find1At{Max,Min}IndexRaw' can be very fast on some platforms, but on
// others it is a quite expensive operation.  Hence we avoid calling them until
// we are certain there is a set bit in the word it is searching, and the
// behavior of those 2 functions is undefined unless '0 != value'.

int BitStringUtil::find0AtMaxIndex(const uint64_t *bitString, int length)
{
    if (length <= 0) {
        BSLS_ASSERT(0 == length);

        return -1;                                                    // RETURN
    }

    const int lastWord = (length - 1) / k_BITS_PER_UINT64;
    const int endPos   = (length - 1) % k_BITS_PER_UINT64 + 1;

    uint64_t  value    = ~bitString[lastWord] & BitMaskUtil::lt64(endPos);

    for (int ii = lastWord; true; ) {
        if (value) {
            return ii * k_BITS_PER_UINT64 + Imp::find1AtMaxIndexRaw(value);
                                                                      // RETURN
        }

        if (--ii < 0) {
            return -1;                                                // RETURN
        }

        value = ~bitString[ii];
    }
}

int BitStringUtil::find1AtMaxIndex(const uint64_t *bitString, int length)
{
    if (length <= 0) {
        BSLS_ASSERT(0 == length);

        return -1;                                                    // RETURN
    }

    const int lastWord = (length - 1) / k_BITS_PER_UINT64;
    const int endPos   = (length - 1) % k_BITS_PER_UINT64 + 1;

    uint64_t  value    = bitString[lastWord] & BitMaskUtil::lt64(endPos);

    for (int ii = lastWord; true; ) {
        if (value) {
            return ii * k_BITS_PER_UINT64 + Imp::find1AtMaxIndexRaw(value);
                                                                      // RETURN
        }

        if (--ii < 0) {
            return -1;                                                // RETURN
        }

        value = bitString[ii];
    }
}

int BitStringUtil::find0AtMaxIndex(const uint64_t *bitString,
                                   int             begin,
                                   int             end)
{
    BSLS_ASSERT(0 <= begin);

    if (begin >= end) {
        BSLS_ASSERT(begin == end);

        return -1;                                                    // RETURN
    }

    const int beginWord = begin / k_BITS_PER_UINT64;
    const int lastWord  = (end - 1) / k_BITS_PER_UINT64;
    const int endPos    = (end - 1) % k_BITS_PER_UINT64 + 1;

    uint64_t  value     = ~bitString[lastWord] & BitMaskUtil::lt64(endPos);

    for (int ii = lastWord; ii > beginWord; value = ~bitString[--ii]) {
        if (value) {
            return ii * k_BITS_PER_UINT64 + Imp::find1AtMaxIndexRaw(value);
                                                                      // RETURN
        }
    }

    const int beginIdx = begin % k_BITS_PER_UINT64;

    value &= rawGe64(beginIdx);
    return value
           ? beginWord * k_BITS_PER_UINT64 + Imp::find1AtMaxIndexRaw(value)
           : -1;
}

int BitStringUtil::find1AtMaxIndex(const uint64_t *bitString,
                                   int             begin,
                                   int             end)
{
    BSLS_ASSERT(0 <= begin);

    if (begin >= end) {
        BSLS_ASSERT(begin == end);

        return -1;                                                    // RETURN
    }

    const int beginWord = begin / k_BITS_PER_UINT64;
    const int lastWord  = (end - 1) / k_BITS_PER_UINT64;
    const int endPos    = (end - 1) % k_BITS_PER_UINT64 + 1;

    uint64_t  value     = bitString[lastWord] & BitMaskUtil::lt64(endPos);

    for (int ii = lastWord; ii > beginWord; value = bitString[--ii]) {
        if (value) {
            return ii * k_BITS_PER_UINT64 + Imp::find1AtMaxIndexRaw(value);
                                                                      // RETURN
        }
    }

    const int beginIdx = begin % k_BITS_PER_UINT64;

    value &= rawGe64(beginIdx);
    return value
           ? beginWord * k_BITS_PER_UINT64 + Imp::find1AtMaxIndexRaw(value)
           : -1;
}

int BitStringUtil::find0AtMinIndex(const uint64_t *bitString, int length)
{
    if (length <= 0) {
        BSLS_ASSERT(0 == length);

        return -1;                                                    // RETURN
    }

    const int lastWord = (length - 1) / k_BITS_PER_UINT64;
    uint64_t  value;

    for (int ii = 0; ii < lastWord; ++ii) {
        value = ~bitString[ii];
        if (value) {
            return ii * k_BITS_PER_UINT64 + Imp::find1AtMinIndexRaw(value);
                                                                      // RETURN
        }
    }

    const int endPos = (length - 1) % k_BITS_PER_UINT64 + 1;

    value = ~bitString[lastWord] & BitMaskUtil::lt64(endPos);
    return value
           ? lastWord * k_BITS_PER_UINT64 + Imp::find1AtMinIndexRaw(value)
           : -1;
}

int BitStringUtil::find1AtMinIndex(const uint64_t *bitString, int length)
{
    if (length <= 0) {
        BSLS_ASSERT(0 == length);

        return -1;                                                    // RETURN
    }

    const int lastWord = (length - 1) / k_BITS_PER_UINT64;
    uint64_t  value;

    for (int ii = 0; ii < lastWord; ++ii) {
        value = bitString[ii];
        if (value) {
            return ii * k_BITS_PER_UINT64 + Imp::find1AtMinIndexRaw(value);
                                                                      // RETURN
        }
    }

    const int endPos = (length - 1) % k_BITS_PER_UINT64 + 1;

    value = bitString[lastWord] & BitMaskUtil::lt64(endPos);
    return value
           ? lastWord * k_BITS_PER_UINT64 + Imp::find1AtMinIndexRaw(value)
           : -1;
}

int BitStringUtil::find0AtMinIndex(const uint64_t *bitString,
                                   int             begin,
                                   int             end)
{
    BSLS_ASSERT(0 <= begin);

    if (begin >= end) {
        BSLS_ASSERT(begin == end);

        return -1;                                                    // RETURN
    }

    const int beginWord = begin / k_BITS_PER_UINT64;
    const int beginIdx  = begin % k_BITS_PER_UINT64;
    const int lastWord  = (end - 1) / k_BITS_PER_UINT64;

    uint64_t  value     = ~bitString[beginWord] & rawGe64(beginIdx);

    for (int ii = beginWord; ii < lastWord; value = ~bitString[++ii]) {
        if (value) {
            return ii * k_BITS_PER_UINT64 + Imp::find1AtMinIndexRaw(value);
                                                                      // RETURN
        }
    }

    const int endPos = (end - 1) % k_BITS_PER_UINT64 + 1;

    value &= BitMaskUtil::lt64(endPos);
    return value
           ? lastWord * k_BITS_PER_UINT64 + Imp::find1AtMinIndexRaw(value)
           : -1;
}

int BitStringUtil::find1AtMinIndex(const uint64_t *bitString,
                                   int             begin,
                                   int             end)
{
    BSLS_ASSERT(bitString);
    BSLS_ASSERT(0 <= begin);

    if (begin >= end) {
        BSLS_ASSERT(begin == end);

        return -1;                                                    // RETURN
    }

    const int beginWord = begin / k_BITS_PER_UINT64;
    const int beginIdx  = begin % k_BITS_PER_UINT64;
    const int lastWord  = (end - 1) / k_BITS_PER_UINT64;

    uint64_t  value     = bitString[beginWord] & rawGe64(beginIdx);

    for (int ii = beginWord; ii < lastWord; value = bitString[++ii]) {
        if (value) {
            return ii * k_BITS_PER_UINT64 + Imp::find1AtMinIndexRaw(value);
                                                                      // RETURN
        }
    }

    const int endPos = (end - 1) % k_BITS_PER_UINT64 + 1;

    value &= BitMaskUtil::lt64(endPos);
    return value
           ? lastWord * k_BITS_PER_UINT64 + Imp::find1AtMinIndexRaw(value)
           : -1;
}

bool BitStringUtil::isAny0(const uint64_t *bitString,
                           int             index,
                           int             numBits)
{
    BSLS_ASSERT(0 <= index);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return false;                                                 // RETURN
    }

    int idx       = index / k_BITS_PER_UINT64;
    int pos       = index % k_BITS_PER_UINT64;
    int numOfBits = bsl::min(k_BITS_PER_UINT64 - pos, numBits);

    if (~bitString[idx] & BitMaskUtil::one64(pos, numOfBits)) {
        return true;                                                  // RETURN
    }
    numBits -= numOfBits;

    while (numBits >= k_BITS_PER_UINT64) {
        if (~bitString[++idx]) {
            return true;                                              // RETURN
        }
        numBits -= k_BITS_PER_UINT64;
    }
    BSLS_ASSERT_SAFE(numBits < k_BITS_PER_UINT64);

    if (numBits <= 0) {
        BSLS_ASSERT_SAFE(0 == numBits);

        return false;                                                 // RETURN
    }

    return ~bitString[++idx] & rawLt64(numBits);
}

bool BitStringUtil::isAny1(const uint64_t *bitString,
                           int             index,
                           int             numBits)
{
    BSLS_ASSERT(0 <= index);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return false;                                                 // RETURN
    }

    int idx       = index / k_BITS_PER_UINT64;
    int pos       = index % k_BITS_PER_UINT64;
    int numOfBits = bsl::min(k_BITS_PER_UINT64 - pos, numBits);

    if (bitString[idx] & BitMaskUtil::one64(pos, numOfBits)) {
        return true;                                                  // RETURN
    }
    numBits -= numOfBits;

    while (numBits >= k_BITS_PER_UINT64) {
        if (bitString[++idx]) {
            return true;                                              // RETURN
        }
        numBits -= k_BITS_PER_UINT64;
    }
    BSLS_ASSERT_SAFE(numBits < k_BITS_PER_UINT64);

    if (numBits <= 0) {
        BSLS_ASSERT_SAFE(0 == numBits);

        return false;                                                 // RETURN
    }

    return bitString[++idx] & rawLt64(numBits);
}

uint64_t BitStringUtil::bits(const uint64_t *bitString, int index, int numBits)
{
    BSLS_ASSERT(bitString);
    BSLS_ASSERT(0 <= index);
    BSLS_ASSERT(     numBits <= k_BITS_PER_UINT64);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return 0;                                                     // RETURN
    }

    int      idx    = index / k_BITS_PER_UINT64;
    int      pos    = index % k_BITS_PER_UINT64;

    int      remLen = k_BITS_PER_UINT64 - pos;
    int      nb     = bsl::min(numBits, remLen);
    uint64_t ret    = (bitString[idx] >> pos) & BitMaskUtil::lt64(nb);

    numBits -= nb;
    if (numBits <= 0) {
        BSLS_ASSERT_SAFE(0 == numBits);

        return ret;                                                   // RETURN
    }

    ret |= (bitString[idx + 1] & rawLt64(numBits)) << remLen;

    return ret;
}

int BitStringUtil::num1(const uint64_t *bitString, int index, int numBits)
{
    BSLS_ASSERT(bitString);
    BSLS_ASSERT(0 <= index);

    if (numBits <= 0) {
        BSLS_ASSERT(0 == numBits);

        return 0;                                                     // RETURN
    }

    int       beginIdx  = index / k_BITS_PER_UINT64;
    const int beginPos  = index % k_BITS_PER_UINT64;

    bitString += beginIdx;
    beginIdx  =  0;            // note 'beginPos' is unchanged and still valid

    const int lastWord = (beginPos + numBits - 1) / k_BITS_PER_UINT64;

    if (0 == lastWord) {
        // All the bits we are interested in live within a single word.

        return BitUtil::numBitsSet(bitString[0] &
                            BitMaskUtil::one64(beginPos, numBits));   // RETURN
    }

    // We have multiple words to traverse.  The first and last might be partial
    // words, so we have to mask them.  The words in between will all be full
    // words.  Note we are traversing from high order to low order words.

    const int endPos = (beginPos + numBits - 1) % k_BITS_PER_UINT64 + 1;

    int ret = BitUtil::numBitsSet(bitString[lastWord] &
                                                    BitMaskUtil::lt64(endPos));

    // We now want visit all thee words not including the highest and lowest
    // order words in 'bitString'.  We set 'array' such that
    // '&array[0] == &bitString[1]', the first word will will visit in this
    // array (as we predecrement 'ii') will be
    // 'array[lastWord - 2] == bitString[lastWord - 1]'.  The last element we
    // will visit will be 'array[0] == bitString[1]'.

    const uint64_t *array = bitString + 1;
    int             ii    = lastWord
                                     - 1     // word before last word
                                     - 1     // adjust from bitString to array
                                     + 1;    // preparation for pre-decrement

    while (ii >= 8) {
        ret +=       BitUtil::numBitsSet(array[--ii]);
        ret +=       BitUtil::numBitsSet(array[--ii]);
        ret +=       BitUtil::numBitsSet(array[--ii]);
        ret +=       BitUtil::numBitsSet(array[--ii]);

        ret +=       BitUtil::numBitsSet(array[--ii]);
        ret +=       BitUtil::numBitsSet(array[--ii]);
        ret +=       BitUtil::numBitsSet(array[--ii]);
        ret +=       BitUtil::numBitsSet(array[--ii]);
    }

    BSLS_ASSERT_SAFE(ii < 8);

    switch (ii) {
      case 7: ret += BitUtil::numBitsSet(array[--ii]);
      case 6: ret += BitUtil::numBitsSet(array[--ii]);
      case 5: ret += BitUtil::numBitsSet(array[--ii]);
      case 4: ret += BitUtil::numBitsSet(array[--ii]);
      case 3: ret += BitUtil::numBitsSet(array[--ii]);
      case 2: ret += BitUtil::numBitsSet(array[--ii]);
      case 1: ret += BitUtil::numBitsSet(array[--ii]);
      default: ;
    }

    BSLS_ASSERT_SAFE(0 == ii);

    // And we are now ready to look at the lowest-order word.

    return ret + BitUtil::numBitsSet(bitString[0] & rawGe64(beginPos));
}

bsl::ostream& BitStringUtil::print(bsl::ostream&   stream,
                                   const uint64_t *bitString,
                                   int             numBits,
                                   int             level,
                                   int             spacesPerLevel)
{
    BSLS_ASSERT(0 <= numBits);

    if (!stream) {
        return stream;                                                // RETURN
    }

    indent(stream, level, spacesPerLevel);
    stream << '[';
    if (0 == numBits) {
        newlineAndIndent(stream, level, spacesPerLevel);
        stream << ']';
        if (spacesPerLevel >= 0) {
            stream << '\n';
        }
        return stream;                                                // RETURN
    }

    bsl::ios_base::fmtflags oldOptions(stream.flags());
    stream << bsl::hex;

    if (level < 0) {
        level = -level;
    }
    int levelPlus1  = level + 1;

    int endPos          = (numBits      - 1) / k_BITS_PER_UINT64 + 1;
    int lastWordBits    = (numBits      - 1) % k_BITS_PER_UINT64 + 1;
    int lastWordNibbles = (lastWordBits - 1) / 4                + 1;

    if (spacesPerLevel >= 0 && endPos > 4) {
        const int startIdx = ((endPos - 1) / 4 + 1) * 4 - 1;
        BSLS_ASSERT_SAFE(startIdx >= endPos - 1);
        BSLS_ASSERT_SAFE(startIdx % 4 == 3);

        for (int idx = startIdx; idx >= 0; --idx) {
            if (3 == idx % 4) {
                newlineAndIndent(stream, levelPlus1, spacesPerLevel);
            }
            else {
                stream << ' ';
            }

            if      (endPos <= idx) {
                stream << "                ";
            }
            else if (endPos - 1 == idx) {
                putSpaces(stream, 16 - lastWordNibbles);
                stream << bsl::setfill('0') << bsl::setw(lastWordNibbles) <<
                            (bitString[idx] & BitMaskUtil::lt64(lastWordBits));
            }
            else {
                stream << bsl::setfill('0') << bsl::setw(16) << bitString[idx];
            }
        }
    }
    else {
        newlineAndIndent(stream, levelPlus1, spacesPerLevel);

        int idx = endPos - 1;
        stream << bsl::setfill('0') << bsl::setw(lastWordNibbles) <<
                            (bitString[idx] & BitMaskUtil::lt64(lastWordBits));

        while (--idx >= 0) {
            stream << ' ' <<
                          bsl::setfill('0') << bsl::setw(16) << bitString[idx];
        }
    }

    stream.flags(oldOptions);

    if (spacesPerLevel >= 0) {
        newlineAndIndent(stream, level, spacesPerLevel);
    }
    else {
        stream << ' ';
    }

    stream << ']';
    if (spacesPerLevel >= 0) {
        stream << '\n';
    }

    return stream;
}

}  // close package namespace
}  // close enterprise namespace

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
