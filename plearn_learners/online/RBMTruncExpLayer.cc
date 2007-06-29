// -*- C++ -*-

// RBMTruncExpLayer.cc
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



#include "RBMTruncExpLayer.h"
#include <plearn/math/TMat_maths.h>
#include "RBMConnection.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMTruncExpLayer,
    "RBM Layer where unit distribution is a truncated exponential in [0,1]",
    "");

RBMTruncExpLayer::RBMTruncExpLayer( real the_learning_rate ) :
    inherited( the_learning_rate )
{
}

RBMTruncExpLayer::RBMTruncExpLayer( int the_size, real the_learning_rate ) :
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

void RBMTruncExpLayer::generateSample()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    /* The cumulative is :
     * C(U) = P(u<U | x) = (1 - exp(U a)) / (1 - exp(a)) if 0 < U < 1,
     *        0 if U <= 0 and
     *        1 if 1 <= U
     *
     * And the inverse, if 0 <= s <=1:
     * C^{-1}(s) = log(1 - s*(1 - exp(a)) / a
     */

    for( int i=0 ; i<size ; i++ )
    {
        real s = random_gen->uniform_sample();
        real a_i = activation[i];

        // Polynomial approximation to avoid numerical instability if a ~ 0
        // C^{-1}(s) ~ s + (s - s^2)/2 * a + O(a^2)
        if( fabs( a_i ) <= 1e-5 )
            sample[i] = s + a_i*( s*(1 - s)/2 );
        else
            sample[i] = logadd( pl_log( 1-s ), pl_log(s) + a_i ) / a_i;
    }
}

void RBMTruncExpLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    /* Conditional expectation:
     * E[u|x] = 1/(1-exp(-a)) - 1/a
     */

    for( int i=0 ; i<size ; i++ )
    {
        real a_i = activation[i];

        // Polynomial approximation to avoid numerical instability
        // f(a) = 1/2 + a/12 - a^3/720 + O(a^5)
        if( fabs( a_i ) <= 0.01 )
            expectation[i] = 0.5 + a_i*(1./12. - a_i*a_i/720.);
        else
            expectation[i] = 1/(1-exp(-a_i)) - 1/a_i;
    }

    expectation_is_up_to_date = true;
}


void RBMTruncExpLayer::fprop( const Vec& input, Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    for( int i=0 ; i<size ; i++ )
    {
        real a_i = input[i] + bias[i];

        // Polynomial approximation to avoid numerical instability
        // f(a) = 1/(1-exp(-a) - 1/a
        // f(a) = 1/2 + a/12 - a^3/720 + O(a^5)
        if( fabs( a_i ) <= 0.01 )
            output[i] = 0.5 + a_i*(1./12. - a_i*a_i/720.);
        else
            output[i] = 1/(1-exp(-a_i)) - 1/a_i;
    }
}

void RBMTruncExpLayer::bpropUpdate(const Vec& input, const Vec& output,
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

    // df/da = exp(a)/(1-exp(a))^2 - 1/a^2

    for( int i=0 ; i<size ; i++ )
    {
        real a_i = input[i] + bias[i];
        real in_grad_i;

        // Polynomial approximation to avoid numerical instability
        // df/da = -1/12 + a^2/240 + O(a^4)
        if( fabs( a_i ) <= 0.01 )
        {
            in_grad_i = output_gradient[i] * ( -1./12. + a_i * a_i / 240. );
        }
        else
        {
            real ea_i = exp( a_i );
            in_grad_i = output_gradient[i] * (
                ea_i/( (1 - ea_i) * (1 - ea_i) ) + 1/(a_i * a_i) );
        }

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



void RBMTruncExpLayer::declareOptions(OptionList& ol)
{
/*
    declareOption(ol, "size", &RBMTruncExpLayer::size,
                  OptionBase::buildoption,
                  "Number of units.");
*/
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMTruncExpLayer::build_()
{
}

void RBMTruncExpLayer::build()
{
    inherited::build();
    build_();
}


void RBMTruncExpLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
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
