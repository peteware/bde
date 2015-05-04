// bdlt_packedcalendar.h                                              -*-C++-*-
#ifndef INCLUDED_BDLT_PACKEDCALENDAR
#define INCLUDED_BDLT_PACKEDCALENDAR

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a compact repository for weekend/holiday information.
//
//@CLASSES:
//  bdlt::PackedCalendar: compact repository of weekend/holiday information
//
//@SEE_ALSO: bdlt_calendar
//
//@DESCRIPTION: This component provides a value-semantic class,
// 'bdlt::PackedCalendar', that represents weekend and holiday information over
// a *valid* *range* of dates.  A 'bdlt::PackedCalendar' is an approximation to
// the same *mathematical* type, and is capable of representing the same subset
// of *mathematical* values, as a 'bdlt::Calendar'.
//
// But unlike 'bdlt::Calendar', which is optimized for runtime efficiency,
// 'bdlt::PackedCalendar' is designed to minimize the amount of in-process
// memory required to represent that information.  For example, a packed
// calendar having a valid range of 40 years, assuming roughly ten holidays
// per year, might consume just over 2K bytes (e.g., 4-8 bytes per holiday)
// plus an additional 4-16 bytes per holiday code (tending higher when each
// holiday code is for a unique holiday).
//
// Default-constructed calendars are empty, and have no valid range.  Calendars
// can also be constructed with an initial valid range, implying that all dates
// within that range are business days.  The 'setValidRange' and 'addDay'
// methods, respectively, initialize or extend the valid range of a calendar,
// and a suite of 'add' methods can be used to populate a calendar with weekend
// days and holidays.
//
// The 'addHolidayCode' method associates an integer "holiday code" with a
// specific date, and can be called repeatedly with different integers and the
// same date to build up a set of holiday codes for that date.  Note that
// holiday codes are unique integers that, from the perspective of the
// calendar, have no particular meaning.  Typically, the user will choose
// holiday code values that are indices into an auxiliary collection (such as a
// 'bsl::vector<bsl::string>') to identify holiday names for end-user display.
//
// Once a calendar is populated, a rich set of accessor methods can be used to
// determine, e.g., if a given date is a business day, or the number of
// non-business days within some subrange of the calendar.  The holidays
// within a calendar can be obtained in increasing (chronological) order using
// an iterator identified by the nested 'HolidayConstIterator' 'typedef'.  The
// set of holiday codes associated with an arbitrary date in a
// 'bdlt_PackedCalendar' (or the current holiday referred to by a
// 'HolidayConstIterator') can be obtained in increasing (numerical) order
// using an iterator identified by /the nested 'HolidayCodeConstIterator'
// 'typedef' (see below).
//
// Calendars are *fully* value-semantic objects, and, as such, necessarily
// support all of the standard value-semantic operations, such as default
// construction, copy construction and assignment, equality comparison,
// and externalization ('bdex' streaming, in particular).  Calendars also
// support the notions of both union and intersection merging operations,
// whereby a calendar can change its value to contain the union or intersection
// of its own contained weekend days, holidays, and holiday codes with those of
// another calendar object.  Such merging operations will, in general, also
// alter the valid range of the resulting calendar.  Note that merged calendars
// can be significantly more efficient for certain repeated
// "is-common-business-day" determinations among two or more calendars.
//
///Weekend Days and Weekend-Days Transitions
///-----------------------------------------
// A calendar maintain a set of dates considered to be weekend days.
// Typically, a calendar's weekend days falls on the same days of the week for
// the entire range of a calendar.  For example, the weekend for United States
// has consisted of Saturday and Sunday since the year 1940.  The
// 'addWeekendDay' and 'AddWeekendDays' methods can be used to specify the
// weekend days for these calendars.
//
// Sometimes, a calendar's weekend days changes over time.  For example,
// Bangladesh's weekend consists of Friday until June 1, 1997 when Bangladesh
// changed its weekends to be both Friday and Saturday.  Later, on October 1,
// 2001 Bangladesh reverted to a weekend of only Friday, until on September 9,
// 2009 Bangladesh again changed its weekends to be both Friday and Saturday.
//
// To optimize for space allocation while supporting both consistent and
// changing weekend days, a calendar represents weekend information using a
// sequence of weekend-days transitions, each of which comprises a date and a
// set of days of the week considered to the be the weekend on and after that
// date.  To represent the weekend days of Bangladesh, a calendar can use a
// sequence of four weekend-days transitions: (1) a transition on January 1,
// 0001 having a weekend day set containing only Friday, (2) a transition at
// June 1, 1997 having a weekend day set containing Friday and Saturday, (3) a
// transition at October 1, 2001 having a weekend day set containing only
// Friday, and (4) a transition at September 9, 2009 having a weekend day set
// containing Friday and Saturday.  To represent the weekend days of the United
// States, a calendar having a range after 1940 can use a single weekend-days
// transition on January 1, 0001 containing Saturday and Sunday.
//
// On construction, a calendar does not contain any weekend-days transitions.
// The 'addWeekendDaysTransition' method adds a new weekend-days transition.
// The 'addWeekendDay' and 'addWeekendDays' methods create a weekend-days
// transition at January 1, 0001, if one doesn't already exist, and update the
// set of weekend days for that transition.  'addWeekendDay' and
// 'addWeekendDays' should be only used for calendars having a consistent set
// of weekend days throughout their entire range.  As such, using these methods
// together with 'addWeekendDaysTransition' is unspecified.
//
///Nested Iterators
///----------------
// Also provided are several STL-style 'const' bidirectional iterators
// accessible as nested 'typedef's.  'HolidayConstIterator',
// 'HolidayCodeConstIterator', and 'WeekendDaysTransitionConstIterator',
// respectively, iterate over a chronologically ordered sequence of holidays, a
// numerically ordered sequence of holiday codes, and a sequence of
// chronologically ordered weekend-days transitions.  As a general rule,
// calling a 'const' method will not invalidate any iterators, and calling a
// non-'const' method might invalidate any of them; it is, however, guaranteed
// that attempting to add *duplicate* holidays or holiday codes will have no
// effect, and therefore will not invalidate any iterators.  It is also
// guaranteed that adding a new code for an existing holiday will not
// invalidate any 'HolidayConstIterator'.
//
///Performance and Exception-Safety Guarantees
///-------------------------------------------
// The asymptotic worst-case performance of representative operations is
// characterized using big-O notation, 'O[f(N,M,W,V)]'.  'N' and 'M' each refer
// to the combined number ('H + C') of holidays 'H' (i.e., method
// 'numHolidays') and holiday codes 'C' (i.e., 'numHolidayCodesTotal') in the
// respective packed calendars.  'W' and 'V' each refer to the (likely small)
// number of weekend-days transition in the respective packed calendars.  Here,
// *Best* *Case* complexity, denoted by 'B[f(N)]', is loosely defined (for
// manipulators) as the worst-case cost, provided that (1) no additional
// internal capacity is required, (2) the bottom of the valid range does not
// change, and (3) that if a holiday (or holiday code) is being added, it is
// being appended *to* *the* *end* of the current sequence (of the latest
// holiday).
//..
//                                    Worst       Best    Exception-Safety
//  Operation                          Case       Case      Guarantee
//  ---------                         -----       ----    ----------------
//  DEFAULT CTOR                      O[1]                No-Throw
//  COPY CTOR(N)                      O[N]                Exception Safe
//  N.DTOR()                          O[1]                No-Throw
//
//  N.OP=(M)                          O[M]                Basic <*>
//
//  N.reserveCapacity(H, C)           O[N]                Strong <*>
//
//  N.setValidRange(b, e)             O[N]        O[1]    Basic <*>
//  N.addDay(d)                       O[N]        O[1]    Basic <*>
//  N.addHoliday(d)                   O[N]        O[1]    Basic <*>
//  N.addHolidayCode(d,c)             O[N]        O[1]    Basic <*>
//
//  N.addWeekendDay(w)                O[1]                No-Throw
//  N.addWeekendDaysTransition(d,w)   O[W]                Basic <*>
//
//  N.intersectBusinessDays(M)        O[N+M+W+V]          Basic <*>
//  N.intersectNonBusinessDays(M)     O[N+M+W+V]          Basic <*>
//  N.unionBusinessDays(M)            O[N+M+W+V]          Basic <*>
//  N.unionNonBusinessDays(M)         O[N+M+W+V]          Basic <*>
//
//  N.removeHoliday(d)                O[N]                No-Throw
//  N.removeHolidayCode(d, c)         O[N]                No-Throw
//  N.removeAll();                    O[1]                No-Throw
//
//  N.swap(M)                         O[1]                No-Throw
//
//  N.firstDate()                     O[1]                No-Throw
//  N.lastDate()                      O[1]                No-Throw
//  N.length()                        O[1]                No-Throw
//
//  N.numHolidays()                   O[1]                No-Throw
//
//  N.numHolidayCodesTotal()          O[1]                No-Throw
//  N.numWeekendDaysInRange()         O[1]                No-Throw
//
//  N.isInRange(d);                   O[1]                No-Throw
//  N.isWeekendDay(w);                O[1]                No-Throw
//  N.isWeekendDay(d)                 O[log(W)]           No-Throw
//
//  N.isHoliday(d);                   O[log(N)]           No_Throw
//  N.isBusinessDay(d);               O[log(N)]           No_Throw
//  N.isNonBusinessDay(d);            O[log(N)]           No_Throw
//
//  N.numHolidayCodes(d)              O[log(N)]           No-Throw
//
//  N.numBusinessDays()               O[N]                No-Throw
//  N.numNonBusinessDays()            O[N]                No-Throw
//
//  other const methods               O[1] .. O[N]        No-Throw
//
//
//  OP==(N,M)                         O[min(N,M)+min(W+V) No-Throw
//  OP!=(N,M)                         O[min(N,M)+min(W+V) No-Throw
//
//                                    <*> No-Throw guarantee when
//                                                      capacity is sufficient.
//..
// Note that *all* of the non-creator methods of 'bdlt_PackedCalendar' provide
// the *No-Throw* guarantee whenever sufficient capacity is already available.
//
// Note that these are largely the same as 'bdlt_Calendar' *except* that the
// accessors 'isBusinessDay' and 'isNonBusinessDay' are logarithmic in the
// number of holidays in 'bdlt_PackedCalendar'.
//
///Usage
///-----
// The two subsections below illustrate various aspects of populating and using
// packed calendars.
//
///Populating Packed Calendars
///- - - - - - - - - - - - - -
// Packed calendars will typically be populated from a database or flat file.
// The user should employ an appropriate population mechanism that provides
// the desired holiday dates and associated holiday codes within some desired
// range.  For example, suppose we have created the following flat-file format
// that encodes calendar information, including holidays and holiday codes (we
// assume, for the simplicity of this example, that "Weekend Days" (i.e.,
// recurring non-business days) are always just Saturdays and Sundays):
//..
//  // HOLIDAY DATE   HOLIDAY CODES
//  // ------------   -------------
//  // Year Mon Day    #    Codes     Comments, separated by Semicolons (;)
//  // ---- --- ---   --- ---------   -------------------------------------
//     2010  1  18     1   57         ;Martin Luther King Day
//     2010  2  15     1   51         ;Presidents Day
//     2010  4   2     2   9 105      ;Easter Sunday (Observed); Good Friday
//     2010  5  31     1   16         ;Memorial Day
//     2010  7   5     1   28         ;Independence Day (Observed)
//     2010  9   6     1   44         ;Labor Day
//     2010 10  11     1   19         ;Columbus Day
//     2010 11   2     0              ;Election Day
//     2010 11  25     1   14         ;Thanksgiving Day
//     2010 12  25     1    4         ;Christmas Day (Observed)
//     2010 12  31     1   22         ;New Year's Day (Observed)
//..
// Let's now create a couple of primitive helper functions to extract
// holiday and holiday-code counts from a given input stream.  First we'll
// create one that skips over headers and comments to get to the next valid
// holiday record:
//..
//  int getNextHoliday(bsl::istream& input, bdlt::Date *holiday, int *numCodes)
//      // Load into the specified 'holiday' the date of the next holiday, and
//      // into 'numCodes' the associated number of holiday codes for the
//      // holiday read from the specified 'input' stream.  Return 0 on
//      // success, and a non-zero value (with no effect on '*holiday' and
//      // '*numCodes') otherwise.
//  {
//      enum { SUCCESS = 0, FAILURE = 1 };
//
//      int year, month, day, codes;
//
//      if (input.good()) {
//          input >> year;
//      }
//      if (input.good()) {
//          input >> month;
//      }
//      if (input.good()) {
//          input >> day;
//      }
//      if (input.good()) {
//          input >> codes;
//      }
//
//      if (input.good() && bdlt::Date::isValid(year, month, day)) {
//          *holiday  = bdlt::Date(year, month, day);
//          *numCodes = codes;
//          return SUCCESS;                                           // RETURN
//      }
//
//      return FAILURE;                                               // RETURN
//  }
//..
// Then we'll write a function that gets us an integer holiday code, or
// invalidates the stream if it cannot (note that negative holiday codes are
// not supported):
//..
//  void getNextHolidayCode(bsl::istream& input, int *result)
//      // Load, into the specified 'result', the value read from the specified
//      // 'input' stream.  If the next token is not an integer, invalidate the
//      // stream with no effect on 'result'.
//  {
//      int holidayCode;
//
//      if (input.good()) {
//          input >> holidayCode;
//      }
//
//      if (input.good()) {
//          *result = holidayCode;
//      }
//  }
//..
// Now that we have these helper functions, it's a simple matter to write a
// calendar loader function, 'load', that populates a given calendar with data
// in this "proprietary" format:
//..
//  void load(bsl::istream& input, bdlt::PackedCalendar *calendar)
//      // Populate the specified 'calendar' with holidays and corresponding
//      // codes read from the specified 'input' stream in our "proprietary"
//      // format (see above).  On success, 'input' will be empty, but
//      // valid; otherwise 'input' will be invalid.
//  {
//      bdlt::Date holiday;
//      int       numCodes;
//
//      while (0 == getNextHoliday(input, &holiday, &numCodes)) {
//          calendar->addHoliday(holiday);                       // add date
//          for (int i = 0; i < numCodes; ++i) {
//              int holidayCode;
//              getNextHolidayCode(input, &holidayCode);
//              calendar->addHolidayCode(holiday, holidayCode);  // add codes
//              input.ignore(256, '\n');  // skip comments
//          }
//      }
//  }
//..
// Note that different formats can easily be accommodated, while still using
// the same basic population strategy.  Also note that it may be substantially
// more efficient to populate calendars in increasing date order, compared
// to either reverse or random order.
//
///Using Packed Calendars
///- - - - - - - - - - -
// Higher-level clients (e.g., a GUI) may need to extract the holiday codes
// for a particular date, use them to look up their corresponding string names
// in a separate repository (e.g., a vector of strings) and to display these
// names to end users.  First let's create a function, that prints the
// names of holidays for a given date:
//..
//  void
//  printHolidayNamesForGivenDate(bsl::ostream&                   output,
//                                const bdlt::PackedCalendar&     calendar,
//                                const bdlt::Date&               date,
//                                const bsl::vector<bsl::string>& holidayNames)
//      // Write, to the specified 'output' stream, the elements in the
//      // specified 'holidayNames' associated (via holiday codes in the
//      // specified 'calendar') to the specified 'date'.  Each holiday
//      // name emitted is followed by a newline ('\n').  The behavior is
//      // undefined unless 'date' is within the valid range of 'calendar'.
//  {
//      for (bdlt::PackedCalendar::HolidayCodeConstIterator
//                                       it = calendar.beginHolidayCodes(date);
//                                       it != calendar.endHolidayCodes(date);
//                                     ++it) {
//          output << holidayNames[*it] << bsl::endl;
//      }
//  }
//..
// Now that we can write the names of holidays for a given date, let's
// write a function that can write out all of the names associated with each
// holiday in the calendar.
//..
//  void
//  printHolidayDatesAndNames(bsl::ostream&                   output,
//                            const bdlt::PackedCalendar&     calendar,
//                            const bsl::vector<bsl::string>& holidayNames)
//      // Write, to the specified 'output' stream, each date associated with
//      // a holiday in the specified 'calendar' followed by any elements in
//      // the specified 'holidayNames' (associated via holiday codes in
//      // 'calendar') corresponding to that date.  Each date emitted is
//      // preceded and followed by a newline ('\n').  Each holiday name
//      // emitted is followed by a newline ('\n').
//
//  {
//      for (bdlt::PackedCalendar::HolidayConstIterator
//                        it = calendar.beginHolidays();
//                                      it != calendar.endHolidays(); ++it) {
//          output << '\n' << *it << '\n';
//          printHolidayNamesForGivenDate(output,
//                                        calendar,
//                                        *it,
//                                        holidayNames);
//      }
//  }
//..
// Next, let's provide a way to write out the same information above, but limit
// it to the date values within a given range.
//..
//  void
//  printHolidaysInRange(bsl::ostream&                   output,
//                       const bdlt::PackedCalendar&     calendar,
//                       const bdlt::Date&               beginDate,
//                       const bdlt::Date&               endDate,
//                       const bsl::vector<bsl::string>& holidayNames)
//      // Write, to the specified 'output' stream, each date associated
//      // with a holiday in the specified 'calendar' within the (inclusive)
//      // range indicated by the specified 'beginDate' and 'endDate',
//      // followed by any elements in the specified 'holidayNames' (associated
//      // via holiday codes in 'calendar') corresponding to that date.  Each
//      // date emitted is preceded and followed by a newline ('\n').  Each
//      // holiday name emitted is followed by a newline ('\n').  The behavior
//      // is undefined unless both 'startDate' and 'endDate' are within the
//      // valid range of 'calendar' and 'startDate <= endDate'.
//
//  {
//      for (bdlt::PackedCalendar::HolidayConstIterator
//                               it = calendar.beginHolidays(beginDate);
//                               it != calendar.endHolidays(endDate);
//                             ++it) {
//          output << '\n' << *it << '\n';
//          printHolidayNamesForGivenDate(output,
//                                        calendar,
//                                        *it,
//                                        holidayNames);
//      }
//  }
//..
// Note that we could now reimplement 'printHolidayDatesAndNames', albeit less
// efficiently, in terms of 'printHolidaysInRange':
//..
//  printHolidayDatesAndNames(bsl::ostream&                   output,
//                            const bdlt::PackedCalendar&     calendar,
//                            const bsl::vector<bsl::string>& holidayNames)
//  {
//      if (!calendar.isEmpty()) {
//          printHolidaysInRange(output,
//                               calendar,
//                               calendar.beginDate()
//                               calendar.endDate());
//      }
//  }
//..
// Finally, low-level clients may also use a populated 'bdlt::PackedCalendar'
// object directly to determine whether a particular day is a valid business
// day; however, that operation, which here is logarithmic in the number of
// holidays, can be performed *much* more efficiently (see 'bdlt::Calendar'):
//..
//  bdlt::Date
//  getNextBusinessDay(const bdlt::PackedCalendar& calendar,
//                     const bdlt::Date&           date)
//      // Return the next business day in the specified 'calendar' after the
//      // specified 'date'.  The behavior is undefined unless such a date
//      // exists within the valid range of 'calendar'.
//  {
//      // Assume there is a business day in the valid range after date.
//
//      bdlt::Date candidate = date;
//      do {
//          ++candidate;
//      } while (calendar.isNonBusinessDay(candidate));
//                                                   // logarithmic complexity!
//      return candidate;
//  }
//..

