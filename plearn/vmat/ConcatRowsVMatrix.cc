// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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
   * $Id: ConcatRowsVMatrix.cc,v 1.1 2002/10/03 07:35:28 plearner Exp $
   ******************************************************* */

#include "ConcatRowsVMatrix.h"

namespace PLearn <%
using namespace std;

/** ConcatRowsVMatrix **/

IMPLEMENT_NAME_AND_DEEPCOPY(ConcatRowsVMatrix);

void ConcatRowsVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "array", &ConcatRowsVMatrix::array, OptionBase::buildoption, "Array of VMatrices");
  inherited::declareOptions(ol);
}

void ConcatRowsVMatrix::build()
{
  inherited::build();
  build_();
}

void ConcatRowsVMatrix::build_()
{
  int n = array.size();
  if (n < 1)
    PLERROR("ConcatRowsVMatrix expects >= 1 underlying-array, got 0");

  // Copy the field names
  fieldinfos = array[0]->getFieldInfos();
  
  width_ = array[0]->width();
  length_ = 0;
  for (int i=0;i<n;i++)
  {
    if (array[i]->width() != width_)
      PLERROR("ConcatRowsVMatrix: underlying-array %d has %d width, while 0-th has %d",i,array[i]->width(),width_);
    length_ += array[i]->length();
  }
}

//! Warning : the string map used is the one from the first of the concatenated matrices
real ConcatRowsVMatrix::getStringVal(int col, const string & str) const
{
  if(col>=width_)
    PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
  return array[0]->getStringVal(col,str);
}

//! Warning : the string map used is the one from the first of the concatenated matrices
string ConcatRowsVMatrix::getValString(int col, real val) const
{
  if(col>=width_)
    PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
  return array[0]->getValString (col,val);
}

//! Warning : the string map used is the one from the first of the concatenated matrices
string ConcatRowsVMatrix::getString(int row, int col) const
{
  int whichvm, rowofvm;
  getpositions(row,whichvm,rowofvm);
  return array[whichvm]->getString(rowofvm,col);
}


void ConcatRowsVMatrix::getpositions(int i, int& whichvm, int& rowofvm) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In ConcatRowsVMatrix::getpositions OUT OF BOUNDS");
#endif

  int pos = 0;
  int k=0;
  while(i>=pos+array[k]->length())
    {
      pos += array[k]->length();
      k++;
    }

  whichvm = k;
  rowofvm = i-pos;
}

real ConcatRowsVMatrix::get(int i, int j) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  return array[whichvm]->get(rowofvm,j);
}

void ConcatRowsVMatrix::getSubRow(int i, int j, Vec v) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  array[whichvm]->getSubRow(rowofvm, j, v);
}

real ConcatRowsVMatrix::dot(int i1, int i2, int inputsize) const
{
  int whichvm1, rowofvm1;
  getpositions(i1,whichvm1,rowofvm1);
  int whichvm2, rowofvm2;
  getpositions(i2,whichvm2,rowofvm2);
  if(whichvm1==whichvm2)
    return array[whichvm1]->dot(rowofvm1, rowofvm2, inputsize);
  else
    return VMatrix::dot(i1,i2,inputsize);
}

real ConcatRowsVMatrix::dot(int i, const Vec& v) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  return array[whichvm]->dot(rowofvm,v);
}


%> // end of namespcae PLearn
