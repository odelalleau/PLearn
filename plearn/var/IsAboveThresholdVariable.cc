// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: IsAboveThresholdVariable.cc,v 1.1 2002/10/23 23:32:34 dorionc Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "IsAboveThresholdVariable.h"

// From Old UnaryVariable.cc: all includes are putted in every file.
// To be revised manually 
#include "DisplayUtils.h"
#include "UnaryVariable.h"
#include "Var.h"
#include "TMat_maths.h"
#include "pl_erf.h"
#include "Var_utils.h"
namespace PLearn <%
using namespace std;


/** IsAboveThresholdVariable **/

IsAboveThresholdVariable::
IsAboveThresholdVariable(Variable* input, real the_threshold, real the_truevalue, real the_falsevalue)
  :UnaryVariable(input, input->length(), input->width()),
   threshold(the_threshold), truevalue(the_truevalue), falsevalue(the_falsevalue)
{}


IMPLEMENT_NAME_AND_DEEPCOPY(IsAboveThresholdVariable);

void IsAboveThresholdVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }


void IsAboveThresholdVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "IsAboveThresholdVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, threshold);
  PLearn::deepRead(in, old2new, truevalue);
  PLearn::deepRead(in, old2new, falsevalue);
  readFooter(in, "IsAboveThresholdVariable");
}


void IsAboveThresholdVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "IsAboveThresholdVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, threshold);
  PLearn::deepWrite(out, already_saved, truevalue);
  PLearn::deepWrite(out, already_saved, falsevalue);
  writeFooter(out, "IsAboveThresholdVariable");
}



void IsAboveThresholdVariable::fprop()
{
  for(int i=0; i<input->nelems(); i++)
    if(input->valuedata[i]>=threshold)
      valuedata[i] = truevalue;
    else
      valuedata[i] = falsevalue;
}


void IsAboveThresholdVariable::bprop() {}

void IsAboveThresholdVariable::symbolicBprop() {}


void IsAboveThresholdVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
}



%> // end of namespace PLearn


