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
// - add dynamic_activations_list and use it in recurrentUpdate


namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DenoisingRecurrentNet,
    "Model made of RBMs linked through time",
    ""
    );

DenoisingRecurrentNet::DenoisingRecurrentNet() :
    use_target_layers_masks( false ),
    end_of_sequence_symbol( -1000 ),
    encoding("note_octav_duration"),
    input_window_size(1),
    tied_input_reconstruction_weights( true ),
    input_noise_prob( 0.15 ),
    input_reconstruction_lr( 0 ),
    hidden_noise_prob( 0.15 ),
    hidden_reconstruction_lr( 0 ),
    tied_hidden_reconstruction_weights( false ),
    noisy_recurrent_lr( 0.000001),
    dynamic_gradient_scale_factor( 1.0 ),
    recurrent_lr( 0.00001 ),
    prediction_cost_weight(1),
    input_reconstruction_cost_weight(0),
    hidden_reconstruction_cost_weight(0),
    current_learning_rate(0),
    nb_stage_reconstruction(0),
    nb_stage_target(0),
    noise(false),
    L1_penalty_factor(0),
    L2_penalty_factor(0)
{
    random_gen = new PRandom();
}

void DenoisingRecurrentNet::declareOptions(OptionList& ol)
{
//    declareOption(ol, "rbm_learning_rate", &DenoisingRecurrentNet::rbm_learning_rate,
//                  OptionBase::buildoption,
//                  "The learning rate used during RBM contrastive "
//                  "divergence learning phase.\n");

//    declareOption(ol, "rbm_nstages", &DenoisingRecurrentNet::rbm_nstages,
//                  OptionBase::buildoption,
//                  "Number of epochs for rbm phase.\n");


    declareOption(ol, "target_layers_weights", 
                  &DenoisingRecurrentNet::target_layers_weights,
                  OptionBase::buildoption,
                  "The training weights of each target layers.\n");

    declareOption(ol, "end_of_sequence_symbol", 
                  &DenoisingRecurrentNet::end_of_sequence_symbol,
                  OptionBase::buildoption,
                  "Value of the first input component for end-of-sequence "
                  "delimiter.\n");

    // TO DO: input_layer is to be removed eventually because only its size is really used
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

    declareOption(ol, "dynamic_reconstruction_connections", 
                  &DenoisingRecurrentNet::dynamic_reconstruction_connections,
                  OptionBase::buildoption,
                  "The RBMConnection for the reconstruction between the hidden layers, "
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





    
    declareOption(ol, "encoding", 
                  &DenoisingRecurrentNet::encoding,
                  OptionBase::buildoption,
                  "Chooses what type of encoding to apply to an input sequence\n"
                  "Possibilities: timeframe, note_duration, note_octav_duration, raw_masked_supervised");

    declareOption(ol, "input_window_size", 
                  &DenoisingRecurrentNet::input_window_size,
                  OptionBase::buildoption,
                  "How many time steps to present as input\n"
                  "If it's 0, then all layers are essentially ignored, and instead an unconditional predictor is trained\n"
                  "This option is ignored when mode is raw_masked_supervised,"
                  "since in this mode the full expanded and preprocessed input and target are given explicitly."
        );

    declareOption(ol, "tied_input_reconstruction_weights", 
                  &DenoisingRecurrentNet::tied_input_reconstruction_weights,
                  OptionBase::buildoption,
                  "Do we want the input reconstruction weights tied or not\n"
                  "Boolean, yes or no");

    declareOption(ol, "input_noise_prob", 
                  &DenoisingRecurrentNet::input_noise_prob,
                  OptionBase::buildoption,
                  "Probability, for each neurone of each input, to be set to zero\n");

    declareOption(ol, "input_reconstruction_lr", 
                  &DenoisingRecurrentNet::input_reconstruction_lr,
                  OptionBase::buildoption,
                  "The learning rate used for the reconstruction\n");

    declareOption(ol, "hidden_noise_prob", 
                  &DenoisingRecurrentNet::hidden_noise_prob,
                  OptionBase::buildoption,
                  "Probability, for each neurone of each hidden layer, to be set to zero\n");

    declareOption(ol, "hidden_reconstruction_lr", 
                  &DenoisingRecurrentNet::hidden_reconstruction_lr,
                  OptionBase::buildoption,
                  "The learning rate used for the dynamic reconstruction through time\n");

    declareOption(ol, "tied_hidden_reconstruction_weights", 
                  &DenoisingRecurrentNet::tied_hidden_reconstruction_weights,
                  OptionBase::buildoption,
                  "Do we want the dynamic reconstruction weights tied or not\n"
                  "Boolean, yes or no");

    declareOption(ol, "noisy_recurrent_lr", 
                  &DenoisingRecurrentNet::noisy_recurrent_lr,
                  OptionBase::buildoption,
                  "The learning rate used in the noisy recurrent phase for the input reconstruction\n");

    declareOption(ol, "dynamic_gradient_scale_factor", 
                  &DenoisingRecurrentNet::dynamic_gradient_scale_factor,
                  OptionBase::buildoption,
                  "The scale factor of the learning rate used in the noisy recurrent phase for the dynamic hidden reconstruction\n");

    declareOption(ol, "recurrent_lr", 
                  &DenoisingRecurrentNet::recurrent_lr,
                  OptionBase::buildoption,
                  "The learning rate used in the fine tuning phase\n");

    declareOption(ol, "mean_encoded_vec", &DenoisingRecurrentNet::mean_encoded_vec,
                  OptionBase::learntoption,
                  "When training with trainUnconditionalPredictor (if input_window_size==0), this is simply used to store the the avg encoded frame");

    declareOption(ol, "prediction_cost_weight", &DenoisingRecurrentNet::prediction_cost_weight,
                  OptionBase::learntoption,
                  "The training weight for the target prediction");

    declareOption(ol, "input_reconstruction_cost_weight", &DenoisingRecurrentNet::input_reconstruction_cost_weight,
                  OptionBase::learntoption,
                  "The training weight for the input reconstruction");

    declareOption(ol, "hidden_reconstruction_cost_weight", &DenoisingRecurrentNet::hidden_reconstruction_cost_weight,
                  OptionBase::learntoption,
                  "The training weight for the hidden reconstruction");

    declareOption(ol, "nb_stage_reconstruction", &DenoisingRecurrentNet::nb_stage_reconstruction,
                  OptionBase::learntoption,
                  "The nomber of stage for de reconstructions");

    declareOption(ol, "nb_stage_target", &DenoisingRecurrentNet::nb_stage_target,
                  OptionBase::learntoption,
                  "The nomber of stage for de target");

    declareOption(ol, "L1_penalty_factor",
                  &DenoisingRecurrentNet::L1_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L1 regularization term, i.e.\n"
                  "minimize L1_penalty_factor * sum_{ij} |weights(i,j)| "
                  "during training.\n");

    declareOption(ol, "L2_penalty_factor",
                  &DenoisingRecurrentNet::L2_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L2 regularization term, i.e.\n"
                  "minimize 0.5 * L2_penalty_factor * sum_{ij} weights(i,j)^2 "
                  "during training.\n");
                  
                  
                  

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
        use_target_layers_masks = (encoding=="raw_masked_supervised");

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
/*
        if( input_layer->size != input_layer_size )
            PLERROR("DenoisingRecurrentNet::build_(): input_layer->size %d "
                    "should be %d", input_layer->size, input_layer_size);
*/
        // Parsing symbols in target
        int tar_layer = 0;
        int tar_layer_size = 0;
        target_symbol_sizes.resize(target_layers.length());
        for( tar_layer=0; tar_layer<target_layers.length(); tar_layer++ )
            target_symbol_sizes[tar_layer].resize(0);

        target_layers_n_of_target_elements.resize( targetsize() );
        target_layers_n_of_target_elements.clear();
        tar_layer = 0;
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

        //if( tar_layer != target_layers.length() )
        //    PLERROR("DenoisingRecurrentNet::build_(): target layers "
        //            "does not cover all targets.");


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

        if( dynamic_reconstruction_connections )
        {

            dynamic_reconstruction_connections->down_size = hidden_layer->size;
            dynamic_reconstruction_connections->up_size = hidden_layer->size;
            if( !dynamic_reconstruction_connections->random_gen )
            {
                dynamic_reconstruction_connections->random_gen = random_gen;
                dynamic_reconstruction_connections->forget();
            }
            dynamic_reconstruction_connections->build();
            
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

    // Public fields
    deepCopyField( target_layers_weights, copies );
    deepCopyField( input_layer, copies);
    deepCopyField( target_layers , copies);
    deepCopyField( hidden_layer, copies);
    deepCopyField( hidden_layer2 , copies);
    deepCopyField( dynamic_connections , copies);
    deepCopyField( dynamic_reconstruction_connections , copies);
    deepCopyField( hidden_connections , copies);
    deepCopyField( input_connections , copies);
    deepCopyField( target_connections , copies);
    deepCopyField( target_layers_n_of_target_elements, copies);
    deepCopyField( input_symbol_sizes, copies);
    deepCopyField( target_symbol_sizes, copies);
    deepCopyField( mean_encoded_vec, copies);
    deepCopyField( input_reconstruction_bias, copies);
    deepCopyField( hidden_reconstruction_bias, copies);
    deepCopyField( hidden_reconstruction_bias2, copies);

    // Protected fields
    deepCopyField( data, copies);
    deepCopyField( acc_target_connections_gr, copies);
    deepCopyField( acc_input_connections_gr, copies);
    deepCopyField( acc_dynamic_connections_gr, copies);
    deepCopyField( acc_reconstruction_dynamic_connections_gr, copies);
    deepCopyField( acc_target_bias_gr, copies);
    deepCopyField( acc_hidden_bias_gr, copies);
    deepCopyField( acc_recons_bias_gr, copies);
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
    deepCopyField( trainset_boundaries, copies);
    deepCopyField( testset_boundaries, copies);
    deepCopyField( seq, copies);
    deepCopyField( encoded_seq, copies);
    deepCopyField( clean_encoded_seq, copies);
    deepCopyField( input_reconstruction_prob, copies);
    deepCopyField( hidden_reconstruction_prob, copies);
    

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
    if( dynamic_reconstruction_connections )
        dynamic_reconstruction_connections->forget();
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

    input_reconstruction_bias.clear();

    stage = 0;
}

void DenoisingRecurrentNet::trainUnconditionalPredictor()
{
    MODULE_LOG << "trainUnconditionalPredictor() called " << endl;

    // reserve memory for sequences
    seq.resize(5000,2); // contains the current sequence

    // real weight = 0; // Unused
    Vec train_costs( getTrainCostNames().length() );
    train_costs.fill(-1);

    if( !initTrain() )
    {
        MODULE_LOG << "train() aborted" << endl;
        return;
    }


    if( stage==0 && nstages==1 )
    {        
        // clear stats of previous epoch
        train_stats->forget();


        int nvecs = 0;
        int nseq = nSequences();        

        ProgressBar* pb = 0;
        if( report_progress)
            pb = new ProgressBar( "Sequences ",nseq);
        for(int i=0; i<nseq; i++)
        {
            getSequence(i, seq);
            encodeSequenceAndPopulateLists(seq, false);
            if(i==0)
            {
                mean_encoded_vec.resize(encoded_seq.width());
                mean_encoded_vec.clear();
            }
            for(int t=0; t<encoded_seq.length(); t++)
            {
                mean_encoded_vec += encoded_seq(t);                
                nvecs++;
            }
        }
        mean_encoded_vec *= 1./nvecs;            
        train_stats->update(train_costs);
        train_stats->finalize();            

        if( pb )
        {
            delete pb;
            pb = 0;
        }
        ++stage;
    }
}


void DenoisingRecurrentNet::train()
{
    if(input_window_size==0)
    {
        trainUnconditionalPredictor();
        return;
    }

    MODULE_LOG << "train() called " << endl;

    // reserve memory for sequences
    seq.resize(5000,2); // contains the current sequence

    // real weight = 0; // Unused
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
        MODULE_LOG << "  input_noise_prob = " <<                input_noise_prob  << endl;              
        MODULE_LOG << "  input_reconstruction_lr = " <<         input_reconstruction_lr  << endl;       
        MODULE_LOG << "  hidden_noise_prob = " <<               hidden_noise_prob  << endl;             
        MODULE_LOG << "  hidden_reconstruction_lr = " <<        hidden_reconstruction_lr  << endl;      
        MODULE_LOG << "  noisy_recurrent_lr = " <<              noisy_recurrent_lr  << endl;            
        MODULE_LOG << "  dynamic_gradient_scale_factor = " <<   dynamic_gradient_scale_factor  << endl; 
        MODULE_LOG << "  recurrent_lr = " <<                    recurrent_lr  << endl;                  


        if( report_progress && stage < end_stage )
            pb = new ProgressBar( "Recurrent training phase of "+classname(),
                                  end_stage - init_stage );

        int nCost = 2;
        train_costs.resize(train_costs.length() + nCost);
        train_n_items.resize(train_n_items.length() + nCost);
        while(stage < end_stage)
        {
            train_costs.clear();
            train_n_items.clear();

            int nseq = nSequences();
            for(int i=0; i<nseq; i++)
            {

                if(input_noise_prob!=0 )
                    noise = true;
                else
                    noise = false;

                
                
                
                getSequence(i, seq);
                encodeSequenceAndPopulateLists(seq, false);

                
              
                //bool corrupt_input = false;//input_noise_prob!=0 && (noisy_recurrent_lr!=0 || input_reconstruction_lr!=0);

                //clean_encoded_seq.resize(encoded_seq.length(), encoded_seq.width());
                //clean_encoded_seq << encoded_seq;

                //if(corrupt_input)  // WARNING: encoded_sequence will be dirty!!!!
                      //  inject_zero_forcing_noise(encoded_seq, input_noise_prob);

                // recurrent no noise phase
                if(stage>=nb_stage_reconstruction){
                    if(recurrent_lr!=0)
                    {
                        
                        setLearningRate( recurrent_lr );                    
                        recurrentFprop(train_costs, train_n_items);
                        recurrentUpdate(0,0,1, prediction_cost_weight,1, train_costs, train_n_items );
                        
                    }
                }

                if(stage<nb_stage_reconstruction || nb_stage_reconstruction == 0 ){

                    // greedy phase hidden
                    if(hidden_reconstruction_lr!=0){
                        setLearningRate( hidden_reconstruction_lr);
                        recurrentFprop(train_costs, train_n_items, true);
                        //recurrentUpdate(0, hidden_reconstruction_cost_weight, 1, 0,1, train_costs, train_n_items );
                        recurrentUpdate(0, hidden_reconstruction_cost_weight, 1, 0,1, train_costs, train_n_items );
                    }

                    /*if(recurrent_lr!=0)
                    {                 
                        setLearningRate( recurrent_lr );                    
                        recurrentFprop(train_costs, train_n_items);
                        //recurrentUpdate(0,0,1, prediction_cost_weight,0, train_costs, train_n_items );
                        recurrentUpdate(0,0,0, prediction_cost_weight,0, train_costs, train_n_items );
                        
                        }*/
                    
                    // greedy phase input
                    if(input_reconstruction_lr!=0){
                        if (noise)
                            encodeSequenceAndPopulateLists(seq, true);
                        setLearningRate( input_reconstruction_lr );
                        recurrentFprop(train_costs, train_n_items, false);
                        if (noise)
                            encodeSequenceAndPopulateLists(seq, false);
                        //recurrentUpdate(input_reconstruction_cost_weight, 0, 1, 0,1, train_costs, train_n_items );
                        recurrentUpdate(input_reconstruction_cost_weight, 0, 1, 0,1, train_costs, train_n_items );
                    }
                    
                    
                    
                    
                }

                // recurrent no noise phase
                /*if(stage>=nb_stage_reconstruction && stage<nb_stage_target+nb_stage_reconstruction){
                    if(recurrent_lr!=0)
                    {
                        
                        if(noise) // need to recover the clean sequence                        
                            encoded_seq << clean_encoded_seq;                  
                        setLearningRate( recurrent_lr );                    
                        recurrentFprop(train_costs, train_n_items);
                        recurrentUpdate(0,0,1, prediction_cost_weight,0, train_costs, train_n_items );
                        
                        }
                    }*/

                


                // recurrent noisy phase
                if(noisy_recurrent_lr!=0)
                {
                    setLearningRate( noisy_recurrent_lr );
                    recurrentFprop(train_costs, train_n_items);
                    recurrentUpdate(input_reconstruction_cost_weight, hidden_reconstruction_cost_weight, 1,1, prediction_cost_weight, train_costs, train_n_items );
                }

                
            }
            noise= false;
            if( pb )
                pb->update( stage + 1 - init_stage);
            
            //double totalCosts = 0;
            for(int i=0; i<train_costs.length(); i++)
            {
                if (i < target_layers_weights.length()){
                    if( !fast_exact_is_equal(target_layers_weights[i],0) ){
                        train_costs[i] /= train_n_items[i];
                        //totalCosts += train_costs[i]*target_layers_weights[i];
                    }
                    else
                        train_costs[i] = MISSING_VALUE;
                }
                
                if (i == train_costs.length()-nCost ){
                    train_costs[i] /= train_n_items[i];
                    //totalCosts += train_costs[i]*input_reconstruction_cost_weight;
                }
                else if (i == train_costs.length()-1)
                    train_costs[i] /= train_n_items[i];
                
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


//! does encoding if needed and populates the list.
void DenoisingRecurrentNet::encodeSequenceAndPopulateLists(Mat seq, bool doNoise) const
{
    if(encoding=="raw_masked_supervised") // old already encoded format (for backward testing)
        splitRawMaskedSupervisedSequence(seq, doNoise);
    else if(encoding=="generic")
        encode_artificialData(seq);
    else
        encodeAndCreateSupervisedSequence(seq);
}

// encodes sequ, then populates: input_list, targets_list, masks_list
void DenoisingRecurrentNet::encodeAndCreateSupervisedSequence(Mat seq) const
{
    if(use_target_layers_masks)
        PLERROR("Bug: use_target_layers_masks is expected to be false (no masks) when in encodeAndCreateSupervisedSequence");

    encodeSequence(seq, encoded_seq);
    // now work with encoded_seq
    int l = encoded_seq.length();
    resize_lists(l-input_window_size);


    int ntargets = target_layers.length();
    targets_list.resize(ntargets);
    //Mat targets = targets_list[0];
    //targets.resize(l, encoded_seq.width());
    targets_list[0].resize(l-input_window_size, encoded_seq.width());   
         
    for(int t=input_window_size; t<l; t++)
    {

        input_list[t-input_window_size] = encoded_seq.subMatRows(t-input_window_size,input_window_size).toVec();
        //perr << "t-input_window_size = " << endl;
        //perr << "subMat:" << endl << encoded_seq.subMatRows(t-input_window_size,input_window_size) << endl;
        //perr << "toVec:" << endl << encoded_seq.subMatRows(t-input_window_size,input_window_size).toVec() << endl;
        //perr << "input_list:" << endl << input_list[t-input_window_size] << endl;
        // target is copied so that when adding noise to input, it doesn't modify target 
        //targets(t-input_window_size) << encoded_seq(t);
        targets_list[0](t-input_window_size) << encoded_seq(t);
    }
}




// For the (backward testing) raw_masked_supervised case. Populates: input_list, targets_list, masks_list
void DenoisingRecurrentNet::splitRawMaskedSupervisedSequence(Mat seq, bool doNoise) const
{
    int l = seq.length();
    resize_lists(l);
    int inputsize_without_masks = inputsize()-targetsize();
    Mat input_part;
    input_part.resize(seq.length(),inputsize_without_masks);
    input_part << seq.subMatColumns(0,inputsize_without_masks);
    Mat mask_part = seq.subMatColumns(inputsize_without_masks, targetsize());
    Mat target_part = seq.subMatColumns(inputsize_without_masks+targetsize(), targetsize());

    if(doNoise)
        inject_zero_forcing_noise(input_part, input_noise_prob);

    for(int i=0; i<l; i++)
        input_list[i] = input_part(i);

    int ntargets = target_layers.length();
    targets_list.resize(ntargets);
    masks_list.resize(ntargets);
    int startcol = 0; // starting column of next target in target_part and mask_part
    for(int k=0; k<ntargets; k++)
    {
        int targsize = target_layers[k]->size;
        targets_list[k] = target_part.subMatColumns(startcol, targsize);
        masks_list[k] = mask_part.subMatColumns(startcol, targsize);
        startcol += targsize;
    }

    encoded_seq.resize(input_part.length(), input_part.width());
    encoded_seq << input_part;
}

void DenoisingRecurrentNet::encode_artificialData(Mat seq) const
{
    int l = seq.length();
    int theInputsize = inputsize();
    int theTargetsize = targetsize();
    resize_lists(l);
    //int inputsize_without_masks = inputsize-targetsize;
    Mat input_part;
    input_part.resize(seq.length(),theInputsize);
    input_part << seq.subMatColumns(0,theInputsize);
    //Mat mask_part = seq.subMatColumns(inputsize, targetsize);
    Mat target_part = seq.subMatColumns(theInputsize, theTargetsize);

    //if(doNoise)
    //    inject_zero_forcing_noise(input_part, input_noise_prob);

    for(int i=0; i<l; i++)
        input_list[i] = input_part(i);

    int ntargets = target_layers.length();
    targets_list.resize(ntargets);
    //masks_list.resize(ntargets);
    int startcol = 0; // starting column of next target in target_part and mask_part
    for(int k=0; k<ntargets; k++)
    {
        int targsize = target_layers[k]->size;
        targets_list[k] = target_part.subMatColumns(startcol, targsize);
        //masks_list[k] = mask_part.subMatColumns(startcol, targsize);
        startcol += targsize;
    }

    encoded_seq.resize(input_part.length(), input_part.width());
    encoded_seq << input_part;


    /*int l = sequence.length();
 
    // reserve one extra bit to mean repetition
    encoded_sequence.resize(l, 1);
    encoded_sequence.clear();

    for(int i=0; i<l; i++)
    {
        int number = int(sequence(i,0));
        encoded_sequence(i,0) = number;        
        }    */
}    

void DenoisingRecurrentNet::resize_lists(int l) const
{
    input_list.resize(l);
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
        int targsize = target_layers[tar]->size;
        target_prediction_list[tar].resize(l, targsize);
        target_prediction_act_no_bias_list[tar].resize(l, targsize);
    }

    nll_list.resize(l,ntargets);
}


// must fill train_costs, train_n_items and     target_prediction_list[0](t)
void DenoisingRecurrentNet::unconditionalFprop(Vec train_costs, Vec train_n_items) const
{
    int pred_size = mean_encoded_vec.length();
    if(pred_size<=0)
        PLERROR("mean_encoded_vec not properly initialized. Did you call trainUnconditionalPredictor prior to unconditionalFprop ?");

    int l = input_list.length();
    int tar = 0;
    train_n_items[tar] += l;
    target_prediction_list[tar].resize(l,pred_size);
    for(int i=0; i<l; i++)
    {        
        Vec target_prediction_i = target_prediction_list[tar](i);
        target_prediction_i << mean_encoded_vec;
        Vec target_vec = targets_list[tar](i);

        /*
        target_layers[tar]->setExpectation(target_prediction_i);
        nll_list(i,tar) = target_layers[tar]->fpropNLL(target_vec); 
        */
        double nllcost = 0;
        for(int k=0; k<target_vec.length(); k++)
            if(target_vec[k]!=0)
                nllcost -= target_vec[k]*safelog(target_prediction_i[k]);
        nll_list(i,tar) = nllcost;

        if (isinf(nll_list(i,tar)))
        {
            PLWARNING("Row %d of sequence of length %d lead to inf cost",i,l);
            perr << "Problem at positions (vec of length " << target_vec.length() << "): ";
            for(int k=0; k<target_vec.length(); k++)
                if(target_vec[k]!=0 && target_prediction_i[k]==0)
                    perr << k << " ";
            perr << endl;
            // perr << "target_vec = " << target_vec << endl;
            // perr << "target_prediction_i = " << target_prediction_i << endl;
        }
        else
            train_costs[tar] += nll_list(i,tar);
    }
}

// fprop accumulates costs in costs and counts in n_items
void DenoisingRecurrentNet::recurrentFprop(Vec train_costs, Vec train_n_items, bool useDynamicConnections) const
{
    int l = input_list.length();
    int ntargets = target_layers.length();

    for(int i=0; i<l; i++ )
    {
        Vec hidden_act_no_bias_i = hidden_act_no_bias_list(i);
        input_connections->fprop( input_list[i], hidden_act_no_bias_i);
        if(useDynamicConnections){
            if( i > 0 && dynamic_connections )
            {
                Vec hidden_i_prev = hidden_list(i-1);
                dynamic_connections->fprop(hidden_i_prev,dynamic_act_no_bias_contribution );
                hidden_act_no_bias_i += dynamic_act_no_bias_contribution;
            }
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
                Vec target_prediction_act_no_bias_i = target_prediction_act_no_bias_list[tar](i);
                target_connections[tar]->fprop(last_hidden, target_prediction_act_no_bias_i);
                target_layers[tar]->fprop(target_prediction_act_no_bias_i, target_prediction_i);
                if( use_target_layers_masks )
                    target_prediction_i *= masks_list[tar](i);

                target_layers[tar]->activation << target_prediction_act_no_bias_i;
                target_layers[tar]->activation += target_layers[tar]->bias;
                target_layers[tar]->setExpectation(target_prediction_i);

                Vec target_vec = targets_list[tar](i);
                nll_list(i,tar) = target_layers[tar]->fpropNLL(target_vec); 
                train_costs[tar] += nll_list(i,tar);

                // Normalize by the number of things to predict
                if( use_target_layers_masks )
                    train_n_items[tar] += sum(masks_list[tar](i));
                else
                    train_n_items[tar]++;
            }
        }
    }
    //if(noise)
    //  inject_zero_forcing_noise(hidden_list, input_noise_prob);
}


void DenoisingRecurrentNet::applyMultipleSoftmaxToInputWindow(Vec input_reconstruction_activation, Vec input_reconstruction_prob)
{
    if(target_layers.length()!=1)
        PLERROR("applyMultipleSoftmaxToInputWindow was thought to work with a single target layer which is a RBMMixedLayer combining differnet multinomial costs");

    // int nelems = target_layers[0]->size();
    int nelems = target_prediction_list[0].width();

    if(input_reconstruction_activation.length() != input_window_size*nelems)
        
        PLERROR("Problem: input_reconstruction_activation.length() != input_window_size*nelems  (%d != %d * %d)",input_reconstruction_activation.length(),input_window_size,nelems);

    for(int k=0; k<input_window_size; k++)
    {
        Vec activation_window = input_reconstruction_activation.subVec(k*nelems, nelems);
        Vec prob_window = input_reconstruction_prob.subVec(k*nelems, nelems);
        target_layers[0]->fprop(activation_window, prob_window);
    }    
}

Mat DenoisingRecurrentNet::getTargetConnectionsWeightMatrix(int tar)
{
    RBMMatrixConnection* conn = dynamic_cast<RBMMatrixConnection*>((RBMConnection*)target_connections[tar]);
    if(conn==0)
        PLERROR("Expecting input connection to be a RBMMatrixConnection. Je sais c'est sale, mais au point ou on est rendu..");
    return conn->weights;
}

Mat DenoisingRecurrentNet::getInputConnectionsWeightMatrix()
{
    RBMMatrixConnection* conn = dynamic_cast<RBMMatrixConnection*>((RBMConnection*)input_connections);
    if(conn==0)
        PLERROR("Expecting input connection to be a RBMMatrixConnection. Je sais c'est sale, mais au point ou on est rendu..");
    return conn->weights;
}

Mat DenoisingRecurrentNet::getDynamicConnectionsWeightMatrix()
{
    RBMMatrixConnection* conn = dynamic_cast<RBMMatrixConnection*>((RBMConnection*)dynamic_connections);
    if(conn==0)
        PLERROR("Expecting input connection to be a RBMMatrixConnection. Je sais c'est sale, mais au point ou on est rendu..");
    return conn->weights;
}

Mat DenoisingRecurrentNet::getDynamicReconstructionConnectionsWeightMatrix()
{
    RBMMatrixConnection* conn = dynamic_cast<RBMMatrixConnection*>((RBMConnection*)dynamic_reconstruction_connections);
    if(conn==0)
        PLERROR("Expecting input connection to be a RBMMatrixConnection. Je sais c'est sale, mais au point ou on est rendu..");
    return conn->weights;
}

void DenoisingRecurrentNet::updateTargetLayer( Vec& grad, Vec& bias, real& lr )
{
    real* b = bias.data();
    real* gb = grad.data();
    int size = bias.length();

    for( int i=0 ; i<size ; i++ )
    {
        
        b[i] -= lr * gb[i];
        
    }

   
}

void DenoisingRecurrentNet::bpropUpdateConnection(const Vec& input, 
                                                  const Vec& output,
                                                  Vec& input_gradient,
                                                  const Vec& output_gradient,
                                                  Mat& weights,
                                                  Mat& acc_weights_gr,
                                                  int& down_size,
                                                  int& up_size,
                                                  real& lr,
                                                  bool accumulate,
                                                  bool using_penalty_factor)
{
    PLASSERT( input.size() == down_size );
    PLASSERT( output.size() == up_size );
    PLASSERT( output_gradient.size() == up_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == down_size,
                      "Cannot resize input_gradient AND accumulate into it" );

        // input_gradient += weights' * output_gradient
        transposeProductAcc( input_gradient, weights, output_gradient );
    }
    else
    {
        input_gradient.resize( down_size );

        // input_gradient = weights' * output_gradient
        transposeProduct( input_gradient, weights, output_gradient );
    }

    // weights -= learning_rate * output_gradient * input'
    //externalProductScaleAcc( weights, output_gradient, input, -lr );
    externalProductScaleAcc( acc_weights_gr, output_gradient, input, -lr );
    
    if((!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0)) && using_penalty_factor)
        applyWeightPenalty(weights, acc_weights_gr, down_size, up_size, lr);
}

void DenoisingRecurrentNet::bpropUpdateHiddenLayer(const Vec& input, 
                                                   const Vec& output,
                                                   Vec& input_gradient,
                                                   const Vec& output_gradient,                                                
                                                   Vec& bias,
                                                   real& lr)
{

    int size = bias.length();

    PLASSERT( input.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );

    
    input_gradient.resize( size );
    input_gradient.clear();
    
    
    for( int i=0 ; i<size ; i++ )
    {
        real output_i = output[i];
        real in_grad_i;
        in_grad_i = output_i * (1-output_i) * output_gradient[i];
        input_gradient[i] += in_grad_i;
        
       
        // update the bias: bias -= learning_rate * input_gradient
        bias[i] -= lr * in_grad_i;
        
    }
    
    //applyBiasDecay();
}

void DenoisingRecurrentNet::applyWeightPenalty(Mat& weights, Mat& acc_weights_gr, int& down_size, int& up_size, real& lr)
{
    // Apply penalty (decay) on weights.
    real delta_L1 = lr * L1_penalty_factor;
    real delta_L2 = lr * L2_penalty_factor;
    /*if (L2_decrease_type == "one_over_t")
        delta_L2 /= (1 + L2_decrease_constant * L2_n_updates);
    else if (L2_decrease_type == "sigmoid_like")
        delta_L2 *= sigmoid((L2_shift - L2_n_updates) * L2_decrease_constant);
    else
        PLERROR("In RBMMatrixConnection::applyWeightPenalty - Invalid value "
                "for L2_decrease_type: %s", L2_decrease_type.c_str());
    */
    for( int i=0; i<up_size; i++)
    {
        real* w_ = weights[i];
        real* a_w_g = acc_weights_gr[i];
        for( int j=0; j<down_size; j++ )
        {
            if( delta_L2 != 0. ){
                //w_[j] *= (1 - delta_L2);
                a_w_g[j] -= w_[j]*delta_L2;
            }

            if( delta_L1 != 0. )
            {
                if( w_[j] > delta_L1 )
                    a_w_g[j] -= delta_L1;
                else if( w_[j] < -delta_L1 )
                    a_w_g[j] += delta_L1;
                else
                    a_w_g[j] = 0.;
            }
        }
    }
    /*if (delta_L2 > 0)
      L2_n_updates++;*/
}

double DenoisingRecurrentNet::fpropUpdateInputReconstructionFromHidden(Vec hidden, Mat& reconstruction_weights, Mat& acc_weights_gr, Vec& input_reconstruction_bias, Vec& input_reconstruction_prob, 
                                                                       Vec clean_input, Vec hidden_gradient, double input_reconstruction_cost_weight, double lr)
{
    double cost = fpropInputReconstructionFromHidden(hidden, reconstruction_weights, input_reconstruction_bias, input_reconstruction_prob, clean_input);
    updateInputReconstructionFromHidden(hidden, reconstruction_weights, acc_weights_gr, input_reconstruction_bias, input_reconstruction_prob, 
                                        clean_input, hidden_gradient, input_reconstruction_cost_weight, lr);
    return cost;
}


//! Builds input_reconstruction_prob from hidden (using reconstruction_weights which is  nhidden x ninputs, and input_reconstruction_bias)
//! Also computes neg log cost and returns it
double DenoisingRecurrentNet::fpropInputReconstructionFromHidden(Vec hidden, Mat reconstruction_weights, Vec& reconstruction_bias, Vec& reconstruction_prob, 
                                                                 Vec clean_input)
{
    // set appropriate sizes
    int fullinputlength = clean_input.length();
    Vec reconstruction_activation;
    if(reconstruction_bias.length()==0)
    {
        reconstruction_bias.resize(fullinputlength);
        reconstruction_bias.clear();
    }
    reconstruction_activation.resize(fullinputlength);
    reconstruction_prob.resize(fullinputlength);

    // predict (denoised) input_reconstruction 
    transposeProduct(reconstruction_activation, reconstruction_weights, hidden); 
    reconstruction_activation += reconstruction_bias;

    for( int j=0 ; j<fullinputlength ; j++ ){
        if(clean_input[j]==1 || clean_input[j]==0)
            reconstruction_prob[j] = fastsigmoid( reconstruction_activation[j] );
        else
            reconstruction_prob[j] = reconstruction_activation[j] ;
    }

    double result_cost = 0;
    if(encoding=="raw_masked_supervised" || encoding=="generic") // complicated input format... consider it's squared error
    {
        double r = 0;
        double neg_log_cost = 0; // neg log softmax
        for(int k=0; k<reconstruction_prob.length(); k++){
            if(clean_input[k]==1 || clean_input[k]==0){
                neg_log_cost -= clean_input[k]*safelog(reconstruction_prob[k]) + (1-clean_input[k])*safelog(1-reconstruction_prob[k]);
            }                
            else{
                r = reconstruction_prob[k] - clean_input[k];
                neg_log_cost += r*r;
            }
            
            
        }
        result_cost = neg_log_cost;
        
        /*real r;
        //reconstruction_prob << reconstruction_activation;
        for(int i=0; i<reconstruction_activation.length(); i++){
            r = reconstruction_activation[i] - clean_input[i];
            result_cost += r*r;
            }*/
    }
    else // suppose it's a multiple softmax
    {
        applyMultipleSoftmaxToInputWindow(reconstruction_activation, reconstruction_prob);
    
        double neg_log_cost = 0; // neg log softmax
        for(int k=0; k<reconstruction_prob.length(); k++)
            if(clean_input[k]!=0)
                neg_log_cost -= clean_input[k]*safelog(reconstruction_prob[k]);
        result_cost = neg_log_cost;
    }
    return result_cost;
}

//! Backpropagates reconstruction cost (after comparison with clean_input) with learning rate input_reconstruction_lr
//! accumulates gradient in hidden_gradient, and updates reconstruction_weights and input_reconstruction_bias
void DenoisingRecurrentNet::updateInputReconstructionFromHidden(Vec hidden, Mat& reconstruction_weights, Mat& acc_weights_gr, Vec& input_reconstruction_bias, Vec input_reconstruction_prob, 
                                                                Vec clean_input, Vec hidden_gradient, double input_reconstruction_cost_weight, double lr)
{
    // gradient of -log softmax is just  output_of_softmax - onehot_target
    // so let's accumulate this in hidden_gradient
    Vec input_reconstruction_activation_grad = input_reconstruction_prob;
    input_reconstruction_activation_grad -= clean_input;
    input_reconstruction_activation_grad *= input_reconstruction_cost_weight;

    // update bias
    multiplyAcc(input_reconstruction_bias, input_reconstruction_activation_grad, -lr);

    // update weight
    // THIS IS COMMENTED OUT BECAUSE THE reconstruction_weights ARE tied (same) TO THE input_connection weights, 
    // WHICH GET UPDATED LATER IN recurrentUpdate SO IF WE UPDATE THEM HERE THEY WOULD GET UPDATED TWICE.
    // WARNING: THIS WOULD NO LONGER BE THE CASE IF THEY WERE NOT TIED!
    externalProductScaleAcc(acc_weights_gr, hidden, input_reconstruction_activation_grad, -lr);

    // accumulate in hidden_gradient
    productAcc(hidden_gradient, reconstruction_weights, input_reconstruction_activation_grad);
}


double DenoisingRecurrentNet::fpropHiddenReconstructionFromLastHidden(Vec theInput, 
                                                                      Vec hidden, 
                                                                      Mat reconstruction_weights, 
                                                                      Mat& acc_weights_gr, 
                                                                      Vec& reconstruction_bias, 
                                                                      Vec& reconstruction_bias2, 
                                                                      Vec hidden_reconstruction_activation_grad, 
                                                                      Vec& reconstruction_prob, 
                                                                      Vec hidden_target, 
                                                                      Vec hidden_gradient, 
                                                                      double hidden_reconstruction_cost_weight, 
                                                                      double lr)
{
    // set appropriate sizes
    int fullhiddenlength = hidden_target.length();
    Vec reconstruction_activation;
    Vec hidden_input_noise;
    Vec hidden_fprop_noise;
    Vec hidden_act_no_bias;
    Vec hidden_exp;
    Vec dynamic_act_no_bias_contribution;
    if(reconstruction_bias.length()==0)
    {
        reconstruction_bias.resize(fullhiddenlength);
        reconstruction_bias.clear();
    }
    if(reconstruction_bias2.length()==0)
    {
        reconstruction_bias2.resize(fullhiddenlength);
        reconstruction_bias2.clear();
    }
    reconstruction_activation.resize(fullhiddenlength);
    reconstruction_prob.resize(fullhiddenlength);

    hidden_fprop_noise.resize(fullhiddenlength);
    hidden_input_noise.resize(fullhiddenlength);
    hidden_act_no_bias.resize(fullhiddenlength);
    hidden_exp.resize(fullhiddenlength);
    dynamic_act_no_bias_contribution.resize(fullhiddenlength);

    input_connections->fprop( theInput, hidden_act_no_bias);
    hidden_input_noise << hidden_target;
    inject_zero_forcing_noise(hidden_input_noise, input_noise_prob);
    dynamic_connections->fprop(hidden_input_noise, dynamic_act_no_bias_contribution );
    hidden_act_no_bias += dynamic_act_no_bias_contribution;
    hidden_layer->fprop( hidden_act_no_bias, hidden_exp);
    //hidden_act_no_bias += reconstruction_bias2;
    //for( int j=0 ; j<fullhiddenlength ; j++ )
    //    hidden_fprop_noise[j] = fastsigmoid(hidden_act_no_bias[j] );

    // predict (denoised) input_reconstruction 
    transposeProduct(reconstruction_activation, reconstruction_weights, hidden_exp); //dynamic matrice tied
    //product(reconstruction_activation, reconstruction_weights, hidden_exp); //dynamic matrice not tied
    reconstruction_activation += reconstruction_bias;

    for( int j=0 ; j<fullhiddenlength ; j++ )
        reconstruction_prob[j] = fastsigmoid( reconstruction_activation[j] );

    //hidden_layer->fprop(reconstruction_activation, reconstruction_prob);

    /********************************************************************************/
    hidden_reconstruction_activation_grad.resize(reconstruction_prob.size());
    hidden_reconstruction_activation_grad << reconstruction_prob;
    hidden_reconstruction_activation_grad -= hidden_target;
    hidden_reconstruction_activation_grad *= hidden_reconstruction_cost_weight;
    

    productAcc(hidden_gradient, reconstruction_weights, hidden_reconstruction_activation_grad); //dynamic matrice tied
    //transposeProductAcc(hidden_gradient, reconstruction_weights, hidden_reconstruction_activation_grad); //dynamic matrice not tied
    
    //update bias
    multiplyAcc(reconstruction_bias, hidden_reconstruction_activation_grad, -lr);
    // update weight
    externalProductScaleAcc(acc_weights_gr, hidden, hidden_reconstruction_activation_grad, -lr); //dynamic matrice tied
    //externalProductScaleAcc(acc_weights_gr, hidden_reconstruction_activation_grad, hidden, -lr); //dynamic matrice not tied
                
    //update bias2
    //multiplyAcc(reconstruction_bias2, hidden_gradient, -lr);
    /********************************************************************************/
    // Vec hidden_reconstruction_activation_grad;
    /*hidden_reconstruction_activation_grad.clear();
    for(int k=0; k<reconstruction_prob.length(); k++){
        //    hidden_reconstruction_activation_grad[k] = safelog(1-reconstruction_prob[k]) - safelog(reconstruction_prob[k]);
        hidden_reconstruction_activation_grad[k] = - reconstruction_activation[k];
        }*/

    double result_cost = 0;
    double neg_log_cost = 0; // neg log softmax
    for(int k=0; k<reconstruction_prob.length(); k++){
        //if(hidden_target[k]!=0)
        neg_log_cost -= hidden_target[k]*safelog(reconstruction_prob[k]) + (1-hidden_target[k])*safelog(1-reconstruction_prob[k]);
    }
    result_cost = neg_log_cost;
    
    return result_cost;
}

double DenoisingRecurrentNet::fpropHiddenSymmetricDynamicMatrix(Vec hidden, Mat reconstruction_weights, Vec& reconstruction_prob, 
                                                                 Vec hidden_target, Vec hidden_gradient, double hidden_reconstruction_cost_weight, double lr)
{
    // set appropriate sizes
    int fullinputlength = hidden_target.length();
    Vec reconstruction_activation;
   
    reconstruction_activation.resize(fullinputlength);
    reconstruction_prob.resize(fullinputlength);

    // predict (denoised) input_reconstruction 
    transposeProduct(reconstruction_activation, reconstruction_weights, hidden); //truc de stan
    //product(reconstruction_activation, reconstruction_weights, hidden); 
    //reconstruction_activation += hidden_layer->bias;
    
    hidden_layer->fprop(reconstruction_activation, reconstruction_prob);

    /********************************************************************************/
    Vec hidden_reconstruction_activation_grad;
    hidden_reconstruction_activation_grad.resize(reconstruction_prob.size());
    hidden_reconstruction_activation_grad << reconstruction_prob;
    hidden_reconstruction_activation_grad -= hidden_target;
    hidden_reconstruction_activation_grad *= hidden_reconstruction_cost_weight;

    productAcc(hidden_gradient, reconstruction_weights, hidden_reconstruction_activation_grad);
    /********************************************************************************/

    double result_cost = 0;
    double neg_log_cost = 0; // neg log softmax
    for(int k=0; k<reconstruction_prob.length(); k++)
        if(hidden_target[k]!=0)
            neg_log_cost -= hidden_target[k]*safelog(reconstruction_prob[k]);
    result_cost = neg_log_cost;
    
    return result_cost;
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
/*
void DenoisingRecurrentNet::recurrentUpdate(real input_reconstruction_weight,
                                            real hidden_reconstruction_weight,
                                            real temporal_gradient_contribution)
{
    hidden_temporal_gradient.resize(hidden_layer->size);
    hidden_temporal_gradient.clear();
    for(int i=hidden_list.length()-1; i>=0; i--){   

        if( hidden_layer2 )
            hidden_gradient.resize(hidden_layer2->size);
        else
            hidden_gradient.resize(hidden_layer->size);
        hidden_gradient.clear();
        if( prediction_cost_weight!=0 )
        {
            for( int tar=0; tar<target_layers.length(); tar++)
            {
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
                {
                    target_layers[tar]->activation << target_prediction_act_no_bias_list[tar](i);
                    target_layers[tar]->activation += target_layers[tar]->bias;
                    target_layers[tar]->setExpectation(target_prediction_list[tar](i));
                    target_layers[tar]->bpropNLL(targets_list[tar](i),nll_list(i,tar),bias_gradient);
                    bias_gradient *= prediction_cost_weight;
                    if(use_target_layers_masks)
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
        }
            
        // Add contribution of input reconstruction cost in hidden_gradient
        if(input_reconstruction_weight!=0)
        {
            Mat reconstruction_weights = getInputConnectionsWeightMatrix();
            Vec clean_input = clean_encoded_seq.subMatRows(i, input_window_size).toVec();

            fpropUpdateInputReconstructionFromHidden(hidden_list(i), reconstruction_weights, input_reconstruction_bias, input_reconstruction_prob, 
                                                     clean_input, hidden_gradient, hidden_reconstruction_weight, current_learning_rate);
        }


        if(i!=0 && dynamic_connections )
        {   


            hidden_layer->bpropUpdate(
                hidden_act_no_bias_list(i), hidden_list(i),
                hidden_temporal_gradient, hidden_gradient);
            input_connections->bpropUpdate(
                input_list[i],
                hidden_act_no_bias_list(i), 
                visi_bias_gradient, hidden_temporal_gradient);// Here, it should be activations - cond_bias, but doesn't matter
                


            // Add contribution of hidden reconstruction cost in hidden_gradient
            if(hidden_reconstruction_weight!=0)
            {
                Mat reconstruction_weights = getDynamicConnectionsWeightMatrix();
                //truc stan
                fpropHiddenReconstructionFromLastHidden(hidden_list(i-1), reconstruction_weights, hidden_reconstruction_prob, hidden_list(i), hidden_gradient, hidden_reconstruction_weight, current_learning_rate);
                //fpropHiddenReconstructionFromLastHidden(hidden_list(i), reconstruction_weights, hidden_reconstruction_prob, hidden_list(i-1), hidden_gradient, hidden_reconstruction_weight, current_learning_rate);
            
            }
            // add contribution to gradient of next time step hidden layer
            if(temporal_gradient_contribution>0)
            { // add weighted contribution of hidden_temporal gradient to hidden_gradient
                // It does this: hidden_gradient += temporal_gradient_contribution*hidden_temporal_gradient;
                multiplyAcc(hidden_gradient, hidden_temporal_gradient, temporal_gradient_contribution);
            }
  

            hidden_layer->bpropUpdate(
                hidden_act_no_bias_list(i), hidden_list(i),
                hidden_temporal_gradient, hidden_gradient);
                
            dynamic_connections->bpropUpdate(
                hidden_list(i-1),
                hidden_act_no_bias_list(i), // Here, it should be dynamic_act_no_bias_contribution, but doesn't matter because a RBMMatrixConnection::bpropUpdate doesn't use its second argument
                hidden_gradient, hidden_temporal_gradient);
                
            
            hidden_temporal_gradient << hidden_gradient;                
        }
        else
        {
            hidden_layer->bpropUpdate(
                hidden_act_no_bias_list(i), hidden_list(i),
                hidden_temporal_gradient, hidden_gradient); // Not really temporal gradient, but this is the final iteration...
            input_connections->bpropUpdate(
                input_list[i],
                hidden_act_no_bias_list(i), 
                visi_bias_gradient, hidden_temporal_gradient);// Here, it should be activations - cond_bias, but doesn't matter

        }
    }
    
}

*/
void DenoisingRecurrentNet::recurrentUpdate(real input_reconstruction_weight,
                                            real hidden_reconstruction_weight,
                                            real temporal_gradient_contribution,
                                            real predic_cost_weight,
                                            real inputAndDynamicPart,
                                            Vec train_costs, 
                                            Vec train_n_items )
{
    TVec < Mat> targetWeights ;
    Mat inputWeights;
    Mat dynamicWeights;
    Mat reconsWeights;
    targetWeights.resize(target_connections.length());
    for( int tar=0; tar<target_layers.length(); tar++)
    {
       targetWeights[tar] = getTargetConnectionsWeightMatrix(tar);
    }
    inputWeights = getInputConnectionsWeightMatrix();
    if(dynamic_connections )
    { 
        dynamicWeights = getDynamicConnectionsWeightMatrix();
        reconsWeights = getDynamicReconstructionConnectionsWeightMatrix();
    }
    acc_target_connections_gr.resize(target_connections.length());
    for( int tar=0; tar<target_layers.length(); tar++)
    {
        acc_target_connections_gr[tar].resize(target_connections[tar]->up_size, target_connections[tar]->down_size);
        acc_target_connections_gr[tar].clear();
    }
    acc_input_connections_gr.resize(input_connections->up_size, input_connections->down_size);
    acc_input_connections_gr.clear();
    if(dynamic_connections )
    { 
        acc_dynamic_connections_gr.resize(dynamic_connections->up_size, dynamic_connections->down_size);
        acc_dynamic_connections_gr.clear();
        acc_reconstruction_dynamic_connections_gr.resize(dynamic_connections->down_size, dynamic_connections->up_size);
        acc_reconstruction_dynamic_connections_gr.clear();
    }


    hidden_temporal_gradient.resize(hidden_layer->size);
    hidden_temporal_gradient.clear();
    for(int i=hidden_list.length()-1; i>=0; i--){   

        if( hidden_layer2 )
            hidden_gradient.resize(hidden_layer2->size);
        else
            hidden_gradient.resize(hidden_layer->size);
        hidden_gradient.clear();
        if( predic_cost_weight!=0 )
        {
            for( int tar=0; tar<target_layers.length(); tar++)
            {
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
                {
                    target_layers[tar]->activation << target_prediction_act_no_bias_list[tar](i);
                    target_layers[tar]->activation += target_layers[tar]->bias;
                    target_layers[tar]->setExpectation(target_prediction_list[tar](i));
                    target_layers[tar]->bpropNLL(targets_list[tar](i),nll_list(i,tar),bias_gradient);
                    bias_gradient *= predic_cost_weight;
                    if(use_target_layers_masks)
                        bias_gradient *= masks_list[tar](i);
                    //target_layers[tar]->update(bias_gradient);
                    updateTargetLayer( bias_gradient, 
                                       target_layers[tar]->bias, 
                                       target_layers[tar]->learning_rate );
                    //Mat targetWeights = getTargetConnectionsWeightMatrix(tar);
                    if( hidden_layer2 ){
                        //target_connections[tar]->bpropUpdate(hidden2_list(i),target_prediction_act_no_bias_list[tar](i),hidden_gradient, bias_gradient,true);
                        bpropUpdateConnection(hidden2_list(i),
                                              target_prediction_act_no_bias_list[tar](i),
                                              hidden_gradient, 
                                              bias_gradient,
                                              targetWeights[tar],
                                              acc_target_connections_gr[tar],
                                              target_connections[tar]->down_size,
                                              target_connections[tar]->up_size,
                                              target_connections[tar]->learning_rate,
                                              true,
                                              false);
                    }
                    else{
                        //target_connections[tar]->bpropUpdate(hidden_list(i),target_prediction_act_no_bias_list[tar](i),hidden_gradient, bias_gradient,true);
                        bpropUpdateConnection(hidden_list(i),
                                              target_prediction_act_no_bias_list[tar](i),
                                              hidden_gradient, 
                                              bias_gradient,
                                              targetWeights[tar],
                                              acc_target_connections_gr[tar],
                                              target_connections[tar]->down_size,
                                              target_connections[tar]->up_size,
                                              target_connections[tar]->learning_rate,
                                              true,
                                              false);
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
        }

        if(inputAndDynamicPart){   
            // Add contribution of input reconstruction cost in hidden_gradient
            if(input_reconstruction_weight!=0)
            {
                //Mat reconstruction_weights = getInputConnectionsWeightMatrix();
                //Vec clean_input = clean_encoded_seq.subMatRows(i, input_window_size).toVec();
                
                train_costs[train_costs.length()-2] += fpropUpdateInputReconstructionFromHidden(hidden_list(i), inputWeights, acc_input_connections_gr, input_reconstruction_bias, input_reconstruction_prob, 
                                                                           input_list[i], hidden_gradient, input_reconstruction_weight, current_learning_rate);
                train_n_items[train_costs.length()-2]++;
            }
            
            
            if(i!=0 && dynamic_connections )
            {   
                
                // Add contribution of hidden reconstruction cost in hidden_gradient
                Vec hidden_reconstruction_activation_grad;
                hidden_reconstruction_activation_grad.resize(hidden_layer->size);
                //Mat reconstruction_weights = getDynamicConnectionsWeightMatrix();
                if(hidden_reconstruction_weight!=0)
                {
                    //Vec hidden_reconstruction_activation_grad;
                    //Mat reconstruction_weights = getDynamicConnectionsWeightMatrix();
                    
                    //truc stan
                    //fpropHiddenSymmetricDynamicMatrix(hidden_list(i-1), reconstruction_weights, hidden_reconstruction_prob, hidden_list(i), hidden_gradient, hidden_reconstruction_weight, current_learning_rate);
                    train_costs[train_costs.length()-1] += fpropHiddenReconstructionFromLastHidden(input_list[i], 
                                                                                                   hidden_list(i), 
                                                                                                   dynamicWeights, //reconsWeights, //dynamicWeights, 
                                                                                                   acc_dynamic_connections_gr, //acc_reconstruction_dynamic_connections_gr, //acc_dynamic_connections_gr, 
                                                                                                   hidden_reconstruction_bias, 
                                                                                                   hidden_reconstruction_bias2, 
                                                                                                   hidden_reconstruction_activation_grad, 
                                                                                                   hidden_reconstruction_prob, 
                                                                                                   hidden_list(i-1), 
                                                                                                   hidden_gradient, 
                                                                                                   hidden_reconstruction_weight, 
                                                                                                   current_learning_rate);
                    //fpropHiddenReconstructionFromLastHidden(hidden_list(i), reconsWeights, acc_reconstruction_dynamic_connections_gr, hidden_reconstruction_bias, hidden_reconstruction_activation_grad, hidden_reconstruction_prob, hidden_list(i-1), hidden_gradient, hidden_reconstruction_weight, current_learning_rate);
                    train_n_items[train_costs.length()-1]++;
                }
                
                
                // add contribution to gradient of next time step hidden layer
                if(temporal_gradient_contribution>0)
                { // add weighted contribution of hidden_temporal gradient to hidden_gradient
                    // It does this: hidden_gradient += temporal_gradient_contribution*hidden_temporal_gradient;
                    multiplyAcc(hidden_gradient, hidden_temporal_gradient, temporal_gradient_contribution);
                    
                }
                
                
                
                
               
                bpropUpdateHiddenLayer(hidden_act_no_bias_list(i), 
                                       hidden_list(i),
                                       hidden_temporal_gradient, 
                                       hidden_gradient,
                                       hidden_layer->bias, 
                                       hidden_layer->learning_rate );
                
                
                //input
                //if(hidden_reconstruction_weight==0)
                //{
                   
                    
                bpropUpdateConnection(input_list[i],
                                      hidden_act_no_bias_list(i), 
                                      visi_bias_gradient, 
                                      hidden_temporal_gradient,// Here, it should be activations - cond_bias, but doesn't matter
                                      inputWeights,
                                      acc_input_connections_gr,
                                      input_connections->down_size,
                                      input_connections->up_size,
                                      input_connections->learning_rate,
                                      false,
                                      true);
                    //}
                
                //Dynamic
                //if(input_reconstruction_weight==0)
                //{
                    /*bpropUpdateHiddenLayer(hidden_act_no_bias_list(i), 
                                       hidden_list(i),
                                       hidden_temporal_gradient, 
                                       hidden_gradient,
                                       hidden_layer->bias, 
                                       hidden_layer->learning_rate );*/

                bpropUpdateConnection(hidden_list(i-1),
                                      hidden_act_no_bias_list(i), // Here, it should be dynamic_act_no_bias_contribution, but doesn't matter because a RBMMatrixConnection::bpropUpdate doesn't use its second argument
                                      hidden_gradient, 
                                      hidden_temporal_gradient, 
                                      dynamicWeights,
                                      acc_dynamic_connections_gr,
                                      dynamic_connections->down_size,
                                      dynamic_connections->up_size,
                                      dynamic_connections->learning_rate,
                                      false,
                                      false);
                    //}
                
                hidden_temporal_gradient << hidden_gradient; 
                //if(hidden_reconstruction_weight!=0)
                //    hidden_temporal_gradient +=  hidden_reconstruction_activation_grad;
            }
            else
            {
                if(input_reconstruction_weight==0)
                {
                    bpropUpdateHiddenLayer(hidden_act_no_bias_list(i), 
                                           hidden_list(i),
                                           hidden_temporal_gradient, // Not really temporal gradient, but this is the final iteration...
                                           hidden_gradient,
                                           hidden_layer->bias, 
                                           hidden_layer->learning_rate );
                    
                    //input
                    bpropUpdateConnection(input_list[i],
                                          hidden_act_no_bias_list(i), 
                                          visi_bias_gradient, 
                                          hidden_temporal_gradient,// Here, it should be activations - cond_bias, but doesn't matter
                                          inputWeights,
                                          acc_input_connections_gr,
                                          input_connections->down_size,
                                          input_connections->up_size,
                                          input_connections->learning_rate,
                                          false,
                                          true);
                }
            }
        }
    }
    //update matrice's connections
    for( int tar=0; tar<target_layers.length(); tar++)
    {
        multiplyAcc(targetWeights[tar], acc_target_connections_gr[tar], 1);
    }
    multiplyAcc(inputWeights, acc_input_connections_gr, 1);
    if(dynamic_connections )
    {
        multiplyAcc(dynamicWeights, acc_dynamic_connections_gr, 1);
        //multiplyAcc(reconsWeights, acc_reconstruction_dynamic_connections_gr, 1);
    }
}


/* TO DO:
verifier nombre de temps
implementer correctement duration_to_number_of_timeframes(duration)
declare nouvelles options et valeurs par defaut correctes
*/


/*
  
Frequences dans le trainset:

**NOTES**
0  DO            0.0872678308077029924            
1  DO#           0.00716010857716887095                   
2  RE            0.178895847137025221                   
3  RE#           0.0037189817684399511                   
4  MI            0.114241135358112283                   
5  FA            0.00517237694231303512                   
6  FA#           0.0806848056083954851                   
7  SOL           0.194776326757432616                   
8  SOL#          0.00301365763994271892                   
9  LA            0.13988928548528437                   
10 LA#           0.00369760831000064084                   
11 SI            0.181482035608181741                   

**OCTAVES**
0  OCT1          0.362130506337230429                   
1  OCT2          0.574048346762989659                   
2  OCT3          0.0635219184816295107                   
3  OCT4          0.000299228418150340866                

**DUREES**
0  1/8           0.00333425951653236984                   
1  1/6           0.000170987667514480506                   
2  1/4           0.0386432128582725951                   
3  1/3           0.00716010857716887095                   
4  2/4           0.569880522367324227                   
5  2/3           0                               
6  3/4           0.00220146621924893673                   
7  4/4           0.305896937183405604                   
8  5/4           4.27469168786201266e-05                   
9  6/4           0.0222283967768824656                   
10 8/4           0.0365058670143415878                   
11 10/4          0.000876311796011712552                   
12 12/4          0.0078440592472267933                   
13 14/4          6.41203753179301933e-05                   
14 16/4          0.00331288605809306001                   
15 18/4          8.54938337572402532e-05                   
16 20/4          0.000726697586936542119                   
17 24/4          0.000619830294739991887                   
18 28/4          0.000149614209075170433                   
19 32/4          0.000256481501271720773         

 */


/*
Format de donnees:

matrice de 2 colonnes:
note, duree

note: midi_number (21..108 numero de touche sur piano)
      ou 0  (silence)
      ou -1 (missing)
      ou -999 (fin de sequence)

duree: voir indices (colonne de gauche) et DUREES dans table de frequences ci-dessus
       1 unite correspond a une noire.

 */

void DenoisingRecurrentNet::encodeSequence(Mat sequence, Mat& encoded_seq) const
{
    //! Possibilities: "timeframe", "note_duration", "note_octav_duration", "generic"
    int prepend_zero_rows = input_window_size;

    // reserve some minimum space for encoded_seq
    encoded_seq.resize(5000, 4);

    if(encoding=="timeframe")
        encode_onehot_timeframe(sequence, encoded_seq, prepend_zero_rows);
    else if(encoding=="note_duration")
        encode_onehot_note_octav_duration(sequence, encoded_seq, prepend_zero_rows, false, 0);
    else if(encoding=="note_octav_duration")
        encode_onehot_note_octav_duration(sequence, encoded_seq, prepend_zero_rows, false, 4);    
    else if(encoding=="raw_masked_supervised")
        PLERROR("raw_masked_supervised means already encoded! You shouldnt have landed here!!!");
    else if(encoding=="generic")
        PLERROR("generic means already encoded! You shouldnt have landed here!!!");
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
    boundaries.resize(10000);
    boundaries.resize(0);
    int l = dataset->length();
    for(int i=0; i<l; i++)
    {
        if(dataset(i,0)==end_of_sequence_symbol)
            boundaries.append(i);
    }
}


int DenoisingRecurrentNet::getDurationBit(int duration)
{
    if(duration==5)  // map infrequent 5 to 4
        duration=4;
    return duration;
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
                                                              bool use_silence, int octav_nbits, int duration_nbits)
{
    int l = sequence.length();
    int note_nbits = use_silence ?13 :12;

    encoded_sequence.resize(prepend_zero_rows+l,note_nbits+octav_nbits+duration_nbits);
    encoded_sequence.clear();
    int octav_min = 10000;
    int octav_max = -10000;

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

        int duration_bit = getDurationBit(int(sequence(i,1)));
        if(duration_bit<0 || duration_bit>=duration_nbits)
            PLERROR("duration_bit out of valid range");
        encoded_sequence(prepend_zero_rows+i,note_nbits+octav_nbits+duration_bit) = 1;
    }
}


int DenoisingRecurrentNet::duration_to_number_of_timeframes(int duration)
{
    PLERROR("duration_to_number_of_timeframes (used only when encoding==timeframe) is not yet implemented");
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
void DenoisingRecurrentNet::inject_zero_forcing_noise(Mat sequence, double noise_prob) const
{
    if(!sequence.isCompact())
        PLERROR("Expected a compact sequence");
    real* p = sequence.data();
    int n = sequence.size();
    while(n--)
    {
        if(*p!=real(0.) && random_gen->uniform_sample()<noise_prob)
            *p = real(0.);
        ++p;
    }
}

// input noise injection
void DenoisingRecurrentNet::inject_zero_forcing_noise(Vec sequence, double noise_prob) const
{
    
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
    current_learning_rate = the_learning_rate;
    input_layer->setLearningRate( the_learning_rate );
    hidden_layer->setLearningRate( the_learning_rate );
    input_connections->setLearningRate( the_learning_rate );
    if( dynamic_connections ){
        //dynamic_connections->setLearningRate( dynamic_gradient_scale_factor*the_learning_rate ); 
        dynamic_connections->setLearningRate( the_learning_rate ); 
    }
    if( dynamic_reconstruction_connections ){
        //dynamic_reconstruction_connections->setLearningRate( dynamic_gradient_scale_factor*the_learning_rate ); 
        dynamic_reconstruction_connections->setLearningRate( the_learning_rate ); 
    }
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
    encoded_seq.resize(5000, 4);


    int pos = 0; // position in testoutputs
    for(int i=0; i<nseq; i++)
    {
        int start = 0;
        if(i>0)
            start = testset_boundaries[i-1]+1;
        int end = testset_boundaries[i];
        int seqlen = end-start; // target_prediction_list[0].length();
        seq.resize(seqlen, w);
        testset->getMat(start,0,seq);
        encodeSequenceAndPopulateLists(seq, false);

        if(input_window_size==0)
            unconditionalFprop(costs, n_items);
        else
            recurrentFprop(costs, n_items);

        if (testoutputs)
        {
            for(int t=0; t<seqlen; t++)
            {
                int sum_target_layers_size = 0;
                for( int tar=0; tar < target_layers.length(); tar++ )
                {
                    if( !fast_exact_is_equal(target_layers_weights[tar],0) )
                    {
                        output.subVec(sum_target_layers_size,target_layers[tar]->size)
                            << target_prediction_list[tar](t);
                    }
                    sum_target_layers_size += target_layers[tar]->size;
                }
                testoutputs->putOrAppendRow(pos++, output);
            }
            output.fill(end_of_sequence_symbol);
            testoutputs->putOrAppendRow(pos++, output);
        }
        else
            pos += seqlen;

        if (report_progress)
            pb->update(pos);
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
        test_stats->update(costs, 1.);
}


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

/*
void DenoisingRecurrentNet::generate(int t, int n)
{
    PLERROR("generate not yet implemented");
}
*/


void DenoisingRecurrentNet::generate(int t, int n)
{
    //PPath* the_filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/generate/scoreGen.amat";
    data = new AutoVMatrix();
    //data->filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/listData/target_tm12_input_t_tm12_tp12/scoreGen_tar_tm12__in_tm12_tp12.amat";
    //data->filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/create_data/scoreGenSuitePerf.amat";
    data->filename = "/home/stan/cvs/Gamme/expressive_data/dataGen.amat";

    data->defineSizes(163,16,0);
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
    int targsize;

    Vec output(outputsize());
    output.clear();
//     Vec costs(nTestCosts());
//     costs.clear();
//     Vec n_items(nTestCosts());
//     n_items.clear();

    int r,r2;
    use_target_layers_masks = true;

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
                        
                        input.subVec(inputsize_without_masks-(tarSize*(t-k))-partTarSize-1,target_layers[tar]->size) << target_prediction_list[tar](ith_sample_in_sequence-k);
                        partTarSize -= target_layers[tar]->size;
                        
                        
                    }
                }
            }       
        }
    

//         for (int k = 1; k <= t; k++)
//         {
//             partTarSize = outputsize();
//             for( int tar=0; tar < target_layers.length(); tar++ )
//             {
//                 if(i>=t){
//                     input.subVec(inputsize_without_masks-(tarSize*(t-k))-partTarSize-1,target_layers[tar]->size) << target_prediction_list[tar](ith_sample_in_sequence-k);
//                     partTarSize -= target_layers[tar]->size;
//                 }
//             }
//         }

        if( fast_exact_is_equal(input[0],end_of_sequence_symbol) )
        {
//             ith_sample_in_sequence = 0;
//             hidden_list.resize(0);
//             hidden_act_no_bias_list.resize(0);
//             hidden2_list.resize(0);
//             hidden2_act_no_bias_list.resize(0);
//             target_prediction_list.resize(0);
//             target_prediction_act_no_bias_list.resize(0);
//             input_list.resize(0);
//             targets_list.resize(0);
//             nll_list.resize(0,0);
//             masks_list.resize(0);

            

            continue;
        }

        // Resize internal variables
        hidden_list.resize(ith_sample_in_sequence+1, hidden_layer->size);
        hidden_act_no_bias_list.resize(ith_sample_in_sequence+1, hidden_layer->size);
        if( hidden_layer2 )
        {
            hidden2_list.resize(ith_sample_in_sequence+1, hidden_layer2->size);
            hidden2_act_no_bias_list.resize(ith_sample_in_sequence+1, hidden_layer2->size);
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
                targsize = target_layers[tar]->size;
                targets_list[tar].resize( ith_sample_in_sequence+1, targsize);
                //targets_list[tar][ith_sample_in_sequence].resize( target_layers[tar]->size);
                target_prediction_list[tar].resize(
                    ith_sample_in_sequence+1, targsize);
                target_prediction_act_no_bias_list[tar].resize(
                    ith_sample_in_sequence+1, targsize);
            }
        }
        nll_list.resize(ith_sample_in_sequence+1,target_layers.length());
        if( use_target_layers_masks )
        {
            masks_list.resize( target_layers.length() );
            for( int tar=0; tar < target_layers.length(); tar++ )
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
                    masks_list[tar].resize( ith_sample_in_sequence+1, target_layers[tar]->size );
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
                    Vec masks_list_tar_i = masks_list[tar](ith_sample_in_sequence);
                    clamp_units(target.subVec(
                                    sum_target_elements,
                                    target_layers_n_of_target_elements[tar]),
                                target_layers[tar],
                                target_symbol_sizes[tar],
                                input.subVec(
                                    inputsize_without_masks 
                                    + sum_target_elements, 
                                    target_layers_n_of_target_elements[tar]),
                                masks_list_tar_i
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
                targets_list[tar](ith_sample_in_sequence) << 
                    target_layers[tar]->expectation;
            }
            sum_target_elements += target_layers_n_of_target_elements[tar];
        }
        
        Vec hidden_act_no_bias_i = hidden_act_no_bias_list(ith_sample_in_sequence);
        input_connections->fprop( input_list[ith_sample_in_sequence], 
                                  hidden_act_no_bias_i);
                
        if( ith_sample_in_sequence > 0 && dynamic_connections )
        {
            dynamic_connections->fprop( 
                hidden_list(ith_sample_in_sequence-1),
                dynamic_act_no_bias_contribution );

            hidden_act_no_bias_list(ith_sample_in_sequence) += 
                dynamic_act_no_bias_contribution;
        }
        
        Vec hidden_i = hidden_list(ith_sample_in_sequence);
        hidden_layer->fprop( hidden_act_no_bias_i, 
                             hidden_i );

        Vec last_hidden = hidden_i;
                 
        if( hidden_layer2 )
        {
            Vec hidden2_i = hidden2_list(ith_sample_in_sequence); 
            Vec hidden2_act_no_bias_i = hidden2_act_no_bias_list(ith_sample_in_sequence);

            hidden_connections->fprop( 
                hidden2_i,
                hidden2_act_no_bias_i);

            hidden_layer2->fprop( 
                hidden2_act_no_bias_i,
                hidden2_i 
                );

            last_hidden = hidden2_i; // last hidden layer vec 
        }
           
       
        for( int tar=0; tar < target_layers.length(); tar++ )
        {
            if( !fast_exact_is_equal(target_layers_weights[tar],0) )
            {
                Vec target_prediction_i = target_prediction_list[tar](i);
                Vec target_prediction_act_no_bias_i = target_prediction_act_no_bias_list[tar](i);
                target_connections[tar]->fprop(
                    last_hidden,
                    target_prediction_act_no_bias_i
                    );
                target_layers[tar]->fprop(
                    target_prediction_act_no_bias_i,
                    target_prediction_i );
                if( use_target_layers_masks )
                    target_prediction_i *= masks_list[tar](ith_sample_in_sequence);
            }
        }
        

        

        sum_target_elements = 0;
        for( int tar=0; tar < target_layers.length(); tar++ )
        {
            if( !fast_exact_is_equal(target_layers_weights[tar],0) )
            {
                target_layers[tar]->activation << 
                    target_prediction_act_no_bias_list[tar](
                        ith_sample_in_sequence);
                target_layers[tar]->activation += target_layers[tar]->bias;
                target_layers[tar]->setExpectation(
                    target_prediction_list[tar](
                        ith_sample_in_sequence));
                nll_list(ith_sample_in_sequence,tar) = 
                    target_layers[tar]->fpropNLL( 
                        targets_list[tar](ith_sample_in_sequence) ); 
//                 costs[tar] += nll_list(ith_sample_in_sequence,tar);
                
//                 // Normalize by the number of things to predict
//                 if( use_target_layers_masks )
//                 {
//                     n_items[tar] += sum(
//                         input.subVec( inputsize_without_masks 
//                                       + sum_target_elements, 
//                                       target_layers_n_of_target_elements[tar]) );
//                 }
//                 else
//                 n_items[tar]++;
            }
            if( use_target_layers_masks )
                sum_target_elements += 
                    target_layers_n_of_target_elements[tar];
        }
        ith_sample_in_sequence++;

        

    }

//     ith_sample_in_sequence = 0;
//     hidden_list.resize(0);
//     hidden_act_no_bias_list.resize(0);
//     hidden2_list.resize(0);
//     hidden2_act_no_bias_list.resize(0);
//     target_prediction_list.resize(0);
//     target_prediction_act_no_bias_list.resize(0);
//     input_list.resize(0);
//     targets_list.resize(0);
//     nll_list.resize(0,0);
//     masks_list.resize(0);   

    
    //Vec tempo;
    //TVec<real> tempo;
    //tempo.resize(visible_layer->size);
    ofstream myfile;
    myfile.open ("/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/generate/test.txt");
    
    for (int i = 0; i < target_prediction_list[0].length() ; i++ ){
       
       
        for( int tar=0; tar < target_layers.length(); tar++ )
        {
            for (int j = 0; j < target_prediction_list[tar](i).length() ; j++ ){
                
                //if(i>n){
                    myfile << target_prediction_list[tar](i)[j] << " ";
                    // }
                    //else{
                    //    myfile << targets_list[tar](i)[j] << " ";
                    // }
                       
           
            }
        }
        myfile << "\n";
    }
     

     myfile.close();

}

