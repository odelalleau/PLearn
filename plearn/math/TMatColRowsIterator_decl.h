// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * $Id$
 * AUTHORS: Pascal Vincent & Yoshua Bengio
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearn/plearn/math/TMatColRowsIterator_decl.h */

#ifndef TMatColRowsIterator_decl_INC
#define TMatColRowsIterator_decl_INC

namespace PLearn {
using namespace std;

//! Model of the Random Access Iterator concept for iterating through a
//! single column of a TMat, one row at a time.  The basic idea is that
//! operator* returns a T for the given column of the current row.  Very
//! useful for passing to STL algorithms.
template <class T>
class TMatColRowsIterator
{
public:
    //! Some useful typedefs
    typedef random_access_iterator_tag iterator_category;
    typedef T                          value_type;
    typedef ptrdiff_t                  difference_type;
    typedef T*                         pointer;
    typedef T&                         reference;
  
private:
    T* ptr;                                    //!< current element pointer
    int mod;                                   //!< mod in underlying matrix

public:
    TMatColRowsIterator()
        : ptr(), mod() {}

    //! This constructor assumes that p points to the proper initial element
    TMatColRowsIterator(T* p, int m)
        : ptr(p), mod(m) {}

    // Implement trivial iterator functions
    bool operator==(const TMatColRowsIterator& other) const {
        return ptr == other.ptr && mod == other.mod;
    }

    reference operator*() {
        return *ptr;
    }

    reference operator->() {                   //!< works if T is some kind
        return *ptr;                             //!<  of smart pointer
    }
  
    // Implement forward iterator functions
    TMatColRowsIterator<T>& operator++() {
        ptr += mod;
        return *this;
    }

    TMatColRowsIterator<T> operator++(int) {
        TMatColRowsIterator<T> r(*this);
        ptr += mod;
        return r;
    }

    // Implement bidirectional iterator functions
    TMatColRowsIterator<T>& operator--() {
        ptr -= mod;
        return *this;
    }

    TMatColRowsIterator<T> operator--(int) {
        TMatColRowsIterator<T> r(*this);
        ptr -= mod;
        return r;
    }

    // Implement random access iterator functions
    TMatColRowsIterator<T>& operator+=(difference_type n) {
        ptr += n*mod;
        return *this;
    }

    TMatColRowsIterator<T> operator+(difference_type n) {
        TMatColRowsIterator<T> r(*this);
        r += n;
        return r;
    }
  
    TMatColRowsIterator<T>& operator-=(difference_type n) {
        ptr -= n*mod;
        return *this;
    }

    TMatColRowsIterator<T> operator-(difference_type n) {
        TMatColRowsIterator<T> r(*this);
        r -= n;
        return r;
    }

    difference_type operator-(const TMatColRowsIterator<T>& y) {
        return (ptr - y.ptr) / mod;
    }
  
    reference operator[](difference_type n) {
        return *(ptr + n*mod);
    }

    bool operator<(const TMatColRowsIterator<T>& y) {
        return ptr < y.ptr;
    }
};

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
