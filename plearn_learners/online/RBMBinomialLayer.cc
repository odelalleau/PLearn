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

/*! \file PLearn/plearn_learners/online/RBMBinomialLayer.cc */



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
    inherited( the_learning_rate ),
    use_signed_samples( false )
{
}

RBMBinomialLayer::RBMBinomialLayer( int the_size, real the_learning_rate ) :
    inherited( the_learning_rate ),
    use_signed_samples( false )
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

    if( use_signed_samples )
        for( int i=0 ; i<size ; i++ )
            sample[i] = 2*random_gen->binomial_sample( (expectation[i]+1)/2 )-1;
    else
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

    if( use_signed_samples )
        for (int k = 0; k < batch_size; k++) {
            for (int i=0 ; i<size ; i++)
                samples(k, i) = 2*random_gen->binomial_sample( (expectations(k, i)+1)/2 )-1;
        }
    else
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

    if( use_signed_samples )
        if (use_fast_approximations)
            for( int i=0 ; i<size ; i++ )
                expectation[i] = fasttanh( activation[i] );
        else
            for( int i=0 ; i<size ; i++ )
                expectation[i] = tanh( activation[i] );
    else
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
    PLASSERT( activations.length() == batch_size );
    if( expectations_are_up_to_date )
        return;

    PLASSERT( expectations.width() == size
              && expectations.length() == batch_size );
    if( use_signed_samples )
        if (use_fast_approximations)
            for (int k = 0; k < batch_size; k++)
                for (int i = 0 ; i < size ; i++)
                    expectations(k, i) = fasttanh(activations(k, i));
        else
            for (int k = 0; k < batch_size; k++)
                for (int i = 0 ; i < size ; i++)
                    expectations(k, i) = tanh(activations(k, i));
    else
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
   
    if( use_signed_samples )
        if (use_fast_approximations)
            for( int i=0 ; i<size ; i++ )
                output[i] = fasttanh( input[i] + bias[i] );
        else
            for( int i=0 ; i<size ; i++ )
                output[i] = tanh( input[i] + bias[i] );
    else
        if (use_fast_approximations)
            for( int i=0 ; i<size ; i++ )
                output[i] = fastsigmoid( input[i] + bias[i] );
        else
            for( int i=0 ; i<size ; i++ )
                output[i] = sigmoid( input[i] + bias[i] );
}

