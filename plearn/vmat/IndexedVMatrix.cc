// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Pascal Vincent, Olivier Delalleau
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
   * $Id: IndexedVMatrix.cc,v 1.2 2003/05/15 14:00:30 tihocan Exp $
   ******************************************************* */

#include "IndexedVMatrix.h"

namespace PLearn <%
using namespace std;

PLEARN_IMPLEMENT_OBJECT_METHODS(IndexedVMatrix, "IndexedVMatrix", VMatrix);


void IndexedVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "m", &IndexedVMatrix::m, OptionBase::buildoption,
      "    The matrix viewed by the IndexedVMatrix\n");
  inherited::declareOptions(ol);
}

string IndexedVMatrix::help()
{
  // ### Provide some useful description of what the class is ...
  return 
    "    VMat class that sees a matrix as a collection of triplets (row, column, value)\n\
    Thus it is a N x 3 matrix, with N = the number of elements in the original matrix.\n"
      + optionHelp();
}

void IndexedVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(m, copies);
}

void IndexedVMatrix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void IndexedVMatrix::build_()
{
  width_ = 3;
  length_ = m->length() * m->width();
  // TODO Get Field Infos ?
}

/////////
// get //
/////////
real IndexedVMatrix::get(int i, int j) const {
  int w = m->width();
  int i_ = i / w; // the value of the first column at row i
  int j_ = i % w; // the value of the second column at row i
  switch (j) {
    case 0:
      return i_;
    case 1:
      return j_;
    case 2:
      return m->get(i_, j_);
    default:
      PLERROR("In IndexedVMatrix::get An IndexedVMatrix has only 3 columns\n");
      return 0;
  }
}

/////////
// put //
/////////
void IndexedVMatrix::put(int i, int j, real value) {
  if (j != 2) {
    PLERROR("In IndexedVMatrix::put You can only modify the third column\n");
  }
  int w = m->width();
  int i_ = i / w; // the value of the first column at row i
  int j_ = i % w; // the value of the second column at row i
  m->put(i_, j_, value);
}


%> // end of namespcae PLearn
