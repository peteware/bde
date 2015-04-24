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
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;
    verbose = argc > 2;
    veryVerbose = argc > 3;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;

    switch (test) { case 0:  // Zero is always the leading case.
      case 5: {
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
      case 4: {
        // --------------------------------------------------------------------
        // TESTING ONE AND ZERO FUNCTIONS
        //   Ensure the methods return the expected value.
        //
        // Concerns:
        //: 1 That the 'one', 'zero', 'one64', and 'zero64' functions catch
        //:   undefined behavior with asserts.
        //: 2 That when the behavior is defined, the functions all return the
        //:   correct result.
        //
        // Plan:
        //: 1 Do negative testing.
        //: 2 Do table-driven testing.
        //: 3 Do exhaustive testing, calculating the expected result through
        //:   a different algorithm than the functions use.
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

        if (verbose) cout << "Negative testing\n";
        {
            bsls::AssertTestHandlerGuard guard;

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

        if (verbose) cout << "Table-driven 32-bit testing\n";
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

          { L_,    2,    4, (uint32_t) ~(0xf << 2),
                                      (uint32_t) 0xf << 2  }, // typical case
          { L_,BPW-5,    3, (uint32_t) ~(7 << (BPW-5)),
                                   (uint32_t) 7 << (BPW-5) }
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

            uint32_t resB = Util::one(INDEX, NUM_BITS);
            if (veryVerbose) {
                P_(LINE);
                P_(INDEX);
                P_(NUM_BITS);
                P_(DATA_A[di].d_mask1);
                P(resB);
            }
            LOOP_ASSERT(LINE, DATA_A[di].d_mask1 == resB);

            ASSERT(~resB == resA);
        }

        if (verbose) cout << "Table-driven 64-bit testing\n";

        const uint64_t f64 = 0xf, seven64 = 7;

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

          { L_,    2,      4,       ~(f64 << 2),       f64 << 2 },
          { L_,BPS-5,      3, ~(seven64 << (BPS-5)),
                                             seven64 << (BPS-5) }
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

            uint64_t resB = Util::one64(INDEX, NUM_BITS);
            if (veryVerbose) {
                P_(LINE);
                P_(INDEX);
                P_(NUM_BITS);
                P_(DATA_B[di].d_mask1);
                P(resB);
            }
            LOOP_ASSERT(LINE, DATA_B[di].d_mask1 == resB);

            ASSERT(~resB == resA);
        }

        if (verbose) cout << "Exhaustive 32-bit testing\n";
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

        if (verbose) cout << "Exhaustive 64-bit testing\n";
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
      case 3: {
        // --------------------------------------------------------------------
        // TESTING ONE ARG MASK GENERATION FUNCTIONS:
        //   Ensure the methods return the expected value.
        //
        // Concerns:
        //: 1 That all 12 one-arg mask functions detect undefined behavior with
        //:   asserts.
        //: 2 That when not exhibiting undefined behavior, all 12 one-arg mask
        //:   functions return the correct result.
        //
        // Plan:
        //: 1 Do negative testing.
        //: 2 Do table-driven testing.  Note that since the functions come in
        //:   pairs whose results are compliments of each other, test one pair
        //:   per table.
        //: 3 Do exhaustive testing, exhausting all input in the range
        //:   '[ 0, wordSize ]', comparing the result with an expected value
        //:   calculated by a means other than that employed by the function.
        //: 4 Do exhaustive testing verifying that functions expected to return
        //    the complement of each other actually do.
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

        int i;

        if (verbose) cout << "Exhaustive Testing of 'eq'" << endl;
        for (i = 0; i < BITS_PER_WORD; ++i) {
            LOOP_ASSERT(i, one << i == Util::eq(i));
        }
        ASSERT(zero == Util::eq(BITS_PER_WORD));
        ASSERT(zero == Util::eq(BITS_PER_WORD + 1000));

        if (verbose) cout << "Exhaustive Testing of 'ne'" << endl;
        for (i = 0; i < (int) BITS_PER_WORD; ++i) {
            LOOP_ASSERT(i, ~(one << i) == Util::ne(i));
        }
        ASSERT(~zero == Util::ne(BITS_PER_WORD));
        ASSERT(~zero == Util::ne(BITS_PER_WORD + 1000));

        if (verbose) cout << "Exhaustive Testing of 'ge'" << endl;
        for (i = 0; i < (int) BITS_PER_WORD; ++i) {
            LOOP_ASSERT(i, ~zero << i == Util::ge(i));
        }
        ASSERT(zero == Util::ge(BITS_PER_WORD));
        ASSERT(zero == Util::ge(BITS_PER_WORD + 1000));

        if (verbose) cout << "Exhaustive Testing of 'gt'" << endl;
        for (i = 0; i < (int) BITS_PER_WORD - 1; ++i) {
            LOOP_ASSERT(i, ~zero << (i + 1) == Util::gt(i));
        }
        ASSERT(zero == Util::gt(BITS_PER_WORD - 1));
        ASSERT(zero == Util::gt(BITS_PER_WORD));
        ASSERT(zero == Util::gt(BITS_PER_WORD + 1000));

        if (verbose) cout << "Exhaustive Testing of 'le'" << endl;
        for (i = 0; i < (int) BITS_PER_WORD - 1; ++i) {
            LOOP_ASSERT(i, (one << (i + 1)) - 1 == Util::le(i));
        }
        ASSERT(~zero == Util::le(BITS_PER_WORD - 1));
        ASSERT(~zero == Util::le(BITS_PER_WORD));
        ASSERT(~zero == Util::le(BITS_PER_WORD + 1000));

        if (verbose) cout << "Exhaustive Testing of 'lt'" << endl;
        for (i = 0; i < (int) BITS_PER_WORD; ++i) {
            LOOP_ASSERT(i, (one << i) - 1 == Util::lt(i));
            ASSERT(Util::lt(i) == ~Util::ge(i));
        }
        ASSERT(~zero == Util::lt(BITS_PER_WORD));
        ASSERT(~zero == Util::lt(BITS_PER_WORD + 1000));

        if (verbose) cout << "Exhaustive Testing of 'eq64'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64; ++i) {
            LOOP_ASSERT(i, one64 << i == Util::eq64(i));
        }
        ASSERT(zero64 == Util::eq64(i));
        ASSERT(zero64 == Util::eq64(i + 1000));

        if (verbose) cout << "Exhaustive Testing of 'ne64'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64; ++i) {
            LOOP_ASSERT(i, ~(one64 << i) == Util::ne64(i));
        }
        ASSERT(~zero64 == Util::ne64(i));
        ASSERT(~zero64 == Util::ne64(i + 1000));

        if (verbose) cout << "Exhaustive Testing of 'ge64'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64; ++i) {
            LOOP_ASSERT(i, ~zero64 << i == Util::ge64(i));
        }
        ASSERT(zero64 == Util::ge64(i));
        ASSERT(zero64 == Util::ge64(i + 1000));

        if (verbose) cout << "Exhaustive Testing of 'gt64'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64 - 1; ++i) {
            LOOP_ASSERT(i, ~zero64 << (i + 1) == Util::gt64(i));
        }
        ASSERT(zero64 == Util::gt64(BITS_PER_UINT64 - 1));
        ASSERT(zero64 == Util::gt64(BITS_PER_UINT64));
        ASSERT(zero64 == Util::gt64(BITS_PER_UINT64 + 1000));

        if (verbose) cout << "Exhaustive Testing of 'le64'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64 - 1; ++i) {
            LOOP_ASSERT(i, (one64 << (i + 1)) - 1 == Util::le64(i));
        }
        ASSERT(~zero64 == Util::le64(BITS_PER_UINT64 - 1));
        ASSERT(~zero64 == Util::le64(BITS_PER_UINT64));
        ASSERT(~zero64 == Util::le64(BITS_PER_UINT64 + 1000));

        if (verbose) cout << "Exhaustive Testing of 'lt64'" << endl;
        for (i = 0; i < (int) BITS_PER_UINT64; ++i) {
            LOOP_ASSERT(i, (one64 << i) - 1 == Util::lt64(i));
        }
        ASSERT(~zero64 == Util::lt64(i));
        ASSERT(~zero64 == Util::lt64(i + 1000));

        if (verbose) cout << "Testing relationships between mask functions\n";
        for (i = 0; i <= (int) BITS_PER_WORD; ++i) {
            LOOP_ASSERT(i, ~Util::ge(i) == Util::lt(i));
            LOOP_ASSERT(i, ~Util::le(i) == Util::gt(i));
            LOOP_ASSERT(i, ~Util::ne(i) == Util::eq(i));
        }

        if (verbose) cout << "Testing relationships between mask functions\n";
        for (i = 0; i <= (int) BITS_PER_UINT64; ++i) {
            LOOP_ASSERT(i, ~Util::ge64(i) == Util::lt64(i));
            LOOP_ASSERT(i, ~Util::le64(i) == Util::gt64(i));
            LOOP_ASSERT(i, ~Util::ne64(i) == Util::eq64(i));
        }
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING ENUM TYPE VARIABLES
        //   Ensure that the 'enum' values in the 'struct' have the correct
        //   values.
        //
        // Plan:
        //: 1 Carefully define a set of 'const' local "helper" variables
        //:   initialized to appropriate intermediate or final values.
        //: 2 Compare the "helper" variables to the actual 'enum' values is
        //:   the 'struct'.
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
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST:
        //   This case exercises some basic functionality, but *tests* nothing.
        //
        // Concern:
        //: 1 Demonstrate some basic use of this component.
        //
        // Plan:
        //: 1 Just call a few functions and look at their output.
        //
        // Testing:
        //   BREATHING TEST:
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "BREATHING TEST" << endl
                          << "==============" << endl;

        ASSERT(0xffffULL == Util::lt64(16));

        ASSERT(0x7f00ULL == Util::one64(8, 7));

        ASSERT(0xffffffffffff0000ULL == Util::ge64(16));

        for (int ii = 0; ii <= Util::k_BITS_PER_UINT32; ++ii) {
            ASSERT(Util::ge(ii) == ~Util::lt(ii));
            ASSERT(Util::gt(ii) == ~Util::le(ii));

            for (int jj = 0; ii + jj <= Util::k_BITS_PER_UINT32; ++jj) {
                ASSERT(Util::one(ii, jj) == ~Util::zero(ii, jj));
            }
        }

        for (int ii = 0; ii <= Util::k_BITS_PER_UINT64; ++ii) {
            ASSERT(Util::ge64(ii) == ~Util::lt64(ii));
            ASSERT(Util::gt64(ii) == ~Util::le64(ii));

            for (int jj = 0; ii + jj <= Util::k_BITS_PER_UINT64; ++jj) {
                ASSERT(Util::one64(ii, jj) == ~Util::zero64(ii, jj));
            }
        }
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
