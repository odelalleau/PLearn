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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "ConfRatedAdaboostCostVariable.h"

namespace PLearn {
using namespace std;

/** ConfRatedAdaboostCostVariable **/

PLEARN_IMPLEMENT_OBJECT(
    ConfRatedAdaboostCostVariable,
    "Cost used for Confidence-rated Adaboost ",
    "See \"Improved Boosting Algorithms Using Confidence-rated Predictions\" by\n"
    "Schapire and Singer.");

////////////////////////////////////
// ConfRatedAdaboostCostVariable //
////////////////////////////////////
ConfRatedAdaboostCostVariable::ConfRatedAdaboostCostVariable(Variable* output, Variable* target, Variable* alpha)
    : inherited(VarArray(output,target) & Var(alpha),output->size(),1)
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

    if (varray[1] && varray[0] && varray[1]->size() != varray[0]->size())
        PLERROR("In ConfRatedAdaboostCostVariable: target and output should have same size");

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
{ l=varray[0]->size(), w=1; }

///////////
// fprop //
///////////
void ConfRatedAdaboostCostVariable::fprop()
{
  
    int signed_target;
    for(int i=0; i<length(); i++)
    {
        signed_target = 2*int(varray[1]->valuedata[i])-1;  
        valuedata[i] = exp(-1*varray[2]->valuedata[0]*signed_target*(2*varray[0]->valuedata[i]-1));
    }
}

///////////
// bprop //
///////////
void ConfRatedAdaboostCostVariable::bprop()
{
    for(int i=0; i<length(); i++)
    {
        varray[0]->gradientdata[i] += -2*gradientdata[i]*valuedata[i]*varray[2]->valuedata[0]*(2*int(varray[1]->valuedata[i])-1);  
        varray[2]->gradientdata[0] += -gradientdata[i]*valuedata[i]*(2*varray[0]->valuedata[i]-1)*(2*int(varray[1]->valuedata[i])-1);  
    }
}

void ConfRatedAdaboostCostVariable::symbolicBprop()
{
    PLERROR("ConfRatedAdaboostCostVariable::symbolicBprop() not implemented");
}

} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
