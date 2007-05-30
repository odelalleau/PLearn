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

#define PL_LOG_MODULE_NAME "RBMModule"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMModule,
    "A Restricted Boltzmann Machine.",
    "An RBM contains a 'visible_layer', a 'hidden_layer' (both instances of a subclass\n"
    "of RBMLayer) and a 'connection' (an instance of a subclass of RBMConnection).\n"
    "It always has the following ports: \n"
    "  - 'visible' : expectations of the visible (normally input) layer\n"
    "  - 'hidden.state' : expectations of the hidden (normally output) layer\n"
    "  - 'hidden_activations.state' : activations of hidden units (given visible)\n"
    "  - 'visible_sample' : random sample obtained on visible units\n"
    "  - 'hidden_sample' : random sample obtained on hidden units\n"
    "  - 'energy' : energy of the joint (visible,hidden) pair or free-energy\n"
    "               of the visible (if given) or of the hidden (if given).\n"
    "  - 'hidden_bias' : externally controlled bias on the hidden units,\n"
    "                    used to implement conditional RBMs\n"
    "  - 'neg_log_likelihood' : USE WITH CARE, this is the exact negative log-likelihood\n"
    "    of the RBM. Computing it requires re-computing the partition function (which must\n"
    "    be recomputed if the parameters have changed) and takes O(2^{min(n_hidden,n_visible)})\n"
    "    computations of the free-energy.\n"
    "An RBM also has other ports that exist only if some options are set.\n"
    "If reconstruction_connection is given, then it has\n"
    "  - 'visible_reconstruction_activations.state' : the deterministic reconstruction of the\n"
    "     visible activations through the conditional expectations of the hidden given the visible.\n"
    "  - 'visible_reconstruction.state' : the deterministic reconstruction of the visible\n"
    "     values (expectations) through the conditional expectations of hidden | visible.\n"
    "  - 'reconstruction_error.state' : the auto-associator reconstruction error (NLL)\n"
    "    obtained by matching the visible_reconstruction with the given visible.\n"
    "If compute_contrastive_divergence is true, then the RBM also has these ports\n"
    "  - 'contrastive_divergence' : the quantity minimized by contrastive-divergence training.\n"
    "  - 'negative_phase_visible_samples.state' : the negative phase stochastic reconstruction\n"
    "    of the visible units, only provided to avoid recomputing them in bpropUpdate.\n"
    "  - 'negative_phase_hidden_expectations.state' : the negative phase hidden units\n"
    "    expected values, only provided to avoid recomputing them in bpropUpdate.\n"
    "\n"
    "The RBM can be trained by gradient descent (wrt to gradients provided on\n"
    "the 'hidden.state' port or on the 'reconstruction_error.state' port)\n"
    "if grad_learning_rate>0 or by contrastive divergence, if cd_learning_rate>0.\n"
);

///////////////
// RBMModule //
///////////////
RBMModule::RBMModule():
    cd_learning_rate(0),
    grad_learning_rate(0),
    compute_contrastive_divergence(false),
    n_Gibbs_steps_CD(1),
    min_n_Gibbs_steps(1),
    n_Gibbs_steps_per_generated_sample(1),
    compute_log_likelihood(false),
    Gibbs_step(0),
    log_partition_function(0),
    partition_function_is_stale(true),
    hidden_bias(NULL),
    weights(NULL),
    hidden_act(NULL),
    hidden_activations_are_computed(false)
{
}

