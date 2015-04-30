// bdlt_packedcalendar.cpp                                            -*-C++-*-
#include <bdlt_packedcalendar.h>

#include <bsls_ident.h>
BSLS_IDENT_RCSID(bdlt_packedcalendar_cpp,"$Id$ $CSID$")

#include <bdlt_date.h>

#include <bslim_printer.h>

#include <bslalg_swaputil.h>

#include <bslma_default.h>

#include <bsls_assert.h>

namespace BloombergLP {
namespace bdlt {

// TBD can not use lower_bound since do not have proper iterators

// HELPER FUNCTIONS
inline
static void addDayImp(Date                      *firstDate,
                      Date                      *endDate,
                      bdlc::PackedIntArray<int> *holidayOffsets,
                      const bdlt::Date&          date)
    // Insert the specified 'date' into the range of the calendar object
    // represented by the specified 'firstDate', 'endDate', and
    // 'holidayOffsets'.  If the specified 'date' is outside the range of the
    // calendar, this range will be extended to include it and 'firstDate' or
    // 'endDate' will be appropriately modified.  Otherwise, this function has
    // no effect.
{
    BSLS_ASSERT(firstDate);
    BSLS_ASSERT(endDate);
    BSLS_ASSERT(holidayOffsets);

    if (date < *firstDate) {
        int delta = *firstDate - date;
        for (bsl::size_t i = 0; i < holidayOffsets->length(); ++i) {
            holidayOffsets->replace(i, holidayOffsets->operator[](i) + delta);
        }
        *firstDate = date;
    }
    if (date > *endDate) {
        *endDate = date;
    }
}

static const unsigned char s_partialWeeks[8][7] =
    // The static PartialWeeks contains all the possible (43) contiguous
    // partial weeks
    // [SUN], [MON], ..., [SUN, MON], [TUE, WED], ..., [SUN, MON, TUE], ...
    // Each value in the table represents the days of week that should be
    // considered for a particular partial week. Array values use the same
    // bitwise representation as 'bdlt::DayOfWeekSet' (bits '1 .. 7'
    // reflect '[ SUN, MON, ..., SAT ]'). The row number indicates the
    // starting day of week of that partial week, and the column number
    // indicates the length of that partial week. Note that the 0th row and
    // column are intentionally left empty to enable a more natural indexing
    // operation.
 { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   {0x00, 0x02, 0x06, 0x0E, 0x1E, 0x3E, 0x7E},
   {0x00, 0x04, 0x0C, 0x1C, 0x3C, 0x7C, 0xFC},
   {0x00, 0x08, 0x18, 0x38, 0x78, 0xF8, 0xFA},
   {0x00, 0x10, 0x30, 0x70, 0xF0, 0xF2, 0xF6},
   {0x00, 0x20, 0x60, 0xE0, 0xE2, 0xE6, 0xEE},
   {0x00, 0x40, 0xC0, 0xC2, 0xC6, 0xCE, 0xDE},
   {0x00, 0x80, 0x82, 0x86, 0x8E, 0x9E, 0xBE} };

int numWeekendDaysInRangeImp(const Date&         firstDate,
                             const Date&         lastDate,
                             const DayOfWeekSet& weekendDays)
    // Return the number of days in the range starting from the specified
    // 'firstDate' to 'lastDate' whose day-of-week are in the specified
    // 'weekendDays' set.  Note that this function returns 0 if 'endDate <
    // lastDate'.
{
    const int len = firstDate <= lastDate ? lastDate - firstDate + 1 : 0;

    // This numWeekendDays is lower bound of the value we are interested in
    const int numWeekendDays = (len / 7) * weekendDays.length();

    const int dayOfWeek = static_cast<int>(firstDate.dayOfWeek());

    // Get the (precomputed) partial week at the end of the range to be
    // considered.
    const DayOfWeekSet& myPartialWeek =
      *reinterpret_cast<const DayOfWeekSet*>(
                                        &(s_partialWeeks[dayOfWeek][len % 7]));
    const DayOfWeekSet mySet = myPartialWeek & weekendDays;

   return numWeekendDays + mySet.length();
}

typedef bsl::vector<bsl::pair<Date, DayOfWeekSet> > WTransitions;

void intersectWeekendDaysTransitions(WTransitions        *result,
                                     const WTransitions&  lhs,
                                     const WTransitions&  rhs)
    // Load, into the specified 'result', the intersection of the specified
    // 'lhs' weekend-days transitions and the specified 'rhs' weekend-days
    // transitions.
{
    if (lhs.empty() || rhs.empty()) {
        return;
    }

    WTransitions::const_iterator lh = lhs.begin();
    WTransitions::const_iterator r  = rhs.begin();

    WTransitions::const_iterator lprev = lhs.end();
    WTransitions::const_iterator rprev = rhs.end();

    while (lh != lhs.end() && r != rhs.end()) {

        if (lh->first < r->first) {
            if (rprev == rhs.end()) {

                // The intersection is empty.

                result->push_back(WTransitions::value_type(
                                                   lh->first, DayOfWeekSet()));
            }
            else {
                WTransitions::value_type val = *lh;
                val.second &= rprev->second;
                result->push_back(val);
            }
            lprev = lh;
            ++lh;
        }
        else if (r->first < lh->first) {
            if (lprev == lhs.end()) {

                // The intersection is empty.

                result->push_back(WTransitions::value_type(
                                                    r->first, DayOfWeekSet()));
            }
            else {
                WTransitions::value_type val = *r;
                val.second &= lprev->second;
                result->push_back(val);
            }
            rprev = r;
            ++r;
        }
        else {
            WTransitions::value_type val = *lh;
            val.second &= r->second;
            result->push_back(val);
            lprev = lh;
            rprev = r;
            ++lh;
            ++r;
        }
    }

    while (lh != lhs.end()) {
        WTransitions::value_type val = *lh;
        val.second &= rprev->second;
        result->push_back(val);
        ++lh;
    }

    while (r != rhs.end()) {
        WTransitions::value_type val = *r;
        val.second &= lprev->second;
        result->push_back(val);
        ++r;
    }
}


void unionWeekendDaysTransitions(WTransitions        *result,
                                 const WTransitions&  lhs,
                                 const WTransitions&  rhs)
   // Load, into the specified 'result', the union of the specified 'lhs'
   // weekend-days transitions and the specified 'rhs' weekend-days
   // transitions.
{
    if (lhs.empty()) {
        *result = rhs;
        return;
    }

    if (rhs.empty()) {
        *result = lhs;
        return;
    }

    WTransitions::const_iterator lh = lhs.begin();
    WTransitions::const_iterator r  = rhs.begin();
    WTransitions::const_iterator lprev = lhs.end();
    WTransitions::const_iterator rprev = rhs.end();

    while (lh != lhs.end() && r != rhs.end()) {

        if (lh->first < r->first) {
            if (rprev == rhs.end()) {
                result->push_back(*lh);
            }
            else {
                WTransitions::value_type val = *lh;
                val.second |= rprev->second;
                result->push_back(val);
            }
            lprev = lh;
            ++lh;
        }
        else if (r->first < lh->first) {
            if (lprev == lhs.end()) {
                result->push_back(*r);
            }
            else {
                WTransitions::value_type val = *r;
                val.second |= lprev->second;
                result->push_back(val);
            }
            rprev = r;
            ++r;
        }
        else {
            WTransitions::value_type val = *lh;
            val.second |= r->second;
            result->push_back(val);
            lprev = lh;
            rprev = r;
            ++lh;
            ++r;
        }
    }

    while (lh != lhs.end()) {
        WTransitions::value_type val = *lh;
        val.second |= rprev->second;
        result->push_back(val);
        ++lh;
    }

    while (r != rhs.end()) {
        WTransitions::value_type val = *r;
        val.second |= lprev->second;
        result->push_back(val);
        ++r;
    }
}

