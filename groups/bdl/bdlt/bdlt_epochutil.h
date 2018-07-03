// bdlt_epochutil.h                                                   -*-C++-*-
#ifndef INCLUDED_BDLT_EPOCHUTIL
#define INCLUDED_BDLT_EPOCHUTIL

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Conversion between absolute/relative time with respect to epoch.
//
//@CLASSES:
//  bdlt::EpochUtil: non-primitive functions on epoch-related conversions
//
//@SEE_ALSO: bdlt_datetime, bdlt_datetimeinterval, bsls_timeinterval
//
//@DESCRIPTION: This component defines a namespace, 'bdlt::EpochUtil',
// providing non-primitive conversions between two different concepts of time.
// Clients can convert between absolute time ('bdlt::Datetime') and relative
// time (such as 'bsl::time_t', 'bdlt::EpochUtil::TimeT64',
// 'bsls::TimeInterval', and 'bdlt::DatetimeInterval') with respect to the Unix
// standard "epoch" (1970/01/01_00:00:00.000 UTC), henceforth, simply referred
// to as "the epoch".  Also provided is a fast, thread-safe method, 'epoch',
// for access to a pre-instantiated 'bdlt::Datetime' object whose (constant)
// value is that of the epoch.
//
// Due to different resolutions, conversions between absolute/relative time are
// possibly lossy when converting from a type with higher resolution to one
// with lower resolution.  The value of the type with higher resolution will be
// truncated (not rounded).  The following table lists the resolution of the
// types involved in this component:
//..
//  Type                      Reference          Resolution
//  ---------------------     --------------     ------------
//  bsl::time_t               relative           seconds
//  bdlt::EpochUtil::TimeT64  relative           seconds
//  bdlt::Datetime            absolute (UTC)     microseconds
//  bdlt::DatetimeInterval    relative           microseconds
//  bsls::TimeInterval        relative           nanoseconds
//..
// Note that the interfaces using 'bdlt::EpochUtil::TimeT64' can be validly
// used for values before the epoch (corresponding to negative 'TimeT64'
// values), whereas the interfaces using 'bsl::time_t' have undefined behavior
// for such input.  Furthermore, even on platforms where 'bsl::time_t' is a
// 64-bit value, clients of this component are strongly encouraged to use the
// 'TimeT64'-based methods to avoid limitations imposed by the
// 'bsl::time_t'-based methods required to ensure identical behavior on all
// supported platforms (e.g., see the function-level documentation for
// 'convertToTimeT(bsl::time_t)').
//
// Also note that these conversions do not take into account the leap seconds
// (25 as of this writing) added to UTC by the International Earth Rotation and
// Reference Systems Service, but simply regard each day as having a fixed
// number of seconds (24 hours * 60 minutes per hour * 60 seconds per minute).
//
///Thread Safety
///-------------
// It is safe to invoke any function defined in this component in two or more
// separate threads simultaneously, provided no other thread is simultaneously
// modifying the argument passed by reference to a non-modifiable user-defined
// type (such as 'bsls::TimeInterval' or 'bdlt::Datetime').
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Converting Between Various Representations of Time
///- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// When processing date/time data, we are often required to deal with a variety
// of ways in which to represent that data, and therefore we need to be able to
// convert between those representations.  We can use the methods contained in
// 'bdlt::EpochUtil' to do this.
//
// First, we set up date/time input values in a variety of formats.  We'll use
// 900ms past midnight of January 1, 2000 as the base date and time, dropping
// the 900ms if the resolution of a format doesn't support it:
//..
//  const bsl::time_t            inputTime         (946684800);
//  const bsls::TimeInterval     inputTimeInterval (946684800, 900000000);
//  const bdlt::DatetimeInterval inputDatetimeInterval(
//                                                0, 0, 0, 0, 946684800900LL);
//  const bdlt::Datetime         inputDatetime     (2000, 1, 1, 0, 0, 0, 900);
//..
// Then, we set up a set of output variables to receive converted values:
//..
//  bsl::time_t            outputTime;
//  bsls::TimeInterval     outputTimeInterval;
//  bdlt::DatetimeInterval outputDatetimeInterval;
//  bdlt::Datetime         outputDatetime;
//..
// Next, because 'bdlt::EpochUtil' uses 'bdlt::Datetime' as the common format
// for conversion, we will set up a pair of variables in this format to
// represent the values we expect to see:
//..
//  const bdlt::Datetime epochDatetimeWithMs   (2000, 1, 1, 0, 0, 0, 900);
//  const bdlt::Datetime epochDatetimeWithoutMs(2000, 1, 1, 0, 0, 0, 0);
//..
// Now, we perform a set of conversions to 'bdlt::Datetime' and verify that the
// results are correct.  We will use the conversion methods that return by
// value:
//..
//  outputDatetime = bdlt::EpochUtil::convertFromTimeT(inputTime);
//  assert(epochDatetimeWithoutMs == outputDatetime);
//
//  outputDatetime =
//                 bdlt::EpochUtil::convertFromTimeInterval(inputTimeInterval);
//  assert(epochDatetimeWithMs    == outputDatetime);
//
//  outputDatetime =
//         bdlt::EpochUtil::convertFromDatetimeInterval(inputDatetimeInterval);
//  assert(epochDatetimeWithMs    == outputDatetime);
//..
// Finally, we perform a set of conversions from 'bdlt::Datetime' and verify
// that the results are correct.  This time, for variety, we will illustrate
// the conversion methods which return through an object pointer:
//..
//  assert(0 == bdlt::EpochUtil::convertToTimeT(&outputTime, inputDatetime));
//  assert(inputTime             == outputTime);
//
//  assert(0 == bdlt::EpochUtil::convertToTimeInterval(&outputTimeInterval,
//                                                     inputDatetime));
//  assert(inputTimeInterval     == outputTimeInterval);
//
//  assert(0 == bdlt::EpochUtil::convertToDatetimeInterval(
//                                                     &outputDatetimeInterval,
//                                                     inputDatetime));
//  assert(inputDatetimeInterval == outputDatetimeInterval);
//..

