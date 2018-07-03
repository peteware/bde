// bdlb_numericparseutil.h                                            -*-C++-*-
#ifndef INCLUDED_BDLB_NUMERICPARSEUTIL
#define INCLUDED_BDLB_NUMERICPARSEUTIL

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide conversions from text into fundamental numeric types.
//
//@CLASSES:
//  bdlb::NumericParseUtil: namespace for parsing functions
//
//@DESCRIPTION: This component provides a namespace, 'bdlb::NumericParseUtil',
// containing utility functions for parsing ascii text representations of
// numeric values into the corresponding value of a fundamental C++ type (like
// 'int' or 'double').
//
// None of the parsing functions in this component consume leading whitespace.
// For parsing to succeed, the sought item must be found at the beginning of
// the input string.
//
// The following two subsections describe the grammar defining the parsing
// rules.
//
///Definition of Symbols Used in Production Rules
///----------------------------------------------
//
// The following grammar is used to specify regular expressions:
//..
//   -     Within brackets the minus means through.  For example, [a-z] is
//         equivalent to [abcd...xyz].  The - can appear as itself only if used
//         as the first or last character.  For example, the character class
//         expression []-] matches the characters ] and -.
//
//   |     Logical OR between two expressions means one must be present.
//
//   ( ... ) Parentheses are used for grouping.  An operator, for example, *,
//         +, {}, can work on a single character or on a regular expression
//         enclosed in parentheses.  For example, (a*(cb+)*)$.
//..
//
///Grammar Production Rules
///------------------------
//..
// <NUMBER> ::= <OPTIONAL_SIGN><DIGIT>+
// <DECIMAL_NUMBER> ::= <OPTIONAL_SIGN><DECIMAL_DIGIT>+
// <POSITIVE_NUMBER> ::= <DIGIT>+
// <OPTIONAL_SIGN> ::= (+|-)?
// <DIGIT> ::= depending on base can include characters 0-9 and case-
//      insensitive letters.  For example, octal digit is in the range
//      [0 .. 7].
// <DECIMAL_DIGIT> ::= [0123456789]
// <OCTAL_DIGIT> ::= [01234567]
// <HEX_DIGIT> ::= [0123456789abcdefABCDEF]
// <SHORT> ::= <NUMBER>
//      <SHORT> must be in range [SHRT_MIN .. SHRT_MAX].
// <USHORT> ::= <NUMBER>
//      <USHORT> must be in range [0 .. USHRT_MAX].
// <INT> ::= <NUMBER>
//      <INT> must be in range [INT_MIN .. INT_MAX].
// <INT64> ::= <NUMBER>
//      <INT64> must be in range
//                           [-0x8000000000000000uLL .. 0x7FFFFFFFFFFFFFFFuLL].
// <UNSIGNED> ::= <NUMBER>
//      <UNSIGED> must be in range [0 .. UINT_MAX].
// <UNSIGED64> ::= <NUMBER>
//      <UNSIGNED64> must be in range
//                           [0 .. 0xFFFFFFFFFFFFFFFFuLL].
// <REAL> ::= <OPTIONAL_SIGN>
//            (<DECIMAL_DIGIT>+ (. <DECIMAL_DIGIT>*)? | . <DECIMAL_DIGIT>+)
//            (e|E <DECIMAL_NUMBER>)
// <INF>    ::= infinity | inf
//              case insensitive
// <NAN-SEQUENCE> ::= [abcdefghijklmnopqrstuvwxyz0123456789_]*
// <NAN>    ::= nan(<NAN-SEQUENCE>) | nan
//              case insensitive
// <DOUBLE> ::= <REAL> | <INF> | <NAN>
//      <DOUBLE> must be in range [DBL_MIN .. DBL_MAX].
//..
//
///Remainder Output Parameter
///--------------------------
// The parsing functions provided by 'NumericParseUtil' typically return an
// optional, second, output parameter named 'remainder'.  The output parameter
// 'remainder' is loaded with a string reference starting at the character
// following the last character successfully parsed as part of the numeric
// value, and ending at the character one past the end of the input string.  If
// the entire input string is parsed successfully, 'remainder' is loaded with
// an empty string reference. However, if the parse function is not successful
// (i.e., it returns a non-zero error status), then it will not modify the
// value of 'remainder'.
//
///Floating Point Values
///---------------------
// The conversion from text to values of type 'double' results in the closest
// representable value to the decimal text.  Note that this is the same as for
// the standard library function 'strtod'.  For example, the ASCII string
// "3.14159" is converted, on some platforms, to 3.1415899999999999.
//
///Special Floating Point Values
///- - - - - - - - - - - - - - -
// The IEEE-754 (double precision) floating point format supports the following
// special values: Not-a-Number (NaN) and Infinity, both in positive or
// negative.  'parseDouble' allows expressions for both:
//
//: *infinity-expression*: results in negative of positive
//:       bsl::numeric_limits<double>::infinity() value.  The expresssion
//:       consists of the following elements:
//:   o an optional plus ('+') or minus ('-') sign
//:   o the word "INF" or INFINITY", ignoring case
//:
//: *not-a-number-expression*: results in a negative or positive
//:     bsl::numeric_limits<double>::quiet_NaN() value.  The expresssion
//:     consists of the following elements:
//:   o an optional plus ('+') or minus ('-') sign
//:   o "NAN" or "NAN(char-sequence)" ignoring the case of "NAN".  The
//:     char-sequence may be empty or contain digits, letters from the Latin
//:     alphabet and underscores.
//
///!Warning!: Microsoft Visual Studio 2013 Output for Infinity and NaN
///- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Microsoft Visual Studio 2013 generates surprising output text when printing
// (using 'printf') or streaming (using C++ iostream) the 'double'
// representations for infinity and NaN.  For example, infinity might be
// rendered "1.#INF00" and NaN might be rendered "1.#IND00" or "1.#NAN0".
// 'parseDouble' will successfully parse this text but will not return the
// result one would naively expect (e.g., returning the value 1.0).
//
///Usage Example
///-------------
// In this section, we show the intended usage of this component.
//
///Example 1: Parsing an Integer Value from a 'StringRef'
/// - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Suppose that we have a 'StringRef' that presumably contains a (not
// necessarily NUL terminated) string representing a 32-bit integer value and
// we want to convert that string into an 'int' (32-bit integer).
//
// First, we create the string:
//..
//  bslstl::StringRef input("20171024", 4);
//..
// Then we create the output variables for the parser:
//..
//  int               year;
//  bslstl::StringRef rest;
//..
// Next we call the parser function:
//..
//  int rv = NumericParseUtil::parseInt(&year, &rest, input);
//..
// Then we verify the results:
//..
//  assert(0    == rv);
//  assert(2017 == year);
//  assert(rest.empty());
//..


