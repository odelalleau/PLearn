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

// Authors: Pascal Lamblin

/*! \file DynamicallyLinkedRBMsModel.cc */


#define PL_LOG_MODULE_NAME "DynamicallyLinkedRBMsModel"
#include <plearn/io/pl_log.h>

#include "DynamicallyLinkedRBMsModel.h"
#include "plearn/math/plapack.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DynamicallyLinkedRBMsModel,
    "Model made of RBMs linked through time",
    ""
    );

DynamicallyLinkedRBMsModel::DynamicallyLinkedRBMsModel() :
    rbm_learning_rate( 0.01 ),
    dynamic_learning_rate( 0.01 ),
    visible_dynamic_learning_rate( 0.01 ),
    fine_tuning_learning_rate( 0.01 ),
    recurrent_net_learning_rate( 0.01),
    untie_weights( false ),
    rbm_nstages( 0 ),
    dynamic_nstages( 0 ),
    fine_tuning_nstages( 0 ),
    recurrent_nstages(0),
    visible_connections_option(0),
    visible_size( -1 )
{
    random_gen = new PRandom();
}

void DynamicallyLinkedRBMsModel::declareOptions(OptionList& ol)
{
    declareOption(ol, "rbm_learning_rate", &DynamicallyLinkedRBMsModel::rbm_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during RBM contrastive "
                  "divergence learning phase");

    declareOption(ol, "dynamic_learning_rate", &DynamicallyLinkedRBMsModel::dynamic_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the dynamic links "
                  "learning phase");

    declareOption(ol, "visible_dynamic_learning_rate", &DynamicallyLinkedRBMsModel::visible_dynamic_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the visible dynamic links "
                  "learning phase");

    declareOption(ol, "fine_tuning_learning_rate", &DynamicallyLinkedRBMsModel::fine_tuning_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the fine tuning "
                  "phase");

    declareOption(ol, "recurrent_net_learning_rate", &DynamicallyLinkedRBMsModel::recurrent_net_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the recurrent phase");

    declareOption(ol, "untie_weights", &DynamicallyLinkedRBMsModel::untie_weights,
                  OptionBase::buildoption,
                  "Indication to untie weights in recurrent net");

    declareOption(ol, "rbm_nstages", &DynamicallyLinkedRBMsModel::rbm_nstages,
                  OptionBase::buildoption,
                  "Number of epochs for rbm phase");

    declareOption(ol, "dynamic_nstages", &DynamicallyLinkedRBMsModel::dynamic_nstages,
                  OptionBase::buildoption,
                  "Number of epochs for dynamic phase");

    declareOption(ol, "fine_tuning_nstages", &DynamicallyLinkedRBMsModel::fine_tuning_nstages,
                  OptionBase::buildoption,
                  "Number of epochs for fine tuning phase");

    declareOption(ol, "recurrent_nstages", &DynamicallyLinkedRBMsModel::recurrent_nstages,
                  OptionBase::buildoption,
                  "Number of epochs for the recurrent phase");

    declareOption(ol, "visible_layer", &DynamicallyLinkedRBMsModel::visible_layer,
                  OptionBase::buildoption,
                  "The visible layer of the RBMs");

    declareOption(ol, "hidden_layer", &DynamicallyLinkedRBMsModel::hidden_layer,
                  OptionBase::buildoption,
                  "The hidden layer of the RBMs. Its size must be set, and will\n"
                  "correspond to the RBMs hidden layer size.");

    declareOption(ol, "connections", &DynamicallyLinkedRBMsModel::connections,
                  OptionBase::buildoption,
                  "The weights of the connections between the RBM "
                  "visible and hidden layers");

    declareOption(ol, "dynamic_connections", &DynamicallyLinkedRBMsModel::dynamic_connections,
                  OptionBase::buildoption,
                  "OnlineLearningModule corresponding to dynamic links "
                  "between RBMs' hidden layers");

    declareOption(ol, "visible_connections", &DynamicallyLinkedRBMsModel::visible_connections,
                  OptionBase::buildoption,
                  "OnlineLearningModule corresponding to dynamic links "
                  "between RBMs' visible layers");

    declareOption(ol, "visible_connections_option", &DynamicallyLinkedRBMsModel::visible_connections_option,
                  OptionBase::buildoption,
                  "Option for the onlineLearningModule corresponding to dynamic links "
                  "between RBMs' visible layers");

    declareOption(ol, "dynamic_connections_copy", &DynamicallyLinkedRBMsModel::dynamic_connections_copy,
                  OptionBase::learntoption,
                  "Independent copy of dynamic connections");

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
        visible_size = 0;
        symbol_sizes.resize(0);
        PP<Dictionary> dict;        
        for(int i=0; i<train_set->inputsize(); i++)
        {
            dict = train_set->getDictionary(i);
            if(dict)
            {
                if( dict->size() == 0 )
                    PLERROR("DynamicallyLinkedRBMsModel::build_(): dictionary "
                        "of field %d is empty", i);
                symbol_sizes.push_back(dict->size());
                // Adjust size to include one-hot vector
                visible_size += dict->size();
            }
            else
            {
                symbol_sizes.push_back(-1);
                visible_size++;
            }
        }

        // Set and verify sizes
        if(hidden_layer->size <= 0)
            PLERROR("DynamicallyLinkedRBMsModel::build_(): hidden_layer->size "
                "must be > 0");

        //cond_bias.resize(hidden_layer->size);

        pos_down_values.resize(visible_size);
        pos_up_values.resize(hidden_layer->size);
        hidden_layer_target.resize(hidden_layer->size);
        hidden_layer_sample.resize(hidden_layer->size);
        visible_layer_sample.resize(visible_size);
        previous_hidden_layer.resize(hidden_layer->size);
        previous_hidden_layer_activation.resize(hidden_layer->size);
        previous_visible_layer.resize(visible_size);

        visi_bias_gradient.resize(visible_size);
        hidden_temporal_gradient.resize(hidden_layer->size);

        visible_layer->size = visible_size;

        connections->down_size = visible_size;
        connections->up_size = hidden_layer->size;
        connections_idem = connections;

        dynamic_connections->input_size = hidden_layer->size;
        dynamic_connections->output_size = hidden_layer->size;

        visible_connections->input_size = visible_size;
        visible_connections->output_size = visible_size;

        // Set random generators
        visible_layer->random_gen = random_gen;
        hidden_layer->random_gen = random_gen;
        connections->random_gen = random_gen;
        dynamic_connections->random_gen = random_gen;
        visible_connections->random_gen = random_gen;

        // Build components
        visible_layer->build();
        hidden_layer->build();
        connections->build();
        dynamic_connections->build();
        visible_connections->build();
        connections_transpose = new RBMMatrixTransposeConnection(connections);
        connections_idem_t = connections_transpose;
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

    deepCopyField(visible_layer, copies);
    deepCopyField( hidden_layer , copies);
    deepCopyField( dynamic_connections , copies);
    deepCopyField( dynamic_connections_copy , copies);
    deepCopyField( visible_connections , copies);
    deepCopyField( connections , copies);
    deepCopyField( connections_idem , copies);
    deepCopyField( connections_idem_t , copies);
    deepCopyField( connections_transpose, copies);
    deepCopyField( connections_transpose_copy, copies);
    deepCopyField( symbol_sizes , copies);
    deepCopyField( cond_bias , copies);
    deepCopyField( visi_cond_bias , copies);
    deepCopyField( bias_gradient , copies);
    deepCopyField( visi_bias_gradient , copies);
    deepCopyField( hidden_layer_target , copies);
    deepCopyField( input_gradient , copies);
    deepCopyField( hidden_gradient , copies);
    deepCopyField( hidden_gradient2 , copies);
    deepCopyField( hidden_temporal_gradient , copies);
    deepCopyField( previous_input , copies);
    deepCopyField( previous_hidden_layer , copies);
    deepCopyField( previous_hidden_layer_activation , copies);
    deepCopyField( previous_visible_layer , copies);
    deepCopyField( hidden_layer_sample , copies);
    deepCopyField( hidden_layer_expectation , copies);
    deepCopyField( visible_layer_sample , copies);
    deepCopyField( visible_layer_input , copies);
    deepCopyField( pos_down_values , copies);
    deepCopyField( pos_up_values , copies);
    deepCopyField( alpha , copies);
    deepCopyField( hidden_list , copies);
    deepCopyField( hidden_activations_list , copies);
    deepCopyField( hidden2_list , copies);
    deepCopyField( hidden2_activations_list , copies);
    deepCopyField( input_prediction_list , copies);
    deepCopyField( input_prediction_activations_list , copies);
    deepCopyField( input_list , copies);
    deepCopyField( nll_list , copies);
    deepCopyField( input_expectation , copies);

    // deepCopyField(, copies);

    //PLERROR("DynamicallyLinkedRBMsModel::makeDeepCopyFromShallowCopy(): "
    //"not implemented yet");
}


int DynamicallyLinkedRBMsModel::outputsize() const
{
    int out_size = 1; // Not really...
    return out_size;
}

void DynamicallyLinkedRBMsModel::forget()
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
    
    visible_layer->forget();
    hidden_layer->forget();
    connections->forget();
    dynamic_connections->forget();
    visible_connections->forget();

    stage = 0;
}

