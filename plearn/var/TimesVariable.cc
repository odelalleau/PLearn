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
   * $Id: TimesVariable.cc,v 1.5 2004/02/20 21:11:54 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "TimesVariable.h"
#include "Var_operators.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** TimesVariable **/

TimesVariable::TimesVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(input1->length()!=input2->length() || input1->width()!=input2->width())
    PLERROR("In TimesVariable: input1 and input2 must have exactly the same size");
}


PLEARN_IMPLEMENT_OBJECT(TimesVariable, "ONE LINE DESCR", "NO HELP");


void TimesVariable::recomputeSize(int& l, int& w) const
{ l=input1->length(); w=input1->width(); }








void TimesVariable::fprop()
{
  for(int k=0; k<nelems(); k++)
    valuedata[k] = input1->valuedata[k]*input2->valuedata[k];
}


void TimesVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    {
      input1->gradientdata[k] += input2->valuedata[k]*gradientdata[k];
      input2->gradientdata[k] += input1->valuedata[k]*gradientdata[k];
    }
}


void TimesVariable::symbolicBprop()
{
  input1->accg(g*input2);
  input2->accg(g*input1);
}



} // end of namespace PLearn


