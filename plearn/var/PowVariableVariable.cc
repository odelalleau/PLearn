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
   * $Id: PowVariableVariable.cc,v 1.3 2003/08/13 08:13:17 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "PowVariableVariable.h"
#include "Var_utils.h"

namespace PLearn <%
using namespace std;


/** PowVariableVariable **/

/* PowVariableVariable: x^y where x and y are variables but y is scalar 
   or it has the same size as x */

PowVariableVariable::PowVariableVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isScalar() && (input1->length()!=input2->length() || input1->width()!=input2->width()))
    PLERROR("IN FunctionPowVariableVariable(Variable* input1, Variable* input2) input1 and input2 must have the same size or input2 must be scalar");
}

  
PLEARN_IMPLEMENT_OBJECT(PowVariableVariable, "ONE LINE DESCR", "NO HELP");


void PowVariableVariable::recomputeSize(int& l, int& w) const
{ l=input1->length(); w=input1->width(); }








void PowVariableVariable::fprop()
{
  if (input2->isScalar())
    {
      real p = input2->valuedata[0];
      for(int i=0; i<nelems(); i++)
        if (input1->valuedata[i]>0)
          valuedata[i] = pow(input1->valuedata[i],p);
        else
          valuedata[i] = 0;
    }
  else
    for(int i=0; i<nelems(); i++)
      if (input1->valuedata[i]>0)
        valuedata[i] = pow(input1->valuedata[i],input2->valuedata[i]);
      else
        valuedata[i] = 0;
}


void PowVariableVariable::bprop()
{
  if (input2->isScalar())
    {
      real p = input2->valuedata[0];
      real& dp = input2->gradientdata[0];
      for(int i=0; i<nelems(); i++)
        {
          if (input1->valuedata[i]>0)
            {
              input1->gradientdata[i] += 
                gradientdata[i] * valuedata[i] * p / input1->valuedata[i];
              dp += gradientdata[i] * valuedata[i] * safeflog(input1->valuedata[i]);
            }
        }
    }
  else
    {
      for(int i=0; i<nelems(); i++)
        {
          if (input1->valuedata[i]>0)
            {
              input1->gradientdata[i] += 
                gradientdata[i] * valuedata[i] * input2->valuedata[i] 
                / input1->valuedata[i];
              input2->gradientdata[i] += 
                gradientdata[i] * valuedata[i] * safeflog(input1->valuedata[i]);
            }
        }
    }
}


void PowVariableVariable::symbolicBprop()
{
  Var gv = g * Var(this);
  Var input1zero = (input1<=0.0);
  Var zero(length(), width());
  input1->accg(ifThenElse(input1zero, zero, gv * input2 / input1));
  if (input2->isScalar())
    input2->accg(dot(gv,ifThenElse(input1zero, zero, log(input1))));
  else
    input2->accg(ifThenElse(input1zero, zero, gv * log(input1)));
}



%> // end of namespace PLearn