#ifndef INCLUDED_BDLSCM_VERSION
#include <bdlscm_version.h>
#endif

#ifndef INCLUDED_BDLT_CALENDARREVERSEITERATOR
#include <bdlt_calendarreverseiterator.h>
#endif

#ifndef INCLUDED_BDLT_DATE
#include <bdlt_date.h>
#endif

#ifndef INCLUDED_BDLT_DAYOFWEEK
#include <bdlt_dayofweek.h>
#endif

#ifndef INCLUDED_BDLT_DAYOFWEEKSET
#include <bdlt_dayofweekset.h>
#endif

#ifndef INCLUDED_BDLC_PACKEDINTARRAY
#include <bdlc_packedintarray.h>
#endif

#ifndef INCLUDED_BDLC_PACKEDINTARRAYUTIL
#include <bdlc_packedintarrayutil.h>
#endif

#ifndef INCLUDED_BSLALG_TYPETRAITS
#include <bslalg_typetraits.h>
#endif

#ifndef INCLUDED_BSLALG_TYPETRAITUSESBSLMAALLOCATOR
#include <bslalg_typetraitusesbslmaallocator.h>
#endif

#ifndef INCLUDED_BSLMA_ALLOCATOR
#include <bslma_allocator.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

#ifndef INCLUDED_BSL_ALGORITHM
#include <bsl_algorithm.h>
#endif

#ifndef INCLUDED_BSL_IOSFWD
#include <bsl_iosfwd.h>
#endif

#ifndef INCLUDED_BSL_OSTREAM
#include <bsl_ostream.h>
#endif

#ifndef INCLUDED_BSL_UTILITY
#include <bsl_utility.h>
#endif

#ifndef INCLUDED_BSL_VECTOR
#include <bsl_vector.h>
#endif

namespace BloombergLP {
namespace bdlt {

class PackedCalendar_BusinessDayConstIterator;
class PackedCalendar_HolidayCodeConstIterator;
class PackedCalendar_HolidayConstIterator;
class PackedCalendar_WeekendDaysTransitionConstIterator;

                          // ====================
                          // class PackedCalendar
                          // ====================

class PackedCalendar {
    // This class implements a space-efficient, fully value-semantic repository
    // of weekend and holiday information over a *valid* *range* of dates.
    // This valid range, '[ firstDate() .. lastDate() ]', spans the first and
    // last dates of a calendar's accessible contents.  A calendar can be
    // "populated" with weekend and holiday information via a suite of 'add'
    // methods.  Any subset of days of the week may be specified as weekend
    // (i.e., recurring non-business) days starting from a specified date by
    // adding a weekend-days transition; holidays within the valid range are
    // specified individually.  When adding a holiday, an arbitrary integer
    // "holiday code" may be associated with that date.  Additional holiday
    // codes for that date may subsequently be added.  Both the holidays and
    // the set of unique holiday codes associated with each holiday date are
    // maintained (internally) in order of increasing value.  Note that the
    // behavior of requesting *any* calendar information for a supplied date
    // whose value is outside the current *valid* *range* for that calendar
    // (unless otherwise noted, e.g., 'isWeekendDay') is undefined.
    //
    // More generally, this class supports a complete set of *value* *semantic*
    // operations, including copy construction, assignment, equality
    // comparison, 'ostream' printing, and 'bdex' serialization.  (A precise
    // operational definition of when two objects have the same value can be
    // found in the description of the homogeneous (free) 'operator==' for this
    // class.)  This class is *exception* *safe*, but provides no general
    // guarantee of rollback: If an exception is thrown during the invocation
    // of a method on a pre-existing object, the object will be left in a
    // coherent state, but (unless otherwise stated) its value is not defined.
    // In no event is memory leaked.  Finally, *aliasing* (e.g., using all or
    // part of an object as both source and destination) for the same
    // operation is supported in all cases.

  public:
    // PUBLIC TYPES
    typedef bsl::pair<Date, DayOfWeekSet>        WeekendDaysTransition;

  private:
    // PRIVATE TYPES
    typedef bdlc::PackedIntArray<int>::const_iterator OffsetsConstIterator;
    typedef bdlc::PackedIntArray<int>::const_iterator CodesIndexConstIterator;
    typedef bdlc::PackedIntArray<int>::const_iterator CodesConstIterator;

    typedef bsl::size_t OffsetsSizeType;
    typedef bsl::size_t CodesIndexSizeType;
    typedef bsl::size_t CodesSizeType;

    typedef bsl::vector<WeekendDaysTransition>   WeekendDaysTransitionSequence;

    struct WeekendDaysTransitionLess {
        // This 'struct' provides a comparator predicate for the type
        // 'WeekendDaysTransition' to enable the use of standard algorithms
        // (such as 'bsl::lower_bound') on ranges of objects of that type.

        // ACCESSORS
        bool operator() (const WeekendDaysTransition& lhs,
                         const WeekendDaysTransition& rhs) const
            // Return 'true' if the value of the specified 'lhs' is less than
            // (ordered before) the value of the specified 'rhs'.  The value of
            // 'lhs' is less than the value of 'rhs' if the date represented by
            // the data member 'first' of 'lhs' is earlier than the date
            // represented by the data member 'first' of 'rhs'.
        {
            return lhs.first < rhs.first;
        }
    };

    // DATA
    Date                      d_firstDate;  // first valid date of calendar
                                            // or (9999,12,31) if this calendar
                                            // is empty

    Date                      d_lastDate;   // last valid date of calendar
                                            // or (0001,01,01) if this calendar
                                            // is empty

    bsl::vector<WeekendDaysTransition>
                              d_weekendDaysTransitions;
                                            // chronological list of weekend
                                            // days transitions

    bdlc::PackedIntArray<int> d_holidayOffsets;
                                            // ordered list of all holidays
                                            // in this calendar stored as
                                            // offsets from 'd_firstDate'

    bdlc::PackedIntArray<int> d_holidayCodesIndex;
                                            // parallel to 'd_holidayOffsets',
                                            // this is a list of indices into
                                            // 'd_holidayCodes'; note that the
                                            // end of each sequence can be
                                            // determined using the value of
                                            // the next entry in this vector
                                            // if it exists, or else the length
                                            // of 'd_holidayCodes', itself

