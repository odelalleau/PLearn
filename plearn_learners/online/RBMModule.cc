// -*- C++ -*-

// RBMModule.cc
//
// Copyright (C) 2007 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file RBMModule.cc */



#include "RBMModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMModule,
    "A Restricted Boltzmann Machine.",
    "An RBM contains a 'visible_layer', a 'hidden_layer' (both instances of a subclass\n"
    "of RBMLayer) and a 'connection' (an instance of a subclass of RBMConnection).\n"
    "It has two ports: the 'visible' port and the 'hidden' port.\n"
    "The RBM can be trained by gradient descent (wrt to gradients provided on\n"
    "the 'hidden' port) or by contrastive divergence.\n"
);

///////////////
// RBMModule //
///////////////
RBMModule::RBMModule():
    cd_learning_rate(0),
    grad_learning_rate(0)
{
}

void RBMModule::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "visible_layer", &RBMModule::visible_layer,
                  OptionBase::buildoption,
        "Visible layer of the RBM.");

    declareOption(ol, "hidden_layer", &RBMModule::hidden_layer,
                  OptionBase::buildoption,
        "Hidden layer of the RBM.");

    declareOption(ol, "connection", &RBMModule::connection,
                  OptionBase::buildoption,
        "Connection between the visible and hidden layers.");

    declareOption(ol, "grad_learning_rate", &RBMModule::grad_learning_rate,
                  OptionBase::buildoption,
        "Learning rate for the gradient descent step.");

    declareOption(ol, "cd_learning_rate", &RBMModule::cd_learning_rate,
                  OptionBase::buildoption,
        "Learning rate for the constrastive divergence step.");

    declareOption(ol, "min_n_Gibbs_steps", &RBMModule::min_n_Gibbs_steps,
                  OptionBase::buildoption,
                  "Used in generative mode (when visible_sample or hidden_sample is requested)\n"
                  "when one has to sample from the joint or a marginal of visible and hidden,\n"
                  "and thus a Gibbs chain has to be run. This option gives the minimum number\n"
                  "of Gibbs steps to perform in the chain before outputting a sample.\n");

    declareOption(ol, "n_Gibbs_steps_per_generated_sample", 
                  &RBMModule::n_Gibbs_steps_per_generated_sample,
                  OptionBase::buildoption,
                  "Used in generative mode (when visible_sample or hidden_sample is requested)\n"
                  "when one has to sample from the joint or a marginal of visible and hidden,\n"
                  "This option gives the number of steps to run in the Gibbs chain between\n"
                  "consecutive generated samples that are produced in output of the fprop method.\n"
                  "By default this is equal to min_n_Gibbs_steps.\n");

    declareOption(ol, "Gibbs_step", 
                  &RBMModule::Gibbs_step,
                  OptionBase::learntoption,
                  "Used in generative mode (when visible_sample or hidden_sample is requested)\n"
                  "when one has to sample from the joint or a marginal of visible and hidden,\n"
                  "Keeps track of the number of steps that have been ran since the beginning\n"
                  "of the chain.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////////////
// bpropAccUpdate //
////////////////////
void RBMModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                               const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_gradient.length() == nPorts() );
    Mat* visible_grad = ports_gradient[0];
    Mat* hidden_grad = ports_gradient[1];
    if (hidden_grad && !hidden_grad->isEmpty() &&
            (!visible_grad || visible_grad->isEmpty()))
    {
        if (grad_learning_rate > 0) {
            setAllLearningRates(grad_learning_rate);
            Mat* hidden_out = ports_value[1];
            Mat* hidden_act = ports_value[2];
            PLASSERT( hidden_out && hidden_act );
            // Compute gradient w.r.t. activations of the hidden layer.
            hidden_layer->bpropUpdate(
                    *hidden_act, *hidden_out, hidden_act_grad, *hidden_grad,
                    false);
            Mat* visible_out = ports_value[0];
            // Compute gradient w.r.t. expectations of the visible layer (=
            // inputs).
            Mat* store_visible_grad = NULL;
            if (visible_grad) {
                PLASSERT( visible_grad->width() == visible_layer->size );
                store_visible_grad = visible_grad;
            } else {
                // We do not actually need to store the gradient, but since it
                // is required in bpropUpdate, we provide a dummy matrix to
                // store it.
                store_visible_grad = &visible_exp_grad;
            }
            store_visible_grad->resize(hidden_grad->length(),
                                       visible_layer->size);
            connection->bpropUpdate(
                    *visible_out, *hidden_act, *store_visible_grad,
                    hidden_act_grad, true);
        }
        if (cd_learning_rate > 0) {
            setAllLearningRates(cd_learning_rate);
            // Perform a step of contrastive divergence.
            PLASSERT( ports_value.length() == nPorts() );
            Mat* visible_exp = ports_value[0];
            Mat* hidden_exp = ports_value[1];
            PLASSERT( visible_exp && hidden_exp );
            // Generate hidden samples.
            hidden_layer->setExpectations(*hidden_exp);
            hidden_layer->generateSamples();
            // Generate visible samples.
            connection->setAsUpInputs(hidden_layer->samples);
            visible_layer->getAllActivations(connection, 0, true);
            visible_layer->generateSamples();
            // (Negative phase) compute corresponding hidden expectations.
            connection->setAsDownInputs(visible_layer->samples);
            hidden_layer->getAllActivations(connection, 0, true);
            hidden_layer->computeExpectations();
            // Perform update.
            visible_layer->update(*visible_exp, visible_layer->samples);
            connection->update(*visible_exp, *hidden_exp,
                               visible_layer->samples,
                               hidden_layer->getExpectations());
            hidden_layer->update(*hidden_exp, hidden_layer->getExpectations());
        }
    } else
        PLERROR("In RBMModule::bpropAccUpdate - Only hidden -> visible "
                "back propagation is currently implemented");
}

