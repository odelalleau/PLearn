// -*- C++ -*-

// StackedAutoassociatorsNet.cc
//
// Copyright (C) 2007 Hugo Larochelle
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

// Authors: Pascal Lamblin

/*! \file StackedAutoassociatorsNet.cc */


#define PL_LOG_MODULE_NAME "StackedAutoassociatorsNet"
#include <plearn/io/pl_log.h>

#include "StackedAutoassociatorsNet.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    StackedAutoassociatorsNet,
    "Neural net, trained layer-wise in a greedy fashion using autoassociators",
    "It is highly inspired by the DeepBeliefNet class, and can use the\n"
    "same RBMLayer and RBMConnection components.\n"
    );

StackedAutoassociatorsNet::StackedAutoassociatorsNet() :
    greedy_learning_rate( 0. ),
    greedy_decrease_ct( 0. ),
    fine_tuning_learning_rate( 0. ),
    fine_tuning_decrease_ct( 0. ),
    l1_neuron_decay( 0. ),
    compute_all_test_costs( false ),
    n_layers( 0 ),
    currently_trained_layer( 0 ),
    final_module_has_learning_rate( false ),
    final_cost_has_learning_rate( false )
{
    // random_gen will be initialized in PLearner::build_()
    random_gen = new PRandom();
    nstages = 0;
}

void StackedAutoassociatorsNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "greedy_learning_rate", 
                  &StackedAutoassociatorsNet::greedy_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the autoassociator "
                  "gradient descent training");

    declareOption(ol, "greedy_decrease_ct", 
                  &StackedAutoassociatorsNet::greedy_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "the autoassociator\n"
                  "gradient descent training. When a hidden layer has finished "
                  "its training,\n"
                  "the learning rate is reset to it's initial value.\n");

    declareOption(ol, "fine_tuning_learning_rate", 
                  &StackedAutoassociatorsNet::fine_tuning_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the fine tuning gradient descent");

    declareOption(ol, "fine_tuning_decrease_ct", 
                  &StackedAutoassociatorsNet::fine_tuning_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "fine tuning\n"
                  "gradient descent.\n");

    declareOption(ol, "l1_neuron_decay", 
                  &StackedAutoassociatorsNet::l1_neuron_decay,
                  OptionBase::buildoption,
                  " L1 penalty weight on the hidden layers, to encourage "
                  "sparsity during\n"
                  "the greedy unsupervised phases.\n");

    declareOption(ol, "training_schedule", 
                  &StackedAutoassociatorsNet::training_schedule,
                  OptionBase::buildoption,
                  "Number of examples to use during each phase of learning:\n"
                  "first the greedy phases, and then the gradient descent.\n"
                  "Unlike for DeepBeliefNet, these numbers should not be\n"
                  "cumulative. They correspond to the number of seen training\n"
                  "examples for each phase.\n"
        );

    declareOption(ol, "layers", &StackedAutoassociatorsNet::layers,
                  OptionBase::buildoption,
                  "The layers of units in the network. The first element\n"
                  "of this vector should be the input layer and the\n"
                  "subsequent elements should be the hidden layers. The\n"
                  "should not be included in this layer.\n");

    declareOption(ol, "connections", &StackedAutoassociatorsNet::connections,
                  OptionBase::buildoption,
                  "The weights of the connections between the layers");

    declareOption(ol, "reconstruction_connections", 
                  &StackedAutoassociatorsNet::reconstruction_connections,
                  OptionBase::buildoption,
                  "The weights of the reconstruction connections between the "
                  "layers");

    declareOption(ol, "final_module", &StackedAutoassociatorsNet::final_module,
                  OptionBase::buildoption,
                  "Module that takes as input the output of the last layer\n"
                  "(layers[n_layers-1), and feeds its output to final_cost\n"
                  "which defines the fine-tuning criteria.\n"
                 );

    declareOption(ol, "final_cost", &StackedAutoassociatorsNet::final_cost,
                  OptionBase::buildoption,
                  "The cost function to be applied on top of the neural network\n"
                  "(i.e. at the output of final_module). Its gradients will be \n"
                  "backpropagated to final_module and then backpropagated to\n"
                  "the layers.\n"
                  );

    declareOption(ol, "partial_costs", &StackedAutoassociatorsNet::partial_costs,
                  OptionBase::buildoption,
                  "Corresponding additional supervised cost function to be "
                  "applied on \n"
                  "top of each hidden layer during the autoassociator "
                  "training stages. \n"
                  "The gradient for these costs are not backpropagated to "
                  "previous layers.\n"
        );

    declareOption(ol, "partial_costs_weights", 
                  &StackedAutoassociatorsNet::partial_costs_weights,
                  OptionBase::buildoption,
                  "Relative weights of the partial costs. If not defined,\n"
                  "weights of 1 will be assumed for all partial costs.\n"
        );

    declareOption(ol, "compute_all_test_costs", 
                  &StackedAutoassociatorsNet::compute_all_test_costs,
                  OptionBase::buildoption,
                  "Indication that, at test time, all costs for all layers \n"
                  "(up to the currently trained layer) should be computed.\n"
        );

    declareOption(ol, "greedy_stages", 
                  &StackedAutoassociatorsNet::greedy_stages,
                  OptionBase::learntoption,
                  "Number of training samples seen in the different greedy "
                  "phases.\n"
        );

    declareOption(ol, "n_layers", &StackedAutoassociatorsNet::n_layers,
                  OptionBase::learntoption,
                  "Number of layers");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void StackedAutoassociatorsNet::build_()
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

    if(train_set)
    {
        // Initialize some learnt variables
        n_layers = layers.length();
        
        if( weightsize_ > 0 )
            PLERROR("StackedAutoassociatorsNet::build_() - \n"
                    "usage of weighted samples (weight size > 0) is not\n"
                    "implemented yet.\n");

        if( training_schedule.length() != n_layers-1 )        
            PLERROR("StackedAutoassociatorsNet::build_() - \n"
                    "training_schedule should have %d elements.\n",
                    n_layers-1);
        
        if( partial_costs && partial_costs.length() != n_layers-1 )
            PLERROR("StackedAutoassociatorsNet::build_() - \n"
                    "partial_costs should have %d elements.\n",
                    n_layers-1);

        if( partial_costs && partial_costs_weights &&
            partial_costs_weights.length() != n_layers-1 )
            PLERROR("StackedAutoassociatorsNet::build_() - \n"
                    "partial_costs_weights should have %d elements.\n",
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
        build_costs();
    }
}

void StackedAutoassociatorsNet::build_layers_and_connections()
{
    MODULE_LOG << "build_layers_and_connections() called" << endl;

    if( connections.length() != n_layers-1 )
        PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                "there should be %d connections.\n",
                n_layers-1);

    if(layers[0]->size != inputsize_)
        PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                "layers[0] should have a size of %d.\n",
                inputsize_);
    
    activations.resize( n_layers );
    expectations.resize( n_layers );
    activation_gradients.resize( n_layers );
    expectation_gradients.resize( n_layers );

    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        if( layers[i]->size != connections[i]->down_size )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() "
                    "- \n"
                    "connections[%i] should have a down_size of %d.\n",
                    i, layers[i]->size);

        if( connections[i]->up_size != layers[i+1]->size )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() "
                    "- \n"
                    "connections[%i] should have a up_size of %d.\n",
                    i, layers[i+1]->size);

        if( layers[i+1]->size != reconstruction_connections[i]->down_size )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() "
                    "- \n"
                    "recontruction_connections[%i] should have a down_size of "
                    "%d.\n",
                    i, layers[i+1]->size);

        if( reconstruction_connections[i]->up_size != layers[i]->size )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() "
                    "- \n"
                    "recontruction_connections[%i] should have a up_size of "
                    "%d.\n",
                    i, layers[i]->size);

        if( !(layers[i]->random_gen) )
        {
            layers[i]->random_gen = random_gen;
            layers[i]->forget();
        }

        if( !(connections[i]->random_gen) )
        {
            connections[i]->random_gen = random_gen;
            connections[i]->forget();
        }

        if( !(reconstruction_connections[i]->random_gen) )
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
    activations[n_layers-1].resize( layers[n_layers-1]->size );
    expectations[n_layers-1].resize( layers[n_layers-1]->size );
    activation_gradients[n_layers-1].resize( layers[n_layers-1]->size );
    expectation_gradients[n_layers-1].resize( layers[n_layers-1]->size );
}

