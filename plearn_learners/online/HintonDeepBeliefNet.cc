// -*- C++ -*-

// HintonDeepBeliefNet.cc
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

// Authors: Pascal Lamblin

/*! \file HintonDeepBeliefNet.cc */

#define PL_LOG_MODULE_NAME "HintonDeepBeliefNet"
#include <plearn/io/pl_log.h>
#if USING_MPI
#include <plearn/sys/PLMPI.h>
#endif

#include "HintonDeepBeliefNet.h"
#include "RBMLayer.h"
#include "RBMMixedLayer.h"
#include "RBMMultinomialLayer.h"
#include "RBMParameters.h"
#include "RBMLLParameters.h"
#include "RBMJointLLParameters.h"
#include <unistd.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    HintonDeepBeliefNet,
    "Does the same thing as Hinton's deep belief nets",
    "or, at least, tries to do so..."
);

/////////////////////////
// HintonDeepBeliefNet //
/////////////////////////
HintonDeepBeliefNet::HintonDeepBeliefNet() :
    learning_rate(0.),
    fine_tuning_learning_rate(-1.),
    fine_tuning_decrease_ct(0.),
    initial_momentum(0.),
    final_momentum(0.),
    momentum_switch_time(-1),
    weight_decay(0.),
    minibatch_size(100),
    update_only_after_minibatch(false),
    sum_parallel_contributions(0),
    use_sample_or_expectation(4)
{
    use_sample_or_expectation[0] = 0;
    use_sample_or_expectation[1] = 1;
    use_sample_or_expectation[2] = 2;
    use_sample_or_expectation[3] = 0;
    random_gen = new PRandom();
    ptimer = new PTimer();
    ptimer->newTimer("training_time");
    ptimer->newTimer("test_time");
}

////////////////////
// declareOptions //
////////////////////
void HintonDeepBeliefNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "learning_rate", &HintonDeepBeliefNet::learning_rate,
                  OptionBase::buildoption,
                  "Learning rate used during greedy learning");

    declareOption(ol, "fine_tuning_learning_rate",
                  &HintonDeepBeliefNet::fine_tuning_learning_rate,
                  OptionBase::buildoption,
                  "Learning rate used during the gradient descent");

    declareOption(ol, "fine_tuning_decrease_ct",
                  &HintonDeepBeliefNet::fine_tuning_decrease_ct,
                  OptionBase::buildoption,
                  "Decrease constant used during the gradient descent\n"
                  "(in fact, it will only be updated only once every epoch.\n");

    declareOption(ol, "initial_momentum",
                  &HintonDeepBeliefNet::initial_momentum,
                  OptionBase::buildoption,
                  "Initial momentum factor (should be between 0 and 1)");

    declareOption(ol, "final_momentum",
                  &HintonDeepBeliefNet::final_momentum,
                  OptionBase::buildoption,
                  "Final momentum factor (should be between 0 and 1)");

    declareOption(ol, "momentum_switch_time",
                  &HintonDeepBeliefNet::momentum_switch_time,
                  OptionBase::buildoption,
                  "Number of samples to be seen by layer i before its momentum"
                  " switches\n"
                  "from initial_momentum to final_momentum.\n");

    declareOption(ol, "weight_decay", &HintonDeepBeliefNet::weight_decay,
                  OptionBase::buildoption,
                  "Weight decay");

    declareOption(ol, "initialization_method",
                  &HintonDeepBeliefNet::initialization_method,
                  OptionBase::buildoption,
                  "The method used to initialize the weights:\n"
                  "  - \"uniform_linear\" = a uniform law in [-1/d, 1/d]\n"
                  "  - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(d),"
                  " 1/sqrt(d)]\n"
                  "  - \"zero\"           = all weights are set to 0,\n"
                  "where d = max( up_layer_size, down_layer_size ).\n");


    declareOption(ol, "training_schedule",
                  &HintonDeepBeliefNet::training_schedule,
                  OptionBase::buildoption,
                  "Total number of examples that should be seen until each"
                  " layer\n"
                  "have been greedily trained.\n"
                  "We should always have training_schedule[i] <"
                  " training_schedule[i+1].\n");

    declareOption(ol, "fine_tuning_method",
                  &HintonDeepBeliefNet::fine_tuning_method,
                  OptionBase::buildoption,
                  "Method for fine-tuning the whole network after greedy"
                  " learning.\n"
                  "One of:\n"
                  "  - \"none\"\n"
                  "  - \"CD\" or \"contrastive_divergence\"\n"
                  "  - \"EGD\" or \"error_gradient_descent\"\n"
                  "  - \"WS\" or \"wake_sleep\".\n");

    declareOption(ol, "layers", &HintonDeepBeliefNet::layers,
                  OptionBase::buildoption,
                  "Layers that learn representations of the input,"
                  " unsupervisedly.\n"
                  "layers[0] is input layer.\n");

    declareOption(ol, "target_layer", &HintonDeepBeliefNet::target_layer,
                  OptionBase::buildoption,
                  "Target (or label) layer");

    declareOption(ol, "params", &HintonDeepBeliefNet::params,
                  OptionBase::buildoption,
                  "RBMParameters linking the unsupervised layers.\n"
                  "params[i] links layers[i] and layers[i+1], except for"
                  "params[n_layers-1],\n"
                  "that links layers[n_layers-1] and last_layer.\n");

    declareOption(ol, "target_params", &HintonDeepBeliefNet::target_params,
                  OptionBase::buildoption,
                  "Parameters linking target_layer and last_layer");

