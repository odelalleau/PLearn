// -*- C++ -*-

// RBMRateLayer.cc
//
// Copyright (C) 2008 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file RBMRateLayer.cc */



#include "RBMRateLayer.h"
#include <plearn/math/TMat_maths.h>
#include "RBMConnection.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMRateLayer,
    "Layer in an RBM consisting in rate-coded units",
    "");

RBMRateLayer::RBMRateLayer( real the_learning_rate ) :
    inherited( the_learning_rate ),
    n_spikes( 10 )
{
}

void RBMRateLayer::generateSample()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectation_is_up_to_date, "Expectation should be computed "
            "before calling generateSample()");

    real exp_i = 0;
    for( int i=0; i<size; i++)
    {
        exp_i = expectation[i];
        sample[i] = round(random_gen->gaussian_mu_sigma(
                              exp_i,exp_i*(1-exp_i/n_spikes)) );
    }
}

void RBMRateLayer::generateSamples()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectations_are_up_to_date, "Expectations should be computed "
                        "before calling generateSamples()");

    PLASSERT( samples.width() == size && samples.length() == batch_size );

    real exp_i = 0;
    for (int k = 0; k < batch_size; k++)
    {
        for( int i=0; i<size; i++)
        {
            exp_i = expectations(k,i);
            samples(k,i) = round(random_gen->gaussian_mu_sigma(
                                     exp_i,exp_i*(1-exp_i/n_spikes)) );
        }
    }
}

void RBMRateLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    if (use_fast_approximations)
        for(int i=0; i<size; i++)
            expectation[i] = n_spikes*fastsigmoid(activation[i]);
    else
        for(int i=0; i<size; i++)
            expectation[i] = n_spikes*sigmoid(activation[i]);
    expectation_is_up_to_date = true;
}

void RBMRateLayer::computeExpectations()
{
    if( expectations_are_up_to_date )
        return;

    PLASSERT( expectations.width() == size
              && expectations.length() == batch_size );

    if (use_fast_approximations)
        for (int k = 0; k < batch_size; k++)
            for(int i=0; i<size; i++)
                expectations(k,i) =  n_spikes*fastsigmoid(activations(k,i));
    else
        for (int k = 0; k < batch_size; k++)
            for(int i=0; i<size; i++)
                expectations(k,i) = n_spikes*sigmoid(activations(k,i));
    expectations_are_up_to_date = true;
}


void RBMRateLayer::fprop( const Vec& input, Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );
    if (use_fast_approximations)
        for(int i=0; i<size; i++)
            output[i] = n_spikes*fastsigmoid(input[i]+bias[i]);
    else
        for(int i=0; i<size; i++)
            output[i] = n_spikes*sigmoid(input[i]+bias[i]);
}

/////////////////
// bpropUpdate //
/////////////////
void RBMRateLayer::bpropUpdate(const Vec& input, const Vec& output,
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
        real in_grad_i;
        in_grad_i = output_i * (1-output_i) * output_gradient[i] * n_spikes;
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

void RBMRateLayer::bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate)
{
    PLERROR("In RBMRateLayer::bpropUpdate(): mini-batch version of bpropUpdate is not "
            "implemented yet");
}

//////////////
// fpropNLL //
//////////////
real RBMRateLayer::fpropNLL(const Vec& target)
{
    PLERROR("In RBMRateLayer::fpropNLL(): not implemented");
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

void RBMRateLayer::bpropNLL(const Vec& target, real nll,
                                   Vec& bias_gradient)
{
    PLERROR("In RBMRateLayer::bpropNLL(): not implemented");
    computeExpectation();

    PLASSERT( target.size() == input_size );
    bias_gradient.resize( size );

    // bias_gradient = expectation - target
    substract(expectation, target, bias_gradient);
}

void RBMRateLayer::declareOptions(OptionList& ol)
{

    declareOption(ol, "n_spikes", &RBMRateLayer::n_spikes,
                  OptionBase::buildoption,
                  "Maximum number of spikes for each neuron.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMRateLayer::build_()
{
    if( n_spikes < 1 )
        PLERROR("In RBMRateLayer::build_(): n_spikes should be positive");
}

void RBMRateLayer::build()
{
    inherited::build();
    build_();
}


void RBMRateLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    //deepCopyField(tmp_softmax, copies);
}

real RBMRateLayer::energy(const Vec& unit_values) const
{
    return -dot(unit_values, bias);
}

real RBMRateLayer::freeEnergyContribution(const Vec& unit_activations)
    const
{
    PLASSERT( unit_activations.size() == size );

    // result = -\sum_{i=0}^{size-1} softplus(a_i)
    real result = 0;
    real* a = unit_activations.data();
    for (int i=0; i<size; i++)
    {
        if (use_fast_approximations)
            result -= n_spikes*tabulated_softplus(a[i]);
        else
            result -= n_spikes*softplus(a[i]);
    }
    return result;
}

void RBMRateLayer::freeEnergyContributionGradient(
    const Vec& unit_activations,
    Vec& unit_activations_gradient,
    real output_gradient, bool accumulate) const
{
    PLASSERT( unit_activations.size() == size );
    unit_activations_gradient.resize( size );
    if( !accumulate ) unit_activations_gradient.clear();
    real* a = unit_activations.data();
    real* ga = unit_activations_gradient.data();
    for (int i=0; i<size; i++)
    {
        if (use_fast_approximations)
            ga[i] -= output_gradient * n_spikes *
                fastsigmoid( a[i] );
        else
            ga[i] -= output_gradient * n_spikes *
                sigmoid( a[i] );
    }
}

int RBMRateLayer::getConfigurationCount()
{
    return INFINITE_CONFIGURATIONS;
}

void RBMRateLayer::getConfiguration(int conf_index, Vec& output)
{
    PLERROR("In RBMRateLayer::getConfiguration(): not implemented");
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