                          // --------------------
                          // class PackedCalendar
                          // --------------------

// PRIVATE MANIPULATORS
int PackedCalendar::addHolidayImp(const int offset)
{
    // Insertion must be in linear time if the holidays are added in-order.

    const int len = d_holidayOffsets.length();

    if (0 == len) {
        d_holidayOffsets.reserveCapacity(1);
        d_holidayCodesIndex.reserveCapacity(1);
        d_holidayOffsets.push_back(offset);
        d_holidayCodesIndex.push_back(0);
        return 0;
    }
    if (d_holidayOffsets.back() < offset) {
        d_holidayOffsets.reserveCapacity(len + 1);
        d_holidayCodesIndex.reserveCapacity(len + 1);
        d_holidayOffsets.push_back(offset);
        d_holidayCodesIndex.push_back(d_holidayCodes.length());
        return len;
    }

    OffsetsConstIterator it = bsl::lower_bound(d_holidayOffsets.begin(),
                                               d_holidayOffsets.end(),
                                               offset);

    BSLS_ASSERT(it != d_holidayOffsets.end());

    if (offset != *it) {
        const int shift = it - d_holidayOffsets.begin();
        d_holidayOffsets.reserveCapacity(len + 1);
        d_holidayCodesIndex.reserveCapacity(len + 1);

        // The calls to 'reserveCapacity' invalidated 'it'.

        it = d_holidayOffsets.begin();
        it += shift;
        it = d_holidayOffsets.insert(it, offset);

        CodesIndexConstIterator jt = d_holidayCodesIndex.begin();
        jt += shift;
        d_holidayCodesIndex.insert(jt, *jt);
    }
    return it - d_holidayOffsets.begin();
}

void PackedCalendar::intersectNonBusinessDaysImp(
                                      const PackedCalendar& other,
                                      bool                  fixIfDeltaPositive)

{
    /* TBD
    const int delta = other.d_firstDate - d_firstDate;

    // We'll use these variables only if 'fixIfDeltaPositive' is true.

    const Date& newFirstDate = delta > 0
                               ? other.d_firstDate
                               : d_firstDate;
    const Date& newLastDate  = other.d_lastDate > d_lastDate
                               ? d_lastDate
                               : other.d_lastDate;

    // By reserving this space, we speed up the insertion process since we
    // invalidate fewer iterators and make the process exception-neutral with
    // rollback.

    d_holidayOffsets.reserveCapacity(other.d_holidayOffsets.length() +
                                                    d_holidayOffsets.length());
    d_holidayCodesIndex.reserveCapacity(other.d_holidayCodesIndex.length() +
                                                 d_holidayCodesIndex.length());
    d_holidayCodes.reserveCapacity(other.d_holidayCodes.length() +
                                                      d_holidayCodes.length());


    WeekendDaysTransitionSequence newWeekendDaysTransitions(
                               this->d_weekendDaysTransitions.get_allocator());
    newWeekendDaysTransitions.reserve(
      d_weekendDaysTransitions.size() + other.d_weekendDaysTransitions.size());
    intersectWeekendDaysTransitions(&newWeekendDaysTransitions,
                                    d_weekendDaysTransitions,
                                    other.d_weekendDaysTransitions);

    // DayOfWeekSet oldWeekendDays = d_weekendDays;
    // d_weekendDays &= other.d_weekendDays;

    OffsetsConstReverseIterator r = d_holidayOffsets.rbegin(); // "read"
                                                               // iterator

    OffsetsConstReverseIterator c = other.d_holidayOffsets.rbegin();
                                                               // "compare"
                                                               // iterator

    // "end" iterators of both 'd_holidayOffsets'

    OffsetsConstReverseIterator rend = d_holidayOffsets.rend();
    OffsetsConstReverseIterator cend = other.d_holidayOffsets.rend();

    // We initialize all new elements to 'd_holidayCodes.length()' so the
    // indexes are good for the last holiday.

    d_holidayOffsets.resize(other.d_holidayOffsets.length()
                                                  + d_holidayOffsets.length());
    d_holidayCodesIndex.resize(other.d_holidayCodesIndex.length()
                                                + d_holidayCodesIndex.length(),
                               d_holidayCodes.length());
    d_holidayCodes.resize(other.d_holidayCodes.length()
                                                    + d_holidayCodes.length());

    // We will use the following non-'const' iterators to update the vectors.

    OffsetsReverseIterator    mOffsets    = d_holidayOffsets.rbegin();
    CodesIndexReverseIterator mCodesIndex = d_holidayCodesIndex.rbegin();
    CodesReverseIterator      mCodes      = d_holidayCodes.rbegin();

    int lastCodeIndex = d_holidayCodes.length(); // last valid code index

    while (c != cend || r != rend) {
        OffsetsConstReverseIterator *read = 0;
        const PackedCalendar *readCalendar = 0;

        if (c != cend && r != rend && *r == (*c + delta)) {
            // The holiday is in both calendars.

            *mOffsets = *r;
        }
        else if (c == cend || (r != rend && (*r > (*c + delta)))) {
            Date h = d_firstDate + *r;
            if (h > newLastDate || h < newFirstDate
             || false == other.isWeekendDay(h)) {
                ++r;
                continue;
            }

            // The holiday is a holiday in this calendar and a weekend day
            // in 'other'.

            read = &r;
            readCalendar = this;
            *mOffsets = *r;
        }
        else {
            Date h = other.d_firstDate + *c;
            if (h > newLastDate || h < newFirstDate
             || false == this->isWeekendDay(h)) {
                ++c;
                continue;
            }

            // The holiday is a holiday in 'other' and a weekend day in this
            // calendar.

            read = &c;
            readCalendar = &other;
            *mOffsets = *c + delta;
        }

        // We need to adjust the value of the offset if the 'd_firstDate' is
        // changing.

        if ((fixIfDeltaPositive  && delta > 0)
         || (!fixIfDeltaPositive && delta < 0)) {
            *mOffsets -= delta;
        }

        const CodesReverseIterator mCodesStart = mCodes;  // Keep the current
                                                          // value of 'mCodes'
                                                          // to compute later
                                                          // the no. of codes
                                                          // for this holiday.

        // *** UNION OF THE HOLIDAY CODES

        if (read) {

            // Find the range of holiday codes in this calendar.

            const OffsetsConstIterator base = read->base() - 1;
            CodesConstIterator codesBegin = readCalendar->
                beginHolidayCodes(base);
            CodesConstIterator codes = readCalendar->endHolidayCodes(base);
            while (codes != codesBegin) {
                --codes;
                *mCodes = *codes;
                ++mCodes;
            }
        }
        else {
            const OffsetsConstIterator rbase = r.base() - 1;
            const CodesConstReverseIterator lhs(endHolidayCodes(rbase));
            const CodesConstReverseIterator lhse(beginHolidayCodes(rbase));

            const OffsetsConstIterator cbase = c.base() - 1;
            const CodesConstReverseIterator rhs(other.endHolidayCodes(cbase));
            const CodesConstReverseIterator rhse(
                                              other.beginHolidayCodes(cbase));

            mCodes = bsl::set_union(lhs, lhse, rhs, rhse, mCodes,
                                    bsl::greater<int>());
        }

        lastCodeIndex -= (mCodes - mCodesStart);

        BSLS_ASSERT(lastCodeIndex >= 0
                 && (OffsetsSizeType) lastCodeIndex <= d_holidayCodes.length());

        *mCodesIndex = lastCodeIndex;

        BSLS_ASSERT(mCodesIndex != d_holidayCodesIndex.rend());
        BSLS_ASSERT(mOffsets != d_holidayOffsets.rend());

        ++mCodesIndex;
        ++mOffsets;

        if (read) {
            ++*read;
        }
        else {
            ++r;
            ++c;
        }
    }

    // Remove the unused part of the vectors.

    d_holidayOffsets.erase(d_holidayOffsets.begin(), mOffsets.base());
    d_holidayCodesIndex.erase(d_holidayCodesIndex.begin(), mCodesIndex.base());
    d_holidayCodes.erase(d_holidayCodes.begin(),
                         d_holidayCodes.begin() + lastCodeIndex);

    // Fix the codes index array, all the indexes are off by the number of
    // elements we remove.

    if (d_holidayCodesIndex.begin() != d_holidayCodesIndex.end()) {
        const int adjustment = *d_holidayCodesIndex.begin();
        if (adjustment != 0) {
            bsl::transform(d_holidayCodesIndex.begin(),
                           d_holidayCodesIndex.end(),
                           d_holidayCodesIndex.begin(),
                           bsl::bind2nd(bsl::minus<int>(), adjustment));
        }
    }

    BSLS_ASSERT(d_holidayOffsets.length() == d_holidayCodesIndex.length());
    BSLS_ASSERT(d_holidayOffsets.empty()
                || (OffsetsSizeType)d_holidayCodesIndex.back()
                                                   <= d_holidayCodes.length());
    d_weekendDaysTransitions.swap(newWeekendDaysTransitions);
    */
}

void PackedCalendar::intersectBusinessDaysImp(
                                      const PackedCalendar& other,
                                      bool                  fixIfDeltaPositive)

{
    /* TBD
    const int delta = other.d_firstDate - d_firstDate;

    // We'll use these variables only if 'fixIfDeltaPositive' is true.

    const Date& newFirstDate = delta > 0
                               ? other.d_firstDate
                               : d_firstDate;
    const Date& newLastDate  = other.d_lastDate > d_lastDate
                               ? d_lastDate
                               : other.d_lastDate;

    // By reserving this space, we speed up the insertion process since we
    // invalidate fewer iterators and make the process exception-neutral with
    // rollback.

    d_holidayOffsets.reserveCapacity(other.d_holidayOffsets.length() +
                                                    d_holidayOffsets.length());
    d_holidayCodesIndex.reserveCapacity(other.d_holidayCodesIndex.length() +
                                                 d_holidayCodesIndex.length());
    d_holidayCodes.reserveCapacity(other.d_holidayCodes.length() +
                                                      d_holidayCodes.length());

    WeekendDaysTransitionSequence newWeekendDaysTransitions(
                               this->d_weekendDaysTransitions.get_allocator());
    newWeekendDaysTransitions.reserve(
      d_weekendDaysTransitions.size() + other.d_weekendDaysTransitions.size());
    unionWeekendDaysTransitions(&newWeekendDaysTransitions,
                                d_weekendDaysTransitions,
                                other.d_weekendDaysTransitions);
    d_weekendDaysTransitions.swap(newWeekendDaysTransitions);

    OffsetsConstReverseIterator r = d_holidayOffsets.rbegin(); // "read"
                                                               // iterator

    OffsetsConstReverseIterator c = other.d_holidayOffsets.rbegin();
                                                               // "compare"
                                                               // iterator

    // "end" iterators of both d_holidayOffsets

    OffsetsConstReverseIterator rend = d_holidayOffsets.rend();
    OffsetsConstReverseIterator cend = other.d_holidayOffsets.rend();

    // We initialize all new elements to 'd_holidayCodes.length()' so the
    // indexes are good for the last holiday.

    d_holidayOffsets.resize(other.d_holidayOffsets.length()
                                                  + d_holidayOffsets.length());
    d_holidayCodesIndex.resize(other.d_holidayCodesIndex.length()
                                                + d_holidayCodesIndex.length(),
                               d_holidayCodes.length());
    d_holidayCodes.resize(other.d_holidayCodes.length()
                                                    + d_holidayCodes.length());

    // We will use the following non-const iterators to update the vectors.

    OffsetsReverseIterator mOffsets = d_holidayOffsets.rbegin();
    CodesIndexReverseIterator mCodesIndex = d_holidayCodesIndex.rbegin();
    CodesReverseIterator mCodes = d_holidayCodes.rbegin();

    int lastCodeIndex = d_holidayCodes.length();  // last valid code index

    while (c != cend || r != rend) {
        OffsetsConstReverseIterator *read = 0;
        const PackedCalendar *readCalendar = 0;

        if (c == cend || (r != rend && *r > (*c + delta))) {
            if (fixIfDeltaPositive) {
                Date h = d_firstDate + *r;
                if (h > newLastDate || h < newFirstDate) {
                    ++r;
                    continue;
                }
            }
            read = &r;
            readCalendar = this;
            *mOffsets = *r;
        }

        // Note that we are also checking for duplicates in the next block.

        if (r == rend || (c != cend && (*c + delta) >= *r)) {
            BSLS_ASSERT(!read);
            BSLS_ASSERT(!readCalendar);

            if (fixIfDeltaPositive) {
                Date h = other.d_firstDate + *c;
                if (h > newLastDate || h < newFirstDate) {
                    ++c;
                    continue;
                }
            }

            // If this holiday is present in 'r' and 'c', we will not set read
            // and 'readCalendar'.

            if (r == rend || *r != (*c + delta)) {
                read = &c;
                readCalendar = &other;
            }
            *mOffsets = *c + delta;
        }

        // We need to adjust the value of the offset if the 'd_firstDate' is
        // changing.

        if ((fixIfDeltaPositive  && delta > 0)
         || (!fixIfDeltaPositive && delta < 0)) {
            *mOffsets -= delta;
        }

        const CodesReverseIterator mCodesStart = mCodes;  // Keep the current
                                                          // value of 'mCodes'
                                                          // to compute later
                                                          // the no. of codes
                                                          // for this holiday.

        // *** UNION OF THE HOLIDAY CODES

        if (read) {

            // Find the range of holiday codes in this calendar.

            const OffsetsConstIterator base = read->base() - 1;
            CodesConstIterator codesBegin = readCalendar->
                                                       beginHolidayCodes(base);
            CodesConstIterator codes = readCalendar->endHolidayCodes(base);
            while (codes != codesBegin) {
                --codes;
                *mCodes = *codes;
                ++mCodes;
            }
        }
        else {
            const OffsetsConstIterator rbase = r.base() - 1;
            const CodesConstReverseIterator lhs(endHolidayCodes(rbase));
            const CodesConstReverseIterator lhse(beginHolidayCodes(rbase));

            const OffsetsConstIterator cbase = c.base() - 1;
            const CodesConstReverseIterator rhs(other.endHolidayCodes(cbase));
            const CodesConstReverseIterator rhse(
                                              other.beginHolidayCodes(cbase));

            mCodes = bsl::set_union(lhs, lhse, rhs, rhse, mCodes,
                                    bsl::greater<int>());
        }

        lastCodeIndex -= (mCodes - mCodesStart);

        BSLS_ASSERT(lastCodeIndex >= 0
                 && (OffsetsSizeType)lastCodeIndex <= d_holidayCodes.length());

        *mCodesIndex = lastCodeIndex;

        BSLS_ASSERT(mCodesIndex != d_holidayCodesIndex.rend());
        BSLS_ASSERT(mOffsets != d_holidayOffsets.rend());

        ++mCodesIndex;
        ++mOffsets;

        if (read) {
            ++*read;
        }
        else {
            ++r;
            ++c;
        }
    }

    // Remove the unused part of the vectors.

    d_holidayOffsets.erase(d_holidayOffsets.begin(), mOffsets.base());
    d_holidayCodesIndex.erase(d_holidayCodesIndex.begin(), mCodesIndex.base());
    d_holidayCodes.erase(d_holidayCodes.begin(),
                         d_holidayCodes.begin() + lastCodeIndex);

    // Fix the codes index array, all the indexes are off by the number of
    // elements we remove.

    if (d_holidayCodesIndex.begin() != d_holidayCodesIndex.end()) {
        const int adjustment = *d_holidayCodesIndex.begin();
        if (adjustment != 0) {
            bsl::transform(d_holidayCodesIndex.begin(),
                           d_holidayCodesIndex.end(),
                           d_holidayCodesIndex.begin(),
                           bsl::bind2nd(bsl::minus<int>(), adjustment));
        }
    }

    BSLS_ASSERT(d_holidayOffsets.length() == d_holidayCodesIndex.length());
    BSLS_ASSERT(d_holidayOffsets.empty()
      || (OffsetsSizeType)d_holidayCodesIndex.back() <= d_holidayCodes.length());
    */
}

// CREATORS
PackedCalendar::PackedCalendar(bslma::Allocator *basicAllocator)
: d_firstDate(9999, 12, 31)
, d_lastDate(1, 1, 1)
, d_weekendDaysTransitions(basicAllocator)
, d_holidayOffsets(basicAllocator)
, d_holidayCodesIndex(basicAllocator)
, d_holidayCodes(basicAllocator)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
}

PackedCalendar::PackedCalendar(const Date&       firstDate,
                               const Date&       lastDate,
                               bslma::Allocator *basicAllocator)
: d_firstDate(firstDate <= lastDate ? firstDate : Date(9999, 12, 31))
, d_lastDate(firstDate <= lastDate ? lastDate : Date(1, 1, 1))
, d_weekendDaysTransitions(basicAllocator)
, d_holidayOffsets(basicAllocator)
, d_holidayCodesIndex(basicAllocator)
, d_holidayCodes(basicAllocator)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
}

