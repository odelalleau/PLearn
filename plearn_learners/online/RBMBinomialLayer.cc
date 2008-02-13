// -*- C++ -*-

// RBMBinomialLayer.cc
//
// Copyright (C) 2006 Pascal Lamblin & Dan Popovici
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

// Authors: Pascal Lamblin & Dan Popovici

/*! \file RBMBinomialLayer.cc */



#include "RBMBinomialLayer.h"
#include <plearn/math/TMat_maths.h>
#include "RBMConnection.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMBinomialLayer,
    "Layer in an RBM formed with binomial units.",
    "");

RBMBinomialLayer::RBMBinomialLayer( real the_learning_rate ) :
    inherited( the_learning_rate )
{
}

RBMBinomialLayer::RBMBinomialLayer( int the_size, real the_learning_rate ) :
    inherited( the_learning_rate )
{
    size = the_size;
    activation.resize( the_size );
    sample.resize( the_size );
    expectation.resize( the_size );
    bias.resize( the_size );
    bias_pos_stats.resize( the_size );
    bias_neg_stats.resize( the_size );
}

////////////////////
// generateSample //
////////////////////
void RBMBinomialLayer::generateSample()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectation_is_up_to_date, "Expectation should be computed "
            "before calling generateSample()");

    //random_gen->manual_seed(1827);

    for( int i=0 ; i<size ; i++ )
        sample[i] = random_gen->binomial_sample( expectation[i] );
}

/////////////////////
// generateSamples //
/////////////////////
void RBMBinomialLayer::generateSamples()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectations_are_up_to_date, "Expectations should be computed "
            "before calling generateSamples()");

    PLASSERT( samples.width() == size && samples.length() == batch_size );

    //random_gen->manual_seed(1827);

    for (int k = 0; k < batch_size; k++) {
        for (int i=0 ; i<size ; i++)
            samples(k, i) = random_gen->binomial_sample( expectations(k, i) );
    }
}

////////////////////////
// computeExpectation //
////////////////////////
void RBMBinomialLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    if (use_fast_approximations)
        for( int i=0 ; i<size ; i++ )
            expectation[i] = fastsigmoid( activation[i] );
    else
        for( int i=0 ; i<size ; i++ )
            expectation[i] = sigmoid( activation[i] );

    expectation_is_up_to_date = true;
}

/////////////////////////
// computeExpectations //
/////////////////////////
void RBMBinomialLayer::computeExpectations()
{
    if( expectations_are_up_to_date )
        return;

    PLASSERT( expectations.width() == size
              && expectations.length() == batch_size );
    if (use_fast_approximations)
        for (int k = 0; k < batch_size; k++)
            for (int i = 0 ; i < size ; i++)
                expectations(k, i) = fastsigmoid(activations(k, i));
    else
        for (int k = 0; k < batch_size; k++)
            for (int i = 0 ; i < size ; i++)
                expectations(k, i) = sigmoid(activations(k, i));

    expectations_are_up_to_date = true;
}

///////////
// fprop //
///////////
void RBMBinomialLayer::fprop( const Vec& input, Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    if (use_fast_approximations)
        for( int i=0 ; i<size ; i++ )
            output[i] = fastsigmoid( input[i] + bias[i] );
    else
        for( int i=0 ; i<size ; i++ )
            output[i] = sigmoid( input[i] + bias[i] );
}

void RBMBinomialLayer::fprop( const Mat& inputs, Mat& outputs ) const
{
    int mbatch_size = inputs.length();
    PLASSERT( inputs.width() == size );
    outputs.resize( mbatch_size, size );

    if (use_fast_approximations)
        for( int k = 0; k < mbatch_size; k++ )
            for( int i = 0; i < size; i++ )
                outputs(k,i) = fastsigmoid( inputs(k,i) + bias[i] );
    else
        for( int k = 0; k < mbatch_size; k++ )
            for( int i = 0; i < size; i++ )
                outputs(k,i) = sigmoid( inputs(k,i) + bias[i] );
}

void RBMBinomialLayer::fprop( const Vec& input, const Vec& rbm_bias,
                              Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( rbm_bias.size() == input_size );
    output.resize( output_size );

    if (use_fast_approximations)
        for( int i=0 ; i<size ; i++ )
            output[i] = fastsigmoid( input[i] + rbm_bias[i]);
    else
        for( int i=0 ; i<size ; i++ )
            output[i] = sigmoid( input[i] + rbm_bias[i]);
}