    bdlc::PackedIntArray<int> d_holidayCodes;
                                            // sequences of holiday codes,
                                            // each partitioned into an ordered
                                            // "chunk" of codes per holiday in
                                            // 'd_holidayOffsets'; chunks are
                                            // stored in the same order as in
                                            // 'd_holidayOffsets'

    bslma::Allocator *d_allocator_p;        // memory allocator (held, not
                                            // owned)

    // FRIENDS
    friend class PackedCalendar_BusinessDayConstIterator;

    friend bool operator==(const PackedCalendar&, const PackedCalendar&);
    friend bool operator!=(const PackedCalendar&, const PackedCalendar&);

  private:
    // PRIVATE MANIPULATORS
    CodesConstIterator beginHolidayCodes(const OffsetsConstIterator& iter);
        // TBD
        // Return an iterator that refers to the first modifiable holiday code
        // associated with the holiday referenced by the specified 'iter'.  If
        // there are no holiday codes associated with the date referenced by
        // 'iter', the returned iterator has the same value as that returned by
        // 'endHolidayCodes(iter)'.  The behavior is undefined unless 'iter'
        // refers to a valid holiday of this calendar.

    CodesConstIterator endHolidayCodes(const OffsetsConstIterator& iter);
        // TBD
        // Return an iterator that indicates the element one past the last
        // modifiable holiday code associated with the date referenced by the
        // specified 'iter'.  If there are no holiday codes associated with the
        // date referenced by 'iter', the returned iterator has the same value
        // as that returned by 'beginHolidayCodes(iter)'.  The behavior is
        // undefined unless 'iter' references a valid holiday in this calendar.

    int addHolidayImp(const int offset);
        // Add the specified 'offset' as a holiday offset in this calendar.  If
        // the date represented by the specified 'offset' is already a
        // holiday, the method has no effect.

    // PRIVATE ACCESSORS
    CodesConstIterator beginHolidayCodes(
                                       const OffsetsConstIterator& iter) const;
        // Return an iterator that refers to the first non-modifiable holiday
        // code for the holiday referenced by the specified 'iter'.  If there
        // are no holiday codes associated with the date referenced by 'iter',
        // the returned iterator has the same value as that returned by
        // 'endHolidayCodes(iter)'.  The behavior is undefined unless 'iter'
        // refers to a valid holiday of this calendar.

    CodesConstIterator endHolidayCodes(const OffsetsConstIterator& iter) const;
        // Return an iterator that indicates the element one past the last
        // holiday code associated with the date referenced by the specified
        // 'iter'.  If there are no holiday codes associated with the date
        // referenced by 'iter', the returned iterator has the same value as
        // that returned by 'beginHolidayCodes(iter)'.  The behavior is
        // undefined unless 'iter' references a valid holiday in this calendar.

  public:
    // TRAITS
    BSLALG_DECLARE_NESTED_TRAITS(PackedCalendar,
                                 bslalg::TypeTraitUsesBslmaAllocator);

    // TYPES
    typedef PackedCalendar_BusinessDayConstIterator   BusinessDayConstIterator;

    typedef PackedCalendar_HolidayConstIterator       HolidayConstIterator;

    typedef PackedCalendar_HolidayCodeConstIterator   HolidayCodeConstIterator;

    typedef bdlt::CalendarReverseIterator<BusinessDayConstIterator>
                                               BusinessDayConstReverseIterator;

    typedef bdlt::CalendarReverseIterator<HolidayConstIterator>
                                                   HolidayConstReverseIterator;

    typedef bdlt::CalendarReverseIterator<HolidayCodeConstIterator>
                                               HolidayCodeConstReverseIterator;

    typedef WeekendDaysTransitionSequence::const_iterator
                                            WeekendDaysTransitionConstIterator;

    // CLASS METHODS
    static int maxSupportedBdexVersion();
        // Return the most current 'bdex' streaming version number supported by
        // this class.  See the 'bdex' package-level documentation for more
        // information on 'bdex' streaming of value-semantic types and
        // containers.

    // CREATORS
    explicit PackedCalendar(bslma::Allocator *basicAllocator = 0);
        // Create an empty calendar having no valid range.  Optionally specify
        // a 'basicAllocator' used to supply memory.  If 'basicAllocator' is 0,
        // the currently installed default allocator is used.

    PackedCalendar(const Date&       firstDate,
                   const Date&       lastDate,
                   bslma::Allocator *basicAllocator = 0);
        // Create a calendar having a valid range from the specified
        // 'firstDate' through the specified 'lastDate' if
        // 'firstDate' <= lastDate'; otherwise, make the valid range empty.
        // Optionally specify a 'basicAllocator' used to supply memory.  If
        // 'basicAllocator' is 0, the currently installed default allocator is
        // used.

    PackedCalendar(const PackedCalendar&  original,
                   bslma::Allocator      *basicAllocator = 0);
        // Create a calendar having the value of the specified 'original'
        // calendar.  Optionally specify a 'basicAllocator' used to supply
        // memory.  If 'basicAllocator' is 0, the currently installed default
        // allocator is used.

    ~PackedCalendar();
        // Destroy this object.

    // MANIPULATORS
    PackedCalendar& operator=(const PackedCalendar& rhs);
        // Assign to this calendar the value of the specified 'rhs' calendar,
        // and return a reference to this modifiable calendar.

    void setValidRange(const Date& firstDate, const Date& lastDate);
        // Set the range of this calendar using the specified 'firstDate' and
        // 'lastDate' as, respectively, the first date and the last date of the
        // calendar if 'firstDate <= lastDate'.  Otherwise, the range is made
        // empty.  Any holiday that is outside the new range and its holiday
        // codes will be removed.

    void addDay(const Date& date);
        // Extend the valid range (if necessary) to include the specified
        // 'date' value.

    void addHoliday(const Date& date);
        // Mark the specified 'date' as a holiday (i.e., a non-business day).
        // Extend the valid range of this calendar if necessary.  Note that
        // this method has no effect if 'date' is already marked as a holiday.

    int addHolidayIfInRange(const Date& date);
        // Mark the specified 'date' as a holiday (i.e., a non-business day) if
        // 'date' is within the valid range of this calendar.  Return 0 if
        // 'date' is in range, and a non-zero value (with no effect on this
        // calendar) otherwise.  This method has no effect if 'date' is already
        // marked as a holiday or is not in the valid range.

    void addHolidayCode(const Date& date, int holidayCode);
        // Mark the specified 'date' as a holiday (i.e., a non-business day)
        // and add the specified 'holidayCode' (if not already present) to the
        // ordered set of codes associated with 'date'.  Extend the valid
        // range of this calendar if necessary.  If 'holidayCode' is already a
        // code for 'date', this method has no effect (i.e., it will neither
        // change the state of this object nor invalidate any iterators).  If
        // 'date' is already marked as a holiday, this method will not
        // invalidate any 'HolidayConstIterator' or 'BusinessDayConstIterator'
        // iterators.

    int addHolidayCodeIfInRange(const Date& date, int holidayCode);
        // Mark the specified 'date' as a holiday (i.e., a non-business day)
        // and add the specified 'holidayCode' (if not already present) to the
        // set of codes associated with 'date' if 'date' is within the valid
        // range of this calendar.  Return 0 if 'date' is in range, and a
        // non-zero value otherwise.  If 'holidayCode' is already a code for
        // 'date' or if 'date' is not in the valid range, this method will
        // neither change the value of the object nor invalidate any iterators.
        // If 'date' is already marked as a holiday, this method will not
        // invalidate any 'HolidayConstIterator' iterators.  Note that this
        // method may be called repeatedly with the same value for 'date' to
        // build up a set of holiday codes for that date.

    void addWeekendDay(DayOfWeek::Enum weekendDay);
        // Add the specified 'weekendDay' to the set of weekend days associated
        // with the weekend-days transition at January 1, 0001 maintained by
        // this calendar.  Create a transition at January 1, 0001 if one does
        // not exist.  The behavior is undefined if weekend-days transitions
        // were added to this calendar via the 'addWeekendDaysTransition'
        // method.

    void addWeekendDays(const DayOfWeekSet& weekendDays);
        // Add the specified 'weekendDays' to the set of weekend days
        // associated with the weekend-days transition at January 1, 0001
        // maintained by this calendar.  Create a transition at January 1, 0001
        // if one does not exist.  The behavior is undefined if weekend-days
        // transitions were added to this calendar via the
        // 'addWeekendDaysTransition' method.

    void addWeekendDaysTransition(const Date&         startDate,
                                  const DayOfWeekSet& weekendDays);
        // Add to this calendar a weekend-days transition on the specified
        // 'date' having the specified 'weekendDays' set.  If a weekend-days
        // transition already exists on 'date', replace the set of weekend days
        // of that transition with 'weekendDays'.  The behavior is undefined if
        // weekend days have been added to this calendar via either the
        // 'addWeekendDay' method or the 'addWeekendDays' method.

    void intersectBusinessDays(const PackedCalendar& other);
        // Merge the specified 'other' calendar into this calendar such that
        // the valid range of this calendar will become the *intersection* of
        // the two calendars' ranges, and the weekend days and holidays for
        // this calendar become the union of those (non-business) days from the
        // two calendars -- i.e., the valid business days of this calendar will
        // become the intersection of those of the two original calendar
        // values.  For each holiday that remains, the resulting holiday codes
        // in this calendar will be the union of the corresponding original
        // holiday codes.

    void intersectNonBusinessDays(const PackedCalendar& other);
        // Merge the specified 'other' calendar into this calendar such that
        // the valid range of this calendar will become the *intersection* of
        // the two calendars' ranges, and the weekend days and holidays for
        // this calendar are the intersection of those (non-business) days from
        // the two calendars -- i.e., the valid business days of this calendar
        // will become the union of those of the two original calendars, over
        // the *intersection* of their ranges.  For each holiday that remains,
        // the resulting holiday codes in this calendar will be the union of
        // the corresponding original holiday codes.

    void unionBusinessDays(const PackedCalendar& other);
        // Merge the specified 'other' calendar into this calendar such that
        // the valid range of this calendar will become the *union* of the two
        // calendars' ranges (or the minimal continuous range spanning the two
        // ranges, if the ranges are non-overlapping), and the weekend days
        // and holidays for this calendar are the intersection of those
        // (non-business) days from the two calendars -- i.e., the valid
        // business days of this calendar will become the union of those of the
        // two original calendar values.  For each holiday that remains, the
        // resulting holiday codes in this calendar will be the union of the
        // corresponding original holiday codes.

    void unionNonBusinessDays(const PackedCalendar& other);
        // Merge the specified 'other' calendar into this calendar such that
        // the valid range of this calendar will become the *union* of the two
        // calendars' ranges (or the minimal continuous range spanning the two
        // ranges, if the ranges are non-overlapping), and the weekend days
        // and holidays for this calendar will become the union of those
        // (non-business) days from the two calendars -- i.e., the valid
        // business days of this calendar will become the intersection of
        // those of the two calendars after each range is extended to cover
        // the resulting one.  For each holiday in either calendar, the
        // resulting holiday codes in this calendar will be the union of the
        // corresponding original holiday codes.

    void removeHoliday(const Date& date);
        // Remove from this calendar the holiday having the specified 'date' if
        // such a holiday exists.  Note that this operation has no effect if
        // 'date' is not a holiday in this calendar even if it is out of range.

    void removeHolidayCode(const Date& date, int holidayCode);
        // Remove from this calendar the specified 'holidayCode' for the
        // holiday having the specified 'date' if such a holiday having
        // 'holidayCode' exists.  Note that this operation has no effect if
        // 'date' is not a holiday in this calendar even if it is out of
        // range, or if the holiday at 'date' does not have 'holidayCode'
        // associated with it.

    void removeAll();
        // Remove all information from this calendar, leaving it with its
        // default constructed "empty" value.

    void reserveHolidayCapacity(int numHolidays);
        // Reserve enough space to store exactly the specified
        // 'numHolidays' within this calendar.  This method has no effect if
        // 'numHolidays < numHolidays()'.

    void reserveHolidayCodeCapacity(int numHolidayCodes);
        // Reserve enough space to store exactly the specified
        // 'numHolidayCodes' within this calendar.  This method has no effect
        // if 'numHolidayCodes <= numHolidayCodesTotal()'.

    void swap(PackedCalendar& other);
        // Swap the value of this object with the value of the specified
        // 'other' object.  This method provides the no-throw guarantee.  The
        // behavior is undefined if the two objects being swapped have
        // non-equal allocators.