PackedCalendar::PackedCalendar(const PackedCalendar&  original,
                               bslma::Allocator      *basicAllocator)
: d_firstDate(original.d_firstDate)
, d_lastDate(original.d_lastDate)
, d_weekendDaysTransitions(original.d_weekendDaysTransitions, basicAllocator)
, d_holidayOffsets(original.d_holidayOffsets, basicAllocator)
, d_holidayCodesIndex(original.d_holidayCodesIndex, basicAllocator)
, d_holidayCodes(original.d_holidayCodes, basicAllocator)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
}

PackedCalendar::~PackedCalendar()
{
    BSLS_ASSERT(d_holidayOffsets.length() == d_holidayCodesIndex.length());
    BSLS_ASSERT(d_holidayOffsets.isEmpty()
                || (OffsetsSizeType)d_holidayCodesIndex.back()
                                                   <= d_holidayCodes.length());
}

// MANIPULATORS
PackedCalendar& PackedCalendar::operator=(const PackedCalendar& rhs)
{
    PackedCalendar(rhs, d_allocator_p).swap(*this);
    return *this;
}

void PackedCalendar::swap(PackedCalendar& other)
{
    // 'swap' is undefined for objects with non-equal allocators.

    BSLS_ASSERT(d_allocator_p == other.d_allocator_p);

    bslalg::SwapUtil::swap(&d_firstDate,         &other.d_firstDate);
    bslalg::SwapUtil::swap(&d_lastDate,          &other.d_lastDate);
    bslalg::SwapUtil::swap(&d_weekendDaysTransitions,
                           &other.d_weekendDaysTransitions);
    bslalg::SwapUtil::swap(&d_holidayOffsets,    &other.d_holidayOffsets);
    bslalg::SwapUtil::swap(&d_holidayCodesIndex, &other.d_holidayCodesIndex);
    bslalg::SwapUtil::swap(&d_holidayCodes,      &other.d_holidayCodes);
}

