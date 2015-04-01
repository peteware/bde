// bdlb_bitmaskutil.t.cpp                                             -*-C++-*-

#include <bdlb_bitmaskutil.h>

#include <bdls_testutil.h>

#include <bslmf_assert.h>
#include <bsls_asserttest.h>
#include <bsls_stopwatch.h>
#include <bsls_types.h>

#include <bsl_algorithm.h>
#include <bsl_iostream.h>

#include <bsl_cctype.h>      // isspace()
#include <bsl_c_limits.h>    // INT_MIN, INT_MAX
#include <bsl_cstdlib.h>     // atoi()
#include <bsl_cstring.h>     // strcmp()

using namespace BloombergLP;
using namespace bsl;  // automatically added by script

//=============================================================================
//                                TEST PLAN
//-----------------------------------------------------------------------------
//                                Overview
//                                --------
// Most of the testing is done in cases 4 and 5.  Since variation in output
// happens over the range '0 <= i <= 32' or '0 <= i <= 64', it is possible to
// exhaustively test these functions for all possible inputs.
//-----------------------------------------------------------------------------
// [ 3] enum { WORD_SIZE = sizeof(int) };
// [ 3] enum { BITS_PER_BYTE = 8 };
// [ 3] enum { BITS_PER_WORD = BITS_PER_BYTE * WORD_SIZE };
// [ 4] uint32_t eq(int index);
// [ 4] uint64_t eq64(int index);
// [ 4] uint32_t ge(int index);
// [ 4] uint64_t ge64(int index);
// [ 4] uint32_t gt(int index);
// [ 4] uint64_t gt64(int index);
// [ 4] uint32_t le(int index);
// [ 4] uint64_t le64(int index);
// [ 4] uint32_t lt(int index);
// [ 4] uint64_t lt64(int index);
// [ 4] uint32_t ne(int index);
// [ 4] uint64_t ne64(int index);
// [ 5] uint32_t zero(int index, int nBits);
// [ 5] uint64_t zero64(int index, int nBits);
// [ 5] uint32_t one(int index, int nBits);
// [ 5] uint64_t one64(int index, int nBITS);
// [ 6] USAGE EXAMPLE
// [ 2] GENERATOR FUNCTION: uint32_t g(const char *spec)
// [ 2] GENERATOR FUNCTION: uint64_t g64(const char *spec)
// [ 1] BREATHING TEST
//-----------------------------------------------------------------------------

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

typedef bdlb::BitMaskUtil Util;

typedef Util::uint32_t uint32_t;
typedef Util::uint64_t uint64_t;

const static uint64_t int64Max  = ~0ULL / 2;
const static uint64_t int64Min  = 1ULL << 63;
const static uint64_t zero64    = 0;
const static uint64_t one64     = 1;
const static uint64_t two64     = 2;

const static uint32_t zero      = 0;
const static uint32_t one       = 1;
const static uint32_t two       = 2;

enum { BITS_PER_WORD   = 8 *     sizeof(int),  BPW = BITS_PER_WORD  };
enum { BITS_PER_UINT64 = 8 * sizeof(uint64_t), BPS = BITS_PER_UINT64 };

#define CAT4(A, B, C, D)  CAT(CAT(A, B), CAT(C, D))
#define CAT(X, Y) CAT_IMP(X, Y)
#define CAT_IMP(X, Y) X##Y

// Hex Bit Strings
#define x0_ "0000"
#define x5_ "0101"
#define xA_ "1010"
#define xF_ "1111"

// Longer Bit Strings
#define B8_0 x0_ x0_
#define B8_1 xF_ xF_
#define B8_01 x5_ x5_
#define B8_10 xA_ xA_
#define B12_0 B8_0 x0_
#define B12_1 B8_1 xF_
#define B16_0 B8_0 B8_0
#define B16_1 B8_1 B8_1
#define B16_01 B8_01 B8_01
#define B16_10 B8_10 B8_10
#define B32_0 B16_0 B16_0
#define B32_1 B16_1 B16_1
#define B32_01 B16_01 B16_01
#define B32_10 B16_10 B16_10

// Platform-Dependent Bit Strings
BSLMF_ASSERT(4 == sizeof(uint32_t));
BSLMF_ASSERT((long long) (int64Min - 1) > (long long) int64Min);

#define EW_0 x0_
#define EW_1 xF_
#define FW_01 B32_01
#define FW_10 B32_10
#define SW_01 B32_01 B32_01
#define SW_10 B32_10 B32_10
#define MID_0   " 00000000 00000000 "
#define MID_1   " 11111111 11111111 "
#define MID_01  " 01010101 01010101 "
#define MID_10  " 10101010 10101010 "

// Word Relative Bit Strings
#define QW_0 EW_0 EW_0
#define QW_1 EW_1 EW_1
#define HW_0 QW_0 QW_0
#define HW_1 QW_1 QW_1
#define FW_0 HW_0 HW_0
#define FW_1 HW_1 HW_1
#define SW_0 FW_0 FW_0
#define SW_1 FW_1 FW_1

//=============================================================================
//                          GLOBAL VARIABLES FOR TESTING
//-----------------------------------------------------------------------------

int verbose;
int veryVerbose;

//=============================================================================
//        GENERATOR FUNCTION 'int g(const char *spec)' FOR TESTING
//-----------------------------------------------------------------------------
// The following function interprets the given 'spec' in order from left to
// right to configure an integer according to a custom language.  Valid
// meaningful characters are the binary digits ('0' and '1') and a period
// ('.') used to indicate a sequence (e.g., "0..0" or "1..1").  At most one
// sequence may appear in a single spec.  Space characters are ignored; all
// other characters are invalid.
//..
// LANGUAGE SPECIFICATION:
// -----------------------
//
// <SPEC>       ::= <EMPTY>                    ; ""
//                | <DIGIT_LIST>               ; "1100"
//                | <ITEM_LIST>                ; "1110..0"
//                | <ITEM_LIST><DIGIT_LIST>    ; "1110..01100"
//
// <ITEM_LIST>  ::= <ITEM>                     ; item without leading digits
//                | <DIGIT_LIST><ITEM>         ; item with leading digits
//
// <DIGIT_LIST> ::= <DIGIT>                    ; digit list of length 1
//                | <DIGIT_LIST><DIGIT>        ; digit list of length > 1
//
// <ITEM>       ::= 0..0 | 1..1                ; 0.., 1..0, ..1 are illegal
//
// <DIGIT>      ::= 0 | 1
//
// <EMPTY>      ::=                            ; ignore all whitespace
//
// Spec String          VALUE   Description
// -----------          -----   --------------------------------------------
// ""                       0   default is 0's
// "   "                    0   white space is ignored
// " 1 "                    1   one
// " 10"                    2   two
// "110"                    6   six
// "0..0"                   0   fill up with 0's
// "0..0110"                6   fill up with leading 0's
// "1..1"                  -1   fill up with 1's
// "1..10"                 -2   fill up with leading 1's
// "1 .\t. 1 0 1"          -3   white space is ignored
// "11..1"                 -1   1 followed by trailing 1's
// "01..1"            INT_MAX   0 followed by trailing 1's
// "10..0"            INT_MIN   1 followed by trailing 0's
//
// "a"                  error   bad character
// "0..1"               error   left and right fill value must match
// "..1"                error   missing left fill value
// "0.."                error   missing right fill value
// "1..11..1"           error   at most one fill item per spec
// "11111111..1111111"  error   if number of digits exceeds BITS_PER_WORD
//..
//-----------------------------------------------------------------------------
//                      Helper Functions for 'g'
//-----------------------------------------------------------------------------

inline
void setBits(uint32_t *integer, int mask, int booleanValue)
    // Set each bit in the specified 'integer' at positions corresponding
    // to '1'-bits in the specified 'mask' to the specified 'booleanValue'.
{
    if (booleanValue) {
        *integer |= mask;
    }
    else {
        *integer &= ~mask;
    }
}

inline
void setLSB(uint32_t *integer, const char *endOfSpec, int charCount)
    // Set the specified 'charCount' least significant bits in the specified
    // 'integer' to the bit pattern corresponding to '0' and '1' characters in
    // the 'charCount' characters *preceding* the specified 'endOfSpec',
    // leaving all other bits of 'integer' unaffected.  Note that
    // 'endOfSpec[-1]' corresponds to the least significant bit of 'integer'.
{
    const int start = -charCount;
    int       mask  = 1;
    for (int i = -1; i >= start; --i) {
        char ch = endOfSpec[i];
        switch (ch) {
          default: continue;
          case '0':
          case '1':
            setBits(integer, mask, '1' == ch);
            mask <<= 1;
        }
    }
}

inline
void setMSB(uint32_t *integer, const char *startOfSpec, int charCount)
    // Set the specified 'charCount' most significant bits in the specified
    // 'integer' to the bit pattern corresponding to '0' and '1' characters in
    // the 'charCount' characters starting at the specified 'startOfSpec',
    // leaving all other bits of 'integer' unaffected.  Note that endOfSpec[0]
    // corresponds to the most significant bit of 'integer'.
{
    unsigned int mask = (unsigned)(1 << (BITS_PER_WORD - 1));
    for (int i = 0; i < charCount; ++i) {
        char ch = startOfSpec[i];
        switch (ch) {
          default: continue;
          case '0':
          case '1':
            setBits(integer, mask, '1' == ch);
            mask >>= 1;
        }
    }
}

static int G_OFF = 0;  // set to 1 only to enable testing of G function errors

