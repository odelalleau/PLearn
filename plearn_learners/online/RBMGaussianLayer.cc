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

/*! \file PLearn/plearn_learners/online/RBMGaussianLayer.cc */



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
    share_quad_coeff( false ),
    size_quad_coeff( 0 ),
    fixed_std_deviation( -1 ),
    compute_mse_instead_of_nll( false ),
    sigma_is_up_to_date( false )
{
}

RBMGaussianLayer::RBMGaussianLayer( int the_size, real the_learning_rate ) :
    inherited( the_learning_rate ),
    min_quad_coeff( 0. ),
    share_quad_coeff( false ),
    size_quad_coeff( 0 ),
    fixed_std_deviation( -1 ),
    compute_mse_instead_of_nll( false ),
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

RBMGaussianLayer::RBMGaussianLayer( int the_size, real the_learning_rate,
                                    bool do_share_quad_coeff ) :
    inherited( the_learning_rate ),
    min_quad_coeff( 0. ),
    fixed_std_deviation( -1 ),
    compute_mse_instead_of_nll( false ),
    quad_coeff_pos_stats( the_size ),
    quad_coeff_neg_stats( the_size ),
    sigma_is_up_to_date( false )
{
    size = the_size;
    activation.resize( the_size );
    sample.resize( the_size );
    expectation.resize( the_size );
    bias.resize( the_size );
    bias_pos_stats.resize( the_size );
    bias_neg_stats.resize( the_size );
    share_quad_coeff = do_share_quad_coeff;
    if ( share_quad_coeff )
       size_quad_coeff=1;
    else
       size_quad_coeff=size;
    quad_coeff=Vec(size_quad_coeff,1.);
    sigma=Vec(size_quad_coeff);
}


void RBMGaussianLayer::generateSample()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectation_is_up_to_date, "Expectation should be computed "
            "before calling generateSample()");

    computeStdDeviation();
    if(share_quad_coeff)
        for( int i=0 ; i<size ; i++ )
            sample[i] = random_gen->gaussian_mu_sigma( expectation[i], sigma[0] );
    else
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

    if(share_quad_coeff)
        for (int k = 0; k < batch_size; k++)
            for (int i=0 ; i<size ; i++)
                samples(k, i) = random_gen->gaussian_mu_sigma( expectations(k, i), sigma[0] );
    else
        for (int k = 0; k < batch_size; k++)
            for (int i=0 ; i<size ; i++)
                samples(k, i) = random_gen->gaussian_mu_sigma( expectations(k, i), sigma[i] );
}


void RBMGaussianLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    // mu = activations[i] / (2 * quad_coeff[i]^2)
    if(share_quad_coeff)
    {
        real a_i = quad_coeff[0];
        for( int i=0 ; i<size ; i++ )
        {
            expectation[i] = activation[i] / (2 * a_i * a_i);
        }
    }
    else
        for( int i=0 ; i<size ; i++ )
        {
            real a_i = quad_coeff[i];
            expectation[i] = activation[i] / (2 * a_i * a_i);
        }

    expectation_is_up_to_date = true;
}

void RBMGaussianLayer::computeExpectations()
{
    if( expectations_are_up_to_date )
        return;

    PLASSERT( expectations.width() == size
              && expectations.length() == batch_size );

    if(share_quad_coeff)
    {
        real a_i = quad_coeff[0];
        for (int k = 0; k < batch_size; k++)
            for (int i = 0 ; i < size ; i++)
                {
                    expectations(k, i) = activations(k, i) / (2 * a_i * a_i) ;
                }
    }
    else
        for (int k = 0; k < batch_size; k++)
            for (int i = 0 ; i < size ; i++)
                {
                    real a_i = quad_coeff[i];
                    expectations(k, i) = activations(k, i) / (2 * a_i * a_i) ;
                }
    expectations_are_up_to_date = true;
}


void RBMGaussianLayer::computeStdDeviation()
{
    if( sigma_is_up_to_date )
        return;

    // sigma = 1 / (sqrt(2) * quad_coeff[i])
    if(share_quad_coeff)
        sigma[0] = 1 / (M_SQRT2 * quad_coeff[0]);
    else
        for( int i=0 ; i<size ; i++ )
            sigma[i] = 1 / (M_SQRT2 * quad_coeff[i]);

    sigma_is_up_to_date = true;
}

