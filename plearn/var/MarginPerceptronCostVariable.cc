// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion
// Copyright (C) 2003 Olivier Delalleau

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
   * $Id: MarginPerceptronCostVariable.cc,v 1.1 2004/04/11 19:51:02 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "MarginPerceptronCostVariable.h"

namespace PLearn {
using namespace std;

/** MarginPerceptronCostVariable **/

PLEARN_IMPLEMENT_OBJECT(
  MarginPerceptronCostVariable,
  "Compute sigmoid of its first input, and then computes the negative "
  "cross-entropy cost",
  "NO HELP");

////////////////////////////////////
// MarginPerceptronCostVariable //
////////////////////////////////////
MarginPerceptronCostVariable::
MarginPerceptronCostVariable(Variable* output, Variable* target, real m)
  :BinaryVariable(output,target,1,1),margin(m)
{
  if(target->size() != 1)
    PLERROR("In MarginPerceptronCostVariable: target represents a class (0...n_classes-1) and must be a single integer");
}

///////////////////
// recomputeSize //
///////////////////
void MarginPerceptronCostVariable::recomputeSize(int& l, int& w) const
{ l=1, w=1; }

///////////
// fprop //
///////////
void MarginPerceptronCostVariable::fprop()
{
  real cost = 0.0;
  int target = int(input2->valuedata[0]);
  for (int i=0; i<input1->size(); i++)
  {
    real output = input1->valuedata[i];
    int signed_target = input1->size()==1?target*2-1:(target==i) - (target!=i);
    real diff = margin - signed_target * output;
    if (diff>0)
      cost += diff;
  }
  valuedata[0] = cost;
}

///////////
// bprop //
///////////
void MarginPerceptronCostVariable::bprop()
{
  real gr = *gradientdata;
  int target = int(input2->valuedata[0]);
  for (int i=0; i<input1->size(); i++)
  {
    real output = input1->valuedata[i];
    int signed_target = input1->size()==1?target*2-1:(target==i) - (target!=i);
    real diff = margin - signed_target * output;
    if (diff>0)
      input1->gradientdata[i] -= gr*signed_target;
  }
}

} // end of namespace PLearn
