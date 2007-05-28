// -*- C++ -*-

// SoftmaxModule.cc
//
// Copyright (C) 2006 Pascal Lamblin
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

/*! \file SoftmaxModule.cc */



#include "SoftmaxModule.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    SoftmaxModule,
    "Computes the softmax function on a vector.",
    ""
);

///////////////////
// SoftmaxModule //
///////////////////
SoftmaxModule::SoftmaxModule()
{}

////////////////////
// declareOptions //
////////////////////
void SoftmaxModule::declareOptions(OptionList& ol)
{
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Hide unused options.

    redeclareOption(ol, "output_size", &SoftmaxModule::output_size,
                    OptionBase::learntoption,
                    "Set at build time.");
}

////////////
// build_ //
////////////
void SoftmaxModule::build_()
{
    output_size = input_size;
}

///////////
// build //
///////////
void SoftmaxModule::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SoftmaxModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

///////////
// fprop //
///////////
void SoftmaxModule::fprop(const Vec& input, Vec& output) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    softmax( input, output );
}

void SoftmaxModule::fprop(const Mat& inputs, Mat& outputs)
{
    PLASSERT( inputs.width() == input_size );
    int n = inputs.length();
    outputs.resize(n, output_size );
    for (int i = 0; i < n; i++)
        softmax(inputs(i), outputs(i));
}

/////////////////
// bpropUpdate //
/////////////////
void SoftmaxModule::bpropUpdate(const Vec& input, const Vec& output,
                                Vec& input_gradient,
                                const Vec& output_gradient,
                                bool accumulate)
{
    PLASSERT( input.size() == input_size );
    PLASSERT( output.size() == output_size );
    PLASSERT( output_gradient.size() == output_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
    {
        input_gradient.resize( input_size );
        input_gradient.clear();
    }

    // input_gradient[i] = output_gradient[i] * output[i]
    //                  - (output_gradient . output ) output[i]
    real outg_dot_out = dot( output_gradient, output );
    for( int i=0 ; i<input_size ; i++ )
    {
        real in_grad_i = (output_gradient[i] - outg_dot_out) * output[i];
        input_gradient[i] += in_grad_i;
    }
}

void SoftmaxModule::bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate)
{
    PLASSERT( inputs.width() == input_size );
    PLASSERT( outputs.width() == output_size );
    PLASSERT( output_gradients.width() == output_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == input_size &&
                input_gradients.length() == inputs.length(),
                "Cannot resize input_gradients and accumulate into it" );
    }
    else
    {
        input_gradients.resize(inputs.length(), input_size);
        input_gradients.fill(0);
    }

    for (int j = 0; j < inputs.length(); j++) {
        // input_gradient[i] = output_gradient[i] * output[i]
        //                  - (output_gradient . output ) output[i]
        real outg_dot_out = dot(output_gradients(j), outputs(j));
        for( int i=0 ; i<input_size ; i++ )
            input_gradients(j, i) +=
                (output_gradients(j, i) - outg_dot_out) * outputs(j, i);
    }
}

////////////
// forget //
////////////
void SoftmaxModule::forget()
{
}

/////////////////////
// setLearningRate //
/////////////////////
void SoftmaxModule::setLearningRate(real dynamic_learning_rate)
{
}

//////////////////
// bbpropUpdate //
//////////////////
void SoftmaxModule::bbpropUpdate(const Vec& input, const Vec& output,
                                 Vec& input_gradient,
                                 const Vec& output_gradient,
                                 Vec& input_diag_hessian,
                                 const Vec& output_diag_hessian,
                                 bool accumulate)
{
    PLERROR( "Not implemented yet, please come back later or complain to"
             " lamblinp." );
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
