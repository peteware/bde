// bdlt_calendarreverseiterator.t.cpp                                 -*-C++-*-
#include <bdlt_calendarreverseiterator.h>

#include <bdls_testutil.h>

#include <bsl_cstdlib.h>
#include <bsl_sstream.h>

using namespace BloombergLP;
using namespace bsl;

// ============================================================================
//                             TEST PLAN
// ----------------------------------------------------------------------------
//                              Overview
//                              --------
// ----------------------------------------------------------------------------
// [  ] CalendarReverseIterator();
// [ 2] CalendarReverseIterator(const iterator& value);
// [  ] CalendarReverseIterator(const CalendarReverseIterator&);
// [ 2] ~CalendarReverseIterator();
// [  ] CalendarReverseIterator& operator=(const CalendarReverseIterator& rhs);
// [  ] CalendarReverseIterator& operator=(const iterator& rhs);
// [  ] CalendarReverseIterator& operator++();
// [  ] CalendarReverseIterator& operator++(int);
// [  ] CalendarReverseIterator& operator--();
// [  ] CalendarReverseIterator& operator--(int);
// [ 4] reference operator*() const;
// [ 4] pointer operator->() const;
// [ 4] iterator forwardIterator() const;
// [ 6] bool operator==(lhs, rhs);
// [ 6] bool operator!=(lhs, rhs);
// ----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [  ] USAGE EXAMPLE
// [ 3] Reserved for 'gg' generator function.
// [ 5] Reserved for 'print' and 'operator<<' functions.
// [ 8] Reserved for 'swap' testing.
// ----------------------------------------------------------------------------

// ============================================================================
//                     STANDARD BDE ASSERT TEST FUNCTION
// ----------------------------------------------------------------------------

namespace {

int testStatus = 0;

void aSsErT(bool condition, const char *message, int line)
{
    if (condition) {
        cout << "Error " __FILE__ "(" << line << "): " << message
             << "    (failed)" << endl;

        if (0 <= testStatus && testStatus <= 100) {
            ++testStatus;
        }
    }
}

}  // close unnamed namespace

// ============================================================================
//               STANDARD BDE TEST DRIVER MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT       BDLS_TESTUTIL_ASSERT
#define ASSERTV      BDLS_TESTUTIL_ASSERTV

#define LOOP_ASSERT  BDLS_TESTUTIL_LOOP_ASSERT
#define LOOP0_ASSERT BDLS_TESTUTIL_LOOP0_ASSERT
#define LOOP1_ASSERT BDLS_TESTUTIL_LOOP1_ASSERT
#define LOOP2_ASSERT BDLS_TESTUTIL_LOOP2_ASSERT
#define LOOP3_ASSERT BDLS_TESTUTIL_LOOP3_ASSERT
#define LOOP4_ASSERT BDLS_TESTUTIL_LOOP4_ASSERT
#define LOOP5_ASSERT BDLS_TESTUTIL_LOOP5_ASSERT
#define LOOP6_ASSERT BDLS_TESTUTIL_LOOP6_ASSERT

#define Q            BDLS_TESTUTIL_Q   // Quote identifier literally.
#define P            BDLS_TESTUTIL_P   // Print identifier and value.
#define P_           BDLS_TESTUTIL_P_  // P(X) without '\n'.
#define T_           BDLS_TESTUTIL_T_  // Print a tab (w/o newline).
#define L_           BDLS_TESTUTIL_L_  // current Line number

// ============================================================================
//                                 USAGE EXAMPLE
// ----------------------------------------------------------------------------

