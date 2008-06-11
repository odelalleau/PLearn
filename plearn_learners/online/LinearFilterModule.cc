// -*- C++ -*-

// LinearFilterModule.cc
//
// Copyright (C) 2005 Jerome Louradour
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

/* *******************************************************
   * $Id: LinearFilterModule.cc,v 1.3 2006/01/18 04:04:06 lamblinp Exp $
   ******************************************************* */

// Authors: Jerome Louradour

/*! \file LinearFilterModule.cc */


#include "LinearFilterModule.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    LinearFilterModule,
    "Affine transformation module, with stochastic gradient descent updates",
    "Neural Network layer, using stochastic gradient to update neuron weights\n"
    "       Output = weights * Input + bias\n"
    "Weights and bias are updated by online gradient descent, with learning\n"
    "rate possibly decreasing in 1/(1 + n_updates_done * decrease_constant).\n"
    "An L1 and L2 regularization penalty can be added to push weights to 0.\n"
    "Weights can be initialized to 0, to a given initial matrix, or randomly\n"
    "from a uniform distribution.\n"
    );

/////////////////////////
// LinearFilterModule //
/////////////////////////
LinearFilterModule::LinearFilterModule():
    start_learning_rate( .001 ),
    decrease_constant( 0. ),
    init_weights_random_scale( 1. ),
    L1_penalty_factor( 0. ),
    L2_penalty_factor( 0. ),
    no_bias(false),
    between_0_and_1(false),
    step_number( 0 )
{}

///////////
// fprop //
///////////
void LinearFilterModule::fprop(const Vec& input, Vec& output) const
{
    PLASSERT_MSG( input.size() == input_size,
                  "input.size() should be equal to this->input_size" );

    output.resize( output_size );

    // Applies linear transformation
    for( int i=0 ; i<output_size ; i++ )
        output[i] = weights[i] * input[i % input_size] + bias[i];
}

void LinearFilterModule::fprop(const Mat& inputs, Mat& outputs)
{
    PLASSERT( inputs.width() == input_size );
    int n = inputs.length();
    outputs.resize(n, output_size);
    for(int is=0;is<n;is++)
        for(int i=0;i<output_size;i++)
            outputs(is,i) = weights[i] * inputs(is, i % input_size);

    // Add bias.
    resizeOnes(n);
    externalProductAcc(outputs, ones, bias); // could be more efficient, but not critical
}

/////////////////
// bpropUpdate //
/////////////////
// We are not using blas routines anymore, because we would iterate several
// times over the weight matrix.
void LinearFilterModule::bpropUpdate(const Vec& input, const Vec& output,
                                      const Vec& output_gradient)
{
    PLASSERT_MSG( input.size() == input_size,
                  "input.size() should be equal to this->input_size" );
    PLASSERT_MSG( output.size() == output_size,
                  "output.size() should be equal to this->output_size" );
    PLASSERT_MSG( output_gradient.size() == output_size,
                  "output_gradient.size() should be equal to this->output_size"
                );

    learning_rate = start_learning_rate / (1+decrease_constant*step_number);

    for( int i=0; i<output_size; i++ )
    {
        real og_i = output_gradient[i];

        real delta_L1 = learning_rate * L1_penalty_factor;
        real delta_L2 = learning_rate * L2_penalty_factor;
        if( delta_L2 > 1 )
            PLWARNING("LinearFilterModule::bpropUpdate:\n"
                      "learning rate = %f is too large!\n", learning_rate);

        real lr_og_i = learning_rate * og_i;
        if( !no_bias )
            bias[i] -= lr_og_i;

            if( delta_L2 > 0. )
                weights[i] *= (1 - delta_L2);

            weights[i] -= input[i % input_size] * lr_og_i;

            if( delta_L1 > 0. )
            {
                if( weights[i] > delta_L1 )
                    weights[i] -= delta_L1;
                else if( weights[i] < -delta_L1 )
                    weights[i] += delta_L1;
                else
                    weights[i] = 0.;
            }

            if( between_0_and_1 )
            {
                if( weights[i] > 1. )
                    weights[i] = 1.;
                if( weights[i] < 0. )
                    weights[i] = 0.;
            }

    }
    step_number++;
}


