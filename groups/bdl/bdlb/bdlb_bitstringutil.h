// bdlb_bitstringutil.h                                               -*-C++-*-
#ifndef INCLUDED_BDLB_BITSTRINGUTIL
#define INCLUDED_BDLB_BITSTRINGUTIL

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide efficient operations on a multi-word sequence of bits.
//
//@CLASSES:
// bdlb::BitStringUtil: namespace for common bit-manipulation procedures
//
//@SEE_ALSO: bdlb_bitutil, bdlb_bitmaskutil, bdlb_bitstringimputil,
//           bdlc_bitarray
//
//@DESCRIPTION: This component provides a utility 'struct',
// 'bdlb::BitStringUtil', that serves as a namespace for a collection of
// efficient, bit-level procedures on sequences of bits represented by arrays
// of 64-bit 'uint64_t' values.
//
///The 'BitString', a Pseudo-Type
///------------------------------
// A contiguous sequence of bits that occupy a positive integer of contiguous
// 'uint64_t' values can be viewed as a string of bits.  This component
// supports operations on such sequences.  The notion of 'BitString', an
// "informal type", is used to document those operations.  Correspondingly,
// 'BitStringUtil' operations are categorized as either "manipulators",
// operations that modify the 'BitString'; and "accessors", operations that
// return information and guarantee that no change to the bit string(s)
// occur(s).
//
// A 'BitString' has two "structural" attributes:
//..
//  Capacity - The number of bits provided in the buffer.  Note that this
//             must be a multiple of 'sizeof(uint64_t) * CHAR_BIT', because
//             the procedures in this 'struct' always access whole 'uint64_t's
//             at a time, even when not modifying all bits.
//
//  Length   - The number of bits currently being operated upon, though the
//             'insert' operation will normally write to bits past the original
//             length of the string, but must not write to bits past the
//             capacity of the string.
//..
// Since this is a pseudo-type, there is no language support for managing these
// values; the user must do so explicitly.
//
// Many operations on 'BitString' refer to a "position" within a 'BitString';
// or a range of positions within a 'BitString':
//..
//  Position - The offset (in bits) of a bit value from the beginning of a
//             'BitString' (also called the "index" of a bit).
//..
// The notion of "position" used in this component is a generalization of the
// notion of a bit's position in a single integer value.
//
// Bits within a 64-bit uint64_t (irrespective of the endian-ness of a
// platform) are here numbered, starting at 0, from the least-significant bit
// to the most-significant bit.  In illustrations, we typically show the
// high-order bits on the left:
//..
//   63 62  . . . . .   5  4  3  2  1  0
//  +-----------------------------------+
//  | 1| 0| . . . . . | 1| 1| 0| 0| 1| 0|
//  +-----------------------------------+
//..
// Thus, "left"-shifting (caused by 'insert'ing low-oder bits) causes bits to
// move up in bit-position (to positions of higher significance) and
// "right"-shifting (i.e., using the '>>' operator) causes bits to move into
// positions of less significance.
//
// This component extends this representation to an extensible sequence of bits
// implemented using an array of 64-bit 'uint64_t's.  For example, the
// 'BitString' shown below is built on an array of three 'uint64_t' values.
// Thus, it has a capacity of 192 (i.e., '3 * sizeof(uint64_t) * CHAR_BIT').
// Note that words are ordered right-to-left, so the lowest order bits are to
// the right.  This is also how the 'bdlb::BitStringUtil::print' function
// orders the words it prints.
//..
//  |<------ word 2 ------>|<------ word 1 ------>|<------ word 0 ------>|
//  | 191 190 . .  129 128 | 127 126 . . .  65 64 | 63 62 . . . . .  1 0 |
//  +----------------------+----------------------+----------------------+
//..
//
///Function Categories
///-------------------
// The functions implemented in this utility component can be broadly
// categorized as "accessors" (i.e., those that return a value but do not
// modify any argument), and "manipulators" (i.e., those that return 'void',
// but take the address of an integer as the first argument in order to modify
// it in place).
//
// Most accessor methods refer to a single range of bits within a single
// 'BitString'.  Note that 'get' is in this category even though the width of
// its 'BitString' is always 1.  The manipulators include:
//..
//                            // Manipulators
//
/// assignment:
///- - - - - -
//  assign
//  assign0
//  assign1
//  assignBits
//
/// bitwise-logical:
/// - - - - - - - -
//  andEqual
//  minusEqual
//  orEqual
//  xorEqual
//
/// copy:
///- - -
//  copy
//  copyRaw
//
/// inssrt / remove:
/// - - - - - - - -
//  insert
//  insert0
//  insert1
//  insertRaw
//  remove
//  removeAndFill0
//
/// other manipulators:
///- - - - - - - - - -
//  swapRaw
//  toggle
//..
//
//                            // Accessors
//
/// compare:
/// - - - -
//  areEqual
//
/// read:
///- - -
//  bit
//  bits
//
/// find:
///- - -
//  find0AtMaxIndex
//  find0AtMinIndex
//  find1AtMaxIndex
//  find1AtMinIndex
//
/// count:
/// - - -
//  isAny0
//  isAny1
//  num0
//  num1
//
/// output:
///- - - -
//  print
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Keeping a calendar of business days
/// - - - - - - - - - - - - - - - - - - - - - - -
// Bitstreams can be used to represent efficient business calendars and do
// operations on such calendars.  First, create a 'Bitstream' with sufficient
// capacity to represent every day of a year (note that 32 * 12 = 384) and set
// a 1-bit in the indices corresponding to the day-of-year (DOY) for each
// weekend day.  For convenience in date calculations the 0 index is not used;
// the 365 days of the year are at indices [1,365].  Further note that the
// values set below all correspond to the year 2013:
//..
//  uint64_t weekends[6] = {0};
//
//  // Start on the first Saturday of the year, Jan 5, 2013.
//
//  for (int i = 5; i < 366; i += 7) {
//      bdlb::BitStringUtil::assign(weekends, i,   1);
//      if (i + 1 < 366) {
//          bdlb::BitStringUtil::assign(weekends, i+1, 1);
//      }
//  }
//..
// Now, we can easily use 'bdeu_bitstreamutil' methods to find days of
// interest.  For example:
//..
//  int firstWeekendDay = bdlb::BitStringUtil::find1AtMinIndex(weekends,
//                                                             365 + 1);
//  int lastWeekendDay  = bdlb::BitStringUtil::find1AtMaxIndex(weekends,
//                                                             365 + 1);
//..
// We can define an enumeration to assist us in representing these DOY values
// into convention dates and confirm the calculated values:
//..
//  enum {
//      JAN = 0,  // Note: First DOY is 'JAN + 1'.
//      FEB = JAN + 31,
//      MAR = FEB + 28,
//      APR = MAR + 31,
//      MAY = APR + 30,
//      JUN = MAY + 31,
//      JUL = JUN + 30,
//      AUG = JUL + 31,
//      SEP = AUG + 31,
//      OCT = SEP + 30,
//      NOV = OCT + 31,
//      DEC = NOV + 30
//  };
//
//  assert(JAN +  5 == firstWeekendDay);
//  assert(DEC + 29 ==  lastWeekendDay);
//..
// The enumeration allows us to easily represent the business holidays of the
// year and significant dates in the business calendar:
//..
//  uint64_t holidays[6] = {0};
//
//  enum {
//      NEW_YEARS_DAY    = JAN +  1,
//      MLK_DAY          = JAN + 21,
//      PRESIDENTS_DAY   = FEB + 18,
//      GOOD_FRIDAY      = MAR + 29,
//      MEMORIAL_DAY     = MAY + 27,
//      INDEPENDENCE_DAY = JUL +  4,
//      LABOR_DAY        = SEP +  2,
//      THANKSGIVING     = NOV + 28,
//      CHRISTMAS        = DEC + 25
//  };
//
//  bdlb::BitStringUtil::assign(holidays, NEW_YEARS_DAY,    true);
//  bdlb::BitStringUtil::assign(holidays, MLK_DAY,          true);
//  bdlb::BitStringUtil::assign(holidays, PRESIDENTS_DAY,   true);
//  bdlb::BitStringUtil::assign(holidays, GOOD_FRIDAY,      true);
//  bdlb::BitStringUtil::assign(holidays, MEMORIAL_DAY,     true);
//  bdlb::BitStringUtil::assign(holidays, INDEPENDENCE_DAY, true);
//  bdlb::BitStringUtil::assign(holidays, LABOR_DAY,        true);
//  bdlb::BitStringUtil::assign(holidays, THANKSGIVING,     true);
//  bdlb::BitStringUtil::assign(holidays, CHRISTMAS,        true);
//
//  enum {
//      Q1 = JAN + 1,
//      Q2 = APR + 1,
//      Q3 = JUN + 1,
//      Q4 = OCT + 1
//  };
//..
// Now, we can query our calendar for the first holiday in the third quarter,
// if any:
//..
//  int firstHolidayOfQ3 = bdlb::BitStringUtil::find1AtMinIndex(holidays,
//                                                              Q3,
//                                                              Q4);
//  assert(INDEPENDENCE_DAY == firstHolidayOfQ3);
//..
//  Our calendars are readily combined to represent days off for either reason
//  (i.e., holiday or weekend):
//..
//  uint64_t allDaysOff[6] = {0};
//  bdlb::BitStringUtil::orEqual(allDaysOff, 1, weekends, 1, 365);
//  bdlb::BitStringUtil::orEqual(allDaysOff, 1, holidays, 1, 365);
//
//  bool isOffMay24 = bdlb::BitStringUtil::bit(allDaysOff, MAY + 24);
//  bool isOffMay25 = bdlb::BitStringUtil::bit(allDaysOff, MAY + 25);
//  bool isOffMay26 = bdlb::BitStringUtil::bit(allDaysOff, MAY + 26);
//  bool isOffMay27 = bdlb::BitStringUtil::bit(allDaysOff, MAY + 27);
//  bool isOffMay28 = bdlb::BitStringUtil::bit(allDaysOff, MAY + 28);
//
//  assert(false == isOffMay24);
//  assert(true  == isOffMay25);
//  assert(true  == isOffMay26);
//  assert(true  == isOffMay27);    // Note May 27, 2013 is Memorial Day.
//  assert(false == isOffMay28);
//..