#ifndef INCLUDED_BDLSCM_VERSION
#include <bdlscm_version.h>
#endif

#ifndef INCLUDED_BDLT_DATETIME
#include <bdlt_datetime.h>
#endif

#ifndef INCLUDED_BDLT_DATETIMEIMPUTIL
#include <bdlt_datetimeimputil.h>
#endif

#ifndef INCLUDED_BDLT_DATETIMEINTERVAL
#include <bdlt_datetimeinterval.h>
#endif

#ifndef INCLUDED_BDLT_TIME
#include <bdlt_time.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

#ifndef INCLUDED_BSLS_TIMEINTERVAL
#include <bsls_timeinterval.h>
#endif

#ifndef INCLUDED_BSLS_TYPES
#include <bsls_types.h>        // 'Int64', 'Uint64'
#endif

#ifndef INCLUDED_BSL_CTIME
#include <bsl_ctime.h>         // 'bsl::time_t'
#endif

namespace BloombergLP {
namespace bdlt {

                             // ================
                             // struct EpochUtil
                             // ================

struct EpochUtil {
    // This 'struct' provides a namespace for a suite of non-primitive
    // functions providing conversions between absolute 'Datetime' values and
    // corresponding relative time intervals with respect to the Unix standard
    // epoch time, returned by the 'epoch' method.  These methods are
    // alias-safe, thread-safe, and exception-neutral.  Functions are provided
    // for returning converted values by value or through a result pointer.

  public:
    // TYPES
    typedef bsls::Types::Int64 TimeT64;
        // 'TimeT64' is an alias for a 64-bit integral type representing
        // seconds from the epoch in UTC.  Note that, in contrast with
        // 'bsl::time_t', this type can be used in conversions to and from
        // 'Datetime' values that are less than the epoch (corresponding to
        // negative 'TimeT64' values).

  private:
    // CLASS DATA
    static const TimeT64 s_earliestAsTimeT64;  // January   1, 0001 00:00:00
    static const TimeT64 s_latestAsTimeT64;    // December 31, 9999 23:59:59

  public:
    // CLASS METHODS
    static const Datetime& epoch();
        // Return a reference providing non-modifiable access to the epoch
        // time: midnight on January 1, 1970.  Note that this value exists
        // before any code is executed and will continue to exist, unchanged,
        // until the program exits.