/////////////////
// bpropUpdate //
/////////////////
void RBMBinomialLayer::bpropUpdate(const Vec& input, const Vec& output,
                                   Vec& input_gradient,
                                   const Vec& output_gradient,
                                   bool accumulate)
{
    PLASSERT( input.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
    {
        input_gradient.resize( size );
        input_gradient.clear();
    }

    if( momentum != 0. )
        bias_inc.resize( size );

    for( int i=0 ; i<size ; i++ )
    {
        real output_i = output[i];
        real in_grad_i = output_i * (1-output_i) * output_gradient[i];
        input_gradient[i] += in_grad_i;

        if( momentum == 0. )
        {
            // update the bias: bias -= learning_rate * input_gradient
            bias[i] -= learning_rate * in_grad_i;
        }
        else
        {
            // The update rule becomes:
            // bias_inc = momentum * bias_inc - learning_rate * input_gradient
            // bias += bias_inc
            bias_inc[i] = momentum * bias_inc[i] - learning_rate * in_grad_i;
            bias[i] += bias_inc[i];
        }
    }

    applyBiasDecay();
}

void RBMBinomialLayer::bpropUpdate(const Mat& inputs, const Mat& outputs,
                                   Mat& input_gradients,
                                   const Mat& output_gradients,
                                   bool accumulate)
{
    PLASSERT( inputs.width() == size );
    PLASSERT( outputs.width() == size );
    PLASSERT( output_gradients.width() == size );

    int mbatch_size = inputs.length();
    PLASSERT( outputs.length() == mbatch_size );
    PLASSERT( output_gradients.length() == mbatch_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == size &&
                input_gradients.length() == mbatch_size,
                "Cannot resize input_gradients and accumulate into it" );
    }
    else
    {
        input_gradients.resize(mbatch_size, size);
        input_gradients.clear();
    }

    if( momentum != 0. )
        bias_inc.resize( size );

    // TODO Can we do this more efficiently? (using BLAS)

    // We use the average gradient over the mini-batch.
    real avg_lr = learning_rate / inputs.length();

    for (int j = 0; j < mbatch_size; j++)
    {
        for( int i=0 ; i<size ; i++ )
        {
            real output_i = outputs(j, i);
            real in_grad_i = output_i * (1-output_i) * output_gradients(j, i);
            input_gradients(j, i) += in_grad_i;

            if( momentum == 0. )
            {
                // update the bias: bias -= learning_rate * input_gradient
                bias[i] -= avg_lr * in_grad_i;
            }
            else
            {
                PLERROR("In RBMBinomialLayer:bpropUpdate - Not implemented for "
                        "momentum with mini-batches");
                // The update rule becomes:
                // bias_inc = momentum * bias_inc - learning_rate * input_gradient
                // bias += bias_inc
                bias_inc[i] = momentum * bias_inc[i] - learning_rate * in_grad_i;
                bias[i] += bias_inc[i];
            }
        }
    }

    applyBiasDecay();
}


//! TODO: add "accumulate" here
void RBMBinomialLayer::bpropUpdate(const Vec& input, const Vec& rbm_bias,
                                   const Vec& output,
                                   Vec& input_gradient, Vec& rbm_bias_gradient,
                                   const Vec& output_gradient)
{
    PLASSERT( input.size() == size );
    PLASSERT( rbm_bias.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );
    input_gradient.resize( size );
    rbm_bias_gradient.resize( size );

    for( int i=0 ; i<size ; i++ )
    {
        real output_i = output[i];
        input_gradient[i] = output_i * (1-output_i) * output_gradient[i];
    }

    rbm_bias_gradient << input_gradient;
    addBiasDecay(rbm_bias_gradient);
}

real RBMBinomialLayer::fpropNLL(const Vec& target)
{
    PLASSERT( target.size() == input_size );

    real ret = 0;
    real target_i, activation_i;
    if(use_fast_approximations){
        for( int i=0 ; i<size ; i++ )
        {
            target_i = target[i];
            activation_i = activation[i];
            ret += tabulated_softplus(activation_i) - target_i * activation_i;
            // nll = - target*log(sigmoid(act)) -(1-target)*log(1-sigmoid(act))
            // but it is numerically unstable, so use instead the following identity:
            //     = target*softplus(-act) +(1-target)*(act+softplus(-act))
            //     = act + softplus(-act) - target*act 
            //     = softplus(act) - target*act
        }
    } else {
        for( int i=0 ; i<size ; i++ )
        {
            target_i = target[i];
            activation_i = activation[i];
            ret += softplus(activation_i) - target_i * activation_i;
        }
    }
    return ret;
}