#ifndef INCLUDED_BDLSCM_VERSION
#include <bdlscm_version.h>
#endif

#ifndef INCLUDED_BDLB_BITUTIL
#include <bdlb_bitutil.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

#ifndef INCLUDED_BSL_IOSFWD
#include <bsl_iosfwd.h>
#endif

namespace BloombergLP {
namespace bdlb {

                            // ====================
                            // struct BitStringUtil
                            // ====================

struct BitStringUtil {
    // This 'struct' provides a namespace for a suite of static functions to
    // manipulate sequences of bits represented as an array of 'int' (also
    // known as a 'BitString').

    // PUBLIC TYPES
    enum { k_BITS_PER_UINT64 = 64 };  // bits used to represent an 'uint64_t'

    typedef BitUtil::uint64_t    uint64_t;

    // CLASS METHODS

                                // Assign

    static void assign(uint64_t *bitString, int index, bool value);
        // Set the bit at the specified 'index' in the specified 'bitString' to
        // the specified 'value'.  The behavior is undefined unless
        // '0 <= index' and 'index' is less than the capacity of 'bitString'.

    static void assign(uint64_t *bitString,
                       int       index,
                       bool      value,
                       int       numBits);
        // Set the specified 'numBits' beginning at the specified 'index' in
        // the specified 'bitString' to the specified 'value'.  The behavior is
        // undefined unless '0 <= index', '0 <= numBits', and 'bitString' has a
        // capacity of at least 'index + numBits'.