enum {
    G_ILLEGAL_CHARACTER     = 1001,
    G_MISMATCHED_RANGE      = 1002,
    G_MISSING_RANGE_END     = 1003,
    G_MISSING_RANGE_START   = 1004,
    G_MISSING_SECOND_DOT    = 1005,
    G_MULTIPLE_RANGES       = 1006,
    G_TOO_MANY_BITS         = 1007
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint32_t g(const char *spec)
    // Return an integer value corresponding the the specified 'spec' as
    // defined above using none of the functions defined in the component
    // under test.
{
    int bitCount = 0;           // total number of bits encountered
    int lastBitIndex = -1;      // index of last bit encountered

    int rangeStartIndex = -1;   // index of  first D in D..D
    int rangeEndIndex = -1;     // index of second D in D..D

    int i;                      // indicates length of spec after loop
    for (i = 0; spec[i]; ++i) {
        switch (spec[i]) {
          case '0':
          case '1': {
            ++bitCount;
            lastBitIndex = i;
          } break;
          case '.': {
            if (-1 != rangeStartIndex) {
                LOOP2_ASSERT(i, spec[i], G_OFF || !"Multiple Ranges");
                return G_MULTIPLE_RANGES;                             // RETURN
            }
            if (0 == bitCount) {
                LOOP2_ASSERT(i, spec[i], G_OFF || !"Missing Range Start");
                return G_MISSING_RANGE_START;                         // RETURN
            }
            while (isspace(spec[++i])) {
                // skip white space
            }
            if ('.' != spec[i]) {
                LOOP2_ASSERT(i, spec[i], G_OFF || !"Missing Second Dot");
                return G_MISSING_SECOND_DOT;                          // RETURN
            }
            while (isspace(spec[++i])) {
                // skip white space
            }
            if ('0' != spec[i] && '1' != spec[i]) {
                LOOP2_ASSERT(i, spec[i], G_OFF || !"Missing Range End");
                return G_MISSING_RANGE_END;                           // RETURN
            }
            if (spec[i] != spec[lastBitIndex]) {
                LOOP2_ASSERT(i, spec[i], G_OFF || !"Nonmatching Range");
                return G_MISMATCHED_RANGE;                            // RETURN
            }

            // Found valid range; record index of beginning and of end.
            rangeStartIndex = lastBitIndex;
            rangeEndIndex = i;
            --bitCount;
          } break;
          default: {
            if (!isspace(spec[i])) {
                LOOP2_ASSERT(i, spec[i], G_OFF || !"Illegal Character");
                return G_ILLEGAL_CHARACTER;                           // RETURN
            }
          } break;
        }
    }

    if (bitCount > BITS_PER_WORD) {
        LOOP2_ASSERT(bitCount, BITS_PER_WORD, G_OFF || !"Too Many Bits");
        return G_TOO_MANY_BITS;                                       // RETURN
    }

    uint32_t result;     // value to be returned

    if (-1 != rangeStartIndex) {
        result = '1' == spec[rangeStartIndex] ? ~0 : 0;
        setMSB(&result, spec, rangeStartIndex);
        setLSB(&result, spec + i, i - 1 - rangeEndIndex);
    }
    else {
        result = 0;
        setLSB(&result, spec + i, i);
    }

    return result;
}

//
//-----------------------------------------------------------------------------
//                      Helper Functions for 'g64'
//-----------------------------------------------------------------------------
inline
void setBits64(uint64_t *integer, uint64_t mask, int booleanValue)
    // Set each bit in the specified 'integer' at positions corresponding
    // to '1'-bits in the specified 'mask' to the specified 'booleanValue'.
{
    if (booleanValue) {
        *integer |= mask;
    }
    else {
        *integer &= ~mask;
    }
}

inline
void setLSB64(uint64_t *integer, const char *endOfSpec, int charCount)
    // Set the specified 'charCount' least significant bits in the specified
    // 'integer' to the bit pattern corresponding to '0' and '1' characters in
    // the 'charCount' characters *preceding* the specified 'endOfSpec',
    // leaving all other bits of 'integer' unaffected.  Note that
    // 'endOfSpec[-1]' corresponds to the least significant bit of 'integer'.
{
    const int start = -charCount;
    uint64_t  mask  = 1;
    for (int i = -1; i >= start; --i) {
        char ch = endOfSpec[i];
        switch (ch) {
          default: continue;
          case '0':
          case '1':
            setBits64(integer, mask, '1' == ch);
            mask <<= 1;
        }
    }
}

inline
void setMSB64(uint64_t *integer, const char *startOfSpec, int charCount)
    // Set the specified 'charCount' most significant bits in the specified
    // 'integer' to the bit pattern corresponding to '0' and '1' characters in
    // the 'charCount' characters starting at the specified 'startOfSpec',
    // leaving all other bits of 'integer' unaffected.  Note that endOfSpec[0]
    // corresponds to the most significant bit of 'integer'.
{
    uint64_t mask = ((uint64_t) 1 << (BITS_PER_UINT64 - 1));
    for (int i = 0; i < charCount; ++i) {
        char ch = startOfSpec[i];
        switch (ch) {
          default: continue;
          case '0':
          case '1':
            setBits64(integer, mask, '1' == ch);
            mask >>= 1;
        }
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint64_t g64(const char *spec)
    // Return an integer value corresponding the the specified 'spec' as
    // defined above using none of the functions defined in the component
    // under test.
{
    int bitCount = 0;           // total number of bits encountered
    int lastBitIndex = -1;      // index of last bit encountered

    int rangeStartIndex = -1;   // index of  first D in D..D
    int rangeEndIndex = -1;     // index of second D in D..D

    int i;                      // indicates length of spec after loop
    for (i = 0; spec[i]; ++i) {
        switch (spec[i]) {
          case '0':
          case '1': {
            ++bitCount;
            lastBitIndex = i;
          } break;
          case '.': {
            if (-1 != rangeStartIndex) {
                LOOP2_ASSERT(i, spec[i], G_OFF || !"Multiple Ranges");
                return G_MULTIPLE_RANGES;                             // RETURN
            }
            if (0 == bitCount) {
                LOOP2_ASSERT(i, spec[i], G_OFF || !"Missing Range Start");
                return G_MISSING_RANGE_START;                         // RETURN
            }
            while (isspace(spec[++i])) {
                // skip white space
            }
            if ('.' != spec[i]) {
                LOOP2_ASSERT(i, spec[i], G_OFF || !"Missing Second Dot");
                return G_MISSING_SECOND_DOT;                          // RETURN
            }
            while (isspace(spec[++i])) {
                // skip white space
            }
            if ('0' != spec[i] && '1' != spec[i]) {
                LOOP2_ASSERT(i, spec[i], G_OFF || !"Missing Range End");
                return G_MISSING_RANGE_END;                           // RETURN
            }
            if (spec[i] != spec[lastBitIndex]) {
                LOOP2_ASSERT(i, spec[i], G_OFF || !"Nonmatching Range");
                return G_MISMATCHED_RANGE;                            // RETURN
            }

            // Found valid range; record index of beginning and of end.
            rangeStartIndex = lastBitIndex;
            rangeEndIndex = i;
            --bitCount;
          } break;
          default: {
            if (!isspace(spec[i])) {
                LOOP2_ASSERT(i, spec[i], G_OFF || !"Illegal Character");
                return G_ILLEGAL_CHARACTER;                           // RETURN
            }
          } break;
        }
    }

    if (bitCount > BITS_PER_UINT64) {
        LOOP2_ASSERT(bitCount, BITS_PER_UINT64, G_OFF || !"Too Many Bits");
        return G_TOO_MANY_BITS;                                       // RETURN
    }

    uint64_t result;     // value to be returned

    if (-1 != rangeStartIndex) {
        result = '1' == spec[rangeStartIndex] ? ~ (uint64_t) 0 : (uint64_t) 0;
        setMSB64(&result, spec, rangeStartIndex);
        setLSB64(&result, spec + i, i - 1 - rangeEndIndex);
    }
    else {
        result = 0;
        setLSB64(&result, spec + i, i);
    }

    return result;
}

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;
    verbose = argc > 2;
    veryVerbose = argc > 3;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;

    switch (test) { case 0:  // Zero is always the leading case.
      case 6: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE
        //   Extracted from component header file.
        //
        // Concerns:
        //: 1 The usage example provided in the component header file compiles,
        //:   links, and runs as shown.
        //
        // Plan:
        //: 1 Incorporate usage example from header into test driver, replace
        //:   leading comment characters with spaces, replace 'assert' with
        //:   'ASSERT', and insert 'if (veryVerbose)' before all output
        //:   operations.  (C-1)
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) cout << "USAGE EXAMPLE\n"
                             "=============\n";

// Note that, in all of these examples, the low-order bit is considered bit '0'
// and resides on the right edge of the bit string.
//
// 'ge' will return a bit mask with all bits below the specified 'index'
// cleared, and all bits at or above the 'index' set:
//..
    ASSERT((uint32_t) 0xffff0000 == bdlb::BitMaskUtil::ge(16));
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
    ASSERT((uint32_t) 0x0000ffff == bdlb::BitMaskUtil::lt(16));
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
    ASSERT((uint32_t) 0x00800000 == bdlb::BitMaskUtil::eq(23));
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
    ASSERT((uint32_t) 0xfffeffff == bdlb::BitMaskUtil::ne(16));
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
    ASSERT((uint32_t) 0x000f0000 == bdlb::BitMaskUtil::one(16, 4));
//
//  +-------------------------------------------------------------------------+
//  | 'bdlb::BitMaskUtil::one(16, 4)' in binary:                              |
//  |                                                                         |
//  | bit 16:                                               *                 |
//  | 4 bits starting at bit 16:                         ****                 |
//  | Result: only those bits set:           00000000000011110000000000000000 |
//  +-------------------------------------------------------------------------+
//
    ASSERT((uint32_t) 0xfff0ffff == bdlb::BitMaskUtil::zero(16, 4));
//
//  +-------------------------------------------------------------------------+
//  | 'bdlb::BitMaskUtil::zero(16, 4)' in binary:                             |
//  |                                                                         |
//  | bit 16:                                               *                 |
//  | 4 bits starting at bit 16:                         ****                 |
//  | Result: only those bits cleared:       11111111111100001111111111111111 |
//  +-------------------------------------------------------------------------+
//
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // TESTING ONE AND ZERO FUNCTIONS
        //   These four functions are implemented using a straightforward
        //   bitwise computation involving only already-tested methods, and so
        //   this test needs only to probe the logic of the computation.  A
        //   small number of test inputs is sufficient.
        //
        // Plan:
        //   For each of an enumerated sequence of individual tests ordered
        //   by initial input, verify that each function returns the expected
        //   value.
        //
        //   After table-driven testing, loops doing exhaustive testing are
        //   are done.
        //
        // Testing:
        //   uint32_t one(int index, int nBits);
        //   uint64_t one64(int index, int nBITS);
        //   uint32_t zero(int index, int nBits);
        //   uint64_t zero64(int index, int nBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING ONE AND ZERO FUNCTIONS" << endl
                          << "==============================" << endl;

        static const struct {
            int      d_lineNum;     // line number
            int      d_index;       // index
            int      d_numBits;     // number of bits
            uint32_t d_mask0;       // expected result from mask0 function
            uint32_t d_mask1;       // expected result from mask1 function
        } DATA_A[] = {
          //L#  Index NumBits mask0 result    mask1 result
          //--  ----- ------- ------------    ------------
          { L_,    0,    0,          ~zero,           zero }, // index 0
          { L_,    0,    1,           ~one,            one },
          { L_,    0,BPW-1, (uint32_t)INT_MIN, (uint32_t) INT_MAX },
          { L_,    0,  BPW,           zero,          ~zero },

          { L_,    1,    0,          ~zero,           zero }, // index 1
          { L_,    1,    1,           ~two,            two },
          { L_,    1,BPW-1,            one,           ~one },

          { L_,BPW-1,    0,          ~zero,           zero }, // BPW-1
          { L_,BPW-1,    1, (uint32_t) INT_MAX, (uint32_t) INT_MIN },

          { L_,  BPW,    0,          ~zero,           zero }, // BPW

          { L_,    2,    4,g("1..1000011"),g("0..0111100") }, // typical case
          { L_,BPW-5,    3, g("110001..1"), g("001110..0") }
        };
        const int NUM_DATA_A = sizeof DATA_A / sizeof *DATA_A;

        for (int di = 0; di < NUM_DATA_A; ++di) {
            const int LINE     = DATA_A[di].d_lineNum;
            const int INDEX    = DATA_A[di].d_index;
            const int NUM_BITS = DATA_A[di].d_numBits;

            uint32_t resA = Util::zero(INDEX, NUM_BITS);
            if (veryVerbose) {
                P_(LINE);
                P_(INDEX);
                P_(NUM_BITS);
                P_(DATA_A[di].d_mask0);
                P(resA);
            }
            LOOP_ASSERT(LINE, DATA_A[di].d_mask0 == resA);

            resA = Util::one(INDEX, NUM_BITS);
            if (veryVerbose) {
                P_(LINE);
                P_(INDEX);
                P_(NUM_BITS);
                P_(DATA_A[di].d_mask1);
                P(resA);
            }
            LOOP_ASSERT(LINE, DATA_A[di].d_mask1 == resA);
        }

        for (int iNB = 0; iNB <= BPW; ++iNB) {
            uint32_t nbBits = Util::lt(iNB);
            for (int iIdx = 0; iIdx < BPW; ++iIdx) {
                ASSERTV(iNB, iIdx, (nbBits << iIdx) == Util::one(iIdx, iNB));
                ASSERTV(iNB, iIdx, (nbBits << iIdx) == ~Util::zero(iIdx, iNB));
            }
            ASSERTV(zero == Util::one(BPW, iNB));
            ASSERTV(zero == Util::one(BPW + 100, iNB));

            ASSERTV(zero == Util::one(BPW, iNB + 100));
            ASSERTV(zero == Util::one(BPW + 100, iNB + 100));

            ASSERTV(~zero == Util::zero(BPW, iNB));
            ASSERTV(~zero == Util::zero(BPW + 100, iNB));

            ASSERTV(~zero == Util::zero(BPW, iNB + 100));
            ASSERTV(~zero == Util::zero(BPW + 100, iNB + 100));
        }

        if (verbose) cout << endl
             << "Testing 'maskZero64' and 'maskOne64' Function" << endl
             << "=============================================" << endl;

        static const struct {
            int      d_lineNum;     // line number
            int      d_index;       // index
            int      d_numBits;     // number of bits
            uint64_t d_mask0;       // expected result from mask0 function
            uint64_t d_mask1;       // expected result from mask1 function
        } DATA_B[] = {
          //L#  Index NumBits      mask0 result    mask1 result
          //--  ----- -------      ------------    ------------
          { L_,    0,      0,           ~zero64,              0 }, // index 0
          { L_,    0,      1,            ~one64,              1 },
          { L_,    0,  BPS-1,          int64Min,       int64Max },
          { L_,    0,    BPS,                 0,        ~zero64 },

          { L_,    1,      0,           ~zero64,              0 }, // index 1
          { L_,    1,      1,            ~two64,              2 },
          { L_,    1,  BPS-1,                 1,         ~one64 },

          { L_,BPS-1,      0,           ~zero64,              0 }, // BPS-1
          { L_,BPS-1,      1,          int64Max,       int64Min },

          { L_,BPS,        0,           ~zero64,              0 }, // BPW

          { L_,    2,      4,  g64("1..1000011"),  g64("0..0111100") },
          { L_,BPS-5,      3,  g64("110001..1"),   g64("001110..0") }
        };
        const int NUM_DATA_B = sizeof DATA_B / sizeof *DATA_B;

        for (int di = 0; di < NUM_DATA_B; ++di) {
            const int LINE     = DATA_B[di].d_lineNum;
            const int INDEX    = DATA_B[di].d_index;
            const int NUM_BITS = DATA_B[di].d_numBits;

            uint64_t resA = Util::zero64(INDEX, NUM_BITS);
            if (veryVerbose) {
                P_(LINE);
                P_(INDEX);
                P_(NUM_BITS);
                P_(DATA_B[di].d_mask0);
                P(resA);
            }
            LOOP_ASSERT(LINE, DATA_B[di].d_mask0 == resA);

            resA = Util::one64(INDEX, NUM_BITS);
            if (veryVerbose) {
                P_(LINE);
                P_(INDEX);
                P_(NUM_BITS);
                P_(DATA_B[di].d_mask1);
                P(resA);
            }
            LOOP_ASSERT(LINE, DATA_B[di].d_mask1 == resA);
        }

        for (int iNB = 0; iNB <= BPS; ++iNB) {
            uint64_t nbBits = Util::lt64(iNB);
            for (int iIdx = 0; iIdx < BPS; ++iIdx) {
                ASSERTV(iNB, iIdx, (nbBits << iIdx) == Util::one64(iIdx, iNB));
                ASSERTV(iNB, iIdx, (nbBits << iIdx) ==
                                                     ~Util::zero64(iIdx, iNB));
            }
            ASSERTV(zero64 == Util::one64(BPS, iNB));
            ASSERTV(zero64 == Util::one64(BPS + 100, iNB));

            ASSERTV(zero64 == Util::one64(BPS, iNB + 100));
            ASSERTV(zero64 == Util::one64(BPS + 100, iNB + 100));

            ASSERTV(~zero64 == Util::zero64(BPS, iNB));
            ASSERTV(~zero64 == Util::zero64(BPS + 100, iNB));

            ASSERTV(~zero64 == Util::zero64(BPS, iNB + 100));
            ASSERTV(~zero64 == Util::zero64(BPS + 100, iNB + 100));
        }
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // TESTING ONE ARG MASK GENERATION FUNCTIONS:
        //
        // Concerns:
        //   That all of the one-arg mask functions work as specced.
        //
        // Plan:
        //   For each of the six single-argument mask generation functions,
        //   verify (in a loop over all valid 'index' values) that each valid
        //   'index' results in the desired mask value.  As a special case for
        //   each mask, verify that index == BITS_PER_WORD produces either 0x0
        //   or ~0 as appropriate.  Note that once a function has been tested,
        //   it may be used as an oracle for testing other functions.
        //
        // Testing:
        //   uint32_t eq(int index);
        //   uint64_t eq64(int index);
        //   uint32_t ne(int index);
        //   uint64_t ne64(int index);
        //   uint32_t ge(int index);
        //   uint64_t ge64(int index);
        //   uint32_t gt(int index);
        //   uint64_t gt64(int index);
        //   uint32_t le(int index);
        //   uint64_t le64(int index);
        //   uint32_t lt(int index);
        //   uint64_t lt64(int index);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING ONE ARG MASK GENERATION FUNCTIONS\n"
                          << "=========================================\n";

        int i, j;  // This keeps MS compiler happy w/o setting flags.

        {
            bsls::AssertTestHandlerGuard guard;

            ASSERT_SAFE_PASS(Util::eq(0));
            ASSERT_SAFE_PASS(Util::eq(BITS_PER_WORD - 1));
            ASSERT_SAFE_PASS(Util::eq(BITS_PER_WORD));
            ASSERT_SAFE_PASS(Util::eq(BITS_PER_WORD + 1000));
            ASSERT_SAFE_FAIL(Util::eq(-1));

            ASSERT_SAFE_PASS(Util::ne(0));
            ASSERT_SAFE_PASS(Util::ne(BITS_PER_WORD - 1));
            ASSERT_SAFE_PASS(Util::ne(BITS_PER_WORD));
            ASSERT_SAFE_PASS(Util::ne(BITS_PER_WORD + 1000));
            ASSERT_SAFE_FAIL(Util::ne(-1));

            ASSERT_SAFE_PASS(Util::ge(0));
            ASSERT_SAFE_PASS(Util::ge(BITS_PER_WORD - 1));
            ASSERT_SAFE_PASS(Util::ge(BITS_PER_WORD));
            ASSERT_SAFE_PASS(Util::ge(BITS_PER_WORD + 1000));
            ASSERT_SAFE_FAIL(Util::ge(-1));

            ASSERT_SAFE_PASS(Util::gt(0));
            ASSERT_SAFE_PASS(Util::gt(BITS_PER_WORD - 11));
            ASSERT_SAFE_PASS(Util::gt(BITS_PER_WORD));
            ASSERT_SAFE_PASS(Util::gt(BITS_PER_WORD + 1000));
            ASSERT_SAFE_FAIL(Util::gt(-1));

            ASSERT_SAFE_PASS(Util::le(0));
            ASSERT_SAFE_PASS(Util::le(BITS_PER_WORD - 1));
            ASSERT_SAFE_PASS(Util::le(BITS_PER_WORD));
            ASSERT_SAFE_PASS(Util::le(BITS_PER_WORD + 1000));
            ASSERT_SAFE_FAIL(Util::le(-1));

            ASSERT_SAFE_PASS(Util::lt(0));
            ASSERT_SAFE_PASS(Util::lt(BITS_PER_WORD - 1));
            ASSERT_SAFE_PASS(Util::lt(BITS_PER_WORD));
            ASSERT_SAFE_PASS(Util::lt(BITS_PER_WORD + 1000));
            ASSERT_SAFE_FAIL(Util::lt(-1));

            ASSERT_SAFE_PASS(Util::one(0, 0));
            ASSERT_SAFE_PASS(Util::one(0, BITS_PER_WORD - 1));
            ASSERT_SAFE_PASS(Util::one(0, BITS_PER_WORD));
            ASSERT_SAFE_PASS(Util::one(0, BITS_PER_WORD + 1000));
            ASSERT_SAFE_PASS(Util::one(BITS_PER_WORD, 0));
            ASSERT_SAFE_PASS(Util::one(BITS_PER_WORD, BITS_PER_WORD));
            ASSERT_SAFE_PASS(Util::one(BITS_PER_WORD, BITS_PER_WORD + 1000));
            ASSERT_SAFE_PASS(Util::one(BITS_PER_WORD+1000, 0));
            ASSERT_SAFE_PASS(Util::one(BITS_PER_WORD+1000, BITS_PER_WORD));
            ASSERT_SAFE_PASS(Util::one(BITS_PER_WORD+1000,
                                                        BITS_PER_WORD + 1000));
            ASSERT_SAFE_FAIL(Util::one(-1, 0));
            ASSERT_SAFE_FAIL(Util::one(-1, BITS_PER_WORD - 1));
            ASSERT_SAFE_FAIL(Util::one(-1, BITS_PER_WORD));
            ASSERT_SAFE_FAIL(Util::one(0, -1));
            ASSERT_SAFE_FAIL(Util::one(BITS_PER_WORD, -1));

            ASSERT_SAFE_PASS(Util::zero(0, 0));
            ASSERT_SAFE_PASS(Util::zero(0, BITS_PER_WORD - 1));
            ASSERT_SAFE_PASS(Util::zero(0, BITS_PER_WORD));
            ASSERT_SAFE_PASS(Util::zero(0, BITS_PER_WORD + 1000));
            ASSERT_SAFE_PASS(Util::zero(BITS_PER_WORD, 0));
            ASSERT_SAFE_PASS(Util::zero(BITS_PER_WORD, BITS_PER_WORD));
            ASSERT_SAFE_PASS(Util::zero(BITS_PER_WORD, BITS_PER_WORD + 1000));
            ASSERT_SAFE_PASS(Util::zero(BITS_PER_WORD+1000, 0));
            ASSERT_SAFE_PASS(Util::zero(BITS_PER_WORD+1000, BITS_PER_WORD));
            ASSERT_SAFE_PASS(Util::zero(BITS_PER_WORD+1000,
                                                        BITS_PER_WORD + 1000));
            ASSERT_SAFE_FAIL(Util::zero(-1, 0));
            ASSERT_SAFE_FAIL(Util::zero(-1, BITS_PER_WORD - 1));
            ASSERT_SAFE_FAIL(Util::zero(-1, BITS_PER_WORD));
            ASSERT_SAFE_FAIL(Util::zero(0, -1));
            ASSERT_SAFE_FAIL(Util::zero(BITS_PER_WORD, -1));

            ASSERT_SAFE_PASS(Util::eq64(0));
            ASSERT_SAFE_PASS(Util::eq64(BITS_PER_UINT64 - 1));
            ASSERT_SAFE_PASS(Util::eq64(BITS_PER_UINT64));
            ASSERT_SAFE_PASS(Util::eq64(BITS_PER_UINT64 + 1000));
            ASSERT_SAFE_FAIL(Util::eq64(-1));

            ASSERT_SAFE_PASS(Util::ne64(0));
            ASSERT_SAFE_PASS(Util::ne64(BITS_PER_UINT64 - 1));
            ASSERT_SAFE_PASS(Util::ne64(BITS_PER_UINT64));
            ASSERT_SAFE_PASS(Util::ne64(BITS_PER_UINT64 + 1000));
            ASSERT_SAFE_FAIL(Util::ne64(-1));

            ASSERT_SAFE_PASS(Util::ge64(0));
            ASSERT_SAFE_PASS(Util::ge64(BITS_PER_UINT64 - 1));
            ASSERT_SAFE_PASS(Util::ge64(BITS_PER_UINT64));
            ASSERT_SAFE_PASS(Util::ge64(BITS_PER_UINT64 + 1000));
            ASSERT_SAFE_FAIL(Util::ge64(-1));

            ASSERT_SAFE_PASS(Util::gt64(0));
            ASSERT_SAFE_PASS(Util::gt64(BITS_PER_UINT64 - 1));
            ASSERT_SAFE_PASS(Util::gt64(BITS_PER_UINT64));
            ASSERT_SAFE_PASS(Util::gt64(BITS_PER_UINT64 + 1000));
            ASSERT_SAFE_FAIL(Util::gt64(-1));

            ASSERT_SAFE_PASS(Util::le64(0));
            ASSERT_SAFE_PASS(Util::le64(BITS_PER_UINT64 - 1));
            ASSERT_SAFE_PASS(Util::le64(BITS_PER_UINT64));
            ASSERT_SAFE_PASS(Util::le64(BITS_PER_UINT64 + 1000));
            ASSERT_SAFE_FAIL(Util::le64(-1));

            ASSERT_SAFE_PASS(Util::lt64(0));
            ASSERT_SAFE_PASS(Util::lt64(BITS_PER_UINT64 - 1));
            ASSERT_SAFE_PASS(Util::lt64(BITS_PER_UINT64));
            ASSERT_SAFE_PASS(Util::lt64(BITS_PER_UINT64 + 1000));
            ASSERT_SAFE_FAIL(Util::lt64(-1));

            ASSERT_SAFE_PASS(Util::one64(0, 0));
            ASSERT_SAFE_PASS(Util::one64(0, BITS_PER_UINT64 - 1));
            ASSERT_SAFE_PASS(Util::one64(0, BITS_PER_UINT64));
            ASSERT_SAFE_PASS(Util::one64(0, BITS_PER_UINT64 + 1000));
            ASSERT_SAFE_PASS(Util::one64(BITS_PER_UINT64, 0));
            ASSERT_SAFE_PASS(Util::one64(BITS_PER_UINT64, BITS_PER_UINT64));
            ASSERT_SAFE_PASS(Util::one64(BITS_PER_UINT64,
                                                      BITS_PER_UINT64 + 1000));
            ASSERT_SAFE_PASS(Util::one64(BITS_PER_UINT64+1000, 0));
            ASSERT_SAFE_PASS(Util::one64(BITS_PER_UINT64+1000,
                                                             BITS_PER_UINT64));
            ASSERT_SAFE_PASS(Util::one64(BITS_PER_UINT64+1000,
                                                      BITS_PER_UINT64 + 1000));
            ASSERT_SAFE_FAIL(Util::one64(-1, 0));
            ASSERT_SAFE_FAIL(Util::one64(-1, BITS_PER_UINT64 - 1));
            ASSERT_SAFE_FAIL(Util::one64(-1, BITS_PER_UINT64));
            ASSERT_SAFE_FAIL(Util::one64(0, -1));
            ASSERT_SAFE_FAIL(Util::one64(BITS_PER_UINT64, -1));

            ASSERT_SAFE_PASS(Util::zero64(0, 0));
            ASSERT_SAFE_PASS(Util::zero64(0, BITS_PER_UINT64 - 1));
            ASSERT_SAFE_PASS(Util::zero64(0, BITS_PER_UINT64));
            ASSERT_SAFE_PASS(Util::zero64(0, BITS_PER_UINT64 + 1000));
            ASSERT_SAFE_PASS(Util::zero64(BITS_PER_UINT64, 0));
            ASSERT_SAFE_PASS(Util::zero64(BITS_PER_UINT64, BITS_PER_UINT64));
            ASSERT_SAFE_PASS(Util::zero64(BITS_PER_UINT64,
                                                      BITS_PER_UINT64 + 1000));
            ASSERT_SAFE_PASS(Util::zero64(BITS_PER_UINT64+1000, 0));
            ASSERT_SAFE_PASS(Util::zero64(BITS_PER_UINT64+1000,
                                                             BITS_PER_UINT64));
            ASSERT_SAFE_PASS(Util::zero64(BITS_PER_UINT64+1000,
                                                      BITS_PER_UINT64 + 1000));
            ASSERT_SAFE_FAIL(Util::zero64(-1, 0));
            ASSERT_SAFE_FAIL(Util::zero64(-1, BITS_PER_UINT64 - 1));
            ASSERT_SAFE_FAIL(Util::zero64(-1, BITS_PER_UINT64));
            ASSERT_SAFE_FAIL(Util::zero64(0, -1));
            ASSERT_SAFE_FAIL(Util::zero64(BITS_PER_UINT64, -1));
        }

        {
            if (verbose) cout << "Table-driven test of 'eq' and 'ne'\n";

            struct {
                int      d_line;
                int      d_index;
                uint32_t d_expectedEq;
            } DATA[] = {
                { L_, 0,     1 },
                { L_, 1,     2 },
                { L_, 2,     4 },
                { L_, 16,    CAT(0x1,    0000) },
                { L_, BPW-1, CAT(0x8000, 0000) },
                { L_, BPW,   0 },
                { L_, BPW+1, 0 },
                { L_, BPW+1000, 0 },
            };
            enum { NUM_DATA = sizeof DATA / sizeof *DATA };

            for (int ti = 0; ti < NUM_DATA; ++ti) {
                const int      LINE   = DATA[ti].d_line;
                const int      INDEX  = DATA[ti].d_index;
                const uint32_t EXP_EQ = DATA[ti].d_expectedEq;
                const uint32_t EXP_NE = ~EXP_EQ;

                const uint32_t eq = Util::eq(INDEX);
                const uint32_t ne = Util::ne(INDEX);
                ASSERTV(LINE, INDEX, eq, EXP_EQ == eq);
                ASSERTV(LINE, INDEX, ne, EXP_NE == ne);
            }
        }

        {
            if (verbose) cout << "Table-driven test of 'ge' and 'lt'\n";

            struct {
                int      d_line;
                int      d_index;
                uint32_t d_expectedGe;
            } DATA[] = {
                { L_, 0,     CAT(0xffff, ffff) },
                { L_, 1,     CAT(0xffff, fffe) },
                { L_, 2,     CAT(0xffff, fffc) },
                { L_, 16,    CAT(0xffff, 0000) },
                { L_, BPW-1, CAT(0x8000, 0000) },
                { L_, BPW,   0 },
                { L_, BPW+1, 0 },
                { L_, BPW+1000, 0 },
            };
            enum { NUM_DATA = sizeof DATA / sizeof *DATA };

            for (int ti = 0; ti < NUM_DATA; ++ti) {
                const int      LINE   = DATA[ti].d_line;
                const int      INDEX  = DATA[ti].d_index;
                const uint32_t EXP_GE = DATA[ti].d_expectedGe;
                const uint32_t EXP_LT = ~EXP_GE;

                const uint32_t ge = Util::ge(INDEX);
                const uint32_t lt = Util::lt(INDEX);
                ASSERTV(LINE, INDEX, ge, EXP_GE == ge);
                ASSERTV(LINE, INDEX, lt, EXP_LT == lt);
            }
        }

        {
            if (verbose) cout << "Table-driven test of 'gt' and 'le'\n";

            struct {
                int      d_line;
                int      d_index;
                uint32_t d_expectedGt;
            } DATA[] = {
                { L_, 0,     CAT(0xffff, fffe) },
                { L_, 1,     CAT(0xffff, fffc) },
                { L_, 2,     CAT(0xffff, fff8) },
                { L_, 16,    CAT(0xfffe, 0000) },
                { L_, BPW-1, CAT(0x0000, 0000) },
                { L_, BPW,   0 },
                { L_, BPW+1, 0 },
                { L_, BPW+1000, 0 },
            };
            enum { NUM_DATA = sizeof DATA / sizeof *DATA };

            for (int ti = 0; ti < NUM_DATA; ++ti) {
                const int      LINE   = DATA[ti].d_line;
                const int      INDEX  = DATA[ti].d_index;
                const uint32_t EXP_GT = DATA[ti].d_expectedGt;
                const uint32_t EXP_LE = ~EXP_GT;

                const uint32_t gt = Util::gt(INDEX);
                const uint32_t le = Util::le(INDEX);
                ASSERTV(LINE, INDEX, gt, EXP_GT == gt);
                ASSERTV(LINE, INDEX, le, EXP_LE == le);

                ASSERTV(LINE, INDEX, le == (Util::lt(INDEX+1)));
                if (INDEX > 0) {
                    ASSERTV(LINE, INDEX, gt == Util::ge(INDEX+1));
                }
            }
        }

        if (verbose) cout << "Testing 'eq'" << endl;
        for (i = 0; i < BITS_PER_WORD; ++i) {
            LOOP_ASSERT(i, one << i == Util::eq(i));
        }
        ASSERT(zero == Util::eq(BITS_PER_WORD));
        ASSERT(zero == Util::eq(BITS_PER_WORD + 1000));

        if (verbose) cout << "Testing 'ne'" << endl;
        for (i = 0; i < (int) BITS_PER_WORD; ++i) {
            LOOP_ASSERT(i, ~(one << i) == Util::ne(i));
        }
        ASSERT(~zero == Util::ne(BITS_PER_WORD));
        ASSERT(~zero == Util::ne(BITS_PER_WORD + 1000));

        if (verbose) cout << "Testing 'ge'" << endl;
        for (i = 0; i < (int) BITS_PER_WORD; ++i) {
            LOOP_ASSERT(i, ~zero << i == Util::ge(i));
        }
        ASSERT(zero == Util::ge(BITS_PER_WORD));
        ASSERT(zero == Util::ge(BITS_PER_WORD + 1000));

        if (verbose) cout << "Testing 'gt'" << endl;
        for (i = 0; i < (int) BITS_PER_WORD - 1; ++i) {
            LOOP_ASSERT(i, ~zero << (i + 1) == Util::gt(i));
        }
        ASSERT(zero == Util::gt(BITS_PER_WORD - 1));
        ASSERT(zero == Util::gt(BITS_PER_WORD));
        ASSERT(zero == Util::gt(BITS_PER_WORD + 1000));

        if (verbose) cout << "Testing 'le'" << endl;
        for (i = 0; i < (int) BITS_PER_WORD - 1; ++i) {
            LOOP_ASSERT(i, (one << (i + 1)) - 1 == Util::le(i));
        }
        ASSERT(~zero == Util::le(BITS_PER_WORD - 1));
        ASSERT(~zero == Util::le(BITS_PER_WORD));
        ASSERT(~zero == Util::le(BITS_PER_WORD + 1000));

        if (verbose) cout << "Testing 'lt'" << endl;
        for (i = 0; i < (int) BITS_PER_WORD; ++i) {
            LOOP_ASSERT(i, (one << i) - 1 == Util::lt(i));
            ASSERT(Util::lt(i) == ~Util::ge(i));
        }
        ASSERT(~zero == Util::lt(BITS_PER_WORD));
        ASSERT(~zero == Util::lt(BITS_PER_WORD + 1000));

        if (verbose) cout << "Testing 'one' and 'zero'" << endl;
        for (i = 0; i < (int) BITS_PER_WORD; ++i) {
            for (j = 0; j < (int) BITS_PER_WORD - i; ++j) {
                const uint32_t expected = ((one << (i + j)) - 1) &
                                                             ~((one << i) - 1);
                LOOP2_ASSERT(i, j, expected ==  Util::one( i, j));
                LOOP2_ASSERT(i, j, expected == ~Util::zero(i, j));
            }
            LOOP2_ASSERT(i, j, Util::ge(i) == Util::one( i, j));
            LOOP2_ASSERT(i, j, Util::lt(i) == Util::zero(i, j));
            LOOP2_ASSERT(i, j, Util::ge(i) == Util::one( i, j + 1000));
            LOOP2_ASSERT(i, j, Util::lt(i) == Util::zero(i, j + 1000));
        }
        LOOP_ASSERT(i,  zero == Util::one( i, 0));
        LOOP_ASSERT(i, ~zero == Util::zero(i, 0));
        LOOP_ASSERT(i,  zero == Util::one( i + 1000, 0));
        LOOP_ASSERT(i, ~zero == Util::zero(i + 1000, 0));
        LOOP_ASSERT(i,  zero == Util::one( i, 1));
        LOOP_ASSERT(i, ~zero == Util::zero(i, 1));
        LOOP_ASSERT(i,  zero == Util::one( i + 1000, 1));
        LOOP_ASSERT(i, ~zero == Util::zero(i + 1000, 1));
        LOOP_ASSERT(i,  zero == Util::one( i, 1000));
        LOOP_ASSERT(i, ~zero == Util::zero(i, 1000));
        LOOP_ASSERT(i,  zero == Util::one( i + 1000, 1000));
        LOOP_ASSERT(i, ~zero == Util::zero(i + 1000, 1000));

        if (verbose) cout << "Testing relationships between mask functions\n";
        for (i = 0; i <= (int) BITS_PER_WORD; ++i) {
            LOOP_ASSERT(i, ~Util::ge(i) == Util::lt(i));
            LOOP_ASSERT(i, ~Util::le(i) == Util::gt(i));
            LOOP_ASSERT(i, ~Util::ne(i) == Util::eq(i));
        }

        {
            if (verbose) cout << "Table-driven test of 'eq64' and 'ne64'\n";

            struct {
                int      d_line;
                int      d_index;
                uint64_t d_expectedEq;
            } DATA[] = {
                { L_, 0,     1 },
                { L_, 1,     2 },
                { L_, 2,     4 },
                { L_, 16,    CAT4(0x0,       0, 1,    0000ULL) },
                { L_, 32,    CAT4(0x0,       1, 0000, 0000ULL) },
                { L_, 48,    CAT4(0x1,    0000, 0000, 0000ULL) },
                { L_, BPS-1, CAT4(0x8000, 0000, 0000, 0000ULL) },
                { L_, BPS,   0 },
                { L_, BPS+1, 0 },
                { L_, BPS+1000, 0 },
            };
            enum { NUM_DATA = sizeof DATA / sizeof *DATA };

            for (int ti = 0; ti < NUM_DATA; ++ti) {
                const int      LINE   = DATA[ti].d_line;
                const int      INDEX  = DATA[ti].d_index;
                const uint64_t EXP_EQ = DATA[ti].d_expectedEq;
                const uint64_t EXP_NE = ~EXP_EQ;

                const uint64_t eq = Util::eq64(INDEX);
                const uint64_t ne = Util::ne64(INDEX);
                ASSERTV(LINE, INDEX, eq, EXP_EQ == eq);
                ASSERTV(LINE, INDEX, ne, EXP_NE == ne);
            }
        }

        {
            if (verbose) cout << "Table-driven test of 'ge64' and 'lt64'\n";

            struct {
                int      d_line;
                int      d_index;
                uint64_t d_expectedGe;
            } DATA[] = {
                { L_, 0,     CAT4(0xffff, ffff, ffff, ffffULL) },
                { L_, 1,     CAT4(0xffff, ffff, ffff, fffeULL) },
                { L_, 2,     CAT4(0xffff, ffff, ffff, fffcULL) },
                { L_, 16,    CAT4(0xffff, ffff, ffff, 0000ULL) },
                { L_, 32,    CAT4(0xffff, ffff, 0000, 0000ULL) },
                { L_, 48,    CAT4(0xffff, 0000, 0000, 0000ULL) },
                { L_, BPS-1, CAT4(0x8000, 0000, 0000, 0000ULL) },
                { L_, BPS,   0 },
                { L_, BPS+1, 0 },
                { L_, BPS+1000, 0 },
            };
            enum { NUM_DATA = sizeof DATA / sizeof *DATA };

            for (int ti = 0; ti < NUM_DATA; ++ti) {
                const int      LINE   = DATA[ti].d_line;
                const int      INDEX  = DATA[ti].d_index;
                const uint64_t EXP_GE = DATA[ti].d_expectedGe;
                const uint64_t EXP_LT = ~EXP_GE;

                const uint64_t ge = Util::ge64(INDEX);
                const uint64_t lt = Util::lt64(INDEX);
                ASSERTV(LINE, INDEX, ge, EXP_GE == ge);
                ASSERTV(LINE, INDEX, lt, EXP_LT == lt);
            }
        }

        {
            if (verbose) cout << "Table-driven test of 'gt64' and 'le64'\n";

            struct {
                int      d_line;
                int      d_index;
                uint64_t d_expectedGt;
            } DATA[] = {
                { L_, 0,     CAT4(0xffff, ffff, ffff, fffeULL) },
                { L_, 1,     CAT4(0xffff, ffff, ffff, fffcULL) },
                { L_, 2,     CAT4(0xffff, ffff, ffff, fff8ULL) },
                { L_, 16,    CAT4(0xffff, ffff, fffe, 0000ULL) },
                { L_, 32,    CAT4(0xffff, fffe, 0000, 0000ULL) },
                { L_, 48,    CAT4(0xfffe, 0000, 0000, 0000ULL) },
                { L_, BPS-1, CAT4(0x0000, 0000, 0000, 0000ULL) },
                { L_, BPS,   0 },
                { L_, BPS+1, 0 },
                { L_, BPS+1000, 0 },
            };
            enum { NUM_DATA = sizeof DATA / sizeof *DATA };

            for (int ti = 0; ti < NUM_DATA; ++ti) {
                const int      LINE   = DATA[ti].d_line;
                const int      INDEX  = DATA[ti].d_index;
                const uint64_t EXP_GT = DATA[ti].d_expectedGt;
                const uint64_t EXP_LE = ~EXP_GT;

                const uint64_t gt = Util::gt64(INDEX);
                const uint64_t le = Util::le64(INDEX);
                ASSERTV(LINE, INDEX, gt, EXP_GT == gt);
                ASSERTV(LINE, INDEX, le, EXP_LE == le);

                ASSERTV(LINE, INDEX, le == Util::lt64(INDEX+1));
                ASSERTV(LINE, INDEX, gt == Util::ge64(INDEX+1));
            }
        }

        if (verbose) cout << "Testing 'eq64'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64; ++i) {
            LOOP_ASSERT(i, one64 << i == Util::eq64(i));
        }
        ASSERT(zero64 == Util::eq64(i));
        ASSERT(zero64 == Util::eq64(i + 1000));

        if (verbose) cout << "Testing 'ne64'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64; ++i) {
            LOOP_ASSERT(i, ~(one64 << i) == Util::ne64(i));
        }
        ASSERT(~zero64 == Util::ne64(i));
        ASSERT(~zero64 == Util::ne64(i + 1000));

