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

//////////////////
// setBatchSize //
//////////////////
void RBMMixedLayer::setBatchSize( int the_batch_size )
{
    inherited::setBatchSize( the_batch_size );
    for( int i = 0; i < n_layers; i++ )
        sub_layers[i]->setBatchSize( the_batch_size );
}

////////////////////
// setExpectation //
////////////////////
void RBMMixedLayer::setExpectation(const Vec& the_expectation)
{
    expectation << the_expectation;
    expectation_is_up_to_date=true;
    for( int i = 0; i < n_layers; i++ )
        sub_layers[i]->expectation_is_up_to_date=true;
}

/////////////////////////
// setExpectationByRef //
/////////////////////////
void RBMMixedLayer::setExpectationByRef(const Vec& the_expectation)
{
    expectation = the_expectation;
    expectation_is_up_to_date=true;

     // Rearrange pointers
    for( int i = 0; i < n_layers; i++ )
    {
        int init_pos = init_positions[i];
        PP<RBMLayer> layer = sub_layers[i];
        int layer_size = layer->size;
        
        layer->setExpectationByRef( expectation.subVec(init_pos, layer_size) );
    }

}

/////////////////////
// setExpectations //
/////////////////////
void RBMMixedLayer::setExpectations(const Mat& the_expectations)
{
    batch_size = the_expectations.length();
    setBatchSize( batch_size );
    expectations << the_expectations;
    expectations_are_up_to_date=true;
    for( int i = 0; i < n_layers; i++ )
        sub_layers[i]->expectations_are_up_to_date=true;
}

//////////////////////////
// setExpectationsByRef //
//////////////////////////
void RBMMixedLayer::setExpectationsByRef(const Mat& the_expectations)
{
    batch_size = the_expectations.length();
    setBatchSize( batch_size );
    expectations = the_expectations;
    expectations_are_up_to_date=true;

    // Rearrange pointers
    for( int i = 0; i < n_layers; i++ )
    {
        int init_pos = init_positions[i];
        PP<RBMLayer> layer = sub_layers[i];
        int layer_size = layer->size;

        layer->setExpectationsByRef(expectations.subMatColumns(init_pos,
                                                              layer_size));
    }
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
    {
        if( minibatch )
            sub_layers[i]->expectations_are_up_to_date = false;
        else
            sub_layers[i]->expectation_is_up_to_date = false;
    }
}


////////////////////
// generateSample //
////////////////////
void RBMMixedLayer::generateSample()
{
    for( int i=0 ; i<n_layers ; i++ )
        sub_layers[i]->generateSample();
}