    static void assign0(uint64_t *bitString, int index);
        // Set the bit at the specified 'index' in the specified 'bitString' to
        // 'false'.  The behavior is undefined unless '0 <= index' and 'index'
        // is less than the capacity of 'bitString'.

    static void assign0(uint64_t *bitString, int index, int numBits);
        // Set the specified 'numBits' beginning at the specified 'index' in
        // the specified 'bitString' to 'false'.  The behavior is undefined
        // unless '0 <= index', '0 <= numBits', and 'bitString' has a capacity
        // of at least 'index + numBits'.

    static void assign1(uint64_t *bitString, int index);
        // Set the bit at the specified 'index' in the specified 'bitString' to
        // 'true'.  The behavior is undefined unless '0 <= index' and 'index'
        // is less than the capacity of 'bitString'.

    static void assign1(uint64_t *bitString, int index, int numBits);
        // Set the specified 'numBits' beginning at the specified 'index' in
        // the specified 'bitString' to 'true'.  The behavior is undefined
        // unless '0 <= index', '0 <= numBits', and 'bitString' has a capacity
        // of at least 'index + numBits'.

    static void assignBits(uint64_t *bitString,
                           int       index,
                           uint64_t  srcBits,
                           int       numBits);
        // Assign the low-order specified 'numBits' from the specified
        // 'srcBits' to the specified 'bitString', starting at the specified
        // 'index'.  The behavior is undefined unless
        // '0 <= numBits <= k_BITS_PER_UINT64'.

