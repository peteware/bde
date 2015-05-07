// bdlcc_rawhashtable.t.cpp                                            -*-C++-*-
#include <bdlcc_rawhashtable.h>

#include <bdls_testutil.h>

#include <bslma_allocator.h>
#include <bslma_default.h>
#include <bslma_defaultallocatorguard.h>
#include <bslma_testallocator.h>
#include <bsls_asserttest.h>

#include <bsl_cstdlib.h>
#include <bsl_iostream.h>
#include <bsl_string.h>
#include <bsl_vector.h>

using namespace BloombergLP;
using namespace bsl;

// ============================================================================
//                              TEST PLAN
// ----------------------------------------------------------------------------
//                              Overview
//                              --------
//
// ----------------------------------------------------------------------------
// CLASS METHODS
// ============================================================================

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

#define ASSERT_SAFE_PASS_RAW(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_PASS_RAW(EXPR)
#define ASSERT_SAFE_FAIL_RAW(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_FAIL_RAW(EXPR)
#define ASSERT_PASS_RAW(EXPR)      BSLS_ASSERTTEST_ASSERT_PASS_RAW(EXPR)
#define ASSERT_FAIL_RAW(EXPR)      BSLS_ASSERTTEST_ASSERT_FAIL_RAW(EXPR)
#define ASSERT_OPT_PASS_RAW(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_PASS_RAW(EXPR)
#define ASSERT_OPT_FAIL_RAW(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_FAIL_RAW(EXPR)

// ============================================================================
//                     GLOBAL TYPEDEFS FOR TESTING
// ----------------------------------------------------------------------------

// ============================================================================
//                                 TEST HELPERS
// ----------------------------------------------------------------------------


// ============================================================================
//                                 MAIN PROGRAM
// ----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int             test = argc > 1 ? atoi(argv[1]) : 0;
    bool         verbose = argc > 2;
    bool     veryVerbose = argc > 3;
    bool veryVeryVerbose = argc > 4;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;


    switch (test) { case 0:
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST:
        //   Developers' Sandbox.
        //
        // Plan:
        //   Perform and ad-hoc test of the primary modifiers and accessors.
        //
        // Testing:
        //   This "test" *exercises* basic functionality, but *tests* nothing.
        // --------------------------------------------------------------------

        if (verbose) cout << endl << "BREATHING TEST" << endl
                                  << "==============" << endl;

        typedef bsl::string Key;
        typedef bsl::string Value;
        typedef bdlcc::Hashtable<Key, Value> Obj;

        {
            // Breathing test constructor and find.
            Obj x(100); const Obj& X = x;
            ASSERT(0 == X.find(bsl::string("A")));
            ASSERT(0 == X.find(bsl::string("B")));
            ASSERT(0 == X.find(bsl::string("C")));

        }

        {
            // Breathing test insert and find.

            Obj x(100); const Obj& X = x;
            ASSERT(0 == X.find(bsl::string("A")));
            ASSERT(0 == X.find(bsl::string("B")));
            ASSERT(0 == X.find(bsl::string("C")));
        
            x.insert(bsl::string("A"), bsl::string("valA"));
            ASSERT(0 != X.find(bsl::string("A")));
            ASSERT(0 == X.find(bsl::string("B")));
            ASSERT(0 == X.find(bsl::string("C")));
            ASSERT("valA" == *X.find(bsl::string("A")));
        
            x.insert(bsl::string("B"), bsl::string("valB"));
            ASSERT(0 != X.find(bsl::string("A")));
            ASSERT(0 != X.find(bsl::string("B")));
            ASSERT(0 == X.find(bsl::string("C")));
            ASSERT("valA" == *X.find(bsl::string("A")));
            ASSERT("valB" == *X.find(bsl::string("B")));
        }
        {
            // Breathing test delete.

            Obj x(100); const Obj& X = x;

            x.insert(bsl::string("A"), bsl::string("valA"));
            x.insert(bsl::string("B"), bsl::string("valB"));
            x.insert(bsl::string("C"), bsl::string("valC"));

            ASSERT("valA" == *X.find(bsl::string("A")));
            ASSERT("valB" == *X.find(bsl::string("B")));
            ASSERT("valC" == *X.find(bsl::string("C")));

            x.remove(bsl::string("A"));

            ASSERT(!X.find(bsl::string("A")));
            ASSERT("valB" == *X.find(bsl::string("B")));
            ASSERT("valC" == *X.find(bsl::string("C")));

            x.remove(bsl::string("B"));

            ASSERT(!X.find(bsl::string("A")));
            ASSERT(!X.find(bsl::string("B")));
            ASSERT("valC" == *X.find(bsl::string("C")));

            x.insert(bsl::string("A"), bsl::string("valA"));        

            ASSERT("valA" == *X.find(bsl::string("A")));
            ASSERT(!X.find(bsl::string("B")));
            ASSERT("valC" == *X.find(bsl::string("C")));

            x.insert(bsl::string("B"), bsl::string("valB"));        

            ASSERT("valA" == *X.find(bsl::string("A")));
            ASSERT("valB" == *X.find(bsl::string("B")));
            ASSERT("valC" == *X.find(bsl::string("C")));
        }
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

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2011
//      All Rights Reserved.
//      Property of Bloomberg L.P.  (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