void StackedAutoassociatorsNet::build_costs()
{
    MODULE_LOG << "build_final_cost() called" << endl;

    final_cost_gradient.resize( final_cost->input_size );
    final_cost->setLearningRate( fine_tuning_learning_rate );

    if( !(final_cost->random_gen) )
    {
        final_cost->random_gen = random_gen;
        final_cost->forget();
    }


    if( !final_module )
        PLERROR("StackedAutoassociatorsNet::build_costs() - \n"
                "final_module should be provided.\n");
    
    if( layers[n_layers-1]->size != final_module->input_size )
        PLERROR("StackedAutoassociatorsNet::build_costs() - \n"
                "final_module should have an input_size of %d.\n", 
                layers[n_layers-1]->size);
    
    if( final_module->output_size != final_cost->input_size )
        PLERROR("StackedAutoassociatorsNet::build_costs() - \n"
                "final_module should have an output_size of %d.\n", 
                final_module->input_size);

    final_module->setLearningRate( fine_tuning_learning_rate );

    if( !(final_module->random_gen) )
    {
        final_module->random_gen = random_gen;
        final_module->forget();
    }


    if(targetsize_ != 1)
        PLERROR("StackedAutoassociatorsNet::build_costs() - \n"
                "target size of %d is not supported.\n", targetsize_);
    
    if(partial_costs)
    {
        partial_costs_positions.resize(partial_costs.length());
        partial_costs_positions.clear();
        for(int i=0; i<partial_costs.length(); i++)
        {
            if(!partial_costs[i])
                PLERROR("StackedAutoassociatorsNet::build_final_cost() - \n"
                        "partial_costs[%i] should be provided.\n",i);
            if( layers[i+1]->size != partial_costs[i]->input_size )
                PLERROR("StackedAutoassociatorsNet::build_costs() - \n"
                        "partial_costs[%i] should have an input_size of %d.\n", 
                        i,layers[i+1]->size);
            if(i==0)
                partial_costs_positions[i] = n_layers-1;
            else
                partial_costs_positions[i] = partial_costs_positions[i-1]
                    + partial_costs[i-1]->name().length();

            if( !(partial_costs[i]->random_gen) )
            {
                partial_costs[i]->random_gen = random_gen;
                partial_costs[i]->forget();
            }
        }
    }
}

// ### Nothing to add here, simply calls build_
void StackedAutoassociatorsNet::build()
{
    inherited::build();
    build_();
}


void StackedAutoassociatorsNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(, copies);

    deepCopyField(training_schedule, copies);
    deepCopyField(layers, copies);
    deepCopyField(connections, copies);
    deepCopyField(reconstruction_connections, copies);
    deepCopyField(final_module, copies);
    deepCopyField(final_cost, copies);
    deepCopyField(partial_costs, copies);
    deepCopyField(partial_costs_weights, copies);
    deepCopyField(activations, copies);
    deepCopyField(expectations, copies);
    deepCopyField(activation_gradients, copies);
    deepCopyField(expectation_gradients, copies);
    deepCopyField(reconstruction_activations, copies);
    deepCopyField(reconstruction_expectations, copies);
    deepCopyField(reconstruction_activation_gradients, copies);
    deepCopyField(reconstruction_expectation_gradients, copies);
    deepCopyField(partial_costs_positions, copies);
    deepCopyField(partial_cost_value, copies);
    deepCopyField(final_cost_input, copies);
    deepCopyField(final_cost_value, copies);
    deepCopyField(final_cost_gradient, copies);
    deepCopyField(greedy_stages, copies);
}