                           // 'time_t'-Based Methods

    static Datetime convertFromTimeT(bsl::time_t time);
        // Return, as a 'Datetime', the absolute datetime computed as the sum
        // of the specified relative 'time' and the epoch.  The behavior is
        // undefined unless '0 <= time' and, for the resultant 'Datetime' 'dt',
        // '0 == convertToTimeT(&time, dt)' (i.e., 'time' is representable as a
        // 32-bit 'int').  Note that the returned value will use Coordinated
        // Universal Time (UTC) as a reference.

    static void convertFromTimeT(Datetime *result, bsl::time_t time);
        // Load into the specified 'result' the absolute datetime converted to
        // a 'Datetime', computed as the sum of the specified relative 'time'
        // and the epoch.  The behavior is undefined unless '0 <= time' and
        // '0 == convertToTimeT(&time, *result)' (i.e., 'time' is representable
        // as a 32-bit 'int').  Note that 'result' will use Coordinated
        // Universal Time (UTC) as a reference.

    static bsl::time_t convertToTimeT(const Datetime& datetime);
        // Return the relative time computed as the difference between the
        // specified absolute 'datetime' and the epoch.  The behavior is
        // undefined unless 'datetime - epoch() >= DatetimeInterval()' and the
        // converted 'datetime' can be represented in the destination format on
        // all supported platforms (i.e., the resultant value is representable
        // as a 32-bit 'int').  Note that 'datetime' is assumed to use
        // Coordinated Universal Time (UTC) as a reference.  Also note that if
        // error detection is desired, the overloaded version that loads the
        // converted 'datetime' into a supplied destination object should be
        // used.

    static int convertToTimeT(bsl::time_t     *result,
                              const Datetime&  datetime);
        // Load into the specified 'result' the relative time computed as the
        // difference between the specified absolute 'datetime' and the epoch.
        // Return 0 on success, and a non-zero value (with no effect on
        // 'result') if 'datetime - epoch() < DatetimeInterval()' or 'datetime'
        // cannot be represented in the destination format on all supported
        // platforms (i.e., the computed '*result' is not representable as a
        // 32-bit 'int').  Note that 'datetime' is assumed to use Coordinated
        // Universal Time (UTC) as a reference.

                           // 'TimeT64'-Based Methods

    static Datetime convertFromTimeT64(TimeT64 time);
        // Return, as a 'Datetime', the absolute datetime computed as the sum
        // of the specified relative 'time' and the epoch.  The behavior is
        // undefined unless the converted 'time' can be represented in the
        // destination format.  Note that the returned value will use
        // Coordinated Universal Time (UTC) as a reference.  Also note that if
        // error detection is desired, the overloaded version that loads the
        // converted 'time' into a supplied destination object should be used.

    static int convertFromTimeT64(Datetime *result, TimeT64 time);
        // Load into the specified 'result' the absolute datetime converted to
        // a 'Datetime', computed as the sum of the specified relative 'time'
        // and the epoch.  Return 0 on success, and a non-zero value (with no
        // effect on 'result') if 'time' cannot be represented in the
        // destination format.  Note that 'result' will use Coordinated
        // Universal Time (UTC) as a reference.

    static TimeT64 convertToTimeT64(const Datetime& datetime);
        // Return the relative time computed as the difference between the
        // specified absolute 'datetime' and the epoch.  Note that 'datetime'
        // is assumed to use Coordinated Universal Time (UTC) as a reference.

    static void convertToTimeT64(TimeT64         *result,
                                 const Datetime&  datetime);
        // Load into the specified 'result' the relative time computed as the
        // difference between the specified absolute 'datetime' and the epoch.
        // Note that 'datetime' is assumed to use Coordinated Universal Time
        // (UTC) as a reference.

                       // 'bsls::TimeInterval'-Based Methods

    static Datetime convertFromTimeInterval(
                                       const bsls::TimeInterval& timeInterval);
        // Return, as a 'Datetime', the absolute datetime computed as the sum
        // of the specified relative 'timeInterval' and the epoch.  The
        // behavior is undefined unless the conversion result can be
        // represented as a 'Datetime'.  Note that the conversion is
        // potentially lossy as the resolution of 'bsls::TimeInterval' is
        // greater than that of 'Datetime'.

