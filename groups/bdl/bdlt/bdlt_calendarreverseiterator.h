// bdlt_calendarreverseiterator.h                                     -*-C++-*-
#ifndef INCLUDED_BDLT_CALENDARREVERSEITERATOR
#define INCLUDED_BDLT_CALENDARREVERSEITERATOR

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide reverse iterator template for calendar iterators.
//
//@CLASSES:
//  bdlt::CalendarReverseIterator: template for calendar reverse iterators
//
//@SEE_ALSO: bdlt_calendar, bdlt_packedcalendar
//
//@DESCRIPTION: This component provides a template,
// 'bdlt::CalendarReverseIterator', that can be used to adapt a calendar
// iterator to be a reverse-iterator (see 'bdlt_calendar' and
// 'bdlt_packedcalendar').  Calendar iterators can not return a reference to an
// underlying element of the calendar and hence can not be used with
// 'bsl::reverse_iterator'.  The reverse iterator template defined in this
// component provides a subset of the 'bsl::reverse_iterator' interface that
// can be used with the calendar iterators defined in 'bdlt'.
//
///Limitation
///----------
// This is *not* a fully compliant implementation of 'std::reverse_iterator'
// according to the C++ standard.  It is an implementation of the minimum
// functionality needed to support the public iterators in the 'bdlt_calendar'
// and 'bdlt_packedcalendar' components.  Within that limitation, it is a
// subset implementation of 'bsl::reverse_iterator'.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Basic Use of 'bdlt::CalendarReverseIterator'
/// - - - - - - - - - - - - - - - - - - - - - - - - - - -
// The following shows the intended usage of this component.
//
// First, we define a bidirectional iterator class:
//..
//  template <class TYPE>
//  class Iterator {
//      // This 'class' basically behaves as a pointer to the parametrized
//      // 'TYPE' with 5 'trait' types defines.  Note that it supports only a
//      // subset of the functionality that a pointer would -- this subset
//      // covers all the functionality that a 'bdlt_CalendarReverseIterator'
//      // needs.
//      //
//      // The would have been implemented by inheriting from a pointer type
//      // and additing the 5 typedefs, but C++ doesn't allow inheritance from
//      // pointer types.
//
//      TYPE *d_value;
//
//      template <class OTHER>
//      friend bool operator==(const Iterator<OTHER>&, const Iterator<OTHER>&);
//
//    public:
//      // PUBLIC TYPES
//      typedef bsl::bidirectional_iterator_tag iterator_category;
//      typedef TYPE                            value_type;
//      typedef int                             difference_type;
//      typedef TYPE *                          pointer;
//      typedef TYPE&                           reference;
//
//      // CREATORS
//      Iterator() : d_value(0)
//          // Construct a 'Iterator' object with the default value.
//      {
//      }
//
//      Iterator(TYPE *value) : d_value(value)                 // IMPLICIT
//          // Construct a 'Iterator' object from the specified 'value'.
//      {
//      }
//
//      // Iterator(const Iterator&) = default;
//
//      // ~Iterator() = default;
//
//      // MANIPULATORS
//      // Iterator& operator=(const Iterator&) = default;
//
//      Iterator& operator++()
//          // Increment this object and return a reference to it.
//      {
//          ++d_value;
//          return *this;
//      }
//
//      Iterator& operator--()
//          // Decrement this object and return a reference to it.
//      {
//          --d_value;
//          return *this;
//      }
//
//      // ACCESSORS
//      reference operator*() const
//          // Return a refrence to the item referred to by this object.
//      {
//          return *d_value;
//      }
//
//      pointer operator->() const
//          // Return a pointer to the item referred to by this object.
//      {
//          return d_value;
//      }
//
//      Iterator operator+(bsl::ptrdiff_t offset) const
//          // Return an iterator referencing the location at the specified
//          // 'offset' from the element referenced by this iterator.
//      {
//          return Iterator(d_value + offset);
//      }
//  };
//
//  template <class TYPE>
//  inline
//  bool operator==(const Iterator<TYPE>& lhs,  const Iterator<TYPE>& rhs)
//      // Return 'true' if the specified 'lhs' is equivalent to the specified
//      // 'rhs' and 'false' otherwise.
//  {
//      return lhs.d_value == rhs.d_value;
//  }
//
//  template <class TYPE>
//  inline
//  bool operator!=(const Iterator<TYPE>& lhs, const Iterator<TYPE>& rhs)
//      // Return 'false' if the specified 'lhs' is equivalent to the specified
//      // 'rhs' and 'true' otherwise.
//  {
//      return !(lhs == rhs);
//  }
//..
// Then, we define 'struct' 'S', the type that will be pointed at by the
// 'Iterator' type:
//..
//  struct S {
//      char d_c;
//      int  d_i;
//  };
//..
// Next, we define 4 values for 'S':
//..
//  const S s0 = { 'A', 3 };
//  const S s1 = { 'B', 5 };
//  const S s2 = { 'C', 7 };
//  const S s3 = { 'D', 9 };
//..
// Then we define 's', a range of 'S' values:
//..
//  S s[] = { s0, s1, s2, s3 };
//  enum { NUM_S = sizeof s / sizeof *s };
//..
// Next, we define 'sfBegin', 'sfEnd'
//..
//  Iterator<S> sfBegin(s + 0), sfEnd(s + NUM_S);
//..
// Then, we declare our reverse iterator type
//..
//  typedef bdlt::CalendarReverseIterator<Iterator<S> > Reverse;
//..
// Next, we declare reverse begin and end iterators to our range of 'S's:
//..
//  const Reverse rBegin(sfEnd), rEnd(sfBegin);
//..
// Now, we traverse our range, streaming out the contents of the 'S' values:
//..
//  bsl::ostringstream stream;
//  for (Reverse it = rBegin; rEnd != it; ++it) {
//      stream << (rBegin == it ? "" : ", ")
//             << "{ "
//             << it->d_c
//             << ", "
//             << it->d_i
//             << " }";
//  }
//  stream << bsl::flush;
//..
// Finally, we verify the range output:
//..
//  assert(stream.str() == "{ D, 9 }, { C, 7 }, { B, 5 }, { A, 3 }");
//..

