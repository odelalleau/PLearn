// -*- C++ -*-

// TopDownAsymetricDeepNetwork.cc
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

/*! \file TopDownAsymetricDeepNetwork.cc */


#define PL_LOG_MODULE_NAME "TopDownAsymetricDeepNetwork"
#include <plearn/io/pl_log.h>

#include "TopDownAsymetricDeepNetwork.h"
#include <plearn/vmat/VMat_computeNearestNeighbors.h>
#include <plearn/vmat/GetInputVMatrix.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMMixedConnection.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    TopDownAsymetricDeepNetwork,
    "Neural net, trained layer-wise in a greedy but focused fashion using autoassociators/RBMs and a supervised non-parametric gradient.",
    "It is highly inspired by the StackedFocusedAutoassociators class,\n"
    "and can use use the same RBMLayer and RBMConnection components.\n"
    );

TopDownAsymetricDeepNetwork::TopDownAsymetricDeepNetwork() :
    cd_learning_rate( 0. ),
    cd_decrease_ct( 0. ),
    greedy_learning_rate( 0. ),
    greedy_decrease_ct( 0. ),
    fine_tuning_learning_rate( 0. ),
    fine_tuning_decrease_ct( 0. ),
    n_classes( -1 ),
    output_weights_l1_penalty_factor(0),
    output_weights_l2_penalty_factor(0),
    fraction_of_masked_inputs( 0 ),
    n_layers( 0 ),
    currently_trained_layer( 0 )
{
    // random_gen will be initialized in PLearner::build_()
    random_gen = new PRandom();
    nstages = 0;
}

