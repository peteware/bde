// bbldc_daycount.h                                                   -*-C++-*-
#ifndef INCLUDED_BBLDC_DAYCOUNT
#define INCLUDED_BBLDC_DAYCOUNT

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Support for day-count calculations of 'enum'-specified conventions.
//
//@CLASSES:
//  bbldc::DayCount: procedures for 'enum'-specified day-count calculations
//
//@DESCRIPTION: This component provides a 'struct', 'bbldc::DayCount', that
// defines a suite of date-related functions, which can be used to compute the
// day-count and the year-fraction between two dates as prescribed by the
// enumerated day-count convention.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Computing Day-Count and Year-Fraction
///- - - - - - - - - - - - - - - - - - - - - - - -
// The following snippets of code illustrate how to use
// 'bbldcu::IsdaActualActual' methods.  First, create two 'bdlt::Dates' 'd1'
// and 'd2':
//..
//  const bdlt::Date d1(2003, 10, 19);
//  const bdlt::Date d2(2003, 12, 31);
//..
// Then, compute the day-count between these two dates according to the ISDA
// Actual/Actual convention:
//..
//  const int daysDiff = bbldc::DayCount::daysDiff(
//                            d1,
//                            d2,
//                            bbldc::DayCountConvention::e_ISDA_ACTUAL_ACTUAL);
//  assert(73 == daysDiff);
//..
// Finally, compute the year fraction between these two dates according to the
// ISDA Actual/Actual convention:
//..
//  const double yearsDiff = bbldc::DayCount::yearsDiff(
//                            d1,
//                            d2,
//                            bbldc::DayCountConvention::e_ISDA_ACTUAL_ACTUAL);
//  // Need fuzzy comparison since 'yearsDiff' is a double.  Expect
//  // '0.2 == yearsDiff'.
//  assert(0.1999 < yearsDiff);
//  assert(0.2001 > yearsDiff);
//..

#ifndef INCLUDED_BBLSCM_VERSION
#include <bblscm_version.h>
#endif

#ifndef INCLUDED_BBLDC_DAYCOUNTCONVENTION
#include <bbldc_daycountconvention.h>
#endif

#ifndef INCLUDED_BDLT_DATE
#include <bdlt_date.h>
#endif

namespace BloombergLP {
namespace bbldc {

                           // ===============
                           // struct DayCount
                           // ===============

struct DayCount {
    // This 'struct' provides a namespace for a suite of pure functions that
    // compute values based on dates according to enumerated day-count
    // conventions.

    // CLASS METHODS
    static int daysDiff(const bdlt::Date&        beginDate,
                        const bdlt::Date&        endDate,
                        DayCountConvention::Enum convention);
        // Return the number of days between the specified 'beginDate' and
        // 'endDate' according to the specified enumerated day-count convention
        // 'convention'.  The behavior is undefined unless
        // 'isSupported(convention)'.  If 'beginDate <= endDate' then the
        // result is non-negative.  Note that reversing the order of
        // 'beginDate' and 'endDate' negates the result.

    static bool isSupported(DayCountConvention::Enum convention);
        // Return 'true' if the specified 'convention' is valid for use in
        // 'daysDiff' and 'yearsDiff', and 'false' otherwise.

    static double yearsDiff(const bdlt::Date&        beginDate,
                            const bdlt::Date&        endDate,
                            DayCountConvention::Enum convention);
        // Return the number of years between the specified 'beginDate' and
        // 'endDate' according to the specified enumerated day-count convention
        // 'convention'.  If 'beginDate <= endDate' then the result is
        // non-negative.  The behavior is undefined unless
        // 'isSupported(convention)' and neither 'beginDate' nor 'endDate' is
        // in the year 1752.  Note that reversing the order of 'beginDate' and
        // 'endDate' negates the result; specifically
        // '|yearsDiff(b, e) + yearsDiff(e, b)| <= 1.0e-15'.
};

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