                                // Bitwise-Logical

    static void andEqual(uint64_t       *dstBitString,
                         int             dstIndex,
                         const uint64_t *srcBitString,
                         int             srcIndex,
                         int             numBits);
        // Bitwise AND and load into the specified 'numBits' of the specified
        // 'dstBitString' starting at the specified 'dstIndex' the 'numBits' of
        // the specified 'srcBitString' starting at the specified 'srcIndex'.
        // The behavior is undefined unless '0 <= dstIndex', '0 <= srcIndex',
        // '0 <= numBits', 'dstBitString' has a length of at least
        // 'dstIndex + numBits', and 'srcBitString' has a length of at least
        // 'srcIndex + numBits'.

    static void minusEqual(uint64_t       *dstBitString,
                           int             dstIndex,
                           const uint64_t *srcBitString,
                           int             srcIndex,
                           int             numBits);
        // Bitwise MINUS and load into the specified 'numBits' of the specified
        // 'dstBitString' starting at the specified 'dstIndex' the 'numBits' of
        // the specified 'srcBitString' starting at the specified 'srcIndex'.
        // The behavior is undefined unless '0 <= dstIndex', '0 <= srcIndex',
        // '0 <= numBits', 'dstBitString' has a length of at least
        // 'dstIndex + numBits', and 'srcBitString' has a length of at least
        // 'srcIndex + numBits'.  Note that the logical difference 'A - B' is
        // defined to be 'A & !B'.

    static void orEqual(uint64_t       *dstBitString,
                        int             dstIndex,
                        const uint64_t *srcBitString,
                        int             srcIndex,
                        int             numBits);
        // Bitwise OR and load into the specified 'numBits' of the specified
        // 'dstBitString' starting at the specified 'dstIndex' the 'numBits' of
        // the specified 'srcBitString' starting at the specified 'srcIndex'.
        // The behavior is undefined unless '0 <= dstIndex', '0 <= srcIndex',
        // '0 <= numBits', 'dstBitString' has a length of at least
        // 'dstIndex + numBits', and 'srcBitString' has a length of at least
        // 'srcIndex + numBits'.

    static void xorEqual(uint64_t       *dstBitString,
                         int             dstIndex,
                         const uint64_t *srcBitString,
                         int             srcIndex,
                         int             numBits);
        // Bitwise XOR and load into the specified 'numBits' of the specified
        // 'dstBitString' starting at the specified 'dstIndex' the 'numBits' of
        // the specified 'srcBitString' starting at the specified 'srcIndex'.
        // The behavior is undefined unless '0 <= dstIndex', '0 <= srcIndex',
        // '0 <= numBits', 'dstBitString' has a length of at least
        // 'dstIndex + numBits', and 'srcBitString' has a length of at least
        // 'srcIndex + numBits'.

                                // Copy

    static void copy(uint64_t       *dstBitString,
                     int             dstIndex,
                     const uint64_t *srcBitString,
                     int             srcIndex,
                     int             numBits);
        // Copy to the specified 'dstBitString', beginning at the specified
        // 'dstIndex', from the specified 'srcBitString', beginning at the
        // specified 'srcIndex', the specified 'numBits'.  The behavior is
        // undefined unless '0 <= dstIndex', '0 <= srcIndex', 'dstBitString'
        // has a capacity of at least 'dstIndex + numBits', and 'srcBitString'
        // has a length of at least 'srcIndex + numBits'.  Note that even if
        // there is an overlap between the source and destination ranges the
        // resulting bits in the destination range will match the bits
        // originally in the source range (as if the bits had been copied into
        // a temporary buffer and then copied into the destination range).  See
        // 'copyRaw'.

    static void copyRaw(uint64_t       *dstBitString,
                        int             dstIndex,
                        const uint64_t *srcBitString,
                        int             srcIndex,
                        int             numBits);
        // Copy to the specified 'dstBitString', beginning at the specified
        // 'dstIndex', from the specified 'srcBitString', beginning at the
        // specified 'srcIndex', the specified 'numBits'.  The behavior is
        // undefined unless '0 <= dstIndex', '0 <= srcIndex', 'dstBitString'
        // has a capacity of at least 'dstIndex + numBits', and 'srcBitString'
        // has a length of at least 'srcIndex + numBits'.  Note that if there
        // is an overlap between the source and destination ranges the
        // resulting bits in the destination range may not match the bits
        // originally in the source range.  See 'copy'.