#ifndef INCLUDED_BDLSCM_VERSION
#include <bdlscm_version.h>
#endif

namespace BloombergLP {
namespace bdlt {

                       // =============================
                       // class CalendarReverseIterator
                       // =============================


template <class ITERATOR>
class CalendarReverseIterator {
    // This class template provides a (non-conforming) subset of
    // 'std::reverse_iterator' interface that can be implemented with respect
    // to calendars defined in the 'bdlt' package.

    typedef ITERATOR iterator;

    iterator d_forwardIter;

  public:
    // PUBLIC TYPES
    typedef typename iterator::iterator_category iterator_category;
    typedef typename iterator::value_type        value_type;
    typedef typename iterator::difference_type   difference_type;
    typedef typename iterator::pointer           pointer;
    typedef typename iterator::reference         reference;

    // CREATORS
    CalendarReverseIterator();
        // Default construct a reverse iterator.  The contained forward
        // iterator will be default constructed.

    explicit
    CalendarReverseIterator(const iterator& value);
        // Construct a reverse iterator pointing at the item that precedes, in
        // the forward sequence (or that follows, in the backward sequence) the
        // item referred to by the specified 'value'.

    // CalendarReverseIterator(const CalendarReverseIterator&) = default;

    // ~CalendarReverseIterator() = default;

    // MANIPULATORS
    CalendarReverseIterator& operator=(const CalendarReverseIterator& rhs);
        // Assign the specified 'rhs' to this object, and return a reference
        // to this object.

    CalendarReverseIterator& operator=(const iterator& rhs);
        // Set the value of this object to refer to the item in the forward
        // sequence before the item referred to by the specified 'rhs'.

    CalendarReverseIterator& operator++();
        // Increment to the next element in the reverse iteration sequence and
        // return a reference providing modifiable access to this reverse
        // iterator.  The behavior is undefined if, on entry, this reverse
        // iterator has the past-the-end value for a reverse iterator over the
        // underlying sequence.

    CalendarReverseIterator  operator++(int);
        // Increment to the next element in the reverse iteration sequence and
        // return a reverse iterator having the pre-increment value of this
        // reverse iterator.  The behavior is undefined if, on entry, this
        // reverse iterator has the past-the-end value for a reverse iterator
        // over the underlying sequence.

    CalendarReverseIterator& operator--();
        // Decrement to the previous element in the reverse iteration sequence
        // and return a reference providing modifiable access to this reverse
        // iterator.  The behavior is undefined if, on entry, this reverse
        // iterator has the same value as a reverse iterator at the start of
        // the underlying sequence.

    CalendarReverseIterator  operator--(int);
        // Decrement to the previous element in the reverse iteration sequence
        // and return a reverse iterator having the pre-decrement value of this
        // reverse iterator.  The behavior is undefined if, on entry, this
        // reverse iterator has the same value as a reverse iterator to the
        // start of the underlying sequence.

    // ACCESSORS
    reference operator*() const;
        // Return a reference to the item referred to by this reverse iterator.
        // The behavior is undefined unless this iterator is within the bounds
        // of the underlying sequence.

