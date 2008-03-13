// -*- C++ -*-

// ScaleGradientModule.cc
//
// Copyright (C) 2008 Pascal Lamblin
//
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

// Authors: Pascal Lamblin

/*! \file ScaleGradientModule.cc */



#include "ScaleGradientModule.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ScaleGradientModule,
    "Scales (or suppress) the gradient that is backpropagated.",
    ""
);

//////////////////
// ScaleGradientModule //
//////////////////
ScaleGradientModule::ScaleGradientModule():
    scale(0)
{
}

////////////////////
// declareOptions //
////////////////////
void ScaleGradientModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "scale", &ScaleGradientModule::scale,
        OptionBase::buildoption,
        "The scaling factor. If 0, no gradient will be backpropagated."
        );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void ScaleGradientModule::build_()
{
}

///////////
// build //
///////////
void ScaleGradientModule::build()
{
    inherited::build();
    build_();
}

////////////
// forget //
////////////
void ScaleGradientModule::forget()
{
}

///////////
// fprop //
///////////
void ScaleGradientModule::fprop(const Mat& inputs, Mat& outputs)
{
    outputs.resize(inputs.length(), inputs.width());
    outputs << inputs;
}

/////////////////
// bpropUpdate //
/////////////////
void ScaleGradientModule::bpropUpdate(const Mat& inputs, const Mat& outputs,
                                      Mat& input_gradients,
                                      const Mat& output_gradients,
                                      bool accumulate)
{
    input_gradients.resize(output_gradients.length(),
                           output_gradients.width());

    if (accumulate)
    {
        if (scale == 0)
            return;
        else // input_gradients += scale * output_gradients
            multiplyAcc(input_gradients, output_gradients, scale);
    }
    else
    {
        if (scale == 0)
            input_gradients.clear();
        else // input_gradients = scale * output_gradients
            multiply(input_gradients, output_gradients, scale);
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ScaleGradientModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

}
// end of namespace PLearn


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
