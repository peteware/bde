// bdlb_bitstringutil.t.cpp                                           -*-C++-*-

#include <bdlb_bitstringutil.h>
#include <bdlb_bitmaskutil.h>

#include <bdls_testutil.h>

#include <bslma_allocator.h>
#include <bslma_default.h>
#include <bsls_asserttest.h>

#include <bsl_algorithm.h>
#include <bsl_iostream.h>
#include <bsl_sstream.h>
#include <bsl_string.h>
#include <bsl_vector.h>

#include <bsl_cstdlib.h>
#include <bsl_cstring.h>
#include <bsl_cctype.h>

#include <bsl_c_limits.h>    // CHAR_BIT

using namespace BloombergLP;
using bsl::cout;
using bsl::cerr;
using bsl::endl;
using bsl::flush;

//=============================================================================
//                             TEST PLAN
//-----------------------------------------------------------------------------
//                                Overview
//                                --------
// The component under test provides static methods that perform various bit
// string related computations.  The goal of this 'bdlb::BitStringUtil' test
// suite is to verify that the methods have the desired effect on bit strings
// and/or return the expected values.  A variety of testing techniques are
// employed here.
//-----------------------------------------------------------------------------
// CLASS METHODS
// [23] int find1AtMinIndex(const uint64_t *bitstring, int length);
// [23] int find1AtMinIndex(const uint64_t *bitstring,
//                          int             index,
//                          int             length);
// [22] int find0AtMinIndex(const uint64_t *bitstring, int length);
// [22] int find0AtMinIndex(const uint64_t *bitstring,
//                          int             index,
//                          int             length);
// [21] int find1AtMaxIndex(const uint64_t *bitstring, int length);
// [21] int find1AtMaxIndex(const uint64_t *bitstring,
//                          int             index,
//                          int             length);
// [20] int find0AtMaxIndex(const uint64_t *bitstring, int length);
// [20] int find0AtMaxIndex(const uint64_t *bitstring,
//                          int             index,
//                          int             length);
// [19] void xorEqual(uint64_t       *dstBitstring,
//                    int             dstIndex,
//                    const uint64_t *srcBitstring,
//                    int             srcIndex,
//                    int             numBits);
// [18] void orEqual(uint64_t       *dstBitstring,
//                   int             dstIndex,
//                   const uint64_t *srcBitstring,
//                   int             srcIndex,
//                   int             numBits);
// [17] void minusEqual(uint64_t       *dstBitstring,
//                      int             dstIndex,
//                      const uint64_t *srcBitstring,
//                      int             srcIndex,
//                      int             numBits);
// [16] void andEqual(uint64_t       *dstBitstring,
//                    int             dstIndex,o
//                    const uint64_t *srcBitstring,
//                    int             srcIndex,
//                    int             numBits);
// [15] void toggle(uint64_t *bitString, int index, int numBits);
// [14] int num0(const uint64_t *bitstring, int index, int numBits);
// [14] int num1(const uint64_t *bitstring, int index, int numBits);
// [13] bsl::ostream& print(bsl::ostream&   stream,
//                          const uint64_t *bitstring,
//                          int             numBits,
//                          int             level = 1,
//                          int             spl = 4,
//                          int             unitsPerLine = 4,
//                          int             bitsPerSubUnit = 8);
// [12] void swapRaw(uint64_t *lhsBitstring,
//                   int       lhsIndex,
//                   uint64_t *rhsBitstring,
//                   int       rhsIndex,
//                   int       numBits);
// [11] void remove(uint64_t *bitstring, int len, int idx, int numBits);
// [11] void removeAndFill0(uint64_t *bitstring, int len, int idx,int numBits);
// [10] void insert(uint64_t *bitstring,
//                  int       length,
//                  int       index,
//                  bool      value,
//                  int       numBits);
// [10] void insert0(uint64_t *bitstring, int length, int index, int numBits);
// [10] void insert1(uint64_t *bitstring, int length, int index, int numBits);
// [10] void insertRaw(uint64_t *bitstring, int length, int index,int numBits);
// [ 9] void copy(uint64_t       *dstBitstring,
//                int             dstIndex,
//                const uint64_t *srcBitstring,
//                int             srcIndex,
//                int             numBits);
// [ 9] void copyRaw(uint64_t       *dstBitstring,
//                   int             dstIndex,
//                   const uint64_t *srcBitstring,
//                   int             srcIndex,
//                   int             numBits);
// [ 7] bool isAny0(const uint64_t *bitstring, int index, int numBits);
// [ 7] bool isAny1(const uint64_t *bitstring, int index, int numBits);
// [ 6] void assignBits(uint64_t *bitstring,
//                      int       index,
//                      uint64_t  srcbits,
//                      int       numBits);
// [ 5] void assign(uint64_t *bitstring, int index, bool value, int numBits);
// [ 5] void assign0(uint64_t *bitstring, int index, int numBits);
// [ 5] void assign1(uint64_t *bitstring, int index, int numBits);
// [ 5] uint64_t bits(const uint64_t *bitstring, int index, int numBits);
// [ 4] void assign(uint64_t *bitstring, int index, bool value);
// [ 4] void assign0(uint64_t *bitstring, int index);
// [ 4] void assign1(uint64_t *bitstring, int index);
// [ 4] bool bit(const uint64_t *bitstring, int index);
// [ 3] bool areEqual(const uint64_t *lhsBitstring,
//                    int             lhsIndex,
//                    const uint64_t *rhsBitstring,
//                    int             rhsIndex,
//                    int             numBits);
// [ 3] bool areEqual(const uint64_t *lhsBitstring,
//                    const uint64_t *rhsBitstring,
//                    int             numBits);
// ----------------------------------------------------------------------------
// [25] USAGE EXAMPLE
// [24] OLD USAGE TEST
// [ 9] TESTING OVERLAPPING COPIES
// [ 8] TESTING NON-OVERLAPPING COPIES
// [ 2] void populateBitString(uint64_t   *bitstring,
//                             int         idx,
//                             const char *ascii);
// [ 2] void populateBitStringHex(uint64_t   *bitstring,
//                                int         idx,
//                                const char *ascii);
// [ 1] BREATHING TEST
// ----------------------------------------------------------------------------

// Note that it was found that Solaris often took about 15 times as long as
// Linux to run thees test cases, so efforts were made to reduce test times
// to 1 or 2 seconds on Linux to avoid test cases timing out on Solaris.
//
// The measures used to accelerate tests were:
//: o Using 'incint' (defined below) to increment indexes and 'numBits' more
//:   aggressively than just doing '++x' every iteration.
//: o Sometime reducing the size of the arrays being tested from 4 words to 3.

// ============================================================================
//                     STANDARD BDE ASSERT TEST FUNCTION
// ----------------------------------------------------------------------------

namespace {

int testStatus = 0;

void aSsErT(bool condition, const char *message, int line)
{
    if (condition) {
        cout << "Error " __FILE__ "(" << line << "): " << message
             << "    (failed)" << endl;

        if (0 <= testStatus && testStatus <= 100) {
            ++testStatus;
        }
    }
}

}  // close unnamed namespace

// ============================================================================
//               STANDARD BDE TEST DRIVER MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT       BDLS_TESTUTIL_ASSERT
#define ASSERTV      BDLS_TESTUTIL_ASSERTV

#define LOOP_ASSERT  BDLS_TESTUTIL_LOOP_ASSERT
#define LOOP0_ASSERT BDLS_TESTUTIL_LOOP0_ASSERT
#define LOOP1_ASSERT BDLS_TESTUTIL_LOOP1_ASSERT
#define LOOP2_ASSERT BDLS_TESTUTIL_LOOP2_ASSERT
#define LOOP3_ASSERT BDLS_TESTUTIL_LOOP3_ASSERT
#define LOOP4_ASSERT BDLS_TESTUTIL_LOOP4_ASSERT
#define LOOP5_ASSERT BDLS_TESTUTIL_LOOP5_ASSERT
#define LOOP6_ASSERT BDLS_TESTUTIL_LOOP6_ASSERT

#define Q            BDLS_TESTUTIL_Q   // Quote identifier literally.
#define P            BDLS_TESTUTIL_P   // Print identifier and value.
#define P_           BDLS_TESTUTIL_P_  // P(X) without '\n'.
#define T_           BDLS_TESTUTIL_T_  // Print a tab (w/o newline).
#define L_           BDLS_TESTUTIL_L_  // current Line number

#define ASSERT_SAFE_PASS(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_PASS(EXPR)
#define ASSERT_SAFE_FAIL(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_FAIL(EXPR)
#define ASSERT_PASS(EXPR)      BSLS_ASSERTTEST_ASSERT_PASS(EXPR)
#define ASSERT_FAIL(EXPR)      BSLS_ASSERTTEST_ASSERT_FAIL(EXPR)
#define ASSERT_OPT_PASS(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_PASS(EXPR)
#define ASSERT_OPT_FAIL(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_FAIL(EXPR)

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

static int verbose = 0;
static int veryVerbose = 0;
static int veryVeryVerbose = 0;

typedef bdlb::BitStringUtil BSU;
typedef bdlb::BitMaskUtil   Mask;
typedef BSU::uint64_t       uint64_t;

enum { k_BITS_PER_UINT64 = BSU::k_BITS_PER_UINT64 };

//=============================================================================
//                  GLOBAL HELPER FUNCTIONS FOR TESTING
//-----------------------------------------------------------------------------

static
void fillWithGarbage(uint64_t *begin, unsigned arraySizeInBytes)
    // This function uses Knuth's MMIX random number generator to fill the
    // specified array 'begin' of specified size 'arraySizeInBytes' with
    // psudo-random garbage.
{
    ASSERT(0 == arraySizeInBytes % sizeof(uint64_t));

    uint64_t *end = begin + arraySizeInBytes / sizeof(uint64_t);

    static uint64_t seed  = 0x0123456789abcdefULL;
    uint64_t        accum = seed;

    for (; begin < end; ++begin) {
        // This is an LCG, so the low-order bits have much poorer quality
        // randomness than the higher order bits, so each word of output is
        // assembled from the high-order bits of two successive iterations.
        // This will give us a period of 2^63 instead of 2^64.

        accum = 6364136223846793005ULL * accum + 1442695040888963407ULL;
        uint64_t firstWord = accum;
        accum = 6364136223846793005ULL * accum + 1442695040888963407ULL;

        *begin = (firstWord & (~0ULL << 32)) | (accum >> 32);
    }

    seed = accum;
}

int IDX_TABLE[] = { 0, 1, 2, 3, 4, 8, 15, 16, 17, 23, 24, 25, 31, 32, 33,
                    63, 64, 65, 127, 128, 129, 191, 192, 193, 254, 255 };
enum { NUM_IDX_TABLE = sizeof IDX_TABLE / sizeof *IDX_TABLE };

enum { SET_UP_ARRAY_DIM = 4 };

static
void setUpArray(uint64_t  array[SET_UP_ARRAY_DIM],
                int      *iteration,
                bool      fast = false)
    // Fill up the specified 'array' with different values depending on
    // the specified '*iteration'.  Increment '*iteration' rapidly if the
    // optionally specified 'fast' is 'true', increment it slowly otherwise.
    //
    // The idea here is to replicate the advantage of table-driven code by
    // having the first 47 values of '*iteration' drive values of the array
    // that normally would have been chosen as special cases by table-driven
    // code.
{
    const int II_MOD = *iteration % SET_UP_ARRAY_DIM;

    switch (*iteration) {
      case 0:
      case 1:
      case 2:
      case 3: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, 0ULL);
        array[II_MOD] = 1;
      } break;
      case 4:
      case 5:
      case 6:
      case 7: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, 0ULL);
        array[II_MOD] = 2;
      } break;
      case 8:
      case 9:
      case 10:
      case 11: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, 0ULL);
        array[II_MOD] = 5;
      } break;
      case 12:
      case 13:
      case 14:
      case 15: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, 0ULL);
        array[II_MOD] = 0x15;
      } break;
      case 16:
      case 17:
      case 18:
      case 19: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, 0ULL);
        array[II_MOD] = ~0ULL;
      } break;
      case 20:
      case 21:
      case 22:
      case 23: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, ~0ULL);
        array[II_MOD] = ~1ULL;
      } break;
      case 24:
      case 25:
      case 26:
      case 27: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, ~0ULL);
        array[II_MOD] = ~2ULL;
      } break;
      case 28:
      case 29:
      case 30:
      case 31: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, ~0ULL);
        array[II_MOD] = ~5ULL;
      } break;
      case 32:
      case 33:
      case 34:
      case 35: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, ~0ULL);
        array[II_MOD] = ~0x15ULL;
      } break;
      case 36:
      case 37:
      case 38:
      case 39: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, ~0ULL);
        array[II_MOD] = 0ULL;
      } break;
      case 40: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, 0ULL);
      } break;
      case 41: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, ~0ULL);
      } break;
      case 42: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, 1ULL);
      } break;
      case 43: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, 5ULL);
      } break;
      case 44: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, 0x15ULL);
      } break;
      case 45: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, ~1ULL);
      } break;
      case 46: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, ~5ULL);
      } break;
      case 47: {
        bsl::fill(array + 0, array + SET_UP_ARRAY_DIM, ~0x15ULL);
      } break;
      default: {
        fillWithGarbage(array, sizeof(array[0]) * SET_UP_ARRAY_DIM);
      }
    }

    // The first 40 cases are just repeating the same theme 4 times each, so
    // if the caller is in a hurry we only take the first of each 4.

    *iteration += fast && *iteration < 40 ? 4 : 1;
}

static inline
void incInt(int *x, const int maxVal)
    // This function is to be used to increment a specified loop counter '*x'
    // with increasing rapidity as it grows, eventually setting it to EXACTLY
    // the specified 'maxVal', after which it will be set to 'maxVal + 1',
    // causing the loop to terminate.
{
    if (maxVal <= *x) {
        ASSERT(maxVal == *x);

        ++*x;
        return;                                                       // RETURN
    }

    *x += bsl::max((*x / 4), 1);
    if (*x > maxVal) {
        *x = maxVal;
    }
}

static inline
int intAbs(int x)
    // Return the absolute value of the specified 'x'.  The behavior is
    // undefined if 'INT_MIN == x'.
{
    return x < 0 ? -x : x;
}

static
bool areBitsEqual(const uint64_t *bitStringLhs,
                  const uint64_t *bitStringRhs,
                  int             index,
                  int             numBits)
    // Return 'true' if the specified 'numBits' of the specified 'bitStringLhs'
    // and the specified 'bitStringRhs' starting at 'index' in both bits
    // strings compare equal and 'false' otherwise.  This is a brute force but
    // reliable way to do simpler comparison than 'BSU::areEqual' is capable
    // of, used for testing before 'areEqual' is vetted.
{
    ASSERT(index >= 0);
    if (numBits <= 0) {
        ASSERT(0 == numBits);

        return true;                                                  // RETURN
    }

    int idx = index / k_BITS_PER_UINT64;
    int pos = index % k_BITS_PER_UINT64;

    const int endIndex = index + numBits;
    const int lastIdx  = (endIndex - 1) / k_BITS_PER_UINT64;
    const int endPos   = (endIndex - 1) % k_BITS_PER_UINT64 + 1;

    for (; idx < lastIdx; ++idx) {
        for (; pos < k_BITS_PER_UINT64; ++pos) {
            if ((bitStringLhs[idx] & (1ULL << pos)) !=
                                         (bitStringRhs[idx] & (1ULL << pos))) {
                return false;                                         // RETURN
            }
        }
        pos = 0;
    }

    for (; pos < endPos; ++pos) {
        if ((bitStringLhs[idx] & (1ULL << pos)) !=
                                         (bitStringRhs[idx] & (1ULL << pos))) {
            return false;                                             // RETURN
        }
    }

    return true;
}

static
int countOnes(const uint64_t *bitString,
              int             idx,
              int             numBits)
    // Count the number of set bits in the specified 'numBits' of specified
    // 'bitString', starting at the specified 'idx'.
{
    int ii = idx / k_BITS_PER_UINT64;
    int jj = idx % k_BITS_PER_UINT64;

    int ret = 0;

    for (int kk = 0; kk < numBits; ++kk) {
        ret += !!(bitString[ii] & (1ULL << jj));

        if (k_BITS_PER_UINT64 == ++jj) {
            jj = 0;
            ++ii;
        }
    }

    return ret;
}

inline
void wordCpy(uint64_t *dst, const uint64_t *src, unsigned sizeInBytes)
    // Copy the specified 'sizeInBytes' bytes from the specified 'src' to the
    // specified 'dst'.  Note this is like 'memcpy' except faster, since it
    // knows it must copy an integral number of alighned 64 bit words.
{
    ASSERT(0 == sizeInBytes % sizeof(uint64_t));

    const uint64_t *endSrc = src + sizeInBytes / sizeof(uint64_t);
    while (src < endSrc) {
        *dst++ = *src++;
    }
}

inline
int wordCmp(const uint64_t *dst, const uint64_t *src, unsigned sizeInBytes)
    // Compare the specified 'sizeInBytes' bytes from the specified 'src' to
    // the specified 'dst'.  Return 0 if they match and a non-zero value
    // otherwise.  Note this is like 'memcmp' except faster, since it knows it
    // must comppare an integral number of alighned 64 bit words.
{
    ASSERT(0 == sizeInBytes % sizeof(uint64_t));

    const uint64_t *endSrc = src + sizeInBytes / sizeof(uint64_t);
    for (; src < endSrc; ++dst, ++src) {
        if (*dst != *src) {
            return *dst > *src ? 1 : -1;                              // RETURN
        }
    }

    return 0;
}

static
bsl::string pHex(const uint64_t *bitString,
                 int             numBits)
    // Return a 'bsl::string' displaying the contents of the first specified
    // 'numBits' of the specified 'bitString', in hex.
{
    bsl::ostringstream oss;

    BSU::print(oss, bitString, numBits, 0, -1);
    return oss.str();
}

static
void populateBitString(uint64_t *bitstring, int index, const char *ascii)
    // Populate the bits starting at the specified 'index' in the specified
    // 'bitstring' with the values specified by the characters in the
    // specified 'ascii'.  The behavior is undefined unless the characters in
    // 'ascii' are either '0' or '1', and 'bitstring' has a capacity of at
    // least 'index + bsl::strlen(ascii)' bits.
{
    ASSERT(bitstring);
    ASSERT(0 <= index);
    ASSERT(ascii);

    int       idx      = index / k_BITS_PER_UINT64;
    int       pos      = index % k_BITS_PER_UINT64;
    uint64_t *wordPtr  = &bitstring[idx];
    int       numChars = bsl::strlen(ascii);

    while (numChars > 0) {
        unsigned char currValue = ascii[numChars - 1];
        if (bsl::isspace(currValue)) {
            --numChars;
            continue;
        }

        ASSERT('0' == currValue || '1' == currValue);

        if (k_BITS_PER_UINT64 == pos) {
            pos = 0;
            ++wordPtr;
        }

        if ('1' == currValue) {
            *wordPtr |=   1ULL << pos;
        }
        else {
            *wordPtr &= ~(1ULL << pos);
        }

        ++pos;
        --numChars;
    }
}

static
void populateBitStringHex(uint64_t *bitstring, int index, const char *ascii)
    // Populate the bits starting at the specified 'index' in the specified
    // 'bitstring' with the values specified by the characters in the specified
    // 'ascii', which is in hex.  The behavior is undefined unless the
    // characters in 'ascii' are either valid hex digits or white space,
    // 'bitstring' has a capacity of at least 'index + bsl::strlen(ascii) * 4'
    // bits, and 'index' is a multiple of 4.
{
    ASSERT(bitstring);
    ASSERT(0 <= index);
    ASSERT(ascii);

    int       idx       = index / k_BITS_PER_UINT64;
    int       pos       = index % k_BITS_PER_UINT64;
    uint64_t *wordPtr   = &bitstring[idx];
    int       NUM_CHARS = bsl::strlen(ascii);

    uint64_t nibble;
    bool     lastCharWasHex = false;
    uint64_t lastNibble     = 17;
    for (int numChars = NUM_CHARS - 1; numChars >= 0;
                                             --numChars, lastNibble = nibble) {
        unsigned char currValue = ascii[numChars];
        if (bsl::isspace(currValue)) {
            continue;
        }

        currValue = static_cast<char>(bsl::tolower(currValue));

        BSLMF_ASSERT('a' > '9');

        int repeat = 1;
        nibble = currValue <= '9'
                 ? (currValue >= '0' ? currValue - '0' : 16)
                 : (currValue >= 'a'
                    ? (currValue <= 'f' ? 10 + currValue - 'a'
                                        : 16)
                    : 16);
        if (nibble < 16) {
            lastCharWasHex = true;
        }
        else {
            ASSERT(17 != lastNibble);
            nibble = lastNibble;
            switch (currValue) {
              case 'y': {    // byte
                repeat = 1;
              } break;
              case 'q': {    // quarter-word
                repeat = 3;
              } break;
              case 'h': {    // half-word (32-bits)
                repeat = 7;
              } break;
              case 'w': {    // word (64 bits)
                repeat = 15;
              } break;
              default: {
                LOOP_ASSERT(currValue, 0 && "Unrecognized char in"
                                                      " populateBitStringHex");
              } break;
            }
            if (!lastCharWasHex) {
                ++repeat;
            }
            lastCharWasHex = false;
        }

        ASSERT(repeat >= 1);
        LOOP_ASSERT(currValue, nibble < 16);

        for (; repeat > 0; --repeat) {
            *wordPtr &= ~(0xfULL << pos);
            *wordPtr |=   nibble << pos;

            if (pos > k_BITS_PER_UINT64 - 4) {
                uint64_t *nextWordPtr = wordPtr + 1;
                int       nextPos     = pos - k_BITS_PER_UINT64;

                  // note nextPos is in range [ -1 .. -3 ]

                *nextWordPtr &= Mask::ge64(nextPos + 4);
                *nextWordPtr |= nibble >> -nextPos;
            }

            pos += 4;
            if (pos >= k_BITS_PER_UINT64) {
                pos -= k_BITS_PER_UINT64;
                ++wordPtr;
            }
        }
    }
}

int numBits(const char *str)
    // Return the number of bits in the specified 'str' ignoring any
    // whitespace.
{
    int n = 0;
    while (*str) {
        if (!bsl::isspace(*str)) {
            ++n;
        }
        ++str;
    }
    return n;
}

int numHexDigits(const char *str)
    // Return the number of bits in the specified 'str' ignoring any
    // whitespace.
{
    bool lastWasHex = false;
    int  n          = 0;
    while (*str) {
        unsigned char currValue = *str;
        if (!bsl::isspace(currValue)) {
            currValue = static_cast<char>(bsl::tolower(currValue));

            switch (currValue) {
              case 'y': {    // byte
                n += 1  + !lastWasHex;
                lastWasHex = false;
              } break;
              case 'q': {    // quarter-word
                n += 3  + !lastWasHex;
                lastWasHex = false;
              } break;
              case 'h': {    // half-word (32-bits)
                n += 7  + !lastWasHex;
                lastWasHex = false;
              } break;
              case 'w': {    // word (64 bits)
                n += 15 + !lastWasHex;
                lastWasHex = false;
              } break;
              default: {
                lastWasHex = true;
                ++n;
              } break;
            }
        }
        ++str;
    }
    return n;
}

bool areEqualOracle(uint64_t *lhs,
                    int       lhsIdx,
                    uint64_t *rhs,
                    int       rhsIdx,
                    int       numBits)
    // Compare the two bit strings of specified length 'numBits' starting at
    // the specified 'lhsIdx' into the specified 'lhs' and the specified
    // 'rhsIdx' into the specified 'rhs' and return 'true' if they match and
    // 'false' otherwise.  This is a really inefficient but reliable way of
    // doing the 'areEqual' function.  Note that we can't use 'bit' because it
    // is not tested until after 'areEqual'.
{
    ASSERT(lhsIdx  >= 0);
    ASSERT(rhsIdx  >= 0);
    ASSERT(numBits >= 0);

    const int endLhsIdx = lhsIdx + numBits;
    for (; lhsIdx < endLhsIdx; ++lhsIdx, ++rhsIdx) {
        const int lhsWord = lhsIdx / k_BITS_PER_UINT64;
        const int lhsPos  = lhsIdx % k_BITS_PER_UINT64;

        const int rhsWord = rhsIdx / k_BITS_PER_UINT64;
        const int rhsPos  = rhsIdx % k_BITS_PER_UINT64;

        if (!(lhs[lhsWord] & (1ULL << lhsPos)) !=
                                          !(rhs[rhsWord] & (1ULL << rhsPos))) {
            return false;                                             // RETURN
        }
    }

    return true;
}

void toggleOracle(uint64_t       *dst,
                  int             dstIdx,
                  int             numBits)
    // Toggle the specified 'numBits' bits of the specified 'dst' beginning at
    // the specified index 'dstIdx'.  This is a really inefficient but reliable
    // way of doing the 'toggle' function, as an oracle for testing.
{
    int endDstIdx = dstIdx + numBits;
    for (; dstIdx < endDstIdx; ++dstIdx) {
        BSU::assign(dst, dstIdx, !BSU::bit(dst, dstIdx));
    }
}

void andOracle(uint64_t       *dst,
               int             dstIdx,
               const uint64_t *src,
               int             srcIdx,
               int             numBits)
    // Bitwise-and the bit strings of specified length 'numBits' starting at
    // the specified 'dstIdx' in the specified 'dst' with the bit string
    // starting at the specified 'srcIdx' in the specified 'src', storing the
    // results in the specified bit string within 'dst'.  This is a really
    // inefficient but reliable way of doing the 'andEqual' function, as an
    // oracle for testing.
{
    int endSrcIdx = srcIdx + numBits;
    for (; srcIdx < endSrcIdx; ++dstIdx, ++srcIdx) {
        BSU::assign(dst, dstIdx, BSU::bit(dst, dstIdx) &&
                                                        BSU::bit(src, srcIdx));
    }
}

void minusOracle(uint64_t       *dst,
                 int             dstIdx,
                 const uint64_t *src,
                 int             srcIdx,
                 int             numBits)
    // Bitwise and the bit string of specified length 'numBits' starting at the
    // specified 'dstIdx' in the specified bit string 'dst' and with the toggle
    // of the bit string starting at the specified 'srcIdx' in the specified
    // 'src', storing the result in the bit string from 'dst'.  This is a
    // really inefficient but reliable way of doing the 'minusEqual' function,
    // as an oracle for testing.
{
    uint64_t toggleSrc[SET_UP_ARRAY_DIM];
    enum { NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64 };

    ASSERT(srcIdx + numBits <= NUM_BITS);
    const int srcWords = (srcIdx + numBits - 1) / k_BITS_PER_UINT64 + 1;
    wordCpy(toggleSrc, src, srcWords * sizeof(uint64_t));
    BSU::toggle(toggleSrc, srcIdx, numBits);

    BSU::andEqual(dst, dstIdx, toggleSrc, srcIdx, numBits);
}

void orOracle(uint64_t       *dst,
              int             dstIdx,
              const uint64_t *src,
              int             srcIdx,
              int             numBits)
    // Bitwise or the bit string of specified length 'numBits' starting at the
    // specified 'dstIdx' in the specified bit string 'dst' and with the bit
    // string starting at the specified 'srcIdx' in the specified 'src',
    // storing the result in the bit string from 'dst'.  This is a really
    // inefficient but reliable way of doing the 'orEqual' function, as an
    // oracle for testing.
{
    int endSrcIdx = srcIdx + numBits;
    for (; srcIdx < endSrcIdx; ++dstIdx, ++srcIdx) {
        BSU::assign(dst, dstIdx, BSU::bit(dst, dstIdx) ||
                                                        BSU::bit(src, srcIdx));
    }
}

void xorOracle(uint64_t       *dst,
               int             dstIdx,
               const uint64_t *src,
               int             srcIdx,
               int             numBits)
    // Bitwise xor the bit string of specified length 'numBits' starting at the
    // specified 'dstIdx' in the specified bit string 'dst' and with the bit
    // string starting at the specified 'srcIdx' in the specified 'src',
    // storing the result in the bit string from 'dst'.  This is a really
    // inefficient but reliable way of doing the 'xorEqual' function, as an
    // oracle for testing.
{
    int endSrcIdx = srcIdx + numBits;
    for (; srcIdx < endSrcIdx; ++dstIdx, ++srcIdx) {
        BSU::assign(dst, dstIdx, BSU::bit(dst, dstIdx) !=
                                                        BSU::bit(src, srcIdx));
    }
}

int findAtMaxOracle(uint64_t *bitString,
                    int       begin,
                    int       end,
                    bool      target)
    // Return the index of the highest order bit that matches the specified
    // 'target' in the bit string starting at the specified index 'begin' and
    // ending before the specified index 'end' in the specified bit string
    // 'bitString'.  This function provides an inefficient but reliable way of
    // doing the 'find*AtMaxIndex' functions for teSTING.
{
    ASSERT(begin >= 0);
    ASSERT(begin <= end);

    if (begin == end) {
        return -1;                                                    // RETURN
    }

    for (int ii = end - 1; ii >= begin; --ii) {
        if (BSU::bit(bitString, ii) == target) {
            return ii;                                                // RETURN
        }
    }

    return -1;
}

