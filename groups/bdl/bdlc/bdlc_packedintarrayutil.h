// bdlc_packedintarrayutil.h                                          -*-C++-*-
#ifndef INCLUDED_BDLC_PACKEDINTARRAYUTIL
#define INCLUDED_BDLC_PACKEDINTARRAYUTIL

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide common non-primitive operations on date objects.
//
//@CLASSES:
//  bdlt::DateUtil: namespace for non-primitive operations on date objects
//
//@SEE_ALSO: bdlc_packedintarray
//
//@DESCRIPTION: This component provides a 'struct', 'bdlt::DateUtil', that
// serves as a namespace for utility functions that operate on 'bdlt::Date'
// objects.
//
// The following list of methods are provided by 'bdlt::DateUtil':
//..
//  'isValidYYYYMMDD'             o Validate or convert to and from the
//  'convertFromYYYYMMDDRaw'        "YYYYMMDD" format
//  'convertFromYYYYMMDD'           (see {"YYYYMMDD" Format}).
//  'convertToYYYYMMDD'
//
//  'nextDayOfWeek'               o Move a date to the next or the previous
//  'nextDayOfWeekInclusive'        specified day of week.
//  'previousDayOfWeek'
//  'previousDayOfWeekInclusive'
//
//  'nthDayOfWeekInMonth'         o Find a specified day of the week in a
//  'lastDayOfWeekInMonth'          specified year and month.
//
//  'addMonthsEom'                o Add a specified number of months to a date
//  'addMonthsNoEom'                using either the end-of-month or the
//  'addMonths'                     non-end-of-month convention (see
//                                  {End-of-Month Adjustment Conventions}).
//
//  'addYearsEom'                 o Add a specified number of years to a date
//  'addYearsNoEom'                 using either the end-of-month or the
//  'addYears'                      non-end-of-month convention (see
//                                  {End-of-Month Adjustment Conventions}).
//..
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Schedule Generation
/// - - - - - - - - - - - - - - -
// Suppose that given a starting date in the 'YYYYMMDD' format, we want to
// generate a schedule for an event that occurs on the same day of the month
// for 12 months.
//
// First, we use the 'bdlt::DateUtil::convertFromYYYYMMDD' function to convert
// the integer into a 'bdlt::Date':
//..
//  const int startingDateYYYYMMDD = 20130430;
//
//  bdlt::Date date;
//  int rc = bdlt::DateUtil::convertFromYYYYMMDD(&date, startingDateYYYYMMDD);
//  assert(0 == rc);
//..
// Now, we use the 'addMonthsEom' function to generate the schedule.  Note
// that 'addMonthsEom' adjusts the resulting date to be the last day of the
// month if the original date is the last day of the month, while
// 'addMonthsNoEom' does not make this adjustment.
//..
//  bsl::vector<bdlt::Date> schedule;
//  schedule.push_back(date);
//
//  for (int i = 1; i < 12; ++i) {
//      schedule.push_back(bdlt::DateUtil::addMonthsEom(date, i));
//  }
//..
// Finally, we print the generated schedule to the console and observe the
// output:
//..
//  bsl::copy(schedule.begin(),
//            schedule.end(),
//            bsl::ostream_iterator<bdlt::Date>(bsl::cout, "\n"));
//
//  // Expected output on the console:
//  //
//  //   30APR2013
//  //   31MAY2013
//  //   30JUN2013
//  //   31JUL2013
//  //   31AUG2013
//  //   30SEP2013
//  //   31OCT2013
//  //   30NOV2013
//  //   31DEC2013
//  //   31JAN2014
//  //   28FEB2014
//  //   31MAR2014
//..
// Notice that the dates have been adjusted to the end of the month.  If we had
// used 'addMonthsNoEom' instead of 'addMonthsEom', this adjustment would not
// have occurred.

#ifndef INCLUDED_BDLSCM_VERSION
#include <bdlscm_version.h>
#endif

#ifndef INCLUDED_BDLC_PACKEDINTARRAY
#include <bdlc_packedintarray.h>
#endif

namespace BloombergLP {
namespace bdlc {

                      // =========================
                      // struct PackedIntArrayUtil
                      // =========================

struct PackedIntArrayUtil {
    // This 'struct' provides a namespace for utility functions that provide
    // non-primitive operations on dates.

  public:
    // CLASS METHODS
    template <class T>
    PackedIntArrayConstIterator<T> lower_bound(
                                          PackedIntArrayConstIterator<T> first,
                                          PackedIntArrayConstIterator<T> last,
                                          const T& value);

    template <class T>
    PackedIntArrayConstIterator<T> upper_bound(
                                          PackedIntArrayConstIterator<T> first,
                                          PackedIntArrayConstIterator<T> last,
                                          const T& value);
};

// ============================================================================
//                            INLINE DEFINITIONS
// ============================================================================

                      // -------------------------
                      // struct PackedIntArrayUtil
                      // -------------------------

// CLASS METHODS
template <class T>
PackedIntArrayConstIterator<T> PackedIntArrayUtil::lower_bound(
                                          PackedIntArrayConstIterator<T> first,
                                          PackedIntArrayConstIterator<T> last,
                                          const T& value)
{
    BSLS_ASSERT(first <= last);

    typedef typename PackedIntArrayConstIterator<T>::difference_type
                                                               difference_type;

    difference_type count = last - first;

    while (count > 0) {
        difference_type                step = count / 2;
        PackedIntArrayConstIterator<T> it   = first + step;
        if (*it < value) {
            first = ++it;
            count -= step + 1;
        }
        else count = step;
    }
    return first;
}

template <class T>
PackedIntArrayConstIterator<T> PackedIntArrayUtil::upper_bound(
                                          PackedIntArrayConstIterator<T> first,
                                          PackedIntArrayConstIterator<T> last,
                                          const T& value)
{
    BSLS_ASSERT(first <= last);

    typedef typename PackedIntArrayConstIterator<T>::difference_type
                                                               difference_type;

    difference_type count = last - first;

    while (count > 0) {
        difference_type                step = count / 2;
        PackedIntArrayConstIterator<T> it   = first + step;
        if (*it <= value) {
            first = ++it;
            count -= step + 1;
        }
        else count = step;
    }
    return first;
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