    template <class STREAM>
    STREAM& bdexStreamIn(STREAM& stream, int version);
        // Assign to this object the value read from the specified input
        // 'stream' using the specified 'version' format and return a reference
        // to the modifiable 'stream'.  If 'stream' is initially invalid, this
        // operation has no effect.  If 'stream' becomes invalid during this
        // operation or if 'version' is not supported, this object is
        // unaltered.  Note that no version is read from 'stream'.  See the
        // 'bdex' package-level documentation for more information on 'bdex'
        // streaming of value-semantic types and containers.

    // ACCESSORS
    template <class STREAM>
    STREAM& bdexStreamOut(STREAM& stream, int version) const;
        // Write this value to the specified output 'stream' using the
        // specified 'version' format and return a reference to the
        // modifiable 'stream'.  If 'version' is not supported, 'stream' is
        // unmodified.  Note that 'version' is not written to 'stream'.
        // See the 'bdex' package-level documentation for more information
        // on 'bdex' streaming of value-semantic types and containers.

    WeekendDaysTransitionConstIterator beginWeekendDaysTransitions() const;
        // Return an iterator providing non-modifiable access to the first
        // weekend-day transition in the chronological sequence of weekend-day
        // transitions maintained by this calendar.

    WeekendDaysTransitionConstIterator endWeekendDaysTransitions() const;
        // Return an iterator providing non-modifiable access to the
        // past-the-end weekend-day transition in the chronological sequence of
        // weekend-day transitions maintained by this calendar.

    int numWeekendDaysTransitions() const;
        // Return the number of weekend-days transitions maintained by this
        // calendar.

    BusinessDayConstIterator beginBusinessDays() const;
        // Return an iterator that refers to the first business day in this
        // calendar.  If this calendar has no valid business days, the returned
        // iterator has the same value as that returned by 'endBusinessDays'.

    BusinessDayConstIterator beginBusinessDays(const Date& date) const;
        // Return an iterator that refers to the first business day that occurs
        // on or after the specified 'date' in this calendar.  If this calendar
        // has no such business day, the returned iterator has the same value
        // as that returned by 'endBusinessDays'.  The behavior is undefined
        // unless 'date' is within the valid range of this calendar.

    HolidayCodeConstIterator beginHolidayCodes(
                                       const HolidayConstIterator& iter) const;
        // Return an iterator that refers to the first holiday code for the
        // holiday referenced by the specified 'iter'.  If there is no holiday
        // code associated with the date referenced by the specified 'iter',
        // the returned iterator has the same value as that returned by
        // 'endHolidayCodes(iter)'.  The behavior is undefined unless 'iter'
        // refers to a valid holiday of this calendar.

    HolidayCodeConstIterator beginHolidayCodes(const Date& date) const;
        // Return an iterator that refers to the first holiday code for the
        // specified 'date'.  If there is no holiday code associated with
        // 'date', the returned iterator has the same value as that returned by
        // 'endHolidayCodes(date)'.  The behavior is undefined unless 'date' is
        // within the valid range of this calendar.  Note that 'date' need not
        // be marked as a holiday in this calendar.

    HolidayConstIterator beginHolidays() const;
        // Return an iterator that refers to the first holiday in this
        // calendar.  If this calendar has no holidays, the returned iterator
        // has the same value as that returned by 'endHolidays'.

    HolidayConstIterator beginHolidays(const Date& date) const;
        // Return an iterator that refers to the first holiday that occurs on
        // or after the specified 'date' in this calendar.  If this calendar
        // has no such holiday, the returned iterator has the same value as
        // that returned by 'endHolidays'.  The behavior is undefined unless
        // 'date' is within the valid range of this calendar.

    HolidayConstIterator beginHolidaysRaw(const Date& date) const;
        // Return an iterator that refers to the first holiday that occurs on
        // or after the specified 'date' in this calendar.  If this calendar
        // has no such holiday, the returned iterator has the same value as
        // that returned by 'endHolidays'.

    BusinessDayConstIterator endBusinessDays() const;
        // Return an iterator that indicates the element one past the last
        // business day in this calendar.  If this calendar has no valid
        // business days, the returned iterator has the same value as that
        // returned by 'beginBusinessDays'.

    BusinessDayConstIterator endBusinessDays(const Date& date) const;
        // Return an iterator that indicates the element one past the first
        // business day that occurs on or before the specified 'date' in this
        // calendar.  If this calendar has no such business day, the returned
        // iterator has the same value as that returned by
        // 'beginBusinessDays'.  The behavior is undefined unless 'date' is
        // within the valid range of this calendar.

    HolidayCodeConstIterator
    endHolidayCodes(const HolidayConstIterator& iter) const;
        // Return an iterator that indicates the element one past the last
        // holiday code associated with the date referenced by the specified
        // 'iter'.  If there are no holiday codes associated with the date
        // referenced by 'iter', the returned iterator has the same value as
        // that returned by 'beginHolidayCodes(iter)'.  The behavior is
        // undefined unless 'iter' references a valid holiday in this calendar.

    HolidayCodeConstIterator endHolidayCodes(const Date& date) const;
        // Return an iterator that indicates the element one past the last
        // holiday code associated with the specified 'date'.  If there are no
        // holiday codes associated with 'date', the returned iterator has the
        // same value as that returned by 'beginHolidayCodes(date)'.  The
        // behavior is undefined unless 'date' is within the valid range of
        // this calendar.  Note that 'date' need not be marked as a holiday in
        // this calendar.

    HolidayConstIterator endHolidays() const;
        // Return an iterator that indicates the element one past the last
        // holiday in this calendar.  If this calendar has no holidays, the
        // returned iterator has the same value as that returned by
        // 'beginHolidays'.

    HolidayConstIterator endHolidays(const Date& date) const;
        // Return an iterator that indicates the element one past the first
        // holiday that occurs on or before the specified 'date' in this
        // calendar.  If this calendar has no such holiday, the returned
        // iterator has the same value as that returned by 'beginHolidays'.
        // The behavior is undefined unless 'date' is within the valid range of
        // this calendar.

    HolidayConstIterator endHolidaysRaw(const Date& date) const;
        // Return an iterator that indicates the element one past the first
        // holiday that occurs on or before the specified 'date' in this
        // calendar.  If this calendar has no such holiday, the returned
        // iterator has the same value as that returned by 'beginHolidays'.

    const Date& firstDate() const;
        // Return a reference to the non-modifiable earliest date in the
        // valid range of this calendar.  The behavior is undefined unless
        // the calendar is non-empty -- i.e., unless '1 <= length()'.

    bool isBusinessDay(const Date& date) const;
        // Return 'true' if the specified 'date' is a business day (i.e.,
        // not a holiday or weekend day), and 'false' otherwise.  The behavior
        // is undefined unless 'date' is within the valid range of this
        // calendar.

    bool isHoliday(const Date& date) const;
        // Return 'true' if the specified 'date' is a holiday in this calendar,
        // and 'false' otherwise.  The behavior is undefined unless 'date' is
        // within the valid range of this calendar.

    bool isInRange(const Date& date) const;
        // Return 'true' if the specified 'date' is within the valid range of
        // this calendar (i.e., 'firstDate() <= date <= lastDate()'), and
        // 'false' otherwise.  Note that the valid range for a
        // 'PackedCalendar' is empty if its length is 0.

    bool isNonBusinessDay(const Date& date) const;
        // Return 'true' if the specified 'date' is not a business day (i.e.,
        // a holiday or weekend day), and 'false' otherwise.  The behavior is
        // undefined unless 'date' is within the valid range of this calendar.
        // Note that:
        //..
        //  !isBusinessday(date)
        //..
        // returns the same result, but calling this method may be faster.

    bool isWeekendDay(const Date& date) const;
        // Return 'true' if the specified 'date' falls on a day of the week
        // that is considered a weekend day in this calendar, and 'false'
        // otherwise.  Note that this method is defined for all 'Date'
        // values, not just those that fall within the valid range, and may
        // be invoked on even an empty calendar (i.e., having '0 == length()').

    bool isWeekendDay(DayOfWeek::Enum dayOfWeek) const;
        // Return 'true' if the specified 'dayOfWeek' is a weekend day in this
        // calendar, and 'false' otherwise.  The behavior is undefined if
        // weekend-days transitions were added to this calendar via the
        // 'addWeekendDaysTransition' method.

    const Date& lastDate() const;
        // Return a reference to the non-modifiable latest date in the valid
        // range of this calendar.  The behavior is undefined unless the
        // calendar is non-empty -- i.e., unless '1 <= length()'.

    int length() const;
        // Return the number of days in the valid range of this calendar,
        // which is defined to be 0 if this calendar is empty, and
        // 'lastDate() - firstDate() + 1' otherwise.

    int numBusinessDays() const;
        // Return the number of days in the valid range of this calendar that
        // are considered business days -- i.e., are neither holidays nor
        // weekend days.  Note that
        // 'numBusinessDays() == length() - numNonBusinessDays()'.

    int numHolidayCodes(const Date& date) const;
        // Return the number of (unique) holiday codes associated with the
        // specified 'date' in this calendar.  The behavior is undefined unless
        // 'date' is within the valid range of this calendar.

    int numHolidayCodesTotal() const;
        // Return the total number of holiday codes for all holidays in this
        // calendar.  Note that this function is used primarily in conjunction
        // with 'reserveHolidayCodeCapacity'.

    int numHolidays() const;
        // Return the number of days in the valid range of this calendar that
        // are individually marked as holidays, irrespective of whether or
        // not the date is also considered a weekend day.

    int numNonBusinessDays() const;
        // Return the number of days in the valid range of this calendar that
        // are *not* considered business days -- i.e., are either holidays,
        // weekend days, or both.  Note that
        // 'numNonBusinessDays() == length() - numNonBusinessDays()'.

    int numWeekendDaysInRange() const;
        // Return the number of days in the valid range of this calendar that
        // are considered weekend days, irrespective of any designated
        // holidays.

    bsl::ostream& print(bsl::ostream& stream,
                        int           level = 0,
                        int           spacesPerLevel = 4) const;
        // Format this object to the specified output 'stream' at the (absolute
        // value of) the optionally specified indentation 'level' and return a
        // reference to the modifiable 'stream'.  If 'level' is specified,
        // optionally specify 'spacesPerLevel', the number of spaces per
        // indentation level for this and all of its nested objects.  If
        // 'level' is negative, suppress indentation of the first line.  If
        // 'spacesPerLevel' is negative, format the entire output on one line,
        // suppressing all but the initial indentation (as governed by
        // 'level').  If 'stream' is not valid on entry, this operation has no
        // effect.

    BusinessDayConstReverseIterator rbeginBusinessDays() const;
        // Return an iterator that refers to the last business day in this
        // calendar.  If this calendar has no valid business days, the returned
        // iterator has the same value as that returned by
        // 'rendBusinessDays'.

    BusinessDayConstReverseIterator
    rbeginBusinessDays(const Date& date) const;
        // Return an iterator that refers to the first business day that occurs
        // on or before the specified 'date' in this calendar.  If this
        // calendar has no such business day, the returned iterator has the
        // same value as that returned by 'rendBusinessDays'.  The behavior
        // is undefined unless 'date' is within the valid range of this
        // calendar.

    HolidayCodeConstReverseIterator
    rbeginHolidayCodes(const HolidayConstIterator& iter) const;
        // Return an iterator that refers to the last holiday code associated
        // with the holiday referenced by the specified 'iter'.  If there are
        // no holiday codes associated with the date referenced by 'iter', the
        // returned iterator has the same value as that returned by
        // 'rendHolidayCodes(iter)'.  The behavior is undefined unless 'iter'
        // refers to a valid holiday of this calendar.

    HolidayCodeConstReverseIterator
    rbeginHolidayCodes(const Date& date) const;
        // Return an iterator that refers to the last holiday code associated
        // with the specified 'date' in this calendar.  If there are no holiday
        // codes associated with 'date', the returned iterator has the same
        // value as that returned by 'rendHolidayCodes(date)'.  The behavior is
        // undefined unless 'date' is within the valid range of this calendar.
        // Note that 'date' need not be marked as a holiday in this calendar.

    HolidayConstReverseIterator rbeginHolidays() const;
        // Return an iterator that refers to the last holiday in this calendar.
        // If this calendar has no holidays, the returned iterator has the same
        // value as that returned by 'rendHolidays'.

    HolidayConstReverseIterator rbeginHolidays(const Date& date) const;
        // Return an iterator that refers to the first holiday that occurs on
        // or before the specified 'date' in this calendar.  If this calendar
        // has no such holiday, the returned iterator has the same value as
        // that returned by 'rendHolidays'.  The behavior is undefined unless
        // 'date' is within the valid range of this calendar.

    BusinessDayConstReverseIterator rendBusinessDays() const;
        // Return an iterator that indicates the element one before the first
        // business day in this calendar.  If this calendar has no valid
        // business days, the returned iterator has the same value as that
        // returned by 'rbeginBusinessDays'.