void TopDownAsymetricDeepNetwork::declareOptions(OptionList& ol)
{
    declareOption(ol, "cd_learning_rate", 
                  &TopDownAsymetricDeepNetwork::cd_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the RBM "
                  "contrastive divergence training.\n");

    declareOption(ol, "cd_decrease_ct", 
                  &TopDownAsymetricDeepNetwork::cd_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "the RBMs contrastive\n"
                  "divergence training. When a hidden layer has finished "
                  "its training,\n"
                  "the learning rate is reset to it's initial value.\n");

    declareOption(ol, "greedy_learning_rate", 
                  &TopDownAsymetricDeepNetwork::greedy_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the autoassociator "
                  "gradient descent training.\n");

    declareOption(ol, "greedy_decrease_ct", 
                  &TopDownAsymetricDeepNetwork::greedy_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "the autoassociator\n"
                  "gradient descent training. When a hidden layer has finished "
                  "its training,\n"
                  "the learning rate is reset to it's initial value.\n");

    declareOption(ol, "fine_tuning_learning_rate", 
                  &TopDownAsymetricDeepNetwork::fine_tuning_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the fine tuning "
                  "gradient descent.\n");

    declareOption(ol, "fine_tuning_decrease_ct", 
                  &TopDownAsymetricDeepNetwork::fine_tuning_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "fine tuning\n"
                  "gradient descent.\n");

    declareOption(ol, "training_schedule", 
                  &TopDownAsymetricDeepNetwork::training_schedule,
                  OptionBase::buildoption,
                  "Number of examples to use during each phase "
                  "of greedy pre-training.\n"
                  "The number of fine-tunig steps is defined by nstages.\n"
        );

    declareOption(ol, "layers", &TopDownAsymetricDeepNetwork::layers,
                  OptionBase::buildoption,
                  "The layers of units in the network. The first element\n"
                  "of this vector should be the input layer and the\n"
                  "subsequent elements should be the hidden layers. The\n"
                  "output layer should not be included in layers.\n"
                  "These layers will be used only for bottom up inference.\n");

    declareOption(ol, "top_down_layers", 
                  &TopDownAsymetricDeepNetwork::top_down_layers,
                  OptionBase::buildoption,
                  "The layers of units used for top down inference during\n"
                  "greedy training of an RBM/autoencoder.");

    declareOption(ol, "connections", &TopDownAsymetricDeepNetwork::connections,
                  OptionBase::buildoption,
                  "The weights of the connections between the layers");

    declareOption(ol, "reconstruction_connections", 
                  &TopDownAsymetricDeepNetwork::reconstruction_connections,
                  OptionBase::buildoption,
                  "The reconstruction weights of the autoassociators");

    declareOption(ol, "n_classes", 
                  &TopDownAsymetricDeepNetwork::n_classes,
                  OptionBase::buildoption,
                  "Number of classes.");

    declareOption(ol, "output_weights_l1_penalty_factor", 
                  &TopDownAsymetricDeepNetwork::output_weights_l1_penalty_factor,
                  OptionBase::buildoption,
                  "Output weights l1_penalty_factor.\n");

    declareOption(ol, "output_weights_l2_penalty_factor", 
                  &TopDownAsymetricDeepNetwork::output_weights_l2_penalty_factor,
                  OptionBase::buildoption,
                  "Output weights l2_penalty_factor.\n");

    declareOption(ol, "fraction_of_masked_inputs", 
                  &TopDownAsymetricDeepNetwork::fraction_of_masked_inputs,
                  OptionBase::buildoption,
                  "Fraction of the autoassociators' random input components "
                  "that are\n"
                  "masked, i.e. unsused to reconstruct the input.\n");

    declareOption(ol, "greedy_stages", 
                  &TopDownAsymetricDeepNetwork::greedy_stages,
                  OptionBase::learntoption,
                  "Number of training samples seen in the different greedy "
                  "phases.\n"
        );

    declareOption(ol, "n_layers", &TopDownAsymetricDeepNetwork::n_layers,
                  OptionBase::learntoption,
                  "Number of layers"
        );

    declareOption(ol, "final_module", 
                  &TopDownAsymetricDeepNetwork::final_module,
                  OptionBase::learntoption,
                  "Output layer of neural net"
        );

    declareOption(ol, "final_cost", 
                  &TopDownAsymetricDeepNetwork::final_cost,
                  OptionBase::learntoption,
                  "Cost on output layer of neural net"
        );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void TopDownAsymetricDeepNetwork::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.

    MODULE_LOG << "build_() called" << endl;

    if(inputsize_ > 0 && targetsize_ > 0)
    {
        // Initialize some learnt variables
        n_layers = layers.length();
        
        if( n_classes <= 0 )
            PLERROR("TopDownAsymetricDeepNetwork::build_() - \n"
                    "n_classes should be > 0.\n");

        if( weightsize_ > 0 )
            PLERROR("TopDownAsymetricDeepNetwork::build_() - \n"
                    "usage of weighted samples (weight size > 0) is not\n"
                    "implemented yet.\n");

        if( training_schedule.length() != n_layers-1 )        
            PLERROR("TopDownAsymetricDeepNetwork::build_() - \n"
                    "training_schedule should have %d elements.\n",
                    n_layers-1);
        
        if(greedy_stages.length() == 0)
        {
            greedy_stages.resize(n_layers-1);
            greedy_stages.clear();
        }

        if(stage > 0)
            currently_trained_layer = n_layers;
        else
        {            
            currently_trained_layer = n_layers-1;
            while(currently_trained_layer>1
                  && greedy_stages[currently_trained_layer-1] <= 0)
                currently_trained_layer--;
        }

        build_layers_and_connections();

        if( !final_module || !final_cost )
            build_output_layer_and_cost();
    }
}

void TopDownAsymetricDeepNetwork::build_output_layer_and_cost()
{
    GradNNetLayerModule* gnl = new GradNNetLayerModule();
    gnl->input_size = layers[n_layers-1]->size;
    gnl->output_size = n_classes;
    gnl->L1_penalty_factor = output_weights_l1_penalty_factor;
    gnl->L2_penalty_factor = output_weights_l2_penalty_factor;
    gnl->random_gen = random_gen;
    gnl->build();

    SoftmaxModule* sm = new SoftmaxModule();
    sm->input_size = n_classes;
    sm->random_gen = random_gen;
    sm->build();

    ModuleStackModule* msm = new ModuleStackModule();
    msm->modules.resize(2);
    msm->modules[0] = gnl;
    msm->modules[1] = sm;
    msm->random_gen = random_gen;
    msm->build();
    final_module = msm;

    final_module->forget();

    NLLCostModule* nll = new NLLCostModule();
    nll->input_size = n_classes;
    nll->random_gen = random_gen;
    nll->build();
    
    ClassErrorCostModule* class_error = new ClassErrorCostModule();
    class_error->input_size = n_classes;
    class_error->random_gen = random_gen;
    class_error->build();

    CombiningCostsModule* comb_costs = new CombiningCostsModule();
    comb_costs->cost_weights.resize(2);
    comb_costs->cost_weights[0] = 1;
    comb_costs->cost_weights[1] = 0;
    comb_costs->sub_costs.resize(2);
    comb_costs->sub_costs[0] = nll;
    comb_costs->sub_costs[1] = class_error;
    comb_costs->build();

    final_cost = comb_costs;
    final_cost->forget();
}

