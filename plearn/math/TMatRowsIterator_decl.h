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


/*! \file PLearn/plearn/math/TMatRowsIterator_decl.h */

#ifndef TMatRowsIterator_decl_INC
#define TMatRowsIterator_decl_INC

namespace PLearn {
using namespace std;

//! Model of the Random Access Iterator concept for iterating through the
//! ROWS of a TMat.  The basic idea is that operator* returns the current
//! row as a TVec.
template <class T>
class TMatRowsIterator
{
public:
    //! Some useful typedefs
    typedef random_access_iterator_tag iterator_category;
    typedef TVec<T>                    value_type;
    typedef ptrdiff_t                  difference_type;
    typedef TVec<T>*                   pointer;
    typedef TVec<T>&                   reference;
  
private:
    T* ptr;                                    //!< current row pointer
    int width;                                 //!< vector width
    int mod;                                   //!< mod in underlying matrix

public:
    TMatRowsIterator()
        : ptr(), width(), mod() {}

    TMatRowsIterator(T* p, int w, int m)
        : ptr(p), width(w), mod(m) {}

    // Implement trivial iterator functions
    bool operator==(const TMatRowsIterator& other) const {
        return ptr == other.ptr && width == other.width && mod == other.mod;
    }

    bool operator!=(const TMatRowsIterator& y)
    { return !operator==(y); }

    value_type operator*() {
        return TVec<T>(width, ptr);
    }

    // cannot define operator-> here since we cannot return a pointer to a
    // temporary (e.g. stack-based) vector and expect this to work properly...
  
    // Implement forward iterator functions
    TMatRowsIterator& operator++() {
        ptr += mod;
        return *this;
    }

    TMatRowsIterator operator++(int) {
        TMatRowsIterator r(*this);
        ptr += mod;
        return r;
    }

    // Implement bidirectional iterator functions
    TMatRowsIterator& operator--() {
        ptr -= mod;
        return *this;
    }

    TMatRowsIterator operator--(int) {
        TMatRowsIterator r(*this);
        ptr -= mod;
        return r;
    }

    // Implement random access iterator functions
    TMatRowsIterator& operator+=(difference_type n) {
        ptr += n*mod;
        return *this;
    }

    TMatRowsIterator operator+(difference_type n) {
        TMatRowsIterator r(*this);
        r += n;
        return r;
    }
  
    TMatRowsIterator& operator-=(difference_type n) {
        ptr -= n*mod;
        return *this;
    }

    TMatRowsIterator operator-(difference_type n) {
        TMatRowsIterator r(*this);
        r -= n;
        return r;
    }

    difference_type operator-(const TMatRowsIterator& y) {
        return (ptr - y.ptr) / mod;
    }
  
    value_type operator[](difference_type n) {
        return TVec<T>(width, ptr + n*mod);
    }

    bool operator<(const TMatRowsIterator& y) {
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