void PackedCalendar::addDay(const Date& date)
{
    addDayImp(&d_firstDate, &d_lastDate, &d_holidayOffsets, date);
}

void PackedCalendar::addHoliday(const Date& date)
{
    addDayImp(&d_firstDate, &d_lastDate, &d_holidayOffsets, date);
    addHolidayImp(date - d_firstDate);

    BSLS_ASSERT(d_holidayOffsets.length() == d_holidayCodesIndex.length());
}

int PackedCalendar::addHolidayIfInRange(const Date& date)
{
    if (isInRange(date)) {
        addHolidayImp(date - d_firstDate);
        return 0;
    }
    return -1;
}

void PackedCalendar::addHolidayCode(const Date& date, int holidayCode)
{
    addDayImp(&d_firstDate, &d_lastDate, &d_holidayOffsets, date);
    const int index = addHolidayImp(date - d_firstDate);
    const OffsetsConstIterator holiday = d_holidayOffsets.begin() + index;
    const CodesConstIterator b = beginHolidayCodes(holiday);
    const CodesConstIterator e = endHolidayCodes(holiday);

    CodesConstIterator it = bsl::lower_bound(b, e, holidayCode);
    if (it == e || holidayCode != *it) {
        it = d_holidayCodes.insert(it, holidayCode);

        // Since we inserted a code in the codes vectors, all the indexes for
        // the holidays following 'date' will be off by one; let's fix that.

        for (bsl::size_t i = index + 1;
             i < d_holidayCodesIndex.length();
             ++i) {
            d_holidayCodesIndex.replace(i, d_holidayCodesIndex[i] + 1);
        }
    }
    BSLS_ASSERT(d_holidayOffsets.length() == d_holidayCodesIndex.length());
    BSLS_ASSERT(d_holidayOffsets.isEmpty()
                || (OffsetsSizeType)d_holidayCodesIndex.back()
                                                   <= d_holidayCodes.length());
}

