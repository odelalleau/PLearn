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
   * $Id: TresholdVMatrix.cc,v 1.1 2002/12/02 22:11:07 zouave Exp $
   ******************************************************* */

#include "TresholdVMatrix.h"

namespace PLearn <%
using namespace std;


/** TresholdVMatrix **/

TresholdVMatrix::TresholdVMatrix(VMat the_underlying_distr, real treshold_, real the_cold_value, real the_hot_value)
  :RowBufferedVMatrix(the_underlying_distr->length(), the_underlying_distr->width()),
   underlying_distr(the_underlying_distr), treshold(treshold_), cold_value(the_cold_value), hot_value(the_hot_value)
{}

void TresholdVMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In TresholdVMatrix::getRow OUT OF BOUNDS");
  if(v.length()!=width())
    PLERROR("In TresholdVMatrix::getRow v.length() must be equal to the VMat's width");
#endif
  underlying_distr->getRow(i,v);
  int p= v.size()-1;
  if(gt_treshold && v[p] <= treshold || !gt_treshold && v[p] < treshold)
    v[p]= cold_value;
  else
    v[p]= hot_value;
}

%> // end of namespcae PLearn
