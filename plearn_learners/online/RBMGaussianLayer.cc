// -*- C++ -*-

// RBMGaussianLayer.cc
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



#include "RBMGaussianLayer.h"
#include <plearn/math/TMat_maths.h>
#include "RBMConnection.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMGaussianLayer,
    "Layer in an RBM, consisting in Gaussian units",
    "");

RBMGaussianLayer::RBMGaussianLayer( real the_learning_rate ) :
    inherited( the_learning_rate ),
    min_quad_coeff( 0. ),
    sigma_is_up_to_date( false )
{
}

RBMGaussianLayer::RBMGaussianLayer( int the_size, real the_learning_rate ) :
    inherited( the_learning_rate ),
    min_quad_coeff( 0. ),
    quad_coeff( the_size, 1. ), // or 1./M_SQRT2 ?
    quad_coeff_pos_stats( the_size ),
    quad_coeff_neg_stats( the_size ),
    sigma( the_size ),
    sigma_is_up_to_date( false )
{
    size = the_size;
    activation.resize( the_size );
    sample.resize( the_size );
    expectation.resize( the_size );
    bias.resize( the_size );
    bias_pos_stats.resize( the_size );
    bias_neg_stats.resize( the_size );
}

void RBMGaussianLayer::generateSample()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectation_is_up_to_date, "Expectation should be computed "
            "before calling generateSample()");
    
    computeStdDeviation();
    for( int i=0 ; i<size ; i++ )
        sample[i] = random_gen->gaussian_mu_sigma( expectation[i], sigma[i] );
}

void RBMGaussianLayer::generateSamples()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectations_are_up_to_date, "Expectations should be computed "
            "before calling generateSamples()");

    computeStdDeviation();
    PLASSERT( samples.width() == size && samples.length() == batch_size );

    for (int k = 0; k < batch_size; k++) {
        for (int i=0 ; i<size ; i++)
            samples(k, i) = random_gen->gaussian_mu_sigma( expectations(k, i), sigma[i] );
    }
}


void RBMGaussianLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    // mu = activations[i] / (2 * quad_coeff[i]^2)
    for( int i=0 ; i<size ; i++ )
    {
        real a_i = quad_coeff[i];
        expectation[i] = - activation[i] / (2 * a_i * a_i);
    }

    expectation_is_up_to_date = true;
}

void RBMGaussianLayer::computeExpectations()
{
    if( expectations_are_up_to_date )
        return;

    PLASSERT( expectations.width() == size
              && expectations.length() == batch_size );

    for (int k = 0; k < batch_size; k++)
        for (int i = 0 ; i < size ; i++)
            {
                real a_i = quad_coeff[i];
                expectations(k, i) = -activations(k, i) / (2 * a_i * a_i) ;
            }

    expectations_are_up_to_date = true;
}


void RBMGaussianLayer::computeStdDeviation()
{
    if( sigma_is_up_to_date )
        return;

    // sigma = 1 / (sqrt(2) * quad_coeff[i])
    for( int i=0 ; i<size ; i++ )
        sigma[i] = 1 / (M_SQRT2 * quad_coeff[i]);

    sigma_is_up_to_date = true;
}

void RBMGaussianLayer::fprop( const Vec& input, Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    for( int i=0 ; i<size ; i++ )
    {
        real a_i = quad_coeff[i];
        output[i] = - (input[i] + bias[i]) / (2 * a_i * a_i);
    }
}

void RBMGaussianLayer::bpropUpdate(const Vec& input, const Vec& output,
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
    {
        bias_inc.resize( size );
        //quad_coeff_inc.resize( size );
    }

    // real two_lr = 2 * learning_rate;
    for( int i=0 ; i<size ; ++i )
    {
        real a_i = quad_coeff[i];
        real in_grad_i = - output_gradient[i] / (2 * a_i * a_i);
        input_gradient[i] += in_grad_i;

        if( momentum == 0. )
        {
            // bias -= learning_rate * input_gradient
            bias[i] -= learning_rate * in_grad_i;

            /* For the moment, we do not want to change the quadratic
               coefficient during the gradient descent phase.

            // update the quadratic coefficient:
            // a_i += learning_rate * out_grad_i * (b_i + input_i) / a_i^3
            // (or a_i += 2 * learning_rate * in_grad_i * (b_i + input_i) / a_i
            quad_coeff[i] += two_lr * in_grad_i * (bias[i] + input[i])
                                                    / quad_coeff[i];
            if( quad_coeff[i] < min_quad_coeff )
                quad_coeff[i] = min_quad_coeff;
            */
        }
        else
        {
            // bias_inc = momentum * bias_inc - learning_rate * input_gradient
            // bias += bias_inc
            bias_inc[i] = momentum * bias_inc[i] - learning_rate * in_grad_i;
            bias[i] += bias_inc[i];

            /*
            // The update rule becomes:
            // a_inc_i = momentum * a_i_inc + learning_rate * out_grad_i
            //                                  * (b_i + input_i) / a_i^3
            // a_i += a_inc_i
            quad_coeff_inc[i] += momentum * quad_coeff_inc[i]
                + two_lr * in_grad_i * (bias[i] + input[i])
                                         / quad_coeff[i];
            quad_coeff[i] += quad_coeff_inc[i];
            if( quad_coeff[i] < min_quad_coeff )
                quad_coeff[i] = min_quad_coeff;
            */
        }
    }
}

