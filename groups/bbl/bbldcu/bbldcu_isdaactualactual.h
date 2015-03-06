// bbldcu_isdaactualactual.h                                          -*-C++-*-
#ifndef INCLUDED_BBLDCU_ISDAACTUALACTUAL
#define INCLUDED_BBLDCU_ISDAACTUALACTUAL

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide support for the ISDA Actual/Actual day-count convention.
//
//@CLASSES:
//  bblscu::IsdaActualActual: procedures relating to a day-count convention
//
//@AUTHOR: John Lakos (jlakos)
//
//@DESCRIPTION: This component provides a namespace for pure procedures that
// manipulate dates as prescribed by the International Swaps and Derivatives
// Association (ISDA) Actual/Actual day-count convention.  In the ISDA
// Actual/Actual convention the day count between two dates is exactly the
// number of days separating the dates as per a conventional calendar.
//
///Usage
///-----
// The following snippets of code illustrate how to use
// 'bblscu::IsdaActualActual' methods.  First, create two 'bdlt::Dates' 'd1'
// and 'd2':
//..
//  const bdlt::Date d1(2003, 10, 19);
//  const bdlt::Date d2(2003, 12, 31);
//..
// To compute the day-count between these two dates:
//..
//  const int daysDiff = bblscu::IsdaActualActual::daysDiff(d1, d2);
//  assert(73 == daysDiff);
//..
// To compute the year fraction between these two dates:
//..
//  const double yearsDiff = bblscu::IsdaActualActual::yearsDiff(d1, d2);
//  // Need fuzzy comparison since 'yearsDiff' is a double.  Expect
//  // '0.2 == yearsDiff'.
//  assert(0.1999 < yearsDiff);
//  assert(0.2001 > yearsDiff);
//..

#ifndef INCLUDED_BBLSCM_VERSION
#include <bblscm_version.h>
#endif

#ifndef INCLUDED_BDLT_DATE
#include <bdlt_date.h>
#endif

namespace BloombergLP {
namespace bbldcu {

                          // =======================
                          // struct IsdaActualActual
                          // =======================

struct IsdaActualActual {
    // This 'struct' provides a namespace for pure procedures determining
    // values based on dates according to the ISDA Actual/Actual day-count
    // convention.

    // CLASS METHODS
    static int daysDiff(const bdlt::Date& beginDate,
                        const bdlt::Date& endDate);
        // Return the number of days between the specified 'beginDate' and
        // 'endDate according to the ISDA Actual/Actual day-count convention.
        // If 'beginDate <= endDate', then the result is non-negative.  Note
        // that reversing the order of 'beginDate' and 'endDate' negates the
        // result.

    static double yearsDiff(const bdlt::Date& beginDate,
                            const bdlt::Date& endDate);
        // Return the number of years between the specified 'beginDate' and
        // 'endDate according to the ISDA Actual/Actual day-count convention.
        // If 'beginDate <= endDate', then the result is non-negative.  The
        // behavior is undefined if either 'beginDate' or 'endDate' is in the
        // year 1752.  Note that reversing the order of 'beginDate' and
        // 'endDate' negates the result.
};

// ============================================================================
//                            INLINE DEFINITIONS
// ============================================================================

                          // -----------------------
                          // struct IsdaActualActual
                          // -----------------------

// CLASS METHODS
inline
int IsdaActualActual::daysDiff(const bdlt::Date& beginDate,
                               const bdlt::Date& endDate)
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
