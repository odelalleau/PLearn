// -*- C++ -*-

// DynamicallyLinkedRBMsModel.cc
//
// Copyright (C) 2006 Stanislas Lauly
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

// Authors: Stanislas Lauly

/*! \file DynamicallyLinkedRBMsModel.cc */


#define PL_LOG_MODULE_NAME "DynamicallyLinkedRBMsModel"
#include <plearn/io/pl_log.h>

#include "DynamicallyLinkedRBMsModel.h"
#include "plearn/math/plapack.h"

// Options to have:
//
// - input_layer
// - input_connection
// - target_layers
// - target_connections
// - target_layers_weights
// - mask_size
// - end_of_sequence_symbol
// - input reconstruction weight
//
// Problems to have in mind:
// 
// - have a proper normalization of costs
// - output one cost per target 
// - make sure gradient descent is proper (change some vectors into matrices, etc.)
// - make sure end_of_sequence_symbol is used appropriately
// - make sure declareOption includes everything, including saved variable
// - verify use of mask is proper
// - do proper resize of recurrent internal variables
// - implement deepcopy appropriately
// - corriger bug avec activation (faut additionner les biais!!!)

// - commiter mse
// - add dynamic_activations_list and use it in recurrent_update
// - verify code works with and without hidden_layer2

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DynamicallyLinkedRBMsModel,
    "Model made of RBMs linked through time",
    ""
    );


DynamicallyLinkedRBMsModel::DynamicallyLinkedRBMsModel() :
    //rbm_learning_rate( 0.01 ),
    recurrent_net_learning_rate( 0.01),
    use_target_layers_masks( false ),
    end_of_sequence_symbol( -1000 )
    //rbm_nstages( 0 ),
{
    random_gen = new PRandom();
}

