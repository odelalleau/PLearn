// -*- C++ -*-

// GaussianDBNClassification.cc
//
// Copyright (C) 2006 Pascal Lamblin
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

/*! \file GaussianDBNClassification.cc */

#define PL_LOG_MODULE_NAME "GaussianDBNClassification"
#include <plearn/io/pl_log.h>

#include "GaussianDBNClassification.h"
#include "RBMLayer.h"
#include "RBMMixedLayer.h"
#include "RBMMultinomialLayer.h"
#include "RBMParameters.h"
#include "RBMLLParameters.h"
#include "RBMQLParameters.h"
#include "RBMJointLLParameters.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    GaussianDBNClassification,
    "Does the same thing as Hinton's deep belief nets",
    ""
);

/////////////////////////
// GaussianDBNClassification //
/////////////////////////
GaussianDBNClassification::GaussianDBNClassification() :
    learning_rate(0.),
    weight_decay(0.),
    use_sample_rather_than_expectation_in_positive_phase_statistics(false)
{
    random_gen = new PRandom();
}

////////////////////
// declareOptions //
////////////////////
void GaussianDBNClassification::declareOptions(OptionList& ol)
{
    declareOption(ol, "learning_rate", &GaussianDBNClassification::learning_rate,
                  OptionBase::buildoption,
                  "Learning rate");

    declareOption(ol, "weight_decay", &GaussianDBNClassification::weight_decay,
                  OptionBase::buildoption,
                  "Weight decay");

    declareOption(ol, "initialization_method",
                  &GaussianDBNClassification::initialization_method,
                  OptionBase::buildoption,
                  "The method used to initialize the weights:\n"
                  "  - \"uniform_linear\" = a uniform law in [-1/d, 1/d]\n"
                  "  - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(d),"
                  " 1/sqrt(d)]\n"
                  "  - \"zero\"           = all weights are set to 0,\n"
                  "where d = max( up_layer_size, down_layer_size ).\n");


    declareOption(ol, "training_schedule",
                  &GaussianDBNClassification::training_schedule,
                  OptionBase::buildoption,
                  "Number of examples to use during each of the different"
                  " greedy\n"
                  "steps of the training phase.\n");

    declareOption(ol, "fine_tuning_method",
                  &GaussianDBNClassification::fine_tuning_method,
                  OptionBase::buildoption,
                  "Method for fine-tuning the whole network after greedy"
                  " learning.\n"
                  "One of:\n"
                  "  - \"none\"\n"
                  "  - \"CD\" or \"contrastive_divergence\"\n"
                  "  - \"EGD\" or \"error_gradient_descent\"\n"
                  "  - \"WS\" or \"wake_sleep\".\n");

    declareOption(ol, "layers", &GaussianDBNClassification::layers,
                  OptionBase::buildoption,
                  "Layers that learn representations of the input,"
                  " unsupervisedly.\n"
                  "layers[0] is input layer.\n");

    declareOption(ol, "target_layer", &GaussianDBNClassification::target_layer,
                  OptionBase::buildoption,
                  "Target (or label) layer");

    declareOption(ol, "params", &GaussianDBNClassification::params,
                  OptionBase::buildoption,
                  "RBMParameters linking the unsupervised layers.\n"
                  "params[i] links layers[i] and layers[i+1], except for"
                  "params[n_layers-1],\n"
                  "that links layers[n_layers-1] and last_layer.\n");

    declareOption(ol, "target_params", &GaussianDBNClassification::target_params,
                  OptionBase::buildoption,
                  "Parameters linking target_layer and last_layer");

    declareOption(ol, "use_sample_rather_than_expectation_in_positive_phase_statistics",
                  &GaussianDBNClassification::use_sample_rather_than_expectation_in_positive_phase_statistics,
                  OptionBase::buildoption,
                  "In positive phase statistics use output->sample * input\n"
                  "rather than output->expectation * input.\n");

    declareOption(ol, "n_layers", &GaussianDBNClassification::n_layers,
                  OptionBase::learntoption,
                  "Number of unsupervised layers, including input layer");

    declareOption(ol, "last_layer", &GaussianDBNClassification::last_layer,
                  OptionBase::learntoption,
                  "Last layer, learning joint representations of input and"
                  " target");

    declareOption(ol, "joint_layer", &GaussianDBNClassification::joint_layer,
                  OptionBase::learntoption,
                  "Concatenation of target_layer and layers[n_layers-1]");

    declareOption(ol, "joint_params", &GaussianDBNClassification::joint_params,
                  OptionBase::learntoption,
                  "Parameters linking joint_layer and last_layer");

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void GaussianDBNClassification::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void GaussianDBNClassification::build_()
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
        PLERROR( "GaussianDBNClassification::build_ - fine_tuning_method \"%s\"\n"
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

void GaussianDBNClassification::build_layers()
{
    MODULE_LOG << "build_layers() called" << endl;
    if( inputsize_ >= 0 )
    {
        assert( layers[0]->size + target_layer->size == inputsize() );
        setPredictorPredictedSizes( layers[0]->size,
                                    target_layer->size, false );
        MODULE_LOG << "  n_predictor = " << n_predictor << endl;
        MODULE_LOG << "  n_predicted = " << n_predicted << endl;
    }

    for( int i=0 ; i<n_layers ; i++ )
        layers[i]->random_gen = random_gen;
    target_layer->random_gen = random_gen;

    last_layer = layers[n_layers-1];

    // concatenate target_layer and layers[n_layers-2] into joint_layer
    TVec< PP<RBMLayer> > the_sub_layers( 2 );
    the_sub_layers[0] = target_layer;
    the_sub_layers[1] = layers[n_layers-2];
    joint_layer = new RBMMixedLayer( the_sub_layers );
    joint_layer->random_gen = random_gen;
}

void GaussianDBNClassification::build_params()
{
    MODULE_LOG << "build_params() called" << endl;
    if( params.length() == 0 )
    {
        params.resize( n_layers-1 );
        for( int i=0 ; i<n_layers-1 ; i++ )
            params[i] = new RBMLLParameters();
    }
    else if( params.length() != n_layers-1 )
        PLERROR( "GaussianDBNClassification::build_params - params.length() should\n"
                 "be equal to layers.length()-1 (%d != %d).\n",
                 params.length(), n_layers-1 );

    activation_gradients.resize( n_layers-1 );
    expectation_gradients.resize( n_layers-1 );
    output_gradient.resize( n_predicted );

    for( int i=0 ; i<n_layers-1 ; i++ )
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
        target_params = new RBMLLParameters();

    //TODO: call changeOptions instead
    target_params->down_units_types = target_layer->units_types;
    target_params->up_units_types = last_layer->units_types;
    target_params->learning_rate = learning_rate;
    target_params->initialization_method = initialization_method;
    target_params->random_gen = random_gen;
    target_params->build();

    // build joint_params from params[n_layers-1] and target_params
    joint_params = new RBMJointLLParameters( target_params,
                                             params[n_layers-2] );
    joint_params->learning_rate = learning_rate;
    joint_params->random_gen = random_gen;
}

////////////
// forget //
////////////
void GaussianDBNClassification::forget()
{
    MODULE_LOG << "forget() called" << endl;
    /*!
      A typical forget() method should do the following:
      - initialize a random number generator with the seed option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
    resetGenerator(seed_);
    for( int i=0 ; i<n_layers-1 ; i++ )
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
void GaussianDBNClassification::generate(Vec& y) const
{
    PLERROR("generate not implemented for GaussianDBNClassification");
}

/////////
// cdf //
/////////
real GaussianDBNClassification::cdf(const Vec& y) const
{
    PLERROR("cdf not implemented for GaussianDBNClassification"); return 0;
}

/////////////////
// expectation //
/////////////////
void GaussianDBNClassification::expectation(Vec& mu) const
{
    mu.resize( predicted_size );

    // Propagate input (predictor_part) until penultimate layer
    layers[0]->expectation << predictor_part;
    for( int i=0 ; i<n_layers-2 ; i++ )
    {
        params[i]->setAsDownInput( layers[i]->expectation );
        layers[i+1]->getAllActivations( (RBMLLParameters*) params[i] );
        layers[i+1]->computeExpectation();
    }

    // Set layers[n_layers-2]->expectation (penultimate) as conditionning input
    // of joint_params
    joint_params->setAsCondInput( layers[n_layers-2]->expectation );

    // Get all activations on target_layer from target_params
    target_layer->getAllActivations( (RBMLLParameters*) joint_params );
    target_layer->computeExpectation();

    mu << target_layer->expectation;
}

/////////////
// density //
/////////////
real GaussianDBNClassification::density(const Vec& y) const
{
    assert( y.size() == n_predicted );

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
real GaussianDBNClassification::log_density(const Vec& y) const
{
    return pl_log( density(y) );
}

/////////////////
// survival_fn //
/////////////////
real GaussianDBNClassification::survival_fn(const Vec& y) const
{
    PLERROR("survival_fn not implemented for GaussianDBNClassification"); return 0;
}

//////////////
// variance //
//////////////
void GaussianDBNClassification::variance(Mat& cov) const
{
    PLERROR("variance not implemented for GaussianDBNClassification");
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GaussianDBNClassification::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(layers, copies);
    deepCopyField(last_layer, copies);
    deepCopyField(target_layer, copies);
    deepCopyField(joint_layer, copies);
    deepCopyField(params, copies);
    deepCopyField(joint_params, copies);
    deepCopyField(target_params, copies);
    deepCopyField(training_schedule, copies);
}

//////////////////
// setPredictor //
//////////////////
void GaussianDBNClassification::setPredictor(const Vec& predictor, bool call_parent)
    const
{
    if (call_parent)
        inherited::setPredictor(predictor, true);
    // ### Add here any specific code required by your subclass.
}

////////////////////////////////
// setPredictorPredictedSizes //
////////////////////////////////
bool GaussianDBNClassification::setPredictorPredictedSizes(int the_predictor_size,
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
        PLERROR( "GaussianDBNClassification::setPredictorPredictedSizes - \n"
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
void GaussianDBNClassification::train()
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
    int sample = 0;
    MODULE_LOG << "  nsamples = " << nsamples << endl;

    // Let's define stage and nstages:
    //   - 0: fresh state, nothing is done
    //   - 1..n_layers-2: params[stage-1] is trained
    //   - n_layers-1: joint_params is trained (including params[n_layers-2])
    //   - n_layers: after the fine tuning

    MODULE_LOG << "initial stage = " << stage << endl;
    MODULE_LOG << "objective: nstages = " << nstages << endl;

    for( ; stage < nstages ; stage++ )
    {
        // clear stats of previous epoch
        train_stats->forget();

        // loops over the training set, until training_schedule[stage] examples
        // have been seen.
        // TODO: modify the training set used?
        int layer = stage;
        int n_samples_to_see = training_schedule[stage];

        // this progress bar shows the number of loops through the whole
        // training set
        ProgressBar* pb = 0;

        if( stage < n_layers-2 )
        {
            MODULE_LOG << "Training parameters between layers " << stage
                << " and " << stage+1 << endl;

            if( report_progress )
                pb = new ProgressBar( "Training " + classname()
                                      + " parameters between layers "
                                      + tostring(stage) + " and "
                                      + tostring(stage+1),
                                      n_samples_to_see );

            int begin_sample = sample;
            int end_sample = begin_sample + n_samples_to_see;
            for( ; sample < end_sample ; sample++ )
            {
                // sample is the index in the training set
                int i = sample % train_set->length();
                train_set->getExample(i, input, target, weight);
                greedyStep( input.subVec(0, n_predictor), layer );

                if( pb )
                    pb->update( sample - begin_sample + 1 );
            }

        }
        else if( stage == n_layers-2 )
        {
            MODULE_LOG << "Training joint parameters, between target,"
                << " penultimate (" << n_layers-2 << ")," << endl
                << "and last (" << n_layers-1 << ") layers." << endl;
            if( report_progress )
                pb = new ProgressBar( "Training " + classname()
                                      + " parameters between target, "
                                      + tostring(stage) + " and "
                                      + tostring(stage+1) + " layers",
                                      n_samples_to_see );

            int begin_sample = sample;
            int end_sample = begin_sample + n_samples_to_see;

            for( ; sample < end_sample ; sample++ )
            {
                // sample is the index in the training set
                int i = sample % train_set->length();
                train_set->getExample(i, input, target, weight);
                jointGreedyStep( input );

                if( pb )
                    pb->update( sample - begin_sample + 1 );
            }
        }
        else if( stage == n_layers-1 )
        {
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

/*
pout << "==================" << endl
    << "Before update:" << endl
    << "up:      " << joint_params->up_units_params << endl
    << "weights: " << endl << joint_params->weights << endl
    << "down:    " << joint_params->down_units_params << endl
    << endl;
// */
                int begin_sample = sample;
                int end_sample = begin_sample + n_samples_to_see;
                for( ; sample < end_sample ; sample++ )
                {
                    // sample is the index in the training set
                    int i = sample % train_set->length();
                    train_set->getExample(i, input, target, weight);
                    fineTuneByGradientDescent( input );

                    if( pb )
                        pb->update( sample - begin_sample + 1 );
                }
/*
pout << "-------" << endl
    << "After update:" << endl
    << "up:      " << joint_params->up_units_params << endl
    << "weights: " << endl << joint_params->weights << endl
    << "down:    " << joint_params->down_units_params << endl
    << endl;
// */
            }
            else
                PLERROR( "Fine-tuning methods other than \"EGD\" are not"
                         " implemented yet." );

        }
        train_stats->finalize(); // finalize statistics for this epoch
    }
    MODULE_LOG << endl;
}