////////////
// build_ //
////////////
void RBMModule::build_()
{
    PLASSERT( cd_learning_rate >= 0 && grad_learning_rate >= 0 );
    if (fast_exact_is_equal(cd_learning_rate, 0) &&
        fast_exact_is_equal(grad_learning_rate, 0) )
        PLWARNING("In RBMModule::build_ - Both 'cd_learning_rate' and "
                "'grad_learning_rate' are set to 0, the RBM will not learn "
                "much");

    // buid port_sizes
    port_sizes.resize(nPorts(), 2);
    port_sizes.fill(-1);
    if (visible_layer) {
        port_sizes(0, 1) = visible_layer->size;
        port_sizes(3, 1) = visible_layer->size;
    }
    if (hidden_layer) {
        port_sizes(1, 1) = hidden_layer->size;
        port_sizes(2, 1) = hidden_layer->size;
        port_sizes(4, 1) = hidden_layer->size;
    }
}

///////////
// build //
///////////
void RBMModule::build()
{
    inherited::build();
    build_();
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RBMModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(hidden_layer,     copies);
    deepCopyField(visible_layer,    copies);
    deepCopyField(connection,       copies);
    deepCopyField(hidden_act_grad,  copies);
    deepCopyField(visible_exp_grad, copies);
}

///////////
// fprop //
///////////
void RBMModule::fprop(const Vec& input, Vec& output) const
{
    PLERROR("In RBMModule::fprop - Not implemented");
}

