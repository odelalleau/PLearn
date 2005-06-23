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
   * $Id$
   ******************************************************* */

#include "SelectRowsVMatrix.h"

namespace PLearn {
using namespace std;

/** SelectRowsVMatrix **/

PLEARN_IMPLEMENT_OBJECT(SelectRowsVMatrix,
    "VMat class that selects samples from a source matrix according to given vector of indices.",
    ""
);

SelectRowsVMatrix::SelectRowsVMatrix() 
{
}

SelectRowsVMatrix::SelectRowsVMatrix(VMat the_source, TVec<int> the_indices)
: indices(the_indices)
{
  source = the_source;
  build_();
}

//! Here the indices will be copied locally into an integer vector
SelectRowsVMatrix::SelectRowsVMatrix(VMat the_source, Vec the_indices)
{
  source = the_source;
  indices.resize(the_indices.length());
  indices << the_indices; // copy to integer indices
  build_();
}

real SelectRowsVMatrix::get(int i, int j) const
{ return source->get(indices[i], j); }

void SelectRowsVMatrix::getSubRow(int i, int j, Vec v) const
{ source->getSubRow(indices[i], j, v); }

real SelectRowsVMatrix::dot(int i1, int i2, int inputsize) const
{ return source->dot(int(indices[i1]), int(indices[i2]), inputsize); }

real SelectRowsVMatrix::dot(int i, const Vec& v) const
{ return source->dot(indices[i],v); }

real SelectRowsVMatrix::getStringVal(int col, const string & str) const
{ return source->getStringVal(col, str); }

string SelectRowsVMatrix::getValString(int col, real val) const
{ return source->getValString(col,val); }

string SelectRowsVMatrix::getString(int row, int col) const
{ return source->getString(indices[row], col); }

const map<string,real>& SelectRowsVMatrix::getStringToRealMapping(int col) const
{ return source->getStringToRealMapping(col);}

const map<real,string>& SelectRowsVMatrix::getRealToStringMapping(int col) const
{ return source->getRealToStringMapping(col);}

void SelectRowsVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "indices", &SelectRowsVMatrix::indices, OptionBase::buildoption, 
        "The array of row indices to extract");

    declareOption(ol, "indices_vmat", &SelectRowsVMatrix::indices_vmat, OptionBase::buildoption, 
        "If provided, will override the 'indices' option: the indices will be taken\n"
        "from the first column of the given VMatrix (taking the closest integer).");

    inherited::declareOptions(ol);
}

void SelectRowsVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
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
  if (indices_vmat) {
    int n = indices_vmat->length();
    indices.resize(n);
    for (int i = 0; i < n; i++)
      indices[i] = int(round(indices_vmat->get(i,0)));
  }
  length_ = indices.length();
  if (source) {
    width_ = source->width();
    if(inputsize_<0)
      inputsize_ = source->inputsize();
    if(targetsize_<0)
      targetsize_ = source->targetsize();
    if(weightsize_<0)
      weightsize_ = source->weightsize();
    fieldinfos = source->fieldinfos;
  }
}

} // end of namespcae PLearn