    pointer operator->() const;
        // Return a pointer to the item referred to by this reverse iterator.
        // The behavior is undefined unless this iterator is within the bounds
        // of the underlying sequence.

    iterator forwardIterator() const;
        // Return the forward iterator pointing to the element in the forward
        // sequence after the element referred to by this reverse-iterator.
};

// FREE OPERATORS
template <class ITERATOR>
inline
bool operator==(const CalendarReverseIterator<ITERATOR>& lhs,
                const CalendarReverseIterator<ITERATOR>& rhs);
    // Return 'true' if the specified 'lhs' reverse iterator has the same value
    // as the specified 'rhs' reverse iterator, and 'false' otherwise.  Two
    // reverse iterators have the same value if they refer to the same element,
    // or both have the past-the-end value for a reverse iterator over the
    // underlying reverse iteration sequence.  The behavior is undefined unless
    // both reverse iterators refer to the same underlying sequence.

template <class ITERATOR>
inline
bool operator!=(const CalendarReverseIterator<ITERATOR>& lhs,
                const CalendarReverseIterator<ITERATOR>& rhs);
    // Return 'true' if the specified 'lhs' reverse iterator does not have the
    // same value as the specified 'rhs' reverse iterator, and 'false'
    // otherwise.  Two reverse iterators do not have the same value if (1) they
    // do not refer to the same element and (2) both do not have the
    // past-the-end value for a reverse iterator over the underlying reverse
    // iteration sequence.  The behavior is undefined unless both reverse
    // iterators refer to the same underlying sequence.

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

                       // -----------------------------
                       // class CalendarReverseIterator
                       // -----------------------------

// CREATORS
template <class ITERATOR>
inline
CalendarReverseIterator<ITERATOR>::CalendarReverseIterator()
: d_forwardIter()
{
}

template <class ITERATOR>
inline
CalendarReverseIterator<ITERATOR>::CalendarReverseIterator(
                                                         const ITERATOR& value)
: d_forwardIter(value)
{
}

// MANIPULATORS
template <class ITERATOR>
inline
CalendarReverseIterator<ITERATOR>&
CalendarReverseIterator<ITERATOR>::operator=(
                                            const CalendarReverseIterator& rhs)
{
    d_forwardIter = rhs.d_forwardIter;
    return *this;
}

template <class ITERATOR>
inline
CalendarReverseIterator<ITERATOR>&
CalendarReverseIterator<ITERATOR>::operator=(const ITERATOR& rhs)
{
    d_forwardIter = rhs;
    return *this;
}

template <class ITERATOR>
inline
CalendarReverseIterator<ITERATOR>&
CalendarReverseIterator<ITERATOR>::operator++()
{
    --d_forwardIter;
    return *this;
}

template <class ITERATOR>
inline
CalendarReverseIterator<ITERATOR>
CalendarReverseIterator<ITERATOR>::operator++(int)
{
    CalendarReverseIterator tmp = *this;
    --d_forwardIter;
    return tmp;
}

template <class ITERATOR>
inline
CalendarReverseIterator<ITERATOR>&
CalendarReverseIterator<ITERATOR>::operator--()
{
    ++d_forwardIter;
    return *this;
}

template <class ITERATOR>
inline
CalendarReverseIterator<ITERATOR>
CalendarReverseIterator<ITERATOR>::operator--(int)
{
    CalendarReverseIterator tmp = *this;
    ++d_forwardIter;
    return tmp;
}

// ACCESSORS
template <class ITERATOR>
inline
typename CalendarReverseIterator<ITERATOR>::reference
CalendarReverseIterator<ITERATOR>::operator*() const
{
    iterator tmp = d_forwardIter;
    return *--tmp;
}

template <class ITERATOR>
inline
typename CalendarReverseIterator<ITERATOR>::pointer
CalendarReverseIterator<ITERATOR>::operator->() const
{
    iterator tmp = d_forwardIter;
    return &*--tmp;
}

template <class ITERATOR>
inline
ITERATOR CalendarReverseIterator<ITERATOR>::forwardIterator() const
{
    return d_forwardIter;
}

// FREE OPERATORS
template <class ITERATOR>
inline
bool operator==(const CalendarReverseIterator<ITERATOR>& lhs,
                const CalendarReverseIterator<ITERATOR>& rhs)
{
    return lhs.forwardIterator() == rhs.forwardIterator();
}

template <class ITERATOR>
inline
bool operator!=(const CalendarReverseIterator<ITERATOR>& lhs,
                const CalendarReverseIterator<ITERATOR>& rhs)
{
    return lhs.forwardIterator() != rhs.forwardIterator();
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
