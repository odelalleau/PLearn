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
   * $Id: InterValuesVariable.cc,v 1.6 2004/04/27 15:58:16 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ConcatRowsVariable.h"
#include "InterValuesVariable.h"
//#include "Var_operators.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** InterValuesVariable **/

// if values = [x1,x2,...,x10], the resulting variable is 
// [(x1+x2)/2,(x2+x3)/2, ... (x9+x10)/2]
PLEARN_IMPLEMENT_OBJECT(InterValuesVariable,
                        "if values = [x1,x2,...,x10], the resulting variable is [(x1+x2)/2,(x2+x3)/2, ... (x9+x10)/2]",
                        "NO HELP");

InterValuesVariable::InterValuesVariable(Variable* values) 
  : inherited(values,values->length()-1,1) 
{
    build_();
}

void
InterValuesVariable::build()
{
    inherited::build();
    build_();
}

void
InterValuesVariable::build_()
{
    if(input && !input->isColumnVec())
        PLERROR("In InterValuesVariable: input must be a column vector (single column matrix)");
}


void InterValuesVariable::recomputeSize(int& l, int& w) const
{
    if (input)
        l = input->length() - 1;
    else
        l = 0;
    w=1;
}


void InterValuesVariable::fprop()
{
  real prev_x = input->valuedata[0];
  for (int i=0;i<nelems();i++)
    {
      real next_x = input->valuedata[i+1];
      valuedata[i] = 0.5 * (prev_x + next_x);
      prev_x = next_x;
    }
}


void InterValuesVariable::bprop()
{
  real* prev_dx = &input->gradientdata[0];
  for (int i=0;i<nelems();i++)
    {
      real* next_dx = &input->gradientdata[i+1];
      *prev_dx += 0.5 * gradientdata[i];
      *next_dx += 0.5 * gradientdata[i];
      prev_dx = next_dx;
    }
}


void InterValuesVariable::symbolicBprop()
{
  Var zero(1);
  Var g1 = new InterValuesVariable(vconcat(zero & g & (VarArray)zero));

  input->accg(g1);
}



} // end of namespace PLearn