#ifndef INCLUDED_BDLSCM_VERSION
#include <bdlscm_version.h>
#endif

#ifndef INCLUDED_BSLS_TYPES
#include <bsls_types.h>
#endif

#ifndef INCLUDED_BSL_STRING
#include <bsl_string.h>
#endif

namespace BloombergLP {

namespace bdlb {
                          // =======================
                          // struct NumericParseUtil
                          // =======================

struct NumericParseUtil {
    // This 'struct' 'NumericParseUtil' provides a namespace for a suite of
    // stateless procedures that perform low-level parsing functionality.

  public:
    // TYPES
    typedef bslstl::StringRef::size_type size_type;
        // Shorter name for readability.

    // CLASS METHODS
    static int characterToDigit(char character, int base);
        // Determine whether the specified 'character' represents a digit in
        // the specified 'base'; return the numeric equivalent if so, and -1
        // otherwise.  The behavior is undefined if either 'character' or
        // 'base' is 0 and unless '2 <= base' and 'base <= 36' (i.e., bases
        // where digits are representable by characters in the range ['0'-'9'],
        // ['a'-'z'], or ['A'-'Z']).

    static int parseDouble(double                   *result,
                           const bslstl::StringRef&  inputString);
    static int parseDouble(double                   *result,
                           bslstl::StringRef        *remainder,
                           const bslstl::StringRef&  inputString);
        // Parse the specified 'inputString' for a sequence of characters
        // matching the production rule <DOUBLE> (see {GRAMMAR PRODUCTION
        // RULES}) and place into the specified 'result' the corresponding
        // value.  Optionally specify 'remainder', in which to store the
        // remainder of the 'inputString' immediately following the
        // successfully parsed text, or the position at which a parse failure
        // was detected.  Return zero on success, and a non-zero value
        // otherwise.  The value of 'result' is unchanged if a parse failure
        // occurs.  The behavior is undefined unless the current locale is the
        // "C" locale, such that 'strcmp(setlocale(0, 0), "C") == 0'.  For more
        // information see {Floating Point Values}.

