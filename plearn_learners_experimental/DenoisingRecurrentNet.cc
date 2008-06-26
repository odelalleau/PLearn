// -*- C++ -*-

// DenoisingRecurrentNet.cc
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

/*! \file DenoisingRecurrentNet.cc */


#define PL_LOG_MODULE_NAME "DenoisingRecurrentNet"
#include <plearn/io/pl_log.h>

#include "DenoisingRecurrentNet.h"
#include "plearn/math/plapack.h"

// - commiter mse
// - ajouter denoising recurrent net. Deux possibilités:
//   1) on ajoute du bruit à l'input, et on reconstruit les targets avec des poids
//      possiblement différents
//     * option denoising_target_layers_weights (c'est là qu'on met l'input)
//     * version de clamp_units qui ajoute le bruit
//   2) on reconstruit l'input directement (sans 2e couche cachée)
//     * toujours clamp_units qui ajoute le bruit
//     * une option qui dit quelle partie de l'input reconstruire et du code 
//       pour bloquer le gradient qui ne doit pas passer (pas très propre, 
//       mais bon...)
//     * une option donnant les connections de reconstruction
//     * du code pour entraîner séparément les hidden_connections (si présentes)
// - pourrait avoir le gradient du denoising recurrent net en même temps que
//   celui du "fine-tuning"
// - add dynamic_activations_list and use it in recurrent_update


namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DenoisingRecurrentNet,
    "Model made of RBMs linked through time",
    ""
    );


DenoisingRecurrentNet::DenoisingRecurrentNet() :
    //rbm_learning_rate( 0.01 ),
    recurrent_net_learning_rate( 0.01),
    use_target_layers_masks( false ),
    end_of_sequence_symbol( -1000 )
    //rbm_nstages( 0 ),
{
    random_gen = new PRandom();
}

