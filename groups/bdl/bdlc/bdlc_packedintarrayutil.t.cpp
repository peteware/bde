// bdlc_packedintarrayutil.t.cpp                                      -*-C++-*-
#include <bdlc_packedintarrayutil.h>

#include <bdlc_packedintarray.h>

#include <bdls_testutil.h>

#include <bsls_asserttest.h>

#include <bsl_algorithm.h>
#include <bsl_iostream.h>
#include <bsl_vector.h>

#include <stdlib.h>

using namespace BloombergLP;
using namespace bsl;

//=============================================================================
//                              TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
// 'bdlc::PackedCalendarUtil' provides a suite of common non-primitive
// operations on 'bdlc::PackedIntArray' objects.  This test driver tests each
// implemented utility function.
//-----------------------------------------------------------------------------
// [ 1] bool isSorted(PIACI first, PIACI last);
// [ 2] PIACI lower_bound(PIACI first, PIACI last, const T& value);
// [ 2] PIACI upper_bound(PIACI first, PIACI last, const T& value);
// ----------------------------------------------------------------------------
// [ 3] USAGE EXAMPLE
// ----------------------------------------------------------------------------

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


// ============================================================================
//                  NEGATIVE-TEST MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT_SAFE_PASS(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_PASS(EXPR)
#define ASSERT_SAFE_FAIL(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_FAIL(EXPR)
#define ASSERT_PASS(EXPR)      BSLS_ASSERTTEST_ASSERT_PASS(EXPR)
#define ASSERT_FAIL(EXPR)      BSLS_ASSERTTEST_ASSERT_FAIL(EXPR)
#define ASSERT_OPT_PASS(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_PASS(EXPR)
#define ASSERT_OPT_FAIL(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_FAIL(EXPR)

