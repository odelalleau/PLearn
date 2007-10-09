// -*- C++ -*-

// StackedFocusedAutoassociatorsNet.cc
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

/*! \file StackedFocusedAutoassociatorsNet.cc */


#define PL_LOG_MODULE_NAME "StackedFocusedAutoassociatorsNet"
#include <plearn/io/pl_log.h>

#include "StackedFocusedAutoassociatorsNet.h"
#include <plearn/vmat/VMat_computeNearestNeighbors.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMMixedConnection.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    StackedFocusedAutoassociatorsNet,
    "Neural net, trained layer-wise in a greedy but focused fashion using autoassociators/RBMs and a supervised non-parametric gradient.",
    "It is highly inspired by the StackedFocusedAutoassociators class,\n"
    "and can use use the same RBMLayer and RBMConnection components.\n"
    );

StackedFocusedAutoassociatorsNet::StackedFocusedAutoassociatorsNet() :
    cd_learning_rate( 0. ),
    cd_decrease_ct( 0. ),
    greedy_learning_rate( 0. ),
    greedy_decrease_ct( 0. ),
    supervised_greedy_learning_rate( 0. ),
    supervised_greedy_decrease_ct( 0. ),
    fine_tuning_learning_rate( 0. ),
    fine_tuning_decrease_ct( 0. ),
    k_neighbors( 1 ),
    n_classes( -1 ),
    n_layers( 0 ),
    train_set_representations_up_to_date(false),
    currently_trained_layer( 0 )
{
    // random_gen will be initialized in PLearner::build_()
    random_gen = new PRandom();
    nstages = 0;
}

void StackedFocusedAutoassociatorsNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "cd_learning_rate", 
                  &StackedFocusedAutoassociatorsNet::cd_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the RBM "
                  "contrastive divergence training");

    declareOption(ol, "cd_decrease_ct", 
                  &StackedFocusedAutoassociatorsNet::cd_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "the RBMs contrastive\n"
                  "divergence training. When a hidden layer has finished "
                  "its training,\n"
                  "the learning rate is reset to it's initial value.\n");

    declareOption(ol, "greedy_learning_rate", 
                  &StackedFocusedAutoassociatorsNet::greedy_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the autoassociator "
                  "gradient descent training");

    declareOption(ol, "greedy_decrease_ct", 
                  &StackedFocusedAutoassociatorsNet::greedy_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "the autoassociator\n"
                  "gradient descent training. When a hidden layer has finished "
                  "its training,\n"
                  "the learning rate is reset to it's initial value.\n");

    declareOption(ol, "supervised_greedy_learning_rate", 
                  &StackedFocusedAutoassociatorsNet::supervised_greedy_learning_rate,
                  OptionBase::buildoption,
                  "Supervised, non-parametric, greedy learning rate");

    declareOption(ol, "supervised_greedy_decrease_ct", 
                  &StackedFocusedAutoassociatorsNet::supervised_greedy_decrease_ct,
                  OptionBase::buildoption,
                  "Supervised, non-parametric, greedy decrease constant");

    declareOption(ol, "fine_tuning_learning_rate", 
                  &StackedFocusedAutoassociatorsNet::fine_tuning_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the fine tuning gradient descent");

    declareOption(ol, "fine_tuning_decrease_ct", 
                  &StackedFocusedAutoassociatorsNet::fine_tuning_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "fine tuning\n"
                  "gradient descent.\n");

    declareOption(ol, "training_schedule", 
                  &StackedFocusedAutoassociatorsNet::training_schedule,
                  OptionBase::buildoption,
                  "Number of examples to use during each phase of greedy pre-training.\n"
                  "The number of fine-tunig steps is defined by nstages.\n"
        );

    declareOption(ol, "layers", &StackedFocusedAutoassociatorsNet::layers,
                  OptionBase::buildoption,
                  "The layers of units in the network. The first element\n"
                  "of this vector should be the input layer and the\n"
                  "subsequent elements should be the hidden layers. The\n"
                  "output layer should not be included in layers.\n");

    declareOption(ol, "connections", &StackedFocusedAutoassociatorsNet::connections,
                  OptionBase::buildoption,
                  "The weights of the connections between the layers");

    declareOption(ol, "reconstruction_connections", 
                  &StackedFocusedAutoassociatorsNet::reconstruction_connections,
                  OptionBase::buildoption,
                  "The reconstruction weights of the autoassociators");

    declareOption(ol, "unsupervised_layers", 
                  &StackedFocusedAutoassociatorsNet::unsupervised_layers,
                  OptionBase::buildoption,
                  "Additional units for greedy unsupervised learning");

    declareOption(ol, "unsupervised_connections", 
                  &StackedFocusedAutoassociatorsNet::unsupervised_connections,
                  OptionBase::buildoption,
                  "Additional connections for greedy unsupervised learning");

    declareOption(ol, "k_neighbors", 
                  &StackedFocusedAutoassociatorsNet::k_neighbors,
                  OptionBase::buildoption,
                  "Number of good nearest neighbors to attract and bad nearest "
                  "neighbors to repel.");

    declareOption(ol, "n_classes", 
                  &StackedFocusedAutoassociatorsNet::n_classes,
                  OptionBase::buildoption,
                  "Number of classes.");

    declareOption(ol, "greedy_stages", 
                  &StackedFocusedAutoassociatorsNet::greedy_stages,
                  OptionBase::learntoption,
                  "Number of training samples seen in the different greedy "
                  "phases.\n"
        );

    declareOption(ol, "n_layers", &StackedFocusedAutoassociatorsNet::n_layers,
                  OptionBase::learntoption,
                  "Number of layers"
        );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void StackedFocusedAutoassociatorsNet::build_()
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
        
        train_set_representations_up_to_date = false;

        if( n_classes <= 0 )
            PLERROR("StackedFocusedAutoassociatorsNet::build_() - \n"
                    "n_classes should be > 0.\n");
        test_votes.resize(n_classes);

        if( k_neighbors <= 0 )
            PLERROR("StackedFocusedAutoassociatorsNet::build_() - \n"
                    "k_neighbors should be > 0.\n");
        test_nearest_neighbors_indices.resize(k_neighbors);

        if( weightsize_ > 0 )
            PLERROR("StackedFocusedAutoassociatorsNet::build_() - \n"
                    "usage of weighted samples (weight size > 0) is not\n"
                    "implemented yet.\n");

        if( training_schedule.length() != n_layers-1 )        
            PLERROR("StackedFocusedAutoassociatorsNet::build_() - \n"
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
    }
}

