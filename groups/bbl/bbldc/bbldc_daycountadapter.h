// bbldc_daycountadapter.h                                            -*-C++-*-
#ifndef INCLUDED_BBLDC_DAYCOUNTADAPTER
#define INCLUDED_BBLDC_DAYCOUNTADAPTER

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a parameterized day-count convention implementation.
//
//@CLASSES:
//  bbldc::DayCountAdapter: 'bbldc::DayCountInterface' realization
//
//@DESCRIPTION: This component provides a parameterized (template)
// implementation, 'bbldc::DayCountAdapter',  of the 'bbldc::DayCountInterface'
// protocol.  The template argument can be any type supporting the following
// two class methods.
//..
//  int daysDiff(const bdet_Date& beginDate,
//               const bdet_Date& endDate) const;
//
//  double yearsDiff(const bdet_Date& beginDate,
//                   const bdet_Date& endDate) const;
//..
// The template class 'bbldc::DayCountAdapter' provides convenient support for
// run-time polymorphic choice of day-count conventions (via conventional use
// of a base-class pointer or reference) without having to implement each
// derived type explicitly.  In this sense, 'bbldc::DayCountAdapter' adapts the
// various concrete day-count convention classes (e.g., 'bbldcu::Isma30360') to
// a run-time binding mechanism.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Adapting 'bbldcu::Isma30360'
///- - - - - - - - - - - - - - - - - - - -
// This example shows the procedure for using 'bbldc::DayCountAdapter' to adapt
// the 'bbldc::Isma30360' day-count convention to the
// 'bbldc::DayCountInterface' and then the use of the day-count methods.
// First, we define an instance of the adapted day-count convention and obtain
// a reference to the 'bbldc::DayCountInterface':
//..
//  bbldc::DayCountAdapter<bbldcu::Isma30360> myDcc;
//  bbldc::DayCountInterface&                 dcc = myDcc;
//..
// Then, create two 'bdlt::Date' 'd1' and 'd2' with which  to use the day-count
// convention methods.
//..
//  const bdlt::Date d1(2003, 10, 18);
//  const bdlt::Date d2(2003, 12, 31);
//..
// Now, use the base-class reference to compute the day-count between these two
// dates:
//..
//  const int daysDiff = dcc.daysDiff(d1, d2);
//  assert(72 == daysDiff);
//..
// Finally, use the base-class reference to compute the year fraction between
// these two dates:
//..
//  const double yearsDiff = dcc.yearsDiff(d1, d2);
//  // Need fuzzy comparison since 'yearsDiff' is a double.  Expect
//  // '0.2 == yearsDiff'.
//  assert(0.1999 < yearsDiff);
//  assert(0.2001 > yearsDiff);
//..

#ifndef INCLUDED_BBLSCM_VERSION
#include <bblscm_version.h>
#endif

#ifndef INCLUDED_BBLDC_DAYCOUNTINTERFACE
#include <bbldc_daycountinterface.h>
#endif

#ifndef INCLUDED_BDLT_DATE
#include <bdlt_date.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

namespace BloombergLP {
namespace bbldc {
                          // =====================
                          // class DayCountAdapter
                          // =====================

template <class CONVENTION>
class DayCountAdapter : public DayCountInterface {
    // This 'class' provides an "adapter" from the specified 'CONVENTION' to
    // the 'bbldc::DayCountInterface' that can be used for determining values
    // based on dates according to the 'CONVENTION' day-count convention.

  public:
    // ACCESSORS
    int daysDiff(const bdlt::Date& beginDate, const bdlt::Date& endDate) const;
        // Return the number of days between the specified 'beginDate' and
        // 'endDate' as per the 'CONVENTION' template policy.  If
        // 'beginDate <= endDate', then the result is non-negative.  Note that
        // reversing the order of 'beginDate' and 'endDate' negates the result.

    double yearsDiff(const bdlt::Date& beginDate,
                     const bdlt::Date& endDate) const;
        // Return the number of years between the specified 'beginDate' and
        // 'endDate' as per the 'CONVENTION' template policy.  If
        // 'beginDate <= endDate', then the result is non-negative.  The
        // behavior is undefined if either 'beginDate' or 'endDate' is in the
        // year 1752.  Note that reversing the order of 'beginDate' and
        // 'endDate' negates the result; specifically
        // '|yearsDiff(b, e) + yearsDiff(e, b)| <= 1.0e-15'.
};

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

                          // ---------------------
                          // class DayCountAdapter
                          // ---------------------

// ACCESSORS
template <class CONVENTION>
inline
int bbldc::DayCountAdapter<CONVENTION>::daysDiff(
                                               const bdlt::Date& beginDate,
                                               const bdlt::Date& endDate) const
{
    return CONVENTION::daysDiff(beginDate, endDate);
}

template <class CONVENTION>
inline
double
bbldc::DayCountAdapter<CONVENTION>:: yearsDiff(const bdlt::Date& beginDate,
                                               const bdlt::Date& endDate) const
{
    BSLS_ASSERT(1752 != beginDate.year());
    BSLS_ASSERT(1752 != endDate.year());

    return CONVENTION::yearsDiff(beginDate, endDate);
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