        if (verbose) cout << "Testing 'ge64'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64; ++i) {
            LOOP_ASSERT(i, ~zero64 << i == Util::ge64(i));
        }
        ASSERT(zero64 == Util::ge64(i));
        ASSERT(zero64 == Util::ge64(i + 1000));

        if (verbose) cout << "Testing 'gt64'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64 - 1; ++i) {
            LOOP_ASSERT(i, ~zero64 << (i + 1) == Util::gt64(i));
        }
        ASSERT(zero64 == Util::gt64(BITS_PER_UINT64 - 1));
        ASSERT(zero64 == Util::gt64(BITS_PER_UINT64));
        ASSERT(zero64 == Util::gt64(BITS_PER_UINT64 + 1000));

        if (verbose) cout << "Testing 'le64'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64 - 1; ++i) {
            LOOP_ASSERT(i, (one64 << (i + 1)) - 1 == Util::le64(i));
        }
        ASSERT(~zero64 == Util::le64(BITS_PER_UINT64 - 1));
        ASSERT(~zero64 == Util::le64(BITS_PER_UINT64));
        ASSERT(~zero64 == Util::le64(BITS_PER_UINT64 + 1000));

        if (verbose) cout << "Testing 'lt64'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64; ++i) {
            LOOP_ASSERT(i, (one64 << i) - 1 == Util::lt64(i));
        }
        ASSERT(~zero64 == Util::lt64(i));
        ASSERT(~zero64 == Util::lt64(i + 1000));

        if (verbose) cout << "Testing 'one' and 'zero'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64; ++i) {
            for (j = 0; j < (int) BITS_PER_UINT64 - i; ++j) {
                const uint64_t expected = ((one64 << (i + j)) - 1) &
                                                           ~((one64 << i) - 1);
                LOOP2_ASSERT(i, j, expected ==  Util::one64( i, j));
                LOOP2_ASSERT(i, j, expected == ~Util::zero64(i, j));
            }
            LOOP2_ASSERT(i, j, Util::ge64(i) == Util::one64( i, j));
            LOOP2_ASSERT(i, j, Util::lt64(i) == Util::zero64(i, j));
            LOOP2_ASSERT(i, j, Util::ge64(i) == Util::one64( i, j + 1000));
            LOOP2_ASSERT(i, j, Util::lt64(i) == Util::zero64(i, j + 1000));
        }
        LOOP_ASSERT(i,  zero64 == Util::one64( i, 0));
        LOOP_ASSERT(i, ~zero64 == Util::zero64(i, 0));
        LOOP_ASSERT(i,  zero64 == Util::one64( i + 1000, 0));
        LOOP_ASSERT(i, ~zero64 == Util::zero64(i + 1000, 0));
        LOOP_ASSERT(i,  zero64 == Util::one64( i, 1));
        LOOP_ASSERT(i, ~zero64 == Util::zero64(i, 1));
        LOOP_ASSERT(i,  zero64 == Util::one64( i + 1000, 1));
        LOOP_ASSERT(i, ~zero64 == Util::zero64(i + 1000, 1));
        LOOP_ASSERT(i,  zero64 == Util::one64( i, 1000));
        LOOP_ASSERT(i, ~zero64 == Util::zero64(i, 1000));
        LOOP_ASSERT(i,  zero64 == Util::one64( i + 1000, 1000));
        LOOP_ASSERT(i, ~zero64 == Util::zero64(i + 1000, 1000));

        if (verbose) cout << "Testing relationships between mask functions\n";
        for (i = 0; i <= (int) BITS_PER_UINT64; ++i) {
            LOOP_ASSERT(i, ~Util::ge64(i) == Util::lt64(i));
            LOOP_ASSERT(i, ~Util::le64(i) == Util::gt64(i));
            LOOP_ASSERT(i, ~Util::ne64(i) == Util::eq64(i));
        }
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // TESTING ENUM TYPE VARIABLES
        //   Each 'enum' must have the correct value.  The value of
        //   'BITS_PER_WORD' is the result of a computation; the tests must be
        //   inspected carefully to avoid accidental duplicate mistakes in the
        //   test logic.
        //
        // Plan:
        //   Carefully define a set of 'const' local "helper" variables
        //   initialized to appropriate intermediate or final values.  Then,
        //   for each of the four 'enum' variables under test, use only the
        //   helper variables to verify that it the 'enum' holds the expected
        //   value.
        //
        // Testing:
        //   enum { WORD_SIZE = sizeof(int) };
        //   enum { BITS_PER_BYTE = 8 };
        //   enum { BITS_PER_WORD = BITS_PER_BYTE * WORD_SIZE };
        // --------------------------------------------------------------------

        if (verbose) cout << "TESTING ENUM TYPE VARIABLES\n"
                             "===========================\n";

        const int EIGHT = 8;           // one fact in one place for testing
        const int BPW   = EIGHT * sizeof(int);
        const int BPW64 = EIGHT * sizeof(uint64_t);

        if (veryVerbose) {
            T_  P(EIGHT);
            T_  P(BPW);  cout << endl;

            T_  P(CHAR_BIT);
            T_  P(Util::k_BITS_PER_UINT32);
            T_  P(Util::k_BITS_PER_UINT64);
        }
        ASSERT(EIGHT == CHAR_BIT);
        ASSERT(BPW   == Util::k_BITS_PER_UINT32);
        ASSERT(BPW64 == Util::k_BITS_PER_UINT64);
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING GENERATOR FUNCTIONS, g & g64
        //   'g' must correctly parse 'spec' according to its specific
        //   language, and return the corresponding integer if 'spec' is valid.
        //   'g' must also correctly diagnose and report an invalid 'spec'.
        //   'g's error-report-suppression flag must be set before testing
        //   invalid 'spec's, and must be explicitly unset after testing.
        //
        // Plan:
        //   For each 'spec' in a tabulated sequence, verify that 'g' returns
        //   the expected value.  First specify a sequence of valid 'spec'
        //   values, then temporarily disable 'g's error-reporting and provide
        //   a sequence of invalid 'spec' values.
        //
        // Testing:
        //   GENERATOR FUNCTION: uint32_t g(const char *spec)
        //   GENERATOR FUNCTION: uint64_t g64(const char *spec)
        // --------------------------------------------------------------------

        if (verbose) cout << "\n" "TESTING GENERATOR FUNCTIONS, g & g64\n"
                                  "====================================\n";

        if (verbose) cout << "\nVerify behavior of g() for valid input.\n";
        {
            static const struct {
                int         d_lineNum;  // line number
                const char *d_spec;     // input spec.
                uint32_t    d_value;    // resulting value
            } DATA[] = {
                //L#  Input Specification               Resulting Value
                //--  -------------------               ---------------

                { L_, "",                               0               },
                { L_, "0",                              0               },
                { L_, "1",                              1               },
                { L_, "00",                             0               },
                { L_, "01",                             1               },
                { L_, "10",                             2               },
                { L_, "11",                             3               },
                { L_, "000",                            0               },
                { L_, "001",                            1               },
                { L_, "010",                            2               },
                { L_, "011",                            3               },
                { L_, "100",                            4               },
                { L_, "101",                            5               },
                { L_, "110",                            6               },
                { L_, "111",                            7               },

                { L_, "0..0",                           0               },
                { L_, "00..0",                          0               },
                { L_, "0..00",                          0               },
                { L_, "10..0",                          (uint32_t) INT_MIN  },
                { L_, "0..01",                          1               },

                { L_, "1..1",                           ~zero           },
                { L_, "11..1",                          ~zero           },
                { L_, "1..11",                          ~zero           },
                { L_, "01..1",                          (uint32_t) INT_MAX },
                { L_, "1..10",                          ~zero ^ 1       },

                { L_, " ",                              g("")           },
                { L_, " 0 ",                            g("0")          },
                { L_, " 1 1 ",                          g("11")         },
                { L_, " 1 0 1 0 . . 0 0 1 0 ",          g("1010..0010") },
                { L_, " 1 0 1 0 . . 0 0 1 0 ",          g("1010..0010") },

                { L_, " 0 . . 0 ",                      g("0..0")       },
                { L_, " 1 0 . . 0 ",                    g("10..0")      },
                { L_, " 0 . . 0 1 ",                    g("0..01")      },

                { L_, " 1 . . 1 ",                      g("1..1")       },
                { L_, " 0 1 . . 1 ",                    g("01..1")      },
                { L_, " 1 . . 1 0",                     g("1..10")      },

                { L_, " 01 10 .. 01 01",                g("0110..0101") },
                { L_, "\t0110.\t.0101\t",               g("0110..0101") },
                { L_, "\t0110.\t.0101\t",               g("0110..0101") },

                { L_, " 10  11. .11 10",                g("1011..1110") },
                { L_, "1   011..11   10",               g("1011..1110") },
                { L_, " 1 0 \t 1 1 . \t . 1 1 \t 1 0 ", g("1011..1110") },
            };
            const int NUM_DATA = sizeof DATA / sizeof *DATA;

            for (int di = 0; di < NUM_DATA; ++di) {
                const int   LINE = DATA[di].d_lineNum;
                const char *spec = DATA[di].d_spec;
                uint32_t    exp  = DATA[di].d_value;
                uint32_t    res  = g(DATA[di].d_spec);

                if (veryVerbose) { P_(LINE); P_(spec); P_(exp); P(res); }
                LOOP_ASSERT(LINE, res == exp);
            }
        }

        if (verbose) cout << "\nVerify invalid input is detected by g().\n";
        {
            static const struct {
                int         d_lineNum;  // line number
                const char *d_spec;     // input spec.
                uint32_t    d_value;    // resulting error code
            } DATA[] = {
                //L#  Input Specification   Resulting Error Code
                //--  -------------------   --------------------
                { L_, "A",                  G_ILLEGAL_CHARACTER     },
                { L_, "2",                  G_ILLEGAL_CHARACTER     },
                { L_, ":",                  G_ILLEGAL_CHARACTER     },
                { L_, "z0",                 G_ILLEGAL_CHARACTER     },
                { L_, "09",                 G_ILLEGAL_CHARACTER     },
                { L_, "0_0",                G_ILLEGAL_CHARACTER     },

                { L_, "0..1",               G_MISMATCHED_RANGE      },
                { L_, "0..10",              G_MISMATCHED_RANGE      },
                { L_, "0..11",              G_MISMATCHED_RANGE      },
                { L_, "00..1",              G_MISMATCHED_RANGE      },
                { L_, "10..1",              G_MISMATCHED_RANGE      },

                { L_, "1..0",               G_MISMATCHED_RANGE      },
                { L_, "1..00",              G_MISMATCHED_RANGE      },
                { L_, "1..01",              G_MISMATCHED_RANGE      },
                { L_, "01..0",              G_MISMATCHED_RANGE      },
                { L_, "11..0",              G_MISMATCHED_RANGE      },

                { L_, "0..",                G_MISSING_RANGE_END     },
                { L_, "00..",               G_MISSING_RANGE_END     },
                { L_, "10..",               G_MISSING_RANGE_END     },

                { L_, "1..",                G_MISSING_RANGE_END     },
                { L_, "01..",               G_MISSING_RANGE_END     },
                { L_, "11..",               G_MISSING_RANGE_END     },

                { L_, "..0",                G_MISSING_RANGE_START   },
                { L_, "..00",               G_MISSING_RANGE_START   },
                { L_, "..01",               G_MISSING_RANGE_START   },

                { L_, "..1",                G_MISSING_RANGE_START   },
                { L_, "..10",               G_MISSING_RANGE_START   },
                { L_, "..11",               G_MISSING_RANGE_START   },

                { L_, "0.1",                G_MISSING_SECOND_DOT    },
                { L_, "0.10",               G_MISSING_SECOND_DOT    },
                { L_, "0.11",               G_MISSING_SECOND_DOT    },
                { L_, "00.1",               G_MISSING_SECOND_DOT    },
                { L_, "10.1",               G_MISSING_SECOND_DOT    },

                { L_, "1.0",                G_MISSING_SECOND_DOT    },
                { L_, "1.00",               G_MISSING_SECOND_DOT    },
                { L_, "1.01",               G_MISSING_SECOND_DOT    },
                { L_, "01.0",               G_MISSING_SECOND_DOT    },
                { L_, "11.0",               G_MISSING_SECOND_DOT    },

                { L_, "0..0.",              G_MULTIPLE_RANGES       },
                { L_, "0..00.",             G_MULTIPLE_RANGES       },
                { L_, "0..01.",             G_MULTIPLE_RANGES       },

                { L_, "1..1.",              G_MULTIPLE_RANGES       },
                { L_, "1..10.",             G_MULTIPLE_RANGES       },
                { L_, "1..11.",             G_MULTIPLE_RANGES       },

                { L_, FW_0,                 0                       },
                { L_, FW_0 "0",             G_TOO_MANY_BITS         },
                { L_, FW_0 "1",             G_TOO_MANY_BITS         },
                { L_, "0" FW_0,             G_TOO_MANY_BITS         },
                { L_, "1" FW_0,             G_TOO_MANY_BITS         },

                { L_, FW_1,                 -one                    },
                { L_, FW_1 "0",             G_TOO_MANY_BITS         },
                { L_, FW_1 "1",             G_TOO_MANY_BITS         },
                { L_, "0" FW_1,             G_TOO_MANY_BITS         },
                { L_, "1" FW_1,             G_TOO_MANY_BITS         },

                { L_, "1..1" FW_0,          0                       },
                { L_, HW_0 "1..1" HW_0,     0                       },
                { L_, FW_0 "1..1",          0                       },

                { L_, "1..1" FW_0 "0",      G_TOO_MANY_BITS         },
                { L_, HW_0 "1..1" HW_0 "0", G_TOO_MANY_BITS         },
                { L_, "0" FW_0 "1..1",      G_TOO_MANY_BITS         },

                { L_, "0..0" FW_1,          -one                    },
                { L_, HW_1 "0..0" HW_1,     -one                    },
                { L_, FW_1 "0..0",          -one                    },

                { L_, "0..0" FW_1 "0",      G_TOO_MANY_BITS         },
                { L_, HW_1 "0..0" HW_1"1",  G_TOO_MANY_BITS         },
                { L_, "1" FW_1 "0..0",      G_TOO_MANY_BITS         },
            };
            const int NUM_DATA = sizeof DATA / sizeof *DATA;

            G_OFF = 1;  // set to 1 to enable testing of G function errors

            for (int di = 0; di < NUM_DATA ; ++di) {
                const int   LINE = DATA[di].d_lineNum;
                const char *spec = DATA[di].d_spec;
                uint32_t    exp  = DATA[di].d_value;
                uint32_t    res  = g(DATA[di].d_spec);

                if (veryVerbose) { P_(LINE); P_(spec); P_(exp); P(res); }

                LOOP_ASSERT(LINE, res == exp);
            }

            G_OFF = 0;  // set to 1 to enable testing of G function errors
        }

        if (verbose) cout <<
                    "\nVerify behavior of the g64 function for valid input.\n";
        {
            static const struct {
                int         d_lineNum;  // line number
                const char *d_spec;     // input spec.
                uint64_t    d_value;    // resulting value
            } DATA[] = {
                //L#  Input Specification               Resulting Value
                //--  -------------------               ---------------

                { L_, "",                               0               },
                { L_, "0",                              0               },
                { L_, "1",                              1               },
                { L_, "00",                             0               },
                { L_, "01",                             1               },
                { L_, "10",                             2               },
                { L_, "11",                             3               },
                { L_, "000",                            0               },
                { L_, "001",                            1               },
                { L_, "010",                            2               },
                { L_, "011",                            3               },
                { L_, "100",                            4               },
                { L_, "101",                            5               },
                { L_, "110",                            6               },
                { L_, "111",                            7               },

                { L_, "0..0",                           0               },
                { L_, "00..0",                          0               },
                { L_, "0..00",                          0               },
                { L_, "10..0",                          int64Min        },
                { L_, "0..01",                          1               },

                { L_, "1..1",                           -one64          },
                { L_, "11..1",                          -one64          },
                { L_, "1..11",                          -one64          },
                { L_, "01..1",                          int64Max        },
                { L_, "1..10",                          -two64          },

                { L_, " ",                              g64("")           },
                { L_, " 0 ",                            g64("0")          },
                { L_, " 1 1 ",                          g64("11")         },
                { L_, " 1 0 1 0 . . 0 0 1 0 ",          g64("1010..0010") },
                { L_, " 1 0 1 0 . . 0 0 1 0 ",          g64("1010..0010") },

                { L_, " 0 . . 0 ",                      g64("0..0")       },
                { L_, " 1 0 . . 0 ",                    g64("10..0")      },
                { L_, " 0 . . 0 1 ",                    g64("0..01")      },

                { L_, " 1 . . 1 ",                      g64("1..1")       },
                { L_, " 0 1 . . 1 ",                    g64("01..1")      },
                { L_, " 1 . . 1 0",                     g64("1..10")      },

                { L_, " 01 10 .. 01 01",                g64("0110..0101") },
                { L_, "\t0110.\t.0101\t",               g64("0110..0101") },
                { L_, "\t0110.\t.0101\t",               g64("0110..0101") },

                { L_, " 10  11. .11 10",                g64("1011..1110") },
                { L_, "1   011..11   10",               g64("1011..1110") },
                { L_, " 1 0 \t 1 1 . \t . 1 1 \t 1 0 ", g64("1011..1110") },
            };
            const int NUM_DATA = sizeof DATA / sizeof *DATA;

            for (int di = 0; di < NUM_DATA; ++di) {
                const int   LINE =     DATA[di].d_lineNum;
                const char *spec =     DATA[di].d_spec;
                uint64_t    exp  =     DATA[di].d_value;
                uint64_t    res  = g64(DATA[di].d_spec);

                if (veryVerbose) { P_(LINE); P_(spec); P_(exp); P(res); }
                LOOP3_ASSERT(LINE, res, exp, res == exp);
            }
        }

        if (verbose) cout << "\nVerify invalid input is detected." << endl;
        {
            static const struct {
                int         d_lineNum;  // line number
                const char *d_spec;     // input spec.
                uint64_t    d_value;    // resulting error code
            } DATA[] = {
                //L#  Input Specification   Resulting Error Code
                //--  -------------------   --------------------
                { L_, "A",                  G_ILLEGAL_CHARACTER     },
                { L_, "2",                  G_ILLEGAL_CHARACTER     },
                { L_, ":",                  G_ILLEGAL_CHARACTER     },
                { L_, "z0",                 G_ILLEGAL_CHARACTER     },
                { L_, "09",                 G_ILLEGAL_CHARACTER     },
                { L_, "0_0",                G_ILLEGAL_CHARACTER     },

                { L_, "0..1",               G_MISMATCHED_RANGE      },
                { L_, "0..10",              G_MISMATCHED_RANGE      },
                { L_, "0..11",              G_MISMATCHED_RANGE      },
                { L_, "00..1",              G_MISMATCHED_RANGE      },
                { L_, "10..1",              G_MISMATCHED_RANGE      },

                { L_, "1..0",               G_MISMATCHED_RANGE      },
                { L_, "1..00",              G_MISMATCHED_RANGE      },
                { L_, "1..01",              G_MISMATCHED_RANGE      },
                { L_, "01..0",              G_MISMATCHED_RANGE      },
                { L_, "11..0",              G_MISMATCHED_RANGE      },

                { L_, "0..",                G_MISSING_RANGE_END     },
                { L_, "00..",               G_MISSING_RANGE_END     },
                { L_, "10..",               G_MISSING_RANGE_END     },

                { L_, "1..",                G_MISSING_RANGE_END     },
                { L_, "01..",               G_MISSING_RANGE_END     },
                { L_, "11..",               G_MISSING_RANGE_END     },

                { L_, "..0",                G_MISSING_RANGE_START   },
                { L_, "..00",               G_MISSING_RANGE_START   },
                { L_, "..01",               G_MISSING_RANGE_START   },

                { L_, "..1",                G_MISSING_RANGE_START   },
                { L_, "..10",               G_MISSING_RANGE_START   },
                { L_, "..11",               G_MISSING_RANGE_START   },

                { L_, "0.1",                G_MISSING_SECOND_DOT    },
                { L_, "0.10",               G_MISSING_SECOND_DOT    },
                { L_, "0.11",               G_MISSING_SECOND_DOT    },
                { L_, "00.1",               G_MISSING_SECOND_DOT    },
                { L_, "10.1",               G_MISSING_SECOND_DOT    },

                { L_, "1.0",                G_MISSING_SECOND_DOT    },
                { L_, "1.00",               G_MISSING_SECOND_DOT    },
                { L_, "1.01",               G_MISSING_SECOND_DOT    },
                { L_, "01.0",               G_MISSING_SECOND_DOT    },
                { L_, "11.0",               G_MISSING_SECOND_DOT    },

                { L_, "0..0.",              G_MULTIPLE_RANGES       },
                { L_, "0..00.",             G_MULTIPLE_RANGES       },
                { L_, "0..01.",             G_MULTIPLE_RANGES       },

                { L_, "1..1.",              G_MULTIPLE_RANGES       },
                { L_, "1..10.",             G_MULTIPLE_RANGES       },
                { L_, "1..11.",             G_MULTIPLE_RANGES       },

                { L_, SW_0,                 0                       },
                { L_, SW_0 "0",             G_TOO_MANY_BITS         },
                { L_, SW_0 "1",             G_TOO_MANY_BITS         },
                { L_, "0" SW_0,             G_TOO_MANY_BITS         },
                { L_, "1" SW_0,             G_TOO_MANY_BITS         },

                { L_, SW_1,                 -one64                   },
                { L_, SW_1 "0",             G_TOO_MANY_BITS         },
                { L_, SW_1 "1",             G_TOO_MANY_BITS         },
                { L_, "0" SW_1,             G_TOO_MANY_BITS         },
                { L_, "1" SW_1,             G_TOO_MANY_BITS         },

                { L_, "1..1" SW_0,          0                       },
                { L_, FW_0 "1..1" FW_0,     0                       },
                { L_, SW_0 "1..1",          0                       },

                { L_, "1..1" SW_0"0",       G_TOO_MANY_BITS         },
                { L_, FW_0 "1..1" FW_0 "0", G_TOO_MANY_BITS         },
                { L_, "0" SW_0"1..1",       G_TOO_MANY_BITS         },

                { L_, "0..0" SW_1,          -one64                  },
                { L_, FW_1 "0..0" FW_1,     -one64                  },
                { L_, SW_1 "0..0",          -one64                  },

                { L_, "0..0" SW_1 "0",      G_TOO_MANY_BITS         },
                { L_, FW_1 "0..0" FW_1 "1", G_TOO_MANY_BITS         },
                { L_, "1" SW_1 "0..0",      G_TOO_MANY_BITS         },
            };
            const int NUM_DATA = sizeof DATA / sizeof *DATA;

            G_OFF = 1;  // set to 1 to enable testing of G function errors

            for (int di = 0; di < NUM_DATA ; ++di) {
                const int   LINE =     DATA[di].d_lineNum;
                const char *spec =     DATA[di].d_spec;
                uint64_t    exp  =     DATA[di].d_value;
                uint64_t    res  = g64(DATA[di].d_spec);

                if (veryVerbose) { P_(LINE); P_(spec); P_(exp); P(res); }

                LOOP3_ASSERT(LINE, res, exp, res == exp);
            }

            G_OFF = 0;  // set to 1 to enable testing of G function errors
        }

      } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST
        //   A utility component typically does not need a breathing test.
        //   This case is provided as a temporary workspace during development.
        //
        // Testing:
        //   BREATHING TEST:
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "BREATHING TEST" << endl
                          << "==============" << endl;

      } break;
      default: {
        cerr << "WARNING: CASE `" << test << "' NOT FOUND." << endl;
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        cerr << "Error, non-zero test status = "
             << testStatus << "." << endl;
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
