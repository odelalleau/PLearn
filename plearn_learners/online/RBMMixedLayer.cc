// -*- C++ -*-

// RBMMixedLayer.cc
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



#include "RBMMixedLayer.h"
#include <plearn/math/TMat_maths.h>
#include "RBMConnection.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMMixedLayer,
    "Layer in an RBM, concatenating other sub-layers",
    "");

RBMMixedLayer::RBMMixedLayer()
{
}

RBMMixedLayer::RBMMixedLayer( TVec< PP<RBMLayer> > the_sub_layers ) :
    sub_layers( the_sub_layers )
{
    build();
}


/////////////////////
// setLearningRate //
/////////////////////
void RBMMixedLayer::setLearningRate( real the_learning_rate )
{
    inherited::setLearningRate( the_learning_rate );

    for( int i=0 ; i<n_layers ; i++ )
        sub_layers[i]->setLearningRate( the_learning_rate );
}

/////////////////
// setMomentum //
/////////////////
void RBMMixedLayer::setMomentum( real the_momentum )
{
    inherited::setMomentum( the_momentum );

    for( int i=0 ; i<n_layers ; i++ )
        sub_layers[i]->setMomentum( the_momentum );
}


///////////////////////
// getUnitActivation //
///////////////////////
void RBMMixedLayer::getUnitActivation( int i, PP<RBMConnection> rbmc,
                                       int offset )
{
    inherited::getUnitActivation( i, rbmc, offset );

    int j = layer_of_unit[i];
    sub_layers[j]->expectation_is_up_to_date = false;
}

///////////////////////
// getAllActivations //
///////////////////////
void RBMMixedLayer::getAllActivations( PP<RBMConnection> rbmc, int offset,
                                       bool minibatch )
{
    inherited::getAllActivations( rbmc, offset, minibatch );
    for( int i=0 ; i<n_layers ; i++ )
        sub_layers[i]->expectation_is_up_to_date = false;
}


////////////////////
// generateSample //
////////////////////
void RBMMixedLayer::generateSample()
{
    for( int i=0 ; i<n_layers ; i++ )
        sub_layers[i]->generateSample();
}

////////////////////////
// computeExpectation //
////////////////////////
void RBMMixedLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    for( int i=0 ; i<n_layers ; i++ )
        sub_layers[i]->computeExpectation();

    expectation_is_up_to_date = true;
}


void RBMMixedLayer::fprop( const Vec& input, Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    for( int i=0 ; i<n_layers ; i++ )
    {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;
        Vec sub_input = input.subVec(begin, size_i);
        Vec sub_output = output.subVec(begin, size_i);

        sub_layers[i]->fprop( sub_input, sub_output );
    }
}

void RBMMixedLayer::fprop( const Vec& input, const Vec& rbm_bias,
                           Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( rbm_bias.size() == input_size );
    output.resize( output_size );

    for( int i=0 ; i<n_layers ; i++ )
    {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;
        Vec sub_input = input.subVec(begin, size_i);
        Vec sub_rbm_bias = rbm_bias.subVec(begin, size_i);
        Vec sub_output = output.subVec(begin, size_i);

        sub_layers[i]->fprop( sub_input, sub_rbm_bias, sub_output );
    }
}


/////////////////
// bpropUpdate //
/////////////////
void RBMMixedLayer::bpropUpdate( const Vec& input, const Vec& output,
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
        // Note that, by construction of 'size', the whole gradient vector
        // should be cleared in the calls to sub_layers->bpropUpdate(..) below.
        input_gradient.resize( size );

    for( int i=0 ; i<n_layers ; i++ )
    {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;
        Vec sub_input = input.subVec( begin, size_i );
        Vec sub_output = output.subVec( begin, size_i );
        Vec sub_input_gradient = input_gradient.subVec( begin, size_i );
        Vec sub_output_gradient = output_gradient.subVec( begin, size_i );

        sub_layers[i]->bpropUpdate( sub_input, sub_output,
                                    sub_input_gradient, sub_output_gradient,
                                    accumulate );
    }
}

void RBMMixedLayer::bpropUpdate(const Mat& inputs, const Mat& outputs,
        Mat& input_gradients,
        const Mat& output_gradients,
        bool accumulate)
{
    PLASSERT( inputs.width() == size );
    PLASSERT( outputs.width() == size );
    PLASSERT( output_gradients.width() == size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == size &&
                input_gradients.length() == inputs.length(),
                "Cannot resize input_gradients and accumulate into it" );
    }
    else
        // Note that, by construction of 'size', the whole gradient vector
        // should be cleared in the calls to sub_layers->bpropUpdate(..) below.
        input_gradients.resize(inputs.length(), size);

    for( int i=0 ; i<n_layers ; i++ )
    {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;
        Mat sub_inputs = inputs.subMatColumns( begin, size_i );
        Mat sub_outputs = outputs.subMatColumns( begin, size_i );
        Mat sub_input_gradients =
            input_gradients.subMatColumns( begin, size_i );
        Mat sub_output_gradients =
            output_gradients.subMatColumns( begin, size_i );

        sub_layers[i]->bpropUpdate( sub_inputs, sub_outputs,
                sub_input_gradients, sub_output_gradients,
                accumulate );
    }
}

