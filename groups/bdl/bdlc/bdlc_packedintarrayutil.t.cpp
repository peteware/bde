// bdlc_packedintarrayutil.t.cpp                                      -*-C++-*-
#include <bdlc_packedintarrayutil.h>

#include <bdlc_packedintarray.h>

#include <bdls_testutil.h>

#include <bsls_asserttest.h>
#include <bsls_stopwatch.h>

#include <bsl_algorithm.h>
#include <bsl_iostream.h>
#include <bsl_limits.h>
#include <bsl_vector.h>

using namespace BloombergLP;
using namespace bsl;

//=============================================================================
//                              TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
// 'bdlt::DateUtil' provides a suite of functions for manipulating dates
// without the use of a calendar.  This test driver tests each implemented
// utility function independently.
//
//-----------------------------------------------------------------------------
// CLASS METHODS
// ----------------------------------------------------------------------------
// [  ] USAGE EXAMPLE

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
    const int         test = argc > 1 ? atoi(argv[1]) : 0;
    const bool     verbose = argc > 2;
    const bool veryVerbose = argc > 3;

    std::cout << "TEST " << __FILE__ << " CASE " << test << std::endl;

    switch (test) { case 0:
      case 17: {
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
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // TESTING 'isValidYYYYMMDD'
        //   The function correctly discriminates between valid and invalid
        //   dates in the "YYYYMMDD" format.
        //
        // Concerns:
        //: 1 The function works for valid and invalid inputs.
        //
        // Plan:
        //: 1 Use the table-driven approach to test representative valid and
        //:   invalid inputs, and verify the function returns the expected
        //:   result.  (C-1)
        //
        // Testing:
        //   bool isValidYYYYMMDD(int yyyymmddValue);
        // --------------------------------------------------------------------

        bsl::vector<int>        mOracle;
        const bsl::vector<int>& ORACLE = mOracle;

        bdlc::PackedIntArray<int>        mArray;
        const bdlc::PackedIntArray<int>& ARRAY = mArray;

        bsl::vector<int>::const_iterator EXP;
        
        bdlc::PackedIntArrayConstIterator<int> rv;


        const int         DATA[] = { 23, 37, 56, 49, 98 };
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
    
      } break;
      default: {
        fprintf(stderr, "WARNING: CASE `%d' NOT FOUND.\n", test);
        testStatus = -1;
      }
    }
    if (testStatus > 0) {
        cerr << "Error, non-zero test status = " << testStatus << "." << endl;
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