////////////////////
// declareOptions //
////////////////////
void RBMModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "visible_layer", &RBMModule::visible_layer,
                  OptionBase::buildoption,
        "Visible layer of the RBM.");

    declareOption(ol, "hidden_layer", &RBMModule::hidden_layer,
                  OptionBase::buildoption,
        "Hidden layer of the RBM.");

    declareOption(ol, "connection", &RBMModule::connection,
                  OptionBase::buildoption,
        "Connection between the visible and hidden layers.");

    declareOption(ol, "reconstruction_connection", 
                  &RBMModule::reconstruction_connection,
                  OptionBase::buildoption,
        "Reconstuction connection between the hidden and visible layers.");

    declareOption(ol, "grad_learning_rate", &RBMModule::grad_learning_rate,
                  OptionBase::buildoption,
        "Learning rate for the gradient descent step.");

    declareOption(ol, "cd_learning_rate", &RBMModule::cd_learning_rate,
                  OptionBase::buildoption,
        "Learning rate for the constrastive divergence step.");

    declareOption(ol, "compute_contrastive_divergence", &RBMModule::compute_contrastive_divergence,
                  OptionBase::buildoption,
        "Compute the constrastive divergence in an output port.");

    declareOption(ol, "n_Gibbs_steps_CD", 
                  &RBMModule::n_Gibbs_steps_CD,
                  OptionBase::buildoption,
                  "Number of Gibbs sampling steps in negative phase of "
                  "contrastive divergence.");

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

    declareOption(ol, "compute_log_likelihood",
                  &RBMModule::compute_log_likelihood,
                  OptionBase::buildoption,
                  "Whether to compute the RBM generative model's log-likelihood\n"
                  "(on the neg_log_likelihood port). If false then the neg_log_likelihood\n"
                  "port just computes the input visible's free energy.\n");

    declareOption(ol, "Gibbs_step", 
                  &RBMModule::Gibbs_step,
                  OptionBase::learntoption,
                  "Used in generative mode (when visible_sample or hidden_sample is requested)\n"
                  "when one has to sample from the joint or a marginal of visible and hidden,\n"
                  "Keeps track of the number of steps that have been run since the beginning\n"
                  "of the chain.\n");

    declareOption(ol, "log_partition_function", 
                  &RBMModule::log_partition_function,
                  OptionBase::learntoption,
                  "log(Z) = log(sum_{h,x} exp(-energy(h,x))\n"
                  "only computed if compute_log_likelihood is true and\n"
                  "the neg_log_likelihood port is requested.\n");

    declareOption(ol, "partition_function_is_stale", 
                  &RBMModule::partition_function_is_stale,
                  OptionBase::learntoption,
                  "Whether parameters have changed since the last computation\n"
                  "of the log_partition_function (to know if it should be recomputed\n"
                  "when the neg_log_likelihood port is requested.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void RBMModule::build_()
{
    PLASSERT( cd_learning_rate >= 0 && grad_learning_rate >= 0 );
    if(visible_layer)
        visible_bias_grad.resize(visible_layer->size);

    // Forward random generator to underlying modules.
    if (random_gen) {
        if (hidden_layer && !hidden_layer->random_gen) {
            hidden_layer->random_gen = random_gen;
            hidden_layer->build();
            hidden_layer->forget();
        }
        if (visible_layer && !visible_layer->random_gen) {
            visible_layer->random_gen = random_gen;
            visible_layer->build();
            visible_layer->forget();
        }
        if (connection && !connection->random_gen) {
            connection->random_gen = random_gen;
            connection->build();
            connection->forget();
        }
        if (reconstruction_connection &&
                !reconstruction_connection->random_gen) {
            reconstruction_connection->random_gen = random_gen;
            reconstruction_connection->build();
            reconstruction_connection->forget();
        }
    }

    // buid ports and port_sizes

    ports.resize(0);
    portname_to_index.clear();
    addPortName("visible");
    addPortName("hidden.state");
    addPortName("hidden_activations.state");
    addPortName("visible_sample");
    addPortName("hidden_sample");
    addPortName("energy");
    addPortName("hidden_bias"); 
    addPortName("weights"); 
    addPortName("neg_log_likelihood");
    if(reconstruction_connection)
    {
        addPortName("visible_reconstruction.state");
        addPortName("visible_reconstruction_activations.state");
        addPortName("reconstruction_error.state");
    }
    if (compute_contrastive_divergence)
    {
        addPortName("contrastive_divergence");
        addPortName("negative_phase_visible_samples.state");
        addPortName("negative_phase_hidden_expectations.state");
    }

    port_sizes.resize(nPorts(), 2);
    port_sizes.fill(-1);
    if (visible_layer) {
        port_sizes(getPortIndex("visible"), 1) = visible_layer->size;
        port_sizes(getPortIndex("visible_sample"), 1) = visible_layer->size;
    }
    if (hidden_layer) {
        port_sizes(getPortIndex("hidden.state"), 1) = hidden_layer->size;
        port_sizes(getPortIndex("hidden_activations.state"), 1) = hidden_layer->size; 
        port_sizes(getPortIndex("hidden_sample"), 1) = hidden_layer->size; 
        port_sizes(getPortIndex("hidden_bias"),1) = hidden_layer->size;
        if(visible_layer)
            port_sizes(getPortIndex("weights"),1) = hidden_layer->size * visible_layer->size;
    }
    port_sizes(getPortIndex("energy"),1) = 1;
    port_sizes(getPortIndex("neg_log_likelihood"),1) = 1;
    if(reconstruction_connection)
    {
        if (visible_layer) {
            port_sizes(getPortIndex("visible_reconstruction.state"),1) = 
                visible_layer->size; 
            port_sizes(getPortIndex("visible_reconstruction_activations.state"),1) = 
                       visible_layer->size; 
        }
        port_sizes(getPortIndex("reconstruction_error.state"),1) = 1; 
    }
    if (compute_contrastive_divergence)
    {
        port_sizes(getPortIndex("contrastive_divergence"),1) = 1; 
        if (visible_layer) 
            port_sizes(getPortIndex("negative_phase_visible_samples.state"),1) = visible_layer->size; 
        if (hidden_layer)
            port_sizes(getPortIndex("negative_phase_hidden_expectations.state"),1) = hidden_layer->size; 
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

/////////////////
// addPortName //
/////////////////
void RBMModule::addPortName(const string& name)
{
    PLASSERT( portname_to_index.find(name) == portname_to_index.end() );
    portname_to_index[name] = ports.length();
    ports.append(name);
}

///////////////////
// computeEnergy //
///////////////////
void RBMModule::computeEnergy(const Mat& visible, const Mat& hidden,
                              Mat& energy, bool positive_phase)
{
    int mbs=hidden.length();
    energy.resize(mbs, 1);
    Mat* hidden_activations = NULL;
    if (positive_phase) {
        computePositivePhaseHiddenActivations(visible);
        hidden_activations = hidden_act;
    } else {
        computeHiddenActivations(visible);
        hidden_activations = & hidden_layer->activations;
    }
    PLASSERT( hidden_activations );
    for (int i=0;i<mbs;i++)
        energy(i,0) = visible_layer->energy(visible(i)) + 
            hidden_layer->energy(hidden(i)) + 
            dot(hidden(i), (*hidden_activations)(i));
}

///////////////////////////////
// computeFreeEnergyOfHidden //
///////////////////////////////
void RBMModule::computeFreeEnergyOfHidden(const Mat& hidden, Mat& energy)
{
    int mbs=hidden.length();
    if (energy.isEmpty())
        energy.resize(mbs,1);
    else {
        PLASSERT( energy.length() == mbs && energy.width() == 1 );
    }
    PLASSERT(visible_layer->classname()=="RBMBinomialLayer");
    computeVisibleActivations(hidden, false);
    for (int i=0;i<mbs;i++)
    {
        energy(i,0) = hidden_layer->energy(hidden(i));
        for (int j=0;j<visible_layer->size;j++)
            energy(i,0) += softplus(visible_layer->activations(i,j));
    }
}

////////////////////////////////
// computeFreeEnergyOfVisible //
////////////////////////////////
void RBMModule::computeFreeEnergyOfVisible(const Mat& visible, Mat& energy,
                                           bool positive_phase)
{
    int mbs=visible.length();
    if (energy.isEmpty())
        energy.resize(mbs,1);
    else {
        PLASSERT( energy.length() == mbs && energy.width() == 1 );
    }
    PLASSERT(hidden_layer->classname()=="RBMBinomialLayer");
    Mat* hidden_activations = NULL;
    if (positive_phase) {
        computePositivePhaseHiddenActivations(visible);
        hidden_activations = hidden_act;
    }
    else {
        computeHiddenActivations(visible);
        hidden_activations = & hidden_layer->activations;
    }
    PLASSERT( hidden_activations && hidden_activations->length() == mbs
            && hidden_activations->width() == hidden_layer->size );
    for (int i=0;i<mbs;i++)
    {
        energy(i,0) = visible_layer->energy(visible(i));
        for (int j=0;j<hidden_layer->size;j++)
            energy(i,0) += softplus((*hidden_activations)(i,j));
    }
}

//////////////////////////////
// computeHiddenActivations //
//////////////////////////////
void RBMModule::computeHiddenActivations(const Mat& visible)
{
    if(weights && !weights->isEmpty())
    {
        Mat old_weights;
        Vec old_activation;
        connection->getAllWeights(old_weights);
        old_activation = hidden_layer->activation;
        int up = connection->up_size;
        int down = connection->down_size;
        PLASSERT( weights->width() == up * down  );
        for(int i=0; i<visible.length(); i++)
        {
            connection->setAllWeights(Mat(up, down, (*weights)(i)));
            connection->setAsDownInput(visible(i));
            hidden_layer->activation = hidden_layer->activations(i);
            hidden_layer->getAllActivations(connection, 0, false);
            if (hidden_bias && !hidden_bias->isEmpty())
                hidden_layer->activation += (*hidden_bias)(i);
        }
        connection->setAllWeights(old_weights);
        hidden_layer->activation = old_activation;
    }
    else
    {
        connection->setAsDownInputs(visible);
        hidden_layer->getAllActivations(connection, 0, true);
        if (hidden_bias && !hidden_bias->isEmpty())
            hidden_layer->activations += *hidden_bias;
    }
}

///////////////////////////////////////////
// computePositivePhaseHiddenActivations //
///////////////////////////////////////////
void RBMModule::computePositivePhaseHiddenActivations(const Mat& visible)
{
    if (hidden_activations_are_computed) {
        // Nothing to do.
        PLASSERT( !hidden_act || !hidden_act->isEmpty() );
        return;
    }
    computeHiddenActivations(visible);
    if (hidden_act && hidden_act->isEmpty())
    {
        hidden_act->resize(visible.length(),hidden_layer->size);
        *hidden_act << hidden_layer->activations;
    }
    hidden_activations_are_computed = true;
}

///////////////////////////////
// computeVisibleActivations //
///////////////////////////////
void RBMModule::computeVisibleActivations(const Mat& hidden,
                                          bool using_reconstruction_connection)
{
    if (using_reconstruction_connection)
    {
        PLASSERT( reconstruction_connection );
        reconstruction_connection->setAsDownInputs(hidden);
        visible_layer->getAllActivations(reconstruction_connection, 0, true);
    }
    else
    {
        if(weights && !weights->isEmpty())
        {
            Mat old_weights;
            Vec old_activation;
            connection->getAllWeights(old_weights);
            old_activation = visible_layer->activation;
            int up = connection->up_size;
            int down = connection->down_size;
            PLASSERT( weights->width() == up * down  );
            for(int i=0; i<hidden.length(); i++)
            {
                connection->setAllWeights(Mat(up,down,(*weights)(i)));
                connection->setAsUpInput(hidden(i));
                visible_layer->activation = visible_layer->activations(i);
                visible_layer->getAllActivations(connection, 0, false);
            }
            connection->setAllWeights(old_weights);
            visible_layer->activation = old_activation;
        }
        else
        {
            connection->setAsUpInputs(hidden);
            visible_layer->getAllActivations(connection, 0, true);
        }
    }
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
    deepCopyField(reconstruction_connection, copies);

    deepCopyField(hidden_exp_grad, copies);
    deepCopyField(hidden_act_grad, copies);
    deepCopyField(visible_exp_grad, copies);
    deepCopyField(visible_act_grad, copies);
    deepCopyField(visible_bias_grad, copies);
    deepCopyField(hidden_exp_store, copies);
    deepCopyField(hidden_act_store, copies);

    deepCopyField(ports, copies);
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
    PLASSERT( visible_layer );
    PLASSERT( hidden_layer );
    PLASSERT( connection );
    Mat* visible = ports_value[getPortIndex("visible")]; 
    Mat* hidden = ports_value[getPortIndex("hidden.state")];
    hidden_act = ports_value[getPortIndex("hidden_activations.state")];
    Mat* visible_sample = ports_value[getPortIndex("visible_sample")];
    Mat* hidden_sample = ports_value[getPortIndex("hidden_sample")];
    Mat* energy = ports_value[getPortIndex("energy")];
    Mat* neg_log_likelihood = ports_value[getPortIndex("neg_log_likelihood")];
    hidden_bias = ports_value[getPortIndex("hidden_bias")];
    weights = ports_value[getPortIndex("weights")];
    Mat* visible_reconstruction = 0;
    Mat* visible_reconstruction_activations = 0;
    Mat* reconstruction_error = 0;
    if(reconstruction_connection)
    {
        visible_reconstruction = 
            ports_value[getPortIndex("visible_reconstruction.state")]; 
        visible_reconstruction_activations = 
            ports_value[getPortIndex("visible_reconstruction_activations.state")];
        reconstruction_error = 
            ports_value[getPortIndex("reconstruction_error.state")];
    }
    Mat* contrastive_divergence = 0;
    Mat* negative_phase_visible_samples = 0;
    Mat* negative_phase_hidden_expectations = 0;
    if (compute_contrastive_divergence)
    {
        contrastive_divergence = ports_value[getPortIndex("contrastive_divergence")]; 
        if (!contrastive_divergence || !contrastive_divergence->isEmpty())
            PLERROR("In RBMModule::fprop - When option "
                    "'compute_contrastive_divergence' is 'true', the "
                    "'contrastive_divergence' port should be provided, as an "
                    "output.");
        negative_phase_visible_samples = 
            ports_value[getPortIndex("negative_phase_visible_samples.state")];
        negative_phase_hidden_expectations = 
            ports_value[getPortIndex("negative_phase_hidden_expectations.state")];
    }

    bool hidden_expectations_are_computed = false;
    hidden_activations_are_computed = false;
    bool found_a_valid_configuration = false;

    if (visible && !visible->isEmpty())
    {
        // When an input is provided, that would restart the chain for
        // unconditional sampling, from that example.
        Gibbs_step = 0; 
        visible_layer->setExpectations(*visible);
    }

    // COMPUTE ENERGY
    if (energy) 
    {
        PLASSERT_MSG( energy->isEmpty(), 
                      "RBMModule: the energy port can only be an output port\n" );
        if (visible && !visible->isEmpty()
            && hidden && !hidden->isEmpty()) 
        {
            // FULLY OBSERVED CASE
            // we know x and h: energy(h,x) = b'x + c'h + h'Wx
            //  = visible_layer->energy(x) + hidden_layer->energy(h) + dot(h,hidden_layer->activation)
            computeEnergy(*visible,*hidden,*energy);
        } else if (visible && !visible->isEmpty())
        {
            // FREE-ENERGY(visible) CASE
            // we know x: free energy = -log sum_h e^{-energy(h,x)}
            //                        = b'x + sum_i log sigmoid(-c_i - W_i'x) .... FOR BINOMIAL HIDDEN LAYER
            //                        = visible_layer->energy(x) + sum_i log hidden_layer->expectation[i]
            // or more robustly,      = visible_layer->energy(x) - sum_i softplus(-hidden_layer->activation[i])
            computeFreeEnergyOfVisible(*visible,*energy);
        }
        else if (hidden && !hidden->isEmpty())
            // FREE-ENERGY(hidden) CASE
            // we know h: free energy = -log sum_x e^{-energy(h,x)}
            //                        = c'h + sum_i log sigmoid(-b_i - W_{.i}'h) .... FOR BINOMIAL INPUT LAYER
            //                        = hidden_layer->energy(h) + sum_i log visible_layer->expectation[i]
            // or more robustly,      = hidden_layer->energy(h) - sum_i softplus(-visible_layer->activation[i])
        {
            computeFreeEnergyOfHidden(*hidden,*energy);
        }
        else 
            PLERROR("RBMModule: unknown configuration to compute energy (currently\n"
                    "only possible if at least visible or hidden are provided).\n");
        found_a_valid_configuration = true;
    }
    if (neg_log_likelihood && neg_log_likelihood->isEmpty() && compute_log_likelihood)
    {
        if (partition_function_is_stale && !during_training)
        {
            // recompute partition function
            if (hidden_layer->size > visible_layer->size)
                // do it by log-summing minus-free-energy of visible configurations
            {
                PLASSERT(visible_layer->classname()=="RBMBinomialLayer");
                // assuming a binary input we sum over all bit configurations
                int n_configurations = 1 << visible_layer->size; // = 2^{visible_layer->size}
                Mat inputs = visible_layer->getExpectations();
                inputs.resize(1,visible_layer->size);
                Vec input = inputs(0);
                // COULD BE DONE MORE EFFICIENTLY BY DOING MANY CONFIGURATIONS
                // AT ONCE IN A 'MINIBATCH'
                Mat free_energy(1,1);
                log_partition_function = 0;
                for (int c=0;c<n_configurations;c++)
                {
                    // convert integer c into a bit-wise visible representation
                    int x=c;
                    for (int i=0;i<visible_layer->size;i++)
                    {
                        input[i]= x & 1; // take least significant bit
                        x >>= 1; // and shift right (divide by 2)
                    }
                    computeFreeEnergyOfVisible(inputs,free_energy,false);
                    if (c==0)
                        log_partition_function = -free_energy(0,0);
                    else
                        log_partition_function = logadd(log_partition_function,-free_energy(0,0));
                }
            }
            else
                // do it by summing free-energy of hidden configurations
            {
                PLASSERT(hidden_layer->classname()=="RBMBinomialLayer");
                // assuming a binary hidden we sum over all bit configurations
                int n_configurations = 1 << hidden_layer->size; // = 2^{hidden_layer->size}
                Mat& inputs = hidden_layer->getExpectations();
                inputs.resize(1,hidden_layer->size);
                Vec input = inputs(0);
                // COULD BE DONE MORE EFFICIENTLY BY DOING MANY CONFIGURATIONS
                // AT ONCE IN A 'MINIBATCH'
                Mat free_energy(1,1);
                log_partition_function = 0;
                for (int c=0;c<n_configurations;c++)
                {
                    // convert integer c into a bit-wise hidden representation
                    int x=c;
                    for (int i=0;i<hidden_layer->size;i++)
                    {
                        input[i]= x & 1; // take least significant bit
                        x >>= 1; // and shift right (divide by 2)
                    }
                    computeFreeEnergyOfHidden(inputs,free_energy);
                    if (c==0)
                        log_partition_function = -free_energy(0,0);
                    else
                        log_partition_function = logadd(log_partition_function,-free_energy(0,0));
                }
            }
            partition_function_is_stale=false;
        }
        if (visible && !visible->isEmpty()
            && hidden && !hidden->isEmpty())
        {
            // neg-log-likelihood(visible,hidden) = energy(visible,visible) + log(partition_function)
            computeEnergy(*visible,*hidden,*neg_log_likelihood);
            *neg_log_likelihood += log_partition_function;
        }
        else if (visible && !visible->isEmpty()) 
        {
            // neg-log-likelihood(visible) = free_energy(visible) + log(partition_function)
            computeFreeEnergyOfVisible(*visible,*neg_log_likelihood);
            *neg_log_likelihood += log_partition_function;
        }
        else if (hidden && !hidden->isEmpty())
        {
            // neg-log-likelihood(hidden) = free_energy(hidden) + log(partition_function)
            computeFreeEnergyOfHidden(*hidden,*neg_log_likelihood);
            *neg_log_likelihood += log_partition_function;
        }
        else PLERROR("RBMModule: neg_log_likelihood currently computable only of the visible as inputs");
    }
    // REGULAR FPROP
    // we are given the visible units and we want to compute the hidden
    // activation and/or the hidden expectation
    if ( visible && !visible->isEmpty() &&
         ((hidden && hidden->isEmpty() ) ||
          (hidden_act && hidden_act->isEmpty())) )
    {
        computePositivePhaseHiddenActivations(*visible);
        if (hidden) {
            PLASSERT( hidden->isEmpty() );
            hidden_layer->computeExpectations();
            hidden_expectations_are_computed=true;
            Mat& hidden_out = hidden_layer->getExpectations();
            hidden->resize(hidden_out.length(), hidden_out.width());
            *hidden << hidden_out;
        }
        // Since we return below, the other ports must be unused.
        //PLASSERT( !visible_sample && !hidden_sample );
        found_a_valid_configuration = true;
    } 
    // SAMPLING
    if ((visible_sample && visible_sample->isEmpty()) // it is asked to sample the visible units
        || (hidden_sample && hidden_sample->isEmpty())) // or to sample the hidden units
    {
        PLWARNING("In RBMModule::fprop - sampling in RBMModule has not been tested");
        if (hidden && !hidden->isEmpty()) // sample visible conditionally on hidden
        {
            sampleVisibleGivenHidden(*hidden);
            visible_sample->resize(visible_layer->samples.length(),visible_layer->samples.width());
            *visible_sample << visible_layer->samples;
            Gibbs_step = 0; // that would restart the chain for unconditional sampling
        }
        else if (visible && !visible->isEmpty()) // if an input is provided, sample hidden conditionally
        {
            sampleHiddenGivenVisible(visible_layer->samples);
            Gibbs_step = 0; // that would restart the chain for unconditional sampling
        }
        else // sample unconditionally: Gibbs sample after k steps
        {
            // the visible_layer->expectations contain the "state" from which we
            // start or continue the chain
            int min_n = min(Gibbs_step+n_Gibbs_steps_per_generated_sample,
                            min_n_Gibbs_steps);
            for (;Gibbs_step<min_n;Gibbs_step++)
            {
                sampleHiddenGivenVisible(visible_layer->samples);
                sampleVisibleGivenHidden(hidden_layer->samples);
            }
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
        found_a_valid_configuration = true;
    }
    // COMPUTE CONTRASTIVE DIVERGENCE CRITERION
    if (contrastive_divergence)
    {
        PLASSERT_MSG( contrastive_divergence->isEmpty(), 
                      "RBMModule: the contrastive_divergence port can only be an output port\n" );
        if (visible && !visible->isEmpty())
        {
            int mbs = visible->length();
            Mat& hidden_expectations = hidden_layer->getExpectations();
            Mat* h=0;
            Mat* h_act=0;
            if (!hidden_activations_are_computed) // it must be because neither hidden nor hidden_act were asked
            {
                PLASSERT(!hidden_act);
                computePositivePhaseHiddenActivations(*visible);
                
                // we need to save the hidden activations somewhere
                hidden_act_store.resize(mbs,hidden_layer->size);
                hidden_act_store << hidden_layer->activations;
                h_act = &hidden_act_store;
            } else 
            {
                // hidden_act must have been computed above if they were requested on port
                PLASSERT(hidden_act && !hidden_act->isEmpty()); 
                h_act = hidden_act;
            }
            if (!hidden_expectations_are_computed) // it must be because hidden outputs were not asked
            {
                PLASSERT(!hidden);
                hidden_layer->computeExpectations();
                hidden_expectations_are_computed=true;
                // we need to save the hidden expectations somewhere
                hidden_exp_store.resize(mbs,hidden_layer->size);
                hidden_exp_store << hidden_expectations;
                h = &hidden_exp_store;
            } else
            {
                // hidden exp. must have been computed above if they were requested on port
                PLASSERT(hidden && !hidden->isEmpty());
                h = hidden;
            }
            // perform negative phase
            for( int i=0; i<n_Gibbs_steps_CD; i++)
            {
                hidden_layer->generateSamples();
                // (Negative phase) Generate visible samples.
                sampleVisibleGivenHidden(hidden_layer->samples);
                // compute corresponding hidden expectations.
                computeHiddenActivations(visible_layer->samples);
            }
            PLASSERT(negative_phase_visible_samples);
            PLASSERT(negative_phase_hidden_expectations);
            negative_phase_visible_samples->resize(mbs,visible_layer->size);
            *negative_phase_visible_samples << visible_layer->samples;
            negative_phase_hidden_expectations->resize(hidden_expectations.length(),
                                                       hidden_expectations.width());
            *negative_phase_hidden_expectations << hidden_expectations;

            // compute the energy (again for now only in the binomial case)
            PLASSERT(hidden_layer->classname()=="RBMBinomialLayer");

            // note that h_act and h may point to hidden_act_store and hidden_exp_store
            PLASSERT(h_act && !h_act->isEmpty()); 
            PLASSERT(h && !h->isEmpty());

            contrastive_divergence->resize(hidden_expectations.length(),1);
            // compute contrastive divergence itself
            for (int i=0;i<mbs;i++)
            {
                (*contrastive_divergence)(i,0) = 
                    // positive phase energy
                    visible_layer->energy((*visible)(i))
                    + hidden_layer->energy((*h)(i))
                    + dot((*h)(i),(*h_act)(i))
                    // minus
                    - 
                    // negative phase energy
                    (visible_layer->energy(visible_layer->samples(i))
                     + hidden_layer->energy(hidden_expectations(i))
                     + dot(hidden_expectations(i),hidden_layer->activations(i)));
            }
        }
        else
            PLERROR("RBMModule: unknown configuration to compute contrastive_divergence (currently\n"
                    "only possible if only visible are provided in input).\n");
        found_a_valid_configuration = true;
    }
    // COMPUTE AUTOASSOCIATOR RECONSTRUCTION ERROR
    if ( visible && !visible->isEmpty() && 
         ( ( visible_reconstruction && visible_reconstruction->isEmpty() ) || 
           ( visible_reconstruction_activations && 
             visible_reconstruction_activations->isEmpty() ) ||
           ( reconstruction_error && reconstruction_error->isEmpty() ) ) ) 
    {        
        // Autoassociator reconstruction cost
        PLASSERT( ports_value.length() == nPorts() );
        computePositivePhaseHiddenActivations(*visible); 
        if(!hidden_expectations_are_computed)
        {
            hidden_layer->computeExpectations();
            hidden_expectations_are_computed=true;
        }

        // Don't need to verify if they are asked in a port, this was done previously
        
        computeVisibleActivations(hidden_layer->getExpectations(),true);
        if(visible_reconstruction_activations) 
        {
            PLASSERT( visible_reconstruction_activations->isEmpty() );
            const Mat& to_store = visible_layer->activations;
            visible_reconstruction_activations->resize(to_store.length(), 
                                                       to_store.width());
            *visible_reconstruction_activations << to_store;
        }
        if (visible_reconstruction || reconstruction_error)
        {        
            visible_layer->computeExpectations();
            if(visible_reconstruction)
            {
                PLASSERT( visible_reconstruction->isEmpty() );
                const Mat& to_store = visible_layer->getExpectations();
                visible_reconstruction->resize(to_store.length(), 
                                                           to_store.width());
                *visible_reconstruction << to_store;
            }
            if(reconstruction_error)
            {
                PLASSERT( reconstruction_error->isEmpty() );
                reconstruction_error->resize(visible->length(),1);
                visible_layer->fpropNLL(*visible,
                                        *reconstruction_error);
            }
        }
        found_a_valid_configuration = true;
    }

    // Reset some class fields to ensure they are not reused by mistake.
    hidden_act = NULL;
    hidden_bias = NULL;
    weights = NULL;
    hidden_activations_are_computed = false;

    if (!found_a_valid_configuration)
        PLERROR("In RBMModule::fprop - Unknown port configuration");

    checkProp(ports_value);

}

////////////////////
// bpropAccUpdate //
////////////////////
void RBMModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                               const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_gradient.length() == nPorts() );
    Mat* visible_grad = ports_gradient[getPortIndex("visible")];
    Mat* hidden_grad = ports_gradient[getPortIndex("hidden.state")];
    Mat* visible = ports_value[getPortIndex("visible")];
    Mat* hidden = ports_value[getPortIndex("hidden.state")];
    hidden_act = ports_value[getPortIndex("hidden_activations.state")];
    Mat* reconstruction_error_grad = 0;
    Mat* hidden_bias_grad = ports_gradient[getPortIndex("hidden_bias")];
    weights = ports_value[getPortIndex("weights")]; 
    Mat* weights_grad = ports_gradient[getPortIndex("weights")];    

    if(reconstruction_connection)
        reconstruction_error_grad = 
            ports_gradient[getPortIndex("reconstruction_error.state")];

    // Ensure the visible gradient is not provided as input. This is because we
    // accumulate more than once in 'visible_grad'.
    PLASSERT_MSG( !visible_grad || visible_grad->isEmpty(), "Cannot provide "
            "an input gradient w.r.t. visible units" );
    
    if (visible && !visible->isEmpty() && 
        (hidden_grad && !hidden_grad->isEmpty()))
    {
        int mbs = visible->length();
        if (grad_learning_rate > 0) {
            setAllLearningRates(grad_learning_rate);
            PLASSERT( hidden && hidden_act );
            // Compute gradient w.r.t. activations of the hidden layer.
            hidden_layer->bpropUpdate(
                    *hidden_act, *hidden, hidden_act_grad, *hidden_grad,
                    false);
            if (hidden_bias_grad)
            {
                PLASSERT( hidden_bias_grad->isEmpty() &&
                          hidden_bias_grad->width() == hidden_layer->size );
                hidden_bias_grad->resize(mbs,hidden_layer->size);
                *hidden_bias_grad += hidden_act_grad;
            }
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
            store_visible_grad->resize(mbs,visible_layer->size);
            
            if (weights_grad)
            {
                int up = connection->up_size;
                int down = connection->down_size;
                PLASSERT( weights && !weights->isEmpty() &&
                          weights_grad->isEmpty() &&
                          weights_grad->width() == up * down );
                weights_grad->resize(mbs, up * down);
                Mat w, wg;
                Vec v,h,vg,hg;
                for(int i=0; i<mbs; i++)
                {
                    w = Mat(up, down,(*weights)(i));
                    wg = Mat(up, down,(*weights_grad)(i));
                    v = (*visible)(i);
                    h = (*hidden_act)(i);
                    vg = (*store_visible_grad)(i);
                    hg = hidden_act_grad(i);
                    connection->petiteCulotteOlivierUpdate(
                        v,
                        w,
                        h,
                        vg,
                        wg,
                        hg,true);
                }
            }
            else
            {
                connection->bpropUpdate(
                    *visible, *hidden_act, *store_visible_grad,
                    hidden_act_grad, true);
            }
            partition_function_is_stale = true;
        }
    } 

    if (cd_learning_rate > 0) {
        EXTREME_MODULE_LOG << "Performing contrastive divergence step in RBM '"
                           << name << "'" << endl;
        // Perform a step of contrastive divergence.
        PLASSERT( visible && !visible->isEmpty() );
            int mbs = visible->length();
            setAllLearningRates(cd_learning_rate);
            PLASSERT( ports_value.length() == nPorts() );
            Mat* negative_phase_visible_samples = 
                compute_contrastive_divergence?ports_value[getPortIndex("negative_phase_visible_samples.state")]:0;
            Mat* negative_phase_hidden_expectations = 
                compute_contrastive_divergence?ports_value[getPortIndex("negative_phase_hidden_expectations.state")]:0;
            PLASSERT( visible && hidden );
            if (!negative_phase_visible_samples || negative_phase_visible_samples->isEmpty())
            {
                // Generate hidden samples.
                hidden_layer->setExpectations(*hidden);
                for( int i=0; i<n_Gibbs_steps_CD; i++)
                {
                    hidden_layer->generateSamples();
                    // (Negative phase) Generate visible samples.
                    sampleVisibleGivenHidden(hidden_layer->samples);
                    // compute corresponding hidden expectations.
                    computeHiddenActivations(visible_layer->samples);
                    hidden_layer->computeExpectations();
                }
                if (!negative_phase_hidden_expectations)
                    negative_phase_hidden_expectations = &(hidden_layer->getExpectations());
                else
                {
                    PLASSERT(negative_phase_hidden_expectations->isEmpty());
                    negative_phase_hidden_expectations->resize(mbs,hidden_layer->size);
                    *negative_phase_hidden_expectations << hidden_layer->getExpectations();
                }
                if (!negative_phase_visible_samples)
                    negative_phase_visible_samples = &(visible_layer->samples);
                else
                {
                    PLASSERT(negative_phase_visible_samples->isEmpty());
                    negative_phase_visible_samples->resize(mbs,visible_layer->size);
                    *negative_phase_visible_samples << visible_layer->samples;
                }
            }
            // Perform update.
            visible_layer->update(*visible, *negative_phase_visible_samples);
            if (weights_grad)
            {
                int up = connection->up_size;
                int down = connection->down_size;
                PLASSERT( weights && !weights->isEmpty() &&
                          weights_grad->isEmpty() &&
                          weights_grad->width() == up * down );
                weights_grad->resize(mbs, up * down);
                    
                Mat wg;
                Vec vp, hp, vn, hn;
                for(int i=0; i<mbs; i++)
                {
                    vp = (*visible)(i);
                    hp = (*hidden)(i);
                    vn = (*negative_phase_visible_samples)(i);
                    hn = (*negative_phase_hidden_expectations)(i);
                    wg = Mat(up, down,(*weights_grad)(i));
                    connection->petiteCulotteOlivierCD(
                        vp, hp,
                        vn,
                        hn,
                        wg,
                        true);
                }
            }
            else
            {
                connection->update(*visible, *hidden,
                                   *negative_phase_visible_samples,
                                   *negative_phase_hidden_expectations);
            }
            hidden_layer->update(*hidden, *negative_phase_hidden_expectations);
            if (hidden_bias_grad)
            {
                if (hidden_bias_grad->isEmpty()) {
                    PLASSERT(hidden_bias_grad->width() == hidden_layer->size);
                    hidden_bias_grad->resize(mbs,hidden_layer->size);
                }
                // d(contrastive_divergence)/dhidden_bias =
                //     hidden - negative_phase_hidden_expectations
                *hidden_bias_grad += *hidden;
                *hidden_bias_grad -= *negative_phase_hidden_expectations;
            }
            partition_function_is_stale = true;
    }

    if (reconstruction_error_grad && !reconstruction_error_grad->isEmpty()) {
        setAllLearningRates(grad_learning_rate);
        PLASSERT( reconstruction_connection != 0 );
        // Perform gradient descent on Autoassociator reconstruction cost
        PLASSERT( ports_value.length() == nPorts() );
        Mat* visible_reconstruction = ports_value[getPortIndex("visible_reconstruction.state")];
        Mat* visible_reconstruction_activations = ports_value[getPortIndex("visible_reconstruction_activations.state")];
        Mat* reconstruction_error = ports_value[getPortIndex("reconstruction_error.state")];
        PLASSERT( hidden != 0 );
        PLASSERT( visible  && hidden_act &&
                  visible_reconstruction && visible_reconstruction_activations &&
                  reconstruction_error);
        int mbs = reconstruction_error_grad->length();

        PLCHECK_MSG( weights, "In RBMModule::bpropAccUpdate(): reconstruction cost "
                     "for conditional weights is not implemented");

        // Backprop reconstruction gradient

        // Must change visible_layer's expectation
        visible_layer->getExpectations() << *visible_reconstruction;
        visible_layer->bpropNLL(*visible,*reconstruction_error,
                                visible_act_grad);

        // Combine with incoming gradient
        PLASSERT( (*reconstruction_error_grad).width() == 1 );
        for (int t=0;t<mbs;t++)
            visible_act_grad(t) *= (*reconstruction_error_grad)(t,0);

        // Visible bias update
        columnSum(visible_act_grad,visible_bias_grad);
        visible_layer->update(visible_bias_grad);

        // Reconstruction connection update
        reconstruction_connection->bpropUpdate(
            *hidden, *visible_reconstruction_activations,
            hidden_exp_grad, visible_act_grad, false);
        
        // Hidden layer bias update
        hidden_layer->bpropUpdate(*hidden_act,
                                  *hidden, hidden_act_grad,
                                  hidden_exp_grad, false);
        if (hidden_bias_grad)
        {
            if (hidden_bias_grad->isEmpty()) {
                PLASSERT( hidden_bias_grad->width() == hidden_layer->size );
                hidden_bias_grad->resize(mbs,hidden_layer->size);
            }
            *hidden_bias_grad += hidden_act_grad;
        }
        // Connection update
        if(visible_grad)
        {
            // The length of 'visible_grad' must be either 0 (if not computed
            // previously) or the size of the mini-batches (otherwise).
            PLASSERT( visible_grad->width() == visible_layer->size &&
                      visible_grad->length() == 0 ||
                      visible_grad->length() == mbs );
            visible_grad->resize(mbs, visible_grad->width());
            connection->bpropUpdate(
                *visible, *hidden_act,
                *visible_grad, hidden_act_grad, true);
        }
        else
        {
            visible_exp_grad.resize(mbs,visible_layer->size);        
            connection->bpropUpdate(
                *visible, *hidden_act,
                visible_exp_grad, hidden_act_grad, true);
        }
        partition_function_is_stale = true;
    }

    checkProp(ports_gradient);

    // Reset pointers to ensure we do not reuse them by mistake.
    hidden_act = NULL;
    weights = NULL;
}

////////////
// forget //
////////////
void RBMModule::forget()
{
    DBG_MODULE_LOG << "Forgetting RBMModule '" << name << "'" << endl;
    PLASSERT( hidden_layer && visible_layer && connection );
    hidden_layer->forget();
    visible_layer->forget();
    connection->forget();
    if (reconstruction_connection)
        reconstruction_connection->forget();
}

//////////////////
// getPortIndex //
//////////////////
int RBMModule::getPortIndex(const string& port)
{
    map<string, int>::const_iterator it = portname_to_index.find(port);
    if (it == portname_to_index.end())
        return -1;
    else
        return it->second;
}

//////////////
// getPorts //
//////////////
const TVec<string>& RBMModule::getPorts()
{
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
    if(reconstruction_connection)
        reconstruction_connection->setLearningRate(lr);
}

//////////////////////////////
// sampleHiddenGivenVisible //
//////////////////////////////
void RBMModule::sampleHiddenGivenVisible(const Mat& visible)
{
    computeHiddenActivations(visible);
    hidden_layer->generateSamples();
}

//////////////////////////////
// sampleVisibleGivenHidden //
//////////////////////////////
void RBMModule::sampleVisibleGivenHidden(const Mat& hidden)
{
    computeVisibleActivations(hidden);
    visible_layer->generateSamples();
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
