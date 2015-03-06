// bbldcu_actual360.t.cpp                                             -*-C++-*-

#include <bbldcu_actual360.h>

#include <bdlt_date.h>

#include <bsl_cstdlib.h>     // atoi()
#include <bsl_iostream.h>

using namespace BloombergLP;
using namespace bsl;  // automatically added by script




// =========================================================================
//                              TEST PLAN
// -------------------------------------------------------------------------
//                              Overview
//                              --------
// The component under test consists of static member functions (pure
// procedures) that compute the day count and term between two dates.  The
// general plan is that the methods are tested against a set of tabulated test
// vectors.
//-----------------------------------------------------------------------------
// [ 1] int daysDiff(const bdlt::Date& beginDate, const bdlt::Date& endDate);
// [ 2] double yearsDiff(const bdlt::Date& beginDate, const bdlt::Date& endDate);
// -------------------------------------------------------------------------
// [ 3] USAGE EXAMPLE
//--------------------------------------------------------------------------



// ============================================================================
//                  STANDARD BDE ASSERT TEST MACROS
// ----------------------------------------------------------------------------

static int testStatus = 0;

static void aSsErT(int c, const char *s, int i)
{
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (0 <= testStatus && testStatus <= 100) ++testStatus;
    }
}

#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }
//-----------------------------------------------------------------------------
#define LOOP_ASSERT(I,X) { \
   if (!(X)) { cout << #I << ": " << I << "\n"; aSsErT(1, #X, __LINE__); }}

#define LOOP2_ASSERT(I,J,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " \
              << J << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP3_ASSERT(I,J,K,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" \
              << #K << ": " << K << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP4_ASSERT(I,J,K,L,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" << \
       #K << ": " << K << "\t" << #L << ": " << L << "\n"; \
       aSsErT(1, #X, __LINE__); } }


//=============================================================================
//                  SEMI-STANDARD TEST OUTPUT MACROS
//-----------------------------------------------------------------------------

#define P(X) cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X) cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define P_(X) cout << #X " = " << (X) << ", "<< flush; // P(X) without '\n'
#define L_ __LINE__                           // current Line number
#define T_  cout << "\t" << flush;          // Print tab w/o newline

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

typedef bbldcu::Actual360 Obj;

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[]) {

    int test = argc > 1 ? atoi(argv[1]) : 0;
    int verbose = argc > 2;
    int veryVerbose = argc > 3;
    int veryVeryVerbose = argc > 4;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;

    switch (test) { case 0:
      case 3: {
       // --------------------------------------------------------------------
        // TESTING USAGE EXAMPLE
        // Concerns:
        //   The usage example provided in the component header file must
        //   compile, link, and run on all platforms as shown.
        //
        // Plan:
        //   Incorporate usage example from header into driver, remove leading
        //   comment characters, and replace 'assert' with 'ASSERT'.  Suppress
        //   all 'cout' statements in non-verbose mode, and add streaming to
        //   a buffer to test programmatically the printing examples.
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Usage Examples"
                          << "\n======================" << endl;

        const bdlt::Date dA(2004,  2,  1);
        const bdlt::Date dB(2004,  3,  1);
        const bdlt::Date dC(2004,  5,  1);
        const bdlt::Date dD(2005,  2,  1);

        int daysDiff;
        daysDiff = bbldcu::Actual360::daysDiff(dA, dB);
        ASSERT( 29 == daysDiff);
        daysDiff = bbldcu::Actual360::daysDiff(dA, dC);
        ASSERT( 90 == daysDiff);
        daysDiff = bbldcu::Actual360::daysDiff(dA, dD);
        ASSERT(366 == daysDiff);
        daysDiff = bbldcu::Actual360::daysDiff(dB, dC);
        ASSERT( 61 == daysDiff);

        double yearsDiff;
        yearsDiff = bbldcu::Actual360::yearsDiff(dA, dC);
        ASSERT(0.25 == yearsDiff);
        yearsDiff = bbldcu::Actual360::yearsDiff(dA, dD);
        ASSERT(yearsDiff < 1.0167 && yearsDiff > 1.0166);
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING STATIC yearsDiff FUNCTION:
        //
        // Concerns:
        //   Each function must return the expected value.
        //
        // Plan:
        //   Specify a set S of {pairs of dates (d1, d2) and their difference
        //   in years D}.  For each of the operators under test, in a loop over
        //   the elements of S, apply the method to dates having the values d1
        //   and d2 and confirm the results using the value D.
        //
        // Testing
        //   double yearsDiff(const bdlt::Date&, const bdlt::Date&);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting: 'yearsDiff(beginDate, endDate)'"
                          << "\n========================================"
                          << endl;

        {
            static const struct {
                int    d_lineNum;   // source line number
                int    d_year1;     // beginDate year
                int    d_month1;    // beginDate month
                int    d_day1;      // beginDate day
                int    d_year2;     // endDate year
                int    d_month2;    // endDate month
                int    d_day2;      // endDate day
                double d_numYears;  // result number of years
            } DATA[] = {
                //       - - - -first- - - -   - - - second- - - -
                //line   year   month   day    year   month   day    numYears
                //----   -----  -----  -----   -----  -----  -----   --------
                { L_,     1992,     2,     1,   1992,     3,     1,   0.0806 },
                { L_,     1992,     2,     1,   1993,     3,     1,   1.0944 },

                { L_,     1993,     1,     1,   1993,     2,    21,   0.1417 },
                { L_,     1993,     1,     1,   1994,     1,     1,   1.0139 },
                { L_,     1993,     1,    15,   1993,     2,     1,   0.0472 },
                { L_,     1993,     2,     1,   1993,     3,     1,   0.0778 },
                { L_,     1993,     2,     1,   1996,     2,     1,   3.0417 },

                { L_,     1993,     2,     1,   1993,     3,     1,   0.0778 },
                { L_,     1993,     2,    15,   1993,     4,     1,   0.1250 },

                { L_,     1993,     3,    15,   1993,     6,    15,   0.2556 },
                { L_,     1993,     3,    31,   1993,     4,     1,   0.0028 },
                { L_,     1993,     3,    31,   1993,     4,    30,   0.0833 },
                { L_,     1993,     3,    31,   1993,    12,    31,   0.7639 },

                { L_,     1993,     7,    15,   1993,     9,    15,   0.1722 },

                { L_,     1993,     8,    21,   1994,     4,    11,   0.6472 },

                { L_,     1993,    11,     1,   1994,     3,     1,   0.3333 },

                { L_,     1993,    12,    15,   1993,    12,    30,   0.0417 },
                { L_,     1993,    12,    15,   1993,    12,    31,   0.0444 },
                { L_,     1993,    12,    31,   1994,     2,     1,   0.0889 },

                { L_,     1996,     1,    15,   1996,     5,    31,   0.3806 },

                { L_,     1998,     2,    27,   1998,     3,    27,   0.0778 },
                { L_,     1998,     2,    28,   1998,     3,    27,   0.0750 },

                { L_,     1999,     1,     1,   1999,     1,    29,   0.0778 },
                { L_,     1999,     1,    29,   1999,     1,    30,   0.0028 },
                { L_,     1999,     1,    29,   1999,     1,    31,   0.0056 },
                { L_,     1999,     1,    29,   1999,     3,    29,   0.1639 },
                { L_,     1999,     1,    29,   1999,     3,    30,   0.1667 },
                { L_,     1999,     1,    29,   1999,     3,    31,   0.1694 },
                { L_,     1999,     1,    30,   1999,     1,    31,   0.0028 },
                { L_,     1999,     1,    30,   1999,     2,    27,   0.0778 },
                { L_,     1999,     1,    30,   1999,     2,    28,   0.0806 },
                { L_,     1999,     1,    30,   1999,     3,    29,   0.1611 },
                { L_,     1999,     1,    30,   1999,     3,    30,   0.1639 },
                { L_,     1999,     1,    30,   1999,     3,    31,   0.1667 },
                { L_,     1999,     1,    31,   1999,     3,    29,   0.1583 },
                { L_,     1999,     1,    31,   1999,     3,    30,   0.1611 },
                { L_,     1999,     1,    31,   1999,     3,    31,   0.1639 },
            };

            const int NUM_DATA = sizeof DATA / sizeof *DATA;

            int di;
            if (verbose) cout <<
                "\nTesting: 'yearsDiff(beginDate, endDate)'" << endl;
            for (di = 0; di < NUM_DATA ; ++di) {
                const int    LINE      = DATA[di].d_lineNum;
                const double NUM_YEARS = DATA[di].d_numYears;
                bdlt::Date x(DATA[di].d_year1,
                            DATA[di].d_month1,
                            DATA[di].d_day1);
                const bdlt::Date& X = x;
                bdlt::Date y(DATA[di].d_year2,
                            DATA[di].d_month2,
                            DATA[di].d_day2);
                const bdlt::Date& Y = y;
                if (veryVerbose) { T_;  P_(X);  P_(Y);  P_(NUM_YEARS); }
                const double RESULT = Obj::yearsDiff(X, Y);

                if (veryVerbose) { P(RESULT); }
                const double diff = NUM_YEARS - RESULT;
                LOOP_ASSERT(LINE, -0.00005 <= diff && diff <= 0.00005);
            }
        }
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // TESTING STATIC daysDiff FUNCTION:
        //
        // Concerns:
        //   Each function must return the expected value.  Use of white-box
        //   testing reduces the number of needed negative-result cases.
        //
        // Plan:
        //   Specify a set S of {pairs of dates (d1, d2) and their difference
        //   in days D}.  For each of the operators under test, in a loop over
        //   the elements of S, apply the method to dates having the values d1
        //   and d2 and confirm the results using the value D.
        //
        // Testing
        //   int daysDiff(const bdlt::Date&, const bdlt::Date&);
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting: 'daysDiff(beginDate, endDate)'"
                          << "\n======================================="
                          << endl;

        {
            static const struct {
                int d_lineNum;   // source line number
                int d_year1;     // beginDate year
                int d_month1;    // beginDate month
                int d_day1;      // beginDate day
                int d_year2;     // endDate year
                int d_month2;    // endDate month
                int d_day2;      // endDate day
                int d_numDays;   // result number of days
            } DATA[] = {
                //       - - - -begin- - - -   - - - end - - - - -
                //line   year   month   day    year   month   day    numDays
                //----   -----  -----  -----   -----  -----  -----   -------
                { L_,     1992,     2,     1,   1992,     3,     1,       29 },
                { L_,     1992,     2,     1,   1993,     3,     1,      394 },

                { L_,     1993,     1,     1,   1993,     2,    21,       51 },
                { L_,     1993,     1,     1,   1994,     1,     1,      365 },
                { L_,     1993,     1,    15,   1993,     2,     1,       17 },
                { L_,     1993,     2,     1,   1993,     3,     1,       28 },
                { L_,     1993,     2,     1,   1996,     2,     1,     1095 },

                { L_,     1993,     2,     1,   1993,     3,     1,       28 },
                { L_,     1993,     2,    15,   1993,     4,     1,       45 },

                { L_,     1993,     3,    15,   1993,     6,    15,       92 },
                { L_,     1993,     3,    31,   1993,     4,     1,        1 },
                { L_,     1993,     3,    31,   1993,     4,    30,       30 },
                { L_,     1993,     3,    31,   1993,    12,    31,      275 },

                { L_,     1993,     7,    15,   1993,     9,    15,       62 },

                { L_,     1993,     8,    21,   1994,     4,    11,      233 },

                { L_,     1993,    11,     1,   1994,     3,     1,      120 },

                { L_,     1993,    12,    15,   1993,    12,    30,       15 },
                { L_,     1993,    12,    15,   1993,    12,    31,       16 },
                { L_,     1993,    12,    31,   1994,     2,     1,       32 },

                { L_,     1996,     1,    15,   1996,     5,    31,      137 },

                { L_,     1998,     2,    27,   1998,     3,    27,       28 },
                { L_,     1998,     2,    28,   1998,     3,    27,       27 },

                { L_,     1999,     1,     1,   1999,     1,    29,       28 },
                { L_,     1999,     1,    29,   1999,     1,    30,        1 },
                { L_,     1999,     1,    29,   1999,     1,    31,        2 },
                { L_,     1999,     1,    29,   1999,     3,    29,       59 },
                { L_,     1999,     1,    29,   1999,     3,    30,       60 },
                { L_,     1999,     1,    29,   1999,     3,    31,       61 },
                { L_,     1999,     1,    30,   1999,     1,    31,        1 },
                { L_,     1999,     1,    30,   1999,     2,    27,       28 },
                { L_,     1999,     1,    30,   1999,     2,    28,       29 },
                { L_,     1999,     1,    30,   1999,     3,    29,       58 },
                { L_,     1999,     1,    30,   1999,     3,    30,       59 },
                { L_,     1999,     1,    30,   1999,     3,    31,       60 },
                { L_,     1999,     1,    31,   1999,     3,    29,       57 },
                { L_,     1999,     1,    31,   1999,     3,    30,       58 },
                { L_,     1999,     1,    31,   1999,     3,    31,       59 },

                { L_,     1999,     2,    27,   1999,     2,    27,        0 },
                { L_,     1999,     2,    27,   1999,     2,    28,        1 },
                { L_,     1999,     2,    28,   1999,     2,    27,       -1 },
                { L_,     1999,     2,    28,   1999,     2,    28,        0 },

                { L_,     2000,     1,    29,   2000,     1,    30,        1 },
                { L_,     2000,     1,    29,   2000,     1,    31,        2 },
                { L_,     2000,     1,    29,   2000,     3,    29,       60 },
                { L_,     2000,     1,    29,   2000,     3,    30,       61 },
                { L_,     2000,     1,    29,   2000,     3,    31,       62 },
                { L_,     2000,     1,    30,   2000,     1,    31,        1 },
                { L_,     2000,     1,    30,   2000,     2,    27,       28 },
                { L_,     2000,     1,    30,   2000,     2,    28,       29 },
                { L_,     2000,     1,    30,   2000,     2,    29,       30 },
                { L_,     2000,     1,    30,   2000,     3,    29,       59 },
                { L_,     2000,     1,    30,   2000,     3,    30,       60 },
                { L_,     2000,     1,    30,   2000,     3,    31,       61 },
                { L_,     2000,     1,    31,   2000,     3,    29,       58 },
                { L_,     2000,     1,    31,   2000,     3,    30,       59 },
                { L_,     2000,     1,    31,   2000,     3,    31,       60 },

                { L_,     2000,     2,    27,   2000,     2,    27,        0 },
                { L_,     2000,     2,    27,   2000,     2,    28,        1 },
                { L_,     2000,     2,    27,   2000,     2,    29,        2 },
                { L_,     2000,     2,    28,   2000,     2,    27,       -1 },
                { L_,     2000,     2,    28,   2000,     2,    28,        0 },
                { L_,     2000,     2,    28,   2000,     2,    29,        1 },
                { L_,     2000,     2,    29,   2000,     2,    27,       -2 },
                { L_,     2000,     2,    29,   2000,     2,    28,       -1 },
                { L_,     2000,     2,    29,   2000,     2,    29,        0 },

                { L_,     2000,     7,    29,   2000,     8,    31,       33 },
                { L_,     2000,     7,    29,   2000,     9,     1,       34 },
                { L_,     2000,     7,    30,   2000,     8,    31,       32 },
                { L_,     2000,     7,    30,   2000,     9,     1,       33 },
                { L_,     2000,     7,    31,   2000,     8,    31,       31 },
                { L_,     2000,     7,    31,   2000,     9,     1,       32 },

                { L_,     2000,     8,     1,   2000,     8,    31,       30 },
                { L_,     2000,     8,     1,   2000,     9,     1,       31 },

                { L_,     2003,     2,    28,   2004,     2,    29,      366 },
            };

            const int NUM_DATA = sizeof DATA / sizeof *DATA;

            int di;
            if (verbose) cout <<
                "\nTesting: 'daysDiff(beginDate, endDate)'" << endl;
            for (di = 0; di < NUM_DATA ; ++di) {
                const int LINE     = DATA[di].d_lineNum;
                const int NUM_DAYS = DATA[di].d_numDays;
                bdlt::Date x(DATA[di].d_year1,
                            DATA[di].d_month1,
                            DATA[di].d_day1);
                const bdlt::Date& X = x;
                bdlt::Date y(DATA[di].d_year2,
                            DATA[di].d_month2,
                            DATA[di].d_day2);
                const bdlt::Date& Y = y;
                if (veryVerbose) { T_;  P_(X);  P_(Y);  P_(NUM_DAYS); }
                const int RESULT = Obj::daysDiff(X, Y);

                if (veryVerbose) { P(RESULT); }
                LOOP_ASSERT(LINE, NUM_DAYS == RESULT);
            }
        }
      } break;
      default: {
        cerr << "WARNING: CASE `" << test << "' NOT == FOUND." << endl;
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