void TopDownAsymetricDeepNetwork::build_layers_and_connections()
{
    MODULE_LOG << "build_layers_and_connections() called" << endl;

    if( connections.length() != n_layers-1 )
        PLERROR("TopDownAsymetricDeepNetwork::build_layers_and_connections() - \n"
                "there should be %d connections.\n",
                n_layers-1);

    if( !fast_exact_is_equal( greedy_learning_rate, 0 ) 
        && reconstruction_connections.length() != n_layers-1 )
        PLERROR("TopDownAsymetricDeepNetwork::build_layers_and_connections() - \n"
                "there should be %d reconstruction connections.\n",
                n_layers-1);
    
    if(  !( reconstruction_connections.length() == 0
            || reconstruction_connections.length() == n_layers-1 ) )
        PLERROR("TopDownAsymetricDeepNetwork::build_layers_and_connections() - \n"
                "there should be either 0 or %d reconstruction connections.\n",
                n_layers-1);
    
    
    if(top_down_layers.length() != n_layers 
       && top_down_layers.length() != 0)
        PLERROR("TopDownAsymetricDeepNetwork::build_layers_and_connections() - \n"
                "there should be either 0 of %d top_down_layers.\n",
                n_layers);
        
    if(layers[0]->size != inputsize_)
        PLERROR("TopDownAsymetricDeepNetwork::build_layers_and_connections() - \n"
                "layers[0] should have a size of %d.\n",
                inputsize_);
    
    if(top_down_layers[0]->size != inputsize_)
        PLERROR("TopDownAsymetricDeepNetwork::build_layers_and_connections() - \n"
                "top_down_layers[0] should have a size of %d.\n",
                inputsize_);
    
    if( fraction_of_masked_inputs < 0 )
        PLERROR("TopDownAsymetricDeepNetwork::build_()"
                " - \n"
                "fraction_of_masked_inputs should be > or equal to 0.\n");

    activations.resize( n_layers );
    expectations.resize( n_layers );
    activation_gradients.resize( n_layers );
    expectation_gradients.resize( n_layers );

    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        if( layers[i]->size != connections[i]->down_size )
            PLERROR("TopDownAsymetricDeepNetwork::build_layers_and_connections() "
                    "- \n"
                    "connections[%i] should have a down_size of %d.\n",
                    i, layers[i]->size);

        if( top_down_layers[i]->size != connections[i]->down_size )
            PLERROR("TopDownAsymetricDeepNetwork::build_layers_and_connections() "
                    "- \n"
                    "top_down_layers[%i] should have a size of %d.\n",
                    i, connections[i]->down_size);

        if( connections[i]->up_size != layers[i+1]->size )
            PLERROR("TopDownAsymetricDeepNetwork::build_layers_and_connections() "
                    "- \n"
                    "connections[%i] should have a up_size of %d.\n",
                    i, layers[i+1]->size);

        if( connections[i]->up_size != top_down_layers[i+1]->size )
            PLERROR("TopDownAsymetricDeepNetwork::build_layers_and_connections() "
                    "- \n"
                    "top_down_layers[%i] should have a up_size of %d.\n",
                    i, connections[i]->up_size);

        if( reconstruction_connections.length() != 0 )
        {
            if( layers[i+1]->size != reconstruction_connections[i]->down_size )
                PLERROR("TopDownAsymetricDeepNetwork::build_layers_and_connections() "
                        "- \n"
                        "recontruction_connections[%i] should have a down_size of "
                            "%d.\n",
                        i, layers[i+1]->size);
            
            if( reconstruction_connections[i]->up_size != layers[i]->size )
                PLERROR("TopDownAsymetricDeepNetwork::build_layers_and_connections() "
                        "- \n"
                        "recontruction_connections[%i] should have a up_size of "
                        "%d.\n",
                        i, layers[i]->size);
        }
        
        if( !(layers[i]->random_gen) )
        {
            layers[i]->random_gen = random_gen;
            layers[i]->forget();
        }

        if( !(top_down_layers[i]->random_gen) )
        {
            top_down_layers[i]->random_gen = random_gen;
            top_down_layers[i]->forget();
        }

        if( !(connections[i]->random_gen) )
        {
            connections[i]->random_gen = random_gen;
            connections[i]->forget();
        }

        if( reconstruction_connections.length() != 0
            && !(reconstruction_connections[i]->random_gen) )
        {
            reconstruction_connections[i]->random_gen = random_gen;
            reconstruction_connections[i]->forget();
        }        

        activations[i].resize( layers[i]->size );
        expectations[i].resize( layers[i]->size );
        activation_gradients[i].resize( layers[i]->size );
        expectation_gradients[i].resize( layers[i]->size );
    }

    if( !(layers[n_layers-1]->random_gen) )
    {
        layers[n_layers-1]->random_gen = random_gen;
        layers[n_layers-1]->forget();
    }
    if( !(top_down_layers[n_layers-1]->random_gen) )
    {
        top_down_layers[n_layers-1]->random_gen = random_gen;
        top_down_layers[n_layers-1]->forget();
    }
    activations[n_layers-1].resize( layers[n_layers-1]->size );
    expectations[n_layers-1].resize( layers[n_layers-1]->size );
    activation_gradients[n_layers-1].resize( layers[n_layers-1]->size );
    expectation_gradients[n_layers-1].resize( layers[n_layers-1]->size );
}