void RBMModule::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == nPorts() );
    Mat* visible = ports_value[0];
    Mat* hidden = ports_value[1];
    Mat* hidden_act = ports_value[2];
    Mat* visible_sample = ports_value[3];
    Mat* hidden_sample = ports_value[4];

    // we are given the visible units and we want to compute the hidden
    // activation and/or the hidden expectation
    if ( visible && !visible->isEmpty() &&
         ((hidden && hidden->isEmpty() ) ||
          (hidden_act && hidden_act->isEmpty())) )
    {
        // visible_layer->setExpectations(*visible);
        connection->setAsDownInputs(*visible); // visible_layer->getExpectations());
        hidden_layer->getAllActivations(connection, 0, true);
        if (hidden_act) {
            // Also store hidden layer activations.
            PLASSERT( hidden_act->isEmpty() );
            const Mat& to_store = hidden_layer->activations;
            hidden_act->resize(to_store.length(), to_store.width());
            *hidden_act << to_store;
        }
        if (hidden) {
            PLASSERT( hidden->isEmpty() );
            hidden_layer->computeExpectations();
            Mat& hidden_out = hidden_layer->getExpectations();
            hidden->resize(hidden_out.length(), hidden_out.width());
            *hidden << hidden_out;
        }
        // Since we return below, the other ports must be unused.
        PLASSERT( !visible_sample && !hidden_sample );
        return;
    } 
    if ((visible_sample && visible_sample->isEmpty()) // it is asked to sample the visible units
        || (hidden_sample && hidden_sample->isEmpty())) // or to sample the hidden units
    {
        if (hidden && !hidden->isEmpty()) // sample visible conditionally on hidden
        {
            connection->setAsUpInputs(*hidden);
            visible_layer->getAllActivations(connection, 0, true);
            visible_layer->generateSamples();
            visible_sample->resize(visible_layer->samples.length(),visible_layer->samples.width());
            *visible_sample << visible_layer->samples;
        }
        else // sample unconditionally: Gibbs sample after k steps
        {
            if (visible && !visible->isEmpty()) // if an input is provided, start the chain from it
            {
                Gibbs_step = 0;
                visible_layer->samples << *visible;
            }
            // else we might want to restore as a separate "state" the last visible_layer->samples,
            // (copied elsewhere) in case the RBM is used to something else than sampling 
            // in between calls to this fprop-for-sampling.
            int min_n = min(Gibbs_step+n_Gibbs_steps_per_generated_sample,
                            min_n_Gibbs_steps);
            for (;Gibbs_step<min_n;Gibbs_step++)
            {
                connection->setAsDownInputs(visible_layer->samples);
                hidden_layer->getAllActivations(connection, 0, true);
                hidden_layer->generateSamples();
                connection->setAsUpInputs(hidden_layer->samples);
                visible_layer->getAllActivations(connection, 0, true);
                visible_layer->generateSamples();
            }
            if (visible_sample && visible_sample->isEmpty()) // provide sample of the visible units
            {
                visible_sample->resize(visible_layer->samples.length(),
                                       visible_layer->samples.width());
                *visible_sample << visible_layer->samples;
            }
            if (hidden_sample && hidden_sample->isEmpty()) // provide sample of the hidden units
            {
                hidden_sample->resize(hidden_layer->samples.length(),
                                      hidden_layer->samples.width());
                *hidden_sample << hidden_layer->samples;
            }
        }        
    } 
    // Remark: the code above probably has not been tested, since the PLERROR
    // will systematically be reached.
    PLERROR("In RBMModule::fprop - Unknown port configuration");
}

////////////
// forget //
////////////
void RBMModule::forget()
{
    PLASSERT( hidden_layer && visible_layer && connection );
    hidden_layer->forget();
    visible_layer->forget();
    connection->forget();
    if (hidden_layer && !hidden_layer->random_gen) {
        hidden_layer->random_gen = random_gen;
        hidden_layer->build();
    }
    if (visible_layer && !visible_layer->random_gen) {
        visible_layer->random_gen = random_gen;
        visible_layer->build();
    }
    if (connection && !connection->random_gen) {
        connection->random_gen = random_gen;
        connection->build();
    }
    hidden_layer->forget();
    visible_layer->forget();
    connection->forget();
}

//////////////
// getPorts //
//////////////
const TVec<string>& RBMModule::getPorts()
{
    static TVec<string> ports;
    if (ports.isEmpty()) {
        ports.append("visible");
        ports.append("hidden");
        ports.append("hidden_activations.state");
        ports.append("visible_sample");
        ports.append("hidden_sample");
    }
    return ports;
}

///////////////////
// getPortsSizes //
///////////////////
const TMat<int>& RBMModule::getPortSizes()
{
    return port_sizes;
}

//////////////////////
// bpropDoesNothing //
//////////////////////
/* THIS METHOD IS OPTIONAL
bool RBMModule::bpropDoesNothing()
{
}
*/

/////////////////////////
// setAllLearningRates //
/////////////////////////
void RBMModule::setAllLearningRates(real lr)
{
    hidden_layer->setLearningRate(lr);
    visible_layer->setLearningRate(lr);
    connection->setLearningRate(lr);
}

/////////////////////
// setLearningRate //
/////////////////////
void RBMModule::setLearningRate(real dynamic_learning_rate)
{
    // Out of safety, force the user to go through the two different learning
    // rate. May need to be removed if it causes unwanted crashes.
    PLERROR("In RBMModule::setLearningRate - Do not use this method, instead "
            "explicitely use 'cd_learning_rate' and 'grad_learning_rate'");
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