    BusinessDayConstReverseIterator
    rendBusinessDays(const Date& date) const;
        // Return an iterator that indicates the element one before the first
        // business day that occurs on or before the specified 'date' in this
        // calendar.  If this calendar has no such business day, the returned
        // iterator has the same value as that returned by
        // 'rbeginBusinessDays'.  The behavior is undefined unless 'date' is
        // within the valid range of this calendar.

    HolidayCodeConstReverseIterator
    rendHolidayCodes(const HolidayConstIterator& iter) const;
        // Return an iterator that indicates the element one before the first
        // holiday code associated with the holiday referenced by the specified
        // 'iter'.  If there are no holiday codes associated with the date
        // referenced by 'iter', the returned iterator has the same value as
        // that returned by 'rbeginHolidayCodes(iter)'.  The behavior is
        // undefined unless 'iter' references a valid holiday in this calendar.

    HolidayCodeConstReverseIterator
    rendHolidayCodes(const Date& date) const;
        // Return an iterator that indicates the element one before the first
        // holiday code associated with the specified 'date'.  If there are no
        // holiday codes associated with 'date', the returned iterator has the
        // same value as that returned by 'rbeginHolidayCodes(date)'.  The
        // behavior is undefined unless 'date' is within the valid range of
        // this calendar.  Note that 'date' need not be marked as a holiday in
        // this calendar.

    HolidayConstReverseIterator rendHolidays() const;
        // Return an iterator that indicates the element one before the first
        // holiday in this calendar.  If this calendar has no holidays, the
        // returned iterator has the same value as that returned by
        // 'rbeginHolidays'.

    HolidayConstReverseIterator rendHolidays(const Date& date) const;
        // Return an iterator that indicates the element one before the first
        // holiday that occurs on or before the specified 'date' in this
        // calendar.  If this calendar has no such holiday, the returned
        // iterator has the same value as that returned by 'rbeginHolidays'.
        // The behavior is undefined unless 'date' is within the valid range of
        // this calendar.
};

// FREE OPERATORS
bool operator==(const PackedCalendar& lhs, const PackedCalendar& rhs);
    // Return 'true' if the specified 'lhs' and 'rhs' calendars have the same
    // value, and 'false' otherwise.  Two calendars have the same value if they
    // have the same valid range (or are both empty), the same weekend days,
    // the same holidays, and each corresponding pair of holidays, has the same
    // (ordered) set of associated holiday codes.

bool operator!=(const PackedCalendar& lhs, const PackedCalendar& rhs);
    // Return 'true' if the specified 'lhs' and 'rhs' calendars do not have the
    // same value, and 'false' otherwise.  Two calendars do not have the same
    // value if they do not have the same valid range (and are not both empty),
    // do not have the same weekend days, do not have the same holidays, or,
    // for at least one corresponding pair of holidays, do not have the same
    // (ordered) set of associated holiday codes.

bsl::ostream& operator<<(bsl::ostream&         stream,
                         const PackedCalendar& calendar);
    // Write the value of the specified 'calendar' to the specified output
    // 'stream', and return a reference to the modifiable 'stream'.

// FREE FUNCTIONS
void swap(PackedCalendar& a, PackedCalendar& b);
    // Swap the values of the specified 'a' and 'b' objects.  This method
    // provides the no-throw guarantee.  The behavior is undefined if the two
    // objects being swapped have non-equal allocators.

                   // ======================================
                   // class PackedCalendar_IteratorDateProxy
                   // ======================================

class PackedCalendar_IteratorDateProxy {
    // This object is a proxy class for 'Date' for use by the arrow
    // operator of 'PackedCalendar_HolidayConstIterator',
    // 'Calendar_BusinessDayConstIter', and
    // 'PackedCalendar_BusinessDayConstIterator'.  An object of this
    // class behaves as the 'Date' object with which it was constructed.

    // DATA
    Date d_date;  // proxied date

  private:
    // NOT IMPLEMENTED
    PackedCalendar_IteratorDateProxy&
    operator=(const PackedCalendar_IteratorDateProxy&);

  public:
    // CREATORS
    PackedCalendar_IteratorDateProxy(const Date& date);
        // Create a proxy object for the specified 'date'.

    ~PackedCalendar_IteratorDateProxy();
        // Destroy this object.

    PackedCalendar_IteratorDateProxy(
                             const PackedCalendar_IteratorDateProxy& original);
        // Create a proxy object referencing the same 'Date' value as the
        // specified 'original' proxy.

    // ACCESSORS
    const Date *operator->() const;
        // Return the address of the proxied date object.
};

                      // ============================
                      // class PackedCalendar_DateRef
                      // ============================

struct PackedCalendar_DateRef : Date {
    // This private class is used by the arrow operator of holiday iterator and
    // business day iterator classes.  The objects instantiated from this class
    // serve as references to 'Date' objects.

  private:
    // NOT IMPLEMENTED
    PackedCalendar_DateRef& operator=(const PackedCalendar_DateRef&);

  public:
    // CREATORS
    explicit PackedCalendar_DateRef(const Date& date);
        // Create a new object using the specified 'date'.

    PackedCalendar_DateRef(const PackedCalendar_DateRef& original);
        // Create a new object having the value of the specified 'original'
        // object.

    ~PackedCalendar_DateRef();
        // Destroy this object.

    // ACCESSORS
    PackedCalendar_IteratorDateProxy operator&() const;
        // Return this reference object.
};

                  // =========================================
                  // class PackedCalendar_HolidayConstIterator
                  // =========================================

class PackedCalendar_HolidayConstIterator {
    // Provide read-only, sequential access in increasing (chronological) order
    // to the holidays in a 'PackedCalendar' object.

    // DATA
    bdlc::PackedIntArray<int>::const_iterator d_iterator;  // array's iterator

    Date                             d_firstDate;   // offset date.  Note that
                                                    // dates are only 4-byte
                                                    // objects, so keeping a
                                                    // reference is not
                                                    // interesting
                                                    // performance-wise and
                                                    // size-wise.

    // FRIENDS
    friend class PackedCalendar;
    friend bool operator==(const PackedCalendar_HolidayConstIterator&,
                           const PackedCalendar_HolidayConstIterator&);
    friend bool operator!=(const PackedCalendar_HolidayConstIterator&,
                           const PackedCalendar_HolidayConstIterator&);

  private:
    // PRIVATE TYPES
    typedef bdlc::PackedIntArray<int>::const_iterator OffsetsConstIterator;

    // PRIVATE CREATORS
    PackedCalendar_HolidayConstIterator(const OffsetsConstIterator& iter,
                                        const Date                  firstDate);
        // Create a holiday iterator using the specified 'iter' and
        // 'firstDate'.

  public:
    // TYPES
    typedef bsl::bidirectional_iterator_tag  iterator_category;
    typedef Date                             value_type;
    typedef int                              difference_type;
    typedef PackedCalendar_IteratorDateProxy pointer;
    typedef PackedCalendar_DateRef           reference;
        // The star operator returns a 'PackedCalendar_DateRef' *by* *value*.


    // CREATORS
    PackedCalendar_HolidayConstIterator(
                          const PackedCalendar_HolidayConstIterator& original);
        // Create an iterator having the value of the specified 'original'
        // iterator.

    ~PackedCalendar_HolidayConstIterator();
        // Destroy this object.

    // MANIPULATORS
    PackedCalendar_HolidayConstIterator& operator=(
                               const PackedCalendar_HolidayConstIterator& rhs);
        // Assign to this iterator the value of the specified 'rhs' iterator,
        // and return a reference to this modifiable iterator.

    PackedCalendar_HolidayConstIterator& operator++();
        // Advance this iterator to refer to the next holiday in the calendar,
        // and return a reference to this modifiable object.  The behavior is
        // undefined unless, on entry, this iterator references a valid
        // holiday.

    PackedCalendar_HolidayConstIterator& operator--();
        // Regress this iterator to refer to the previous holiday in the
        // calendar and return a reference to this modifiable object.  The
        // behavior is undefined unless, on entry, this iterator references a
        // valid holiday.

    // ACCESSORS
    PackedCalendar_DateRef operator*() const;
        // Return a 'PackedCalendar_DateRef' object which contains the
        // date value of the holiday referenced by this iterator.

    PackedCalendar_IteratorDateProxy operator->() const;
        // Return a date value proxy for the current holiday.
};

// FREE OPERATORS
bool operator==(const PackedCalendar_HolidayConstIterator& lhs,
                const PackedCalendar_HolidayConstIterator& rhs);
    // Return 'true' if the specified 'lhs' and 'rhs' iterators point to the
    // same entry in the same calendar, and 'false' otherwise.  The behavior is
    // undefined unless 'lhs' and 'rhs' both iterate over the same calendar.

bool operator!=(const PackedCalendar_HolidayConstIterator& lhs,
                const PackedCalendar_HolidayConstIterator& rhs);
    // Return 'true' if the specified 'lhs' and 'rhs' iterators do not point to
    // the same entry in the same calendar, and 'false' otherwise.  The
    // behavior is undefined unless 'lhs' and 'rhs' both iterate over the same
    // calendar.

PackedCalendar_HolidayConstIterator
                operator++(PackedCalendar_HolidayConstIterator& iterator, int);
    // Advance the specified 'iterator' to refer to the next holiday in the
    // calendar, and return the previous value of 'iterator'.  The behavior is
    // undefined unless, on entry, 'iterator' references a valid business day.

PackedCalendar_HolidayConstIterator
                operator--(PackedCalendar_HolidayConstIterator& iterator, int);
    // Regress the specified 'iterator' to refer to the previous holiday in the
    // calendar, and return the previous value of 'iterator'.  The behavior is
    // undefined unless, on entry, 'iterator' references a valid holiday.

               // =============================================
               // class PackedCalendar_HolidayCodeConstIterator
               // =============================================

class PackedCalendar_HolidayCodeConstIterator {
    // Provide read-only, sequential access in increasing (numerical) order to
    // the holiday codes in a 'PackedCalendar' object.

    // DATA
    bdlc::PackedIntArray<int>::const_iterator d_iterator;  // array's iterator

    // FRIENDS
    friend class PackedCalendar;
    friend bool operator==(const PackedCalendar_HolidayCodeConstIterator&,
                           const PackedCalendar_HolidayCodeConstIterator&);
    friend bool operator!=(const PackedCalendar_HolidayCodeConstIterator&,
                           const PackedCalendar_HolidayCodeConstIterator&);
    friend bsl::ptrdiff_t operator-(
                               const PackedCalendar_HolidayCodeConstIterator&,
                               const PackedCalendar_HolidayCodeConstIterator&);

  private:
    // PRIVATE TYPES
    typedef bdlc::PackedIntArray<int>::const_iterator CodesConstIterator;

    // PRIVATE CREATORS
    PackedCalendar_HolidayCodeConstIterator(const CodesConstIterator& iter);
        // Create a holiday-code iterator using the specified 'iter'.

  public:
    // TYPES
    typedef bsl::bidirectional_iterator_tag  iterator_category;
    typedef int                              value_type;
    typedef int                              difference_type;
    typedef int                             *pointer;
    typedef int                              reference;
        // The star operator returns an 'int' *by value*.

    // CREATORS
    PackedCalendar_HolidayCodeConstIterator(
                      const PackedCalendar_HolidayCodeConstIterator& original);
        // Create an object having the value of the specified 'original'
        // iterator.

    ~PackedCalendar_HolidayCodeConstIterator();
        // Destroy this object.

    // MANIPULATORS
    PackedCalendar_HolidayCodeConstIterator& operator=(
                           const PackedCalendar_HolidayCodeConstIterator& rhs);
        // Assign to this object the value of the specified 'rhs' iterator, and
        // return a reference to this modifiable iterator.

    PackedCalendar_HolidayCodeConstIterator& operator++();
        // Advance this iterator to refer to the next holiday code for the
        // associated date in the calendar, and return a reference to this
        // modifiable object.  The behavior is undefined unless, on entry, this
        // iterator references a valid holiday code.

    PackedCalendar_HolidayCodeConstIterator& operator--();
        // Regress this iterator to refer to the previous holiday code for the
        // associated date in the calendar, and return a reference to this
        // modifiable object.  The behavior is undefined unless, on entry, this
        // iterator references a valid holiday code.

