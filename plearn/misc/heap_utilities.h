// -*- C++ -*-

// heap_utilities.h
//
// Copyright (C) 2005 Nicolas Chapados 
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org

/* *******************************************************      
   * $Id: heap_utilities.h,v 1.1 2005/01/06 16:30:21 chapados Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file heap_utilities.h */


#ifndef heap_utilities_INC
#define heap_utilities_INC

// From C++ stdlib
#include <algorithm>
#include <iterator>
#include <functional>


namespace PLearn {

/**
 * This function restores the heap condition for a heap that has been
 * "damaged" (i.e. for which the heap condition might be locally
 * violated).  The heap is contained between [first,last), and the damaged
 * object is pointed to by 'damaged'.
 *
 * Definition of the heap condition: for any object and its two children
 * (child1 and child2), comp(object,childi) is false.
 */
template <typename RandomAccessIterator, typename StrictWeakOrdering>
void
update_heap(RandomAccessIterator first, RandomAccessIterator last,
            RandomAccessIterator damaged, StrictWeakOrdering comp)
{
  typedef typename std::iterator_traits<RandomAccessIterator>::difference_type
    difference_type;
  difference_type cur = damaged-first;
  difference_type size = last-first;
  typename
    std::iterator_traits<RandomAccessIterator>::value_type value = *damaged;

  // Push down as much as possible, then push up
  difference_type second_child = 2*cur+2;
  while (second_child < size) {              // there are two children
    if (comp(first[second_child], first[second_child-1]))
      second_child--;                        // take larger child
    first[cur] = first[second_child];        // fill hole
    cur = second_child;
    second_child = 2 * (second_child+1);
  }
  if (second_child == size) {
    first[cur] = first[second_child-1];      // hole is in first child
    cur = second_child-1;
  }
  first[cur] = value;
  push_heap(first,first+cur+1,comp);
}


/**
 * Version of update_heap that uses less<T> as the strict weak ordering
 */
template <typename RandomAccessIterator>
void
update_heap(RandomAccessIterator first, RandomAccessIterator last,
            RandomAccessIterator damaged)
{
  typedef typename std::iterator_traits<RandomAccessIterator>::value_type value_type;
  update_heap(first,last,damaged, std::less<value_type>());
}


/**
 * Verify that the heap condition is satisfied.  Similar to the
 * non-standard 'is_heap' function of the SGI STL implementation.  Return
 * true if the heap is valid.
 */
template <typename RandomAccessIterator, typename StrictWeakOrdering>
bool is_valid_heap(RandomAccessIterator first, RandomAccessIterator last,
                   StrictWeakOrdering comp)
{
  if (first==last)
    return true;

  RandomAccessIterator parent = first;
  int i=1;
  for (RandomAccessIterator it = first+1 ; it < last ; ++it, ++i) {
    if (comp(*parent, *it))
      return false;
    if ((i & 1) == 0)
      ++parent;
  }
  return true;
}


//! Version of is_valid_heap that uses less<T> as the strict weak ordering
template <typename RandomAccessIterator>
bool is_valid_heap(RandomAccessIterator first, RandomAccessIterator last)
{
  typedef typename std::iterator_traits<RandomAccessIterator>::value_type value_type;
  return is_valid_heap(first,last,std::less<value_type>());
}


} // end namespace PLearn

#endif