/*
    declareOption(ol, "use_sample_rather_than_expectation_in_positive_phase_statistics",
                  &HintonDeepBeliefNet::use_sample_rather_than_expectation_in_positive_phase_statistics,
                  OptionBase::buildoption,
                  "In positive phase statistics use output->sample * input\n"
                  "rather than output->expectation * input.\n");
*/
    declareOption(ol, "use_sample_or_expectation",
                  &HintonDeepBeliefNet::use_sample_or_expectation,
                  OptionBase::buildoption,
                  "Vector providing information on which information to use"
                  " during the\n"
                  "contrastive divergence step:\n"
                  "  - 0 means that we use the expectation only,\n"
                  "  - 1 means that we sample (for the next step), but we use"
                  " the\n"
                  "    expectation in the CD update formula,\n"
                  "  - 2 means that we use the sample only.\n"
                  "The order of the arguments matches the steps of CD:\n"
                  "  - visible unit during positive phase (you should keep it"
                  " to 0),\n"
                  "  - hidden unit during positive phase,\n"
                  "  - visible unit during negative phase,\n"
                  "  - hidden unit during negative phase (you should keep it"
                  " to 0).\n");

    declareOption(ol, "minibatch_size",
                  &HintonDeepBeliefNet::minibatch_size,
                  OptionBase::buildoption,
                  "This is the number of examples seen by one process\n"
                  "during training after which the weight updates are shared\n"
                  "among all the processes. When update_only_after_minibatch,\n"
                  "training is done by minibatches, with parameter updates\n"
                  "only after each minibatch of that size.\n");

    declareOption(ol, "update_only_after_minibatch",
                  &HintonDeepBeliefNet::update_only_after_minibatch,
                  OptionBase::buildoption,
                  "update parameters only after a minibatch has been seen.\n"
                  "CURRENT IMPLEMENTATION IS INEFFICIENT, NOT EXPLOITING\n"
                  "THE FAST MATRIX OPERATIONS POSSIBLE.\n");

    declareOption(ol, "sum_parallel_contributions",
                  &HintonDeepBeliefNet::sum_parallel_contributions,
                  OptionBase::buildoption,
                  "Only used when USING_MPI for parallelization\n"
                  "sum or average the delta-w contributions from different processes?\n");

    declareOption(ol, "n_layers", &HintonDeepBeliefNet::n_layers,
                  OptionBase::learntoption,
                  "Number of unsupervised layers, including input layer");

    declareOption(ol, "last_layer", &HintonDeepBeliefNet::last_layer,
                  OptionBase::learntoption,
                  "Last layer, learning joint representations of input and"
                  " target");

    declareOption(ol, "joint_layer", &HintonDeepBeliefNet::joint_layer,
                  OptionBase::nosave,
                  "Concatenation of target_layer and layers[n_layers-1]");

    declareOption(ol, "joint_params", &HintonDeepBeliefNet::joint_params,
                  OptionBase::nosave,
                  "Parameters linking joint_layer and last_layer");

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void HintonDeepBeliefNet::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void HintonDeepBeliefNet::build_()
{
    MODULE_LOG << "build_() called" << endl;
    n_layers = layers.length();
    if( n_layers <= 1 )
        return;

    if( fine_tuning_learning_rate < 0. )
        fine_tuning_learning_rate = learning_rate;

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
        PLERROR( "HintonDeepBeliefNet::build_ - fine_tuning_method \"%s\"\n"
                 "is unknown.\n", fine_tuning_method.c_str() );
    MODULE_LOG << "  fine_tuning_method = \"" << fine_tuning_method << "\""
        <<  endl;
    //TODO: build structure to store gradients during gradient descent

    if( training_schedule.length() != n_layers-1 )
        training_schedule = TVec<int>( n_layers-1, 1000000 );
    MODULE_LOG << "  training_schedule = " << training_schedule << endl;
    MODULE_LOG << endl;

    build_layers();
    build_params();
}

