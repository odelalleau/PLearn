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
   * $Id: WeightedSumSquareVariable.cc,v 1.4 2004/02/17 21:05:19 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "WeightedSumSquareVariable.h"
#include "Var_operators.h"
//#include "Var_utils.h"

namespace PLearn <%
using namespace std;




/** WeightedSumSquareVariable **/


PLEARN_IMPLEMENT_OBJECT(WeightedSumSquareVariable, "ONE LINE DESCR", "NO HELP");

WeightedSumSquareVariable::WeightedSumSquareVariable(Variable* input, Variable* weights)
  :BinaryVariable(input,weights,1,1)
{
  if(input->nelems() != weights->nelems())
    PLERROR("In WeightedSumSquareVariable: input and weights must be the same size;"
	    " input->nelems()=%d weights->nelems()=&d.",
	    input->nelems(), weights->nelems());
}


void WeightedSumSquareVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }








void WeightedSumSquareVariable::fprop()
{
  int n=input1->nelems();
  *valuedata= 0;
  for(int i=0; i<n; i++)
    *valuedata+= input1->valuedata[i]*input1->valuedata[i] * input2->valuedata[i];
}


void WeightedSumSquareVariable::bprop()
{
  int n=input1->nelems();
  for(int i=0; i<n; i++)
    {
      input1->gradientdata[i]+= 2.0 * input1->valuedata[i] * input2->valuedata[i] * *gradientdata;
      input2->gradientdata[i]+= input1->valuedata[i] * input1->valuedata[i] * *gradientdata;
    }
}


void WeightedSumSquareVariable::symbolicBprop()
{
  input1->accg(2.0 * (g*input1*input2));
  input2->accg(g*input1*input1);
}



%> // end of namespace PLearn


