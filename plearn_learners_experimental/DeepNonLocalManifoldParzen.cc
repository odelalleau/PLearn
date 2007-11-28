// -*- C++ -*-

// DeepNonLocalManifoldParzen.cc
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

// Authors: Hugo Larochelle

/*! \file DeepNonLocalManifoldParzen.cc */


#define PL_LOG_MODULE_NAME "DeepNonLocalManifoldParzen"
#include <plearn/io/pl_log.h>

#include "DeepNonLocalManifoldParzen.h"
#include <plearn/vmat/VMat_computeNearestNeighbors.h>
#include <plearn/vmat/GetInputVMatrix.h>
#include <plearn_learners/online/GradNNetLayerModule.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DeepNonLocalManifoldParzen,
    "Neural net, trained layer-wise to predict the manifold structure of the data.",
    "This information is used in a Manifold Parzen Windows classifier."
    );

DeepNonLocalManifoldParzen::DeepNonLocalManifoldParzen() :
    cd_learning_rate( 0. ),
    cd_decrease_ct( 0. ),
    greedy_learning_rate( 0. ),
    greedy_decrease_ct( 0. ),
    fine_tuning_learning_rate( 0. ),
    fine_tuning_decrease_ct( 0. ),
    k_neighbors( 1 ),
    n_components( 1 ),
    min_sigma_noise( 0 ),
    n_classes( -1 ),
    output_connections_l1_penalty_factor( 0 ),
    output_connections_l2_penalty_factor( 0 ),
    n_layers( 0 ),
    save_manifold_parzen_parameters( false ),
    manifold_parzen_parameters_are_up_to_date( false ),
    currently_trained_layer( 0 )
{
    // random_gen will be initialized in PLearner::build_()
    random_gen = new PRandom();
    nstages = 0;
}

