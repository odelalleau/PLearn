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
   * $Id: VarRowVariable.cc,v 1.5 2004/04/27 15:58:16 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "VarRowVariable.h"
#include "RowAtPositionVariable.h"

namespace PLearn {
using namespace std;

/** VarRowVariable **/
  
PLEARN_IMPLEMENT_OBJECT(VarRowVariable,
                        "Variable that is the row of the input1 variable indexed by the input2 variable",
                        "NO HELP");

VarRowVariable::VarRowVariable(Variable* input1, Variable* input2)
  : inherited(input1, input2, 1, input1->width())
{
    build_();
}

void
VarRowVariable::build()
{
    inherited::build();
    build_();
}

void
VarRowVariable::build_()
{
    if (input1 && input2) {
        if (!input2->isScalar())
            PLERROR("IN VarRowVariable(Variable* input1, Variable* input2) input2 must be scalar as it is supposed to be an integer index");
        input1->allowPartialUpdates();
    }
}


void VarRowVariable::recomputeSize(int& l, int& w) const
{
    l = 1;
    if (input1)
        w = input1->width();
    else
        w = 0;
}

void VarRowVariable::fprop()
{
  int i = int(input2->valuedata[0]);
  value << input1->matValue(i);
}


void VarRowVariable::bprop()
{
  int i = int(input2->valuedata[0]);
  Vec input1_gradient_i = input1->matGradient(i);
  input1_gradient_i += gradient;
  input1->updateRow(i);
}


void VarRowVariable::symbolicBprop()
{
  input1->accg(new RowAtPositionVariable(g,input2,input1->length()));
}


void VarRowVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue(); 
  int i = int(input2->valuedata[0]);
  rValue << input1->matRValue(i);
}



} // end of namespace PLearn