                                // Insert / Remove

    static void insert(uint64_t *bitString,
                       int       initialLength,
                       int       dstIndex,
                       bool      value,
                       int       numBits);
        // Insert the specified 'numBits', each having the specified 'value',
        // into the specified 'bitString' having the specified 'initialLength',
        // beginning at the specified 'dstIndex'.  Bits at or above 'dstIndex'
        // are shifted up by 'numBits' index positions.  The behavior is
        // undefined unless '0 <= numBits', '0 <= dstIndex <= initialLength',
        // and 'bitString' has a capacity of at least
        // 'initialLength + numBits'.

    static void insert0(uint64_t *bitString,
                        int       initialLength,
                        int       dstIndex,
                        int       numBits);
        // Insert the specified 'numBits' 0 bits into the specified 'bitString'
        // having the specified 'initialLength' beginning at the specified
        // 'dstIndex'.  Bits at or above 'dstIndex' are shifted up by 'numBits'
        // index positions.  The behavior is undefined unless '0 <= numBits',
        // '0 <= dstIndex <= initialLength', and 'bitString' has a capacity of
        // at least 'initialLength + numBits'.

    static void insert1(uint64_t *bitString,
                        int       initialLength,
                        int       dstIndex,
                        int       numBits);
        // Insert the specified 'numBits' 1 bits into the specified 'bitString'
        // having the specified 'initialLength' beginning at the specified
        // 'dstIndex'.  Bits at or above 'dstIndex' are shifted up by 'numBits'
        // index positions.  The behavior is undefined unless '0 <= numBits',
        // '0 <= dstIndex <= initialLength', and 'bitString' has a capacity of
        // at least 'initialLength + numBits'.

    static void insertRaw(uint64_t *bitString,
                          int       initialLength,
                          int       dstIndex,
                          int       numBits);
        // Insert the specified 'numBits' into the specified 'bitString' having
        // the specified 'initialLength' beginning at the specified 'dstIndex'.
        // Bits at or above 'dstIndex' are shifted up by 'numBits' index
        // positions.  The behavior is undefined unless '0 <= numBits',
        // '0 <= dstIndex <= initialLength', and 'bitString' has a capacity of
        // at least 'initialLength + numBits'.  Note that the inserted bits are
        // not assigned any value.

    static void remove(uint64_t *bitString,
                       int       length,
                       int       index,
                       int       numBits);
        // Remove the specified 'numBits' from the specified 'bitString' of
        // the specified 'length' beginning at the specified 'index'.  Bits
        // above 'index + numBits' are shifted down by 'numBits' index
        // positions and the length of 'bitString' is reduced by 'numBits'.
        // The behavior is undefined unless '0 <= length', '0 <= index',
        // '0 <= numBits', and 'index + numBits <= length'.  Note that
        // the value of the removed high-order bits is not modified.

    static void removeAndFill0(uint64_t *bitString,
                               int       length,
                               int       index,
                               int       numBits);
        // Remove the specified 'numBits' from the specified 'bitString' having
        // the specified 'length' beginning at the specified 'index'.  Bits
        // above 'index + numBits' are shifted down by 'numBits' index
        // positions and the last 'numBits' of 'bitString' are set to 0.  The
        // behavior is undefined unless '0 <= length', '0 <= index',
        // '0 <= numBits', and 'index + numBits <= length'.  Note that the
        // length of 'bitString' is not changed.

                                // Other Manipulators

    static void swapRaw(uint64_t *bitString1,
                        int       index1,
                        uint64_t *bitString2,
                        int       index2,
                        int       numBits);
        // Exchange the specified 'numBits' beginning at the specified 'index1'
        // in the specified 'bitString1' with the 'numBits' beginning at the
        // specified 'index2' in the specified 'bitString2'.  The behavior is
        // undefined unless '0 <= index1', '0 <= index2', '0 <= numBits',
        // 'bitString1' has a length of at least 'index1 + numBits',
        // 'bitString2' has a length of at least 'index2 + numBits', and there
        // is *no* overlap between the swapped ranges of bits.

    static void toggle(uint64_t *bitString, int index, int numBits);
        // Invert the values of the specified 'numBits' in the specified
        // 'bitString' beginning at the specified 'index'.  The behavior is
        // undefined unless '0 <= index', '0 <= numBits', and 'bitString' has a
        // length of at least 'index + numBits'.