int PackedCalendar::addHolidayCodeIfInRange(const Date& date, int holidayCode)
{
    if (isInRange(date)) {
        addHolidayCode(date, holidayCode);
        return 0;
    }
    return -1;
}


void PackedCalendar::addWeekendDay(DayOfWeek::Enum weekendDay)
{
    BSLS_ASSERT(d_weekendDaysTransitions.empty()
                || (1           == d_weekendDaysTransitions.size() &&
                    Date(1,1,1) == d_weekendDaysTransitions[0].first));

    if (d_weekendDaysTransitions.empty()) {
        DayOfWeekSet weekendDays;
        weekendDays.add(weekendDay);
        d_weekendDaysTransitions.push_back(
                              WeekendDaysTransition(Date(1,1,1), weekendDays));
    }
    else {
        BSLS_ASSERT(d_weekendDaysTransitions[0].first == Date(1,1,1));

        d_weekendDaysTransitions[0].second.add(weekendDay);
    }
}

void PackedCalendar::addWeekendDays(const DayOfWeekSet& weekendDays)
{
    BSLS_ASSERT(d_weekendDaysTransitions.empty()
                || (1           == d_weekendDaysTransitions.size() &&
                    Date(1,1,1) == d_weekendDaysTransitions[0].first));

    if (d_weekendDaysTransitions.empty()) {
        d_weekendDaysTransitions.push_back(
                         WeekendDaysTransition(Date(1,1,1), weekendDays));
    }
    else {
        BSLS_ASSERT(d_weekendDaysTransitions[0].first == Date(1,1,1));

        d_weekendDaysTransitions[0].second |= weekendDays;
    }
}

void PackedCalendar::addWeekendDaysTransition(const Date&         date,
                                              const DayOfWeekSet& weekendDays)
{
    WeekendDaysTransition newTransition(date, weekendDays);

    WeekendDaysTransitionSequence::iterator it =
                             bsl::lower_bound(d_weekendDaysTransitions.begin(),
                                              d_weekendDaysTransitions.end(),
                                              newTransition,
                                              WeekendDaysTransitionLess());

    if (it != d_weekendDaysTransitions.end() && it->first == date) {
        it->second = weekendDays;
    }
    else {
        d_weekendDaysTransitions.insert(it, newTransition);
    }
}

void
PackedCalendar::intersectBusinessDays(const PackedCalendar& other)
{
    // The 'true' indicates that we want to intersect the range.

    intersectBusinessDaysImp(other, true);

    d_firstDate = d_firstDate > other.d_firstDate
                  ? d_firstDate
                  : other.d_firstDate;
    d_lastDate  = d_lastDate < other.d_lastDate
                  ? d_lastDate
                  : other.d_lastDate;

    // Normalize "empty" calendars.

    if (d_firstDate > d_lastDate) {
        BSLS_ASSERT(d_holidayOffsets.isEmpty());
        BSLS_ASSERT(d_holidayCodesIndex.isEmpty());
        BSLS_ASSERT(d_holidayCodes.isEmpty());

        setValidRange(d_firstDate, d_lastDate);
    }
}

void
PackedCalendar::intersectNonBusinessDays(const PackedCalendar& other)
{
    // The 'true' indicates that we want to intersect the range.

    intersectNonBusinessDaysImp(other, true);

    const int firstDelta = other.d_firstDate - d_firstDate;
    const int lastDelta  = other.d_lastDate  - d_lastDate;
    if (firstDelta > 0) {
        d_firstDate += firstDelta;
    }
    if (lastDelta < 0) {
        d_lastDate += lastDelta;
    }

    // Normalize "empty" calendars.

    if (d_firstDate > d_lastDate) {
        BSLS_ASSERT(d_holidayOffsets.isEmpty());
        BSLS_ASSERT(d_holidayCodesIndex.isEmpty());
        BSLS_ASSERT(d_holidayCodes.isEmpty());

        setValidRange(d_firstDate, d_lastDate);
    }
}