void DynamicallyLinkedRBMsModel::train()
{
    MODULE_LOG << "train() called " << endl;

    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    /* TYPICAL CODE:

    static Vec input;  // static so we don't reallocate memory each time...
    static Vec target; // (but be careful that static means shared!)
    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    real weight;

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    while(stage<nstages)
    {
        // clear statistics of previous epoch
        train_stats->forget();

        //... train for 1 stage, and update train_stats,
        // using train_set->getExample(input, target, weight)
        // and train_stats->update(train_costs)

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
    */

    if(fine_tuning_nstages >= 0){   
        nstages = rbm_nstages + dynamic_nstages + fine_tuning_nstages;
    }
    if(recurrent_nstages >= 0){   
        nstages = rbm_nstages + dynamic_nstages + fine_tuning_nstages + recurrent_nstages;
    }

    if(visible_size < 0)
        PLERROR("DynamicallyLinkedRBMsModel::train(): \n"
                "build() must be called before train()");                

    Vec input( inputsize() );
    Vec target( targetsize() );// Unused
    real weight = 0; // Unused
    Vec train_costs( getTrainCostNames().length() );

    if( !initTrain() )
    {
        MODULE_LOG << "train() aborted" << endl;
        return;
    }

    ProgressBar* pb = 0;

    // clear stats of previous epoch
    train_stats->forget();


    /***** RBM training phase *****/
    if(stage < rbm_nstages)
    {
        MODULE_LOG << "Training connection weights between RBMs" << endl;

        int init_stage = stage;
        int end_stage = min( rbm_nstages, nstages );

        MODULE_LOG << "  stage = " << stage << endl;
        MODULE_LOG << "  end_stage = " << end_stage << endl;
        MODULE_LOG << "  rbm_learning_rate = " << rbm_learning_rate << endl;

        if( report_progress && stage < end_stage )
            pb = new ProgressBar( "RBM training phase of "+classname(),
                                  end_stage - init_stage );

        visible_layer->setLearningRate( rbm_learning_rate );
        connections->setLearningRate( rbm_learning_rate );
        hidden_layer->setLearningRate( rbm_learning_rate );
        real mean_cost = 0;
        while(stage < end_stage)
        {
            for(int sample=0 ; sample<train_set->length() ; sample++ )
            {
                train_set->getExample(sample, input, target, weight);

               
                if(train_set->getString(sample,0) == "<oov>")
                    continue;

                clamp_visible_units(input);

                mean_cost += rbm_update();

            }

            if( pb )
                pb->update( stage + 1 - init_stage);
            if(verbosity>0)
                cout << "mean cost at stage " << stage << 
                    " = " << mean_cost/train_set->length() << endl;
            mean_cost = 0;
            stage++;
        }    
        if( pb )
        {
            delete pb;
            pb = 0;
        }

        //cout << "RBM training phase" << endl;
    }




    /***** dynamic phase training  *****/

    if(stage < rbm_nstages +  dynamic_nstages)
    {
        MODULE_LOG << "Training dynamic connections between RBMs' hidden layers" << endl;

        int init_stage = stage;
        int end_stage = min( rbm_nstages + dynamic_nstages, nstages );

        MODULE_LOG << "  stage = " << stage << endl;
        MODULE_LOG << "  end_stage = " << end_stage << endl;
        MODULE_LOG << "  dynamic_learning_rate = " << dynamic_learning_rate << endl;
        MODULE_LOG << "  visible_dynamic_learning_rate = " << visible_dynamic_learning_rate << endl;


        if( report_progress && stage < end_stage )
            pb = new ProgressBar( "Dynamic training phase of "+classname(),
                                  end_stage - init_stage );

        previous_hidden_layer.resize(hidden_layer->size);
        previous_visible_layer.resize(visible_layer->size);
        dynamic_connections->setLearningRate( dynamic_learning_rate );
        visible_connections->setLearningRate( visible_dynamic_learning_rate );
        real mean_cost = 0;
        while(stage < end_stage)
        {
            for(int sample=0 ; sample<train_set->length() ; sample++ )
            {
                if(sample > 0)
                {
                    previous_hidden_layer << hidden_layer_sample;
                    previous_visible_layer << visible_layer_sample;
                    // ** or **
                    // hidden_layer->generateSample();
                    // previous_hidden_layer << hidden_layer->sample;
                }
                else
                {
                    previous_hidden_layer.clear();
                    previous_visible_layer.clear();
                }

                train_set->getExample(sample, input, target, weight);

                if(train_set->getString(sample,0) == "<oov>")
                {
                    hidden_layer_sample.clear();
                    visible_layer_sample.clear();
                    continue;
                }
            
                clamp_visible_units(input);
                
                mean_cost += dynamic_connections_update();
                                
            }
            if( pb )
                pb->update( stage + 1 - init_stage);

            if(verbosity>0)
                cout << "mean cost at stage " << stage << 
                    " = " << mean_cost/train_set->length() << endl;
            mean_cost = 0;
            stage++;
        }    
        if( pb )
        {
            delete pb;
            pb = 0;
        }

        //Make a copy of the dynamique phase
        //CopiesMap map;
        //dynamic_connections_copy = dynamic_connections->deepCopy(map);

        //cout << "dynamic phase training" << endl;
    }


    //cout << dynamic_connections_copy->weights << endl;


    //alpha = Vec(hidden_layer->size,1);



    /***** fine-tuning *****/
    if( stage >= nstages )
        return;

    if(stage < rbm_nstages +  dynamic_nstages + fine_tuning_nstages )
    {
        MODULE_LOG << "Training the whole model" << endl;

        int init_stage = stage;
        //int end_stage = max(0,nstages-(rbm_nstages + dynamic_nstages));
        //int end_stage = nstages;
        int end_stage = min( rbm_nstages + dynamic_nstages + fine_tuning_nstages, nstages );

        MODULE_LOG << "  stage = " << stage << endl;
        MODULE_LOG << "  end_stage = " << end_stage << endl;
        MODULE_LOG << "  fine_tuning_learning_rate = " << fine_tuning_learning_rate << endl;

        if( report_progress && stage < end_stage )
            pb = new ProgressBar( "Fine-tuning training phase of "+classname(),
                                  end_stage - init_stage );

        previous_hidden_layer.resize(hidden_layer->size);
        dynamic_connections->setLearningRate( fine_tuning_learning_rate );
        visible_layer->setLearningRate( fine_tuning_learning_rate );
        connections->setLearningRate( fine_tuning_learning_rate );

        real mean_cost = 0;


        while(stage < end_stage)
        {
            for(int sample=0 ; sample<train_set->length() ; sample++ )
            {
                if(sample > 0)
                {
                    previous_hidden_layer << hidden_layer_sample;

                    // ** or **
                    // hidden_layer->generateSample();
                    // previous_hidden_layer << hidden_layer->sample;
                }
                else
                    previous_hidden_layer.clear();


                train_set->getExample(sample, input, target, weight);

                

                if(train_set->getString(sample,0) == "<oov>")
                {
                    hidden_layer_sample.clear();
                    continue;
                }

                

                clamp_visible_units(input);
                
                mean_cost += fine_tuning_update();     
                //cout << "lalalalalalallalalalalalalala4" << endl;
            }

            if( pb )
                pb->update( stage + 1 - init_stage);

            if(verbosity>0)
                cout << "mean cost at stage " << stage << 
                    " = " << mean_cost/train_set->length() << endl;
            mean_cost = 0;
            stage++;
        }    
        if( pb )
        {
            delete pb;
            pb = 0;
        }

    }

    //cout << dynamic_connections->weights - dynamic_connections_copy->weights<< endl;    
    


 /***** Recurrent phase *****/
    if( stage >= nstages )
        return;

    if(stage >= rbm_nstages +  dynamic_nstages + fine_tuning_nstages)
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

        previous_hidden_layer.resize(hidden_layer->size);
        dynamic_connections->setLearningRate( recurrent_net_learning_rate );
        visible_layer->setLearningRate( recurrent_net_learning_rate );
        connections->setLearningRate( recurrent_net_learning_rate );
        connections_transpose->setLearningRate( recurrent_net_learning_rate );
        if(dynamic_connections_copy)
            dynamic_connections_copy->setLearningRate( recurrent_net_learning_rate );
        if(connections_transpose_copy)
            connections_transpose_copy->setLearningRate( recurrent_net_learning_rate );

        real mean_cost = 0;
        int ith_sample_in_sequence = 0;
        int nb_oov = 0;

        while(stage < end_stage)
        {
            if(untie_weights && 
               stage == rbm_nstages + dynamic_nstages + fine_tuning_nstages)
            {
                
                //CopiesMap map;
                //dynamic_connections_copy = dynamic_connections->deepCopy(map);

                //CopiesMap map2;
                //connections_transpose_copy = connections_transpose->deepCopy(map2);
                //connections_transpose = connections_transpose_copy;  
/*
                TMat<real> U,V;//////////*********************crap James
                TVec<real> S;
                U.resize(hidden_layer->size,hidden_layer->size);
                V.resize(hidden_layer->size,hidden_layer->size);
                S.resize(hidden_layer->size);
                U << dynamic_connections->weights;
                
                SVD(U,dynamic_connections->weights,S,V);
                S.fill(-0.5);
                productScaleAcc(dynamic_connections->bias,dynamic_connections->weights,S,1,0);
*/
            }
            
            for(int sample=0 ; sample<train_set->length() ; sample++ )
            {

                train_set->getExample(sample, input, target, weight);
                

                hidden_list.resize(ith_sample_in_sequence+1,hidden_layer->size);
                hidden_activations_list.resize(ith_sample_in_sequence+1,hidden_layer->size);
                hidden2_list.resize(ith_sample_in_sequence+1,hidden_layer->size);
                hidden2_activations_list.resize(ith_sample_in_sequence+1,hidden_layer->size);
                input_prediction_list.resize(
                    ith_sample_in_sequence+1,visible_layer->size);
                input_prediction_activations_list.resize(
                    ith_sample_in_sequence+1,visible_layer->size);
                input_list.resize(ith_sample_in_sequence+1,visible_layer->size);
                nll_list.resize(ith_sample_in_sequence+1);


                if(train_set->getString(sample,0) == "<oov>")
                {/*
                    nb_oov++;
                    hidden_list.resize(ith_sample_in_sequence,hidden_layer->size);
                    hidden_activations_list.resize(ith_sample_in_sequence,hidden_layer->size);
                    hidden2_list.resize(ith_sample_in_sequence,hidden_layer->size);
                    hidden2_activations_list.resize(ith_sample_in_sequence,hidden_layer->size);
                    input_prediction_list.resize(
                        ith_sample_in_sequence,visible_layer->size);
                    input_prediction_activations_list.resize(
                        ith_sample_in_sequence,visible_layer->size);
                    input_list.resize(ith_sample_in_sequence,visible_layer->size);
                    nll_list.resize(ith_sample_in_sequence);
                    recurrent_update();
                    ith_sample_in_sequence = 0;
                    hidden_list.clear();
                    hidden2_list.clear();
                    input_prediction_list.clear();
                    input_list.clear();
                    nll_list.clear();
                    continue;
                 */
                    input_list(ith_sample_in_sequence) << previous_input;
                    connections->setAsDownInput( previous_input );
                    hidden_layer->getAllActivations( connections_idem );
                    hidden_layer->computeExpectation();
                    previous_hidden_layer << hidden_layer->expectation;
                    previous_hidden_layer_activation << hidden_layer->activation;
                    hidden_list(ith_sample_in_sequence) << previous_hidden_layer;
                    hidden_activations_list(ith_sample_in_sequence) 
                        << previous_hidden_layer_activation;
                    hidden2_list(ith_sample_in_sequence) << hidden_layer->expectation;
                    hidden2_activations_list(ith_sample_in_sequence) << 
                        hidden_layer->activation;
                    input_prediction_list(ith_sample_in_sequence) << 
                        visible_layer->expectation;
                    input_prediction_activations_list(ith_sample_in_sequence) << 
                        visible_layer->activation;
                    //cout << "hidden_expectation crap james :" <<  hidden_list << endl;
                    //update
                    nb_oov++;
                    recurrent_update();
                    
                    ith_sample_in_sequence = 0;
                    hidden_list.clear();
                    hidden2_list.clear();
                    input_prediction_list.clear();
                    input_list.clear();
                    nll_list.clear();
                    continue;
                }
/*
                hidden_list.resize(ith_sample_in_sequence+1,hidden_layer->size);
                hidden_activations_list.resize(ith_sample_in_sequence+1,hidden_layer->size);
                hidden2_list.resize(ith_sample_in_sequence+1,hidden_layer->size);
                hidden2_activations_list.resize(ith_sample_in_sequence+1,hidden_layer->size);
                input_prediction_list.resize(
                    ith_sample_in_sequence+1,visible_layer->size);
                input_prediction_activations_list.resize(
                    ith_sample_in_sequence+1,visible_layer->size);
                input_list.resize(ith_sample_in_sequence+1,visible_layer->size);
                nll_list.resize(ith_sample_in_sequence+1);
*/
         
                clamp_visible_units(input);

                if(ith_sample_in_sequence > 0)
                {
                   
                    input_list(ith_sample_in_sequence) << previous_input;
                    //h*_{t-1}
                    //////////////////////////////////
                    dynamic_connections->fprop(previous_hidden_layer, cond_bias);
                    hidden_layer->setAllBias(cond_bias); //**************************

//                    if (visible_connections_option){
//                        //v*_{t-1} VISIBLE DYNAMIC CONNECTION
//                        //////////////////////////////////
//                        visible_connections->fprop(previous_input, visi_cond_bias);
//                        visible_layer->getAllBias(visi_cond_bias); //**************************
//                    }

                    //up phase
                    connections->setAsDownInput( previous_input );
                    hidden_layer->getAllActivations( connections_idem );
                    hidden_layer->computeExpectation();
                    //////////////////////////////////

                    previous_hidden_layer << hidden_layer->expectation;//h_{t-2} au prochain tour//******************************
                    previous_hidden_layer_activation << hidden_layer->activation;
            
                    
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

                    previous_input << visible_layer->expectation;//v_{t-1}
             
                }
                else
                {
                    
                    input_list(ith_sample_in_sequence).fill(-1);
 
                    previous_hidden_layer.clear();//h_{t-1}
                    //previous_hidden_layer.fill(0.5);//**************************crap James
                    previous_hidden_layer_activation.clear();//h_{t-1}

                    if(dynamic_connections_copy)
                        dynamic_connections_copy->fprop( previous_hidden_layer ,
                                                         hidden_layer->activation);//conection entre h_{t-1} et h_{t}
                    else
                        dynamic_connections->fprop(
                            previous_hidden_layer,hidden_layer->activation);//conection entre h_{t-1} et h_{t}

                    //dynamic_connections_copy->fprop(previous_hidden_layer,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
                    hidden_layer->expectation_is_not_up_to_date();
                    hidden_layer->computeExpectation();//h_{t}

                    previous_input.resize(visible_layer->size);
                    previous_input << visible_layer->expectation;
 

//                    if (visible_connections_option){
//
//                        /////////VISIBLE DYNAMIC CONNECTION
//                        previous_visible_layer.clear();//v_{t-1}
//                        visible_connections->fprop(previous_visible_layer,visible_layer->activation);//conection entre v_{t-1} et v_{t}
//                
//                        visible_layer->expectation_is_not_up_to_date();
//                        visible_layer->computeExpectation();//v_{t}
//                
//                        visi_bias_tempo.resize(visible_layer->bias.length());
//                        visi_bias_tempo << visible_layer->bias;
//                    }
            
                }

                // cout << "trililililililili" << endl;

                connections_transpose->setAsDownInput( hidden_layer->expectation );
                visible_layer->getAllActivations( connections_idem_t );

                //connections->setAsUpInput( hidden_layer->expectation );
                //visible_layer->getAllActivations( connections_idem );

                visible_layer->computeExpectation();
 
                // Copies for backprop
                hidden_list(ith_sample_in_sequence) << previous_hidden_layer;
                hidden_activations_list(ith_sample_in_sequence) 
                    << previous_hidden_layer_activation;
                hidden2_list(ith_sample_in_sequence) << hidden_layer->expectation;
                hidden2_activations_list(ith_sample_in_sequence) << 
                    hidden_layer->activation;
                input_prediction_list(ith_sample_in_sequence) << 
                    visible_layer->expectation;
                input_prediction_activations_list(ith_sample_in_sequence) << 
                    visible_layer->activation;

 
                nll_list[ith_sample_in_sequence] = 
                    visible_layer->fpropNLL(previous_input) / inputsize() ;
                mean_cost += nll_list[ith_sample_in_sequence];
                ith_sample_in_sequence++;
                /////////VISIBLE DYNAMIC CONNECTION
//                if (visible_connections_option){
//                    visible_layer->getAllBias(visi_bias_tempo); 
//                }

               
            }
            if( pb )
                pb->update( stage + 1 - init_stage);
            
            if(verbosity>0)
                cout << "mean cost at stage " << stage << 
                    " = " << mean_cost/train_set->length() << endl;
            mean_cost = 0;
            stage++;
        }    
        if( pb )
        {
            delete pb;
            pb = 0;
        }

    }


    train_stats->finalize();
}


