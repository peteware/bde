// bdlc_packedintarrayutil.h                                          -*-C++-*-
#ifndef INCLUDED_BDLC_PACKEDINTARRAYUTIL
#define INCLUDED_BDLC_PACKEDINTARRAYUTIL

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide common non-primitive operations on 'bdlc::PackedIntArray'.
//
//@CLASSES:
//  bdlc::PackedIntArrayUtil: namespace for non-primitive operations on
//                            'bdlc::PackedIntArray' objects
//
//@SEE_ALSO: bdlc_packedintarray
//
//@DESCRIPTION: This component provides a 'struct', 'bdlc::PackedIntArrayUtil',
// that serves as a namespace for utility functions that operate on
// 'bdlc::PackedIntArray' objects.
//
// The following list of methods are provided by 'bdlc::PackedIntArrayUtil':
//..
//  'isSorted'         Returns 'true' if the range from a
//                     'bdlc::PackedIntArray' is sorted, and 'false' otherwise.
//
//  'lower_bound'      Returns an iterator to the first element in a sorted
//                     range from a 'bdlc::PackedIntArray' which compares
//                     greater than or equal to a specified value.
//
//  'upper_bound'      Returns an iterator to the first element in a sorted
//                     range from a 'bdlc::PackedIntArray' which compares
//                     greater than a specified value.
//..
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: lower_bound
/// - - - - - - - - - - -
// Suppose that given an 'bdlc::PackedIntArray', we want to find the first
// value greater than or equal to the value 17.  First, create and populate
// with sorted data the 'bdlc::PackedIntArray' to be searched::
//..
//  bdlc::PackedIntArray<int> array;
//
//  array.push_back( 5);
//  array.push_back( 9);
//  array.push_back(15);
//  array.push_back(19);
//  array.push_back(23);
//  array.push_back(36);
//  assert(6 == array.length());
//..
// Finally, use 'bdlc::PackedIntArrayUtil::lower_bound' to find the desired
// value:
//..
//  assert(19 == *bdlc::PackedIntArrayUtil::lower_bound(array.begin(),
//                                                      array.end(),
//                                                      17));
//..

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
    // non-primitive operations on 'bdlc::PackedIntArray'.

  public:
    // CLASS METHODS
    template <class T>
    static bool isSorted(PackedIntArrayConstIterator<T> first,
                         PackedIntArrayConstIterator<T> last);
        // Return 'true' if the range from the specified 'first' (inclusive) to
        // the specified 'last' (exclusive) is sorted, and 'false' otherwise.

    template <class T>
    static PackedIntArrayConstIterator<T> lower_bound(
                                          PackedIntArrayConstIterator<T> first,
                                          PackedIntArrayConstIterator<T> last,
                                          const T& value);
        // Return an iterator to the first element in the sorted range from the
        // specified 'first' (inclusive) to the specified 'last' (exclusive)
        // which compares greater than or equal to the specified 'value'.  The
        // behavior is undefined unless 'first <= last' and the range is
        // sorted.

    template <class T>
    static PackedIntArrayConstIterator<T> upper_bound(
                                          PackedIntArrayConstIterator<T> first,
                                          PackedIntArrayConstIterator<T> last,
                                          const T& value);
        // Return an iterator to the first element in the sorted range from the
        // specified 'first' (inclusive) to the specified 'last' (exclusive)
        // which compares greater than the specified 'value'.  The behavior is
        // undefined unless 'first <= last' and the range is sorted.
};

// ============================================================================
//                            INLINE DEFINITIONS
// ============================================================================

                      // -------------------------
                      // struct PackedIntArrayUtil
                      // -------------------------

// CLASS METHODS
template <class T>
bool PackedIntArrayUtil::isSorted(PackedIntArrayConstIterator<T> first,
                                  PackedIntArrayConstIterator<T> last)
{
    PackedIntArrayConstIterator<T> at   = first;
    PackedIntArrayConstIterator<T> prev = first;

    while (at < last) {
        if (*prev > *at) {
            return false;                                             // RETURN
        }
        prev = at++;
    }

    return true;
}

template <class T>
PackedIntArrayConstIterator<T> PackedIntArrayUtil::lower_bound(
                                          PackedIntArrayConstIterator<T> first,
                                          PackedIntArrayConstIterator<T> last,
                                          const T& value)
{
    BSLS_ASSERT(first <= last);
    BSLS_ASSERT_SAFE(isSorted(first, last));

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
    BSLS_ASSERT_SAFE(isSorted(first, last));

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