    static void convertFromTimeInterval(
                                      Datetime                  *result,
                                      const bsls::TimeInterval&  timeInterval);
        // Load into the specified 'result' the absolute datetime converted to
        // a 'Datetime', computed as the sum of the specified relative
        // 'timeInterval' and the epoch.  The behavior is undefined unless the
        // conversion result can be represented as a 'Datetime'.  Note that the
        // conversion is potentially lossy as the resolution of
        // 'bsls::TimeInterval' is greater than that of 'Datetime'.

    static bsls::TimeInterval convertToTimeInterval(const Datetime& datetime);
        // Return, as a 'bsls::TimeInterval', the relative time computed as the
        // difference between the specified absolute 'datetime' and the epoch.

                   // 'DatetimeInterval'-Based Methods

    static Datetime convertFromDatetimeInterval(
                                     const DatetimeInterval& datetimeInterval);
        // Return, as a 'Datetime', the absolute datetime computed as the sum
        // of the specified relative 'datetimeInterval' and the epoch.  The
        // behavior is undefined unless the conversion result can be
        // represented as a 'Datetime'.

    static void convertFromDatetimeInterval(
                                    Datetime                *result,
                                    const DatetimeInterval&  datetimeInterval);
        // Load into the specified 'result' the absolute datetime converted to
        // a 'Datetime', computed as the sum of the specified relative
        // 'datetimeInterval' and the epoch.  The behavior is undefined unless
        // the conversion result can be represented as a 'Datetime'.

    static DatetimeInterval convertToDatetimeInterval(
                                                     const Datetime& datetime);
        // Return, as a 'DatetimeInterval', the relative time computed as the
        // difference between the specified absolute 'datetime' and the epoch.

    // DEPRECATED CLASS METHODS
    static int convertToTimeInterval(bsls::TimeInterval *result,
                                     const Datetime&     datetime);
        // !DEPRECATED!: Use 'convertToTimeInterval(datetime)' instead.
        //
        // If 'datetime - epoch() >= DatetimeInterval()', load into the
        // specified 'result' the relative time converted to a
        // 'bsls::TimeInterval', computed as the difference between the
        // specified absolute 'datetime' and the epoch, and return 0.
        // Otherwise, return a non-zero value (with no effect on 'result').

    static int convertToDatetimeInterval(DatetimeInterval *result,
                                         const Datetime&   datetime);
        // !DEPRECATED!: Use 'convertToDatetimeInterval(datetime)' instead.
        //
        // If 'datetime - epoch() >= DatetimeInterval()', load into the
        // specified 'result' the relative time converted to a
        // 'DatetimeInterval', computed as the difference between the specified
        // absolute 'datetime' and the epoch, and return 0.  Otherwise, return
        // a non-zero value (with no effect on 'result').
};

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

                             // ----------------
                             // struct EpochUtil
                             // ----------------

// CLASS METHODS
inline
const Datetime& EpochUtil::epoch()
{
    return *DatetimeImpUtil::epoch_1970_01_01();
}

                           // 'time_t'-Based Methods

inline
Datetime EpochUtil::convertFromTimeT(bsl::time_t time)
{
    BSLS_ASSERT_SAFE(0 <= time);

    Datetime datetime(epoch());
    datetime.addSeconds(time);

    return datetime;
}

inline
void EpochUtil::convertFromTimeT(Datetime *result, bsl::time_t time)
{
    BSLS_ASSERT_SAFE(result);
    BSLS_ASSERT_SAFE(0 <= time);

    *result = epoch();
    result->addSeconds(time);
}

inline
bsl::time_t EpochUtil::convertToTimeT(const Datetime& datetime)
{
    const DatetimeInterval dti = datetime - epoch();

    BSLS_ASSERT_SAFE(dti                >= DatetimeInterval());
    BSLS_ASSERT_SAFE(dti.totalSeconds() <= 0x7fffffffLL);

    // Note that, with safe-assertions disabled, the representation of
    // 'bsl::time_t' must not affect the resultant 'bsl::time_t' (i.e., in case
    // 'bsl::time_t' is 64-bit).

    return bsl::time_t(static_cast<int>(dti.totalSeconds()));
}

inline
int EpochUtil::convertToTimeT(bsl::time_t *result, const Datetime& datetime)
{
    BSLS_ASSERT_SAFE(result);

    const DatetimeInterval dti = datetime - epoch();

    if (dti < DatetimeInterval()) {
        return 1;                                                     // RETURN
    }

    const bsls::Types::Int64 seconds = dti.totalSeconds();

    if (seconds > 0x7fffffffLL) {
        return 1;                                                     // RETURN
    }

    *result = static_cast<bsl::time_t>(seconds);

    return 0;
}