// ============================================================================
//                                 MAIN PROGRAM
// ----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    const int     test = argc > 1 ? atoi(argv[1]) : 0;
    const bool verbose = argc > 2;

    std::cout << "TEST " << __FILE__ << " CASE " << test << std::endl;

    switch (test) { case 0:
      case 3: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE
        //   Extracted from component header file.
        //
        // Concerns:
        //: 1 The usage example provided in the component header file compiles,
        //:   links, and runs as shown.
        //
        // Plan:
        //: 1 Incorporate usage example from header into test driver, remove
        //:   leading comment characters, and replace 'assert' with 'ASSERT'.
        //:   (C-1)
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "USAGE EXAMPLE" << endl
                          << "=============" << endl;

///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: lower_bound
/// - - - - - - - - - - -
// Suppose that given an 'bdlc::PackedIntArray', we want to find the first
// value greater than or equal to the value 17.  First, create and populate
// with sorted data the 'bdlc::PackedIntArray' to be searched::
//..
    bdlc::PackedIntArray<int> array;

    array.push_back( 5);
    array.push_back( 9);
    array.push_back(15);
    array.push_back(19);
    array.push_back(23);
    array.push_back(36);
    ASSERT(6 == array.length());
//..
// Finally, use 'bdlc::PackedIntArrayUtil::lower_bound' to find the desired
// value:
//..
    ASSERT(19 == *bdlc::PackedIntArrayUtil::lower_bound(array.begin(),
                                                        array.end(),
                                                        17));
//..
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING 'lower_value' AND 'upper_value'
        //   The methods operate as expected.
        //
        // Concerns:
        //: 1 The methods return the expected iterator value.
        //:
        //: 2 QoI: asserted precondition violations are detected when enabled.
        //
        // Plan:
        //: 1 Use a 'bsl::vector', 'bsl::lower_bound', and 'bsl::upper_bound'
        //:   as an oracle to verify the methods' operation.  (C-1)
        //:
        //: 2 Verify defensive checks are triggered for invalid values.  (C-2)
        //
        // Testing:
        //   PIACI lower_bound(PIACI first, PIACI last, const T& value);
        //   PIACI upper_bound(PIACI first, PIACI last, const T& value);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING 'lower_value' AND 'upper_value'" << endl
                          << "=======================================" << endl;

        if (verbose) cout << "\nVerifying results against oracle." << endl;

        bsl::vector<int>        mOracle;
        const bsl::vector<int>& ORACLE = mOracle;

        bdlc::PackedIntArray<int>        mArray;
        const bdlc::PackedIntArray<int>& ARRAY = mArray;

        bsl::vector<int>::const_iterator EXP;

        bdlc::PackedIntArrayConstIterator<int> rv;


        const int         DATA[] = { 23, 37, 49, 56, 98 };
        const bsl::size_t NUM = sizeof DATA / sizeof *DATA;

        {
            // Verify the methods with an empty array.

            for (int v = 0; v < 100; ++v) {
                EXP = bsl::lower_bound(ORACLE.begin(), ORACLE.end(), v);
                rv = bdlc::PackedIntArrayUtil::lower_bound(
                                                ARRAY.begin(), ARRAY.end(), v);
                LOOP_ASSERT(v, rv != ARRAY.end() || EXP == ORACLE.end());
                LOOP_ASSERT(v, rv == ARRAY.end() || *rv  == *EXP);
            }
            for (int v = 0; v < 100; ++v) {
                EXP = bsl::upper_bound(ORACLE.begin(), ORACLE.end(), v);
                rv = bdlc::PackedIntArrayUtil::upper_bound(
                                                ARRAY.begin(), ARRAY.end(), v);
                LOOP_ASSERT(v, rv != ARRAY.end() || EXP == ORACLE.end());
                LOOP_ASSERT(v, rv == ARRAY.end() || *rv  == *EXP);
            }
        }

        for (bsl::size_t i = 0; i < NUM; ++i) {

            // Insert a value and then verify the methods.

            mOracle.push_back(DATA[i]);
            mArray.push_back(DATA[i]);

            for (int v = 0; v < 100; ++v) {
                EXP = bsl::lower_bound(ORACLE.begin(), ORACLE.end(), v);
                rv = bdlc::PackedIntArrayUtil::lower_bound(
                                                ARRAY.begin(), ARRAY.end(), v);
                LOOP2_ASSERT(i, v, rv != ARRAY.end() || EXP == ORACLE.end());
                LOOP2_ASSERT(i, v, rv == ARRAY.end() || *rv  == *EXP);
            }
            for (int v = 0; v < 100; ++v) {
                EXP = bsl::upper_bound(ORACLE.begin(), ORACLE.end(), v);
                rv = bdlc::PackedIntArrayUtil::upper_bound(
                                                ARRAY.begin(), ARRAY.end(), v);
                LOOP2_ASSERT(i, v, rv != ARRAY.end() || EXP == ORACLE.end());
                LOOP2_ASSERT(i, v, rv == ARRAY.end() || *rv  == *EXP);
            }
        }

        if (verbose) cout << "\nNegative testing." << endl;
        {
            bsls::AssertFailureHandlerGuard
                                          hG(bsls::AssertTest::failTestDriver);

            // Valid.

            ASSERT_PASS(bdlc::PackedIntArrayUtil::lower_bound(
                                               ARRAY.begin(), ARRAY.end(), 3));

            ASSERT_PASS(bdlc::PackedIntArrayUtil::upper_bound(
                                               ARRAY.begin(), ARRAY.end(), 3));

            // 'first > last'

            ASSERT_FAIL(bdlc::PackedIntArrayUtil::lower_bound(
                                               ARRAY.end(), ARRAY.begin(), 3));

            ASSERT_FAIL(bdlc::PackedIntArrayUtil::upper_bound(
                                               ARRAY.end(), ARRAY.begin(), 3));

            // Unsorted data.

            mArray.push_back(1);

            ASSERT_SAFE_FAIL(bdlc::PackedIntArrayUtil::lower_bound(
                                               ARRAY.begin(), ARRAY.end(), 3));

            ASSERT_SAFE_FAIL(bdlc::PackedIntArrayUtil::upper_bound(
                                               ARRAY.begin(), ARRAY.end(), 3));
        }
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // TESTING 'isSorted'
        //   The method operates as expected.
        //
        // Concerns:
        //: 1 The method returns the expected value.
        //:
        //: 2 If the range is empty, the method always returns 'true'.
        //
        // Plan:
        //: 1 Use the table-driven technique to test the method.  (C-1)
        //:
        //: 2 Directly test empty ranges.  (C-2)
        //
        // Testing:
        //   bool isSorted(PIACI first, PIACI last);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING 'isSorted'" << endl
                          << "==================" << endl;

        static const struct {
            int         d_line;       // source line number
            const int   d_array[3];   // data to test
            bsl::size_t d_len;        // length of 'd_array'
            bool        d_exp;        // expected value
        } DATA[] = {
            // LINE     ARRAY     LEN  EXP
            // ----  -----------  ---  -----
            {    L_,         { },   0, true  },
            {    L_,       { 1 },   1, true  },
            {    L_,    { 1, 2 },   2, true  },
            {    L_,    { 2, 1 },   2, false },
            {    L_, { 1, 2, 3 },   3, true  },
            {    L_, { 1, 3, 2 },   3, false },
            {    L_, { 2, 1, 3 },   3, false },
            {    L_, { 2, 3, 1 },   3, false },
            {    L_, { 3, 1, 2 },   3, false },
            {    L_, { 3, 2, 1 },   3, false }
        };
        bsl::size_t NUM = sizeof DATA / sizeof *DATA;

        for (bsl::size_t i = 0; i < NUM; ++i) {
            const int         LINE  = DATA[i].d_line;
            const int *const  ARRAY = DATA[i].d_array;
            const bsl::size_t LEN   = DATA[i].d_len;
            bool              EXP   = DATA[i].d_exp;

            bdlc::PackedIntArray<int> array;
            for (bsl::size_t j = 0; j < LEN; ++j) {
                array.push_back(ARRAY[j]);
            }
            LOOP_ASSERT(LINE, LEN == array.length());

            LOOP_ASSERT(LINE, EXP ==  bdlc::PackedIntArrayUtil::isSorted(
                                                  array.begin(), array.end()));

            // Verify empty range.

            LOOP_ASSERT(LINE, true == bdlc::PackedIntArrayUtil::isSorted(
                                                  array.end(), array.begin()));
        }
      } break;
      default: {
        bsl::cerr << "WARNING: CASE `" << test << "' NOT FOUND." << bsl::endl;
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        bsl::cerr << "Error, non-zero test status = " << testStatus
                  << "." << bsl::endl;
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
