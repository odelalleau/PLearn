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
   * $Id: RowBufferedVMatrix.cc,v 1.9 2004/07/21 16:30:55 chrish42 Exp $
   ******************************************************* */

#include "RowBufferedVMatrix.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

/** RowBufferedVMatrix **/

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(RowBufferedVMatrix,
    "A base class for VMatrices that keep the last row(s) in a buffer for faster access.",
    "");

RowBufferedVMatrix::RowBufferedVMatrix(int the_length, int the_width)
  :VMatrix(the_length, the_width), 
  current_row_index(-1), current_row(the_width), 
  other_row_index(-1), other_row(the_width) 
{}

RowBufferedVMatrix::RowBufferedVMatrix()
  :current_row_index(-1), other_row_index(-1)
{}

/////////
// get //
/////////
real RowBufferedVMatrix::get(int i, int j) const
{
  if(current_row_index!=i)
    {
      current_row.resize(width_);
      getNewRow(i, current_row);
      current_row_index = i;
    }
  return current_row[j];
}

////////////
// getRow //
////////////
void RowBufferedVMatrix::getRow(int i, Vec v) const {
  if (current_row_index != i) {
    current_row.resize(width_);
    getNewRow(i, current_row);
    current_row_index = i;
  }
  v.copyFrom(current_row.data(), width_);
}

///////////////
// getSubRow //
///////////////
void RowBufferedVMatrix::getSubRow(int i, int j, Vec v) const
{
  if(current_row_index!=i)
    {
      current_row.resize(width_);
      getNewRow(i,current_row);
      current_row_index = i;
    }
  v.copyFrom(current_row.data()+j, v.length());
}

/////////
// dot //
/////////
real RowBufferedVMatrix::dot(int i1, int i2, int inputsize) const
{
  int w = width_;
  current_row.resize(w);
  other_row.resize(w);

  if(i1==current_row_index)
    {
      if(i2==i1)
        return pownorm(current_row.subVec(0,inputsize));
      if(i2!=other_row_index)
        {
          getNewRow(i2,other_row);
          other_row_index = i2;
        }
    }
  else if(i1==other_row_index)
    {
      if(i2==i1)
        return pownorm(other_row.subVec(0,inputsize));
      if(i2!=current_row_index)
        {
          getNewRow(i2,current_row);
          current_row_index = i2;
        }
    }
  else // i1 not cached
    {
      if(i2==current_row_index)
        {
          getNewRow(i1,other_row);
          other_row_index = i1;
        }
      else if(i2==other_row_index)
        {
          getNewRow(i1,current_row);
          current_row_index = i1;
        }
      else // neither i1 nor i2 are cached
        {
          getNewRow(i1,current_row);
          getNewRow(i2,other_row);
          current_row_index = i1;
          other_row_index = i2;
        }
    }
  return PLearn::dot(current_row.subVec(0,inputsize), other_row.subVec(0,inputsize));
}
 
/////////
// dot //
/////////
real RowBufferedVMatrix::dot(int i, const Vec& v) const
{
  if(i!=current_row_index)
    {
      current_row.resize(width_);
      getNewRow(i,current_row);
      i = current_row_index;
    }
  return PLearn::dot(current_row.subVec(0,v.length()),v);
}

} // end of namespcae PLearn
