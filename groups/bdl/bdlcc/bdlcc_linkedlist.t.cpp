// bdlcc_linkedlist.t.cpp                                                -*-C++-*-
#include <bdlcc_linkedlist.h>

#include <bdls_testutil.h>

#include <bsls_assert.h>
#include <bsls_asserttest.h>
#include <bsls_platform.h>

#include <bsl_vector.h>

using namespace bsl;
using namespace BloombergLP;


//=============================================================================
//                             TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
//-----------------------------------------------------------------------------
// MACROS
// [ 6] BDLS_TESTUTIL_ASSERT
//-----------------------------------------------------------------------------
// [ 8] USAGE EXAMPLE
// [ 1] BREATHING TEST

// ============================================================================
//                    STANDARD BDE ASSERT TEST MACRO
// ----------------------------------------------------------------------------

namespace {

int testStatus = 0;

void aSsErT(int c, const char *s, int i)
{
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (0 <= testStatus && testStatus <= 100) ++testStatus;
    }
}

}  // close unnamed namespace

// ============================================================================
//                       STANDARD BDE TEST DRIVER MACROS
// ----------------------------------------------------------------------------

#define ASSERT       BDLS_TESTUTIL_ASSERT
#define LOOP_ASSERT  BDLS_TESTUTIL_LOOP_ASSERT
#define LOOP0_ASSERT BDLS_TESTUTIL_LOOP0_ASSERT
#define LOOP1_ASSERT BDLS_TESTUTIL_LOOP1_ASSERT
#define LOOP2_ASSERT BDLS_TESTUTIL_LOOP2_ASSERT
#define LOOP3_ASSERT BDLS_TESTUTIL_LOOP3_ASSERT
#define LOOP4_ASSERT BDLS_TESTUTIL_LOOP4_ASSERT
#define LOOP5_ASSERT BDLS_TESTUTIL_LOOP5_ASSERT
#define LOOP6_ASSERT BDLS_TESTUTIL_LOOP6_ASSERT
#define ASSERTV      BDLS_TESTUTIL_ASSERTV

#define Q   BDLS_TESTUTIL_Q   // Quote identifier literally.
#define P   BDLS_TESTUTIL_P   // Print identifier and value.
#define P_  BDLS_TESTUTIL_P_  // P(X) without '\n'.
#define T_  BDLS_TESTUTIL_T_  // Print a tab (w/o newline).
#define L_  BDLS_TESTUTIL_L_  // current Line number

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
//                               TEST APPARATUS
// ----------------------------------------------------------------------------

typedef bdlcc::List List;
typedef bdlcc::Node Node;
//=============================================================================
//                                 MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;
    int verbose = argc > 2;
    int veryVerbose = argc > 3;


    cerr << "TEST " << __FILE__ << " CASE " << test << endl;

    switch (test) { case 0:  // Zero is always the leading case.
      case 2: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE
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


        if (verbose) cerr << endl
                          << "USAGE EXAMPLE" << endl
                          << "=============" << endl;

	  } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST
        //   Developers' Sandbox.
        //
        // Plan:
        //   Perform and ad-hoc test of the primary modifiers and accessors.
        //
        // Testing:
        //   This "test" *exercises* basic functionality, but *tests* nothing.
        // --------------------------------------------------------------------

        if (verbose) cerr << endl
                          << "BREATHING TEST" << endl
                          << "==============" << endl;

		bsl::vector<Node> nodes;
		nodes.resize(10);
		
		for (unsigned int i = 0; i < nodes.size(); ++i) {
			nodes[i].d_value = i;
		}
		
		List list;
		for (unsigned int i = 0; i < nodes.size(); ++i) {
			list.insert(&nodes[i]);
		}
		list.print(bsl::cout);
		
		ASSERT(&nodes[3] == list.remove(&nodes[3]));
		ASSERT(0         == list.remove(&nodes[3]));
		
		ASSERT(&nodes[5] == list.remove(&nodes[5]));
		ASSERT(0         == list.remove(&nodes[5]));

		list.print(bsl::cout);
		
		Node node3;
		node3.d_value = 15;
		
		bsl::cout << "insert" << bsl::endl;
		list.insert(&node3);
		bsl::cout << "print" << bsl::endl;
		list.print(bsl::cout);
		
		
      } break;
      default: {
        cerr << "WARNING: CASE `" << test << "' NOT FOUND." << endl;
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        cerr << "Error, non-zero test status = " << testStatus << "." << endl;
    }
    return testStatus;
}

// ----------------------------------------------------------------------------
// Copyright (C) 2012 Bloomberg Finance L.P.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