void DynamicallyLinkedRBMsModel::declareOptions(OptionList& ol)
{
//    declareOption(ol, "rbm_learning_rate", &DynamicallyLinkedRBMsModel::rbm_learning_rate,
//                  OptionBase::buildoption,
//                  "The learning rate used during RBM contrastive "
//                  "divergence learning phase.\n");

    declareOption(ol, "recurrent_net_learning_rate", 
                  &DynamicallyLinkedRBMsModel::recurrent_net_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the recurrent phase.\n");

//    declareOption(ol, "rbm_nstages", &DynamicallyLinkedRBMsModel::rbm_nstages,
//                  OptionBase::buildoption,
//                  "Number of epochs for rbm phase.\n");


    declareOption(ol, "target_layers_weights", 
                  &DynamicallyLinkedRBMsModel::target_layers_weights,
                  OptionBase::buildoption,
                  "The training weights of each target layers.\n");

    declareOption(ol, "use_target_layers_masks", 
                  &DynamicallyLinkedRBMsModel::use_target_layers_masks,
                  OptionBase::buildoption,
                  "Indication that a mask indicating which target to predict\n"
                  "is present in the input part of the VMatrix dataset.\n");

    declareOption(ol, "end_of_sequence_symbol", 
                  &DynamicallyLinkedRBMsModel::end_of_sequence_symbol,
                  OptionBase::buildoption,
                  "Value of the first input component for end-of-sequence "
                  "delimiter.\n");

    declareOption(ol, "input_layer", &DynamicallyLinkedRBMsModel::input_layer,
                  OptionBase::buildoption,
                  "The input layer of the model.\n");

    declareOption(ol, "target_layers", &DynamicallyLinkedRBMsModel::target_layers,
                  OptionBase::buildoption,
                  "The target layers of the model.\n");

    declareOption(ol, "hidden_layer", &DynamicallyLinkedRBMsModel::hidden_layer,
                  OptionBase::buildoption,
                  "The hidden layer of the model.\n");

    declareOption(ol, "hidden_layer2", &DynamicallyLinkedRBMsModel::hidden_layer2,
                  OptionBase::buildoption,
                  "The second hidden layer of the model (optional).\n");

    declareOption(ol, "dynamic_connections", 
                  &DynamicallyLinkedRBMsModel::dynamic_connections,
                  OptionBase::buildoption,
                  "The RBMConnection between the first hidden layers, "
                  "through time.\n");

    declareOption(ol, "hidden_connections", 
                  &DynamicallyLinkedRBMsModel::hidden_connections,
                  OptionBase::buildoption,
                  "The RBMConnection between the first and second "
                  "hidden layers (optional).\n");

    declareOption(ol, "input_connections", 
                  &DynamicallyLinkedRBMsModel::input_connections,
                  OptionBase::buildoption,
                  "The RBMConnection from input_layer to hidden_layer.\n");

    declareOption(ol, "target_connections", 
                  &DynamicallyLinkedRBMsModel::target_connections,
                  OptionBase::buildoption,
                  "The RBMConnection from input_layer to hidden_layer.\n");

    /*
    declareOption(ol, "", 
                  &DynamicallyLinkedRBMsModel::,
                  OptionBase::buildoption,
                  "");
    */


    declareOption(ol, "target_layers_n_of_target_elements", 
                  &DynamicallyLinkedRBMsModel::target_layers_n_of_target_elements,
                  OptionBase::learntoption,
                  "Number of elements in the target part of a VMatrix associated\n"
                  "to each target layer.\n");

    declareOption(ol, "input_symbol_sizes", 
                  &DynamicallyLinkedRBMsModel::input_symbol_sizes,
                  OptionBase::learntoption,
                  "Number of symbols for each symbolic field of train_set.\n");

    declareOption(ol, "target_symbol_sizes", 
                  &DynamicallyLinkedRBMsModel::target_symbol_sizes,
                  OptionBase::learntoption,
                  "Number of symbols for each symbolic field of train_set.\n");

    /*
    declareOption(ol, "", &DynamicallyLinkedRBMsModel::,
                  OptionBase::learntoption,
                  "");
     */

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DynamicallyLinkedRBMsModel::build_()
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
        PLASSERT( target_layers_weights.length() == target_layers.length() );

        // Parsing symbols in input
        int input_layer_size = 0;
        input_symbol_sizes.resize(0);
        PP<Dictionary> dict;
        int inputsize_without_masks = inputsize() 
            - ( use_target_layers_masks ? targetsize() : 0 );
        for(int i=0; i<inputsize_without_masks; i++)
        {
            dict = train_set->getDictionary(i);
            if(dict)
            {
                if( dict->size() == 0 )
                    PLERROR("DynamicallyLinkedRBMsModel::build_(): dictionary "
                        "of field %d is empty", i);
                input_symbol_sizes.push_back(dict->size());
                // Adjust size to include one-hot vector
                input_layer_size += dict->size();
            }
            else
            {
                input_symbol_sizes.push_back(-1);
                input_layer_size++;
            }
        }

        if( input_layer->size != input_layer_size )
            PLERROR("DynamicallyLinkedRBMsModel::build_(): input_layer->size %d "
                    "should be %d", input_layer->size, input_layer_size);

        // Parsing symbols in target
        int tar_layer = 0;
        int tar_layer_size = 0;
        target_symbol_sizes.resize(target_layers.length());
        target_layers_n_of_target_elements.clear();
        for( int tar=0; tar<targetsize(); tar++)
        {
            if( tar_layer > target_layers.length() )
                PLERROR("DynamicallyLinkedRBMsModel::build_(): target layers "
                        "does not cover all targets.");            

            target_symbol_sizes[tar_layer].resize(0);
            dict = train_set->getDictionary(tar+inputsize());
            if(dict)
            {
                if( use_target_layers_masks )
                    PLERROR("DynamicallyLinkedRBMsModel::build_(): masks for "
                            "symbolic targets is not implemented.");
                if( dict->size() == 0 )
                    PLERROR("DynamicallyLinkedRBMsModel::build_(): dictionary "
                            "of field %d is empty", tar);

                target_symbol_sizes[tar_layer].push_back(dict->size());
                target_layers_n_of_target_elements[tar_layer]++;
                tar_layer_size += dict->size();
            }
            else
            {
                target_symbol_sizes[tar_layer].push_back(-1);
                target_layers_n_of_target_elements[tar_layer]++;
                tar_layer_size++;
            }

            if( target_layers[tar_layer]->size == tar_layer_size )
            {
                tar_layer++;
                tar_layer_size = 0;
            }
        }

        if( tar_layer != target_layers.length() )
            PLERROR("DynamicallyLinkedRBMsModel::build_(): target layers "
                    "does not cover all targets.");


        // Building weights and layers
        if( !input_layer->random_gen )
        {
            input_layer->random_gen = random_gen;
            input_layer->forget();
        }

        if( !hidden_layer->random_gen )
        {
            hidden_layer->random_gen = random_gen;
            hidden_layer->forget();
        }

        input_connections->down_size = input_layer->size;
        input_connections->up_size = hidden_layer->size;
        if( !input_connections->random_gen )
        {
            input_connections->random_gen = random_gen;
            input_connections->forget();
        }
        input_connections->build();


        dynamic_connections->down_size = hidden_layer->size;
        dynamic_connections->up_size = hidden_layer->size;
        if( !dynamic_connections->random_gen )
        {
            dynamic_connections->random_gen = random_gen;
            dynamic_connections->forget();
        }
        dynamic_connections->build();

        if( hidden_layer2 )
        {
            if( !hidden_layer2->random_gen )
            {
                hidden_layer2->random_gen = random_gen;
                hidden_layer2->forget();
            }

            hidden_connections->down_size = hidden_layer->size;
            hidden_connections->up_size = hidden_layer2->size;
            if( !hidden_connections->random_gen )
            {
                hidden_connections->random_gen = random_gen;
                hidden_connections->forget();
            }
            hidden_connections->build();
        }

        for( int tar_layer = 0; tar_layer < target_layers.length(); tar_layer++ )
        {
            if( !target_layers[tar_layer]->random_gen )
            {
                target_layers[tar_layer]->random_gen = random_gen;
                target_layers[tar_layer]->forget();
            }

            if( hidden_layer2 )
                target_connections[tar_layer]->down_size = hidden_layer2->size;
            else
                target_connections[tar_layer]->down_size = hidden_layer->size;

            target_connections[tar_layer]->up_size = target_layers[tar_layer]->size;
            if( !target_connections[tar_layer]->random_gen )
            {
                target_connections[tar_layer]->random_gen = random_gen;
                target_connections[tar_layer]->forget();
            }
            target_connections[tar_layer]->build();
        }

    }
}

// ### Nothing to add here, simply calls build_
void DynamicallyLinkedRBMsModel::build()
{
    inherited::build();
    build_();
}


void DynamicallyLinkedRBMsModel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField( input_layer, copies);
    deepCopyField( target_layers , copies);
    deepCopyField( hidden_layer, copies);
    deepCopyField( hidden_layer2 , copies);
    deepCopyField( dynamic_connections , copies);
    deepCopyField( hidden_connections , copies);
    deepCopyField( input_connections , copies);
    deepCopyField( target_connections , copies);
    deepCopyField( target_layers_n_of_target_elements, copies);
    deepCopyField( input_symbol_sizes, copies);
    deepCopyField( target_symbol_sizes, copies);
    

    deepCopyField( bias_gradient , copies);
    deepCopyField( hidden_gradient , copies);
    deepCopyField( hidden_temporal_gradient , copies);
    deepCopyField( hidden_list , copies);
    deepCopyField( hidden_act_no_bias_list , copies);
    deepCopyField( hidden2_list , copies);
    deepCopyField( hidden2_act_no_bias_list , copies);
    deepCopyField( target_prediction_list , copies);
    deepCopyField( target_prediction_act_no_bias_list , copies);
    deepCopyField( input_list , copies);
    deepCopyField( targets_list , copies);
    deepCopyField( nll_list , copies);
    deepCopyField( masks_list , copies);
    deepCopyField( dynamic_act_no_bias_contribution, copies);


    // deepCopyField(, copies);

    //PLERROR("DynamicallyLinkedRBMsModel::makeDeepCopyFromShallowCopy(): "
    //"not implemented yet");
}