                                // Compare

    static bool areEqual(const uint64_t *bitString1,
                         int             index1,
                         const uint64_t *bitString2,
                         int             index2,
                         int             numBits);
        // Return 'true' if the specified 'numBits' beginning at the specified
        // 'index1' in the specified 'bitString1' are bitwise equal to the
        // 'numBits' beginning at the specified 'index2' in the specified
        // 'bitString2', and 'false' otherwise.  The behavior is undefined
        // unless '0 <= index1', '0 <= index2', '0 <= numBits', 'bitString1'
        // has a length of at least 'index1 + numBits', and 'bitString2' has a
        // length of at least 'index2 + numBits'.

    static bool areEqual(const uint64_t *bitString1,
                         const uint64_t *bitString2,
                         int             numBits);
        // Return 'true' if the first specified 'numBits' in the specified
        // 'bitString1' match the corresponding bits in the specified
        // 'bitString2'.

                                // Read

    static bool bit(const uint64_t *bitString, int index);
        // Return the bit value at the specified 'index' in the specified
        // 'bitString'.  The behavior is undefined unless '0 <= index' and
        // 'index' is less than the length of 'bitString'.

    static uint64_t bits(const uint64_t *bitString, int index, int numBits);
        // Return the specified 'numBits' beginning at the specified 'index' in
        // the specified 'bitString' as the low-order bits of the returned
        // value.  The behavior is undefined unless '0 <= index',
        // '0 <= numBits <= sizeof(uint64_t) * CHAR_BIT', and 'bitString' has a
        // length of at least 'index + numBits'.

                                // Find

    static int find0AtMaxIndex(const uint64_t *bitString, int length);
    static int find1AtMaxIndex(const uint64_t *bitString, int length);
        // Return the index of the most-significant 0 (for 'find0*') or 1 (for
        // 'find1*') bit in the specified 'bitString' having the specified
        // 'length', if such a bit exists, and a negative value otherwise.  The
        // behavior is undefined unless '0 <= length'.

    static int find0AtMaxIndex(const uint64_t *bitString, int begin, int end);
    static int find1AtMaxIndex(const uint64_t *bitString, int begin, int end);
        // Return the index of the most-significant 0 (for 'find0*') or 1 (for
        // 'find1*') bit in the specified 'bitString' in the specified range
        // '[ begin, end )', if such a bit exists, and a negative value
        // otherwise.  The behavior is undefined unless '0 <= begin' and
        // 'begin <= end < (length of bit string)'.

    static int find0AtMinIndex(const uint64_t *bitString, int length);
    static int find1AtMinIndex(const uint64_t *bitString, int length);
        // Return the index of the least-significant 0 (for 'find0*') or 1 (for
        // 'find1*') bit in the specified 'bitString' having the specified
        // 'length', if such a bit exists, and a negative value otherwise.  The
        // behavior is undefined unless '0 <= length'.

    static int find0AtMinIndex(const uint64_t *bitString, int begin, int end);
    static int find1AtMinIndex(const uint64_t *bitString, int begin, int end);
        // Return the index of the least-significant 0 (for 'find0*') or 1 (for
        // 'find1*') bit in the specified 'bitString' in the specified range
        // '[ begin, end )', if such a bit exists, and a negative value
        // otherwise.  The behavior is undefined unless '0 <= begin' and
        // 'begin <= end < (length of bit string)'.

                                // Count

    static bool isAny0(const uint64_t *bitString, int index, int numBits);
        // Return 'true' if any of the specified 'numBits' beginning at the
        // specified 'index' in the specified 'bitString' are 0, and 'false'
        // otherwise.  The behavior is undefined unless '0 <= numBits',
        // '0 <= index', and 'bitString' has a length of at least
        // 'index + numBits'.

    static bool isAny1(const uint64_t *bitString, int index, int numBits);
        // Return 'true' if any of the specified 'numBits' beginning at the
        // specified 'index' in the specified 'bitString' are 1, and 'false'
        // otherwise.  The behavior is undefined unless '0 <= numBits',
        // '0 <= index', and 'bitString' has a length of at least
        // 'index + numBits'.

    static int num0(const uint64_t *bitString, int index, int numBits);
        // Return the number of 0 bits in the specified 'numBits' beginning at
        // the specified 'index' in the specified 'bitString'.  The behavior is
        // undefined unless '0 <= index', '0 <= numBits', and 'bitString' has a
        // length of at least 'index + numBits'.

