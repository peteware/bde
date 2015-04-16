// bbldc_daycount.cpp                                                 -*-C++-*-
#include <bbldc_daycount.h>

#include <bsls_ident.h>
BSLS_IDENT_RCSID(bbldc_daycount_cpp,"$Id$ $CSID$")

#include <bbldcu_actual360.h>
#include <bbldcu_actual365fixed.h>
#include <bbldcu_isdaactualactual.h>
#include <bbldcu_isma30360.h>
#include <bbldcu_psa30360eom.h>
#include <bbldcu_sia30360eom.h>
#include <bbldcu_sia30360neom.h>

#include <bsls_assert.h>

namespace BloombergLP {
namespace bbldc {

                           // ---------------
                           // struct DayCount
                           // ---------------

// CLASS METHODS
int DayCount::daysDiff(const bdlt::Date&        beginDate,
                       const bdlt::Date&        endDate,
                       DayCountConvention::Enum convention)
{
    int numDays;

    switch (convention) {
      case DayCountConvention::e_ACTUAL_360: {
        numDays = bbldcu::Actual360::daysDiff(beginDate, endDate);
      } break;
      case DayCountConvention::e_ACTUAL_365_FIXED: {
        numDays = bbldcu::Actual365Fixed::daysDiff(beginDate, endDate);
      } break;
      case DayCountConvention::e_ISDA_ACTUAL_ACTUAL: {
        numDays = bbldcu::IsdaActualActual::daysDiff(beginDate, endDate);
      } break;
      case DayCountConvention::e_ISMA_30_360: {
        numDays = bbldcu::Isma30360::daysDiff(beginDate, endDate);
      } break;
      case DayCountConvention::e_PSA_30_360_EOM: {
        numDays = bbldcu::Psa30360eom::daysDiff(beginDate, endDate);
      } break;
      case DayCountConvention::e_SIA_30_360_EOM: {
        numDays = bbldcu::Sia30360eom::daysDiff(beginDate, endDate);
      } break;
      case DayCountConvention::e_SIA_30_360_NEOM: {
        numDays = bbldcu::Sia30360neom::daysDiff(beginDate, endDate);
      } break;
      default: {
        BSLS_ASSERT(0 && "Unrecognized convention");
        numDays = 0;
      } break;
    }

    return numDays;
}

double DayCount::yearsDiff(const bdlt::Date&        beginDate,
                           const bdlt::Date&        endDate,
                           DayCountConvention::Enum convention)
{
    BSLS_ASSERT(1752 != beginDate.year());
    BSLS_ASSERT(1752 != endDate.year());

    double numYears;

    switch (convention) {
      case DayCountConvention::e_ACTUAL_360: {
        numYears = bbldcu::Actual360::yearsDiff(beginDate, endDate);
      } break;
      case DayCountConvention::e_ACTUAL_365_FIXED: {
        numYears = bbldcu::Actual365Fixed::yearsDiff(beginDate, endDate);
      } break;
      case DayCountConvention::e_ISDA_ACTUAL_ACTUAL: {
        numYears = bbldcu::IsdaActualActual::yearsDiff(beginDate, endDate);
      } break;
      case DayCountConvention::e_ISMA_30_360: {
        numYears = bbldcu::Isma30360::yearsDiff(beginDate, endDate);
      } break;
      case DayCountConvention::e_PSA_30_360_EOM: {
        numYears = bbldcu::Psa30360eom::yearsDiff(beginDate, endDate);
      } break;
      case DayCountConvention::e_SIA_30_360_EOM: {
        numYears = bbldcu::Sia30360eom::yearsDiff(beginDate, endDate);
      } break;
      case DayCountConvention::e_SIA_30_360_NEOM: {
        numYears = bbldcu::Sia30360neom::yearsDiff(beginDate, endDate);
      } break;
      default: {
        BSLS_ASSERT(0 && "Unrecognized convention");
        numYears = 0.0;
      } break;
    }

    return numYears;
}

}  // close package namespace
}  // close enterprise namespace

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