int DynamicallyLinkedRBMsModel::outputsize() const
{
    int out_size = 0;
    for( int i=0; i<target_layers.length(); i++ )
        out_size += target_layers[i]->size;
    return out_size;
}

void DynamicallyLinkedRBMsModel::forget()
{
    inherited::forget();

    input_layer->forget();
    hidden_layer->forget();
    input_connections->forget();
    dynamic_connections->forget();
    if( hidden_layer2 )
    {
        hidden_layer2->forget();
        hidden_connections->forget();
    }

    for( int i=0; i<target_layers.length(); i++ )
    {
        target_layers[i]->forget();
        target_connections[i]->forget();
    }

    stage = 0;
}

void DynamicallyLinkedRBMsModel::train()
{
    MODULE_LOG << "train() called " << endl;

    Vec input( inputsize() );
    Vec target( targetsize() );
    real weight = 0; // Unused
    Vec train_costs( getTrainCostNames().length() );
    train_costs.clear();
    TVec<int> train_n_items( getTrainCostNames().length() );

    if( !initTrain() )
    {
        MODULE_LOG << "train() aborted" << endl;
        return;
    }

    ProgressBar* pb = 0;

    // clear stats of previous epoch
    train_stats->forget();


    /***** RBM training phase *****/
//    if(rbm_stage < rbm_nstages)
//    {
//    }


    /***** Recurrent phase *****/
    if( stage >= nstages )
        return;

    if( stage < nstages )
    {        

        MODULE_LOG << "Training the whole model" << endl;

        int init_stage = stage;
        //int end_stage = max(0,nstages-(rbm_nstages + dynamic_nstages));
        int end_stage = nstages;

        MODULE_LOG << "  stage = " << stage << endl;
        MODULE_LOG << "  end_stage = " << end_stage << endl;
        MODULE_LOG << "  recurrent_net_learning_rate = " << recurrent_net_learning_rate << endl;

        if( report_progress && stage < end_stage )
            pb = new ProgressBar( "Recurrent training phase of "+classname(),
                                  end_stage - init_stage );

        setLearningRate( recurrent_net_learning_rate );

        int ith_sample_in_sequence = 0;
        int inputsize_without_masks = inputsize() 
            - ( use_target_layers_masks ? targetsize() : 0 );
        int sum_target_elements = 0;
        while(stage < end_stage)
        {
/*
                TMat<real> U,V;//////////crap James
                TVec<real> S;
                U.resize(hidden_layer->size,hidden_layer->size);
                V.resize(hidden_layer->size,hidden_layer->size);
                S.resize(hidden_layer->size);
                U << dynamic_connections->weights;
                
                SVD(U,dynamic_connections->weights,S,V);
                S.fill(-0.5);
                productScaleAcc(dynamic_connections->bias,dynamic_connections->weights,S,1,0);
*/
            train_costs.clear();
            train_n_items.clear();
            for(int sample=0 ; sample<train_set->length() ; sample++ )
            {
                train_set->getExample(sample, input, target, weight);

                if( fast_exact_is_equal(input[0],end_of_sequence_symbol) )
                {
                    //update
                    recurrent_update();
                    
                    ith_sample_in_sequence = 0;
                    hidden_list.resize(0);
                    hidden_act_no_bias_list.resize(0);
                    hidden2_list.resize(0);
                    hidden2_act_no_bias_list.resize(0);
                    target_prediction_list.resize(0);
                    target_prediction_act_no_bias_list.resize(0);
                    input_list.resize(0);
                    targets_list.resize(0);
                    nll_list.resize(0,0);
                    masks_list.resize(0);
                    continue;
                }

                // Resize internal variables
                hidden_list.resize(ith_sample_in_sequence+1);
                hidden_act_no_bias_list.resize(ith_sample_in_sequence+1);
                if( hidden_layer2 )
                {
                    hidden2_list.resize(ith_sample_in_sequence+1);
                    hidden2_act_no_bias_list.resize(ith_sample_in_sequence+1);
                }
                 
                input_list.resize(ith_sample_in_sequence+1);
                input_list[ith_sample_in_sequence].resize(input_layer->size);

                targets_list.resize( target_layers.length() );
                target_prediction_list.resize( target_layers.length() );
                target_prediction_act_no_bias_list.resize( target_layers.length() );
                for( int tar=0; tar < target_layers.length(); tar++ )
                {
                    targets_list[tar].resize( ith_sample_in_sequence+1);
                    targets_list[tar][ith_sample_in_sequence].resize( 
                        target_layers[tar]->size);
                    target_prediction_list[tar].resize(
                        ith_sample_in_sequence+1);
                    target_prediction_act_no_bias_list[tar].resize(
                        ith_sample_in_sequence+1);

                }
                nll_list.resize(ith_sample_in_sequence+1,target_layers.length());
                if( use_target_layers_masks )
                {
                    masks_list.resize( target_layers.length() );
                    for( int tar=0; tar < target_layers.length(); tar++ )
                        masks_list[tar].resize( ith_sample_in_sequence+1 );
                }

                // Forward propagation

                // Fetch right representation for input
                clamp_units(input.subVec(0,inputsize_without_masks),
                            input_layer,
                            input_symbol_sizes);                
                input_list[ith_sample_in_sequence] << input_layer->expectation;

                // Fetch right representation for target
                sum_target_elements = 0;
                for( int tar=0; tar < target_layers.length(); tar++ )
                {
                    if( use_target_layers_masks )
                    {
                        clamp_units(target.subVec(
                                        sum_target_elements,
                                        target_layers_n_of_target_elements[tar]),
                                    target_layers[tar],
                                    target_symbol_sizes[tar],
                                    input.subVec(
                                        inputsize_without_masks 
                                        + sum_target_elements, 
                                        target_layers_n_of_target_elements[tar]),
                                    masks_list[tar][ith_sample_in_sequence]
                            );

                    }
                    else
                    {
                        clamp_units(target.subVec(
                                        sum_target_elements,
                                        target_layers_n_of_target_elements[tar]),
                                    target_layers[tar],
                                    target_symbol_sizes[tar]);
                    }
                    sum_target_elements += target_layers_n_of_target_elements[tar];
                    targets_list[tar][ith_sample_in_sequence] << 
                        target_layers[tar]->expectation;
                }
                
                input_connections->fprop( input_list[ith_sample_in_sequence], 
                                          hidden_act_no_bias_list[ith_sample_in_sequence]);
                
                if( ith_sample_in_sequence > 0 )
                {
                    dynamic_connections->fprop( 
                        hidden_list[ith_sample_in_sequence-1],
                        dynamic_act_no_bias_contribution );

                    hidden_act_no_bias_list[ith_sample_in_sequence] += 
                        dynamic_act_no_bias_contribution;
                }
                 
                hidden_layer->fprop( hidden_act_no_bias_list[ith_sample_in_sequence], 
                                     hidden_list[ith_sample_in_sequence] );
                 
                if( hidden_layer2 )
                {
                    hidden_connections->fprop( 
                        hidden_list[ith_sample_in_sequence],
                        hidden2_act_no_bias_list[ith_sample_in_sequence]);

                    hidden_layer2->fprop( 
                        hidden2_act_no_bias_list[ith_sample_in_sequence],
                        hidden2_list[ith_sample_in_sequence] 
                        );

                    for( int tar=0; tar < target_layers.length(); tar++ )
                    {
                        target_connections[tar]->fprop(
                            hidden2_list[ith_sample_in_sequence],
                            target_prediction_act_no_bias_list[tar][
                                ith_sample_in_sequence]
                            );
                        target_layers[tar]->fprop(
                            target_prediction_act_no_bias_list[tar][
                                ith_sample_in_sequence],
                            target_prediction_list[tar][
                                ith_sample_in_sequence] );
                        if( use_target_layers_masks )
                            target_prediction_list[tar][ ith_sample_in_sequence] *= 
                                masks_list[tar][ith_sample_in_sequence];
                    }
                }
                else
                {
                    for( int tar=0; tar < target_layers.length(); tar++ )
                    {
                        target_connections[tar]->fprop(
                            hidden_list[ith_sample_in_sequence],
                            target_prediction_act_no_bias_list[tar][
                                ith_sample_in_sequence]
                            );
                        target_layers[tar]->fprop(
                            target_prediction_act_no_bias_list[tar][
                                ith_sample_in_sequence],
                            target_prediction_list[tar][
                                ith_sample_in_sequence] );
                        if( use_target_layers_masks )
                            target_prediction_list[tar][ ith_sample_in_sequence] *= 
                                masks_list[tar][ith_sample_in_sequence];
                    }
                }

                sum_target_elements = 0;
                for( int tar=0; tar < target_layers.length(); tar++ )
                {
                    target_layers[tar]->activation << 
                        target_prediction_act_no_bias_list[tar][
                            ith_sample_in_sequence];
                    target_layers[tar]->activation += target_layers[tar]->bias;
                    target_layers[tar]->setExpectation(
                        target_prediction_list[tar][
                            ith_sample_in_sequence]);
                    nll_list(ith_sample_in_sequence,tar) = 
                        target_layers[tar]->fpropNLL( 
                            targets_list[tar][ith_sample_in_sequence] ); 
                    train_costs[tar] += nll_list(ith_sample_in_sequence,tar);

                    // Normalize by the number of things to predict
                    if( use_target_layers_masks )
                    {
                        train_n_items[tar] += sum(
                            input.subVec( inputsize_without_masks 
                                          + sum_target_elements, 
                                          target_layers_n_of_target_elements[tar]) );
                        sum_target_elements += 
                            target_layers_n_of_target_elements[tar];
                    }
                    else
                        train_n_items[tar]++;
                }
                ith_sample_in_sequence++;
               
               
            }
            if( pb )
                pb->update( stage + 1 - init_stage);
            
            for(int i=0; i<train_costs.length(); i++)
                train_costs[i] /= train_n_items[i];

            if(verbosity>0)
                cout << "mean costs at stage " << stage << 
                    " = " << train_costs << endl;
            stage++;
            train_stats->update(train_costs);
        }    
        if( pb )
        {
            delete pb;
            pb = 0;
        }

    }


    train_stats->finalize();
}