// Note the following code is used by both the usage example and the rest of
// this test driver.

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
    template <class TYPE>
    class Iterator {
        // This 'class' basically behaves as a pointer to the parametrized
        // 'TYPE' with 5 'trait' types defines.  Note that it supports only a
        // subset of the functionality that a pointer would -- this subset
        // covers all the functionality that a 'bdlt_CalendarReverseIterator'
        // needs.
        //
        // The would have been implemented by inheriting from a pointer type
        // and additing the 5 typedefs, but C++ doesn't allow inheritance from
        // pointer types.

        TYPE *d_value;

        template <class OTHER>
        friend bool operator==(const Iterator<OTHER>&, const Iterator<OTHER>&);

      public:
        // PUBLIC TYPES
        typedef bsl::bidirectional_iterator_tag iterator_category;
        typedef TYPE                            value_type;
        typedef int                             difference_type;
        typedef TYPE *                          pointer;
        typedef TYPE&                           reference;

        // CREATORS
        Iterator() : d_value(0)
            // Construct a 'Iterator' object with the default value.
        {
        }

        Iterator(TYPE *value) : d_value(value)                 // IMPLICIT
            // Construct a 'Iterator' object from the specified 'value'.
        {
        }

        // Iterator(const Iterator&) = default;

        // ~Iterator() = default;

        // MANIPULATORS
        // Iterator& operator=(const Iterator&) = default;

        Iterator& operator++()
            // Increment this object and return a reference to it.
        {
            ++d_value;
            return *this;
        }

        Iterator& operator--()
            // Decrement this object and return a reference to it.
        {
            --d_value;
            return *this;
        }

        // ACCESSORS
        reference operator*() const
            // Return a refrence to the item referred to by this object.
        {
            return *d_value;
        }

        pointer operator->() const
            // Return a pointer to the item referred to by this object.
        {
            return d_value;
        }

        Iterator operator+(bsl::ptrdiff_t offset) const
            // Return an iterator referencing the location at the specified
            // 'offset' from the element referenced by this iterator.
        {
            return Iterator(d_value + offset);
        }
    };

    template <class TYPE>
    inline
    bool operator==(const Iterator<TYPE>& lhs,  const Iterator<TYPE>& rhs)
        // Return 'true' if the specified 'lhs' is equivalent to the specified
        // 'rhs' and 'false' otherwise.
    {
        return lhs.d_value == rhs.d_value;
    }

    template <class TYPE>
    inline
    bool operator!=(const Iterator<TYPE>& lhs, const Iterator<TYPE>& rhs)
        // Return 'false' if the specified 'lhs' is equivalent to the specified
        // 'rhs' and 'true' otherwise.
    {
        return !(lhs == rhs);
    }
//..
// Then, we define 'struct' 'S', the type that will be pointed at by the
// 'Iterator' type:
//..
    struct S {
        char d_c;
        int  d_i;
    };
//..
// Next, we define 4 values for 'S':
//..
    const S s0 = { 'A', 3 };
    const S s1 = { 'B', 5 };
    const S s2 = { 'C', 7 };
    const S s3 = { 'D', 9 };
//..
// Then we define 's', a range of 'S' values:
//..
    S s[] = { s0, s1, s2, s3 };
    enum { NUM_S = sizeof s / sizeof *s };
//..
// Next, we define 'sfBegin', 'sfEnd'
//..
    Iterator<S> sfBegin(s + 0), sfEnd(s + NUM_S);
//..
// Then, we declare our reverse iterator type
//..
    typedef bdlt::CalendarReverseIterator<Iterator<S> > Reverse;
//..
// Next, we declare reverse begin and end iterators to our range of 'S's:
//..
    const Reverse rBegin(sfEnd), rEnd(sfBegin);
//..

// ============================================================================
//                     GLOBAL TYPEDEFS FOR TESTING
// ----------------------------------------------------------------------------

typedef bdlt::CalendarReverseIterator<Iterator<S> >      SObj;
typedef bdlt::CalendarReverseIterator<Iterator<double> > Obj;

// ============================================================================
//                             GLOBAL TEST DATA
// ----------------------------------------------------------------------------

S origS[] = { s0, s1, s2, s3 };

double v[] =     { 0.5, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9 };
double origV[] = { 0.5, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9 };
enum { NUM_V = sizeof v / sizeof *v };
Iterator<double> vfBegin(v + 0), vfEnd(v + NUM_V);