int findAtMinOracle(uint64_t *bitString,
                    int       begin,
                    int       end,
                    bool      target)
    // Return the index of the lowest order bit that matches the specified
    // 'target' in the bit string starting at the specified index 'begin' and
    // ending before the specified index 'end' in the specified bit string
    // 'bitString'.  This function provides an inefficient but reliable way of
    // doing the 'find*AtMaxIndex' functions for teSTING.
{
    ASSERT(begin >= 0);
    ASSERT(begin <= end);

    if (begin == end) {
        return -1;                                                    // RETURN
    }

    for (int ii = begin; ii < end; ++ii) {
        if (BSU::bit(bitString, ii) == target) {
            return ii;                                                // RETURN
        }
    }

    return -1;
}

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? bsl::atoi(argv[1]) : 0;
    verbose = argc > 2;
    veryVerbose = argc > 3;
    veryVeryVerbose = argc > 4;

    cout << "TEST " << __FILE__ << " CASE " << test << bsl::endl;

    switch (test) { case 0:  // Zero is always the leading case.
      case 25: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE
        //   Copied into component header file.
        //
        // Concerns:
        //: 1 The usage example provided in the component header file compiles,
        //:   links, and runs as shown.
        //
        // Plan:
        //: 1 Get usage example compiling and working in test driver.
        //: 2 Incorporate usage example from test driver into header, replace
        //:   leading comment characters with spaces, replace 'ASSERT' with
        //:   'assert', and remove 'if (veryVerbose)' qualifiying all output
        //:   operations.  (C-1)
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING USAGE EXAMPLE\n"
                             "=====================\n";

///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Keeping a calendar of business days
/// - - - - - - - - - - - - - - - - - - - - - - -
// Bitstreams can be used to represent efficient business calendars and do
// operations on such calendars.  First, create a 'Bitstream' with sufficient
// capacity to represent every day of a year (note that 32 * 12 = 384) and set
// a 1-bit in the indices corresponding to the day-of-year (DOY) for each
// weekend day.  For convenience in date calculations the 0 index is not used;
// the 365 days of the year are at indices [1,365].  Further note that the
// values set below all correspond to the year 2013:
//..
    uint64_t weekends[6] = {0};

    // Start on the first Saturday of the year, Jan 5, 2013.

    for (int i = 5; i < 366; i += 7) {
        bdlb::BitStringUtil::assign(weekends, i,   1);
        if (i + 1 < 366) {
            bdlb::BitStringUtil::assign(weekends, i+1, 1);
        }
    }
//..
// Now, we can easily use 'bdeu_bitstreamutil' methods to find days of
// interest.  For example:
//..
    int firstWeekendDay = bdlb::BitStringUtil::find1AtMinIndex(weekends,
                                                               365 + 1);
    int lastWeekendDay  = bdlb::BitStringUtil::find1AtMaxIndex(weekends,
                                                               365 + 1);
//..
// We can define an enumeration to assist us in representing these DOY values
// into convention dates and confirm the calculated values:
//..
    enum {
        JAN = 0,  // Note: First DOY is 'JAN + 1'.
        FEB = JAN + 31,
        MAR = FEB + 28,
        APR = MAR + 31,
        MAY = APR + 30,
        JUN = MAY + 31,
        JUL = JUN + 30,
        AUG = JUL + 31,
        SEP = AUG + 31,
        OCT = SEP + 30,
        NOV = OCT + 31,
        DEC = NOV + 30
    };

    ASSERT(JAN +  5 == firstWeekendDay);
    ASSERT(DEC + 29 ==  lastWeekendDay);
//..
// The enumeration allows us to easily represent the business holidays of the
// year and significant dates in the business calendar:
//..
    uint64_t holidays[6] = {0};

    enum {
        NEW_YEARS_DAY    = JAN +  1,
        MLK_DAY          = JAN + 21,
        PRESIDENTS_DAY   = FEB + 18,
        GOOD_FRIDAY      = MAR + 29,
        MEMORIAL_DAY     = MAY + 27,
        INDEPENDENCE_DAY = JUL +  4,
        LABOR_DAY        = SEP +  2,
        THANKSGIVING     = NOV + 28,
        CHRISTMAS        = DEC + 25
    };

    bdlb::BitStringUtil::assign(holidays, NEW_YEARS_DAY,    true);
    bdlb::BitStringUtil::assign(holidays, MLK_DAY,          true);
    bdlb::BitStringUtil::assign(holidays, PRESIDENTS_DAY,   true);
    bdlb::BitStringUtil::assign(holidays, GOOD_FRIDAY,      true);
    bdlb::BitStringUtil::assign(holidays, MEMORIAL_DAY,     true);
    bdlb::BitStringUtil::assign(holidays, INDEPENDENCE_DAY, true);
    bdlb::BitStringUtil::assign(holidays, LABOR_DAY,        true);
    bdlb::BitStringUtil::assign(holidays, THANKSGIVING,     true);
    bdlb::BitStringUtil::assign(holidays, CHRISTMAS,        true);

    enum {
        Q1 = JAN + 1,
        Q2 = APR + 1,
        Q3 = JUN + 1,
        Q4 = OCT + 1
    };
//..
// Now, we can query our calendar for the first holiday in the third quarter,
// if any:
//..
    int firstHolidayOfQ3 = bdlb::BitStringUtil::find1AtMinIndex(holidays,
                                                                Q3,
                                                                Q4);
    ASSERT(INDEPENDENCE_DAY == firstHolidayOfQ3);
//..
//  Our calendars are readily combined to represent days off for either reason
//  (i.e., holiday or weekend):
//..
    uint64_t allDaysOff[6] = {0};
    bdlb::BitStringUtil::orEqual(allDaysOff, 1, weekends, 1, 365);
    bdlb::BitStringUtil::orEqual(allDaysOff, 1, holidays, 1, 365);

    bool isOffMay24 = bdlb::BitStringUtil::bit(allDaysOff, MAY + 24);
    bool isOffMay25 = bdlb::BitStringUtil::bit(allDaysOff, MAY + 25);
    bool isOffMay26 = bdlb::BitStringUtil::bit(allDaysOff, MAY + 26);
    bool isOffMay27 = bdlb::BitStringUtil::bit(allDaysOff, MAY + 27);
    bool isOffMay28 = bdlb::BitStringUtil::bit(allDaysOff, MAY + 28);

    ASSERT(false == isOffMay24);
    ASSERT(true  == isOffMay25);
    ASSERT(true  == isOffMay26);
    ASSERT(true  == isOffMay27);    // Note May 27, 2013 is Memorial Day.
    ASSERT(false == isOffMay28);
//..
      } break;
      case 24: {
        // --------------------------------------------------------------------
        // OLD USAGE TEST
        //   Ensure the old usage example still works.
        //
        // Concerns:
        //: 1 This was evidently used as a usage example at some point in the
        //:   past, but is not longer used in the component doc.  It is kept
        //:   around because, since it's done, it providess some free testing.
        //
        // Plan:
        //: 1 Compile and execute the old usage example.
        //
        // Testing:
        //   OLD USAGE TEST
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "OLD USAGE TEST\n"
                             "==============\n";

// This is an old usage test, no longer used in the interface file, just
// maintained for some free testing.
//..
        uint64_t S[] = { 0xB, 0x0 };

        ASSERT(BSU::areEqual(S, 0, S, 0, 10)  ==  true);
        ASSERT(BSU::areEqual(S, 3, S, 1,  2)  ==  true);
        ASSERT(BSU::areEqual(S, 3, S, 0,  3)  ==  false);

        ASSERT(BSU::isAny0(S, 0, 2)           ==  false);
        ASSERT(BSU::isAny0(S, 4, 6)           ==  true);
        ASSERT(BSU::isAny1(S, 0, 2)           ==  true);
        ASSERT(BSU::isAny1(S, 4, 6)           ==  false);

        ASSERT(BSU::num0(S, 0, 2)             ==  0);
        ASSERT(BSU::num0(S, 4, 6)             ==  6);
        ASSERT(BSU::num1(S, 0, 2)             ==  2);
        ASSERT(BSU::num1(S, 4, 6)             ==  0);

        ASSERT(BSU::find0AtMaxIndex(S, 10)    ==  9);
        ASSERT(BSU::find1AtMaxIndex(S, 10)    ==  3);
        ASSERT(BSU::find0AtMinIndex(S, 10)    ==  2);
        ASSERT(BSU::find1AtMinIndex(S, 10)    ==  0);

        ASSERT(BSU::find0AtMaxIndex(S, 4, 10) ==  9);
        ASSERT(BSU::find1AtMaxIndex(S, 3, 10) ==  3);
        ASSERT(BSU::find0AtMinIndex(S, 4, 10) ==  4);
        ASSERT(BSU::find1AtMinIndex(S, 3, 10) ==  3);

        ASSERT(BSU::bit (S, 0)                       ==  1);
        ASSERT(BSU::bit (S, 1)                       ==  1);
        ASSERT(BSU::bit (S, 2)                       ==  0);
        ASSERT(BSU::bits(S, 0, 1)                    ==  1);
        ASSERT(BSU::bits(S, 0, 2)                    ==  3);
        ASSERT(BSU::bits(S, 0, 3)                    ==  3);
        ASSERT(BSU::bits(S, 1, 9)                    ==  5);
        ASSERT(BSU::bits(S, 4, 9)                    ==  0);
        ASSERT(BSU::bits(S, 0, 64)                   == 11);

        const char         *EXP = "    [\n        00b\n    ]\n";
        bsl::ostringstream  stream;
        BSU::print(stream, S, 10);
        LOOP2_ASSERT(EXP, stream.str(), EXP == stream.str());

///Manipulators
/// - - - - - -
// The following manipulator methods operate on a single bitstring.
//..
//                                    index: 9  8  7  6  5  4  3  2  1  0   Ln
//                                           -  -  -  -  -  -  -  -  -  -   --
//    Source Argument                    D:  0  0  0  0  0  0  1  0  1  1   10
//
//    Example Usage                   Destination After Operation           Ln
//    =============                   ===================================  ====
//    assign (&D, 10, V, 2)           [V  V] 0  0  0  0  0  0  1  0  1  1  [12]

        {
            uint64_t D0 = 0xB;
            BSU::assign(&D0, 10, false, 2);
            uint64_t E0 = 0x0;
            populateBitString(&E0, 0, "000000001011");
            ASSERT(E0 == D0);

            uint64_t D1 = 0xB;
            BSU::assign(&D1, 10, true, 2);
            uint64_t E1 = 0x0;
            populateBitString(&E1, 0, "110000001011");

            ASSERT(E1 == D1);
        }

//  assign0(&D, 10, 2)              [0  0] 0  0  0  0  0  0  1  0  1  1  [12]

        {
            uint64_t D = 0xB;
            BSU::assign0(&D, 10, 2);
            uint64_t E = 0x0;
            populateBitString(&E, 0, "000000001011");
            ASSERT(E == D);
        }

//  assign1(&D, 10, 2)              [1  1] 0  0  0  0  0  0  1  0  1  1  [12]

        {
            uint64_t D = 0xB;
            BSU::assign1(&D, 10, 2);
            uint64_t E = 0x0;
            populateBitString(&E, 0, "110000001011");
            ASSERT(E == D);
        }

//  insert   (&D, 10, 4, V, 2)       0  0  0  0  0  0 [V  V] 1  0  1  1  [12]

        {
            uint64_t D0 = 0xB;
            BSU::insert(&D0, 10, 4, false, 2);
            uint64_t E0 = 0x0;
            populateBitString(&E0, 0, "000000001011");
            ASSERT(E0 == D0);

            uint64_t D1 = 0xB;
            BSU::insert(&D1, 10, 4, true, 2);
            uint64_t E1 = 0x0;
            populateBitString(&E1, 0, "000000111011");
            ASSERT(E1 == D1);
        }

//  insert0  (&D, 10, 4, 2)          0  0  0  0  0  0 [0  0] 1  0  1  1  [12]

        {
            uint64_t D = 0xB;
            BSU::insert0(&D, 10, 4, 2);
            uint64_t E = 0x0;
            populateBitString(&E, 0, "000000001011");
            ASSERT(E == D);
        }

//  insert1  (&D, 10, 4, 2)          0  0  0  0  0  0 [1  1] 1  0  1  1  [12]

        {
            uint64_t D = 0xB;
            BSU::insert1(&D, 10, 4, 2);
            uint64_t E = 0x0;
            populateBitString(&E, 0, "000000111011");
            ASSERT(E == D);
        }

//  insertRaw(&D, 10, 4, 2)          0  0  0  0  0  0 [X  X] 1  0  1  1  [12]

        {
            uint64_t D = 0xB;
            BSU::insertRaw(&D, 10, 4, 2);
            uint64_t E = 0x0;
            populateBitString(&E, 0, "000000001011");
            ASSERT(E == (D & 0x0FF));
            ASSERT(E == (D & 0x3FF));
        }

//  removeAndFill0  (&D, 10, 3, 3)        [0  0  0] 0 [0  0  0] 0  1  1   10

        {
            uint64_t D = 0xB;
            BSU::removeAndFill0(&D, 10, 3, 3);
            uint64_t E = 0x0;
            populateBitString(&E, 0, "0000000011");
            ASSERT(E == D);
        }

//  remove(&D, 10, 3, 3)                            0 [0  0  0] 0  1  1  [ 7]

        {
            uint64_t D = 0xB;
            BSU::remove(&D, 10, 3, 3);
            uint64_t E = 0x0;
            populateBitString(&E, 0, "0000011");
            ASSERT(E == (D & 0x7F));
        }

//  set    (&D, 5, 1)                      0  0  0  0 [1] 0  1  0  1  1   10

        {
            uint64_t D = 0xB;
            BSU::assign(&D, 5, 1);
            uint64_t E = 0x0;
            populateBitString(&E, 0, "0000101011");
            ASSERT(E == D);
        }

//  set   (&D, 6, true, 2)                 0  0 [1  1] 0  0  1  0  1  1   10

        {
            uint64_t D = 0xB;
            BSU::assign(&D, 6, true, 2);
            uint64_t E = 0x0;
            populateBitString(&E, 0, "0011001011");
            ASSERT(E == D);
        }

//  toggle(&D, 2, 3)                       0  0  0  0  0 [1  0  1] 1  1   10

        {
            uint64_t D = 0xB;
            BSU::toggle(&D, 2, 3);
            uint64_t E = 0x0;
            populateBitString(&E, 0, "0000010111");
            ASSERT(E == D);
        }

//..
// There is also a 'swapRaw' method in which the second bitstring is
// non-'const'.
//..
//                                    index: 9  8  7  6  5  4  3  2  1  0   Ln
//                                           -  -  -  -  -  -  -  -  -  -   --
//    Arguments                         S1:  0  0  0  0  0  0  1  0  1  1   10
//                                      S2:        1  0  1  0  1  0  0  1    8
//
//    Example Usage                     Bitstrings After Operation          Ln
//    =============                     =================================  ====
//    swapRaw(&S1, 6, &S2, 4, 2)        S1:  0  0 [1  0] 0  0  1  0  1  1   10
//                                      S2:        1  0 [0  0] 1  0  0  1    8

        {
            uint64_t S1 = 0x0B;
            uint64_t S2 = 0xA9;
            BSU::swapRaw(&S1, 6, &S2, 4, 2);
            uint64_t ES1 = 0x0;
            populateBitString(&ES1, 0, "0010001011");
            uint64_t ES2 = 0x0;
            populateBitString(&ES2, 0,   "10001001");
            ASSERT(ES1 == S1);
            ASSERT(ES2 == S2);
        }
//..
      } break;
      case 23: {
        // --------------------------------------------------------------------
        // TESTING 'find1AtMinIndex' METHODS
        //   Ensure the methods return the expected value.
        //
        // Concerns:
        //: 1 Test both 'find1AtMinIndex' functions over a wide range of valid
        //:   inputs.
        //: 2 Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: 1 Use table-driven code to test both 'find1AtMinIndex' and the
        //:   'findAtMinOracle' function in this test driver.
        //: 2 Iterate over different test arrays with 'setUpArray'.
        //:   o Iterate over 'length' values from 0 to the size of the array.
        //:     1 Find the min 1 bit, if any, in the array using both
        //:       'find1AtMinIndex' and 'findAtMinOracle' and verify their
        //:       results match.
        //:     2 Iterate 'index' from 0 to 'length'.
        //:       o Find the min 1 bit, if any, in the array using both
        //:         'find1AtMinIndex' and 'findAtMinOracle' for the given
        //:         'index' and 'length' and verify their results match.
        //: 3 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //
        // Testing:
        //   int find1AtMinIndex(const uint64_t *bitstring, int length);
        //   int find1AtMinIndex(const uint64_t *bitstring,
        //                       int             begin,
        //                       int             end);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING 'find1AtMinIndex' METHODS\n"
                          << "=================================\n";

        const struct {
            int         d_line;
            const char *d_array_p;
            int         d_index;
            int         d_smallestIdx;
            int         d_lessThanIdx;
            int         d_lessThanEqualIdx;
            int         d_greaterThanIdx;
            int         d_greaterThanEqualIdx;
        } DATA [] = {
// Line  array              idx    S    LT   LE    GT    GE
// ----  -----              ---   --    --   --    --    --
{   L_,  "1",                 0,   0,   -1,   0,   -1,    0   },
{   L_,  "0",                 0,  -1,   -1,  -1,   -1,   -1   },
{   L_,  "00",                0,  -1,   -1,  -1,   -1,   -1   },
{   L_,  "00",                1,  -1,   -1,  -1,   -1,   -1   },

{   L_,  "01",                0,   0,   -1,   0,   -1,    0   },
{   L_,  "01",                1,   0,    0,   0,   -1,   -1   },
{   L_,  "10",                0,   1,   -1,  -1,    1,    1   },
{   L_,  "10",                1,   1,   -1,   1,   -1,    1   },
{   L_,  "11",                0,   0,   -1,   0,    1,    0   },
{   L_,  "11",                1,   0,    0,   0,   -1,    1   },

{   L_,  "10010000 11110000 11110000 11110000",
                              2,   4,   -1,  -1,    4,    4   },
{   L_,  "10010000 11110000 11110000 00001011",
                              2,   0,    0,   0,    3,    3   },
{   L_,  "00000000 00000000 00000000 00000000",
                              0,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "0 00000000 00000000 00000000 00000000",
                              0,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "00000000 00000000 00000000 00000000",
                              2,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "00000000 00000000 00000000 00000000",
                             31,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "0 00000000 00000000 00000000 00000000",
                             31,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "11110000 11110000 11110000 11110000",
                              15,   4,   4,   4,   20,    15   },
{   L_,  "11110000 11110000 01110000 11110000",
                              15,   4,   4,   4,   20,    20   },
{   L_,  "10010000 11110000 11110000 11110000",
                              0,    4,  -1,  -1,    4,     4   },
{   L_,  "00010000 11110000 11110000 11110001",
                              0,    0,  -1,   0,    4,     0   },
{   L_,  "11010000 11110000 11110000 11110000",
                             30,    4,   4,   4,   31,    30   },
{   L_,  "01010000 11110000 11110000 11110000",
                             30,    4,   4,   4,   -1,    30   },
{   L_,  "0 11010000 11110000 11110000 11111111",
                             30,    0,   0,   0,   31,    30   },
{   L_,  "01 01010000 11110000 11110000 11111111",
                             30,    0,   0,   0,   32,    30   },
{   L_,  "01010000 11110000 11110000 11110001",
                             31,    0,   0,   0,   -1,    -1   },
{   L_,  "10010000 11110000 11110000 11110001",
                             31,    0,   0,   0,   -1,    31   },
{   L_,  "0 10010000 11110001 00000000 00000000",
                             31,   16,  16,  16,   -1,    31   },
{   L_,  "1 10010000 11110001 00000000 00000000",
                             31,   16,  16,  16,   32,    31   },
{   L_,  "0 00010000 11110001 00000000 00000000",
                             32,   16,  16,  16,   -1,    -1   },
{   L_,  "1 00010000 11110001 00000000 00000000",
                             32,   16,  16,  16,   -1,    32   },
{   L_,
"10010000 11110000 11110000 11110000 00010000 11110000 11110000 00011111",
                             0,     0,  -1,   0,    1,     0   },
{   L_,
"00010000 11110000 11110000 11110000 00010000 11110000 11110000 00011110",
                             0,     1,  -1,  -1,    1,     1   },
{   L_,
"10010000 11110000 11110000 11110000 00010000 11110000 11110000 00001111",
                            63,     0,   0,   0,   -1,    63   },
{   L_,
"00010000 11110000 11110000 11110000 00010000 11110000 11110000 00001111",
                            63,     0,   0,   0,   -1,    -1   },
{   L_,
"1 10010000 11110000 11110000 11110000 00010000 11110000 11110000 00001111",
                            63,     0,   0,   0,   64,    63   },
{   L_,
"0 10010000 11110000 11110000 11110000 00010000 11110000 11110000 00001111",
                            63,     0,   0,   0,   -1,    63   },
{   L_,
"0 00010000 11110000 11110000 11110000 00010000 11110000 11110000 00001111",
                            63,     0,   0,   0,   -1,    -1   },
{   L_,
"00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000",
                             0,   -1,   -1,  -1,   -1,    -1   },
{   L_,
"000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000",
                             0,   -1,   -1,  -1,   -1,    -1   },
{   L_,
"00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000",
                            63,   -1,   -1,  -1,   -1,    -1   },
{   L_,
"0 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000",
                            63,   -1,   -1,  -1,   -1,    -1   },
{ L_,
                                                                "00001000"
"00000000 00000000 00000000 00000000  00000000 00000000 00000000 00000000"
"00000000 00000000 00000000 00000000  00000000 00000000 00000000 00000010"
"00000000 00000000 00000000 00000000  00000000 00000000 00000000 00000000",
                            130,  65,   65,  65,  195,   195   },

        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        const int MAX_ARRAY_SIZE = 4;
        for (int i = 0; i < NUM_DATA; ++i) {
            const int   LINE      = DATA[i].d_line;
            const char *STR       = DATA[i].d_array_p;
            const int   LEN       = numBits(STR);
            const int   IDX       = DATA[i].d_index;
            const int   EXP_S     = DATA[i].d_smallestIdx;
            const int   EXP_GT    = DATA[i].d_greaterThanIdx;
            const int   EXP_GE    = DATA[i].d_greaterThanEqualIdx;
            const int   EXP_LT    = DATA[i].d_lessThanIdx;
            const int   EXP_LE    = DATA[i].d_lessThanEqualIdx;

            if (veryVerbose) {
                P_(LINE) P_(STR) P_(IDX) P_(EXP_S) P_(EXP_LT) P_(EXP_LE)
                P_(EXP_GT) P_(EXP_GE)
            }

            uint64_t array[MAX_ARRAY_SIZE] = { 0 };
            populateBitString(array, 0, STR);

            if (veryVerbose) {
                BSU::print(cout, array, MAX_ARRAY_SIZE * k_BITS_PER_UINT64);
            }

            const int S  = BSU::find1AtMinIndex(array, LEN);
            LOOP3_ASSERT(LINE,  S, EXP_S,   S == EXP_S);
            ASSERT(EXP_S  == findAtMinOracle(array, 0, LEN, true));

            const int GT = BSU::find1AtMinIndex(array, IDX + 1, LEN);
            LOOP3_ASSERT(LINE, GT, EXP_GT, GT == EXP_GT);
            ASSERT(EXP_GT == findAtMinOracle(array, IDX + 1, LEN, true));

            const int GE = BSU::find1AtMinIndex(array, IDX, LEN);
            LOOP3_ASSERT(LINE, GE, EXP_GE, GE == EXP_GE);
            ASSERT(EXP_GE == findAtMinOracle(array, IDX, LEN, true));

            const int LT = BSU::find1AtMinIndex(array, IDX);
            LOOP3_ASSERT(LINE, LT, EXP_LT, LT == EXP_LT);
            ASSERT(EXP_LT == findAtMinOracle(array, 0, IDX, true));

            const int LE = BSU::find1AtMinIndex(array, IDX + 1);
            LOOP3_ASSERT(LINE, LE, EXP_LE, LE == EXP_LE);
            ASSERT(EXP_LE == findAtMinOracle(array, 0, IDX + 1, true));
        }

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t bits[SET_UP_ARRAY_DIM], control[SET_UP_ARRAY_DIM];

        for (int ii = 0; ii < 150;) {
            setUpArray(control, &ii);
            wordCpy(bits, control, sizeof(bits));

            if (veryVerbose) {
                P_(ii);    P(pHex(bits, NUM_BITS));
            }

            for (int length = 0; length <= NUM_BITS; ++length) {
                {
                    const int EXP = findAtMinOracle(bits, 0, length, true);
                    if (-1 == EXP) {
                        ASSERT(! BSU::isAny1(bits, 0, length));
                    }
                    else {
                        ASSERT(EXP < length);
                        ASSERT(BSU::bit(bits, EXP));
                        ASSERT(! BSU::isAny1(bits, 0, EXP));
                    }

                    ASSERT(EXP == BSU::find1AtMinIndex(bits, length));
                }

                for (int idx = 0; idx <= length; ++idx) {
                    const int EXP = findAtMinOracle(bits, idx, length, true);
                    if (-1 == EXP) {
                        ASSERT(! BSU::isAny1(bits, idx, length - idx));
                    }
                    else {
                        ASSERT(idx <= EXP && EXP < length);
                        ASSERT(BSU::bit(bits, EXP));
                        ASSERT(! BSU::isAny1(bits, idx, EXP - idx));
                    }

                    ASSERT(EXP == BSU::find1AtMinIndex(bits, idx, length));
                }
            }

            ASSERT(0 == wordCmp(bits, control, sizeof(bits)));
        }

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::find1AtMinIndex(bits, 0));
            ASSERT_PASS(BSU::find1AtMinIndex(bits, 100));
            ASSERT_FAIL(BSU::find1AtMinIndex(bits,  -1));

            ASSERT_PASS(BSU::find1AtMinIndex(bits, 0));
            ASSERT_PASS(BSU::find1AtMinIndex(bits, 0, 100));
            ASSERT_PASS(BSU::find1AtMinIndex(bits, 10, 100));
            ASSERT_FAIL(BSU::find1AtMinIndex(bits, 0,  -1));
            ASSERT_FAIL(BSU::find1AtMinIndex(bits, 10,  -1));
            ASSERT_FAIL(BSU::find1AtMinIndex(bits, -1, 100));
        }
      } break;
      case 22: {
        // --------------------------------------------------------------------
        // TESTING 'find0AtMinIndex' METHODS
        //   Ensure the methods return the expected value.
        //
        // Concerns:
        //: o Test that both 'find0AtMinIndex' functions work properly over a
        //:   wide range of valid inputs.
        //: o Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: 1 Use table-driven code to test both 'find0AtMinIndex' and the
        //:   'findAtMinOracle' function in this test driver.
        //: 2 Iterate over different test arrays with 'setUpArray'.
        //:   o Iterate over 'length' values from 0 to the size of the array.
        //:     1 Find the min 0 bit, if any, in the array using both
        //:       'find0AtMinIndex' and 'findAtMinOracle' and verify their
        //:       results match.
        //:     2 Iterate 'index' from 0 to 'lenght'.
        //:       o Find the min 0 bit, if any, in the array using both
        //:         'find0AtMinIndex' and 'findAtMinOracle' for the given
        //:         'index' and 'length' and verify their results match.
        //: 3 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //
        // Testing:
        //   int find0AtMinIndex(const uint64_t *bitstring, int length);
        //   int find0AtMinIndex(const uint64_t *bitstring,
        //                       int             begin,
        //                       int             end);
        // --------------------------------------------------------------------

        if (verbose) cout << "TESTING 'find0AtMinIndex' METHODS\n"
                          << "=================================\n";

        const struct {
            int         d_line;
            const char *d_array_p;
            int         d_index;
            int         d_smallestIdx;
            int         d_lessThanIdx;
            int         d_lessThanEqualIdx;
            int         d_greaterThanIdx;
            int         d_greaterThanEqualIdx;
        } DATA [] = {
// Line  array              idx    S    LT   LE    GT    GE
// ----  -----              ---   --    --   --    --    --
{   L_,  "1",                 0,  -1,   -1,  -1,   -1,   -1   },
{   L_,  "0",                 0,   0,   -1,   0,   -1,    0   },
{   L_,  "00",                0,   0,   -1,   0,    1,    0   },
{   L_,  "00",                1,   0,    0,   0,   -1,    1   },

{   L_,  "01",                0,   1,   -1,  -1,    1,    1   },
{   L_,  "01",                1,   1,   -1,   1,   -1,    1   },
{   L_,  "10",                0,   0,   -1,   0,   -1,    0   },
{   L_,  "10",                1,   0,    0,   0,   -1,   -1   },
{   L_,  "11",                0,  -1,   -1,  -1,   -1,   -1   },
{   L_,  "11",                1,  -1,   -1,  -1,   -1,   -1   },

{   L_,  "10010000 11110000 11110000 00001111",
                              2,   4,   -1,  -1,    4,    4   },
{   L_,  "10010000 11110000 11110000 00001011",
                              2,   2,   -1,   2,    4,    2   },
{   L_,  "11111111 11111111 11111111 11111111",
                              0,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "11111111 11111111 11111111 11111111",
                              2,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "11111111 11111111 11111111 11111111",
                             31,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "1 11111111 11111111 11111111 11111111",
                             31,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "11110000 11110000 11110000 11110000",
                              15,   0,   0,   0,   16,    16   },
{   L_,  "11110000 11110000 01110000 11110000",
                              15,   0,   0,   0,   16,    15   },
{   L_,  "10010000 11110000 11110000 11110000",
                              0,    0,  -1,   0,    1,     0   },
{   L_,  "00010000 11110000 11110000 11110001",
                              0,    1,  -1,  -1,    1,     1   },
{   L_,  "10010000 11110000 11110000 11110000",
                             30,    0,   0,   0,   -1,    30   },
{   L_,  "00010000 11110000 11110000 11110000",
                             30,    0,   0,   0,   31,    30   },
{   L_,  "0 00010000 11110000 11110000 11111111",
                             30,    8,   8,   8,   31,    30   },
{   L_,  "00 00010000 11110000 11110000 11111111",
                             30,    8,   8,   8,   31,    30   },
{   L_,  "10010000 11110000 11110000 11110000",
                             31,    0,   0,   0,   -1,    -1   },
{   L_,  "00010000 11110000 11110000 11110000",
                             31,    0,   0,   0,   -1,    31   },
{   L_,  "1 00010000 11110000 11111111 11111111",
                             31,   16,  16,  16,   -1,    31   },
{   L_,  "0 00010000 11110000 11111111 11111111",
                             31,   16,  16,  16,   32,    31   },
{   L_,  "1 00010000 11110000 11111111 11111111",
                             32,   16,  16,  16,   -1,    -1   },
{   L_,  "0 00010000 11110000 11111111 11111111",
                             32,   16,  16,  16,   -1,    32   },
{   L_,
"10010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                             0,     0,  -1,   0,    1,     0   },
{   L_,
"00010000 11110000 11110000 11110000 00010000 11110000 11110000 11110001",
                             0,     1,  -1,  -1,    1,     1   },
{   L_,
"00010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,     0,   0,   0,   -1,    63   },
{   L_,
"10010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,     0,   0,   0,   -1,    -1   },
{   L_,
"0 00010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,     0,   0,   0,   64,    63   },
{   L_,
"1 00010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,     0,   0,   0,   -1,    63   },
{   L_,
"1 10010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,     0,   0,   0,   -1,    -1   },
{   L_,
"111111111 11111111 11111111 11111111 11111111 11111111 11111111 111111111",
                             0,   -1,   -1,  -1,   -1,    -1   },
{   L_,
"11 111111111 11111111 11111111 11111111 11111111 11111111 11111111 111111111",
                             0,   -1,   -1,  -1,   -1,    -1   },
{   L_,
"111111111 11111111 11111111 11111111 11111111 11111111 11111111 111111111",
                            63,   -1,   -1,  -1,   -1,    -1   },
{   L_,
"1 111111111 11111111 11111111 11111111 11111111 11111111 11111111 111111111",
                            63,   -1,   -1,  -1,   -1,    -1   },
{ L_,
                                                                "11110111"
"11111111 11111111 11111111 11111111  11111111 11111111 11111111 11111111"
"11111111 11111111 11111111 11111111  11111111 11111111 11111111 11111101"
"11111111 11111111 11111111 11111111  11111111 11111111 11111111 11111111",
                            69,   65,   65,  65,  195,   195   },
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        for (int i = 0; i < NUM_DATA; ++i) {
            const int   LINE      = DATA[i].d_line;
            const char *STR       = DATA[i].d_array_p;
            const int   LEN       = numBits(STR);
            const int   IDX       = DATA[i].d_index;
            const int   EXP_S     = DATA[i].d_smallestIdx;
            const int   EXP_GT    = DATA[i].d_greaterThanIdx;
            const int   EXP_GE    = DATA[i].d_greaterThanEqualIdx;
            const int   EXP_LT    = DATA[i].d_lessThanIdx;
            const int   EXP_LE    = DATA[i].d_lessThanEqualIdx;

            if (veryVerbose) {
                P_(LINE) P_(STR) P_(IDX) P_(EXP_S) P_(EXP_LT) P_(EXP_LE)
                P_(EXP_GT) P_(EXP_GE)
            }

            const int MAX_ARRAY_SIZE = 4;

            uint64_t array[MAX_ARRAY_SIZE];
            bsl::memset(array, 0xFF, sizeof array);
            populateBitString(array, 0, STR);

            if (veryVerbose) {
                BSU::print(cout, array, MAX_ARRAY_SIZE * k_BITS_PER_UINT64);
            }

            const int S  = BSU::find0AtMinIndex(array, LEN);
            LOOP3_ASSERT(LINE, S, EXP_S, S == EXP_S);
            ASSERT(EXP_S  == findAtMinOracle(array, 0, LEN, false));

            const int GT = BSU::find0AtMinIndex(array, IDX + 1, LEN);
            LOOP3_ASSERT(LINE, GT, EXP_GT, GT == EXP_GT);
            ASSERT(EXP_GT == findAtMinOracle(array, IDX + 1, LEN, false));

            const int GE = BSU::find0AtMinIndex(array, IDX, LEN);
            LOOP3_ASSERT(LINE, GE, EXP_GE, GE == EXP_GE);
            ASSERT(EXP_GE == findAtMinOracle(array, IDX, LEN, false));

            const int LT = BSU::find0AtMinIndex(array, IDX);
            LOOP3_ASSERT(LINE, LT, EXP_LT, LT == EXP_LT);
            LOOP3_ASSERT(LINE, EXP_LT, findAtMinOracle(array, 0, IDX, false),
                              EXP_LT == findAtMinOracle(array, 0, IDX, false));

            const int LE = BSU::find0AtMinIndex(array, IDX + 1);
            LOOP3_ASSERT(LINE, LE, EXP_LE, LE == EXP_LE);
            ASSERT(EXP_LE == findAtMinOracle(array, 0, IDX + 1, false));
        }

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t bits[SET_UP_ARRAY_DIM], control[SET_UP_ARRAY_DIM];

        for (int ii = 0; ii < 150; ) {
            setUpArray(control, &ii);
            wordCpy(bits, control, sizeof(bits));

            if (veryVerbose) {
                P_(ii);    P(pHex(bits, NUM_BITS));
            }

            for (int length = 0; length <= NUM_BITS; ++length) {
                {
                    const int EXP = findAtMinOracle(bits, 0, length, false);
                    if (-1 == EXP) {
                        ASSERT(! BSU::isAny0(bits, 0, length));
                    }
                    else {
                        ASSERT(EXP < length);
                        ASSERT(! BSU::bit(bits, EXP));
                        ASSERT(! BSU::isAny0(bits, 0, EXP));
                    }

                    ASSERT(EXP == BSU::find0AtMinIndex(bits, length));
                }

                for (int idx = 0; idx <= length; ++idx) {
                    const int EXP = findAtMinOracle(bits, idx, length, false);
                    if (-1 == EXP) {
                        ASSERT(! BSU::isAny0(bits, idx, length - idx));
                    }
                    else {
                        ASSERT(idx <= EXP && EXP < length);
                        ASSERT(! BSU::bit(bits, EXP));
                        ASSERT(! BSU::isAny0(bits, idx, EXP - idx));
                    }

                    ASSERT(EXP == BSU::find0AtMinIndex(bits, idx, length));
                }
            }

            ASSERT(0 == wordCmp(bits, control, sizeof(bits)));
        }

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::find0AtMinIndex(bits, 0));
            ASSERT_PASS(BSU::find0AtMinIndex(bits, 100));
            ASSERT_FAIL(BSU::find0AtMinIndex(bits,  -1));

            ASSERT_PASS(BSU::find0AtMinIndex(bits, 0));
            ASSERT_PASS(BSU::find0AtMinIndex(bits, 0, 100));
            ASSERT_PASS(BSU::find0AtMinIndex(bits, 10, 100));
            ASSERT_FAIL(BSU::find0AtMinIndex(bits, 0,  -1));
            ASSERT_FAIL(BSU::find0AtMinIndex(bits, 10,  -1));
            ASSERT_FAIL(BSU::find0AtMinIndex(bits, -1, 100));
        }
      } break;
      case 21: {
        // --------------------------------------------------------------------
        // TESTING 'find1AtMaxIndex' METHODS
        //   Ensure the methods return the expected value.
        //
        // Concerns:
        //: o Test that both 'find0AtMaxIndex' functions work properly over a
        //:   wide range of valid inputs.
        //: o Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: 1 Use table-driven code to test both 'find1AtMaxIndex' and the
        //:   'findAtMaxOracle' function in this test driver.
        //: 2 Iterate over different test arrays with 'setUpArray'.
        //:   o Iterate over 'length' values from 0 to the size of the array.
        //:     1 Find the max 0 bit, if any, in the array using both
        //:       'find1AtMaxIndex' and 'findAtMaxOracle' and verify their
        //:       results match.
        //:     2 Iterate 'index' from 0 to 'lenght'.
        //:       o Find the max 0 bit, if any, in the array using both
        //:         'find1AtMaxIndex' and 'findAtMaxOracle' for the given
        //:         'index' and 'length' and verify their results match.
        //: 3 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //
        // Testing:
        //   int find1AtMaxIndex(const uint64_t *bitstring, int length);
        //   int find1AtMaxIndex(const uint64_t *bitstring,
        //                       int             begin,
        //                       int             end);
        // --------------------------------------------------------------------

        if (verbose) cout << "TESTING 'find1AtMaxIndex' METHODS\n"
                             "=================================\n";

        const struct {
            int         d_line;
            const char *d_array_p;
            int         d_index;
            int         d_largestIdx;
            int         d_lessThanIdx;
            int         d_lessThanEqualIdx;
            int         d_greaterThanIdx;
            int         d_greaterThanEqualIdx;
        } DATA [] = {
// Line  array              idx    L    LT   LE    GT    GE
// ----  -----              ---   --    --   --    --    --
{   L_,  "1",                 0,   0,   -1,   0,   -1,    0   },
{   L_,  "0",                 0,  -1,   -1,  -1,   -1,   -1   },
{   L_,  "00",                0,  -1,   -1,  -1,   -1,   -1   },
{   L_,  "00",                1,  -1,   -1,  -1,   -1,   -1   },
{   L_,  "01",                0,   0,   -1,   0,   -1,    0   },
{   L_,  "01",                1,   0,    0,   0,   -1,   -1   },
{   L_,  "10",                0,   1,   -1,  -1,    1,    1   },
{   L_,  "10",                1,   1,   -1,   1,   -1,    1   },
{   L_,  "11",                0,   1,   -1,   0,    1,    1   },
{   L_,  "11",                1,   1,    0,   1,   -1,    1   },

{   L_,  "10010000 11110000 11110000 11110000",
                              4,   31,  -1,   4,   31,    31   },
{   L_,  "10010000 11110000 11110000 11100001",
                              4,   31,   0,   0,   31,    31   },
{   L_,  "00000000 00000000 00000000 00000000",
                              0,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "00000000 00000000 00000000 00000000",
                              2,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "00000000 00000000 00000000 00000000",
                             31,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "0 00000000 00000000 00000000 00000000",
                             31,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "00001111 11110000 00001111 11110000",
                              16,  27,  11,  11,   27,    27   },
{   L_,  "00001111 11110000 10001111 11110000",
                              15,  27,  11,  15,   27,    27   },
{   L_,  "01010000 11110000 11110000 00001111",
                              0,   30,  -1,   0,   30,    30   },
{   L_,  "10010000 11110000 11110000 00001111",
                              0,   31,  -1,   0,   31,    31   },
{   L_,  "1 00010000 11110000 11110000 00001111",
                              0,   32,  -1,   0,   32,    32   },
{   L_,  "11 00010000 11110000 11110000 00001111",
                              0,   33,  -1,   0,   33,    33   },
{   L_,  "01010000 11110000 11110000 11110000",
                             30,   30,  28,  30,   -1,    30   },
{   L_,  "10010000 11110000 11110000 11110000",
                             30,   31,  28,  28,   31,    31   },
{   L_,  "1 01010000 11110000 11110000 11110000",
                             30,   32,  28,  30,   32,    32   },
{   L_,  "11 01010000 11110000 11110000 11110000",
                             30,   33,  28,  30,   33,    33   },
{   L_,  "01010000 11110000 11110000 11110000",
                             31,   30,  30,  30,   -1,    -1   },
{   L_,  "10010000 11110000 11110000 11110000",
                             31,   31,  28,  31,   -1,    31   },
{   L_,  "0 10010000 11110000 11110000 11110000",
                             31,   31,  28,  31,   -1,    31   },
{   L_,  "1 10010000 11110000 11110000 11110000",
                             31,   32,  28,  31,   32,    32   },
{   L_,  "1 10010000 11110000 11110000 11110000",
                             32,   32,  31,  32,   -1,    32   },
{   L_,  "1 10010000 11110000 11110000 11110000",
                             32,   32,  31,  32,   -1,    32   },
{   L_,
"01010000 11110000 11110000 11110000 00010000 11110000 11110000 11110001",
                             0,   62,   -1,   0,   62,    62   },
{   L_,
"10010000 11110000 11110000 11110000 00010000 11110000 11110000 11110001",
                             0,   63,   -1,   0,   63,    63   },
{   L_,
"10010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                             0,   63,   -1,  -1,   63,    63   },
{   L_,
"1 00010000 11110000 11110000 11110000 00010000 11110000 11110000 11110001",
                             0,   64,   -1,   0,   64,    64   },
{   L_,
"11010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,   63,   62,  63,   -1,    63   },
{   L_,
"01010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,   62,   62,  62,   -1,    -1   },
{   L_,
"1 11010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,   64,   62,  63,   64,    64   },
{   L_,
"0 11010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,   63,   62,  63,   -1,    63   },
{   L_,
"000000000 00000000 00000000 00000000 00000000 00000000 00000000 000000000",
                             0,   -1,   -1,  -1,   -1,    -1   },
{   L_,
"00 000000000 00000000 00000000 00000000 00000000 00000000 00000000 000000000",
                             0,   -1,   -1,  -1,   -1,    -1   },
{   L_,
"000000000 00000000 00000000 00000000 00000000 00000000 00000000 000000000",
                            63,   -1,   -1,  -1,   -1,    -1   },
{   L_,
"0 000000000 00000000 00000000 00000000 00000000 00000000 00000000 000000000",
                            63,   -1,   -1,  -1,   -1,    -1   },
{ L_,
                                                                    "0000"
"00000000 00000000 00000000 00000000  00000000 00000000 00000000 00000000"
"00000000 00000000 00000000 00000000  00000000 00000000 00000000 00000000"
"00000000 00000000 00000000 00000000  00000000 00000000 00000000 00001000",
                        64*3+3,    3,    3,   3,   -1,    -1,  },
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        for (int i = 0; i < NUM_DATA; ++i) {
            const int   LINE      = DATA[i].d_line;
            const char *STR       = DATA[i].d_array_p;
            const int   LEN       = numBits(STR);
            const int   IDX       = DATA[i].d_index;
            const int   EXP_L     = DATA[i].d_largestIdx;
            const int   EXP_GT    = DATA[i].d_greaterThanIdx;
            const int   EXP_GE    = DATA[i].d_greaterThanEqualIdx;
            const int   EXP_LT    = DATA[i].d_lessThanIdx;
            const int   EXP_LE    = DATA[i].d_lessThanEqualIdx;

            const int MAX_ARRAY_SIZE = 4;

            if (veryVerbose) {
                P_(LINE) P_(STR) P_(IDX) P_(EXP_L) P_(EXP_LT) P_(EXP_LE)
                P_(EXP_GT) P_(EXP_GE)
            }

            uint64_t array[MAX_ARRAY_SIZE];
            bsl::fill(array + 0, array + MAX_ARRAY_SIZE, 0ULL);
            populateBitString(array, 0, STR);

            if (veryVerbose) {
                BSU::print(cout, array, MAX_ARRAY_SIZE * k_BITS_PER_UINT64);
            }

            const int L  = BSU::find1AtMaxIndex(  array, LEN);
            LOOP3_ASSERT(LINE, L, EXP_L, L == EXP_L);
            ASSERT(EXP_L  == findAtMaxOracle(array, 0, LEN, true));

            const int GT = BSU::find1AtMaxIndex(array,   IDX + 1, LEN);
            LOOP3_ASSERT(LINE, GT, EXP_GT, GT == EXP_GT);
            ASSERT(EXP_GT == findAtMaxOracle(array, IDX+1, LEN, true));

            const int GE = BSU::find1AtMaxIndex(array,   IDX,     LEN);
            LOOP3_ASSERT(LINE, GE, EXP_GE, GE == EXP_GE);
            ASSERT(EXP_GE == findAtMaxOracle(array, IDX, LEN, true));

            const int LT = BSU::find1AtMaxIndex(array, IDX);
            LOOP3_ASSERT(LINE, LT, EXP_LT, LT == EXP_LT);
            ASSERT(EXP_LT == findAtMaxOracle(array, 0, IDX, true));

            const int LE = BSU::find1AtMaxIndex(array, IDX + 1);
            LOOP3_ASSERT(LINE, LE, EXP_LE, LE == EXP_LE);
            ASSERT(EXP_LE == findAtMaxOracle(array, 0, IDX + 1, true));
        }

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t bits[SET_UP_ARRAY_DIM] = { 0 }, control[SET_UP_ARRAY_DIM];

        for (int ii = 0; ii < 150; ) {
            setUpArray(control, &ii);
            wordCpy(bits, control, sizeof(bits));

            if (veryVerbose) {
                P_(ii);    P(pHex(bits, NUM_BITS));
            }

            for (int length = 0; length <= NUM_BITS; ++length) {
                {
                    const int EXP = findAtMaxOracle(bits, 0, length, true);
                    if (-1 == EXP) {
                        ASSERT(! BSU::isAny1(bits, 0, length));
                    }
                    else {
                        ASSERT(EXP < length);
                        ASSERT(BSU::bit(bits, EXP));
                        ASSERT(! BSU::isAny1(bits, EXP + 1, length - EXP - 1));
                    }

                    ASSERT(EXP == BSU::find1AtMaxIndex(bits, length));
                }

                for (int idx = 0; idx <= length; ++idx) {
                    const int EXP = findAtMaxOracle(bits, idx, length, true);
                    if (-1 == EXP) {
                        ASSERT(! BSU::isAny1(bits, idx, length - idx));
                    }
                    else {
                        ASSERT(idx <= EXP && EXP < length);
                        ASSERT(BSU::bit(bits, EXP));
                        ASSERT(! BSU::isAny1(bits, EXP + 1, length - EXP - 1));
                    }

                    ASSERT(EXP == BSU::find1AtMaxIndex(bits, idx, length));
                }
            }

            ASSERT(0 == wordCmp(bits, control, sizeof(bits)));
        }

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::find1AtMaxIndex(bits, 0));
            ASSERT_PASS(BSU::find1AtMaxIndex(bits, 100));
            ASSERT_FAIL(BSU::find1AtMaxIndex(bits,  -1));

            ASSERT_PASS(BSU::find1AtMaxIndex(bits, 0));
            ASSERT_PASS(BSU::find1AtMaxIndex(bits, 0, 100));
            ASSERT_PASS(BSU::find1AtMaxIndex(bits, 10, 100));
            ASSERT_FAIL(BSU::find1AtMaxIndex(bits, 0,  -1));
            ASSERT_FAIL(BSU::find1AtMaxIndex(bits, 10,  -1));
            ASSERT_FAIL(BSU::find1AtMaxIndex(bits, -1, 100));
        }
      } break;
      case 20: {
        // --------------------------------------------------------------------
        // TESTING 'find0AtMaxIndex' METHODS
        //   Ensure the methods return the expected value.
        //
        // Concerns:
        //: o Test that both 'find0AtMaxIndex' functions work properly over a
        //:   wide range of valid inputs.
        //: o Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: 1 Use table-driven code to test both 'find0AtMaxIndex' and the
        //:   'findAtMaxOracle' function in this test driver.
        //: 2 Iterate over different test arrays with 'setUpArray'.
        //:   o Iterate over 'length' values from 0 to the size of the array.
        //:     1 Find the max 0 bit, if any, in the array using both
        //:       'find0AtMaxIndex' and 'findAtMaxOracle' and verify their
        //:       results match.
        //:     2 Iterate 'index' from 0 to 'lenght'.
        //:       o Find the max 0 bit, if any, in the array using both
        //:         'find0AtMaxIndex' and 'findAtMaxOracle' for the given
        //:         'index' and 'length' and verify their results match.
        //: 3 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //
        // Testing:
        //   int find0AtMaxIndex(const uint64_t *bitstring, int length);
        //   int find0AtMaxIndex(const uint64_t *bitstring,
        //                       int             begin,
        //                       int             end);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING 'find0AtMaxIndex' METHODS\n"
                             "=================================\n";

        const struct {
            int         d_line;
            const char *d_array_p;
            int         d_index;
            int         d_largestIdx;
            int         d_lessThanIdx;
            int         d_lessThanEqualIdx;
            int         d_greaterThanIdx;
            int         d_greaterThanEqualIdx;
        } DATA [] = {
// Line  array              idx    L    LT   LE    GT    GE
// ----  -----              ---   --    --   --    --    --
{   L_,  "1",                 0,  -1,   -1,  -1,   -1,   -1   },
{   L_,  "0",                 0,   0,   -1,   0,   -1,    0   },
{   L_,  "00",                0,   1,   -1,   0,    1,    1   },
{   L_,  "00",                1,   1,    0,   1,   -1,    1   },
{   L_,  "01",                0,   1,   -1,  -1,    1,    1   },
{   L_,  "01",                1,   1,   -1,   1,   -1,    1   },
{   L_,  "10",                0,   0,   -1,   0,   -1,    0   },
{   L_,  "10",                1,   0,    0,   0,   -1,   -1   },
{   L_,  "11",                0,  -1,   -1,  -1,   -1,   -1   },
{   L_,  "11",                1,  -1,   -1,  -1,   -1,   -1   },
{   L_,  "10010000 11110000 11110000 11110000",
                              2,   30,   1,   2,   30,    30   },
{   L_,  "10010000 11110000 11110000 11110100",
                              2,   30,   1,   1,   30,    30   },
{   L_,  "11111111 11111111 11111111 11111111",
                              0,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "11111111 11111111 11111111 11111111",
                              2,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "11111111 11111111 11111111 11111111",
                             31,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "1 11111111 11111111 11111111 11111111",
                             31,   -1,  -1,  -1,   -1,    -1   },
{   L_,  "11110000 11110000 11110000 11110000",
                              15,  27,  11,  11,   27,    27   },
{   L_,  "11110000 11110000 01110000 11110000",
                              15,  27,  11,  15,   27,    27   },
{   L_,  "10010000 11110000 11110000 11110000",
                              0,   30,  -1,   0,   30,    30   },
{   L_,  "00010000 11110000 11110000 11110000",
                              0,   31,  -1,   0,   31,    31   },
{   L_,  "0 00010000 11110000 11110000 11110000",
                              0,   32,  -1,   0,   32,    32   },
{   L_,  "00 00010000 11110000 11110000 11110000",
                              0,   33,  -1,   0,   33,    33   },
{   L_,  "10010000 11110000 11110000 11110000",
                             30,   30,  29,  30,   -1,    30   },
{   L_,  "00010000 11110000 11110000 11110000",
                             30,   31,  29,  30,   31,    31   },
{   L_,  "0 00010000 11110000 11110000 11110000",
                             30,   32,  29,  30,   32,    32   },
{   L_,  "00 00010000 11110000 11110000 11110000",
                             30,   33,  29,  30,   33,    33   },
{   L_,  "10010000 11110000 11110000 11110000",
                             31,   30,  30,  30,   -1,    -1   },
{   L_,  "00010000 11110000 11110000 11110000",
                             31,   31,  30,  31,   -1,    31   },
{   L_,  "1 00010000 11110000 11110000 11110000",
                             31,   31,  30,  31,   -1,    31   },
{   L_,  "0 00010000 11110000 11110000 11110000",
                             31,   32,  30,  31,   32,    32   },
{   L_,  "1 00010000 11110000 11110000 11110000",
                             32,   31,  31,  31,   -1,    -1   },
{   L_,  "0 00010000 11110000 11110000 11110000",
                             32,   32,  31,  32,   -1,    32   },
{   L_,
"10010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                             0,   62,   -1,   0,   62,    62   },
{   L_,
"00010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                             0,   63,   -1,   0,   63,    63   },
{   L_,
"00010000 11110000 11110000 11110000 00010000 11110000 11110000 11110001",
                             0,   63,   -1,  -1,   63,    63   },
{   L_,
"0 00010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                             0,   64,   -1,   0,   64,    64   },
{   L_,
"00010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,   63,   62,  63,   -1,    63   },
{   L_,
"10010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,   62,   62,  62,   -1,    -1   },
{   L_,
"0 00010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,   64,   62,  63,   64,    64   },
{   L_,
"1 00010000 11110000 11110000 11110000 00010000 11110000 11110000 11110000",
                            63,   63,   62,  63,   -1,    63   },
{   L_,
"111111111 11111111 11111111 11111111 11111111 11111111 11111111 111111111",
                             0,   -1,   -1,  -1,   -1,    -1   },
{   L_,
"11 111111111 11111111 11111111 11111111 11111111 11111111 11111111 111111111",
                             0,   -1,   -1,  -1,   -1,    -1   },
{   L_,
"111111111 11111111 11111111 11111111 11111111 11111111 11111111 111111111",
                            63,   -1,   -1,  -1,   -1,    -1   },
{   L_,
"1 111111111 11111111 11111111 11111111 11111111 11111111 11111111 111111111",
                            63,   -1,   -1,  -1,   -1,    -1   },
{ L_,
                                                                    "1111"
"11111111 11111111 11111111 11111111  11111111 11111111 11111111 11111111"
"11111111 11111111 11111111 11111111  11111111 11111111 11111111 11111111"
"11111111 11111111 11111111 11111111  11111111 11111111 11111111 11101111",
                      3*64 + 3,    4,    4,   4,   -1,    -1   },
        };

        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        for (int i = 0; i < NUM_DATA; ++i) {
            const int   LINE   = DATA[i].d_line;
            const char *STR    = DATA[i].d_array_p;
            const int   LEN    = numBits(STR);
            const int   IDX    = DATA[i].d_index;
            const int   EXP_L  = DATA[i].d_largestIdx;
            const int   EXP_GT = DATA[i].d_greaterThanIdx;
            const int   EXP_GE = DATA[i].d_greaterThanEqualIdx;
            const int   EXP_LT = DATA[i].d_lessThanIdx;
            const int   EXP_LE = DATA[i].d_lessThanEqualIdx;

            const int MAX_ARRAY_SIZE = 4;

            if (veryVerbose) {
                P_(LINE) P_(STR) P_(IDX) P_(EXP_L) P_(EXP_LT) P_(EXP_LE)
                P_(EXP_GT) P_(EXP_GE)
            }

            uint64_t array[MAX_ARRAY_SIZE];
            bsl::fill(array + 0, array + MAX_ARRAY_SIZE, ~0ULL);
            populateBitString(array, 0, STR);

            if (veryVerbose) {
                BSU::print(cout, array, MAX_ARRAY_SIZE * k_BITS_PER_UINT64);
            }

            const int L  = BSU::find0AtMaxIndex(  array, LEN);
            LOOP3_ASSERT(LINE, L, EXP_L, L == EXP_L);
            ASSERT(EXP_L  == findAtMaxOracle(array, 0, LEN, false));

            const int GT = BSU::find0AtMaxIndex(array,   IDX + 1, LEN);
            LOOP3_ASSERT(LINE, GT, EXP_GT, GT == EXP_GT);
            ASSERT(EXP_GT == findAtMaxOracle(array, IDX+1, LEN, false));

            const int GE = BSU::find0AtMaxIndex(array,   IDX,     LEN);
            LOOP3_ASSERT(LINE, GE, EXP_GE, GE == EXP_GE);
            ASSERT(EXP_GE == findAtMaxOracle(array, IDX, LEN, false));

            const int LT = BSU::find0AtMaxIndex(array, IDX);
            LOOP3_ASSERT(LINE, LT, EXP_LT, LT == EXP_LT);
            ASSERT(EXP_LT == findAtMaxOracle(array, 0, IDX, false));

            const int LE = BSU::find0AtMaxIndex(array, IDX + 1);
            LOOP3_ASSERT(LINE, LE, EXP_LE, LE == EXP_LE);
            ASSERT(EXP_LE == findAtMaxOracle(array, 0, IDX + 1, false));
        }

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t bits[SET_UP_ARRAY_DIM] = { 0 }, control[SET_UP_ARRAY_DIM];

        for (int ii = 0; ii < 150; ) {
            setUpArray(control, &ii);
            wordCpy(bits, control, sizeof(bits));

            if (veryVerbose) {
                P_(ii);    P(pHex(bits, NUM_BITS));
            }

            for (int length = 0; length <= NUM_BITS; ++length) {
                {
                    const int EXP = findAtMaxOracle(bits, 0, length, false);
                    if (-1 == EXP) {
                        ASSERT(! BSU::isAny0(bits, 0, length));
                    }
                    else {
                        ASSERT(EXP < length);
                        ASSERT(! BSU::bit(bits, EXP));
                        ASSERT(! BSU::isAny0(bits, EXP + 1, length - EXP - 1));
                    }

                    ASSERT(EXP == BSU::find0AtMaxIndex(bits, length));
                }

                for (int idx = 0; idx <= length; ++idx) {
                    const int EXP = findAtMaxOracle(bits, idx, length, false);
                    if (-1 == EXP) {
                        ASSERT(! BSU::isAny0(bits, idx, length - idx));
                    }
                    else {
                        ASSERT(idx <= EXP && EXP < length);
                        ASSERT(! BSU::bit(bits, EXP));
                        ASSERT(! BSU::isAny0(bits, EXP + 1, length - EXP - 1));
                    }

                    ASSERT(EXP == BSU::find0AtMaxIndex(bits, idx, length));
                }

            }

            ASSERT(0 == wordCmp(bits, control, sizeof(bits)));
        }

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::find0AtMaxIndex(bits, 0));
            ASSERT_PASS(BSU::find0AtMaxIndex(bits, 100));
            ASSERT_FAIL(BSU::find0AtMaxIndex(bits,  -1));

            ASSERT_PASS(BSU::find0AtMaxIndex(bits, 0));
            ASSERT_PASS(BSU::find0AtMaxIndex(bits, 0, 100));
            ASSERT_PASS(BSU::find0AtMaxIndex(bits, 10, 100));
            ASSERT_FAIL(BSU::find0AtMaxIndex(bits, 0,  -1));
            ASSERT_FAIL(BSU::find0AtMaxIndex(bits, 10,  -1));
            ASSERT_FAIL(BSU::find0AtMaxIndex(bits, -1, 100));
        }
      } break;
      case 19: {
        // --------------------------------------------------------------------
        // TESTING 'xorEqual'
        //   Ensure the method has the right effect on 'dstBitString'.
        //
        // Concerns:
        //    Test 'xorEqual'.
        //
        // Plan:
        //: o Do table-driven testing of both 'xorEqual' and 'xorOracle'.
        //: o Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: o Do exhaustive testing with 'dst' and 'src' arrays set up by
        //:   'setUpArray'.
        //:   1 Verify that 'xor'ing 'dst' with a toggle of itself results in
        //:     all 1's.
        //:   2 Verify that 'xor'ing 'dst' with itself results in all 0's.
        //:   3 Loop through a variety of values of 'dstIndex', 'srcIndex', and
        //:     'numBits'.
        //:     o Perform numerous 'xorEqual' operations on the 'src' and 'dst'
        //:       arrays with ALL_TRUE and 'ALL_FALSE' arrays, then test the
        //:       results, which should be predictable.
        //:     o Perform 'xorOracle' on the 'src' and 'dst' arrays, storing
        //:       the result in 'result'.
        //:     o Perform 'xorEqual' on 'src' and 'dst', and compare 'result'
        //:       with 'dst'.
        //
        // Testing:
        //   void xorEqual(uint64_t       *dstBitstring,
        //                 int             dstIndex,
        //                 const uint64_t *srcBitstring,
        //                 int             srcIndex,
        //                 int             numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTESTING 'xorEqual'"
                          << "\n==================" << bsl::endl;

        const struct {
            int         d_line;
            const char *d_dstBitstring_p;
            int         d_dstIndex;
            const char *d_srcBitstring_p;
            int         d_srcIndex;
            int         d_numBits;
            const char *d_result_p;
        } DATA [] = {
//<<------<<
// Line dst  dIdx  src  sIdx  NB   Result
// ---- ---  ----  ---- ----  --   ------
{  L_,  "",     0, "",     0,  0,     ""    },
{  L_,  "0",    0, "0",    0,  1,     "0"   },
{  L_,  "0",    0, "1",    0,  1,     "1"   },
{  L_,  "1",    0, "0",    0,  1,     "1"   },
{  L_,  "1",    0, "1",    0,  1,     "0"   },
{  L_,  "00",   4, "0",    0,  1,     "00"  },
{  L_,  "10",   4, "0",    0,  1,     "10"  },
{  L_,  "00",   4, "1",    0,  1,     "10"  },
{  L_,  "11",   4, "1",    0,  1,     "01"  },
{  L_,  "10",   4, "01",   4,  1,     "10"  },
{  L_,  "00",   4, "11",   4,  1,     "10"  },
{  L_,  "00",   4, "01",   4,  1,     "00"  },
{  L_,  "00",   4, "11",   4,  1,     "10"  },
{  L_,  "11",   4, "11",   4,  1,     "01"  },
{  L_,  "11",   0, "01",   0,  5,     "10"  },
{  L_,  "11",   0, "00",   0,  5,     "11"  },
{  L_,  "11",   0, "10",   0,  5,     "01"  },
{  L_,  "10",   0, "00",   0,  5,     "10"  },
{  L_,  "01",   0, "11",   0,  5,     "10"  },
{  L_,  "10",   0, "10",   0,  5,     "00"  },
{  L_,  "10",   0, "11",   0,  5,     "01"  },
{  L_,  "11",   0, "11",   0,  5,     "00"  },

{ L_, "1234",             0, "1234",            0, 16, "0000" },
{ L_, "1234 ww0",       128, "5670 wh0",       96, 16, "4444 ww0", },
{ L_, "1234 whqy0",     120, "5670 wwh0",     160, 16, "4444 whqy0" },

{ L_, "12341234 yf",      8, "12341234 f",      4, 32, "00000000 yf" },
{ L_, "12341234 ww0",   128, "56705670 wh0",   96, 32, "44444444 ww0", },
{ L_, "12341234 whqy0", 120, "56705670 wwh0", 160, 32, "44444444 whqy0" },
//>>------>>
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        for (int i = 0; i < NUM_DATA; ++i) {
            const int   LINE      = DATA[i].d_line;
            const char *DSTR      = DATA[i].d_dstBitstring_p;
            const int   DI        = DATA[i].d_dstIndex;
            const char *SSTR      = DATA[i].d_srcBitstring_p;
            const int   SI        = DATA[i].d_srcIndex;
            const int   NB        = DATA[i].d_numBits;
            const char *RESULT    = DATA[i].d_result_p;

            if (veryVerbose) {
                P_(LINE) P_(DSTR) P_(DI) P_(SSTR) P_(SI) P_(NB) P(RESULT)
            }

            const int MAX_ARRAY_SIZE = 4;

            uint64_t dstControl[MAX_ARRAY_SIZE] = { 0 };
            uint64_t srcControl[MAX_ARRAY_SIZE] = { 0 };
            uint64_t result[    MAX_ARRAY_SIZE] = { 0 };
            uint64_t dst[MAX_ARRAY_SIZE], src[MAX_ARRAY_SIZE];
            populateBitStringHex(dstControl, 0, DSTR);
            populateBitStringHex(srcControl, 0, SSTR);
            populateBitStringHex(result,     0, RESULT);

            if (veryVerbose) {
                BSU::print(cout, dst, MAX_ARRAY_SIZE * k_BITS_PER_UINT64);
                BSU::print(cout, src, MAX_ARRAY_SIZE * k_BITS_PER_UINT64);
                BSU::print(cout, result, MAX_ARRAY_SIZE * k_BITS_PER_UINT64);
            }

            wordCpy(dst, dstControl, sizeof(dst));
            wordCpy(src, srcControl, sizeof(src));

            BSU::xorEqual(dst, DI, src, SI, NB);
            LOOP2_ASSERT(LINE, pHex(dst, DI + NB),
                                       BSU::areEqual(dst, DI, result, DI, NB));
            ASSERT(! wordCmp(src, srcControl, sizeof(src)));

            wordCpy(dst, dstControl, sizeof(dst));

            xorOracle(dst, DI, src, SI, NB);
            LOOP_ASSERT(LINE, BSU::areEqual(dst, DI, result, DI, NB));
            ASSERT(! wordCmp(src, srcControl, sizeof(src)));
        }

        const int NUM_BITS = 3 * k_BITS_PER_UINT64;

        uint64_t ALL_TRUE[ SET_UP_ARRAY_DIM];
        uint64_t ALL_FALSE[SET_UP_ARRAY_DIM];

        uint64_t *endTrue  = ALL_TRUE  + SET_UP_ARRAY_DIM;
        uint64_t *endFalse = ALL_FALSE + SET_UP_ARRAY_DIM;

        bsl::fill(ALL_TRUE  + 0, endTrue,  ~0ULL);
        bsl::fill(ALL_FALSE + 0, endFalse,  0ULL);

        uint64_t controlDst[SET_UP_ARRAY_DIM], dst[SET_UP_ARRAY_DIM] = { 0 };
        uint64_t toggleDst[ SET_UP_ARRAY_DIM];
        uint64_t controlSrc[SET_UP_ARRAY_DIM], src[SET_UP_ARRAY_DIM] = { 0 };
        uint64_t toggleSrc[ SET_UP_ARRAY_DIM];
        uint64_t result[    SET_UP_ARRAY_DIM];

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::xorEqual(dst,  0, src,  0,  0));
            ASSERT_PASS(BSU::xorEqual(dst,  0, src,  0, 70));
            ASSERT_PASS(BSU::xorEqual(dst, 70, src, 70, 70));
            ASSERT_FAIL(BSU::xorEqual(dst, -1, src, 70, 70));
            ASSERT_FAIL(BSU::xorEqual(dst, 70, src, -1, 70));
            ASSERT_FAIL(BSU::xorEqual(dst, 70, src, 70, -1));
        }

        for (int ii = 0, jj = 36; ii < 72; ) {
            setUpArray(controlDst, &ii, true);
            setUpArray(controlSrc, &jj, true);
            jj %= 72;

            if (veryVerbose) {
                P_(ii);    P(pHex(controlDst, NUM_BITS));
                P_(jj);    P(pHex(controlSrc, NUM_BITS));
            }

            wordCpy(toggleDst, controlDst, sizeof(dst));
            BSU::toggle(toggleDst, 0, SET_UP_ARRAY_DIM * k_BITS_PER_UINT64);
            wordCpy(toggleSrc, controlSrc, sizeof(src));
            BSU::toggle(toggleSrc, 0, SET_UP_ARRAY_DIM * k_BITS_PER_UINT64);

            wordCpy(dst, controlDst, sizeof(dst));
            BSU::xorEqual(dst, 0, toggleDst, 0, NUM_BITS);
            ASSERT(! BSU::isAny0(dst, 0, NUM_BITS));

            wordCpy(dst, controlDst, sizeof(dst));
            BSU::xorEqual(dst, 0, dst, 0, NUM_BITS);
            ASSERT(! BSU::isAny1(dst, 0, NUM_BITS));

            wordCpy(src, controlSrc, sizeof(src));

            const int maxIdx = NUM_BITS - 1;
            for (int dstIdx = 0; dstIdx <= maxIdx; incInt(&dstIdx, maxIdx)) {
                for (int srcIdx = 0; srcIdx <= maxIdx;
                                                     incInt(&srcIdx, maxIdx)) {
                    const int maxNumBits = NUM_BITS - bsl::max(srcIdx, dstIdx);
                    for (int numBits = 0; numBits <= maxNumBits;
                                                incInt(&numBits, maxNumBits)) {
                        const int dstTop = dstIdx + numBits;

                        // dst | ALL_FALSE

                        wordCpy(dst, controlDst, sizeof(dst));
                        BSU::xorEqual(dst, dstIdx, ALL_FALSE, srcIdx, numBits);
                        ASSERT(0 == wordCmp(dst, controlDst, sizeof(dst)));

                        // dst | ALL_TRUE

                        BSU::xorEqual(dst, dstIdx, ALL_TRUE, srcIdx, numBits);
                        ASSERT(BSU::areEqual(dst, 0, controlDst, 0, dstIdx));
                        ASSERT(BSU::areEqual(dst, dstIdx, toggleDst, dstIdx,
                                                                     numBits));
                        ASSERT(BSU::areEqual(dst, dstTop, controlDst, dstTop,
                                                           NUM_BITS - dstTop));

                        // (dst = ALL_FALSE) | src

                        bsl::fill(dst + 0, dst + SET_UP_ARRAY_DIM, 0ULL);
                        BSU::xorEqual(dst, dstIdx, src, srcIdx, numBits);
                        ASSERT(! BSU::isAny1(dst, 0, dstIdx));
                        ASSERT(BSU::areEqual(dst, dstIdx, src, srcIdx,
                                                                     numBits));
                        ASSERT(! BSU::isAny1(dst, dstTop, NUM_BITS - dstTop));

                        // (dst = ALL_TRUE) | src

                        bsl::fill(dst + 0,    dst    + SET_UP_ARRAY_DIM,~0ULL);
                        BSU::xorEqual(dst, dstIdx, src, srcIdx, numBits);
                        ASSERT(! BSU::isAny0(dst, 0, dstIdx));
                        ASSERT(BSU::areEqual(dst, dstIdx, toggleSrc, srcIdx,
                                                                     numBits));
                        ASSERT(! BSU::isAny0(dst, dstTop, NUM_BITS - dstTop));

                        // result xorOracle src

                        wordCpy(result, controlDst, sizeof(dst));
                        xorOracle(result, dstIdx, src, srcIdx, numBits);

                        // dst | src

                        wordCpy(dst, controlDst, sizeof(dst));
                        BSU::xorEqual(dst, dstIdx, src, srcIdx, numBits);

                        // confirm result == dst

                        ASSERT(0 == wordCmp(result, dst, sizeof(dst)));

                        // confirm src never changed

                        ASSERT(0 == wordCmp(src, controlSrc, sizeof(src)));
                    }
                }
            }
        }
      } break;
      case 18: {
        // --------------------------------------------------------------------
        // TESTING 'orEqual'
        //   Ensure the method has the right effect on 'dstBitString'.
        //
        // Concerns:
        //    Test 'orEqual'.
        //
        // Plan:
        //: o Do table-driven testing of both 'orEqual' and 'orOracle'.
        //: o Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: o Do exhaustive testing with 'dst' and 'src' arrays set up by
        //:   'setUpArray'.  Loop through a variety of values of 'dstIndex',
        //:   'srcIndex', and 'numBits'.
        //:   1 Perform numerous 'orEqual' operations on the 'src' and 'dst'
        //:     arrays with ALL_TRUE and 'ALL_FALSE' arrays, then test the
        //:     results, which should be predictable.
        //:   2 Perform 'orOracle' on the 'src' and 'dst' arrays, storing the
        //:     result in 'result'.
        //:   3 Perform 'andEqual' on 'src' and 'dst', and compare 'result'
        //:     with 'dst'.
        //
        // Testing:
        //   void orEqual(uint64_t       *dstBitstring,
        //                int             dstIndex,
        //                const uint64_t *srcBitstring,
        //                int             srcIndex,
        //                int             numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING 'orEqual'\n"
                             "=================\n";

        const struct {
            int         d_line;
            const char *d_dstBitstring_p;
            int         d_dstIndex;
            const char *d_srcBitstring_p;
            int         d_srcIndex;
            int         d_numBits;
            const char *d_result_p;
        } DATA [] = {
            // Line dst  dIdx  src  sIdx  NB   Result
            // ---- ---  ----  ---- ----  --   ------
            {  L_,  "",     0, "",     0,  0,     ""    },
            {  L_,  "0",    0, "0",    0,  1,     "0"   },
            {  L_,  "0",    0, "1",    0,  1,     "1"   },
            {  L_,  "1",    0, "0",    0,  1,     "1"   },
            {  L_,  "1",    0, "1",    0,  1,     "1"   },
            {  L_,  "01",   4, "0",    0,  1,     "01"  },
            {  L_,  "11",   4, "0",    0,  1,     "11"  },
            {  L_,  "00",   4, "1",    0,  1,     "10"  },
            {  L_,  "10",   4, "1",    0,  1,     "10"  },
            {  L_,  "10",   4, "01",   4,  1,     "10"  },
            {  L_,  "01",   4, "11",   4,  1,     "11"  },
            {  L_,  "10",   4, "01",   4,  1,     "10"  },
            {  L_,  "00",   4, "11",   4,  1,     "10"  },
            {  L_,  "11",   4, "11",   4,  1,     "11"  },
            {  L_,  "11",   0, "01",   0,  8,     "11"  },
            {  L_,  "11",   0, "00",   0,  8,     "11"  },
            {  L_,  "11",   0, "10",   0,  8,     "11"  },
            {  L_,  "10",   0, "00",   0,  8,     "10"  },
            {  L_,  "01",   0, "11",   0,  8,     "11"  },
            {  L_,  "10",   0, "10",   0,  8,     "10"  },
            {  L_,  "10",   0, "11",   0,  8,     "11"  },
            {  L_,  "11",   0, "11",   0,  8,     "11"  },
//<<-------<<
{ L_, "53cc5d03 1234 5e42f4 hw0", 120, "8888", 0, 16,
                                                  "53cc5d03 9abc 5e42f4 hw0" },
{ L_, "53cc5d03 1234 5e42f4 hw0", 120, "4444", 0, 16,
                                                  "53cc5d03 5674 5e42f4 hw0" },

{ L_, "53cc5d03 8888 5e42f4 hw0", 120, "1234", 0, 16,
                                                  "53cc5d03 9abc 5e42f4 hw0" },
{ L_, "53cc5d03 4444 5e42f4 hw0", 120, "1234", 0, 16,
                                                  "53cc5d03 5674 5e42f4 hw0" },

{ L_, "53cc5d03 1234 5e42f4 hw0", 120, "ea5c6 8888 hqyb", 56, 16,
                                                  "53cc5d03 9abc 5e42f4 hw0" },
{ L_, "53cc5d03 1234 5e42f4 hw0", 120, "ea5c6 4444 hqyb", 56, 16,
                                                  "53cc5d03 5674 5e42f4 hw0" },

{ L_, "53cc5d03 8888 5e42f4 hw0", 120, "ea5c6 1234 hqyb", 56, 16,
                                                  "53cc5d03 9abc 5e42f4 hw0" },
{ L_, "53cc5d03 4444 5e42f4 hw0", 120, "ea5c6 1234 hqyb", 56, 16,
                                                  "53cc5d03 5674 5e42f4 hw0" },

//>>-------<<
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        const int MAX_ARRAY_SIZE = 4;
        for (int i = 0; i < NUM_DATA; ++i) {
            const int   LINE      = DATA[i].d_line;
            const char *DSTR      = DATA[i].d_dstBitstring_p;
            const int   DI        = DATA[i].d_dstIndex;
            const char *SSTR      = DATA[i].d_srcBitstring_p;
            const int   SI        = DATA[i].d_srcIndex;
            const int   NB        = DATA[i].d_numBits;
            const char *RESULT    = DATA[i].d_result_p;

            if (veryVerbose) {
                P_(LINE) P_(DSTR) P_(DI) P_(SSTR) P_(SI) P_(NB) P(RESULT)
            }

            uint64_t controlDst[MAX_ARRAY_SIZE] = { 0 };
            uint64_t dst[       MAX_ARRAY_SIZE] = { 0 };
            uint64_t src[       MAX_ARRAY_SIZE] = { 0 };
            uint64_t result[    MAX_ARRAY_SIZE] = { 0 };
            populateBitStringHex(controlDst, 0, DSTR);
            populateBitStringHex(src,        0, SSTR);
            populateBitStringHex(result,     0, RESULT);

            if (veryVerbose) {
                BSU::print(cout, dst, MAX_ARRAY_SIZE * k_BITS_PER_UINT64);
                BSU::print(cout, src, MAX_ARRAY_SIZE * k_BITS_PER_UINT64);
                BSU::print(cout, result, MAX_ARRAY_SIZE * k_BITS_PER_UINT64);
            }

            wordCpy(dst, controlDst, sizeof(dst));

            BSU::orEqual(dst, DI, src, SI, NB);
            LOOP_ASSERT(LINE, BSU::areEqual(dst, DI, result, DI, NB));

            wordCpy(dst, controlDst, sizeof(dst));

            orOracle(dst, DI, src, SI, NB);
            LOOP_ASSERT(LINE, BSU::areEqual(dst, DI, result, DI, NB));
        }

        const int NUM_BITS = 3 * k_BITS_PER_UINT64;

        uint64_t ALL_TRUE[ SET_UP_ARRAY_DIM];
        uint64_t ALL_FALSE[SET_UP_ARRAY_DIM];

        uint64_t *endTrue  = ALL_TRUE  + SET_UP_ARRAY_DIM;
        uint64_t *endFalse = ALL_FALSE + SET_UP_ARRAY_DIM;

        bsl::fill(ALL_TRUE  + 0, endTrue,  ~0ULL);
        bsl::fill(ALL_FALSE + 0, endFalse,  0ULL);

        uint64_t controlDst[SET_UP_ARRAY_DIM], dst[SET_UP_ARRAY_DIM] = { 0 };
        uint64_t controlSrc[SET_UP_ARRAY_DIM], src[SET_UP_ARRAY_DIM] = { 0 };
        uint64_t result[    SET_UP_ARRAY_DIM];

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::orEqual(dst,  0, src,  0,  0));
            ASSERT_PASS(BSU::orEqual(dst,  0, src,  0, 70));
            ASSERT_PASS(BSU::orEqual(dst, 70, src, 70, 70));
            ASSERT_FAIL(BSU::orEqual(dst, -1, src, 70, 70));
            ASSERT_FAIL(BSU::orEqual(dst, 70, src, -1, 70));
            ASSERT_FAIL(BSU::orEqual(dst, 70, src, 70, -1));
        }

        for (int ii = 0, jj = 36; ii < 80; ) {
            setUpArray(controlDst, &ii, true);
            setUpArray(controlSrc, &jj, true);
            jj %= 80;

            if (veryVerbose) {
                P_(ii);    P(pHex(controlDst, NUM_BITS));
                P_(jj);    P(pHex(controlSrc, NUM_BITS));
            }

            const int maxIdx = NUM_BITS - 1;
            for (int dstIdx = 0; dstIdx <= maxIdx; incInt(&dstIdx, maxIdx)) {
                for (int srcIdx = 0; srcIdx <= maxIdx;
                                                     incInt(&srcIdx, maxIdx)) {
                    const int maxNumBits = NUM_BITS - bsl::max(srcIdx, dstIdx);
                    for (int numBits = 0; numBits <= maxNumBits;
                                                incInt(&numBits, maxNumBits)) {
                        const int dstTop = dstIdx + numBits;

                        // dst | ALL_FALSE

                        wordCpy(dst, controlDst, sizeof(dst));
                        BSU::orEqual(dst, dstIdx, ALL_FALSE, srcIdx, numBits);
                        ASSERT(0 == wordCmp(dst, controlDst, sizeof(dst)));

                        // dst | ALL_TRUE

                        BSU::orEqual(dst, dstIdx, ALL_TRUE, srcIdx, numBits);
                        ASSERT(BSU::areEqual(dst, 0, controlDst, 0, dstIdx));
                        ASSERT(! BSU::isAny0(dst, dstIdx, numBits));
                        ASSERT(BSU::areEqual(dst, dstTop, controlDst, dstTop,
                                                           NUM_BITS - dstTop));

                        // (dst = ALL_FALSE) | src

                        bsl::fill(dst + 0, dst + SET_UP_ARRAY_DIM, 0ULL);
                        wordCpy(src, controlSrc, sizeof(src));
                        BSU::orEqual(dst, dstIdx, src, srcIdx, numBits);
                        ASSERT(! BSU::isAny1(dst, 0, dstIdx));
                        ASSERT(BSU::areEqual(dst, dstIdx, src, srcIdx,
                                                                     numBits));
                        ASSERT(! BSU::isAny1(dst, dstTop, NUM_BITS - dstTop));

                        // (dst = ALL_TRUE) | src

                        bsl::fill(dst + 0,    dst    + SET_UP_ARRAY_DIM,~0ULL);
                        BSU::orEqual(dst, dstIdx, src, srcIdx, numBits);
                        ASSERT(! BSU::isAny0(dst, 0, NUM_BITS));

                        // result orOracle src

                        wordCpy(result, controlDst, sizeof(dst));
                        orOracle(result, dstIdx, src, srcIdx, numBits);

                        // dst | src

                        wordCpy(dst, controlDst, sizeof(dst));
                        BSU::orEqual(dst, dstIdx, src, srcIdx, numBits);

                        // confirm result == dst

                        ASSERT(0 == wordCmp(result, dst, sizeof(dst)));

                        // confirm src never changed

                        ASSERT(0 == wordCmp(src, controlSrc, sizeof(src)));
                    }
                }
            }
        }
      } break;
      case 17: {
        // --------------------------------------------------------------------
        // TESTING 'minusEqual'
        //   Ensure the method has the right effect on 'dstBitString'.
        //
        // Concerns:
        //    Test 'minusEqual'.
        //
        // Plan:
        //: o Do table-driven testing of both 'minusEqual' and 'minusOracle'.
        //: o Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: o Do exhaustive testing with 'dst' and 'src' arrays set up by
        //:   'setUpArray'.  Loop through a variety of values of 'dstIndex',
        //:   'srcIndex', and 'numBits'.
        //:   1 Perform numerous 'minusEqual' operations on the 'src' and 'dst'
        //:     arrays with ALL_TRUE and 'ALL_FALSE' arrays, then test the
        //:     results, which should be predictable.
        //:   2 Perform 'minusOracle' on the 'src' and 'dst' arrays, storing
        //:     the result in 'result'.
        //:   3 Perform 'minusEqual' on 'src' and 'dst', and compare 'result'
        //:     with 'dst'.
        //
        // Testing:
        //   void minusEqual(uint64_t       *dstBitstring,
        //                   int             dstIndex,
        //                   const uint64_t *srcBitstring,
        //                   int             srcIndex,
        //                   int             numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING 'minusEqual'\n"
                             "====================\n";

        const struct {
            int         d_line;
            const char *d_dstBitstring_p;
            int         d_dstIndex;
            const char *d_srcBitstring_p;
            int         d_srcIndex;
            int         d_numBits;
            const char *d_result_p;
        } DATA [] = {
// Line dst  dIdx  src  sIdx  NB   Result
// ---- ---  ----  ---- ----  --   ------
{ L_,  "",     0, "",     0,  0,     ""    },
{ L_,  "0",    0, "0",    0,  1,     "0"   },
{ L_,  "0",    0, "1",    0,  1,     "0"   },
{ L_,  "1",    0, "0",    0,  1,     "1"   },
{ L_,  "1",    0, "1",    0,  1,     "0"   },
{ L_,  "00",   4, "0",    0,  1,     "00"  },
{ L_,  "10",   4, "0",    0,  1,     "10"  },
{ L_,  "00",   4, "1",    0,  1,     "00"  },
{ L_,  "10",   4, "1",    0,  1,     "00"  },
{ L_,  "11",   4, "01",   4,  4,     "11"  },
{ L_,  "01",   4, "11",   4,  4,     "01"  },
{ L_,  "11",   4, "01",   4,  4,     "11"  },
{ L_,  "01",   4, "11",   4,  4,     "01"  },
{ L_,  "11",   4, "11",   4,  4,     "01"  },
{ L_,  "11",   0, "01",   0,  8,     "10"  },
{ L_,  "11",   0, "00",   0,  8,     "11"  },
{ L_,  "11",   0, "10",   0,  8,     "01"  },
{ L_,  "10",   0, "00",   0,  8,     "10"  },
{ L_,  "01",   0, "11",   0,  8,     "00"  },
{ L_,  "10",   0, "10",   0,  8,     "00"  },
{ L_,  "10",   0, "11",   0,  8,     "00"  },
{ L_,  "11",   0, "11",   0,  8,     "00"  },


{ L_, "77901f ffff 611a y0", 24, "0000", 0, 16, "77901f ffff 611a y0" },
{ L_, "77901f ffff 611a y0", 24, "aaaa", 0, 16, "77901f 5555 611a y0" },
{ L_, "77901f ffff 611a y0", 24, "7777", 0, 16, "77901f 8888 611a y0" },
{ L_, "77901f ffff 611a y0", 24, "137f", 0, 16, "77901f ec80 611a y0" },
{ L_, "77901f ffff 611a y0", 24, "f731", 0, 16, "77901f 08ce 611a y0" },

{ L_, "77901f 0000 611a y0", 24, "48c0", 0, 16, "77901f 0000 611a y0" },
{ L_, "77901f 0000 611a y0", 24, "26ae", 0, 16, "77901f 0000 611a y0" },
{ L_, "77901f 0000 611a y0", 24, "159d", 0, 16, "77901f 0000 611a y0" },

{ L_, "77901f 1234 611a y0", 24, "0000", 0, 16, "77901f 1234 611a y0" },
{ L_, "77901f 1234 611a y0", 24, "1111", 0, 16, "77901f 0224 611a y0" },
{ L_, "77901f 1234 611a y0", 24, "2222", 0, 16, "77901f 1014 611a y0" },

{ L_, "77901f 8ace 611a y0", 24, "0000", 0, 16, "77901f 8ace 611a y0" },
{ L_, "77901f 8ace 611a y0", 24, "1111", 0, 16, "77901f 8ace 611a y0" },
{ L_, "77901f 8ace 611a y0", 24, "5555", 0, 16, "77901f 8a8a 611a y0" },
{ L_, "77901f 8ace 611a y0", 24, "7777", 0, 16, "77901f 8888 611a y0" },


{ L_, "77901f ffff 611a y0", 24, "a 0000 611a", 16, 16, "77901f ffff 611ay0" },
{ L_, "77901f ffff 611a y0", 24, "a aaaa 611a", 16, 16, "77901f 5555 611ay0" },
{ L_, "77901f ffff 611a y0", 24, "a 7777 611a", 16, 16, "77901f 8888 611ay0" },
{ L_, "77901f ffff 611a y0", 24, "a 137f 611a", 16, 16, "77901f ec80 611ay0" },
{ L_, "77901f ffff 611a y0", 24, "a f731 611a", 16, 16, "77901f 08ce 611ay0" },

{ L_, "77901f 0000 611a y0", 24, "a 48c0 611a", 16, 16, "77901f 0000 611ay0" },
{ L_, "77901f 0000 611a y0", 24, "a 26ae 611a", 16, 16, "77901f 0000 611ay0" },
{ L_, "77901f 0000 611a y0", 24, "a 159d 611a", 16, 16, "77901f 0000 611ay0" },

{ L_, "77901f 1234 611a y0", 24, "a 0000 611a", 16, 16, "77901f 1234 611ay0" },
{ L_, "77901f 1234 611a y0", 24, "a 1111 611a", 16, 16, "77901f 0224 611ay0" },
{ L_, "77901f 1234 611a y0", 24, "a 2222 611a", 16, 16, "77901f 1014 611ay0" },

{ L_, "77901f 8ace 611a y0", 24, "a 0000 611a", 16, 16, "77901f 8ace 611ay0" },
{ L_, "77901f 8ace 611a y0", 24, "a 1111 611a", 16, 16, "77901f 8ace 611ay0" },
{ L_, "77901f 8ace 611a y0", 24, "a 5555 611a", 16, 16, "77901f 8a8a 611ay0" },
{ L_, "77901f 8ace 611a y0", 24, "a 7777 611a", 16, 16, "77901f 8888 611ay0" },


{ L_, "01f ffff 611a y0", 24, "a 0000 611a wa", 80, 16, "01f ffff 611ay0" },
{ L_, "01f ffff 611a y0", 24, "a aaaa 611a wa", 80, 16, "01f 5555 611ay0" },
{ L_, "01f ffff 611a y0", 24, "a 7777 611a wa", 80, 16, "01f 8888 611ay0" },
{ L_, "01f ffff 611a y0", 24, "a 137f 611a wa", 80, 16, "01f ec80 611ay0" },
{ L_, "01f ffff 611a y0", 24, "a f731 611a wa", 80, 16, "01f 08ce 611ay0" },

{ L_, "01f 0000 611a y0", 24, "a 48c0 611a wa", 80, 16, "01f 0000 611ay0" },
{ L_, "01f 0000 611a y0", 24, "a 26ae 611a wa", 80, 16, "01f 0000 611ay0" },
{ L_, "01f 0000 611a y0", 24, "a 159d 611a wa", 80, 16, "01f 0000 611ay0" },

{ L_, "01f 1234 611a y0", 24, "a 0000 611a wa", 80, 16, "01f 1234 611ay0" },
{ L_, "01f 1234 611a y0", 24, "a 1111 611a wa", 80, 16, "01f 0224 611ay0" },
{ L_, "01f 1234 611a y0", 24, "a 2222 611a wa", 80, 16, "01f 1014 611ay0" },

{ L_, "01f 8ace 611a y0", 24, "a 0000 611a wa", 80, 16, "01f 8ace 611ay0" },
{ L_, "01f 8ace 611a y0", 24, "a 1111 611a wa", 80, 16, "01f 8ace 611ay0" },
{ L_, "01f 8ace 611a y0", 24, "a 5555 611a wa", 80, 16, "01f 8a8a 611ay0" },
{ L_, "01f 8ace 611a y0", 24, "a 7777 611a wa", 80, 16, "01f 8888 611ay0" },


{ L_, "01f ffff 611a y0", 24, "a 0000 611a yhwa", 120, 16, "01f ffff 611ay0" },
{ L_, "01f ffff 611a y0", 24, "a aaaa 611a yhwa", 120, 16, "01f 5555 611ay0" },
{ L_, "01f ffff 611a y0", 24, "a 7777 611a yhwa", 120, 16, "01f 8888 611ay0" },
{ L_, "01f ffff 611a y0", 24, "a 137f 611a yhwa", 120, 16, "01f ec80 611ay0" },
{ L_, "01f ffff 611a y0", 24, "a f731 611a yhwa", 120, 16, "01f 08ce 611ay0" },

{ L_, "01f 0000 611a y0", 24, "a 48c0 611a yhwa", 120, 16, "01f 0000 611ay0" },
{ L_, "01f 0000 611a y0", 24, "a 26ae 611a yhwa", 120, 16, "01f 0000 611ay0" },
{ L_, "01f 0000 611a y0", 24, "a 159d 611a yhwa", 120, 16, "01f 0000 611ay0" },

{ L_, "01f 1234 611a y0", 24, "a 0000 611a yhwa", 120, 16, "01f 1234 611ay0" },
{ L_, "01f 1234 611a y0", 24, "a 1111 611a yhwa", 120, 16, "01f 0224 611ay0" },
{ L_, "01f 1234 611a y0", 24, "a 2222 611a yhwa", 120, 16, "01f 1014 611ay0" },

{ L_, "01f 8ace 611a y0", 24, "a 0000 611a yhwa", 120, 16, "01f 8ace 611ay0" },
{ L_, "01f 8ace 611a y0", 24, "a 1111 611a yhwa", 120, 16, "01f 8ace 611ay0" },
{ L_, "01f 8ace 611a y0", 24, "a 5555 611a yhwa", 120, 16, "01f 8a8a 611ay0" },
{ L_, "01f 8ace 611a y0", 24, "a 7777 611a yhwa", 120, 16, "01f 8888 611ay0" },


{ L_, "77901f ffff 611a yhww0", 184, "0000", 0, 16, "77901f ffff 611a yhww0" },
{ L_, "77901f ffff 611a yhww0", 184, "aaaa", 0, 16, "77901f 5555 611a yhww0" },
{ L_, "77901f ffff 611a yhww0", 184, "7777", 0, 16, "77901f 8888 611a yhww0" },
{ L_, "77901f ffff 611a yhww0", 184, "137f", 0, 16, "77901f ec80 611a yhww0" },
{ L_, "77901f ffff 611a yhww0", 184, "f731", 0, 16, "77901f 08ce 611a yhww0" },

{ L_, "77901f 0000 611a yhww0", 184, "48c0", 0, 16, "77901f 0000 611a yhww0" },
{ L_, "77901f 0000 611a yhww0", 184, "26ae", 0, 16, "77901f 0000 611a yhww0" },
{ L_, "77901f 0000 611a yhww0", 184, "159d", 0, 16, "77901f 0000 611a yhww0" },

{ L_, "77901f 1234 611a yhww0", 184, "0000", 0, 16, "77901f 1234 611a yhww0" },
{ L_, "77901f 1234 611a yhww0", 184, "1111", 0, 16, "77901f 0224 611a yhww0" },
{ L_, "77901f 1234 611a yhww0", 184, "2222", 0, 16, "77901f 1014 611a yhww0" },

{ L_, "77901f 8ace 611a yhww0", 184, "0000", 0, 16, "77901f 8ace 611a yhww0" },
{ L_, "77901f 8ace 611a yhww0", 184, "1111", 0, 16, "77901f 8ace 611a yhww0" },
{ L_, "77901f 8ace 611a yhww0", 184, "5555", 0, 16, "77901f 8a8a 611a yhww0" },
{ L_, "77901f 8ace 611a yhww0", 184, "7777", 0, 16, "77901f 8888 611a yhww0" },
};
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        for (int i = 0; i < NUM_DATA; ++i) {
            const int   LINE      = DATA[i].d_line;
            const char *DSTR      = DATA[i].d_dstBitstring_p;
            const int   DI        = DATA[i].d_dstIndex;
            const char *SSTR      = DATA[i].d_srcBitstring_p;
            const int   SI        = DATA[i].d_srcIndex;
            const int   NB        = DATA[i].d_numBits;
            const char *RESULT    = DATA[i].d_result_p;

            if (veryVerbose) {
                P_(LINE) P_(DSTR) P_(DI) P_(SSTR) P_(SI) P_(NB) P(RESULT)
            }

            uint64_t controlDst[SET_UP_ARRAY_DIM] = {0};
            uint64_t dst[       SET_UP_ARRAY_DIM];
            uint64_t controlSrc[SET_UP_ARRAY_DIM] = {0};
            uint64_t src[       SET_UP_ARRAY_DIM];
            uint64_t result[    SET_UP_ARRAY_DIM] = {0};

            populateBitStringHex(controlDst, 0, DSTR);
            wordCpy(dst, controlDst, sizeof(dst));
            populateBitStringHex(controlSrc, 0, SSTR);
            wordCpy(src, controlSrc, sizeof(src));
            populateBitStringHex(result,     0, RESULT);

            if (veryVerbose) {
                const int maxNumBits = bsl::max(DI, SI) + NB;

                P(LINE);
                BSU::print(cout, dst,    maxNumBits);
                BSU::print(cout, src,    maxNumBits);
                BSU::print(cout, result, maxNumBits);
            }

            const int NUM_DIM = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

            minusOracle(dst, DI, src, SI, NB);
            LOOP3_ASSERT(LINE, pHex(dst, NUM_DIM), pHex(result, NUM_DIM),
                                       0 == wordCmp(dst, result, sizeof(dst)));
            ASSERT(0 == wordCmp(src, controlSrc, sizeof(src)));

            wordCpy(dst, controlDst, sizeof(dst));

            BSU::minusEqual(dst, DI, src, SI, NB);
            LOOP_ASSERT(LINE, 0 == wordCmp(dst, result, sizeof(dst)));
            ASSERT(0 == wordCmp(src, controlSrc, sizeof(src)));
        }

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t ALL_TRUE[ SET_UP_ARRAY_DIM];
        uint64_t ALL_FALSE[SET_UP_ARRAY_DIM];

        uint64_t *endTrue  = ALL_TRUE  + SET_UP_ARRAY_DIM;
        uint64_t *endFalse = ALL_FALSE + SET_UP_ARRAY_DIM;

        bsl::fill(ALL_TRUE  + 0, endTrue,  ~0ULL);
        bsl::fill(ALL_FALSE + 0, endFalse,  0ULL);

        uint64_t controlDst[SET_UP_ARRAY_DIM], dst[SET_UP_ARRAY_DIM] = { 0 };
        uint64_t controlSrc[SET_UP_ARRAY_DIM], src[SET_UP_ARRAY_DIM] = { 0 };
        uint64_t result[    SET_UP_ARRAY_DIM];

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::minusEqual(dst,  0, src,  0,  0));
            ASSERT_PASS(BSU::minusEqual(dst,  0, src,  0, 70));
            ASSERT_PASS(BSU::minusEqual(dst, 70, src, 70, 70));
            ASSERT_FAIL(BSU::minusEqual(dst, -1, src, 70, 70));
            ASSERT_FAIL(BSU::minusEqual(dst, 70, src, -1, 70));
            ASSERT_FAIL(BSU::minusEqual(dst, 70, src, 70, -1));
        }

        for (int ii = 0, jj = 36; ii < 80; ) {
            setUpArray(controlDst, &ii, true);
            setUpArray(controlSrc, &jj, true);
            jj %= 80;

            if (veryVerbose) {
                P_(ii);    P(pHex(controlDst, NUM_BITS));
                P_(jj);    P(pHex(controlSrc, NUM_BITS));
            }

            const int maxIdx = NUM_BITS - 1;
            for (int dstIdx = 0; dstIdx <= maxIdx; incInt(&dstIdx, maxIdx)) {
                for (int srcIdx = 0; srcIdx <= maxIdx;
                                                     incInt(&srcIdx, maxIdx)) {
                    const int maxNumBits = NUM_BITS - bsl::max(srcIdx, dstIdx);
                    for (int numBits = 0; numBits <= maxNumBits;
                                                incInt(&numBits, maxNumBits)) {
                        const int dstTop = dstIdx + numBits;

                        // dst - ALL_FALSE

                        wordCpy(dst, controlDst, sizeof(dst));
                        BSU::minusEqual(dst, dstIdx, ALL_FALSE, srcIdx,
                                                                      numBits);
                        ASSERT(0 == wordCmp(dst, controlDst, sizeof(dst)));

                        // dst - ALL_TRUE

                        BSU::minusEqual(dst, dstIdx, ALL_TRUE, srcIdx,
                                                                      numBits);
                        ASSERT(BSU::areEqual(dst, 0, controlDst, 0, dstIdx))
                        ASSERT(! BSU::isAny1(dst, dstIdx, numBits));
                        ASSERT(BSU::areEqual(dst, dstTop, controlDst, dstTop,
                                                           NUM_BITS - dstTop));

                        // (dst = ALL_FALSE) - src

                        bsl::fill(dst + 0, dst + SET_UP_ARRAY_DIM, 0ULL);
                        wordCpy(src, controlSrc, sizeof(src));
                        BSU::minusEqual(dst, dstIdx, src, srcIdx, numBits);
                        ASSERT(! BSU::isAny1(dst, 0, NUM_BITS));

                        // (dst = ALL_TRUE) - src

                        bsl::fill(result + 0, result + SET_UP_ARRAY_DIM,~0ULL);
                        minusOracle(result, dstIdx, src, srcIdx, numBits);

                        bsl::fill(dst + 0,    dst    + SET_UP_ARRAY_DIM,~0ULL);
                        BSU::minusEqual(dst, dstIdx, src, srcIdx, numBits);
                        ASSERT(! wordCmp(result, dst, sizeof(result)));
                        ASSERT(! BSU::isAny0(dst, 0, dstIdx));
                        ASSERT(! BSU::isAny0(dst, dstTop, NUM_BITS - dstTop));

                        // result = result oracleAnd src

                        wordCpy(result, controlDst, sizeof(dst));
                        minusOracle(result, dstIdx, src, srcIdx, numBits);

                        // dst = dst & src

                        wordCpy(dst, controlDst, sizeof(dst));
                        BSU::minusEqual(dst, dstIdx, src, srcIdx, numBits);

                        // confirm result == dst

                        ASSERT(0 == wordCmp(result, dst, sizeof(dst)));

                        // confirm src never changed

                        ASSERT(0 == wordCmp(src, controlSrc, sizeof(src)));
                    }
                }
            }
        }
      } break;
      case 16: {
        // --------------------------------------------------------------------
        // TESTING 'andEqual'
        //   Ensure the method has the right effect on 'dstBitString'.
        //
        // Concerns:
        //   Test 'andEqual'.
        //
        // Plan:
        //: o Do table-driven testing of both 'andEqual' and 'andOracle'.
        //: o Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: o Do exhaustive testing with 'dst' and 'src' arrays set up by
        //:   'setUpArray'.  Loop through a variety of values of 'dstIndex',
        //:   'srcIndex', and 'numBits'.
        //:   1 Perform numerous 'andEqual' operations on the 'src' and 'dst'
        //:     arrays with ALL_TRUE and 'ALL_FALSE' arrays, then test the
        //:     results, which should be predictable.
        //:   2 Perform 'andOracle' on the 'src' and 'dst' arrays, storing the
        //:     result in 'result'.
        //:   3 Perform 'andEqual' on 'src' and 'dst', and compare 'result'
        //:     with 'dst'.
        //
        // Testing:
        //   void andEqual(uint64_t       *dstBitstring,
        //                 int             dstIndex,
        //                 const uint64_t *srcBitstring,
        //                 int             srcIndex,
        //                 int             numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING 'andEqual'\n"
                          << "==================\n";

        static const struct {
            int         d_line;
            const char *d_dstBitstring_p;
            int         d_dstIndex;
            const char *d_srcBitstring_p;
            int         d_srcIndex;
            int         d_numBits;
            const char *d_result_p;
        } DATA [] = {
//<<------<<
// Line dst  dIdx  src  sIdx  NB   Result
// ---- ---  ----  ---- ----  --   ------
{ L_,    "",   0,   "",   0,  0,       ""  },


{ L_,   "0",   0,  "0",   0,  1,      "0"  },
{ L_,   "0",   0,  "1",   0,  1,      "0"  },
{ L_,   "1",   0,  "0",   0,  1,      "0"  },
{ L_,   "1",   0,  "1",   0,  1,      "1"  },

{ L_,   "0",   0,  "0",   0,  4,      "0"  },
{ L_,   "0",   0,  "f",   0,  4,      "0"  },
{ L_,   "f",   0,  "0",   0,  4,      "0"  },
{ L_,   "f",   0,  "f",   0,  4,      "f"  },

{ L_,   "0",   0,  "0",   0,  4,      "0"  },
{ L_,   "0",   0,  "a",   0,  4,      "0"  },
{ L_,   "a",   0,  "0",   0,  4,      "0"  },
{ L_,   "a",   0,  "a",   0,  4,      "a"  },

{ L_,   "f",   0,  "a",   0,  4,      "a"  },
{ L_,   "a",   0,  "f",   0,  4,      "a"  },

{ L_,   "0",   0,  "0",   0,  4,      "0"  },
{ L_,   "0",   0,  "7",   0,  4,      "0"  },
{ L_,   "7",   0,  "0",   0,  4,      "0"  },
{ L_,   "7",   0,  "7",   0,  4,      "7"  },

{ L_,   "f",   0,  "7",   0,  4,      "7"  },
{ L_,   "7",   0,  "f",   0,  1,      "7"  },


{ L_,   "01",   4,  "0",   0,  1,      "01"  },
{ L_,   "02",   4,  "1",   0,  1,      "02"  },
{ L_,   "13",   4,  "0",   0,  1,      "03"  },
{ L_,   "14",   4,  "1",   0,  1,      "14"  },

{ L_,   "05",   4,  "0",   0,  4,      "05"  },
{ L_,   "06",   4,  "f",   0,  4,      "06"  },
{ L_,   "f7",   4,  "0",   0,  4,      "07"  },
{ L_,   "f8",   4,  "f",   0,  4,      "f8"  },

{ L_,   "09",   4,  "0",   0,  4,      "09"  },
{ L_,   "0a",   4,  "a",   0,  4,      "0a"  },
{ L_,   "ab",   4,  "0",   0,  4,      "0b"  },
{ L_,   "ac",   4,  "a",   0,  4,      "ac"  },

{ L_,   "fd",   4,  "a",   0,  4,      "ad"  },
{ L_,   "af",   4,  "f",   0,  4,      "af"  },

{ L_,   "00",   4,  "0",   0,  4,      "00"  },
{ L_,   "01",   4,  "7",   0,  4,      "01"  },
{ L_,   "72",   4,  "0",   0,  4,      "02"  },
{ L_,   "73",   4,  "7",   0,  4,      "73"  },

{ L_,   "f4",   4,  "7",   0,  4,      "74"  },
{ L_,   "75",   4,  "f",   0,  1,      "75"  },


{ L_, "101 0123456789abcdef",  68,  "6e20a75",  12,  1,
                                                     "101 0123456789abcdef" },
{ L_, "202 0123456789abcdef",  68,  "6e21a75",  12,  1,
                                                     "202 0123456789abcdef" },
{ L_, "313 0123456789abcdef",  68,  "6e20a75",  12,  1,
                                                     "303 0123456789abcdef" },
{ L_, "414 0123456789abcdef",  68,  "6e21a75",  12,  1,
                                                     "414 0123456789abcdef" },

{ L_, "505 0123456789abcdef",  68,  "6e20a75",  12,  4,
                                                     "505 0123456789abcdef" },
{ L_, "606 0123456789abcdef",  68,  "6e2fa75",  12,  4,
                                                     "606 0123456789abcdef" },
{ L_, "7f7 0123456789abcdef",  68,  "6e20a75",  12,  4,
                                                     "707 0123456789abcdef" },
{ L_, "8f8 0123456789abcdef",  68,  "6e2fa75",  12,  4,
                                                     "8f8 0123456789abcdef" },

{ L_, "909 0123456789abcdef",  68,  "6e20a75",  12,  4,
                                                     "909 0123456789abcdef" },
{ L_, "a0a 0123456789abcdef",  68,  "6e2aa75",  12,  4,
                                                     "a0a 0123456789abcdef" },
{ L_, "bab 0123456789abcdef",  68,  "6e20a75",  12,  4,
                                                     "b0b 0123456789abcdef" },
{ L_, "cac 0123456789abcdef",  68,  "6e2aa75",  12,  4,
                                                     "cac 0123456789abcdef" },

{ L_, "dfd 0123456789abcdef",  68,  "6e2aa75",  12,  4,
                                                     "dad 0123456789abcdef" },
{ L_, "faf 0123456789abcdef",  68,  "6e2fa75",  12,  4,
                                                     "faf 0123456789abcdef" },

{ L_, "000 0123456789abcdef",  68,  "6e20a75",  12,  4,
                                                     "000 0123456789abcdef" },
{ L_, "101 0123456789abcdef",  68,  "6e27a75",  12,  4,
                                                     "101 0123456789abcdef" },
{ L_, "272 0123456789abcdef",  68,  "6e20a75",  12,  4,
                                                     "202 0123456789abcdef" },
{ L_, "373 0123456789abcdef",  68,  "6e27a75",  12,  4,
                                                     "373 0123456789abcdef" },

{ L_, "4f4 0123456789abcdef",  68,  "6e27a75",  12,  4,
                                                     "474 0123456789abcdef" },
{ L_, "575 0123456789abcdef",  68,  "6e2fa75",  12,  4,
                                                     "575 0123456789abcdef" },

{ L_, "110 0123456789abcdef",  60,  "6e200a75",  12,  8,
                                                     "110 0123456789abcdef" },
{ L_, "220 0123456789abcdef",  60,  "6e211a75",  12,  8,
                                                     "220 0123456789abcdef" },
{ L_, "331 1123456789abcdef",  60,  "6e200a75",  12,  8,
                                                     "330 0123456789abcdef" },
{ L_, "441 1123456789abcdef",  60,  "6e211a75",  12,  8,
                                                     "441 1123456789abcdef" },

{ L_, "550 0123456789abcdef",  60,  "6e200a75",  12,  8,
                                                     "550 0123456789abcdef" },
{ L_, "660 0123456789abcdef",  60,  "6e2ffa75",  12,  8,
                                                     "660 0123456789abcdef" },
{ L_, "77f f123456789abcdef",  60,  "6e200a75",  12,  8,
                                                     "770 0123456789abcdef" },
{ L_, "88f f123456789abcdef",  60,  "6e2ffa75",  12,  8,
                                                     "88f f123456789abcdef" },

{ L_, "990 0123456789abcdef",  60,  "6e200a75",  12,  8,
                                                     "990 0123456789abcdef" },
{ L_, "aa0 0123456789abcdef",  60,  "6e2aaa75",  12,  8,
                                                     "aa0 0123456789abcdef" },
{ L_, "bba a123456789abcdef",  60,  "6e200a75",  12,  8,
                                                     "bb0 0123456789abcdef" },
{ L_, "cca a123456789abcdef",  60,  "6e2aaa75",  12,  8,
                                                     "cca a123456789abcdef" },

{ L_, "ddf f123456789abcdef",  60,  "6e2aaa75",  12,  8,
                                                     "dda a123456789abcdef" },
{ L_, "ffa a123456789abcdef",  60,  "6e2ffa75",  12,  8,
                                                     "ffa a123456789abcdef" },

{ L_, "000 0123456789abcdef",  60,  "6e200a75",  12,  8,
                                                     "000 0123456789abcdef" },
{ L_, "110 0123456789abcdef",  60,  "6e277a75",  12,  8,
                                                     "110 0123456789abcdef" },
{ L_, "227 7123456789abcdef",  60,  "6e200a75",  12,  8,
                                                     "220 0123456789abcdef" },
{ L_, "337 7123456789abcdef",  60,  "6e277a75",  12,  8,
                                                     "337 7123456789abcdef" },

{ L_, "44f f123456789abcdef",  60,  "6e277a75",  12,  8,
                                                      "447 7123456789abcdef" },
{ L_, "557 7123456789abcdef",  60,  "6e2ffa75",  12,  8,
                                                      "557 7123456789abcdef" },

{ L_, "d6b5 aaaa yqhww0", 184, "6e0 ffff a75wb", 76, 16, "d6b5 aaaa yqhww0" },
{ L_, "d6b5 aaaa yqhww0", 184, "6e0 7777 a75wb", 76, 16, "d6b5 2222 yqhww0" },
{ L_, "d6b5 aaaa yqhww0", 184, "6e0 2468 a75wb", 76, 16, "d6b5 2028 yqhww0" },
{ L_, "d6b5 aaaa yqhww0", 184, "6e0 8642 a75wb", 76, 16, "d6b5 8202 yqhww0" },
{ L_, "d6b5 aaaa yqhww0", 184, "6e0 8663 a75wb", 76, 16, "d6b5 8222 yqhww0" },
//>>------>>
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        for (int i = 0; i < NUM_DATA; ++i) {
            const int   LINE      = DATA[i].d_line;
            const char *DSTR      = DATA[i].d_dstBitstring_p;
            const int   DI        = DATA[i].d_dstIndex;
            const char *SSTR      = DATA[i].d_srcBitstring_p;
            const int   SI        = DATA[i].d_srcIndex;
            const int   NB        = DATA[i].d_numBits;
            const char *RESULT    = DATA[i].d_result_p;

            if (veryVerbose) {
                P_(LINE) P_(DSTR) P_(DI) P_(SSTR) P_(SI) P_(NB) P(RESULT)
            }

            uint64_t controlDst[SET_UP_ARRAY_DIM] = {0};
            uint64_t dst[       SET_UP_ARRAY_DIM];
            uint64_t controlSrc[SET_UP_ARRAY_DIM] = {0};
            uint64_t src[       SET_UP_ARRAY_DIM];
            uint64_t result[    SET_UP_ARRAY_DIM] = {0};

            populateBitStringHex(controlDst, 0, DSTR);
            wordCpy(dst, controlDst, sizeof(dst));
            populateBitStringHex(controlSrc, 0, SSTR);
            wordCpy(src, controlSrc, sizeof(src));
            populateBitStringHex(result,     0, RESULT);

            if (veryVerbose) {
                const int maxNumBits = bsl::max(DI, SI) + NB;

                P(LINE);
                BSU::print(cout, dst,    maxNumBits);
                BSU::print(cout, src,    maxNumBits);
                BSU::print(cout, result, maxNumBits);
            }

            const int NUM_DIM = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

            andOracle(dst, DI, src, SI, NB);
            LOOP3_ASSERT(LINE, pHex(dst, NUM_DIM), pHex(result, NUM_DIM),
                                       0 == wordCmp(dst, result, sizeof(dst)));
            ASSERT(0 == wordCmp(src, controlSrc, sizeof(src)));

            wordCpy(dst, controlDst, sizeof(dst));

            BSU::andEqual(dst, DI, src, SI, NB);
            LOOP_ASSERT(LINE, 0 == wordCmp(dst, result, sizeof(dst)));
            ASSERT(0 == wordCmp(src, controlSrc, sizeof(src)));
        }

        const int NUM_BITS = 3 * k_BITS_PER_UINT64;

        uint64_t ALL_TRUE[ SET_UP_ARRAY_DIM];
        uint64_t ALL_FALSE[SET_UP_ARRAY_DIM];

        uint64_t *endTrue  = ALL_TRUE  + SET_UP_ARRAY_DIM;
        uint64_t *endFalse = ALL_FALSE + SET_UP_ARRAY_DIM;

        bsl::fill(ALL_TRUE  + 0, endTrue,  ~0ULL);
        bsl::fill(ALL_FALSE + 0, endFalse,  0ULL);

        uint64_t controlDst[SET_UP_ARRAY_DIM], dst[SET_UP_ARRAY_DIM] = { 0 };
        uint64_t controlSrc[SET_UP_ARRAY_DIM], src[SET_UP_ARRAY_DIM] = { 0 };
        uint64_t result[    SET_UP_ARRAY_DIM];

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::andEqual(dst,  0, src,  0,  0));
            ASSERT_PASS(BSU::andEqual(dst,  0, src,  0, 70));
            ASSERT_PASS(BSU::andEqual(dst, 70, src, 70, 70));
            ASSERT_FAIL(BSU::andEqual(dst, -1, src, 70, 70));
            ASSERT_FAIL(BSU::andEqual(dst, 70, src, -1, 70));
            ASSERT_FAIL(BSU::andEqual(dst, 70, src, 70, -1));
        }

        for (int ii = 0, jj = 36; ii < 80; ) {
            setUpArray(controlDst, &ii, true);
            setUpArray(controlSrc, &jj, true);
            jj %= 80;

            if (veryVerbose) {
                P_(ii);    P(pHex(controlDst, NUM_BITS));
                P_(jj);    P(pHex(controlSrc, NUM_BITS));
            }

            const int maxIdx = NUM_BITS - 1;
            for (int dstIdx = 0; dstIdx <= maxIdx; incInt(&dstIdx, maxIdx)) {
                for (int srcIdx = 0; srcIdx <= maxIdx;
                                                     incInt(&srcIdx, maxIdx)) {
                    const int maxNumBits = NUM_BITS - bsl::max(srcIdx, dstIdx);
                    for (int numBits = 0; numBits <= maxNumBits;
                                                incInt(&numBits, maxNumBits)) {

                        // dst & ALL_TRUE

                        wordCpy(dst, controlDst, sizeof(dst));
                        BSU::andEqual(dst, dstIdx, ALL_TRUE, srcIdx, numBits);
                        ASSERT(0 == wordCmp(dst, controlDst, sizeof(dst)));

                        // dst & ALL_FALSE

                        BSU::andEqual(dst, dstIdx, ALL_FALSE, srcIdx, numBits);
                        ASSERT(BSU::areEqual(dst, 0, controlDst, 0, dstIdx))
                        ASSERT(! BSU::isAny1(dst, dstIdx, numBits));
                        ASSERT(BSU::areEqual(dst,        dstIdx + numBits,
                                             controlDst, dstIdx + numBits,
                                             NUM_BITS -  dstIdx - numBits));

                        // (dst = ALL_TRUE) & src

                        bsl::fill(dst + 0, dst + SET_UP_ARRAY_DIM, ~0ULL);
                        wordCpy(src, controlSrc, sizeof(src));
                        BSU::andEqual(dst, dstIdx, src, srcIdx, numBits);
                        ASSERT(! BSU::isAny0(dst, 0, dstIdx));
                        ASSERT(BSU::areEqual(dst, dstIdx, src, srcIdx,
                                                                     numBits));
                        ASSERT(! BSU::isAny0(dst, dstIdx + numBits,
                                                 NUM_BITS - dstIdx - numBits));

                        // (dst = ALL_FALSE) & src

                        bsl::fill(dst + 0, dst + SET_UP_ARRAY_DIM, 0ULL);
                        BSU::andEqual(dst, dstIdx, src, srcIdx, numBits);
                        ASSERT(! BSU::isAny1(dst, 0, NUM_BITS));

                        // result = result oracleAnd src

                        wordCpy(result, controlDst, sizeof(dst));
                        andOracle(result, dstIdx, src, srcIdx, numBits);

                        // dst = dst & src

                        wordCpy(dst, controlDst, sizeof(dst));
                        BSU::andEqual(dst, dstIdx, src, srcIdx, numBits);

                        // confirm result == dst

                        ASSERT(0 == wordCmp(result, dst, sizeof(dst)));

                        // confirm src never changed

                        ASSERT(0 == wordCmp(src, controlSrc, sizeof(src)));
                    }
                }
            }
        }
      } break;
      case 15: {
        // --------------------------------------------------------------------
        // TESTING 'toggle'
        //   Ensure the method has the right effect on 'bitString'.
        //
        // Concerns:
        //   Test toggle.
        //
        // Plan:
        //: o Use table-driven code to test both 'toggle' and the simple,
        //:   inefficient 'toggleOracle'.  Also verify that toggling twice
        //:   returns the array to its initial state.
        //: o Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: o Loop, populating an array with 'setUpArray', then do nested
        //:   loops iterating over 'index' and 'numBits'.
        //:   1 On the first iteration of 'setUpArray', do special testing of
        //:     the cases of all-true and all-false.
        //:   2 Apply 'toggle' and 'toggleOracle' and verify that their results
        //:     match, and that they do not disturb bits outside the range
        //:     specified.
        //:   3 Toggle back and verify the array is back to its initial state.
        //
        // Testing:
        //   void toggle(uint64_t *bitString, int index, int numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTESTING 'toggle'"
                          << "\n================" << bsl::endl;

        const struct {
            int         d_line;
            const char *d_array_p;
            int         d_index;
            int         d_numBits;
            const char *d_result_p;
        } DATA [] = {
            // Line  array                  idx  NB   Result
            // ----  -----                  ---  --   ------
            {   L_,  "",                     0,   0,     "" },
            {   L_,  "1",                    0,   0,    "1" },
            {   L_,  "1",                    0,   1,    "0" },
            {   L_,  "0",                    0,   1,    "1" },
            {   L_,  "00",                   0,   1,   "01" },
            {   L_,  "00",                   4,   1,   "10" },
            {   L_,  "10",                   4,   1,   "00" },
            {   L_,  "01",                   4,   1,   "11" },
            {   L_,  "101",                  4,   4,  "1f1" },
            {   L_,  "101",                  0,   1,  "100" },
            {   L_,  "101",                  8,   1,  "001" },
            {   L_,  "10",                   0,   5,   "0f" },
            {   L_,  "01",                   0,   8,   "fe" },
            {   L_,  "00",                   0,   8,   "ff" },
            {   L_,  "11",                   0,   8,   "ee" },
            {   L_,  "101",                  4,   8,  "ef1" },
            {   L_,  "010",                  4,   8,  "fe0" },
//<<------<<
{ L_, "6f6f1d ffff 9fdaea 8db265b3 e075ad52", 88, 16,
                                      "6f6f1d 0000 9fdaea 8db265b3 e075ad52" },
{ L_, "6f6f1d 0000 9fdaea 8db265b3 e075ad52", 88, 16,
                                      "6f6f1d ffff 9fdaea 8db265b3 e075ad52" },
{ L_, "6f6f1d aaaa 9fdaea 8db265b3 e075ad52", 88, 16,
                                      "6f6f1d 5555 9fdaea 8db265b3 e075ad52" },
{ L_, "6f6f1d 1234 9fdaea 8db265b3 e075ad52", 88, 16,
                                      "6f6f1d edcb 9fdaea 8db265b3 e075ad52" },

{ L_, "6f6f1d ffff 9fdaea 8db265b3 e075ad52 h7", 120, 16,
                                   "6f6f1d 0000 9fdaea 8db265b3 e075ad52 h7" },
{ L_, "6f6f1d 0000 9fdaea 8db265b3 e075ad52 h7", 120, 16,
                                   "6f6f1d ffff 9fdaea 8db265b3 e075ad52 h7" },
{ L_, "6f6f1d aaaa 9fdaea 8db265b3 e075ad52 h7", 120, 16,
                                   "6f6f1d 5555 9fdaea 8db265b3 e075ad52 h7" },
{ L_, "6f6f1d 1234 9fdaea 8db265b3 e075ad52 h7", 120, 16,
                                   "6f6f1d edcb 9fdaea 8db265b3 e075ad52 h7" },
//>>------>>
        };

        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        for (int i = 0; i < NUM_DATA; ++i) {
            const int       LINE      = DATA[i].d_line;
            const char     *STR       = DATA[i].d_array_p;
            const unsigned  LEN       = bsl::strlen(STR);
            const int       IDX       = DATA[i].d_index;
            const int       NB        = DATA[i].d_numBits;
            const char     *RESULT    = DATA[i].d_result_p;
            LOOP_ASSERT(LINE, LEN == bsl::strlen(RESULT));

            if (veryVerbose) {
                P_(LINE) P_(STR) P_(IDX) P_(NB) P(RESULT);
            }

            uint64_t control[SET_UP_ARRAY_DIM] = { 0 };
            uint64_t array[  SET_UP_ARRAY_DIM] = { 0 };
            uint64_t result[ SET_UP_ARRAY_DIM] = { 0 };
            populateBitStringHex(control, 0, STR);
            populateBitStringHex(result,  0, RESULT);

            if (veryVerbose) {
                BSU::print(cout, array,  NUM_BITS);
                BSU::print(cout, result, NUM_BITS);
            }

            // check toggle

            wordCpy(array, control, sizeof(array));
            toggleOracle(array, IDX, NB);
            LOOP_ASSERT(LINE, !wordCmp(array, result,  sizeof(array)));

            // check toggle back

            toggleOracle(array, IDX, NB);
            LOOP_ASSERT(LINE, !wordCmp(array, control, sizeof(array)));

            // check toggle

            wordCpy(array, control, sizeof(array));
            BSU::toggle(array, IDX, NB);
            LOOP_ASSERT(LINE, !wordCmp(array, result, sizeof(array)));

            // check toggle back

            BSU::toggle(array, IDX, NB);
            LOOP_ASSERT(LINE, !wordCmp(array, control, sizeof(array)));
        }

        uint64_t bits[   SET_UP_ARRAY_DIM] = { 0 };
        uint64_t control[SET_UP_ARRAY_DIM];
        uint64_t result[ SET_UP_ARRAY_DIM];

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::toggle(bits, 0, 0));
            ASSERT_PASS(BSU::toggle(bits, 0, 70));
            ASSERT_PASS(BSU::toggle(bits, 70, 70));
            ASSERT_FAIL(BSU::toggle(bits, -1, 70));
            ASSERT_FAIL(BSU::toggle(bits, 70, -1));
        }

        for (int ii = 0; ii < 150; ) {
            setUpArray(control, &ii);

            if (veryVerbose) {
                P_(ii);    P(pHex(bits, NUM_BITS));
            }

            for (int idx = 0; idx <= NUM_BITS; ++idx) {
                const int maxNumBits = NUM_BITS - idx;
                for (int numBits = 0; numBits <= maxNumBits;
                                                incInt(&numBits, maxNumBits)) {
                    const int top = idx + numBits;
                    if (0 == ii) {
                        // ALL_FALSE

                        bsl::fill(bits + 0, bits + SET_UP_ARRAY_DIM, 0ULL);
                        BSU::toggle(bits, idx, numBits);
                        ASSERT(! BSU::isAny1(bits, 0, idx));
                        ASSERT(! BSU::isAny0(bits, idx, numBits));
                        ASSERT(! BSU::isAny1(bits, top, NUM_BITS - top));

                        // toggle back to ALL_FALSE

                        BSU::toggle(bits, idx, numBits);
                        ASSERT(! BSU::isAny1(bits, 0, NUM_BITS));

                        // ALL_TRUE

                        bsl::fill(bits + 0, bits + SET_UP_ARRAY_DIM, ~0ULL);
                        BSU::toggle(bits, idx, numBits);
                        ASSERT(! BSU::isAny0(bits, 0, idx));
                        ASSERT(! BSU::isAny1(bits, idx, numBits));
                        ASSERT(! BSU::isAny0(bits, top, NUM_BITS - top));

                        // toggle back to ALL_TRUE

                        BSU::toggle(bits, idx, numBits);
                        ASSERT(! BSU::isAny0(bits, 0, NUM_BITS));
                    }

                    wordCpy(result, control, sizeof(result));
                    toggleOracle(result, idx, numBits);
                    ASSERT(BSU::areEqual(result, 0, control, 0, idx));
                    ASSERT(BSU::areEqual(result, top, control, top,
                                                              NUM_BITS - top));

                    wordCpy(bits, control, sizeof(bits));
                    BSU::toggle(bits, idx, numBits);
                    ASSERT(BSU::areEqual(bits, 0, control, 0, idx));
                    ASSERT(BSU::areEqual(bits, top, control, top,
                                                              NUM_BITS - top));

                    ASSERT(! wordCmp(bits, result, sizeof(bits)));

                    // toggle back

                    BSU::toggle(bits, idx, numBits);
                    ASSERT(! wordCmp(bits, control, sizeof(bits)));
                }
            }
        }
      } break;
      case 14: {
        // --------------------------------------------------------------------
        // TESTING 'num0' and 'num1'
        //   Ensure the methods return the expected value.
        //
        // Concerns:
        //: o Test that 'num0' and 'num1' correctly count bits.
        //: o The functions under test have special logic dealing with
        //:   multiples of 8 words, so it's important to test on arrays much
        //:   greater than 8 words long.
        //: o Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: o Write the 'countOnes' oracle, which is very simple and
        //:   inefficient and relatively foolproof.
        //: o Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: o Do testing of 'num0', 'num1', and 'countOnes' on arrays of
        //:   all-true and all-false, since the results are very predicable.
        //: o Iterate on garbage data with nested loops iterating on 'index',
        //:   and 'numBits'.
        //: o Apply 'num0' and 'num1', verifying their results against
        //:   'countOnes'.
        //: o After applying any of those functions, verify the original
        //:   array has not been modified.
        //
        // Testing:
        //   int num0(const uint64_t *bitstring, int index, int numBits);
        //   int num1(const uint64_t *bitstring, int index, int numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING 'num0' and 'num1'\n"
                          << "=========================\n";

        const int MULTIPLE = 5;
        const int DIM      = SET_UP_ARRAY_DIM * MULTIPLE;
        const int NUM_BITS = DIM * k_BITS_PER_UINT64;

        uint64_t ALL_TRUE[ DIM];
        uint64_t ALL_FALSE[DIM];

        uint64_t *endTrue  = ALL_TRUE  + DIM;
        uint64_t *endFalse = ALL_FALSE + DIM;

        bsl::fill(ALL_TRUE  + 0, endTrue,  ~0ULL);
        bsl::fill(ALL_FALSE + 0, endFalse,  0ULL);

#define CHECK_TRUE()  ASSERT(! BSU::isAny0(ALL_TRUE,  0, NUM_BITS))
#define CHECK_FALSE() ASSERT(! BSU::isAny1(ALL_FALSE, 0, NUM_BITS))

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::num1(ALL_FALSE, 0, 0));
            ASSERT_PASS(BSU::num1(ALL_FALSE, 100, 640));
            ASSERT_FAIL(BSU::num1(ALL_FALSE,  -1, 640));
            ASSERT_FAIL(BSU::num1(ALL_FALSE, 100,  -1));

            ASSERT_PASS(BSU::num0(ALL_FALSE, 0, 0));
            ASSERT_PASS(BSU::num0(ALL_FALSE, 100, 640));
            ASSERT_FAIL(BSU::num0(ALL_FALSE,  -1, 640));
            ASSERT_FAIL(BSU::num0(ALL_FALSE, 100,  -1));

            CHECK_FALSE();
        }

        for (int idx = 0; idx <= NUM_BITS; incInt(&idx, NUM_BITS)) {
            const int maxNumBits = NUM_BITS - idx;
            for (int numBits = 0; numBits <= maxNumBits;
                                                incInt(&numBits, maxNumBits)) {
                ASSERT(countOnes(ALL_TRUE,  idx, numBits) == numBits);
                CHECK_TRUE();
                ASSERT(BSU::num1(ALL_TRUE,  idx, numBits) == numBits);
                CHECK_TRUE();
                ASSERT(BSU::num0(ALL_FALSE, idx, numBits) == numBits);
                CHECK_FALSE();

                ASSERT(countOnes(ALL_FALSE, idx, numBits) == 0);
                CHECK_FALSE();
                ASSERT(BSU::num0(ALL_TRUE,  idx, numBits) == 0);
                CHECK_TRUE();
                ASSERT(BSU::num1(ALL_FALSE, idx, numBits) == 0);
                CHECK_FALSE();
            }
        }

#undef CHECK_TRUE
#undef CHECK_FALSE

        uint64_t bits[DIM], control[DIM];

        for (int ii = 0; ii < 150; ++ii) {
            fillWithGarbage(control, sizeof(control));
            wordCpy(bits, control, sizeof(bits));

            if (veryVerbose) {
                P_(ii);    P(pHex(bits, NUM_BITS));
            }
            for (int idx = 0; idx <= NUM_BITS; incInt(&idx, NUM_BITS)) {
                const int maxNumBits = NUM_BITS - idx;
                for (int numBits = 0; numBits <= maxNumBits;
                                                incInt(&numBits, maxNumBits)) {
                    const int EXP_1 = countOnes(bits, idx, numBits);
                    const int EXP_0 = numBits - EXP_1;
                    ASSERT(0 == wordCmp(bits, control, sizeof(bits)));

                    ASSERT(EXP_1 == BSU::num1(bits, idx, numBits));
                    ASSERT(0 == wordCmp(bits, control, sizeof(bits)));

                    ASSERT(EXP_0 == BSU::num0(bits, idx, numBits));
                    ASSERT(0 == wordCmp(bits, control, sizeof(bits)));
                }
            }
        }
      } break;
      case 13: {
        // --------------------------------------------------------------------
        // TESTING 'print'
        //   Ensure the method produces the correct output.
        //
        // Concerns:
        //: 1 Test that asserted precondition violations are detected when
        //:   enabled.
        //: 2 That the function correctly prints out the state of 'bitString'.
        //
        // Plan:
        //: o Do table-driven testing.
        //:   1 Each iteration, verify that 'print' produces the correct
        //:     output.
        //:   2 Each iteration, do negative testing passing a negative value to
        //:     'numBits'.
        //: o Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //
        // Testing:
        //   bsl::ostream& print(bsl::ostream&   stream,
        //                       const uint64_t *bitString,
        //                       int             numBits,
        //                       int             level,
        //                       int             spl);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTESTING 'print'"
                          << "\n===============" << bsl::endl;

        const struct {
            int         d_line;
            const char *d_inputString_p;
            int         d_numBits;
            int         d_level;
            int         d_spl;
            const char *d_expString_p;
        } DATA [] = {
            {
                L_,
                "",
                0,
                1,
                4,
                "    [\n"
                "    ]\n"
            },
            {
                L_,
                "",
                0,
                -1,
                4,
                "[\n"
                "    ]\n"
            },
            {
                L_,
                "",
                0,
                1,
                -4,
                "    [ ]"
            },
            {
                L_,
                "",
                0,
                -1,
                -4,
                "[ ]"
            },

            {
                L_,
                "",
                0,
                0,
                0,
                "[\n"
                "]\n"
            },
            {
                L_,
                "",
                0,
                0,
                4,
                "[\n"
                "]\n"
            },
            {
                L_,
                "",
                0,
                1,
                0,
                "[\n"
                "]\n"
            },
            {
                L_,
                "",
                0,
                0,
                0,
                "[\n"
                "]\n"
            },

            {
                L_,
                "2",
                2,
                1,
                4,
                "    [\n"
                "        2\n"
                "    ]\n"
            },
            {
                L_,
                "1",
                1,
                -1,
                4,
                "[\n"
                "        1\n"
                "    ]\n"
            },
            {
                L_,
                "1",
                1,
                1,
                -4,
                "    [ 1 ]"
            },
            {
                L_,
                "1",
                1,
                -1,
                -4,
                "[ 1 ]"
            },
            {
                L_,
                "1",
                1,
                0,
                4,
                "[\n"
                "    1\n"
                "]\n"
            },
            {
                L_,
                "1",
                1,
                1,
                0,
                "[\n"
                "1\n"
                "]\n"
            },
            {
                L_,
                "1",
                1,
                0,
                0,
                "[\n"
                "1\n"
                "]\n"
            },

            {
                L_,
                "10",
                5,
                1,
                4,
                "    [\n"
                "        10\n"
                "    ]\n"
            },
            {
                L_,
                "10",
                5,
                -1,
                4,
                "[\n"
                "        10\n"
                "    ]\n"
            },
            {
                L_,
                "10",
                5,
                1,
                -4,
                "    [ 10 ]"
            },
            {
                L_,
                "10",
                5,
                -1,
                -4,
                "[ 10 ]"
            },
            {
                L_,
                "10",
                5,
                0,
                4,
                "[\n"
                "    10\n"
                "]\n"
            },
            {
                L_,
                "10",
                5,
                1,
                0,
                "[\n"
                "10\n"
                "]\n"
            },
            {
                L_,
                "10",
                5,
                0,
                0,
                "[\n"
                "10\n"
                "]\n"
            },

            {
                L_,
                "10",
                5,
                1,
                4,
                "    [\n"
                "        10\n"
                "    ]\n"
            },
            {
                L_,
                "10",
                5,
                -1,
                4,
                "[\n"
                "        10\n"
                "    ]\n"
            },
            {
                L_,
                "10",
                5,
                1,
                -4,
                "    [ 10 ]"
            },
            {
                L_,
                "10",
                5,
                -1,
                -4,
                "[ 10 ]"
            },
            {
                L_,
                "10",
                8,
                0,
                4,
                "[\n"
                "    10\n"
                "]\n"
            },
            {
                L_,
                "10",
                8,
                1,
                0,
                "[\n"
                "10\n"
                "]\n"
            },
            {
                L_,
                "10",
                8,
                0,
                0,
                "[\n"
                "10\n"
                "]\n"
            },

            {
                L_,
                "1 0ef1631b87b47882",
                65,
                1,
                4,
                "    [\n"
                "        "
                    "1 0ef1631b87b47882" "\n"
                "    ]\n"
            },
            {
                L_,
                "1dbcd3e4ad41cc70 17ff2ac621cd9445 fe26dbec0694a88f"
                                                            "3e0ed2f28fbc4c40"
                "b01e38cbcc657646 77dad9a364677b33 c1c5468902bfaff5"
                                                            "8db93c524d9400ff",
                8 * 64,
                -1,
                4,
                "[\n"
                "        "
                "1dbcd3e4ad41cc70 17ff2ac621cd9445 fe26dbec0694a88f" " "
                                                           "3e0ed2f28fbc4c40\n"
                "        "
                "b01e38cbcc657646 77dad9a364677b33 c1c5468902bfaff5" " "
                                                           "8db93c524d9400ff\n"
                "    ]\n"
            },
            {
                L_,
                "                 17ff2ac621cd9445 fe26dbec0694a88f"
                                                            "3e0ed2f28fbc4c40"
                "b01e38cbcc657646 77dad9a364677b33 c1c5468902bfaff5"
                                                            "8db93c524d9400ff",
                6 * 64 + 22,
                1,
                4,
                "    [\n"
                "        "
                "                           0d9445 fe26dbec0694a88f" " "
                                                           "3e0ed2f28fbc4c40\n"
                "        "
                "b01e38cbcc657646 77dad9a364677b33 c1c5468902bfaff5" " "
                                                           "8db93c524d9400ff\n"
                "    ]\n"
            },
            {
                L_,
                "b01e38cbcc657646 77dad9a364677b33 c1c5468902bfaff5"
                                                            "8db93c524d9400ff",
                3,
                -1,
                -4,
                "[ 7 ]"
            },
            {
                L_,
                "d6b5a889fd139dd0 e2530f22dd503f92 22a2aa9b91d7148c"
                                                           " 12e5a74d313b343b"
                "6dad858fbf046bec 0c65ff70e33b9034 15bd0583dacb86e3"
                                                           " 43df8d07a9f9f0fc",
                4 * 64 + 3,
                1,
                -4,
                "    [ 3 "
                "6dad858fbf046bec 0c65ff70e33b9034 15bd0583dacb86e3"
                                                          " 43df8d07a9f9f0fc ]"
            },
            {
                L_,
                "d6b5a889fd139dd0 e2530f22dd503f92 22a2aa9b91d7148c"
                                                          " 12e5a74d313b343b"
                "6dad858fbf046bec 0c65ff70e33b9034 15bd0583dacb86e3"
                                                          " 43df8d07a9f9f0fc",
                5 * 64 + 23,
                0,
                4,
                "[\n"
                "    "
                "                                            57148c"
                                                          " 12e5a74d313b343b\n"
                "    "
                "6dad858fbf046bec 0c65ff70e33b9034 15bd0583dacb86e3"
                                                          " 43df8d07a9f9f0fc\n"
                "]\n"
            },
            {
                L_,
                "d6b5a889fd139dd0 e2530f22dd503f92 22a2aa9b91d7148c"
                                                          " 12e5a74d313b343b"
                "6dad858fbf046bec 0c65ff70e33b9034 15bd0583dacb86e3"
                                                          " 43df8d07a9f9f0fc",
                5 * 64 + 23,
                1,
                4,
                "    [\n"
                "        "
                "                                            57148c"
                                                          " 12e5a74d313b343b\n"
                "        "
                "6dad858fbf046bec 0c65ff70e33b9034 15bd0583dacb86e3"
                                                          " 43df8d07a9f9f0fc\n"
                "    ]\n"
            },
            {
                L_,
                "d6b5a889fd139dd0 e2530f22dd503f92 22a2aa9b91d7148c"
                                                          " 12e5a74d313b343b"
                "6dad858fbf046bec 0c65ff70e33b9034 15bd0583dacb86e3"
                                                          " 43df8d07a9f9f0fc",
                5 * 64 + 23,
                1,
                0,
                "[\n"
                "                                            57148c"
                                                          " 12e5a74d313b343b\n"
                "6dad858fbf046bec 0c65ff70e33b9034 15bd0583dacb86e3"
                                                          " 43df8d07a9f9f0fc\n"
                "]\n"
            },
            {
                L_,
                "d6b5a889fd139dd0 e2530f22dd503f92 22a2aa9b91d7148c"
                                                          " 12e5a74d313b343b"
                "6dad858fbf046bec 0c65ff70e33b9034 15bd0583dacb86e3"
                                                          " 43df8d07a9f9f0fc",
                6 * 64 + 5,
                0,
                0,
                "[\n"
                "                               12 22a2aa9b91d7148c"
                                                          " 12e5a74d313b343b\n"
                "6dad858fbf046bec 0c65ff70e33b9034 15bd0583dacb86e3"
                                                          " 43df8d07a9f9f0fc\n"
                "]\n"
            },
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        const int MAX_ARRAY_SIZE = 4 * 4;
        for (int i = 0; i < NUM_DATA; ++i) {
            const int   LINE  = DATA[i].d_line;
            const char *INPUT = DATA[i].d_inputString_p;
            int         NB    = DATA[i].d_numBits;
            int         LEVEL = DATA[i].d_level;
            int         SPL   = DATA[i].d_spl;
            const char *EXP   = DATA[i].d_expString_p;

            if (veryVerbose) {
                P(LINE) P_(INPUT) P_(LEVEL) P_(SPL) P(EXP);
            }

            bsl::ostringstream stream;
            uint64_t           bitstring[MAX_ARRAY_SIZE] = { 0 };

            populateBitStringHex(bitstring, 0, INPUT);
            BSU::print(stream,
                       bitstring,
                       NB,
                       LEVEL,
                       SPL);
            const bsl::string& str = stream.str();
            LOOP3_ASSERT(LINE, EXP, str, EXP == str);

            if (0 == i) {
                bsls::AssertTestHandlerGuard guard;

                // The only precondition is that 'numBits >= 0'.  Both 'LEVEL'
                // and 'SPL' are allowed to be -ve.

                ASSERT_FAIL(BSU::print(stream,
                                       bitstring,
                                       0 == NB ? -1 : -NB,
                                       LEVEL,
                                       SPL));
            }
        }

        {
            bsls::AssertTestHandlerGuard guard;

            uint64_t x = 1;
            bsl::ostringstream oss;

            ASSERT_PASS(BSU::print(oss, &x, 32, 0, 0));
            ASSERT_FAIL(BSU::print(oss, &x, -1, 0, 0));
        }
      } break;
      case 12: {
        // --------------------------------------------------------------------
        // TESTING 'swapRaw'
        //   Ensure 'swapRaw' correctly swaps the specified bit strings.
        //
        // Concerns:
        //: 1 That 'swapRaw' performs correctly on valid input.
        //: 2 That 'swapRaw' triggers an assert when told to swap overlapping
        //:   areas, on all builds.
        //: 3 Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: o Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: o Do nested loops varying two indexes into the array, and varying
        //:   the number of bits to swap.
        //: o Determine if the areas to be swapped ovERLAP.
        //:   1 If no overlap, do the swap and then verify that it works.
        //:   2 If overlap, do negative testing to ensure the assert_opt to
        //:     detect overlaps in 'swapRaw' catches it.
        //
        // Testing:
        //   void swapRaw(uint64_t *lhsBitstring,
        //                int       lhsIndex,
        //                uint64_t *rhsBitstring,
        //                int       rhsIndex,
        //                int       numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING 'swapRaw'\n"
                          << "=================\n";

        const int NUM_BITS = 2 * SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t control[2 * SET_UP_ARRAY_DIM] = { 0 };
        uint64_t bits[   2 * SET_UP_ARRAY_DIM] = { 0 };

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::swapRaw(bits, 0, control, 0, 0));
            ASSERT_PASS(BSU::swapRaw(bits, 0, control, 0, 100));
            ASSERT_PASS(BSU::swapRaw(bits, 100, control, 100, 100));
            ASSERT_FAIL(BSU::swapRaw(bits, -1, control, 100, 100));
            ASSERT_FAIL(BSU::swapRaw(bits, 100, control, -1, 100));
            ASSERT_FAIL(BSU::swapRaw(bits, 100, control, 100, -1));

            ASSERT_PASS(BSU::swapRaw(bits, 0, bits + 1, 0, 64));
            ASSERT_PASS(BSU::swapRaw(bits + 1, 0, bits, 0, 64));
            ASSERT_FAIL(BSU::swapRaw(bits, 1, bits + 1, 0, 64));
            ASSERT_FAIL(BSU::swapRaw(bits, 0, bits + 1, 0, 65));
            ASSERT_FAIL(BSU::swapRaw(bits + 1, 0, bits, 1, 64));
            ASSERT_FAIL(BSU::swapRaw(bits + 1, 0, bits, 0, 65));
        }

        for (int ii = 0; ii < 20; ++ii) {
            // To test this code properly it was necessary to have a long array
            // to test for a wide variety of of combinations of 'idxA', 'idxB',
            // and 'numBits'.  To get this done in less than 15 seconds on
            // Solaris we have to do only 20 test cases, so we use
            // 'fillWithGarbage' instead of 'setUpArray'.

            fillWithGarbage(control, sizeof(control));

            if (veryVerbose) {
                P_(ii);    P(pHex(control, NUM_BITS));
            }

            int shiftA = 0;
            int shiftB = 0;

            const int maxIdx = NUM_BITS - 1;
            for (int idxA = 0; idxA <= maxIdx; incInt(&idxA, maxIdx)) {

                for (int idxB = 0; idxB <= maxIdx; incInt(&idxB, maxIdx)) {

                    const int maxNumBits = NUM_BITS - bsl::max(idxA, idxB);
                    for (int numBits = 0; numBits <= maxNumBits;
                                                incInt(&numBits, maxNumBits)) {
                        // Shift the pointers to the arrays around to ensure
                        // the overlap detection logic in 'swapRaw' still holds
                        // up when both bitArray ptrs don't match.

                        shiftA += 3;
                        if (shiftA > 0) {
                            shiftA -= 41;
                        }
                        shiftB += 5;
                        if (shiftB > 0) {
                            shiftB -= 41;
                        }

                        uint64_t *bitsShiftA = bits + shiftA;
                        int       idxShiftA  = idxA -
                                                    shiftA * k_BITS_PER_UINT64;

                        uint64_t *bitsShiftB = bits + shiftB;
                        int       idxShiftB  = idxB -
                                                    shiftB * k_BITS_PER_UINT64;

                        if (idxA + numBits <= idxB || idxB + numBits <= idxA) {
                            wordCpy(bits, control, sizeof(bits));

                            BSU::swapRaw(bitsShiftA,
                                         idxShiftA,
                                         bitsShiftB,
                                         idxShiftB,
                                         numBits);

                            const int idxMin    = bsl::min(idxA, idxB);
                            const int endIdxMax = bsl::max(idxA, idxB) +
                                                                       numBits;

                            // First, verify all the bits that should have been
                            // unchanged.

                            ASSERT(BSU::areEqual(bits, 0, control, 0, idxMin));
                            ASSERT(BSU::areEqual(bits,
                                                 endIdxMax,
                                                 control,
                                                 endIdxMax,
                                                 NUM_BITS - endIdxMax));
                            if (idxMin + numBits < endIdxMax - numBits) {
                                ASSERT(BSU::areEqual(
                                            bits,
                                            idxMin + numBits,
                                            control,
                                            idxMin + numBits,
                                            endIdxMax - idxMin - 2 * numBits));
                            }

                            // Then, verify the bits that were swapped.

                            ASSERT(BSU::areEqual(bits,
                                                 idxA,
                                                 control,
                                                 idxB,
                                                 numBits));
                            ASSERTV(ii, idxA, idxB, numBits,
                                    pHex(bits,    NUM_BITS),
                                    pHex(control, NUM_BITS),
                                    BSU::areEqual(bits,
                                                  idxB,
                                                  control,
                                                  idxA,
                                                  numBits));
                        }
                        else {
                            bsls::AssertTestHandlerGuard guard;

                            const int preTestStatus = testStatus;

                            ASSERT_OPT_FAIL(BSU::swapRaw(bitsShiftA,
                                                         idxShiftA,
                                                         bitsShiftB,
                                                         idxShiftB,
                                                         numBits));

                            if (testStatus > preTestStatus) {
                                ASSERTV(ii, idxA, idxB, numBits, 0);
                            }
                        }
                    }
                }
            }
        }
      } break;
      case 11: {
        // --------------------------------------------------------------------
        // TESTING 'remove' and 'removeAndFill0'
        //   Ensure the methods have the right effect on 'bitString'.
        //
        // Concerns:
        //: o That 'remove' and 'removeAndFill0' function according to spec.
        //: o Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: 1 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: 2 Iterate through different initial values of an array using
        //:   'setUpArray'.
        //:   o Do nested loops iterating through 'length' and 'idx'.
        //:   o Apply 'remove' and 'removeAndFill0' with '0 == numBits' and
        //:     verify they don't change the array.
        //:   o Iterate over different values of 'numBits'.
        //:     o Call 'remove' and verify the changes are as expected.
        //:     o Call 'removeAndFill0' and verify the changes are as expected.
        //
        // Testing:
        //   void remove(uint64_t *bitstring, int len, int idx, int numBits);
        //   void removeAndFill0(uint64_t *bitstring,
        //                       int       len,
        //                       int       idx,
        //                       int       numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << "TESTING 'remove' and 'removeAndFill0'\n"
                             "=====================================\n";

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t control[SET_UP_ARRAY_DIM];
        uint64_t bits[   SET_UP_ARRAY_DIM] = { 0 };

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::remove(bits, 100, 0, 0));
            ASSERT_PASS(BSU::remove(bits, 100, 0, 10));
            ASSERT_PASS(BSU::remove(bits, 100, 0, 100));
            ASSERT_FAIL(BSU::remove(bits, 100, -1, 10));
            ASSERT_FAIL(BSU::remove(bits, 100, 0, 101));
            ASSERT_FAIL(BSU::remove(bits, -1, 0, 0));

            ASSERT_PASS(BSU::removeAndFill0(bits, 100, 0, 0));
            ASSERT_PASS(BSU::removeAndFill0(bits, 100, 0, 10));
            ASSERT_PASS(BSU::removeAndFill0(bits, 100, 0, 100));
            ASSERT_FAIL(BSU::removeAndFill0(bits, 100, -1, 10));
            ASSERT_FAIL(BSU::removeAndFill0(bits, 100, 0, 101));
            ASSERT_FAIL(BSU::removeAndFill0(bits, -1, 0, 0));
        }

        for (int ii = 0; ii < 150; ) {
            setUpArray(control, &ii);

            if (veryVerbose) {
                P_(ii);    P(pHex(control, NUM_BITS));
            }

            const int maxIdx = NUM_BITS - 1;
            for (int idx = 0; idx <= maxIdx; incInt(&idx, maxIdx)) {
                const int maxDeltaLength = NUM_BITS - idx;
                for (int length = idx, deltaLength = 0; length <= NUM_BITS;
                                          incInt(&deltaLength, maxDeltaLength),
                                                  length = idx + deltaLength) {
                    wordCpy(bits, control, sizeof(bits));

                    BSU::remove(bits, length, idx, 0);
                    ASSERT(! wordCmp(bits, control, sizeof(bits)));

                    BSU::removeAndFill0(bits, length, idx, 0);
                    ASSERT(! wordCmp(bits, control, sizeof(bits)));

                    const int maxNumBits = length - idx;
                    for (int numBits = 1; numBits <= maxNumBits;
                                                incInt(&numBits, maxNumBits)) {

                        // remove

                        wordCpy(bits, control, sizeof(bits));
                        BSU::remove(bits, length, idx, numBits);
                        ASSERT(BSU::areEqual(bits, 0, control, 0, idx));
                        ASSERT(BSU::areEqual(bits, idx,
                                             control, idx + numBits,
                                             length - (idx + numBits)));
                        ASSERT(BSU::areEqual(bits,    length - numBits,
                                             control, length - numBits,
                                             NUM_BITS - (length - numBits)));

                        // removeAndFill0

                        wordCpy(bits, control, sizeof(bits));
                        BSU::removeAndFill0(bits, length, idx, numBits);

                        ASSERT(BSU::areEqual(bits, 0, control, 0, idx));
                        ASSERT(BSU::areEqual(bits,    idx,
                                             control, idx + numBits,
                                             length - (idx + numBits)));
                        ASSERT(! BSU::isAny1(bits, length - numBits, numBits));
                        ASSERT(BSU::areEqual(bits, length, control, length,
                                             NUM_BITS - length));
                    }
                }
            }
        }
      } break;
      case 10: {
        // --------------------------------------------------------------------
        // TESTING 'insert*'
        //   Ensure the methods have the right effect on 'bitString'.
        //
        // Concerns:
        //: 1 That 'insert', 'insert0', 'insert1', and 'insertRaw' all move the
        //:   bits specified appropriately to the left.
        //: 2 That 'insert', 'insert0', and 'insert1' all initialize the bits
        //:   vacated by the left shift of the other bits approriately.
        //: 3 Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: 1 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: 2 Iterate over initial array states using 'setUpArray'.
        //:   o Iterate over values of 'idx' varying from 0 to 'NUM_BITS-1'.
        //:     1 Do inserts with all 4 funcitons with '0 == numBits' and
        //:       verify there is no change to the bit string.
        //:       o Do nested loops varying 'numBits' and 'length'.
        //:       o Call the four 'insert' methods.  Verify that the bits were
        //:         shifted appropriately, that the bits before 'idx' were
        //:         unchanged, and (when appropriate) that the range
        //:         '[ idx, idx + numBits )' was filled in appropriately.
        //
        // Testing:
        //   void insert(uint64_t  *bitstring,
        //               int        length,
        //               int        index,
        //               bool       value,
        //               int        numBits);
        //   void insert0(uint64_t *bitstring, int length,int idx,int numBits);
        //   void insert1(uint64_t *bitstring, int length,int idx,int numBits);
        //   void insertRaw(uint64_t *bitstring, int len, int idx,int numBits);
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << endl
                               << "TESTING 'insert*'\n"
                               << "=================\n";

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t control[SET_UP_ARRAY_DIM];
        uint64_t bits[   SET_UP_ARRAY_DIM] = { 0 };

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::insert(bits, 0, 0, true, 0));
            ASSERT_PASS(BSU::insert(bits, 10, 0, true, 10));
            ASSERT_PASS(BSU::insert(bits, 100, 40, true, 30));
            ASSERT_PASS(BSU::insert(bits, 100, 100, true, 60));
            ASSERT_FAIL(BSU::insert(bits, -1, 0, true, 0));
            ASSERT_FAIL(BSU::insert(bits, 0, -1, true, 0));
            ASSERT_FAIL(BSU::insert(bits, 0, 0, true, -1));
            ASSERT_FAIL(BSU::insert(bits, -1, 40, true, 30));
            ASSERT_FAIL(BSU::insert(bits, 100, -1, true, 30));
            ASSERT_FAIL(BSU::insert(bits, 100, 40, true, -1));
            ASSERT_FAIL(BSU::insert(bits, 100, 101, true, 60));

            ASSERT_PASS(BSU::insert(bits, 0, 0, false, 0));
            ASSERT_PASS(BSU::insert(bits, 10, 0, false, 10));
            ASSERT_PASS(BSU::insert(bits, 100, 40, false, 30));
            ASSERT_PASS(BSU::insert(bits, 100, 100, false, 60));
            ASSERT_FAIL(BSU::insert(bits, -1, 0, false, 0));
            ASSERT_FAIL(BSU::insert(bits, 0, -1, false, 0));
            ASSERT_FAIL(BSU::insert(bits, 0, 0, false, -1));
            ASSERT_FAIL(BSU::insert(bits, -1, 40, false, 30));
            ASSERT_FAIL(BSU::insert(bits, 100, -1, false, 30));
            ASSERT_FAIL(BSU::insert(bits, 100, 40, false, -1));
            ASSERT_FAIL(BSU::insert(bits, 100, 101, false, 60));

            ASSERT_PASS(BSU::insertRaw(bits, 0, 0, 0));
            ASSERT_PASS(BSU::insertRaw(bits, 10, 0, 10));
            ASSERT_PASS(BSU::insertRaw(bits, 100, 40, 30));
            ASSERT_PASS(BSU::insertRaw(bits, 100, 100, 60));
            ASSERT_FAIL(BSU::insertRaw(bits, -1, 0, 0));
            ASSERT_FAIL(BSU::insertRaw(bits, 0, -1, 0));
            ASSERT_FAIL(BSU::insertRaw(bits, 0, 0, -1));
            ASSERT_FAIL(BSU::insertRaw(bits, -1, 40, 30));
            ASSERT_FAIL(BSU::insertRaw(bits, 100, -1, 30));
            ASSERT_FAIL(BSU::insertRaw(bits, 100, 40, -1));
            ASSERT_FAIL(BSU::insertRaw(bits, 100, 101, 60));

            ASSERT_PASS(BSU::insert0(bits, 0, 0, 0));
            ASSERT_PASS(BSU::insert0(bits, 10, 0, 10));
            ASSERT_PASS(BSU::insert0(bits, 100, 40, 30));
            ASSERT_PASS(BSU::insert0(bits, 100, 100, 60));
            ASSERT_FAIL(BSU::insert0(bits, -1, 0, 0));
            ASSERT_FAIL(BSU::insert0(bits, 0, -1, 0));
            ASSERT_FAIL(BSU::insert0(bits, 0, 0, -1));
            ASSERT_FAIL(BSU::insert0(bits, -1, 40, 30));
            ASSERT_FAIL(BSU::insert0(bits, 100, -1, 30));
            ASSERT_FAIL(BSU::insert0(bits, 100, 40, -1));
            ASSERT_FAIL(BSU::insert0(bits, 100, 101, 60));

            ASSERT_PASS(BSU::insert1(bits, 0, 0, 0));
            ASSERT_PASS(BSU::insert1(bits, 10, 0, 10));
            ASSERT_PASS(BSU::insert1(bits, 100, 40, 30));
            ASSERT_PASS(BSU::insert1(bits, 100, 100, 60));
            ASSERT_FAIL(BSU::insert1(bits, -1, 0, 0));
            ASSERT_FAIL(BSU::insert1(bits, 0, -1, 0));
            ASSERT_FAIL(BSU::insert1(bits, 0, 0, -1));
            ASSERT_FAIL(BSU::insert1(bits, -1, 40, 30));
            ASSERT_FAIL(BSU::insert1(bits, 100, -1, 30));
            ASSERT_FAIL(BSU::insert1(bits, 100, 40, -1));
            ASSERT_FAIL(BSU::insert1(bits, 100, 101, 60));
        }

        for (int ii = 0; ii < 70; ) {
            setUpArray(control, &ii, true);

            if (veryVerbose) {
                P_(ii);    P(pHex(control, NUM_BITS));
            }

            const int maxIdx = NUM_BITS - 1;
            for (int idx = 0; idx <= maxIdx; incInt(&idx, maxIdx)) {
                wordCpy(bits, control, sizeof(bits));

                // Note this value of 'length' is overridden in the nested
                // loops below.

                int length = idx < NUM_BITS / 2 ? NUM_BITS / 2 : NUM_BITS;

                // insert true

                BSU::insert(   bits, length, idx, true,  0);
                ASSERT(0 == wordCmp(bits, control, sizeof(bits)));

                // insert1

                BSU::insert1(  bits, length, idx, 0);
                ASSERT(0 == wordCmp(bits, control, sizeof(bits)));

                // insert false

                BSU::insert(   bits, length, idx, false, 0);
                ASSERT(0 == wordCmp(bits, control, sizeof(bits)));

                // insert0

                BSU::insert0(  bits, length, idx, 0);
                ASSERT(0 == wordCmp(bits, control, sizeof(bits)));

                // insertRaw

                BSU::insertRaw(bits, length, idx, 0);
                ASSERT(0 == wordCmp(bits, control, sizeof(bits)));

                const int maxNumBits = NUM_BITS - idx;
                ASSERT(maxNumBits >= 1);
                for (int numBits = 1; numBits <= maxNumBits;
                                                incInt(&numBits, maxNumBits)) {
                    LOOP_ASSERT(numBits, numBits >= 1);
                    const int maxLength = NUM_BITS - numBits;
                    for (length = idx; length <= maxLength;
                                                  incInt(&length, maxLength)) {
                        if (veryVeryVerbose) {
                            P_(idx); P_(numBits); P(length);
                        }

                        // insert true

                        wordCpy(bits, control, sizeof(bits));
                        BSU::insert(   bits, length, idx, true,  numBits);
                        ASSERT(BSU::areEqual(bits, 0, control, 0, idx));
                        ASSERT(! BSU::isAny0(bits, idx, numBits));
                        ASSERT(BSU::areEqual(bits, idx + numBits,
                                             control, idx,
                                             length - idx));
                        ASSERT(BSU::areEqual(bits,    length + numBits,
                                             control, length + numBits,
                                             NUM_BITS - length - numBits));
                        // insert1

                        wordCpy(bits, control, sizeof(bits));
                        BSU::insert1(  bits, length, idx, numBits);
                        ASSERT(BSU::areEqual(bits, 0, control, 0, idx));
                        ASSERT(! BSU::isAny0(bits, idx, numBits));
                        ASSERT(BSU::areEqual(bits, idx + numBits,
                                             control, idx,
                                             length - idx));
                        ASSERT(BSU::areEqual(bits,    length + numBits,
                                             control, length + numBits,
                                             NUM_BITS - length - numBits));

                        // insert false

                        wordCpy(bits, control, sizeof(bits));
                        BSU::insert(   bits, length, idx, false, numBits);
                        ASSERT(BSU::areEqual(bits, 0, control, 0, idx));
                        ASSERT(! BSU::isAny1(bits, idx, numBits));
                        ASSERT(BSU::areEqual(bits, idx + numBits,
                                             control, idx,
                                             length - idx));
                        ASSERT(BSU::areEqual(bits,    length + numBits,
                                             control, length + numBits,
                                             NUM_BITS - length - numBits));

                        // insert0

                        wordCpy(bits, control, sizeof(bits));
                        BSU::insert0(  bits, length, idx, numBits);
                        ASSERT(BSU::areEqual(bits, 0, control, 0, idx));
                        ASSERT(! BSU::isAny1(bits, idx, numBits));
                        ASSERT(BSU::areEqual(bits, idx + numBits,
                                             control, idx,
                                             length - idx));
                        ASSERT(BSU::areEqual(bits,    length + numBits,
                                             control, length + numBits,
                                             NUM_BITS - length - numBits));

                        // insertRaw

                        wordCpy(bits, control, sizeof(bits));
                        BSU::insertRaw(bits, length, idx, numBits);
                        ASSERT(BSU::areEqual(bits, 0, control, 0,
                                                               idx + numBits));
                        ASSERT(BSU::areEqual(bits, idx + numBits,
                                             control, idx,
                                             length - idx));
                        ASSERT(BSU::areEqual(bits,    length + numBits,
                                             control, length + numBits,
                                             NUM_BITS - length - numBits));
                    }
                }
            }
        }
      } break;
      case 9: {
        // --------------------------------------------------------------------
        // TESTING OVERLAPPING COPIES
        //   Ensure the copy methods work properly in the overlapping case.
        //
        // Concerns:
        //: 1 That 'copy' and 'copyRaw' correctly deal with overlapping copies.
        //:   Note that 'copyRaw' can only do overlapping copies in the case
        //:   where the destination is lower than the source.
        //: 2 Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: 1 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: 2 Do nested loops varying 'dstIdx', 'srcIdx', and 'numBits'.
        //:   o Skip iterations where there is no overlap.
        //:   o Call 'copy' and observe it was correct.
        //:   o If the overlap is a right overlap, call 'copyRaw' and observe
        //:     it was correct.
        //
        // Testing:
        //   void copyRaw(uint64_t       *dstBitstring,
        //                int             dstIndex,
        //                const uint64_t *srcBitstring,
        //                int             srcIndex,
        //                int             numBits);
        //   void copy(uint64_t       *dstBitstring,
        //             int             dstIndex,
        //             const uint64_t *srcBitstring,
        //             int             srcIndex,
        //             int             numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING OVERLAPPING COPIES\n"
                             "==========================\n";

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t control[SET_UP_ARRAY_DIM];
        uint64_t bits[   SET_UP_ARRAY_DIM] = { 0 };

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::copy(bits, 0, bits, 0, 0));
            ASSERT_PASS(BSU::copy(bits, 70, bits, 10, 100));
            ASSERT_FAIL(BSU::copy(bits, 70, bits, 70, -1));
            ASSERT_FAIL(BSU::copy(bits, 70, bits, -1, 100));
            ASSERT_FAIL(BSU::copy(bits, -1, bits, 70, 100));

            ASSERT_PASS(BSU::copyRaw(bits, 0, bits, 0, 0));

            // right non-overlapping copy

            ASSERT_PASS(BSU::copyRaw(bits, 70, bits, 0, 70));

            // left overlapping copy - not guaranteed to be detected