void StackedFocusedAutoassociatorsNet::build_layers_and_connections()
{
    MODULE_LOG << "build_layers_and_connections() called" << endl;

    if( connections.length() != n_layers-1 )
        PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() - \n"
                "there should be %d connections.\n",
                n_layers-1);

    if( reconstruction_connections.length() != n_layers-1 )
        PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() - \n"
                "there should be %d reconstruction connections.\n",
                n_layers-1);

    if(unsupervised_layers.length() != n_layers-2 
       && unsupervised_layers.length() != 0)
        PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() - \n"
                "there should be either 0 of %d unsupervised_layers.\n",
                n_layers-2);
        
    if(unsupervised_connections.length() != n_layers-2 
       && unsupervised_connections.length() != 0)
        PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() - \n"
                "there should be either 0 of %d unsupervised_connections.\n",
                n_layers-2);
        
    if(unsupervised_connections.length() != unsupervised_layers.length())
        PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() - \n"
                "there should be as many unsupervised_connections and "
                "unsupervised_layers.\n");
        

    if(layers[0]->size != inputsize_)
        PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() - \n"
                "layers[0] should have a size of %d.\n",
                inputsize_);
    

    activations.resize( n_layers );
    expectations.resize( n_layers );
    activation_gradients.resize( n_layers );
    expectation_gradients.resize( n_layers );

    greedy_layers.resize(n_layers-1);
    greedy_connections.resize(n_layers-1);
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        if( layers[i]->size != connections[i]->down_size )
            PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() "
                    "- \n"
                    "connections[%i] should have a down_size of %d.\n",
                    i, layers[i]->size);

        if( connections[i]->up_size != layers[i+1]->size )
            PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() "
                    "- \n"
                    "connections[%i] should have a up_size of %d.\n",
                    i, layers[i+1]->size);

        if(unsupervised_layers.length() != 0 &&
           unsupervised_connections.length() != 0 && 
           unsupervised_layers[i] && unsupervised_connections[i])
        {
            if( layers[i]->size != 
                unsupervised_connections[i]->down_size )
                PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() "
                        "- \n"
                        "connections[%i] should have a down_size of %d.\n",
                        i, unsupervised_layers[i]->size);
            
            if( unsupervised_connections[i]->up_size != 
                unsupervised_layers[i]->size )
                PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() "
                        "- \n"
                        "connections[%i] should have a up_size of %d.\n",
                        i, unsupervised_layers[i+1]->size);
            
            if( layers[i+1]->size + unsupervised_layers[i]->size != 
                reconstruction_connections[i]->down_size )
                PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() "
                        "- \n"
                        "recontruction_connections[%i] should have a down_size of "
                        "%d.\n",
                        i, layers[i+1]->size + unsupervised_layers[i]->size);
            
            if( reconstruction_connections[i]->up_size != 
                layers[i]->size )
                PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() "
                        "- \n"
                        "recontruction_connections[%i] should have a up_size of "
                        "%d.\n",
                        i, layers[i]->size);

            if( !(unsupervised_layers[i]->random_gen) )
            {
                unsupervised_layers[i]->random_gen = random_gen;
                unsupervised_layers[i]->forget();
            }
            
            if( !(unsupervised_connections[i]->random_gen) )
            {
                unsupervised_connections[i]->random_gen = random_gen;
                unsupervised_connections[i]->forget();
            }

            PP<RBMMixedLayer> greedy_layer = new RBMMixedLayer();
            greedy_layer->sub_layers.resize(2);
            greedy_layer->sub_layers[0] = layers[i+1];
            greedy_layer->sub_layers[1] = unsupervised_layers[i];
            greedy_layer->build();

            PP<RBMMixedConnection> greedy_connection = new RBMMixedConnection();
            greedy_connection->sub_connections.resize(2,1);
            greedy_connection->sub_connections(1,0) = connections[i];
            greedy_connection->sub_connections(2,0) = unsupervised_connections[i];
            greedy_connection->build();
            
            greedy_layers[i] = greedy_layer;
            greedy_connections[i] = greedy_connection;
        }
        else
        {
            if( layers[i+1]->size != reconstruction_connections[i]->down_size )
                PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() "
                        "- \n"
                        "recontruction_connections[%i] should have a down_size of "
                        "%d.\n",
                        i, layers[i+1]->size);
            
            if( reconstruction_connections[i]->up_size != layers[i]->size )
                PLERROR("StackedFocusedAutoassociatorsNet::build_layers_and_connections() "
                        "- \n"
                        "recontruction_connections[%i] should have a up_size of "
                        "%d.\n",
                        i, layers[i]->size);
 
            greedy_layers[i] = layers[i+1];
            greedy_connections[i] = connections[i];
        }

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

// ### Nothing to add here, simply calls build_
void StackedFocusedAutoassociatorsNet::build()
{
    inherited::build();
    build_();
}


void StackedFocusedAutoassociatorsNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(, copies);

    // Public options
    deepCopyField(training_schedule, copies);
    deepCopyField(layers, copies);
    deepCopyField(connections, copies);
    deepCopyField(reconstruction_connections, copies);
    deepCopyField(unsupervised_layers, copies);
    deepCopyField(unsupervised_connections, copies);

    // Protected options
    deepCopyField(activations, copies);
    deepCopyField(expectations, copies);
    deepCopyField(activation_gradients, copies);
    deepCopyField(expectation_gradients, copies);
    deepCopyField(greedy_activation, copies);
    deepCopyField(greedy_expectation, copies);
    deepCopyField(greedy_activation_gradient, copies);
    deepCopyField(greedy_expectation_gradient, copies);
    deepCopyField(reconstruction_activations, copies);
    deepCopyField(reconstruction_activation_gradients, copies);
    deepCopyField(reconstruction_expectation_gradients, copies);
    deepCopyField(greedy_layers, copies);
    deepCopyField(greedy_connections, copies);
    deepCopyField(similar_example_representation, copies);
    deepCopyField(dissimilar_example_representation, copies);
    deepCopyField(input_representation, copies);
    deepCopyField(previous_input_representation, copies);
    deepCopyField(dissimilar_gradient_contribution, copies);
    deepCopyField(pos_down_val, copies);
    deepCopyField(pos_up_val, copies);
    deepCopyField(neg_down_val, copies);
    deepCopyField(neg_up_val, copies);
    deepCopyField(class_datasets, copies);
    deepCopyField(other_classes_proportions, copies);
    deepCopyField(nearest_neighbors_indices, copies);
    deepCopyField(test_nearest_neighbors_indices, copies);
    deepCopyField(test_votes, copies);
    deepCopyField(train_set_representations, copies);
    deepCopyField(train_set_representations_vmat, copies);
    deepCopyField(train_set_targets, copies);
    deepCopyField(greedy_stages, copies);
}


int StackedFocusedAutoassociatorsNet::outputsize() const
{
    //if(currently_trained_layer < n_layers)
    //    return layers[currently_trained_layer]->size;
    //return layers[n_layers-1]->size;
    return n_classes;
}

void StackedFocusedAutoassociatorsNet::forget()
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

    train_set_representations_up_to_date = false;

    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        connections[i]->forget();
        reconstruction_connections[i]->forget();
    }
    
    stage = 0;
    greedy_stages.clear();
}

