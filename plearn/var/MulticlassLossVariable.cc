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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "MulticlassLossVariable.h"

namespace PLearn {
using namespace std;


/** MulticlassLossVariable **/

PLEARN_IMPLEMENT_OBJECT(MulticlassLossVariable, 
                        "ONE LINE DESCR",
                        "NO HELP");

MulticlassLossVariable::MulticlassLossVariable(Variable* netout, Variable* target)
    : inherited(netout,target,1,1)
{
    build_();
}

void
MulticlassLossVariable::build()
{
    inherited::build();
    build_();
}

void
MulticlassLossVariable::build_()
{
    if (input1 && input2) {
        // input1 and input2 are (respectively) netout and target from constructor
        if(input1->size() != input2->size())
            PLERROR("In MulticlassLossVariable: netout and target must the same size");
    }
}


void MulticlassLossVariable::recomputeSize(int& l, int& w) const
{ l=1, w=1; }


void MulticlassLossVariable::fprop()
{
    real cost = 0.0;
    for (int i=0; i<input1->size(); i++)
    {
        real output = input1->valuedata[i];
        real target = input2->valuedata[i];
        PLASSERT( fast_exact_is_equal(target, 1) ||
                fast_exact_is_equal(target, 0) );
        cost += fast_exact_is_equal(target, 1) ? output<0.5 : output>0.5;
    }
    valuedata[0] = cost;
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