// Simply updates and propagates back gradient
void LinearFilterModule::bpropUpdate(const Vec& input, const Vec& output,
                                      Vec& input_gradient,
                                      const Vec& output_gradient,
                                      bool accumulate)
{
    PLASSERT_MSG( input.size() == input_size,
                  "input.size() should be equal to this->input_size" );
    PLASSERT_MSG( output.size() == output_size,
                  "output.size() should be equal to this->output_size" );
    PLASSERT_MSG( output_gradient.size() == output_size,
                  "output_gradient.size() should be equal to this->output_size"
                );

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

    learning_rate = start_learning_rate / (1+decrease_constant*step_number);

    for( int i=0; i<output_size; i++ )
    {
        real og_i = output_gradient[i];

        real delta_L1 = learning_rate * L1_penalty_factor;
        real delta_L2 = learning_rate * L2_penalty_factor;
        if( delta_L2 > 1 )
            PLWARNING("LinearFilterModule::bpropUpdate:\n"
                      "learning rate = %f is too large!\n", learning_rate);

        real lr_og_i = learning_rate * og_i;
        if( !no_bias )
            bias[i] -= lr_og_i;

        input_gradient[i % input_size] += weights[i] * og_i;

        if( delta_L2 > 0. )
            weights[i] *= (1 - delta_L2);

        weights[i] -= input[i % input_size] * lr_og_i;

        if( delta_L1 > 0. )
        {
                if( weights[i] > delta_L1 )
                    weights[i] -= delta_L1;
                else if( weights[i] < -delta_L1 )
                    weights[i] += delta_L1;
                else
                    weights[i] = 0.;
        }
            if( between_0_and_1 )
            {
                if( weights[i] > 1. )
                    weights[i] = 1.;
                if( weights[i] < 0. )
                    weights[i] = 0.;
            }
    }
    step_number++;
}

void LinearFilterModule::bpropUpdate(const Mat& inputs, const Mat& outputs,
        Mat& input_gradients,
        const Mat& output_gradients,
        bool accumulate)
{
    PLASSERT( inputs.width() == input_size );
    PLASSERT( outputs.width() == output_size );
    PLASSERT( output_gradients.width() == output_size );

    int n = inputs.length();

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == input_size &&
                input_gradients.length() == n,
                "Cannot resize input_gradients and accumulate into it" );
    }
    else
    {
        input_gradients.resize(n, input_size);
        input_gradients.fill(0);
    }

    learning_rate = start_learning_rate / (1+decrease_constant*step_number);
    real avg_lr = learning_rate / n; // To obtain an average on a mini-batch.

    // With L2 regularization, weights are scaled by a coefficient equal to
    // 1 - learning rate * penalty.
    real l2_scaling =
        L2_penalty_factor > 0 ? 1 - learning_rate * L2_penalty_factor
                              : 1;
    PLASSERT_MSG(l2_scaling > 0, "Learning rate too large");

    // Compute input gradient.
    for(int i_sample = 0; i_sample < outputs.length() ;i_sample++)
        for(int i = 0; i < output_size; i++)
            input_gradients(i_sample, i % input_size ) += weights[i] * output_gradients(i_sample,  i );

    // Update bias.
    if( !no_bias )
    {
        resizeOnes(n);
        transposeProductScaleAcc(bias, output_gradients, ones, -avg_lr, real(1));
    }

    // Update weights.
    for(int i_sample = 0; i_sample < outputs.length() ;i_sample++)
        for(int i = 0; i < output_size; i++ )
        {
            weights[i] -= avg_lr * l2_scaling * output_gradients(i_sample, i) * inputs(i_sample, i % input_size);
            if( between_0_and_1 )
            {
                if( weights[i] > 1. )
                    weights[i] = 1.;
                if( weights[i] < 0. )
                    weights[i] = 0.;
            }
        }

    // Apply L1 penalty if needed (note: this is not very efficient).
    if (L1_penalty_factor > 0) {
        real delta_L1 = learning_rate * L1_penalty_factor;
        for( int i=0; i<output_size; i++ )
        {
                if( weights[i] > delta_L1 )
                    weights[i] -= delta_L1;
                else if( weights[i] < -delta_L1 )
                    weights[i] += delta_L1;
                else
                    weights[i] = 0.;
        }
    }
    step_number += n;
}


//////////////////
// bbpropUpdate //
//////////////////
void LinearFilterModule::bbpropUpdate(const Vec& input, const Vec& output,
                                       const Vec& output_gradient,
                                       const Vec& output_diag_hessian)
{
    PLASSERT_MSG( output_diag_hessian.size() == output_size,
                  "output_diag_hessian.size() should be equal to"
                  " this->output_size" );
    bpropUpdate( input, output, output_gradient );
}

/* This implementation is incorrect. Let the PLERROR defined in parent version
// Propagates back output_gradient and output_diag_hessian
void LinearFilterModule::bbpropUpdate(const Vec& input, const Vec& output,
                                       Vec&  input_gradient,
                                       const Vec& output_gradient,
                                       Vec&  input_diag_hessian,
                                       const Vec& output_diag_hessian,
                                       bool accumulate)
{
    bpropUpdate( input, output, input_gradient, output_gradient, accumulate );
}
*/