void DeepNonLocalManifoldParzen::declareOptions(OptionList& ol)
{
    declareOption(ol, "cd_learning_rate", 
                  &DeepNonLocalManifoldParzen::cd_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the RBM "
                  "contrastive divergence training");

    declareOption(ol, "cd_decrease_ct", 
                  &DeepNonLocalManifoldParzen::cd_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "the RBMs contrastive\n"
                  "divergence training. When a hidden layer has finished "
                  "its training,\n"
                  "the learning rate is reset to it's initial value.\n");

    declareOption(ol, "greedy_learning_rate", 
                  &DeepNonLocalManifoldParzen::greedy_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the autoassociator "
                  "gradient descent training");

    declareOption(ol, "greedy_decrease_ct", 
                  &DeepNonLocalManifoldParzen::greedy_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "the autoassociator\n"
                  "gradient descent training. When a hidden layer has finished "
                  "its training,\n"
                  "the learning rate is reset to it's initial value.\n");

    declareOption(ol, "fine_tuning_learning_rate", 
                  &DeepNonLocalManifoldParzen::fine_tuning_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the fine tuning gradient descent");

    declareOption(ol, "fine_tuning_decrease_ct", 
                  &DeepNonLocalManifoldParzen::fine_tuning_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "fine tuning\n"
                  "gradient descent.\n");

    declareOption(ol, "training_schedule", 
                  &DeepNonLocalManifoldParzen::training_schedule,
                  OptionBase::buildoption,
                  "Number of examples to use during each phase of greedy pre-training.\n"
                  "The number of fine-tunig steps is defined by nstages.\n"
        );

    declareOption(ol, "layers", &DeepNonLocalManifoldParzen::layers,
                  OptionBase::buildoption,
                  "The layers of units in the network. The first element\n"
                  "of this vector should be the input layer and the\n"
                  "subsequent elements should be the hidden layers. The\n"
                  "output layer should not be included in layers.\n");

    declareOption(ol, "connections", &DeepNonLocalManifoldParzen::connections,
                  OptionBase::buildoption,
                  "The weights of the connections between the layers");

    declareOption(ol, "reconstruction_connections", 
                  &DeepNonLocalManifoldParzen::reconstruction_connections,
                  OptionBase::buildoption,
                  "The reconstruction weights of the autoassociators");

    declareOption(ol, "unsupervised_layers", 
                  &DeepNonLocalManifoldParzen::unsupervised_layers,
                  OptionBase::buildoption,
                  "Additional units for greedy unsupervised learning");

    declareOption(ol, "unsupervised_connections", 
                  &DeepNonLocalManifoldParzen::unsupervised_connections,
                  OptionBase::buildoption,
                  "Additional connections for greedy unsupervised learning");

    declareOption(ol, "k_neighbors", 
                  &DeepNonLocalManifoldParzen::k_neighbors,
                  OptionBase::buildoption,
                  "Number of good nearest neighbors to attract and bad nearest "
                  "neighbors to repel.");

    declareOption(ol, "n_components", 
                  &DeepNonLocalManifoldParzen::n_components,
                  OptionBase::buildoption,
                  "Dimensionality of the manifold");

    declareOption(ol, "n_classes", 
                  &DeepNonLocalManifoldParzen::n_classes,
                  OptionBase::buildoption,
                  "Number of classes.");

    declareOption(ol, "output_connections_l1_penalty_factor", 
                  &DeepNonLocalManifoldParzen::output_connections_l1_penalty_factor,
                  OptionBase::buildoption,
                  "Output weights L1 penalty factor");

    declareOption(ol, "output_connections_l2_penalty_factor", 
                  &DeepNonLocalManifoldParzen::output_connections_l2_penalty_factor,
                  OptionBase::buildoption,
                  "Output weights L2 penalty factor");

    declareOption(ol, "save_manifold_parzen_parameters", 
                  &DeepNonLocalManifoldParzen::save_manifold_parzen_parameters,
                  OptionBase::buildoption,
                  "Indication that the parameters for the manifold parzen\n"
                  "windows estimator should be saved during test, to speed up "
                  "testing.");

    declareOption(ol, "manifold_parzen_parameters_are_up_to_date", 
                  &DeepNonLocalManifoldParzen::manifold_parzen_parameters_are_up_to_date,
                  OptionBase::buildoption,
                  "Indication that the saved manifold parzen parameters are\n"
                  "up to date.");

    declareOption(ol, "greedy_stages", 
                  &DeepNonLocalManifoldParzen::greedy_stages,
                  OptionBase::learntoption,
                  "Number of training samples seen in the different greedy "
                  "phases.\n"
        );

    declareOption(ol, "n_layers", &DeepNonLocalManifoldParzen::n_layers,
                  OptionBase::learntoption,
                  "Number of layers"
        );

    declareOption(ol, "output_connections", 
                  &DeepNonLocalManifoldParzen::output_connections,
                  OptionBase::learntoption,
                  "Output weights"
        );

    declareOption(ol, "train_set", 
                  &DeepNonLocalManifoldParzen::train_set,
                  OptionBase::learntoption,
                  "Training set"
        );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DeepNonLocalManifoldParzen::build_()
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
        
        // Builds some variables using the training set
        setTrainingSet(train_set, false);

        if( n_classes <= 0 )
            PLERROR("DeepNonLocalManifoldParzen::build_() - \n"
                    "n_classes should be > 0.\n");
        test_votes.resize(n_classes);

        if( k_neighbors <= 0 )
            PLERROR("DeepNonLocalManifoldParzen::build_() - \n"
                    "k_neighbors should be > 0.\n");

        if( weightsize_ > 0 )
            PLERROR("DeepNonLocalManifoldParzen::build_() - \n"
                    "usage of weighted samples (weight size > 0) is not\n"
                    "implemented yet.\n");

        if( training_schedule.length() != n_layers-1 )        
            PLERROR("DeepNonLocalManifoldParzen::build_() - \n"
                    "training_schedule should have %d elements.\n",
                    n_layers-1);
        
        if( n_components < 1 || n_components > inputsize_)
            PLERROR("DeepNonLocalManifoldParzen::build_() - \n"
                    "n_components should be > 0 and < or = to inputsize.\n");

        if( min_sigma_noise < 0)
            PLERROR("DeepNonLocalManifoldParzen::build_() - \n"
                    "min_sigma_noise should be > or = to 0.\n")

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

void DeepNonLocalManifoldParzen::build_layers_and_connections()
{
    MODULE_LOG << "build_layers_and_connections() called" << endl;

    if( connections.length() != n_layers-1 )
        PLERROR("DeepNonLocalManifoldParzen::build_layers_and_connections() - \n"
                "there should be %d connections.\n",
                n_layers-1);

    if( !fast_exact_is_equal( greedy_learning_rate, 0 ) 
        && reconstruction_connections.length() != n_layers-1 )
        PLERROR("DeepNonLocalManifoldParzen::build_layers_and_connections() - \n"
                "there should be %d reconstruction connections.\n",
                n_layers-1);
    
    if(  !( reconstruction_connections.length() == 0
            || reconstruction_connections.length() == n_layers-1 ) )
        PLERROR("DeepNonLocalManifoldParzen::build_layers_and_connections() - \n"
                "there should be either 0 or %d reconstruction connections.\n",
                n_layers-1);
        

    if(layers[0]->size != inputsize_)
        PLERROR("DeepNonLocalManifoldParzen::build_layers_and_connections() - \n"
                "layers[0] should have a size of %d.\n",
                inputsize_);

    activations.resize( n_layers );
    expectations.resize( n_layers );
    activation_gradients.resize( n_layers );
    expectation_gradients.resize( n_layers );

    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        if( layers[i]->size != connections[i]->down_size )
            PLERROR("DeepNonLocalManifoldParzen::build_layers_and_connections() "
                    "- \n"
                    "connections[%i] should have a down_size of %d.\n",
                    i, layers[i]->size);

        if( connections[i]->up_size != layers[i+1]->size )
            PLERROR("DeepNonLocalManifoldParzen::build_layers_and_connections() "
                    "- \n"
                    "connections[%i] should have a up_size of %d.\n",
                    i, layers[i+1]->size);

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
    activations[n_layers-1].resize( layers[n_layers-1]->size );
    expectations[n_layers-1].resize( layers[n_layers-1]->size );
    activation_gradients[n_layers-1].resize( layers[n_layers-1]->size );
    expectation_gradients[n_layers-1].resize( layers[n_layers-1]->size );

    int output_size = n_components*inputsize + (predict_mu ? inputsize : 0) + 1;
    all_outputs.resize( output_size );

    if( !output_connections || output_connections->output_size != output_size)
    {
        PP<GradNNetLayerModule> ow = new GradNNetLayerModule;
        ow->input_size = layers[n_layers-1]->size;
        ow->output_size = output_size;
        ow->L1_penalty_factor = output_connections_l1_penalty_factor;
        ow->L2_penalty_factor = output_connections_l2_penalty_factor;
        ow->build();
        output_connections = ow;
    }
}

// ### Nothing to add here, simply calls build_
void DeepNonLocalManifoldParzen::build()
{
    inherited::build();
    build_();
}


void DeepNonLocalManifoldParzen::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(, copies);

    // Public options
    deepCopyField(training_schedule, copies);
    deepCopyField(layers, copies);
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
    deepCopyField(output_connections, copies);
    deepCopyField(input_representation, copies);
    deepCopyField(previous_input_representation, copies);
    deepCopyField(all_outputs, copies);
    deepCopyField(all_outputs_gradient, copies);
    deepCopyField(F, copies);
    deepCopyField(F_copy, copies);
    deepCopyField(mu, copies);
    deepCopyField(pre_sigma_noise, copies);
    deepCopyField(Ut, copies);
    deepCopyField(U, copies);
    deepCopyField(V, copies);
    deepCopyField(z, copies);
    deepCopyField(invSigma_F, copies);
    deepCopyField(invSigma_z, copies);
    deepCopyField(temp_ncomp, copies);
    deepCopyField(diff_neighbor_input, copies);
    deepCopyField(sm_svd, copies);
    deepCopyField(sn, copies);
    deepCopyField(S, copies);
    deepCopyField(uk, copies);
    deepCopyField(fk, copies);
    deepCopyField(uk2, copies);
    deepCopyField(inv_sigma_zj, copies);
    deepCopyField(zj, copies);
    deepCopyField(inv_sigma_fk, copies);
    deepCopyField(diff, copies);
    deepCopyField(pos_down_val, copies);
    deepCopyField(pos_up_val, copies);
    deepCopyField(neg_down_val, copies);
    deepCopyField(neg_up_val, copies);
    deepCopyField(eigenvectors, copies);
    deepCopyField(eigenvalues, copies);
    deepCopyField(sigma_noises, copies);
    deepCopyField(mus, copies);
    deepCopyField(class_datasets, copies);
    deepCopyField(nearest_neighbors_indices, copies);
    deepCopyField(test_votes, copies);
    deepCopyField(greedy_stages, copies);
}


int DeepNonLocalManifoldParzen::outputsize() const
{
    //if(currently_trained_layer < n_layers)
    //    return layers[currently_trained_layer]->size;
    //return layers[n_layers-1]->size;
    return n_classes;
}

void DeepNonLocalManifoldParzen::forget()
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

    manifold_parzen_parameters_are_up_to_date = false;

    for( int i=0 ; i<n_layers-1 ; i++ )
        connections[i]->forget();
    
    for( int i=0 ; i<n_layers ; i++ )
        layers[i]->forget();
    
    for( int i=0; i<reconstruction_connections.length(); i++)
        reconstruction_connections[i]->forget();

    if( output_connections )
        output_connections->forget();

    stage = 0;
    greedy_stages.clear();
}

