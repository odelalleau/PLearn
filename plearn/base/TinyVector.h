// -*- C++ -*-

// TinyVector.h: Definition of a tiny vector
// Copyright (c) 2002 by Nicolas Chapados

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

#ifndef TINYVECTOR_H_INC
#define TINYVECTOR_H_INC

//#include <utility>
//#include <stdexcept>			     // for out_of_range
#include <algorithm>			     // for lexicographical_compare
#include <typeinfo>

#include "plerror.h"

/*! \file TinyVector.h */

namespace PLearn {
using namespace std;

//!  Forward declaration
template <class T> class TinyVectorTrait;


//############################  CLASS  TINYVECTOR  ########################

/**
 *  @class TinyVector
 *  @brief Compile-time fixed-size vector with interface close to std::vector.
 *
 *
 *  A tiny vector tries to mimic (a small subset of) the interface of a
 *  built-in standard std::vector<>, but it has an additional restriction: it
 *  can hold a compile-time maximum number of elements.  In exchange for this
 *  restriction, it allocates the elements of the vector within the class,
 *  which completely eliminates the need for dynamic allocation.  In addition,
 *  the size of the vector is not stored explicitly, but rather determined from
 *  the number of "missing values".  What exactly should be considered a
 *  missing value differs for each type T, which is why a
 *  TinyVectorTrait::Missing is defined for useful types.  You can also define
 *  one yourself.
 */

template < class T, unsigned N, class TTrait = TinyVectorTrait<T> >
class TinyVector
{
public:
    //!  Typedefs
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
    inline TinyVector();
    inline explicit TinyVector(size_type n, const T& val=T());
    //!  Use default copy constructor, destructor, assignment operator

    // (Disabled for now; too many ambiguities)
    // //!  Construct/Copy from Input iterator
    // template <class In>
    // TinyVector(In first, In last)  { assign(first, last); }
	
    template <class In>
    void assign(In first, In last) {
        //!  could not define it out-of-line; bug in gcc?
        resize(last-first);
        for (size_type i=0; first != last && i < N; ++i, ++first)
            arr[i] = *first;
    }
	
    inline void assign(size_type n, const T& val);	//!<  n copies of val

public:
    //!  Stack operations 
    void push_back(const T& x);	     //!<  add to end
    void pop_back();		     //!<  remove last element

public:
    //!  (list operations are NOT provided at the moment)

public:
    //!  Size and capacity operations
    size_type size() const;		     //!<  number of elements
    bool empty() const {
        return size() == 0;
    }
    size_type max_size() {
        return N;
    }
    void resize(size_type sz, const T& val=T()); //!<  added elts init by val
    void reserve(size_type n);	     //!<  make room for total of n elts

public:
    //  Other functions

    //! Swap this with the argument
    void swap(TinyVector&);

    //! Fill with a constant (keeping the size constant)
    void fill(const T&);