// ### Nothing to add here, simply calls build_
void TopDownAsymetricDeepNetwork::build()
{
    inherited::build();
    build_();
}


void TopDownAsymetricDeepNetwork::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(, copies);

    // Public options
    deepCopyField(training_schedule, copies);
    deepCopyField(layers, copies);
    deepCopyField(top_down_layers, copies);
    deepCopyField(connections, copies);
    deepCopyField(reconstruction_connections, copies);

    // Protected options
    deepCopyField(activations, copies);
    deepCopyField(expectations, copies);
    deepCopyField(activation_gradients, copies);
    deepCopyField(expectation_gradients, copies);
    deepCopyField(reconstruction_activations, copies);
    deepCopyField(reconstruction_activation_gradients, copies);
    deepCopyField(reconstruction_expectation_gradients, copies);
    deepCopyField(input_representation, copies);
    deepCopyField(masked_autoassociator_input, copies);
    deepCopyField(autoassociator_input_indices, copies);
    deepCopyField(pos_down_val, copies);
    deepCopyField(pos_up_val, copies);
    deepCopyField(neg_down_val, copies);
    deepCopyField(neg_up_val, copies);
    deepCopyField(final_cost_input, copies);
    deepCopyField(final_cost_value, copies);
    deepCopyField(final_cost_gradient, copies);
    deepCopyField(greedy_stages, copies);
    deepCopyField(final_module, copies);
    deepCopyField(final_cost, copies);
}


int TopDownAsymetricDeepNetwork::outputsize() const
{
//    if(currently_trained_layer < n_layers)
//        return layers[currently_trained_layer]->size;
    return n_classes;
}

void TopDownAsymetricDeepNetwork::forget()
{
    inherited::forget();

    for( int i=0 ; i<n_layers ; i++ )
        layers[i]->forget();
    
    for( int i=0 ; i<n_layers ; i++ )
        top_down_layers[i]->forget();
    
    for( int i=0 ; i<n_layers-1 ; i++ )
        connections[i]->forget();
    
    for( int i=0; i<reconstruction_connections.length(); i++)
        reconstruction_connections[i]->forget();

    build_output_layer_and_cost();

    stage = 0;
    greedy_stages.clear();
}