    static int parseInt(int                      *result,
                        const bslstl::StringRef&  inputString,
                        int                       base = 10);
    static int parseInt(int                      *result,
                        bslstl::StringRef        *remainder,
                        const bslstl::StringRef&  inputString,
                        int                       base = 10);
        // Parse the specified 'inputString' for the maximal sequence of
        // characters forming an <INT> (see {GRAMMAR PRODUCTION RULES}) in the
        // optionally specified 'base' or in base 10 if 'base' is not
        // specified, and place into the specified 'result' the corresponding
        // value.  Optionally specify 'remainder', in which to store the
        // remainder of the 'inputString' immediately following the
        // successfully parsed text, or the position at which a parse failure
        // was detected.  If the parsed number is outside of the
        // '[INT_MIN .. INT_MAX]' range the 'result' will be the longest number
        // that does not over/underflow and 'remainder' start at the first
        // digit that would make the number too large/small.  Return zero on
        // success, and a non-zero value otherwise.  The value of 'result' is
        // unchanged if a parse failure occurs.  The behavior is undefined
        // unless '2 <= base' and 'base <= 36' (i.e., bases where digits are
        // representable by characters in the range ['0'-'9'], ['a'-'z'], or
        // ['A'-'Z']).
        //
        // A parse failure can occur for the following reasons:
        //..
        //   1. The 'inputString' is empty.
        //   2. The first character of 'inputString' is not a valid digit in
        //      'base', or an optional sign followed by a valid digit.
        //..

    static int parseInt64(bsls::Types::Int64       *result,
                          const bslstl::StringRef&  inputString,
                          int                       base = 10);
    static int parseInt64(bsls::Types::Int64       *result,
                          bslstl::StringRef        *remainder,
                          const bslstl::StringRef&  inputString,
                          int                       base = 10);
        // Parse the specified 'inputString' for the maximal sequence of
        // characters forming a valid <INT64> (see {GRAMMAR PRODUCTION RULES})
        // in the optionally specified 'base' or in base 10 if 'base' is not
        // specified, and place into the specified 'result' the corresponding
        // value.  Optionally specify 'remainder', in which to store the
        // remainder of the 'inputString' immediately following the
        // successfully parsed text, or the position at which a parse failure
        // was detected.  If the parsed number is outside of the
        // '[-0x8000000000000000uLL .. 0x7FFFFFFFFFFFFFFFull]' range the
        // 'result' will be the longest number that does not over/underflow and
        // 'remainder' will start at the first digit that would make the number
        // too large/small.  Return zero on success, and a non-zero value
        // otherwise.  The value of 'result' is unchanged if a parse failure
        // occurs.  The behavior is undefined unless '2 <= base' and
        // 'base <= 36' (i.e., bases where digits are representable by
        // characters in the range ['0'-'9'], ['a'-'z'], or ['A'-'Z']).
        //
        // A parse failure can occur for the following reasons:
        //..
        //   1. The 'inputString' is empty.
        //   2. The first character of 'inputString' is not a valid digit in
        //      'base', or an optional sign followed by a valid digit.
        //..

    static int parseShort(short                    *result,
                          const bslstl::StringRef&  inputString,
                          int                       base = 10);
    static int parseShort(short                    *result,
                          bslstl::StringRef        *remainder,
                          const bslstl::StringRef&  inputString,
                          int                       base = 10);
        // Parse the specified 'inputString' for the maximal sequence of
        // characters forming a valid <SHORT> (see {GRAMMAR PRODUCTION RULES})
        // in the optionally specified 'base' or in base 10 if 'base' is not
        // specified, and place into the specified 'result' the corresponding
        // value.  Optionally specify 'remainder', in which to store the
        // remainder of the 'inputString' immediately following the
        // successfully parsed text, or the position at which a parse failure
        // was detected.  If the parsed number is outside of the
        // '[SHRT_MIN .. SHRT_MAX]' range the 'result' will be the longest
        // number that does not over/underflow and 'remainder' will start at
        // the first digit that would make the number too large/small.  Return
        // zero on success, and a non-zero value otherwise.  Return zero on
        // success, and a non-zero value otherwise.  The value of 'result' is
        // unchanged if a parse failure occurs.  The behavior is undefined
        // unless '2 <= base' and 'base <= 36' (i.e., bases where digits are
        // representable by characters in the range ['0'-'9'], ['a'-'z'] or
        // ['A'-'Z']).
        //
        // A parse failure can occur for the following reasons:
        //..
        //   1. The 'inputString' is empty.
        //   2. The first character of 'inputString' is not a valid digit in
        //      'base', or an optional sign followed by a valid digit.
        //..