void HintonDeepBeliefNet::build_layers()
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

    // concatenate target_layer and layers[n_layers-2] into joint_layer,
    // if it is not already done
    if( !joint_layer
        || joint_layer->sub_layers.size() !=2
        || joint_layer->sub_layers[0] != target_layer
        || joint_layer->sub_layers[1] != layers[n_layers-2] )
    {
        TVec< PP<RBMLayer> > the_sub_layers( 2 );
        the_sub_layers[0] = target_layer;
        the_sub_layers[1] = layers[n_layers-2];
        joint_layer = new RBMMixedLayer( the_sub_layers );
    }
    joint_layer->random_gen = random_gen;
}

void HintonDeepBeliefNet::build_params()
{
    MODULE_LOG << "build_params() called" << endl;
    if( params.length() == 0 )
    {
        params.resize( n_layers-1 );
        for( int i=0 ; i<n_layers-1 ; i++ )
            params[i] = new RBMLLParameters();
    }
    else if( params.length() != n_layers-1 )
        PLERROR( "HintonDeepBeliefNet::build_params - params.length() should\n"
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
    target_params->initialization_method = initialization_method;
    target_params->random_gen = random_gen;
    target_params->build();

    // build joint_params from params[n_layers-1] and target_params
    // if it is not already done
    if( !joint_params
        || joint_params->target_params != target_params
        || joint_params->cond_params != params[n_layers-2] )
    {
        joint_params = new RBMJointLLParameters( target_params,
                                                 params[n_layers-2] );
    }
    joint_params->random_gen = random_gen;

    // share the biases
    for( int i=0 ; i<n_layers-2 ; i++ )
        params[i]->up_units_bias = params[i+1]->down_units_bias;
}

////////////
// forget //
////////////
void HintonDeepBeliefNet::forget()
{
    MODULE_LOG << "forget() called" << endl;
    /*!
      A typical forget() method should do the following:
      - initialize a random number generator with the seed option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
    ptimer->resetAllTimers();
    resetGenerator(seed_);
    for( int i=0 ; i<n_layers-1 ; i++ )
        params[i]->forget();

    for( int i=0 ; i<n_layers ; i++ )
        layers[i]->reset();

#if USING_MPI
    global_params.resize(0);
#endif
    target_params->forget();
    target_layer->reset();

    stage = 0;
}

//////////////
// generate //
//////////////
void HintonDeepBeliefNet::generate(Vec& y) const
{
    PLERROR("generate not implemented for HintonDeepBeliefNet");
}

/////////
// cdf //
/////////
real HintonDeepBeliefNet::cdf(const Vec& y) const
{
    PLERROR("cdf not implemented for HintonDeepBeliefNet"); return 0;
}

/////////////////
// expectation //
/////////////////
void HintonDeepBeliefNet::expectation(Vec& mu) const
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
real HintonDeepBeliefNet::density(const Vec& y) const
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
real HintonDeepBeliefNet::log_density(const Vec& y) const
{
    return pl_log( density(y) );
}

/////////////////
// survival_fn //
/////////////////
real HintonDeepBeliefNet::survival_fn(const Vec& y) const
{
    PLERROR("survival_fn not implemented for HintonDeepBeliefNet"); return 0;
}

//////////////
// variance //
//////////////
void HintonDeepBeliefNet::variance(Mat& cov) const
{
    PLERROR("variance not implemented for HintonDeepBeliefNet");
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void HintonDeepBeliefNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(ptimer, copies);
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
void HintonDeepBeliefNet::setPredictor(const Vec& predictor, bool call_parent)
    const
{
    if (call_parent)
        inherited::setPredictor(predictor, true);
    // ### Add here any specific code required by your subclass.
}

////////////////////////////////
// setPredictorPredictedSizes //
////////////////////////////////
bool HintonDeepBeliefNet::setPredictorPredictedSizes(int the_predictor_size,
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
        PLERROR( "HintonDeepBeliefNet::setPredictorPredictedSizes - \n"
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
void HintonDeepBeliefNet::train()
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

    Vec input( inputsize() );
    Vec target( targetsize() ); // unused
    real weight; // unused
    Vec train_costs(3);
    int nsamples = train_set->length();
    ptimer->startTimer("training_time");
#if USING_MPI
    // initialize global parameters for allowing to easily share them across
    // multiple CPUs

    // wait until we can attach a gdb process
    //pout << "START WAITING..." << endl;
    //sleep(20);
    //pout << "DONE WAITING!" << endl;
    MPI_Barrier(MPI_COMM_WORLD);
    int total_bsize=minibatch_size*PLMPI::size;
#endif
    // forget(); // DEBUGGING TO GET REPRODUCIBLE RESULTS
    if (global_params.size()==0)
    {
        int n_params = joint_params->nParameters(1,1);
        for (int i=0;i<params.length()-1;i++)
            n_params += params[i]->nParameters(0,1);
        global_params.resize(n_params);
        previous_global_params.resize(n_params);
        Vec p=global_params;
        for (int i=0;i<params.length()-1;i++)
            p=params[i]->makeParametersPointHere(p,0,1);
        p=joint_params->makeParametersPointHere(p,1,1);
        if (p.length()!=0)
            PLERROR("HintonDeepBeliefNet: Inconsistencies between nParameters and makeParametersPointHere!");
    }

    MODULE_LOG << "  nsamples = " << nsamples << endl;
    MODULE_LOG << "  initial stage = " << stage << endl;
    MODULE_LOG << "  objective: nstages = " << nstages << endl;

    if( !initTrain() )
    {
        MODULE_LOG << "train() aborted" << endl;
        return;
    }

    ProgressBar* pb = 0;

    // clear stats of previous epoch
    train_stats->forget();

    /***** initial greedy training *****/
    for( int layer=0 ; layer < n_layers-2 ; layer++ )
    {
        MODULE_LOG << "Training parameters between layers " << layer
            << " and " << layer+1 << endl;

        int end_stage = min( training_schedule[layer], nstages );

        MODULE_LOG << "  stage = " << stage << endl;
        MODULE_LOG << "  end_stage = " << end_stage << endl;

        if( report_progress && stage < end_stage )
        {
            pb = new ProgressBar( "Training layer "+tostring(layer)
                                  +" of "+classname(),
                                  end_stage - stage );
        }

        params[layer]->learning_rate = learning_rate;

        int momentum_switch_stage = momentum_switch_time;
        if( layer > 0 )
            momentum_switch_stage += training_schedule[layer-1];

        if( stage <= momentum_switch_stage )
            params[layer]->momentum = initial_momentum;
        else
            params[layer]->momentum = final_momentum;

        // make a copy of the parameters as they were at the beginning of
        // the minibatch
        previous_global_params << global_params;

        if (!sum_parallel_contributions || update_only_after_minibatch)
        {
            delta_params.resize(global_params.length());
            delta_params.clear();
        }

        for( ; stage<end_stage ; stage++ )
        {
#if USING_MPI
            // only look at some of the examples, associated with this process
            // number (rank)
            if (stage%PLMPI::size==PLMPI::rank)
            {
#endif
//                resetGenerator(1); // DEBUGGING HACK TO MAKE SURE RESULTS ARE INDEPENDENT OF PARALLELIZATION
                int sample = stage % nsamples;
                train_set->getExample(sample, input, target, weight);
                greedyStep( input.subVec(0, n_predictor), layer );

                if( stage == momentum_switch_stage )
                    params[layer]->momentum = final_momentum;

                if( pb )
                {
                    if( layer == 0 )
                        pb->update( stage + 1 );
                    else
                        pb->update( stage - training_schedule[layer-1] + 1 );
                }
#if USING_MPI
            }
            // time to share among processors
            if (stage%total_bsize==0 || stage==end_stage-1)
                shareParamsMPI();
#else
            if (update_only_after_minibatch && (stage%minibatch_size==0 || stage==end_stage-1))
            {
                global_params += delta_params;
                delta_params.clear();
            }
#endif
        }
    }

    /***** joint training *****/
    MODULE_LOG << "Training joint parameters, between target,"
        << " penultimate (" << n_layers-2 << ")," << endl
        << "and last (" << n_layers-1 << ") layers." << endl;

    int end_stage = min( training_schedule[n_layers-2], nstages );

    MODULE_LOG << "  stage = " << stage << endl;
    MODULE_LOG << "  end_stage = " << end_stage << endl;

    if( report_progress && stage < end_stage )
        pb = new ProgressBar( "Training joint layer (target and "
                             +tostring(n_layers-2)+") of "+classname(),
                             end_stage - stage );

    joint_params->learning_rate = learning_rate;
//    target_params->learning_rate = learning_rate;

    int previous_stage = (n_layers < 3) ? 0 : training_schedule[n_layers-3];
    int momentum_switch_stage = momentum_switch_time + previous_stage;
    if( stage <= momentum_switch_stage )
        joint_params->momentum = initial_momentum;
    else
        joint_params->momentum = final_momentum;

    int last = min(training_schedule[n_layers-2],nstages);
    for( ; stage<last ; stage++ )
    {
#if USING_MPI
        // only look at some of the examples, associated with this process number (rank)
        if (stage%PLMPI::size==PLMPI::rank)
        {
#endif
            int sample = stage % nsamples;
            train_set->getExample(sample, input, target, weight);
            jointGreedyStep( input );

            if( stage == momentum_switch_stage )
                joint_params->momentum = final_momentum;

            if( pb )
                pb->update( stage - previous_stage + 1 );
#if USING_MPI
        }
        // time to share among processors
        if (stage%total_bsize==0 || stage==last-1)
            shareParamsMPI();
#else
        if (update_only_after_minibatch && (stage%minibatch_size==0 || stage==end_stage-1))
        {
            global_params += delta_params;
            delta_params.clear();
        }
#endif
    }

    /***** fine-tuning *****/
    MODULE_LOG << "Fine-tuning all parameters, using method "
        << fine_tuning_method << endl;

    int init_stage = stage;
    if( report_progress && stage < nstages )
        pb = new ProgressBar( "Fine-tuning parameters of all layers of "
                             +classname(),
                             nstages - init_stage );

    MODULE_LOG << "  fine_tuning_learning_rate = "
        << fine_tuning_learning_rate << endl;

    for( int i=0 ; i<n_layers-1 ; i++ )
        params[i]->learning_rate = fine_tuning_learning_rate;
    joint_params->learning_rate = fine_tuning_learning_rate;
    target_params->learning_rate = fine_tuning_learning_rate;

    if( fine_tuning_method == "" ) // do nothing
    {
        stage = nstages;
        if( pb )
            pb->update( nstages - init_stage + 1 );
    }
    else if( fine_tuning_method == "EGD" )
    {
        int begin_sample = stage % nsamples;
        for( ; stage<nstages ; stage++ )
        {
#if USING_MPI
            // only look at some of the examples, associated with this process number (rank)
            if (stage%PLMPI::size==PLMPI::rank)
            {
#endif
                int sample = stage % nsamples;
                if( sample == begin_sample )
                    train_stats->forget();
                if( !fast_exact_is_equal( fine_tuning_learning_rate, 0. ) )
                {
                    real cur_learning_rate = fine_tuning_learning_rate
                        / (1. + fine_tuning_decrease_ct*(stage-init_stage) );
                    for( int i=0 ; i<n_layers-1 ; i++ )
                        params[i]->learning_rate = cur_learning_rate;
                    joint_params->learning_rate = cur_learning_rate;
                    target_params->learning_rate = cur_learning_rate;
                }

                train_set->getExample(sample, input, target, weight);
                fineTuneByGradientDescent( input, train_costs );
                train_stats->update( train_costs );

                if( pb )
                    pb->update( stage - init_stage + 1 );
#if USING_MPI
            }
            // time to share among processors
            if (stage%total_bsize==0 || stage==nstages-1)
                shareParamsMPI();
#else
            if (update_only_after_minibatch && (stage%minibatch_size==0 || stage==end_stage-1))
            {
                global_params += delta_params;
                delta_params.clear();
            }
#endif
        }
    }
    else
        PLERROR( "Fine-tuning methods other than \"EGD\" are not"
                 " implemented yet." );

    if( pb )
        delete pb;

    ptimer->stopTimer("training_time");
    real training_time = ptimer->getTimer("training_time");
    train_costs[2] = training_time;
    train_stats->update(train_costs);
    MODULE_LOG << "Training finished in " << endl << training_time << " seconds." << endl;
    train_stats->finalize(); // finalize statistics 
}

// assumes that down_layer->expectation is set
void HintonDeepBeliefNet::contrastiveDivergenceStep(
    const PP<RBMLayer>& down_layer,
    const PP<RBMParameters>& parameters,
    const PP<RBMLayer>& up_layer )
{
    // positive phase
    if( use_sample_or_expectation[0] == 0 )
        parameters->setAsDownInput( down_layer->expectation );
    else
    {
        down_layer->generateSample();
        parameters->setAsDownInput( down_layer->sample );
    }

    up_layer->getAllActivations( parameters );
    up_layer->computeExpectation();
    up_layer->generateSample();

    // accumulate stats using the right vector (sample or expectation)
    if( use_sample_or_expectation[0] == 2 )
    {
        if( use_sample_or_expectation[1] == 2 )
            parameters->accumulatePosStats(down_layer->sample,
                                           up_layer->sample );
        else
            parameters->accumulatePosStats(down_layer->sample,
                                           up_layer->expectation );
    }
    else
    {
        if( use_sample_or_expectation[1] == 2 )
            parameters->accumulatePosStats(down_layer->expectation,
                                           up_layer->sample);
        else
            parameters->accumulatePosStats(down_layer->expectation,
                                           up_layer->expectation );
    }

    // down propagation
    if( use_sample_or_expectation[1] == 0 )
        parameters->setAsUpInput( up_layer->expectation );
    else
        parameters->setAsUpInput( up_layer->sample );

    down_layer->getAllActivations( parameters );
    down_layer->computeExpectation();
    down_layer->generateSample();

    if( use_sample_or_expectation[2] == 0 )
        parameters->setAsDownInput( down_layer->expectation );
    else
        parameters->setAsDownInput( down_layer->sample );

    up_layer->getAllActivations( parameters );
    up_layer->computeExpectation();

    // accumulate stats using the right vector (sample or expectation)
    if( use_sample_or_expectation[3] == 2 )
    {
        up_layer->generateSample();
        if( use_sample_or_expectation[2] == 2 )
            parameters->accumulateNegStats( down_layer->sample,
                                            up_layer->sample );
        else
            parameters->accumulateNegStats( down_layer->expectation,
                                            up_layer->sample );
    }
    else
    {
        if( use_sample_or_expectation[2] == 2 )
            parameters->accumulateNegStats( down_layer->sample,
                                            up_layer->expectation );
        else
            parameters->accumulateNegStats( down_layer->expectation,
                                            up_layer->expectation );
    }

    // update
    if (update_only_after_minibatch)
    {
        previous_global_params << global_params;
        parameters->update();
        // delta_params += global_params - previous_global_params
        substractAcc(global_params,previous_global_params,delta_params);
        global_params << previous_global_params;
    }
    else
        parameters->update();
}

void HintonDeepBeliefNet::greedyStep( const Vec& predictor, int index )
{
    // deterministic propagation until we reach index
    layers[0]->expectation << predictor;
    for( int i=0 ; i<index ; i++ )
    {
        params[i]->setAsDownInput( layers[i]->expectation );
        layers[i+1]->getAllActivations( (RBMLLParameters*) params[i] );
        layers[i+1]->computeExpectation();
    }

    // perform one step of CD
    contrastiveDivergenceStep( layers[index],
                               (RBMLLParameters*) params[index],
                               layers[index+1] );
}

void HintonDeepBeliefNet::jointGreedyStep( const Vec& input )
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

    contrastiveDivergenceStep( (RBMLayer *) joint_layer,
                               (RBMLLParameters *) joint_params,
                               last_layer );
}

