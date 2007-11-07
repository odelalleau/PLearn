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
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file PLearn/plearn/math/Mat.h */

#ifndef MAT_INC
#define MAT_INC

#include "TMat.h"
#include <plearn/io/MatIO.h>

namespace PLearn {
using namespace std;

// ********* Functions ***********


inline ostream& operator<<(ostream& out, const Vec& v)
{ 
    v.print(out);
    return out;
}
/*
  inline istream& operator>>(istream& in, const Vec& v)
  { 
  v.input(in);
  return in;
  }
*/
Vec* newVecArray(int n);
Vec* newVecArray(int n, int the_length);

template<>
inline void deepCopyField(Vec& field, CopiesMap& copies)
{
    field.makeDeepCopyFromShallowCopy(copies);
}

Mat* newMatArray(int n);
Mat* newMatArray(int n, int the_length, int the_width);

/*!   Returns an array of n matrices, that are submatrices of m
  Such that marray[i] contains all the rows of m that had value i
  in their indexcolumn. 
  The matrices of the returned array do not contain the indexcolumn
  Side effect: rows of m are sorted according to indexcolumn
*/
Mat* newIndexedMatArray(int n, Mat& m, int indexcolumn);

Mat unitmatrix(int n);

/*!   "cross" product of two sets.
  Matrices m1 and m2 are regarded as two sets of vectors (their rows)
  m1^m2 returns the set of all possible concatenations of a vector from m1 and a vector from m2
  ex: Mat(2,1,"1 2   3 4") ^ Mat(2,2,"10 20  30 40") 
  ==> 
  1 2 10 20
  1 2 30 40
  3 4 10 20
  3 4 30 40
*/
Mat operator^(const Mat& m1, const Mat& m2);

template<>
inline void deepCopyField(Mat& field, CopiesMap& copies)
{
    field.makeDeepCopyFromShallowCopy(copies);
}

} // end of namespace PLearn

//!  For use within debugger (gdb) only
void printvec(const PLearn::Vec& v);
void printmat(const PLearn::Mat& m);

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
