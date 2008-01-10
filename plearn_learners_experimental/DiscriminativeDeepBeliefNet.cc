// -*- C++ -*-

// DiscriminativeDeepBeliefNet.cc
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

/*! \file DiscriminativeDeepBeliefNet.cc */


#define PL_LOG_MODULE_NAME "DiscriminativeDeepBeliefNet"
#include <plearn/io/pl_log.h>

#include "DiscriminativeDeepBeliefNet.h"
#include <plearn/vmat/VMat_computeNearestNeighbors.h>
#include <plearn/vmat/GetInputVMatrix.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMMixedConnection.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DiscriminativeDeepBeliefNet,
    "Neural net, trained layer-wise in a greedy but focused fashion using autoassociators/RBMs and a supervised non-parametric gradient.",
    "It is highly inspired by the StackedFocusedAutoassociators class,\n"
    "and can use use the same RBMLayer and RBMConnection components.\n"
    );

DiscriminativeDeepBeliefNet::DiscriminativeDeepBeliefNet() :
    cd_learning_rate( 0. ),
    cd_decrease_ct( 0. ),
    fine_tuning_learning_rate( 0. ),
    fine_tuning_decrease_ct( 0. ),
    k_neighbors( 1 ),
    n_classes( -1 ),
    discriminative_criteria_weight( 0. ), 
    output_weights_l1_penalty_factor(0),
    output_weights_l2_penalty_factor(0),
    compare_joint_in_discriminative_criteria( false ),
    do_not_use_generative_criteria( false ),
//    cancel_normalization_terms( false ),
    n_layers( 0 ),
    nearest_neighbors_are_up_to_date( false ),
    currently_trained_layer( 0 )
{
    // random_gen will be initialized in PLearner::build_()
    random_gen = new PRandom();
    nstages = 0;
}

void DiscriminativeDeepBeliefNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "cd_learning_rate", 
                  &DiscriminativeDeepBeliefNet::cd_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the RBM "
                  "contrastive divergence training.\n");

    declareOption(ol, "cd_decrease_ct", 
                  &DiscriminativeDeepBeliefNet::cd_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "the RBMs contrastive\n"
                  "divergence training. When a hidden layer has finished "
                  "its training,\n"
                  "the learning rate is reset to it's initial value.\n");

    declareOption(ol, "fine_tuning_learning_rate", 
                  &DiscriminativeDeepBeliefNet::fine_tuning_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the fine tuning gradient descent.\n");

    declareOption(ol, "fine_tuning_decrease_ct", 
                  &DiscriminativeDeepBeliefNet::fine_tuning_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "fine tuning\n"
                  "gradient descent.\n");

    declareOption(ol, "training_schedule", 
                  &DiscriminativeDeepBeliefNet::training_schedule,
                  OptionBase::buildoption,
                  "Number of examples to use during each phase of greedy pre-training.\n"
                  "The number of fine-tunig steps is defined by nstages.\n"
        );

    declareOption(ol, "layers", &DiscriminativeDeepBeliefNet::layers,
                  OptionBase::buildoption,
                  "The layers of units in the network. The first element\n"
                  "of this vector should be the input layer and the\n"
                  "subsequent elements should be the hidden layers. The\n"
                  "output layer should not be included in layers.\n");

    declareOption(ol, "connections", &DiscriminativeDeepBeliefNet::connections,
                  OptionBase::buildoption,
                  "The weights of the connections between the layers.\n");

    declareOption(ol, "unsupervised_layers", 
                  &DiscriminativeDeepBeliefNet::unsupervised_layers,
                  OptionBase::buildoption,
                  "Additional units for greedy unsupervised learning.\n");

    declareOption(ol, "unsupervised_connections", 
                  &DiscriminativeDeepBeliefNet::unsupervised_connections,
                  OptionBase::buildoption,
                  "Additional connections for greedy unsupervised learning.\n");

    declareOption(ol, "k_neighbors", 
                  &DiscriminativeDeepBeliefNet::k_neighbors,
                  OptionBase::buildoption,
                  "Number of good nearest neighbors to attract and bad nearest "
                  "neighbors to repel.\n");

    declareOption(ol, "n_classes", 
                  &DiscriminativeDeepBeliefNet::n_classes,
                  OptionBase::buildoption,
                  "Number of classes.\n");

    declareOption(ol, "discriminative_criteria_weight", 
                  &DiscriminativeDeepBeliefNet::discriminative_criteria_weight,
                  OptionBase::buildoption,
                  "Weight of the discriminative criteria.\n");

    declareOption(ol, "output_weights_l1_penalty_factor", 
                  &DiscriminativeDeepBeliefNet::output_weights_l1_penalty_factor,
                  OptionBase::buildoption,
                  "Output weights l1_penalty_factor.\n");

    declareOption(ol, "output_weights_l2_penalty_factor", 
                  &DiscriminativeDeepBeliefNet::output_weights_l2_penalty_factor,
                  OptionBase::buildoption,
                  "Output weights l2_penalty_factor.\n");

    declareOption(ol, "compare_joint_in_discriminative_criteria", 
                  &DiscriminativeDeepBeliefNet::compare_joint_in_discriminative_criteria,
                  OptionBase::buildoption,
                  "Indication that the discriminative criteria should use the joint\n"
                  "over the input and the hidden units, instead of the conditional\n"
                  "over the hidden units given the input units.\n");

    declareOption(ol, "do_not_use_generative_criteria", 
                  &DiscriminativeDeepBeliefNet::do_not_use_generative_criteria,
                  OptionBase::buildoption,
                  "Indication that the generative criteria should not be used during learning\n"
                  "(does not work with compare_joint_in_discriminative_criteria = true).\n");

//    declareOption(ol, "cancel_normalization_terms", 
//                  &DiscriminativeDeepBeliefNet::cancel_normalization_terms,
//                  OptionBase::buildoption,
//                  "Indication that the discriminative and generative criteria should cancel\n"
//                  "their normalization terms. This is for the "
//                  "compare_joint_in_discriminative_criteria\n"
//                  "option, and this option ignores the value of discriminative_criteria_weight.\n");

    declareOption(ol, "greedy_stages", 
                  &DiscriminativeDeepBeliefNet::greedy_stages,
                  OptionBase::learntoption,
                  "Number of training samples seen in the different greedy "
                  "phases.\n"
        );

    declareOption(ol, "n_layers", &DiscriminativeDeepBeliefNet::n_layers,
                  OptionBase::learntoption,
                  "Number of layers.\n"
        );

    declareOption(ol, "final_module", 
                  &DiscriminativeDeepBeliefNet::final_module,
                  OptionBase::learntoption,
                  "Output layer of neural net.\n"
        );

    declareOption(ol, "final_cost", 
                  &DiscriminativeDeepBeliefNet::final_cost,
                  OptionBase::learntoption,
                  "Cost on output layer of neural net.\n"
        );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DiscriminativeDeepBeliefNet::build_()
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
            PLERROR("DiscriminativeDeepBeliefNet::build_() - \n"
                    "n_classes should be > 0.\n");

        if( k_neighbors <= 0 )
            PLERROR("DiscriminativeDeepBeliefNet::build_() - \n"
                    "k_neighbors should be > 0.\n");

        if( weightsize_ > 0 )
            PLERROR("DiscriminativeDeepBeliefNet::build_() - \n"
                    "usage of weighted samples (weight size > 0) is not\n"
                    "implemented yet.\n");

        if( training_schedule.length() != n_layers-1 )        
            PLERROR("DiscriminativeDeepBeliefNet::build_() - \n"
                    "training_schedule should have %d elements.\n",
                    n_layers-1);
        
        if( compare_joint_in_discriminative_criteria && do_not_use_generative_criteria)
            PLERROR("DiscriminativeDeepBeliefNet::build_() - \n"
                    "compare_joint_in_discriminative_criteria can't be used with\n"
                    "do_not_use_generative_criteria.\n");

//        if( (!compare_joint_in_discriminative_criteria || do_not_use_generative_criteria)
//            && cancel_normalization_terms )
//            PLERROR("DiscriminativeDeepBeliefNet::build_() - \n"
//                    "cancel_normalization_terms should be used with\n"
//                    "compare_joint_in_discriminative_criteria and \n"
//                    "do_not_use_generative_criteria without .\n");
            
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

void DiscriminativeDeepBeliefNet::build_output_layer_and_cost()
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

void DiscriminativeDeepBeliefNet::build_layers_and_connections()
{
    MODULE_LOG << "build_layers_and_connections() called" << endl;

    if( connections.length() != n_layers-1 )
        PLERROR("DiscriminativeDeepBeliefNet::build_layers_and_connections() - \n"
                "there should be %d connections.\n",
                n_layers-1);
     
    if(unsupervised_layers.length() != n_layers-1 
       && unsupervised_layers.length() != 0)
        PLERROR("DiscriminativeDeepBeliefNet::build_layers_and_connections() - \n"
                "there should be either 0 of %d unsupervised_layers.\n",
                n_layers-1);
        
    if(unsupervised_connections.length() != n_layers-1 
       && unsupervised_connections.length() != 0)
        PLERROR("DiscriminativeDeepBeliefNet::build_layers_and_connections() - \n"
                "there should be either 0 of %d unsupervised_connections.\n",
                n_layers-1);
        
    if(unsupervised_connections.length() != unsupervised_layers.length())
        PLERROR("DiscriminativeDeepBeliefNet::build_layers_and_connections() - \n"
                "there should be as many unsupervised_connections and "
                "unsupervised_layers.\n");
        

    if(layers[0]->size != inputsize_)
        PLERROR("DiscriminativeDeepBeliefNet::build_layers_and_connections() - \n"
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
            PLERROR("DiscriminativeDeepBeliefNet::build_layers_and_connections() "
                    "- \n"
                    "connections[%i] should have a down_size of %d.\n",
                    i, layers[i]->size);

        if( connections[i]->up_size != layers[i+1]->size )
            PLERROR("DiscriminativeDeepBeliefNet::build_layers_and_connections() "
                    "- \n"
                    "connections[%i] should have a up_size of %d.\n",
                    i, layers[i+1]->size);

        if(unsupervised_layers.length() != 0 &&
           unsupervised_connections.length() != 0 && 
           unsupervised_layers[i] && unsupervised_connections[i])
        {
            if( layers[i]->size != 
                unsupervised_connections[i]->down_size )
                PLERROR("DiscriminativeDeepBeliefNet::build_layers_and_connections() "
                        "- \n"
                        "connections[%i] should have a down_size of %d.\n",
                        i, unsupervised_layers[i]->size);
            
            if( unsupervised_connections[i]->up_size != 
                unsupervised_layers[i]->size )
                PLERROR("DiscriminativeDeepBeliefNet::build_layers_and_connections() "
                        "- \n"
                        "connections[%i] should have a up_size of %d.\n",
                        i, unsupervised_layers[i+1]->size);
            
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
            greedy_layer->size = layers[i+1]->size + unsupervised_layers[i]->size;
            greedy_layer->build();

            PP<RBMMixedConnection> greedy_connection = new RBMMixedConnection();
            greedy_connection->sub_connections.resize(2,1);
            greedy_connection->sub_connections(0,0) = connections[i];
            greedy_connection->sub_connections(1,0) = unsupervised_connections[i];
            greedy_connection->build();
            
            greedy_layers[i] = greedy_layer;
            greedy_connections[i] = greedy_connection;
        }
        else
        {
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
void DiscriminativeDeepBeliefNet::build()
{
    inherited::build();
    build_();
}


void DiscriminativeDeepBeliefNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(, copies);

    // Public options
    deepCopyField(training_schedule, copies);
    deepCopyField(layers, copies);
    deepCopyField(connections, copies);
    deepCopyField(unsupervised_layers, copies);
    deepCopyField(unsupervised_connections, copies);

    // Protected options
    deepCopyField(activations, copies);
    deepCopyField(expectations, copies);
    deepCopyField(activation_gradients, copies);
    deepCopyField(expectation_gradients, copies);
    deepCopyField(greedy_layers, copies);
    deepCopyField(greedy_connections, copies);
    deepCopyField(dissimilar_example_representation, copies);
    deepCopyField(input_representation, copies);
    deepCopyField(pos_down_val, copies);
    deepCopyField(pos_up_val, copies);
    deepCopyField(neg_down_val, copies);
    deepCopyField(neg_up_val, copies);
    deepCopyField(disc_pos_down_val1, copies);
    deepCopyField(disc_pos_up_val1, copies);
    deepCopyField(disc_pos_down_val2, copies);
    deepCopyField(disc_pos_up_val2, copies);
    deepCopyField(disc_neg_down_val, copies);
    deepCopyField(disc_neg_up_val, copies);
    deepCopyField(final_cost_input, copies);
    deepCopyField(final_cost_value, copies);
    deepCopyField(final_cost_gradient, copies);
    deepCopyField(other_class_datasets, copies);
    deepCopyField(nearest_neighbors_indices, copies);
    deepCopyField(greedy_stages, copies);
    deepCopyField(final_module, copies);
    deepCopyField(final_cost, copies);
}


int DiscriminativeDeepBeliefNet::outputsize() const
{
    if( currently_trained_layer>n_layers-1 )
        return 1;
    else
        return layers[currently_trained_layer]->size;
}

void DiscriminativeDeepBeliefNet::forget()
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

    for( int i=0 ; i<n_layers ; i++ )
        layers[i]->forget();
    
    for( int i=0 ; i<n_layers-1 ; i++ )
        connections[i]->forget();
    
    if(unsupervised_layers.length() != 0)
        for( int i=0 ; i<n_layers-1 ; i++ )
            unsupervised_layers[i]->forget();
    
    if(unsupervised_connections.length() != 0)
        for( int i=0 ; i<n_layers-1 ; i++ )
            unsupervised_connections[i]->forget();
    
    build_output_layer_and_cost();

    stage = 0;
    greedy_stages.clear();
}

void DiscriminativeDeepBeliefNet::train()
{
    MODULE_LOG << "train() called " << endl;
    MODULE_LOG << "  training_schedule = " << training_schedule << endl;

    Vec input( inputsize() );
    Vec dissimilar_example( inputsize() );
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
        updateNearestNeighbors();
            
        MODULE_LOG << "Training connection weights between layers " << i
            << " and " << i+1 << endl;

        int end_stage = training_schedule[i];
        int* this_stage = greedy_stages.subVec(i,1).data();
        init_stage = *this_stage;

        MODULE_LOG << "  stage = " << *this_stage << endl;
        MODULE_LOG << "  end_stage = " << end_stage << endl;

        if( report_progress && *this_stage < end_stage )
            pb = new ProgressBar( "Training layer "+tostring(i)
                                  +" of "+classname(),
                                  end_stage - init_stage );

        train_costs.fill(MISSING_VALUE);
 
        dissimilar_example_representation.resize(layers[i]->size);
        input_representation.resize(layers[i]->size);

        pos_down_val.resize(layers[i]->size);
        pos_up_val.resize(greedy_layers[i]->size);
        neg_down_val.resize(layers[i]->size);
        neg_up_val.resize(greedy_layers[i]->size);

        disc_pos_down_val1.resize(layers[i]->size);
        disc_pos_up_val1.resize(layers[i+1]->size);
        disc_pos_down_val2.resize(layers[i]->size);
        disc_pos_up_val2.resize(layers[i+1]->size);
        disc_neg_down_val.resize(layers[i]->size);
        disc_neg_up_val.resize(layers[i+1]->size);

        for( ; *this_stage<end_stage ; (*this_stage)++ )
        {
            sample = *this_stage % nsamples;
            train_set->getExample(sample, input, target, weight);

            // Find dissimilar example
            int dissim_index = nearest_neighbors_indices(
                sample,random_gen->uniform_multinomial_sample(k_neighbors));
            
            other_class_datasets[(int)round(target[0])]->getExample(dissim_index,
                                                                    dissimilar_example, 
                                                                    target2, weight2);
            
            if(((int)round(target[0])) == ((int)round(target2[0])))
                PLERROR("DiscriminativeDeepBeliefNet::train(): dissimilar"
                        " example is from same class!");

            greedyStep( input, target, i, train_costs, *this_stage,
                        dissimilar_example);
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

void DiscriminativeDeepBeliefNet::greedyStep( 
    const Vec& input, const Vec& target, int index, 
    Vec train_costs, int this_stage, Vec dissimilar_example )
{
    PLASSERT( index < n_layers );
    real lr;

    // Get dissimilar example representation
    computeRepresentation(dissimilar_example, dissimilar_example_representation, 
                          index);

    // Get example representation
    computeRepresentation(input, input_representation, 
                          index);

    if( !do_not_use_generative_criteria )
    {
        // CD generative learning stats
        
        // Positive phase
        greedy_connections[index]->setAsDownInput( input_representation );
        greedy_layers[index]->getAllActivations( greedy_connections[index] );
        greedy_layers[index]->computeExpectation();
        greedy_layers[index]->generateSample();
        
        pos_down_val << input_representation;
        pos_up_val << greedy_layers[index]->expectation;
        
        if( !compare_joint_in_discriminative_criteria )
        {
            disc_pos_down_val1 << input_representation;
            disc_pos_up_val1 << layers[index+1]->expectation;
        }
        
//        if( !cancel_normalization_terms )
//        {
        // Negative phase
        greedy_connections[index]->setAsUpInput( greedy_layers[index]->sample );    
        layers[index]->getAllActivations( greedy_connections[index] );
        layers[index]->computeExpectation();
        layers[index]->generateSample();
        
        greedy_connections[index]->setAsDownInput( layers[index]->sample );
        greedy_layers[index]->getAllActivations( greedy_connections[index] );
        greedy_layers[index]->computeExpectation();
        
        neg_down_val << layers[index]->sample;
        neg_up_val << greedy_layers[index]->expectation;
//      }
    }
    else if( !compare_joint_in_discriminative_criteria )
    {
        
        connections[index]->setAsDownInput( input_representation );
        layers[index+1]->getAllActivations( connections[index] );
        layers[index+1]->computeExpectation();
        
        disc_pos_down_val1 << input_representation;
        disc_pos_up_val1 << layers[index+1]->expectation;
    }

    // CD discriminative criteria stats

    if( !compare_joint_in_discriminative_criteria )
    {
        // Positive phase
        connections[index]->setAsDownInput( dissimilar_example_representation );
        layers[index+1]->getAllActivations( connections[index] );
        layers[index+1]->computeExpectation();
        
        disc_pos_down_val2 << dissimilar_example_representation;
        disc_pos_up_val2 << layers[index+1]->expectation;
    }

    // Negative phase
    disc_neg_down_val << input_representation;
    disc_neg_down_val += dissimilar_example_representation;
    disc_neg_down_val /= 2;
    connections[index]->setAsDownInput( disc_neg_down_val );
    layers[index+1]->getAllActivations( connections[index] );
    layers[index+1]->computeExpectation();

    disc_neg_up_val << layers[index+1]->expectation;

    if( compare_joint_in_discriminative_criteria )
        //&& !cancel_normalization_terms)
    {
        layers[index+1]->generateSample();
        connections[index]->setAsUpInput( layers[index+1]->sample );
        layers[index]->getAllActivations( connections[index] );
        layers[index]->computeExpectation();
        layers[index]->generateSample();

        connections[index]->setAsDownInput( layers[index]->sample );
        layers[index+1]->getAllActivations( connections[index] );
        layers[index+1]->computeExpectation();

        disc_pos_down_val1 << layers[index]->sample;
        disc_pos_up_val1 << layers[index+1]->expectation;
    }

    // RBM updates
    if( !do_not_use_generative_criteria )
        //&& !cancel_normalization_terms )
    {
        lr = cd_learning_rate/(1 + cd_decrease_ct 
                               * this_stage); 
        
        layers[index]->setLearningRate( lr );
        greedy_connections[index]->setLearningRate( lr );
        greedy_layers[index]->setLearningRate( lr );
        
        layers[index]->update( pos_down_val, neg_down_val );
        greedy_connections[index]->update( pos_down_val, pos_up_val,
                                           neg_down_val, neg_up_val );
        greedy_layers[index]->update( pos_up_val, neg_up_val );
    }
    
    if( //cancel_normalization_terms || 
        discriminative_criteria_weight != 0 )
    {
        lr = discriminative_criteria_weight * 
            cd_learning_rate/(1 + cd_decrease_ct 
                              * this_stage); 
        
        if( !compare_joint_in_discriminative_criteria )
        {
            layers[index]->setLearningRate( lr );
            connections[index]->setLearningRate( lr );
            layers[index+1]->setLearningRate( lr );
            
            layers[index]->accumulatePosStats( disc_pos_down_val1 );
            layers[index]->accumulatePosStats( disc_pos_down_val2 );
            layers[index]->accumulateNegStats( disc_neg_down_val );
            layers[index]->update();
            
            connections[index]->accumulatePosStats( disc_pos_down_val1,
                                                    disc_pos_up_val1 );
            connections[index]->accumulatePosStats( disc_pos_down_val2,
                                                    disc_pos_up_val2 );
            connections[index]->accumulateNegStats( disc_neg_down_val,
                                                    disc_neg_up_val );
            connections[index]->update();
            
            layers[index+1]->accumulatePosStats( disc_pos_up_val1 );
            layers[index+1]->accumulatePosStats( disc_pos_up_val2 );
            layers[index+1]->accumulateNegStats( disc_neg_up_val );
            layers[index+1]->update();
        }
        else //if( !cancel_normalization_terms )
        {
            layers[index]->setLearningRate( lr );
            connections[index]->setLearningRate( lr );
            layers[index+1]->setLearningRate( lr );
            
            layers[index]->accumulatePosStats( disc_pos_down_val1 );
            layers[index]->accumulateNegStats( disc_neg_down_val );
            layers[index]->update();
            
            connections[index]->accumulatePosStats( disc_pos_down_val1,
                                                    disc_pos_up_val1 );
            connections[index]->accumulateNegStats( disc_neg_down_val,
                                                    disc_neg_up_val );
            connections[index]->update();
            
            layers[index+1]->accumulatePosStats( disc_pos_up_val1 );
            layers[index+1]->accumulateNegStats( disc_neg_up_val );
            layers[index+1]->update();
        }
//        else
//        {
//            lr = cd_learning_rate/(1 + cd_decrease_ct 
//                                   * this_stage); 
//            layers[index]->setLearningRate( lr );
//            connections[index]->setLearningRate( lr );
//            layers[index+1]->setLearningRate( lr );
//            
//            layers[index]->accumulatePosStats( pos_down_val );
//            layers[index]->accumulateNegStats( disc_neg_down_val );
//            layers[index]->update();
//            
//            connections[index]->accumulatePosStats( pos_down_val,
//                                                    pos_up_val );
//            connections[index]->accumulateNegStats( disc_neg_down_val,
//                                                    disc_neg_up_val );
//            connections[index]->update();
//            
//            layers[index+1]->accumulatePosStats( pos_up_val );
//            layers[index+1]->accumulateNegStats( disc_neg_up_val );
//            layers[index+1]->update();
//        }
    }
}

void DiscriminativeDeepBeliefNet::fineTuningStep( 
    const Vec& input, const Vec& target,
    Vec& train_costs )
{
    // Get example representation

    computeRepresentation(input, input_representation, 
                          n_layers-1);

    // Compute supervised gradient
    final_module->fprop( input_representation, final_cost_input );
    final_cost->fprop( final_cost_input, target, final_cost_value );
    
    final_cost->bpropUpdate( final_cost_input, target,
                             final_cost_value[0],
                             final_cost_gradient );
    final_module->bpropUpdate( input_representation,
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

void DiscriminativeDeepBeliefNet::computeRepresentation(const Vec& input,
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

void DiscriminativeDeepBeliefNet::computeOutput(const Vec& input, Vec& output) const
{
    if( currently_trained_layer>n_layers-1 )
    {
        computeRepresentation(input,input_representation, 
                              n_layers-1);
        final_module->fprop( input_representation, final_cost_input );
        output[0] = argmax(final_cost_input);
    }
    else
    {
        computeRepresentation(input, output,
                              currently_trained_layer);
    }
}

void DiscriminativeDeepBeliefNet::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    //Assumes that computeOutput has been called

    costs.resize( getTestCostNames().length() );
    costs.fill( MISSING_VALUE );

    if( currently_trained_layer>n_layers-1 )
        if( ((int)round(output[0])) == ((int)round(target[0])) )
            costs.last() = 0;
        else
            costs.last() = 1;
}

TVec<string> DiscriminativeDeepBeliefNet::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).

    TVec<string> cost_names(0);

    cost_names.append( "class_error" );

    return cost_names;
}

TVec<string> DiscriminativeDeepBeliefNet::getTrainCostNames() const
{
    return getTestCostNames();
}

void DiscriminativeDeepBeliefNet::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set,call_forget);
    nearest_neighbors_are_up_to_date = false;
}

void DiscriminativeDeepBeliefNet::updateNearestNeighbors()
{
    if( !nearest_neighbors_are_up_to_date )
    {
        MODULE_LOG << "Computing nearest neighbors" << endl;

        Vec input( inputsize() );
        Vec target( targetsize() );
        real weight; // unused
        
        other_class_datasets.resize(n_classes);
        for(int k=0; k<n_classes; k++)
        {
            other_class_datasets[k] = new ClassSubsetVMatrix();
            other_class_datasets[k]->classes.resize(0);
            for(int l=0; l<n_classes; l++)
                if( l != k )
                    other_class_datasets[k]->classes.append(l);
            other_class_datasets[k]->source = train_set;
            other_class_datasets[k]->build();
        }
        
        
        // Find training nearest neighbors
        input.resize(train_set->inputsize());
        target.resize(train_set->targetsize());
        nearest_neighbors_indices.resize(train_set->length(), k_neighbors);
        TVec<int> nearest_neighbors_indices_row;
        for(int i=0; i<train_set.length(); i++)
        {
            train_set->getExample(i,input,target,weight);
            nearest_neighbors_indices_row = nearest_neighbors_indices(i);
            computeNearestNeighbors(
                new GetInputVMatrix((VMatrix *)
                                    other_class_datasets[(int)round(target[0])]),
                input,
                nearest_neighbors_indices_row,
                -1);
        }
    }
    
    nearest_neighbors_are_up_to_date = true;
}
//#####  Helper functions  ##################################################

void DiscriminativeDeepBeliefNet::setLearningRate( real the_learning_rate )
{
    layers[0]->setLearningRate( the_learning_rate );
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        greedy_layers[i]->setLearningRate( the_learning_rate );
        greedy_connections[i]->setLearningRate( the_learning_rate );
    }

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
