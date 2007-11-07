// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * AUTHORS: Yoshua Bengio
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearn/plearn/math/BottomNI.h */

#ifndef BottomNI_INC
#define BottomNI_INC

#include <plearn/base/general.h>
#include "TMat.h"

namespace PLearn {
using namespace std;

/*!     This is a very simple class, that allows you to keep track of the bottom
  N values encountered among many and their associated integer index (or corresponding position).
  For keeping track of the TOP N values, see the companion class TopNI.
*/
template <class T>
class BottomNI
{
protected:
    int n_zeros; //!< number of zeros
    int N;
    TVec< pair<T,int> > bottomn;
    pair<T,int>* bottomnptr; //!<  bottomn.data()
    pair<T,int>* maxpair;
  
public:

    //!  Default constructor, you must then call init.
    BottomNI()
        : n_zeros(0), N(0), bottomnptr(0), maxpair(0)
    { }
    
    //!  N is the number of (value,index) pairs to remember.
    BottomNI(int the_N)
        : n_zeros(0), N(0), bottomnptr(0), maxpair(0)
    { init(the_N); }

    //!  N is the number of (value,point) pairs to remember.
    void init(int the_N)
    {
        N = the_N;
        bottomn.resize(N); //!<  reserve space for N elements
        bottomn.resize(0); //!<  set length back to 0
        bottomnptr = bottomn.data(); //!<  ptr for faster access
        n_zeros=0;
    }

    void reset() { init(N); }

    //!  call this for each new value seen
    void update(T value, int index)
    {
        int l=bottomn.length();
        if(value==0)
            n_zeros++;
        if(l<N)
        {
            bottomn.append( pair<T,int>(value,index) );
            if (l==0 || value>maxpair->first)
                maxpair = &bottomnptr[l];
        }
        else if(value<maxpair->first)
        {
            maxpair->first = value;
            maxpair->second = index;

            //!  find the new maximum:
            pair<T,int>* it = bottomnptr;
            int n = N;
            while(n--)
            {
                if(it->first>maxpair->first)
                    maxpair = it;
                ++it;
            }
        }
    }

    int nZeros() const {return n_zeros;}

    void sort() 
    { std::sort(bottomnptr, bottomnptr+bottomn.length()); }

    //!  call this at the end to get the the smallest N values with associated integer index
    const TVec< pair<T,int> >& getBottomN() { return bottomn; }
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