void RBMBinomialLayer::fprop( const Mat& inputs, Mat& outputs )
{
    int mbatch_size = inputs.length();
    PLASSERT( inputs.width() == size );
    outputs.resize( mbatch_size, size );

    if( use_signed_samples )
        if (use_fast_approximations)
            for( int k = 0; k < mbatch_size; k++ )
                for( int i = 0; i < size; i++ )
                    outputs(k,i) = fasttanh( inputs(k,i) + bias[i] );
        else
            for( int k = 0; k < mbatch_size; k++ )
                for( int i = 0; i < size; i++ )
                    outputs(k,i) = tanh( inputs(k,i) + bias[i] );
    else
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

    if( use_signed_samples )
        if (use_fast_approximations)
            for( int i=0 ; i<size ; i++ )
                output[i] = fasttanh( input[i] + rbm_bias[i]);
        else
            for( int i=0 ; i<size ; i++ )
                output[i] =tanh( input[i] + rbm_bias[i]);
    else
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

    if( use_signed_samples )
    {
        for( int i=0 ; i<size ; i++ )
        {
            real output_i = output[i];
            real in_grad_i;
            in_grad_i = (1 -  output_i * output_i) * output_gradient[i];
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
    }
    else
    {
        for( int i=0 ; i<size ; i++ )
        {
            real output_i = output[i];
            real in_grad_i;
            in_grad_i = output_i * (1-output_i) * output_gradient[i];
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

    if( use_signed_samples )
    {
        for (int j = 0; j < mbatch_size; j++)
        {
            for( int i=0 ; i<size ; i++ )
            {
                real output_i = outputs(j, i);
                real in_grad_i;
                in_grad_i = (1 - output_i * output_i) * output_gradients(j, i);
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
    }
    else
    {
        for (int j = 0; j < mbatch_size; j++)
        {
            for( int i=0 ; i<size ; i++ )
            {
                real output_i = outputs(j, i);
                real in_grad_i;
                in_grad_i = output_i * (1-output_i) * output_gradients(j, i);
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

    if( use_signed_samples )
    {
        for( int i=0 ; i<size ; i++ )
        {
            real output_i = output[i];

            input_gradient[i] = ( 1 - output_i * output_i ) * output_gradient[i];
        }
    }
    else
    {
        for( int i=0 ; i<size ; i++ )
        {
            real output_i = output[i];
            input_gradient[i] = output_i * (1-output_i) * output_gradient[i];
        }
    }

    rbm_bias_gradient << input_gradient;
}

real RBMBinomialLayer::fpropNLL(const Vec& target)
{
    PLASSERT( target.size() == input_size );
    real ret = 0;
    real target_i, activation_i;
    if( use_signed_samples )
    {
        if(use_fast_approximations){
            for( int i=0 ; i<size ; i++ )
            {
                target_i = (target[i]+1)/2;
                activation_i = 2*activation[i];

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
                target_i = (target[i]+1)/2;
                activation_i = 2*activation[i];
                ret += softplus(activation_i) - target_i * activation_i;
            }
        }
    }
    else
    {
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
    }

    return ret;
}

real RBMBinomialLayer::fpropNLL(const Vec& target, const Vec& cost_weights)
{
    PLASSERT( target.size() == input_size );
    PLASSERT( target.size() == cost_weights.size() );
    PLASSERT (cost_weights.size() == size );

    real ret = 0;
    real target_i, activation_i;
    if( use_signed_samples )
    {
        if(use_fast_approximations){
            for( int i=0 ; i<size ; i++ )
            {
                if(cost_weights[i] != 0)
                {
                    target_i = (target[i]+1)/2;
                    activation_i = 2*activation[i];

                    ret += cost_weights[i]*(tabulated_softplus(activation_i) - target_i * activation_i);
                }
                // nll = - target*log(sigmoid(act)) -(1-target)*log(1-sigmoid(act))
                // but it is numerically unstable, so use instead the following identity:
                //     = target*softplus(-act) +(1-target)*(act+softplus(-act))
                //     = act + softplus(-act) - target*act
                //     = softplus(act) - target*act
            }
        } else {
            for( int i=0 ; i<size ; i++ )
            {
                if(cost_weights[i] != 0)
                {
                    target_i = (target[i]+1)/2;
                    activation_i = 2*activation[i];
                    ret += cost_weights[i]*(softplus(activation_i) - target_i * activation_i);
                }
            }
        }
    }
    else
    {
        if(use_fast_approximations){
            for( int i=0 ; i<size ; i++ )
            {
                if(cost_weights[i] != 0)
                {
                    target_i = target[i];
                    activation_i = activation[i];
                    ret += cost_weights[i]*(tabulated_softplus(activation_i) - target_i * activation_i);
                }
                // nll = - target*log(sigmoid(act)) -(1-target)*log(1-sigmoid(act))
                // but it is numerically unstable, so use instead the following identity:
                //     = target*softplus(-act) +(1-target)*(act+softplus(-act))
                //     = act + softplus(-act) - target*act
                //     = softplus(act) - target*act
            }
        } else {
            for( int i=0 ; i<size ; i++ )
            {
                if(cost_weights[i] != 0)
                {
                    target_i = target[i];
                    activation_i = activation[i];
                    ret += cost_weights[i]*(softplus(activation_i) - target_i * activation_i);
                }
            }
        }
    }

    return ret;
}


void RBMBinomialLayer::fpropNLL(const Mat& targets, const Mat& costs_column)
{
    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );

    if( use_signed_samples )
    {
        for (int k=0;k<batch_size;k++) // loop over minibatch
        {
            real nll = 0;
            real* activation = activations[k];
            real* target = targets[k];
            if(use_fast_approximations){
                for( int i=0 ; i<size ; i++ ) // loop over outputs
                {
                    nll += tabulated_softplus(2*activation[i])
                        - (target[i]+1) * activation[i] ;
                }
            } else {
                for( int i=0 ; i<size ; i++ ) // loop over outputs
                {
                    nll += softplus(2*activation[i]) - (target[i]+1)*activation[i] ;
                }
            }
            costs_column(k,0) = nll;
        }
    }
    else
    {
        for (int k=0;k<batch_size;k++) // loop over minibatch
        {
            real nll = 0;
            real* activation = activations[k];
            real* target = targets[k];
            if(use_fast_approximations){
                for( int i=0 ; i<size ; i++ ) // loop over outputs
                {
                    nll += tabulated_softplus(activation[i])
                        -target[i] * activation[i] ;
                }
            } else {
                for( int i=0 ; i<size ; i++ ) // loop over outputs
                {
                    nll += softplus(activation[i]) - target[i] * activation[i] ;
                }
            }
            costs_column(k,0) = nll;
        }
    }
}

void RBMBinomialLayer::bpropNLL(const Vec& target, real nll, Vec& bias_gradient)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );
    bias_gradient.resize( size );

    // bias_gradient = expectation - target
    substract(expectation, target, bias_gradient);
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
}

void RBMBinomialLayer::declareOptions(OptionList& ol)
{

    declareOption(ol, "use_signed_samples", &RBMBinomialLayer::use_signed_samples,
                  OptionBase::buildoption,
                  "Indication that samples should be in {-1,1}, not {0,1}.\n");

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
    if( use_signed_samples )
    {
        for (int i=0; i<size; i++)
        {
            if (use_fast_approximations)
                result -= tabulated_softplus(2*a[i]) - a[i];
            else
                result -= softplus(2*a[i]) - a[i];
        }
    }
    else
    {
        for (int i=0; i<size; i++)
        {
            if (use_fast_approximations)
                result -= tabulated_softplus(a[i]);
            else
                result -= softplus(a[i]);
        }
    }
    return result;
}

void RBMBinomialLayer::freeEnergyContributionGradient(
    const Vec& unit_activations,
    Vec& unit_activations_gradient,
    real output_gradient, bool accumulate) const
{
    PLASSERT( unit_activations.size() == size );
    unit_activations_gradient.resize( size );
    if( !accumulate ) unit_activations_gradient.clear();
    real* a = unit_activations.data();
    real* ga = unit_activations_gradient.data();
    if( use_signed_samples )
    {
        for (int i=0; i<size; i++)
        {
            if (use_fast_approximations)
                ga[i] -= output_gradient *
                    ( fasttanh( a[i] ) );
            else
                ga[i] -= output_gradient *
                    ( tanh( a[i] ) );
        }
    }
    else
    {
        for (int i=0; i<size; i++)
        {
            if (use_fast_approximations)
                ga[i] -= output_gradient *
                    fastsigmoid( a[i] );
            else
                ga[i] -= output_gradient *
                    sigmoid( a[i] );
        }
    }
}

int RBMBinomialLayer::getConfigurationCount()
{
    return size < 31 ? 1<<size : INFINITE_CONFIGURATIONS;
}

void RBMBinomialLayer::getConfiguration(int conf_index, Vec& output)
{
    PLASSERT( output.length() == size );
    PLASSERT( conf_index >= 0 && conf_index < getConfigurationCount() );

    if( use_signed_samples )
    {
        for ( int i = 0; i < size; ++i ) {
            output[i] = 2 * (conf_index & 1) - 1;
            conf_index >>= 1;
        }
    }
    else
    {
        for ( int i = 0; i < size; ++i ) {
            output[i] = conf_index & 1;
            conf_index >>= 1;
        }
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
