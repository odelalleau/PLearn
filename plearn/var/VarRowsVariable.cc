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

#include "VarRowsVariable.h"

namespace PLearn {
using namespace std;


/** VarRowsVariable **/

PLEARN_IMPLEMENT_OBJECT(VarRowsVariable,
                        "Variable that is a subset of a matrix's rows; "
                        "input1 : matrix from which rows are selected; "
                        "input2 : vector whose elements are row indices in input1",
                        "NO HELP");

VarRowsVariable::VarRowsVariable(Variable *input1, Variable *input2)
    : inherited(input1, input2, input2->length(), input1->width())
{
    build_();
}

void
VarRowsVariable::build()
{
    inherited::build();
    build_();
}

void
VarRowsVariable::build_()
{
    if (input1)
        input1->allowPartialUpdates();
}

void VarRowsVariable::recomputeSize(int& l, int& w) const
{
    if (input1 && input2) {
        l = input2->length();
        w = input1->width();
    } else
        l = w = 0;
}

void VarRowsVariable::fprop()
{
    matValue << input1->matValue.rows(input2->value);
}


void VarRowsVariable::bprop()
{
    for (int i = 0; i < input2->length(); ++i)
    {
        int r=int(input2->value[i]);
        input1->matGradient.row(r) += matGradient.row(i);
        input1->updateRow(r);
    }
}


void VarRowsVariable::symbolicBprop()
{
    PLERROR("VarRowsVariable::symbolicBprop() not implemented");
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
