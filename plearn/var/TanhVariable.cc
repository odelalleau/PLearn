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
   * $Id: TanhVariable.cc,v 1.6 2004/04/27 16:02:26 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "TanhVariable.h"
#include "Var_operators.h"

namespace PLearn {
using namespace std;


/** TanhVariable **/

PLEARN_IMPLEMENT_OBJECT(TanhVariable,
                        "ONE LINE DESCR",
                        "NO HELP");

TanhVariable::TanhVariable(Variable* input) 
  : inherited(input, input->length(), input->width())
{}

void TanhVariable::recomputeSize(int& l, int& w) const
{
    if (input) {
        l = input->length();
        w = input->width();
    } else
        l = w = 0;
}

void TanhVariable::fprop()
{
  int l = nelems();
  real* inputptr = input->valuedata;
  real* ptr = valuedata;
  for(int i=0; i<l; i++)
    *ptr++ = tanh(*inputptr++);
}


void TanhVariable::bprop()
{
  int l = nelems();
  real* inputgradientptr = input->gradientdata;
  real* gradientptr = gradientdata;
  real* valueptr = valuedata;
  for(int i=0; i<l; i++)
    *inputgradientptr++ += *gradientptr++ * (1.0-square(*valueptr++));
}


void TanhVariable::bbprop()
{
  if (input->diaghessian.length()==0)
    input->resizeDiagHessian();
  for(int i=0; i<nelems(); i++)
    {
      real yi=valuedata[i];
      real fprime=(1-yi*yi);
      input->diaghessiandata[i] += diaghessiandata[i] * fprime * fprime;
    }
}


void TanhVariable::symbolicBprop()
{
  Var v(this);
  input->accg(g * (1. - square(v)));
}


// R(tanh(x)) = (1-tanh(x)^2)R(x)
void TanhVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int l = nelems();
  real* inputptr = input->rvaluedata;
  real* valueptr = valuedata;
  real* ptr = rvaluedata;
  for(int i=0; i<l; i++)
    *ptr++ = *inputptr++ * (1.0 - square(*valueptr++));
}



} // end of namespace PLearn