//          ASSERT_FAIL(BSU::copyRaw(bits, 70, bits, 10, 100));

            ASSERT_FAIL(BSU::copyRaw(bits, 70, bits, 70, -1));
            ASSERT_FAIL(BSU::copyRaw(bits, 70, bits, -1, 100));
            ASSERT_FAIL(BSU::copyRaw(bits, -1, bits, 70, 100));
        }

        for (int ii = 0; ii < 150; ) {
            setUpArray(control, &ii);

            if (veryVerbose) {
                P_(ii);    P(pHex(control, NUM_BITS));
            }

            int shiftDstBy = 0;
            int shiftSrcBy = 0;

            const int maxIdx = NUM_BITS - 1;
            for (int srcIdx = 0; srcIdx <= maxIdx; incInt(&srcIdx, maxIdx)) {
                for (int dstIdx = 0; dstIdx <= maxIdx;
                                                     incInt(&dstIdx, maxIdx)) {
                    const int maxNumBits = NUM_BITS - bsl::max(dstIdx, srcIdx);
                    for (int numBits = 0; numBits <= maxNumBits;
                                                incInt(&numBits, maxNumBits)) {
                        if (intAbs(srcIdx - dstIdx) >= numBits) {
                            // Not overlapping, we already tested in the
                            // previous test case.

                            continue;
                        }

                        // Move the two array around to ensure the functions
                        // aren't confused by shifting the arrays.

                        shiftDstBy += 3;
                        if (shiftDstBy > 0) {
                            shiftDstBy -= 41;
                        }
                        shiftSrcBy += 5;
                        if (shiftSrcBy > 0) {
                            shiftSrcBy -= 41;
                        }

                        uint64_t  *shiftDst    = bits + shiftDstBy;
                        const int  shiftDstIdx = dstIdx -
                                                shiftDstBy * k_BITS_PER_UINT64;

                        uint64_t  *shiftSrc    = bits + shiftSrcBy;
                        const int  shiftSrcIdx = srcIdx -
                                                shiftSrcBy * k_BITS_PER_UINT64;

                        // copy

                        wordCpy(bits, control, sizeof(bits));
                        BSU::copy(shiftDst,
                                  shiftDstIdx,
                                  shiftSrc,
                                  shiftSrcIdx,
                                  numBits);

                        ASSERT(BSU::areEqual(bits, 0, control, 0, dstIdx));
                        ASSERT(BSU::areEqual(bits, dstIdx, control, srcIdx,
                                                                     numBits));
                        ASSERT(BSU::areEqual(bits,    dstIdx + numBits,
                                             control, dstIdx + numBits,
                                             NUM_BITS - dstIdx - numBits));

                        if (dstIdx <= srcIdx) {
                            // A left copy should work, try 'copyRaw'.

                            wordCpy(bits, control, sizeof(bits));
                            BSU::copyRaw(shiftDst,
                                         shiftDstIdx,
                                         shiftSrc,
                                         shiftSrcIdx,
                                         numBits);

                            ASSERT(BSU::areEqual(bits, 0, control, 0, dstIdx));
                            ASSERT(BSU::areEqual(bits, dstIdx, control, srcIdx,
                                                                     numBits));
                            ASSERT(BSU::areEqual(bits,    dstIdx + numBits,
                                                 control, dstIdx + numBits,
                                                 NUM_BITS - dstIdx - numBits));
                        }
                    }
                }
            }
        }
      } break;
      case 8: {
        // --------------------------------------------------------------------
        // TESTING NON-OVERLAPPING COPIES
        //   Ensure the copy methods work properly in the non-overlapping case.
        //
        // Concerns:
        //: o Test copying in the case where src and dst are in different
        //:   buffers and hence never overlap.
        //: o Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: 1 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: 2 Iterate, verying src index, dst index, and number of bits to
        //:   copy.  Perform copies and verify the behaviour was as expected.
        //
        // Testing:
        //   void copyRaw(uint64_t       *dstBitstring,
        //                int             dstIndex,
        //                const uint64_t *srcBitstring,
        //                int             srcIndex,
        //                int             numBits);
        //   void copy(uint64_t       *dstBitstring,
        //             int             dstIndex,
        //             const uint64_t *srcBitstring,
        //             int             srcIndex,
        //             int             numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING NON-OVERLAPPING COPIES\n"
                             "==============================\n";

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t dstControl[SET_UP_ARRAY_DIM], dst[SET_UP_ARRAY_DIM] = { 0 };
        uint64_t srcControl[SET_UP_ARRAY_DIM], src[SET_UP_ARRAY_DIM] = { 0 };

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::copy(dst, 0, src, 0, 0));
            ASSERT_PASS(BSU::copy(dst, 0, src, 0, 70));
            ASSERT_PASS(BSU::copy(dst, 70, src, 0, 70));
            ASSERT_PASS(BSU::copy(dst, 70, src, 170, 70));
            ASSERT_FAIL(BSU::copy(dst, -1, src, 170, 70));
            ASSERT_FAIL(BSU::copy(dst, 70, src,  -1, 70));
            ASSERT_FAIL(BSU::copy(dst, 70, src, 170, -1));

            ASSERT_PASS(BSU::copyRaw(dst, 0, src, 0, 0));
            ASSERT_PASS(BSU::copyRaw(dst, 0, src, 0, 70));
            ASSERT_PASS(BSU::copyRaw(dst, 70, src, 0, 70));
            ASSERT_PASS(BSU::copyRaw(dst, 70, src, 170, 70));
            ASSERT_FAIL(BSU::copyRaw(dst, -1, src, 170, 70));
            ASSERT_FAIL(BSU::copyRaw(dst, 70, src,  -1, 70));
            ASSERT_FAIL(BSU::copyRaw(dst, 70, src, 170, -1));
        }

        for (int ii = 0; ii < 100; ) {
            int jj = (ii * 11 + 3) % 100;
            setUpArray(dstControl, &ii);
            setUpArray(srcControl, &jj);

            const int maxIdx = NUM_BITS - 1;
            for (int srcIdx = 0; srcIdx <= maxIdx; incInt(&srcIdx, maxIdx)) {
                for (int dstIdx = 0; dstIdx <= maxIdx;
                                                     incInt(&dstIdx, maxIdx)) {
                    int maxNumBits = NUM_BITS - bsl::max(dstIdx, srcIdx);
                    for (int numBits = 0; numBits <= maxNumBits;
                                                incInt(&numBits, maxNumBits)) {
                        // copyRaw

                        wordCpy(dst, dstControl, sizeof(dst));
                        wordCpy(src, srcControl, sizeof(src));

                        BSU::copyRaw(dst, dstIdx, src, srcIdx, numBits);

                        ASSERT(!wordCmp(src, srcControl, sizeof(src)));
                        ASSERT(BSU::areEqual(dst, 0, dstControl,0,dstIdx));
                        ASSERT(BSU::areEqual(dst, dstIdx, src, srcIdx,
                                                                     numBits));
                        ASSERT(BSU::areEqual(dst,        dstIdx + numBits,
                                                dstControl, dstIdx + numBits,
                                                 NUM_BITS - dstIdx - numBits));

                        // copy

                        wordCpy(dst, dstControl, sizeof(dst));

                        BSU::copy(dst, dstIdx, src, srcIdx, numBits);

                        ASSERT(!wordCmp(src, srcControl, sizeof(src)));
                        ASSERT(BSU::areEqual(dst, 0, dstControl,0,dstIdx));
                        ASSERT(BSU::areEqual(dst, dstIdx, src, srcIdx,
                                                                     numBits));
                        ASSERT(BSU::areEqual(dst,        dstIdx + numBits,
                                                dstControl, dstIdx + numBits,
                                                 NUM_BITS - dstIdx - numBits));
                    }
                }
            }
        }
      } break;
      case 7: {
        // --------------------------------------------------------------------
        // TESTING 'isAny0' and 'isAny1'
        //   Ensure the methods return the expected value.
        //
        // Concerns:
        //: o That 'isAny0' and 'isAny1' correctly detect the presence of clear
        //:   or set bits.
        //: o Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: 1 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: 2 Iterate over different values of the 'control' buffer, using
        //:   'setUpArray'.
        //:   o Iterate over values of 'idx' and 'numBits'.
        //:     1 Apply 'isAny0' and 'isAny1' to copies of 'ALL_TRUE' and
        //:       'all_false', with predictable results (given that
        //:       'numBits > 0'.
        //:     2 Apply 'isAny0' and 'isAny1' to copies of the 'control'
        //:       buffer.  If the return value is 'false', that should mean
        //:       that stretch of the buffer should equal 'ALL_TRUE' or
        //:       'ALL_FALSE'.
        //
        // Testing:
        //   bool isAny0(const uint64_t *bitstring, int index, int numBits);
        //   bool isAny1(const uint64_t *bitstring, int index, int numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING 'isAny0' and 'isAny1'\n"
                          << "=============================\n";

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t ALL_TRUE[ SET_UP_ARRAY_DIM], allTrue[ SET_UP_ARRAY_DIM];
        uint64_t ALL_FALSE[SET_UP_ARRAY_DIM], allFalse[SET_UP_ARRAY_DIM];

        bsl::fill(ALL_TRUE  + 0, ALL_TRUE  + SET_UP_ARRAY_DIM, ~0ULL);
        bsl::fill(ALL_FALSE + 0, ALL_FALSE + SET_UP_ARRAY_DIM, 0ULL);
        bsl::fill(allTrue   + 0, allTrue   + SET_UP_ARRAY_DIM, ~0ULL);
        bsl::fill(allFalse  + 0, allFalse  + SET_UP_ARRAY_DIM, 0ULL);

        uint64_t control[SET_UP_ARRAY_DIM];
        uint64_t bits[   SET_UP_ARRAY_DIM] = { 0 };

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::isAny0(bits, 0, 0));
            ASSERT_PASS(BSU::isAny0(bits, 70, 70));
            ASSERT_FAIL(BSU::isAny0(bits, -1, 70));
            ASSERT_FAIL(BSU::isAny0(bits, 70, -1));

            ASSERT_PASS(BSU::isAny1(bits, 0, 0));
            ASSERT_PASS(BSU::isAny1(bits, 70, 70));
            ASSERT_FAIL(BSU::isAny1(bits, -1, 70));
            ASSERT_FAIL(BSU::isAny1(bits, 70, -1));
        }

        for (int ii = 0; ii < 75; ) {
            setUpArray(control, &ii);
            wordCpy(bits, control, sizeof(bits));

            if (veryVerbose) {
                P_(ii);    P(pHex(control, NUM_BITS));
            }

            for (int idx = 0; idx < NUM_BITS; ++idx) {
                ASSERT(false == BSU::isAny0(bits, idx, 0));
                ASSERT(0 == wordCmp(bits, control, sizeof(bits)));
                ASSERT(false == BSU::isAny1(bits, idx, 0));
                ASSERT(0 == wordCmp(bits, control, sizeof(bits)));

                for (int numBits = 1; idx + numBits <= NUM_BITS; ++numBits) {
                    // isAny0

                    ASSERT(false == BSU::isAny0(allTrue, idx, numBits));
                    ASSERT(true  == BSU::isAny1(allTrue, idx, numBits));
                    ASSERT(0 == wordCmp(allTrue, ALL_TRUE, sizeof(bits)));

                    ASSERT(true  == BSU::isAny0(allFalse, idx, numBits));
                    ASSERT(false == BSU::isAny1(allFalse, idx, numBits));
                    ASSERT(0 == wordCmp(allFalse, ALL_FALSE, sizeof(bits)));

                    wordCpy(bits, control, sizeof(bits));

                    LOOP3_ASSERT(ii, idx, numBits,
                        BSU::isAny0(bits, idx, numBits) ==
                            ! BSU::areEqual(bits, idx, ALL_TRUE,  0, numBits));
                    ASSERT(0 == wordCmp(bits, control, sizeof(bits)));

                    // isAny1

                    LOOP3_ASSERT(ii, idx, numBits,
                        BSU::isAny1(bits, idx, numBits) ==
                            ! BSU::areEqual(bits, idx, ALL_FALSE, 0, numBits));
                    ASSERT(0 == wordCmp(bits, control, sizeof(bits)));
                }
            }
        }
      } break;
      case 6: {
        // --------------------------------------------------------------------
        // TESTING 'assignBits'
        //   Ensure the method has the right effect on 'bitString'.
        //
        // Concerns:
        //: o That 'assignBits' performs properly on valid input.
        //: o Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: 1 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: 2 Iterate, using 'setUpArray' to populate 'control'.
        //:   o Iterate, using 'setUpArray' to populate the one word scalar
        //:     'src'.
        //:     1 terrate nested loops over values of 'index' and 'numBits'.
        //:       o Call 'assignBits'
        //:       o Use 'areEqual' to check all of 'dst'.
        //
        // Testing:
        //   void assignBits(uint64_t *bitstring,
        //                   int       index,
        //                   uint64_t  srcBits,
        //                   int       numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING 'assignBits'\n"
                             "====================\n";

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t control[ SET_UP_ARRAY_DIM];
        uint64_t dst[     SET_UP_ARRAY_DIM];
        uint64_t srcArray[SET_UP_ARRAY_DIM];

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::assignBits(dst, 0, 0, 0));
            ASSERT_PASS(BSU::assignBits(dst, 0, 0, 10));
            ASSERT_PASS(BSU::assignBits(dst, 0, 0, k_BITS_PER_UINT64));

            ASSERT_PASS(BSU::assignBits(dst, 0, -1, 0));
            ASSERT_PASS(BSU::assignBits(dst, 0, -1, 10));
            ASSERT_PASS(BSU::assignBits(dst, 0, -1, k_BITS_PER_UINT64));

            ASSERT_FAIL(BSU::assignBits(dst, -1, -1, 10));
            ASSERT_FAIL(BSU::assignBits(dst,  0, -1, k_BITS_PER_UINT64 + 1));
        }

        for (int ii = 0; ii < 72; ) {
            setUpArray(control, &ii);

            for (int jj = 0; jj < 72; ) {
                setUpArray(srcArray, &jj, true);
                const uint64_t src = srcArray[0];

                if (veryVerbose) {
                    P_(ii);    P(pHex(dst,  NUM_BITS));
                    P_(jj);    P(pHex(&src, k_BITS_PER_UINT64));
                }

                const int maxIdx = NUM_BITS - 1;
                for (int index = 0; index <= maxIdx; incInt(&index, maxIdx)) {
                    const int maxNumBits = bsl::min<int>(NUM_BITS - index,
                                                         k_BITS_PER_UINT64);
                    for (int numBits = 0; numBits <= maxNumBits; ++numBits) {
                        wordCpy(dst, control, sizeof(dst));

                        BSU::assignBits(dst, index, src, numBits);

                        ASSERT(BSU::areEqual(dst, 0, control, 0, index));
                        ASSERT(BSU::areEqual(dst, index, &src, 0, numBits));
                        ASSERT(BSU::areEqual(dst, index + numBits, control,
                               index + numBits, NUM_BITS - (index + numBits)));
                    }
                }
            }
        }
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // TESTING MULTI-BIT 'bits', 'assign', 'assign0', 'assign1'
        //   Ensure the methods have the right effect on 'bitString'.
        //
        // Concerns:
        //: o That 'bits', 'assign', 'assign0', and 'assign1' all function
        //:   properly on valid input.
        //: o Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: 1 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: 2 Iterate, populating a buffer with bit patterns using
        //:   'setUpArray'.
        //:   o Iterate nested loops over 'idx' and 'numBits'.
        //:     1 If 'numBits' fits within a word, tests 'bits'
        //:       o Using C-style bit operations to calculate 'exp', the
        //:         expected return value of 'bits'.
        //:       o Call 'bits' and compare to 'exp'.
        //:     2 Call 'assign' with the bool set to 'true'.
        //:     3 Use 'areEqual' to verify that assign had the correct effect.
        //:     4 Call 'assign1' on a separate buffer.
        //:     5 Verify that the buffers operated on by 'assign' and 'assign1'
        //:       match.
        //:     6 Call 'assign' with the bool set to 'false'.
        //:     7 Use 'areEqual' to verify that assign had the correct effect.
        //:     8 Call 'assign0' on a separate buffer.
        //:     9 Verify that the buffers operated on by 'assign' and 'assign0'
        //:       match.
        //
        // Testing:
        //   uint64_t bits(const uint64_t *bitstring, int index, int numBits);
        //   void assign(uint64_t *bitstring, int index, bool value,
        //                                                        int numBits);
        //   void assign0(uint64_t *bitstring, int index, int numBits);
        //   void assign1(uint64_t *bitstring, int index, int numBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                      << "TESTING MULTI-BIT 'bits' AND 'assign0', 'assign1'\n"
                         "=================================================\n";

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t ALL_TRUE[ SET_UP_ARRAY_DIM];
        uint64_t ALL_FALSE[SET_UP_ARRAY_DIM];

        bsl::fill(ALL_TRUE  + 0, ALL_TRUE  + SET_UP_ARRAY_DIM, ~0ULL);
        bsl::fill(ALL_FALSE + 0, ALL_FALSE + SET_UP_ARRAY_DIM, 0ULL);

        uint64_t control[SET_UP_ARRAY_DIM];
        uint64_t bits[SET_UP_ARRAY_DIM] = { 0 }, bitsB[SET_UP_ARRAY_DIM];

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::bits(bits, 0, 0));
            ASSERT_PASS(BSU::bits(bits, 0, 1));
            ASSERT_PASS(BSU::bits(bits, 0, 47));
            ASSERT_PASS(BSU::bits(bits, 100, 10));
            ASSERT_PASS(BSU::bits(bits, 100, k_BITS_PER_UINT64));
            ASSERT_FAIL(BSU::bits(bits, -1, 10));
            ASSERT_FAIL(BSU::bits(bits, 100, -1));
            ASSERT_FAIL(BSU::bits(bits, 100, k_BITS_PER_UINT64 + 1));

            ASSERT_PASS(BSU::assign(bits,  0, true,  0));
            ASSERT_PASS(BSU::assign(bits,  0, true, 70));
            ASSERT_PASS(BSU::assign(bits, 70, true, 70));
            ASSERT_FAIL(BSU::assign(bits, -1, true, 70));
            ASSERT_FAIL(BSU::assign(bits, 70, true, -1));

            ASSERT_PASS(BSU::assign(bits,  0, false,  0));
            ASSERT_PASS(BSU::assign(bits,  0, false, 70));
            ASSERT_PASS(BSU::assign(bits, 70, false, 70));
            ASSERT_FAIL(BSU::assign(bits, -1, false, 70));
            ASSERT_FAIL(BSU::assign(bits, 70, false, -1));

            ASSERT_PASS(BSU::assign0(bits,  0,  0));
            ASSERT_PASS(BSU::assign0(bits,  0, 70));
            ASSERT_PASS(BSU::assign0(bits, 70, 70));
            ASSERT_FAIL(BSU::assign0(bits, -1, 70));
            ASSERT_FAIL(BSU::assign0(bits, 70, -1));

            ASSERT_PASS(BSU::assign1(bits,  0,  0));
            ASSERT_PASS(BSU::assign1(bits,  0, 70));
            ASSERT_PASS(BSU::assign1(bits, 70, 70));
            ASSERT_FAIL(BSU::assign1(bits, -1, 70));
            ASSERT_FAIL(BSU::assign1(bits, 70, -1));
        }

        for (int ii = 0; ii < 72; ) {
            setUpArray(control, &ii, true);

            if (veryVerbose) {
                P_(ii);    P(pHex(control, NUM_BITS));
            }

            for (int idx = 0; idx < NUM_BITS; ++idx) {
                int word = idx / k_BITS_PER_UINT64;
                int pos  = idx % k_BITS_PER_UINT64;

                for (int numBits = 0; idx + numBits <= NUM_BITS; ++numBits) {
                    if (numBits <= k_BITS_PER_UINT64) {
                        int      rem = k_BITS_PER_UINT64 - pos;
                        uint64_t exp;
                        if (rem >= numBits) {
                            exp =  (control[word] >> pos) &
                                                           Mask::lt64(numBits);
                        }
                        else {
                            ASSERT(word < SET_UP_ARRAY_DIM - 1);

                            exp =   control[word] >> pos; // & lt64(rem) unnec.
                            exp |= (control[word + 1] &
                                             Mask::lt64(numBits - rem)) << rem;
                        }

                        // bits

                        wordCpy(bits, control, sizeof(bits));
                        LOOP_ASSERT(ii, BSU::bits(bits, idx, numBits) == exp);
                        LOOP_ASSERT(ii, 0 == wordCmp(bits, control,
                                                                sizeof(bits)));
                    }

                    // assign(... true ...)

                    wordCpy(bits, control, sizeof(bits));
                    BSU::assign(bits, idx, true, numBits);
                    LOOP_ASSERT(ii, BSU::areEqual(bits, 0, control, 0, idx));
                    LOOP_ASSERT(ii, BSU::areEqual(bits, idx, ALL_TRUE, 0,
                                                                     numBits));
                    LOOP_ASSERT(ii, BSU::areEqual(bits, idx + numBits,
                                                  control, idx + numBits,
                                                  NUM_BITS - idx - numBits));


                    // assign1

                    wordCpy(bitsB, control, sizeof(bits));
                    BSU::assign1(bitsB, idx, numBits);
                    LOOP_ASSERT(ii, ! wordCmp(bitsB, bits, sizeof(bits)));

                    // assign(... false ...)

                    wordCpy(bits, control, sizeof(bits));
                    BSU::assign(bits, idx, false, numBits);
                    LOOP_ASSERT(ii, BSU::areEqual(bits, 0, control, 0, idx));
                    LOOP_ASSERT(ii, BSU::areEqual(bits, idx, ALL_FALSE, 0,
                                                                     numBits));
                    LOOP_ASSERT(ii, BSU::areEqual(bits, idx + numBits,
                                                  control, idx + numBits,
                                                  NUM_BITS - idx - numBits));

                    // assign0

                    wordCpy(bitsB, control, sizeof(bits));
                    BSU::assign0(bitsB, idx, numBits);
                    LOOP_ASSERT(ii, ! wordCmp(bitsB, bits, sizeof(bits)));
                }
            }
        }
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // TESTING SINGLE-BIT 'bit', 'assign', 'assign0', and 'assign1'
        //   Ensure 'bit' returns the expected values, and the 'assign*'
        //   methods have the right effect on 'bitString'.
        //
        // Concerns:
        //: o That single-bit read and write operations function properly.
        //: o Test that asserted precondition violations are detected when
        //:   enabled.
        //
        // Plan:
        //: 1 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: 2 Iterate, populating a buffer with 'setUpArray'.
        //:   o Iterate 'idx' over all values across the buffer.
        //:     1 Use bitwise C operations to read EXP, the 'idx'th bit of the
        //:       buffer.
        //:     2 Compare 'EXP' to a call to 'bit'.
        //:     3 Use 'assign' to assign a 'true' bit at 'idx' to buffer
        //:       'bits'.
        //:     4 Verify 'bits' is as expected.
        //:     5 Use 'assign0' to assign a false bit at 'idx' to buffer
        //:       'bitsB'.
        //:     6 Confirm 'bits' and 'bitsB' match.
        //:     7 Use 'assign' to assign a 'false' bit at 'idx' to buffer
        //:       'bits'.
        //:     8 Verify 'bits' is as expected.
        //:     9 Use 'assign1' to assign a false bit at 'idx' to buffer
        //:       'bitsB'.
        //:     10 Confirm 'bits' and 'bitsB' match.
        //
        // Testing:
        //   bool bit(const uint64_t *bitstring, int index);
        //   void assign(uint64_t *bitstring, int index, bool value);
        //   void assign0(uint64_t *bitstring, int index);
        //   void assign1(uint64_t *bitstring, int index);
        // --------------------------------------------------------------------

        if (verbose) cout <<
              "TESTING SINGLE-BIT 'bit', 'assign', 'assign0', and 'assign1'\n"
              "============================================================\n";

        uint64_t BOOL[] = { 0, ~0ULL };

        const int NUM_BITS = SET_UP_ARRAY_DIM * k_BITS_PER_UINT64;

        uint64_t control[SET_UP_ARRAY_DIM], bits[SET_UP_ARRAY_DIM] = { 0 };
        uint64_t bitsB[  SET_UP_ARRAY_DIM] = { 0 };

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_SAFE_PASS(BSU::bit(bits, 0));
            ASSERT_SAFE_PASS(BSU::bit(bits, 1));
            ASSERT_SAFE_FAIL(BSU::bit(bits, -1));

            ASSERT_SAFE_PASS(BSU::assign(bits, 0, true));
            ASSERT_SAFE_PASS(BSU::assign(bits, 1, true));
            ASSERT_SAFE_PASS(BSU::assign(bits, 10, true));
            ASSERT_SAFE_FAIL(BSU::assign(bits, -1, true));

            ASSERT_SAFE_PASS(BSU::assign(bits, 0, false));
            ASSERT_SAFE_PASS(BSU::assign(bits, 1, false));
            ASSERT_SAFE_PASS(BSU::assign(bits, 10, false));
            ASSERT_SAFE_FAIL(BSU::assign(bits, -1, false));

            ASSERT_SAFE_PASS(BSU::assign0(bits, 0));
            ASSERT_SAFE_PASS(BSU::assign0(bits, 1));
            ASSERT_SAFE_PASS(BSU::assign0(bits, 10));
            ASSERT_SAFE_FAIL(BSU::assign0(bits, -1));

            ASSERT_SAFE_PASS(BSU::assign1(bits, 0));
            ASSERT_SAFE_PASS(BSU::assign1(bits, 1));
            ASSERT_SAFE_PASS(BSU::assign1(bits, 10));
            ASSERT_SAFE_FAIL(BSU::assign1(bits, -1));
        }

        for (int ii = 0; ii < 150; ) {
            setUpArray(control, &ii);

            if (veryVerbose) {
                P_(ii);    P(pHex(control, NUM_BITS));
            }

            for (int idx = 0; idx < NUM_BITS; ++idx) {
                const int index = idx / k_BITS_PER_UINT64;
                const int pos   = idx % k_BITS_PER_UINT64;

                wordCpy(bits, control, sizeof(bits));

                bool EXP = control[index] & (1ULL << pos);

                LOOP2_ASSERT(ii, idx, EXP == BSU::bit(bits, idx));

                ASSERT(0 == wordCmp(bits, control, sizeof(bits)));

                BSU::assign(bits, idx, true);

                LOOP2_ASSERT(ii, idx, BSU::areEqual(bits, idx, &BOOL[1], 0,1));
                LOOP2_ASSERT(ii, idx, (bits[index] & (1ULL << pos)));
                LOOP2_ASSERT(ii, idx, BSU::areEqual(bits, 0, control, 0, idx));
                LOOP2_ASSERT(ii, idx, BSU::areEqual(bits, idx+1, control,idx+1,
                                                        NUM_BITS - (idx + 1)));

                wordCpy(bitsB, control, sizeof(bitsB));

                BSU::assign1(bitsB, idx);

                ASSERT(0 == wordCmp(bits, bitsB, sizeof(bits)));

                wordCpy(bits, control, sizeof(bits));

                BSU::assign(bits, idx, false);

                LOOP2_ASSERT(ii, idx, BSU::areEqual(bits, idx, &BOOL[0], 0,1));
                LOOP2_ASSERT(ii, idx, !(bits[index] & (1ULL << pos)));
                LOOP2_ASSERT(ii, idx, BSU::areEqual(bits, 0, control, 0, idx));
                LOOP2_ASSERT(ii, idx, BSU::areEqual(bits, idx+1, control,idx+1,
                                                        NUM_BITS - (idx + 1)));

                wordCpy(bitsB, control, sizeof(bitsB));

                BSU::assign0(bitsB, idx);

                ASSERT(0 == wordCmp(bits, bitsB, sizeof(bits)));
            }
        }
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // TESTING 'areEqual'
        //   Ensure the methods return the expected value.
        //
        // Concerns:
        //: o That 'areEqual' correctly compares bit ranges.
        //: o Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //
        // Plan:
        //: 1 Do table-driven testing.
        //: 2 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values.
        //: 3 Iterate, populating buffers 'lhs' and 'rhs' with two different
        //:   bit strings.
        //: 4 Iterate two indexes for the 'lhs' and 'rhs', respectively, and
        //:   iterate 'numBits' over the full range possible for that pair of
        //:   indexes.
        //:   o Use 'areEqualOracle', which is written in a simple, reliable,
        //:     but inefficient way, to determine whether the ranges in the
        //:     two buffers are equal.
        //:   o Call 'areEqual' and verify that the result matches the oracle.
        //:   o When '0 == lhsIdx' and '0 == rhsIdx', verify the result of the
        //:     3 argument 'areEqual' function.
        //
        // Testing:
        //   bool areEqual(const uint64_t *lhsBitstring,
        //                 int             lhsIndex,
        //                 const uint64_t *rhsBitstring,
        //                 int             rhsIndex,
        //                 int             numBits);
        //   bool areEqual(const uint64_t *lhsBitstring,
        //                 const uint64_t *rhsBitstring,
        //                 int             numBits);
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "\nTESTING 'areEqual'"
                               << "\n==================" << bsl::endl;

        const struct {
            int         d_line;
            const char *d_lhsString_p;
            int         d_lhsIndex;
            const char *d_rhsString_p;
            int         d_rhsIndex;
            int         d_numBits;
            bool        d_expResult;
        } DATA [] = {
//  <<----<<
    // Line  LhsString          LI  RhsString          RI    NB      Result
    // ----  ---------          --  ---------          --    --      ------
    {   L_,  "",                0,  "",                0,    0,      true    },
    {   L_,  "",                0,  "1",               0,    0,      true    },
    {   L_,  "",                0,  "10",              0,    0,      true    },
    {   L_,  "",                0,  "10",              1,    0,      true    },
    {   L_,  "",                0,  "101",             0,    0,      true    },
    {   L_,  "",                0,  "101",             1,    0,      true    },
    {   L_,  "",                0,  "101",             2,    0,      true    },

    {   L_,  "1",               0,  "",                0,    0,      true    },
    {   L_,  "10",              0,  "",                0,    0,      true    },
    {   L_,  "10",              1,  "",                0,    0,      true    },
    {   L_,  "101",             0,  "",                0,    0,      true    },
    {   L_,  "101",             1,  "",                0,    0,      true    },
    {   L_,  "101",             2,  "",                0,    0,      true    },

    {   L_,  "1",               0,  "1",               0,    0,      true    },
    {   L_,  "10",              0,  "10",              1,    0,      true    },
    {   L_,  "10",              1,  "10",              0,    0,      true    },
    {   L_,  "101",             0,  "101",             0,    0,      true    },
    {   L_,  "101",             1,  "101",             2,    0,      true    },
    {   L_,  "101",             2,  "101",             1,    0,      true    },

    {   L_,  "1",               0,  "1",               0,    1,      true    },
    {   L_,  "0",               0,  "1",               0,    1,      false   },
    {   L_,  "10",              0,  "10",              0,    1,      true    },
    {   L_,  "10",              0,  "10",              1,    1,      false   },
    {   L_,  "10",              1,  "10",              0,    1,      false   },
    {   L_,  "10",              1,  "10",              1,    1,      true    },

    {   L_,  "10101",           0,  "11101",           0,    3,      true    },
    {   L_,  "10101",           1,  "11101",           0,    3,      false   },
    {   L_,  "10101",           2,  "11101",           0,    3,      true    },
    {   L_,  "10101",           0,  "11101",           1,    3,      false   },
    {   L_,  "10101",           0,  "11101",           2,    3,      false   },

    {   L_,  "11111 11111111 11111111 11111111 11111111",  0,
             "11111 11111111 11111111 11111111 11111111",  0,  0,  true   },

    {   L_,  "11111 11111111 11111111 11111111 11111111",  1,
             "11111 11111111 11111111 11111111 11111111",  1,  0,  true   },

    {   L_,  "11111 11111111 11111111 11111111 11111111",  0,
             "11111 11111111 11111111 11111111 11111111",  0,  1,  true   },

    {   L_,  "11111 11111111 11111111 11111111 11111111",  1,
             "11111 11111111 11111111 11111111 11111111",  0,  1,  true   },

    {   L_,  "11111 11111111 11111111 11111111 11111111",  0,
             "11111 11111111 11111111 11111111 11111111",  1,  1,  true   },

    {   L_,  "11111 11111111 11111111 11111111 11111111",  0,
             "11111 11111111 11111111 11111111 11111111",  0, 35,  true  },

    {   L_,  "11111 11111111 11111111 11111111 11111111",  1,
             "11111 11111111 11111111 11111111 11111111",  0, 35,  true  },

    {   L_,  "11111 11111111 11111111 11111111 11111111",  0,
             "11111 11111111 11111111 11111111 11111111",  1, 35,  true  },

    //                                               v
    {   L_,  "11111 11111111 11111111 11111111 11111101",  0,
             "11111 11111111 11111111 11111111 11111111",  0,  0,   true  },

    //                                               v
    {   L_,  "11111 11111111 11111111 11111111 11111101",  1,
             "11111 11111111 11111111 11111111 11111111",  1,  0,   true  },

    //                                               v
    {   L_,  "11111 11111111 11111111 11111111 11111101",  0,
             "11111 11111111 11111111 11111111 11111111",  0,  1,   true  },

    //                                               v
    {   L_,  "11111 11111111 11111111 11111111 11111101",  1,
             "11111 11111111 11111111 11111111 11111111",  0,  1,   false },

    //                                               v
    {   L_,  "11111 11111111 11111111 11111111 11111101",  0,
             "11111 11111111 11111111 11111111 11111111",  1,  1,   true  },

    //              v
    {   L_,  "11111 01111111 11111111 11111111 11111111",  0,
             "11111 11111111 11111111 11111111 11111111",  0, 31,   true  },

    //              v
    {   L_,  "11111 01111111 11111111 11111111 11111111",  0,
             "11111 11111111 11111111 11111111 11111111",  0, 32,   false },

    //            v
    {   L_,  "11110 11111111 11111111 11111111 11111111",  0,
             "11111 11111111 11111111 11111111 11111111",  0, 33,   false },

    //            v
    {   L_,  "11110 11111111 11111111 11111111 11111111",  1,
             "11111 11111111 11111111 11111111 11111111",  0, 32,   false },

    //            v
    {   L_,  "11110 11111111 11111111 11111111 11111111",  0,
             "11111 11111111 11111111 11111111 11111111",  1, 33,   false },

    {
      L_,
 "111 01111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
 "111 01111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
      63,
      true
    },
    {
      L_,
//     v
 "111 00111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
 "111 01111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
      63,
      false
    },
    {
      L_,
//     v
 "111 01111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
 "111 00111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
      63,
      false
    },
    {
      L_,
 "111 01111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
 "111 01111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      1,
      63,
      false
    },
    {
      L_,
 "111 01111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      1,
 "111 01111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
      63,
      false
    },
    {
      L_,
 "111 01111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      1,
 "111 01111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      1,
      63,
      true
    },

    {
      L_,
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
      64,
      true
    },
    {
      L_,
//    v
 "110 01111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
      64,
      false
    },
    {
      L_,
//    v
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
 "110 01111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
      64,
      false
    },
    {
      L_,
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      1,
      64,
      false
    },
    {
      L_,
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      1,
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
      64,
      false
    },
    {
      L_,
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      1,
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      1,
      64,
      true
    },

    {
      L_,
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
      65,
      true
    },
    {
      L_,
//  v
 "111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
      65,
      false
    },
    {
      L_,
//  v
 "110 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
 "111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111",
      0,
      65,
      false
    },
//  >>---->>
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        for (int i = 0; i < NUM_DATA; ++i) {
            const int   LINE  = DATA[i].d_line;
            const char *LSTR  = DATA[i].d_lhsString_p;
            const int   LI    = DATA[i].d_lhsIndex;
            const char *RSTR  = DATA[i].d_rhsString_p;
            const int   RI    = DATA[i].d_rhsIndex;
            const int   NB    = DATA[i].d_numBits;
            const bool  EXP   = DATA[i].d_expResult;

            if (veryVerbose) {
                P(LINE) P_(LSTR) P(LI) P_(RSTR) P(RI)
                P_(NB) P(EXP)
            }

            const int MAX_ARRAY_SIZE = 4;

            uint64_t lhs[MAX_ARRAY_SIZE] = { 0 };
            uint64_t rhs[MAX_ARRAY_SIZE] = { 0 };

            populateBitString(lhs, 0, LSTR);
            populateBitString(rhs, 0, RSTR);

            LOOP_ASSERT(LINE, EXP == BSU::areEqual(lhs, LI, rhs, RI, NB));
        }

        const int NUM_BITS = 3 * k_BITS_PER_UINT64;

        uint64_t lhs[SET_UP_ARRAY_DIM] = { 0 }, lhsCopy[SET_UP_ARRAY_DIM];
        uint64_t rhs[SET_UP_ARRAY_DIM] = { 0 }, rhsCopy[SET_UP_ARRAY_DIM];

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_PASS(BSU::areEqual(lhs, 0, rhs, 0, 0));
            ASSERT_PASS(BSU::areEqual(lhs, 0, rhs, 0, 1));
            ASSERT_PASS(BSU::areEqual(lhs, 0, rhs, 0, SET_UP_ARRAY_DIM));
            ASSERT_PASS(BSU::areEqual(lhs, 1, rhs, 0, SET_UP_ARRAY_DIM - 1));
            ASSERT_PASS(BSU::areEqual(lhs, 0, rhs, 1, SET_UP_ARRAY_DIM - 1));
            ASSERT_FAIL(BSU::areEqual(lhs, 0, rhs, 0, -1));
            ASSERT_FAIL(BSU::areEqual(lhs, -1, rhs, 0, 1));
            ASSERT_FAIL(BSU::areEqual(lhs, 0, rhs, -1, 1));
        }

        for (int ii = 0, jj = 20; ii < 200; ) {
            setUpArray(lhs, &ii, true);
            setUpArray(rhs, &jj, true);

            if (veryVerbose) {
                P_(ii);    P(pHex(lhs, NUM_BITS));
                P_(jj);    P(pHex(rhs, NUM_BITS));
            }

            wordCpy(lhsCopy, lhs, sizeof(lhs));
            wordCpy(rhsCopy, rhs, sizeof(rhs));

            const int maxIdx = NUM_BITS - 1;
            for (int lhsIdx = 0; lhsIdx <= maxIdx; incInt(&lhsIdx, maxIdx)) {
                for (int rhsIdx = 0; rhsIdx <= maxIdx;
                                                     incInt(&rhsIdx, maxIdx)) {
                    const int maxNumBits = NUM_BITS - bsl::max(rhsIdx, lhsIdx);
                    for (int numBits = 0; numBits <= maxNumBits; ++numBits) {
                        const bool EXP = areEqualOracle(lhs, lhsIdx,
                                                         rhs, rhsIdx, numBits);
                        ASSERTV(pHex(lhs, NUM_BITS) + pHex(rhs, NUM_BITS),
                                                  lhsIdx, rhsIdx, numBits, EXP,
                              BSU::areEqual(lhs, lhsIdx, rhs, rhsIdx, numBits),
                                 EXP == BSU::areEqual(lhs, lhsIdx, rhs, rhsIdx,
                                                                     numBits));

                        if (0 == (lhsIdx | rhsIdx)) {
                            ASSERT(EXP == BSU::areEqual(lhs, rhs, numBits));
                        }
                    }
                }
            }

            ASSERT(0 == wordCmp(lhsCopy, lhs, sizeof(lhs)));
            ASSERT(0 == wordCmp(rhsCopy, rhs, sizeof(rhs)));
        }
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING 'populateBitString' & 'populateBitStringHex' FUNCTIONS
        //   Ensure the methods have the right effect on 'bitString'.
        //
        // Concerns:
        //   The helper functions populates the bit array according to the
        //   input.
        //
        // Plan:
        //: 1 Do table-driven testing of the binary function by using the
        //:   function to populate a bufferr, and comparing the buffer to a
        //:   buffer in the table.
        //: 2 Do table-driven testing of the hex function by using the function
        //:   to populate a bufferr, and comparing the buffer to a buffer in
        //:   the table.
        //
        // Testing:
        //   void populateBitString(uint64_t   *bitstring,
        //                          int         idx,
        //                          const char *ascii);
        //   void populateBitStringHex(uint64_t   *bitstring,
        //                             int         idx,
        //                             const char *ascii);
        // --------------------------------------------------------------------

        if (verbose) cout <<
            "TESTING 'populateBitString' & 'populateBitStringHex' FUNCTIONS\n"
            "==============================================================\n";

        {
            const int MAX_RESULT_SIZE = 2 ;

            const struct {
                int         d_line;
                const char *d_inputString_p;
                int         d_index;
                uint64_t    d_expResult[MAX_RESULT_SIZE];
            } DATA [] = {
//      <<<<<<<<
        // Line  InputString       Index                      Expected
        // ----  -----------       -----                      --------
        {   L_,  "",                  0,   {          0,          0 }},
        {   L_,  "",                  1,   {          0,          0 }},
        {   L_,  "",                 31,   {          0,          0 }},
        {   L_,  "",                 32,   {          0,          0 }},
        {   L_,  "",                 63,   {          0,          0 }},
        {   L_,  "",                 64,   {          0,          0 }},
        {   L_,  "",                 95,   {          0,          0 }},

        {   L_, "1",                  0,   {          1,          0 }},
        {   L_, "1",                  1,   {          2,          0 }},
        {   L_, "1",                 31,   { 0x80000000,          0 }},
        {   L_, "1",                 32,   { 1ULL << 32,          0 }},
        {   L_, "1",                 63,   { 1ULL << 63,          0 }},
        {   L_, "1",                 64,   {          0,          1 }},
        {   L_, "1",                 95,   {          0, 0x80000000 }},

        {   L_, "10",                 0,   {          2,          0 }},
        {   L_, "10",                 1,   {          4,          0 }},
        {   L_, "10",                30,   { 0x80000000,          0 }},
        {   L_, "10",                31,   { 1ULL << 32,          0 }},
        {   L_, "10",                32,   { 1ULL << 33,          0 }},
        {   L_, "10",                62,   { 1ULL << 63,          0 }},
        {   L_, "10",                63,   {          0,          1 }},
        {   L_, "10",                64,   {          0,          2 }},
        {   L_, "10",                94,   {          0, 0x80000000 }},

        {   L_, "10101",              0,   {       0x15,          0 }},
        {   L_, "10101",              1,   {       0x2A,          0 }},
        {   L_, "10101",             29,   { 0x2A0000000ULL,          0 }},
        {   L_, "10101",             30,   { 0x540000000ULL,          0 }},
        {   L_, "10101",             31,   { 0xA80000000ULL,          0 }},
        {   L_, "10101",             32,   {  0x15ULL << 32,          0 }},
        {   L_, "10101",             62,   { 1ULL << 62,          5 }},
        {   L_, "10101",             63,   { 1ULL << 63,        0xA }},
        {   L_, "10101",             64,   {          0,       0x15 }},
        {   L_, "10101",             91,   {          0, 0xA8000000 }},

        {   L_, "1111 00001111 00001111 00001111 00001111",
                                  0,   {  0xF0F0F0F0FULL,                0 }},
        {   L_, "1111 00001111 00001111 00001111 00001111",
                                  2,   { 0x3C3C3C3C3CULL,                0 }},
        {   L_, "1111 00001111 00001111 00001111 00001111",
                                 15,   { 0x7878787878000ULL,             0 }},
        {   L_, "1111 00001111 00001111 00001111 00001111",
                                 30,   { 0xC3C3C3C3C0000000ULL,          3 }},
        {   L_, "1111 00001111 00001111 00001111 00001111",
                                 31,   { 0x8787878780000000ULL,          7 }},
        {   L_, "1111 00001111 00001111 00001111 00001111",
                                 32,   { 0x0F0F0F0F00000000ULL,        0xF }},
        {   L_, "1111 00001111 00001111 00001111 00001111",
                                 48,   { 0x0F0F000000000000ULL,    0xF0F0F }},
        {   L_, "1111 00001111 00001111 00001111 00001111",
                                 55,   { 0x8780000000000000ULL,  0x7878787 }},
        {   L_, "1111 00001111 00001111 00001111 00001111",
                                 60,   { 0xF000000000000000ULL, 0xF0F0F0F0 }},
//      >>>>>>>>
            };

            const int NUM_DATA = sizeof DATA / sizeof *DATA;

            for (int i = 0; i < NUM_DATA; ++i) {
                const int        LINE      = DATA[i].d_line;
                const char      *INPUT_STR = DATA[i].d_inputString_p;
                const int        INDEX     = DATA[i].d_index;
                const uint64_t  *EXP       = DATA[i].d_expResult;

                uint64_t actual[MAX_RESULT_SIZE] = { 0 };

                if (veryVerbose) {
                    P_(LINE) P_(INPUT_STR) P(INDEX)
                    P_(EXP[0]) P_(EXP[1]) P_(EXP[2])
                }

                populateBitString(actual, INDEX, INPUT_STR);
                LOOP_ASSERT(LINE, 0 == wordCmp(actual, EXP,
                                                               sizeof actual));
            }
        }

        {
            const int MAX_RESULT_SIZE = 2 ;

            const struct {
                int         d_line;
                const char *d_inputString_p;
                int         d_index;
                uint64_t    d_expResult[MAX_RESULT_SIZE];
            } DATA [] = {
//      <<<<<<<<
        // Line  InputString       Index                      Expected
        // ----  -----------       -----                      --------
        {   L_,  "",                  0,   {          0,          0 }},
        {   L_,  "",                  1,   {          0,          0 }},
        {   L_,  "",                 31,   {          0,          0 }},
        {   L_,  "",                 32,   {          0,          0 }},
        {   L_,  "",                 63,   {          0,          0 }},
        {   L_,  "",                 64,   {          0,          0 }},
        {   L_,  "",                 95,   {          0,          0 }},

        {   L_, "1",                  0,   {          1,          0 }},
        {   L_, "1",                  1,   {          2,          0 }},
        {   L_, "1",                 31,   { 0x80000000,          0 }},
        {   L_, "1",                 32,   { 1ULL << 32,          0 }},
        {   L_, "1",                 63,   { 1ULL << 63,          0 }},
        {   L_, "1",                 64,   {          0,          1 }},
        {   L_, "1",                 95,   {          0, 0x80000000 }},

        {   L_, "2",                  0,   {          2,          0 }},
        {   L_, "2",                  1,   {          4,          0 }},
        {   L_, "2",                 30,   { 0x80000000,          0 }},
        {   L_, "2",                 31,   { 1ULL << 32,          0 }},
        {   L_, "2",                 32,   { 1ULL << 33,          0 }},
        {   L_, "2",                 62,   { 1ULL << 63,          0 }},
        {   L_, "2",                 63,   {          0,          1 }},
        {   L_, "2",                 64,   {          0,          2 }},
        {   L_, "2",                 94,   {          0, 0x80000000 }},

        {   L_, "15",                 0,   {       0x15,          0 }},
        {   L_, "15",                 1,   {       0x2A,          0 }},
        {   L_, "15",                29,   { 0x2A0000000ULL,          0 }},
        {   L_, "15",                30,   { 0x540000000ULL,          0 }},
        {   L_, "15",                31,   { 0xA80000000ULL,          0 }},
        {   L_, "15",                32,   {  0x15ULL << 32,          0 }},
        {   L_, "15",                62,   { 1ULL << 62,          5 }},
        {   L_, "15",                63,   { 1ULL << 63,        0xA }},
        {   L_, "15",                64,   {          0,       0x15 }},
        {   L_, "15",                91,   {          0, 0xA8000000 }},

        {   L_, "f0f0f0f0f",      0,   {  0xF0F0F0F0FULL,                0 }},
        {   L_, "f0f0f0f0f",      2,   { 0x3C3C3C3C3CULL,                0 }},
        {   L_, "f0f0f0f0f",     15,   { 0x7878787878000ULL,             0 }},
        {   L_, "f0f0f0f0f",     30,   { 0xC3C3C3C3C0000000ULL,          3 }},
        {   L_, "f0f0f0f0f",     31,   { 0x8787878780000000ULL,          7 }},
        {   L_, "f0f0f0f0f",     48,   { 0x0F0F000000000000ULL,    0xF0F0F }},
        {   L_, "f0f0f0f0f",     55,   { 0x8780000000000000ULL,  0x7878787 }},
        {   L_, "f0f0f0f0f",     60,   { 0xF000000000000000ULL, 0xF0F0F0F0 }},

        {   L_, "5Wf",            0,   { 0xFFFFFFFFFFFFFFFFULL,          5 }},
        {   L_, "5Hf",            0,   {        0x5FFFFFFFFULL,          0 }},
        {   L_, "5qf",            0,   {            0X5FFFFULL,          0 }},
        {   L_, "5yf",            0,   {              0x5FFULL,          0 }},

        {   L_, "5WfH0",          0,   { 0xFFFFFFFF00000000ULL,
                                                            0x5FFFFFFFFULL }},
        {   L_, "5HfH0q0",        0,   { 0xFFFF000000000000ULL, 0x5FFFFULL }},
        {   L_, "5qfH0q0y0",      0,   { 0xFF00000000000000ULL,   0X5FFULL }},
        {   L_, "5yfH0q0y00",     0,   { 0xF000000000000000ULL,    0x5FULL }},

        {   L_, "5WfH0",          2,   { 0xFFFFFFFC00000000ULL,
                                                            0x17FFFFFFFFULL }},
        {   L_, "5HfH0q0",        2,   { 0xFFFC000000000000ULL, 0x17FFFFULL }},
        {   L_, "5qfH0q0y0",      2,   { 0xFC00000000000000ULL,   0X17FFULL }},
        {   L_, "5yfH0q0y00",     2,   { 0xC000000000000000ULL,    0x17FULL }},

        {   L_, "5HfHq0",         2,   { 0xFFFC000000000000ULL, 0x17FFFFULL }},
        {   L_, "5qfHqy0",        2,   { 0xFC00000000000000ULL,   0X17FFULL }},
        {   L_, "5yfHqy00",       2,   { 0xC000000000000000ULL,    0x17FULL }},

        {   L_, "5WaH0",          2,   { 0xaaaaaaa800000000ULL,
                                                            0x16aaaaaaaaULL }},
        {   L_, "5HaH0q0",        2,   { 0xAAA8000000000000ULL, 0x16AAAAULL }},
        {   L_, "5qaH0q0y0",      2,   { 0xA800000000000000ULL,   0X16AAULL }},
        {   L_, "5yaH0q0y00",     2,   { 0x8000000000000000ULL,    0x16AULL }},

        {   L_, "5HaHq0",         2,   { 0xAAA8000000000000ULL, 0x16AAAAULL }},
        {   L_, "5qaHqy0",        2,   { 0xA800000000000000ULL,   0X16AAULL }},
        {   L_, "5yaHqy00",       2,   { 0x8000000000000000ULL,    0x16AULL }},
//      >>>>>>>>
            };

            const int NUM_DATA = sizeof DATA / sizeof *DATA;
            const int NUM_BITS = MAX_RESULT_SIZE * k_BITS_PER_UINT64;

            for (int i = 0; i < NUM_DATA; ++i) {
                const int        LINE      = DATA[i].d_line;
                const char      *INPUT_STR = DATA[i].d_inputString_p;
                const int        INDEX     = DATA[i].d_index;
                const uint64_t  *EXP       = DATA[i].d_expResult;

                uint64_t actual[MAX_RESULT_SIZE] = { 0 };

                if (veryVerbose) {
                    P_(LINE) P_(INPUT_STR) P(INDEX)
                    P_(EXP[0]) P_(EXP[1]) P_(EXP[2])
                }

                populateBitStringHex(actual, INDEX, INPUT_STR);
                LOOP3_ASSERT(LINE, pHex(actual, NUM_BITS), pHex(EXP, NUM_BITS),
                                     0 == wordCmp(actual, EXP, sizeof actual));

                // Now check for splash to bits that should hve been
                // unaffected.

                for (int jj = 0; jj < 8; ++jj) {
                    const int end = INDEX + 4 * numHexDigits(INPUT_STR);

                    uint64_t control[MAX_RESULT_SIZE];
                    fillWithGarbage(control, sizeof(control));
                    wordCpy(actual, control, sizeof(actual));

                    populateBitStringHex(actual, INDEX, INPUT_STR);
                    ASSERT(areBitsEqual(actual, control, 0, INDEX));
                    ASSERT(areBitsEqual(actual, control, end, NUM_BITS - end));
                }
            }
        }
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST
        //   This test !exercises! basic functionality, but !tests! nothing.
        //
        // Concerns:
        //   Operations on "bit strings" should produce reasonable results.
        //
        // Plan:
        //   A utility component typically does not need a breathing test.
        //   This case is provided as a temporary workspace during development.
        //
        // Testing:
        //   BREATHING TEST
        // --------------------------------------------------------------------

        if (verbose) cout << "\nBREATHING TEST"
                               << "\n==============" << bsl::endl;

        const int SIZE             = 3;
        uint64_t  bitString0[SIZE] = { 0 };
        const int ARRAY_SIZE       = sizeof bitString0;
        const int ARRAY_NUM_BITS   = ARRAY_SIZE * CHAR_BIT;
        const int num0             = (SIZE - 1) * sizeof(uint64_t) * CHAR_BIT;

        if (veryVerbose) {
            cout << "\nbitString0 = " << bsl::endl;
            BSU::print(cout, bitString0, ARRAY_NUM_BITS);
        }

        for (int i = 0; i < num0; ++i) {
            LOOP_ASSERT(i, !BSU::bit(bitString0, i));
        }

        BSU::assign(bitString0, 8, true);

        if (veryVerbose) {
            cout << "\nbitString0 = " << bsl::endl;
            BSU::print(cout, bitString0, ARRAY_NUM_BITS);
        }

        for (int i = 0; i < num0; ++i) {
            LOOP_ASSERT(i, (8 == i) ==  BSU::bit(bitString0, i));
        }

        BSU::copyRaw(bitString0, 64, bitString0, 0, 64);

        if (veryVerbose) {
            cout << "\nbitString0 = " << bsl::endl;
            BSU::print(cout, bitString0, ARRAY_NUM_BITS);
        }

        for (int i = 0; i < num0; ++i) {
            LOOP_ASSERT(i, (8 == i || 72 == i) == BSU::bit(bitString0, i));
        }

        // All 1's
        uint64_t bitString1[SIZE] = { ~0ULL, ~0ULL, ~0ULL };

        if (veryVerbose) {
            cout << "\nbitString1 = " << bsl::endl;
            BSU::print(cout, bitString1, ARRAY_NUM_BITS);
        }

        for (int i = 0; i < num0; ++i) {
            LOOP_ASSERT(i, BSU::bit(bitString1, i));
        }

        BSU::assign(bitString1, 9, false);

        if (veryVerbose) {
            cout << "\nbitString1 = " << bsl::endl;
            BSU::print(cout, bitString1, ARRAY_NUM_BITS);
        }

        for (int i = 0; i < num0; ++i) {
            LOOP_ASSERT(i, (9 == i) == !BSU::bit(bitString1, i));
        }

        BSU::copyRaw(bitString1, 64, bitString1, 0, 64);

        if (veryVerbose) {
            cout << "\nbitString1 = " << bsl::endl;
            BSU::print(cout, bitString1, ARRAY_NUM_BITS);
        }

        for (int i = 0; i < num0; ++i) {
            LOOP_ASSERT(i, (9 == i || 73 == i) ==  !BSU::bit(bitString1, i));
        }

        // combinations

        BSU::copyRaw(bitString0, 0, bitString1, 64, 64);

        // bitString0: all bits <  64 except for bit 9  set
        //             all bits >= 64 except for bit 72 clear

        if (veryVerbose) {
            cout << "\nbitString0 = " << bsl::endl;
            BSU::print(cout, bitString0, ARRAY_NUM_BITS);
        }

        ASSERT(bitString0[0] == bitString1[0]);
        ASSERT(bitString0[1] != bitString1[1]);

        BSU::copyRaw(bitString1, 64, bitString0, 64, 64);

        // bitString1: all bits <  64 except for bit 9  set
        //             all bits >= 64 except for bit 72 clear

        if (veryVerbose) {
            cout << "\nbitString1 = " << bsl::endl;
            BSU::print(cout, bitString1, ARRAY_NUM_BITS);
        }

        ASSERT(bitString0[0] == bitString1[0]);
        ASSERT(bitString0[1] == bitString1[1]);

        for (int i = 0; i < num0; ++i) {
            ASSERT(BSU::bit(bitString0, i) == BSU::bit(bitString1, i));
            if (i < 64) {
                ASSERT(( 9 != i) == BSU::bit(bitString0, i));
                ASSERT(( 9 != i) == BSU::bit(bitString1, i));
            }
            else {
                ASSERT((72 == i) == BSU::bit(bitString0, i));
                ASSERT((72 == i) == BSU::bit(bitString1, i));
            }
        }

        uint64_t expBitString[SIZE];
        wordCpy(expBitString, bitString1, ARRAY_SIZE);
        ASSERT(0 == wordCmp(expBitString, bitString1, ARRAY_SIZE));

        // no effect when '0 == numBits'

        BSU::assign(bitString1, 2, true,  0);

        ASSERT(0 == wordCmp(expBitString, bitString1, ARRAY_SIZE));

        BSU::assign(bitString1, 2, false, 0);

        ASSERT(0 == wordCmp(expBitString, bitString1, ARRAY_SIZE));

        if (veryVerbose) {
            cout << "\nbitString1 = " << bsl::endl;
            BSU::print(cout, bitString1, ARRAY_NUM_BITS);
        }

        BSU::assign(bitString1, 65, true, 1);
        ASSERT(0 != wordCmp(expBitString, bitString1, ARRAY_SIZE));

        expBitString[1] |= 2;
        ASSERT(0 == wordCmp(expBitString, bitString1, ARRAY_SIZE));

        if (veryVerbose) {
            cout << "\nbitString1 = " << bsl::endl;
            BSU::print(cout, bitString1, ARRAY_NUM_BITS);
        }

        BSU::assign(bitString1, 0, false, 9);
        expBitString[0] ^= 0X1ff;
        ASSERT(0 == wordCmp(expBitString, bitString1, ARRAY_SIZE));

        if (veryVerbose) {
            cout << "\nbitString1 = " << bsl::endl;
            BSU::print(cout, bitString1, ARRAY_NUM_BITS);
        }

        BSU::assign(bitString0,  0, false, 32);
        BSU::assign(bitString0, 32,  true, 32);
        BSU::assign(bitString0, 64, false, 32);
        BSU::assign(bitString0, 96,  true, 32);

        ASSERT(0xffffffff00000000ULL == bitString0[0]);
        ASSERT(0xffffffff00000000ULL == bitString0[1]);

        BSU::assign(bitString1,  0,  true, 32);
        BSU::assign(bitString1, 32, false, 32);
        BSU::assign(bitString1, 64,  true, 32);
        BSU::assign(bitString1, 96, false, 32);

        ASSERT(0x00000000ffffffffULL == bitString1[0]);
        ASSERT(0x00000000ffffffffULL == bitString1[1]);
        if (veryVerbose) {
            cout << "\nbitString0 = " << bsl::endl;
            BSU::print(cout, bitString0, ARRAY_NUM_BITS);
            cout << "\nbitString1 = " << bsl::endl;
            BSU::print(cout, bitString1, ARRAY_NUM_BITS);
        }

        //////////////////////////////////////////////////////

        BSU::copyRaw(bitString1, 16, bitString0, 8, 72);
        expBitString[0] = 0xffffff000000ffffULL;
        expBitString[1] = 0x00000000ff0000ffULL;
        ASSERT(0 == wordCmp(expBitString, bitString1, ARRAY_SIZE));

        if (veryVerbose) {
            cout << "\nbitString1 = " << bsl::endl;
            BSU::print(cout, bitString1, ARRAY_NUM_BITS);
        }

        wordCpy(expBitString, bitString0, ARRAY_SIZE);
        BSU::copyRaw(bitString0, 68, bitString1, 8, 20);
        expBitString[1] = 0xffffffff00000ff0ULL;
        LOOP2_ASSERT(pHex(bitString0,   ARRAY_NUM_BITS),
                     pHex(expBitString, ARRAY_NUM_BITS),
                       0 == wordCmp(expBitString, bitString0, ARRAY_SIZE));

        if (veryVerbose) {
            cout << "\nbitString0 = " << bsl::endl;
            BSU::print(cout, bitString0, ARRAY_NUM_BITS);
            cout << "\nbitString1 = " << bsl::endl;
            BSU::print(cout, bitString1, ARRAY_NUM_BITS);
        }

        BSU::copyRaw(bitString1, 24, bitString1, 4, 16);
        expBitString[0] = 0xffffff0fff00ffffULL;
        expBitString[1] = 0x00000000ff0000ffULL;
        expBitString[2] = ~0ULL;
        ASSERT(0 == wordCmp(expBitString, bitString1, ARRAY_SIZE));

        if (veryVerbose) {
            cout << "\nbitString0 = " << bsl::endl;
            BSU::print(cout, bitString0, ARRAY_NUM_BITS);
            cout << "\nbitString1 = " << bsl::endl;
            BSU::print(cout, bitString1, ARRAY_NUM_BITS);
            cout << "\nexpBitString = " << bsl::endl;
            BSU::print(cout, expBitString, ARRAY_NUM_BITS);
        }

        bitString0[0]   = 0xfedcba0987654321ULL;
        bitString0[1]   = 0x1234567890abcdefULL;
        bitString0[2]   = 0x0b0b0b0b0b0b0b0bULL;

        BSU::insert1(bitString0, num0, 8, 12);

        expBitString[0] = 0xcba09876543fff21ULL;
        expBitString[1] = 0x4567890abcdeffedULL;
        expBitString[2] = 0x0b0b0b0b0b0b0123ULL;
        LOOP2_ASSERT(pHex(bitString0,   ARRAY_NUM_BITS),
                     pHex(expBitString, ARRAY_NUM_BITS),
                       0 == wordCmp(expBitString, bitString0, ARRAY_SIZE));

        if (veryVerbose) {
            cout << "\nbitString0 = " << bsl::endl;
            BSU::print(cout, bitString0, ARRAY_NUM_BITS);
            cout << "\nexpBitString = " << bsl::endl;
            BSU::print(cout, expBitString, ARRAY_NUM_BITS);
        }

        BSU::removeAndFill0(bitString0, num0, 8, 12);

        expBitString[0] = 0xfedcba0987654321ULL;
        expBitString[1] = 0x0004567890abcdefULL;
        expBitString[2] = 0x0b0b0b0b0b0b0123ULL;

        LOOP2_ASSERT(pHex(bitString0,   ARRAY_NUM_BITS),
                     pHex(expBitString, ARRAY_NUM_BITS),
                       0 == wordCmp(expBitString, bitString0, ARRAY_SIZE));

        bitString0[1]   = 0x1234567890abcdefULL;
        bitString0[2]   = 0x0b0b0b0b0b0b0b0bULL;
        expBitString[1] = 0x1234567890abcdefULL;
        expBitString[2] = 0x0b0b0b0b0b0b0b0bULL;

        // inserting 0 bits changes nothing

        BSU::insert1(bitString0, num0, 1, 0);

        LOOP2_ASSERT(pHex(bitString0,   ARRAY_NUM_BITS),
                     pHex(expBitString, ARRAY_NUM_BITS),
                       0 == wordCmp(expBitString, bitString0, ARRAY_SIZE));

        // removing 0 bits changes nothing

        BSU::removeAndFill0(bitString0, num0, 8, 0);

        LOOP2_ASSERT(pHex(bitString0,   ARRAY_NUM_BITS),
                     pHex(expBitString, ARRAY_NUM_BITS),
                       0 == wordCmp(expBitString, bitString0, ARRAY_SIZE));

        BSU::copy(bitString0, 0, bitString0, 4, 68);

        expBitString[0] = 0xffedcba098765432ULL;
        expBitString[1] = 0x1234567890abcdeeULL;
        expBitString[2] = 0x0b0b0b0b0b0b0b0bULL;

        LOOP2_ASSERT(pHex(bitString0,   ARRAY_NUM_BITS),
                     pHex(expBitString, ARRAY_NUM_BITS),
                       0 == wordCmp(expBitString, bitString0, ARRAY_SIZE));

        bitString0[0]   = 0xfedcba0987654321ULL;
        bitString0[1]   = 0x1234567890abcdefULL;

        BSU::copy(bitString0, 4, bitString0, 0, 68);

        expBitString[0] = 0xedcba09876543211ULL;
        expBitString[1] = 0x1234567890abcdffULL;

        LOOP2_ASSERT(pHex(bitString0,   ARRAY_NUM_BITS),
                     pHex(expBitString, ARRAY_NUM_BITS),
                       0 == wordCmp(expBitString, bitString0, ARRAY_SIZE));
      } break;
      default: {
        bsl::cerr << "WARNING: CASE `" << test << "' NOT FOUND.\n";
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        bsl::cerr << "Error, non-zero test status = " << testStatus << "."
                  << bsl::endl;
    }

    return testStatus;
}

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
