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
   * $Id: ConfRatedAdaboostCostVariable.cc,v 1.1 2004/11/12 20:02:51 larocheh Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ConfRatedAdaboostCostVariable.h"

namespace PLearn {
using namespace std;

/** ConfRatedAdaboostCostVariable **/

PLEARN_IMPLEMENT_OBJECT(
  ConfRatedAdaboostCostVariable,
  "Cost used for confidence-rated Adaboost ",
  "NO HELP");

////////////////////////////////////
// ConfRatedAdaboostCostVariable //
////////////////////////////////////
ConfRatedAdaboostCostVariable::ConfRatedAdaboostCostVariable(Variable* output, Variable* target, Variable* alpha)
  : inherited(VarArray(output,target) & Var(alpha),1,1)
{
    build_();
}

void
ConfRatedAdaboostCostVariable::build()
{
    inherited::build();
    build_();
}

void
ConfRatedAdaboostCostVariable::build_()
{
    // varrar[1] is target from constructor
    if (varray[1] && varray[1]->size() != 1)
        PLERROR("In ConfRatedAdaboostCostVariable: target represents a class (0,1) and must be a single integer");
    // varray[0] is output from constructor
    if (varray[0] && varray[0]->size() != 1)
        PLERROR("In ConfRatedAdaboostCostVariable: output represents a class (0,1) and must be a single real");
    // varray[2] is output from constructor
    if (varray[2] && varray[2]->size() != 1)
        PLERROR("In ConfRatedAdaboostCostVariable: target must be a single real");
}

void
ConfRatedAdaboostCostVariable::declareOptions(OptionList &ol)
{
}

///////////////////
// recomputeSize //
///////////////////
void ConfRatedAdaboostCostVariable::recomputeSize(int& l, int& w) const
{ l=1, w=1; }

///////////
// fprop //
///////////
void ConfRatedAdaboostCostVariable::fprop()
{
  int signed_target = 2*int(varray[1]->valuedata[0])-1;  
  valuedata[0] = exp(-1*varray[2]->valuedata[0]*signed_target*(2*varray[0]->valuedata[0]-1));
}

///////////
// bprop //
///////////
void ConfRatedAdaboostCostVariable::bprop()
{
  varray[0]->gradientdata[0] = -2*gradientdata[0]*valuedata[0]*varray[2]->valuedata[0]*(2*int(varray[1]->valuedata[0])-1);  
  varray[2]->gradientdata[0] = -gradientdata[0]*valuedata[0]*(2*varray[0]->valuedata[0]-1)*(2*int(varray[1]->valuedata[0])-1);  
}

void ConfRatedAdaboostCostVariable::symbolicBprop()
{
  PLERROR("ConfRatedAdaboostCostVariable::symbolicBprop() not implemented");
}

} // end of namespace PLearn
