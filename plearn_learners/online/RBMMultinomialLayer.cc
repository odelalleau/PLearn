// -*- C++ -*-

// RBMMultinomialLayer.cc
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

/*! \file RBMPLayer.cc */



#include "RBMMultinomialLayer.h"
#include <plearn/math/TMat_maths.h>
#include "RBMConnection.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMMultinomialLayer,
    "Layer in an RBM, consisting in one multinomial unit",
    "");

RBMMultinomialLayer::RBMMultinomialLayer( real the_learning_rate ) :
    inherited( the_learning_rate )
{
}

RBMMultinomialLayer::RBMMultinomialLayer( int the_size,
                                          real the_learning_rate ) :
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

void RBMMultinomialLayer::generateSample()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectation_is_up_to_date, "Expectation should be computed "
            "before calling generateSample()");

    int i = random_gen->multinomial_sample( expectation );
    fill_one_hot( sample, i, real(0.), real(1.) );
}

void RBMMultinomialLayer::generateSamples()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectations_are_up_to_date, "Expectations should be computed "
                        "before calling generateSamples()");

    PLASSERT( samples.width() == size && samples.length() == batch_size );

    for (int k = 0; k < batch_size; k++)
    {
        int i = random_gen->multinomial_sample( expectations(k) );
        fill_one_hot( samples(k), i, real(0.), real(1.) );
    }
}

void RBMMultinomialLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    // expectation = softmax(-activation)
    softmax(activation, expectation);
    expectation_is_up_to_date = true;
}

void RBMMultinomialLayer::computeExpectations()
{
    if( expectations_are_up_to_date )
        return;

    PLASSERT( expectations.width() == size
              && expectations.length() == batch_size );

    // expectation = softmax(-activation)
    for (int k = 0; k < batch_size; k++)
        softmax(activations(k), expectations(k));

    expectations_are_up_to_date = true;
}


void RBMMultinomialLayer::fprop( const Vec& input, Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    // inefficient
    softmax( input+bias, output );
}

///////////
// fprop //
///////////
void RBMMultinomialLayer::fprop( const Vec& input, const Vec& rbm_bias,
                                 Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( rbm_bias.size() == input_size );
    output.resize( output_size );

    // inefficient
    softmax( input+rbm_bias, output );
}

/////////////////
// bpropUpdate //
/////////////////
void RBMMultinomialLayer::bpropUpdate(const Vec& input, const Vec& output,
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

    // input_gradient[i] =
    //      (output_gradient[i] - output_gradient . output) output[i]
    real outg_dot_out = dot( output_gradient, output );
    real* out = output.data();
    real* outg = output_gradient.data();
    real* ing = input_gradient.data();
    real* b = bias.data();
    real* binc = momentum==0?0:bias_inc.data();

    for( int i=0 ; i<size ; i++ )
    {
        real ing_i = (outg[i] - outg_dot_out) * out[i];
        ing[i] += ing_i;

        if( momentum == 0. )
        {
            // update the bias: bias -= learning_rate * input_gradient
            b[i] -= learning_rate * ing_i;
        }
        else
        {
            // The update rule becomes:
            // bias_inc = momentum * bias_inc - learning_rate * input_gradient
            // bias += bias_inc
            binc[i] = momentum * binc[i] - learning_rate * ing_i;
            b[i] += binc[i];
        }
    }
}

void RBMMultinomialLayer::bpropUpdate(const Mat& inputs, const Mat& outputs,
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
                input_gradients.length() == inputs.length(),
                "Cannot resize input_gradient and accumulate into it." );
    }
    else
    {
        input_gradients.resize(inputs.length(), size);
        input_gradients.clear();
    }


    if( momentum != 0. )
        bias_inc.resize( size );

    // TODO see if we can have a speed-up by reorganizing the different steps

    // input_gradients[k][i] =
    //   (output_gradients[k][i]-output_gradients[k].outputs[k]) outputs[k][i]
    real mean_lr = learning_rate / mbatch_size;
    for( int k=0; k<mbatch_size; k++ )
    {
        real outg_dot_out = dot( output_gradients(k), outputs(k) );
        real* out = outputs(k).data();
        real* outg = output_gradients(k).data();
        real* ing = input_gradients(k).data();
        real* b = bias.data();
        real* binc = momentum==0?0:bias_inc.data();

        for( int i=0 ; i<size ; i++ )
        {
            real ing_ki = (outg[i] - outg_dot_out) * out[i];
            ing[i] += ing_ki;

            if( momentum == 0. )
            {
                // update the bias: bias -= learning_rate * input_gradient
                b[i] -= mean_lr * ing_ki;
            }
            else
            {
                PLCHECK_MSG(false,
                            "Momentum not correctly implemented with batch");
                // The update rule becomes:
                // bias_inc = momentum*bias_inc - learning_rate*input_gradient
                // bias += bias_inc
                binc[i] = momentum * binc[i] - mean_lr * ing_ki;
                b[i] += binc[i];
            }
        }
    }
}