void RBMGaussianLayer::fprop( const Vec& input, Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    if(share_quad_coeff)
    {
        real a_i = quad_coeff[0];
        for( int i=0 ; i<size ; i++ )
        {
            output[i] = (input[i] + bias[i]) / (2 * a_i * a_i);
        }
    }
    else
        for( int i=0 ; i<size ; i++ )
        {
            real a_i = quad_coeff[i];
            output[i] = (input[i] + bias[i]) / (2 * a_i * a_i);
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
        //quad_coeff_inc.resize( size );//quad_coeff_inc.resize( 1 );
    }

    // real two_lr = 2 * learning_rate;
    real a_i = quad_coeff[0];
    for( int i=0 ; i<size ; ++i )
    {
        if(!share_quad_coeff)
            a_i = quad_coeff[i];
        real in_grad_i = output_gradient[i] / (2 * a_i * a_i);
        input_gradient[i] += in_grad_i;

        if( momentum == 0. )
        {
            // bias -= learning_rate * input_gradient
            bias[i] -= learning_rate * in_grad_i;

            /* For the moment, we do not want to change the quadratic
               coefficient during the gradient descent phase.

            // update the quadratic coefficient:
            // a_i -= learning_rate * out_grad_i * (b_i + input_i) / a_i^3
            // (or a_i -= 2 * learning_rate * in_grad_i * (b_i + input_i) / a_i
            a_i -= two_lr * in_grad_i * (bias[i] + input[i])
                                                    / a_i;
            if( a_i < min_quad_coeff )
                a_i = min_quad_coeff;
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
            // a_inc_i = momentum * a_i_inc - learning_rate * out_grad_i
            //                                  * (b_i + input_i) / a_i^3
            // a_i += a_inc_i
            quad_coeff_inc[i] = momentum * quad_coeff_inc[i]
                - two_lr * in_grad_i * (bias[i] + input[i])
                                         / a_i;
            a_i += quad_coeff_inc[i];
            if( a_i < min_quad_coeff )
                a_i = min_quad_coeff;
            */
        }
    }

    applyBiasDecay();
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
    clearStats();

    if( fixed_std_deviation > 0 )
        quad_coeff.fill( 1 / ( M_SQRT2 * fixed_std_deviation ) );
    else
        quad_coeff.fill( 1. );
    inherited::forget();
}

////////////////////
// declareOptions //
////////////////////
void RBMGaussianLayer::declareOptions(OptionList& ol)
{
    declareOption(ol, "min_quad_coeff", &RBMGaussianLayer::min_quad_coeff,
                  OptionBase::buildoption,
                  "Minimum bound on the value of the quadratic coefficients.");

    declareOption(ol, "quad_coeff", &RBMGaussianLayer::quad_coeff,
                  OptionBase::learntoption,
                  "Quadratic coefficients of the units.");

    declareOption(ol, "sigma", &RBMGaussianLayer::sigma,
                  OptionBase::learntoption,
                  "Standard deviations.");

    declareOption(ol, "share_quad_coeff", &RBMGaussianLayer::share_quad_coeff,
                  OptionBase::buildoption,
                  "Should all the units share the same quadratic coefficients?\n"
                  "Suitable to avoid unstability (overfitting)  in cases where\n"
                  "all the units have the same 'meaning'  (pixels of an image)");

    declareOption(ol, "fixed_std_deviation", &RBMGaussianLayer::fixed_std_deviation,
                  OptionBase::buildoption,
                  "Value for the usually learned standard deviation, "
                  "if it should not be learned.\n"
                  "This will fix the value of the quad coeffs to the "
                  "appropriate value.\n"
                  "If <= 0, then this option is ignored.\n");

    declareOption(ol, "compute_mse_instead_of_nll", &RBMGaussianLayer::compute_mse_instead_of_nll,
                  OptionBase::buildoption,
                  "Indication that fpropNLL should compute the MSE instead of the NLL..\n"
                  "bpropNLL will also give the appropriate gradient. Might want to\n"
                  "set fixed_std_deviation to 1 in this case.\n");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void RBMGaussianLayer::build_()
{
    bool needs_forget = false;

    if(share_quad_coeff)
        size_quad_coeff=1;
    else
        size_quad_coeff=size;

    if (sigma.size() != size_quad_coeff)
    {
        sigma.resize( size_quad_coeff );
        sigma_is_up_to_date = false;
        quad_coeff.resize( size_quad_coeff );
        needs_forget = true;
    }

    if (fixed_std_deviation > 0)
    {
        if (share_quad_coeff)
            PLERROR("In RBMGaussianLayer::build_(): fixed_std_deviation should not "
                    "be > 0 when share_quad_coeff is true.");

        quad_coeff.fill( 1 / ( M_SQRT2 * fixed_std_deviation ) );
    }


    quad_coeff_pos_stats.resize( size );
    quad_coeff_neg_stats.resize( size );

    if (needs_forget)
        forget();

    clearStats();
}

///////////
// build //
///////////
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
    if ( fixed_std_deviation <= 0 )
    {
        if (share_quad_coeff)
            for( int i=0 ; i<size ; i++ )
            {
           real x_i = pos_values[i];
           quad_coeff_pos_stats[i] += 2 * quad_coeff[0] * x_i * x_i;
            }
        else
            for( int i=0 ; i<size ; i++ )
            {
                real x_i = pos_values[i];
                quad_coeff_pos_stats[i] += 2 * quad_coeff[i] * x_i * x_i;
            }
    }

    inherited::accumulatePosStats( pos_values );
}

