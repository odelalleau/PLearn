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
   * $Id: VarElementVariable.cc,v 1.4 2004/02/20 21:11:54 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "VarElementVariable.h"
#include "ElementAtPositionVariable.h"

namespace PLearn {
using namespace std;


/** VarElementVariable **/

VarElementVariable::VarElementVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, 1, 1)
{
  if(input2->nelems()>2)
    PLERROR("IN VarElementVariable(Variable* input1, Variable* input2) input2 must have 1 (a k position index) or 2 elements (an i,j position index)");
}

  
PLEARN_IMPLEMENT_OBJECT(VarElementVariable, "ONE LINE DESCR", "NO HELP");


void VarElementVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }








void VarElementVariable::fprop()
{
  if(input2->isScalar()) // scalar position index k
    {
      int k = int(input2->valuedata[0]);
#ifdef BOUNDCHECK
      if (k >= input1->length())
          PLERROR("VarElementVariable::fprop() - k = %d is out of range (size is %d)", k, input1->length());
#endif
      valuedata[0] = input1->valuedata[k];
    }
  else // (i,j) position index
    {
      int i = int(input2->valuedata[0]);
      int j = int(input2->valuedata[1]);
#ifdef BOUNDCHECK
      if ((i * input1->width() + j) >= input1->width() * input1->length())
          PLERROR("VarElementVariable::fprop() - (%d, %d) out of range"
                  "(size is %d)", i, j, input1->length() * input1->width());
#endif
      valuedata[0] = input1->valuedata[i*input1->width()+j];
    }
}


void VarElementVariable::bprop()
{
  if(input2->isScalar()) // scalar position index k
    {
      int k = int(input2->valuedata[0]);
      input1->gradientdata[k] += gradientdata[0];
    }
  else // (i,j) position index
    {
      int i = int(input2->valuedata[0]);
      int j = int(input2->valuedata[1]);
      input1->gradientdata[i*input1->width()+j] += gradientdata[0];
    }
}


void VarElementVariable::symbolicBprop()
{
  input1->accg(new ElementAtPositionVariable(g,input2,input1->length(),input1->width()));
}


void VarElementVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  if(input2->isScalar()) // scalar position index k
    {
      int k = int(input2->valuedata[0]);
      rvaluedata[0] = input1->rvaluedata[k];
    }
  else // (i,j) position index
    {
      int i = int(input2->valuedata[0]);
      int j = int(input2->valuedata[1]);
      rvaluedata[0] = input1->rvaluedata[i*input1->width()+j];
    }
}



} // end of namespace PLearn