void RBMBinomialLayer::fpropNLL(const Mat& targets, const Mat& costs_column)
{
    // computeExpectations(); // why?

    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );

    for (int k=0;k<batch_size;k++) // loop over minibatch
    {
        real nll = 0;
        real* activation = activations[k];
        real* target = targets[k];
        if(use_fast_approximations){
            for( int i=0 ; i<size ; i++ ) // loop over outputs
            {
                if(!fast_exact_is_equal(target[i],0.0))
                    // nll -= target[i] * pl_log(expectations[i]); 
                    // but it is numerically unstable, so use instead
                    // log (1/(1+exp(-x))) = -log(1+exp(-x)) = -softplus(-x)
                    nll += target[i] * tabulated_softplus(-activation[i]);
                if(!fast_exact_is_equal(target[i],1.0))
                    // nll -= (1-target[i]) * pl_log(1-output[i]);
                    // log (1 - 1/(1+exp(-x))) = log(exp(-x)/(1+exp(-x)))
                    //                         = log(1/(1+exp(x)))
                    //                         = -log(1+exp(x))
                    //                         = -softplus(x)
                    nll += (1-target[i]) * tabulated_softplus(activation[i]);
            }
        } else {
            for( int i=0 ; i<size ; i++ ) // loop over outputs
            {
                if(!fast_exact_is_equal(target[i],0.0))
                    // nll -= target[i] * pl_log(expectations[i]); 
                    // but it is numerically unstable, so use instead
                    // log (1/(1+exp(-x))) = -log(1+exp(-x)) = -softplus(-x)
                    nll += target[i] * softplus(-activation[i]);
                if(!fast_exact_is_equal(target[i],1.0))
                    // nll -= (1-target[i]) * pl_log(1-output[i]);
                    // log (1 - 1/(1+exp(-x))) = log(exp(-x)/(1+exp(-x)))
                    //                         = log(1/(1+exp(x)))
                    //                         = -log(1+exp(x))
                    //                         = -softplus(x)
                    nll += (1-target[i]) * softplus(activation[i]);
            }
        }
        costs_column(k,0) = nll;
    }
}

void RBMBinomialLayer::bpropNLL(const Vec& target, real nll, Vec& bias_gradient)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );
    bias_gradient.resize( size );

    // bias_gradient = expectation - target
    substract(expectation, target, bias_gradient);
    addBiasDecay(bias_gradient);
}

void RBMBinomialLayer::bpropNLL(const Mat& targets, const Mat& costs_column,
                                Mat& bias_gradients)
{
    computeExpectations();

    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );
    bias_gradients.resize( batch_size, size );

    // bias_gradients = expectations - targets
    substract(expectations, targets, bias_gradients);

    addBiasDecay(bias_gradients);
}

void RBMBinomialLayer::declareOptions(OptionList& ol)
{
/*
    declareOption(ol, "size", &RBMBinomialLayer::size,
                  OptionBase::buildoption,
                  "Number of units.");
*/
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMBinomialLayer::build_()
{
}

void RBMBinomialLayer::build()
{
    inherited::build();
    build_();
}


void RBMBinomialLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

real RBMBinomialLayer::energy(const Vec& unit_values) const
{
    return -dot(unit_values, bias);
}

real RBMBinomialLayer::freeEnergyContribution(const Vec& unit_activations)
    const
{
    PLASSERT( unit_activations.size() == size );

    // result = -\sum_{i=0}^{size-1} softplus(a_i)
    real result = 0;
    real* a = unit_activations.data();
    for (int i=0; i<size; i++)
    {
        if (use_fast_approximations)
            result -= tabulated_softplus(a[i]);
        else
            result -= softplus(a[i]);
    }
    return result;
}

int RBMBinomialLayer::getConfigurationCount()
{
    return size < 31 ? 1<<size : INFINITE_CONFIGURATIONS;
}

void RBMBinomialLayer::getConfiguration(int conf_index, Vec& output)
{
    PLASSERT( output.length() == size );
    PLASSERT( conf_index >= 0 && conf_index < getConfigurationCount() );

    for ( int i = 0; i < size; ++i ) {
        output[i] = conf_index & 1;
        conf_index >>= 1;
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
