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
   * $Id: ExpVariable.cc,v 1.1 2002/10/23 23:32:34 dorionc Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ExpVariable.h"

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


/** ExpVariable **/

ExpVariable::ExpVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) {}


IMPLEMENT_NAME_AND_DEEPCOPY(ExpVariable);

void ExpVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }


void ExpVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ExpVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ExpVariable");
}


void ExpVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ExpVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ExpVariable");
}


void ExpVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    valuedata[i] = safeexp(input->valuedata[i]);
}


void ExpVariable::bprop()
{
  for(int i=0; i<nelems(); i++)
    input->gradientdata[i] += gradientdata[i]*valuedata[i];
}


//! Incorrect!
// void ExpVariable::bbprop()
// {
//   if (input->diaghessian.length()==0)
//     input->resizeDiagHessian();
//   for(int i=0; i<nelems(); i++)
//     {
//       real yi = valuedata[i];
//       input->diaghessiandata[i] += diaghessiandata[i] * yi*yi + gradientdata[i] * yi;
//     }
// }

void ExpVariable::symbolicBprop()
{
  input->accg(g * Var(this));
}


// R{exp(x)} = exp(x) R(x)
void ExpVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  for(int i=0; i<nelems(); i++)
    rvaluedata[i] = input->rvaluedata[i] * valuedata[i];
}



%> // end of namespace PLearn