    static int parseUint(unsigned int             *result,
                         const bslstl::StringRef&  inputString,
                         int                       base = 10);
    static int parseUint(unsigned int             *result,
                         bslstl::StringRef        *remainder,
                         const bslstl::StringRef&  inputString,
                         int                       base = 10);
        // Parse the specified 'inputString' for the maximal sequence of
        // characters forming an <UNSIGNED> (see {GRAMMAR PRODUCTION RULES}) in
        // the optionally specified 'base' or in base 10 if 'base' is not
        // specified, and place into the specified 'result' the corresponding
        // value.  Optionally specify 'remainder', in which to store the
        // remainder of the 'inputString' immediately following the
        // successfully parsed text, or the position at which a parse failure
        // was detected.  If the parsed number is outside of the
        // '[0 .. UINT_MAX]' range the 'result' will be the longest number that
        // does not overflow and 'remainder' will start at the first digit that
        // would make the number too large.  Return zero on success, and a
        // non-zero value otherwise.  The value of 'result' is unchanged if a
        // parse failure occurs.  The behavior is undefined unless '2 <= base'
        // and 'base <= 36' (i.e., bases where digits are representable by
        // characters in the range ['0'-'9'], ['a'-'z'], or ['A'-'Z']).
        //
        // A parse failure can occur for the following reasons:
        //..
        //   1. The 'inputString' is empty.
        //   2. The first character of 'inputString' is not a valid digit in
        //      'base', or an optional sign followed by a valid digit.
        //..

    static int parseUint64(bsls::Types::Uint64      *result,
                           const bslstl::StringRef&  inputString,
                           int                       base = 10);
    static int parseUint64(bsls::Types::Uint64      *result,
                           bslstl::StringRef        *remainder,
                           const bslstl::StringRef&  inputString,
                           int                       base = 10);
        // Parse the specified 'inputString' for the maximal sequence of
        // characters forming a valid <UNSIGNED64> (see {GRAMMAR PRODUCTION
        // RULES}) in the optionally specified 'base' or in base 10 if 'base'
        // is not specified, and place into the specified 'result' the
        // corresponding value.  Optionally specify 'remainder', in which to
        // store the remainder of the 'inputString' immediately following the
        // successfully parsed text, or the position at which a parse failure
        // was detected.  If the parsed number is outside of the
        // '[0 .. 0XFFFFFFFFFFFFFFFF]' range the 'result' will be the longest
        // number that does not overflow and 'remainder' will start at the
        // first digit that would make the number too large.  Return zero on
        // success, and a non-zero value otherwise.  The value of 'result' is
        // unchanged if a parse failure occurs.  The behavior is undefined
        // unless '2 <= base' and 'base <= 36' (i.e., bases where digits are
        // representable by characters in the range ['0'-'9'], ['a'-'z'], or
        // ['A'-'Z']).
        //
        // A parse failure can occur for the following reasons:
        //..
        //   1. The 'inputString' is empty.
        //   2. The first character of 'inputString' is not a valid digit in
        //      'base', or an optional sign followed by a valid digit.
        //..

