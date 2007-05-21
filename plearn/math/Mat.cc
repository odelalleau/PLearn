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

#include "Mat.h"
#include "TMat_maths.h"

// for sort...
// #include <vector>
// #include <functional>
// #include <algorithm>

namespace PLearn {
using namespace std;

/*
  bool Vec::hasMissing() const
  {
  real* v = data(); // get vec data start
  for (int i=0; i<length(); i++)
  if (is_missing(v[i]))
  return true;
  return false;
  }
*/


Vec* newVecArray(int n)
{
    return new Vec[n];
}

Vec* newVecArray(int n, int the_length)
{
    Vec* varray = new Vec[n];
    for(int i=0; i<n; i++)
        varray[i].resize(the_length);
    return varray;
}

Mat* newMatArray(int n)
{
    return new Mat[n];
}

Mat* newMatArray(int n, int the_length, int the_width)
{
    Mat* marray = new Mat[n];
    for (int i=0; i<n; i++)
        marray[i].resize(the_length,the_width);
    return marray;
}

Mat* newIndexedMatArray(int n, Mat& m, int indexcolumn)
{
    if(indexcolumn!=0 && indexcolumn!=m.width()-1)
        PLERROR("In newIndexedMatArray(int n, const Mat& m, int indexcolumn): indexcolumn must be either the first or the last column of the matrix");
    sortRows(m, indexcolumn);
    Mat inputs, classnums;
    if(indexcolumn==0)
    {
        inputs = m.subMatColumns(1,m.width()-1);
        classnums = m.column(0);
    }
    else // indexcolumn is last column
    {
        inputs = m.subMatColumns(0,m.width()-1);
        classnums = m.column(m.width()-1);      
    }
    if(!fast_exact_is_equal(classnums(0,0),                    0)    ||
       !fast_exact_is_equal(classnums(classnums.length()-1,0), n-1))
        PLERROR("In newIndexedMatArray(int n, const Mat& m, int indexcolumn) Values in the indexcolumn should range from 0 to n-1");

    Mat* marray = new Mat[n];
    int pos = 0;
    for(int classnum=0; classnum<n; classnum++)
    {
        int startpos = pos;
        while(pos<classnums.length() && int(classnums(pos,0))==classnum)
            pos++;
        marray[classnum] = inputs.subMatRows(startpos,pos-startpos);
    }
    return marray;
}


Mat operator^(const Mat& m1, const Mat& m2)
{
    Mat result(m1.length()*m2.length(), m1.width()+m2.width());
    Mat lefthalf = result.subMatColumns(0,m1.width());
    Mat righthalf = result.subMatColumns(m1.width(),m2.width());
    int i=0;
    for(int i1=0; i1<m1.length(); i1++)
        for(int i2=0; i2<m2.length(); i2++)
        {
            lefthalf(i) << m1(i1);
            righthalf(i) << m2(i2);
            i++;
        }
    return result;
}

Mat unitmatrix(int n)
{
    Mat m(n,n);
    for(int i=0; i<n; i++)
        m(i,i) = 1.0;
    return m;
}

} // end of namespace PLearn


// For use within debugger (gdb) only
// For use within debugger (gdb) only
void printvec(const PLearn::Vec& v)
{ std::cout << v << std::endl; }

void printmat(const PLearn::Mat& m)
{ std::cout << m << std::endl; }


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