void StackedFocusedAutoassociatorsNet::train()
{
    MODULE_LOG << "train() called " << endl;
    MODULE_LOG << "  training_schedule = " << training_schedule << endl;

    Vec input( inputsize() );
    Vec similar_example( inputsize() );
    Vec dissimilar_example( inputsize() );
    Vec target( targetsize() );
    Vec target2( targetsize() );
    real weight; // unused
    real weight2; // unused

    Vec similar_example_index(1);

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

        if( report_progress && *this_stage < end_stage )
            pb = new ProgressBar( "Training layer "+tostring(i)
                                  +" of "+classname(),
                                  end_stage - init_stage );

        train_costs.fill(MISSING_VALUE);
        reconstruction_activations.resize(layers[i]->size);
        reconstruction_activation_gradients.resize(layers[i]->size);
        reconstruction_expectation_gradients.resize(layers[i]->size);

        similar_example_representation.resize(layers[i+1]->size);
        dissimilar_example_representation.resize(layers[i+1]->size);
        dissimilar_gradient_contribution.resize(layers[i+1]->size);
        input_representation.resize(layers[i+1]->size);

        greedy_activation.resize(greedy_layers[i]->size);
        greedy_expectation.resize(greedy_layers[i]->size);
        greedy_activation_gradient.resize(greedy_layers[i]->size);
        greedy_expectation_gradient.resize(greedy_layers[i]->size);

        pos_down_val.resize(layers[i]->size);
        pos_up_val.resize(greedy_layers[i]->size);
        neg_down_val.resize(layers[i]->size);
        neg_up_val.resize(greedy_layers[i]->size);

        for( ; *this_stage<end_stage ; (*this_stage)++ )
        {
            
            sample = *this_stage % nsamples;
            train_set->getExample(sample, input, target, weight);
            // Find similar example

            int sim_index = random_gen->uniform_multinomial_sample(k_neighbors);
            train_set->getExample(nearest_neighbors_indices(sample,sim_index),
                                  similar_example, target2, weight2);

            if(round(target[0]) != round(target2[0]))
                PLERROR("StackedFocusedAutoassociatorsNet::train(): similar"
                    " example is not from same class!");

            // Find dissimilar example

            int dissim_class_index = random_gen->multinomial_sample(
                other_classes_proportions(round(target[0])));

            int dissim_index = random_gen->uniform_multinomial_sample(
                class_datasets[dissim_class_index]->length());

            class_datasets[dissim_class_index]->getExample(dissim_index,
                                  dissimilar_example, target2, weight2);

            if(round(target[0]) == round(target2[0]))
                PLERROR("StackedFocusedAutoassociatorsNet::train(): dissimilar"
                    " example is from same class!");

            greedyStep( input, target, i, train_costs, *this_stage,
                        similar_example, dissimilar_example);
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

        similar_example_representation.resize(
            layers[n_layers-1]->size);
        dissimilar_example_representation.resize(
            layers[n_layers-1]->size);
        dissimilar_gradient_contribution.resize(
            layers[n_layers-1]->size);
        similar_example.resize(inputsize());
        dissimilar_example.resize(inputsize());

        for( ; stage<nstages ; stage++ )
        {
            sample = stage % nsamples;
            if( !fast_exact_is_equal( fine_tuning_decrease_ct, 0. ) )
                setLearningRate( fine_tuning_learning_rate
                                 / (1. + fine_tuning_decrease_ct * stage ) );

            train_set->getExample( sample, input, target, weight );

            // Find similar example

            int sim_index = random_gen->uniform_multinomial_sample(k_neighbors);
            train_set->getExample(nearest_neighbors_indices(sample,sim_index),
                                  similar_example, target2, weight2);

            if(round(target[0]) != round(target2[0]))
                PLERROR("StackedFocusedAutoassociatorsNet::train(): similar"
                    " example is not from same class!");

            // Find dissimilar example

            int dissim_class_index = random_gen->multinomial_sample(
                other_classes_proportions(round(target[0])));

            int dissim_index = random_gen->uniform_multinomial_sample(
                class_datasets[dissim_class_index]->length());

            class_datasets[dissim_class_index]->getExample(dissim_index,
                                  dissimilar_example, target2, weight2);

            if(round(target[0]) == round(target2[0]))
                PLERROR("StackedFocusedAutoassociatorsNet::train(): dissimilar"
                    " example is from same class!");

            fineTuningStep( input, target, train_costs, 
                            similar_example, dissimilar_example);
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

void StackedFocusedAutoassociatorsNet::greedyStep( 
    const Vec& input, const Vec& target, int index, 
    Vec train_costs, int this_stage, Vec similar_example, Vec dissimilar_example )
{
    PLASSERT( index < n_layers );
    real lr;
    train_set_representations_up_to_date = false;

    // Get similar example representation
    
    computeRepresentation(similar_example, similar_example_representation, 
                          index+1);

    // Get dissimilar example representation

    computeRepresentation(dissimilar_example, dissimilar_example_representation, 
                          index+1);

    // Get example representation

    computeRepresentation(input, previous_input_representation, 
                          index);
    greedy_connections[index]->fprop(previous_input_representation,
                                     greedy_activation);
    greedy_layers[index]->fprop(greedy_activation,
                                greedy_expectation);
    input_representation << greedy_expectation.subVec(0,layers[index+1]->size);

    // Autoassociator learning

    if( !fast_exact_is_equal( greedy_learning_rate, 0 ) )
    {
        if( !fast_exact_is_equal( greedy_decrease_ct , 0 ) )
            lr = greedy_learning_rate/(1 + greedy_decrease_ct 
                                       * this_stage); 
        else
            lr = greedy_learning_rate;

        layers[index]->setLearningRate( lr );
        greedy_connections[index]->setLearningRate( lr );
        reconstruction_connections[index]->setLearningRate( lr );
        greedy_layers[index]->setLearningRate( lr );

        reconstruction_connections[ index ]->fprop( greedy_expectation,
                                                    reconstruction_activations);
        layers[ index ]->fprop( reconstruction_activations,
                                layers[ index ]->expectation);
        
        layers[ index ]->activation << reconstruction_activations;
        layers[ index ]->expectation_is_up_to_date = true;
        real rec_err = layers[ index ]->fpropNLL(previous_input_representation);
        train_costs[index] = rec_err;
        
        layers[ index ]->bpropNLL(previous_input_representation, rec_err,
                                  reconstruction_activation_gradients);
    }

    // Compute supervised gradient
    
    // Similar example contribution
    substract(input_representation,similar_example_representation,
              expectation_gradients[index+1]);
    expectation_gradients[index+1] *= 4/layers[index+1]->size;
    
    // Dissimilar example contribution
    real dist = sqrt(powdistance(input_representation,
                                 dissimilar_example_representation,
                                 2));
    
    substract(input_representation,dissimilar_example_representation,
              dissimilar_gradient_contribution);

    dissimilar_gradient_contribution *= -5.54*
        safeexp(-2.77*dist/layers[index+1]->size)/dist;
    
    expectation_gradients[index+1] += dissimilar_gradient_contribution;

    // RBM learning
    if( !fast_exact_is_equal( cd_learning_rate, 0 ) )
    {
        greedy_layers[index]->expectation << greedy_expectation;
        greedy_layers[index]->expectation_is_up_to_date = true;
        greedy_layers[index]->generateSample();
        
        // accumulate positive stats using the expectation
        // we deep-copy because the value will change during negative phase
        pos_down_val = expectations[index];
        pos_up_val = greedy_layers[index]->expectation;
        
        // down propagation, starting from a sample of layers[index+1]
        greedy_connections[index]->setAsUpInput( greedy_layers[index]->sample );
        
        layers[index]->getAllActivations( greedy_connections[index] );
        layers[index]->computeExpectation();
        layers[index]->generateSample();
        
        // negative phase
        greedy_connections[index]->setAsDownInput( layers[index]->sample );
        greedy_layers[index]->getAllActivations( greedy_connections[index] );
        greedy_layers[index]->computeExpectation();
        // accumulate negative stats
        // no need to deep-copy because the values won't change before update
        neg_down_val = layers[index]->sample;
        neg_up_val = greedy_layers[index]->expectation;
    }
    
    // Update hidden layer bias and weights

    if( !fast_exact_is_equal( greedy_learning_rate, 0 ) )
    {
        layers[ index ]->update(reconstruction_activation_gradients);
    
        reconstruction_connections[ index ]->bpropUpdate( 
            greedy_expectation,
            reconstruction_activations, 
            reconstruction_expectation_gradients, 
            reconstruction_activation_gradients);

        greedy_layers[ index ]->bpropUpdate( 
            greedy_activation,
            greedy_expectation,
            // reused
            reconstruction_activation_gradients,
            reconstruction_expectation_gradients);
        
        greedy_connections[ index ]->bpropUpdate( 
            previous_input_representation,
            greedy_activation,
            reconstruction_expectation_gradients, //reused
            reconstruction_activation_gradients);
    }
     

    if( !fast_exact_is_equal( supervised_greedy_decrease_ct , 0 ) )
        lr = supervised_greedy_learning_rate/(1 + supervised_greedy_decrease_ct 
                               * this_stage); 
    else
        lr = supervised_greedy_learning_rate;
    
    layers[index]->setLearningRate( lr );
    connections[index]->setLearningRate( lr );
    layers[index+1]->setLearningRate( lr );
    
    layers[ index+1 ]->bpropUpdate( 
        greedy_activation.subVec(0,layers[index+1]->size),
        greedy_expectation.subVec(0,layers[index+1]->size),
        activation_gradients[index+1], 
        expectation_gradients[index+1]);
    
    connections[ index ]->bpropUpdate( 
        previous_input_representation,
        greedy_activation.subVec(0,layers[index+1]->size),
        expectation_gradients[index],
        activation_gradients[index+1]);

    // RBM updates

    if( !fast_exact_is_equal( cd_learning_rate, 0 ) )
    {
        if( !fast_exact_is_equal( cd_decrease_ct , 0 ) )
            lr = cd_learning_rate/(1 + cd_decrease_ct 
                                       * this_stage); 
        else
            lr = cd_learning_rate;

        layers[index]->setLearningRate( lr );
        greedy_connections[index]->setLearningRate( lr );
        greedy_layers[index]->setLearningRate( lr );

        layers[index]->update( pos_down_val, neg_down_val );
        greedy_connections[index]->update( pos_down_val, pos_up_val,
                                    neg_down_val, neg_up_val );
        greedy_layers[index]->update( pos_up_val, neg_up_val );
    }
}

void StackedFocusedAutoassociatorsNet::fineTuningStep( 
    const Vec& input, const Vec& target,
    Vec& train_costs, Vec similar_example, Vec dissimilar_example )
{
    train_set_representations_up_to_date = false;

    // Get similar example representation
    
    computeRepresentation(similar_example, similar_example_representation, 
                          n_layers-1);

    // Get dissimilar example representation

    computeRepresentation(dissimilar_example, dissimilar_example_representation, 
                          n_layers-1);

    // Get example representation

    computeRepresentation(input, previous_input_representation, 
                          n_layers-1);

    // Compute supervised gradient

    // Similar example contribution
    substract(input_representation,similar_example_representation,
              expectation_gradients[n_layers-1]);
    expectation_gradients[n_layers-1] *= 4/layers[n_layers-1]->size;
    
    // Dissimilar example contribution
    real dist = sqrt(powdistance(input_representation,
                                 dissimilar_example_representation,
                                 2));
    
    substract(input_representation,dissimilar_example_representation,
              dissimilar_gradient_contribution);

    dissimilar_gradient_contribution *= -5.54*
        safeexp(-2.77*dist/layers[n_layers-1]->size)/dist;
    
    expectation_gradients[n_layers-1] += dissimilar_gradient_contribution;


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

void StackedFocusedAutoassociatorsNet::computeRepresentation(const Vec& input,
                                                             Vec& representation,
                                                             int layer) const
{
    if(layer == 0)
    {
        representation.resize(input.length());
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

void StackedFocusedAutoassociatorsNet::computeOutput(const Vec& input, Vec& output) const
{
    updateTrainSetRepresentations();

    computeRepresentation(input,input_representation, 
                          max(currently_trained_layer,n_layers-1));

    computeNearestNeighbors(train_set_representations_vmat,input_representation,
                            test_nearest_neighbors_indices);

    test_votes.clear();
    for(int i=0; i<test_nearest_neighbors_indices.length(); i++)
        test_votes[train_set_targets[test_nearest_neighbors_indices[i]]]++;

    output[0] = argmax(test_votes);

}

void StackedFocusedAutoassociatorsNet::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{

    //Assumes that computeOutput has been called

    costs.resize( getTestCostNames().length() );
    costs.fill( MISSING_VALUE );

    if( currently_trained_layer<n_layers )
    {
        greedy_connections[currently_trained_layer-1]->fprop(
            expectations[currently_trained_layer-1],
            greedy_activation);
        
        greedy_layers[currently_trained_layer-1]->fprop(greedy_activation,
                                    greedy_expectation);
        
        reconstruction_connections[ currently_trained_layer-1 ]->fprop( 
            greedy_expectation,
            reconstruction_activations);
        layers[ currently_trained_layer-1 ]->fprop( 
            reconstruction_activations,
            layers[ currently_trained_layer-1 ]->expectation);
        
        layers[ currently_trained_layer-1 ]->activation << 
            reconstruction_activations;
        layers[ currently_trained_layer-1 ]->expectation_is_up_to_date = true;
        costs[ currently_trained_layer-1 ]  = 
            layers[ currently_trained_layer-1 ]->fpropNLL(
                expectations[currently_trained_layer-1]);
    }

    if( round(output[0]) == round(target[0]) )
        costs[n_layers-1] = 0;
    else
        costs[n_layers-1] = 1;
}

//////////
// test //
//////////
void StackedFocusedAutoassociatorsNet::updateTrainSetRepresentations() const
{
    if(!train_set_representations_up_to_date)
    {
        // Precompute training set examples' representation
        int l = max(currently_trained_layer,n_layers-1);
        Vec input( inputsize() );
        Vec target( targetsize() );
        Vec train_set_representation;
        real weight;

        train_set_representations.resize(train_set->length(), layers[l]->size);
        train_set_targets.resize(train_set->length());
        
        for(int i=0; i<train_set->length(); i++)
        {
            train_set->getExample(i,input,target,weight);
            computeRepresentation(input,train_set_representation,l);
            train_set_representations(i) << train_set_representation;
            train_set_targets[i] = round(target[0]);
        }
        train_set_representations_vmat = VMat(train_set_representations);

        train_set_representations_up_to_date = true;
    }
}

TVec<string> StackedFocusedAutoassociatorsNet::getTestCostNames() const
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

TVec<string> StackedFocusedAutoassociatorsNet::getTrainCostNames() const
{
    return getTestCostNames() ;    
}

void StackedFocusedAutoassociatorsNet::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set,call_forget);
    
    train_set_representations_up_to_date = false;

    Vec input( inputsize() );
    Vec target( targetsize() );
    real weight; // unused

    // Separate classes
    for(int k=0; k<n_classes; k++)
    {
        class_datasets[k] = new ClassSubsetVMatrix();
        class_datasets[k]->classes.resize(1);
        class_datasets[k]->classes[0] = k;
        class_datasets[k]->source = training_set;
        class_datasets[k]->build();
    }

    // Find other classes proportions
    other_classes_proportions.fill(0);
    for(int k=0; k<n_classes; k++)
    {
        real sum = 0;
        for(int j=0; j<n_classes; j++)
        {
            if(j==k) continue;
            other_classes_proportions(k,j) = class_datasets[j]->length();
            sum += class_datasets[j]->length();
        }
        other_classes_proportions(k) /= sum;
    }

    // Find training nearest neighbors
    input.resize(training_set->inputsize());
    target.resize(training_set->targetsize());
    nearest_neighbors_indices.resize(training_set->length(), k_neighbors);
    TVec<int> nearest_neighbors_indices_row;
    for(int k=0; k<n_classes; k++)
    {
        for(int i=0; i<class_datasets[k]->length(); i++)
        {
            class_datasets[k]->getExample(i,input,target,weight);
            nearest_neighbors_indices_row = nearest_neighbors_indices(
                class_datasets[k]->indices[i]);
            computeNearestNeighbors((VMatrix *)class_datasets[k],input,
                                    nearest_neighbors_indices_row,
                                    i);
        }
    }
}


//#####  Helper functions  ##################################################

void StackedFocusedAutoassociatorsNet::setLearningRate( real the_learning_rate )
{
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        layers[i]->setLearningRate( the_learning_rate );
        connections[i]->setLearningRate( the_learning_rate );
    }
    layers[n_layers-1]->setLearningRate( the_learning_rate );
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