void RBMGaussianLayer::accumulateNegStats( const Vec& neg_values )
{
    if ( fixed_std_deviation <= 0 )
    {
        if (share_quad_coeff)
            for( int i=0 ; i<size ; i++ )
            {
                real x_i = neg_values[i];
                quad_coeff_neg_stats[i] += 2 * quad_coeff[0] * x_i * x_i;
            }
        else
            for( int i=0 ; i<size ; i++ )
            {
                real x_i = neg_values[i];
                quad_coeff_neg_stats[i] += 2 * quad_coeff[i] * x_i * x_i;
            }
    }
    inherited::accumulateNegStats( neg_values );
}

void RBMGaussianLayer::update()
{
    // quad_coeff -= learning_rate * (quad_coeff_pos_stats/pos_count
    //                                - quad_coeff_neg_stats/neg_count)
    if ( fixed_std_deviation <= 0 )
    {
        real pos_factor = -learning_rate / pos_count;
        real neg_factor = learning_rate / neg_count;

        real* a = quad_coeff.data();
        real* aps = quad_coeff_pos_stats.data();
        real* ans = quad_coeff_neg_stats.data();

        if( momentum == 0. )
        {
            if(share_quad_coeff)
            {
                real update=0;
                for( int i=0 ; i<size ; i++ )
                {
                    update += pos_factor * aps[i] + neg_factor * ans[i];
                }
                a[0] += update/(real)size;
                if( a[0] < min_quad_coeff )
                    a[0] = min_quad_coeff;
            }
            else
                for( int i=0 ; i<size ; i++ )
                {
                    a[i] += pos_factor * aps[i] + neg_factor * ans[i];
                    if( a[i] < min_quad_coeff )
                        a[i] = min_quad_coeff;
                }
        }
        else
        {
            if(share_quad_coeff)
            {
                quad_coeff_inc.resize( 1 );
                real* ainc = quad_coeff_inc.data();
                for( int i=0 ; i<size ; i++ )
                {
                    ainc[0] = momentum*ainc[0] + pos_factor*aps[i] + neg_factor*ans[i];
                    ainc[0] /= (real)size;
                    a[0] += ainc[0];
                }
                if( a[0] < min_quad_coeff )
                    a[0] = min_quad_coeff;
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
        }

        // We will need to recompute sigma
        sigma_is_up_to_date = false;
    }

    // will update the bias, and clear the statistics
    inherited::update();
}

void RBMGaussianLayer::update( const Vec& pos_values, const Vec& neg_values )
{
    // quad_coeff[i] -= learning_rate * 2 * quad_coeff[i] * (pos_values[i]^2
    //                                                       - neg_values[i]^2)
    if ( fixed_std_deviation <= 0 )
    {
        real two_lr = 2 * learning_rate;
        real* a = quad_coeff.data();
        real* pv = pos_values.data();
        real* nv = neg_values.data();

        if( momentum == 0. )
        {
            if (share_quad_coeff)
            {
                real update=0;
                for( int i=0 ; i<size ; i++ )
                {
                    update += two_lr * a[0] * (nv[i]*nv[i] - pv[i]*pv[i]);
                }
                a[0] += update/(real)size;
                if( a[0] < min_quad_coeff )
                    a[0] = min_quad_coeff;
            }
            else
                for( int i=0 ; i<size ; i++ )
                {
                    a[i] += two_lr * a[i] * (nv[i]*nv[i] - pv[i]*pv[i]);
                    if( a[i] < min_quad_coeff )
                        a[i] = min_quad_coeff;
                }
        }
        else
        {
            real* ainc = quad_coeff_inc.data();
            if(share_quad_coeff)
            {
                quad_coeff_inc.resize( 1 );
                for( int i=0 ; i<size ; i++ )
                {
                    ainc[0] = momentum*ainc[0]
                        + two_lr * a[0] * (nv[i]*nv[i] - pv[i]*pv[i]);
                    ainc[0] /= (real)size;
                    a[0] += ainc[0];
                }
                if( a[0] < min_quad_coeff )
                    a[0] = min_quad_coeff;
            }
            else
            {
                quad_coeff_inc.resize( size );
                for( int i=0 ; i<size ; i++ )
                {
                    ainc[i] = momentum*ainc[i]
                        + two_lr * a[i] * (nv[i]*nv[i] - pv[i]*pv[i]);
                    a[i] += ainc[i];
                    if( a[i] < min_quad_coeff )
                        a[i] = min_quad_coeff;
                }
            }
        }

        // We will need to recompute sigma
        sigma_is_up_to_date = false;
    }

    // update the bias
    inherited::update( pos_values, neg_values );
}

void RBMGaussianLayer::update( const Mat& pos_values, const Mat& neg_values )
{

    PLASSERT( pos_values.width() == size );
    PLASSERT( neg_values.width() == size );

    int batch_size = pos_values.length();
    PLASSERT( neg_values.length() == batch_size );

    // quad_coeff[i] -= learning_rate * 2 * quad_coeff[i] * (pos_values[i]^2
    //                                                       - neg_values[i]^2)
    if ( fixed_std_deviation <= 0 )
    {
        real two_lr = 2 * learning_rate / batch_size;
        real* a = quad_coeff.data();

        if( momentum == 0. )
        {
            if (share_quad_coeff)
                for( int k=0; k<batch_size; k++ )
                {
                    real *pv_k = pos_values[k];
                    real *nv_k = neg_values[k];
                    real update=0;
                    for( int i=0; i<size; i++ )
                    {
                        update += two_lr * a[0] * (nv_k[i]*nv_k[i] - pv_k[i]*pv_k[i]);
                    }
                    a[0] += update/(real)size;
                    if( a[0] < min_quad_coeff )
                        a[0] = min_quad_coeff;
                }
            else
                for( int k=0; k<batch_size; k++ )
                {
                    real *pv_k = pos_values[k];
                    real *nv_k = neg_values[k];
                    for( int i=0; i<size; i++ )
                    {
                        a[i] += two_lr * a[i] * (nv_k[i]*nv_k[i] - pv_k[i]*pv_k[i]);
                        if( a[i] < min_quad_coeff )
                            a[i] = min_quad_coeff;
                    }
                }
        }
        else
            PLCHECK_MSG( false,
                         "momentum and minibatch are not compatible yet" );

        // We will need to recompute sigma
        sigma_is_up_to_date = false;
    }

    // Update the bias
    inherited::update( pos_values, neg_values );
}

real RBMGaussianLayer::energy(const Vec& unit_values) const
{
    PLASSERT( unit_values.length() == size );

    real en = 0.;
    real tmp;
    if (size > 0)
    {
        real* v = unit_values.data();
        real* a = quad_coeff.data();
        real* b = bias.data();
        if(share_quad_coeff)
            for(register int i=0; i<size; i++)
            {
                tmp = a[0]*v[i];
                en += tmp*tmp - b[i]*v[i];
            }
        else
            for(register int i=0; i<size; i++)
            {
                tmp = a[i]*v[i];
                en += tmp*tmp - b[i]*v[i];
            }
    }
    return en;
}

real RBMGaussianLayer::freeEnergyContribution(const Vec& unit_activations)
    const
{
    PLASSERT( unit_activations.size() == size );

    // result = \sum_{i=0}^{size-1} (-(a_i/(2 q_i))^2 + log(q_i)) - n/2 log(Pi)
    real result = -0.5 * size * LogPi;
    for (int i=0; i<size; i++)
    {
        real a_i = unit_activations[i];
        real q_i = share_quad_coeff ? quad_coeff[i] : quad_coeff[0];
        result += pl_log(q_i);
        result -= a_i * a_i / (4 * q_i * q_i);
    }
    return result;
}

real RBMGaussianLayer::fpropNLL(const Vec& target)
{
    PLASSERT( target.size() == input_size );
    computeExpectation();
    computeStdDeviation();

    real ret = 0;
    if( compute_mse_instead_of_nll )
    {
        real r;
        for( int i=0 ; i<size ; i++ )
        {
            r = (target[i] - expectation[i]);
            ret += r * r;
        }
    }
    else
    {
        if(share_quad_coeff)
            for( int i=0 ; i<size ; i++ )
            {
                real r = (target[i] - expectation[i]) * quad_coeff[0];
                ret += r * r + pl_log(sigma[0]);
            }
        else
            for( int i=0 ; i<size ; i++ )
            {
                // ret += (target[i]-expectation[i])^2/(2 sigma[i]^2)
                //      + log(sqrt(2*Pi) * sigma[i])
                real r = (target[i] - expectation[i]) * quad_coeff[i];
                ret += r * r + pl_log(sigma[i]);

            }
        ret += 0.5*size*Log2Pi;
    }
    return ret;
}

void RBMGaussianLayer::fpropNLL(const Mat& targets, const Mat& costs_column)
{

    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );

    computeExpectations();
    computeStdDeviation();

    real nll;
    real *expectation, *target;

    if( compute_mse_instead_of_nll )
    {
        for (int k=0;k<batch_size;k++) // loop over minibatch
        {
            nll = 0;
            expectation = expectations[k];
            target = targets[k];
            real r;
            for( register int i=0 ; i<size ; i++ ) // loop over outputs
            {
                r = (target[i] - expectation[i]);
                nll += r * r;
            }
            costs_column(k,0) = nll;
        }
    }
    else
    {
        if(share_quad_coeff)
            for (int k=0;k<batch_size;k++) // loop over minibatch
            {
                nll = 0;
                expectation = expectations[k];
                target = targets[k];
                for( register int i=0 ; i<size ; i++ ) // loop over outputs
                {
                    real r = (target[i] - expectation[i]) * quad_coeff[0];
                    nll += r * r + pl_log(sigma[0]);
                }
                nll += 0.5*size*Log2Pi;
                costs_column(k,0) = nll;
            }
        else
            for (int k=0;k<batch_size;k++) // loop over minibatch
            {
                nll = 0;
                expectation = expectations[k];
                target = targets[k];
                for( register int i=0 ; i<size ; i++ ) // loop over outputs
                {
                    // nll += (target[i]-expectation[i])^2/(2 sigma[i]^2)
                    //      + log(sqrt(2*Pi) * sigma[i])
                    real r = (target[i] - expectation[i]) * quad_coeff[i];
                    nll += r * r + pl_log(sigma[i]);
                }
                nll += 0.5*size*Log2Pi;
                costs_column(k,0) = nll;
            }
    }
}

void RBMGaussianLayer::bpropNLL(const Vec& target, real nll, Vec& bias_gradient)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );
    bias_gradient.resize( size );

    // bias_gradient = expectation - target
    substract(expectation, target, bias_gradient);

    if( compute_mse_instead_of_nll )
        bias_gradient *= 2.;
    addBiasDecay(bias_gradient);

}

void RBMGaussianLayer::bpropNLL(const Mat& targets, const Mat& costs_column,
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

    if( compute_mse_instead_of_nll )
        bias_gradients *= 2.;
    addBiasDecay(bias_gradients);

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