                           // 'TimeT64'-Based Methods

inline
Datetime EpochUtil::convertFromTimeT64(TimeT64 time)
{
    BSLS_ASSERT_SAFE(s_earliestAsTimeT64 <= time);
    BSLS_ASSERT_SAFE(                       time <= s_latestAsTimeT64);

    Datetime datetime(epoch());
    datetime.addSeconds(time);

    return datetime;
}

inline
int EpochUtil::convertFromTimeT64(Datetime *result, TimeT64 time)
{
    BSLS_ASSERT_SAFE(result);

    if (time < s_earliestAsTimeT64 || time > s_latestAsTimeT64) {
        return 1;                                                     // RETURN
    }

    *result = epoch();
    result->addSeconds(time);

    return 0;
}

inline
EpochUtil::TimeT64
EpochUtil::convertToTimeT64(const Datetime& datetime)
{
    int hour;
    int minute;
    int second;

    datetime.getTime(&hour, &minute, &second);

    Datetime dt(datetime.date());
    dt.setTime(hour, minute, second);

    return TimeT64((dt - epoch()).totalSeconds());
}

inline
void EpochUtil::convertToTimeT64(TimeT64 *result, const Datetime& datetime)
{
    BSLS_ASSERT_SAFE(result);

    *result = EpochUtil::convertToTimeT64(datetime);
}

                       // 'bsls::TimeInterval'-Based Methods

inline
Datetime EpochUtil::convertFromTimeInterval(
                                        const bsls::TimeInterval& timeInterval)
{
    return epoch() + timeInterval;
}

inline
void EpochUtil::convertFromTimeInterval(
                                       Datetime                  *result,
                                       const bsls::TimeInterval&  timeInterval)
{
    BSLS_ASSERT_SAFE(result);

    *result = epoch() + timeInterval;
}

inline
bsls::TimeInterval EpochUtil::convertToTimeInterval(const Datetime& datetime)
{
    const DatetimeInterval dti = datetime - epoch();

    return bsls::TimeInterval(dti.totalSeconds(),
                              dti.milliseconds() * 1000000
                                                  + dti.microseconds() * 1000);
}

                   // 'DatetimeInterval'-Based Methods

inline
Datetime EpochUtil::convertFromDatetimeInterval(
                                      const DatetimeInterval& datetimeInterval)
{
    return epoch() + datetimeInterval;
}

inline
void EpochUtil::convertFromDatetimeInterval(
                                     Datetime                *result,
                                     const DatetimeInterval&  datetimeInterval)
{
    BSLS_ASSERT_SAFE(result);

    *result = epoch() + datetimeInterval;
}

inline
DatetimeInterval EpochUtil::convertToDatetimeInterval(const Datetime& datetime)
{
    return datetime - epoch();
}

// DEPRECATED CLASS METHODS
inline
int EpochUtil::convertToTimeInterval(bsls::TimeInterval *result,
                                     const Datetime&     datetime)
{
    BSLS_ASSERT_SAFE(result);

    const DatetimeInterval dti = datetime - epoch();

    if (dti < DatetimeInterval()) {
        return 1;                                                     // RETURN
    }
    result->setInterval(dti.totalSeconds(),
                        dti.milliseconds() * 1000000
                                                  + dti.microseconds() * 1000);

    return 0;
}

inline
int EpochUtil::convertToDatetimeInterval(DatetimeInterval *result,
                                         const Datetime&   datetime)
{
    BSLS_ASSERT_SAFE(result);

    if (datetime - epoch() < DatetimeInterval()) {
        return 1;                                                     // RETURN
    }
    *result = datetime - epoch();

    return 0;
}

}  // close package namespace
}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2017 Bloomberg Finance L.P.
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
