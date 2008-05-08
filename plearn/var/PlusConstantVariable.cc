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

#include "PlusConstantVariable.h"

namespace PLearn {
using namespace std;


/** PlusConstantVariable **/

PLEARN_IMPLEMENT_OBJECT(
        PlusConstantVariable,
        "Adds a scalar constant to a matrix Variable.",
        ""
);

//////////////////////////
// PlusConstantVariable //
//////////////////////////
PlusConstantVariable::PlusConstantVariable():
    cst(0)
{}

PlusConstantVariable::PlusConstantVariable(Variable* input, real c,
                                           bool call_build_):
    inherited(input, input->length(), input->width(), call_build_),
    cst(c)
{
    if (call_build_)
        build_();
}

///////////
// build //
///////////
void PlusConstantVariable::build() {
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void PlusConstantVariable::build_() {
    // Nothing to do here.
}

////////////////////
// declareOptions //
////////////////////
void PlusConstantVariable::declareOptions(OptionList &ol)
{
    inherited::declareOptions(ol);
    declareOption(ol, "cst", &PlusConstantVariable::cst,
                  OptionBase::buildoption,
        "The constant to be added.");
}

void PlusConstantVariable::recomputeSize(int& l, int& w) const
{
    if (input) {
        l = input->length();
        w = input->width();
    } else
        l = w = 0;
}

void PlusConstantVariable::fprop()
{
    for(int k=0; k<nelems(); k++)
        valuedata[k] = input->valuedata[k] + cst;
}


void PlusConstantVariable::bprop()
{
    for(int k=0; k<nelems(); k++)
        input->gradientdata[k] += gradientdata[k];
}


void PlusConstantVariable::symbolicBprop()
{
    input->accg(g);
}


// R(x+c) = R(x)
void PlusConstantVariable::rfprop()
{
    if (rValue.length()==0) resizeRValue();
    for(int k=0; k<nelems(); k++)
        rvaluedata[k] = input->rvaluedata[k];
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