    // ACCESSORS
    int operator*() const;
        // Return the holiday code referenced by this iterator.
};

// FREE OPERATORS
bool operator==(const PackedCalendar_HolidayCodeConstIterator& lhs,
                const PackedCalendar_HolidayCodeConstIterator& rhs);
    // Return 'true' if the specified 'lhs' and 'rhs' iterators points to the
    // same entry, and 'false' otherwise.  The behavior is undefined unless
    // 'lhs' and 'rhs' both reference the same holiday in the same calendar.

bool operator!=(const PackedCalendar_HolidayCodeConstIterator& lhs,
                const PackedCalendar_HolidayCodeConstIterator& rhs);
    // Return 'true' if the specified 'lhs' and 'rhs' iterators do not point to
    // the same entry, and 'false' otherwise.  The behavior is undefined unless
    // 'lhs' and 'rhs' both reference the same holiday in the same calendar.

PackedCalendar_HolidayCodeConstIterator
            operator++(PackedCalendar_HolidayCodeConstIterator& iterator, int);
    // Advance the specified 'iterator' to refer to the next holiday code for
    // the associated date in the calendar, and return the previous value of
    // 'iterator'.  The behavior is undefined unless, on entry, 'iterator'
    // references a valid holiday code.

PackedCalendar_HolidayCodeConstIterator
            operator--(PackedCalendar_HolidayCodeConstIterator& iterator, int);
    // Regress the specified 'iterator' to refer to the previous holiday code
    // for the associated date in the calendar, and return the previous value
    // of 'iterator'.  The behavior is undefined unless, on entry, 'iterator'
    // references a valid holiday code.

bsl::ptrdiff_t operator-(const PackedCalendar_HolidayCodeConstIterator& lhs,
                         const PackedCalendar_HolidayCodeConstIterator& rhs);
    // Return the number of elements between specified 'lhs' and 'rhs'.  The
    // behavior is undefined unless 'lhs' and 'rhs' reference the same array.

              // =============================================
              // class PackedCalendar_BusinessDayConstIterator
              // =============================================

class PackedCalendar_BusinessDayConstIterator {
    // Provide read-only, sequential access in increasing (chronological) order
    // to the business days in a 'PackedCalendar' object.

    // DATA
    bdlc::PackedIntArray<int>::const_iterator d_offsetIter; // iterator for the
                                                            // holiday offsets

    const PackedCalendar             *d_calendar_p;     // pointer to the
                                                        // calendar

    int                               d_currentOffset;  // offset of the date
                                                        // referenced by this
                                                        // iterator

    bool                              d_endFlag;        // indicates an 'end'
                                                        // iterator if set to
                                                        // true

    // FRIENDS
    friend class PackedCalendar;
    friend bool operator==(const PackedCalendar_BusinessDayConstIterator&,
                           const PackedCalendar_BusinessDayConstIterator&);
    friend bool operator!=(const PackedCalendar_BusinessDayConstIterator&,
                           const PackedCalendar_BusinessDayConstIterator&);

  private:
    // PRIVATE TYPES
    typedef bdlc::PackedIntArray<int>::const_iterator OffsetsConstIterator;

    // PRIVATE CREATORS
    PackedCalendar_BusinessDayConstIterator(const PackedCalendar& calendar,
                                            const Date&           startDate,
                                            bool                  endIterFlag);
        // Create a business day iterator for the specified 'calendar'.
        // If the specified 'endIterFlag' is 'false', then this iterator
        // references the first business day on or after the specified
        // 'startDate'; otherwise, this iterator references one business day
        // *past* the first business day on or after 'startDate'.  If no
        // business day matching the above specification exists, then this
        // iterator will reference one day past the end of its range.

    // PRIVATE MANIPULATORS
    void nextBusinessDay();
        // Advance this iterator to the next business day.  The behavior is
        // undefined if this method is called when 'd_endFlag' is already true.

    void previousBusinessDay();
        // Regress this iterator to the previous business day.  The behavior is
        // undefined if the iterator is already at 'beginBusinessDay'.

  public:
    // TYPES
    typedef bsl::bidirectional_iterator_tag  iterator_category;
    typedef Date                             value_type;
    typedef int                              difference_type;
    typedef PackedCalendar_IteratorDateProxy pointer;
    typedef PackedCalendar_DateRef           reference;
        // The star operator returns a 'PackedCalendar_DateRef' *by* *value*.

    // CREATORS
    PackedCalendar_BusinessDayConstIterator(
                      const PackedCalendar_BusinessDayConstIterator& original);
        // Create an iterator having the value of the specified 'original'
        // iterator.

    ~PackedCalendar_BusinessDayConstIterator();
        // Destroy this object.

    // MANIPULATORS
    PackedCalendar_BusinessDayConstIterator& operator=(
                           const PackedCalendar_BusinessDayConstIterator& rhs);
        // Assign to this iterator the value of the specified 'rhs' iterator,
        // and return a reference to this modifiable iterator.

    PackedCalendar_BusinessDayConstIterator& operator++();
        // Advance this iterator to refer to the next business day in the
        // calendar and return a reference to this modifiable object.  The
        // behavior is undefined unless, on entry, this iterator references a
        // valid business day.

    PackedCalendar_BusinessDayConstIterator& operator--();
        // Regress this iterator to refer to the previous business day in the
        // calendar, and return a reference to this modifiable object.  The
        // behavior is undefined unless, on entry, this iterator references a
        // valid business day.

    // ACCESSORS
    PackedCalendar_DateRef operator*() const;
        // Return a 'PackedCalendar_DateRef' object which contains the
        // date value of the business day referenced by this iterator.

    PackedCalendar_IteratorDateProxy operator->() const;
        // Return a date value proxy for the current business day.
};

// FREE OPERATORS
inline
bool operator==(const PackedCalendar_BusinessDayConstIterator& lhs,
                const PackedCalendar_BusinessDayConstIterator& rhs);
    // Return 'true' if the specified 'lhs' and 'rhs' iterators point to the
    // same entry in the same calendar, and 'false' otherwise.  The behavior is
    // undefined unless 'lhs' and 'rhs' both iterate over the same calendar.

inline
bool operator!=(const PackedCalendar_BusinessDayConstIterator& lhs,
                const PackedCalendar_BusinessDayConstIterator& rhs);
    // Return 'true' if the specified 'lhs' and 'rhs' iterators do not point to
    // the same entry in the same calendar, and 'false' otherwise.  The
    // behavior is undefined unless 'lhs' and 'rhs' both iterate over the same
    // calendar.

inline
PackedCalendar_BusinessDayConstIterator operator++(
                       PackedCalendar_BusinessDayConstIterator& iterator, int);
    // Advance the specified 'iterator' to refer to the next business day in
    // the calendar, and return the previous value of 'iterator'.  The behavior
    // is undefined unless, on entry, 'iterator' references a valid business
    // day.

inline
PackedCalendar_BusinessDayConstIterator operator--(
                       PackedCalendar_BusinessDayConstIterator& iterator, int);
    // Regress the specified 'iterator' to refer to the previous business day
    // in the calendar, and return the previous value of 'iterator'.  The
    // behavior is undefined unless, on entry, 'iterator' references a valid
    // business day.

// ============================================================================
//                      INLINE FUNCTION DEFINITIONS
// ============================================================================

                 // --------------------------------------
                 // class PackedCalendar_IteratorDateProxy
                 // --------------------------------------

// CREATORS
inline
PackedCalendar_IteratorDateProxy::PackedCalendar_IteratorDateProxy(
                                                              const Date& date)
: d_date(date)
{
}

inline
PackedCalendar_IteratorDateProxy::~PackedCalendar_IteratorDateProxy()
{
}

inline
PackedCalendar_IteratorDateProxy::PackedCalendar_IteratorDateProxy(
                              const PackedCalendar_IteratorDateProxy& original)
: d_date(original.d_date)
{
}

// ACCESSORS
inline
const Date *PackedCalendar_IteratorDateProxy::operator->() const
{
    return &d_date;
}

                        // ----------------------------
                        // class PackedCalendar_DateRef
                        // ----------------------------

// CREATORS
inline
PackedCalendar_DateRef::PackedCalendar_DateRef(const Date& date)
: Date(date)
{
}

inline
PackedCalendar_DateRef::PackedCalendar_DateRef(
                                        const PackedCalendar_DateRef& original)
: Date(original)
{
}

inline
PackedCalendar_DateRef::~PackedCalendar_DateRef()
{
}

// ACCESSORS
inline
PackedCalendar_IteratorDateProxy PackedCalendar_DateRef::operator&() const
{
    return *this;
}

                    // -----------------------------------------
                    // class PackedCalendar_HolidayConstIterator
                    // -----------------------------------------

// CREATORS
inline
PackedCalendar_HolidayConstIterator::
          PackedCalendar_HolidayConstIterator(const OffsetsConstIterator& iter,
                                              const Date            firstDate)
: d_iterator(iter)
, d_firstDate(firstDate)
{
}

inline
PackedCalendar_HolidayConstIterator::PackedCalendar_HolidayConstIterator(
                           const PackedCalendar_HolidayConstIterator& original)
: d_iterator(original.d_iterator)
, d_firstDate(original.d_firstDate)
{
}

inline
PackedCalendar_HolidayConstIterator::~PackedCalendar_HolidayConstIterator()
{
}

// MANIPULATORS
inline
PackedCalendar_HolidayConstIterator&PackedCalendar_HolidayConstIterator::
                      operator=(const PackedCalendar_HolidayConstIterator& rhs)
{
    d_iterator  = rhs.d_iterator;
    d_firstDate = rhs.d_firstDate;
    return *this;
}

inline
PackedCalendar_HolidayConstIterator&
                              PackedCalendar_HolidayConstIterator::operator++()
{
    ++d_iterator;
    return *this;
}

inline
PackedCalendar_HolidayConstIterator&
                              PackedCalendar_HolidayConstIterator::operator--()
{
    --d_iterator;
    return *this;
}

inline
PackedCalendar_HolidayConstIterator
                 operator++(PackedCalendar_HolidayConstIterator& iterator, int)
{
    PackedCalendar_HolidayConstIterator tmp(iterator);
    ++iterator;
    return tmp;
}

inline
PackedCalendar_HolidayConstIterator
                 operator--(PackedCalendar_HolidayConstIterator& iterator, int)
{
    PackedCalendar_HolidayConstIterator tmp(iterator);
    --iterator;
    return tmp;
}

// ACCESSORS
inline
PackedCalendar_DateRef PackedCalendar_HolidayConstIterator::operator*() const
{
    return PackedCalendar_DateRef(d_firstDate + *d_iterator);
}

inline
PackedCalendar_IteratorDateProxy
                        PackedCalendar_HolidayConstIterator::operator->() const
{
    return PackedCalendar_IteratorDateProxy(this->operator*());
}

// FREE OPERATORS
inline
bool operator==(const PackedCalendar_HolidayConstIterator& lhs,
                const PackedCalendar_HolidayConstIterator& rhs)
{
    return lhs.d_iterator == rhs.d_iterator;
}

inline
bool operator!=(const PackedCalendar_HolidayConstIterator& lhs,
                const PackedCalendar_HolidayConstIterator& rhs)
{
    return lhs.d_iterator != rhs.d_iterator;
}

                // ---------------------------------------------
                // class PackedCalendar_HolidayCodeConstIterator
                // ---------------------------------------------

// CREATORS
inline
PackedCalendar_HolidayCodeConstIterator::
        PackedCalendar_HolidayCodeConstIterator(const CodesConstIterator& iter)
: d_iterator(iter)
{
}

inline
PackedCalendar_HolidayCodeConstIterator::
PackedCalendar_HolidayCodeConstIterator(
                       const PackedCalendar_HolidayCodeConstIterator& original)
: d_iterator(original.d_iterator)
{
}

inline
PackedCalendar_HolidayCodeConstIterator::
                                     ~PackedCalendar_HolidayCodeConstIterator()
{
}

// MANIPULATORS
inline
PackedCalendar_HolidayCodeConstIterator&
PackedCalendar_HolidayCodeConstIterator::
                  operator=(const PackedCalendar_HolidayCodeConstIterator& rhs)
{
    d_iterator = rhs.d_iterator;
    return *this;
}

inline
PackedCalendar_HolidayCodeConstIterator&
                          PackedCalendar_HolidayCodeConstIterator::operator++()
{
    ++d_iterator;
    return *this;
}

inline
PackedCalendar_HolidayCodeConstIterator&
                          PackedCalendar_HolidayCodeConstIterator::operator--()
{
    --d_iterator;
    return *this;
}

inline
PackedCalendar_HolidayCodeConstIterator
             operator++(PackedCalendar_HolidayCodeConstIterator& iterator, int)
{
    bdlt::PackedCalendar_HolidayCodeConstIterator tmp(iterator);
    ++iterator;
    return tmp;
}

inline
PackedCalendar_HolidayCodeConstIterator
             operator--(PackedCalendar_HolidayCodeConstIterator& iterator, int)
{
    PackedCalendar_HolidayCodeConstIterator tmp(iterator);
    --iterator;
    return tmp;
}

// ACCESSORS
inline
int PackedCalendar_HolidayCodeConstIterator::operator*() const
{
    return *d_iterator;
}

// FREE OPERATORS
inline
bool operator==(const PackedCalendar_HolidayCodeConstIterator& lhs,
                const PackedCalendar_HolidayCodeConstIterator& rhs)
{
    return lhs.d_iterator == rhs.d_iterator;
}

inline
bool operator!=(const PackedCalendar_HolidayCodeConstIterator& lhs,
                const PackedCalendar_HolidayCodeConstIterator& rhs)
{
    return lhs.d_iterator != rhs.d_iterator;
}

inline
bsl::ptrdiff_t operator-(const PackedCalendar_HolidayCodeConstIterator& lhs,
                         const PackedCalendar_HolidayCodeConstIterator& rhs)
{
    return lhs.d_iterator - rhs.d_iterator;
}

