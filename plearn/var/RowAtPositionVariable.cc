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

#include "RowAtPositionVariable.h"
#include "VarRowVariable.h"

namespace PLearn {
using namespace std;


/** RowAtPositionVariable **/

PLEARN_IMPLEMENT_OBJECT(RowAtPositionVariable,
                        "Variables positionned inside a larger zero variable ...",
                        "NO HELP");

RowAtPositionVariable::RowAtPositionVariable()
    : length_(-1)
{ }
  
RowAtPositionVariable::RowAtPositionVariable(Variable* input1, Variable* input2, int the_length)
    : inherited(input1, input2, the_length, input1->width()), length_(the_length)
{
    build_();
}

void
RowAtPositionVariable::build()
{
    inherited::build();
    build_();
}

void
RowAtPositionVariable::build_()
{
    if (input1 && input2) {
        if(!input1->isRowVec())
            PLERROR("In RowAtPositionVariable: input1 must be a single row");
        if(!input2->isScalar())
            PLERROR("In RowAtPositionVariable: position must be a scalar");
    }
}

void
RowAtPositionVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "length_", &RowAtPositionVariable::length_, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void RowAtPositionVariable::recomputeSize(int& l, int& w) const
{
    l=length_;
    if (input1)
        w = input1->width();
    else
        w = 0;
}

void RowAtPositionVariable::fprop()
{
    value.clear();
    int i = (int)input2->valuedata[0];
    matValue(i) << input1->value;
}


void RowAtPositionVariable::bprop()
{
    int i = (int)input2->valuedata[0];
    input1->gradient += matGradient(i);
}


void RowAtPositionVariable::symbolicBprop()
{
    input1->accg(new VarRowVariable(g,input2));
}


void RowAtPositionVariable::rfprop()
{
    if (rValue.length()==0) resizeRValue();
    //rValue.clear();
    int i = (int)input2->valuedata[0];
    matRValue(i) << input1->rValue;
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