void GaussianDBNClassification::greedyStep( const Vec& predictor, int index )
{
    // deterministic propagation until we reach index
    layers[0]->expectation << predictor;
    for( int i=0 ; i<index ; i++ )
    {
        params[i]->setAsDownInput( layers[i]->expectation );
        layers[i+1]->getAllActivations( (RBMLLParameters*) params[i] );
        layers[i+1]->computeExpectation();
    }

    // positive phase
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

void GaussianDBNClassification::jointGreedyStep( const Vec& input )
{
    // deterministic propagation until we reach n_layers-2, setting the input
    // of the "input" part of joint_layer
    layers[0]->expectation << input.subVec( 0, n_predictor );
    for( int i=0 ; i<n_layers-2 ; i++ )
    {
        params[i]->setAsDownInput( layers[i]->expectation );
        layers[i+1]->getAllActivations( (RBMLLParameters*) params[i] );
        layers[i+1]->computeExpectation();
    }

    // now fill the "target" part of joint_layer
    target_layer->expectation << input.subVec( n_predictor, n_predicted );

    // positive phase
    joint_params->setAsDownInput( joint_layer->expectation );
    last_layer->getAllActivations( (RBMLLParameters*) joint_params );
    last_layer->computeExpectation();
    last_layer->generateSample();
    if (use_sample_rather_than_expectation_in_positive_phase_statistics)
        joint_params->accumulatePosStats( joint_layer->expectation,
                                          last_layer->sample );
    else
        joint_params->accumulatePosStats( joint_layer->expectation,
                                          last_layer->expectation );

    // down propagation
    joint_params->setAsUpInput( last_layer->sample );
    joint_layer->getAllActivations( (RBMLLParameters*) joint_params );

    // negative phase
    joint_layer->generateSample();
    joint_params->setAsDownInput( joint_layer->sample );
    last_layer->getAllActivations( (RBMLLParameters*) joint_params );
    last_layer->computeExpectation();
    joint_params->accumulateNegStats( joint_layer->sample,
                                      last_layer->expectation );

    // update
    joint_params->update();
}

void GaussianDBNClassification::fineTuneByGradientDescent( const Vec& input )
{
    // split input in predictor_part and predicted_part
    splitCond(input);

    // compute predicted_part expectation, conditioned on predictor_part
    // (forward pass)
    expectation( output_gradient );

    int actual_index = argmax(predicted_part);
    output_gradient[actual_index] -= 1.;

    joint_params->bpropUpdate( layers[n_layers-2]->expectation,
                               target_layer->expectation,
                               expectation_gradients[n_layers-2],
                               output_gradient );

    for( int i=n_layers-2 ; i>0 ; i-- )
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
}

void GaussianDBNClassification::computeCostsFromOutputs(const Vec& input,
                                                  const Vec& output,
                                                  const Vec& target,
                                                  Vec& costs) const
{
    char c = outputs_def[0];
    if( c == 'l' || c == 'd' )
        inherited::computeCostsFromOutputs(input, output, target, costs);
    else if( c == 'e' )
    {
        costs.resize( 2 );
        splitCond(input);

        // actual_index is the actual 'target'
        int actual_index = argmax(predicted_part);
#ifdef BOUNDCHECK
        for( int i=0 ; i<n_predicted ; i++ )
            assert( is_equal( predicted_part[i], 0. ) ||
                    i == actual_index && is_equal( predicted_part[i], 1. ) );
#endif
        costs[0] = -pl_log( output[actual_index] );

        // predicted_index is the most probable predicted class
        int predicted_index = argmax(output);
        if( predicted_index == actual_index )
            costs[1] = 0;
        else
            costs[1] = 1;
    }
}

TVec<string> GaussianDBNClassification::getTestCostNames() const
{
    char c = outputs_def[0];
    TVec<string> result;
    if( c == 'l' || c == 'd' )
        result.append( "NLL" );
    else if( c == 'e' )
    {
        result.append( "NLL" );
        result.append( "class_error" );
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