void PackedCalendar::unionBusinessDays(const PackedCalendar& other)
{
    // The 'false' indicates that we do not want to intersect the range, but
    // instead unite it.

    intersectNonBusinessDaysImp(other, false);

    const int firstDelta = other.d_firstDate - d_firstDate;
    const int lastDelta  = other.d_lastDate - d_lastDate;

    if (firstDelta < 0) {
        d_firstDate += firstDelta;
    }
    if (lastDelta > 0) {
        d_lastDate += lastDelta;
    }

}

void
PackedCalendar::unionNonBusinessDays(const PackedCalendar& other)
{
    // The 'false' indicates that we do not want to intersect the range, but
    // instead unite it.

    intersectBusinessDaysImp(other, false);

    d_firstDate = d_firstDate < other.d_firstDate
                  ? d_firstDate
                  : other.d_firstDate;
    d_lastDate  = d_lastDate > other.d_lastDate
                  ? d_lastDate
                  : other.d_lastDate;
}

void PackedCalendar::removeHoliday(const Date& date)
{
    const int offset = date - d_firstDate;
    const OffsetsConstIterator oit = bsl::lower_bound(d_holidayOffsets.begin(),
                                                      d_holidayOffsets.end(),
                                                      offset);

    if (oit != d_holidayOffsets.end() && *oit == offset) {
        const CodesConstIterator b = beginHolidayCodes(oit);
        const CodesConstIterator e = endHolidayCodes(oit);
        const int codesRemoved = e - b;

        d_holidayCodes.erase(b, e);
        CodesIndexConstIterator cit = d_holidayCodesIndex.begin() +
                                             (oit - d_holidayOffsets.begin());
        /* TBD
        bsl::transform(cit + 1, d_holidayCodesIndex.end(), cit + 1,
                       bsl::bind2nd(bsl::minus<int>(), codesRemoved));
        */

        cit = d_holidayCodesIndex.begin() + (oit - d_holidayOffsets.begin());
        d_holidayCodesIndex.erase(cit, cit + 1);
        d_holidayOffsets.erase(oit, oit + 1);
    }
    BSLS_ASSERT(d_holidayOffsets.length() == d_holidayCodesIndex.length());
    BSLS_ASSERT(d_holidayOffsets.isEmpty()
      || (OffsetsSizeType)d_holidayCodesIndex.back() <= d_holidayCodes.length());
}

void PackedCalendar::removeHolidayCode(const Date& date, int holidayCode)
{
    /* TBD
    const int offset = date - d_firstDate;
    const OffsetsConstIterator oit = bsl::lower_bound(d_holidayOffsets.begin(),
                                                      d_holidayOffsets.end(),
                                                      offset);

    if (oit != d_holidayOffsets.end() && *oit == offset) {
        const CodesIterator b = beginHolidayCodes(oit);
        const CodesIterator e = endHolidayCodes(oit);

        CodesIterator cit = bsl::lower_bound(b, e, holidayCode);
        if (cit != e && *cit == holidayCode) {
            d_holidayCodes.erase(cit, cit + 1);
            const int shift = oit - d_holidayOffsets.begin() + 1;
            bsl::transform(d_holidayCodesIndex.begin() + shift,
                           d_holidayCodesIndex.end(),
                           d_holidayCodesIndex.begin() + shift,
                           bsl::bind2nd(bsl::minus<int>(), 1));
        }
    }
    BSLS_ASSERT(d_holidayOffsets.length() == d_holidayCodesIndex.length());
    BSLS_ASSERT(d_holidayOffsets.isEmpty()
      || (OffsetsSizeType)d_holidayCodesIndex.back() <= d_holidayCodes.length());
    */
}

void PackedCalendar::removeAll()
{
    d_firstDate.setYearMonthDay(9999, 12, 31);
    d_lastDate.setYearMonthDay(1, 1, 1);
    d_weekendDaysTransitions.clear();
    d_holidayOffsets.removeAll();
    d_holidayCodesIndex.removeAll();
    d_holidayCodes.removeAll();
}

void PackedCalendar::setValidRange(const Date& firstDate, const Date& lastDate)
{
    /* TBD
    if (firstDate > lastDate) {
        WeekendDaysTransitionSequence weekendDaysTransitions(
                                     d_weekendDaysTransitions.get_allocator());
        d_weekendDaysTransitions.swap(weekendDaysTransitions);
        removeAll();
        d_weekendDaysTransitions.swap(weekendDaysTransitions);

        return;
    }

    if (lastDate < d_firstDate || firstDate > d_lastDate) {
        d_holidayOffsets.removeAll();
        d_holidayCodesIndex.removeAll();
        d_holidayCodes.removeAll();
        d_firstDate = firstDate;
        d_lastDate = lastDate;
        return;
    }

    OffsetsConstIterator b  = d_holidayOffsets.begin();
    OffsetsConstIterator e  = d_holidayOffsets.end();
    OffsetsConstIterator it = bsl::lower_bound(b,
                                               e,
                                               lastDate - d_firstDate + 1);

    if (it != e) {
        BSLS_ASSERT(lastDate <= d_lastDate);

        CodesIndexConstIterator jt(d_holidayCodesIndex.begin());
        jt += (it - b);
        CodesIterator kt(d_holidayCodes.begin());
        kt += *jt;
        d_holidayCodes.erase(kt, d_holidayCodes.end());
        d_holidayCodesIndex.erase(jt, d_holidayCodesIndex.end());
        d_holidayOffsets.erase(it, d_holidayOffsets.end());
    }
    d_lastDate = lastDate;

    if (d_firstDate == firstDate) {
        return;
    }

    if (d_firstDate < firstDate) {
        b = d_holidayOffsets.begin();
        e = d_holidayOffsets.end();

        // We're looking for all holidays lower and up to firstDate -
        // d_firstDate - 1.  upper_bound will return an iterator one past the
        // last element we want to remove.

        it = bsl::upper_bound(b, e, firstDate - d_firstDate - 1);
        CodesIndexConstIterator jt = d_holidayCodesIndex.begin();
        jt += (it - b);
        CodesIterator kt = d_holidayCodes.begin();
        kt += (it != e) ? *jt : d_holidayCodes.length();

        d_holidayCodes.erase(d_holidayCodes.begin(), kt);
        d_holidayCodesIndex.erase(d_holidayCodesIndex.begin(), jt);
        d_holidayOffsets.erase(d_holidayOffsets.begin(), it);
    }

    if (d_holidayCodesIndex.begin() != d_holidayCodesIndex.end()) {
        const int adjustment = *d_holidayCodesIndex.begin();
        if (adjustment != 0) {
            bsl::transform(d_holidayCodesIndex.begin(),
                           d_holidayCodesIndex.end(),
                           d_holidayCodesIndex.begin(),
                           bsl::bind2nd(bsl::minus<int>(), adjustment));
        }
    }

    if (firstDate != d_firstDate
     && d_holidayOffsets.begin() != d_holidayOffsets.end()) {
        bsl::transform(d_holidayOffsets.begin(), d_holidayOffsets.end(),
                       d_holidayOffsets.begin(),
                       bsl::bind2nd(bsl::minus<int>(),
                                    firstDate - d_firstDate));
    }

    d_firstDate = firstDate;

    BSLS_ASSERT((int)d_holidayOffsets.length() <= (lastDate - firstDate + 1));
    BSLS_ASSERT(d_holidayOffsets.length() == d_holidayCodesIndex.length());
    BSLS_ASSERT(d_holidayOffsets.isEmpty()
      || (OffsetsSizeType)d_holidayCodesIndex.back() <= d_holidayCodes.length());
    */
}