void DynamicallyLinkedRBMsModel::clamp_units(const Vec layer_vector,
                                             PP<RBMLayer> layer,
                                             TVec<int> symbol_sizes) const
{
    int it = 0;
    int ss = -1;
    for(int i=0; i<layer_vector.length(); i++)
    {
        ss = symbol_sizes[i];
        // If input is a real ...
        if(ss < 0) 
        {
            layer->expectation[it++] = layer_vector[i];
        }
        else // ... or a symbol
        {
            // Convert to one-hot vector
            layer->expectation.subVec(it,ss).clear();
            layer->expectation[it+(int)layer_vector[i]] = 1;
            it += ss;
        }
    }
    layer->setExpectation( layer->expectation );
}

void DynamicallyLinkedRBMsModel::clamp_units(const Vec layer_vector,
                                             PP<RBMLayer> layer,
                                             TVec<int> symbol_sizes,
                                             const Vec original_mask,
                                             Vec& formated_mask) const
{
    int it = 0;
    int ss = -1;
    PLASSERT( original_mask.length() == layer_vector.length() );
    formated_mask.resize(layer->size);
    for(int i=0; i<layer_vector.length(); i++)
    {
        ss = symbol_sizes[i];
        // If input is a real ...
        if(ss < 0) 
        {
            formated_mask[it] = original_mask[i];
            layer->expectation[it++] = layer_vector[i];
        }
        else // ... or a symbol
        {
            // Convert to one-hot vector
            layer->expectation.subVec(it,ss).clear();
            formated_mask.subVec(it,ss).fill(original_mask[i]);
            layer->expectation[it+(int)layer_vector[i]] = 1;
            it += ss;
        }
    }
    layer->setExpectation( layer->expectation );
}

