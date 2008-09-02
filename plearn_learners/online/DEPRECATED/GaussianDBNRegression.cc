// -*- C++ -*-

// GaussianDBNRegression.cc
//
// Copyright (C) 2006 Dan Popovici, Pascal Lamblin
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

// Authors: Dan Popovici

/*! \file PLearn/plearn_learners/online/DEPRECATED/GaussianDBNRegression.cc */

#define PL_LOG_MODULE_NAME "GaussianDBNRegression"
#include <plearn/io/pl_log.h>

#include "GaussianDBNRegression.h"
#include "RBMLayer.h"
#include "RBMMixedLayer.h"
#include "RBMMultinomialLayer.h"
#include "RBMParameters.h"
#include "RBMLLParameters.h"
#include "RBMQLParameters.h"
#include "RBMLQParameters.h"
#include "RBMJointLLParameters.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    GaussianDBNRegression,
    "Does the same thing as Hinton's deep belief nets",
    ""
);

/////////////////////////
// GaussianDBNRegression //
/////////////////////////
GaussianDBNRegression::GaussianDBNRegression() :
    learning_rate(0.),
    weight_decay(0.),
    use_sample_rather_than_expectation_in_positive_phase_statistics(false)
{
    random_gen = new PRandom();
}

////////////////////
// declareOptions //
////////////////////
void GaussianDBNRegression::declareOptions(OptionList& ol)
{
    declareOption(ol, "learning_rate", &GaussianDBNRegression::learning_rate,
                  OptionBase::buildoption,
                  "Learning rate");

    declareOption(ol, "weight_decay", &GaussianDBNRegression::weight_decay,
                  OptionBase::buildoption,
                  "Weight decay");

    declareOption(ol, "initialization_method",
                  &GaussianDBNRegression::initialization_method,
                  OptionBase::buildoption,
                  "The method used to initialize the weights:\n"
                  "  - \"uniform_linear\" = a uniform law in [-1/d, 1/d]\n"
                  "  - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(d),"
                  " 1/sqrt(d)]\n"
                  "  - \"zero\"           = all weights are set to 0,\n"
                  "where d = max( up_layer_size, down_layer_size ).\n");


    declareOption(ol, "training_schedule",
                  &GaussianDBNRegression::training_schedule,
                  OptionBase::buildoption,
                  "Number of examples to use during each of the different"
                  " greedy\n"
                  "steps of the training phase.\n");

    declareOption(ol, "fine_tuning_method",
                  &GaussianDBNRegression::fine_tuning_method,
                  OptionBase::buildoption,
                  "Method for fine-tuning the whole network after greedy"
                  " learning.\n"
                  "One of:\n"
                  "  - \"none\"\n"
                  "  - \"CD\" or \"contrastive_divergence\"\n"
                  "  - \"EGD\" or \"error_gradient_descent\"\n"
                  "  - \"WS\" or \"wake_sleep\".\n");

    declareOption(ol, "layers", &GaussianDBNRegression::layers,
                  OptionBase::buildoption,
                  "Layers that learn representations of the input,"
                  " unsupervisedly.\n"
                  "layers[0] is input layer.\n");

    declareOption(ol, "target_layer", &GaussianDBNRegression::target_layer,
                  OptionBase::buildoption,
                  "Target (or label) layer");

    declareOption(ol, "params", &GaussianDBNRegression::params,
                  OptionBase::buildoption,
                  "RBMParameters linking the unsupervised layers.\n"
                  "params[i] links layers[i] and layers[i+1], except for"
                  "params[n_layers-1],\n"
                  "that links layers[n_layers-1] and last_layer.\n");

    declareOption(ol, "target_params", &GaussianDBNRegression::target_params,
                  OptionBase::buildoption,
                  "Parameters linking target_layer and last_layer");
    
    declareOption(ol, "input_params", &GaussianDBNRegression::input_params,
                  OptionBase::buildoption,
                  "Parameters linking layer[0] and layer[1]");

    declareOption(ol, "use_sample_rather_than_expectation_in_positive_phase_statistics",
                  &GaussianDBNRegression::use_sample_rather_than_expectation_in_positive_phase_statistics,
                  OptionBase::buildoption,
                  "In positive phase statistics use output->sample * input\n"
                  "rather than output->expectation * input.\n");

    declareOption(ol, "n_layers", &GaussianDBNRegression::n_layers,
                  OptionBase::learntoption,
                  "Number of unsupervised layers, including input layer");

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void GaussianDBNRegression::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void GaussianDBNRegression::build_()
{
    MODULE_LOG << "build_() called" << endl;
    n_layers = layers.length();
    if( n_layers <= 1 )
        return;

    // check value of initialization_method
    string im = lowerstring( initialization_method );
    if( im == "" || im == "uniform_sqrt" )
        initialization_method = "uniform_sqrt";
    else if( im == "uniform_linear" )
        initialization_method = im;
    else if( im == "zero" )
        initialization_method = im;
    else
        PLERROR( "RBMParameters::build_ - initialization_method\n"
                 "\"%s\" unknown.\n", initialization_method.c_str() );
    MODULE_LOG << "  initialization_method = \"" << initialization_method
        << "\"" << endl;

    // check value of fine_tuning_method
    string ftm = lowerstring( fine_tuning_method );
    if( ftm == "" | ftm == "none" )
        fine_tuning_method = "";
    else if( ftm == "cd" | ftm == "contrastive_divergence" )
        fine_tuning_method = "CD";
    else if( ftm == "egd" | ftm == "error_gradient_descent" )
        fine_tuning_method = "EGD";
    else if( ftm == "ws" | ftm == "wake_sleep" )
        fine_tuning_method = "WS";
    else
        PLERROR( "GaussianDBNRegression::build_ - fine_tuning_method \"%s\"\n"
                 "is unknown.\n", fine_tuning_method.c_str() );
    MODULE_LOG << "  fine_tuning_method = \"" << fine_tuning_method << "\""
        <<  endl;
    //TODO: build structure to store gradients during gradient descent

    if( training_schedule.length() != n_layers )
        training_schedule = TVec<int>( n_layers, 1000000 );
    MODULE_LOG << "  training_schedule = " << training_schedule << endl;
    MODULE_LOG << endl;

    build_layers();
    build_params();
}

void GaussianDBNRegression::build_layers()
{
    MODULE_LOG << "build_layers() called" << endl;
    if( inputsize_ >= 0 )
    {
        PLASSERT( layers[0]->size + target_layer->size == inputsize() );
        setPredictorPredictedSizes( layers[0]->size,
                                    target_layer->size, false );
        MODULE_LOG << "  n_predictor = " << n_predictor << endl;
        MODULE_LOG << "  n_predicted = " << n_predicted << endl;
    }

    for( int i=0 ; i<n_layers ; i++ )
        layers[i]->random_gen = random_gen;
    target_layer->random_gen = random_gen;
    
    last_layer = layers[n_layers-1];

}

void GaussianDBNRegression::build_params()
{
    MODULE_LOG << "build_params() called" << endl;
    if( params.length() == 0 )
    {
        input_params = new RBMQLParameters() ; 
        params.resize( n_layers-1 );
        for( int i=1 ; i<n_layers-1 ; i++ )
            params[i] = new RBMLLParameters();
        // params[0] is not being using, it is not being created
    }
    else if( params.length() != n_layers-1 )
        PLERROR( "GaussianDBNRegression::build_params - params.length() should\n"
                 "be equal to layers.length()-1 (%d != %d).\n",
                 params.length(), n_layers-1 );

    activation_gradients.resize( n_layers+1 );
    expectation_gradients.resize( n_layers+1 );
    output_gradient.resize( n_predicted );

    input_params->down_units_types = layers[0]->units_types;
    input_params->up_units_types = layers[1]->units_types;
    input_params->learning_rate = learning_rate;
    input_params->initialization_method = initialization_method;
    input_params->random_gen = random_gen;
    input_params->build();

    activation_gradients[0].resize( input_params->down_layer_size );
    expectation_gradients[0].resize( input_params->down_layer_size );


    for( int i=1 ; i<n_layers-1 ; i++ )
    {
        //TODO: call changeOptions instead
        
        params[i]->down_units_types = layers[i]->units_types;
        params[i]->up_units_types = layers[i+1]->units_types;
        params[i]->learning_rate = learning_rate;
        params[i]->initialization_method = initialization_method;
        params[i]->random_gen = random_gen;
        params[i]->build();
        
        activation_gradients[i].resize( params[i]->down_layer_size );
        expectation_gradients[i].resize( params[i]->down_layer_size );
        
    }


    if( target_layer && !target_params )
        target_params = new RBMLQParameters();

    //TODO: call changeOptions instead
    target_params->down_units_types = last_layer->units_types;
    target_params->up_units_types = target_layer->units_types;
    target_params->learning_rate = learning_rate;
    target_params->initialization_method = initialization_method;
    target_params->random_gen = random_gen;
    target_params->build();

}

////////////
// forget //
////////////
void GaussianDBNRegression::forget()
{
    MODULE_LOG << "forget() called" << endl;
    /*!
      A typical forget() method should do the following:
      - initialize a random number generator with the seed option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
    resetGenerator(seed_);
    input_params->forget() ; 
    for( int i=1 ; i<n_layers-1 ; i++ )
        params[i]->forget();

    for( int i=0 ; i<n_layers ; i++ )
        layers[i]->reset();

    target_params->forget();
    target_layer->reset();

    stage = 0;
}

//////////////
// generate //
//////////////
void GaussianDBNRegression::generate(Vec& y) const
{
    PLERROR("generate not implemented for GaussianDBNRegression");
}

/////////
// cdf //
/////////
real GaussianDBNRegression::cdf(const Vec& y) const
{
    PLERROR("cdf not implemented for GaussianDBNRegression"); return 0;
}

/////////////////
// expectation //
/////////////////
void GaussianDBNRegression::expectation(Vec& mu) const
{
    mu.resize( predicted_size );

    // Propagate input (predictor_part) until penultimate layer
    layers[0]->expectation << predictor_part;
    input_params->setAsDownInput(layers[0]->expectation) ; 
    layers[1]->getAllActivations( (RBMQLParameters*) input_params );
    layers[1]->computeExpectation();
    
    for( int i=1 ; i<n_layers-1 ; i++ )
    {
        params[i]->setAsDownInput( layers[i]->expectation );
        layers[i+1]->getAllActivations( (RBMLLParameters*) params[i] );
        layers[i+1]->computeExpectation();
    }
        
    target_params->setAsDownInput( last_layer->expectation );
    target_layer->getAllActivations( (RBMLQParameters*) target_params );
    target_layer->computeExpectation();

    mu << target_layer->expectation;

}

/////////////
// density //
/////////////
real GaussianDBNRegression::density(const Vec& y) const
{
    PLASSERT( y.size() == n_predicted );

    // TODO: 'y'[0] devrait plutot etre l'entier "index" lui-meme!
    int index = argmax( y );

    // If y != onehot( index ), then density is 0
    if( !is_equal( y[index], 1. ) )
        return 0;
    for( int i=0 ; i<n_predicted ; i++ )
        if( !is_equal( y[i], 0 ) && i != index )
            return 0;

    expectation( store_expect );
    return store_expect[index];
}


/////////////////
// log_density //
/////////////////
real GaussianDBNRegression::log_density(const Vec& y) const
{
    return pl_log( density(y) );
}

/////////////////
// survival_fn //
/////////////////
real GaussianDBNRegression::survival_fn(const Vec& y) const
{
    PLERROR("survival_fn not implemented for GaussianDBNRegression"); return 0;
}

//////////////
// variance //
//////////////
void GaussianDBNRegression::variance(Mat& cov) const
{
    PLERROR("variance not implemented for GaussianDBNRegression");
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GaussianDBNRegression::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(layers, copies);
    deepCopyField(last_layer, copies);
    deepCopyField(target_layer, copies);
    deepCopyField(params, copies);
    deepCopyField(input_params, copies);
    deepCopyField(target_params, copies);
    deepCopyField(training_schedule, copies);
}

//////////////////
// setPredictor //
//////////////////
void GaussianDBNRegression::setPredictor(const Vec& predictor, bool call_parent)
    const
{
    if (call_parent)
        inherited::setPredictor(predictor, true);
    // ### Add here any specific code required by your subclass.
}

////////////////////////////////
// setPredictorPredictedSizes //
////////////////////////////////
bool GaussianDBNRegression::setPredictorPredictedSizes(int the_predictor_size,
                                                     int the_predicted_size,
                                                     bool call_parent)
{
    bool sizes_have_changed = false;
    if (call_parent)
        sizes_have_changed = inherited::setPredictorPredictedSizes(
            the_predictor_size, the_predicted_size, true);

    // ### Add here any specific code required by your subclass.
    if( the_predictor_size >= 0 && the_predictor_size != layers[0]->size ||
        the_predicted_size >= 0 && the_predicted_size != target_layer->size )
        PLERROR( "GaussianDBNRegression::setPredictorPredictedSizes - \n"
                 "n_predictor should be equal to layer[0]->size (%d)\n"
                 "n_predicted should be equal to target_layer->size (%d).\n",
                 layers[0]->size, target_layer->size );

    n_predictor = layers[0]->size;
    n_predicted = target_layer->size;

    // Returned value.
    return sizes_have_changed;
}


///////////
// train //
///////////
void GaussianDBNRegression::train()
{
    MODULE_LOG << "train() called" << endl;
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

    Vec input( inputsize() );
    Vec target( targetsize() ); // unused
    real weight; // unused

    if( !initTrain() )
    {
        MODULE_LOG << "train() aborted" << endl;
        return;
    }

    int nsamples = train_set->length();
    MODULE_LOG << "  nsamples = " << nsamples << endl;

    // Let's define stage and nstages:
    //   - 0: fresh state, nothing is done
    //   - 1..n_layers-2: params[stage-1] is trained
    //   - n_layers-1: joint_params is trained (including params[n_layers-2])
    //   - n_layers: after the fine tuning

    MODULE_LOG << "initial stage = " << stage << endl;
    MODULE_LOG << "objective: nstages = " << nstages << endl;
        
    // clear stats of previous epoch
    train_stats->forget();

    for(int layer=0 ; layer<n_layers-1 ; ++layer) { 
            
        MODULE_LOG << "Training parameters between layers " << layer
            << " and " << layer+1 << endl;
        
        // this progress bar shows the number of loops through the whole
        // training set
        ProgressBar* pb = 0;
              
        int end_stage = min( training_schedule[layer], nstages );              
        if( report_progress && stage < end_stage )
        {
            pb = new ProgressBar( "Training layer "+tostring(layer)+
                    "of" + classname(), end_stage - stage );
        }


        for( ; stage < end_stage ; stage++ )
        {

                // sample is the index in the training set
                int sample = stage % train_set->length();
                train_set->getExample(sample, input, target, weight);
                greedyStep( input.subVec(0, n_predictor), layer );

                if( pb )
                {
                    if( layer == 0 )
                        pb->update(stage + 1);
                    else
                        pb->update(stage - training_schedule[layer-1] + 1);
                }

                Mat inputs(train_set.length() , n_predictor) ; 
                Mat outputs(train_set.length() , n_predicted);
                Mat theta(1 + n_predictor , n_predicted) ; 
                Vec output_value(n_predicted) ; 

                for(int i=0 ; i<train_set.length() ; ++i) { 
                    train_set->getExample(i, input, target, weight);
                    // split input in predictor_part and predicted_part
                    splitCond(input);

                    // compute predicted_part expectation, conditioned on predictor_part
                    // (forward pass)
                    expectation( output_value );
                    for(int j=0 ; j<n_predictor ; ++j) { 
                        inputs[i][j] = last_layer->expectation[j] ; 
//                        cout << last_layer->expectation[j] << " " ; 
                    }
                    for(int j=0 ; j<n_predicted ; ++j) { 
                        outputs[i][j] = input[j+n_predictor] ; 
                    }
                }

//                pout << "inputs " << endl << inputs << endl  ; 
                
//                pout << "outputs " << endl << outputs << endl  ; 
                
                linearRegression(inputs,outputs,0.0,theta); 
                // init the a_i term
                target_params->up_units_params[1].fill(1) ; 

//                pout << "Theta" << theta << endl ; 

                // set the bias (b_i)
                for(int i=0 ; i<n_predicted ; ++i) { 
                    target_params->up_units_params[0][i] = - 2.0 * theta[i][0] ; 
                }

                for(int i=0 ; i<n_predicted ; ++i) { 
                    for(int j=0 ; j<n_predictor ; ++j) { 
                        target_params->weights[i][j] = -2.0 * theta[j][i+1] ; 
                    }
                }

                
        }
            

    }
/*            
    MODULE_LOG << "Fine-tuning all parameters, using method "
    << fine_tuning_method << endl;

            if( fine_tuning_method == "" ) // do nothing
                sample += n_samples_to_see;
            else if( fine_tuning_method == "EGD" )
            {
                if( report_progress )
                    pb = new ProgressBar( "Training all " + classname()
                                          + " parameters by fine tuning",
                                          n_samples_to_see );

*/
                                          
/*
pout << "==================" << endl
    << "Before update:" << endl
    << "up:      " << joint_params->up_units_params << endl
    << "weights: " << endl << joint_params->weights << endl
    << "down:    " << joint_params->down_units_params << endl
    << endl;
// */

                // linear regression for last weights
                
                
/*
                int begin_sample = sample;
                int end_sample = begin_sample + n_samples_to_see;
                for( ; sample < end_sample ; sample++ )
                {
                    // sample is the index in the training set
                    int i = sample % train_set->length();
                    train_set->getExample(i, input, target, weight);
                    fineTuneByGradientDescentLastLayer( input );

                    if( pb )
                        pb->update( sample - begin_sample + 1 );
                }

                sample = begin_sample ; 
                for( ; sample < 100 ; sample++ )
                {
                    // sample is the index in the training set
                    int i = sample % train_set->length();
                    train_set->getExample(i, input, target, weight);
                    fineTuneByGradientDescent( input );

                    if( pb )
                        pb->update( sample - begin_sample + 1 );
                }
*/                

                
/*
pout << "-------" << endl
    << "After update:" << endl
    << "up:      " << joint_params->up_units_params << endl
    << "weights: " << endl << joint_params->weights << endl
    << "down:    " << joint_params->down_units_params << endl
    << endl;
// */

    train_stats->finalize(); // finalize statistics for this epoch
    MODULE_LOG << endl;
}

void GaussianDBNRegression::greedyStep( const Vec& predictor, int index )
{
    // deterministic propagation until we reach index
    layers[0]->expectation << predictor;

    input_params->setAsDownInput( layers[0]->expectation );
    layers[1]->getAllActivations( (RBMQLParameters*) input_params );
    layers[1]->computeExpectation();
        
    for( int i=1 ; i<index ; i++ )
    {
        params[i]->setAsDownInput( layers[i]->expectation );
        layers[i+1]->getAllActivations( (RBMLLParameters*) params[i] );
        layers[i+1]->computeExpectation();
    }

    // positive phase
    if (index == 0) {
        input_params->setAsDownInput( layers[index]->expectation );
        layers[index+1]->getAllActivations((RBMQLParameters*) input_params);
        layers[index+1]->computeExpectation();
        layers[index+1]->generateSample();
        if (use_sample_rather_than_expectation_in_positive_phase_statistics)
            input_params->accumulatePosStats(layers[index]->expectation,
                    layers[index+1]->sample );
        else
            input_params->accumulatePosStats(layers[index]->expectation,
                    layers[index+1]->expectation );

        // down propagation
        input_params->setAsUpInput( layers[index+1]->sample );
        layers[index]->getAllActivations( (RBMQLParameters*) input_params );

        // negative phase
        layers[index]->generateSample();
        input_params->setAsDownInput( layers[index]->sample );
        layers[index+1]->getAllActivations((RBMQLParameters*) input_params);
        layers[index+1]->computeExpectation();
        input_params->accumulateNegStats( layers[index]->sample,
                layers[index+1]->expectation );

        // update
        input_params->update();

    }
    else {
        params[index]->setAsDownInput( layers[index]->expectation );
        layers[index+1]->getAllActivations((RBMLLParameters*) params[index]);
        layers[index+1]->computeExpectation();
        layers[index+1]->generateSample();
        if (use_sample_rather_than_expectation_in_positive_phase_statistics)
            params[index]->accumulatePosStats(layers[index]->expectation,
                    layers[index+1]->sample );
        else
            params[index]->accumulatePosStats(layers[index]->expectation,
                    layers[index+1]->expectation );

        // down propagation
        params[index]->setAsUpInput( layers[index+1]->sample );
        layers[index]->getAllActivations( (RBMLLParameters*) params[index] );

        // negative phase
        layers[index]->generateSample();
        params[index]->setAsDownInput( layers[index]->sample );
        layers[index+1]->getAllActivations((RBMLLParameters*) params[index]);
        layers[index+1]->computeExpectation();
        params[index]->accumulateNegStats( layers[index]->sample,
                layers[index+1]->expectation );

        // update
        params[index]->update();

    }
    

}



void GaussianDBNRegression::fineTuneByGradientDescentLastLayer( const Vec& input )
{
    // split input in predictor_part and predicted_part
    splitCond(input);

    // compute predicted_part expectation, conditioned on predictor_part
    // (forward pass)
    expectation( output_gradient );

    int target_size = predicted_part.size() ; 

    expectation_gradients[n_layers].resize(target_size) ; 
    
    for(int i=0 ; i < target_size ; ++i) { 
        expectation_gradients[n_layers][i] = 2 * (output_gradient[i] - predicted_part[i]) ;
    }

    target_layer->bpropUpdate( target_layer->activations,
                               target_layer->expectation,
                               activation_gradients[n_layers] ,
                               expectation_gradients[n_layers]) ; 
    
    target_params->bpropUpdate( layers[n_layers-1]->expectation,
                               target_layer->activations,
                               expectation_gradients[n_layers-1],
                               activation_gradients[n_layers] );
    
}

void GaussianDBNRegression::fineTuneByGradientDescent( const Vec& input )
{
    // split input in predictor_part and predicted_part
    splitCond(input);

    // compute predicted_part expectation, conditioned on predictor_part
    // (forward pass)
    expectation( output_gradient );

    int target_size = predicted_part.size() ; 

    expectation_gradients[n_layers].resize(target_size) ; 
    
    for(int i=0 ; i < target_size ; ++i) { 
        expectation_gradients[n_layers][i] = 2 * (output_gradient[i] - predicted_part[i]) ;
    }

    target_layer->bpropUpdate( target_layer->activations,
                               target_layer->expectation,
                               activation_gradients[n_layers] ,
                               expectation_gradients[n_layers]) ; 
    
    target_params->bpropUpdate( layers[n_layers-1]->expectation,
                               target_layer->activations,
                               expectation_gradients[n_layers-1],
                               activation_gradients[n_layers] );

    for( int i=n_layers-1 ; i>1 ; i-- )
    {
        layers[i]->bpropUpdate( layers[i]->activations,
                                layers[i]->expectation,
                                activation_gradients[i],
                                expectation_gradients[i] );
        params[i-1]->bpropUpdate( layers[i-1]->expectation,
                                  layers[i]->activations,
                                  expectation_gradients[i-1],
                                  activation_gradients[i] );
    }
    
        layers[1]->bpropUpdate( layers[1]->activations,
                                layers[1]->expectation,
                                activation_gradients[1],
                                expectation_gradients[1] );
        
        input_params->bpropUpdate( layers[0]->expectation,
                                  layers[1]->activations,
                                  expectation_gradients[0],
                                  activation_gradients[1] );
                                  

}

void GaussianDBNRegression::computeCostsFromOutputs(const Vec& input,
                                                  const Vec& output,
                                                  const Vec& target,
                                                  Vec& costs) const
{
    char c = outputs_def[0];
    if( c == 'l' || c == 'd' )
        inherited::computeCostsFromOutputs(input, output, target, costs);
    else if( c == 'e' )
    {
        costs.resize( 1 );
        costs[0] = .0 ; 
        splitCond(input);
        
        int output_size = output.length(); 
        for(int i=0 ; i<output_size ; ++i) { 
            costs[0] += square(output[i] - predicted_part[i]) ;
        }

        costs[0] /= output_size ; 

    }
}

TVec<string> GaussianDBNRegression::getTestCostNames() const
{
    char c = outputs_def[0];
    TVec<string> result;
    if( c == 'l' || c == 'd' )
        result.append( "NLL" );
    else if( c == 'e' )
    {
        result.append( "MSE" );
    }
    return result;
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