// ACCESSORS
PackedCalendar::HolidayCodeConstIterator
                      PackedCalendar::beginHolidayCodes(const Date& date) const
{
    BSLS_ASSERT(isInRange(date));

    const int offset = date - d_firstDate;
    const OffsetsConstIterator offsetBegin = d_holidayOffsets.begin();
    const OffsetsConstIterator offsetEnd   = d_holidayOffsets.end();
    const OffsetsConstIterator i = bsl::lower_bound(offsetBegin,
                                                    offsetEnd,
                                                    offset);
    int iterIndex;
    if (i == offsetEnd || *i != offset) {
        iterIndex = d_holidayCodes.length();
    }
    else {
        iterIndex = d_holidayCodesIndex[i - offsetBegin];
    }
    return HolidayCodeConstIterator(d_holidayCodes.begin() + iterIndex);
}

PackedCalendar::HolidayCodeConstIterator
                        PackedCalendar::endHolidayCodes(const Date& date) const
{
    BSLS_ASSERT(isInRange(date));

    const int offset = date - d_firstDate;
    const OffsetsConstIterator offsetBegin = d_holidayOffsets.begin();
    const OffsetsConstIterator offsetEnd   = d_holidayOffsets.end();
    const OffsetsConstIterator i = bsl::lower_bound(offsetBegin,
                                                    offsetEnd,
                                                    offset);
    int iterIndex;
    if (i == offsetEnd || *i != offset) {
        iterIndex = d_holidayCodes.length();
    }
    else {
        iterIndex = i != (offsetEnd - 1)
                    ? d_holidayCodesIndex[i - offsetBegin + 1]
                    : d_holidayCodes.length();
    }
    return HolidayCodeConstIterator(d_holidayCodes.begin() + iterIndex);
}

bool PackedCalendar::isWeekendDay(const Date& date) const
{
    if (d_weekendDaysTransitions.empty()) {
        return false;
    }

    DayOfWeekSet dummySet;

    WeekendDaysTransitionSequence::const_iterator it =
                        bsl::upper_bound(d_weekendDaysTransitions.begin(),
                                         d_weekendDaysTransitions.end(),
                                         WeekendDaysTransition(date, dummySet),
                                         WeekendDaysTransitionLess());

    if (it == d_weekendDaysTransitions.begin()) {
        return false;
    }
    else {
        --it;
        return it->second.isMember(date.dayOfWeek());
    }
}

int PackedCalendar::numHolidayCodes(const Date& date) const
{
    BSLS_ASSERT(isInRange(date));

    const OffsetsConstIterator it = bsl::lower_bound(d_holidayOffsets.begin(),
                                                     d_holidayOffsets.end(),
                                                     date - d_firstDate);
    if (it == d_holidayOffsets.end() || *it != (date - d_firstDate)) {
        return 0;
    }

    return endHolidayCodes(it) - beginHolidayCodes(it);
}

int PackedCalendar::numNonBusinessDays() const
{
    const int length = d_holidayOffsets.length();
    int sum = numWeekendDaysInRange() + length;

    for (OffsetsConstIterator it = d_holidayOffsets.begin();
                                          it != d_holidayOffsets.end(); ++it) {
        sum -= isWeekendDay(d_firstDate + *it);
    }

    return sum;
}

bsl::ostream& PackedCalendar::print(bsl::ostream& stream,
                                    int           level,
                                    int           spacesPerLevel) const
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(stream.bad())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;
        return stream;                                                // RETURN
    }
/* TBD
    const char NL = spacesPerLevel >= 0 ? '\n' : ' ';

    bslim::Printer printer(&stream, level, spacesPerLevel);
    printer.start(true);  // 'true' -> suppress '['

    stream << buffer;

    printer.end(true);    // 'true' -> suppress ']'

    return stream;

    stream << "{" <<  NL;

    stream << "[ ";
    WeekendDaysTransitionConstIterator itr = beginWeekendDaysTransitions();
    while(itr != endWeekendDaysTransitions())
    {
        stream << itr->first << " : ";
        itr->second.print(stream, -1, -1);
        if (++itr != endWeekendDaysTransitions()) {
            stream << ", ";
        }
    }
    if (!d_weekendDaysTransitions.empty()) {
        stream << " ";
    }
    stream << "]" << NL;

    for (OffsetsConstIterator i = d_holidayOffsets.begin();
                                            i != d_holidayOffsets.end(); ++i) {

        if (spacesPerLevel >= 0) {
            bdeu_Print::indent(stream, level + 1, spacesPerLevel);
        }
        stream << (d_firstDate + *i);

        const CodesConstIterator b = beginHolidayCodes(i);
        const CodesConstIterator e = endHolidayCodes(i);
        if (b != e) {
            stream << " {" << NL;
        }

        for (CodesConstIterator j = b; j != e; ++j) {
            if (spacesPerLevel >= 0) {
                bdeu_Print::indent(stream, level + 2, spacesPerLevel);
            }
            stream << *j << NL;
        }

        if (spacesPerLevel >= 0) {
            bdeu_Print::indent(stream, level + 1, spacesPerLevel);
        }
        if (b != e) {
            stream << "}";
        }

        stream << NL;
    }
    if (spacesPerLevel >= 0) {
        bdeu_Print::indent(stream, level, spacesPerLevel);
    }
    stream << "}";

    if (spacesPerLevel >= 0) {
        stream << NL;
    }
*/
    return stream;
}

