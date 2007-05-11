// -*- C++ -*-

// RBMMatrixConnectionNatGrad.cc
//
// Copyright (C) 2006 Yoshua Bengio
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

// Authors: Yoshua Bengio

/*! \file RBMMatrixConnectionNatGrad.cc */



#include "RBMMatrixConnectionNatGrad.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMMatrixConnectionNatGrad,
    "Subclass of RBMMatrixConnection which uses a block-diagonal natural gradient.\n",
    "The natural gradient algorithm used to adjust the update direction is the\n"
    "one implemented in NatGradEstimator, a template of which is provided by the\n"
    "user. One such estimator is adapted separately for each neuron (for the input\n"
    "weights of each neuron), i.e. for each row of the weights matrix.\n");

RBMMatrixConnectionNatGrad::RBMMatrixConnectionNatGrad( real the_learning_rate ) :
    inherited(the_learning_rate)
{
}

void RBMMatrixConnectionNatGrad::declareOptions(OptionList& ol)
{
    declareOption(ol, "natgrad_template", &RBMMatrixConnectionNatGrad::natgrad_template,
                  OptionBase::learntoption,
                  "An object of type NatGradEstimator which will be copied for each row of the\n"
                  "weights matrix; each will compute the adjustment to the update direction\n"
                  "based on the natural gradient.\n");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMMatrixConnectionNatGrad::build_()
{
    cd_natgrad.resize(up_size);
    bp_natgrad.resize(up_size);
    for (int i=0;i<up_size;i++)
    {
        cd_natgrad[i] = PLearn::deepCopy(natgrad_template);
        bp_natgrad[i] = PLearn::deepCopy(natgrad_template);
    }
    weights_gradient.resize(up_size,down_size);
    natural_gradient.resize(down_size);
}

void RBMMatrixConnectionNatGrad::build()
{
    inherited::build();
    build_();
}


void RBMMatrixConnectionNatGrad::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(natgrad_template, copies);
    deepCopyField(cd_natgrad, copies);
    deepCopyField(bp_natgrad, copies);
    deepCopyField(weights_gradient, copies);
    deepCopyField(natural_gradient, copies);
}

void RBMMatrixConnectionNatGrad::update( const Mat& pos_down_values, // v_0
                                         const Mat& pos_up_values,   // h_0
                                         const Mat& neg_down_values, // v_1
                                         const Mat& neg_up_values )  // h_1
{
    // weights -= learning_rate * ( h_0 v_0' - h_1 v_1' );
    // or:
    // weights[i][j] += learning_rate * (h_1[i] v_1[j] - h_0[i] v_0[j]);

    PLASSERT( pos_up_values.width() == weights.length() );
    PLASSERT( neg_up_values.width() == weights.length() );
    PLASSERT( pos_down_values.width() == weights.width() );
    PLASSERT( neg_down_values.width() == weights.width() );
    if( momentum == 0. )
    {
        // We use the average gradient over a mini-batch.
        real mbnorm = 1. / pos_down_values.length();
        productScaleAcc(weights_gradient, pos_up_values, true, pos_down_values, false,
                        mbnorm, 0);
        productScaleAcc(weights_gradient, neg_up_values, true, neg_down_values, false,
                        -mbnorm, 1);

        for (int i=0;i<up_size;i++)
        {
            (*cd_natgrad[i])(pos_count,weights_gradient(i),natural_gradient);
            multiplyAcc(weights(i),natural_gradient,-learning_rate);
        }
        pos_count++;
    }
    else
        PLERROR("RBMMatrixConnectionNatGrad::update with momentum - Not implemented");
}


void RBMMatrixConnectionNatGrad::bpropUpdate(const Mat& inputs, 
                                             const Mat& outputs,
                                             Mat& input_gradients,
                                             const Mat& output_gradients,
                                             bool accumulate)
{
    PLASSERT( inputs.width() == down_size );
    PLASSERT( outputs.width() == up_size );
    PLASSERT( output_gradients.width() == up_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == down_size &&
                      input_gradients.length() == inputs.length(),
                      "Cannot resize input_gradients and accumulate into it" );

        // input_gradients += output_gradient * weights
        productAcc(input_gradients, output_gradients, weights);
    }
    else
    {
        input_gradients.resize(inputs.length(), down_size);
        // input_gradients = output_gradient * weights
        product(input_gradients, output_gradients, weights);
    }

    // weights_gradient = 1/n * output_gradients' * inputs
    productScaleAcc(weights_gradient, output_gradients, true, inputs, false,
                    1. / inputs.length(), 0);
    for (int i=0;i<up_size;i++)
    {
        (*bp_natgrad[i])(pos_count,weights_gradient(i),natural_gradient);
        multiplyAcc(weights(i),natural_gradient,-learning_rate);
    }
    neg_count++;
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
