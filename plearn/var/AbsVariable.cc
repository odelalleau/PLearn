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
   * $Id: AbsVariable.cc,v 1.6 2004/04/27 16:02:26 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "AbsVariable.h"
#include "IfThenElseVariable.h"
#include "IsAboveThresholdVariable.h"
#include "Var_operators.h"

namespace PLearn {
using namespace std;



/** AbsVariable **/

PLEARN_IMPLEMENT_OBJECT(AbsVariable, "ONE LINE DESCR", "NO HELP");

AbsVariable::AbsVariable(Variable* input)
  : inherited(input, input->length(), input->width())
{}


void AbsVariable::recomputeSize(int& l, int& w) const
{
    if (input) {
        l = input->length();
        w = input->width();
    } else
        l = w = 0;
}

void AbsVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    {
      real val = input->valuedata[i];
      valuedata[i] = val>=0 ?val :-val;
    }
}


void AbsVariable::bprop()
{
  for(int i=0; i<nelems(); i++)
    {
      real val = input->valuedata[i];
      if(val>=0)        
        input->gradientdata[i] += gradientdata[i];
      else
        input->gradientdata[i] -= gradientdata[i];
    }
}


void AbsVariable::symbolicBprop()
{
  input->accg(ifThenElse(input >= 0., g, -g));
}


// R{abs(x)} = R(x) while x >= 0
// R{abs(x)} = -R(x) while x < 0
void AbsVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int n=nelems();
  for(int i=0; i<n; i++)
    if (input->valuedata[i] < 0) 
        rvaluedata[i] = -input->rvaluedata[i];
        else 
        rvaluedata[i] = input->rvaluedata[i];
}



} // end of namespace PLearn