void HintonDeepBeliefNet::fineTuneByGradientDescent( const Vec& input,
                                                     const Vec& train_costs )
{
    // split input in predictor_part and predicted_part
    splitCond(input);

    // compute predicted_part expectation, conditioned on predictor_part
    // (forward pass)
    expectation( output_gradient );

    int actual_index = argmax(predicted_part);

    // update train_costs
#ifdef BOUNDCHECK
    for( int i=0 ; i<n_predicted ; i++ )
        assert( is_equal( predicted_part[i], 0. ) ||
                i == actual_index && is_equal( predicted_part[i], 1. ) );
#endif
    train_costs[0] = -pl_log( target_layer->expectation[actual_index] );
    int predicted_index = argmax( target_layer->expectation );
    if( predicted_index == actual_index )
        train_costs[1] = 0;
    else
        train_costs[1] = 1;

    // output gradient
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

void HintonDeepBeliefNet::computeCostsFromOutputs(const Vec& input,
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

TVec<string> HintonDeepBeliefNet::getTestCostNames() const
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
    result.append("time");
    return result;
}

TVec<string> HintonDeepBeliefNet::getTrainCostNames() const
{
    return getTestCostNames();
}

#if USING_MPI
void HintonDeepBeliefNet::shareParamsMPI()
{
    if (sum_parallel_contributions && !update_only_after_minibatch)
    {
        if (PLMPI::rank!=0)
            // after this line global_params contains the delta for all cpus except root
            global_params -= previous_global_params;
        // while the root contains the previous global params + its delta
        previous_global_params << global_params;
        // hence summing everything (result in cpu0.global_params)
        // yields the sum of all the changes plus the previous global params:
        MPI_Reduce(previous_global_params.data(),global_params.data(),
                   global_params.length(), PLMPI_REAL, MPI_SUM, 0, MPI_COMM_WORLD);
        // send it back to every one
        MPI_Bcast(global_params.data(), global_params.length(),
                  PLMPI_REAL, 0, MPI_COMM_WORLD);
        // and save it for next sharing step
        previous_global_params << global_params;
    }
    else // average contributions
    {
        if (update_only_after_minibatch)
        {
            global_params += delta_params;
            delta_params.clear();
        }
        //substract(global_params, previous_global_params, delta_params);
        previous_global_params << global_params;
        //MPI_Reduce(delta_params.data(),global_params.data(),
        MPI_Reduce(previous_global_params.data(),global_params.data(),
                   global_params.length(), PLMPI_REAL, MPI_SUM, 0, MPI_COMM_WORLD);
        global_params *= 1.0/PLMPI::size;
        //global_params += previous_global_params;
        MPI_Bcast(global_params.data(), global_params.length(),
                  PLMPI_REAL, 0, MPI_COMM_WORLD);
        //previous_global_params << global_params;
    }
}
#endif

void HintonDeepBeliefNet::test(VMat testset, PP<VecStatsCollector> test_stats,
                               VMat testoutputs, VMat testcosts) const
{
    int l = testset.length();
    Vec input;
    Vec target;
    real weight;

    Vec output(outputsize());

    Vec costs(nTestCosts());

    // testset->defineSizes(inputsize(),targetsize(),weightsize());

    int prank=
#if USING_MPI
        PLMPI::rank;
#else
        0;
#endif
    int psize=
#if USING_MPI
        PLMPI::size;
#else
        1;
#endif

    if (prank==0)
        ptimer->startTimer("test_time");
    ProgressBar* pb = NULL;
    if(report_progress)
        pb = new ProgressBar("Testing learner",l);

    if (l == 0) {
        // Empty test set: we give -1 cost arbitrarily.
        costs.fill(-1);
        test_stats->update(costs);
    }
    int n=int(ceil(l/real(psize)));
    Mat my_res(n,costs.size()+2);
    Mat all_res;
    if (prank==0) all_res.resize(n*psize,costs.size()+2);
    Vec learner_costs = costs.subVec(0,costs.size()-1);
    int k=0;
    for(int i=0; i<l; i++)
     if (i%psize==prank)
     {
        testset.getExample(i, input, target, weight);

        // Always call computeOutputAndCosts, since this is better
        // behaved with stateful learners
        computeOutputAndCosts(input,target,output,learner_costs);

        if(testoutputs)
            testoutputs->putOrAppendRow(i,output);

        if(testcosts)
            testcosts->putOrAppendRow(i, costs);

        if(test_stats)
        {
            my_res.subMat(k,0,1,learner_costs.length()) << learner_costs;
            my_res(k,costs.length()-1) = 0;
            my_res(k,costs.length()) = weight;
            my_res(k++,costs.length()+1) = 1;
        }

        if(report_progress)
            pb->update(i);
     }

#if USING_MPI
    if (prank==0)
       MPI_Gather(my_res.data(),my_res.size(),PLMPI_REAL,
                  all_res.data(),my_res.size(),PLMPI_REAL,0,MPI_COMM_WORLD);
    else
       MPI_Gather(my_res.data(),my_res.size(),PLMPI_REAL,
                  0,my_res.size(),PLMPI_REAL,0,MPI_COMM_WORLD);
#endif

    if (prank==0)
    {
        ptimer->stopTimer("test_time");
        real test_time = ptimer->getTimer("test_time");
        int nc=costs.length();
        for (int i=0;i<all_res.length();i++)
          if (all_res(i,nc+1)==1.0)
          {
              if (i==all_res.length()-1)
                  all_res(i,nc-1)=test_time;
              else
                  all_res(i,nc-1)=0;
              test_stats->update(all_res(i).subVec(0,nc),
                                 all_res(i,nc));
          }
    }

    if(pb)
        delete pb;
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
