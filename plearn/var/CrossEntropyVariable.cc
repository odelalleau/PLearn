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
   * $Id: CrossEntropyVariable.cc,v 1.4 2003/10/28 20:35:55 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "CrossEntropyVariable.h"

namespace PLearn <%
using namespace std;



/** CrossEntropyVariable **/

PLEARN_IMPLEMENT_OBJECT(CrossEntropyVariable, "ONE LINE DESCR", "NO HELP");

CrossEntropyVariable::CrossEntropyVariable(Variable* netout, Variable* target)
  :BinaryVariable(netout,target,1,1)
{
  if(netout->size() != target->size())
    PLERROR("In CrossEntropyVariable: netout and target must have the same size");
}


void CrossEntropyVariable::recomputeSize(int& l, int& w) const
{ l=1, w=1; }


void CrossEntropyVariable::fprop()
{
  real cost = 0.0;
  for (int i=0; i<input1->size(); i++)
  {
    real output = input1->valuedata[i];
    real target = input2->valuedata[i];
    cost += target*log(output) + (1.0-target)*log(1.0-output);
  }
  valuedata[0] = -cost;
}


void CrossEntropyVariable::bprop()
{
  real gr = *gradientdata;
  for (int i=0; i<input1->size(); i++)
  {
    real output = input1->valuedata[i];
    real target = input2->valuedata[i];
    input1->gradientdata[i] += gr*(-target/output + (1.0-target)/(1.0-output));
  }
}



%> // end of namespace PLearn


