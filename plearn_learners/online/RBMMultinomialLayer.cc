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
    units_types = string( the_size, 'l' );
    activation.resize( the_size );
    sample.resize( the_size );
    expectation.resize( the_size );
    bias.resize( the_size );
    bias_pos_stats.resize( the_size );
    bias_neg_stats.resize( the_size );
}
/*
//! Uses "rbmp" to obtain the activations of unit "i" of this layer.
//! This activation vector is computed by the "i+offset"-th unit of "rbmp"
void RBMMultinomialLayer::getUnitActivations( int i, PP<RBMParameters> rbmp,
                                              int offset )
{
    Vec activation = activations.subVec( i, 1 );
    rbmp->computeUnitActivations( i+offset, 1, activation );
    expectation_is_up_to_date = false;
}

void RBMMultinomialLayer::getAllActivations( PP<RBMParameters> rbmp,
                                             int offset )
{
    rbmp->computeUnitActivations( offset, size, activations );
    expectation_is_up_to_date = false;
}
*/

void RBMMultinomialLayer::generateSample()
{
    computeExpectation();

    int i = random_gen->multinomial_sample( expectation );
    fill_one_hot( sample, i, 0., 1. );
}

void RBMMultinomialLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    // expectation = softmax(-activation)
    softmaxMinus(activation, expectation);
    expectation_is_up_to_date = true;
}


void RBMMultinomialLayer::fprop( const Vec& input, Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    // inefficient
    softmaxMinus( input+bias, output );
}

void RBMMultinomialLayer::fprop( const Vec& input, const Vec& rbm_bias,
                                 Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( rbm_bias.size() == input_size );
    output.resize( output_size );

    // inefficient
    softmaxMinus( input+rbm_bias, output );
}

void RBMMultinomialLayer::bpropUpdate(const Vec& input, const Vec& output,
                                      Vec& input_gradient,
                                      const Vec& output_gradient)
{
    PLASSERT( input.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );
    input_gradient.resize( size );

    // input_gradient[i] =
    //      (output_gradient . output - output_gradient[i] ) output[i]
    real outg_dot_out = dot( output_gradient, output );
    real* out = output.data();
    real* outg = output_gradient.data();
    real* ing = input_gradient.data();
    for( int i=0 ; i<size ; i++ )
        ing[i] = (outg_dot_out - outg[i]) * out[i];

    if( momentum == 0. )
    {
        // update the bias: bias -= learning_rate * input_gradient
        multiplyAcc( bias, input_gradient, -learning_rate );
    }
    else
    {
        bias_inc.resize( size );
        // The update rule becomes:
        // bias_inc = momentum * bias_inc - learning_rate * input_gradient
        // bias += bias_inc
        multiplyScaledAdd(input_gradient, momentum, -learning_rate, bias_inc);
        bias += bias_inc;
    }
}

void RBMMultinomialLayer::bpropUpdate(const Vec& input, const Vec& rbm_bias, 
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

    // input_gradient[i] =
    //      (output_gradient . output - output_gradient[i] ) output[i]
    real outg_dot_out = dot( output_gradient, output );
    real* out = output.data();
    real* outg = output_gradient.data();
    real* ing = input_gradient.data();
    for( int i=0 ; i<size ; i++ )
        ing[i] = (outg_dot_out - outg[i]) * out[i];

    rbm_bias_gradient << input_gradient;
}

real RBMMultinomialLayer::fpropNLL(const Vec& target)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );

    real ret = 0;
    
    real target_i, expectation_i;
    for( int i=0 ; i<size ; i++ )
    {
        target_i = target[i];
        expectation_i = expectation[i];
        if(!fast_exact_is_equal(target_i,0.0))
            ret -= target_i * pl_log(expectation_i);
    }
    return ret;
}

void RBMMultinomialLayer::bpropNLL(const Vec& target, real nll, Vec bias_gradient)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );
    bias_gradient.resize( size );

    real sum_tar = sum( target );
    real* exp = expectation.data();
    real* tar = target.data();
    real* biasg = bias_gradient.data();
    for( int i=0 ; i<size ; i++ )
        biasg[i] = tar[i] - sum_tar * exp[i];    
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
    if( size < 0 )
        size = int(units_types.size());
    if( size != (int) units_types.size() )
        units_types = string( size, 'l' );
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
