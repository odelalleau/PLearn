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
   * $Id: SelectColumnsVMatrix.cc,v 1.8 2004/08/05 13:49:26 tihocan Exp $
   ******************************************************* */

#include "SelectColumnsVMatrix.h"

namespace PLearn {
using namespace std;

/** SelectColumnsVMatrix **/

PLEARN_IMPLEMENT_OBJECT(SelectColumnsVMatrix,
    "Selects variables from a source matrix according to given vector of indices.",
    "Alternatively, the variables can be given by their names."
);

//////////////////////////
// SelectColumnsVMatrix //
//////////////////////////
SelectColumnsVMatrix::SelectColumnsVMatrix()
{}

SelectColumnsVMatrix::SelectColumnsVMatrix(VMat the_source, TVec<string> the_fields)
: fields(the_fields)
{
  source = the_source;
  build_();
}

SelectColumnsVMatrix::SelectColumnsVMatrix(VMat the_source, TVec<int> the_indices)
: indices(the_indices)
{
  source = the_source;
  build_();
}

SelectColumnsVMatrix::SelectColumnsVMatrix(VMat the_source, Vec the_indices)
{
  source = the_source;
  indices.resize(the_indices.length());
  // copy the real the_indices into the integer indices
  indices << the_indices;
  build_();
}

real SelectColumnsVMatrix::get(int i, int j) const
{ return source->get(i, indices[j]); }

void SelectColumnsVMatrix::getSubRow(int i, int j, Vec v) const
{
  for(int jj=0; jj<v.length(); jj++)
    v[jj] = source->get(i, indices[j+jj]); 
}

void SelectColumnsVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "fields", &SelectColumnsVMatrix::fields, OptionBase::buildoption,
      "The names of the fields to extract (will override 'indices' if provided).");

  declareOption(ol, "indices", &SelectColumnsVMatrix::indices, OptionBase::buildoption,
      "The array of column indices to extract.");

  inherited::declareOptions(ol);
}

void SelectColumnsVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(indices, copies);
}

///////////
// build //
///////////
void SelectColumnsVMatrix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void SelectColumnsVMatrix::build_()
{
  if (source) {
    if (fields.isNotEmpty()) {
      // Find out the indices from the fields.
      indices.clear();
      for (int i = 0; i < fields.length(); i++) {
        string the_field = fields[i];
        indices.append(source->getFieldIndex(the_field));
      }
    }
    width_ = indices.length();
    length_ = source->length();
    // Copy the appropriate VMFields
    fieldinfos.resize(width());
    if (source->getFieldInfos().size() > 0)
      for (int i=0; i<width(); ++i)
        fieldinfos[i] = source->getFieldInfos()[indices[i]];
  }
}

} // end of namespcae PLearn