void TopDownAsymetricDeepNetwork::train()
{
    MODULE_LOG << "train() called " << endl;
    MODULE_LOG << "  training_schedule = " << training_schedule << endl;

    Vec input( inputsize() );
    Vec target( targetsize() );
    real weight; // unused

    TVec<string> train_cost_names = getTrainCostNames() ;
    Vec train_costs( train_cost_names.length() );
    train_costs.fill(MISSING_VALUE) ;

    int nsamples = train_set->length();
    int sample;

    PP<ProgressBar> pb;

    // clear stats of previous epoch
    train_stats->forget();

    int init_stage;

    /***** initial greedy training *****/
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        MODULE_LOG << "Training connection weights between layers " << i
            << " and " << i+1 << endl;

        int end_stage = training_schedule[i];
        int* this_stage = greedy_stages.subVec(i,1).data();
        init_stage = *this_stage;

        MODULE_LOG << "  stage = " << *this_stage << endl;
        MODULE_LOG << "  end_stage = " << end_stage << endl;
        MODULE_LOG << "  greedy_learning_rate = " << greedy_learning_rate << endl;
        MODULE_LOG << "  cd_learning_rate = " << cd_learning_rate << endl;

        if( report_progress && *this_stage < end_stage )
            pb = new ProgressBar( "Training layer "+tostring(i)
                                  +" of "+classname(),
                                  end_stage - init_stage );

        train_costs.fill(MISSING_VALUE);
        reconstruction_activations.resize(layers[i]->size);
        reconstruction_activation_gradients.resize(layers[i]->size);
        reconstruction_expectation_gradients.resize(layers[i]->size);

        input_representation.resize(layers[i]->size);
        pos_down_val.resize(layers[i]->size);
        pos_up_val.resize(layers[i+1]->size);
        neg_down_val.resize(layers[i]->size);
        neg_up_val.resize(layers[i+1]->size);
        if( fraction_of_masked_inputs > 0 )
        {
            masked_autoassociator_input.resize(layers[i]->size);
            autoassociator_input_indices.resize(layers[i]->size);
            for( int j=0 ; j < autoassociator_input_indices.length() ; j++ )
                autoassociator_input_indices[j] = j;
        }

        for( ; *this_stage<end_stage ; (*this_stage)++ )
        {
            
            sample = *this_stage % nsamples;
            train_set->getExample(sample, input, target, weight);
            greedyStep( input, target, i, train_costs, *this_stage);
            train_stats->update( train_costs );

            if( pb )
                pb->update( *this_stage - init_stage + 1 );
        }
    }

    /***** fine-tuning by gradient descent *****/
    if( stage < nstages )
    {

        MODULE_LOG << "Fine-tuning all parameters, by gradient descent" << endl;
        MODULE_LOG << "  stage = " << stage << endl;
        MODULE_LOG << "  nstages = " << nstages << endl;
        MODULE_LOG << "  fine_tuning_learning_rate = " << 
            fine_tuning_learning_rate << endl;

        init_stage = stage;
        if( report_progress && stage < nstages )
            pb = new ProgressBar( "Fine-tuning parameters of all layers of "
                                  + classname(),
                                  nstages - init_stage );

        setLearningRate( fine_tuning_learning_rate );
        train_costs.fill(MISSING_VALUE);

        final_cost_input.resize(n_classes);
        final_cost_value.resize(2); // Should be resized anyways
        final_cost_gradient.resize(n_classes);
        input_representation.resize(layers.last()->size);
        for( ; stage<nstages ; stage++ )
        {
            sample = stage % nsamples;
            if( !fast_exact_is_equal( fine_tuning_decrease_ct, 0. ) )
                setLearningRate( fine_tuning_learning_rate
                                 / (1. + fine_tuning_decrease_ct * stage ) );

            train_set->getExample( sample, input, target, weight );

            fineTuningStep( input, target, train_costs);
            train_stats->update( train_costs );

            if( pb )
                pb->update( stage - init_stage + 1 );
        }
    }
    
    train_stats->finalize();
    MODULE_LOG << "  train costs = " << train_stats->getMean() << endl;


    // Update currently_trained_layer
    if(stage > 0)
        currently_trained_layer = n_layers;
    else
    {            
        currently_trained_layer = n_layers-1;
        while(currently_trained_layer>1 
              && greedy_stages[currently_trained_layer-1] <= 0)
            currently_trained_layer--;
    }
}

