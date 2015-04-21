// bdlb_bitstringimputil.t.cpp                                        -*-C++-*-

#include <bdlb_bitstringimputil.h>

#include <bdls_testutil.h>

#include <bsls_asserttest.h>

#include <bsl_algorithm.h>
#include <bsl_iostream.h>

#include <bsl_cctype.h>      // isspace()
#include <bsl_climits.h>     // INT_MIN, INT_MAX
#include <bsl_cstdlib.h>     // atoi()

using namespace BloombergLP;
using namespace bsl;  // automatically added by script

//=============================================================================
//                                TEST PLAN
//-----------------------------------------------------------------------------
//                                Overview
//                                --------
// Exploit sign-extension to simulate leading 1's and 0's portably.
// Loop over the number of bits per word, where that makes sense.  Use a
// special generator function g(spec) that recognizes up to one fill pattern
// of the form 0..0 or 1..1 to create integers of unspecified length more
// easily.  Note that the behavior of shifting the number of bits per word on
// a integer is undefined.  The general category partitioning is based on
// boundary cases such as 0, 1, BITS_PER_WORD - 1, and BITS_PER_WORD.
// Note that it was necessary to break 'main' up into separate files because
// of unacceptably long build times on windows.
//-----------------------------------------------------------------------------
// [14] void xorEqWord(  uint64_t *dScalar, uint64_t srcScalar);
// [13] void setEqWord(  uint64_t *dScalar, uint64_t srcScalar);
// [12] void orEqWord(   uint64_t *dScalar, uint64_t srcScalar);
// [11] void minusEqWord(uint64_t *dScalar, uint64_t srcScalar);
// [10] void andEqWord(  uint64_t *dScalar, uint64_t srcScalar);
// [ 9] int find1AtMaxIndexRaw(uint64_t srcInteger)
// [ 9] int find1AtMinIndexRaw(uint64_t srcInteger)
// [ 8] void minusEqBits(uint64_t *dInt, int dIdx, uint64_t sInt, int nBits)
// [ 7] void xorEqBits(  uint64_t *dInt, int dIdx, uint64_t sInt, int nBits)
// [ 6] void orEqBits(   uint64_t *dInt, int dIdx, uint64_t sInt, int nBits)
// [ 5] void andEqBits(  uint64_t *dInt, int dIdx, uint64_t sInt, int nBits)
// [ 4] void setEqBits(  uint64_t *dInt, int dIdx, uint64_t sInt, int nBits)
// [ 3] k_BITS_PER_UINT64
//-----------------------------------------------------------------------------
// [16] USAGE EXAMPLE
// [15] NEGATIVE TESTING
// [ 2] TEST GENERATOR FUNCTION: uint64_t g(const char *spec)
// [ 2] TEST GENERATOR FUNCTION: uint64_t g64(const char *spec)
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

//=============================================================================
//                        NEGATIVE TESTING MACROS
//-----------------------------------------------------------------------------

#define ASSERT_SAFE_PASS(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_PASS(EXPR)
#define ASSERT_SAFE_FAIL(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_FAIL(EXPR)
#define ASSERT_PASS(EXPR)      BSLS_ASSERTTEST_ASSERT_PASS(EXPR)
#define ASSERT_FAIL(EXPR)      BSLS_ASSERTTEST_ASSERT_FAIL(EXPR)
#define ASSERT_OPT_PASS(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_PASS(EXPR)
#define ASSERT_OPT_FAIL(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_FAIL(EXPR)

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

typedef bdlb::BitStringImpUtil       Util;
typedef Util::uint64_t               uint64_t;

const static uint64_t uint64Max = static_cast<uint64_t>(-1);
const static uint64_t int64Max  = uint64Max / 2;
const static uint64_t int64Min  = -int64Max - 1;
const static uint64_t zero      = 0;
const static uint64_t one       = 1;

enum { BITS_PER_WORD   = 8 * sizeof(int),
       BPW             = BITS_PER_WORD,
       BITS_PER_UINT64 = 8 * sizeof(uint64_t),
       BPS             = BITS_PER_UINT64 };

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
void setBits(int *integer, int mask, int booleanValue)
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
void setLSB(int *integer, const char *endOfSpec, int charCount)
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
void setMSB(int *integer, const char *startOfSpec, int charCount)
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

static int g(const char *spec)
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

    int result;     // value to be returned

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
          case '0':
          case '1': {
            setBits64(integer, mask, '1' == ch);
            mask >>= 1;
          } break;
          default: {
            ;
          }
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

static
uint64_t x8(unsigned int x)
    // Return a 'uint64_t' filled with 8 repetitions of the specified 8-bit
    // 'x'.  The behavior is undefined if 'x >= 256'.
{
    BSLS_ASSERT_SAFE(x < 256);

    x |= x <<  8;
    x |= x << 16;

    return x | ((uint64_t) x << 32);
}

int find1AtMaxOracle(uint64_t value)
{

    for (int result = BPS - 1; result >= 0; --result) {
        if (value & (one << result)) {
            return result;                                            // RETURN
        }
    }

    ASSERT(0 && "0 paassed to find1AtMaxOracle");
    return -1;
}

