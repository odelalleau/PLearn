// -*- C++ -*-
//
// ArrayAllocatorTrivial.h
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


/* *******************************************************
 * * $Id: ArrayAllocatorTrivial.h,v 1.2 2003/03/06 18:26:01 ducharme Exp $
 * ******************************************************* */


#ifndef ARRAYALLOCATORTRIVIAL_H
#define ARRAYALLOCATORTRIVIAL_H


#include <vector>
#include <algorithm>     //!<  for swap
#include "general.h"
#include "ArrayAllocatorIndex.h"

namespace PLearn <%
using namespace std;



//######################  CLASS  ARRAYALLOCATORTRIVIAL  #######################
//!  
//!  This allocator solely performs allocation.

template <class T, unsigned SizeBits>
class ArrayAllocatorTrivial
{
public:
    typedef ArrayAllocatorTrivial<T,SizeBits> self;
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;

    typedef unsigned index_base;
    typedef ArrayAllocatorIndex<index_base, SizeBits> index_type;

public:
    ArrayAllocatorTrivial(unsigned numberObjects);
    //!  Default copy ctor, dtor, op=

    //!  Allocate N objects of type T and return a (memory) pointer to it
    pointer allocate(size_t n);

    //!  "Deallocate" pointer (no-op)
    void deallocate(pointer p, size_type n) {}
    void deallocate(index_type) {}

    //!  Change the maximum number of objects
    void resize(size_type newMaxObjs);
    size_type max_size() const {
      return arr.size();
    }

    //!  Interchange two allocators
    void swap(self&);

public:
    //!  Index/pointer conversion
    inline index_type toIndex(pointer p, size_type n);
    inline pointer toPointer(index_type);
    
private:
    vector<T> arr;
    index_base free_point;
};



//#####  Implementation  ######################################################

template <class T, unsigned SizeBits>
ArrayAllocatorTrivial<T,SizeBits>::ArrayAllocatorTrivial(unsigned numberObjects)
    : arr(numberObjects), free_point(1)
{}


template <class T, unsigned SizeBits>
void ArrayAllocatorTrivial<T,SizeBits>::resize(size_type newMaxObjs)
{
  arr.resize(newMaxObjs);
  free_point = max(1, min(free_point, arr.size()));
}


template <class T, unsigned SizeBits>
T* ArrayAllocatorTrivial<T,SizeBits>::allocate(size_t n)
{
  if(free_point + n > arr.size()) {
    PLERROR("Size of memory pool exceeded (%d objects)", arr.size());
    return 0;
  }
  T* p = &arr[free_point];
  free_point += n;
  return p;
}


template <class T, unsigned SizeBits>
void ArrayAllocatorTrivial<T,SizeBits>::swap(self& other)
{
  arr.swap(other);
  swap(free_point, other.free_point);
}


template <class T, unsigned SizeBits>
inline ArrayAllocatorTrivial<T,SizeBits>::index_type
ArrayAllocatorTrivial<T,SizeBits>::toIndex(pointer p, size_type n)
{
  if (p)
    return index_type(p-&arr[0], n);
  else
    return index_type(0,0);
}


template <class T, unsigned SizeBits>
inline ArrayAllocatorTrivial<T,SizeBits>::pointer
ArrayAllocatorTrivial<T,SizeBits>::toPointer(index_type i)
{
  if (i.isNull())
    return 0;
  else
    return &arr[0] + i.index;
}

%> // end of namespace PLearn


#endif