void DenoisingRecurrentNet::declareOptions(OptionList& ol)
{
//    declareOption(ol, "rbm_learning_rate", &DenoisingRecurrentNet::rbm_learning_rate,
//                  OptionBase::buildoption,
//                  "The learning rate used during RBM contrastive "
//                  "divergence learning phase.\n");

    declareOption(ol, "recurrent_net_learning_rate", 
                  &DenoisingRecurrentNet::recurrent_net_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the recurrent phase.\n");

//    declareOption(ol, "rbm_nstages", &DenoisingRecurrentNet::rbm_nstages,
//                  OptionBase::buildoption,
//                  "Number of epochs for rbm phase.\n");


    declareOption(ol, "target_layers_weights", 
                  &DenoisingRecurrentNet::target_layers_weights,
                  OptionBase::buildoption,
                  "The training weights of each target layers.\n");

    declareOption(ol, "use_target_layers_masks", 
                  &DenoisingRecurrentNet::use_target_layers_masks,
                  OptionBase::buildoption,
                  "Indication that a mask indicating which target to predict\n"
                  "is present in the input part of the VMatrix dataset.\n");

    declareOption(ol, "end_of_sequence_symbol", 
                  &DenoisingRecurrentNet::end_of_sequence_symbol,
                  OptionBase::buildoption,
                  "Value of the first input component for end-of-sequence "
                  "delimiter.\n");

    declareOption(ol, "input_layer", &DenoisingRecurrentNet::input_layer,
                  OptionBase::buildoption,
                  "The input layer of the model.\n");

    declareOption(ol, "target_layers", &DenoisingRecurrentNet::target_layers,
                  OptionBase::buildoption,
                  "The target layers of the model.\n");

    declareOption(ol, "hidden_layer", &DenoisingRecurrentNet::hidden_layer,
                  OptionBase::buildoption,
                  "The hidden layer of the model.\n");

    declareOption(ol, "hidden_layer2", &DenoisingRecurrentNet::hidden_layer2,
                  OptionBase::buildoption,
                  "The second hidden layer of the model (optional).\n");

    declareOption(ol, "dynamic_connections", 
                  &DenoisingRecurrentNet::dynamic_connections,
                  OptionBase::buildoption,
                  "The RBMConnection between the first hidden layers, "
                  "through time (optional).\n");

    declareOption(ol, "hidden_connections", 
                  &DenoisingRecurrentNet::hidden_connections,
                  OptionBase::buildoption,
                  "The RBMConnection between the first and second "
                  "hidden layers (optional).\n");

    declareOption(ol, "input_connections", 
                  &DenoisingRecurrentNet::input_connections,
                  OptionBase::buildoption,
                  "The RBMConnection from input_layer to hidden_layer.\n");

    declareOption(ol, "target_connections", 
                  &DenoisingRecurrentNet::target_connections,
                  OptionBase::buildoption,
                  "The RBMConnection from input_layer to hidden_layer.\n");

    /*
    declareOption(ol, "", 
                  &DenoisingRecurrentNet::,
                  OptionBase::buildoption,
                  "");
    */


    declareOption(ol, "target_layers_n_of_target_elements", 
                  &DenoisingRecurrentNet::target_layers_n_of_target_elements,
                  OptionBase::learntoption,
                  "Number of elements in the target part of a VMatrix associated\n"
                  "to each target layer.\n");

    declareOption(ol, "input_symbol_sizes", 
                  &DenoisingRecurrentNet::input_symbol_sizes,
                  OptionBase::learntoption,
                  "Number of symbols for each symbolic field of train_set.\n");

    declareOption(ol, "target_symbol_sizes", 
                  &DenoisingRecurrentNet::target_symbol_sizes,
                  OptionBase::learntoption,
                  "Number of symbols for each symbolic field of train_set.\n");

    /*
    declareOption(ol, "", &DenoisingRecurrentNet::,
                  OptionBase::learntoption,
                  "");
     */

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DenoisingRecurrentNet::build_()
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
        PLASSERT( target_connections.length() == target_layers.length() );
        PLASSERT( target_layers.length() > 0 );
        PLASSERT( input_layer );
        PLASSERT( hidden_layer );
        PLASSERT( input_connections );

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
                    PLERROR("DenoisingRecurrentNet::build_(): dictionary "
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
            PLERROR("DenoisingRecurrentNet::build_(): input_layer->size %d "
                    "should be %d", input_layer->size, input_layer_size);

        // Parsing symbols in target
        int tar_layer = 0;
        int tar_layer_size = 0;
        target_symbol_sizes.resize(target_layers.length());
        for( int tar_layer=0; tar_layer<target_layers.length(); 
             tar_layer++ )
            target_symbol_sizes[tar_layer].resize(0);
        target_layers_n_of_target_elements.resize( targetsize() );
        target_layers_n_of_target_elements.clear();

        for( int tar=0; tar<targetsize(); tar++)
        {
            if( tar_layer > target_layers.length() )
                PLERROR("DenoisingRecurrentNet::build_(): target layers "
                        "does not cover all targets.");            

            dict = train_set->getDictionary(tar+inputsize());
            if(dict)
            {
                if( use_target_layers_masks )
                    PLERROR("DenoisingRecurrentNet::build_(): masks for "
                            "symbolic targets is not implemented.");
                if( dict->size() == 0 )
                    PLERROR("DenoisingRecurrentNet::build_(): dictionary "
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
            PLERROR("DenoisingRecurrentNet::build_(): target layers "
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


        if( dynamic_connections )
        {
            dynamic_connections->down_size = hidden_layer->size;
            dynamic_connections->up_size = hidden_layer->size;
            if( !dynamic_connections->random_gen )
            {
                dynamic_connections->random_gen = random_gen;
                dynamic_connections->forget();
            }
            dynamic_connections->build();
        }

        if( hidden_layer2 )
        {
            if( !hidden_layer2->random_gen )
            {
                hidden_layer2->random_gen = random_gen;
                hidden_layer2->forget();
            }

            PLASSERT( hidden_connections );

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
            PLASSERT( target_layers[tar_layer] );
            PLASSERT( target_connections[tar_layer] );

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
void DenoisingRecurrentNet::build()
{
    inherited::build();
    build_();
}


void DenoisingRecurrentNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
    deepCopyField( visi_bias_gradient , copies);
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

    //PLERROR("DenoisingRecurrentNet::makeDeepCopyFromShallowCopy(): "
    //"not implemented yet");
}


int DenoisingRecurrentNet::outputsize() const
{
    int out_size = 0;
    for( int i=0; i<target_layers.length(); i++ )
        out_size += target_layers[i]->size;
    return out_size;
}

void DenoisingRecurrentNet::forget()
{
    inherited::forget();

    input_layer->forget();
    hidden_layer->forget();
    input_connections->forget();
    if( dynamic_connections )
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

void DenoisingRecurrentNet::train()
{
    MODULE_LOG << "train() called " << endl;

    // reserve memory for sequences
    seq.resize(5000,2); // contains the current sequence
    encoded_seq.resize(5000, 4);

    Vec input( inputsize() );
    Vec target( targetsize() );
    real weight = 0; // Unused
    Vec train_costs( getTrainCostNames().length() );
    train_costs.clear();
    Vec train_n_items( getTrainCostNames().length() );

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

        // TO DO: check this line
        setLearningRate( recurrent_net_learning_rate );

        int ith_sample_in_sequence = 0;
        int inputsize_without_masks = inputsize() 
            - ( use_target_layers_masks ? targetsize() : 0 );
        int sum_target_elements = 0;

        while(stage < end_stage)
        {
            train_costs.clear();
            train_n_items.clear();

            int nseq = nSequences();
            for(int i=0; i<nseq; i++)
            {
                getSequence(i, seq);
                if(encoding=="raw_masked_supervised")
                {
                    splitMaskedSupervisedSequence(seq);
                }
                else
                {
                    encodeSequence(seq, encoded_seq);
                    createSupervisedSequence(encoded_seq);
                }

                resize_lists();
                fprop(train_costs, train_n_items);
                recurrent_update();
            }

            if( pb )
                pb->update( stage + 1 - init_stage);
            
            for(int i=0; i<train_costs.length(); i++)
            {
                if( !fast_exact_is_equal(target_layers_weights[i],0) )
                    train_costs[i] /= train_n_items[i];
                else
                    train_costs[i] = MISSING_VALUE;
            }

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

// TO DO: penser a gestion des prepended dans ce cas
// Populates: inputslist, targets_list, masks_list
void DenoisingRecurrentNet::createSupervisedSequence(Mat encoded_seq)
{
    PLERROR("Not implemented yet");
}



// TO DO: penser a prepend dans ce cas

// Populates: input_list, targets_list, masks_list
void DenoisingRecurrentNet::splitMaskedSupervisedSequence(Mat seq)
{
    int inputsize_without_masks = inputsize()-targetsize();
    Mat input_part = seq.subMatColumns(0,inputsize_without_masks);
    Mat mask_part = seq.subMatColumns(inputsize_without_masks, targetsize());
    Mat target_part = seq.subMatColumns(inputsize_without_masks+targetsize(), targetsize());

    int l = input_part.length();
    input_list.resize(l);
    for(int i=0; i<l; i++)
        input_list[i] = input_part(i);

    int ntargets = target_layers.length();
    targets_list.resize(ntagets);
    masks_list.resize(ntargets);
    int startcol = 0; // starting column of next target in target_part and mask_part
    for(int k=0; k<ntargets; k++)
    {
        int targsize = target_layers[k]->size;
        targets_list[k] = target_part.subMatColumns(startcol, targsize);
        masks_list[k] = mask_part.subMatColumns(startcol, targsize);
        startcol += targsize;
    }
}


void DenoisingRecurrentNet::resize_lists()
{
    int l = input_list.length();

    hidden_list.resize(l, hidden_layer->size);
    hidden_act_no_bias_list.resize(l, hidden_layer->size);

    if( hidden_layer2 )
    {
        hidden2_list.resize(l, hidden_layer2->size);
        hidden2_act_no_bias_list.resize(l, hidden_layer2->size);
    }

    int ntargets = target_layers.length();
    target_prediction_list.resize( ntargets );
    target_prediction_act_no_bias_list.resize( ntargets );

    for( int tar=0; tar < ntargets; tar++ )
    {
        int targsize = target_layers[k]->size;
        target_prediction_list[tar].resize(l, targsize);
        target_prediction_act_no_bias_list[tar].resize(l, targsize);
    }

    nll_list.resize(l,ntargets);
}

// TODO: think properly about prepended stuff

// fprop accumulates costs in costs and n_items in n_items
void DenoisingRecurrentNet::fprop(Vec train_costs, Vec train_n_items)
{
    int l = input_list.length();
    int ntargets = target_layers.length();

    for(int i=0; i<input_list.length(); i++ )
    {
        Vec hidden_act_no_bias_i = hidden_act_no_bias_list(i);
        input_connections->fprop( input_list[i], hidden_act_no_bias_i);

        if( i > 0 && dynamic_connections )
        {
            Vec hidden_i_prev = hidden_list(i-1);
            dynamic_connections->fprop(hidden_i_prev,dynamic_act_no_bias_contribution );
            hidden_act_no_bias_i += dynamic_act_no_bias_contribution;
        }
        
        Vec hidden_i = hidden_list(i);
        hidden_layer->fprop( hidden_act_no_bias_i, 
                             hidden_i);
        
        Vec last_hidden = hidden_i;
                 
        if( hidden_layer2 )
        {
            Vec hidden2_i = hidden2_list(i); 
            Vec hidden2_act_no_bias_i = hidden2_act_no_bias_list(i);

            hidden_connections->fprop(hidden_i, hidden2_act_no_bias_i);            
            hidden_layer2->fprop(hidden2_act_no_bias_i, hidden2_i);

            last_hidden = hidden2_i; // last hidden layer vec 
        }

        for( int tar=0; tar < ntargets; tar++ )
        {
            if( !fast_exact_is_equal(target_layers_weights[tar],0) )
            {
                Vec target_prediction_i = target_prediction_list[tar](i);
                Vec target_prediction_act_no_bias_i = target_prediction_act_no_bias_i;
                target_connections[tar]->fprop(last_hidden, target_prediction_act_no_bias_list_i);
                target_layers[tar]->fprop(target_prediction_act_no_bias_i, target_prediction_list_i);
                if( use_target_layers_masks )
                    target_prediction_i *= masks_list[tar](i);
            }
        }

        sum_target_elements = 0;
        for( int tar=0; tar < ntargets; tar++ )
        {
            if( !fast_exact_is_equal(target_layers_weights[tar],0) )
            {
                target_layers[tar]->activation << target_prediction_act_no_bias_list[tar](i);
                target_layers[tar]->activation += target_layers[tar]->bias;
                target_layers[tar]->setExpectation(target_prediction_list[tar](i));
                Vec target_vec = targets_list[tar](i);
                nll_list(i,tar) = target_layers[tar]->fpropNLL(target_vec); 
                train_costs[tar] += nll_list(i,tar);
                        
                // Normalize by the number of things to predict
                if( use_target_layers_masks )
                {
                    train_n_items[tar] += sum(
                        input.subVec( inputsize_without_masks 
                                      + sum_target_elements, 
                                      target_layers_n_of_target_elements[tar]) );
                }
                else
                    train_n_items[tar]++;
            }
            if( use_target_layers_masks )
                sum_target_elements += target_layers_n_of_target_elements[tar];
                    
        }
    }
}

/*
input_list
targets_list
masks_list
hidden_list
hidden_act_no_bias_list
hidden2_list
hidden2_act_no_bias_list
target_prediction_list
target_prediction_act_no_bias_list
nll_list
*/

void DenoisingRecurrentNet::recurrent_update()
{
    hidden_temporal_gradient.resize(hidden_layer->size);
    hidden_temporal_gradient.clear();
    for(int i=hidden_list.length()-1; i>=0; i--){   

        if( hidden_layer2 )
            hidden_gradient.resize(hidden_layer2->size);
        else
            hidden_gradient.resize(hidden_layer->size);
        hidden_gradient.clear();
        if(use_target_layers_masks)
        {
            for( int tar=0; tar<target_layers.length(); tar++)
            {
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
                {
                    target_layers[tar]->activation << target_prediction_act_no_bias_list[tar](i);
                    target_layers[tar]->activation += target_layers[tar]->bias;
                    target_layers[tar]->setExpectation(target_prediction_list[tar](i));
                    target_layers[tar]->bpropNLL(targets_list[tar](i),nll_list(i,tar),bias_gradient);
                    bias_gradient *= target_layers_weights[tar];
                    bias_gradient *= masks_list[tar](i);
                    target_layers[tar]->update(bias_gradient);
                    if( hidden_layer2 )
                        target_connections[tar]->bpropUpdate(hidden2_list(i),target_prediction_act_no_bias_list[tar](i),
                                                             hidden_gradient, bias_gradient,true);
                    else
                        target_connections[tar]->bpropUpdate(hidden_list(i),target_prediction_act_no_bias_list[tar](i),
                                                             hidden_gradient, bias_gradient,true);
                }
            }
        }
        else
        {
            for( int tar=0; tar<target_layers.length(); tar++)
            {
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
                {
                    target_layers[tar]->activation << target_prediction_act_no_bias_list[tar](i);
                    target_layers[tar]->activation += target_layers[tar]->bias;
                    target_layers[tar]->setExpectation(target_prediction_list[tar](i));
                    target_layers[tar]->bpropNLL(targets_list[tar](i),nll_list(i,tar),bias_gradient);
                    bias_gradient *= target_layers_weights[tar];
                    target_layers[tar]->update(bias_gradient);
                    if( hidden_layer2 )
                        target_connections[tar]->bpropUpdate(hidden2_list(i),target_prediction_act_no_bias_list[tar](i),
                                                             hidden_gradient, bias_gradient,true); 
                    else
                        target_connections[tar]->bpropUpdate(hidden_list(i),target_prediction_act_no_bias_list[tar](i),
                                                             hidden_gradient, bias_gradient,true); 
                        
                }
            }
        }

        if (hidden_layer2)
        {
            hidden_layer2->bpropUpdate(
                hidden2_act_no_bias_list(i), hidden2_list(i),
                bias_gradient, hidden_gradient);
                
            hidden_connections->bpropUpdate(
                hidden_list(i),
                hidden2_act_no_bias_list(i), 
                hidden_gradient, bias_gradient);
        }
            
        if(i!=0 && dynamic_connections )
        {   
            hidden_gradient += hidden_temporal_gradient;
                
            hidden_layer->bpropUpdate(
                hidden_act_no_bias_list(i), hidden_list(i),
                hidden_temporal_gradient, hidden_gradient);
                
            dynamic_connections->bpropUpdate(
                hidden_list[i-1],
                hidden_act_no_bias_list(i), // Here, it should be cond_bias, but doesn't matter
                hidden_gradient, hidden_temporal_gradient);
                
            hidden_temporal_gradient << hidden_gradient;
                
            input_connections->bpropUpdate(
                input_list(i),
                hidden_act_no_bias_list(i), 
                visi_bias_gradient, hidden_temporal_gradient);// Here, it should be activations - cond_bias, but doesn't matter
                
        }
        else
        {
            hidden_layer->bpropUpdate(
                hidden_act_no_bias_list(i), hidden_list(i),
                hidden_temporal_gradient, hidden_gradient); // Not really temporal gradient, but this is the final iteration...
            input_connections->bpropUpdate(
                input_list(i),
                hidden_act_no_bias_list(i), 
                visi_bias_gradient, hidden_temporal_gradient);// Here, it should be activations - cond_bias, but doesn't matter

        }
    }
    
}

/*
void DenoisingRecurrentNet::old_recurrent_update()
{
    hidden_temporal_gradient.resize(hidden_layer->size);
    hidden_temporal_gradient.clear();
    for(int i=hidden_list.length()-1; i>=0; i--){   

        if( hidden_layer2 )
            hidden_gradient.resize(hidden_layer2->size);
        else
            hidden_gradient.resize(hidden_layer->size);
        hidden_gradient.clear();
        if(use_target_layers_masks)
        {
            for( int tar=0; tar<target_layers.length(); tar++)
            {
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
                {
                    target_layers[tar]->activation << target_prediction_act_no_bias_list[tar][i];
                    target_layers[tar]->activation += target_layers[tar]->bias;
                    target_layers[tar]->setExpectation(target_prediction_list[tar][i]);
                    target_layers[tar]->bpropNLL(targets_list[tar][i],nll_list(i,tar),bias_gradient);
                    bias_gradient *= target_layers_weights[tar];
                    bias_gradient *= masks_list[tar][i];
                    target_layers[tar]->update(bias_gradient);
                    if( hidden_layer2 )
                        target_connections[tar]->bpropUpdate(hidden2_list[i],target_prediction_act_no_bias_list[tar][i],
                                                             hidden_gradient, bias_gradient,true);
                    else
                        target_connections[tar]->bpropUpdate(hidden_list[i],target_prediction_act_no_bias_list[tar][i],
                                                             hidden_gradient, bias_gradient,true);
                }
            }
        }
        else
        {
            for( int tar=0; tar<target_layers.length(); tar++)
            {
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
                {
                    target_layers[tar]->activation << target_prediction_act_no_bias_list[tar][i];
                    target_layers[tar]->activation += target_layers[tar]->bias;
                    target_layers[tar]->setExpectation(target_prediction_list[tar][i]);
                    target_layers[tar]->bpropNLL(targets_list[tar][i],nll_list(i,tar),bias_gradient);
                    bias_gradient *= target_layers_weights[tar];
                    target_layers[tar]->update(bias_gradient);
                    if( hidden_layer2 )
                        target_connections[tar]->bpropUpdate(hidden2_list[i],target_prediction_act_no_bias_list[tar][i],
                                                             hidden_gradient, bias_gradient,true); 
                    else
                        target_connections[tar]->bpropUpdate(hidden_list[i],target_prediction_act_no_bias_list[tar][i],
                                                             hidden_gradient, bias_gradient,true); 
                        
                }
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
            
        if(i!=0 && dynamic_connections )
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

        }
    }
    
}
*/


/*
void DenoisingRecurrentNet::oldtrain()
{
    MODULE_LOG << "train() called " << endl;

    Vec input( inputsize() );
    Vec target( targetsize() );
    real weight = 0; // Unused
    Vec train_costs( getTrainCostNames().length() );
    train_costs.clear();
    Vec train_n_items( getTrainCostNames().length() );

    if( !initTrain() )
    {
        MODULE_LOG << "train() aborted" << endl;
        return;
    }

    ProgressBar* pb = 0;

    // clear stats of previous epoch
    train_stats->forget();


//    if(rbm_stage < rbm_nstages)
//    {
//    }


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
                    if( !fast_exact_is_equal(target_layers_weights[tar],0) )
                    {                        
                        targets_list[tar].resize( ith_sample_in_sequence+1);
                        targets_list[tar][ith_sample_in_sequence].resize( 
                            target_layers[tar]->size);
                        target_prediction_list[tar].resize(
                            ith_sample_in_sequence+1);
                        target_prediction_act_no_bias_list[tar].resize(
                            ith_sample_in_sequence+1);
                    }
                }
                nll_list.resize(ith_sample_in_sequence+1,target_layers.length());
                if( use_target_layers_masks )
                {
                    masks_list.resize( target_layers.length() );
                    for( int tar=0; tar < target_layers.length(); tar++ )
                        if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
                    if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
                        targets_list[tar][ith_sample_in_sequence] << 
                            target_layers[tar]->expectation;
                    }
                    sum_target_elements += target_layers_n_of_target_elements[tar];
                }
                
                input_connections->fprop( input_list[ith_sample_in_sequence], 
                                          hidden_act_no_bias_list[ith_sample_in_sequence]);
                
                if( ith_sample_in_sequence > 0 && dynamic_connections )
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
                        if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
                }
                else
                {
                    for( int tar=0; tar < target_layers.length(); tar++ )
                    {
                        if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
                }

                sum_target_elements = 0;
                for( int tar=0; tar < target_layers.length(); tar++ )
                {
                    if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
                        }
                        else
                            train_n_items[tar]++;
                    }
                    if( use_target_layers_masks )
                        sum_target_elements += 
                            target_layers_n_of_target_elements[tar];
                    
                }
                ith_sample_in_sequence++;
            }
            if( pb )
                pb->update( stage + 1 - init_stage);
            
            for(int i=0; i<train_costs.length(); i++)
            {
                if( !fast_exact_is_equal(target_layers_weights[i],0) )
                    train_costs[i] /= train_n_items[i];
                else
                    train_costs[i] = MISSING_VALUE;
            }

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
*/

/* TO DO:
verifier nombre de temps
implementer correctmeent duration_to_number_of_timeframes(duration)
declare nouvelles options et valeurs par defaut correctes
*/


/*
Format de donnees:

matrice de 2 colonnes:
note, duree

note: midi_number (21..108 numero de touche sur piano)
      ou 0  (silence)
      ou -1 (missing)
      ou -999 (fin de sequence)

duree: 1 double-croche
       2 
..16   exprimee en 1/16 de mesure (resultat du quantize de Stan)


 */

void DenoisingRecurrentNet::encodeSequence(Mat sequence, Mat& encoded_sequence)
{
    //! Possibilities: "timeframe", "note_duration", "note_octav_duration", "generic"
    int prepend_zero_rows = input_window_size;

    if(encoding=="timeframe")
        encode_onehot_timeframe(sequence, encoded_sequence, prepend_zero_rows);
    else if(encoding=="note_duration")
        encode_onehot_note_octav_duration(sequence, encoded_sequence, prepend_zero_rows);
    else if(encoding=="note_octav_duration")
        encode_onehot_note_octav_duration(sequence, encoded_sequence, prepend_zero_rows, true, 4);    
    else if(encoding=="raw_masked_supervised")
        PLERROR("raw_masked_supervised encoding not yet implemented");
    else if(encoding=="generic")
        PLERROR("generic encoding not yet implemented");
    else
        PLERROR("unsupported encoding: %s",encoding.c_str());
}


void DenoisingRecurrentNet::getSequence(int i, Mat& seq) const
{ 
    int start = 0;
    if(i>0)
        start = trainset_boundaries[i-1]+1;
    int end = trainset_boundaries[i];
    int w = train_set->width();
    seq.resize(end-start, w);
    train_set->getMat(start,0,seq);
}


void DenoisingRecurrentNet::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set, call_forget);
    locateSequenceBoundaries(training_set, trainset_boundaries, end_of_sequence_symbol);
}


void DenoisingRecurrentNet::locateSequenceBoundaries(VMat dataset, TVec<int>& boundaries, real end_of_sequence_symbol)
{
    boundaries.resize(0);
    int l = dataset->length();
    for(int i=0; i<l; i++)
    {
        if(dataset(i,0)==end_of_sequence_symbol)
            boundaries.append(i);
    }
}




// encodings


/*
  use note_nbits=13 bits for note + octav_nbits bits for octav + duration_nbits bits for duration
  bit positions are numbered starting at 0.

  if note is a silence (midi_number==0) then bit at position 12 is on
  otherwise bit at position midi_number%12 is on

  To compute octav bit position, we first compute the min and max of midi_number/12
  this gives us the octav_min.
  Then bit at position note_nbits+(midi_number/12)-octav_min is switched to on.

  bit at position note_nbits+octav_nbits+duration is on
 */

void DenoisingRecurrentNet::encode_onehot_note_octav_duration(Mat sequence, Mat& encoded_sequence, int prepend_zero_rows,
                                                              bool use_silence, in octav_nbits, int duration_nbits)
{
    int l = sequence.length();
    encoded_sequence.resize(prepend_zero_rows+l,note_nbits+octav_nbits+duration_nbits);
    encoded_sequence.clear();
    int octav_min = 10000;
    int octav_max = -10000;

    int note_nbits = use_silence ?13 :12;

    if(octav_nbits>0)
    {
        for(int i=0; i<l; i++)
        {
            int midi_number = int(sequence(i,0));
            int octav = midi_number/12;
            if(octav<octav_min)
                octav_min = octav;
            if(octav>octav_max)
                octav_max = octav;
        }
        if(octav_max-octav_min > octav_nbits)
            PLERROR("Octav range too big. Does not fit in octav_nbits");
    }

    
    for(int i=0; i<l; i++)
    {
        int midi_number = int(sequence(i,0));
        if(midi_number==0) // silence
        {
            if(use_silence)
                encoded_sequence(prepend_zero_rows+i,12) = 1;
        }
        else
            encoded_sequence(prepend_zero_rows+i,midi_number%12) = 1;

        if(octav_nbits>0)
        {
            int octavpos = midi_number/12-octav_min;
            encoded_sequence(prepend_zero_rows+i,note_nbits+octavpos) = 1;
        }

        int duration = int(sequence(i,1));
        if(duration<0 || duration>=duration_nbits)
            PLERROR("duration out of valid range");
        encoded_sequence(prepend_zero_rows+i,note_nbits+octav_nbits+duration) = 1;
    }
}


int DenoisingRecurrentNet::duration_to_number_of_timeframes(int duration)
{
    return duration+1;
}

/*
  use note_nbits+1 bits for note at every timeframe
  last bit indicates continuation of the preceeding note.
 */

void DenoisingRecurrentNet::encode_onehot_timeframe(Mat sequence, Mat& encoded_sequence, 
                                                    int prepend_zero_rows, bool use_silence)
{
    int l = sequence.length();
    int newl = 0;

    // First compute length of timeframe sequence
    for(int i=0; i<l; i++)
    {
        int duration = int(sequence(i,1));
        newl += duration_to_number_of_timeframes(duration);
    }

    int nnotes = use_silence ?13 :12;

    // reserve one extra bit to mean repetition
    encoded_sequence.resize(prepend_zero_rows+newl, nnotes+1);
    encoded_sequence.clear();

    int k=prepend_zero_rows;
    for(int i=0; i<l; i++)
    {
        int midi_number = int(sequence(i,0));
        if(midi_number==0) // silence
        {
            if(use_silence)
                encoded_sequence(k++,12) = 1;
        }
        else
            encoded_sequence(k++,midi_number%12) = 1;

        int duration = int(sequence(i,1));
        int nframes = duration_to_number_of_timeframes(duration);
        while(--nframes>0) // setb repetition bit
            encoded_sequence(k++,nnotes) = 1;            
    }    
}
    

// input noise injection
void inject_zero_forcing_noise(Mat sequence, double noise_prob)
{
    if(!sequence.isCompact())
        PLEERROR("Expected a compact sequence");
    real* p = sequence.data();
    int n = sequence.size();
    while(n--)
    {
        if(*p!=real(0.) && random_gen->uniform_sample()<noise_prob)
            *p = real(0.);
        ++p;
    }
}


void DenoisingRecurrentNet::clamp_units(const Vec layer_vector,
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

void DenoisingRecurrentNet::clamp_units(const Vec layer_vector,
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

void DenoisingRecurrentNet::setLearningRate( real the_learning_rate )
{
    input_layer->setLearningRate( the_learning_rate );
    hidden_layer->setLearningRate( the_learning_rate );
    input_connections->setLearningRate( the_learning_rate );
    if( dynamic_connections )
        dynamic_connections->setLearningRate( the_learning_rate ); //HUGO: multiply by dynamic_connections_learning_weight;
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


void DenoisingRecurrentNet::computeOutput(const Vec& input, Vec& output) const
{
    PLERROR("DenoisingRecurrentNet::computeOutput(): this is a dynamic, "
            "generative model, that can only compute negative log-likelihood "
            "costs for a whole VMat");
}

void DenoisingRecurrentNet::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    PLERROR("DenoisingRecurrentNet::computeCostsFromOutputs(): this is a "
            "dynamic, generative model, that can only compute negative "
            "log-likelihooh costs for a whole VMat");
}



void DenoisingRecurrentNet::test(VMat testset, PP<VecStatsCollector> test_stats,
                  VMat testoutputs, VMat testcosts)const
{
    int len = testset.length();
    Vec input;
    Vec target;
    real weight;


    Vec output(outputsize());
    output.clear();
    Vec costs(nTestCosts());
    costs.clear();
    Vec n_items(nTestCosts());
    n_items.clear();

    PP<ProgressBar> pb;
    if (report_progress) 
        pb = new ProgressBar("Testing learner", len);

    if (len == 0) {
        // Empty test set: we give -1 cost arbitrarily.
        costs.fill(-1);
        test_stats->update(costs);
    }

    int w = testset->width();
    locateSequenceBoundaries(testset, testset_boundaries, end_of_sequence_symbol);
    int nseq = testset_boundaries.length();

    seq.resize(5000,2); // contains the current sequence
    seq.resize(5000, 4);

    for(int i=0; i<nseq; i++)
    {
        int start = 0;
        if(i>0)
            start = testset_boundaries[i-1]+1;
        int end = testset_boundaries[i];
        seq.resize(end-start, w);
        testset->getMat(start,0,seq);

        if(encoding=="raw_masked_supervised")
        {
            splitMaskedSupervisedSequence(seq);
        }
        else
        {
            encodeSequence(seq, encoded_seq);
            createSupervisedSequence(encoded_seq);
        }

        resize_lists();
        fprop(test_costs, test_n_items);

        /*
        if (testoutputs)
        {
            int sum_target_layers_size = 0;
            for( int tar=0; tar < target_layers.length(); tar++ )
            {
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
                {
                    output.subVec(sum_target_layers_size,target_layers[tar]->size)
                        << target_prediction_list[tar][ ith_sample_in_sequence ];
                }
                sum_target_layers_size += target_layers[tar]->size;
            }
            testoutputs->putOrAppendRow(i, output);
        }
        */

    }

        if (report_progress)
            pb->update(i);

    }

    for(int i=0; i<costs.length(); i++)
    {
        if( !fast_exact_is_equal(target_layers_weights[i],0) )
            costs[i] /= n_items[i];
        else
            costs[i] = MISSING_VALUE;
    }
    if (testcosts)
        testcosts->putOrAppendRow(0, costs);
    
    if (test_stats)
        test_stats->update(costs, weight);


    if( pb )
        pb->update( stage + 1 - init_stage);

}

/*
void DenoisingRecurrentNet::oldtest(VMat testset, PP<VecStatsCollector> test_stats,
                  VMat testoutputs, VMat testcosts)const
{ 

    int len = testset.length();
    Vec input;
    Vec target;
    real weight;

    Vec output(outputsize());
    output.clear();
    Vec costs(nTestCosts());
    costs.clear();
    Vec n_items(nTestCosts());
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

            if (testoutputs)
            {
                output.fill(end_of_sequence_symbol);
                testoutputs->putOrAppendRow(i, output);
            }

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
            if( !fast_exact_is_equal(target_layers_weights[tar],0) )
            {
                targets_list[tar].resize( ith_sample_in_sequence+1);
                targets_list[tar][ith_sample_in_sequence].resize( 
                    target_layers[tar]->size);
                target_prediction_list[tar].resize(
                    ith_sample_in_sequence+1);
                target_prediction_act_no_bias_list[tar].resize(
                    ith_sample_in_sequence+1);
            }
        }
        nll_list.resize(ith_sample_in_sequence+1,target_layers.length());
        if( use_target_layers_masks )
        {
            masks_list.resize( target_layers.length() );
            for( int tar=0; tar < target_layers.length(); tar++ )
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
            if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
                targets_list[tar][ith_sample_in_sequence] << 
                    target_layers[tar]->expectation;
            }
            sum_target_elements += target_layers_n_of_target_elements[tar];
        }
                
        input_connections->fprop( input_list[ith_sample_in_sequence], 
                                  hidden_act_no_bias_list[ith_sample_in_sequence]);
                
        if( ith_sample_in_sequence > 0 && dynamic_connections )
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
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
        }
        else
        {
            for( int tar=0; tar < target_layers.length(); tar++ )
            {
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
        }

        if (testoutputs)
        {
            int sum_target_layers_size = 0;
            for( int tar=0; tar < target_layers.length(); tar++ )
            {
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
                {
                    output.subVec(sum_target_layers_size,target_layers[tar]->size)
                        << target_prediction_list[tar][ ith_sample_in_sequence ];
                }
                sum_target_layers_size += target_layers[tar]->size;
            }
            testoutputs->putOrAppendRow(i, output);
        }

        sum_target_elements = 0;
        for( int tar=0; tar < target_layers.length(); tar++ )
        {
            if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
                }
                else
                    n_items[tar]++;
            }
            if( use_target_layers_masks )
                sum_target_elements += 
                    target_layers_n_of_target_elements[tar];
        }
        ith_sample_in_sequence++;

        if (report_progress)
            pb->update(i);

    }

    for(int i=0; i<costs.length(); i++)
    {
        if( !fast_exact_is_equal(target_layers_weights[i],0) )
            costs[i] /= n_items[i];
        else
            costs[i] = MISSING_VALUE;
    }
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
*/

TVec<string> DenoisingRecurrentNet::getTestCostNames() const
{
    TVec<string> cost_names(0);
    for( int i=0; i<target_layers.length(); i++ )
        cost_names.append("target" + tostring(i) + ".NLL");
    return cost_names;
}

TVec<string> DenoisingRecurrentNet::getTrainCostNames() const
{
    return getTestCostNames();
}

void DenoisingRecurrentNet::generate(int t, int n)
{
    //PPath* the_filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/generate/scoreGen.amat";
    data = new AutoVMatrix();
    //data->filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/listData/target_tm12_input_t_tm12_tp12/scoreGen_tar_tm12__in_tm12_tp12.amat";
    data->filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/create_data/scoreGenSuitePerf.amat";

    data->defineSizes(208,16,0);
    //data->inputsize = 21;
    //data->targetsize = 0;
    //data->weightsize = 0;
    data->build();

    
    
   
   

    int len = data->length();
    int tarSize = outputsize();
    int partTarSize;
    Vec input;
    Vec target;
    real weight;

    Vec output(outputsize());
    output.clear();
    /*Vec costs(nTestCosts());
    costs.clear();
    Vec n_items(nTestCosts());
    n_items.clear();*/

    int r,r2;
    
    int ith_sample_in_sequence = 0;
    int inputsize_without_masks = inputsize() 
        - ( use_target_layers_masks ? targetsize() : 0 );
    int sum_target_elements = 0;
    for (int i = 0; i < len; i++)
    {
        data->getExample(i, input, target, weight);
        if(i>n)
        {
            for (int k = 1; k <= t; k++)
            {
                if(k<=i){
                    partTarSize = outputsize();
                    for( int tar=0; tar < target_layers.length(); tar++ )
                    {
                        
                        input.subVec(inputsize_without_masks-(tarSize*(t-k))-partTarSize-1,target_layers[tar]->size) << target_prediction_list[tar][ith_sample_in_sequence-k];
                        partTarSize -= target_layers[tar]->size;
                        
                        
                    }
                }
            }       
        }
    
/*
        for (int k = 1; k <= t; k++)
        {
            partTarSize = outputsize();
            for( int tar=0; tar < target_layers.length(); tar++ )
            {
                if(i>=t){
                    input.subVec(inputsize_without_masks-(tarSize*(t-k))-partTarSize-1,target_layers[tar]->size) << target_prediction_list[tar][ith_sample_in_sequence-k];
                    partTarSize -= target_layers[tar]->size;
                }
            }
        }
*/
        if( fast_exact_is_equal(input[0],end_of_sequence_symbol) )
        {
            /*  ith_sample_in_sequence = 0;
            hidden_list.resize(0);
            hidden_act_no_bias_list.resize(0);
            hidden2_list.resize(0);
            hidden2_act_no_bias_list.resize(0);
            target_prediction_list.resize(0);
            target_prediction_act_no_bias_list.resize(0);
            input_list.resize(0);
            targets_list.resize(0);
            nll_list.resize(0,0);
            masks_list.resize(0);*/

            

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
            if( !fast_exact_is_equal(target_layers_weights[tar],0) )
            {
                targets_list[tar].resize( ith_sample_in_sequence+1);
                targets_list[tar][ith_sample_in_sequence].resize( 
                    target_layers[tar]->size);
                target_prediction_list[tar].resize(
                    ith_sample_in_sequence+1);
                target_prediction_act_no_bias_list[tar].resize(
                    ith_sample_in_sequence+1);
            }
        }
        nll_list.resize(ith_sample_in_sequence+1,target_layers.length());
        if( use_target_layers_masks )
        {
            masks_list.resize( target_layers.length() );
            for( int tar=0; tar < target_layers.length(); tar++ )
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
            if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
                targets_list[tar][ith_sample_in_sequence] << 
                    target_layers[tar]->expectation;
            }
            sum_target_elements += target_layers_n_of_target_elements[tar];
        }
                
        input_connections->fprop( input_list[ith_sample_in_sequence], 
                                  hidden_act_no_bias_list[ith_sample_in_sequence]);
                
        if( ith_sample_in_sequence > 0 && dynamic_connections )
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
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
        }
        else
        {
            for( int tar=0; tar < target_layers.length(); tar++ )
            {
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
        }

        

        sum_target_elements = 0;
        for( int tar=0; tar < target_layers.length(); tar++ )
        {
            if( !fast_exact_is_equal(target_layers_weights[tar],0) )
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
                /*costs[tar] += nll_list(ith_sample_in_sequence,tar);
                
                // Normalize by the number of things to predict
                if( use_target_layers_masks )
                {
                    n_items[tar] += sum(
                        input.subVec( inputsize_without_masks 
                                      + sum_target_elements, 
                                      target_layers_n_of_target_elements[tar]) );
                }
                else
                n_items[tar]++;*/
            }
            if( use_target_layers_masks )
                sum_target_elements += 
                    target_layers_n_of_target_elements[tar];
        }
        ith_sample_in_sequence++;

        

    }

    /*  
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


    */









    
    //Vec tempo;
    //TVec<real> tempo;
    //tempo.resize(visible_layer->size);
    ofstream myfile;
    myfile.open ("/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/generate/test.txt");
    
    for (int i = 0; i < target_prediction_list[0].length() ; i++ ){
       
       
        for( int tar=0; tar < target_layers.length(); tar++ )
        {
            for (int j = 0; j < target_prediction_list[tar][i].length() ; j++ ){
                
                if(i>n){
                    myfile << target_prediction_list[tar][i][j] << " ";
                }
                else{
                    myfile << targets_list[tar][i][j] << " ";
                }
                       
           
            }
        }
        myfile << "\n";
    }
     

     myfile.close();

}
/*
void DenoisingRecurrentNet::gen()
{
    //PPath* the_filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/generate/scoreGen.amat";
    data = new AutoVMatrix();
    data->filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/generate/scoreGen.amat";
    data->defineSizes(21,0,0);
    //data->inputsize = 21;
    //data->targetsize = 0;
    //data->weightsize = 0;
    data->build();

    
    int len = data->length();
    Vec score;
    Vec target;
    real weight;
    Vec bias_tempo;
    Vec visi_bias_tempo;
   
   
    
    previous_hidden_layer.resize(hidden_layer->size);
    connections_idem = connections;

    for (int ith_sample = 0; ith_sample < len ; ith_sample++ ){
        
        data->getExample(ith_sample, score, target, weight);
        //score << data(ith_sample);
        input_prediction_list.resize(
            ith_sample+1,visible_layer->size);
        if(ith_sample > 0)
        {
            
            //input_list(ith_sample_in_sequence) << previous_input;
            //h*_{t-1}
            //////////////////////////////////
            dynamic_connections->fprop(previous_hidden_layer, cond_bias);
            hidden_layer->setAllBias(cond_bias); 
            
            
            
            //up phase
            connections->setAsDownInput( input_prediction_list(ith_sample-1) );
            hidden_layer->getAllActivations( connections_idem );
            hidden_layer->computeExpectation();
            //////////////////////////////////
            
            //previous_hidden_layer << hidden_layer->expectation;//h_{t-2} au prochain tour
            //previous_hidden_layer_act_no_bias << hidden_layer->activation;
            
            
            //h*_{t}
            ////////////
            if(dynamic_connections_copy)
                dynamic_connections_copy->fprop( hidden_layer->expectation ,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
            else
                dynamic_connections->fprop( hidden_layer->expectation ,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
            //dynamic_connections_copy->fprop( hidden_layer->expectation ,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
            hidden_layer->expectation_is_not_up_to_date();
            hidden_layer->computeExpectation();//h_{t}
            ///////////
            
            //previous_input << visible_layer->expectation;//v_{t-1}
            
        }
        else
        {
            
            previous_hidden_layer.clear();//h_{t-1}
            if(dynamic_connections_copy)
                dynamic_connections_copy->fprop( previous_hidden_layer ,
                                                 hidden_layer->activation);//conection entre h_{t-1} et h_{t}
            else
                dynamic_connections->fprop(previous_hidden_layer,
                                           hidden_layer->activation);//conection entre h_{t-1} et h_{t}
            
            hidden_layer->expectation_is_not_up_to_date();
            hidden_layer->computeExpectation();//h_{t}
            //previous_input.resize(data->inputsize);
            //previous_input << data(ith_sample);
            
        }
        
        //connections_transpose->setAsDownInput( hidden_layer->expectation );
        //visible_layer->getAllActivations( connections_idem_t );
        
        connections->setAsUpInput( hidden_layer->expectation );
        visible_layer->getAllActivations( connections_idem );
        
        visible_layer->computeExpectation();
        //visible_layer->generateSample();
        partition(score.subVec(14,taillePart), visible_layer->activation.subVec(14+taillePart,taillePart), visible_layer->activation.subVec(14+(taillePart*2),taillePart));
        partition(score.subVec(14,taillePart), visible_layer->expectation.subVec(14+taillePart,taillePart), visible_layer->expectation.subVec(14+(taillePart*2),taillePart));


        visible_layer->activation.subVec(0,14+taillePart) << score;
        visible_layer->expectation.subVec(0,14+taillePart) << score;

        input_prediction_list(ith_sample) << visible_layer->expectation;
        
    }
    
    //Vec tempo;
    TVec<real> tempo;
    tempo.resize(visible_layer->size);
    ofstream myfile;
    myfile.open ("/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/generate/test.txt");
    
    for (int i = 0; i < len ; i++ ){
        tempo << input_prediction_list(i);
        
        //cout << tempo[2] << endl;
       
        for (int j = 0; j < tempo.length() ; j++ ){
            
            
                
                
               myfile << tempo[j] << " ";
               

               
           
        }
        myfile << "\n";
    }
     

     myfile.close();

}*/
//void DenoisingRecurrentNet::generate(int nbNotes)
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