    static int num1(const uint64_t *bitString, int index, int numBits);
        // Return the number of 1 bits in the specified 'numBits' beginning at
        // the specified 'index' in the specified 'bitString'.  The behavior is
        // undefined unless '0 <= index', '0 <= numBits', and 'bitString' has a
        // length of at least 'index + numBits'.

                                // Printing

    static bsl::ostream& print(bsl::ostream&   stream,
                               const uint64_t *bitString,
                               int             numBits,
                               int             level          = 1,
                               int             spacesPerLevel = 4);
        // Format to the specified output 'stream' the specified 'numBits' in
        // the specified 'bitString' in hexadecimal and return a reference to
        // the modifiable 'stream'.  The bit values at increasing indexes are
        // formatted from right to left within a line, and then with increasing
        // indexes on subsequent lines.  Optionally specify 'level', the
        // indentation level for each line output.  Optionally specify
        // 'spacesPerLevel', the number of spaces per indentation level.  Each
        // line is indented by the absolute value of 'level * spacesPerLevel'.
        // If 'spacesPerLevel' is negative, suppress line breaks and format the
        // entire output on one line.  If 'stream' is initially invalid, this
        // operation has no effect.  Note that a trailing newline is provided
        // in multiline mode only.
};

// ============================================================================
//                              INLINE DEFINITIONS
// ============================================================================

                            // --------------------
                            // struct BitStringUtil
                            // --------------------

// CLASS METHODS

                            // Manipulators

                                // Assign

inline
void BitStringUtil::assign(uint64_t *bitString, int index, bool value)
{
    BSLS_ASSERT_SAFE(0 <= index);

    const int idx = index / k_BITS_PER_UINT64;
    const int pos = index % k_BITS_PER_UINT64;

    if (value) {
        bitString[idx] |=  (1ULL << pos);
    }
    else {
        bitString[idx] &= ~(1ULL << pos);
    }
}

inline
void BitStringUtil::assign0(uint64_t *bitString, int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    const int idx = index / k_BITS_PER_UINT64;
    const int pos = index % k_BITS_PER_UINT64;

    bitString[idx] &= ~(1ULL << pos);
}

inline
void BitStringUtil::assign1(uint64_t *bitString, int index)
{
    BSLS_ASSERT_SAFE(0 <= index);

    const int idx = index / k_BITS_PER_UINT64;
    const int pos = index % k_BITS_PER_UINT64;

    bitString[idx] |= 1ULL << pos;
}

                                // Insert / Remove

inline
void BitStringUtil::insert(uint64_t *bitString,
                           int       initialLength,
                           int       dstIndex,
                           bool      value,
                           int       numBits)
{
    // preconditions checked by 'insertRaw'

    insertRaw(bitString, initialLength, dstIndex, numBits);
    assign(bitString, dstIndex, value, numBits);
}

inline
void BitStringUtil::insert0(uint64_t *bitString,
                            int       initialLength,
                            int       dstIndex,
                            int       numBits)
{
    // preconditions checked by 'insertRaw'

    insertRaw(bitString, initialLength, dstIndex, numBits);
    assign0(bitString, dstIndex, numBits);
}

inline
void BitStringUtil::insert1(uint64_t *bitString,
                            int       initialLength,
                            int       dstIndex,
                            int       numBits)
{
    // preconditions checked by 'insertRaw'

    insertRaw(bitString, initialLength, dstIndex, numBits);
    assign1(bitString, dstIndex, numBits);
}

inline
void BitStringUtil::removeAndFill0(uint64_t *bitString,
                                   int       length,
                                   int       index,
                                   int       numBits)
{
    // preconditions checked by 'remove'

    remove(bitString, length, index, numBits);
    assign0(bitString, length - numBits, numBits);
}

                                // Accessors

                                // Read
inline
bool BitStringUtil::bit(const uint64_t *bitString, int index)
{
    BSLS_ASSERT(0 <= index);

    const int idx = index / k_BITS_PER_UINT64;
    const int pos = index % k_BITS_PER_UINT64;

    return bitString[idx] & (1ULL << pos);
}

                                // Count

inline
int BitStringUtil::num0(const uint64_t *bitString, int index, int numBits)
{
    // Asserts will be performed by 'num1'.

    return numBits - num1(bitString, index, numBits);
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
