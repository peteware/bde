// bbldcu_actual360.h                                                 -*-C++-*-
#ifndef INCLUDED_BBLDCU_ACTUAL360
#define INCLUDED_BBLDCU_ACTUAL360

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide support for the Actual/360 day-count convention.
//
//@CLASSES:
//  bbldc::Actual360: procedures relating to Actual/360 day-count convention
//
//@DESCRIPTION: This component provides a namespace for pure procedures that
// manipulate dates as prescribed by the Actual/360 day-count convention.  In
// this convention, we simply measure the Julian days that have occurred in a
// time period, and to calculate years, divide that by 360.  Note that this
// means the number of years between January 1, 2005 and January 1, 2006 comes
// out to about 1.0139.  No end-of-month rule adjustments are made.  Given
// 'beginDate' and 'endDate':
//..
//  yearsDiff ::= sign(endDate - beginDate) *
//                         (Julian days between beginDate and endDate) / 360.0
//..
//
///Usage
///-----
// The following snippets of code illustrate how to use 'bbldc::Actual360'
// methods.  First, create two 'bdlt::Dates' 'd1' and 'd2':
//..
//  const bdlt::Date dA(2004,  2,  1);
//  const bdlt::Date dB(2004,  3,  1);
//  const bdlt::Date dC(2004,  5,  1);
//  const bdlt::Date dD(2005,  2,  1);
//..
// To compute the day-count between these two dates:
//..
//  int daysDiff;
//  daysDiff = bbldc::Actual360::daysDiff(dA, dB);
//  assert( 29 == daysDiff);
//  daysDiff = bbldc::Actual360::daysDiff(dA, dC);
//  assert( 90 == daysDiff);
//  daysDiff = bbldc::Actual360::daysDiff(dA, dD);
//  assert(366 == daysDiff);
//  daysDiff = bbldc::Actual360::daysDiff(dB, dC);
//  assert( 61 == daysDiff);
//..
// To compute the year fraction between these two dates:
//..
//  double yearsDiff;
//  yearsDiff = bbldc::Actual360::yearsDiff(dA, dC);
//  assert(0.25 == yearsDiff);
//  yearsDiff = bbldc::Actual360::yearsDiff(dA, dD);
//  assert(yearsDiff < 1.0167 && yearsDiff > 1.0166);
//..

#ifndef INCLUDED_BBLSCM_VERSION
#include <bblscm_version.h>
#endif

#ifndef INCLUDED_BDLT_DATE
#include <bdlt_date.h>
#endif

namespace BloombergLP {
namespace bbldcu {

                           // ================
                           // struct Actual360
                           // ================

struct Actual360 {
    // This 'struct' provides a namespace for pure procedures determining
    // values based on dates according to the Actual/360 day-count convention.

    // CLASS METHODS
    static int daysDiff(const bdlt::Date& beginDate,
                        const bdlt::Date& endDate);
        // Return the number of days between the specified 'beginDate' and
        // 'endDate' according to the Actual/360 day-count convention.  If
        // 'beginDate <= endDate' then the result is non-negative.  Note that
        // reversing the order of 'beginDate' and 'endDate' negates the result.

    static double yearsDiff(const bdlt::Date& beginDate,
                            const bdlt::Date& endDate);
        // Return the number of years between the specified 'beginDate' and
        // 'endDate' according to the Actual/360 day-count convention.  If
        // 'beginDate <= endDate' then the result is non-negative.  Note that
        // reversing the order of 'beginDate' and 'endDate' negates the result.
};

// ============================================================================
//                            INLINE DEFINITIONS
// ============================================================================

                           // ----------------
                           // struct Actual360
                           // ----------------

// CLASS METHODS
inline
int Actual360::daysDiff(const bdlt::Date& beginDate, const bdlt::Date& endDate)
{
    return endDate - beginDate;
}

}  // close package namespace
}  // close enterprise namespace

#endif

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
