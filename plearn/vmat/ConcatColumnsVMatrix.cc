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
   * $Id: ConcatColumnsVMatrix.cc,v 1.7 2004/04/05 23:14:13 morinf Exp $
   ******************************************************* */

#include "ConcatColumnsVMatrix.h"

namespace PLearn {
using namespace std;

/** ConcatColumnsVMatrix **/

PLEARN_IMPLEMENT_OBJECT(ConcatColumnsVMatrix, "ONE LINE DESCR", "NO HELP");

void ConcatColumnsVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "array", &ConcatColumnsVMatrix::array, OptionBase::buildoption, "Array of VMatrices");
  inherited::declareOptions(ol);
}

void ConcatColumnsVMatrix::build()
{
  inherited::build();
  build_();
}

void ConcatColumnsVMatrix::build_()
{
  length_ = width_ = 0;
  if(array.size())
    length_ = array[0]->length();
  else
    PLERROR("ConcatColumnsVMatrix expects >= 1 underlying-array, got 0");

  for(int i=0; i<array.size(); i++)
    {
      if(array[i]->length()!=length_)
        PLERROR("ConcatColumnsVMatrix: Problem concatenating to VMatrices with different lengths");
      if(array[i]->width() == -1)
        PLERROR("In ConcatColumnsVMatrix constructor. Non-fixed width distribution not supported");
      width_ += array[i]->width();
    }

  // Copy the original fieldinfos.  Be careful if only some of the
  // matrices have fieldinfos
  fieldinfos.resize(width_);
  int fieldindex = 0;
  for (int i=0; i<array.size(); ++i) 
    {
      int len = array[i]->getFieldInfos().size();
      if (len > 0) // infos exist for this VMat
        {
          for (int j=0; j<len; ++j)
            fieldinfos[fieldindex++] = array[i]->getFieldInfos()[j];
        }
      else // infos don't exist for this VMat, use the index as the name for those fields.
        {
          len = array[i]->width();
          for(int j=0; j<len; ++j)
            fieldinfos[fieldindex++] = VMField(tostring(fieldindex));
        }
    }
}

void ConcatColumnsVMatrix::getRow(int i, Vec samplevec) const
{
  if (length_==-1)
    PLERROR("In ConcatColumnsVMatrix::getRow(int i, Vec samplevec) not supported for distributions with different (or infinite) lengths\nCall sample without index instead");
  samplevec.resize(width_);
  int pos = 0;
  for(int n=0; n<array.size(); n++)
    {
      int nvars = array[n]->width();
      Vec samplesubvec = samplevec.subVec(pos, nvars);
      array[n]->getRow(i,samplesubvec);
      pos += nvars;
    }
}

real ConcatColumnsVMatrix::getStringVal(int col, const string & str) const
{
  if(col>=width_)
    PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
  int pos=0,k=0;
  while(col>=pos+array[k]->width())
    {
      pos += array[k]->width();
      k++;
    }
//  return array[k]->getStringVal(pos+col,str);
    return array[k]->getStringVal(col-pos,str);
}

string ConcatColumnsVMatrix::getValString(int col, real val) const
{
  if(col>=width_)
    PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
  int pos=0,k=0;
  while(col>=pos+array[k]->width())
    {
      pos += array[k]->width();
      k++;
    }
//  return array[k]->getValString(pos+col,val);
  return array[k]->getValString(col-pos,val);
}

const map<string,real>& ConcatColumnsVMatrix::getStringMapping(int col) const
{
  if(col>=width_)
    PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
  int pos=0,k=0;
  while(col>=pos+array[k]->width())
    {
      pos += array[k]->width();
      k++;
    }
//  return array[k]->getStringToRealMapping(pos+col);
  return array[k]->getStringToRealMapping(col-pos);
}

string ConcatColumnsVMatrix::getString(int row, int col) const
{
  if(col>=width_)
    PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
  int pos=0,k=0;
  while(col>=pos+array[k]->width())
    {
      pos += array[k]->width();
      k++;
    }
//  return array[k]->getString(row,pos+col);
  return array[k]->getString(row,col-pos);
}



real ConcatColumnsVMatrix::dot(int i1, int i2, int inputsize) const
{
  real res = 0.;
  for(int k=0; ;k++)
    {
      const VMat& vm = array[k];
      int vmwidth = vm.width();
      if(inputsize<=vmwidth)
        {
          res += vm->dot(i1,i2,inputsize);
          break;
        }
      else
        {
          res += vm->dot(i1,i2,vmwidth);
          inputsize -= vmwidth;
        }
    }
  return res;
}

real ConcatColumnsVMatrix::dot(int i, const Vec& v) const
{
  if (length_==-1)
    PLERROR("In ConcatColumnsVMatrix::getRow(int i, Vec samplevec) not supported for distributions with different (or infinite) lengths\nCall sample without index instead");

  real res = 0.;
  int pos = 0;
  for(int n=0; n<array.size(); n++)
    {
      int nvars = std::min(array[n]->width(),v.length()-pos);
      if(nvars<=0)
        break;
      Vec subv = v.subVec(pos, nvars);
      res += array[n]->dot(i,subv);
      pos += nvars;
    }
  return res;
}


} // end of namespcae PLearn
