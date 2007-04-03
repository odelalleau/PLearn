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

#include "GradientAdaboostCostVariable.h"

namespace PLearn {
using namespace std;

/** GradientAdaboostCostVariable **/

PLEARN_IMPLEMENT_OBJECT(
    GradientAdaboostCostVariable,
    "Cost for weak learner in MarginBoost version of AdaBoost",
    "Cost for a weak learner used in the functional gradient descent view of\n"
    "boosting on a margin-based loss function. See \"Functional Gradient \n"
    "Techniques for Combining Hypotheses\" by Mason et al.");

////////////////////////////////////
// GradientAdaboostCostVariable //
////////////////////////////////////
GradientAdaboostCostVariable::GradientAdaboostCostVariable(Variable* output, Variable* target)
    : inherited(output,target,output->size(),1)
{
    build_();
}

void
GradientAdaboostCostVariable::build()
{
    inherited::build();
    build_();
}

void
GradientAdaboostCostVariable::build_()
{
    if (input2 && input2->size() != input1->size())
        PLERROR("In GradientAdaboostCostVariable: target and output should have same size");
}

void
GradientAdaboostCostVariable::declareOptions(OptionList &ol)
{
    inherited::declareOptions(ol);
}

///////////////////
// recomputeSize //
///////////////////
void GradientAdaboostCostVariable::recomputeSize(int& l, int& w) const
{ l=input1->size(), w=1; }

///////////
// fprop //
///////////
void GradientAdaboostCostVariable::fprop()
{
    for(int i=0; i<length(); i++)
        valuedata[i] = -1*(2*input1->valuedata[i]-1)*(2*input2->valuedata[i]-1);
}

///////////
// bprop //
///////////
void GradientAdaboostCostVariable::bprop()
{
    for(int i=0; i<length(); i++)
        input1->gradientdata[i] += (gradientdata[i])*-2*(2*input2->valuedata[i]-1);
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
