// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
// Copyright (C) 2003 Olivier Delalleau
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
   * $Id: SelectRowsVMatrix.cc,v 1.11 2004/03/23 23:08:08 morinf Exp $
   ******************************************************* */

#include "SelectRowsVMatrix.h"

namespace PLearn {
using namespace std;

/** SelectRowsVMatrix **/

PLEARN_IMPLEMENT_OBJECT(SelectRowsVMatrix, "ONE LINE DESCR", "NO HELP");

SelectRowsVMatrix::SelectRowsVMatrix() 
{
}

SelectRowsVMatrix::SelectRowsVMatrix(VMat the_distr, TVec<int> the_indices)
  : inherited(the_indices.length(),the_distr->width()),
    distr(the_distr),indices(the_indices)
{
  build_();
}

//! Here the indices will be copied locally into an integer vector
SelectRowsVMatrix::SelectRowsVMatrix(VMat the_distr, Vec the_indices)
  : inherited(the_indices.length(),the_distr->width()),
    distr(the_distr),indices(the_indices.length())
{
  indices << the_indices; // copy to integer indices
  build_();
}

real SelectRowsVMatrix::get(int i, int j) const
{ return distr->get(indices[i], j); }

void SelectRowsVMatrix::getSubRow(int i, int j, Vec v) const
{ distr->getSubRow(indices[i], j, v); }

real SelectRowsVMatrix::dot(int i1, int i2, int inputsize) const
{ return distr->dot(int(indices[i1]), int(indices[i2]), inputsize); }

real SelectRowsVMatrix::dot(int i, const Vec& v) const
{ return distr->dot(indices[i],v); }

real SelectRowsVMatrix::getStringVal(int col, const string & str) const
{ return distr->getStringVal(col, str); }

string SelectRowsVMatrix::getValString(int col, real val) const
{ return distr->getValString(col,val); }

string SelectRowsVMatrix::getString(int row, int col) const
{ return distr->getString(row,col); }

const map<string,real>& SelectRowsVMatrix::getStringToRealMapping(int col) const
{ return distr->getStringToRealMapping(col);}

const map<real,string>& SelectRowsVMatrix::getRealToStringMapping(int col) const
{ return distr->getRealToStringMapping(col);}

void SelectRowsVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &SelectRowsVMatrix::distr, OptionBase::buildoption,
        "    The matrix viewed by the SelectRowsVMatrix");
    declareOption(ol, "indices", &SelectRowsVMatrix::indices, OptionBase::buildoption, 
        "    The array of row indices to extract");
    inherited::declareOptions(ol);
}

void SelectRowsVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(distr, copies);
  deepCopyField(indices, copies);
}

///////////
// build //
///////////
void SelectRowsVMatrix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void SelectRowsVMatrix::build_()
{
  length_ = indices.length();
  width_ = distr->width();
  inputsize_ = distr->inputsize();
  targetsize_ = distr->targetsize();
  weightsize_ = distr->weightsize();
  fieldinfos = distr->fieldinfos;
}

//////////
// help //
//////////
string SelectRowsVMatrix::help()
{
  return
    "    VMat class that selects samples from a sub-distribution\n\
    according to given vector of indices.\n"
    + optionHelp();
}

} // end of namespcae PLearn
