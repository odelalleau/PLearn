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
   * $Id: SoftmaxLossVariable.cc,v 1.5 2004/02/20 21:11:53 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ExpVariable.h"
#include "RowAtPositionVariable.h"
#include "SoftmaxLossVariable.h"
#include "Var_operators.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** SoftmaxLossVariable **/
PLEARN_IMPLEMENT_OBJECT(SoftmaxLossVariable, "ONE LINE DESCR", "NO HELP");

SoftmaxLossVariable::SoftmaxLossVariable(Variable* input1, Variable* input2) 
:BinaryVariable(input1, input2, 1, 1)
{
  if(!input2->isScalar())
    PLERROR("In RowAtPositionVariable: position must be a scalar");
}


void SoftmaxLossVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }








void SoftmaxLossVariable::fprop()
{
  int classnum = (int)input2->valuedata[0];
  real input_index = input1->valuedata[classnum];
  real sum=0;
  for(int i=0; i<input1->nelems(); i++)
    sum += safeexp(input1->valuedata[i]-input_index);
  valuedata[0] = 1.0/sum;
}


void SoftmaxLossVariable::bprop()
{
  int classnum = (int)input2->valuedata[0];
  real input_index = input1->valuedata[classnum];
  real vali = valuedata[0];
  for(int i=0; i<input1->nelems(); i++)
  {
    if (i!=classnum)
       //input1->gradientdata[i] = -gradientdata[i]/*?*/*vali*vali*safeexp(input1->valuedata[i]-input_index);
       input1->gradientdata[i] = -gradientdata[i]*vali*vali*safeexp(input1->valuedata[i]-input_index);
    else
       input1->gradientdata[i] = gradientdata[i]*vali*(1.-vali);
  }
}


void SoftmaxLossVariable::bbprop()
{
  PLERROR("SofmaxVariable::bbprop() not implemented");
}


void SoftmaxLossVariable::symbolicBprop()
{
  Var gi = -g * Var(this) * Var(this) * exp(input1-input1(input2));
  Var gindex = new RowAtPositionVariable(g * Var(this), input2, input1->length());
  input1->accg(gi+gindex);
}


// R{ s_i = exp(x_i) / sum_j exp(x_j) }   = (s_i(1-s_i) - sum_{k!=i} s_i s_k) R(s_i) = s_i ((1-s_i) - sum_{k!=i} s_k) R(s_i)
void SoftmaxLossVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();

  int classnum = (int)input2->valuedata[0];
  real input_index = input1->valuedata[classnum];
  real vali = valuedata[0];
  real sum = 0;
  for(int i=0; i<input1->nelems(); i++)
  {
    real res =vali * input1->rvaluedata[i];
    if (i != classnum)
       sum -= res * vali* safeexp(input1->valuedata[i]-input_index);
    else sum += res * (1 - vali);
  }
  rvaluedata[0] = sum;
}



} // end of namespace PLearn