void DynamicallyLinkedRBMsModel::setLearningRate( real the_learning_rate )
{
    input_layer->setLearningRate( the_learning_rate );
    hidden_layer->setLearningRate( the_learning_rate );
    input_connections->setLearningRate( the_learning_rate );
    dynamic_connections->forget();
    if( hidden_layer2 )
    {
        hidden_layer2->setLearningRate( the_learning_rate );
        hidden_connections->setLearningRate( the_learning_rate );
    }

    for( int i=0; i<target_layers.length(); i++ )
    {
        target_layers[i]->setLearningRate( the_learning_rate );
        target_connections[i]->setLearningRate( the_learning_rate );
    }
}

void DynamicallyLinkedRBMsModel::recurrent_update()
{
    
        hidden_temporal_gradient.clear();
        for(int i=hidden_list.length()-1; i>=0; i--){   

            hidden_gradient.clear();
            if(use_target_layers_masks)
            {
                for( int tar=0; tar<target_layers.length(); tar++)
                {
                    target_layers[tar]->activation << target_prediction_act_no_bias_list[tar][i];
                    target_layers[tar]->activation += target_layers[tar]->bias;
                    target_layers[tar]->setExpectation(target_prediction_list[tar][i]);
                    target_layers[tar]->bpropNLL(targets_list[tar][i],nll_list(i,tar),bias_gradient);
                    bias_gradient *= target_layers_weights[tar];
                    bias_gradient *= masks_list[tar][i];
                    target_layers[tar]->update(bias_gradient);
                    target_connections[tar]->bpropUpdate(hidden2_list[i],target_prediction_act_no_bias_list[tar][i],
                                                         hidden_gradient, bias_gradient,true);
                }
            }
            else
            {
                for( int tar=0; tar<target_layers.length(); tar++)
                {
                    target_layers[tar]->activation << target_prediction_act_no_bias_list[tar][i];
                    target_layers[tar]->activation += target_layers[tar]->bias;
                    target_layers[tar]->setExpectation(target_prediction_list[tar][i]);
                    target_layers[tar]->bpropNLL(targets_list[tar][i],nll_list(i,tar),bias_gradient);
                    bias_gradient *= target_layers_weights[tar];
                    target_layers[tar]->update(bias_gradient);
                    target_connections[tar]->bpropUpdate(hidden2_list[i],target_prediction_act_no_bias_list[tar][i],
                                                         hidden_gradient, bias_gradient,true); 
                }
            }

            if (hidden_layer2)
            {
                hidden_layer2->bpropUpdate(
                    hidden2_act_no_bias_list[i], hidden2_list[i],
                    bias_gradient, hidden_gradient);
                
                hidden_connections->bpropUpdate(
                    hidden_list[i],
                    hidden2_act_no_bias_list[i], 
                    hidden_gradient, bias_gradient);
            }
            
            if(i!=0)
            {   
                hidden_gradient += hidden_temporal_gradient;
                
                hidden_layer->bpropUpdate(
                    hidden_act_no_bias_list[i], hidden_list[i],
                    hidden_temporal_gradient, hidden_gradient);
                
                dynamic_connections->bpropUpdate(
                    hidden_list[i-1],
                    hidden_act_no_bias_list[i], // Here, it should be cond_bias, but doesn't matter
                    hidden_gradient, hidden_temporal_gradient);
                
                hidden_temporal_gradient << hidden_gradient;
                
                input_connections->bpropUpdate(
                    input_list[i],
                    hidden_act_no_bias_list[i], 
                    visi_bias_gradient, hidden_temporal_gradient);// Here, it should be activations - cond_bias, but doesn't matter
                
            }
            else
            {
                hidden_layer->bpropUpdate(
                    hidden_act_no_bias_list[i], hidden_list[i],
                    hidden_temporal_gradient, hidden_gradient); // Not really temporal gradient, but this is the final iteration...
                input_connections->bpropUpdate(
                    input_list[i],
                    hidden_act_no_bias_list[i], 
                    visi_bias_gradient, hidden_temporal_gradient);// Here, it should be activations - cond_bias, but doesn't matter

                // Could learn initial value for h_{-1}
            }
        }
    
}

void DynamicallyLinkedRBMsModel::computeOutput(const Vec& input, Vec& output) const
{
    PLERROR("DynamicallyLinkedRBMsModel::computeOutput(): this is a dynamic, "
            "generative model, that can only compute negative log-likelihood "
            "costs for a whole VMat");
}

void DynamicallyLinkedRBMsModel::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    PLERROR("DynamicallyLinkedRBMsModel::computeCostsFromOutputs(): this is a "
            "dynamic, generative model, that can only compute negative "
            "log-likelihooh costs for a whole VMat");
}