int PackedCalendar::numWeekendDaysInRange() const
{
    if (d_weekendDaysTransitions.empty() || d_firstDate > d_lastDate) {
        return 0;
    }

    // Find the nearest transition that has a date less than or equal to the
    // first date of calendar.

    DayOfWeekSet dummySet;
    WeekendDaysTransitionSequence::const_iterator itr =
                            bsl::upper_bound(d_weekendDaysTransitions.begin(),
                                             d_weekendDaysTransitions.end(),
                                             WeekendDaysTransition(d_firstDate,
                                                                   dummySet),
                                             WeekendDaysTransitionLess());

    int numWeekendDays = 0;
    Date firstDate;
    if (itr != d_weekendDaysTransitions.begin()) {
        --itr;
        firstDate = d_firstDate;
    }
    else {
        firstDate = itr->first;
    }

    do {
        const DayOfWeekSet& weekendDays = itr->second;
        Date lastDate;

        ++itr;
        if (itr != d_weekendDaysTransitions.end() &&
                                                    itr->first <= d_lastDate) {
            lastDate = itr->first - 1;
        }
        else {
            lastDate = d_lastDate;
        }

        numWeekendDays += numWeekendDaysInRangeImp(firstDate,
                                                   lastDate,
                                                   weekendDays);

        if (itr == d_weekendDaysTransitions.end() )
        {
            break;
        }

        firstDate = itr->first;

    } while (firstDate <= d_lastDate);

    return numWeekendDays;
}

// FREE OPERATORS
bool operator==(const PackedCalendar& lhs, const PackedCalendar& rhs)
{
    return lhs.d_firstDate              == rhs.d_firstDate
        && lhs.d_lastDate               == rhs.d_lastDate
        && lhs.d_weekendDaysTransitions == rhs.d_weekendDaysTransitions
        && lhs.d_holidayOffsets         == rhs.d_holidayOffsets
        && lhs.d_holidayCodesIndex      == rhs.d_holidayCodesIndex
        && lhs.d_holidayCodes           == rhs.d_holidayCodes;
}


// FREE OPERATORS
bsl::ostream& operator<<(bsl::ostream& stream, const PackedCalendar& calendar)
{
    calendar.print(stream, 0, -1);
    return stream;
}

                  // ---------------------------------------------
                  // class PackedCalendar_BusinessDayConstIterator
                  // ---------------------------------------------

// PRIVATE MANIPULATORS
void PackedCalendar_BusinessDayConstIterator::nextBusinessDay()
{
    BSLS_ASSERT(false == d_endFlag);

    const int lastOffset =
                          d_calendar_p->lastDate() - d_calendar_p->firstDate();

    while (d_currentOffset < lastOffset) {
        ++d_currentOffset;

        if (d_offsetIter == d_calendar_p->d_holidayOffsets.end()
         || d_currentOffset < *d_offsetIter) {
            if (!d_calendar_p->isWeekendDay(
                                d_calendar_p->firstDate() + d_currentOffset)) {

                // We found the next business day.

                return;
            }
        }
        else {
            ++d_offsetIter;
        }
    }

    // We did not find a next business day.  This iterator becomes an 'end'
    // iterator.

    d_endFlag = true;
}

void PackedCalendar_BusinessDayConstIterator::previousBusinessDay()
{

    while (d_currentOffset >= 0) {
        if (false == d_endFlag) {
            --d_currentOffset;
        }
        else {
            d_endFlag = false;
        }

        if (d_offsetIter != d_calendar_p->d_holidayOffsets.begin()) {
            OffsetsConstIterator iter = d_offsetIter;
            --iter;
            if (d_currentOffset > *iter) {
                if (!d_calendar_p->isWeekendDay(
                                d_calendar_p->firstDate() + d_currentOffset)) {

                    // We found the previous business day.

                    return;
                }
            }
            else {
                --d_offsetIter;
            }

        }
        else if (!d_calendar_p->isWeekendDay(
                                d_calendar_p->firstDate() + d_currentOffset)) {

            // We found the previous business day.

            return;
        }
    }

    // No previous business day was found.  This is undefined behavior.

    BSLS_ASSERT(0);
}

// CREATORS
PackedCalendar_BusinessDayConstIterator::
     PackedCalendar_BusinessDayConstIterator(const PackedCalendar& calendar,
                                             const Date&           startDate,
                                             bool                  endIterFlag)
: d_calendar_p(&calendar)
, d_currentOffset(startDate - calendar.firstDate())
, d_endFlag(false)
{
    if (calendar.firstDate() > calendar.lastDate()) {
        d_endFlag = true;
        return;
    }

    d_offsetIter = bsl::lower_bound(calendar.d_holidayOffsets.begin(),
                                    calendar.d_holidayOffsets.end(),
                                    d_currentOffset);

    bool businessDayFlag = true;

    // Adjust 'd_offsetIter' to reference the first holiday after
    // 'd_currentOffset'.

    if (d_offsetIter != calendar.d_holidayOffsets.end()
     && *d_offsetIter == d_currentOffset) {
        businessDayFlag = false;
        ++d_offsetIter;
    }

    if (calendar.isWeekendDay(calendar.firstDate() + d_currentOffset)) {
        businessDayFlag = false;
    }

    if (false == businessDayFlag || true == endIterFlag) {
        // If 'startDate' is not a business day, move 'd_currentDate' to the
        // next business day.  If 'endIterFlag' is true, this constructor is
        // called from 'endBusinessDays' or 'endBusinessDays(Date& date)'.
        // We need to move the iterator to point to one element past the first
        // business day that occurs on or before the specified 'startDate'.  So
        // call 'nextBusinessDay' to move the iterator to the next element.

        nextBusinessDay();
    }
}

// MANIPULATORS
PackedCalendar_BusinessDayConstIterator&
PackedCalendar_BusinessDayConstIterator::operator=(
                            const PackedCalendar_BusinessDayConstIterator& rhs)
{
    d_offsetIter    = rhs.d_offsetIter;
    d_calendar_p    = rhs.d_calendar_p;
    d_currentOffset = rhs.d_currentOffset;
    d_endFlag       = rhs.d_endFlag;

    return *this;
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
