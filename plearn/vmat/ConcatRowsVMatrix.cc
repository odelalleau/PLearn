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
   * $Id: ConcatRowsVMatrix.cc,v 1.8 2004/08/05 13:47:29 tihocan Exp $
   ******************************************************* */

#include "ConcatRowsVMatrix.h"
#include "SelectColumnsVMatrix.h"

namespace PLearn {
using namespace std;

/** ConcatRowsVMatrix **/

PLEARN_IMPLEMENT_OBJECT(ConcatRowsVMatrix,
    "Concatenates the rows of a number of VMat.",
    "It can also be used to select fields which are common to those VMat,\n"
    "using the 'only_common_fields' option.\n"
    "Otherwise, the fields are just assumed to be those of the first VMat.\n"
);

///////////////////////
// ConcatRowsVMatrix //
///////////////////////
ConcatRowsVMatrix::ConcatRowsVMatrix(TVec<VMat> the_array)
: array(the_array),
  only_common_fields(false)
{
  if (array.size() > 0)
    build_();
};

ConcatRowsVMatrix::ConcatRowsVMatrix(VMat d1, VMat d2)
: only_common_fields(false)
{
  array.resize(2);
  array[0] = d1;
  array[1] = d2;
  build_();
};

////////////////////
// declareOptions //
////////////////////
void ConcatRowsVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "array", &ConcatRowsVMatrix::array, OptionBase::buildoption,
      "The VMat to concatenate.");

  declareOption(ol, "only_common_fields", &ConcatRowsVMatrix::only_common_fields, OptionBase::buildoption,
      "If set to 1, then only the fields whose names are common to all VMat\n"
      "in 'array' will be kept (and reordered if needed).");

  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ConcatRowsVMatrix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void ConcatRowsVMatrix::build_()
{
  int n = array.size();
  if (n < 1)
    PLERROR("ConcatRowsVMatrix expects underlying-array of length >= 1, got 0");

  // Get the fields info.
  fieldinfos = array[0]->getFieldInfos();
  if (only_common_fields) {
    // Find out which fields need to be kept.
    TVec<VMField> final_fields(fieldinfos.length());
    final_fields << fieldinfos;
    TVec<VMField> other_fields;
    TVec<VMField> tmp(final_fields.length());
    for (int i = 1; i < array.length(); i++) {
      map<string, bool> can_be_kept;
      other_fields = array[i]->getFieldInfos();
      for (int j = 0; j < other_fields.length(); j++) {
        can_be_kept[other_fields[j].name] = true;
      }
      tmp.resize(0);
      for (int j = 0; j < final_fields.length(); j++)
        if (can_be_kept.count(final_fields[j].name) > 0)
          tmp.append(final_fields[j]);
      final_fields.resize(tmp.length());
      final_fields << tmp;
    }
    fieldinfos.resize(final_fields.length());
    fieldinfos << final_fields;
    // Get the corresponding field names.
    TVec<string> final_fieldnames(final_fields.length());
    for (int i = 0; i < final_fields.length(); i++)
      final_fieldnames[i] = final_fields[i].name;
    // Now fill 'to_concat' with the selected columns of each VMat in 'array'.
    to_concat.resize(array.length());
    for (int i = 0; i < array.length(); i++)
      to_concat[i] = new SelectColumnsVMatrix(array[i], final_fieldnames);
  } else {
    to_concat = array;
  }

  width_ = to_concat[0]->width();
  inputsize_ = to_concat[0]->inputsize();
  targetsize_ = to_concat[0]->targetsize();
  weightsize_ = to_concat[0]->weightsize();
  length_ = 0;
  for (int i=0;i<n;i++)
  {
    if (to_concat[i]->width() != width_)
      PLERROR("ConcatRowsVMatrix: underlying-VMat %d has %d width, while 0-th has %d",i,array[i]->width(),width_);
    length_ += to_concat[i]->length();
  }
}

//////////////////
// getStringVal //
//////////////////
real ConcatRowsVMatrix::getStringVal(int col, const string & str) const
{
  if(col>=width_)
    PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
  return to_concat[0]->getStringVal(col,str);
}

//////////////////
// getValString //
//////////////////
string ConcatRowsVMatrix::getValString(int col, real val) const
{
  if(col>=width_)
    PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
  return to_concat[0]->getValString (col,val);
}

///////////////
// getString //
///////////////
string ConcatRowsVMatrix::getString(int row, int col) const
{
  int whichvm, rowofvm;
  getpositions(row,whichvm,rowofvm);
  return to_concat[whichvm]->getString(rowofvm,col);
}

////////////////////////////
// getStringToRealMapping //
////////////////////////////
const map<string,real>& ConcatRowsVMatrix::getStringToRealMapping(int col) const
{
  return to_concat[0]->getStringToRealMapping(col);
}

//////////////////
// getpositions //
//////////////////
void ConcatRowsVMatrix::getpositions(int i, int& whichvm, int& rowofvm) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In ConcatRowsVMatrix::getpositions OUT OF BOUNDS");
#endif

  int pos = 0;
  int k=0;
  while(i>=pos+to_concat[k]->length())
    {
      pos += to_concat[k]->length();
      k++;
    }

  whichvm = k;
  rowofvm = i-pos;
}

/////////
// get //
/////////
real ConcatRowsVMatrix::get(int i, int j) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  return to_concat[whichvm]->get(rowofvm,j);
}

///////////////
// getSubRow //
///////////////
void ConcatRowsVMatrix::getSubRow(int i, int j, Vec v) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  to_concat[whichvm]->getSubRow(rowofvm, j, v);
}

/////////
// dot //
/////////
real ConcatRowsVMatrix::dot(int i1, int i2, int inputsize) const
{
  int whichvm1, rowofvm1;
  getpositions(i1,whichvm1,rowofvm1);
  int whichvm2, rowofvm2;
  getpositions(i2,whichvm2,rowofvm2);
  if(whichvm1==whichvm2)
    return to_concat[whichvm1]->dot(rowofvm1, rowofvm2, inputsize);
  else
    return VMatrix::dot(i1,i2,inputsize);
}

real ConcatRowsVMatrix::dot(int i, const Vec& v) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  return to_concat[whichvm]->dot(rowofvm,v);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ConcatRowsVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies) {
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields
  // ### that you wish to be deepCopied rather than
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("ConcatRowsVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");

}

////////////
// putMat //
////////////
void ConcatRowsVMatrix::putMat(int i, int j, Mat m) {
  int whichvm, rowofvm;
  for (int row = 0; row < length(); row++) {
    getpositions(row + i, whichvm, rowofvm);
    to_concat[whichvm]->putSubRow(rowofvm, j, m(row));
  }
}

} // end of namespcae PLearn
