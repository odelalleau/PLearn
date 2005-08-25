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

#include "ThresholdBpropVariable.h"


namespace PLearn {
using namespace std;



/** ThresholdBpropVariable **/

PLEARN_IMPLEMENT_OBJECT(ThresholdBpropVariable,
                        "Variable that copies its input but imposes\n"
                        "a threshold on the gradient flow.",
                        "In its fprop it simply copies its unique input variable.\n"
                        "In for bprop, it multiplies the fprop value by the\n"
                        "gradient_threshold_factor option to obtain the threshold,\n"
                        "beyond which the gradient is fixed.\n");

ThresholdBpropVariable::ThresholdBpropVariable(Variable* input, real the_gradient_threshold_factor)
    : inherited(input, input->length(), input->width()),
      gradient_threshold_factor(the_gradient_threshold_factor)
{
    build_();
}

void
ThresholdBpropVariable::build()
{
    inherited::build();
    build_();
}

void
ThresholdBpropVariable::build_()
{
}

void
ThresholdBpropVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "gradient_threshold_factor", &ThresholdBpropVariable::gradient_threshold_factor, 
                  OptionBase::buildoption, "Factor by which the value of the variable is multiplied to obtain the absolute threshold\n"
                  "on the gradient.");
    inherited::declareOptions(ol);
}

void ThresholdBpropVariable::recomputeSize(int& l, int& w) const
{
    if (input) {
        l = input->length();
        w = input->width();
    } else
        l = w = 0;
}

void ThresholdBpropVariable::fprop()
{
    for(int k=0; k<input->nelems(); k++)
        valuedata[k] = input->valuedata[k];
}


void ThresholdBpropVariable::bprop()
{
    if(gradient_threshold_factor!=0)
    {
        for(int k=0; k<input->nelems(); k++)
            if(abs(gradientdata[k]) > abs(gradient_threshold_factor * input->valuedata[k]))
                input->gradientdata[k] += abs(gradient_threshold_factor * input->valuedata[k]) * (gradientdata[k]>0 ? 1 : -1);
            else
                input->gradientdata[k] += gradientdata[k];
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
