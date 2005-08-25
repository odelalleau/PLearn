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

#include "ReshapeVariable.h"

namespace PLearn {
using namespace std;


/** ReshapeVariable **/

PLEARN_IMPLEMENT_OBJECT(ReshapeVariable,
                        "Variable that views another variable, but with a different length() and width() "
                        "(the only restriction being that length()*width() remain the same)",
                        "NO HELP");

ReshapeVariable::ReshapeVariable(Variable* v, int the_length, int the_width)
    : inherited(v, the_length, the_width), length_(the_length), width_(the_width)
{
    build_();
}

void
ReshapeVariable::build()
{
    inherited::build();
    build_();
}

void
ReshapeVariable::build_()
{
    if (input) {
        if(length() * width() != input.length() * input.width())
            PLERROR("In ReshapeVariable: length()*width() is different from the original var's length()*width()");
    }
}

void
ReshapeVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "length_", &ReshapeVariable::length_, OptionBase::buildoption, "");
    declareOption(ol, "width_", &ReshapeVariable::width_, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void ReshapeVariable::recomputeSize(int& l, int& w) const
{ l=length_; w=width_; }

void ReshapeVariable::fprop()
{
    for(int k=0; k<nelems(); k++)
        valuedata[k] = input->valuedata[k];
}


void ReshapeVariable::bprop()
{
    for(int k=0; k<nelems(); k++)
        input->gradientdata[k] += gradientdata[k];
}


void ReshapeVariable::symbolicBprop()
{
    input->accg(new ReshapeVariable(g,input->length(),input->width()));
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
