// -*- C++ -*-

// SmallVector.h: Small vectors
// Copyright (c) 2000 by Nicolas Chapados

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


/*! \file SmallVector.h */

#ifndef SMALLVECTOR
#define SMALLVECTOR

#include <typeinfo>
#include <algorithm>

#include "ArrayAllocatorTrivial.h"
#include "general.h"

namespace PLearn {
using namespace std;



//###########################  CLASS  SMALLVECTOR  ############################
/*!   
  A small vector is designed to have a predetermined maximum fixed size,
  specified as a template arguments (with SizeBits; e.g. SizeBits=8 for
  maximum size=256).  It is built over an ArrayAllocator.  These vectors
  trade representational efficiency for resizing performance (i.e. they
  always require a copy to resize).
*/

template <class T, unsigned SizeBits,
          class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
class SmallVector
{
public:
    //!  Typedefs
    typedef SmallVector<T,SizeBits> self_type;
    typedef Allocator alloc_type;
    
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef T* iterator;
    typedef const T* const_iterator;
	
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;

public:
    //!  Iterators
    inline iterator begin();
    inline const_iterator begin() const;
    inline iterator end();
    inline const_iterator end() const;

public:
    //!  Unchecked element access
    inline reference operator[](size_type n);
    inline const_reference operator[](size_type n) const;

    //!  Checked element access
    reference at(size_type n);
    const_reference at(size_type n) const;

    inline reference front();	     //!<  first element
    inline const_reference front() const;
    inline reference back();	     //!<  last element
    inline const_reference back() const;

public:
    //!  Constructors, etc.
    inline SmallVector();
    inline SmallVector(size_type n, const T& val=T());
    ~SmallVector();
    SmallVector(const self_type&);
    self_type& operator=(const self_type&);

    //!  Construct/Copy from Input iterator
    template <class In>
    SmallVector(In first, In last) : i(0,0) {
        assign(first,last);
    }
	
    template <class In>
    void assign(In first, In last) {
        //!  could not define it out-of-line; bug in gcc?
        resize(last-first);
        iterator dest = begin();
        while (first != last)
            *dest++ = *first++;
    }
	
    inline void assign(size_type n, const T& val);	//!<  n copies of val

public:
    //!  Stack operations 
    void push_back(const T& x);		     //!<  add to end
    void pop_back();			     //!<  remove last element

public:
    //!  (list operations are NOT provided at the moment)

public:
    //!  Size and capacity operations
    size_type size() const;		     //!<  number of elements
    bool empty() const {
        return size() == 0;
    }
    size_type max_size() {
        typename alloc_type::index_type dummy(unsigned(-1), unsigned(-1));
        return dummy.size;
    }
    void resize(size_type sz, const T& val=T()); //!<  added elts init by val
    void reserve(size_type n);		     //!<  make room for total of n elts
	
public:
    //!  Other functions
    void swap(SmallVector&);