////////////
// forget //
////////////
// Forget the bias and reinitialize the weights
void LinearFilterModule::forget()
{
    learning_rate = start_learning_rate;
    step_number = 0;

    bias.resize( output_size );
    if( init_bias.size() > 0 )
    {
        if( init_bias.size() != output_size )
            PLERROR( "init_bias (%d) should have length equal to output_size (%d)",
                     init_bias.size(), output_size );
        bias << init_bias;
    }
    else
        bias.clear();
    if( no_bias )
        bias.clear();

    weights.resize( output_size );
    if( init_weights.size() > 0 )
    {
        if( init_weights.length() != output_size )
            PLERROR( "init_weights (%d) should have size equal to (output_size) (%d)",
                     init_weights.length(),
                     output_size );

        weights << init_weights;
    }
    else if( init_weights_random_scale < 0. )
    {
        real r = - init_weights_random_scale / sqrt( (real)input_size );
        random_gen->fill_random_uniform(weights, 1.-r, 1.);
    }
    else
    {
        real r = init_weights_random_scale / sqrt( (real)input_size );
        random_gen->fill_random_uniform(weights, 0., r);
    }
}

void LinearFilterModule::setLearningRate( real dynamic_learning_rate )
{
    start_learning_rate = dynamic_learning_rate;
    step_number = 0;
    // learning_rate will automatically be set in bpropUpdate()
}

///////////
// build //
///////////
void LinearFilterModule::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LinearFilterModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(init_weights, copies);
    deepCopyField(init_bias,    copies);
    deepCopyField(weights,      copies);
    deepCopyField(bias,         copies);
    deepCopyField(ones,         copies);
}

////////////////////
// declareOptions //
////////////////////
void LinearFilterModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "start_learning_rate",
                  &LinearFilterModule::start_learning_rate,
                  OptionBase::buildoption,
                  "Learning-rate of stochastic gradient optimization");

    declareOption(ol, "decrease_constant",
                  &LinearFilterModule::decrease_constant,
                  OptionBase::buildoption,
                  "Decrease constant of stochastic gradient optimization");

    declareOption(ol, "init_weights", &LinearFilterModule::init_weights,
                  OptionBase::buildoption,
                  "Optional initial weights of the neurons (one row per neuron).\n"
                  "If not provided then weights are initialized according to a uniform\n"
                  "distribution (see init_weights_random_scale)\n");

    declareOption(ol, "init_bias", &LinearFilterModule::init_bias,
                  OptionBase::buildoption,
                  "Optional initial bias of the neurons. If not provided, they are set to 0.\n");

    declareOption(ol, "init_weights_random_scale",
                  &LinearFilterModule::init_weights_random_scale,
                  OptionBase::buildoption,
                  "If init_weights is not provided, the weights are initialized randomly\n"
                  "from a uniform in [-r,r], with r = init_weights_random_scale/input_size.\n"
                  "To clear the weights initially, just set this option to 0.");

    declareOption(ol, "L1_penalty_factor",
                  &LinearFilterModule::L1_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L1 regularization term, i.e.\n"
                  "minimize L1_penalty_factor * sum_{ij} |weights(i,j)| during training.\n");

    declareOption(ol, "L2_penalty_factor",
                  &LinearFilterModule::L2_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L2 regularization term, i.e.\n"
                  "minimize 0.5 * L2_penalty_factor * sum_{ij} weights(i,j)^2 during training.\n");

    declareOption(ol, "no_bias",
                  &LinearFilterModule::no_bias,
                  OptionBase::buildoption,
                  "Wether or not to add biases.\n");

    declareOption(ol, "between_0_and_1",
                  &LinearFilterModule::between_0_and_1,
                  OptionBase::buildoption,
                  "Should all weights stay between 0 and 1.\n");

    declareOption(ol, "weights", &LinearFilterModule::weights,
                  OptionBase::learntoption,
                  "Input weights of the neurons (one weight per neuron)");

    declareOption(ol, "bias", &LinearFilterModule::bias,
                  OptionBase::learntoption,
                  "Bias of the neurons");

    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void LinearFilterModule::build_()
{
    if( input_size < 0 ) // has not been initialized
        return;

    if( output_size < 0 )
        PLERROR("LinearFilterModule::build_: 'output_size' is < 0 (%i),\n"
                " you should set it to a positive integer (the number of"
                " neurons).\n", output_size);

    if( weights.length() != output_size
        || bias.size() != output_size )
    {
        forget();
    }
}

////////////////
// resizeOnes //
////////////////
void LinearFilterModule::resizeOnes(int n)
{
    if (ones.length() < n) {
        ones.resize(n);
        ones.fill(1);
    } else if (ones.length() > n)
        ones.resize(n);
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