void DenoisingRecurrentNet::generateArtificial()
{
    //PPath* the_filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/generate/scoreGen.amat";
    data = new AutoVMatrix();
    //data->filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/data/listData/target_tm12_input_t_tm12_tp12/scoreGen_tar_tm12__in_tm12_tp12.amat";
    //data->filename = "/home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/create_data/scoreGenSuitePerf.amat";
    //data->filename = "/home/stan/cvs/Gamme/expressive_data/dataGen.amat";
    data->filename = "/home/stan/Documents/recherche_maitrise/artificialData/generate/dataGen.amat";
    data->defineSizes(1,1,0);
    //data->defineSizes(163,16,0);
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
    int targsize;

    Vec output(outputsize());
    output.clear();
//     Vec costs(nTestCosts());
//     costs.clear();
//     Vec n_items(nTestCosts());
//     n_items.clear();

    int r,r2;
    use_target_layers_masks = false;

    int ith_sample_in_sequence = 0;
    int inputsize_without_masks = inputsize() 
        - ( use_target_layers_masks ? targetsize() : 0 );
    int sum_target_elements = 0;
    for (int i = 0; i < len; i++)
    {
        data->getExample(i, input, target, weight);
        /*if(i>n)
        {
            for (int k = 1; k <= t; k++)
            {
                if(k<=i){
                    partTarSize = outputsize();
                    for( int tar=0; tar < target_layers.length(); tar++ )
                    {
                        
                        input.subVec(inputsize_without_masks-(tarSize*(t-k))-partTarSize-1,target_layers[tar]->size) << target_prediction_list[tar](ith_sample_in_sequence-k);
                        partTarSize -= target_layers[tar]->size;
                        
                        
                    }
                }
            }       
            }*/
    

//         for (int k = 1; k <= t; k++)
//         {
//             partTarSize = outputsize();
//             for( int tar=0; tar < target_layers.length(); tar++ )
//             {
//                 if(i>=t){
//                     input.subVec(inputsize_without_masks-(tarSize*(t-k))-partTarSize-1,target_layers[tar]->size) << target_prediction_list[tar](ith_sample_in_sequence-k);
//                     partTarSize -= target_layers[tar]->size;
//                 }
//             }
//         }

        if( fast_exact_is_equal(input[0],end_of_sequence_symbol) )
        {
//             ith_sample_in_sequence = 0;
//             hidden_list.resize(0);
//             hidden_act_no_bias_list.resize(0);
//             hidden2_list.resize(0);
//             hidden2_act_no_bias_list.resize(0);
//             target_prediction_list.resize(0);
//             target_prediction_act_no_bias_list.resize(0);
//             input_list.resize(0);
//             targets_list.resize(0);
//             nll_list.resize(0,0);
//             masks_list.resize(0);

            

            continue;
        }

        // Resize internal variables
        hidden_list.resize(ith_sample_in_sequence+1, hidden_layer->size);
        hidden_act_no_bias_list.resize(ith_sample_in_sequence+1, hidden_layer->size);
        if( hidden_layer2 )
        {
            hidden2_list.resize(ith_sample_in_sequence+1, hidden_layer2->size);
            hidden2_act_no_bias_list.resize(ith_sample_in_sequence+1, hidden_layer2->size);
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
                targsize = target_layers[tar]->size;
                targets_list[tar].resize( ith_sample_in_sequence+1, targsize);
                //targets_list[tar][ith_sample_in_sequence].resize( target_layers[tar]->size);
                target_prediction_list[tar].resize(
                    ith_sample_in_sequence+1, targsize);
                target_prediction_act_no_bias_list[tar].resize(
                    ith_sample_in_sequence+1, targsize);
            }
        }
        nll_list.resize(ith_sample_in_sequence+1,target_layers.length());
        if( use_target_layers_masks )
        {
            masks_list.resize( target_layers.length() );
            for( int tar=0; tar < target_layers.length(); tar++ )
                if( !fast_exact_is_equal(target_layers_weights[tar],0) )
                    masks_list[tar].resize( ith_sample_in_sequence+1, target_layers[tar]->size );
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
                    Vec masks_list_tar_i = masks_list[tar](ith_sample_in_sequence);
                    clamp_units(target.subVec(
                                    sum_target_elements,
                                    target_layers_n_of_target_elements[tar]),
                                target_layers[tar],
                                target_symbol_sizes[tar],
                                input.subVec(
                                    inputsize_without_masks 
                                    + sum_target_elements, 
                                    target_layers_n_of_target_elements[tar]),
                                masks_list_tar_i
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
                targets_list[tar](ith_sample_in_sequence) << 
                    target_layers[tar]->expectation;
            }
            sum_target_elements += target_layers_n_of_target_elements[tar];
        }
        
        Vec hidden_act_no_bias_i = hidden_act_no_bias_list(ith_sample_in_sequence);
        input_connections->fprop( input_list[ith_sample_in_sequence], 
                                  hidden_act_no_bias_i);
                
        if( ith_sample_in_sequence > 0 && dynamic_connections )
        {
            dynamic_connections->fprop( 
                hidden_list(ith_sample_in_sequence-1),
                dynamic_act_no_bias_contribution );

            hidden_act_no_bias_list(ith_sample_in_sequence) += 
                dynamic_act_no_bias_contribution;
        }
        
        Vec hidden_i = hidden_list(ith_sample_in_sequence);
        hidden_layer->fprop( hidden_act_no_bias_i, 
                             hidden_i );

        Vec last_hidden = hidden_i;
                 
        if( hidden_layer2 )
        {
            Vec hidden2_i = hidden2_list(ith_sample_in_sequence); 
            Vec hidden2_act_no_bias_i = hidden2_act_no_bias_list(ith_sample_in_sequence);

            hidden_connections->fprop( 
                hidden2_i,
                hidden2_act_no_bias_i);

            hidden_layer2->fprop( 
                hidden2_act_no_bias_i,
                hidden2_i 
                );

            last_hidden = hidden2_i; // last hidden layer vec 
        }
           
       
        for( int tar=0; tar < target_layers.length(); tar++ )
        {
            if( !fast_exact_is_equal(target_layers_weights[tar],0) )
            {
                Vec target_prediction_i = target_prediction_list[tar](i);
                Vec target_prediction_act_no_bias_i = target_prediction_act_no_bias_list[tar](i);
                target_connections[tar]->fprop(
                    last_hidden,
                    target_prediction_act_no_bias_i
                    );
                target_layers[tar]->fprop(
                    target_prediction_act_no_bias_i,
                    target_prediction_i );
                if( use_target_layers_masks )
                    target_prediction_i *= masks_list[tar](ith_sample_in_sequence);
            }
        }
        

        

        sum_target_elements = 0;
        for( int tar=0; tar < target_layers.length(); tar++ )
        {
            if( !fast_exact_is_equal(target_layers_weights[tar],0) )
            {
                target_layers[tar]->activation << 
                    target_prediction_act_no_bias_list[tar](
                        ith_sample_in_sequence);
                target_layers[tar]->activation += target_layers[tar]->bias;
                target_layers[tar]->setExpectation(
                    target_prediction_list[tar](
                        ith_sample_in_sequence));
                nll_list(ith_sample_in_sequence,tar) = 
                    target_layers[tar]->fpropNLL( 
                        targets_list[tar](ith_sample_in_sequence) ); 
//                 costs[tar] += nll_list(ith_sample_in_sequence,tar);
                
//                 // Normalize by the number of things to predict
//                 if( use_target_layers_masks )
//                 {
//                     n_items[tar] += sum(
//                         input.subVec( inputsize_without_masks 
//                                       + sum_target_elements, 
//                                       target_layers_n_of_target_elements[tar]) );
//                 }
//                 else
//                 n_items[tar]++;
            }
            if( use_target_layers_masks )
                sum_target_elements += 
                    target_layers_n_of_target_elements[tar];
        }
        ith_sample_in_sequence++;

        

    }

//     ith_sample_in_sequence = 0;
//     hidden_list.resize(0);
//     hidden_act_no_bias_list.resize(0);
//     hidden2_list.resize(0);
//     hidden2_act_no_bias_list.resize(0);
//     target_prediction_list.resize(0);
//     target_prediction_act_no_bias_list.resize(0);
//     input_list.resize(0);
//     targets_list.resize(0);
//     nll_list.resize(0,0);
//     masks_list.resize(0);   

    
    //Vec tempo;
    //TVec<real> tempo;
    //tempo.resize(visible_layer->size);
    ofstream myfile;
    myfile.open ("/home/stan/Documents/recherche_maitrise/artificialData/generate/generationResult.txt");
    
    for (int i = 0; i < target_prediction_list[0].length() ; i++ ){
       
       
        for( int tar=0; tar < target_layers.length(); tar++ )
        {
            for (int j = 0; j < target_prediction_list[tar](i).length() ; j++ ){
                
                //if(i>n){
                    myfile << target_prediction_list[tar](i)[j] << " ";
                    myfile << targets_list[tar](i)[j] << " ";
                    // }
                    //else{
                    //    myfile << targets_list[tar](i)[j] << " ";
                    // }
                       
           
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
