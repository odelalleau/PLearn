// -*- C++ -*-
//
// ArrayAllocator.h
//
// Copyright (C) 2000 Nicolas Chapados
// 
// Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
// 
//    1. Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     
//    2. Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//       
//    3. The name of the authors may not be used to endorse or promote
//       products derived from this software without specific prior written
//      permission. 
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


#ifndef ARRAYALLOCATOR_H
#define ARRAYALLOCATOR_H


#include <vector>
#include <algorithm>   //!<  for swap
#include "general.h"
#include "ArrayAllocatorIndex.h"

namespace PLearn <%
using namespace std;



//######################  CLASS  ARRAYALLOCATOROPTIONS  #######################
/*!   
  This class provides a unique stop to set all options in an
  ArrayAllocator object.  These options must be passed upon constructing
  the object.
*/

template <class T, class Enclosing>
class Option
{
public:
    typedef T value_type;
    typedef Enclosing enclosing_type;
    
public:
    Option(const T& defaultValue, Enclosing* encl)
      : optionValue(defaultValue), enclosing(encl)
      {}

    Enclosing& operator()(const T& newValue) {
      optionValue = newValue;
      return *enclosing;
    }
    const T& operator()() const {
      return optionValue;
    }
    
private:
    T optionValue;
    Enclosing* enclosing;
};


class ArrayAllocatorOptions
{
    typedef ArrayAllocatorOptions self;
    
public:
    //!  Default values
    ArrayAllocatorOptions()
      : numObjs(100,this),
    deallocatorType(DeallocatorUnsorted,this)
    { }
    
    //!  Set number of objects in the array
    Option<size_t, self> numObjs;

    //!  Set the type of deallocator to use
    enum DeallocatorType {
      DeallocatorNull = 0,
      DeallocatorUnsorted = 1,
      DeallocatorSorted = 2
    };
    Option<DeallocatorType, self> deallocatorType;
};



//##########################  CLASS  ARRAYALLOCATOR  ##########################
/*!   
  This class provides an allocator interface for objects of type T.  A
  maximum of InitialObjs (specified upon construction) can be maintained
  by the allocator.  Arrays of such objects are supported; the maximum
  length of an array of objects (in bits) is given by the SizeBits
  template argument.
  
  Furthermore, the maximum number of objects that can be stored (in bits)
  is 8 * sizeof(index_base) - SizeBits, where 8*sizeof(index_base) is
  usually 32 for most computers.  Hence, if arrays of a maximum size of
  256 elements are to be stored (8 bits), then only 16777216 elements can
  be allocated.
  
  Internally, index number 0 is reserved to denote a null pointer, and not
  otherwise used in the array.
  
  The free list is represented as follows: the variable free_root
  represents the index of the first free block (the size field of
  free_root is not used).  In the memory location at that index, we assume
  that we have an index_type structure (instead of a T), and this
  index_type contains two things: the index of the next free block (or
  zero if none), along with the size of the current free block.
*/

template <class T, unsigned SizeBits>
class ArrayAllocator
{
public:
    typedef ArrayAllocator<T,SizeBits> self_type;
    typedef T value_type;
    typedef unsigned index_base;
    typedef ArrayAllocatorIndex<index_base, SizeBits> index_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;

public:
    ArrayAllocator(const ArrayAllocatorOptions&);
    //!  default copy constructor, destructor, assignment operator

    //!  Allocate N objects of type T and return a (memory) pointer to it
    pointer allocate(size_t n);

    //!  Deallocate given pointer/size, or given index only 
    void deallocate(pointer p, size_type n);
    void deallocate(index_type);

    void resize(size_type NewMaxObjs);
    size_type max_size() const {
      return arr.size();
    }
    
public:
    //!  Index/pointer conversion
    inline index_type toIndex(pointer p, size_type n);
    inline pointer toPointer(index_type);
    
public:
    void swap(self_type&);
    
private:
    //!  Small utility
    index_type& derefIndex(index_type i) {
      return (index_type&)arr[i.index];
    }
    index_type& derefIndex(unsigned i) {
      return (index_type&)arr[i];
    }
    
private:
    vector<T> arr;
    index_type free_root;
    ArrayAllocatorOptions options;
};



//#####  Implementation  ######################################################