void DeepNonLocalManifoldParzen::train()
{
    MODULE_LOG << "train() called " << endl;
    MODULE_LOG << "  training_schedule = " << training_schedule << endl;

    Vec input( inputsize() );
    Vec nearest_neighbor( inputsize() );
    Mat nearest_neighbors( k_neighbors, inputsize() );
    Vec target( targetsize() );
    Vec target2( targetsize() );
    real weight; // unused
    real weight2; // unused

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

        pos_down_val.resize(layers[i]->size);
        pos_up_val.resize(greedy_layers[i]->size);
        neg_down_val.resize(layers[i]->size);
        neg_up_val.resize(greedy_layers[i]->size);

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

        if( stage == 0 )
        {
            MODULE_LOG << "Finding the nearest neighbors" << endl;
            // Find training nearest neighbors
            TVec<int> nearest_neighbors_indices_row;
            for(int k=0; k<n_classes; k++)
            {
                for(int i=0; i<class_datasets[k]->length(); i++)
                {
                    class_datasets[k]->getExample(i,input,target,weight);
                    nearest_neighbors_indices_row = nearest_neighbors_indices(
                        class_datasets[k]->indices[i]);
                    computeNearestNeighbors(
                        new GetInputVMatrix((VMatrix *)class_datasets[k]),input,
                        nearest_neighbors_indices_row,
                        i);
                }
            }
        }

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

            // Find nearest neighbors
            for( int k=0; k<k_neighbors; k++ )
            {
                class_datasets[(int)round(target[0])]->getExample(
                    nearest_neighbors_indices(sample,k),
                    nearest_neighbor, target2, weight2);

                if(round(target[0]) != round(target2[0]))
                    PLERROR("DeepNonLocalManifoldParzen::train(): similar"
                            " example is not from same class!");
                nearest_neighbors(k) << nearest_neighbor;
            }


            fineTuningStep( input, target, train_costs, 
                            nearest_neighbors);
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

void DeepNonLocalManifoldParzen::greedyStep( 
    const Vec& input, const Vec& target, int index, 
    Vec train_costs, int this_stage)
{
    PLASSERT( index < n_layers );
    real lr;
    manifold_parzen_parameters_are_up_to_date = false;

    // Get example representation

    computeRepresentation(input, previous_input_representation, 
                          index);
    connections[index]->fprop(previous_input_representation,
                                     activations[index+1]);
    layers[index+1]->fprop(activations[index+1],
                           expectations[index+1]);

    // Autoassociator learning
    if( !fast_exact_is_equal( greedy_learning_rate, 0 ) )
    {
        if( !fast_exact_is_equal( greedy_decrease_ct , 0 ) )
            lr = greedy_learning_rate/(1 + greedy_decrease_ct 
                                       * this_stage); 
        else
            lr = greedy_learning_rate;

        layers[index]->setLearningRate( lr );
        connections[index]->setLearningRate( lr );
        reconstruction_connections[index]->setLearningRate( lr );
        layers[index+1]->setLearningRate( lr );

        reconstruction_connections[ index ]->fprop( expectations[index+1],
                                                    reconstruction_activations);
        layers[ index ]->fprop( reconstruction_activations,
                                layers[ index ]->expectation);
        
        layers[ index ]->activation << reconstruction_activations;
        layers[ index ]->setExpectationByRef(layers[ index ]->expectation);
        real rec_err = layers[ index ]->fpropNLL(previous_input_representation);
        train_costs[index] = rec_err;
        
        layers[ index ]->bpropNLL(previous_input_representation, rec_err,
                                  reconstruction_activation_gradients);
    }

    // RBM learning
    if( !fast_exact_is_equal( cd_learning_rate, 0 ) )
    {
        layers[index+1]->setExpectation( expectations[index+1] );
        layers[index+1]->generateSample();
        
        // accumulate positive stats using the expectation
        // we deep-copy because the value will change during negative phase
        pos_down_val = expectations[index];
        pos_up_val << layers[index+1]->expectation;
        
        // down propagation, starting from a sample of layers[index+1]
        connections[index]->setAsUpInput( layers[index+1]->sample );
        
        layers[index]->getAllActivations( connections[index] );
        layers[index]->computeExpectation();
        layers[index]->generateSample();
        
        // negative phase
        connections[index]->setAsDownInput( layers[index]->sample );
        layers[index+1]->getAllActivations( connections[index+1] );
        layers[index+1]->computeExpectation();
        // accumulate negative stats
        // no need to deep-copy because the values won't change before update
        neg_down_val = layers[index]->sample;
        neg_up_val = layers[index+1]->expectation;
    }
    
    // Update hidden layer bias and weights

    if( !fast_exact_is_equal( greedy_learning_rate, 0 ) )
    {
        layers[ index ]->update(reconstruction_activation_gradients);
    
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
        
        connections[ index ]->bpropUpdate( 
            previous_input_representation,
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

        layers[index]->setLearningRate( lr );
        connections[index]->setLearningRate( lr );
        layers[index+1]->setLearningRate( lr );

        layers[index]->update( pos_down_val, neg_down_val );
        connections[index]->update( pos_down_val, pos_up_val,
                                    neg_down_val, neg_up_val );
        layers[index+1]->update( pos_up_val, neg_up_val );
    }
}

void DeepNonLocalManifoldParzen::computeManifoldParzenParameters( 
    const Vec& input, Mat& F, Vec& mu, 
    Vec& pre_sigma_noise, Mat& U, Vec& sm_svd) const
{
    // Get example representation
    computeRepresentation(input, input_representation, 
                          n_layers-1);

    // Get parameters
    output_connections->fprop( input_representation, all_outputs );

    F.resize(n_components, inputsize());
    mu.resize(inputsize());
    pre_sigma_noise.resize(1);

    F << all_outputs.subVec(0,n_components * inputsize()).toMat(
        n_components, inputsize());
    mu << all_outputs.subVec(n_components * inputsize(),inputsize());
    pre_sigma_noise << all_outputs.subVec( n_components * (inputsize() + 1), 1 );

    F_copy.resize(F.length(),F.width());
    sm_svd.resize(n_components);
    // N.B. this is the SVD of F'
    F_copy << F;
    lapackSVD(F_copy, Ut, S, V,'A',1.5);
    U.resize(n_components,inputsize());
    for (int k=0;k<n_components;k++)
    {
        sm_svd[k] = mypow(S[k],2);
        U(k) << Ut(k);
    }
}


void DeepNonLocalManifoldParzen::fineTuningStep( 
    const Vec& input, const Vec& target,
    Vec& train_costs, Mat nearest_neighbors )
{
    manifold_parzen_parameters_are_up_to_date = false;

    computeManifoldParzenParameters( input, F, mu, pre_sigma_noise, U, sm_svd );

    real sigma_noise = square(pre_sigma_noise[0], 2) + min_sigma_noise;

    real mahal = 0;
    real norm_term = 0;
    real dotp = 0;
    real coef = 0;
    real n = inputsize();
    inv_Sigma_z.resize(inputsize());
    inv_Sigma_z.clear();
    real tr_inv_Sigma = 0;
    train_costs.last() = 0;
    for(int j=0; j<k_neighbors;j++)
    {
        zj = z(j);
        substract(nearest_neighbors(j),input,diff_neighbor_input); 
        substract(diff_neighbor_input,mu,zj); 
      
        mahal = -0.5*pownorm(zj)/sigma_noise;      
        norm_term = - n/2.0 * Log2Pi - 0.5*(n-n_components)*pl_log(sigma_noise);

        inv_sigma_zj = inv_Sigma_z(j);
        inv_sigma_zj << zj; 
        inv_sigma_zj /= sigma_noise;

        if(j==0)
            tr_inv_Sigma = n/sigma_noise;

        for(int k=0; k<n_components; k++)
        { 
            uk = U(k);
            dotp = dot(zj,uk);
            coef = (1.0/(sm_svd[k]+sigma_noise) - 1.0/sigma_noise);
            multiplyAcc(inv_sigma_zj,uk,dotp*coef);
            mahal -= square(dotp)*0.5*coef;
            norm_term -= 0.5*pl_log(sm_svd[k]);
            if(j==0)
                tr_inv_Sigma += coef;
        }

        train_costs.last() += -1*(norm_term + mahal);
    }

    train_costs.last() / k_neighbors;

    inv_Sigma_F.resize( n_components, inputsize() );
    inv_Sigma_F.clear();
    for(int k=0; k<n_components; k++)
    { 
        fk = F(k);
        inv_sigma_fk = inv_Sigma_F(k);
        inv_sigma_fk << fk;
        inv_sigma_fk /= sigma_noise;
        for(int k2=0; k2<n_components;k2++)
        {
            uk2 = U(k2);
            multiplyAcc(inv_sigma_fk,uk2,
                        (1.0/(sm_svd[k2]+sigma_noise) - 1.0/sigma_noise)*
                        dot(fk,uk2));
        }
    }

    all_outputs_gradient.clear();
    real coef = 1/train_set->length();
    for(int neighbor=0; neighbor<k_neighbors; neighbor++)
    {
        // dNLL/dF
        product(temp_ncomp,F,inv_Sigma_z(neighbor));
        bprop_to_bases(all_outputs_gradient.toVec(0,n_components * inputsize()).toMat(n_components,inputsize()),
                       inv_Sigma_F,
                       temp_ncomp,inv_Sigma_z(neighbor),
                       coef);

        // dNLL/dmu
        multiplyAcc(all_outputs_gradient.subVec(n_components * inputsize(),
                                                inputsize()), 
                    inv_Sigma_z(neighbor),
                    -coef) ;

        // dNLL/dsn
        all_outputs_gradient[(n_components + 1 )* inputsize()] += coef* 
            0.5*(tr_inv_Sigma - pownorm(inv_Sigma_z(neighbor))) * 
            2 * pre_sigma_noise[0];
    }

    // Propagating supervised gradient
    output_connections->bpropUpdate( input_representation, all_outputs,
                                     expectation_gradients[n_layers-1], 
                                     all_outputs_gradient);

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

// grad_F += alpa ( M - v1 v2')
void DeepNonLocalManifoldParzen::bprop_to_bases(const Mat& R, const Mat& M, 
                                                const Vec& v1, 
                                                const Vec& v2, real alpha)
{
#ifdef BOUNDCHECK
    if (M.length() != R.length() || M.width() != R.width() 
        || v1.length()!=M.length() || M.width()!=v2.length() )
        PLERROR("DeepNonLocalManifoldParzen::bprop_to_bases(): incompatible "
                "arguments' sizes");
#endif

    const real* v_1=v1.data();
    const real* v_2=v2.data();
    for (int i=0;i<M.length();i++)
    {
        real* mi = M[i];
        real* ri = R[i];
        real v1i = v_1[i];
        for (int j=0;j<M.width();j++)
            ri[j] += alpha*(mi[j] - v1i * v_2[j]);
    }
}


void DeepNonLocalManifoldParzen::computeRepresentation(const Vec& input,
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

    for( int i=0 ; i<layer; i++ )
    {
        connections[i]->fprop( expectations[i], activations[i+1] );
        layers[i+1]->fprop(activations[i+1],expectations[i+1]);
    }
    representation.resize(expectations[layer].length());
    representation << expectations[layer];
}

void DeepNonLocalManifoldParzen::computeOutput(const Vec& input, Vec& output) const
{
    test_votes.resize(n_classes);
    test_votes.clear();

    // Variables for probability computations
    real log_p_x_g_y = 0;
    real mahal = 0;
    real norm_term = 0;
    real n = inputsize();
    real dotp = 0;
    real coef = 0;
    real sigma_noise = 0;
    
    Vec input_j(inputsize());
    Vec target(targetsize());
    real weight;

    if( save_manifold_parzen_parameters )
    {
        updateManifoldParzenParameters();

        int input_j_index;
        for( int i=0; i<n_classes; i++ )
        {
            for( int j=0; j<class_datasets[i]->length(); j++ )
            {
                class_datasets[i]->getExample(input_j,target,weight);

                input_j_index = class_datasets[i]->indices[j];
                U << eigenvectors[input_j_index];
                sm_svd << eigenvalues[input_j_index];
                sigma_noise = sigma_noises[input_j_index];
                mu << mus[input_j_index];

                substract(input,input_j,diff_neighbor_input); 
                substract(diff_neighbor_input,mu,diff); 
                    
                mahal = -0.5*pownorm(diff)/sigma_noise;      
                norm_term = - n/2.0 * Log2Pi - 0.5*(n-n_components)*
                    pl_log(sigma_noise);

                for(int k=0; k<n_components; k++)
                { 
                    uk = U(k);
                    dotp = dot(diff,uk);
                    coef = (1.0/(sm_svd[k]+sigma_noise) - 1.0/sigma_noise);
                    mahal -= square(dotp)*0.5*coef;
                    norm_term -= 0.5*pl_log(sm_svd[k]);
                }
                
                if( j==0 )
                    log_p_x_g_y = norm_term + mahal;
                else
                    log_p_x_g_y = logadd(norm_term + mahal, log_p_x_g_y);
            }

            test_votes[i] = log_p_x_g_y;
        }
    }
    else
    {

        for( int i=0; i<n_classes; i++ )
        {
            for( int j=0; j<class_datasets[i]->length(); j++ )
            {
                class_datasets[i]->getExample(input_j,target,weight);

                computeManifoldParzenParameters( input_j, F, mu, 
                                                 pre_sigma_noise, U, sm_svd );
                
                sigma_noise = square(pre_sigma_noise[0], 2) + min_sigma_noise;
                
                substract(input,input_j,diff_neighbor_input); 
                substract(diff_neighbor_input,mu,diff); 
                    
                mahal = -0.5*pownorm(diff)/sigma_noise;      
                norm_term = - n/2.0 * Log2Pi - 0.5*(n-n_components)*
                    pl_log(sigma_noise);

                for(int k=0; k<n_components; k++)
                { 
                    uk = U(k);
                    dotp = dot(diff,uk);
                    coef = (1.0/(sm_svd[k]+sigma_noise) - 1.0/sigma_noise);
                    mahal -= square(dotp)*0.5*coef;
                    norm_term -= 0.5*pl_log(sm_svd[k]);
                }
                
                if( j==0 )
                    log_p_x_g_y = norm_term + mahal;
                else
                    log_p_x_g_y = logadd(norm_term + mahal, log_p_x_g_y);
            }

            test_votes[i] = log_p_x_g_y;
        }
    }


    output[0] = argmax(test_votes);
}

void DeepNonLocalManifoldParzen::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{

    //Assumes that computeOutput has been called

    costs.resize( getTestCostNames().length() );
    costs.fill( MISSING_VALUE );

    if( currently_trained_layer<n_layers 
        && reconstruction_connections.length() != 0 )
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
        layers[ currently_trained_layer-1 ]->setExpectationByRef( 
            layers[ currently_trained_layer-1 ]->expectation);
        costs[ currently_trained_layer-1 ]  = 
            layers[ currently_trained_layer-1 ]->fpropNLL(
                expectations[currently_trained_layer-1]);
    }

    if( ((int)round(output[0])) == ((int)round(target[0])) )
        costs[n_layers-1] = 0;
    else
        costs[n_layers-1] = 1;
}

//////////
// test //
//////////
void DeepNonLocalManifoldParzen::updateManifoldParzenParameters() const
{
    if(!manifold_parzen_parameters_are_up_to_date)
    {
        // Precompute manifold parzen parameters
        Vec input( inputsize() );
        Vec target( targetsize() );
        real weight;

        eigenvectors.resize(train_set->length());
        eigenvalues.resize(train_set->length(),n_components);
        sigma_noises.resize(train_set->length());
        mus.resize(train_set->length(), inputsize());

        for( int i=0; i<train_set->length(); i++ )
        {
            train_set[i]->getExample(input,target,weight);

            computeManifoldParzenParameters( input, F, mu, 
                                             pre_sigma_noise, U, sm_svd );
            
            sigma_noise = square(pre_sigma_noise[0], 2) + min_sigma_noise;

            eigenvectors[i].resize(n_components,inputsize());
            eigenvectors[i] << U;
            eigenvalues[i] << sm_svd;
            sigma_noises[i] = sigma_noise;
            mus[i] << mu;
        }
        
        manifold_parzen_parameters_are_up_to_date = true;
    }
}

TVec<string> DeepNonLocalManifoldParzen::getTestCostNames() const
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

TVec<string> DeepNonLocalManifoldParzen::getTrainCostNames() const
{
    TVec<string> cost_names = getTestCostNames();
    cost_names.append( "NLL_neighbors" );
    return cost_names ;    
}

void DeepNonLocalManifoldParzen::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set,call_forget);
    
    manifold_parzen_parameters_are_up_to_date = false;

    Vec input( inputsize() );
    Vec target( targetsize() );
    real weight; // unused
    
    // Separate classes
    class_datasets.resize(n_classes);
    for(int k=0; k<n_classes; k++)
    {
        class_datasets[k] = new ClassSubsetVMatrix();
        class_datasets[k]->classes.resize(1);
        class_datasets[k]->classes[0] = k;
        class_datasets[k]->source = training_set;
        class_datasets[k]->build();
    }
    
    //// Find other classes proportions
    //class_proportions.resize(n_classes);
    //class_proportions.fill(0);
    //real sum = 0;
    //for(int k=0; k<n_classes; k++)
    //{
    //    class_proportions[k] = class_datasets[k]->length();
    //    sum += class_datasets[k]->length();
    //}
    //class_proportions /= sum;
}


//#####  Helper functions  ##################################################

void DeepNonLocalManifoldParzen::setLearningRate( real the_learning_rate )
{
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        layers[i]->setLearningRate( the_learning_rate );
        connections[i]->setLearningRate( the_learning_rate );
    }
    layers[n_layers-1]->setLearningRate( the_learning_rate );
    output_connections->setLearningRate( the_learning_rate );
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