    static int parseUshort(unsigned short           *result,
                           const bslstl::StringRef&  inputString,
                           int                       base = 10);
    static int parseUshort(unsigned short           *result,
                           bslstl::StringRef        *remainder,
                           const bslstl::StringRef&  inputString,
                           int                       base = 10);
        // Parse the specified 'inputString' for the maximal sequence of
        // characters forming a valid <USHORT> (see {GRAMMAR PRODUCTION RULES})
        // in the optionally specified 'base' or in base 10 if 'base' is not
        // specified, and place into the specified 'result' the corresponding
        // value.  Optionally specify 'remainder', in which to store the
        // remainder of the 'inputString' immediately following the
        // successfully parsed text, or the position at which a parse failure
        // was detected.  If the parsed number is outside of the
        // '[0 .. USHRT_MAX]' range the 'result' will be the longest number
        // that does not over/underflow and 'remainder' will start at the first
        // digit that would make the number too large/small.  Return zero on
        // success, and a non-zero value otherwise.  Return zero on success,
        // and a non-zero value otherwise.  The value of 'result' is unchanged
        // if a parse failure occurs.  The behavior is undefined unless
        // '2 <= base' and 'base <= 36' (i.e., bases where digits are
        // representable by characters in the range ['0'-'9'], ['a'-'z'] or
        // ['A'-'Z']).
        //
        // A parse failure can occur for the following reasons:
        //..
        //   1. The 'inputString' is empty.
        //   2. The first character of 'inputString' is not a valid digit in
        //      'base', or an optional sign followed by a valid digit.
        //..

    static int parseSignedInteger(bsls::Types::Int64       *result,
                                  const bslstl::StringRef&  inputString,
                                  int                       base,
                                  const bsls::Types::Int64  minValue,
                                  const bsls::Types::Int64  maxValue);
    static int parseSignedInteger(bsls::Types::Int64       *result,
                                  bslstl::StringRef        *remainder,
                                  const bslstl::StringRef&  inputString,
                                  int                       base,
                                  const bsls::Types::Int64  minValue,
                                  const bsls::Types::Int64  maxValue);
        // Parse the specified 'inputString' for an optional sign followed by a
        // sequence of characters representing digits in the specified 'base',
        // consuming the maximum that will form a number whose value is less
        // than or equal to the specified 'maxValue' and greater than or equal
        // to the specified 'minValue'.  Place into the specified 'result' the
        // extracted value.  Optionally specify 'remainder', in which to store
        // the remainder of the 'inputString' immediately following the
        // successfully parsed text, or the position at which a parse failure
        // was detected.  Return 0 on success, and a non-zero value otherwise.
        // The value of 'result' is unchanged if a parse failure occurs.  The
        // behavior is undefined unless 'maxValue' a positive integer, and
        // 'minValue' is negative (this is required to allow for efficient
        // implementation).  The behavior is also undefined unless '2 <= base'
        // and 'base <= 36', (i.e., bases where digits are representable by
        // characters '[ 0 .. 9 ]', '[ a .. z ]' or '[ A .. Z ]').
        //
        // A parse failure can occur for the following reasons:
        //..
        //   1. The parsed string is not a '<NUMBER>', i.e., does not contain
        //      an optional sign followed by at least one digit.
        //   2. The first digit in 'inputString' is larger than 'maxValue' or
        //      smaller than 'minValue'.
        //   3. The first digit is not a valid number for the 'base'.
        //..

    static int parseUnsignedInteger(bsls::Types::Uint64       *result,
                                    const bslstl::StringRef&   inputString,
                                    int                        base,
                                    const bsls::Types::Uint64  maxValue);
    static int parseUnsignedInteger(bsls::Types::Uint64       *result,
                                    bslstl::StringRef         *remainder,
                                    const bslstl::StringRef&   inputString,
                                    int                        base,
                                    const bsls::Types::Uint64  maxValue);
    static int parseUnsignedInteger(bsls::Types::Uint64       *result,
                                    const bslstl::StringRef&   inputString,
                                    int                        base,
                                    const bsls::Types::Uint64  maxValue,
                                    int                        maxNumDigits);
    static int parseUnsignedInteger(bsls::Types::Uint64       *result,
                                    bslstl::StringRef         *remainder,
                                    const bslstl::StringRef&   inputString,
                                    int                        base,
                                    const bsls::Types::Uint64  maxValue,
                                    int                        maxNumDigits);
        // Parse the specified 'inputString' for a sequence of characters
        // representing digits in the specified 'base', consuming the maximum
        // up to the optionally specified 'maxNumDigits' that form a number
        // whose value does not exceed the specified 'maxValue'.  Place into
        // the specified 'result' the extracted value.  Optionally specify
        // 'remainder', in which to store the remainder of the 'inputString'
        // immediately following the successfully parsed text, or the position
        // at which a parse failure was detected.  Return 0 on success, and a
        // non-zero value otherwise.  The value of 'result' is unchanged if a
        // parse failure occurs.  If 'maxNumDigits' is not specified, it
        // defaults to a number larger than the number of possible digits in an
        // unsigned 64-bit integer.  The behavior is undefined unless
        // '2 < = base' and  'base <= 36' (i.e., bases where digits are
        // representable by characters '[ 0 .. 9 ]', '[ a .. z ]' or
        // '[ A .. Z ]').
        //
        // A parse failure can occur for the following reasons:
        //..
        //   1. The 'inputString' is not a '<POSITIVE_NUMBER>', i.e., does
        //      not begin with a digit.
        //   2. The first digit in 'inputString' is larger than 'maxValue'.
        //   3. The first digit is not a valid number for the 'base'.
        //..
};

// ============================================================================
//                 INLINE AND TEMPLATE FUNCTION DEFINITIONS
// ============================================================================