////////////////////
// generateSample //
////////////////////
void RBMMixedLayer::generateSamples()
{
    for( int i=0 ; i<n_layers ; i++ )
        sub_layers[i]->generateSamples();
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

/////////////////////////
// computeExpectations //
/////////////////////////
void RBMMixedLayer::computeExpectations()
{
    if( expectations_are_up_to_date )
        return;

    for( int i=0 ; i < n_layers ; i++ )
        sub_layers[i]->computeExpectations();

    expectations_are_up_to_date = true;
}

///////////
// fprop //
///////////
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

///////////
// fprop //
///////////
void RBMMixedLayer::fprop( const Mat& inputs, Mat& outputs ) const
{
    int mbatch_size = inputs.length();
    PLASSERT( inputs.width() == size );
    outputs.resize( mbatch_size, size );

    for( int i=0 ; i<n_layers ; i++ )
    {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;
        Mat sub_inputs = inputs.subMatColumns(begin, size_i);
        Mat sub_outputs = outputs.subMatColumns(begin, size_i);

        // GCC bug? This doesn't work:
        // sub_layers[i]->fprop( sub_inputs, sub_outputs );
        sub_layers[i]->OnlineLearningModule::fprop( sub_inputs, sub_outputs );
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

    int batch_size = inputs.length();
    PLASSERT( outputs.length() == batch_size );
    PLASSERT( output_gradients.length() == batch_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == size &&
                input_gradients.length() == batch_size,
                "Cannot resize input_gradients and accumulate into it" );
    }
    else
        // Note that, by construction of 'size', the whole gradient vector
        // should be cleared in the calls to sub_layers->bpropUpdate(..) below.
        input_gradients.resize(batch_size, size);

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

void RBMMixedLayer::fpropNLL(const Mat& targets, const Mat& costs_column)
{
    computeExpectation();

    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );

    costs_column.clear();
    mat_nlls.resize(batch_size, n_layers);
    for( int i=0 ; i<n_layers ; i++ )
    {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;
        sub_layers[i]->fpropNLL( targets.subMatColumns(begin, size_i),
                                 mat_nlls.column(i) );
        for( int j=0; j < batch_size; ++j )
            costs_column(j,0) += mat_nlls(j, i);
    }
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

void RBMMixedLayer::bpropNLL(const Mat& targets, const Mat& costs_column,
                             Mat& bias_gradients)
{
    computeExpectations();

    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );
    bias_gradients.resize( batch_size, size );

    for( int i=0 ; i<n_layers ; i++ )
    {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;

        Mat sub_targets = targets.subMatColumns(begin, size_i);
        Mat sub_bias_gradients = bias_gradients.subMatColumns(begin, size_i);
        // TODO: something else than store mat_nlls...
        sub_layers[i]->bpropNLL( sub_targets, mat_nlls.column(i),
                                 sub_bias_gradients );
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

void RBMMixedLayer::update( const Mat& pos_values, const Mat& neg_values )
{
    for( int i=0 ; i<n_layers ; i++ )
    {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;
        Mat sub_pos_values = pos_values.subMatColumns( begin, size_i );
        Mat sub_neg_values = neg_values.subMatColumns( begin, size_i );

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

void RBMMixedLayer::forget()
{
    inherited::forget();
    if( !random_gen )
    {
        PLWARNING("RBMMixedLayer: cannot forget() without random_gen");
        return;
    }
    for( int i=0; i<n_layers; i++ )
    {
        if( !(sub_layers[i]->random_gen) )
            sub_layers[i]->random_gen = random_gen;
        sub_layers[i]->forget();
    }
}

void RBMMixedLayer::build_()
{
    size = 0;
    n_layers = sub_layers.size();
    init_positions.resize(n_layers);

    // Fill init_positions
    for( int i = 0; i < n_layers; i++ )
    {
        init_positions[i] = size;
        size += sub_layers[i]->size;
    }

    // Resize
    layer_of_unit.resize( size );

    activation.resize( size );
    activations.resize( batch_size, size );

    sample.resize( size );
    samples.resize( batch_size, size );

    expectation.resize( size );
    expectations.resize( batch_size, size );

    bias.resize( size );

    // Second loop, to initialize activation, expectation, etc.
    for( int i = 0; i < n_layers; i++ )
    {
        int init_pos = init_positions[i];
        PP<RBMLayer> layer = sub_layers[i];
        int layer_size = layer->size;

        layer_of_unit.subVec(init_pos, layer_size).fill(i);
        layer->batch_size = batch_size;

        layer->activation = activation.subVec(init_pos, layer_size);
        layer->activations = activations.subMatColumns(init_pos, layer_size);

        layer->sample = sample.subVec(init_pos, layer_size);
        layer->samples = samples.subMatColumns(init_pos, layer_size);

        layer->setExpectationByRef( expectation.subVec(init_pos, layer_size) );
        layer->setExpectationsByRef(expectations.subMatColumns(init_pos,
                                                              layer_size));

        bias.subVec(init_pos, layer_size) << layer->bias;
        layer->bias = bias.subVec(init_pos, layer_size);        

        // We changed fields of layer, so we need to rebuild it (especially
        // if it is another RBMMixedLayer)
        layer->build();

        if( learning_rate >= 0. )
            layer->setLearningRate( learning_rate );

        if( momentum >= 0. )
            layer->setMomentum( momentum );

        // If we have a random_gen and sub_layers[i] does not, share it
        if( random_gen && !(sub_layers[i]->random_gen) )
        {
            layer->random_gen = random_gen;
            layer->forget();
        }
    }

    input_size = size;
    output_size = size;
}

void RBMMixedLayer::build()
{
    inherited::build();
    build_();
}


void RBMMixedLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(sub_layers,       copies);
    deepCopyField(init_positions,   copies);
    deepCopyField(layer_of_unit,    copies);
    deepCopyField(nlls,             copies);
    deepCopyField(mat_nlls,         copies);
}

real RBMMixedLayer::energy(const Vec& unit_values) const
{
    real energy = 0;

    for ( int i = 0; i < n_layers; ++i ) {
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;
        Vec values = unit_values.subVec( begin, size_i );
        energy += sub_layers[i]->energy(values);
    }

    return energy;
}

int RBMMixedLayer::getConfigurationCount()
{
    int count = 1;

    for ( int i = 0; i < n_layers; ++i ) {
        int cc_layer_i = sub_layers[i]->getConfigurationCount();
        // Avoiding overflow
        if ( INFINITE_CONFIGURATIONS/cc_layer_i <= count )
            return INFINITE_CONFIGURATIONS;
        count *= cc_layer_i;
    }

    return count;
}

void RBMMixedLayer::getConfiguration(int conf_index, Vec& output)
{
    PLASSERT( output.length() == size );
    PLASSERT( conf_index >= 0 && conf_index < getConfigurationCount() );

    int conf_i = conf_index;
    for ( int i = 0; i < n_layers; ++i ) {
        int conf_layer_i = sub_layers[i]->getConfigurationCount();
        int begin = init_positions[i];
        int size_i = sub_layers[i]->size;
        Vec output_i = output.subVec( begin, size_i );
        sub_layers[i]->getConfiguration(conf_i % conf_layer_i, output_i);
        conf_i /= conf_layer_i;
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
