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
   * $Id: ConcatRowsSubVMatrix.cc,v 1.4 2004/04/05 22:49:56 morinf Exp $
   ******************************************************* */

#include "ConcatRowsSubVMatrix.h"

namespace PLearn {
using namespace std;


/** ConcatRowsSubVMatrix **/

PLEARN_IMPLEMENT_OBJECT(ConcatRowsSubVMatrix, "ONE LINE DESC", "ONE LINE HELP");

ConcatRowsSubVMatrix::ConcatRowsSubVMatrix()
{
}

ConcatRowsSubVMatrix::ConcatRowsSubVMatrix(VMat the_distr, TVec<int>& the_start, TVec<int>& the_len)
  : inherited(-1,the_distr->width()), distr(the_distr), start(the_start), len(the_len)
{
  //! Copy parent field names
/*
  fieldinfos = the_distr->getFieldInfos();
  
  check();
*/
  build();
}

ConcatRowsSubVMatrix::ConcatRowsSubVMatrix(VMat the_distr, int start1, int len1, int start2, int len2)
  : inherited(-1,the_distr->width()), distr(the_distr), start(2), len(2)
{
  //! Copy parent field names
  //fieldinfos = the_distr->getFieldInfos();
  
  start[0]=start1;
  start[1]=start2;
  len[0]=len1;
  len[1]=len2;
  //check();
  build();
}
/*
void ConcatRowsSubVMatrix::check()
{
  length_=0;
  for (int i=0;i<start.length();i++)
  {
    if (start[i]<0 || start[i]+len[i]>distr->length())
      PLERROR("ConcatRowsSubVMatrix: out-of-range specs for sub-distr %d, "
            "start=%d, len=%d, underlying distr length=%d",i,start[i],len[i],
            distr->length());
    length_ += len[i];
  }
}
*/

void ConcatRowsSubVMatrix::getpositions(int i, int& whichvm, int& rowofvm) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In ConcatRowsSubVMatrix::getpositions OUT OF BOUNDS");
#endif

  int pos = 0;
  int k=0;
  while(i>=pos+len[k])
    {
      pos += len[k];
      k++;
    }

  whichvm = k;
  rowofvm = i-pos;
}

real ConcatRowsSubVMatrix::get(int i, int j) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  return distr->get(start[whichvm]+rowofvm,j);
}

void ConcatRowsSubVMatrix::getSubRow(int i, int j, Vec v) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  distr->getSubRow(start[whichvm]+rowofvm, j, v);
}

real ConcatRowsSubVMatrix::dot(int i1, int i2, int inputsize) const
{
  int whichvm1, rowofvm1;
  getpositions(i1,whichvm1,rowofvm1);
  int whichvm2, rowofvm2;
  getpositions(i2,whichvm2,rowofvm2);
  return distr->dot(start[whichvm1]+rowofvm1, start[whichvm2]+rowofvm2, inputsize);
}

real ConcatRowsSubVMatrix::dot(int i, const Vec& v) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  return distr->dot(start[whichvm]+rowofvm,v);
}

void
ConcatRowsSubVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &ConcatRowsSubVMatrix::distr, OptionBase::buildoption, "");
    declareOption(ol, "start", &ConcatRowsSubVMatrix::start, OptionBase::buildoption, "");
    declareOption(ol, "len", &ConcatRowsSubVMatrix::len, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void
ConcatRowsSubVMatrix::build()
{
    inherited::build();
    build_();
}

void
ConcatRowsSubVMatrix::build_()
{
    if (distr) {
        fieldinfos = distr->getFieldInfos();
        length_=0;
        for (int i = 0; i < start.length(); i++) {
            if (start[i]<0 || start[i]+len[i]>distr->length())
                PLERROR("ConcatRowsSubVMatrix: out-of-range specs for sub-distr %d, "
                        "start=%d, len=%d, underlying distr length=%d",i,start[i],len[i],
                        distr->length());
            length_ += len[i];
        }
    }
}

} // end of namespcae PLearn