    //! For compatibility with TVec, copy() simply returns a copy of ourselves
    TinyVector copy() const { return *this; }
	
private:
    T arr[N];
};


//!  Equality operator
template <class T, unsigned N, class TTrait>
bool operator==(const TinyVector<T,N,TTrait>&,
                const TinyVector<T,N,TTrait>&);

//!  Lexicographical Ordering
template <class T, unsigned N, class TTrait>
bool operator<(const TinyVector<T,N,TTrait>&,
               const TinyVector<T,N,TTrait>&);

//!  Other operators (should be defined in std::rel_ops, but does not work
//!  properly with gcc yet).
template <class T, unsigned N, class TTrait>
inline bool operator!=(const TinyVector<T,N,TTrait>& x,
                       const TinyVector<T,N,TTrait>& y)
{
    return !(x == y);
}

template <class T, unsigned N, class TTrait>
inline bool operator> (const TinyVector<T,N,TTrait>& x,
                       const TinyVector<T,N,TTrait>& y)
{
    return y < x;
}

template <class T, unsigned N, class TTrait>
inline bool operator<=(const TinyVector<T,N,TTrait>& x,
                       const TinyVector<T,N,TTrait>& y)
{
    return !(y < x);
}

template <class T, unsigned N, class TTrait>
inline bool operator>=(const TinyVector<T,N,TTrait>& x,
                       const TinyVector<T,N,TTrait>& y)
{
    return !(x < y);
}

//! To emulate PLearn TVecs, operator<< implements a copy
template <class T, unsigned N, class TTrait>
inline void operator<<(TinyVector<T,N,TTrait>& x,
                       const TinyVector<T,N,TTrait>& y)
{
    x = y;
}


//#########################  CLASS  TINYVECTORTRAIT  ######################

/**
 *  @class TinyVectorTrait
 *  @brief Define "missing-value" for a number of types with TinyVector.
 *
 *  The class TinyVectorTrait is specialized for each class T of interest.  It
 *  defines a static constant named "Missing", which is the missing value for
 *  the type.  These missing values are used, in turn, to determine what is and
 *  is not an element of a TinyVector.
 *
 *  In addition, it defines a type "IOType" which is the type of value that we
 *  should convert to/from when performing PStream I/O.  This is mostly useful
 *  for ensuring that the character types don't get serialized and deserialized
 *  as characters, but integers.
 */
    
template <typename T> class TinyVectorTrait {};

template <> class TinyVectorTrait<unsigned char> {
public:
    static const unsigned char Missing = UCHAR_MAX;
    typedef unsigned IOType;
};

template <> class TinyVectorTrait<signed char> {
public:
    static const signed char Missing = CHAR_MAX;
    typedef int IOType;
};

template <> class TinyVectorTrait<char> {
public:
    static const char Missing = CHAR_MAX;
    typedef int IOType;
};

template <> class TinyVectorTrait<unsigned short> {
public:
    static const unsigned short Missing = USHRT_MAX;
    typedef unsigned short IOType;
};

template <> class TinyVectorTrait<short> {
public:
    static const short Missing = SHRT_MAX;
    typedef short IOType;
};

template <> class TinyVectorTrait<unsigned int> {
public:
    static const unsigned int Missing = UINT_MAX;
    typedef unsigned IOType;
};

template <> class TinyVectorTrait<int> {
public:
    static const int Missing = INT_MAX;
    typedef int IOType;
};

  
//#####  Implementation of Iterators  #####################################

template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::iterator
TinyVector<T,N,TTrait>::begin()
{
    //!  This is always correct, even for zero-size vectors
    return &arr[0];
}

template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::const_iterator
TinyVector<T,N,TTrait>::begin() const
{
    //!  This is always correct, even for zero-size vectors
    return &arr[0];
}

template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::iterator
TinyVector<T,N,TTrait>::end()
{
    return &arr[0] + size();
}

template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::const_iterator
TinyVector<T,N,TTrait>::end() const
{
    return &arr[0] + size();
}


//#####  Implementation of Element Access  ################################

template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::reference
TinyVector<T,N,TTrait>::operator[](size_type n)
{
#ifdef BOUNDCHECK
    if (n >= size())
        PLERROR("%s: out-of-range.",typeid(*this).name());
#endif
    return arr[n];
}
    
template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::const_reference
TinyVector<T,N,TTrait>::operator[](size_type n) const
{
#ifdef BOUNDCHECK
    if (n >= size())
        PLERROR("%s: out-of-range.",typeid(*this).name());
#endif
    return arr[n];
}
    
template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::reference
TinyVector<T,N,TTrait>::at(size_type n)
{
    //!  n cannot be less than zero, because size_type is usually unsigned
    if (n >= size())
        PLERROR("%s: out-of-range.",typeid(*this).name());
	
    return arr[n];
}
    
template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::const_reference
TinyVector<T,N,TTrait>::at(size_type n) const
{
    //!  n cannot be less than zero, because size_type is usually unsigned
    if (n >= size())
        PLERROR("%s: out-of-range.",typeid(*this).name());
	
    return arr[n];
}
    
template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::reference
TinyVector<T,N,TTrait>::front()
{
    if (empty())
        PLERROR("%s: out-of-range.",typeid(*this).name());
	
    return arr[0];
}
    
template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::const_reference
TinyVector<T,N,TTrait>::front() const
{
    if (empty())
        PLERROR("%s: out-of-range.",typeid(*this).name());
	
    return arr[0];
}
    
template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::reference
TinyVector<T,N,TTrait>::back()
{
    if (empty())
        PLERROR("%s: out-of-range.",typeid(*this).name());
	
    return *(end()-1);
}
    
template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::const_reference
TinyVector<T,N,TTrait>::back() const
{
    if (empty())
        PLERROR("%s: out-of-range.",typeid(*this).name());
	
    return *(end()-1);
}
    
    
//#####  Implementation of Constructors, etc.  ############################

template <class T, unsigned N, class TTrait>
void TinyVector<T,N,TTrait>::assign(size_type n, const T& val)
{
    if (n > N)
        PLERROR("%s: out-of-range.",typeid(*this).name());

    resize(n);
    for (size_type i=0; i<n; ++i)
        arr[i] = val;
}

template <class T, unsigned N, class TTrait>
TinyVector<T,N,TTrait>::TinyVector()
{
    assign(static_cast<size_type>(N), 
           static_cast<const T&>(TTrait::Missing));
}

template <class T, unsigned N, class TTrait>
TinyVector<T,N,TTrait>::TinyVector(size_type n, const T& val)
{
    assign(n, val);
    if (n<N)
        for (size_type i=n; i<N; ++i)
            arr[i] = TTrait::Missing;
}


//#####  Implementation of Stack Operations  ##############################

template <class T, unsigned N, class TTrait>
void TinyVector<T,N,TTrait>::push_back(const T& x)
{
    size_type s = size();
    if (s >= N)
        PLERROR("%s: out-of-range.",typeid(*this).name());
	
    arr[s] = x;
}

template <class T, unsigned N, class TTrait>
void TinyVector<T,N,TTrait>::pop_back()
{
    size_type s = size();
    if (s == 0)
        PLERROR("%s: out-of-range.",typeid(*this).name());
	
    arr[s-1] = TTrait::Missing;
}

    
//#####  Implementation of Size/Capacity Operations  ######################

template <class T, unsigned N, class TTrait>
typename TinyVector<T,N,TTrait>::size_type
TinyVector<T,N,TTrait>::size() const
{
    difference_type p = N-1;

    while (p >= 0 && arr[p] == static_cast<T>(TTrait::Missing))
        p--;
    return p+1;
}

template <class T, unsigned N, class TTrait>
void TinyVector<T,N,TTrait>::resize(size_type sz, const T& val)
{
    if (sz > max_size())
        PLERROR("%s: out-of-range.",typeid(*this).name());
	
    size_type s = size();
    while (s < sz)
        arr[s++] = val;
    while (sz < N)
        arr[sz++] = TTrait::Missing;
}
    
template <class T, unsigned N, class TTrait>
void TinyVector<T,N,TTrait>::reserve(size_type n)
{
    if (n > max_size())
        PLERROR("%s: out-of-range.",typeid(*this).name());
	
}
    

//#####  Implementation of Other Functions  ###############################

template <class T, unsigned N, class TTrait>
void TinyVector<T,N,TTrait>::swap(TinyVector<T,N,TTrait>& other)
{
    using namespace std;		     //!<  if necessary for swap;
					     //!  otherwise uses Koenig lookup
    for (size_type i=0; i<N; ++i)
        swap(arr[i], other.arr[i]);
}

template <class T, unsigned N, class TTrait>
void TinyVector<T,N,TTrait>::fill(const T& value)
{
    for (int i=0, n=size() ; i<n ; ++i)
        arr[i] = value;
}

template <class T, unsigned N, class TTrait>
bool operator==(const TinyVector<T,N,TTrait>& x,
                const TinyVector<T,N,TTrait>& y)
{
    bool equal = true;
    typename TinyVector<T,N,TTrait>::const_iterator
        xit=x.begin(), xend=x.end(), yit=y.begin(), yend=y.end();
    if (xend-xit != yend-yit)
        return false;
    for ( ; equal && xit != xend && yit != yend ; ++xit, ++yit)
        equal = (*xit == *yit);
    return equal;
}

template <class T, unsigned N, class TTrait>
bool operator<(const TinyVector<T,N,TTrait>& x,
               const TinyVector<T,N,TTrait>& y)
{
    return std::lexicographical_compare(x.begin(), x.end(),
                                        y.begin(), y.end());
}
    
} // end of namespace PLearn

#endif // TINYVECTOR_H_INC


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