void DynamicallyLinkedRBMsModel::clamp_visible_units(const Vec& input) const
{
    int it = 0;
    int ss = -1;
    input_expectation.resize(visible_layer->size);
    for(int i=0; i<inputsize_; i++)
    {
        ss = symbol_sizes[i];
        // If input is a real ...
        if(ss < 0) 
        {
            //cout << "yoyoyoyoyo" << endl;
            input_expectation[it++] = input[i];
        }
        else // ... or a symbol
        {
            // Convert to one-hot vector
            input_expectation.subVec(it,ss).clear();
            input_expectation[it+(int)input[i]] = 1;
            it += ss;
        }
    }
    visible_layer->setExpectation(input_expectation);
}

real DynamicallyLinkedRBMsModel::rbm_update()
{

    //###### Positive phase #####################

    //up phase
    connections->setAsDownInput( visible_layer->expectation );
    
    hidden_layer->getAllActivations( connections_idem );

    hidden_layer->computeExpectation();


    //save the stats for the positive phase
    pos_down_values << visible_layer->expectation;
    pos_up_values << hidden_layer->expectation;



    //down phase
    hidden_layer->generateSample();
    
    connections->setAsUpInput( hidden_layer->sample );

    visible_layer->getAllActivations( connections_idem );

    visible_layer->computeExpectation();
    
    visible_layer->generateSample();




    //############ Negative phase  ##################

    connections->setAsDownInput( visible_layer->sample );
    
    hidden_layer->getAllActivations( connections_idem );

    hidden_layer->computeExpectation();

    hidden_layer->generateSample();

    //############ CD update #########################

    visible_layer->update( 
        pos_down_values, visible_layer->sample ); // ... of visible_layer bias ...

    hidden_layer->update( 
        pos_up_values, hidden_layer->expectation );// ... of hidden_layer bias ...
    
    connections->update( 
        pos_down_values, pos_up_values, visible_layer->sample
        , hidden_layer->expectation  ); // ... of connections between layers.
    
    // Compute reconstruction error
    
    connections->setAsUpInput( pos_up_values );

    visible_layer->getAllActivations( connections_idem );

    
    visible_layer->computeExpectation();
    
    
    return visible_layer->fpropNLL(pos_down_values);
   
}