const double v0 = v[0];
const double v1 = v[1];
const double v2 = v[2];
const double v3 = v[3];
const double v4 = v[4];
const double v5 = v[5];
const double v6 = v[6];
const double v7 = v[7];
const double v8 = v[8];
const double v9 = v[9];

// ============================================================================
//                              MAIN PROGRAM
// ----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    const int             test = argc > 1 ? atoi(argv[1]) : 0;
    const bool         verbose = argc > 2;
    const bool     veryVerbose = argc > 3;
    const bool veryVeryVerbose = argc > 4;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;;

    switch (test) { case 0:
      case 7: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE
        //   Extracted from component header file.
        //
        // Concerns:
        //: 1 The usage example provided in the component header file compiles,
        //:   links, and runs as shown.
        //
        // Plan:
        //: 1 Incorporate usage example from header into test driver, remove
        //:   leading comment characters, and replace 'assert' with 'ASSERT'.
        //:   (C-1)
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "USAGE EXAMPLE" << endl
                          << "=============" << endl;

// Now, we traverse our range, streaming out the contents of the 'S' values:
//..
    bsl::ostringstream stream;
    for (Reverse it = rBegin; rEnd != it; ++it) {
        stream << (rBegin == it ? "" : ", ")
               << "{ "
               << it->d_c
               << ", "
               << it->d_i
               << " }";
    }
    stream << bsl::flush;
//..
// Finally, we verify the range output:
//..
    ASSERT(stream.str() == "{ D, 9 }, { C, 7 }, { B, 5 }, { A, 3 }");
//..

      } break;
      case 60: {
        // TBD
        // --------------------------------------------------------------------
        // ASSIGNMENT OPERATOR
        //
        // Concerns:
        //: 1 That the assignment operator sets the lhs to equal the rhs.
        //:
        //: 2 That the 'rhs' is unaffected.
        //:
        //: 3 That the reference returned refers to a modifiable.
        //:
        //: 4 That the reference returned is to the lhs.
        //
        // Plan:
        //: 1 Generate two iterators via nested loops across a range of
        //:   doubles.
        //:
        //: 2 Create 'itC', a third iterator.
        //:
        //: 3 Take additional copies of both of those two.
        //:
        //: 4 Assign to 'itC' the value of the iterator other iterator than the
        //:   one it was created from.  (Note the ohter iterator will sometimes
        //:   have the same value as the inital one), and observe 'itC' now has
        //:   the assigned value.
        //:
        //: 5 Observe the iterator copied is unchanged.
        //:
        //: 6 Repeat the process in the other direction.
        //:
        //: 7 Assign again, but take a pointer to the result.  The type of the
        //:   pointer will prove that the reference returned is to a
        //:   modifiable.
        //:
        //: 8 Observe that the pointer points to 'itC'.
        //:
        //: 9 Assign an object to itself, and observe that the value is
        //:   unchanged.
        // --------------------------------------------------------------------

        if (verbose) cout << "TESTING ASSIGNMENT OPERATOR" << endl
                          << "===========================" << endl;

        const Obj vrBegin(vfEnd), vrEnd(vfBegin);
        ASSERT(vrBegin != vrEnd);

        for (Obj itA = vrBegin; vrEnd != itA; ++itA) {
            for (Obj itB = vrBegin; vrEnd != itB; ++itB) {
                if (veryVeryVerbose) { P_(*itA); P(*itB); }

                Obj itC;
                const Obj itACopy = itA;
                const Obj itBCopy = itB;

                itC = itA;
                ASSERT(itA == itC);
                ASSERT(itACopy == itA);

                itC = itB;
                ASSERT(itB == itC);
                ASSERT(itBCopy == itB);

                Obj * const pit = &(itC = itA);
                ASSERT(pit == &itC);
                ASSERT(itACopy == itA);

                // Aliasing

                itC = itC;
                ASSERT(itA == itC);
            }
        }
      } break;
      case 50: {
        // --------------------------------------------------------------------
        // OTHER MANIPULATORS
        //
        // Concerns:
        //   That postfix increment and decrement work as specced.  (Note that
        //   the underlying forward iterator is not required to support these
        //   operations itself).
        //: 1 That the value of the iterator is affected appropriately.
        //:
        //: 2 That the return value is the value of the iterator before the
        //:   operation.
        //
        // Plan:
        //: 1 Create an iterator, where we have another copy of the iterrator
        //:   with the same value.
        //:
        //: 2 Do the operation, saving a copy of the return value in another
        //:   iterator.
        //:
        //: 3 Observe the returned value has the same value as the initial
        //:   value.
        //:
        //: 4 Observe the iterator operated on was changed.
        //:
        //: 5 Dereference both iterators and verify they're pointing at the
        //:   doubles in the range that are expected.
        // --------------------------------------------------------------------

        if (verbose) cout << "TESTING OTHER MANIPULATORS" << endl
                          << "==========================" << endl;

        const Obj vrBegin(vfEnd), vrEnd(vfBegin);
        ASSERT(vrBegin != vrEnd);

        if (verbose) cout << "Post-increment\n";
        {
            Obj it = vrBegin;
            const Obj jt = it++;
            ASSERT(vrBegin == jt);
            ASSERT(vrBegin != it);
            ASSERT(it != jt);
            ASSERT(&v[9] == &*jt);
            ASSERT(&v[8] == &*it);
        }

        if (verbose) cout << "Post-decrement\n";
        {
            Obj it = vrBegin;
            ++it; ++it;
            const Obj before = it;
            const Obj jt = it--;
            ASSERT(before == jt);
            ASSERT(before != it);
            ASSERT(it != jt);
            ASSERT(&v[7] == &*jt);
            ASSERT(&v[8] == &*it);
        }
      } break;
      case 6: {
        // --------------------------------------------------------------------
        // EQUALITY-COMPARISON OPERATORS
        //   Ensure that '==' and '!=' are the operational definition of value.
        //
        // Concerns:
        //: 1 Two objects, 'X' and 'Y', compare equal if and only if their
        //:   corresponding year/month/day representations compare equal.
        //:
        //: 2 'true  == (X == X)' (i.e., identity).
        //:
        //: 3 'false == (X != X)' (i.e., identity).
        //:
        //: 4 'X == Y' if and only if 'Y == X' (i.e., commutativity).
        //:
        //: 5 'X != Y' if and only if 'Y != X' (i.e., commutativity).
        //:
        //: 6 'X != Y' if and only if '!(X == Y)'.
        //:
        //: 7 Comparison is symmetric with respect to user-defined conversion
        //:   (i.e., both comparison operators are free functions).
        //:
        //: 8 Non-modifiable objects can be compared (i.e., objects or
        //:   references providing only non-modifiable access).
        //:
        //: 9 The equality-comparison operators' signatures and return types
        //:   are standard.
        //
        // Plan:
        //: 1 Use the respective addresses of 'operator==' and 'operator!=' to
        //:   initialize function pointers having the appropriate signatures
        //:   and return types for the two homogeneous, free equality-
        //:   comparison operators defined in this component.  (C-7..9)
        //:
        //: 2 Using the floating point and 'S' arrays, for each element index
        //:   'ti' and 'tj' create reverse iterators from forward iterators
        //:   accessing the indexed elements and verify the results of the
        //:   operators using the comparason of the indices as an oracle for
        //:   the results.  (C-1..6)
        //
        // Testing:
        //   bool operator==(const Date& lhs, const Date& rhs);
        //   bool operator!=(const Date& lhs, const Date& rhs);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "EQUALITY-COMPARISON OPERATORS" << endl
                          << "=============================" << endl;

        if (verbose) cout <<
                "\nAssign the address of each operator to a variable." << endl;
        {
            typedef bool (*operatorPtr)(const Obj&, const Obj&);

            // Verify that the signatures and return types are standard.

            operatorPtr operatorEq = bdlt::operator==;
            operatorPtr operatorNe = bdlt::operator!=;

            (void)operatorEq;  // quash potential compiler warnings
            (void)operatorNe;
        }

        if (verbose) cout << "\nTest with doubles." << endl;

        for (int ti = 0; ti < NUM_V; ++ti) {
            Obj lhs(vfBegin + ti + 1);

            for (int tj = 0; tj < NUM_V; ++tj) {
                Obj rhs(vfBegin + tj + 1);

                LOOP2_ASSERT(ti, tj, (ti == tj) == (lhs == rhs));
                LOOP2_ASSERT(ti, tj, (ti != tj) == (lhs != rhs));
            }
        }

        if (verbose) cout << "\nTest with 'S's." << endl;

        for (int ti = 0; ti < NUM_V; ++ti) {
            SObj lhs(sfBegin + ti + 1);

            for (int tj = 0; tj < NUM_V; ++tj) {
                SObj rhs(sfBegin + tj + 1);

                LOOP2_ASSERT(ti, tj, (ti == tj) == (lhs == rhs));
                LOOP2_ASSERT(ti, tj, (ti != tj) == (lhs != rhs));
            }
        }
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // PRINT AND OUTPUT OPERATOR (<<)
        //   There are no 'print' and 'operator<<' for this component.
        //
        // Testing:
        //   Reserved for 'print' and 'operator<<' functions.
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "PRINT AND OUTPUT OPERATOR (<<)" << endl
                          << "==============================" << endl;

        if (verbose) cout << "No 'print' and 'operator<<' functions." << endl;

      } break;
      case 4: {
        // --------------------------------------------------------------------
        // BASIC ACCESSORS
        //   Ensure each basic accessor properly interprets object state.
        //
        // Concerns:
        //: 1 Each of the basic accessors returns the expected value.
        //:
        //: 2 Each basic accessor method is declared 'const'.
        //
        // Plan:
        //: 1 Traverse the range of doubles with a reverse iterator.
        //:   1 Compare the result of 'operator*' with the expected value
        //:     arrived at through array access.
        //:   2 Assign a new value to the reference returned by 'operator*',
        //:     verifying it is a reference to a modifying, and verify through
        //:     array access that the right memory location was modified.
        //:
        //: 2 Traverse the range of 'S's with a reverse iterator.
        //:   1 Compare the result of 'operator->' with the expected value
        //:     arrived at through array access.
        //:   2 Assign a new value to the reference returned by 'operator->',
        //:     verifying it is a reference to a modifying, and verify through
        //:     array access that the right memory location was modified.
        //:
        //: 3 Traverse the doubles with a reverse iterator and a forward
        //:   iterator at the same time.
        //:   1 Apply the 'forwardIterator' accessor to the reverse iterator
        //:     and verify the result is equal to the forward iterator.
        //
        // Testing:
        //   reference operator*() const;
        //   pointer operator->() const;
        //   iterator forwardIterator() const;
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "BASIC ACCESSORS" << endl
                          << "===============" << endl;

        if (verbose) cout << "Initialize reverse 'begin' and 'end' from\n"
                             "'Iterator' type iterators\n";

        if (verbose) cout << "Traverse the doubles verifying 'operator*'.\n";
        {
            double x = 3.2;

            for (int ti = 0; ti < NUM_V; ++ti, x *= 7.1) {
                Obj it(vfBegin + ti + 1);

                if (veryVerbose) { P_(v[ti]) P(*it); }

                LOOP_ASSERT(ti, v[ti] == *it);
                *it = x;
                LOOP_ASSERT(ti, v[ti] == x);

                v[ti] = origV[ti];                 // restore to original state
            }
        }

        if (verbose) cout << "Traverse the 'S's verifying 'operator->'.\n";
        {
            int i = 7;
            char c = 'f';

            for (int ti = 0;
                 ti < NUM_S;
                 ++ti, i *= 9, c = static_cast<char>('a' + i % 26)) {
                SObj it(sfBegin + ti + 1);

                if (veryVerbose) { P_(it->d_c) P(it->d_i); }

                LOOP_ASSERT(ti, s[ti].d_c == it->d_c);
                LOOP_ASSERT(ti, s[ti].d_i == it->d_i);

                it->d_c = c;
                it->d_i = i;

                LOOP_ASSERT(ti, s[ti].d_c == c);
                LOOP_ASSERT(ti, s[ti].d_i == i);

                s[ti] = origS[ti];                 // restore to original state
            }
        }

        if (verbose) {
            cout << "Traverse the doubles verifying 'forwardIterator'.\n";
        }
        {
            Iterator<double> f = vfBegin + 1;

            for (int ti = 0; ti < NUM_V; ++ti, ++f) {
                Obj it(vfBegin + ti + 1);

                LOOP_ASSERT(ti, f == it.forwardIterator());
            }
        }
      } break;
      case 20: {
        // TBD
        // --------------------------------------------------------------------
        // BASIC MANIPULATORS / COPY C'TOR (bootstrap)
        //
        // Concerns:
        //: 1 The constructor from the forward iterator works properly.
        //:
        //: 2 The copy c'tor works properly.
        //:
        //: 3 Pre increment and decrement have the desired effect on the
        //:   iterator.
        //:
        //: 4 The return value of pre increment and decrement is correct.
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "BASIC MANIPULATORS (bootstrap)" << endl
                          << "==============================" << endl;

        if (verbose) cout << "Initialize reverse 'begin' and 'end' from\n"
                             "'Iterator' type iterators\n";

        const Obj vrBegin(vfEnd), vrEnd(vfBegin);
        ASSERT(vrBegin != vrEnd);

        Obj it = vrBegin;

        if (verbose) cout << "Test pre increment\n";

        ASSERT(v9 == *it);
        ++it;
        const Obj jit = it;
        ASSERT(&v[8] == &*it);
        ASSERT(&v[8] == &*jit);

        if (verbose) cout << "Test pre decrement\n";
        {
            --it;
            const Obj hit = it;
            ASSERT(&v[9] == &*it);
            ASSERT(&v[9] == &*hit);
        }

        if (verbose) cout << "Test return values of both manipulators\n";
        {
            ASSERT(&v[8] == &*++it);
            ASSERT(&v[9] == &*--it);
        }
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // GENERATOR FUNCTION 'gg'
        //   There is no 'gg' function for this component.
        //
        // Testing:
        //   Reserved for 'gg' generator function.
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "GENERATOR FUNCTION 'gg'" << endl
                          << "=======================" << endl;

        if (verbose) cout << "No 'gg' function for 'bdlt::Date'." << endl;

      } break;
      case 2: {
        // --------------------------------------------------------------------
        // VALUE CTOR & DTOR
        //   Ensure that we can use the value constructor to create an object
        //   having any state relevant for thorough testing, and use the
        //   destructor to destroy it safely.
        //
        // Concerns:
        //: 1 The value constructor can create an object to have any value that
        //:   does not violate the method's documented preconditions.
        //:
        //: 2 An object can be safely destroyed.
        //
        // Plan:
        //: 1 Directly test the value constructor.  (C-1)
        //:
        //: 2 Let the object created in P-1 go out of scope.  (C-2)
        //
        // Testing:
        //   CalendarReverseIterator(const iterator& value);
        //   ~CalendarReverseIterator();
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "VALUE CTOR & DTOR" << endl
                          << "=================" << endl;

        for (int i = 0; i < NUM_S; ++i) {
            Obj reverseIterator(vfBegin + i + 1);

            LOOP_ASSERT(i, *reverseIterator == *(vfBegin + i));
        }
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST
        //
        // Concerns:
        //: 1 The class is sufficiently functional to enable comprehensive
        //:   testing in subsequent test cases.
        //
        // Plan:
        //: 1 Start with an array of 'double's, and create 'Iterator' iterators
        //:   pointing to the begin ('vfBegin') and end ('vfEnd') of that
        //:   array.
        //:
        //: 2 Create reverse begin and end iterators from the forward
        //:   iterators.
        //:
        //: 3 Create other reverse iterator objects by copying from the
        //:   begin and end reverse iterators, and manipulate with with pre
        //:   and post increments and decrements, verifying that, when
        //:   dereferenced, they show that they are pointing to the right
        //:   'double' values.
        //:
        //: 4 Traverse the length of the range.
        //:
        //: 5 Verify that a refrence to a modifiable is returned by modifying
        //:   the 'doubles' returned to, across the range.
        //:
        //: 6 Traverse the range again, verifying that the 'double' values are
        //:   as expected.
        //
        // Testing:
        //   BREATHING TEST
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "BREATHING TEST" << endl
                          << "==============" << endl;

        if (verbose) cout << "Initialize reverse 'begin' and 'end' from\n"
                             "'Iterator' type iterators\n";

        const Obj vrBegin(vfEnd), vrEnd(vfBegin);
        ASSERT(vrBegin != vrEnd);

        if (verbose) cout << "Test default c'tor\n";
        {
            Obj it;
            it = vrBegin;
            ASSERT(vrBegin == it);
            ASSERT(vrEnd   != it);
        }

        if (verbose) cout << "Test copy c'tor\n";

        Obj it = vrBegin;
        ASSERT(vrBegin == it);
        ASSERT(vrEnd   != it);

        if (verbose) cout << "Test 'operator*'\n";
        {
            ASSERT(v9 == *it);
        }

        if (verbose) cout << "Test 'operator++' and its return value.\n";
        {
            ASSERT(v8 == *++it);
            ASSERT(v8 == *it);
            ASSERT(vrBegin != it);
        }

        if (verbose) cout << "Test post decrement.\n";

        Obj jit = it--;
        ASSERT(v8 == *jit);
        ASSERT(v9 == *it);
        ASSERT(vrBegin == it);
        ASSERT(it != jit);

        if (verbose) cout << "Test post increment.\n";
        {
            ASSERT(v9 == *it++);
            ASSERT(v8 == *it);
        }

        if (verbose) cout << "Test pre decrement.\n";
        {
            ASSERT(v9 == *--it);
            ASSERT(vrBegin == it);
            ASSERT(vrEnd != it);
        }

        if (verbose) cout << "Pre increment through the sequence.\n";
        {
            ASSERT(vrBegin == it++);
            ASSERT(v8 == *it);
            ASSERT(v7 == *++it);
            ASSERT(v6 == *++it);
            ASSERT(v5 == *++it);
            ASSERT(v4 == *++it);
            ASSERT(v3 == *++it);
            ASSERT(v2 == *++it);
            ASSERT(v1 == *++it);
            ASSERT(v0 == *++it);
            ASSERT(vrEnd != it);
            ASSERT(vrEnd == ++it);
            ASSERT(vrEnd == it);
        }

        if (verbose) cout << "Decrement back a bit from the end.\n";
        {
            ASSERT(v0 == *--it);
            ASSERT(v1 == *--it);
            ASSERT(v1 == *it--);
        }

        if (verbose) cout << "Iterate through the whole sequence.\n";
        {
            int ti = NUM_V - 1;
            for (it = vrBegin; vrEnd != it; ++it, --ti) {
                if (veryVerbose) { P_(v[ti]) P(*it); }

                LOOP3_ASSERT(v[ti], ti, *it, v[ti] == *it);
            }
            ASSERT(-1 == ti);
        }

        if (verbose) cout << "Reverse the order of the sequence via 'swap'.\n";
        {
            Obj itB(vrEnd); --itB;
            int ti = 0;
            for (it = vrBegin; ti < 5; --itB, ++it, ++ti) {
                if (veryVerbose) { P_(*it) P(*itB); }

                bsl::swap(*it, *itB);
            }
            for (it = vrBegin, ti = 0; vrEnd != it; ++it, ++ti) {
                if (veryVerbose) { P_(origV[ti]) P(*it); }

                ASSERT(origV[ti] == *it);
            }
        }
      } break;
      default: {
        cerr << "WARNING: CASE `" << test << "' NOT FOUND." << endl;
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        cerr << "Error, non-zero test status = " << testStatus << "." << endl;
    }
    return testStatus;
}
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