    //!  Set here the allocator you want (we capture the object)
    static void allocator(alloc_type* the_alloc) {
        PLASSERT(the_alloc);
        alloc = the_alloc;
    }
    static alloc_type& allocator() {
        return *alloc;
    }
    
private:
    static alloc_type* alloc;		     //!<  underlying allocator
    typename alloc_type::index_type i;	     //!<  index of vector in memory
};

//!  hash function for hash tables
//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
//inline unsigned int hash(SmallVector<T,SizeBits,Allocator>& v)
template <class T, unsigned SizeBits, class Allocator>
inline unsigned int hashval(const SmallVector<T,SizeBits,Allocator>& v)
{ return hashbytes(const_cast<char*>(&v[0]),sizeof(T)*size()); }

//!  Equality operator
template <class T, unsigned SizeBits, class Allocator>
bool operator==(const SmallVector<T,SizeBits,Allocator>& a,
                const SmallVector<T,SizeBits,Allocator>& b);

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
//inline bool operator!=(const SmallVector<T,SizeBits,Allocator>& x,
//!   const SmallVector<T,SizeBits,Allocator>& y) { return !(x==y); }
template <class T, unsigned SizeBits, class Allocator>
inline bool operator!=(const SmallVector<T,SizeBits,Allocator>& x,
                       const SmallVector<T,SizeBits,Allocator>& y) { return !(x==y); }


//!  Lexicographical Ordering
//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
//bool operator<(const SmallVector<T,SizeBits,Allocator>&,
//!                const SmallVector<T,SizeBits,Allocator>&);
template <class T, unsigned SizeBits>
bool operator<(const SmallVector<T,SizeBits>&,
               const SmallVector<T,SizeBits>&);


//#####  Static Members  ######################################################

template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
Allocator* SmallVector<T,SizeBits,Allocator>::alloc;



//#####  Implementation of Iterators  #########################################

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::iterator
SmallVector<T,SizeBits,Allocator>::begin()
{
    //!  This is always correct, even for zero-size vectors
    return allocator().toPointer(i);
}

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::const_iterator
SmallVector<T,SizeBits,Allocator>::begin() const
{
    //!  This is always correct, even for zero-size vectors
    return allocator().toPointer(i);
}

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::iterator
SmallVector<T,SizeBits,Allocator>::end()
{
    return begin() + size();
}

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::const_iterator
SmallVector<T,SizeBits,Allocator>::end() const
{
    return begin() + size();
}


//#####  Implementation of Element Access  ####################################

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::reference
SmallVector<T,SizeBits,Allocator>::operator[](size_type n)
{
#ifdef BOUNDCHECK
    if (n >= size())
	PLERROR("%s: out-of-range.",typeid(*this).name());
#endif
    return *(begin()+n);
}
    
//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::const_reference
SmallVector<T,SizeBits,Allocator>::operator[](size_type n) const
{
#ifdef BOUNDCHECK
    if (n >= size())
	PLERROR("%s: out-of-range.",typeid(*this).name());
#endif
    return *(begin()+n);
}
    
//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::reference
SmallVector<T,SizeBits,Allocator>::at(size_type n)
{
    //!  n cannot be less than zero, because size_type is usually unsigned
    if (n >= size())
	PLERROR("%s: out-of-range.",typeid(*this).name());
	
    return *(begin()+n);
}
    
//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::const_reference
SmallVector<T,SizeBits,Allocator>::at(size_type n) const
{
    //!  n cannot be less than zero, because size_type is usually unsigned
    if (n >= size())
	PLERROR("%s: out-of-range.",typeid(*this).name());
	
    return *(begin()+n);
}
    
//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::reference
SmallVector<T,SizeBits,Allocator>::front()
{
    if (empty())
	PLERROR("%s: out-of-range.",typeid(*this).name());
	
    return *begin();
}
    
//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::const_reference
SmallVector<T,SizeBits,Allocator>::front() const
{
    if (empty())
	PLERROR("%s: out-of-range.",typeid(*this).name());
	
    return *begin();
}
    
//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::reference
SmallVector<T,SizeBits,Allocator>::back()
{
    if (empty())
	PLERROR("%s: out-of-range.",typeid(*this).name());
	
    return *(end()-1);
}
    
//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::const_reference
SmallVector<T,SizeBits,Allocator>::back() const
{
    if (empty())
	PLERROR("%s: out-of-range.",typeid(*this).name());
	
    return *(end()-1);
}

    
//#####  Implementation of Constructors, etc.  ################################

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
void SmallVector<T,SizeBits,Allocator>::assign(size_type n, const T& val)
{
    if (n > max_size())
	PLERROR("%s: out-of-range.",typeid(*this).name());

    resize(n);
    for (size_type i=0; i<n; ++i)
	(*this)[i] = val;
}

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
inline SmallVector<T,SizeBits,Allocator>::SmallVector()
    : i(0,0)
{ }

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
inline SmallVector<T,SizeBits,Allocator>::SmallVector(size_type n, const T& val)
    : i(0,0)
{
    assign(n, val);
}

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
SmallVector<T,SizeBits,Allocator>::SmallVector(const self_type& other)
    : i(0)
{
    assign(other.begin(), other.end());
}

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
SmallVector<T,SizeBits,Allocator>::~SmallVector()
{
    allocator().deallocate(i);
}

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::self_type&
SmallVector<T,SizeBits,Allocator>::operator=(const self_type& other)
{
    if (size() != other.size()) {
        self_type tmp(other);
        swap(tmp);
    }
    else {
        assign(other.begin(), other.end());
    }
    return *this;
}


//#####  Implementation of Stack Operations  ##################################

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
void SmallVector<T,SizeBits,Allocator>::push_back(const T& x)
{
    size_type s = size();
    resize(s+1);
    (*this)[s] = x;
}

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
void SmallVector<T,SizeBits,Allocator>::pop_back()
{
    size_type s = size();
    if (s == 0)
	PLERROR("%s: out-of-range.",typeid(*this).name());
    resize(s-1);
}

    
//#####  Implementation of Size/Capacity Operations  ##########################

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
typename SmallVector<T,SizeBits,Allocator>::size_type
SmallVector<T,SizeBits,Allocator>::size() const
{
    if (i.isNull())
	return 0;
    else
	return i.size;
}

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
void SmallVector<T,SizeBits,Allocator>::resize(size_type sz, const T& val)
{
    if (sz > max_size())
	PLERROR("%s: out-of-range.",typeid(*this).name());

    pointer newdata = allocator().allocate(sz);
    typename alloc_type::index_type newi = allocator().toIndex(newdata, sz);

    //!  Copy old data on new data
    pointer olddata = allocator().toPointer(i);
    size_type n = std::min(int(sz), int(i.size));
    while (n--)
	*newdata++ = *olddata++;

    //!  Initialize remaining of new data
    n = std::max(0, int(sz) - int(i.size));
    while (n--)
	*newdata++ = val;

    //!  make new size effective
    allocator().deallocate(i);
    i = newi;
}


//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
void SmallVector<T,SizeBits,Allocator>::reserve(size_type n)
{
    if (n > max_size())
	PLERROR("%s: out-of-range.",typeid(*this).name());
	
}


//#####  Implementation of Other Functions  ###################################

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
void SmallVector<T,SizeBits,Allocator>::swap(SmallVector<T,SizeBits,Allocator>& other)
{
    std::swap(i,other.i);
}

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
bool operator==(const SmallVector<T,SizeBits,Allocator>& x,
                const SmallVector<T,SizeBits,Allocator>& y)
{
    bool equal = true;
    typename SmallVector<T,SizeBits,Allocator>::const_iterator
	xit=x.begin(), xend=x.end(), yit=y.begin(), yend=y.end();
    if (xend-xit != yend-yit)
	return false;
    for ( ; equal && xit != xend && yit != yend ; ++xit, ++yit)
	equal = (*xit == *yit);
    return equal;
}

//template <class T, unsigned SizeBits, class Allocator = ArrayAllocatorTrivial<T,SizeBits> >
template <class T, unsigned SizeBits, class Allocator>
bool operator<(const SmallVector<T,SizeBits,Allocator>& x,
               const SmallVector<T,SizeBits,Allocator>& y)
{
    return std::lexicographical_compare(x.begin(), x.end(),
                                        y.begin(), y.end());
}

} // end of namespace PLearn


#endif


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