real DynamicallyLinkedRBMsModel::dynamic_connections_update()
{
    

    // Obtain target hidden_layer h_t
    connections->setAsDownInput(visible_layer->expectation);
    hidden_layer->getAllActivations(connections_idem);
    hidden_layer->computeExpectation();
    hidden_layer_target << hidden_layer->expectation;
    hidden_layer->generateSample();
    hidden_layer_sample << hidden_layer->sample;
    
    // Use "previous_hidden_layer" field and "dynamic_connections" module 
    // to set bias of "hidden_layer"
    dynamic_connections->fprop(previous_hidden_layer,hidden_layer->activation);
    hidden_layer->expectation_is_not_up_to_date();
    hidden_layer->computeExpectation();
    
    
    
    // Ask "hidden_layer" for maximum likelyhood gradient on bias
    real nll = hidden_layer->fpropNLL(hidden_layer_target);
    hidden_layer->bpropNLL(hidden_layer_target, nll, bias_gradient);
    
    
    dynamic_connections->bpropUpdate(previous_hidden_layer,
                                     hidden_layer->activation,
                                     input_gradient, bias_gradient);
    
    
    
    
    
    
    
    
    //////////////////// VISIBLE DYNAMIC CONNECTION
    /*  if (visible_connections_option){
        visible_layer_sample << visible_layer->expectation;
        
        // Use "previous_visible_layer" field and "visible_connections" module 
        // to set bias of "visible_layer"
        
        visible_connections->fprop(previous_visible_layer,visible_layer->activation);
        visible_layer->expectation_is_not_up_to_date();
        visible_layer->computeExpectation();
        
        
        
        // Ask "visible_layer" for maximum likelyhood gradient on bias
        real nll_visi = visible_layer->fpropNLL(visible_layer_sample);
        visible_layer->bpropNLL(visible_layer_sample, nll_visi, visi_bias_gradient);
        
        
        visible_connections->bpropUpdate(previous_visible_layer,
                                         visible_layer->activation,
                                         input_gradient, visi_bias_gradient);
                                         }*/

    return nll;
}