               // ---------------------------------------------
               // class PackedCalendar_BusinessDayConstIterator
               // ---------------------------------------------

// CREATORS
inline
PackedCalendar_BusinessDayConstIterator::
                                     ~PackedCalendar_BusinessDayConstIterator()
{
}

inline
PackedCalendar_BusinessDayConstIterator::
PackedCalendar_BusinessDayConstIterator(
                       const PackedCalendar_BusinessDayConstIterator& original)
: d_offsetIter(original.d_offsetIter)
, d_calendar_p(original.d_calendar_p)
, d_currentOffset(original.d_currentOffset)
, d_endFlag(original.d_endFlag)
{
}

// MANIPULATORS
inline
PackedCalendar_BusinessDayConstIterator&
                          PackedCalendar_BusinessDayConstIterator::operator++()
{
    nextBusinessDay();
    return *this;
}

inline
PackedCalendar_BusinessDayConstIterator&
                          PackedCalendar_BusinessDayConstIterator::operator--()
{
    previousBusinessDay();
    return *this;
}

inline
PackedCalendar_BusinessDayConstIterator operator++(
                        PackedCalendar_BusinessDayConstIterator& iterator, int)
{
    PackedCalendar_BusinessDayConstIterator tmp(iterator);
    ++iterator;
    return tmp;
}

inline
PackedCalendar_BusinessDayConstIterator operator--(
                        PackedCalendar_BusinessDayConstIterator& iterator, int)
{
    PackedCalendar_BusinessDayConstIterator tmp(iterator);
    --iterator;
    return tmp;
}

// ACCESSORS
inline
PackedCalendar_DateRef
                     PackedCalendar_BusinessDayConstIterator::operator*() const
{
    return PackedCalendar_DateRef(d_calendar_p->firstDate() + d_currentOffset);
}

inline
PackedCalendar_IteratorDateProxy
                    PackedCalendar_BusinessDayConstIterator::operator->() const
{
    return PackedCalendar_IteratorDateProxy(this->operator*());
}

// FREE OPERATORS
inline
bool
operator==(const PackedCalendar_BusinessDayConstIterator& lhs,
           const PackedCalendar_BusinessDayConstIterator& rhs)
{
    return lhs.d_calendar_p     == rhs.d_calendar_p
        && lhs.d_endFlag        == rhs.d_endFlag
        && (lhs.d_currentOffset == rhs.d_currentOffset
         || lhs.d_endFlag == true);
}

inline
bool
operator!=(const PackedCalendar_BusinessDayConstIterator& lhs,
           const PackedCalendar_BusinessDayConstIterator& rhs)
{
    return lhs.d_calendar_p != rhs.d_calendar_p
        || lhs.d_endFlag    != rhs.d_endFlag
        || (lhs.d_endFlag   == false
         && lhs.d_currentOffset != rhs.d_currentOffset);
}

                         // --------------------
                         // class PackedCalendar
                         // --------------------

