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
   * $Id: UnequalConstantVariable.cc,v 1.1 2002/10/23 23:32:34 dorionc Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "UnequalConstantVariable.h"

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


/** UnequalConstantVariable **/

string UnequalConstantVariable::info() const
{ return string("EqualConstantVariable (!= ")+tostring(c)+")"; }


UnequalConstantVariable::UnequalConstantVariable(Variable* input1, real c_)
  : UnaryVariable(input1, input1->length(), input1->width()), c(c_)
{}

  
IMPLEMENT_NAME_AND_DEEPCOPY(UnequalConstantVariable);

void UnequalConstantVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }


void UnequalConstantVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "UnequalConstantVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, c);
  readFooter(in, "UnequalConstantVariable");
}


void UnequalConstantVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "UnequalConstantVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, c);
  writeFooter(out, "UnequalConstantVariable");
}



void UnequalConstantVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    valuedata[i] = (input->valuedata[i] != c);
}


// not really differentiable (zero gradient almost everywhere)
void UnequalConstantVariable::bprop() {}

void UnequalConstantVariable::symbolicBprop() {}



%> // end of namespace PLearn