void TopDownAsymetricDeepNetwork::greedyStep( 
    const Vec& input, const Vec& target, int index, 
    Vec train_costs, int this_stage)
{
    PLASSERT( index < n_layers );
    real lr;

    // Get example representation
    computeRepresentation(input, input_representation, 
                          index);
    // Autoassociator learning
    if( !fast_exact_is_equal( greedy_learning_rate, 0 ) )
    {
        if( !fast_exact_is_equal( greedy_decrease_ct , 0 ) )
            lr = greedy_learning_rate/(1 + greedy_decrease_ct 
                                       * this_stage); 
        else
            lr = greedy_learning_rate;

        if( fraction_of_masked_inputs > 0 )
            random_gen->shuffleElements(autoassociator_input_indices);

        top_down_layers[index]->setLearningRate( lr );
        connections[index]->setLearningRate( lr );
        reconstruction_connections[index]->setLearningRate( lr );
        layers[index+1]->setLearningRate( lr );

        if( fraction_of_masked_inputs > 0 )
        {
            masked_autoassociator_input << input_representation;
            for( int j=0 ; j < round(fraction_of_masked_inputs*layers[index]->size) ; j++)
                masked_autoassociator_input[ autoassociator_input_indices[j] ] = 0; 
            connections[index]->fprop( masked_autoassociator_input, activations[index+1]);
        }
        else
            connections[index]->fprop(input_representation,
                                      activations[index+1]);
        layers[index+1]->fprop(activations[index+1], expectations[index+1]);

        reconstruction_connections[ index ]->fprop( expectations[index+1],
                                                    reconstruction_activations);
        top_down_layers[ index ]->fprop( reconstruction_activations,
                                top_down_layers[ index ]->expectation);
        
        top_down_layers[ index ]->activation << reconstruction_activations;
        top_down_layers[ index ]->setExpectationByRef(
            top_down_layers[ index ]->expectation);
        real rec_err = top_down_layers[ index ]->fpropNLL(
            input_representation);
        train_costs[index] = rec_err;
        
        top_down_layers[ index ]->bpropNLL(
            input_representation, rec_err,
            reconstruction_activation_gradients);
    }

    // RBM learning
    if( !fast_exact_is_equal( cd_learning_rate, 0 ) )
    {
        connections[index]->setAsDownInput( input_representation );
        layers[index+1]->getAllActivations( connections[index] );
        layers[index+1]->computeExpectation();
        layers[index+1]->generateSample();
        
        // accumulate positive stats using the expectation
        // we deep-copy because the value will change during negative phase
        pos_down_val = expectations[index];
        pos_up_val << layers[index+1]->expectation;
        
        // down propagation, starting from a sample of layers[index+1]
        connections[index]->setAsUpInput( layers[index+1]->sample );
        
        top_down_layers[index]->getAllActivations( connections[index] );
        top_down_layers[index]->computeExpectation();
        top_down_layers[index]->generateSample();
        
        // negative phase
        connections[index]->setAsDownInput( top_down_layers[index]->sample );
        layers[index+1]->getAllActivations( connections[index] );
        layers[index+1]->computeExpectation();
        // accumulate negative stats
        // no need to deep-copy because the values won't change before update
        neg_down_val = top_down_layers[index]->sample;
        neg_up_val = layers[index+1]->expectation;
    }
    
    // Update hidden layer bias and weights

    if( !fast_exact_is_equal( greedy_learning_rate, 0 ) )
    {
        top_down_layers[ index ]->update(reconstruction_activation_gradients);
    
        reconstruction_connections[ index ]->bpropUpdate( 
            expectations[index+1],
            reconstruction_activations, 
            reconstruction_expectation_gradients, 
            reconstruction_activation_gradients);

        layers[ index+1 ]->bpropUpdate( 
            activations[index+1],
            expectations[index+1],
            // reused
            reconstruction_activation_gradients,
            reconstruction_expectation_gradients);
        
        if( fraction_of_masked_inputs > 0 )
            connections[ index ]->bpropUpdate( 
                masked_autoassociator_input,
                activations[index+1],
                reconstruction_expectation_gradients, //reused
                reconstruction_activation_gradients);
        else
            connections[ index ]->bpropUpdate( 
                input_representation,
                activations[index+1],
                reconstruction_expectation_gradients, //reused
                reconstruction_activation_gradients);
    }
     

    // RBM updates
    if( !fast_exact_is_equal( cd_learning_rate, 0 ) )
    {
        if( !fast_exact_is_equal( cd_decrease_ct , 0 ) )
            lr = cd_learning_rate/(1 + cd_decrease_ct 
                                       * this_stage); 
        else
            lr = cd_learning_rate;

        top_down_layers[index]->setLearningRate( lr );
        connections[index]->setLearningRate( lr );
        layers[index+1]->setLearningRate( lr );

        top_down_layers[index]->update( pos_down_val, neg_down_val );
        connections[index]->update( pos_down_val, pos_up_val,
                                    neg_down_val, neg_up_val );
        layers[index+1]->update( pos_up_val, neg_up_val );
    }
}

