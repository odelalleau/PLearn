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


/*! \file PLearn/plearn/math/TMatElementIterator_decl.h */

#ifndef TMatTMatElementIterator_decl_INC
#define TMatTMatElementIterator_decl_INC

namespace PLearn {
using namespace std;

template<class T>
class TMatElementIterator
{
private:
    int width;
    int mod_minus_width;
    T* ptr; // current element pointer
    T* rowend; // after-last element of current row

public:

    typedef forward_iterator_tag iterator_category;
    typedef T value_type;
    typedef int size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;

    inline TMatElementIterator(T* begin, int width, int mod)
        :width(width), mod_minus_width(mod-width), ptr(begin), rowend(begin+width)
    {}

    inline TMatElementIterator<T>& operator++()
    { 
        ++ptr;
        if(ptr==rowend)
        {
            ptr += mod_minus_width;
            rowend = ptr+width; 
        }
        return *this;
    }

    inline TMatElementIterator<T> operator++(int)
    { 
        TMatElementIterator<T> prev(*this);
        ++ptr;
        if(ptr==rowend)
        {
            ptr += mod_minus_width;
            rowend = ptr+width; 
        }
        return prev;
    }

    inline T* operator->() const
    { return ptr; }
  
    inline T& operator*() const
    { return *ptr; }

    inline bool operator==(const TMatElementIterator& other)
    { return ptr==other.ptr; }

    inline bool operator!=(const TMatElementIterator& other)
    { return ptr!=other.ptr; }

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