                            // -----------------
                            // Level-0 Functions
                            // -----------------

// ACCESSORS
inline
bool PackedCalendar::isInRange(const Date& date) const
{
    return d_firstDate <= date && date <= d_lastDate;
}

// CLASS METHODS
inline
int PackedCalendar::maxSupportedBdexVersion()
{
    return 1;
}

// PRIVATE MANIPULATORS
inline
PackedCalendar::CodesConstIterator
            PackedCalendar::beginHolidayCodes(const OffsetsConstIterator& iter)
{
    const int indexOffset = static_cast<int>(iter - d_holidayOffsets.begin());
    const int codeOffset  = d_holidayCodesIndex[indexOffset];
    return d_holidayCodes.begin() + codeOffset;
}

inline
PackedCalendar::CodesConstIterator
              PackedCalendar::endHolidayCodes(const OffsetsConstIterator& iter)
{
    // Use 'OffsetsSizeType' instead of 'int' to avoid a gcc warning.

    const OffsetsSizeType endIndexOffset = iter - d_holidayOffsets.begin() + 1;

    const int iterIndex = endIndexOffset == d_holidayCodesIndex.length()
                          ? static_cast<int>(d_holidayCodes.length())
                          : d_holidayCodesIndex[endIndexOffset];
    return d_holidayCodes.begin() + iterIndex;
}

// PRIVATE ACCESSORS
inline
PackedCalendar::CodesConstIterator
      PackedCalendar::beginHolidayCodes(const OffsetsConstIterator& iter) const
{
    const int indexOffset = static_cast<int>(iter - d_holidayOffsets.begin());
    const int codeOffset  = d_holidayCodesIndex[indexOffset];
    return d_holidayCodes.begin() + codeOffset;
}

inline
PackedCalendar::CodesConstIterator
        PackedCalendar::endHolidayCodes(const OffsetsConstIterator& iter) const
{
    // Use 'OffsetsSizeType' instead of 'int' to avoid a gcc warning.

    const OffsetsSizeType endIndexOffset = iter - d_holidayOffsets.begin() + 1;

    const int iterIndex = endIndexOffset == d_holidayCodesIndex.length()
                          ? static_cast<int>(d_holidayCodes.length())
                          : d_holidayCodesIndex[endIndexOffset];
    return d_holidayCodes.begin() + iterIndex;
}

// MANIPULATORS
template <class STREAM>
STREAM& PackedCalendar::bdexStreamIn(STREAM& stream, int version)
{
    /*
    if (stream) {
        switch (version) {  // Switch on the schema version (starting with 1).
          case 2: {
            PackedCalendar inCal(d_allocator_p);
            inCal.d_firstDate.bdexStreamIn(stream, 1);
            if (!stream) {
                return stream;
            }

            inCal.d_lastDate.bdexStreamIn(stream, 1);
            if (!stream ||
                   (inCal.d_firstDate > inCal.d_lastDate
                 && (   inCal.d_firstDate != Date(9999,12,31)
                     || inCal.d_lastDate  != Date(1,1,1)))) {
                stream.invalidate();
                return stream;
            }
            int length = inCal.d_lastDate - inCal.d_firstDate + 1;

            int transitionsLength;
            stream.getLength(transitionsLength);
            if (!stream || transitionsLength < 0)
            {
                stream.invalidate();
                return stream;
            }

            int offsetsLength;
            stream.getLength(offsetsLength);
            if (   !stream
                ||    (inCal.d_firstDate >= inCal.d_lastDate
                   && offsetsLength != 0)
                ||    (inCal.d_firstDate <  inCal.d_lastDate
                   && (offsetsLength < 0 || offsetsLength > length))) {
                stream.invalidate();
                return stream;
            }
            BSLS_ASSERT_SAFE(offsetsLength >= 0);

            int codesLength;
            stream.getLength(codesLength);
            if (!stream || (0 == offsetsLength && codesLength != 0)) {
                stream.invalidate();
                return stream;
            }

            inCal.d_weekendDaysTransitions.resize(transitionsLength);

            // 'd_holidayCodesIndex' and 'd_holidayOffsets' have the same size.

            inCal.d_holidayOffsets.resize(offsetsLength);
            inCal.d_holidayCodesIndex.resize(offsetsLength);
            inCal.d_holidayCodes.resize(codesLength);

            for (WeekendDaysTransitionSequence::iterator it =
                     inCal.d_weekendDaysTransitions.begin();
                 it != inCal.d_weekendDaysTransitions.end();
                 ++it) {
                it->first.bdexStreamIn(stream, 1);
                if (!stream) {
                    return stream;
                }

                if (it != inCal.d_weekendDaysTransitions.begin() &&
                    it->first <= (it - 1)->first) {
                    stream.invalidate();
                    return stream;
                }

                it->second.bdexStreamIn(stream, 1);
                if (!stream) {
                    return stream;
                }
            }

            int previousValue = -1;
            int i = 0;
            for (OffsetsConstIterator it = inCal.d_holidayOffsets.begin();
                 i < offsetsLength; ++i, ++it) {
                stream.getInt32(*it);
                if (   !stream || *it < 0 || *it >= length
                       || *it <= previousValue) {
                    stream.invalidate();
                    return stream;
                }
                previousValue = *it;
            }

            previousValue = -1;
            i = 0;
            for (CodesIndexConstIterator it =
                                             inCal.d_holidayCodesIndex.begin();
                 i < offsetsLength;
                 ++i, ++it) {
                stream.getInt32(*it);

                // This vector is ordered but duplicates are allowed.  The
                // first element must be 0.

                if (!stream || *it < 0 || *it < previousValue
                    || *it > codesLength  || (0 == i && 0 != *it)) {

                    // If we get here, some of the code indices could
                    // potentially be greater than 'codesLength'.  That would
                    // trigger an assertion in the destructor.  So call
                    // 'removeAll' to clean up.

                    inCal.removeAll();
                    stream.invalidate();
                    return stream;
                }
                previousValue = *it;
            }

            CodesIndexConstIterator it = inCal.d_holidayCodesIndex.begin();
            CodesIndexConstIterator end = inCal.d_holidayCodesIndex.end();

            // Skip the holidays that have no codes.

            while (it != end && *it == 0) {
                ++it;
            }

            // 'it' is now positioned at the first holiday with one or more
            // codes or at the end.

            bool previousValueFlag = false; // This flag will be used
                                            // to determine if we
                                            // are inside an ordered
                                            // sequence of codes
                                            // (i.e 'previousValue'
                                            // refers to a code
                                            // for the same holiday
                                            // as 'value').

            i = 0;
            for (CodesConstIterator jt = inCal.d_holidayCodes.begin();
                    i < codesLength; ++i, ++jt) {
                stream.getInt32(*jt);
                if (   !stream
                    || (previousValueFlag && *jt <= previousValue)) {
                    stream.invalidate();
                    return stream;
                }

                // Regardless of whether or not there is more data, advance the
                // index iterator as needed and update 'previousValueFlag' if
                // 'it' moves.

                if (it != end && i == (*it - 1)) {
                    previousValueFlag = false;

                    while (it != end && i == (*it - 1)) {
                        ++it; // Skip the holidays that have no codes.
                    }
                }
                else {
                    previousValueFlag = true;
                }
                previousValue = *jt;
            }
            BSLS_ASSERT_SAFE(it == end);

            swap(inCal);  // This cannot throw.
          } break;
          case 1: {
            PackedCalendar inCal(d_allocator_p);
            inCal.d_firstDate.bdexStreamIn(stream, 1);
            if (!stream) {
                return stream;
            }

            inCal.d_lastDate.bdexStreamIn(stream, 1);
            if (!stream ||
                   (inCal.d_firstDate > inCal.d_lastDate
                 && (   inCal.d_firstDate != Date(9999,12,31)
                     || inCal.d_lastDate  != Date(1,1,1)))) {
                stream.invalidate();
                return stream;
            }
            int length = inCal.d_lastDate - inCal.d_firstDate + 1;


            DayOfWeekSet weekendDays;
            weekendDays.bdexStreamIn(stream, 1);
            if (!stream) {
                return stream;
            }

            if (weekendDays.length() > 0) {
                inCal.addWeekendDays(weekendDays);
            }

            int offsetsLength;
            stream.getLength(offsetsLength);
            if (   !stream
                ||    (inCal.d_firstDate >= inCal.d_lastDate
                   && offsetsLength != 0)
                ||    (inCal.d_firstDate <  inCal.d_lastDate
                   && (offsetsLength < 0 || offsetsLength > length))) {
                stream.invalidate();
                return stream;
            }
            BSLS_ASSERT_SAFE(offsetsLength >= 0);

            int codesLength;
            stream.getLength(codesLength);
            if (!stream || (0 == offsetsLength && codesLength != 0)) {
                stream.invalidate();
                return stream;
            }

            // 'd_holidayCodesIndex' and 'd_holidayOffsets' have the same size.

            inCal.d_holidayOffsets.resize(offsetsLength);
            inCal.d_holidayCodesIndex.resize(offsetsLength);
            inCal.d_holidayCodes.resize(codesLength);

            int previousValue = -1;
            int i = 0;
            for (OffsetsConstIterator it = inCal.d_holidayOffsets.begin();
                    i < offsetsLength; ++i, ++it) {
                stream.getInt32(*it);
                if (   !stream || *it < 0 || *it >= length
                    || *it <= previousValue) {
                    stream.invalidate();
                    return stream;
                }
                previousValue = *it;
            }

            previousValue = -1;
            i = 0;
            for (CodesIndexConstIterator it =
                                             inCal.d_holidayCodesIndex.begin();
                    i < offsetsLength;
                    ++i, ++it) {
                stream.getInt32(*it);

                // This vector is ordered but duplicates are allowed.  The
                // first element must be 0.

                if (!stream || *it < 0 || *it < previousValue
                 || *it > codesLength  || (0 == i && 0 != *it)) {

                    // If we get here, some of the code indices could
                    // potentially be greater than 'codesLength'.  That would
                    // trigger an assertion in the destructor.  So call
                    // 'removeAll' to clean up.

                    inCal.removeAll();
                    stream.invalidate();
                    return stream;
                }
                previousValue = *it;
            }

            CodesIndexConstIterator it = inCal.d_holidayCodesIndex.begin();
            CodesIndexConstIterator end = inCal.d_holidayCodesIndex.end();

            // Skip the holidays that have no codes.

            while (it != end && *it == 0) {
                ++it;
            }

            // 'it' is now positioned at the first holiday with one or more
            // codes or at the end.

            bool previousValueFlag = false; // This flag will be used
                                            // to determine if we
                                            // are inside an ordered
                                            // sequence of codes
                                            // (i.e 'previousValue'
                                            // refers to a code
                                            // for the same holiday
                                            // as 'value').

            i = 0;
            for (CodesConstIterator jt = inCal.d_holidayCodes.begin();
                    i < codesLength; ++i, ++jt) {
                stream.getInt32(*jt);
                if (   !stream
                    || (previousValueFlag && *jt <= previousValue)) {
                    stream.invalidate();
                    return stream;
                }

                // Regardless of whether or not there is more data, advance the
                // index iterator as needed and update 'previousValueFlag' if
                // 'it' moves.

                if (it != end && i == (*it - 1)) {
                    previousValueFlag = false;

                    while (it != end && i == (*it - 1)) {
                        ++it; // Skip the holidays that have no codes.
                    }
                }
                else {
                    previousValueFlag = true;
                }
                previousValue = *jt;
            }
            BSLS_ASSERT_SAFE(it == end);

            swap(inCal);  // This cannot throw.
          } break;
          default: {
            stream.invalidate();
          }
        }
    }
    */
    return stream;
}

inline
void PackedCalendar::reserveHolidayCapacity(int numHolidays)
{
    BSLS_ASSERT_SAFE(0 <= numHolidays);

    d_holidayOffsets.reserveCapacity(numHolidays);
}

inline
void PackedCalendar::reserveHolidayCodeCapacity(int numHolidayCodes)
{
    BSLS_ASSERT_SAFE(0 <= numHolidayCodes);

    d_holidayCodes.reserveCapacity(numHolidayCodes);
}

// ACCESSORS
template <class STREAM>
STREAM& PackedCalendar::bdexStreamOut(STREAM& stream, int version) const
{
    switch (version) {  // Switch on the schema version (starting with 1).
      case 2: {
        d_firstDate.bdexStreamOut(stream, 1);
        d_lastDate.bdexStreamOut(stream, 1);

        stream.putLength(static_cast<int>(d_weekendDaysTransitions.size()));
        stream.putLength(static_cast<int>(d_holidayOffsets.length()));
        stream.putLength(static_cast<int>(d_holidayCodes.length()));

        for (WeekendDaysTransitionSequence::size_type i = 0;
             i < d_weekendDaysTransitions.size();
             ++i) {
            d_weekendDaysTransitions[i].first.bdexStreamOut(stream, 1);
            d_weekendDaysTransitions[i].second.bdexStreamOut(stream, 1);
        }

        for (OffsetsSizeType i = 0; i < d_holidayOffsets.length(); ++i) {
            stream.putInt32(d_holidayOffsets[i]);
        }
        for (CodesIndexSizeType i = 0; i < d_holidayCodesIndex.length(); ++i) {
            stream.putInt32(d_holidayCodesIndex[i]);
        }
        for (CodesSizeType i = 0; i < d_holidayCodes.length(); ++i) {
            stream.putInt32(d_holidayCodes[i]);
        }
      } break;
      case 1: {
        d_firstDate.bdexStreamOut(stream, 1);
        d_lastDate.bdexStreamOut(stream, 1);

        if (!d_weekendDaysTransitions.empty() &&
            d_weekendDaysTransitions[0].first == Date(1, 1, 1)) {
            d_weekendDaysTransitions[0].second.bdexStreamOut(stream, 1);
        }
        else {
            DayOfWeekSet tempSet;
            tempSet.bdexStreamOut(stream, 1);
        }

        stream.putLength(static_cast<int>(d_holidayOffsets.length()));
        stream.putLength(static_cast<int>(d_holidayCodes.length()));

        for (OffsetsSizeType i = 0; i < d_holidayOffsets.length(); ++i) {
            stream.putInt32(d_holidayOffsets[i]);
        }
        for (CodesIndexSizeType i = 0; i < d_holidayCodesIndex.length(); ++i) {
            stream.putInt32(d_holidayCodesIndex[i]);
        }
        for (CodesSizeType i = 0; i < d_holidayCodes.length(); ++i) {
            stream.putInt32(d_holidayCodes[i]);
        }
      } break;
      default: {
        stream.invalidate();
      }

    }
    return stream;
}

inline
PackedCalendar::WeekendDaysTransitionConstIterator
                            PackedCalendar::beginWeekendDaysTransitions() const
{
    return d_weekendDaysTransitions.begin();
}

inline
PackedCalendar::WeekendDaysTransitionConstIterator
                              PackedCalendar::endWeekendDaysTransitions() const
{
    return d_weekendDaysTransitions.end();
}

inline
PackedCalendar::BusinessDayConstIterator
                                      PackedCalendar::beginBusinessDays() const
{
    return BusinessDayConstIterator(*this, d_firstDate, false);
}

inline
PackedCalendar::BusinessDayConstIterator
                      PackedCalendar::beginBusinessDays(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    return BusinessDayConstIterator(*this, date, false);
}

inline
PackedCalendar::HolidayCodeConstIterator
      PackedCalendar::beginHolidayCodes(const HolidayConstIterator& iter) const
{
    return HolidayCodeConstIterator(beginHolidayCodes(iter.d_iterator));
}

inline
int PackedCalendar::numWeekendDaysTransitions() const
{
    return d_weekendDaysTransitions.size();
}

inline
PackedCalendar::HolidayConstIterator PackedCalendar::beginHolidays() const
{
    return HolidayConstIterator(d_holidayOffsets.begin(), d_firstDate);
}

inline
PackedCalendar::HolidayConstIterator
                          PackedCalendar::beginHolidays(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    return beginHolidaysRaw(date);
}

inline
PackedCalendar::HolidayConstIterator
                       PackedCalendar::beginHolidaysRaw(const Date& date) const
{
    OffsetsConstIterator i = bdlc::PackedIntArrayUtil::lower_bound(
                                                      d_holidayOffsets.begin(),
                                                      d_holidayOffsets.end(),
                                                      date - d_firstDate);
    return HolidayConstIterator(i, d_firstDate);
}

inline
PackedCalendar::BusinessDayConstIterator
                                        PackedCalendar::endBusinessDays() const
{
    return BusinessDayConstIterator(*this, d_lastDate, true);
}

inline
PackedCalendar::BusinessDayConstIterator
                        PackedCalendar::endBusinessDays(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    return BusinessDayConstIterator(*this, date, true);
}

inline
PackedCalendar::HolidayCodeConstIterator
        PackedCalendar::endHolidayCodes(const HolidayConstIterator& iter) const
{
    return endHolidayCodes(iter.d_iterator);
}

inline
PackedCalendar::HolidayConstIterator PackedCalendar::endHolidays() const
{
    return HolidayConstIterator(d_holidayOffsets.end(), d_firstDate);
}

inline
PackedCalendar::HolidayConstIterator
                            PackedCalendar::endHolidays(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    return endHolidaysRaw(date);
}

inline
PackedCalendar::HolidayConstIterator
                         PackedCalendar::endHolidaysRaw(const Date& date) const
{
    OffsetsConstIterator i = bdlc::PackedIntArrayUtil::lower_bound(
                                                      d_holidayOffsets.begin(),
                                                      d_holidayOffsets.end(),
                                                      date - d_firstDate + 1);
    return HolidayConstIterator(i, d_firstDate);
}

inline
const Date& PackedCalendar::firstDate() const
{
    return d_firstDate;
}

inline
bool PackedCalendar::isBusinessDay(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    return !isNonBusinessDay(date);
}

inline
bool PackedCalendar::isHoliday(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    const int offset = date - d_firstDate;
    const OffsetsConstIterator offsetEnd = d_holidayOffsets.end();
    const OffsetsConstIterator i = bdlc::PackedIntArrayUtil::lower_bound(
                                                      d_holidayOffsets.begin(),
                                                      offsetEnd,
                                                      offset);
    if (i == offsetEnd || *i != offset) {
        return false;
    }
    return true;
}

inline
bool PackedCalendar::isNonBusinessDay(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    return isWeekendDay(date) || isHoliday(date);
}

inline
bool PackedCalendar::isWeekendDay(DayOfWeek::Enum dayOfWeek) const
{
    BSLS_ASSERT_SAFE(d_weekendDaysTransitions.size() <= 1);

    if (d_weekendDaysTransitions.empty()) {
        return false;
    }
    else {
        BSLS_ASSERT_SAFE(d_weekendDaysTransitions[0].first == Date(1,1,1));
        return d_weekendDaysTransitions[0].second.isMember(dayOfWeek);
    }
}

inline
const Date& PackedCalendar::lastDate() const
{
    return d_lastDate;
}

inline
int PackedCalendar::length() const
{
    return d_firstDate <= d_lastDate ? d_lastDate - d_firstDate + 1 : 0;
}

inline
int PackedCalendar::numBusinessDays() const
{
    return length() - numNonBusinessDays();
}

inline
int PackedCalendar::numHolidayCodesTotal() const
{
    return static_cast<int>(d_holidayCodes.length());
}

inline
int PackedCalendar::numHolidays() const
{
    return static_cast<int>(d_holidayOffsets.length());
}

inline
PackedCalendar::BusinessDayConstReverseIterator
                                     PackedCalendar::rbeginBusinessDays() const
{
    return BusinessDayConstReverseIterator(endBusinessDays());
}

inline
PackedCalendar::BusinessDayConstReverseIterator
                     PackedCalendar::rbeginBusinessDays(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    return BusinessDayConstReverseIterator(endBusinessDays(date));
}

inline
PackedCalendar::HolidayCodeConstReverseIterator
                     PackedCalendar::rbeginHolidayCodes(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    return HolidayCodeConstReverseIterator(endHolidayCodes(date));
}

inline
PackedCalendar::HolidayCodeConstReverseIterator
     PackedCalendar::rbeginHolidayCodes(const HolidayConstIterator& iter) const
{
    return HolidayCodeConstReverseIterator(endHolidayCodes(iter));
}

inline
PackedCalendar::HolidayConstReverseIterator
                                         PackedCalendar::rbeginHolidays() const
{
    return HolidayConstReverseIterator(endHolidays());
}

inline
PackedCalendar::HolidayConstReverseIterator
                         PackedCalendar::rbeginHolidays(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    return HolidayConstReverseIterator(endHolidays(date));
}

inline
PackedCalendar::BusinessDayConstReverseIterator
                                       PackedCalendar::rendBusinessDays() const
{
    return BusinessDayConstReverseIterator(beginBusinessDays());
}

inline
PackedCalendar::BusinessDayConstReverseIterator
                       PackedCalendar::rendBusinessDays(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    return BusinessDayConstReverseIterator(beginBusinessDays(date));
}

inline
PackedCalendar::HolidayCodeConstReverseIterator
                       PackedCalendar::rendHolidayCodes(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    return HolidayCodeConstReverseIterator(beginHolidayCodes(date));
}

inline
PackedCalendar::HolidayCodeConstReverseIterator
       PackedCalendar::rendHolidayCodes(const HolidayConstIterator& iter) const
{
    return HolidayCodeConstReverseIterator(beginHolidayCodes(iter));
}

inline
PackedCalendar::HolidayConstReverseIterator
                                           PackedCalendar::rendHolidays() const
{
    return HolidayConstReverseIterator(beginHolidays());
}

inline
PackedCalendar::HolidayConstReverseIterator
                           PackedCalendar::rendHolidays(const Date& date) const
{
    BSLS_ASSERT_SAFE(isInRange(date));

    return HolidayConstReverseIterator(beginHolidays(date));
}

// FREE OPERATORS
inline
bool operator!=(const PackedCalendar& lhs, const PackedCalendar& rhs)
{
    return !(lhs == rhs);
}

// FREE FUNCTIONS
inline
void swap(PackedCalendar& a, PackedCalendar& b)
{
    a.swap(b);
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
