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


/*! \file TopNI.h */

#ifndef TopNI_INC
#define TopNI_INC

#include <plearn/base/general.h>
#include "TMat.h"

namespace PLearn {
using namespace std;

/*!     This is a very simple class, that allows you to keep track of the top
  N values encountered among many and their associated integer index (or corresponding position).
  For keeping track of the bottom N values, see the companion class BottomNI.
*/
template <class T>
class TopNI
{
protected:
    int N;
    TVec< pair<T,int> > topn;
    pair<T,int>* topnptr; //!<  topn.data()
    pair<T,int>* minpair;
  
public:

    //!  Default constructor, you must then call init.
    TopNI()
    {}
    
    //!  N is the number of (value,index) pairs to remember.
    TopNI(int the_N) { init(the_N); }

    //!  N is the number of (value,point) pairs to remember.
    void init(int the_N)
    {
        N = the_N;
        topn.resize(N); //!<  reserve space for N elements
        topn.resize(0); //!<  set length back to 0
        topnptr = topn.data(); //!<  ptr for faster access
    }

    void reset() { init(N); }

    //!  call this for each new value seen
    void update(T value, int index)
    {
        int l=topn.length();
        if(l<N)
        {
            topn.append( pair<T,int>(value,index) );
            if (l==0 || value<minpair->first)
                minpair = &topnptr[l];
        }
        else if(value>minpair->first)
        {
            minpair->first = value;
            minpair->second = index;

            //!  find the new minimum:
            pair<T,int>* it = topnptr;
            int n = N;
            while(n--)
            {
                if(it->first<minpair->first)
                    minpair = it;
                ++it;
            }
        }
    }

    void sort() 
    { std::sort(topnptr, topnptr+topn.length(), greater<pair <T,int> >() ); }

    //!  call this at the end to get the the smallest N values with associated integer index
    const TVec< pair<T,int> >& getTopN() { return topn; }
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