real DynamicallyLinkedRBMsModel::fine_tuning_update()
{

 

     



    cond_bias.resize(hidden_layer->size);
   
    dynamic_connections->fprop(previous_hidden_layer, cond_bias);


  

    hidden_layer->setAllBias(cond_bias);
    



 //###### Positive phase #####################

    //up phase
    connections->setAsDownInput( visible_layer->expectation );
    hidden_layer->getAllActivations( connections_idem );
    hidden_layer->computeExpectation();


    //save the stats for the positive phase
    pos_down_values << visible_layer->expectation;
    pos_up_values << hidden_layer->expectation;



    //down phase
    hidden_layer->generateSample();
    hidden_layer_sample << hidden_layer->sample;
    connections->setAsUpInput( hidden_layer->sample );

    visible_layer->getAllActivations( connections_idem );

    visible_layer->computeExpectation();
    
    visible_layer->generateSample();




    //############ Negative phase  ##################

    connections->setAsDownInput( visible_layer->sample );
    
    hidden_layer->getAllActivations( connections_idem );

    hidden_layer->computeExpectation();

    hidden_layer->generateSample();

    


    //cout <<  hidden_layer->bias << endl;
     //############ CD update #########################

    visible_layer->update( 
        pos_down_values, visible_layer->sample ); // ... of visible_layer bias ...

    


// cout << pos_up_values << endl;
// cout << "blabla" << endl;
// cout << hidden_layer->sample << endl;
    bias_gradient.resize( hidden_layer-> size );






    hidden_layer->bpropCD(pos_up_values, hidden_layer->expectation, bias_gradient);



    dynamic_connections->bpropUpdate(previous_hidden_layer,
                                     cond_bias,
                                     input_gradient, bias_gradient);


    // hidden_layer->update( 
    //    pos_up_values, hidden_layer->sample );// ... of hidden_layer bias ...
    
    connections->update( 
        pos_down_values, pos_up_values, visible_layer->sample
        , hidden_layer->expectation ); // ... of connections between layers.

    //cout << ((PLearn::PP<PLearn::GradNNetLayerModule>)dynamic_connections)->weights << endl;








    // Use "previous_hidden_layer" field and "dynamic_connections" module 
    // to set bias of "hidden_layer"

    // Positive phase

    // Negative phase

    // CD update ...

    // .. ask "hidden_layer" for CD gradient on bias and ...

    // ... bpropUpdate through dynamic_connections 
    // to update conditional bias (*)
    
    // ... of visible_layer bias ...

    // ... of connections between layers.

    // NB.: it is important not to update the hidden_layer's bias
    //      using usual CD update. Conditional bias was updated
    //      before at (*)





    // Compute reconstruction error

    //cond_bias.clear();
    //dynamic_connections->fprop(previous_hidden_layer, cond_bias);
    //hidden_layer->getAllBias(cond_bias);

    



    connections->setAsUpInput( pos_up_values );

    visible_layer->getAllActivations( connections_idem );

    visible_layer->computeExpectation();
    
    return visible_layer->fpropNLL(pos_down_values);

    


}