                          // -----------------------
                          // struct NumericParseUtil
                          // -----------------------

inline
int NumericParseUtil::parseDouble(double                   *result,
                                  const bslstl::StringRef&  inputString)
{
    BSLS_ASSERT(result);

    bslstl::StringRef rest;
    return parseDouble(result, &rest, inputString);
}

inline
int NumericParseUtil::parseInt(int                      *result,
                               const bslstl::StringRef&  inputString,
                               int                       base)
{
    BSLS_ASSERT(result);

    bslstl::StringRef rest;
    return parseInt(result, &rest, inputString, base);
}

inline
int NumericParseUtil::parseInt64(bsls::Types::Int64       *result,
                                 const bslstl::StringRef&  inputString,
                                 int                       base)
{
    BSLS_ASSERT(result);

    bslstl::StringRef rest;
    return parseInt64(result, &rest, inputString, base);
}

inline
int NumericParseUtil::parseShort(short                    *result,
                                 const bslstl::StringRef&  inputString,
                                 int                       base)
{
    BSLS_ASSERT(result);

    bslstl::StringRef rest;
    return parseShort(result, &rest, inputString, base);
}

inline
int NumericParseUtil::parseUint(unsigned int             *result,
                                const bslstl::StringRef&  inputString,
                                int                       base)
{
    BSLS_ASSERT(result);

    bslstl::StringRef rest;
    return parseUint(result, &rest, inputString, base);
}

inline
int NumericParseUtil::parseUint64(bsls::Types::Uint64      *result,
                                  const bslstl::StringRef&  inputString,
                                  int                       base)
{
    BSLS_ASSERT(result);

    bslstl::StringRef rest;
    return parseUint64(result, &rest, inputString, base);
}

inline
int NumericParseUtil::parseUshort(unsigned short           *result,
                                  const bslstl::StringRef&  inputString,
                                  int                       base)
{
    BSLS_ASSERT(result);

    bslstl::StringRef rest;
    return parseUshort(result, &rest, inputString, base);
}

inline
int NumericParseUtil::parseSignedInteger(bsls::Types::Int64       *result,
                                         const bslstl::StringRef&  inputString,
                                         int                       base,
                                         const bsls::Types::Int64  minValue,
                                         const bsls::Types::Int64  maxValue)
{
    BSLS_ASSERT(result);

    bslstl::StringRef rest;
    return parseSignedInteger(result,
                              &rest,
                              inputString,
                              base,
                              minValue,
                              maxValue);
}

inline
int NumericParseUtil::parseUnsignedInteger(
                                         bsls::Types::Uint64      *result,
                                         const bslstl::StringRef&  inputString,
                                         int                       base,
                                         const bsls::Types::Uint64 maxValue)
{
    BSLS_ASSERT(result);

    bslstl::StringRef rest;
    return parseUnsignedInteger(result, &rest, inputString, base, maxValue);
}

inline
int NumericParseUtil::parseUnsignedInteger(
                                        bsls::Types::Uint64      *result,
                                        const bslstl::StringRef&  inputString,
                                        int                       base,
                                        const bsls::Types::Uint64 maxValue,
                                        int                       maxNumDigits)
{
    BSLS_ASSERT(result);

    bslstl::StringRef rest;
    return parseUnsignedInteger(result,
                                &rest,
                                inputString,
                                base,
                                maxValue,
                                maxNumDigits);
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
