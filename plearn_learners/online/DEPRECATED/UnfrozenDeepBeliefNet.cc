// -*- C++ -*-

// UnfrozenDeepBeliefNet.cc
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

/*! \file PLearn/plearn_learners/online/DEPRECATED/UnfrozenDeepBeliefNet.cc */

#define PL_LOG_MODULE_NAME "UnfrozenDeepBeliefNet"
#include <plearn/io/pl_log.h>

#include "UnfrozenDeepBeliefNet.h"
#include "RBMLayer.h"
#include "RBMMixedLayer.h"
#include "RBMMultinomialLayer.h"
#include "RBMParameters.h"
#include "RBMLLParameters.h"
#include "RBMJointLLParameters.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    UnfrozenDeepBeliefNet,
    "HintonDeepBeliefNet without freezing weights of earlier layers",
    ""
);

/////////////////////////
// UnfrozenDeepBeliefNet //
/////////////////////////
UnfrozenDeepBeliefNet::UnfrozenDeepBeliefNet() :
    inherited()
{
}

////////////////////
// declareOptions //
////////////////////
void UnfrozenDeepBeliefNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "learning_rates", &UnfrozenDeepBeliefNet::learning_rate,
                  OptionBase::buildoption,
                  "Learning rate of each layer");

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);

    redeclareOption(ol, "learning_rate", &UnfrozenDeepBeliefNet::learning_rate,
                    OptionBase::buildoption,
                    "Global learning rate, will not be used if learning_rates"
                    " is provided.");

    redeclareOption(ol, "training_schedule",
                  &UnfrozenDeepBeliefNet::training_schedule,
                  OptionBase::buildoption,
                  "No training_schedule, all layers are always learned.");

    redeclareOption(ol, "fine_tuning_method",
                    &UnfrozenDeepBeliefNet::fine_tuning_method,
                    OptionBase::nosave,
                    "No fine-tuning");
}

///////////
// build //
///////////
void UnfrozenDeepBeliefNet::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void UnfrozenDeepBeliefNet::build_()
{
    MODULE_LOG << "build_() called" << endl;
    MODULE_LOG << "stage = " << stage << endl;

    // check value of fine_tuning_method
    string ftm = lowerstring( fine_tuning_method );
    if( ftm == "" | ftm == "none" )
        fine_tuning_method = "";
    else
        PLERROR( "UnfrozenDeepBeliefNet::build_ - fine_tuning_method \"%s\"\n"
                 "is unknown.\n", fine_tuning_method.c_str() );
    MODULE_LOG << "  fine_tuning_method = \"" << fine_tuning_method << "\""
        <<  endl;

    if( learning_rates.length() != n_layers-1 )
        learning_rates = Vec( n_layers-1, learning_rate );

    for( int i=0 ; i<n_layers-2 ; i++ )
        params[i]->learning_rate = learning_rates[i];
    joint_params->learning_rate = learning_rates[n_layers-2];

    MODULE_LOG << "end of build_()" << endl;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void UnfrozenDeepBeliefNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


///////////
// train //
///////////
void UnfrozenDeepBeliefNet::train()
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
    Vec train_costs( 2 );

    if( !initTrain() )
    {
        MODULE_LOG << "train() aborted" << endl;
        return;
    }

    int nsamples = train_set->length();
    MODULE_LOG << "nsamples = " << nsamples << endl;

    MODULE_LOG << "initial stage = " << stage << endl;
    MODULE_LOG << "objective: nstages = " << nstages << endl;

    for( ; stage < nstages ; stage++ )
    {
        // sample is the index in the training set
        int sample = stage % nsamples;
        if( sample == 0 )
        {
            MODULE_LOG << "train_stats->forget() called" << endl;
            train_stats->forget();
        }
/*
        MODULE_LOG << "stage = " << stage << endl;
        MODULE_LOG << "sample = " << sample << endl;
// */
        if( (100*stage) % nsamples == 0 )
            MODULE_LOG << "stage = " << stage << endl;

        train_set->getExample(sample, input, target, weight);
        splitCond( input );

        // deterministic forward propagation
        layers[0]->expectation << predictor_part;
        for( int i=0 ; i<n_layers-2 ; i++ )
        {
            params[i]->setAsDownInput( layers[i]->expectation );
            layers[i+1]->getAllActivations( (RBMLLParameters*) params[i] );
            layers[i+1]->computeExpectation();
            layers[i+1]->generateSample();
            params[i]->accumulatePosStats( layers[i]->expectation,
                                           layers[i+1]->expectation );
        }

        // compute output and cost at this point, even though it is not the
        // criterion that will be directly optimized
        joint_params->setAsCondInput( layers[n_layers-2]->expectation );
        target_layer->getAllActivations( (RBMLLParameters*) joint_params );
        target_layer->computeExpectation();
        // get costs
        int actual_index = argmax(predicted_part);
        train_costs[0] = -pl_log( target_layer->expectation[actual_index] );
        if( argmax( target_layer->expectation ) == actual_index )
            train_costs[1] = 0;
        else
            train_costs[1] = 1;

        // end of the forward propagation
        target_layer->expectation << predicted_part;
        joint_params->setAsDownInput( joint_layer->expectation );
        last_layer->getAllActivations( (RBMLLParameters*) joint_params );
        last_layer->computeExpectation();
        last_layer->generateSample();
        joint_params->accumulatePosStats( joint_layer->expectation,
                                          last_layer->expectation );


        // for each params, one step of CD
        for( int i=0 ; i<n_layers-2 ; i++ )
        {
            // down propagation
            params[i]->setAsUpInput( layers[i+1]->sample );
            layers[i]->getAllActivations( (RBMLLParameters*) params[i] );

            // negative phase
            layers[i]->generateSample();
            params[i]->setAsDownInput( layers[i]->sample );
            layers[i+1]->getAllActivations( (RBMLLParameters*) params[i] );
            layers[i+1]->computeExpectation();
            params[i]->accumulateNegStats( layers[i]->sample,
                                           layers[i+1]->expectation );
            params[i]->update();
        }
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

        //update
        joint_params->update();

        train_stats->update( train_costs );
    }
    train_stats->finalize();
    MODULE_LOG << endl;
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