void DynamicallyLinkedRBMsModel::recurrent_update()
{
    // Notes: 
    //    - not all lists are useful (some *_activations_* are not)
    int segment = hidden_list.length()/2;
    int seg =0;
    for(int k=hidden_list.length()-3; k>=-segment; k-=segment){ 
        seg = k;
        if(seg < 0)
            seg = 0;
        //cout << "segment: " << seg << endl;
        hidden_temporal_gradient.clear();
        //for(int i=hidden_list.length()-2; i>=0; i--){  
        for(int i=hidden_list.length()-2; i>=seg; i--){     
            
            //visible_layer->expectation << input_prediction_list(i);
            //visible_layer->activation << ?????;
            visible_layer->setExpectation(input_prediction_list(i));
            
            visible_layer->bpropNLL(input_list(i+1),nll_list[i],visi_bias_gradient);
            
            visible_layer->update(visi_bias_gradient);
            
            
            connections_transpose->bpropUpdate(
                hidden2_list(i),input_prediction_activations_list(i),
                hidden_gradient, visi_bias_gradient);
            
            
            //hidden_layer->setExpectation(hidden_list(i+1));//////////////////////////////
            //hidden_layer->bpropNLL(hidden2_list(i),nll_list[i], hidden_gradient2);////////////////////////////////
            
            
            
            hidden_layer->bpropUpdate(
                hidden2_activations_list(i), hidden2_list(i),
                bias_gradient, hidden_gradient);
            
            //hidden_layer->update(hidden_gradient2);/////////////////////////////////////
            
            
            //bias_gradient += hidden_gradient2;///////////////////////////////////
            
            
            
            if(dynamic_connections_copy)
                dynamic_connections_copy->bpropUpdate(
                    hidden_list(i),
                    hidden2_activations_list(i), 
                    hidden_gradient, bias_gradient);        
            else
                dynamic_connections->bpropUpdate(
                    hidden_list(i),
                    hidden2_activations_list(i), 
                    hidden_gradient, bias_gradient);
            
            if(i!=0)
            {
                
                hidden_gradient += hidden_temporal_gradient;
                
                hidden_layer->bpropUpdate(
                    hidden_activations_list(i), hidden_list(i),
                    hidden_temporal_gradient, hidden_gradient);
                
                dynamic_connections->bpropUpdate(
                    hidden_list(i-1),
                    hidden_activations_list(i), // Here, it should be cond_bias, but doesn't matter
                    hidden_gradient, hidden_temporal_gradient);
                
                hidden_temporal_gradient << hidden_gradient;
                
                connections->bpropUpdate(
                    input_list(i),
                    hidden_activations_list(i), 
                    visi_bias_gradient, hidden_temporal_gradient);// Here, it should be activations - cond_bias, but doesn't matter
                
            }
            else
            {
                // Could learn initial value for h_{-1}
            }
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
    Vec bias_tempo;
    Vec visi_bias_tempo;
    real weight;

    Vec output(outputsize());
    Vec costs(nTestCosts());

    PP<ProgressBar> pb;
    if (report_progress) 
        pb = new ProgressBar("Testing learner", len);

    if (len == 0) {
        // Empty test set: we give -1 cost arbitrarily.
        costs.fill(-1);
        test_stats->update(costs);
    }


    int begin = 0;
    int nb_oov = 0;
    for (int i = 0; i < len; i++)
    {
        testset.getExample(i, input, target, weight);
      
       


        if(testset->getString(i,0) == "<oov>")
        {
            begin = 0;
            nb_oov++;
            continue;
        }
        




        clamp_visible_units(input);




        if(begin > 0)
        {

            //h*_{t-1}
            //////////////////////////////////
            dynamic_connections->fprop(previous_hidden_layer, cond_bias);
            hidden_layer->setAllBias(cond_bias); //**************************

            /* if (visible_connections_option){
                //v*_{t-1} VISIBLE DYNAMIC CONNECTION
                //////////////////////////////////
                visible_connections->fprop(previous_input, visi_cond_bias);
                visible_layer->setAllBias(visi_cond_bias); //**************************
                }*/

            //up phase
            connections->setAsDownInput( previous_input );
            hidden_layer->getAllActivations( connections_idem );
            hidden_layer->computeExpectation();
            //////////////////////////////////

            previous_hidden_layer << hidden_layer->expectation;//h_{t-2} au prochain tour//******************************
            

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

            previous_input << visible_layer->expectation;//v_{t-1}
            
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

            //dynamic_connections_copy->fprop(previous_hidden_layer,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
            hidden_layer->expectation_is_not_up_to_date();
            hidden_layer->computeExpectation();//h_{t}

            previous_input.resize(visible_layer->size);
            previous_input << visible_layer->expectation;

            bias_tempo.resize(hidden_layer->bias.length());
            bias_tempo << hidden_layer->bias;


            /*  if (visible_connections_option){

                /////////VISIBLE DYNAMIC CONNECTION
                previous_visible_layer.clear();//v_{t-1}
                visible_connections->fprop(previous_visible_layer,visible_layer->activation);//conection entre v_{t-1} et v_{t}
                
                visible_layer->expectation_is_not_up_to_date();
                visible_layer->computeExpectation();//v_{t}
                
                visi_bias_tempo.resize(visible_layer->bias.length());
                visi_bias_tempo << visible_layer->bias;
                }*/
            
            begin++;
        }


       






     
        connections_transpose->setAsDownInput( hidden_layer->expectation );
        visible_layer->getAllActivations( connections_idem_t );

        //connections->setAsUpInput( hidden_layer->expectation );
        //visible_layer->getAllActivations( connections_idem );


       
        visible_layer->computeExpectation();

       

        costs[0] = visible_layer->fpropNLL(previous_input) / inputsize() ;

       
        hidden_layer->setAllBias(bias_tempo); 

        /////////VISIBLE DYNAMIC CONNECTION
        /* if (visible_connections_option){
            visible_layer->setAllBias(visi_bias_tempo); 
            }*/

        // costs[0] = 0; //nll/nb_de_temps_par_mesure

        if (testoutputs)
            testoutputs->putOrAppendRow(i, output);

        if (testcosts)
            testcosts->putOrAppendRow(i, costs);

        if (test_stats)
            test_stats->update(costs, weight);

        if (report_progress)
            pb->update(i);
    }

    //costs[0] = costs[0]/(len - nb_oov) ;

    //cout << "Probabilite moyenne : " << costs[0] << endl;

}


TVec<string> DynamicallyLinkedRBMsModel::getTestCostNames() const
{
    TVec<string> cost_names;
    cost_names.append( "NLL" );
    return cost_names;
}

TVec<string> DynamicallyLinkedRBMsModel::getTrainCostNames() const
{
    return getTestCostNames();
}

void DynamicallyLinkedRBMsModel::generate(int nbNotes)
{
    
    previous_hidden_layer.resize(hidden_layer->size);
    connections_idem = connections;

    for (int ith_sample = 0; ith_sample < nbNotes ; ith_sample++ ){
        
        input_prediction_list.resize(
            ith_sample+1,visible_layer->size);
        if(ith_sample > 0)
        {
            
            //input_list(ith_sample_in_sequence) << previous_input;
            //h*_{t-1}
            //////////////////////////////////
            dynamic_connections->fprop(previous_hidden_layer, cond_bias);
            hidden_layer->setAllBias(cond_bias); //**************************
            
            
            
            //up phase
            connections->setAsDownInput( input_prediction_list(ith_sample-1) );
            hidden_layer->getAllActivations( connections_idem );
            hidden_layer->computeExpectation();
            //////////////////////////////////
            
            //previous_hidden_layer << hidden_layer->expectation;//h_{t-2} au prochain tour//******************************
            //previous_hidden_layer_activation << hidden_layer->activation;
            
            
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
            
            
        }
        
        //connections_transpose->setAsDownInput( hidden_layer->expectation );
        //visible_layer->getAllActivations( connections_idem_t );
        
        connections->setAsUpInput( hidden_layer->expectation );
        visible_layer->getAllActivations( connections_idem );
        
        visible_layer->computeExpectation();
        visible_layer->generateSample();
        
        input_prediction_list(ith_sample) << visible_layer->sample;
        
    }
    
    //Vec tempo;
    TVec<int> tempo;
    tempo.resize(visible_layer->size);
    int theNote;
    int nbNoteVisiLayer = input_prediction_list(1).length()/13;
    ofstream myfile;
    int theLayer;
    myfile.open ("/u/laulysta/recherche_maitrise/projet_GenerationDeMusique/data/generate/test.txt");
    
    for (int i = 0; i < nbNotes ; i++ ){
        tempo << input_prediction_list(i);
        
        //cout << tempo[2] << endl;
       
        for (int j = 0; j < tempo.length() ; j++ ){
            
            if (tempo[j] == 1){
                theLayer = (j/13);
                
                theNote = j - (13*theLayer);
               

                if (theNote<=11){
                    //print theNote
                    //cout << theNote+50 << " ";
                    myfile << theNote << " ";
                }
                else{
                    //print #
                    //cout << "# ";
                    myfile << "# ";
                    
                }
     
            }
           
        }
        myfile << "\n";
    }
     myfile << "<oov> <oov> \n";

     myfile.close();

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