void DynamicallyLinkedRBMsModel::test(VMat testset, PP<VecStatsCollector> test_stats,
                  VMat testoutputs, VMat testcosts)const
{ 

    int len = testset.length();
    Vec input;
    Vec target;
    real weight;

    Vec output(outputsize());
    Vec costs(nTestCosts());
    costs.clear();
    TVec<int> n_items(nTestCosts());
    n_items.clear();

    PP<ProgressBar> pb;
    if (report_progress) 
        pb = new ProgressBar("Testing learner", len);

    if (len == 0) {
        // Empty test set: we give -1 cost arbitrarily.
        costs.fill(-1);
        test_stats->update(costs);
    }
    
    int ith_sample_in_sequence = 0;
    int inputsize_without_masks = inputsize() 
        - ( use_target_layers_masks ? targetsize() : 0 );
    int sum_target_elements = 0;
    for (int i = 0; i < len; i++)
    {
        testset.getExample(i, input, target, weight);

        if( fast_exact_is_equal(input[0],end_of_sequence_symbol) )
        {
            ith_sample_in_sequence = 0;
            hidden_list.resize(0);
            hidden_act_no_bias_list.resize(0);
            hidden2_list.resize(0);
            hidden2_act_no_bias_list.resize(0);
            target_prediction_list.resize(0);
            target_prediction_act_no_bias_list.resize(0);
            input_list.resize(0);
            targets_list.resize(0);
            nll_list.resize(0,0);
            masks_list.resize(0);
            continue;
        }

        // Resize internal variables
        hidden_list.resize(ith_sample_in_sequence+1);
        hidden_act_no_bias_list.resize(ith_sample_in_sequence+1);
        if( hidden_layer2 )
        {
            hidden2_list.resize(ith_sample_in_sequence+1);
            hidden2_act_no_bias_list.resize(ith_sample_in_sequence+1);
        }
                 
        input_list.resize(ith_sample_in_sequence+1);
        input_list[ith_sample_in_sequence].resize(input_layer->size);

        targets_list.resize( target_layers.length() );
        target_prediction_list.resize( target_layers.length() );
        target_prediction_act_no_bias_list.resize( target_layers.length() );
        for( int tar=0; tar < target_layers.length(); tar++ )
        {
            targets_list[tar].resize( ith_sample_in_sequence+1);
            targets_list[tar][ith_sample_in_sequence].resize( 
                target_layers[tar]->size);
            target_prediction_list[tar].resize(
                ith_sample_in_sequence+1);
            target_prediction_act_no_bias_list[tar].resize(
                ith_sample_in_sequence+1);

        }
        nll_list.resize(ith_sample_in_sequence+1,target_layers.length());
        if( use_target_layers_masks )
        {
            masks_list.resize( target_layers.length() );
            for( int tar=0; tar < target_layers.length(); tar++ )
                masks_list[tar].resize( ith_sample_in_sequence+1 );
        }

        // Forward propagation

        // Fetch right representation for input
        clamp_units(input.subVec(0,inputsize_without_masks),
                    input_layer,
                    input_symbol_sizes);                
        input_list[ith_sample_in_sequence] << input_layer->expectation;

        // Fetch right representation for target
        sum_target_elements = 0;
        for( int tar=0; tar < target_layers.length(); tar++ )
        {
            if( use_target_layers_masks )
            {
                clamp_units(target.subVec(
                                sum_target_elements,
                                target_layers_n_of_target_elements[tar]),
                            target_layers[tar],
                            target_symbol_sizes[tar],
                            input.subVec(
                                inputsize_without_masks 
                                + sum_target_elements, 
                                target_layers_n_of_target_elements[tar]),
                            masks_list[tar][ith_sample_in_sequence]
                    );

            }
            else
            {
                clamp_units(target.subVec(
                                sum_target_elements,
                                target_layers_n_of_target_elements[tar]),
                            target_layers[tar],
                            target_symbol_sizes[tar]);
            }
            sum_target_elements += target_layers_n_of_target_elements[tar];
            targets_list[tar][ith_sample_in_sequence] << 
                target_layers[tar]->expectation;
        }
                
        input_connections->fprop( input_list[ith_sample_in_sequence], 
                                  hidden_act_no_bias_list[ith_sample_in_sequence]);
                
        if( ith_sample_in_sequence > 0 )
        {
            dynamic_connections->fprop( 
                hidden_list[ith_sample_in_sequence-1],
                dynamic_act_no_bias_contribution );

            hidden_act_no_bias_list[ith_sample_in_sequence] += 
                dynamic_act_no_bias_contribution;
        }
                 
        hidden_layer->fprop( hidden_act_no_bias_list[ith_sample_in_sequence], 
                             hidden_list[ith_sample_in_sequence] );
                 
        if( hidden_layer2 )
        {
            hidden_connections->fprop( 
                hidden_list[ith_sample_in_sequence],
                hidden2_act_no_bias_list[ith_sample_in_sequence]);

            hidden_layer2->fprop( 
                hidden2_act_no_bias_list[ith_sample_in_sequence],
                hidden2_list[ith_sample_in_sequence] 
                );

            for( int tar=0; tar < target_layers.length(); tar++ )
            {
                target_connections[tar]->fprop(
                    hidden2_list[ith_sample_in_sequence],
                    target_prediction_act_no_bias_list[tar][
                        ith_sample_in_sequence]
                    );
                target_layers[tar]->fprop(
                    target_prediction_act_no_bias_list[tar][
                        ith_sample_in_sequence],
                    target_prediction_list[tar][
                        ith_sample_in_sequence] );
                if( use_target_layers_masks )
                    target_prediction_list[tar][ ith_sample_in_sequence] *= 
                        masks_list[tar][ith_sample_in_sequence];
            }
        }
        else
        {
            for( int tar=0; tar < target_layers.length(); tar++ )
            {
                target_connections[tar]->fprop(
                    hidden_list[ith_sample_in_sequence],
                    target_prediction_act_no_bias_list[tar][
                        ith_sample_in_sequence]
                    );
                target_layers[tar]->fprop(
                    target_prediction_act_no_bias_list[tar][
                        ith_sample_in_sequence],
                    target_prediction_list[tar][
                        ith_sample_in_sequence] );
                if( use_target_layers_masks )
                    target_prediction_list[tar][ ith_sample_in_sequence] *= 
                        masks_list[tar][ith_sample_in_sequence];
            }
        }

        if (testoutputs)
        {
            int sum_target_layers_size = 0;
            for( int tar=0; tar < target_layers.length(); tar++ )
            {
                output.subVec(sum_target_layers_size,target_layers[tar]->size)
                    << target_prediction_list[tar][ ith_sample_in_sequence ];
                sum_target_layers_size += target_layers[tar]->size;
            }
            testoutputs->putOrAppendRow(i, output);
        }

        sum_target_elements = 0;
        for( int tar=0; tar < target_layers.length(); tar++ )
        {
            target_layers[tar]->activation << 
                target_prediction_act_no_bias_list[tar][
                    ith_sample_in_sequence];
            target_layers[tar]->activation += target_layers[tar]->bias;
            target_layers[tar]->setExpectation(
                target_prediction_list[tar][
                    ith_sample_in_sequence]);
            nll_list(ith_sample_in_sequence,tar) = 
                target_layers[tar]->fpropNLL( 
                    targets_list[tar][ith_sample_in_sequence] ); 
            costs[tar] += nll_list(ith_sample_in_sequence,tar);

            // Normalize by the number of things to predict
            if( use_target_layers_masks )
            {
                n_items[tar] += sum(
                    input.subVec( inputsize_without_masks 
                                  + sum_target_elements, 
                                  target_layers_n_of_target_elements[tar]) );
                sum_target_elements += 
                    target_layers_n_of_target_elements[tar];
            }
            else
                n_items[tar]++;
        }
        ith_sample_in_sequence++;

        if (report_progress)
            pb->update(i);

    }

    for(int i=0; i<costs.length(); i++)
        costs[i] /= n_items[i];

    if (testcosts)
        testcosts->putOrAppendRow(0, costs);
    
    if (test_stats)
        test_stats->update(costs, weight);
    
    ith_sample_in_sequence = 0;
    hidden_list.resize(0);
    hidden_act_no_bias_list.resize(0);
    hidden2_list.resize(0);
    hidden2_act_no_bias_list.resize(0);
    target_prediction_list.resize(0);
    target_prediction_act_no_bias_list.resize(0);
    input_list.resize(0);
    targets_list.resize(0);
    nll_list.resize(0,0);
    masks_list.resize(0);   
}