int find1AtMinOracle(uint64_t value)
{

    for (int result = 0; result < BPS; ++result) {
        if (value & (one << result)) {
            return result;                                            // RETURN
        }
    }

    ASSERT(0 && "0 paassed to find1AtMinOracle");
    return -1;
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
      case 15: {
    //-------------------------------------------------------------------------
    // TESTING USAGE EXAMPLE
    //   Exported, without modification, to the header file.
    //
    // Concerns:
    //   That the usage example from the .h file is correct.
    //-------------------------------------------------------------------------

    if (verbose) cout << "Testing Usage Example\n"
                         "=====================\n";

///Manipulators
/// - - - - - -
//..
    uint64_t myInt;

    myInt = 0x3333;
    bdlb::BitStringImpUtil::andEqBits(&myInt, 8,  0, 8);
    ASSERT(  0x33 == myInt);
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
    myInt = 0x3333;
    bdlb::BitStringImpUtil::andEqBits(&myInt, 8, -1, 8);    // Note '-1' is all
                                                           // '1's.
    ASSERT(0x3333 == myInt);
//..
// 'orEqBits' will take a slice of a second integer 'srcInteger' and bitwise
// OR it with another integer:
//..
    myInt = 0x33333333;
    bdlb::BitStringImpUtil::orEqBits(&myInt, 16, -1, 8);
    ASSERT((int) 0x33ff3333 == myInt);
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
    myInt = 0;
    bdlb::BitStringImpUtil::orEqBits(&myInt, 16, -1, 8);
    ASSERT((int) 0x00ff0000 == myInt);
//..
// Another interface with 'xorEqBits' will take a section of a second scalar
// and bitwise XOR it with a first scalar:
//..
    myInt = 0x77777777;
    bdlb::BitStringImpUtil::xorEqBits(&myInt, 16, 0xff, 8);
    ASSERT((int) 0x77887777 == myInt);
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
    myInt = 0x77777777;
    bdlb::BitStringImpUtil::xorEqBits(&myInt, 16, 0x55, 8);
    ASSERT((int) 0x77227777 == myInt);
//..
///Accessors
///- - - - -
// The 'find[01]At(Max,Min)IndexRaw' routines are used for finding the highest
// (or lowest) set (or clear) bit in a scalar, possibly within a subset of the
// integer:
//..
    ASSERT( 8 == bdlb::BitStringImpUtil::find1AtMaxIndexRaw(0x00000101));
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
    ASSERT( 8 == bdlb::BitStringImpUtil::find1AtMinIndexRaw(0xffff0100));
//..
      } break;
      case 14: {
        // --------------------------------------------------------------------
        // TESTING XOREQWORD
        //   Ensure the method has the expected effect on 'dstScalar'.
        //
        // Concerns:
        //: 1 That the behavior of 'xorEqWord' matches the bitwise C expression
        //:   '*dstScalar ^= srcScalar'.
        //: 2 No negative testing is necessary as the is no possible undefined
        //:   behavior.
        //
        // Plan:
        //   Since 'bitN' of the result is never affected in any way by 'bitM'
        //   of the operands, unless 'N == M', we can just test exhaustively
        //   test for all combinations of the first 8 bits, repeated over the
        //   word using the 'x8' function.
        //: 1 Iterate x over all combinations of the low-ofder 8 bits.
        //: 2 Create 'xx' by repeating the low-order 8 bits of 'x' 8 times
        //:   across the 64-bit word.
        //: 3 In a nested loop, iterate y over all combinations of the
        //:   low-ofder 8 bits.
        //: 4 Create 'yy' by repeating the low-order 8 bits of 'y' 8 times
        //:   across the 64-bit word.
        //: 5 Evalute 'xx ^= yy' twice, once using the function under test,
        //:   once using the corresponding bitwise C operator, and verify that
        //:   they match.
        //
        // Testing:
        //   void xorEqWord(uint64_t *dstScalar, uint64_t srcScalar);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING XOREQWORD\n"
                          << "=================\n";

        enum { LIMIT = 1 << 8 };

        for (unsigned int x = 0; x < LIMIT; ++x) {
            const uint64_t xx = x8(x);

            for (unsigned int y = 0; y < LIMIT; ++y) {
                const uint64_t yy  = x8(y);
                uint64_t       xxa = xx;
                uint64_t       xxb = xx;

                Util::xorEqWord(&xxa, yy);
                xxb ^= yy;

                ASSERT(xxa == xxb);
            }
        }
      } break;
      case 13: {
        // --------------------------------------------------------------------
        // TESTING SETEQWORD
        //   Ensure the method has the expected effect on 'dstScalar'.
        //
        // Concerns:
        //: 1 That the behavior of 'setEqWord' matches the bitwise C expression
        //:   '*dstScalar = srcScalar'.
        //: 2 No negative testing is necessary as the is no possible undefined
        //:   behavior.
        //
        // Plan:
        //   Since 'bitN' of the result is never affected in any way by 'bitM'
        //   of the operands, unless 'N == M', we can just test exhaustively
        //   test for all combinations of the first 8 bits, repeated over the
        //   word using the 'x8' function.
        //: 1 Iterate x over all combinations of the low-ofder 8 bits.
        //: 2 Create 'xx' by repeating the low-order 8 bits of 'x' 8 times
        //:   across the 64-bit word.
        //: 3 In a nested loop, iterate y over all combinations of the
        //:   low-ofder 8 bits.
        //: 4 Create 'yy' by repeating the low-order 8 bits of 'y' 8 times
        //:   across the 64-bit word.
        //: 5 Evalute 'xx = yy' twice, once using the function under test, once
        //:   using the corresponding bitwise C operator, and verify that they
        //:   match.
        //
        // Testing:
        //   void setEqWord(uint64_t *dstScalar, uint64_t srcScalar);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING SETEQWORD\n"
                          << "=================\n";

        enum { LIMIT = 1 << 8 };

        for (unsigned int x = 0; x < LIMIT; ++x) {
            const uint64_t xx = x8(x);

            for (unsigned int y = 0; y < LIMIT; ++y) {
                const uint64_t yy  = x8(y);
                uint64_t       xxa = xx;
                uint64_t       xxb = xx;

                Util::setEqWord(&xxa, yy);
                xxb = yy;

                ASSERT(xxa == xxb);
            }
        }
      } break;
      case 12: {
        // --------------------------------------------------------------------
        // TESTING OREQWORD
        //   Ensure the method has the expected effect on 'dstScalar'.
        //
        // Concerns:
        //: 1 That the behavior of 'orEqWord' matches the bitwise C expression
        //:   '*dstScalar |= srcScalar'.
        //: 2 No negative testing is necessary as the is no possible undefined
        //:   behavior.
        //
        // Plan:
        //   Since 'bitN' of the result is never affected in any way by 'bitM'
        //   of the operands, unless 'N == M', we can just test exhaustively
        //   test for all combinations of the first 8 bits, repeated over the
        //   word using the 'x8' function.
        //: 1 Iterate x over all combinations of the low-ofder 8 bits.
        //: 2 Create 'xx' by repeating the low-order 8 bits of 'x' 8 times
        //:   across the 64-bit word.
        //: 3 In a nested loop, iterate y over all combinations of the
        //:   low-ofder 8 bits.
        //: 4 Create 'yy' by repeating the low-order 8 bits of 'y' 8 times
        //:   across the 64-bit word.
        //: 5 Evalute 'xx |= yy' twice, once using the function under test,
        //:   once using the corresponding bitwise C operator, and verify that
        //:   they match.
        //
        // Testing:
        //   void orEqWord(uint64_t *dstScalar, uint64_t srcScalar);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING OREQWORD\n"
                          << "================\n";

        enum { LIMIT = 1 << 8 };

        for (unsigned int x = 0; x < LIMIT; ++x) {
            const uint64_t xx = x8(x);

            for (unsigned int y = 0; y < LIMIT; ++y) {
                const uint64_t yy  = x8(y);
                uint64_t       xxa = xx;
                uint64_t       xxb = xx;

                Util::orEqWord(&xxa, yy);
                xxb |= yy;

                ASSERT(xxa == xxb);
            }
        }
      } break;
      case 11: {
        // --------------------------------------------------------------------
        // TESTING MINUSEQWORD
        //   Ensure the method has the expected effect on 'dstScalar'.
        //
        // Concerns:
        //: 1 That the behavior of 'minusEqWord' matches the bitwise C
        //:   expression '*dstScalar |= srcScalar'.
        //: 2 No negative testing is necessary as the is no possible undefined
        //:   behavior.
        //
        // Plan:
        //   Since 'bitN' of the result is never affected in any way by 'bitM'
        //   of the operands, unless 'N == M', we can just test exhaustively
        //   test for all combinations of the first 8 bits, repeated over the
        //   word using the 'x8' function.
        //: 1 Iterate x over all combinations of the low-ofder 8 bits.
        //: 2 Create 'xx' by repeating the low-order 8 bits of 'x' 8 times
        //:   across the 64-bit word.
        //: 3 In a nested loop, iterate y over all combinations of the
        //:   low-ofder 8 bits.
        //: 4 Create 'yy' by repeating the low-order 8 bits of 'y' 8 times
        //:   across the 64-bit word.
        //: 5 Evalute 'xx -= yy' twice, once using the function under test,
        //:   once using the corresponding bitwise C operator, and verify that
        //:   they match.
        //
        // Testing:
        //   void minusEqWord(uint64_t *dstScalar, uint64_t srcScalar);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING MINUSEQ\n"
                          << "===============\n";

        enum { LIMIT = 1 << 8 };

        for (unsigned int x = 0; x < LIMIT; ++x) {
            const uint64_t xx = x8(x);

            for (unsigned int y = 0; y < LIMIT; ++y) {
                const uint64_t yy  = x8(y);
                uint64_t       xxa = xx;
                uint64_t       xxb = xx;

                Util::minusEqWord(&xxa, yy);
                xxb &= ~yy;

                ASSERT(xxa == xxb);
            }
        }
      } break;
      case 10: {
        // --------------------------------------------------------------------
        // TESTING ANDEQWORD
        //   Ensure the method has the expected effect on 'dstScalar'.
        //
        // Concerns:
        //: 1 That the behavior of 'andEqWord' matches the bitwise C expression
        //:   '*dstScalar |= srcScalar'.
        //: 2 No negative testing is necessary as the is no possible undefined
        //:   behavior.
        //
        // Plan:
        //   Since 'bitN' of the result is never affected in any way by 'bitM'
        //   of the operands, unless 'N == M', we can just test exhaustively
        //   test for all combinations of the first 8 bits, repeated over the
        //   word using the 'x8' function.
        //: 1 Iterate x over all combinations of the low-ofder 8 bits.
        //: 2 Create 'xx' by repeating the low-order 8 bits of 'x' 8 times
        //:   across the 64-bit word.
        //: 3 In a nested loop, iterate y over all combinations of the
        //:   low-ofder 8 bits.
        //: 4 Create 'yy' by repeating the low-order 8 bits of 'y' 8 times
        //:   across the 64-bit word.
        //: 5 Evalute 'xx &= yy' twice, once using the function under test,
        //:   once using the corresponding bitwise C operator, and verify that
        //:   they match.
        //
        // Testing:
        //   void andEqWord(uint64_t *dstScalar, uint64_t srcScalar);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING ANDEQWORD\n"
                          << "=================\n";

        enum { LIMIT = 1 << 8 };

        for (unsigned int x = 0; x < LIMIT; ++x) {
            const uint64_t xx = x8(x);

            for (unsigned int y = 0; y < LIMIT; ++y) {
                const uint64_t yy  = x8(y);
                uint64_t       xxa = xx;
                uint64_t       xxb = xx;

                Util::andEqWord(&xxa, yy);
                xxb &= yy;

                ASSERT(xxa == xxb);
            }
        }
      } break;
      case 9: {
        // --------------------------------------------------------------------
        // TESTING SEARCH FUNCTIONS:
        //   Ensure the methods return the expected value.
        //
        // Concerns:
        //: 1 That the search functions correctly identify undefined behavior.
        //: 2 That the search functions return correct values.
        //
        // Plan:
        //: 1 Do negative testing.
        //: 2 Do table-driven testing.
        //: 3 During table-driven testing, apply 'find1AtMaxOracle' and
        //:   'find1AtMinOracle', which are very reliable, brute-force, but
        //:   inefficient functions, to verify that the tables are indeed
        //:   correct.
        //
        // Testing:
        //   int find1AtMaxIndexRaw(uint64_t srcInteger)
        //   int find1AtMinIndexRaw(uint64_t srcInteger)
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING SEARCH FUNCTIONS" << endl
                          << "========================" << endl;

        if (verbose) cout << "Negative Testing\n";
        {
            bsls::AssertTestHandlerGuard guard;

            // find1AtMaxIndexRaw

            ASSERT_SAFE_PASS(Util::find1AtMaxIndexRaw(1));
            ASSERT_SAFE_PASS(Util::find1AtMaxIndexRaw(10));
            ASSERT_SAFE_PASS(Util::find1AtMaxIndexRaw(-1LL));
            ASSERT_SAFE_FAIL(Util::find1AtMaxIndexRaw(0));

            // find1AtMinIndexRaw

            ASSERT_SAFE_PASS(Util::find1AtMinIndexRaw(1));
            ASSERT_SAFE_PASS(Util::find1AtMinIndexRaw(10));
            ASSERT_SAFE_PASS(Util::find1AtMinIndexRaw(-1LL));
            ASSERT_SAFE_FAIL(Util::find1AtMinIndexRaw(0));
        }

        if (verbose) cout << "\tTesting find1AtMaxIndexRaw" << endl;
        {
            static const struct {
                int         d_lineNum;     // line number
                const char *d_src;         // source integer
                int         d_expected;    // expected result
            } DATA[] = {
                //L#                  src          expected result
                //--             --------         ----------------
                { L_,              "0..01",       0                },
                { L_,             "0..011",       1                },
                { L_,            "0..0111",       2                },
                { L_,           "0..01111",       3                },
                { L_,          "0..011111",       4                },
                { L_,         "0..0111111",       5                },
                { L_,        "0..01111111",       6                },
                { L_,       "0..011111111",       7                },

                { L_,             "100..0",       BPW - 1          },
                { L_,             "010..0",       BPW - 2          },
                { L_,             "110..0",       BPW - 1          },

                { L_,    "000000" "100..0",       BPW - 8 + 1      },
                { L_,    "000000" "010..0",       BPW - 8          },
                { L_,    "000000" "110..0",       BPW - 8 + 1      },

                { L_,        B8_0 "100..0",       BPW - 8 - 1      },
                { L_,        B8_0 "010..0",       BPW - 8 - 2      },
                { L_,        B8_0 "110..0",       BPW - 8 - 1      },

                { L_, B8_0 "000000" "100..0",     BPW - 16 + 1     },
                { L_, B8_0 "000000" "010..0",     BPW - 16         },
                { L_, B8_0 "000000" "110..0",     BPW - 16 + 1     },

                { L_,       B16_0 "100..0",       BPW - 16 - 1     },
                { L_,       B16_0 "010..0",       BPW - 16 - 2     },
                { L_,       B16_0 "110..0",       BPW - 16 - 1     }
            };
            const int NUM_DATA = sizeof DATA / sizeof *DATA;

            int i, j;
            for (i = 0; i < NUM_DATA; ++i) {
                const int   LINE_HI     = DATA[i].d_lineNum;
                const char *specHi      = DATA[i].d_src;
                int         srcHi       = g(specHi);
                const int   EXPECTED_HI = DATA[i].d_expected;

                for (j = 0; j < NUM_DATA; ++j) {
                    const int    LINE_LO     = DATA[j].d_lineNum;
                    const char  *specLo      = DATA[j].d_src;
                    unsigned int srcLo       = g(specLo);
                    const int    EXPECTED_LO = DATA[j].d_expected;

                    const int EXPECTED = (EXPECTED_HI < 0) ? EXPECTED_LO
                                                           : EXPECTED_HI + 32;
                    uint64_t  src      = ((uint64_t)srcHi << 32)
                                        | (uint64_t)srcLo;
                    int       result   = Util::find1AtMaxIndexRaw(src);

                    if (veryVerbose) {
                        P_(LINE_HI); P_(specHi); P(srcHi);
                        P_(LINE_LO); P_(specLo); P(srcLo);
                        P_(src); P_(EXPECTED); P(result);
                    }
                    LOOP4_ASSERT(LINE_HI, LINE_LO, EXPECTED, result,
                                 EXPECTED == result);
                    ASSERT(EXPECTED == find1AtMaxOracle(src));
                }
            }
        }

        if (verbose) cout << "\tTesting find1AtMinIndexRaw" << endl;
        {
            static const struct {
                int         d_lineNum;     // line number
                const char *d_src;         // source integer
                int         d_expected;    // expected result
            } DATA[] = {
                //L#                  src          expected result
                //--             --------         ----------------
                { L_,               "1..1",       0                },
                { L_,              "1..10",       1                },
                { L_,             "1..100",       2                },
                { L_,            "1..1000",       3                },
                { L_,           "1..10000",       4                },
                { L_,          "1..100000",       5                },
                { L_,         "1..1000000",       6                },
                { L_,        "1..10000000",       7                },

                { L_,             "100..0",       BPW - 1          },
                { L_,             "010..0",       BPW - 2          },
                { L_,             "110..0",       BPW - 2          },

                { L_,    "111111" "100..0",       BPW - 8 + 1      },
                { L_,    "111111" "010..0",       BPW - 8          },
                { L_,    "111111" "110..0",       BPW - 8          },

                { L_,        B8_1 "100..0",       BPW - 8 - 1      },
                { L_,        B8_1 "010..0",       BPW - 8 - 2      },
                { L_,        B8_1 "110..0",       BPW - 8 - 2      },

                { L_, B8_1 "111111" "100..0",     BPW - 16 + 1     },
                { L_, B8_1 "111111" "010..0",     BPW - 16         },
                { L_, B8_1 "111111" "110..0",     BPW - 16         },

                { L_,       B16_1 "100..0",       BPW - 16 - 1     },
                { L_,       B16_1 "010..0",       BPW - 16 - 2     },
                { L_,       B16_1 "110..0",       BPW - 16 - 2     }

            };
            const int NUM_DATA = sizeof DATA / sizeof *DATA;

            int i, j;
            for (i = 0; i < NUM_DATA; ++i) {
                const int   LINE_HI     = DATA[i].d_lineNum;
                const char *specHi      = DATA[i].d_src;
                int         srcHi       = g(specHi);
                const int   EXPECTED_HI = DATA[i].d_expected;

                for (j = 0; j < NUM_DATA; ++j) {
                    const int     LINE_LO     = DATA[j].d_lineNum;
                    const char   *specLo      = DATA[j].d_src;
                    unsigned int  srcLo       = g(specLo);
                    const int     EXPECTED_LO = DATA[j].d_expected;
                    const int     EXPECTED    = (EXPECTED_LO == BPW)
                                                ? EXPECTED_HI + 32
                                                : EXPECTED_LO;
                    uint64_t      src         = ((uint64_t)srcHi << 32)
                                               | (uint64_t)srcLo;
                    int           result      = Util::find1AtMinIndexRaw(src);

                    if (veryVerbose) {
                        P_(LINE_HI); P_(specHi); P(srcHi);
                        P_(LINE_LO); P_(specLo); P(srcLo);
                        P_(src); P_(EXPECTED); P(result);
                    }
                    LOOP4_ASSERT(LINE_HI, LINE_LO, EXPECTED, result,
                                 EXPECTED == result);
                    ASSERT(EXPECTED == find1AtMinOracle(src));
                }
            }
        }
      } break;
      case 8: {
        // --------------------------------------------------------------------
        // TESTING 'xorEqBits' FUNCTION
        //   Ensure the method has the expected effect on 'dstScalar'.
        //
        // Concerns:
        //: 1 That the function properly detects undefined behavior.
        //: 2 That the function has the correct effect on the '*dstScalar'
        //:   output.
        //
        // Plan:
        //: 1 Do negative testing.
        //: 2 Do table-driven testing.  Note that this used to be a 5-arg,
        //:   rather than a 4-arg, function, taking an 'sindex' operand that
        //:   right-shifted 'srcScalar' before applying.  We use the full
        //:   table by right-shifting 'src' ourselves in the test driver and
        //:   are thus able to make use of the old test cases.
        //
        // Testing:
        //   void xorEqBits(uint64_t_t *dInt,
        //                  int dIdx,
        //                  uint64_t_t sInt,
        //                  int nBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "Testing 'xorEqBits' Function" << endl
                          << "============================" << endl;

        if (verbose) cout << "Negative Testing\n";
        {
            bsls::AssertTestHandlerGuard guard;

            uint64_t dst;

            ASSERT_SAFE_PASS(Util::xorEqBits(&dst, 0, 20, 0));
            ASSERT_SAFE_PASS(Util::xorEqBits(&dst, 10, 20, 0));
            ASSERT_SAFE_PASS(Util::xorEqBits(&dst, 10, 20, 10));
            ASSERT_SAFE_PASS(Util::xorEqBits(&dst, 10, -1LL,
                                                          BITS_PER_UINT64-10));

            ASSERT_SAFE_FAIL(Util::xorEqBits(&dst, -1LL, 20, 0));
            ASSERT_SAFE_FAIL(Util::xorEqBits(&dst,  0, 20, -1LL));
            ASSERT_SAFE_FAIL(Util::xorEqBits(&dst, 10, 20, -1LL));
            ASSERT_SAFE_FAIL(Util::xorEqBits(&dst, -1LL, 20, 10));
            ASSERT_SAFE_FAIL(Util::xorEqBits(&dst, 10, -1LL,
                                                          BITS_PER_UINT64- 9));
        }

        if (verbose) cout << "Table-driven Testing\n";
        const char *DSTA = "1101101100100";    // typical case
        const char *DSTB = "110110110010..0";    // typical case

        static const struct {
            int         d_lineNum;     // line number
            const char *d_src;         // source integer
            int         d_sindex;      // source index
            const char *d_dst;         // destination integer
            int         d_dindex;      // destination index
            int         d_numBits;     // num of bits
            uint64_t    d_xorEqual;    // expected result from xorEqual
        } DATA_B[] = {
            //L#    src    sIdx     dst    dIdx  nBits      xorEqual Result
            //--  ------    ---   ------   ----   ----  -------------------
            { L_, "0..0",     0,  "0..0",     0,     0,                   0 },
            { L_, "0..0",     0,  "0..0",     0,     1,                   0 },
            { L_, "0..0",     0,  "0..0",     0, BPS-1,                   0 },
            { L_, "0..0",     0,  "0..0",     0,   BPS,                   0 },
            { L_, "0..0",     0,  "0..0",     1,     0,                   0 },
            { L_, "0..0",     0,  "0..0",     1,     1,                   0 },
            { L_, "0..0",     0,  "0..0",     1, BPS-1,                   0 },
            { L_, "0..0",     0,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0",     0,  "0..0", BPS-1,     1,                   0 },
            { L_, "0..0",     0,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0",     0,  "1..1",     0,     0,               ~zero },
            { L_, "0..0",     0,  "1..1",     0,     1,               ~zero },
            { L_, "0..0",     0,  "1..1",     0, BPS-1,               ~zero },
            { L_, "0..0",     0,  "1..1",     0,   BPS,               ~zero },
            { L_, "0..0",     0,  "1..1",     1,     0,               ~zero },
            { L_, "0..0",     0,  "1..1",     1,     1,               ~zero },
            { L_, "0..0",     0,  "1..1",     1, BPS-1,               ~zero },
            { L_, "0..0",     0,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0",     0,  "1..1", BPS-1,     1,               ~zero },
            { L_, "0..0",     0,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0",     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     0,   BPS,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0",     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     0,   BPS,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,   BPS,     0,          g64(SW_10) },

            //L#    src    sIdx     dst    dIdx  nBits      xorEqual Result
            //--  ------    ---   ------   ----   ----  -------------------
            { L_, "0..0",     0,    DSTA,     2,     4,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     2,     5,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     2,     6,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     2,     7,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     2,     8,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     4,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     5,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     6,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     7,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     8,g64("1101101100100") },

            { L_, "0..0",     0,    DSTB,    53,     4,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    53,     5,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    53,     6,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    53,     7,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    53,     8,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    54,     4,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    54,     5,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    54,     6,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    54,     7,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    54,     8,g64("110110110010..0")},

            { L_, "0..0",     1,  "0..0",     0,     0,                   0 },
            { L_, "0..0",     1,  "0..0",     0,     1,                   0 },
            { L_, "0..0",     1,  "0..0",     0, BPS-1,                   0 },
            { L_, "0..0",     1,  "0..0",     1,     0,                   0 },
            { L_, "0..0",     1,  "0..0",     1,     1,                   0 },
            { L_, "0..0",     1,  "0..0",     1, BPS-1,                   0 },
            { L_, "0..0",     1,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0",     1,  "0..0", BPS-1,     1,                   0 },
            { L_, "0..0",     1,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0",     1,  "1..1",     0,     0,               ~zero },
            { L_, "0..0",     1,  "1..1",     0,     1,               ~zero },
            { L_, "0..0",     1,  "1..1",     0, BPS-1,               ~zero },
            { L_, "0..0",     1,  "1..1",     1,     0,               ~zero },
            { L_, "0..0",     1,  "1..1",     1,     1,               ~zero },
            { L_, "0..0",     1,  "1..1",     1, BPS-1,               ~zero },
            { L_, "0..0",     1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0",     1,  "1..1", BPS-1,     1,               ~zero },
            { L_, "0..0",     1,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0",     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0",     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,   BPS,     0,          g64(SW_10) },

            //L#    src    sIdx     dst    dIdx  nBits      xorEqual Result
            //--  ------    ---   ------   ----   ----  -------------------
            { L_, "0..0",     1,    DSTA,     2,     4,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     2,     5,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     2,     6,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     2,     7,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     2,     8,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     4,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     5,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     6,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     7,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     8,g64("1101101100100") },

            { L_, "0..0",     1,    DSTB,     2,     4,  g64(DSTB) },
            { L_, "0..0",     2,    DSTB,     2,     5,  g64(DSTB) },
            { L_, "0..0",     3,    DSTB,     2,     6,  g64(DSTB) },
            { L_, "0..0",     4,    DSTB,     2,     7,  g64(DSTB) },
            { L_, "0..0",     5,    DSTB,     2,     8,  g64(DSTB) },
            { L_, "0..0",     6,    DSTB,     3,     4,  g64(DSTB) },
            { L_, "0..0",     7,    DSTB,     3,     5,  g64(DSTB) },
            { L_, "0..0",    41,    DSTB,     3,     6,  g64(DSTB) },
            { L_, "0..0",    42,    DSTB,     3,     7,  g64(DSTB) },
            { L_, "0..0",    43,    DSTB,     3,     8,  g64(DSTB) },

            { L_, "0..0", BPS-1,  "0..0",     0,     0,                   0 },
            { L_, "0..0", BPS-1,  "0..0",     0,     1,                   0 },
            { L_, "0..0", BPS-1,  "0..0",     1,     0,                   0 },
            { L_, "0..0", BPS-1,  "0..0",     1,     1,                   0 },
            { L_, "0..0", BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0", BPS-1,  "0..0", BPS-1,     1,                   0 },
            { L_, "0..0", BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0", BPS-1,  "1..1",     0,     0,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",     0,     1,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",     1,     0,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",     1,     1,               ~zero },
            { L_, "0..0", BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0", BPS-1,  "1..1", BPS-1,     1,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0", BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0", BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "0..0",   BPS,  "0..0",     0,     0,                   0 },
            { L_, "0..0",   BPS,  "0..0",     1,     0,                   0 },
            { L_, "0..0",   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0",   BPS,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0",   BPS,  "1..1",     0,     0,               ~zero },
            { L_, "0..0",   BPS,  "1..1",     1,     0,               ~zero },
            { L_, "0..0",   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0",   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0",   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0",   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0",   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0",   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0",   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0",   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0",   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0",   BPS,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",     0,  "0..0",     0,     0,                   0 },
            { L_, "1..1",     0,  "0..0",     0,     1,                   1 },
            { L_, "1..1",     0,  "0..0",     0, BPS-1,            int64Max },
            { L_, "1..1",     0,  "0..0",     0,   BPS,               ~zero },
            { L_, "1..1",     0,  "0..0",     1,     0,                   0 },
            { L_, "1..1",     0,  "0..0",     1,     1,                   2 },
            { L_, "1..1",     0,  "0..0",     1, BPS-1,              zero-2 },
            { L_, "1..1",     0,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1",     0,  "0..0", BPS-1,     1,            int64Min },
            { L_, "1..1",     0,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1",     0,  "1..1",     0,     0,               ~zero },
            { L_, "1..1",     0,  "1..1",     0,     1,              zero-2 },
            { L_, "1..1",     0,  "1..1",     0, BPS-1,            int64Min },
            { L_, "1..1",     0,  "1..1",     0,   BPS,                   0 },
            { L_, "1..1",     0,  "1..1",     1,     0,               ~zero },
            { L_, "1..1",     0,  "1..1",     1,     1,              zero-3 },
            { L_, "1..1",     0,  "1..1",     1, BPS-1,                   1 },
            { L_, "1..1",     0,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1",     0,  "1..1", BPS-1,     1,            int64Max },
            { L_, "1..1",     0,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1",     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     0,     1,        g64(SW_01)^1 },
            { L_, "1..1",     0,   SW_01,     0, BPS-1, g64(SW_10)^int64Min },
            { L_, "1..1",     0,   SW_01,     0,   BPS,          g64(SW_10) },
            { L_, "1..1",     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     1,     1,        g64(SW_01)|2 },
            { L_, "1..1",     0,   SW_01,     1, BPS-1,        g64(SW_10)|1 },
            { L_, "1..1",     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01, BPS-1,     1, g64(SW_01)|int64Min },
            { L_, "1..1",     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1",     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     0,     1,        g64(SW_10)|1 },
            { L_, "1..1",     0,   SW_10,     0, BPS-1, g64(SW_01)|int64Min },
            { L_, "1..1",     0,   SW_10,     0,   BPS,          g64(SW_01) },
            { L_, "1..1",     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     1,     1,        g64(SW_10)^2 },
            { L_, "1..1",     0,   SW_10,     1, BPS-1,        g64(SW_01)^1 },
            { L_, "1..1",     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_, "1..1",     0,   SW_10,   BPS,     0,          g64(SW_10) },

            //L#    src    sIdx     dst    dIdx  nBits      xorEqual Result
            //--  ------    ---   ------   ----   ----  -------------------
            { L_, "1..1",     0,    DSTA,     2,     4,g64("1101101011000") },
            { L_, "1..1",     0,    DSTA,     2,     5,g64("1101100011000") },
            { L_, "1..1",     0,    DSTA,     2,     6,g64("1101110011000") },
            { L_, "1..1",     0,    DSTA,     2,     7,g64("1101010011000") },
            { L_, "1..1",     0,    DSTA,     2,     8,g64("1100010011000") },
            { L_, "1..1",     0,    DSTA,     3,     4,g64("1101100011100") },
            { L_, "1..1",     0,    DSTA,     3,     5,g64("1101110011100") },
            { L_, "1..1",     0,    DSTA,     3,     6,g64("1101010011100") },
            { L_, "1..1",     0,    DSTA,     3,     7,g64("1100010011100") },
            { L_, "1..1",     0,    DSTA,     3,     8,g64("1110010011100") },

            { L_, "1..1",     0,    DSTB,    53,     4,g64("110110101100..0")},
            { L_, "1..1",     0,    DSTB,    53,     5,g64("110110001100..0")},
            { L_, "1..1",     0,    DSTB,    53,     6,g64("110111001100..0")},
            { L_, "1..1",     0,    DSTB,    53,     7,g64("110101001100..0")},
            { L_, "1..1",     0,    DSTB,    53,     8,g64("110001001100..0")},
            { L_, "1..1",     0,    DSTB,    54,     4,g64("110110001110..0")},
            { L_, "1..1",     0,    DSTB,    54,     5,g64("110111001110..0")},
            { L_, "1..1",     0,    DSTB,    54,     6,g64("110101001110..0")},
            { L_, "1..1",     0,    DSTB,    54,     7,g64("110001001110..0")},
            { L_, "1..1",     0,    DSTB,    54,     8,g64("111001001110..0")},

            { L_, "1..1",     1,  "0..0",     0,     0,                   0 },
            { L_, "1..1",     1,  "0..0",     0,     1,                   1 },
            { L_, "1..1",     1,  "0..0",     0, BPS-1,            int64Max },
            { L_, "1..1",     1,  "0..0",     1,     0,                   0 },
            { L_, "1..1",     1,  "0..0",     1,     1,                   2 },
            { L_, "1..1",     1,  "0..0",     1, BPS-1,              zero-2 },
            { L_, "1..1",     1,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1",     1,  "0..0", BPS-1,     1,            int64Min },
            { L_, "1..1",     1,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1",     1,  "1..1",     0,     0,               ~zero },
            { L_, "1..1",     1,  "1..1",     0,     1,              zero-2 },
            { L_, "1..1",     1,  "1..1",     0, BPS-1,            int64Min },
            { L_, "1..1",     1,  "1..1",     1,     0,               ~zero },
            { L_, "1..1",     1,  "1..1",     1,     1,              zero-3 },
            { L_, "1..1",     1,  "1..1",     1, BPS-1,                   1 },
            { L_, "1..1",     1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1",     1,  "1..1", BPS-1,     1,            int64Max },
            { L_, "1..1",     1,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1",     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     0,     1,        g64(SW_01)^1 },
            { L_, "1..1",     1,   SW_01,     0, BPS-1, g64(SW_10)^int64Min },
            { L_, "1..1",     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     1,     1,        g64(SW_01)|2 },
            { L_, "1..1",     1,   SW_01,     1, BPS-1,        g64(SW_10)|1 },
            { L_, "1..1",     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01, BPS-1,     1, g64(SW_01)|int64Min },
            { L_, "1..1",     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1",     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     0,     1,        g64(SW_10)|1 },
            { L_, "1..1",     1,   SW_10,     0, BPS-1, g64(SW_01)|int64Min },
            { L_, "1..1",     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     1,     1,        g64(SW_10)^2 },
            { L_, "1..1",     1,   SW_10,     1, BPS-1,        g64(SW_01)^1 },
            { L_, "1..1",     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_, "1..1",     1,   SW_10,   BPS,     0,          g64(SW_10) },

            //L#    src    sIdx     dst    dIdx  nBits      xorEqual Result
            //--  ------    ---   ------   ----   ----  -------------------
            { L_, "1..1",     1,    DSTA,     2,     4,g64("1101101011000") },
            { L_, "1..1",     1,    DSTA,     2,     5,g64("1101100011000") },
            { L_, "1..1",     1,    DSTA,     2,     6,g64("1101110011000") },
            { L_, "1..1",     1,    DSTA,     2,     7,g64("1101010011000") },
            { L_, "1..1",     1,    DSTA,     2,     8,g64("1100010011000") },
            { L_, "1..1",     1,    DSTA,     3,     4,g64("1101100011100") },
            { L_, "1..1",     1,    DSTA,     3,     5,g64("1101110011100") },
            { L_, "1..1",     1,    DSTA,     3,     6,g64("1101010011100") },
            { L_, "1..1",     1,    DSTA,     3,     7,g64("1100010011100") },
            { L_, "1..1",     1,    DSTA,     3,     8,g64("1110010011100") },

            { L_, "1..1",     1,    DSTB,    53,     4,g64("110110101100..0")},
            { L_, "1..1",     8,    DSTB,    53,     5,g64("110110001100..0")},
            { L_, "1..1",    16,    DSTB,    53,     6,g64("110111001100..0")},
            { L_, "1..1",    32,    DSTB,    53,     7,g64("110101001100..0")},
            { L_, "1..1",    44,    DSTB,    53,     8,g64("110001001100..0")},
            { L_, "1..1",    45,    DSTB,    54,     4,g64("110110001110..0")},
            { L_, "1..1",    46,    DSTB,    54,     5,g64("110111001110..0")},
            { L_, "1..1",    47,    DSTB,    54,     6,g64("110101001110..0")},
            { L_, "1..1",    48,    DSTB,    54,     7,g64("110001001110..0")},
            { L_, "1..1",    50,    DSTB,    54,     8,g64("111001001110..0")},

            { L_, "1..1", BPS-1,  "0..0",     0,     0,                   0 },
            { L_, "1..1", BPS-1,  "0..0",     0,     1,                   1 },
            { L_, "1..1", BPS-1,  "0..0",     1,     0,                   0 },
            { L_, "1..1", BPS-1,  "0..0",     1,     1,                   2 },
            { L_, "1..1", BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1", BPS-1,  "0..0", BPS-1,     1,            int64Min },
            { L_, "1..1", BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1", BPS-1,  "1..1",     0,     0,               ~zero },
            { L_, "1..1", BPS-1,  "1..1",     0,     1,              zero-2 },
            { L_, "1..1", BPS-1,  "1..1",     1,     0,               ~zero },
            { L_, "1..1", BPS-1,  "1..1",     1,     1,              zero-3 },
            { L_, "1..1", BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1", BPS-1,  "1..1", BPS-1,     1,            int64Max },
            { L_, "1..1", BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1", BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01,     0,     1,        g64(SW_01)^1 },
            { L_, "1..1", BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01,     1,     1,        g64(SW_01)|2 },
            { L_, "1..1", BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01, BPS-1,     1, g64(SW_01)|int64Min },
            { L_, "1..1", BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1", BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10,     0,     1,        g64(SW_10)|1 },
            { L_, "1..1", BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10,     1,     1,        g64(SW_10)^2 },
            { L_, "1..1", BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_, "1..1", BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",   BPS,  "0..0",     0,     0,                   0 },
            { L_, "1..1",   BPS,  "0..0",     1,     0,                   0 },
            { L_, "1..1",   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1",   BPS,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1",   BPS,  "1..1",     0,     0,               ~zero },
            { L_, "1..1",   BPS,  "1..1",     1,     0,               ~zero },
            { L_, "1..1",   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1",   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1",   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1",   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1",   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1",   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1",   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1",   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1",   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1",   BPS,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,     0,  "0..0",     0,     0,                   0 },
            { L_,  SW_01,     0,  "0..0",     0,     1,                   1 },
            { L_,  SW_01,     0,  "0..0",     0, BPS-1,          g64(SW_01) },
            { L_,  SW_01,     0,  "0..0",     0,   BPS,          g64(SW_01) },
            { L_,  SW_01,     0,  "0..0",     1,     0,                   0 },
            { L_,  SW_01,     0,  "0..0",     1,     1,                   2 },
            { L_,  SW_01,     0,  "0..0",     1, BPS-1,          g64(SW_10) },
            { L_,  SW_01,     0,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01,     0,  "0..0", BPS-1,     1,            int64Min },
            { L_,  SW_01,     0,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01,     0,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01,     0,  "1..1",     0,     1,              zero-2 },
            { L_,  SW_01,     0,  "1..1",     0, BPS-1,          g64(SW_10) },
            { L_,  SW_01,     0,  "1..1",     0,   BPS,          g64(SW_10) },
            { L_,  SW_01,     0,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01,     0,  "1..1",     1,     1,              zero-3 },
            { L_,  SW_01,     0,  "1..1",     1, BPS-1,          g64(SW_01) },
            { L_,  SW_01,     0,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01,     0,  "1..1", BPS-1,     1,            int64Max },
            { L_,  SW_01,     0,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01,     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     0,     1,        g64(SW_01)^1 },
            { L_,  SW_01,     0,   SW_01,     0, BPS-1,                   0 },
            { L_,  SW_01,     0,   SW_01,     0,   BPS,                   0 },
            { L_,  SW_01,     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     1,     1,        g64(SW_01)|2 },
            { L_,  SW_01,     0,   SW_01,     1, BPS-1,               ~zero },
            { L_,  SW_01,     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01, BPS-1,     1, g64(SW_01)|int64Min },
            { L_,  SW_01,     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01,     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     0,     1,        g64(SW_10)|1 },
            { L_,  SW_01,     0,   SW_10,     0, BPS-1,               ~zero },
            { L_,  SW_01,     0,   SW_10,     0,   BPS,               ~zero },
            { L_,  SW_01,     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     1,     1,        g64(SW_10)^2 },
            { L_,  SW_01,     0,   SW_10,     1, BPS-1,                   0 },
            { L_,  SW_01,     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_,  SW_01,     0,   SW_10,   BPS,     0,          g64(SW_10) },

            //L#    src    sIdx     dst    dIdx  nBits      xorEqual Result
            //--  ------    ---   ------   ----   ----  -------------------
            { L_,  SW_01,     0,    DSTA,     2,     4,g64("1101101110000") },
            { L_,  SW_01,     0,    DSTA,     2,     5,g64("1101100110000") },
            { L_,  SW_01,     0,    DSTA,     2,     6,g64("1101100110000") },
            { L_,  SW_01,     0,    DSTA,     2,     7,g64("1101000110000") },
            { L_,  SW_01,     0,    DSTA,     2,     8,g64("1101000110000") },
            { L_,  SW_01,     0,    DSTA,     3,     4,g64("1101101001100") },
            { L_,  SW_01,     0,    DSTA,     3,     5,g64("1101111001100") },
            { L_,  SW_01,     0,    DSTA,     3,     6,g64("1101111001100") },
            { L_,  SW_01,     0,    DSTA,     3,     7,g64("1100111001100") },
            { L_,  SW_01,     0,    DSTA,     3,     8,g64("1100111001100") },

            { L_,  SW_01,     0,    DSTB,    53,     4,g64("110110111000..0")},
            { L_,  SW_01,     0,    DSTB,    53,     5,g64("110110011000..0")},
            { L_,  SW_01,     0,    DSTB,    53,     6,g64("110110011000..0")},
            { L_,  SW_01,     0,    DSTB,    53,     7,g64("110100011000..0")},
            { L_,  SW_01,     0,    DSTB,    53,     8,g64("110100011000..0")},
            { L_,  SW_01,     0,    DSTB,    54,     4,g64("110110100110..0")},
            { L_,  SW_01,     0,    DSTB,    54,     5,g64("110111100110..0")},
            { L_,  SW_01,     0,    DSTB,    54,     6,g64("110111100110..0")},
            { L_,  SW_01,     0,    DSTB,    54,     7,g64("110011100110..0")},
            { L_,  SW_01,     0,    DSTB,    54,     8,g64("110011100110..0")},

            { L_,  SW_01,     1,  "0..0",     0,     0,                   0 },
            { L_,  SW_01,     1,  "0..0",     0,     1,                   0 },
            { L_,  SW_01,     1,  "0..0",     0, BPS-1, g64(SW_10)^int64Min },
            { L_,  SW_01,     1,  "0..0",     1,     0,                   0 },
            { L_,  SW_01,     1,  "0..0",     1,     1,                   0 },
            { L_,  SW_01,     1,  "0..0",     1, BPS-1,        g64(SW_01)^1 },
            { L_,  SW_01,     1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01,     1,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_01,     1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01,     1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01,     1,  "1..1",     0,     1,               ~zero },
            { L_,  SW_01,     1,  "1..1",     0, BPS-1, g64(SW_01)|int64Min },
            { L_,  SW_01,     1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01,     1,  "1..1",     1,     1,               ~zero },
            { L_,  SW_01,     1,  "1..1",     1, BPS-1,        g64(SW_10)|1 },
            { L_,  SW_01,     1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01,     1,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_01,     1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01,     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     0, BPS-1,            int64Max },
            { L_,  SW_01,     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     1, BPS-1,                   1 },
            { L_,  SW_01,     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01,     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     0, BPS-1,            int64Min },
            { L_,  SW_01,     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     1, BPS-1,              zero-2 },
            { L_,  SW_01,     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,   BPS,     0,          g64(SW_10) },

            //L#    src    sIdx     dst    dIdx  nBits      xorEqual Result
            //--  ------    ---   ------   ----   ----  -------------------

            { L_,  SW_01,     1,    DSTA,     2,     4,g64("1101101001100") },
            { L_,  SW_01,     1,    DSTA,     2,     5,g64("1101101001100") },
            { L_,  SW_01,     1,    DSTA,     2,     6,g64("1101111001100") },
            { L_,  SW_01,     1,    DSTA,     2,     7,g64("1101111001100") },
            { L_,  SW_01,     1,    DSTA,     2,     8,g64("1100111001100") },
            { L_,  SW_01,     1,    DSTA,     3,     4,g64("1101100110100") },
            { L_,  SW_01,     1,    DSTA,     3,     5,g64("1101100110100") },
            { L_,  SW_01,     1,    DSTA,     3,     6,g64("1101000110100") },
            { L_,  SW_01,     1,    DSTA,     3,     7,g64("1101000110100") },
            { L_,  SW_01,     1,    DSTA,     3,     8,g64("1111000110100") },

            { L_,  SW_01,     1,    DSTB,    53,     4,g64("110110100110..0")},
            { L_,  SW_01,     1,    DSTB,    53,     5,g64("110110100110..0")},
            { L_,  SW_01,     1,    DSTB,    53,     6,g64("110111100110..0")},
            { L_,  SW_01,     1,    DSTB,    53,     7,g64("110111100110..0")},
            { L_,  SW_01,     1,    DSTB,    53,     8,g64("110011100110..0")},
            { L_,  SW_01,     1,    DSTB,    54,     4,g64("110110011010..0")},
            { L_,  SW_01,     1,    DSTB,    54,     5,g64("110110011010..0")},
            { L_,  SW_01,     1,    DSTB,    54,     6,g64("110100011010..0")},
            { L_,  SW_01,     1,    DSTB,    54,     7,g64("110100011010..0")},
            { L_,  SW_01,     1,    DSTB,    54,     8,g64("111100011010..0")},

            { L_,  SW_01, BPS-1,  "0..0",     0,     0,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",     0,     1,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",     1,     0,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",     1,     1,                   0 },
            { L_,  SW_01, BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01, BPS-1,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01, BPS-1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",     0,     1,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",     1,     1,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01, BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01, BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,   BPS,  "0..0",     0,     0,                   0 },
            { L_,  SW_01,   BPS,  "0..0",     1,     0,                   0 },
            { L_,  SW_01,   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01,   BPS,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01,   BPS,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01,   BPS,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01,   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01,   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01,   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01,   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01,   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01,   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01,   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01,   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01,   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01,   BPS,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,     0,  "0..0",     0,     0,                   0 },
            { L_,  SW_10,     0,  "0..0",     0,     1,                   0 },
            { L_,  SW_10,     0,  "0..0",     0, BPS-1, g64(SW_10)^int64Min },
            { L_,  SW_10,     0,  "0..0",     0,   BPS,          g64(SW_10) },
            { L_,  SW_10,     0,  "0..0",     1,     0,                   0 },
            { L_,  SW_10,     0,  "0..0",     1,     1,                   0 },
            { L_,  SW_10,     0,  "0..0",     1, BPS-1,        g64(SW_01)^1 },
            { L_,  SW_10,     0,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10,     0,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_10,     0,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10,     0,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10,     0,  "1..1",     0,     1,               ~zero },
            { L_,  SW_10,     0,  "1..1",     0, BPS-1, g64(SW_01)|int64Min },
            { L_,  SW_10,     0,  "1..1",     0,   BPS,          g64(SW_01) },
            { L_,  SW_10,     0,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10,     0,  "1..1",     1,     1,               ~zero },
            { L_,  SW_10,     0,  "1..1",     1, BPS-1,        g64(SW_10)|1 },
            { L_,  SW_10,     0,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10,     0,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_10,     0,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10,     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     0, BPS-1,            int64Max },
            { L_,  SW_10,     0,   SW_01,     0,   BPS,               ~zero },
            { L_,  SW_10,     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     1, BPS-1,                   1 },
            { L_,  SW_10,     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10,     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     0, BPS-1,            int64Min },
            { L_,  SW_10,     0,   SW_10,     0,   BPS,                   0 },
            { L_,  SW_10,     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     1, BPS-1,              zero-2 },
            { L_,  SW_10,     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,   BPS,     0,          g64(SW_10) },

            //L#    src    sIdx     dst    dIdx  nBits      xorEqual Result
            //--  ------    ---   ------   ----   ----  -------------------

            { L_,  SW_10,     0,    DSTA,     2,     4,g64("1101101001100") },
            { L_,  SW_10,     0,    DSTA,     2,     5,g64("1101101001100") },
            { L_,  SW_10,     0,    DSTA,     2,     6,g64("1101111001100") },
            { L_,  SW_10,     0,    DSTA,     2,     7,g64("1101111001100") },
            { L_,  SW_10,     0,    DSTA,     2,     8,g64("1100111001100") },
            { L_,  SW_10,     0,    DSTA,     3,     4,g64("1101100110100") },
            { L_,  SW_10,     0,    DSTA,     3,     5,g64("1101100110100") },
            { L_,  SW_10,     0,    DSTA,     3,     6,g64("1101000110100") },
            { L_,  SW_10,     0,    DSTA,     3,     7,g64("1101000110100") },
            { L_,  SW_10,     0,    DSTA,     3,     8,g64("1111000110100") },

            { L_,  SW_10,     0,    DSTB,    53,     4,g64("110110100110..0")},
            { L_,  SW_10,     0,    DSTB,    53,     5,g64("110110100110..0")},
            { L_,  SW_10,     0,    DSTB,    53,     6,g64("110111100110..0")},
            { L_,  SW_10,     0,    DSTB,    53,     7,g64("110111100110..0")},
            { L_,  SW_10,     0,    DSTB,    53,     8,g64("110011100110..0")},
            { L_,  SW_10,     0,    DSTB,    54,     4,g64("110110011010..0")},
            { L_,  SW_10,     0,    DSTB,    54,     5,g64("110110011010..0")},
            { L_,  SW_10,     0,    DSTB,    54,     6,g64("110100011010..0")},
            { L_,  SW_10,     0,    DSTB,    54,     7,g64("110100011010..0")},
            { L_,  SW_10,     0,    DSTB,    54,     8,g64("111100011010..0")},

            { L_,  SW_10,     1,  "0..0",     0,     0,                   0 },
            { L_,  SW_10,     1,  "0..0",     0,     1,                   1 },
            { L_,  SW_10,     1,  "0..0",     0, BPS-1,          g64(SW_01) },
            { L_,  SW_10,     1,  "0..0",     1,     0,                   0 },
            { L_,  SW_10,     1,  "0..0",     1,     1,                   2 },
            { L_,  SW_10,     1,  "0..0",     1, BPS-1,          g64(SW_10) },
            { L_,  SW_10,     1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10,     1,  "0..0", BPS-1,     1,            int64Min },
            { L_,  SW_10,     1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10,     1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10,     1,  "1..1",     0,     1,              zero-2 },
            { L_,  SW_10,     1,  "1..1",     0, BPS-1,          g64(SW_10) },
            { L_,  SW_10,     1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10,     1,  "1..1",     1,     1,              zero-3 },
            { L_,  SW_10,     1,  "1..1",     1, BPS-1,          g64(SW_01) },
            { L_,  SW_10,     1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10,     1,  "1..1", BPS-1,     1,            int64Max },
            { L_,  SW_10,     1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10,     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     0,     1,        g64(SW_01)^1 },
            { L_,  SW_10,     1,   SW_01,     0, BPS-1,                   0 },
            { L_,  SW_10,     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     1,     1,        g64(SW_01)|2 },
            { L_,  SW_10,     1,   SW_01,     1, BPS-1,               ~zero },
            { L_,  SW_10,     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01, BPS-1,     1, g64(SW_01)|int64Min },
            { L_,  SW_10,     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10,     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     0,     1,        g64(SW_10)|1 },
            { L_,  SW_10,     1,   SW_10,     0, BPS-1,               ~zero },
            { L_,  SW_10,     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     1,     1,        g64(SW_10)^2 },
            { L_,  SW_10,     1,   SW_10,     1, BPS-1,                   0 },
            { L_,  SW_10,     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_,  SW_10,     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,     1,    DSTA,     2,     4,g64("1101101110000") },
            { L_,  SW_10,     1,    DSTA,     2,     5,g64("1101100110000") },
            { L_,  SW_10,     1,    DSTA,     2,     6,g64("1101100110000") },
            { L_,  SW_10,     1,    DSTA,     2,     7,g64("1101000110000") },
            { L_,  SW_10,     1,    DSTA,     2,     8,g64("1101000110000") },
            { L_,  SW_10,     1,    DSTA,     3,     4,g64("1101101001100") },
            { L_,  SW_10,     1,    DSTA,     3,     5,g64("1101111001100") },
            { L_,  SW_10,     1,    DSTA,     3,     6,g64("1101111001100") },
            { L_,  SW_10,     1,    DSTA,     3,     7,g64("1100111001100") },
            { L_,  SW_10,     1,    DSTA,     3,     8,g64("1100111001100") },

            { L_,  SW_10,     1,    DSTB,    53,     4,g64("110110111000..0")},
            { L_,  SW_10,     1,    DSTB,    53,     5,g64("110110011000..0")},
            { L_,  SW_10,     1,    DSTB,    53,     6,g64("110110011000..0")},
            { L_,  SW_10,     1,    DSTB,    53,     7,g64("110100011000..0")},
            { L_,  SW_10,     1,    DSTB,    53,     8,g64("110100011000..0")},
            { L_,  SW_10,     1,    DSTB,    54,     4,g64("110110100110..0")},
            { L_,  SW_10,     1,    DSTB,    54,     5,g64("110111100110..0")},
            { L_,  SW_10,     1,    DSTB,    54,     6,g64("110111100110..0")},
            { L_,  SW_10,     1,    DSTB,    54,     7,g64("110011100110..0")},
            { L_,  SW_10,     1,    DSTB,    54,     8,g64("110011100110..0")},

            { L_,  SW_10, BPS-1,  "0..0",     0,     0,                   0 },
            { L_,  SW_10, BPS-1,  "0..0",     0,     1,                   1 },
            { L_,  SW_10, BPS-1,  "0..0",     1,     0,                   0 },
            { L_,  SW_10, BPS-1,  "0..0",     1,     1,                   2 },
            { L_,  SW_10, BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10, BPS-1,  "0..0", BPS-1,     1,            int64Min },
            { L_,  SW_10, BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10, BPS-1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1",     0,     1,              zero-2 },
            { L_,  SW_10, BPS-1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1",     1,     1,              zero-3 },
            { L_,  SW_10, BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1", BPS-1,     1,            int64Max },
            { L_,  SW_10, BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10, BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01,     0,     1,        g64(SW_01)^1 },
            { L_,  SW_10, BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01,     1,     1,        g64(SW_01)^2 },
            { L_,  SW_10, BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01, BPS-1,     1, g64(SW_01)|int64Min },
            { L_,  SW_10, BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10, BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10,     0,     1,        g64(SW_10)|1 },
            { L_,  SW_10, BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10,     1,     1,        g64(SW_10)^2 },
            { L_,  SW_10, BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_,  SW_10, BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,   BPS,  "0..0",     0,     0,                   0 },
            { L_,  SW_10,   BPS,  "0..0",     1,     0,                   0 },
            { L_,  SW_10,   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10,   BPS,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10,   BPS,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10,   BPS,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10,   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10,   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10,   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10,   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10,   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10,   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10,   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10,   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10,   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10,   BPS,   SW_10,   BPS,     0,          g64(SW_10) }
        };
        const int NUM_DATA_B = sizeof DATA_B / sizeof *DATA_B;

        for (int di = 0; di < NUM_DATA_B; ++di) {
            const int   LINE     = DATA_B[di].d_lineNum;
            const char *spec     = DATA_B[di].d_dst;
            const int   srcIndex = DATA_B[di].d_sindex;

            uint64_t dst = g64(spec);
            uint64_t src = g64(DATA_B[di].d_src) >> srcIndex;

            if (veryVerbose) {
                P_(LINE); P_(spec); P_(DATA_B[di].d_xorEqual); P(dst);
            }

            dst = g64(spec);
            Util::xorEqBits(&dst, DATA_B[di].d_dindex, src,
                                                         DATA_B[di].d_numBits);
            LOOP_ASSERT(LINE, DATA_B[di].d_xorEqual == dst);
        }
      } break;
      case 7: {
        // --------------------------------------------------------------------
        // TESTING SETEQBITS FUNNCION
        //   Ensure the method has the expected effect on 'dstScalar'.
        //
        // Concerns:
        //: 1 That the function properly detects undefined behavior.
        //: 2 That the function has the correct effect on the '*dstScalar'
        //:   output.
        //
        // Plan:
        //: 1 Do negative testing.
        //: 2 Do table-driven testing.  Note that this used to be a 5-arg,
        //:   rather than a 4-arg, function, taking an 'sindex' operand that
        //:   right-shifted 'srcScalar' before applying.  We use the full
        //:   table by right-shifting 'src' ourselves in the test driver and
        //:   are thus able to make use of the old test cases.
        //
        // Testing:
        //   void setEqBits(uint64_t *dInt,
        //                  int       dIdx,
        //                  uint64_t  sInt,
        //                  int       nBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING SETEQBITS FUNNCION\n"
                          << "=========================\n";

        if (verbose) cout << "Negative Testing\n";
        {
            bsls::AssertTestHandlerGuard guard;

            uint64_t dst;

            ASSERT_SAFE_PASS(Util::setEqBits(&dst, 0, 20, 0));
            ASSERT_SAFE_PASS(Util::setEqBits(&dst, 10, 20, 0));
            ASSERT_SAFE_PASS(Util::setEqBits(&dst, 10, 20, 10));
            ASSERT_SAFE_PASS(Util::setEqBits(&dst, 10, -1LL,
                                                          BITS_PER_UINT64-10));

            ASSERT_SAFE_FAIL(Util::setEqBits(&dst, -1LL, 20, 0));
            ASSERT_SAFE_FAIL(Util::setEqBits(&dst,  0, 20, -1LL));
            ASSERT_SAFE_FAIL(Util::setEqBits(&dst, 10, 20, -1LL));
            ASSERT_SAFE_FAIL(Util::setEqBits(&dst, -1LL, 20, 10));
            ASSERT_SAFE_FAIL(Util::setEqBits(&dst, 10, -1LL,
                                                          BITS_PER_UINT64- 9));
        }

        if (verbose) cout << "Table-driven Testing\n";

        const char *DSTA = "1101101100100";    // typical case
        const char *VALA = "111011101110111";  // typical case

        if (verbose) cout << endl
                          << "Testing replaceValue64 Functions\n"
                          << "================================\n";

        static const struct {
            int         d_lineNum;  // line number
            const char *d_value;    // value
            const char *d_dst;      // destination integer
            int         d_index;    // destination index
            int         d_numBits;  // num of bits
            uint64_t    d_res;      // expected result (setValue/replaceValue)
        } DATA_B[] = {
            // L#   value      dst  Index  nBits                    result
            // --  ------   ------  -----  -----               -----------
            { L_,  "0..0",  "0..0",     0,     0,                        0 },
            { L_,  "0..0",  "0..0",     0,     1,                        0 },
            { L_,  "0..0",  "0..0",     0, BPS-1,                        0 },
            { L_,  "0..0",  "0..0",     0,   BPS,                        0 },
            { L_,  "0..0",  "0..0",     1,     0,                        0 },
            { L_,  "0..0",  "0..0",     1,     1,                        0 },
            { L_,  "0..0",  "0..0",     1, BPS-1,                        0 },
            { L_,  "0..0",  "0..0", BPS-1,     0,                        0 },
            { L_,  "0..0",  "0..0", BPS-1,     1,                        0 },
            { L_,  "0..0",  "0..0",   BPS,     0,                        0 },

            { L_,  "0..0",  "1..1",     0,     0,                    ~zero },
            { L_,  "0..0",  "1..1",     0,     1,                   zero-2 },
            { L_,  "0..0",  "1..1",     0, BPS-1,                 int64Min },
            { L_,  "0..0",  "1..1",     0,   BPS,                        0 },
            { L_,  "0..0",  "1..1",     1,     0,                    ~zero },
            { L_,  "0..0",  "1..1",     1,     1,                   zero-3 },
            { L_,  "0..0",  "1..1",     1, BPS-1,                        1 },
            { L_,  "0..0",  "1..1", BPS-1,     0,                    ~zero },
            { L_,  "0..0",  "1..1", BPS-1,     1,                 int64Max },
            { L_,  "0..0",  "1..1",   BPS,     0,                    ~zero },

            { L_,  "0..0",   SW_01,     0,     0,               g64(SW_01) },
            { L_,  "0..0",   SW_01,     0,     1,             g64(SW_01)^1 },
            { L_,  "0..0",   SW_01,     0, BPS-1,                        0 },
            { L_,  "0..0",   SW_01,     0,   BPS,                        0 },
            { L_,  "0..0",   SW_01,     1,     0,               g64(SW_01) },
            { L_,  "0..0",   SW_01,     1,     1,               g64(SW_01) },
            { L_,  "0..0",   SW_01,     1, BPS-1,                        1 },
            { L_,  "0..0",   SW_01, BPS-1,     0,               g64(SW_01) },
            { L_,  "0..0",   SW_01, BPS-1,     1,               g64(SW_01) },
            { L_,  "0..0",   SW_01,   BPS,     0,               g64(SW_01) },

            { L_,  "0..0",   SW_10,     0,     0,               g64(SW_10) },
            { L_,  "0..0",   SW_10,     0,     1,               g64(SW_10) },
            { L_,  "0..0",   SW_10,     0, BPS-1,                 int64Min },
            { L_,  "0..0",   SW_10,     0,   BPS,                        0 },
            { L_,  "0..0",   SW_10,     1,     0,               g64(SW_10) },
            { L_,  "0..0",   SW_10,     1,     1,             g64(SW_10)^2 },
            { L_,  "0..0",   SW_10,     1, BPS-1,                        0 },
            { L_,  "0..0",   SW_10, BPS-1,     0,               g64(SW_10) },
            { L_,  "0..0",   SW_10, BPS-1,     1,      g64(SW_10)&int64Max },
            { L_,  "0..0",   SW_10,   BPS,     0,               g64(SW_10) },

            // DSTA                                   g64("1101101100100")
            { L_,  "0..0",    DSTA,     2,     4,     g64("1101101000000") },
            { L_,  "0..0",    DSTA,     2,     5,     g64("1101100000000") },
            { L_,  "0..0",    DSTA,     2,     6,     g64("1101100000000") },
            { L_,  "0..0",    DSTA,     2,     7,     g64("1101000000000") },
            { L_,  "0..0",    DSTA,     2,     8,     g64("1100000000000") },
            { L_,  "0..0",    DSTA,     3,     4,     g64("1101100000100") },
            { L_,  "0..0",    DSTA,     3,     5,     g64("1101100000100") },
            { L_,  "0..0",    DSTA,     3,     6,     g64("1101000000100") },
            { L_,  "0..0",    DSTA,     3,     7,     g64("1100000000100") },
            { L_,  "0..0",    DSTA,     3,     8,     g64("1100000000100") },

            { L_,  "1..1",  "0..0",     0,     0,                        0 },
            { L_,  "1..1",  "0..0",     0,     1,                        1 },
            { L_,  "1..1",  "0..0",     0, BPS-1,                 int64Max },
            { L_,  "1..1",  "0..0",     0,   BPS,                    ~zero },
            { L_,  "1..1",  "0..0",     1,     0,                        0 },
            { L_,  "1..1",  "0..0",     1,     1,                        2 },
            { L_,  "1..1",  "0..0",     1, BPS-1,                   zero-2 },
            { L_,  "1..1",  "0..0", BPS-1,     0,                        0 },
            { L_,  "1..1",  "0..0", BPS-1,     1,                 int64Min },
            { L_,  "1..1",  "0..0",   BPS,     0,                        0 },

            { L_,  "1..1",  "1..1",     0,     0,                    ~zero },
            { L_,  "1..1",  "1..1",     0,     1,                    ~zero },
            { L_,  "1..1",  "1..1",     0, BPS-1,                    ~zero },
            { L_,  "1..1",  "1..1",     0,   BPS,                    ~zero },
            { L_,  "1..1",  "1..1",     1,     0,                    ~zero },
            { L_,  "1..1",  "1..1",     1,     1,                    ~zero },
            { L_,  "1..1",  "1..1",     1, BPS-1,                    ~zero },
            { L_,  "1..1",  "1..1", BPS-1,     0,                    ~zero },
            { L_,  "1..1",  "1..1", BPS-1,     1,                    ~zero },
            { L_,  "1..1",  "1..1",   BPS,     0,                    ~zero },

            { L_,  "1..1",   SW_01,     0,     0,               g64(SW_01) },
            { L_,  "1..1",   SW_01,     0,     1,               g64(SW_01) },
            { L_,  "1..1",   SW_01,     0, BPS-1,                 int64Max },
            { L_,  "1..1",   SW_01,     0,   BPS,                    ~zero },
            { L_,  "1..1",   SW_01,     1,     0,               g64(SW_01) },
            { L_,  "1..1",   SW_01,     1,     1,             g64(SW_01)|2 },
            { L_,  "1..1",   SW_01,     1, BPS-1,                    ~zero },
            { L_,  "1..1",   SW_01, BPS-1,     0,               g64(SW_01) },
            { L_,  "1..1",   SW_01, BPS-1,     1,      g64(SW_01)|int64Min },
            { L_,  "1..1",   SW_01,   BPS,     0,               g64(SW_01) },

            { L_,  "1..1",   SW_10,     0,     0,               g64(SW_10) },
            { L_,  "1..1",   SW_10,     0,     1,             g64(SW_10)|1 },
            { L_,  "1..1",   SW_10,     0, BPS-1,                    ~zero },
            { L_,  "1..1",   SW_10,     0,   BPS,                    ~zero },
            { L_,  "1..1",   SW_10,     1,     0,               g64(SW_10) },
            { L_,  "1..1",   SW_10,     1,     1,               g64(SW_10) },
            { L_,  "1..1",   SW_10,     1, BPS-1,                   zero-2 },
            { L_,  "1..1",   SW_10, BPS-1,     0,               g64(SW_10) },
            { L_,  "1..1",   SW_10, BPS-1,     1,               g64(SW_10) },
            { L_,  "1..1",   SW_10,   BPS,     0,               g64(SW_10) },

            // DSTA                                   g64("1101101100100")
            { L_,  "1..1",    DSTA,     2,     4,     g64("1101101111100") },
            { L_,  "1..1",    DSTA,     2,     5,     g64("1101101111100") },
            { L_,  "1..1",    DSTA,     2,     6,     g64("1101111111100") },
            { L_,  "1..1",    DSTA,     2,     7,     g64("1101111111100") },
            { L_,  "1..1",    DSTA,     2,     8,     g64("1101111111100") },
            { L_,  "1..1",    DSTA,     3,     4,     g64("1101101111100") },
            { L_,  "1..1",    DSTA,     3,     5,     g64("1101111111100") },
            { L_,  "1..1",    DSTA,     3,     6,     g64("1101111111100") },
            { L_,  "1..1",    DSTA,     3,     7,     g64("1101111111100") },
            { L_,  "1..1",    DSTA,     3,     8,     g64("1111111111100") },

            { L_,   SW_01,  "0..0",     0,     0,                        0 },
            { L_,   SW_01,  "0..0",     0,     1,                        1 },
            { L_,   SW_01,  "0..0",     0, BPS-1,               g64(SW_01) },
            { L_,   SW_01,  "0..0",     0,   BPS,               g64(SW_01) },
            { L_,   SW_01,  "0..0",     1,     0,                        0 },
            { L_,   SW_01,  "0..0",     1,     1,                        2 },
            { L_,   SW_01,  "0..0",     1, BPS-1,               g64(SW_10) },
            { L_,   SW_01,  "0..0", BPS-1,     0,                        0 },
            { L_,   SW_01,  "0..0", BPS-1,     1,                 int64Min },
            { L_,   SW_01,  "0..0",   BPS,     0,                        0 },

            { L_,   SW_01,  "1..1",     0,     0,                    ~zero },
            { L_,   SW_01,  "1..1",     0,     1,                    ~zero },
            { L_,   SW_01,  "1..1",     0, BPS-1,      g64(SW_01)|int64Min },
            { L_,   SW_01,  "1..1",     0,   BPS,               g64(SW_01) },
            { L_,   SW_01,  "1..1",     1,     0,                    ~zero },
            { L_,   SW_01,  "1..1",     1,     1,                    ~zero },
            { L_,   SW_01,  "1..1",     1, BPS-1,             g64(SW_10)|1 },
            { L_,   SW_01,  "1..1", BPS-1,     0,                    ~zero },
            { L_,   SW_01,  "1..1", BPS-1,     1,                    ~zero },
            { L_,   SW_01,  "1..1",   BPS,     0,                    ~zero },

            { L_,   SW_01,   SW_01,     0,     0,               g64(SW_01) },
            { L_,   SW_01,   SW_01,     0,     1,               g64(SW_01) },
            { L_,   SW_01,   SW_01,     0, BPS-1,               g64(SW_01) },
            { L_,   SW_01,   SW_01,     0,   BPS,               g64(SW_01) },
            { L_,   SW_01,   SW_01,     1,     0,               g64(SW_01) },
            { L_,   SW_01,   SW_01,     1,     1,             g64(SW_01)|2 },
            { L_,   SW_01,   SW_01,     1, BPS-1,             g64(SW_10)|1 },
            { L_,   SW_01,   SW_01, BPS-1,     0,               g64(SW_01) },
            { L_,   SW_01,   SW_01, BPS-1,     1,      g64(SW_01)|int64Min },
            { L_,   SW_01,   SW_01,   BPS,     0,               g64(SW_01) },

            { L_,   SW_01,   SW_10,     0,     0,               g64(SW_10) },
            { L_,   SW_01,   SW_10,     0,     1,             g64(SW_10)|1 },
            { L_,   SW_01,   SW_10,     0, BPS-1,      g64(SW_01)|int64Min },
            { L_,   SW_01,   SW_10,     0,   BPS,               g64(SW_01) },
            { L_,   SW_01,   SW_10,     1,     0,               g64(SW_10) },
            { L_,   SW_01,   SW_10,     1,     1,               g64(SW_10) },
            { L_,   SW_01,   SW_10,     1, BPS-1,               g64(SW_10) },
            { L_,   SW_01,   SW_10, BPS-1,     0,               g64(SW_10) },
            { L_,   SW_01,   SW_10, BPS-1,     1,               g64(SW_10) },
            { L_,   SW_01,   SW_10,   BPS,     0,               g64(SW_10) },

            // DSTA                                   g64("1101101100100")
            { L_,   SW_01,    DSTA,     2,     4,     g64("1101101010100") },
            { L_,   SW_01,    DSTA,     2,     5,     g64("1101101010100") },
            { L_,   SW_01,    DSTA,     2,     6,     g64("1101101010100") },
            { L_,   SW_01,    DSTA,     2,     7,     g64("1101101010100") },
            { L_,   SW_01,    DSTA,     2,     8,     g64("1100101010100") },
            { L_,   SW_01,    DSTA,     3,     4,     g64("1101100101100") },
            { L_,   SW_01,    DSTA,     3,     5,     g64("1101110101100") },
            { L_,   SW_01,    DSTA,     3,     6,     g64("1101010101100") },
            { L_,   SW_01,    DSTA,     3,     7,     g64("1101010101100") },
            { L_,   SW_01,    DSTA,     3,     8,     g64("1101010101100") },

            { L_,   SW_10,  "0..0",     0,     0,                        0 },
            { L_,   SW_10,  "0..0",     0,     1,                        0 },
            { L_,   SW_10,  "0..0",     0, BPS-1,      g64(SW_10)^int64Min },
            { L_,   SW_10,  "0..0",     0,   BPS,               g64(SW_10) },
            { L_,   SW_10,  "0..0",     1,     0,                        0 },
            { L_,   SW_10,  "0..0",     1,     1,                        0 },
            { L_,   SW_10,  "0..0",     1, BPS-1,             g64(SW_01)^1 },
            { L_,   SW_10,  "0..0", BPS-1,     0,                        0 },
            { L_,   SW_10,  "0..0", BPS-1,     1,                        0 },
            { L_,   SW_10,  "0..0",   BPS,     0,                        0 },

            { L_,   SW_10,  "1..1",     0,     0,                    ~zero },
            { L_,   SW_10,  "1..1",     0,     1,                   zero-2 },
            { L_,   SW_10,  "1..1",     0, BPS-1,               g64(SW_10) },
            { L_,   SW_10,  "1..1",     0,   BPS,               g64(SW_10) },
            { L_,   SW_10,  "1..1",     1,     0,                    ~zero },
            { L_,   SW_10,  "1..1",     1,     1,                   zero-3 },
            { L_,   SW_10,  "1..1",     1, BPS-1,               g64(SW_01) },
            { L_,   SW_10,  "1..1", BPS-1,     0,                    ~zero },
            { L_,   SW_10,  "1..1", BPS-1,     1,                 int64Max },
            { L_,   SW_10,  "1..1",   BPS,     0,                    ~zero },

            { L_,   SW_10,   SW_01,     0,     0,               g64(SW_01) },
            { L_,   SW_10,   SW_01,     0,     1,             g64(SW_01)^1 },
            { L_,   SW_10,   SW_01,     0, BPS-1,      g64(SW_10)^int64Min },
            { L_,   SW_10,   SW_01,     0,   BPS,               g64(SW_10) },
            { L_,   SW_10,   SW_01,     1,     0,               g64(SW_01) },
            { L_,   SW_10,   SW_01,     1,     1,               g64(SW_01) },
            { L_,   SW_10,   SW_01,     1, BPS-1,               g64(SW_01) },
            { L_,   SW_10,   SW_01, BPS-1,     0,               g64(SW_01) },
            { L_,   SW_10,   SW_01, BPS-1,     1,               g64(SW_01) },
            { L_,   SW_10,   SW_01,   BPS,     0,               g64(SW_01) },

            { L_,   SW_10,   SW_10,     0,     0,               g64(SW_10) },
            { L_,   SW_10,   SW_10,     0,     1,               g64(SW_10) },
            { L_,   SW_10,   SW_10,     0, BPS-1,               g64(SW_10) },
            { L_,   SW_10,   SW_10,     0,   BPS,               g64(SW_10) },
            { L_,   SW_10,   SW_10,     1,     0,               g64(SW_10) },
            { L_,   SW_10,   SW_10,     1,     1,             g64(SW_10)^2 },
            { L_,   SW_10,   SW_10,     1, BPS-1,             g64(SW_01)^1 },
            { L_,   SW_10,   SW_10, BPS-1,     0,               g64(SW_10) },
            { L_,   SW_10,   SW_10, BPS-1,     1,      g64(SW_10)^int64Min },
            { L_,   SW_10,   SW_10,   BPS,     0,               g64(SW_10) },

            // DSTA                                   g64("1101101100100")
            { L_,   SW_10,    DSTA,     2,     4,     g64("1101101101000") },
            { L_,   SW_10,    DSTA,     2,     5,     g64("1101100101000") },
            { L_,   SW_10,    DSTA,     2,     6,     g64("1101110101000") },
            { L_,   SW_10,    DSTA,     2,     7,     g64("1101010101000") },
            { L_,   SW_10,    DSTA,     2,     8,     g64("1101010101000") },
            { L_,   SW_10,    DSTA,     3,     4,     g64("1101101010100") },
            { L_,   SW_10,    DSTA,     3,     5,     g64("1101101010100") },
            { L_,   SW_10,    DSTA,     3,     6,     g64("1101101010100") },
            { L_,   SW_10,    DSTA,     3,     7,     g64("1100101010100") },
            { L_,   SW_10,    DSTA,     3,     8,     g64("1110101010100") },

            // VALA    =                           g64("0111011101110111")
            { L_,    VALA,  "0..0",     0,     0,                        0 },
            { L_,    VALA,  "0..0",     0,     1,                        1 },
            { L_,    VALA,  "0..0",     0, BPS-1,                g64(VALA) },
            { L_,    VALA,  "0..0",     0,   BPS,                g64(VALA) },
            { L_,    VALA,  "0..0",     1,     0,                        0 },
            { L_,    VALA,  "0..0",     1,     1,                        2 },
            { L_,    VALA,  "0..0",     1, BPS-1,  g64("1110111011101110") },
            { L_,    VALA,  "0..0", BPS-1,     0,                        0 },
            { L_,    VALA,  "0..0", BPS-1,     1,                 int64Min },
            { L_,    VALA,  "0..0",   BPS,     0,                        0 },

            { L_,    VALA,  "1..1",     0,     0,                    ~zero },
            { L_,    VALA,  "1..1",     0,     1,                    ~zero },
            { L_,    VALA,  "1..1",     0, BPS-1,g64("10..0111011101110111") },
            { L_,    VALA,  "1..1",     0,   BPS,                g64(VALA) },
            { L_,    VALA,  "1..1",     1,     0,                    ~zero },
            { L_,    VALA,  "1..1",     1,     1,                    ~zero },
            { L_,    VALA,  "1..1",     1, BPS-1,  g64("1110111011101111") },
            { L_,    VALA,  "1..1", BPS-1,     0,                    ~zero },
            { L_,    VALA,  "1..1", BPS-1,     1,                    ~zero },
            { L_,    VALA,  "1..1",   BPS,     0,                    ~zero },

            { L_,    VALA,   SW_01,     0,     0,               g64(SW_01) },
            { L_,    VALA,   SW_01,     0,     1,               g64(SW_01) },
            { L_,    VALA,   SW_01,     0, BPS-1,                g64(VALA) },
            { L_,    VALA,   SW_01,     0,   BPS,                g64(VALA) },
            { L_,    VALA,   SW_01,     1,     0,               g64(SW_01) },
            { L_,    VALA,   SW_01,     1,     1,             g64(SW_01)|2 },
            { L_,    VALA,   SW_01,     1, BPS-1,  g64("1110111011101111") },
            { L_,    VALA,   SW_01, BPS-1,     0,               g64(SW_01) },
            { L_,    VALA,   SW_01, BPS-1,     1,      g64(SW_01)|int64Min },
            { L_,    VALA,   SW_01,   BPS,     0,               g64(SW_01) },

            { L_,    VALA,   SW_10,     0,     0,               g64(SW_10) },
            { L_,    VALA,   SW_10,     0,     1,             g64(SW_10)|1 },
            { L_,    VALA,   SW_10,     0, BPS-1,g64("10..0111011101110111")},
            { L_,    VALA,   SW_10,     0,   BPS,                g64(VALA) },
            { L_,    VALA,   SW_10,     1,     0,               g64(SW_10) },
            { L_,    VALA,   SW_10,     1,     1,               g64(SW_10) },
            { L_,    VALA,   SW_10,     1, BPS-1,  g64("1110111011101110") },
            { L_,    VALA,   SW_10, BPS-1,     0,               g64(SW_10) },
            { L_,    VALA,   SW_10, BPS-1,     1,               g64(SW_10) },
            { L_,    VALA,   SW_10,   BPS,     0,               g64(SW_10) },

            // VALA    =                           g64("0111011101110111")
            // DSTA    =                           g64("0001101101100100")
            { L_,    VALA,    DSTA,     2,     4,  g64("0001101101011100") },
            { L_,    VALA,    DSTA,     2,     5,  g64("0001101101011100") },
            { L_,    VALA,    DSTA,     2,     6,  g64("0001101111011100") },
            { L_,    VALA,    DSTA,     2,     7,  g64("0001101111011100") },
            { L_,    VALA,    DSTA,     2,     8,  g64("0001100111011100") },
            { L_,    VALA,    DSTA,     3,     4,  g64("0001101100111100") },
            { L_,    VALA,    DSTA,     3,     5,  g64("0001101110111100") },
            { L_,    VALA,    DSTA,     3,     6,  g64("0001101110111100") },
            { L_,    VALA,    DSTA,     3,     7,  g64("0001101110111100") },
            { L_,    VALA,    DSTA,     3,     8,  g64("0001101110111100") },
        };
        const int NUM_DATA_B = sizeof DATA_B / sizeof *DATA_B;

        for (int di = 0; di < NUM_DATA_B; ++di) {
            const int   LINE     = DATA_B[di].d_lineNum;
            const char *spec     = DATA_B[di].d_dst;
            const char *value    = DATA_B[di].d_value;

            uint64_t dst = g64(spec);
            uint64_t val = g64(value);
            Util::setEqBits(&dst, DATA_B[di].d_index, val,
                                                         DATA_B[di].d_numBits);

            if (veryVerbose) {
                P_(LINE); P_(spec); P_(DATA_B[di].d_res); P(dst);
            }

            LOOP_ASSERT(LINE, DATA_B[di].d_res == dst);
        }
      } break;
      case 6: {
        // --------------------------------------------------------------------
        // TESTING OREQBITS FUNCTION
        //   Ensure the method has the expected effect on 'dstScalar'.
        //
        // Concerns:
        //: 1 That the function properly detects undefined behavior.
        //: 2 That the function has the correct effect on the '*dstScalar'
        //:   output.
        //
        // Plan:
        //: 1 Do negative testing.
        //: 2 Do table-driven testing.  Note that this used to be a 5-arg,
        //:   rather than a 4-arg, function, taking an 'sindex' operand that
        //:   right-shifted 'srcScalar' before applying.  We use the full
        //:   table by right-shifting 'src' ourselves in the test driver and
        //:   are thus able to make use of the old test cases.
        //
        // Testing:
        //   void orEqBits(uint64_t *dInt, int dIdx,
        //                 uint64_t sInt, int nBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "Testing orEqBits Function" << endl
                          << "=========================" << endl;

        if (verbose) cout << "Negative Testing\n";
        {
            bsls::AssertTestHandlerGuard guard;

            uint64_t dst;

            ASSERT_SAFE_PASS(Util::orEqBits(&dst, 0, 20, 0));
            ASSERT_SAFE_PASS(Util::orEqBits(&dst, 10, 20, 0));
            ASSERT_SAFE_PASS(Util::orEqBits(&dst, 10, 20, 10));
            ASSERT_SAFE_PASS(Util::orEqBits(&dst, 10, -1LL,
                                                          BITS_PER_UINT64-10));

            ASSERT_SAFE_FAIL(Util::orEqBits(&dst, -1LL, 20, 0));
            ASSERT_SAFE_FAIL(Util::orEqBits(&dst,  0, 20, -1LL));
            ASSERT_SAFE_FAIL(Util::orEqBits(&dst, 10, 20, -1LL));
            ASSERT_SAFE_FAIL(Util::orEqBits(&dst, -1LL, 20, 10));
            ASSERT_SAFE_FAIL(Util::orEqBits(&dst, 10, -1LL,
                                                          BITS_PER_UINT64- 9));
        }

        if (verbose) cout << "Table-driven Testing\n";

        const char *DSTA = "1101101100100";    // typical case
        const char *DSTB = "110110110010..0";  // typical case

        const char *DSTC = "110110110010..0";  // typical case
        const int   DCF  = 51;     // DSTC floor -- offset of DSTA offsets to
                                   // convert them to DSTC offsets
        static const struct {
            int         d_lineNum;   // line number
            const char *d_src;       // source integer
            int         d_sindex;    // source index
            const char *d_dst;       // destination integer
            int         d_dindex;    // destination index
            int         d_numBits;   // num of bits
            uint64_t    d_orEqual;   // expected result from orEqual function
        } DATA_B[] = {
            //L#    src     sIdx   dst    dIdx   nBits        orEqual Result
            //--  ------    ----  ------  -----  -----  ---------------------

            { L_, "0..0",     0,  "0..0",     0,     0,                   0 },
            { L_, "0..0",     0,  "0..0",     0,     1,                   0 },
            { L_, "0..0",     0,  "0..0",     0, BPS-1,                   0 },
            { L_, "0..0",     0,  "0..0",     0,   BPS,                   0 },
            { L_, "0..0",     0,  "0..0",     1,     0,                   0 },
            { L_, "0..0",     0,  "0..0",     1,     1,                   0 },
            { L_, "0..0",     0,  "0..0",     1, BPS-1,                   0 },
            { L_, "0..0",     0,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0",     0,  "0..0", BPS-1,     1,                   0 },
            { L_, "0..0",     0,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0",     0,  "1..1",     0,     0,               ~zero },
            { L_, "0..0",     0,  "1..1",     0,     1,               ~zero },
            { L_, "0..0",     0,  "1..1",     0, BPS-1,               ~zero },
            { L_, "0..0",     0,  "1..1",     0,   BPS,               ~zero },
            { L_, "0..0",     0,  "1..1",     1,     0,               ~zero },
            { L_, "0..0",     0,  "1..1",     1,     1,               ~zero },
            { L_, "0..0",     0,  "1..1",     1, BPS-1,               ~zero },
            { L_, "0..0",     0,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0",     0,  "1..1", BPS-1,     1,               ~zero },
            { L_, "0..0",     0,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0",     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     0,   BPS,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0",     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     0,   BPS,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "0..0",     0,    DSTA,     2,     4,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     2,     5,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     2,     6,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     2,     7,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     2,     8,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     4,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     5,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     6,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     7,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     8,g64("1101101100100") },

            //L#    src     sIdx   dst    dIdx   nBits        orEqual Result
            //--  ------    ----  ------  -----  -----  ---------------------
            // DSTC                                        "110110110010..0"

            { L_, "0..0",     0,    DSTC, DCF+2,     4,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTC, DCF+2,     5,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTC, DCF+2,     6,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTC, DCF+2,     7,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTC, DCF+2,     8,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTC, DCF+3,     4,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTC, DCF+3,     5,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTC, DCF+3,     6,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTC, DCF+3,     7,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTC, DCF+3,     8,g64("110110110010..0")},

            { L_, "0..0",     1,  "0..0",     0,     0,                   0 },
            { L_, "0..0",     1,  "0..0",     0,     1,                   0 },
            { L_, "0..0",     1,  "0..0",     0, BPS-1,                   0 },
            { L_, "0..0",     1,  "0..0",     1,     0,                   0 },
            { L_, "0..0",     1,  "0..0",     1,     1,                   0 },
            { L_, "0..0",     1,  "0..0",     1, BPS-1,                   0 },
            { L_, "0..0",     1,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0",     1,  "0..0", BPS-1,     1,                   0 },
            { L_, "0..0",     1,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0",     1,  "1..1",     0,     0,               ~zero },
            { L_, "0..0",     1,  "1..1",     0,     1,               ~zero },
            { L_, "0..0",     1,  "1..1",     0, BPS-1,               ~zero },
            { L_, "0..0",     1,  "1..1",     1,     0,               ~zero },
            { L_, "0..0",     1,  "1..1",     1,     1,               ~zero },
            { L_, "0..0",     1,  "1..1",     1, BPS-1,               ~zero },
            { L_, "0..0",     1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0",     1,  "1..1", BPS-1,     1,               ~zero },
            { L_, "0..0",     1,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0",     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0",     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "0..0",     1,    DSTA,     2,     4,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     2,     5,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     2,     6,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     2,     7,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     2,     8,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     4,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     5,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     6,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     7,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     8,g64("1101101100100") },

            //L#    src     sIdx   dst    dIdx   nBits        orEqual Result
            //--  ------    ----  ------  -----  -----  ---------------------
            // DSTC                                        "110110110010..0"

            { L_, "0..0",     1,    DSTC, DCF+2,     4,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTC, DCF+2,     5,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTC, DCF+2,     6,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTC, DCF+2,     7,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTC, DCF+2,     8,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTC, DCF+3,     4,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTC, DCF+3,     5,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTC, DCF+3,     6,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTC, DCF+3,     7,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTC, DCF+3,     8,g64("110110110010..0")},

            { L_, "0..0", BPS-1,  "0..0",     0,     0,                   0 },
            { L_, "0..0", BPS-1,  "0..0",     0,     1,                   0 },
            { L_, "0..0", BPS-1,  "0..0",     1,     0,                   0 },
            { L_, "0..0", BPS-1,  "0..0",     1,     1,                   0 },
            { L_, "0..0", BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0", BPS-1,  "0..0", BPS-1,     1,                   0 },
            { L_, "0..0", BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0", BPS-1,  "1..1",     0,     0,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",     0,     1,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",     1,     0,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",     1,     1,               ~zero },
            { L_, "0..0", BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0", BPS-1,  "1..1", BPS-1,     1,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0", BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0", BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "0..0",   BPS,  "0..0",     0,     0,                   0 },
            { L_, "0..0",   BPS,  "0..0",     1,     0,                   0 },
            { L_, "0..0",   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0",   BPS,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0",   BPS,  "1..1",     0,     0,               ~zero },
            { L_, "0..0",   BPS,  "1..1",     1,     0,               ~zero },
            { L_, "0..0",   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0",   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0",   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0",   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0",   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0",   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0",   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0",   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0",   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0",   BPS,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",     0,  "0..0",     0,     0,                   0 },
            { L_, "1..1",     0,  "0..0",     0,     1,                   1 },
            { L_, "1..1",     0,  "0..0",     0, BPS-1,            int64Max },
            { L_, "1..1",     0,  "0..0",     0,   BPS,               ~zero },
            { L_, "1..1",     0,  "0..0",     1,     0,                   0 },
            { L_, "1..1",     0,  "0..0",     1,     1,              one<<1 },
            { L_, "1..1",     0,  "0..0",     1, BPS-1,              zero-2 },
            { L_, "1..1",     0,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1",     0,  "0..0", BPS-1,     1,            int64Min },
            { L_, "1..1",     0,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1",     0,  "1..1",     0,     0,               ~zero },
            { L_, "1..1",     0,  "1..1",     0,     1,               ~zero },
            { L_, "1..1",     0,  "1..1",     0, BPS-1,               ~zero },
            { L_, "1..1",     0,  "1..1",     0,   BPS,               ~zero },
            { L_, "1..1",     0,  "1..1",     1,     0,               ~zero },
            { L_, "1..1",     0,  "1..1",     1,     1,               ~zero },
            { L_, "1..1",     0,  "1..1",     1, BPS-1,               ~zero },
            { L_, "1..1",     0,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1",     0,  "1..1", BPS-1,     1,               ~zero },
            { L_, "1..1",     0,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1",     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     0, BPS-1,            int64Max },
            { L_, "1..1",     0,   SW_01,     0,   BPS,               ~zero },
            { L_, "1..1",     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     1,     1,
                                                      g64(SW_01) | (one<<1) },
            { L_, "1..1",     0,   SW_01,     1, BPS-1,               ~zero },
            { L_, "1..1",     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01, BPS-1,     1, g64(SW_01)^int64Min },
            { L_, "1..1",     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1",     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     0,     1,      g64(SW_10) | 1 },
            { L_, "1..1",     0,   SW_10,     0, BPS-1,               ~zero },
            { L_, "1..1",     0,   SW_10,     0,   BPS,               ~zero },
            { L_, "1..1",     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     1, BPS-1,              zero-2 },
            { L_, "1..1",     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",     0,    DSTA,     2,     4,g64("1101101111100") },
            { L_, "1..1",     0,    DSTA,     2,     5,g64("1101101111100") },
            { L_, "1..1",     0,    DSTA,     2,     6,g64("1101111111100") },
            { L_, "1..1",     0,    DSTA,     2,     7,g64("1101111111100") },
            { L_, "1..1",     0,    DSTA,     2,     8,g64("1101111111100") },
            { L_, "1..1",     0,    DSTA,     3,     4,g64("1101101111100") },
            { L_, "1..1",     0,    DSTA,     3,     5,g64("1101111111100") },
            { L_, "1..1",     0,    DSTA,     3,     6,g64("1101111111100") },
            { L_, "1..1",     0,    DSTA,     3,     7,g64("1101111111100") },
            { L_, "1..1",     0,    DSTA,     3,     8,g64("1111111111100") },

            //L#    src     sIdx   dst    dIdx   nBits        orEqual Result
            //--  ------    ----  ------  -----  -----  ---------------------
            // DSTC                                        "110110110010..0"

            { L_, "1..1",     0,    DSTC, DCF+2,     4,g64("110110111110..0")},
            { L_, "1..1",     0,    DSTC, DCF+2,     5,g64("110110111110..0")},
            { L_, "1..1",     0,    DSTC, DCF+2,     6,g64("110111111110..0")},
            { L_, "1..1",     0,    DSTC, DCF+2,     7,g64("110111111110..0")},
            { L_, "1..1",     0,    DSTC, DCF+2,     8,g64("110111111110..0")},
            { L_, "1..1",     0,    DSTC, DCF+3,     4,g64("110110111110..0")},
            { L_, "1..1",     0,    DSTC, DCF+3,     5,g64("110111111110..0")},
            { L_, "1..1",     0,    DSTC, DCF+3,     6,g64("110111111110..0")},
            { L_, "1..1",     0,    DSTC, DCF+3,     7,g64("110111111110..0")},
            { L_, "1..1",     0,    DSTC, DCF+3,     8,g64("111111111110..0")},

            { L_, "1..1",     1,  "0..0",     0,     0,                   0 },
            { L_, "1..1",     1,  "0..0",     0,     1,                   1 },
            { L_, "1..1",     1,  "0..0",     0, BPS-1,            int64Max },
            { L_, "1..1",     1,  "0..0",     1,     0,                   0 },
            { L_, "1..1",     1,  "0..0",     1,     1,              one<<1 },
            { L_, "1..1",     1,  "0..0",     1, BPS-1,              zero-2 },
            { L_, "1..1",     1,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1",     1,  "0..0", BPS-1,     1,            int64Min },
            { L_, "1..1",     1,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1",     1,  "1..1",     0,     0,               ~zero },
            { L_, "1..1",     1,  "1..1",     0,     1,               ~zero },
            { L_, "1..1",     1,  "1..1",     0, BPS-1,               ~zero },
            { L_, "1..1",     1,  "1..1",     1,     0,               ~zero },
            { L_, "1..1",     1,  "1..1",     1,     1,               ~zero },
            { L_, "1..1",     1,  "1..1",     1, BPS-1,               ~zero },
            { L_, "1..1",     1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1",     1,  "1..1", BPS-1,     1,               ~zero },
            { L_, "1..1",     1,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1",     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     0, BPS-1,            int64Max },
            { L_, "1..1",     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     1,     1,
                                                      g64(SW_01) | (one<<1) },
            { L_, "1..1",     1,   SW_01,     1, BPS-1,               ~zero },
            { L_, "1..1",     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01, BPS-1,     1, g64(SW_01)^int64Min },
            { L_, "1..1",     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1",     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     0,     1,      g64(SW_10) | 1 },
            { L_, "1..1",     1,   SW_10,     0, BPS-1,               ~zero },
            { L_, "1..1",     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     1, BPS-1,              zero-2 },
            { L_, "1..1",     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",     1,    DSTA,     2,     4,g64("1101101111100") },
            { L_, "1..1",     1,    DSTA,     2,     5,g64("1101101111100") },
            { L_, "1..1",     1,    DSTA,     2,     6,g64("1101111111100") },
            { L_, "1..1",     1,    DSTA,     2,     7,g64("1101111111100") },
            { L_, "1..1",     1,    DSTA,     2,     8,g64("1101111111100") },
            { L_, "1..1",     1,    DSTA,     3,     4,g64("1101101111100") },
            { L_, "1..1",     1,    DSTA,     3,     5,g64("1101111111100") },
            { L_, "1..1",     1,    DSTA,     3,     6,g64("1101111111100") },
            { L_, "1..1",     1,    DSTA,     3,     7,g64("1101111111100") },
            { L_, "1..1",     1,    DSTA,     3,     8,g64("1111111111100") },

            //L#    src     sIdx   dst    dIdx   nBits        orEqual Result
            //--  ------    ----  ------  -----  -----  ---------------------
            // DSTC                                        "110110110010..0"

            { L_, "1..1",     1,    DSTC, DCF+2,     4,g64("110110111110..0")},
            { L_, "1..1",     1,    DSTC, DCF+2,     5,g64("110110111110..0")},
            { L_, "1..1",     1,    DSTC, DCF+2,     6,g64("110111111110..0")},
            { L_, "1..1",     1,    DSTC, DCF+2,     7,g64("110111111110..0")},
            { L_, "1..1",     1,    DSTC, DCF+2,     8,g64("110111111110..0")},
            { L_, "1..1",     1,    DSTC, DCF+3,     4,g64("110110111110..0")},
            { L_, "1..1",     1,    DSTC, DCF+3,     5,g64("110111111110..0")},
            { L_, "1..1",     1,    DSTC, DCF+3,     6,g64("110111111110..0")},
            { L_, "1..1",     1,    DSTC, DCF+3,     7,g64("110111111110..0")},
            { L_, "1..1",     1,    DSTC, DCF+3,     8,g64("111111111110..0")},

            { L_, "1..1", BPS-1,  "0..0",     0,     0,                   0 },
            { L_, "1..1", BPS-1,  "0..0",     0,     1,                   1 },
            { L_, "1..1", BPS-1,  "0..0",     1,     0,                   0 },
            { L_, "1..1", BPS-1,  "0..0",     1,     1,                   2 },
            { L_, "1..1", BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1", BPS-1,  "0..0", BPS-1,     1,            int64Min },
            { L_, "1..1", BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1", BPS-1,  "1..1",     0,     0,               ~zero },
            { L_, "1..1", BPS-1,  "1..1",     0,     1,               ~zero },
            { L_, "1..1", BPS-1,  "1..1",     1,     0,               ~zero },
            { L_, "1..1", BPS-1,  "1..1",     1,     1,               ~zero },
            { L_, "1..1", BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1", BPS-1,  "1..1", BPS-1,     1,               ~zero },
            { L_, "1..1", BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1", BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01,     1,     1,       g64(SW_01) | 2},
            { L_, "1..1", BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01, BPS-1,     1, g64(SW_01)^int64Min },
            { L_, "1..1", BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1", BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10,     0,     1,      g64(SW_10) | 1 },
            { L_, "1..1", BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",   BPS,  "0..0",     0,     0,                   0 },
            { L_, "1..1",   BPS,  "0..0",     1,     0,                   0 },
            { L_, "1..1",   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1",   BPS,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1",   BPS,  "1..1",     0,     0,               ~zero },
            { L_, "1..1",   BPS,  "1..1",     1,     0,               ~zero },
            { L_, "1..1",   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1",   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1",   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1",   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1",   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1",   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1",   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1",   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1",   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1",   BPS,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,     0,  "0..0",     0,     0,                   0 },
            { L_,  SW_01,     0,  "0..0",     0,     1,                   1 },
            { L_,  SW_01,     0,  "0..0",     0, BPS-1,          g64(SW_01) },
            { L_,  SW_01,     0,  "0..0",     0,   BPS,          g64(SW_01) },
            { L_,  SW_01,     0,  "0..0",     1,     0,                   0 },
            { L_,  SW_01,     0,  "0..0",     1,     1,              one<<1 },
            { L_,  SW_01,     0,  "0..0",     1, BPS-1,          g64(SW_10) },
            { L_,  SW_01,     0,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01,     0,  "0..0", BPS-1,     1,            int64Min },
            { L_,  SW_01,     0,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01,     0,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01,     0,  "1..1",     0,     1,               ~zero },
            { L_,  SW_01,     0,  "1..1",     0, BPS-1,               ~zero },
            { L_,  SW_01,     0,  "1..1",     0,   BPS,               ~zero },
            { L_,  SW_01,     0,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01,     0,  "1..1",     1,     1,               ~zero },
            { L_,  SW_01,     0,  "1..1",     1, BPS-1,               ~zero },
            { L_,  SW_01,     0,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01,     0,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_01,     0,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01,     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     0,   BPS,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     1,     1,
                                                      g64(SW_01) | (one<<1) },
            { L_,  SW_01,     0,   SW_01,     1, BPS-1,               ~zero },
            { L_,  SW_01,     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01, BPS-1,     1, g64(SW_01)^int64Min },
            { L_,  SW_01,     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01,     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     0,     1,      g64(SW_10) | 1 },
            { L_,  SW_01,     0,   SW_10,     0, BPS-1,               ~zero },
            { L_,  SW_01,     0,   SW_10,     0,   BPS,               ~zero },
            { L_,  SW_01,     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,     0,    DSTA,     2,     4,g64("1101101110100") },
            { L_,  SW_01,     0,    DSTA,     2,     5,g64("1101101110100") },
            { L_,  SW_01,     0,    DSTA,     2,     6,g64("1101101110100") },
            { L_,  SW_01,     0,    DSTA,     2,     7,g64("1101101110100") },
            { L_,  SW_01,     0,    DSTA,     2,     8,g64("1101101110100") },
            { L_,  SW_01,     0,    DSTA,     3,     4,g64("1101101101100") },
            { L_,  SW_01,     0,    DSTA,     3,     5,g64("1101111101100") },
            { L_,  SW_01,     0,    DSTA,     3,     6,g64("1101111101100") },
            { L_,  SW_01,     0,    DSTA,     3,     7,g64("1101111101100") },
            { L_,  SW_01,     0,    DSTA,     3,     8,g64("1101111101100") },

            //L#    src     sIdx   dst    dIdx   nBits        orEqual Result
            //--  ------    ----  ------  -----  -----  ---------------------
            // DSTC                                        "110110110010..0"

            { L_,  SW_01,     0,    DSTC, DCF+2,     4,g64("110110111010..0")},
            { L_,  SW_01,     0,    DSTC, DCF+2,     5,g64("110110111010..0")},
            { L_,  SW_01,     0,    DSTC, DCF+2,     6,g64("110110111010..0")},
            { L_,  SW_01,     0,    DSTC, DCF+2,     7,g64("110110111010..0")},
            { L_,  SW_01,     0,    DSTC, DCF+2,     8,g64("110110111010..0")},
            { L_,  SW_01,     0,    DSTC, DCF+3,     4,g64("110110110110..0")},
            { L_,  SW_01,     0,    DSTC, DCF+3,     5,g64("110111110110..0")},
            { L_,  SW_01,     0,    DSTC, DCF+3,     6,g64("110111110110..0")},
            { L_,  SW_01,     0,    DSTC, DCF+3,     7,g64("110111110110..0")},
            { L_,  SW_01,     0,    DSTC, DCF+3,     8,g64("110111110110..0")},

            { L_,  SW_01,     1,  "0..0",     0,     0,                   0 },
            { L_,  SW_01,     1,  "0..0",     0,     1,                   0 },
            { L_,  SW_01,     1,  "0..0",     0, BPS-1, g64(SW_10)^int64Min },
            { L_,  SW_01,     1,  "0..0",     1,     0,                   0 },
            { L_,  SW_01,     1,  "0..0",     1,     1,                   0 },
            { L_,  SW_01,     1,  "0..0",     1, BPS-1,      g64(SW_01) ^ 1 },
            { L_,  SW_01,     1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01,     1,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_01,     1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01,     1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01,     1,  "1..1",     0,     1,               ~zero },
            { L_,  SW_01,     1,  "1..1",     0, BPS-1,               ~zero },
            { L_,  SW_01,     1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01,     1,  "1..1",     1,     1,               ~zero },
            { L_,  SW_01,     1,  "1..1",     1, BPS-1,               ~zero },
            { L_,  SW_01,     1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01,     1,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_01,     1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01,     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     0, BPS-1,            int64Max },
            { L_,  SW_01,     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01,     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     1, BPS-1,              zero-2 },
            { L_,  SW_01,     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,     1,    DSTA,     2,     4,g64("1101101101100") },
            { L_,  SW_01,     1,    DSTA,     2,     5,g64("1101101101100") },
            { L_,  SW_01,     1,    DSTA,     2,     6,g64("1101111101100") },
            { L_,  SW_01,     1,    DSTA,     2,     7,g64("1101111101100") },
            { L_,  SW_01,     1,    DSTA,     2,     8,g64("1101111101100") },
            { L_,  SW_01,     1,    DSTA,     3,     4,g64("1101101110100") },
            { L_,  SW_01,     1,    DSTA,     3,     5,g64("1101101110100") },
            { L_,  SW_01,     1,    DSTA,     3,     6,g64("1101101110100") },
            { L_,  SW_01,     1,    DSTA,     3,     7,g64("1101101110100") },
            { L_,  SW_01,     1,    DSTA,     3,     8,g64("1111101110100") },

            //L#    src     sIdx   dst    dIdx   nBits        orEqual Result
            //--  ------    ----  ------  -----  -----  ---------------------
            // DSTC                                        "110110110010..0"

            { L_,  SW_01,     1,    DSTC, DCF+2,     4,g64("110110110110..0")},
            { L_,  SW_01,     1,    DSTC, DCF+2,     5,g64("110110110110..0")},
            { L_,  SW_01,     1,    DSTC, DCF+2,     6,g64("110111110110..0")},
            { L_,  SW_01,     1,    DSTC, DCF+2,     7,g64("110111110110..0")},
            { L_,  SW_01,     1,    DSTC, DCF+2,     8,g64("110111110110..0")},
            { L_,  SW_01,     1,    DSTC, DCF+3,     4,g64("110110111010..0")},
            { L_,  SW_01,     1,    DSTC, DCF+3,     5,g64("110110111010..0")},
            { L_,  SW_01,     1,    DSTC, DCF+3,     6,g64("110110111010..0")},
            { L_,  SW_01,     1,    DSTC, DCF+3,     7,g64("110110111010..0")},
            { L_,  SW_01,     1,    DSTC, DCF+3,     8,g64("111110111010..0")},

            { L_,  SW_01, BPS-1,  "0..0",     0,     0,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",     0,     1,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",     1,     0,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",     1,     1,                   0 },
            { L_,  SW_01, BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01, BPS-1,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01, BPS-1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",     0,     1,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",     1,     1,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01, BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01, BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,   BPS,  "0..0",     0,     0,                   0 },
            { L_,  SW_01,   BPS,  "0..0",     1,     0,                   0 },
            { L_,  SW_01,   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01,   BPS,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01,   BPS,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01,   BPS,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01,   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01,   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01,   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01,   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01,   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01,   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01,   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01,   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01,   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01,   BPS,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,     0,  "0..0",     0,     0,                   0 },
            { L_,  SW_10,     0,  "0..0",     0,     1,                   0 },
            { L_,  SW_10,     0,  "0..0",     0, BPS-1, g64(SW_10)^int64Min },
            { L_,  SW_10,     0,  "0..0",     0,   BPS,          g64(SW_10) },
            { L_,  SW_10,     0,  "0..0",     1,     0,                   0 },
            { L_,  SW_10,     0,  "0..0",     1,     1,                   0 },
            { L_,  SW_10,     0,  "0..0",     1, BPS-1,      g64(SW_01) ^ 1 },
            { L_,  SW_10,     0,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10,     0,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_10,     0,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10,     0,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10,     0,  "1..1",     0,     1,               ~zero },
            { L_,  SW_10,     0,  "1..1",     0, BPS-1,               ~zero },
            { L_,  SW_10,     0,  "1..1",     0,   BPS,               ~zero },
            { L_,  SW_10,     0,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10,     0,  "1..1",     1,     1,               ~zero },
            { L_,  SW_10,     0,  "1..1",     1, BPS-1,               ~zero },
            { L_,  SW_10,     0,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10,     0,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_10,     0,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10,     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     0, BPS-1,            int64Max },
            { L_,  SW_10,     0,   SW_01,     0,   BPS,               ~zero },
            { L_,  SW_10,     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10,     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     0,   BPS,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     1, BPS-1,              zero-2 },
            { L_,  SW_10,     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,     0,    DSTA,     2,     4,g64("1101101101100") },
            { L_,  SW_10,     0,    DSTA,     2,     5,g64("1101101101100") },
            { L_,  SW_10,     0,    DSTA,     2,     6,g64("1101111101100") },
            { L_,  SW_10,     0,    DSTA,     2,     7,g64("1101111101100") },
            { L_,  SW_10,     0,    DSTA,     2,     8,g64("1101111101100") },
            { L_,  SW_10,     0,    DSTA,     3,     4,g64("1101101110100") },
            { L_,  SW_10,     0,    DSTA,     3,     5,g64("1101101110100") },
            { L_,  SW_10,     0,    DSTA,     3,     6,g64("1101101110100") },
            { L_,  SW_10,     0,    DSTA,     3,     7,g64("1101101110100") },
            { L_,  SW_10,     0,    DSTA,     3,     8,g64("1111101110100") },

            //L#    src     sIdx   dst    dIdx   nBits        orEqual Result
            //--  ------    ----  ------  -----  -----  ---------------------
            // DSTC                                        "110110110010..0"

            { L_,  SW_10,     0,    DSTC, DCF+2,     4,g64("110110110110..0")},
            { L_,  SW_10,     0,    DSTC, DCF+2,     5,g64("110110110110..0")},
            { L_,  SW_10,     0,    DSTC, DCF+2,     6,g64("110111110110..0")},
            { L_,  SW_10,     0,    DSTC, DCF+2,     7,g64("110111110110..0")},
            { L_,  SW_10,     0,    DSTC, DCF+2,     8,g64("110111110110..0")},
            { L_,  SW_10,     0,    DSTC, DCF+3,     4,g64("110110111010..0")},
            { L_,  SW_10,     0,    DSTC, DCF+3,     5,g64("110110111010..0")},
            { L_,  SW_10,     0,    DSTC, DCF+3,     6,g64("110110111010..0")},
            { L_,  SW_10,     0,    DSTC, DCF+3,     7,g64("110110111010..0")},
            { L_,  SW_10,     0,    DSTC, DCF+3,     8,g64("111110111010..0")},

            { L_,  SW_10,     1,  "0..0",     0,     0,                   0 },
            { L_,  SW_10,     1,  "0..0",     0,     1,                   1 },
            { L_,  SW_10,     1,  "0..0",     0, BPS-1,          g64(SW_01) },
            { L_,  SW_10,     1,  "0..0",     1,     0,                   0 },
            { L_,  SW_10,     1,  "0..0",     1,     1,              one<<1 },
            { L_,  SW_10,     1,  "0..0",     1, BPS-1,          g64(SW_10) },
            { L_,  SW_10,     1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10,     1,  "0..0", BPS-1,     1,            int64Min },
            { L_,  SW_10,     1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10,     1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10,     1,  "1..1",     0,     1,               ~zero },
            { L_,  SW_10,     1,  "1..1",     0, BPS-1,               ~zero },
            { L_,  SW_10,     1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10,     1,  "1..1",     1,     1,               ~zero },
            { L_,  SW_10,     1,  "1..1",     1, BPS-1,               ~zero },
            { L_,  SW_10,     1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10,     1,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_10,     1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10,     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     1,     1,
                                                      g64(SW_01) | (one<<1) },
            { L_,  SW_10,     1,   SW_01,     1, BPS-1,               ~zero },
            { L_,  SW_10,     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01, BPS-1,     1, g64(SW_01)^int64Min },
            { L_,  SW_10,     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10,     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     0,     1,      g64(SW_10) | 1 },
            { L_,  SW_10,     1,   SW_10,     0, BPS-1,               ~zero },
            { L_,  SW_10,     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,     1,    DSTA,     2,     4,g64("1101101110100") },
            { L_,  SW_10,     1,    DSTA,     2,     5,g64("1101101110100") },
            { L_,  SW_10,     1,    DSTA,     2,     6,g64("1101101110100") },
            { L_,  SW_10,     1,    DSTA,     2,     7,g64("1101101110100") },
            { L_,  SW_10,     1,    DSTA,     2,     8,g64("1101101110100") },
            { L_,  SW_10,     1,    DSTA,     3,     4,g64("1101101101100") },
            { L_,  SW_10,     1,    DSTA,     3,     5,g64("1101111101100") },
            { L_,  SW_10,     1,    DSTA,     3,     6,g64("1101111101100") },
            { L_,  SW_10,     1,    DSTA,     3,     7,g64("1101111101100") },
            { L_,  SW_10,     1,    DSTA,     3,     8,g64("1101111101100") },

            //L#    src     sIdx   dst    dIdx   nBits        orEqual Result
            //--  ------    ----  ------  -----  -----  ---------------------
            // DSTC                                        "110110110010..0"

            { L_,  SW_10,     1,    DSTC, DCF+2,     4,g64("110110111010..0")},
            { L_,  SW_10,     1,    DSTC, DCF+2,     5,g64("110110111010..0")},
            { L_,  SW_10,     1,    DSTC, DCF+2,     6,g64("110110111010..0")},
            { L_,  SW_10,     1,    DSTC, DCF+2,     7,g64("110110111010..0")},
            { L_,  SW_10,     1,    DSTC, DCF+2,     8,g64("110110111010..0")},
            { L_,  SW_10,     1,    DSTC, DCF+3,     4,g64("110110110110..0")},
            { L_,  SW_10,     1,    DSTC, DCF+3,     5,g64("110111110110..0")},
            { L_,  SW_10,     1,    DSTC, DCF+3,     6,g64("110111110110..0")},
            { L_,  SW_10,     1,    DSTC, DCF+3,     7,g64("110111110110..0")},
            { L_,  SW_10,     1,    DSTB, DCF+3,     8,g64("110111110110..0")},

            { L_,  SW_10, BPS-1,  "0..0",     0,     0,                   0 },
            { L_,  SW_10, BPS-1,  "0..0",     0,     1,                   1 },
            { L_,  SW_10, BPS-1,  "0..0",     1,     0,                   0 },
            { L_,  SW_10, BPS-1,  "0..0",     1,     1,                   2 },
            { L_,  SW_10, BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10, BPS-1,  "0..0", BPS-1,     1,            int64Min },
            { L_,  SW_10, BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10, BPS-1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1",     0,     1,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1",     1,     1,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10, BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01,     1,     1,      g64(SW_01) | 2 },
            { L_,  SW_10, BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01, BPS-1,     1, g64(SW_01)^int64Min },
            { L_,  SW_10, BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10, BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10,     0,     1,      g64(SW_10) | 1 },
            { L_,  SW_10, BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,   BPS,  "0..0",     0,     0,                   0 },
            { L_,  SW_10,   BPS,  "0..0",     1,     0,                   0 },
            { L_,  SW_10,   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10,   BPS,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10,   BPS,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10,   BPS,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10,   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10,   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10,   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10,   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10,   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10,   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10,   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10,   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10,   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10,   BPS,   SW_10,   BPS,     0,          g64(SW_10) }
        };
        const int NUM_DATA_B = sizeof DATA_B / sizeof *DATA_B;

        for (int di = 0; di < NUM_DATA_B; ++di) {
            const int   LINE     = DATA_B[di].d_lineNum;
            const char *spec     = DATA_B[di].d_dst;
            const int   srcIndex = DATA_B[di].d_sindex;

            uint64_t dst = g64(spec);
            uint64_t src = g64(DATA_B[di].d_src) >> srcIndex;

            if (veryVerbose) {
                P_(LINE); P_(spec); P_(DATA_B[di].d_orEqual); P(dst);
            }

            dst = g64(spec);
            Util::orEqBits(&dst, DATA_B[di].d_dindex, src,
                                                         DATA_B[di].d_numBits);
            LOOP_ASSERT(LINE, DATA_B[di].d_orEqual == dst);
        }
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // TESTING 'minusEqBits' FUNCTION:
        //   Ensure the method has the expected effect on 'dstScalar'.
        //
        // Concerns:
        //: 1 That the function properly detects undefined behavior.
        //: 2 That the function has the correct effect on the '*dstScalar'
        //:   output.
        //
        // Plan:
        //: 1 Do negative testing.
        //: 2 Do table-driven testing.  Note that this used to be a 5-arg,
        //:   rather than a 4-arg, function, taking an 'sindex' operand that
        //:   right-shifted 'srcScalar' before applying.  We use the full
        //:   table by right-shifting 'src' ourselves in the test driver and
        //:   are thus able to make use of the old test cases.
        //
        // Testing:
        //   void minusEqBits(int *dInt, int dIdx,int sInt,int nBits);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING 'minusEqBits' FUNCTION:" << endl
                          << "===============================" << endl;

        if (verbose) cout << "Negative Testing\n";
        {
            bsls::AssertTestHandlerGuard guard;

            uint64_t dst;

            ASSERT_SAFE_PASS(Util::minusEqBits(&dst, 0, 20, 0));
            ASSERT_SAFE_PASS(Util::minusEqBits(&dst, 10, 20, 0));
            ASSERT_SAFE_PASS(Util::minusEqBits(&dst, 10, 20, 10));
            ASSERT_SAFE_PASS(Util::minusEqBits(&dst, 10, -1LL,
                                                          BITS_PER_UINT64-10));

            ASSERT_SAFE_FAIL(Util::minusEqBits(&dst, -1LL, 20, 0));
            ASSERT_SAFE_FAIL(Util::minusEqBits(&dst,  0, 20, -1LL));
            ASSERT_SAFE_FAIL(Util::minusEqBits(&dst, 10, 20, -1LL));
            ASSERT_SAFE_FAIL(Util::minusEqBits(&dst, -1LL, 20, 10));
            ASSERT_SAFE_FAIL(Util::minusEqBits(&dst, 10, -1LL,
                                                          BITS_PER_UINT64- 9));
        }

        if (verbose) cout << "Table-driven Testing\n";

        const char *DSTA = "1101101100100";    // typical case
        const char *DSTB = "110110110010..0";    // typical case

        static const struct {
            int         d_lineNum;     // line number
            const char *d_src;         // source integer
            int         d_sindex;      // source index
            const char *d_dst;         // destination integer
            int         d_dindex;      // destination index
            int         d_numBits;     // num of bits
            uint64_t    d_minusEqual;  // expected result from minusEqual
        } DATA_B[] = {
            //L#      src   sIdx   dst     dIdx   nBits   minusEqual Result
            //--   ------   ----   -----   ----   -----   -----------------
            { L_, "0..0",     0,  "0..0",     0,     0,                   0 },
            { L_, "0..0",     0,  "0..0",     0,     1,                   0 },
            { L_, "0..0",     0,  "0..0",     0, BPS-1,                   0 },
            { L_, "0..0",     0,  "0..0",     0,   BPS,                   0 },
            { L_, "0..0",     0,  "0..0",     1,     0,                   0 },
            { L_, "0..0",     0,  "0..0",     1,     1,                   0 },
            { L_, "0..0",     0,  "0..0",     1, BPS-1,                   0 },
            { L_, "0..0",     0,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0",     0,  "0..0", BPS-1,     1,                   0 },
            { L_, "0..0",     0,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0",     0,  "1..1",     0,     0,               ~zero },
            { L_, "0..0",     0,  "1..1",     0,     1,               ~zero },
            { L_, "0..0",     0,  "1..1",     0, BPS-1,               ~zero },
            { L_, "0..0",     0,  "1..1",     0,   BPS,               ~zero },
            { L_, "0..0",     0,  "1..1",     1,     0,               ~zero },
            { L_, "0..0",     0,  "1..1",     1,     1,               ~zero },
            { L_, "0..0",     0,  "1..1",     1, BPS-1,               ~zero },
            { L_, "0..0",     0,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0",     0,  "1..1", BPS-1,     1,               ~zero },
            { L_, "0..0",     0,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0",     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     0,   BPS,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0",     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     0,   BPS,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "0..0",     0,    DSTA,     2,     4,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     2,     5,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     2,     6,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     2,     7,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     2,     8,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     4,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     5,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     6,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     7,g64("1101101100100") },
            { L_, "0..0",     0,    DSTA,     3,     8,g64("1101101100100") },

            //L#      src   sIdx   dst     dIdx   nBits   minusEqual Result
            //--   ------   ----   -----   ----   -----   -----------------

            //                                  // DSTB =  "110110110010..0"
            { L_, "0..0",     0,    DSTB,    53,     4,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    53,     5,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    53,     6,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    53,     7,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    53,     8,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    54,     4,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    54,     5,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    54,     6,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    54,     7,g64("110110110010..0")},
            { L_, "0..0",     0,    DSTB,    54,     8,g64("110110110010..0")},

            { L_, "0..0",     1,  "0..0",     0,     0,                   0 },
            { L_, "0..0",     1,  "0..0",     0,     1,                   0 },
            { L_, "0..0",     1,  "0..0",     0, BPS-1,                   0 },
            { L_, "0..0",     1,  "0..0",     1,     0,                   0 },
            { L_, "0..0",     1,  "0..0",     1,     1,                   0 },
            { L_, "0..0",     1,  "0..0",     1, BPS-1,                   0 },
            { L_, "0..0",     1,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0",     1,  "0..0", BPS-1,     1,                   0 },
            { L_, "0..0",     1,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0",     1,  "1..1",     0,     0,               ~zero },
            { L_, "0..0",     1,  "1..1",     0,     1,               ~zero },
            { L_, "0..0",     1,  "1..1",     0, BPS-1,               ~zero },
            { L_, "0..0",     1,  "1..1",     1,     0,               ~zero },
            { L_, "0..0",     1,  "1..1",     1,     1,               ~zero },
            { L_, "0..0",     1,  "1..1",     1, BPS-1,               ~zero },
            { L_, "0..0",     1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0",     1,  "1..1", BPS-1,     1,               ~zero },
            { L_, "0..0",     1,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0",     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0",     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "0..0",     1,    DSTA,     2,     4,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     2,     5,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     2,     6,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     2,     7,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     2,     8,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     4,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     5,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     6,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     7,g64("1101101100100") },
            { L_, "0..0",     1,    DSTA,     3,     8,g64("1101101100100") },

            //L#      src   sIdx   dst     dIdx   nBits   minusEqual Result
            //--   ------   ----   -----   ----   -----   -----------------

            //                                  // DSTB =  "110110110010..0"
            { L_, "0..0",     1,    DSTB,    53,     4,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTB,    53,     5,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTB,    53,     6,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTB,    53,     7,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTB,    53,     8,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTB,    54,     4,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTB,    54,     5,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTB,    54,     6,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTB,    54,     7,g64("110110110010..0")},
            { L_, "0..0",     1,    DSTB,    54,     8,g64("110110110010..0")},

            { L_, "0..0", BPS-1,  "0..0",     0,     0,                   0 },
            { L_, "0..0", BPS-1,  "0..0",     0,     1,                   0 },
            { L_, "0..0", BPS-1,  "0..0",     1,     0,                   0 },
            { L_, "0..0", BPS-1,  "0..0",     1,     1,                   0 },
            { L_, "0..0", BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0", BPS-1,  "0..0", BPS-1,     1,                   0 },
            { L_, "0..0", BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0", BPS-1,  "1..1",     0,     0,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",     0,     1,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",     1,     0,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",     1,     1,               ~zero },
            { L_, "0..0", BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0", BPS-1,  "1..1", BPS-1,     1,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0", BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0", BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "0..0",   BPS,  "0..0",     0,     0,                   0 },
            { L_, "0..0",   BPS,  "0..0",     1,     0,                   0 },
            { L_, "0..0",   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0",   BPS,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0",   BPS,  "1..1",     0,     0,               ~zero },
            { L_, "0..0",   BPS,  "1..1",     1,     0,               ~zero },
            { L_, "0..0",   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0",   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0",   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0",   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0",   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0",   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0",   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0",   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0",   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0",   BPS,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",     0,  "0..0",     0,     0,                   0 },
            { L_, "1..1",     0,  "0..0",     0,     1,                   0 },
            { L_, "1..1",     0,  "0..0",     0, BPS-1,                   0 },
            { L_, "1..1",     0,  "0..0",     0,   BPS,                   0 },
            { L_, "1..1",     0,  "0..0",     1,     0,                   0 },
            { L_, "1..1",     0,  "0..0",     1,     1,                   0 },
            { L_, "1..1",     0,  "0..0",     1, BPS-1,                   0 },
            { L_, "1..1",     0,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1",     0,  "0..0", BPS-1,     1,                   0 },
            { L_, "1..1",     0,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1",     0,  "1..1",     0,     0,               ~zero },
            { L_, "1..1",     0,  "1..1",     0,     1,              zero-2 },
            { L_, "1..1",     0,  "1..1",     0, BPS-1,            int64Min },
            { L_, "1..1",     0,  "1..1",     0,   BPS,                   0 },
            { L_, "1..1",     0,  "1..1",     1,     0,               ~zero },
            { L_, "1..1",     0,  "1..1",     1,     1,              zero-3 },
            { L_, "1..1",     0,  "1..1",     1, BPS-1,                   1 },
            { L_, "1..1",     0,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1",     0,  "1..1", BPS-1,     1,            int64Max },
            { L_, "1..1",     0,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1",     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     0,     1,        g64(SW_01)^1 },
            { L_, "1..1",     0,   SW_01,     0, BPS-1,                   0 },
            { L_, "1..1",     0,   SW_01,     0,   BPS,                   0 },
            { L_, "1..1",     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     1, BPS-1,                   1 },
            { L_, "1..1",     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1",     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     0, BPS-1,            int64Min },
            { L_, "1..1",     0,   SW_10,     0,   BPS,                   0 },
            { L_, "1..1",     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     1,     1,        g64(SW_10)^2 },
            { L_, "1..1",     0,   SW_10,     1, BPS-1,                   0 },
            { L_, "1..1",     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_, "1..1",     0,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",     0,    DSTA,     2,     4,g64("1101101000000") },
            { L_, "1..1",     0,    DSTA,     2,     5,g64("1101100000000") },
            { L_, "1..1",     0,    DSTA,     2,     6,g64("1101100000000") },
            { L_, "1..1",     0,    DSTA,     2,     7,g64("1101000000000") },
            { L_, "1..1",     0,    DSTA,     2,     8,g64("1100000000000") },
            { L_, "1..1",     0,    DSTA,     3,     4,g64("1101100000100") },
            { L_, "1..1",     0,    DSTA,     3,     5,g64("1101100000100") },
            { L_, "1..1",     0,    DSTA,     3,     6,g64("1101000000100") },
            { L_, "1..1",     0,    DSTA,     3,     7,g64("1100000000100") },
            { L_, "1..1",     0,    DSTA,     3,     8,g64("1100000000100") },

            //L#      src   sIdx   dst     dIdx   nBits   minusEqual Result
            //--   ------   ----   -----   ----   -----   -----------------
            //                                  // DSTB =  "110110110010..0"

            { L_, "1..1",     0,    DSTB,    53,     4,g64("110110100000..0")},
            { L_, "1..1",     0,    DSTB,    53,     5,g64("110110000000..0")},
            { L_, "1..1",     0,    DSTB,    53,     6,g64("110110000000..0")},
            { L_, "1..1",     0,    DSTB,    53,     7,g64("110100000000..0")},
            { L_, "1..1",     0,    DSTB,    53,     8,g64("110000000000..0")},
            { L_, "1..1",     0,    DSTB,    54,     4,g64("110110000010..0")},
            { L_, "1..1",     0,    DSTB,    54,     5,g64("110110000010..0")},
            { L_, "1..1",     0,    DSTB,    54,     6,g64("110100000010..0")},
            { L_, "1..1",     0,    DSTB,    54,     7,g64("110000000010..0")},
            { L_, "1..1",     0,    DSTB,    54,     8,g64("110000000010..0")},

            { L_, "1..1",     1,  "0..0",     0,     0,                   0 },
            { L_, "1..1",     1,  "0..0",     0,     1,                   0 },
            { L_, "1..1",     1,  "0..0",     0, BPS-1,                   0 },
            { L_, "1..1",     1,  "0..0",     1,     0,                   0 },
            { L_, "1..1",     1,  "0..0",     1,     1,                   0 },
            { L_, "1..1",     1,  "0..0",     1, BPS-1,                   0 },
            { L_, "1..1",     1,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1",     1,  "0..0", BPS-1,     1,                   0 },
            { L_, "1..1",     1,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1",     1,  "1..1",     0,     0,               ~zero },
            { L_, "1..1",     1,  "1..1",     0,     1,              zero-2 },
            { L_, "1..1",     1,  "1..1",     0, BPS-1,            int64Min },
            { L_, "1..1",     1,  "1..1",     1,     0,               ~zero },
            { L_, "1..1",     1,  "1..1",     1,     1,              zero-3 },
            { L_, "1..1",     1,  "1..1",     1, BPS-1,                   1 },
            { L_, "1..1",     1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1",     1,  "1..1", BPS-1,     1,            int64Max },
            { L_, "1..1",     1,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1",     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     0,     1,        g64(SW_01)^1 },
            { L_, "1..1",     1,   SW_01,     0, BPS-1,                   0 },
            { L_, "1..1",     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     1, BPS-1,                   1 },
            { L_, "1..1",     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1",     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     0, BPS-1,            int64Min },
            { L_, "1..1",     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     1,     1,        g64(SW_10)^2 },
            { L_, "1..1",     1,   SW_10,     1, BPS-1,                   0 },
            { L_, "1..1",     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_, "1..1",     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",     1,    DSTA,     2,     4,g64("1101101000000") },
            { L_, "1..1",     1,    DSTA,     2,     5,g64("1101100000000") },
            { L_, "1..1",     1,    DSTA,     2,     6,g64("1101100000000") },
            { L_, "1..1",     1,    DSTA,     2,     7,g64("1101000000000") },
            { L_, "1..1",     1,    DSTA,     2,     8,g64("1100000000000") },
            { L_, "1..1",     1,    DSTA,     3,     4,g64("1101100000100") },
            { L_, "1..1",     1,    DSTA,     3,     5,g64("1101100000100") },
            { L_, "1..1",     1,    DSTA,     3,     6,g64("1101000000100") },
            { L_, "1..1",     1,    DSTA,     3,     7,g64("1100000000100") },
            { L_, "1..1",     1,    DSTA,     3,     8,g64("1100000000100") },

            //L#      src   sIdx   dst     dIdx   nBits   minusEqual Result
            //--   ------   ----   -----   ----   -----   -----------------
            //                                  // DSTB =  "110110110010..0"

            { L_, "1..1",     1,    DSTB,    53,     4,g64("110110100000..0")},
            { L_, "1..1",     1,    DSTB,    53,     5,g64("110110000000..0")},
            { L_, "1..1",     1,    DSTB,    53,     6,g64("110110000000..0")},
            { L_, "1..1",     1,    DSTB,    53,     7,g64("110100000000..0")},
            { L_, "1..1",     1,    DSTB,    53,     8,g64("110000000000..0")},
            { L_, "1..1",     1,    DSTB,    54,     4,g64("110110000010..0")},
            { L_, "1..1",     1,    DSTB,    54,     5,g64("110110000010..0")},
            { L_, "1..1",     1,    DSTB,    54,     6,g64("110100000010..0")},
            { L_, "1..1",     1,    DSTB,    54,     7,g64("110000000010..0")},
            { L_, "1..1",     1,    DSTB,    54,     8,g64("110000000010..0")},

            { L_, "1..1", BPS-1,  "0..0",     0,     0,                   0 },
            { L_, "1..1", BPS-1,  "0..0",     0,     1,                   0 },
            { L_, "1..1", BPS-1,  "0..0",     1,     0,                   0 },
            { L_, "1..1", BPS-1,  "0..0",     1,     1,                   0 },
            { L_, "1..1", BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1", BPS-1,  "0..0", BPS-1,     1,                   0 },
            { L_, "1..1", BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1", BPS-1,  "1..1",     0,     0,               ~zero },
            { L_, "1..1", BPS-1,  "1..1",     0,     1,              zero-2 },
            { L_, "1..1", BPS-1,  "1..1",     1,     0,               ~zero },
            { L_, "1..1", BPS-1,  "1..1",     1,     1,              zero-3 },
            { L_, "1..1", BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1", BPS-1,  "1..1", BPS-1,     1,            int64Max },
            { L_, "1..1", BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1", BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01,     0,     1,        g64(SW_01)^1 },
            { L_, "1..1", BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1", BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10,     1,     1,        g64(SW_10)^2 },
            { L_, "1..1", BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_, "1..1", BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",   BPS,  "0..0",     0,     0,                   0 },
            { L_, "1..1",   BPS,  "0..0",     1,     0,                   0 },
            { L_, "1..1",   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1",   BPS,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1",   BPS,  "1..1",     0,     0,               ~zero },
            { L_, "1..1",   BPS,  "1..1",     1,     0,               ~zero },
            { L_, "1..1",   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1",   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1",   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1",   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1",   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1",   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1",   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1",   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1",   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1",   BPS,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,     0,  "0..0",     0,     0,                   0 },
            { L_,  SW_01,     0,  "0..0",     0,     1,                   0 },
            { L_,  SW_01,     0,  "0..0",     0, BPS-1,                   0 },
            { L_,  SW_01,     0,  "0..0",     0,   BPS,                   0 },
            { L_,  SW_01,     0,  "0..0",     1,     0,                   0 },
            { L_,  SW_01,     0,  "0..0",     1,     1,                   0 },
            { L_,  SW_01,     0,  "0..0",     1, BPS-1,                   0 },
            { L_,  SW_01,     0,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01,     0,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_01,     0,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01,     0,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01,     0,  "1..1",     0,     1,              zero-2 },
            { L_,  SW_01,     0,  "1..1",     0, BPS-1,          g64(SW_10) },
            { L_,  SW_01,     0,  "1..1",     0,   BPS,          g64(SW_10) },
            { L_,  SW_01,     0,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01,     0,  "1..1",     1,     1,              zero-3 },
            { L_,  SW_01,     0,  "1..1",     1, BPS-1,          g64(SW_01) },
            { L_,  SW_01,     0,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01,     0,  "1..1", BPS-1,     1,            int64Max },
            { L_,  SW_01,     0,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01,     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     0,     1,        g64(SW_01)^1 },
            { L_,  SW_01,     0,   SW_01,     0, BPS-1,                   0 },
            { L_,  SW_01,     0,   SW_01,     0,   BPS,                   0 },
            { L_,  SW_01,     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01,     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     0,   BPS,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     1,     1,        g64(SW_10)^2 },
            { L_,  SW_01,     0,   SW_10,     1, BPS-1,                   0 },
            { L_,  SW_01,     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_,  SW_01,     0,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,     0,    DSTA,     2,     4,g64("1101101100000") },
            { L_,  SW_01,     0,    DSTA,     2,     5,g64("1101100100000") },
            { L_,  SW_01,     0,    DSTA,     2,     6,g64("1101100100000") },
            { L_,  SW_01,     0,    DSTA,     2,     7,g64("1101000100000") },
            { L_,  SW_01,     0,    DSTA,     2,     8,g64("1101000100000") },
            { L_,  SW_01,     0,    DSTA,     3,     4,g64("1101101000100") },
            { L_,  SW_01,     0,    DSTA,     3,     5,g64("1101101000100") },
            { L_,  SW_01,     0,    DSTA,     3,     6,g64("1101101000100") },
            { L_,  SW_01,     0,    DSTA,     3,     7,g64("1100101000100") },
            { L_,  SW_01,     0,    DSTA,     3,     8,g64("1100101000100") },

            //L#      src   sIdx   dst     dIdx   nBits   minusEqual Result
            //--   ------   ----   -----   ----   -----   -----------------
            //                                  // DSTB =  "110110110010..0"

            { L_,  SW_01,     0,    DSTB,    53,     4,g64("110110110000..0")},
            { L_,  SW_01,     0,    DSTB,    53,     5,g64("110110010000..0")},
            { L_,  SW_01,     0,    DSTB,    53,     6,g64("110110010000..0")},
            { L_,  SW_01,     0,    DSTB,    53,     7,g64("110100010000..0")},
            { L_,  SW_01,     0,    DSTB,    53,     8,g64("110100010000..0")},
            { L_,  SW_01,     0,    DSTB,    54,     4,g64("110110100010..0")},
            { L_,  SW_01,     0,    DSTB,    54,     5,g64("110110100010..0")},
            { L_,  SW_01,     0,    DSTB,    54,     6,g64("110110100010..0")},
            { L_,  SW_01,     0,    DSTB,    54,     7,g64("110010100010..0")},
            { L_,  SW_01,     0,    DSTB,    54,     8,g64("110010100010..0")},

            { L_,  SW_01,     1,  "0..0",     0,     0,                   0 },
            { L_,  SW_01,     1,  "0..0",     0,     1,                   0 },
            { L_,  SW_01,     1,  "0..0",     0, BPS-1,                   0 },
            { L_,  SW_01,     1,  "0..0",     1,     0,                   0 },
            { L_,  SW_01,     1,  "0..0",     1,     1,                   0 },
            { L_,  SW_01,     1,  "0..0",     1, BPS-1,                   0 },
            { L_,  SW_01,     1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01,     1,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_01,     1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01,     1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01,     1,  "1..1",     0,     1,               ~zero },
            { L_,  SW_01,     1,  "1..1",     0, BPS-1, g64(SW_01)|int64Min },
            { L_,  SW_01,     1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01,     1,  "1..1",     1,     1,               ~zero },
            { L_,  SW_01,     1,  "1..1",     1, BPS-1,        g64(SW_10)|1 },
            { L_,  SW_01,     1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01,     1,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_01,     1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01,     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     1, BPS-1,                   1 },
            { L_,  SW_01,     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01,     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     0, BPS-1,            int64Min },
            { L_,  SW_01,     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,     1,    DSTA,     2,     4,g64("1101101000100") },
            { L_,  SW_01,     1,    DSTA,     2,     5,g64("1101101000100") },
            { L_,  SW_01,     1,    DSTA,     2,     6,g64("1101101000100") },
            { L_,  SW_01,     1,    DSTA,     2,     7,g64("1101101000100") },
            { L_,  SW_01,     1,    DSTA,     2,     8,g64("1100101000100") },
            { L_,  SW_01,     1,    DSTA,     3,     4,g64("1101100100100") },
            { L_,  SW_01,     1,    DSTA,     3,     5,g64("1101100100100") },
            { L_,  SW_01,     1,    DSTA,     3,     6,g64("1101000100100") },
            { L_,  SW_01,     1,    DSTA,     3,     7,g64("1101000100100") },
            { L_,  SW_01,     1,    DSTA,     3,     8,g64("1101000100100") },

            //L#      src   sIdx   dst     dIdx   nBits   minusEqual Result
            //--   ------   ----   -----   ----   -----   -----------------
            //                                  // DSTB =  "110110110010..0"

            { L_,  SW_01,     1,    DSTB,    53,     4,g64("110110100010..0")},
            { L_,  SW_01,     1,    DSTB,    53,     5,g64("110110100010..0")},
            { L_,  SW_01,     1,    DSTB,    53,     6,g64("110110100010..0")},
            { L_,  SW_01,     1,    DSTB,    53,     7,g64("110110100010..0")},
            { L_,  SW_01,     1,    DSTB,    53,     8,g64("110010100010..0")},
            { L_,  SW_01,     1,    DSTB,    54,     4,g64("110110010010..0")},
            { L_,  SW_01,     1,    DSTB,    54,     5,g64("110110010010..0")},
            { L_,  SW_01,     1,    DSTB,    54,     6,g64("110100010010..0")},
            { L_,  SW_01,     1,    DSTB,    54,     7,g64("110100010010..0")},
            { L_,  SW_01,     1,    DSTB,    54,     8,g64("110100010010..0")},

            { L_,  SW_01, BPS-1,  "0..0",     0,     0,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",     0,     1,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",     1,     0,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",     1,     1,                   0 },
            { L_,  SW_01, BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01, BPS-1,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01, BPS-1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",     0,     1,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",     1,     1,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01, BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01, BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,   BPS,  "0..0",     0,     0,                   0 },
            { L_,  SW_01,   BPS,  "0..0",     1,     0,                   0 },
            { L_,  SW_01,   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01,   BPS,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01,   BPS,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01,   BPS,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01,   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01,   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01,   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01,   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01,   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01,   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01,   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01,   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01,   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01,   BPS,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,     0,  "0..0",     0,     0,                   0 },
            { L_,  SW_10,     0,  "0..0",     0,     1,                   0 },
            { L_,  SW_10,     0,  "0..0",     0, BPS-1,                   0 },
            { L_,  SW_10,     0,  "0..0",     0,   BPS,                   0 },
            { L_,  SW_10,     0,  "0..0",     1,     0,                   0 },
            { L_,  SW_10,     0,  "0..0",     1,     1,                   0 },
            { L_,  SW_10,     0,  "0..0",     1, BPS-1,                   0 },
            { L_,  SW_10,     0,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10,     0,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_10,     0,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10,     0,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10,     0,  "1..1",     0,     1,               ~zero },
            { L_,  SW_10,     0,  "1..1",     0, BPS-1, g64(SW_01)|int64Min },
            { L_,  SW_10,     0,  "1..1",     0,   BPS,          g64(SW_01) },
            { L_,  SW_10,     0,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10,     0,  "1..1",     1,     1,               ~zero },
            { L_,  SW_10,     0,  "1..1",     1, BPS-1,        g64(SW_10)|1 },
            { L_,  SW_10,     0,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10,     0,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_10,     0,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10,     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     0,   BPS,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     1, BPS-1,                   1 },
            { L_,  SW_10,     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,   BPS,     0,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_10,     0,     0,          g64(SW_10) },

            { L_,  SW_10,     0,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     0, BPS-1,            int64Min },
            { L_,  SW_10,     0,   SW_10,     0,   BPS,                   0 },
            { L_,  SW_10,     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,     0,    DSTA,     2,     4,g64("1101101000100") },
            { L_,  SW_10,     0,    DSTA,     2,     5,g64("1101101000100") },
            { L_,  SW_10,     0,    DSTA,     2,     6,g64("1101101000100") },
            { L_,  SW_10,     0,    DSTA,     2,     7,g64("1101101000100") },
            { L_,  SW_10,     0,    DSTA,     2,     8,g64("1100101000100") },
            { L_,  SW_10,     0,    DSTA,     3,     4,g64("1101100100100") },
            { L_,  SW_10,     0,    DSTA,     3,     5,g64("1101100100100") },
            { L_,  SW_10,     0,    DSTA,     3,     6,g64("1101000100100") },
            { L_,  SW_10,     0,    DSTA,     3,     7,g64("1101000100100") },
            { L_,  SW_10,     0,    DSTA,     3,     8,g64("1101000100100") },

            { L_,  SW_10,     1,  "0..0",     0,     0,                   0 },
            { L_,  SW_10,     1,  "0..0",     0,     1,                   0 },
            { L_,  SW_10,     1,  "0..0",     0, BPS-1,                   0 },
            { L_,  SW_10,     1,  "0..0",     1,     0,                   0 },
            { L_,  SW_10,     1,  "0..0",     1,     1,                   0 },
            { L_,  SW_10,     1,  "0..0",     1, BPS-1,                   0 },
            { L_,  SW_10,     1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10,     1,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_10,     1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10,     1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10,     1,  "1..1",     0,     1,              zero-2 },
            { L_,  SW_10,     1,  "1..1",     0, BPS-1,          g64(SW_10) },
            { L_,  SW_10,     1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10,     1,  "1..1",     1,     1,              zero-3 },
            { L_,  SW_10,     1,  "1..1",     1, BPS-1,          g64(SW_01) },
            { L_,  SW_10,     1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10,     1,  "1..1", BPS-1,     1,            int64Max },
            { L_,  SW_10,     1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10,     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     0,     1,        g64(SW_01)^1 },
            { L_,  SW_10,     1,   SW_01,     0, BPS-1,                   0 },
            { L_,  SW_10,     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10,     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     1,     1,        g64(SW_10)^2 },
            { L_,  SW_10,     1,   SW_10,     1, BPS-1,                   0 },
            { L_,  SW_10,     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_,  SW_10,     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,     1,    DSTA,     2,     4,g64("1101101100000") },
            { L_,  SW_10,     1,    DSTA,     2,     5,g64("1101100100000") },
            { L_,  SW_10,     1,    DSTA,     2,     6,g64("1101100100000") },
            { L_,  SW_10,     1,    DSTA,     2,     7,g64("1101000100000") },
            { L_,  SW_10,     1,    DSTA,     2,     8,g64("1101000100000") },
            { L_,  SW_10,     1,    DSTA,     3,     4,g64("1101101000100") },
            { L_,  SW_10,     1,    DSTA,     3,     5,g64("1101101000100") },
            { L_,  SW_10,     1,    DSTA,     3,     6,g64("1101101000100") },
            { L_,  SW_10,     1,    DSTA,     3,     7,g64("1100101000100") },
            { L_,  SW_10,     1,    DSTA,     3,     8,g64("1100101000100") },

            //L#      src   sIdx   dst     dIdx   nBits   minusEqual Result
            //--   ------   ----   -----   ----   -----   -----------------
            //                                  // DSTB =  "110110110010..0"

            { L_,  SW_10,     1,    DSTB,    53,     4,g64("110110110000..0")},
            { L_,  SW_10,     1,    DSTB,    53,     5,g64("110110010000..0")},
            { L_,  SW_10,     1,    DSTB,    53,     6,g64("110110010000..0")},
            { L_,  SW_10,     1,    DSTB,    53,     7,g64("110100010000..0")},
            { L_,  SW_10,     1,    DSTB,    53,     8,g64("110100010000..0")},
            { L_,  SW_10,     1,    DSTB,    54,     4,g64("110110100010..0")},
            { L_,  SW_10,     1,    DSTB,    54,     5,g64("110110100010..0")},
            { L_,  SW_10,     1,    DSTB,    54,     6,g64("110110100010..0")},
            { L_,  SW_10,     1,    DSTB,    54,     7,g64("110010100010..0")},
            { L_,  SW_10,     1,    DSTB,    54,     8,g64("110010100010..0")},

            { L_,  SW_10, BPS-1,  "0..0",     0,     0,                   0 },
            { L_,  SW_10, BPS-1,  "0..0",     0,     1,                   0 },
            { L_,  SW_10, BPS-1,  "0..0",     1,     0,                   0 },
            { L_,  SW_10, BPS-1,  "0..0",     1,     1,                   0 },
            { L_,  SW_10, BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10, BPS-1,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_10, BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10, BPS-1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1",     0,     1,              zero-2 },
            { L_,  SW_10, BPS-1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1",     1,     1,              zero-3 },
            { L_,  SW_10, BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1", BPS-1,     1,            int64Max },
            { L_,  SW_10, BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10, BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01,     0,     1,        g64(SW_01)^1 },
            { L_,  SW_10, BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10, BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10,     1,     1,        g64(SW_10)^2 },
            { L_,  SW_10, BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_,  SW_10, BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,   BPS,  "0..0",     0,     0,                   0 },
            { L_,  SW_10,   BPS,  "0..0",     1,     0,                   0 },
            { L_,  SW_10,   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10,   BPS,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10,   BPS,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10,   BPS,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10,   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10,   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10,   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10,   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10,   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10,   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10,   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10,   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10,   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10,   BPS,   SW_10,   BPS,     0,          g64(SW_10) }
        };
        const int NUM_DATA_B = sizeof DATA_B / sizeof *DATA_B;

        for (int di = 0; di < NUM_DATA_B; ++di) {
            const int   LINE = DATA_B[di].d_lineNum;
            const char *spec = DATA_B[di].d_dst;

            uint64_t dst = g64(spec);
            uint64_t src = g64(DATA_B[di].d_src);
            src >>= DATA_B[di].d_sindex;

            if (veryVerbose) {
                P_(LINE); P_(spec); P_(DATA_B[di].d_minusEqual); P(dst);
            }

            dst = g64(spec);
            Util::minusEqBits(&dst, DATA_B[di].d_dindex, src,
                                                         DATA_B[di].d_numBits);
            LOOP_ASSERT(LINE, DATA_B[di].d_minusEqual == dst);
        }
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // TESTING ANDEQBITS FUNCTION (5 ARGUMENTS):
        //   Ensure the method has the expected effect on 'dstScalar'.
        //
        // Concerns:
        //: 1 That the function properly detects undefined behavior.
        //: 2 That the function has the correct effect on the '*dstScalar'
        //:   output.
        //
        // Plan:
        //: 1 Do negative testing.
        //: 2 Do table-driven testing.  Note that this used to be a 5-arg,
        //:   rather than a 4-arg, function, taking an 'sindex' operand that
        //:   right-shifted 'srcScalar' before applying.  We use the full
        //:   table by right-shifting 'src' ourselves in the test driver and
        //:   are thus able to make use of the old test cases.
        //
        // Testing:
        //   void andEqBits(uint64_t *dInt,
        //                  int dIdx,
        //                  uint64_t sInt,
        //                  int nBits);
        // --------------------------------------------------------------------

        if (verbose) cout << "Testing andEqBits Function" << endl
                          << "==========================" << endl;

        if (verbose) cout << "Negative Testing\n";
        {
            bsls::AssertTestHandlerGuard guard;

            uint64_t dst;

            ASSERT_SAFE_PASS(Util::andEqBits(&dst, 0, 20, 0));
            ASSERT_SAFE_PASS(Util::andEqBits(&dst, 10, 20, 0));
            ASSERT_SAFE_PASS(Util::andEqBits(&dst, 10, 20, 10));
            ASSERT_SAFE_PASS(Util::andEqBits(&dst, 10, -1LL,
                                                          BITS_PER_UINT64-10));

            ASSERT_SAFE_FAIL(Util::andEqBits(&dst, -1LL, 20, 0));
            ASSERT_SAFE_FAIL(Util::andEqBits(&dst,  0, 20, -1LL));
            ASSERT_SAFE_FAIL(Util::andEqBits(&dst, 10, 20, -1LL));
            ASSERT_SAFE_FAIL(Util::andEqBits(&dst, -1LL, 20, 10));
            ASSERT_SAFE_FAIL(Util::andEqBits(&dst, 10, -1LL,
                                                          BITS_PER_UINT64- 9));
        }

        if (verbose) cout << "Table-driven Testing\n";

        const char *DSTB = "1101101100100";    // typical case
        const char *DSTC = "1101101100..0";    // typical case

        static const struct {
            int         d_lineNum;   // line number
            const char *d_src;       // source integer
            int         d_sindex;    // source index
            const char *d_dst;       // destination integer
            int         d_dindex;    // destination index
            int         d_numBits;   // num of bits
            uint64_t    d_andEqual;  // expected result from andEqual function
        } DATA_B[] = {
            //L#     src   sIdx      dst   dIdx  nBits      andEqual Result
            //--  ------   ----   ------   ----  -----      ---------------
            { L_, "0..0",     0,  "0..0",     0,     0,                   0 },
            { L_, "0..0",     0,  "0..0",     0,     1,                   0 },
            { L_, "0..0",     0,  "0..0",     0, BPS-1,                   0 },
            { L_, "0..0",     0,  "0..0",     0,   BPS,                   0 },
            { L_, "0..0",     0,  "0..0",     1,     0,                   0 },
            { L_, "0..0",     0,  "0..0",     1,     1,                   0 },
            { L_, "0..0",     0,  "0..0",     1, BPS-1,                   0 },
            { L_, "0..0",     0,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0",     0,  "0..0", BPS-1,     1,                   0 },
            { L_, "0..0",     0,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0",     0,  "1..1",     0,     0,               ~zero },
            { L_, "0..0",     0,  "1..1",     0,     1,              zero-2 },
            { L_, "0..0",     0,  "1..1",     0, BPS-1,            int64Min },
            { L_, "0..0",     0,  "1..1",     0,   BPS,                   0 },
            { L_, "0..0",     0,  "1..1",     1,     0,               ~zero },
            { L_, "0..0",     0,  "1..1",     1,     1,  ~((uint64_t) 1<<1) },
            { L_, "0..0",     0,  "1..1",     1, BPS-1,                   1 },
            { L_, "0..0",     0,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0",     0,  "1..1", BPS-1,     1,            int64Max },
            { L_, "0..0",     0,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0",     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     0,     1,      g64(SW_01) ^ 1 },
            { L_, "0..0",     0,   SW_01,     0, BPS-1,                   0 },
            { L_, "0..0",     0,   SW_01,     0,   BPS,                   0 },
            { L_, "0..0",     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,     1, BPS-1,                   1 },
            { L_, "0..0",     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "0..0",     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0",     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     0, BPS-1,            int64Min },
            { L_, "0..0",     0,   SW_10,     0,   BPS,                   0 },
            { L_, "0..0",     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10,     1,     1,
                                             g64(SW_10) ^ ((uint64_t) 1<<1) },
            { L_, "0..0",     0,   SW_10,     1, BPS-1,                   0 },
            { L_, "0..0",     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0",     0,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_, "0..0",     0,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "0..0",     0,    DSTB,     2,     4,g64("1101101000000") },
            { L_, "0..0",     0,    DSTB,     2,     5,g64("1101100000000") },
            { L_, "0..0",     0,    DSTB,     2,     6,g64("1101100000000") },
            { L_, "0..0",     0,    DSTB,     2,     7,g64("1101000000000") },
            { L_, "0..0",     0,    DSTB,     2,     8,g64("1100000000000") },
            { L_, "0..0",     0,    DSTB,     3,     4,g64("1101100000100") },
            { L_, "0..0",     0,    DSTB,     3,     5,g64("1101100000100") },
            { L_, "0..0",     0,    DSTB,     3,     6,g64("1101000000100") },
            { L_, "0..0",     0,    DSTB,     3,     7,g64("1100000000100") },
            { L_, "0..0",     0,    DSTB,     3,     8,g64("1100000000100") },

            { L_, "0..0",     0,    DSTC,     0,     1,g64("1101101100..0") },
            { L_, "0..0",     3,    DSTC,    62,     2,g64("0001101100..0") },
            { L_, "0..0",     3,    DSTC,    59,     2,g64("1100001100..0") },
            { L_, "0..0",     0,    DSTC,    59,     2,g64("1100001100..0") },
            { L_, "0..0",     3,    DSTC,    56,     2,g64("1101100000..0") },
            { L_, "0..0",     0,    DSTC,    56,     1,g64("1101101000..0") },
            { L_, "0..0",     0,    DSTC,    58,     1,g64("1101101100..0") },

            { L_, "0..0",     1,  "0..0",     0,     0,                   0 },
            { L_, "0..0",     1,  "0..0",     0,     1,                   0 },
            { L_, "0..0",     1,  "0..0",     0, BPS-1,                   0 },
            { L_, "0..0",     1,  "0..0",     1,     0,                   0 },
            { L_, "0..0",     1,  "0..0",     1,     1,                   0 },
            { L_, "0..0",     1,  "0..0",     1, BPS-1,                   0 },
            { L_, "0..0",     1,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0",     1,  "0..0", BPS-1,     1,                   0 },
            { L_, "0..0",     1,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0",     1,  "1..1",     0,     0,               ~zero },
            { L_, "0..0",     1,  "1..1",     0,     1,              zero-2 },
            { L_, "0..0",     1,  "1..1",     0, BPS-1,            int64Min },
            { L_, "0..0",     1,  "1..1",     1,     0,               ~zero },
            { L_, "0..0",     1,  "1..1",     1,     1,  ~((uint64_t) 1<<1) },
            { L_, "0..0",     1,  "1..1",     1, BPS-1,                   1 },
            { L_, "0..0",     1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0",     1,  "1..1", BPS-1,     1,            int64Max },
            { L_, "0..0",     1,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0",     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     0,     1,      g64(SW_01) ^ 1 },
            { L_, "0..0",     1,   SW_01,     0, BPS-1,                   0 },
            { L_, "0..0",     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,     1, BPS-1,                   1 },
            { L_, "0..0",     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "0..0",     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0",     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     0, BPS-1,            int64Min },
            { L_, "0..0",     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10,     1,     1,
                                              g64(SW_10) ^ ((uint64_t) 1<<1) },
            { L_, "0..0",     1,   SW_10,     1, BPS-1,                   0 },
            { L_, "0..0",     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0",     1,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_, "0..0",     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "0..0",     1,    DSTB,     2,     4,g64("1101101000000") },
            { L_, "0..0",     1,    DSTB,     2,     5,g64("1101100000000") },
            { L_, "0..0",     1,    DSTB,     2,     6,g64("1101100000000") },
            { L_, "0..0",     1,    DSTB,     2,     7,g64("1101000000000") },
            { L_, "0..0",     1,    DSTB,     2,     8,g64("1100000000000") },
            { L_, "0..0",     1,    DSTB,     3,     4,g64("1101100000100") },
            { L_, "0..0",     1,    DSTB,     3,     5,g64("1101100000100") },
            { L_, "0..0",     1,    DSTB,     3,     6,g64("1101000000100") },
            { L_, "0..0",     1,    DSTB,     3,     7,g64("1100000000100") },
            { L_, "0..0",     1,    DSTB,     3,     8,g64("1100000000100") },

            { L_, "0..0", BPS-1,  "0..0",     0,     0,                   0 },
            { L_, "0..0", BPS-1,  "0..0",     0,     1,                   0 },
            { L_, "0..0", BPS-1,  "0..0",     1,     0,                   0 },
            { L_, "0..0", BPS-1,  "0..0",     1,     1,                   0 },
            { L_, "0..0", BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0", BPS-1,  "0..0", BPS-1,     1,                   0 },
            { L_, "0..0", BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0", BPS-1,  "1..1",     0,     0,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",     0,     1,              zero-2 },
            { L_, "0..0", BPS-1,  "1..1",     1,     0,               ~zero },
            { L_, "0..0", BPS-1,  "1..1",     1,     1,           ~(one<<1) },
            { L_, "0..0", BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0", BPS-1,  "1..1", BPS-1,     1,            int64Max },
            { L_, "0..0", BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0", BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,     0,     1,      g64(SW_01) ^ 1 },
            { L_, "0..0", BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "0..0", BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0", BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10,     1,     1,
                                                      g64(SW_10) ^ (one<<1) },
            { L_, "0..0", BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0", BPS-1,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_, "0..0", BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "0..0",   BPS,  "0..0",     0,     0,                   0 },
            { L_, "0..0",   BPS,  "0..0",     1,     0,                   0 },
            { L_, "0..0",   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_, "0..0",   BPS,  "0..0",   BPS,     0,                   0 },

            { L_, "0..0",   BPS,  "1..1",     0,     0,               ~zero },
            { L_, "0..0",   BPS,  "1..1",     1,     0,               ~zero },
            { L_, "0..0",   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_, "0..0",   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_, "0..0",   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "0..0",   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "0..0",   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "0..0",   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "0..0",   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "0..0",   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "0..0",   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "0..0",   BPS,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",     0,  "0..0",     0,     0,                   0 },
            { L_, "1..1",     0,  "0..0",     0,     1,                   0 },
            { L_, "1..1",     0,  "0..0",     0, BPS-1,                   0 },
            { L_, "1..1",     0,  "0..0",     0,   BPS,                   0 },
            { L_, "1..1",     0,  "0..0",     1,     0,                   0 },
            { L_, "1..1",     0,  "0..0",     1,     1,                   0 },
            { L_, "1..1",     0,  "0..0",     1, BPS-1,                   0 },
            { L_, "1..1",     0,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1",     0,  "0..0", BPS-1,     1,                   0 },
            { L_, "1..1",     0,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1",     0,  "1..1",     0,     0,               ~zero },
            { L_, "1..1",     0,  "1..1",     0,     1,               ~zero },
            { L_, "1..1",     0,  "1..1",     0, BPS-1,               ~zero },
            { L_, "1..1",     0,  "1..1",     0,   BPS,               ~zero },
            { L_, "1..1",     0,  "1..1",     1,     0,               ~zero },
            { L_, "1..1",     0,  "1..1",     1,     1,               ~zero },
            { L_, "1..1",     0,  "1..1",     1, BPS-1,               ~zero },
            { L_, "1..1",     0,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1",     0,  "1..1", BPS-1,     1,               ~zero },
            { L_, "1..1",     0,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1",     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     0,   BPS,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "1..1",     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1",     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     0,   BPS,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "1..1",     0,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",     0,    DSTB,     2,     4,g64("1101101100100") },
            { L_, "1..1",     0,    DSTB,     2,     5,g64("1101101100100") },
            { L_, "1..1",     0,    DSTB,     2,     6,g64("1101101100100") },
            { L_, "1..1",     0,    DSTB,     2,     7,g64("1101101100100") },
            { L_, "1..1",     0,    DSTB,     2,     8,g64("1101101100100") },
            { L_, "1..1",     0,    DSTB,     3,     4,g64("1101101100100") },
            { L_, "1..1",     0,    DSTB,     3,     5,g64("1101101100100") },
            { L_, "1..1",     0,    DSTB,     3,     6,g64("1101101100100") },
            { L_, "1..1",     0,    DSTB,     3,     7,g64("1101101100100") },
            { L_, "1..1",     0,    DSTB,     3,     8,g64("1101101100100") },

            { L_, "1..1",     1,  "0..0",     0,     0,                   0 },
            { L_, "1..1",     1,  "0..0",     0,     1,                   0 },
            { L_, "1..1",     1,  "0..0",     0, BPS-1,                   0 },
            { L_, "1..1",     1,  "0..0",     1,     0,                   0 },
            { L_, "1..1",     1,  "0..0",     1,     1,                   0 },
            { L_, "1..1",     1,  "0..0",     1, BPS-1,                   0 },
            { L_, "1..1",     1,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1",     1,  "0..0", BPS-1,     1,                   0 },
            { L_, "1..1",     1,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1",     1,  "1..1",     0,     0,               ~zero },
            { L_, "1..1",     1,  "1..1",     0,     1,               ~zero },
            { L_, "1..1",     1,  "1..1",     0, BPS-1,               ~zero },
            { L_, "1..1",     1,  "1..1",     1,     0,               ~zero },
            { L_, "1..1",     1,  "1..1",     1,     1,               ~zero },
            { L_, "1..1",     1,  "1..1",     1, BPS-1,               ~zero },
            { L_, "1..1",     1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1",     1,  "1..1", BPS-1,     1,               ~zero },
            { L_, "1..1",     1,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1",     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "1..1",     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1",     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "1..1",     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",     1,    DSTB,     2,     4,g64("1101101100100") },
            { L_, "1..1",     1,    DSTB,     2,     5,g64("1101101100100") },
            { L_, "1..1",     1,    DSTB,     2,     6,g64("1101101100100") },
            { L_, "1..1",     1,    DSTB,     2,     7,g64("1101101100100") },
            { L_, "1..1",     1,    DSTB,     2,     8,g64("1101101100100") },
            { L_, "1..1",     1,    DSTB,     3,     4,g64("1101101100100") },
            { L_, "1..1",     1,    DSTB,     3,     5,g64("1101101100100") },
            { L_, "1..1",     1,    DSTB,     3,     6,g64("1101101100100") },
            { L_, "1..1",     1,    DSTB,     3,     7,g64("1101101100100") },
            { L_, "1..1",     1,    DSTB,     3,     8,g64("1101101100100") },

            { L_, "1..1", BPS-1,  "0..0",     0,     0,                   0 },
            { L_, "1..1", BPS-1,  "0..0",     0,     1,                   0 },
            { L_, "1..1", BPS-1,  "0..0",     1,     0,                   0 },
            { L_, "1..1", BPS-1,  "0..0",     1,     1,                   0 },
            { L_, "1..1", BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1", BPS-1,  "0..0", BPS-1,     1,                   0 },
            { L_, "1..1", BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1", BPS-1,  "1..1",     0,     0,               ~zero },
            { L_, "1..1", BPS-1,  "1..1",     0,     1,               ~zero },
            { L_, "1..1", BPS-1,  "1..1",     1,     0,               ~zero },
            { L_, "1..1", BPS-1,  "1..1",     1,     1,               ~zero },
            { L_, "1..1", BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1", BPS-1,  "1..1", BPS-1,     1,               ~zero },
            { L_, "1..1", BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1", BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01,     0,     1,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01,     1,     1,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_, "1..1", BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1", BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10,     0,     1,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10,     1,     1,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_, "1..1", BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_, "1..1",   BPS,  "0..0",     0,     0,                   0 },
            { L_, "1..1",   BPS,  "0..0",     1,     0,                   0 },
            { L_, "1..1",   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_, "1..1",   BPS,  "0..0",   BPS,     0,                   0 },

            { L_, "1..1",   BPS,  "1..1",     0,     0,               ~zero },
            { L_, "1..1",   BPS,  "1..1",     1,     0,               ~zero },
            { L_, "1..1",   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_, "1..1",   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_, "1..1",   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_, "1..1",   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_, "1..1",   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_, "1..1",   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_, "1..1",   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_, "1..1",   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_, "1..1",   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_, "1..1",   BPS,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,     0,  "0..0",     0,     0,                   0 },
            { L_,  SW_01,     0,  "0..0",     0,     1,                   0 },
            { L_,  SW_01,     0,  "0..0",     0, BPS-1,                   0 },
            { L_,  SW_01,     0,  "0..0",     0,   BPS,                   0 },
            { L_,  SW_01,     0,  "0..0",     1,     0,                   0 },
            { L_,  SW_01,     0,  "0..0",     1,     1,                   0 },
            { L_,  SW_01,     0,  "0..0",     1, BPS-1,                   0 },
            { L_,  SW_01,     0,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01,     0,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_01,     0,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01,     0,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01,     0,  "1..1",     0,     1,               ~zero },
            { L_,  SW_01,     0,  "1..1",     0, BPS-1, g64(SW_01)^int64Min },
            { L_,  SW_01,     0,  "1..1",     0,   BPS,          g64(SW_01) },
            { L_,  SW_01,     0,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01,     0,  "1..1",     1,     1,               ~zero },
            { L_,  SW_01,     0,  "1..1",     1, BPS-1,      g64(SW_10) ^ 1 },
            { L_,  SW_01,     0,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01,     0,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_01,     0,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01,     0,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     0,   BPS,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,     1, BPS-1,                   1 },
            { L_,  SW_01,     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_01,     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01,     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     0, BPS-1,            int64Min },
            { L_,  SW_01,     0,   SW_10,     0,   BPS,                   0 },
            { L_,  SW_01,     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_01,     0,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,     0,    DSTB,     2,     4,g64("1101101000100") },
            { L_,  SW_01,     0,    DSTB,     2,     5,g64("1101101000100") },
            { L_,  SW_01,     0,    DSTB,     2,     6,g64("1101101000100") },
            { L_,  SW_01,     0,    DSTB,     2,     7,g64("1101101000100") },
            { L_,  SW_01,     0,    DSTB,     2,     8,g64("1100101000100") },
            { L_,  SW_01,     0,    DSTB,     3,     4,g64("1101100100100") },
            { L_,  SW_01,     0,    DSTB,     3,     5,g64("1101100100100") },
            { L_,  SW_01,     0,    DSTB,     3,     6,g64("1101000100100") },
            { L_,  SW_01,     0,    DSTB,     3,     7,g64("1101000100100") },
            { L_,  SW_01,     0,    DSTB,     3,     8,g64("1101000100100") },

            { L_,  SW_01,     0,    DSTC,     0,     1,g64("1101101100..0") },
            { L_,  SW_01,     0,    DSTC,    62,     2,g64("0101101100..0") },
            { L_,  SW_01,    54,    DSTC,    62,     2,g64("0101101100..0") },
            { L_,  SW_01,     0,    DSTC,    59,     2,g64("1100101100..0") },
            { L_,  SW_01,    48,    DSTC,    59,     2,g64("1100101100..0") },
            { L_,  SW_01,     0,    DSTC,    56,     5,g64("1101000100..0") },
            { L_,  SW_01,    50,    DSTC,    56,     5,g64("1101000100..0") },
            { L_,  SW_01,     0,    DSTC,    56,     8,g64("0101000100..0") },
            { L_,  SW_01,    54,    DSTC,    56,     8,g64("0101000100..0") },
            { L_,  SW_01,     0,    DSTC,     0,    64,g64("0101000100..0") },

            { L_,  SW_10,     0,    DSTC,     0,     1,g64("1101101100..0") },
            { L_,  SW_10,     0,    DSTC,    62,     2,g64("1001101100..0") },
            { L_,  SW_10,    44,    DSTC,    62,     2,g64("1001101100..0") },
            { L_,  SW_10,     0,    DSTC,    59,     3,g64("1101001100..0") },
            { L_,  SW_10,    50,    DSTC,    59,     3,g64("1101001100..0") },
            { L_,  SW_10,     1,    DSTC,    59,     3,g64("1100101100..0") },
            { L_,  SW_10,    51,    DSTC,    59,     3,g64("1100101100..0") },
            { L_,  SW_10,     0,    DSTC,    56,     8,g64("1000101000..0") },
            { L_,  SW_10,    52,    DSTC,    56,     8,g64("1000101000..0") },
            { L_,  SW_10,     0,    DSTC,     0,    64,g64("1000101000..0") },

            { L_,  SW_01,     1,  "0..0",     0,     0,                   0 },
            { L_,  SW_01,     1,  "0..0",     0,     1,                   0 },
            { L_,  SW_01,     1,  "0..0",     0, BPS-1,                   0 },
            { L_,  SW_01,     1,  "0..0",     1,     0,                   0 },
            { L_,  SW_01,     1,  "0..0",     1,     1,                   0 },
            { L_,  SW_01,     1,  "0..0",     1, BPS-1,                   0 },
            { L_,  SW_01,     1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01,     1,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_01,     1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01,     1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01,     1,  "1..1",     0,     1,              zero-2 },
            { L_,  SW_01,     1,  "1..1",     0, BPS-1,          g64(SW_10) },
            { L_,  SW_01,     1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01,     1,  "1..1",     1,     1,  ~((uint64_t) 1<<1) },
            { L_,  SW_01,     1,  "1..1",     1, BPS-1,          g64(SW_01) },
            { L_,  SW_01,     1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01,     1,  "1..1", BPS-1,     1,            int64Max },
            { L_,  SW_01,     1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01,     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     0,     1,      g64(SW_01) ^ 1 },
            { L_,  SW_01,     1,   SW_01,     0, BPS-1,                   0 },
            { L_,  SW_01,     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_01,     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01,     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10,     1,     1,
                                              g64(SW_10) ^ ((uint64_t) 1<<1) },
            { L_,  SW_01,     1,   SW_10,     1, BPS-1,                   0 },
            { L_,  SW_01,     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01,     1,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_,  SW_01,     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,     1,    DSTB,     2,     4,g64("1101101100000") },
            { L_,  SW_01,     1,    DSTB,     2,     5,g64("1101100100000") },
            { L_,  SW_01,     1,    DSTB,     2,     6,g64("1101100100000") },
            { L_,  SW_01,     1,    DSTB,     2,     7,g64("1101000100000") },
            { L_,  SW_01,     1,    DSTB,     2,     8,g64("1101000100000") },
            { L_,  SW_01,     1,    DSTB,     3,     4,g64("1101101000100") },
            { L_,  SW_01,     1,    DSTB,     3,     5,g64("1101101000100") },
            { L_,  SW_01,     1,    DSTB,     3,     6,g64("1101101000100") },
            { L_,  SW_01,     1,    DSTB,     3,     7,g64("1100101000100") },
            { L_,  SW_01,     1,    DSTB,     3,     8,g64("1100101000100") },

            //                      DSTC =                 "1101101100..0"
            { L_,  SW_01,     1,    DSTC,     3,     8,g64("1101101100..0") },
            { L_,  SW_01,     1,    DSTC,     3,    61,g64("0101000100..0") },
            { L_,  SW_01,     1,    DSTC,     2,    62,g64("1000101000..0") },
            { L_,  SW_01,     1,    DSTC,     2,    60,g64("1100101000..0") },
            { L_,  SW_01,     1,    DSTC,     2,    57,g64("1101101000..0") },

            { L_,  SW_01, BPS-1,  "0..0",     0,     0,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",     0,     1,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",     1,     0,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",     1,     1,                   0 },
            { L_,  SW_01, BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01, BPS-1,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_01, BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01, BPS-1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",     0,     1,              zero-2 },
            { L_,  SW_01, BPS-1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1",     1,     1,              zero-3 },
            { L_,  SW_01, BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01, BPS-1,  "1..1", BPS-1,     1,            int64Max },
            { L_,  SW_01, BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01, BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,     0,     1,      g64(SW_01) ^ 1 },
            { L_,  SW_01, BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_01, BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01, BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10,     1,     1,
                                             g64(SW_10) ^ ((uint64_t) 1<<1) },
            { L_,  SW_01, BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01, BPS-1,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_,  SW_01, BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_01,   BPS,  "0..0",     0,     0,                   0 },
            { L_,  SW_01,   BPS,  "0..0",     1,     0,                   0 },
            { L_,  SW_01,   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_01,   BPS,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_01,   BPS,  "1..1",     0,     0,               ~zero },
            { L_,  SW_01,   BPS,  "1..1",     1,     0,               ~zero },
            { L_,  SW_01,   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_01,   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_01,   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_01,   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_01,   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_01,   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_01,   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_01,   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_01,   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_01,   BPS,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,     0,  "0..0",     0,     0,                   0 },
            { L_,  SW_10,     0,  "0..0",     0,     1,                   0 },
            { L_,  SW_10,     0,  "0..0",     0, BPS-1,                   0 },
            { L_,  SW_10,     0,  "0..0",     0,   BPS,                   0 },
            { L_,  SW_10,     0,  "0..0",     1,     0,                   0 },
            { L_,  SW_10,     0,  "0..0",     1,     1,                   0 },
            { L_,  SW_10,     0,  "0..0",     1, BPS-1,                   0 },
            { L_,  SW_10,     0,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10,     0,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_10,     0,  "0..0",   BPS,     0,                   0 },
            { L_,  SW_10,     0,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10,     0,  "1..1",     0,     1,              zero-2 },
            { L_,  SW_10,     0,  "1..1",     0, BPS-1,          g64(SW_10) },
            { L_,  SW_10,     0,  "1..1",     0,   BPS,          g64(SW_10) },
            { L_,  SW_10,     0,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10,     0,  "1..1",     1,     1,  ~((uint64_t) 1<<1) },
            { L_,  SW_10,     0,  "1..1",     1, BPS-1,          g64(SW_01) },
            { L_,  SW_10,     0,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10,     0,  "1..1", BPS-1,     1,            int64Max },
            { L_,  SW_10,     0,  "1..1",   BPS,     0,               ~zero },
            { L_,  SW_10,     0,   SW_01,     0,     0,          g64(SW_01) },

            { L_,  SW_10,     0,   SW_01,     0,     1,      g64(SW_01) ^ 1 },
            { L_,  SW_10,     0,   SW_01,     0, BPS-1,                   0 },
            { L_,  SW_10,     0,   SW_01,     0,   BPS,                   0 },
            { L_,  SW_10,     0,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,     1, BPS-1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_10,     0,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10,     0,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     0, BPS-1,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     0,   BPS,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10,     1,     1,
                                                      g64(SW_10) ^ (one<<1) },
            { L_,  SW_10,     0,   SW_10,     1, BPS-1,                   0 },
            { L_,  SW_10,     0,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10,     0,   SW_10, BPS-1,     1, g64(SW_10)^int64Min },
            { L_,  SW_10,     0,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,     0,   DSTB,     2,     4, g64("1101101100000") },
            { L_,  SW_10,     0,   DSTB,     2,     5, g64("1101100100000") },
            { L_,  SW_10,     0,   DSTB,     2,     6, g64("1101100100000") },
            { L_,  SW_10,     0,   DSTB,     2,     7, g64("1101000100000") },
            { L_,  SW_10,     0,   DSTB,     2,     8, g64("1101000100000") },
            { L_,  SW_10,     0,   DSTB,     3,     4, g64("1101101000100") },
            { L_,  SW_10,     0,   DSTB,     3,     5, g64("1101101000100") },
            { L_,  SW_10,     0,   DSTB,     3,     6, g64("1101101000100") },
            { L_,  SW_10,     0,   DSTB,     3,     7, g64("1100101000100") },
            { L_,  SW_10,     0,   DSTB,     3,     8, g64("1100101000100") },

            { L_,  SW_10,     1,  "0..0",     0,     0,                   0 },
            { L_,  SW_10,     1,  "0..0",     0,     1,                   0 },
            { L_,  SW_10,     1,  "0..0",     0, BPS-1,                   0 },
            { L_,  SW_10,     1,  "0..0",     1,     0,                   0 },
            { L_,  SW_10,     1,  "0..0",     1,     1,                   0 },
            { L_,  SW_10,     1,  "0..0",     1, BPS-1,                   0 },
            { L_,  SW_10,     1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10,     1,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_10,     1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10,     1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10,     1,  "1..1",     0,     1,               ~zero },
            { L_,  SW_10,     1,  "1..1",     0, BPS-1, g64(SW_01)^int64Min },
            { L_,  SW_10,     1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10,     1,  "1..1",     1,     1,               ~zero },
            { L_,  SW_10,     1,  "1..1",     1, BPS-1,      g64(SW_10) ^ 1 },
            { L_,  SW_10,     1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10,     1,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_10,     1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10,     1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     0, BPS-1,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,     1, BPS-1,                   1 },
            { L_,  SW_10,     1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_10,     1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10,     1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     0, BPS-1,            int64Min },
            { L_,  SW_10,     1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,     1, BPS-1,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_10,     1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,     1,    DSTB,     2,     4,g64("1101101000100") },
            { L_,  SW_10,     1,    DSTB,     2,     5,g64("1101101000100") },
            { L_,  SW_10,     1,    DSTB,     2,     6,g64("1101101000100") },
            { L_,  SW_10,     1,    DSTB,     2,     7,g64("1101101000100") },
            { L_,  SW_10,     1,    DSTB,     2,     8,g64("1100101000100") },
            { L_,  SW_10,     1,    DSTB,     3,     4,g64("1101100100100") },
            { L_,  SW_10,     1,    DSTB,     3,     5,g64("1101100100100") },
            { L_,  SW_10,     1,    DSTB,     3,     6,g64("1101000100100") },
            { L_,  SW_10,     1,    DSTB,     3,     7,g64("1101000100100") },
            { L_,  SW_10,     1,    DSTB,     3,     8,g64("1101000100100") },

            { L_,  SW_10, BPS-1,  "0..0",     0,     0,                   0 },
            { L_,  SW_10, BPS-1,  "0..0",     0,     1,                   0 },
            { L_,  SW_10, BPS-1,  "0..0",     1,     0,                   0 },
            { L_,  SW_10, BPS-1,  "0..0",     1,     1,                   0 },
            { L_,  SW_10, BPS-1,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10, BPS-1,  "0..0", BPS-1,     1,                   0 },
            { L_,  SW_10, BPS-1,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10, BPS-1,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1",     0,     1,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1",     1,     1,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1", BPS-1,     1,               ~zero },
            { L_,  SW_10, BPS-1,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10, BPS-1,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01,     0,     1,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01,     1,     1,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01, BPS-1,     1,          g64(SW_01) },
            { L_,  SW_10, BPS-1,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10, BPS-1,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10,     0,     1,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10,     1,     1,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10, BPS-1,     1,          g64(SW_10) },
            { L_,  SW_10, BPS-1,   SW_10,   BPS,     0,          g64(SW_10) },

            { L_,  SW_10,   BPS,  "0..0",     0,     0,                   0 },
            { L_,  SW_10,   BPS,  "0..0",     1,     0,                   0 },
            { L_,  SW_10,   BPS,  "0..0", BPS-1,     0,                   0 },
            { L_,  SW_10,   BPS,  "0..0",   BPS,     0,                   0 },

            { L_,  SW_10,   BPS,  "1..1",     0,     0,               ~zero },
            { L_,  SW_10,   BPS,  "1..1",     1,     0,               ~zero },
            { L_,  SW_10,   BPS,  "1..1", BPS-1,     0,               ~zero },
            { L_,  SW_10,   BPS,  "1..1",   BPS,     0,               ~zero },

            { L_,  SW_10,   BPS,   SW_01,     0,     0,          g64(SW_01) },
            { L_,  SW_10,   BPS,   SW_01,     1,     0,          g64(SW_01) },
            { L_,  SW_10,   BPS,   SW_01, BPS-1,     0,          g64(SW_01) },
            { L_,  SW_10,   BPS,   SW_01,   BPS,     0,          g64(SW_01) },

            { L_,  SW_10,   BPS,   SW_10,     0,     0,          g64(SW_10) },
            { L_,  SW_10,   BPS,   SW_10,     1,     0,          g64(SW_10) },
            { L_,  SW_10,   BPS,   SW_10, BPS-1,     0,          g64(SW_10) },
            { L_,  SW_10,   BPS,   SW_10,   BPS,     0,          g64(SW_10) }
        };
        const int NUM_DATA_B = sizeof DATA_B / sizeof *DATA_B;

        for (int di = 0; di < NUM_DATA_B; ++di) {
            const int   LINE     = DATA_B[di].d_lineNum;
            const char *spec     = DATA_B[di].d_dst;
            const int   srcIndex = DATA_B[di].d_sindex;

            uint64_t dst = g64(spec);
            uint64_t src = g64(DATA_B[di].d_src) >> srcIndex;

            if (veryVerbose) {
                P_(LINE); P_(spec); P_(DATA_B[di].d_andEqual); P(dst);
            }

            if (0 == DATA_B[di].d_sindex) {
                dst = g64(spec);
                Util::andEqBits(&dst, DATA_B[di].d_dindex, src,
                                                         DATA_B[di].d_numBits);
                LOOP_ASSERT(LINE, DATA_B[di].d_andEqual == dst);
            }
        }
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // ENUM TEST
        //   Ensure the enum is correct.
        //
        // Concerns
        //: 1 The the enum defined by the class is as expected.
        //
        // Plan
        //: 1 Test the enum.
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "Testing 'enum' variables" << endl
                          << "========================" << endl;

        ASSERT(                  64 == Util::k_BITS_PER_UINT64);
        ASSERT(8 * sizeof(uint64_t) == Util::k_BITS_PER_UINT64);
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING GENERATOR FUNCTION g64
        //   Ensure the generator function is correct.
        //
        // Concerns:
        //: 1 'g64' must correctly parse 'spec' according to its specific
        //:   language, and return the corresponding 'uint64_t' if 'spec' is
        //:   valid.
        //: 2 'g64' must also correctly diagnose and report an invalid 'spec'.
        //: 3 'g's error-report-suppression flag must be set before testing
        //:   invalid 'spec's, and must be explicitly unset after testing.
        //
        // Plan:
        //: 1 For a sequence of valid 'spac's, verify that 'g' returns the
        //:   correct value.
        //: 2 For a sequence of idvalid 'spac's, verify that 'g' returns the
        //:   correct diagnosis.
        //
        // Testing:
        //   GENERATOR FUNCTION: int g64(const char *spec)
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING GENERATOR FUNCTION g64\n"
                          << "==============================\n";

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

                { L_, "1..1",                           zero-1          },
                { L_, "11..1",                          zero-1          },
                { L_, "1..11",                          zero-1          },
                { L_, "01..1",                          int64Max        },
                { L_, "1..10",                          zero-2          },

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

                { L_, SW_1,                 zero-1                  },
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

                { L_, "0..0" SW_1,          zero-1                  },
                { L_, FW_1 "0..0" FW_1,     zero-1                  },
                { L_, SW_1 "0..0",          zero-1                  },

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
        // BREATHING TEST:
        //   This case tests nothing.
        //
        // Concern:
        //: 1 A utility component typically does not need a breathing test.
        //:   This case is provided as a place-holder before getting into
        //:   real testing.
        //
        // Plan:
        //: 1 Do nothing.
        //
        // Testing:
        //   This case tests nothing.
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