template <class T, unsigned SizeBits>
ArrayAllocator<T,SizeBits>::ArrayAllocator(const ArrayAllocatorOptions& opt)
    : arr(0), free_root(0,0), options(opt)
{
  resize(options.numObjs());
}


template <class T, unsigned SizeBits>
void ArrayAllocator<T,SizeBits>::resize(size_type new_max_objs)
{
/*!       Since the maximum block size is limited to SizeBits, we cannot
      create a single big new free region with the new objects; instead,
      we must create several free regions of maximum size and link them
      together in the free list.
*/

  size_t next_free_block = arr.size();
  if (new_max_objs <= next_free_block)
    return;

  //!  zero is not a valid starting index
  next_free_block = max(1,int(next_free_block));

  arr.resize(new_max_objs);
  index_type dummy(unsigned(-1), unsigned(-1));
  size_t max_block_size = dummy.size;
    
  while (next_free_block < new_max_objs) {
    size_t block_size = min(max_block_size, new_max_objs - next_free_block);

    index_type new_index(free_root.index, block_size);
    derefIndex(next_free_block) = new_index;
    free_root = index_type(next_free_block,0);

    next_free_block += block_size;
  }
}


template <class T, unsigned SizeBits>
ArrayAllocator<T,SizeBits>::pointer
ArrayAllocator<T,SizeBits>::allocate(size_t n)
{
  index_type *free_pointer = &free_root;

/*!       Take a look at the block pointed to by free_pointer, and if it's big
      enough, take a chunk from the BEGINNING, and modify *free_pointer to
      point to the next correct free block
*/
  while (free_pointer && !free_pointer->isNull()) {
    unsigned size = derefIndex(*free_pointer).size;

    if (size < n)
      free_pointer = &derefIndex(*free_pointer);
    else
      if (size == n) {
        pointer p = toPointer(*free_pointer);
        *free_pointer = index_type(derefIndex(*free_pointer).index,
            free_pointer->size);
        return p;
      }
      else {
        pointer p = toPointer(*free_pointer);
        derefIndex(free_pointer->index + n) =
          index_type(derefIndex(*free_pointer).index, size - n);
        *free_pointer = index_type(free_pointer->index + n,
            free_pointer->size);
        return p;
      }
  }
  PLERROR("Could not allocate; size of memory pool exhausted\n%s\n",
      (typeid(*this).name()));
  return 0;				     //!<  could not allocate
}


template <class T, unsigned SizeBits>
void ArrayAllocator<T,SizeBits>::deallocate(pointer p, size_type n)
{
  deallocate(index_type(p,n));
}


template <class T, unsigned SizeBits>
void ArrayAllocator<T,SizeBits>::deallocate(index_type i)
{
  //!  The current code is frankly not too object-oriented...
  switch(options.deallocatorType()) {
    case ArrayAllocatorOptions::DeallocatorNull :
      //!  Null deallocator: do nothing
      break;

    case ArrayAllocatorOptions::DeallocatorUnsorted :
      //!  Unsorted deallocator: just add block to free list; don't
      //!  collapse adjacent free blocks
      if (! i.isNull()) {
        derefIndex(i) = index_type(free_root.index, i.size);
        free_root.index = i.index;
      }
      break;

    case ArrayAllocatorOptions::DeallocatorSorted :
      //!  Not implemented yet...
      PLERROR("ArrayAllocator: the sorted deallocator is not currently "
          "implemented");
      break;
  }
}


template <class T, unsigned SizeBits>
inline ArrayAllocator<T,SizeBits>::index_type
ArrayAllocator<T,SizeBits>::toIndex(pointer p, size_type n)
{
  if (p)
    return index_type(p-&arr[0], n);
  else
    return index_type(0,0);
}


template <class T, unsigned SizeBits>
inline ArrayAllocator<T,SizeBits>::pointer
ArrayAllocator<T,SizeBits>::toPointer(index_type i)
{
  if (i.isNull())
    return 0;
  else
    return &arr[0] + i.index;
}


template <class T, unsigned SizeBits>
void ArrayAllocator<T,SizeBits>::swap(self_type& other)
{
  swap(arr, other.arr);
  swap(free_root, other.free_root);
}

%> // end of namespace PLearn


#endif
