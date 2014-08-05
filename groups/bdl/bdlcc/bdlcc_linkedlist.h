// bdlcc_linkedlist.h                                                    -*-C++-*-
#ifndef INCLUDED_BDLCC_TESTUTIL
#define INCLUDED_BDLCC_TESTUTIL

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide test utilities for components in 'bdl' and above.
//
//@CLASSES:
//
//@SEE ALSO:
//
//@DESCRIPTION:
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Writing a test driver
/// - - - - - - - - - - - - - - - -

#ifndef INCLUDED_BDLSCM_VERSION
#include <bdlscm_version.h>
#endif

#ifndef INCLUDED_BSLS_ATOMIC
#include <bsls_atomic.h>
#endif

#ifndef INCLUDED_BSL_OSTREAM
#include <bsl_ostream.h>
#endif

#include <bsl_iostream.h>

namespace BloombergLP {
namespace bdlcc {

struct Node {
     int                       d_value;
	 bsls::AtomicPointer<Node> d_next;
	 
	 Node() : d_value(0), d_next(0) {}
	 
	 Node(const Node& original) : d_value(original.d_value), d_next(0) { 
		BSLS_ASSERT(0 == original.d_next.loadRelaxed());
	 }
	 
	 Node& operator=(const Node& original)  { 
		BSLS_ASSERT(0 == original.d_next.loadRelaxed());
		d_value = original.d_value;
		d_next  = 0;
		return *this;
	 }
};

class List {

	// DATA
	Node d_head;
	
	static const bsl::size_t k_DELETED_MASK = ~static_cast<bsl::size_t>(1);
	
	static bool isNodeMarkedDeleted(const Node *node) {
		return isDeletionBitSet(node->d_next.loadRelaxed());
	}

	static bool isDeletionBitSet(const Node *ptr) {
		return reinterpret_cast<bsl::size_t>(ptr) & 1;
	}
	
	static Node *setDeletionBit(Node *nextAddress) {
		return reinterpret_cast<Node *>(reinterpret_cast<bsl::size_t>(nextAddress) | 1);
	}

	
	static Node *maskDeletedBit(Node *nextAddress) {
		return reinterpret_cast<Node *>(reinterpret_cast<bsl::size_t>(nextAddress) & k_DELETED_MASK);
	}
	
	static const Node *maskDeletedBit(const Node *nextAddress) {
		return reinterpret_cast<const Node *>(reinterpret_cast<bsl::size_t>(nextAddress) & k_DELETED_MASK);
	}
	
  public:
	List() { d_head.d_value = -1; }
	
	void insert(Node *node) {
		Node *current    = &d_head;
		Node *next       = current->d_next.loadRelaxed();
		Node *lowerBound = next;
		while (true) {			
			while (lowerBound) {
				if (node->d_value < lowerBound->d_value) {
					break;
				}				
				current    = lowerBound;
				next       = current->d_next.loadRelaxed();
				lowerBound = next;
				while (lowerBound && isNodeMarkedDeleted(lowerBound)) {
					lowerBound = maskDeletedBit(lowerBound->d_next.loadRelaxed());
				}
			}			
			node->d_next = lowerBound;
			Node *prevNext = current->d_next.testAndSwapAcqRel(next, node);
			if (prevNext == next) {
				break; 
			}
			next = prevNext;			
		}		
	}
	
	Node *remove(Node *node) {
		Node *current = &d_head;
		Node *next    = current->d_next.loadRelaxed();
		
		while (next && next != node) {
			current = next;
			next    = maskDeletedBit(next->d_next.loadRelaxed());
		}
		
		while (true) {			
			Node *nextNextPtr = next->d_next.loadRelaxed();
			if (0 == next || isDeletionBitSet(nextNextPtr)) {
				// Node has already been removed.				
				return 0;
			}
						
			Node *prevNextNextPtr = 
				next->d_next.testAndSwapAcqRel(nextNextPtr, setDeletionBit(nextNextPtr));
				
			if (prevNextNextPtr == nextNextPtr) {
				return node;
			}
			nextNextPtr = prevNextNextPtr;
		}		
	}
	
	void debugPrint(const Node *nextPtr)  {
		if (0 == nextPtr) {
			bsl::cout << "NULL" << bsl::endl;
			return;
		}
		
		if (isNodeMarkedDeleted(nextPtr)) {
			bsl::cout << "x";
		}
		bsl::cout << maskDeletedBit(nextPtr)->d_value << bsl::endl;
	}
	
	bsl::ostream& print(bsl::ostream& stream) const {
		stream << "[";
		const Node *current = &d_head;
		while (current) {
			if (isNodeMarkedDeleted(current)) {
				stream << "x";
			}
			stream << current->d_value << " ";
			current = maskDeletedBit(current->d_next.loadRelaxed());
		}
		stream << "]" << bsl::endl;
		
		
		return stream;
	}
};
// ============================================================================
//                      INLINE FUNCTION DEFINITIONS
// ============================================================================

}  // close package namespace
}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright (C) 2012 Bloomberg Finance L.P.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