//! TODO: add "accumulate" here
void RBMMultinomialLayer::bpropUpdate(const Vec& input, const Vec& rbm_bias,
                                      const Vec& output,
                                      Vec& input_gradient,
                                      Vec& rbm_bias_gradient,
                                      const Vec& output_gradient)
{
    PLASSERT( input.size() == size );
    PLASSERT( rbm_bias.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );
    input_gradient.resize( size );
    rbm_bias_gradient.resize( size );

    // input_gradient[i] =
    //      (output_gradient . output - output_gradient[i] ) output[i]
    real outg_dot_out = dot( output_gradient, output );
    real* out = output.data();
    real* outg = output_gradient.data();
    real* ing = input_gradient.data();
    for( int i=0 ; i<size ; i++ )
        ing[i] = (outg[i] - outg_dot_out) * out[i];

    rbm_bias_gradient << input_gradient;
}

//////////////
// fpropNLL //
//////////////
real RBMMultinomialLayer::fpropNLL(const Vec& target)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );

#ifdef BOUNDCHECK
    if (!target.hasMissing())
    {
        PLASSERT_MSG( min(target) >= 0.,
                      "Elements of \"target\" should be positive" );
        // Ensure the distribution probabilities sum to 1. We relax a
        // bit the default tolerance as probabilities using
        // exponentials could suffer numerical imprecisions.
        if (!is_equal( sum(target), 1., 1., 1e-5, 1e-5 ))
            PLERROR("In RBMMultinomialLayer::fpropNLL - Elements of \"target\""
                    " should sum to 1 (found a sum = %f)",
                    sum(target));
    }
#endif

    real nll = 0;
    real target_i, expectation_i;
    for (int i=0; i<size; i++)
    {
        target_i = target[i];
        expectation_i = expectation[i];
        if(!fast_exact_is_equal(target_i, 0.0))
            nll -= target_i * pl_log(expectation_i);
    }
    return nll;
}

void RBMMultinomialLayer::fpropNLL(const Mat& targets, const Mat& costs_column)
{
    computeExpectations();

    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );

    real target_i, expectation_i;
    for (int k=0; k<batch_size; k++) // loop over minibatch
    {
#ifdef BOUNDCHECK
        if (!targets(k).hasMissing())
        {
            PLASSERT_MSG( min(targets(k)) >= 0.,
                          "Elements of \"targets\" should be positive" );
            // Ensure the distribution probabilities sum to 1. We relax a
            // bit the default tolerance as probabilities using
            // exponentials could suffer numerical imprecisions.
            if (!is_equal( sum(targets(k)), 1., 1., 1e-5, 1e-5 ))
                PLERROR("In RBMMultinomialLayer::fpropNLL - Elements of"
                        " \"target\" should sum to 1 (found a sum = %f at row"
                        " %d)",
                        sum(targets(k)), k);
        }
#endif
        real nll = 0;
        real* expectation = expectations[k];
        real* target = targets[k];
        for(int i=0; i<size; i++)
        {
            target_i = target[i];
            expectation_i = expectation[i];
            if(!fast_exact_is_equal(target_i, 0.0))
                nll -= target_i * pl_log(expectation_i);
        }
        costs_column(k, 0) = nll;
    }
}

void RBMMultinomialLayer::bpropNLL(const Vec& target, real nll,
                                   Vec& bias_gradient)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );
    bias_gradient.resize( size );

    // bias_gradient = expectation - target
    substract(expectation, target, bias_gradient);
}

void RBMMultinomialLayer::bpropNLL(const Mat& targets, const Mat& costs_column,
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

void RBMMultinomialLayer::declareOptions(OptionList& ol)
{
/*
    declareOption(ol, "size", &RBMMultinomialLayer::size,
                  OptionBase::buildoption,
                  "Number of units.");
*/
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMMultinomialLayer::build_()
{
}

void RBMMultinomialLayer::build()
{
    inherited::build();
    build_();
}


void RBMMultinomialLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

real RBMMultinomialLayer::energy(const Vec& unit_values) const
{
    return -dot(unit_values, bias);
}

real RBMMultinomialLayer::freeEnergyContribution(const Vec& unit_activations)
    const
{
    // result = -log(\sum_{i=0}^{size-1} exp(a_i))
    return -logadd(unit_activations);
}

int RBMMultinomialLayer::getConfigurationCount()
{
    return size;
}

void RBMMultinomialLayer::getConfiguration(int conf_index, Vec& output)
{
    PLASSERT( output.length() == size );
    PLASSERT( conf_index >= 0 && conf_index < getConfigurationCount() );

    for ( int i = 0; i < size; ++i ) {
        output[i] = i == conf_index ? 1 : 0;
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
