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
   * $Id: InvertElementsVariable.cc,v 1.3 2003/08/13 08:13:17 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "InvertElementsVariable.h"
#include "Var_utils.h"

namespace PLearn <%
using namespace std;


/** InvertElementsVariable **/

InvertElementsVariable::InvertElementsVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) {}


PLEARN_IMPLEMENT_OBJECT(InvertElementsVariable, "ONE LINE DESCR", "NO HELP");

void InvertElementsVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }








void InvertElementsVariable::fprop()
{
  for(int k=0; k<nelems(); k++)
    valuedata[k] = 1.0/input->valuedata[k];
}


void InvertElementsVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    {
      real inputvalue_k = input->valuedata[k];
      input->gradientdata[k] -= gradientdata[k]/(inputvalue_k*inputvalue_k);
    }
}


void InvertElementsVariable::symbolicBprop()
{
  Var v = input * input;
  input->accg(-g/v);
}


// R(1/x) = -1/(x^2) R(x)
void InvertElementsVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  for(int k=0; k<nelems(); k++)
    {
    real value_k = valuedata[k];
    rvaluedata[k] = - input->rvaluedata[k] * value_k * value_k;
    }
}



%> // end of namespace PLearn