void TopDownAsymetricDeepNetwork::fineTuningStep( 
    const Vec& input, const Vec& target,
    Vec& train_costs)
{
    // Get example representation
    computeRepresentation(input, input_representation, 
                          n_layers-1);

    final_module->fprop( input_representation, final_cost_input );
    final_cost->fprop( final_cost_input, target, final_cost_value );
        
    final_cost->bpropUpdate( final_cost_input, target,
                             final_cost_value[0],
                             final_cost_gradient );
    final_module->bpropUpdate( input_representation,
                               final_cost_input,
                               expectation_gradients[ n_layers-1 ],
                               final_cost_gradient );
    train_costs.last() = final_cost_value[0];
    for( int i=n_layers-1 ; i>0 ; i-- )
    {
        layers[i]->bpropUpdate( activations[i],
                                expectations[i],
                                activation_gradients[i],
                                expectation_gradients[i] );
        
        
        connections[i-1]->bpropUpdate( expectations[i-1],
                                       activations[i],
                                       expectation_gradients[i-1],
                                       activation_gradients[i] );
    }        
}

void TopDownAsymetricDeepNetwork::computeRepresentation(
    const Vec& input,
    Vec& representation,
    int layer) const
{
    if(layer == 0)
    {
        representation.resize(input.length());
        expectations[0] << input;
        representation << input;
        return;
    }

    expectations[0] << input;
    for( int i=0 ; i<layer; i++ )
    {
        connections[i]->fprop( expectations[i], activations[i+1] );
        layers[i+1]->fprop(activations[i+1],expectations[i+1]);
    }
    representation.resize(expectations[layer].length());
    representation << expectations[layer];
}

void TopDownAsymetricDeepNetwork::computeOutput(
    const Vec& input, Vec& output) const
{
    computeRepresentation(input,input_representation, 
                          min(currently_trained_layer,n_layers-1));
    final_module->fprop( input_representation, final_cost_input );
    output[0] = argmax(final_cost_input);
}

void TopDownAsymetricDeepNetwork::computeCostsFromOutputs(
    const Vec& input, const Vec& output,
    const Vec& target, Vec& costs) const
{

    //Assumes that computeOutput has been called
    costs.resize( getTestCostNames().length() );
    costs.fill( MISSING_VALUE );

    if( currently_trained_layer<n_layers 
        && reconstruction_connections.length() != 0 )
    {
        reconstruction_connections[ currently_trained_layer-1 ]->fprop( 
            expectations[currently_trained_layer],
            reconstruction_activations);
        top_down_layers[ currently_trained_layer-1 ]->fprop( 
            reconstruction_activations,
            top_down_layers[ currently_trained_layer-1 ]->expectation);
        
        top_down_layers[ currently_trained_layer-1 ]->activation << 
            reconstruction_activations;
        top_down_layers[ currently_trained_layer-1 ]->setExpectationByRef( 
            top_down_layers[ currently_trained_layer-1 ]->expectation);
        costs[ currently_trained_layer-1 ]  = 
            top_down_layers[ currently_trained_layer-1 ]->fpropNLL(
                expectations[currently_trained_layer-1]);
    }

    if( ((int)round(output[0])) == ((int)round(target[0])) )
        costs[n_layers-1] = 0;
    else
        costs[n_layers-1] = 1;
}

TVec<string> TopDownAsymetricDeepNetwork::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).

    TVec<string> cost_names(0);

    for( int i=0; i<layers.size()-1; i++)
        cost_names.push_back("reconstruction_error_" + tostring(i+1));
        
    cost_names.append( "class_error" );

    return cost_names;
}

TVec<string> TopDownAsymetricDeepNetwork::getTrainCostNames() const
{
    TVec<string> cost_names = getTestCostNames();
    cost_names.append( "NLL" );
    return cost_names;    
}

//#####  Helper functions  ##################################################

void TopDownAsymetricDeepNetwork::setLearningRate( real the_learning_rate )
{
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        layers[i]->setLearningRate( the_learning_rate );
        top_down_layers[i]->setLearningRate( the_learning_rate );
        connections[i]->setLearningRate( the_learning_rate );
    }
    layers[n_layers-1]->setLearningRate( the_learning_rate );
    top_down_layers[n_layers-1]->setLearningRate( the_learning_rate );

    final_module->setLearningRate( the_learning_rate );
    final_cost->setLearningRate( the_learning_rate );
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