int StackedAutoassociatorsNet::outputsize() const
{
    if(currently_trained_layer < n_layers)
        return layers[currently_trained_layer]->size;
    return final_module->output_size;
}

void StackedAutoassociatorsNet::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!)
    /*!
      A typical forget() method should do the following:
      - call inherited::forget() to initialize its random number generator
        with the 'seed' option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
    inherited::forget();

    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        connections[i]->forget();
        reconstruction_connections[i]->forget();
    }
    
    final_module->forget();
    final_cost->forget();

    for( int i=0 ; i<partial_costs.length() ; i++ )
        if( partial_costs[i] )
            partial_costs[i]->forget();

    stage = 0;
    greedy_stages.clear();
}

void StackedAutoassociatorsNet::train()
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

    real lr = 0;
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

        if( report_progress && *this_stage < end_stage )
            pb = new ProgressBar( "Training layer "+tostring(i)
                                  +" of "+classname(),
                                  end_stage - init_stage );

        train_costs.fill(MISSING_VALUE);
        lr = greedy_learning_rate;
        layers[i]->setLearningRate( lr );
        connections[i]->setLearningRate( lr );
        reconstruction_connections[i]->setLearningRate( lr );
        layers[i+1]->setLearningRate( lr );

        reconstruction_activations.resize(layers[i+1]->size);
        reconstruction_expectations.resize(layers[i+1]->size);
        reconstruction_activation_gradients.resize(layers[i+1]->size);
        reconstruction_expectation_gradients.resize(layers[i+1]->size);

        for( ; *this_stage<end_stage ; (*this_stage)++ )
        {
            if( !fast_exact_is_equal( greedy_decrease_ct , 0 ) )
            {
                lr = greedy_learning_rate/(1 + greedy_decrease_ct 
                                           * (*this_stage)); 
                layers[i]->setLearningRate( lr );
                connections[i]->setLearningRate( lr );
                reconstruction_connections[i]->setLearningRate( lr );
                layers[i+1]->setLearningRate( lr );                
            }

            sample = *this_stage % nsamples;
            train_set->getExample(sample, input, target, weight);
            greedyStep( input, target, i, train_costs );
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
        for( ; stage<nstages ; stage++ )
        {
            sample = stage % nsamples;
            if( !fast_exact_is_equal( fine_tuning_decrease_ct, 0. ) )
                setLearningRate( fine_tuning_learning_rate
                                 / (1. + fine_tuning_decrease_ct * stage ) );

            train_set->getExample( sample, input, target, weight );
            fineTuningStep( input, target, train_costs );
            train_stats->update( train_costs );

            if( pb )
                pb->update( stage - init_stage + 1 );
        }
    }
    
    train_stats->finalize();
        
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

void StackedAutoassociatorsNet::greedyStep( const Vec& input, const Vec& target, int index, Vec train_costs )
{
    PLASSERT( index < n_layers );

    expectations[0] << input;
    for( int i=0 ; i<index + 1; i++ )
    {
        connections[i]->fprop( expectations[i], activations[i+1] );
        layers[i+1]->fprop(activations[i+1],expectations[i+1]);
    }

    if( partial_costs && partial_costs[ index ] )
    {
        partial_costs[ index ]->fprop( expectations[ index + 1],
                                       target, partial_cost_value );

        // Update partial cost (might contain some weights for example)
        partial_costs[ index ]->bpropUpdate( expectations[ index + 1 ],
                                             target, partial_cost_value,
                                             expectation_gradients[ index + 1 ]
                                             );

        train_costs.subVec(partial_costs_positions[index],
                           partial_cost_value.length()) << partial_cost_value;

        if( !fast_exact_is_equal( partial_costs_weights.length(), 0 ) )
            expectation_gradients[ index + 1 ] *= partial_costs_weights[index];

        // Update hidden layer bias and weights
        layers[ index+1 ]->bpropUpdate( activations[ index + 1 ],
                                        expectations[ index + 1 ],
                                        activation_gradients[ index + 1 ],
                                        expectation_gradients[ index + 1 ] );

        connections[ index ]->bpropUpdate( expectations[ index ],
                                           activations[ index + 1 ],
                                           expectation_gradients[ index ],
                                           activation_gradients[ index + 1 ] );
    }

    reconstruction_connections[ index ]->fprop( expectations[ index + 1],
                                                reconstruction_activations);
    layers[ index ]->fprop( reconstruction_activations,
                            layers[ index ]->expectation);
    
    layers[ index ]->expectation_is_up_to_date = true;
    train_costs[index] = layers[ index ]->fpropNLL(expectations[index]);

    layers[ index ]->bpropNLL(expectations[index], train_costs[index],
                                  reconstruction_activation_gradients);

    layers[ index ]->update(reconstruction_activation_gradients);

    // // This is a bad update! Propagates gradient through sigmoid again!
    // layers[ index ]->bpropUpdate( reconstruction_activations, 
    //                                   layers[ index ]->expectation,
    //                                   reconstruction_activation_gradients,
    //                                   reconstruction_expectation_gradients);

    reconstruction_connections[ index ]->bpropUpdate( 
        expectations[ index + 1], 
        reconstruction_activations, 
        reconstruction_expectation_gradients, //reused
        reconstruction_activation_gradients);

    if(!fast_exact_is_equal(l1_neuron_decay,0))
    {
        // Compute L1 penalty gradient on neurons
        real* hid = expectations[ index + 1 ].data();
        real* grad = reconstruction_expectation_gradients.data();
        int len = expectations[ index + 1 ].length();
        for(int i=0; i<len; i++)
        {
            if(*hid > 0)
                *grad -= l1_neuron_decay;
            else if(*hid < 0)
                *grad += l1_neuron_decay;
            hid++;
            grad++;
        }
    }

    // Update hidden layer bias and weights
    layers[ index+1 ]->bpropUpdate( activations[ index + 1 ],
                                    expectations[ index + 1 ],
                                    reconstruction_activation_gradients, // reused
                                    reconstruction_expectation_gradients);    

    connections[ index ]->bpropUpdate( 
        expectations[ index ],
        activations[ index + 1 ],
        reconstruction_expectation_gradients, //reused
        reconstruction_activation_gradients);

}

void StackedAutoassociatorsNet::fineTuningStep( const Vec& input, const Vec& target,
                                    Vec& train_costs )
{
    // fprop
    expectations[0] << input;
    for( int i=0 ; i<n_layers-1; i++ )
    {
        connections[i]->fprop( expectations[i], activations[i+1] );
        layers[i+1]->fprop(activations[i+1],expectations[i+1]);
    }

    final_module->fprop( expectations[ n_layers-1 ],
                         final_cost_input );
    final_cost->fprop( final_cost_input, target, final_cost_value );

    train_costs.subVec(train_costs.length()-final_cost_value.length(),
                       final_cost_value.length()) <<
        final_cost_value;

    final_cost->bpropUpdate( final_cost_input, target,
                             final_cost_value[0],
                             final_cost_gradient );
    final_module->bpropUpdate( expectations[ n_layers-1 ],
                               final_cost_input,
                               expectation_gradients[ n_layers-1 ],
                               final_cost_gradient );

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

void StackedAutoassociatorsNet::computeOutput(const Vec& input, Vec& output) const
{
    // fprop

    expectations[0] << input;

    for(int i=0 ; i<currently_trained_layer-1 ; i++ )
    {
        connections[i]->fprop( expectations[i], activations[i+1] );
        layers[i+1]->fprop(activations[i+1],expectations[i+1]);
    }

    if( currently_trained_layer<n_layers )
    {
        connections[currently_trained_layer-1]->fprop( 
            expectations[currently_trained_layer-1], 
            activations[currently_trained_layer] );
        layers[currently_trained_layer]->fprop(
            activations[currently_trained_layer],
            output);
    }
    else        
        final_module->fprop( expectations[ currently_trained_layer - 1],
                             output );
}

void StackedAutoassociatorsNet::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    //Assumes that computeOutput has been called

    costs.resize( getTestCostNames().length() );
    costs.fill( MISSING_VALUE );

    if(compute_all_test_costs)
    {
        for(int i=0; i<currently_trained_layer-1; i++)
        {
            reconstruction_connections[ i ]->fprop( expectations[ i+1 ],
                                                    reconstruction_activations);
            layers[ i ]->fprop( reconstruction_activations,
                                    layers[ i ]->expectation);
            
            layers[ i ]->expectation_is_up_to_date = true;
            costs[i] = layers[ i ]->fpropNLL(expectations[ i ]);
            
            if( partial_costs && partial_costs[i])
            {
                partial_costs[ i ]->fprop( expectations[ i + 1],
                                           target, partial_cost_value );
                costs.subVec(partial_costs_positions[i],
                             partial_cost_value.length()) << 
                    partial_cost_value;
            }
        }
    }

    if( currently_trained_layer<n_layers )
    {
        reconstruction_connections[ currently_trained_layer-1 ]->fprop( 
            output,
            reconstruction_activations);
        layers[ currently_trained_layer-1 ]->fprop( 
            reconstruction_activations,
            layers[ currently_trained_layer-1 ]->expectation);
        
        layers[ currently_trained_layer-1 ]->expectation_is_up_to_date = true;
        costs[ currently_trained_layer-1 ] = 
            layers[ currently_trained_layer-1 ]->fpropNLL(
                expectations[ currently_trained_layer-1 ]);

        if( partial_costs && partial_costs[ currently_trained_layer-1 ] )
        {
            partial_costs[ currently_trained_layer-1 ]->fprop( 
                output,
                target, partial_cost_value );
            costs.subVec(partial_costs_positions[currently_trained_layer-1],
                         partial_cost_value.length()) << partial_cost_value;
        }
    }
    else
    {
        final_cost->fprop( output, target, final_cost_value );        
        costs.subVec(costs.length()-final_cost_value.length(),
                     final_cost_value.length()) <<
            final_cost_value;
    }
}

TVec<string> StackedAutoassociatorsNet::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).

    TVec<string> cost_names(0);

    for( int i=0; i<layers.size()-1; i++)
        cost_names.push_back("reconstruction_error_" + tostring(i+1));
    
    for( int i=0 ; i<partial_costs.size() ; i++ )
    {
        TVec<string> cost_names = partial_costs[i]->name();
        for(int j=0; j<cost_names.length(); j++)
            cost_names.push_back("partial_cost_" + tostring(i+1) + "_" + 
                cost_names[j]);
    }

    cost_names.append( final_cost->name() );

    return cost_names;
}

TVec<string> StackedAutoassociatorsNet::getTrainCostNames() const
{
    return getTestCostNames() ;    
}


//#####  Helper functions  ##################################################

void StackedAutoassociatorsNet::setLearningRate( real the_learning_rate )
{
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        layers[i]->setLearningRate( the_learning_rate );
        connections[i]->setLearningRate( the_learning_rate );
    }
    layers[n_layers-1]->setLearningRate( the_learning_rate );

    final_cost->setLearningRate( fine_tuning_learning_rate );
    final_module->setLearningRate( fine_tuning_learning_rate );
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
