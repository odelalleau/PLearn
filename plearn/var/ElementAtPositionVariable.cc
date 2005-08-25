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

#include "ElementAtPositionVariable.h"
#include "VarElementVariable.h"

namespace PLearn {
using namespace std;


/** ElementAtPositionVariable **/

PLEARN_IMPLEMENT_OBJECT(
    ElementAtPositionVariable,
    "Indicator-like variable",
    "A variable of size length() x width(), filled with zeros except for the "
    "single element indexed by input2 =(i,j) or input2 = (k).");
  
ElementAtPositionVariable::ElementAtPositionVariable()
    : length_(0), width_(0)
{ }

  
ElementAtPositionVariable::ElementAtPositionVariable(Variable* input1, Variable* input2, int the_length, int the_width)
    : inherited(input1, input2, the_length, the_width), length_(the_length), width_(the_width)
{
    build_();
}

void
ElementAtPositionVariable::build()
{
    inherited::build();
    build_();
}

void
ElementAtPositionVariable::build_()
{
    if (input1 && !input1->isScalar())
        PLERROR("In ElementAtPositionVariable: element must be a scalar var");
    if (input2 && input2->nelems()>2)
        PLERROR("In ElementAtPositionVariable: position must have 1 or 2 elements");
}

void
ElementAtPositionVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "length_", &ElementAtPositionVariable::length_, OptionBase::buildoption, "");
    declareOption(ol, "width_", &ElementAtPositionVariable::width_, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void ElementAtPositionVariable::recomputeSize(int& l, int& w) const
{ l=length_; w=width_; }

void ElementAtPositionVariable::fprop()
{
    value.clear();
    if (input2->isScalar()) // input2 is a scalar (interpreted as a k)
    {
        int k = (int)input2->valuedata[0];
        valuedata[k] = input1->valuedata[0];
    }
    else // input2 has 2 elements (interpreted as (i,j) coordinates)
    {
        int i = (int)input2->valuedata[0];
        int j = (int)input2->valuedata[1];
        valuedata[i*width()+j] = input1->valuedata[0];
    }
}


void ElementAtPositionVariable::bprop()
{
    if (input2->isScalar()) // input2 is a scalar (interpreted as a k)
    {
        int k = (int)input2->valuedata[0];
        input1->gradientdata[0] += gradientdata[k];
    }
    else // input2 has 2 elements (interpreted as (i,j) coordinates)
    {
        int i = (int)input2->valuedata[0];
        int j = (int)input2->valuedata[1];
        input1->gradientdata[0] += gradientdata[i*width()+j];
    }  
}


void ElementAtPositionVariable::symbolicBprop()
{
    input1->accg(new VarElementVariable(g,input2));
}



void ElementAtPositionVariable::rfprop()
{
    if (rValue.length()==0) resizeRValue();
    rValue.clear();
    if (input2->isScalar()) // input2 is a scalar (interpreted as a k)
    {
        int k = (int)input2->valuedata[0];
        rvaluedata[k] = input1->rvaluedata[0];
    }
    else // input2 has 2 elements (interpreted as (i,j) coordinates)
    {
        int i = (int)input2->valuedata[0];
        int j = (int)input2->valuedata[1];
        rvaluedata[i*width()+j] = input1->rvaluedata[0];
    }
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