void RBMMixedLayer::bpropUpdate(const Vec& input, const Vec& rbm_bias,
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

    for( int i=0 ; i<n_layers ; i++ )
    {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;
        Vec sub_input = input.subVec( begin, size_i );
        Vec sub_rbm_bias = rbm_bias.subVec( begin, size_i );
        Vec sub_output = output.subVec( begin, size_i );
        Vec sub_input_gradient = input_gradient.subVec( begin, size_i );
        Vec sub_rbm_bias_gradient = rbm_bias_gradient.subVec( begin, size_i);
        Vec sub_output_gradient = output_gradient.subVec( begin, size_i );

        sub_layers[i]->bpropUpdate( sub_input, sub_rbm_bias, sub_output,
                                    sub_input_gradient, sub_rbm_bias_gradient,
                                    sub_output_gradient );
    }
}

real RBMMixedLayer::fpropNLL(const Vec& target)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );
    nlls.resize(n_layers);

    real ret = 0;
    real nlli = 0;
    for( int i=0 ; i<n_layers ; i++ )
    {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;
        nlli = sub_layers[i]->fpropNLL( target.subVec(begin, size_i));
        nlls[i] = nlli;
        ret += nlli;
    }
    return ret;
}

void RBMMixedLayer::bpropNLL(const Vec& target, real nll, Vec& bias_gradient)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );
    bias_gradient.resize( size );

    for( int i=0 ; i<n_layers ; i++ )
    {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;

        Vec sub_target = target.subVec(begin, size_i);
        Vec sub_bias_gradient = bias_gradient.subVec(begin, size_i);
        sub_layers[i]->bpropNLL( sub_target, nlls[i], sub_bias_gradient );
    }
}

void RBMMixedLayer::declareOptions(OptionList& ol)
{
    declareOption(ol, "sub_layers", &RBMMixedLayer::sub_layers,
                  OptionBase::buildoption,
                  "The concatenated RBMLayers composing this layer.");

    declareOption(ol, "init_positions", &RBMMixedLayer::init_positions,
                  OptionBase::learntoption,
                  " Initial index of the sub_layers.");

    declareOption(ol, "layer_of_unit", &RBMMixedLayer::layer_of_unit,
                  OptionBase::learntoption,
                  "layer_of_unit[i] is the index of sub_layer containing unit"
                  " i.");

    declareOption(ol, "n_layers", &RBMMixedLayer::n_layers,
                  OptionBase::learntoption,
                  "Number of sub-layers.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "bias", &RBMMixedLayer::bias,
                    OptionBase::nosave,
                    "bias is the concatenation of the sub_layer's biases.");

    redeclareOption(ol, "learning_rate", &RBMMixedLayer::learning_rate,
                    OptionBase::nosave,
                    "There is no global learning rate, only sublayers'.");

    redeclareOption(ol, "momentum", &RBMMixedLayer::momentum,
                    OptionBase::nosave,
                    "There is no global momentum, only sublayers'.");
}

void RBMMixedLayer::accumulatePosStats( const Vec& pos_values )
{
    for( int i=0 ; i<n_layers ; i++ )
    {
        Vec sub_pos_values = pos_values.subVec( init_positions[i],
                                                sub_layers[i]->size );
        sub_layers[i]->accumulatePosStats( sub_pos_values );
    }
    pos_count++;
}

void RBMMixedLayer::accumulateNegStats( const Vec& neg_values )
{
    for( int i=0 ; i<n_layers ; i++ )
    {
        Vec sub_neg_values = neg_values.subVec( init_positions[i],
                                                sub_layers[i]->size );
        sub_layers[i]->accumulateNegStats( sub_neg_values );
    }
    neg_count++;
}

void RBMMixedLayer::update()
{
    for( int i=0 ; i<n_layers ; i++ )
        sub_layers[i]->update();

    clearStats();
}

void RBMMixedLayer::update( const Vec& pos_values, const Vec& neg_values )
{
    for( int i=0 ; i<n_layers ; i++ )
    {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;
        Vec sub_pos_values = pos_values.subVec( begin, size_i );
        Vec sub_neg_values = neg_values.subVec( begin, size_i );

        sub_layers[i]->update( sub_pos_values, sub_neg_values );
    }
}

void RBMMixedLayer::reset()
{
    for( int i=0 ; i<n_layers ; i++ )
        sub_layers[i]->reset();

    expectation_is_up_to_date = false;
}

void RBMMixedLayer::clearStats()
{
    for( int i=0 ; i<n_layers ; i++ )
        sub_layers[i]->clearStats();

    pos_count = 0;
    neg_count = 0;
}

void RBMMixedLayer::build_()
{
    size = 0;
    activation.resize( 0 );
    sample.resize( 0 );
    expectation.resize( 0 );
    expectation_is_up_to_date = false;
    bias.resize( 0 );
    layer_of_unit.resize( 0 );

    n_layers = sub_layers.size();
    init_positions.resize( n_layers );

    for( int i=0 ; i<n_layers ; i++ )
    {
        init_positions[i] = size;

        PP<RBMLayer> cur_layer = sub_layers[i];
        size += cur_layer->size;

        activation = merge( activation, cur_layer->activation );
        sample = merge( sample, cur_layer->sample );
        expectation = merge( expectation, cur_layer->expectation );
        bias = merge( bias, cur_layer->bias );
        layer_of_unit.append( TVec<int>( cur_layer->size, i ) );

        if( learning_rate >= 0. )
            sub_layers[i]->setLearningRate( learning_rate );

        if( momentum >= 0. )
            sub_layers[i]->setMomentum( momentum );

        sub_layers[i]->random_gen = random_gen;
    }
}

void RBMMixedLayer::build()
{
    inherited::build();
    build_();
}


void RBMMixedLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(sub_layers, copies);
    deepCopyField(init_positions, copies);
    deepCopyField(layer_of_unit, copies);
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