TVec<string> DynamicallyLinkedRBMsModel::getTestCostNames() const
{
    TVec<string> cost_names(0);
    for( int i=0; i<target_layers.length(); i++ )
        cost_names.append("target" + tostring(i) + ".NLL");
    return cost_names;
}

TVec<string> DynamicallyLinkedRBMsModel::getTrainCostNames() const
{
    return getTestCostNames();
}

//void DynamicallyLinkedRBMsModel::gen()
//{
//    //PPath* the_filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/generate/scoreGen.amat";
//    data = new AutoVMatrix();
//    data->filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/generate/scoreGen.amat";
//    data->defineSizes(21,0,0);
//    //data->inputsize = 21;
//    //data->targetsize = 0;
//    //data->weightsize = 0;
//    data->build();
//
//    
//    int len = data->length();
//    Vec score;
//    Vec target;
//    real weight;
//    Vec bias_tempo;
//    Vec visi_bias_tempo;
//   
//   
//    
//    previous_hidden_layer.resize(hidden_layer->size);
//    connections_idem = connections;
//
//    for (int ith_sample = 0; ith_sample < len ; ith_sample++ ){
//        
//        data->getExample(ith_sample, score, target, weight);
//        //score << data(ith_sample);
//        input_prediction_list.resize(
//            ith_sample+1,visible_layer->size);
//        if(ith_sample > 0)
//        {
//            
//            //input_list(ith_sample_in_sequence) << previous_input;
//            //h*_{t-1}
//            //////////////////////////////////
//            dynamic_connections->fprop(previous_hidden_layer, cond_bias);
//            hidden_layer->setAllBias(cond_bias); //**************************
//            
//            
//            
//            //up phase
//            connections->setAsDownInput( input_prediction_list(ith_sample-1) );
//            hidden_layer->getAllActivations( connections_idem );
//            hidden_layer->computeExpectation();
//            //////////////////////////////////
//            
//            //previous_hidden_layer << hidden_layer->expectation;//h_{t-2} au prochain tour//******************************
//            //previous_hidden_layer_act_no_bias << hidden_layer->activation;
//            
//            
//            //h*_{t}
//            ////////////
//            if(dynamic_connections_copy)
//                dynamic_connections_copy->fprop( hidden_layer->expectation ,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
//            else
//                dynamic_connections->fprop( hidden_layer->expectation ,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
//            //dynamic_connections_copy->fprop( hidden_layer->expectation ,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
//            hidden_layer->expectation_is_not_up_to_date();
//            hidden_layer->computeExpectation();//h_{t}
//            ///////////
//            
//            //previous_input << visible_layer->expectation;//v_{t-1}
//            
//        }
//        else
//        {
//            
//            previous_hidden_layer.clear();//h_{t-1}
//            if(dynamic_connections_copy)
//                dynamic_connections_copy->fprop( previous_hidden_layer ,
//                                                 hidden_layer->activation);//conection entre h_{t-1} et h_{t}
//            else
//                dynamic_connections->fprop(previous_hidden_layer,
//                                           hidden_layer->activation);//conection entre h_{t-1} et h_{t}
//            
//            hidden_layer->expectation_is_not_up_to_date();
//            hidden_layer->computeExpectation();//h_{t}
//            //previous_input.resize(data->inputsize);
//            //previous_input << data(ith_sample);
//            
//        }
//        
//        //connections_transpose->setAsDownInput( hidden_layer->expectation );
//        //visible_layer->getAllActivations( connections_idem_t );
//        
//        connections->setAsUpInput( hidden_layer->expectation );
//        visible_layer->getAllActivations( connections_idem );
//        
//        visible_layer->computeExpectation();
//        //visible_layer->generateSample();
//        partition(score.subVec(14,taillePart), visible_layer->activation.subVec(14+taillePart,taillePart), visible_layer->activation.subVec(14+(taillePart*2),taillePart));
//        partition(score.subVec(14,taillePart), visible_layer->expectation.subVec(14+taillePart,taillePart), visible_layer->expectation.subVec(14+(taillePart*2),taillePart));
//
//
//        visible_layer->activation.subVec(0,14+taillePart) << score;
//        visible_layer->expectation.subVec(0,14+taillePart) << score;
//
//        input_prediction_list(ith_sample) << visible_layer->expectation;
//        
//    }
//    
//    //Vec tempo;
//    TVec<real> tempo;
//    tempo.resize(visible_layer->size);
//    ofstream myfile;
//    myfile.open ("/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/generate/test.txt");
//    
//    for (int i = 0; i < len ; i++ ){
//        tempo << input_prediction_list(i);
//        
//        //cout << tempo[2] << endl;
//       
//        for (int j = 0; j < tempo.length() ; j++ ){
//            
//            
//                
//                
//               myfile << tempo[j] << " ";
//               
//
//               
//           
//        }
//        myfile << "\n";
//    }
//     
//
//     myfile.close();
//
//}
//void DynamicallyLinkedRBMsModel::generate(int nbNotes)
//{
//    
//    previous_hidden_layer.resize(hidden_layer->size);
//    connections_idem = connections;
//
//    for (int ith_sample = 0; ith_sample < nbNotes ; ith_sample++ ){
//        
//        input_prediction_list.resize(
//            ith_sample+1,visible_layer->size);
//        if(ith_sample > 0)
//        {
//            
//            //input_list(ith_sample_in_sequence) << previous_input;
//            //h*_{t-1}
//            //////////////////////////////////
//            dynamic_connections->fprop(previous_hidden_layer, cond_bias);
//            hidden_layer->setAllBias(cond_bias); //**************************
//            
//            
//            
//            //up phase
//            connections->setAsDownInput( input_prediction_list(ith_sample-1) );
//            hidden_layer->getAllActivations( connections_idem );
//            hidden_layer->computeExpectation();
//            //////////////////////////////////
//            
//            //previous_hidden_layer << hidden_layer->expectation;//h_{t-2} au prochain tour//******************************
//            //previous_hidden_layer_act_no_bias << hidden_layer->activation;
//            
//            
//            //h*_{t}
//            ////////////
//            if(dynamic_connections_copy)
//                dynamic_connections_copy->fprop( hidden_layer->expectation ,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
//            else
//                dynamic_connections->fprop( hidden_layer->expectation ,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
//            //dynamic_connections_copy->fprop( hidden_layer->expectation ,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
//            hidden_layer->expectation_is_not_up_to_date();
//            hidden_layer->computeExpectation();//h_{t}
//            ///////////
//            
//            //previous_input << visible_layer->expectation;//v_{t-1}
//            
//        }
//        else
//        {
//            
//            previous_hidden_layer.clear();//h_{t-1}
//            if(dynamic_connections_copy)
//                dynamic_connections_copy->fprop( previous_hidden_layer ,
//                                                 hidden_layer->activation);//conection entre h_{t-1} et h_{t}
//            else
//                dynamic_connections->fprop(previous_hidden_layer,
//                                           hidden_layer->activation);//conection entre h_{t-1} et h_{t}
//            
//            hidden_layer->expectation_is_not_up_to_date();
//            hidden_layer->computeExpectation();//h_{t}
//            
//            
//        }
//        
//        //connections_transpose->setAsDownInput( hidden_layer->expectation );
//        //visible_layer->getAllActivations( connections_idem_t );
//        
//        connections->setAsUpInput( hidden_layer->expectation );
//        visible_layer->getAllActivations( connections_idem );
//        
//        visible_layer->computeExpectation();
//        visible_layer->generateSample();
//        
//        input_prediction_list(ith_sample) << visible_layer->sample;
//        
//    }
//    
//    //Vec tempo;
//    TVec<int> tempo;
//    tempo.resize(visible_layer->size);
//    int theNote;
//    //int nbNoteVisiLayer = input_prediction_list(1).length()/13;
//    ofstream myfile;
//    int theLayer;
//    myfile.open ("/home/stan/Documents/recherche_maitrise/DDBN_musicGeneration/data/generate/test.txt");
//    
//    for (int i = 0; i < nbNotes ; i++ ){
//        tempo << input_prediction_list(i);
//        
//        //cout << tempo[2] << endl;
//       
//        for (int j = 0; j < tempo.length() ; j++ ){
//            
//            if (tempo[j] == 1){
//                theLayer = (j/13);
//                
//                theNote = j - (13*theLayer);
//               
//
//                if (theNote<=11){
//                    //print theNote
//                    //cout << theNote+50 << " ";
//                    myfile << theNote << " ";
//                }
//                else{
//                    //print #
//                    //cout << "# ";
//                    myfile << "# ";
//                    
//                }
//     
//            }
//           
//        }
//        myfile << "\n";
//    }
//     myfile << "<oov> <oov> \n";
//
//     myfile.close();
//
//}

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