void RBMGaussianLayer::reset()
{
    inherited::reset();
    sigma.clear();
    sigma_is_up_to_date = false;
}

void RBMGaussianLayer::clearStats()
{
    quad_coeff_pos_stats.clear();
    quad_coeff_neg_stats.clear();

    inherited::clearStats();
}

void RBMGaussianLayer::forget()
{
    quad_coeff.fill( 2. );

    inherited::forget();
}

void RBMGaussianLayer::declareOptions(OptionList& ol)
{
    declareOption(ol, "min_quad_coeff", &RBMGaussianLayer::min_quad_coeff,
                  OptionBase::buildoption,
                  "Minimum bound on the value of the quadratic coefficients.");

    declareOption(ol, "quad_coeff", &RBMGaussianLayer::quad_coeff,
                  OptionBase::learntoption,
                  "Quadratic coefficients of the units.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMGaussianLayer::build_()
{
    sigma.resize( size );
    sigma_is_up_to_date = false;

    quad_coeff.resize( size );
    quad_coeff.fill( 2. );
//    quad_coeff.fill( 1. );
    quad_coeff_pos_stats.resize( size );
    quad_coeff_neg_stats.resize( size );
}

void RBMGaussianLayer::build()
{
    inherited::build();
    build_();
}

void RBMGaussianLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(quad_coeff, copies);
    deepCopyField(quad_coeff_pos_stats, copies);
    deepCopyField(quad_coeff_neg_stats, copies);
    deepCopyField(quad_coeff_inc, copies);
    deepCopyField(sigma, copies);
}

void RBMGaussianLayer::accumulatePosStats( const Vec& pos_values )
{
    for( int i=0 ; i<size ; i++ )
    {
        real x_i = pos_values[i];
        quad_coeff_pos_stats[i] += 2 * quad_coeff[i] * x_i * x_i;
    }
    inherited::accumulatePosStats( pos_values );
}

void RBMGaussianLayer::accumulateNegStats( const Vec& neg_values )
{
    for( int i=0 ; i<size ; i++ )
    {
        real x_i = neg_values[i];
        quad_coeff_neg_stats[i] += 2 * quad_coeff[i] * x_i * x_i;
    }
    inherited::accumulateNegStats( neg_values );
}

void RBMGaussianLayer::update()
{
    // quad_coeff -= learning_rate * (quad_coeff_pos_stats/pos_count
    //                                - quad_coeff_neg_stats/neg_count)
    real pos_factor = -learning_rate / pos_count;
    real neg_factor = learning_rate / neg_count;

    real* a = quad_coeff.data();
    real* aps = quad_coeff_pos_stats.data();
    real* ans = quad_coeff_neg_stats.data();

    if( momentum == 0. )
    {
        // no need to use bias_inc
        for( int i=0 ; i<size ; i++ )
        {
            a[i] += pos_factor * aps[i] + neg_factor * ans[i];
            if( a[i] < min_quad_coeff )
                a[i] = min_quad_coeff;
        }
    }
    else
    {
        quad_coeff_inc.resize( size );
        real* ainc = quad_coeff_inc.data();
        for( int i=0 ; i<size ; i++ )
        {
            ainc[i] = momentum*ainc[i] + pos_factor*aps[i] + neg_factor*ans[i];
            a[i] += ainc[i];
            if( a[i] < min_quad_coeff )
                a[i] = min_quad_coeff;
        }
    }

    // We will need to recompute sigma
    sigma_is_up_to_date = false;

    // will update the bias, and clear the statistics
    inherited::update();
}

void RBMGaussianLayer::update( const Vec& pos_values, const Vec& neg_values)
{
    // quad_coeff[i] -= learning_rate * 2 * quad_coeff[i] * (pos_values[i]^2
    //                                                       - neg_values[i]^2)
    real two_lr = 2 * learning_rate;
    real* a = quad_coeff.data();
    real* pv = pos_values.data();
    real* nv = neg_values.data();

    if( momentum == 0. )
    {
        for( int i=0 ; i<size ; i++ )
        {
            a[i] += two_lr * a[i] * (nv[i]*nv[i] - pv[i]*pv[i]);
            if( a[i] < min_quad_coeff )
                a[i] = min_quad_coeff;
        }
    }
    else
    {
        quad_coeff_inc.resize( size );
        real* ainc = quad_coeff_inc.data();
        for( int i=0 ; i<size ; i++ )
        {
            ainc[i] = momentum*ainc[i]
                + two_lr * a[i] * (nv[i]*nv[i] - pv[i]*pv[i]);
            a[i] += ainc[i];
            if( a[i] < min_quad_coeff )
                a[i] = min_quad_coeff;
        }
    }

    // We will need to recompute sigma
    sigma_is_up_to_date = false;

    // update the bias
    inherited::update( pos_values, neg_values );
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
