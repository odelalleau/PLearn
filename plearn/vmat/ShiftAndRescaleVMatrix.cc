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
   * $Id: ShiftAndRescaleVMatrix.cc,v 1.1 2002/10/03 07:35:28 plearner Exp $
   ******************************************************* */

#include "ShiftAndRescaleVMatrix.h"
#include "VMat_maths.h"

namespace PLearn <%
using namespace std;

/** ShiftAndRescaleVMatrix **/

ShiftAndRescaleVMatrix::
ShiftAndRescaleVMatrix(VMat underlying_distr, Vec the_shift, Vec the_scale)
  : VMatrix(underlying_distr->length(), underlying_distr->width()),
  distr(underlying_distr), shift(the_shift), scale(the_scale)
{
  fieldinfos = distr->getFieldInfos();
}


ShiftAndRescaleVMatrix::
ShiftAndRescaleVMatrix(VMat underlying_distr, int n_inputs)
  :VMatrix(underlying_distr->length(), underlying_distr->width()),
  distr(underlying_distr), shift(underlying_distr->width()),
   scale(underlying_distr->width())
{
  fieldinfos = distr->getFieldInfos();
  
  computeMeanAndStddev(underlying_distr, shift, scale);
  negateElements(shift);
  for (int i=0;i<scale.length();i++) 
    if (scale[i]==0)
      {
        PLWARNING("ShiftAndRescale: data column number %d is constant",i);
        scale[i]=1;
      }
  invertElements(scale);
  shift.subVec(n_inputs,shift.length()-n_inputs).fill(0);
  scale.subVec(n_inputs,shift.length()-n_inputs).fill(1);
}

ShiftAndRescaleVMatrix::
ShiftAndRescaleVMatrix(VMat underlying_distr, int n_inputs, int n_train)
  :VMatrix(underlying_distr->length(), underlying_distr->width()),
  distr(underlying_distr), shift(underlying_distr->width()),
   scale(underlying_distr->width())
{
  fieldinfos = distr->getFieldInfos();
  
  VMat train_distr = underlying_distr.subMatRows(0,n_train);
  computeMeanAndStddev(train_distr, shift, scale);
  shift.subVec(n_inputs,shift.length()-n_inputs).fill(0);
  scale.subVec(n_inputs,shift.length()-n_inputs).fill(1);
  // cout << "shift = " << shift << endl;
  // cout << "scale = " << scale << endl;
  negateElements(shift);
  invertElements(scale);
}
                                         
real ShiftAndRescaleVMatrix::get(int i, int j) const
{
  return (distr->get(i,j) + shift[j]) * scale[j];
}

void ShiftAndRescaleVMatrix::getSubRow(int i, int j, Vec v) const
{
  distr->getSubRow(i,j,v);
  for(int jj=0; jj<v.length(); jj++)
    v[jj] = (v[jj] + shift[j+jj]) * scale[j+jj];
}

%> // end of namespcae PLearn
