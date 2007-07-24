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
#include <plearn/vmat/VMat.h>
#include <plearn_learners/online/RBMMatrixConnection.h>

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
    "  - 'visible_sample' : random sample obtained on visible units (input or output port)\n"
    "  - 'visible_expectation' : expectation of visible units (output port ONLY)\n"
    "  - 'visible_activation' : ectation of visible units (output port ONLY)\n"
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
    n_Gibbs_steps_per_generated_sample(-1),
    compute_log_likelihood(false),
    minimize_log_likelihood(false),
    Gibbs_step(0),
    log_partition_function(0),
    partition_function_is_stale(true),
    standard_cd_grad(true),
    standard_cd_bias_grad(true),
    standard_cd_weights_grad(true),
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
        "Reconstruction connection between the hidden and visible layers.");

    declareOption(ol, "grad_learning_rate", &RBMModule::grad_learning_rate,
                  OptionBase::buildoption,
        "Learning rate for the gradient descent step.");

    declareOption(ol, "cd_learning_rate", &RBMModule::cd_learning_rate,
                  OptionBase::buildoption,
        "Learning rate for the constrastive divergence step. Note that when\n"
        "set to 0, the gradient of the contrastive divergence will not be\n"
        "computed at all.");

    declareOption(ol, "compute_contrastive_divergence", &RBMModule::compute_contrastive_divergence,
                  OptionBase::buildoption,
        "Compute the constrastive divergence in an output port.");

    declareOption(ol, "standard_cd_grad",
                  &RBMModule::standard_cd_grad,
                  OptionBase::buildoption,
        "Whether to use the standard contrastive divergence gradient for\n"
        "updates, or the true gradient of the contrastive divergence. This\n"
        "affects only the gradient w.r.t. internal parameters of the layers\n"
        "and connections. Currently, this option works only with layers of\n"
        "the type 'RBMBinomialLayer', connected by a 'RBMMatrixConnection'.");

    declareOption(ol, "standard_cd_bias_grad",
                  &RBMModule::standard_cd_bias_grad,
                  OptionBase::buildoption,
        "This option is only used when biases of the hidden layer are given\n"
        "through the 'hidden_bias' port. When this is the case, the gradient\n"
        "of contrastive divergence w.r.t. these biases is either computed:\n"
        "- by the usual formula if 'standard_cd_bias_grad' is true\n"
        "- by the true gradient if 'standard_cd_bias_grad' is false.");

    declareOption(ol, "standard_cd_weights_grad",
                  &RBMModule::standard_cd_weights_grad,
                  OptionBase::buildoption,
        "This option is only used when weights of the connection are given\n"
        "through the 'weights' port. When this is the case, the gradient of\n"
        "contrastive divergence w.r.t. weights is either computed:\n"
        "- by the usual formula if 'standard_cd_weights_grad' is true\n"
        "- by the true gradient if 'standard_cd_weights_grad' is false.");

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
                  "Whether to compute the exact RBM generative model's log-likelihood\n"
                  "(on the neg_log_likelihood port). If false then the neg_log_likelihood\n"
                  "port just computes the input visible's free energy.\n");

    declareOption(ol, "minimize_log_likelihood",
                  &RBMModule::minimize_log_likelihood,
                  OptionBase::buildoption,
                  "Whether to minimize the exact RBM generative model's log-likelihood\n"
                  "i.e. take stochastic gradient steps w.r.t. the log-likelihood instead\n"
                  "of w.r.t. the contrastive divergence.\n");

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
    addPortName("visible_expectation");
    addPortName("visible_activation");
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
        addPortName("negative_phase_hidden_activations.state");
    }

    port_sizes.resize(nPorts(), 2);
    port_sizes.fill(-1);
    if (visible_layer) {
        port_sizes(getPortIndex("visible"), 1) = visible_layer->size;
        port_sizes(getPortIndex("visible_sample"), 1) = visible_layer->size;
        port_sizes(getPortIndex("visible_expectation"), 1) = visible_layer->size;
        port_sizes(getPortIndex("visible_activation"), 1) = visible_layer->size;
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
        if (fast_exact_is_equal(cd_learning_rate, 0))
            PLWARNING("In RBMModule::build_ - Contrastive divergence is "
                    "computed but 'cd_learning_rate' is set to 0: no internal "
                    "update will be performed AND no contrastive divergence "
                    "gradient will be propagated.");
    }

    PLCHECK_MSG(!(!standard_cd_grad && standard_cd_bias_grad), "You cannot "
            "compute the standard CD gradient w.r.t. external hidden bias and "
            "use the 'true' CD gradient w.r.t. internal hidden bias");

    if (n_Gibbs_steps_per_generated_sample<0)
        n_Gibbs_steps_per_generated_sample = min_n_Gibbs_steps;

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
// FULLY OBSERVED CASE
// we know x and h:
// energy(h,x) = -b'x - c'h - h'Wx
//  = visible_layer->energy(x) + hidden_layer->energy(h)
//      - dot(h, hidden_layer->activation-c)
//  = visible_layer->energy(x) - dot(h, hidden_layer->activation)
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
        energy(i,0) = visible_layer->energy(visible(i))
            - dot(hidden(i), (*hidden_activations)(i));
            // Why not: + hidden_layer->energy(hidden(i)) ?
}

///////////////////////////////
// computeFreeEnergyOfHidden //
///////////////////////////////
// FREE-ENERGY(hidden) CASE
// we know h:
// free energy = -log sum_x e^{-energy(h,x)}
//  = -c'h - sum_i log sigmoid(b_i + W_{.i}'h) .... FOR BINOMIAL INPUT LAYER
// or more robustly,
//  = hidden_layer->energy(h) - sum_i softplus(visible_layer->activation[i])
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
        if (use_fast_approximations)
            for (int j=0;j<visible_layer->size;j++)
                energy(i,0) -= tabulated_softplus(visible_layer->activations(i,j));
        else
            for (int j=0;j<visible_layer->size;j++)
                energy(i,0) -= softplus(visible_layer->activations(i,j));
    }
}

////////////////////////////////
// computeFreeEnergyOfVisible //
////////////////////////////////
// FREE-ENERGY(visible) CASE
// we know x:
// free energy = -log sum_h e^{-energy(h,x)}
//  = -b'x - sum_i log sigmoid(c_i + W_i'x) .... FOR BINOMIAL HIDDEN LAYER
// or more robustly,
//  = visible_layer->energy(x) - sum_i softplus(hidden_layer->activation[i])
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
        if (use_fast_approximations)
            for (int j=0;j<hidden_layer->size;j++)
                energy(i,0) -= tabulated_softplus((*hidden_activations)(i,j));
        else
            for (int j=0;j<hidden_layer->size;j++)
                energy(i,0) -= softplus((*hidden_activations)(i,j));
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
        hidden_layer->setBatchSize( visible.length() );
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
        reconstruction_connection->setAsUpInputs(hidden);
        visible_layer->getAllActivations(reconstruction_connection, 0, true);
    }
    else
    {
        if(weights && !weights->isEmpty())
        {
            PLASSERT( connection->classname() == "RBMMatrixConnection" );
            Mat old_weights;
            Vec old_activation;
            connection->getAllWeights(old_weights);
            old_activation = visible_layer->activation;
            int up = connection->up_size;
            int down = connection->down_size;
            PLASSERT( weights->width() == up * down  );
            visible_layer->setBatchSize( hidden.length() );
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
    deepCopyField(store_weights_grad, copies);
    deepCopyField(store_hidden_bias_grad, copies);
    deepCopyField(visible_exp_grad, copies);
    deepCopyField(visible_act_grad, copies);
    deepCopyField(visible_bias_grad, copies);
    deepCopyField(hidden_exp_store, copies);
    deepCopyField(hidden_act_store, copies);

    deepCopyField(ports, copies);
    deepCopyField(energy_inputs, copies);
}

///////////
// fprop //
///////////
void RBMModule::fprop(const Vec& input, Vec& output) const
{
    PLERROR("In RBMModule::fprop - Not implemented");
}

void RBMModule::computePartitionFunction()
{
	int hidden_configurations = hidden_layer->getConfigurationCount();
	int visible_configurations = visible_layer->getConfigurationCount();
	
	PLASSERT_MSG(hidden_configurations != RBMLayer::INFINITE_CONFIGURATIONS ||
                    visible_configurations != RBMLayer::INFINITE_CONFIGURATIONS,
		"To compute exact log-likelihood of an RBM maximum configurations of hidden "
				"or visible layer must be less than 2^31.");
	
	// Compute partition function
	if (hidden_configurations > visible_configurations)
		// do it by log-summing minus-free-energy of visible configurations
	{
		energy_inputs.resize(1, visible_layer->size);
		Vec input = energy_inputs(0);
		// COULD BE DONE MORE EFFICIENTLY BY DOING MANY CONFIGURATIONS
		// AT ONCE IN A 'MINIBATCH'
		Mat free_energy(1, 1);
		log_partition_function = 0;
		for (int c = 0; c < visible_configurations; c++)
		{
			visible_layer->getConfiguration(c, input);
			computeFreeEnergyOfVisible(energy_inputs, free_energy, false);
			if (c==0)
				log_partition_function = -free_energy(0,0);
			else
				log_partition_function = logadd(log_partition_function,-free_energy(0,0));
		}
	}
	else
		// do it by summing free-energy of hidden configurations
	{
		energy_inputs.resize(1, hidden_layer->size);
		Vec input = energy_inputs(0);
		// COULD BE DONE MORE EFFICIENTLY BY DOING MANY CONFIGURATIONS
		// AT ONCE IN A 'MINIBATCH'
		Mat free_energy(1, 1);
		log_partition_function = 0;
		for (int c = 0; c < hidden_configurations; c++)
		{
			hidden_layer->getConfiguration(c, input);
			computeFreeEnergyOfHidden(energy_inputs, free_energy);
			if (c==0)
				log_partition_function = -free_energy(0,0);
			else
				log_partition_function = logadd(log_partition_function,-free_energy(0,0));
		}
	}
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
    Mat* visible_expectation = ports_value[getPortIndex("visible_expectation")];
    Mat* visible_activation = ports_value[getPortIndex("visible_activation")];
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
    Mat* negative_phase_hidden_activations = NULL;
    if (compute_contrastive_divergence)
    {
        contrastive_divergence = ports_value[getPortIndex("contrastive_divergence")];
/* YB: I don't agree with this error message: the behavior should be adapted to the provided ports. 
      if (!contrastive_divergence || !contrastive_divergence->isEmpty())
            PLERROR("In RBMModule::fprop - When option "
                    "'compute_contrastive_divergence' is 'true', the "
                    "'contrastive_divergence' port should be provided, as an "
                    "output.");*/
        negative_phase_visible_samples =
            ports_value[getPortIndex("negative_phase_visible_samples.state")];
        negative_phase_hidden_expectations =
            ports_value[getPortIndex("negative_phase_hidden_expectations.state")];
        negative_phase_hidden_activations =
            ports_value[getPortIndex("negative_phase_hidden_activations.state")];
    }
    bool hidden_expectations_are_computed = false;
    hidden_activations_are_computed = false;
    bool found_a_valid_configuration = false;

    if (visible && !visible->isEmpty())
    {
        // When an input is provided, that would restart the chain for
        // unconditional sampling, from that example.
        Gibbs_step = 0;
        visible_layer->samples.resize(visible->length(),visible->width());
        visible_layer->samples << *visible;
    }

    // COMPUTE ENERGY
    if (energy)
    {
        PLASSERT_MSG( energy->isEmpty(),
                      "RBMModule: the energy port can only be an output port\n" );
        if (visible && !visible->isEmpty()
            && hidden && !hidden->isEmpty())
        {
            computeEnergy(*visible, *hidden, *energy);
        }
        else if (visible && !visible->isEmpty())
        {
            computeFreeEnergyOfVisible(*visible,*energy);
        }
        else if (hidden && !hidden->isEmpty())
        {
            computeFreeEnergyOfHidden(*hidden,*energy);
        }
        else
        {
            PLERROR("RBMModule: unknown configuration to compute energy (currently\n"
                    "only possible if at least visible or hidden are provided).\n");
        }
        found_a_valid_configuration = true;
    }
    // COMPUTE NLL
    if (neg_log_likelihood && neg_log_likelihood->isEmpty() && compute_log_likelihood)
    {
        if (partition_function_is_stale && !during_training)
        {
            computePartitionFunction();
            partition_function_is_stale=false;
        }
        if (visible && !visible->isEmpty()
            && hidden && !hidden->isEmpty())
        {
            // neg-log-likelihood(visible,hidden) = energy(visible,hidden) + log(partition_function)
            computeEnergy(*visible,*hidden,*neg_log_likelihood);
            *neg_log_likelihood += log_partition_function;
        }
        else if (visible && !visible->isEmpty())
        {
            // neg-log-likelihood(visible) = free_energy(visible) + log(partition_function)
            computeFreeEnergyOfVisible(*visible,*neg_log_likelihood,hidden_act);
            *neg_log_likelihood += log_partition_function;
        }
        else if (hidden && !hidden->isEmpty())
        {
            // neg-log-likelihood(hidden) = free_energy(hidden) + log(partition_function)
            computeFreeEnergyOfHidden(*hidden,*neg_log_likelihood);
            *neg_log_likelihood += log_partition_function;
        }
        else PLERROR("RBMModule: neg_log_likelihood currently computable only of the visible as inputs");
        found_a_valid_configuration = true;
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
            PLCHECK_MSG( !hidden_layer->expectations_are_up_to_date, "Safety "
                    "check: how were expectations computed previously?" );
            hidden_layer->computeExpectations();
            hidden_expectations_are_computed=true;
            const Mat& hidden_out = hidden_layer->getExpectations();
            hidden->resize(hidden_out.length(), hidden_out.width());
            *hidden << hidden_out;
        }
        // Since we return below, the other ports must be unused.
        //PLASSERT( !visible_sample && !hidden_sample );
        found_a_valid_configuration = true;
    }

    // DOWNWARD FPROP
    // we are given hidden  and we want to compute the visible or visible_activation
    if ( hidden && !hidden->isEmpty() && 
         ((visible && visible->isEmpty()) ||
          (visible_activation && visible_activation->isEmpty())) )
    {
        computeVisibleActivations(*hidden,true);
        if (visible_activation)
        {
            PLASSERT_MSG(visible_activation->isEmpty(),"visible_activation should be an output");
            visible_activation->resize(visible_layer->activations.length(),
                                       visible_layer->size);
            *visible_activation << visible_layer->activations;
        }
        if (visible)
        {
            PLASSERT_MSG(visible->isEmpty(),"visible should be an output");
            visible_layer->computeExpectations();
            const Mat expectations=visible_layer->getExpectations();
            visible->resize(expectations.length(),visible_layer->size);
            *visible_activation << visible_layer->activations;
        }
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

        Mat h;
        if (hidden && !hidden->isEmpty())
            h = *hidden;
        else {
            if(!hidden_expectations_are_computed)
            {
                computePositivePhaseHiddenActivations(*visible);
                hidden_layer->computeExpectations();
                hidden_expectations_are_computed=true;
            }
            h = hidden_layer->getExpectations();
        }

        // Don't need to verify if they are asked in a port, this was done previously

        computeVisibleActivations(h, true);
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
    // COMPUTE VISIBLE GIVEN HIDDEN
    else if ( visible_reconstruction && visible_reconstruction->isEmpty()
         && hidden && !hidden->isEmpty())
    {
        // Don't need to verify if they are asked in a port, this was done previously
        computeVisibleActivations(*hidden,true);
        if(visible_reconstruction_activations)
        {
            PLASSERT( visible_reconstruction_activations->isEmpty() );
            const Mat& to_store = visible_layer->activations;
            visible_reconstruction_activations->resize(to_store.length(),
                                                       to_store.width());
            *visible_reconstruction_activations << to_store;
        }
        visible_layer->computeExpectations();
        PLASSERT( visible_reconstruction->isEmpty() );
        const Mat& to_store = visible_layer->getExpectations();
        visible_reconstruction->resize(to_store.length(),
                                       to_store.width());
        *visible_reconstruction << to_store;
        found_a_valid_configuration = true;
    }

    // SAMPLING
    if ((visible_sample && visible_sample->isEmpty())               // is asked to sample visible units (discrete)
        || (visible_expectation && visible_expectation->isEmpty())  //              "                   (continous)
        || (hidden_sample && hidden_sample->isEmpty()))             // or to sample hidden units
    {
        if (hidden_sample && !hidden_sample->isEmpty()) // sample visible conditionally on hidden
        {
            sampleVisibleGivenHidden(*hidden_sample);
            Gibbs_step=0;
            //cout << "sampling visible from hidden" << endl;
        }
        else if (visible_sample && !visible_sample->isEmpty()) // if an input is provided, sample hidden conditionally
        {
            sampleHiddenGivenVisible(*visible_sample);
            hidden_activations_are_computed = false;
            Gibbs_step=0;
            //cout << "sampling hidden from visible" << endl;
        }
        else if (visible_expectation && !visible_expectation->isEmpty())
        {
             PLERROR("In RBMModule::fprop visible_expectation can only be an output port (use visible as input port");
        }
        else // sample unconditionally: Gibbs sample after k steps
        {
            // the visible_layer->expectations contain the "state" from which we
            // start or continue the chain
            if (visible_layer->samples.isEmpty())
            {
                if (visible && !visible->isEmpty())
                    visible_layer->samples << *visible;
                else if (!visible_layer->getExpectations().isEmpty())
                    visible_layer->samples << visible_layer->getExpectations();
                else if (!hidden_layer->samples.isEmpty())
                    sampleVisibleGivenHidden(hidden_layer->samples);    
                else if (!hidden_layer->getExpectations().isEmpty())
                    sampleVisibleGivenHidden(hidden_layer->getExpectations());    
            }
            int min_n = max(Gibbs_step+n_Gibbs_steps_per_generated_sample,
                            min_n_Gibbs_steps);
            //cout << "Gibbs sampling " << Gibbs_step+1;
            for (;Gibbs_step<min_n;Gibbs_step++)
            {
                sampleHiddenGivenVisible(visible_layer->samples);
                sampleVisibleGivenHidden(hidden_layer->samples);
            }
            hidden_activations_are_computed = false;
            //cout << " -> " << Gibbs_step << endl;
        }

        if ( hidden && hidden->isEmpty())   // fill hidden.state with expectations
        {
              const Mat& hidden_expect = hidden_layer->getExpectations();
              hidden->resize(hidden_expect.length(), hidden_expect.width());
              *hidden << hidden_expect;
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
        if (visible_expectation && visible_expectation->isEmpty()) // provide expectation of the visible units
        {
            const Mat& to_store = visible_layer->getExpectations();
            visible_expectation->resize(to_store.length(),
                                        to_store.width());
            *visible_expectation << to_store;
        }
        if (hidden && hidden->isEmpty())
        {
            hidden->resize(hidden_layer->samples.length(),
                           hidden_layer->samples.width());
            *hidden << hidden_layer->samples;
        }
        if (hidden_act && hidden_act->isEmpty())
        {
            hidden_act->resize(hidden_layer->samples.length(),
                               hidden_layer->samples.width());
            *hidden_act << hidden_layer->getExpectations();
        }
        found_a_valid_configuration = true;
    }// END SAMPLING

    // COMPUTE CONTRASTIVE DIVERGENCE CRITERION
    if (contrastive_divergence)
    {
        PLASSERT_MSG( contrastive_divergence->isEmpty(),
                      "RBMModule: the contrastive_divergence port can only be an output port\n" );
        if (visible && !visible->isEmpty())
        {
            int mbs = visible->length();
            const Mat& hidden_expectations = hidden_layer->getExpectations();
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
            }
            else
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
            }
            else
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
                hidden_activations_are_computed = false;
                hidden_layer->computeExpectations();
            }
            PLASSERT(negative_phase_visible_samples);
            PLASSERT(negative_phase_hidden_expectations &&
                     negative_phase_hidden_expectations->isEmpty());
            PLASSERT(negative_phase_hidden_activations &&
                     negative_phase_hidden_activations->isEmpty());
            negative_phase_visible_samples->resize(mbs,visible_layer->size);
            *negative_phase_visible_samples << visible_layer->samples;
            negative_phase_hidden_expectations->resize(hidden_expectations.length(),
                                                       hidden_expectations.width());
            *negative_phase_hidden_expectations << hidden_expectations;
            const Mat& neg_hidden_act = hidden_layer->activations;
            negative_phase_hidden_activations->resize(neg_hidden_act.length(),
                                                      neg_hidden_act.width());
            *negative_phase_hidden_activations << neg_hidden_act;

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
                    - dot((*h)(i),(*h_act)(i))
                    // minus
                    -
                    // negative phase energy
                    (visible_layer->energy(visible_layer->samples(i))
                     - dot(hidden_expectations(i),hidden_layer->activations(i)));
            }
        }
        else
            PLERROR("RBMModule: unknown configuration to compute contrastive_divergence (currently\n"
                    "only possible if only visible are provided in input).\n");
        found_a_valid_configuration = true;
    }


    // Reset some class fields to ensure they are not reused by mistake.
    hidden_act = NULL;
    hidden_bias = NULL;
    weights = NULL;
    hidden_activations_are_computed = false;



    if (!found_a_valid_configuration)
    {
        /*
        if (visible)
            cout << "visible_empty : "<< (bool) visible->isEmpty() << endl;
        if (hidden)
            cout << "hidden_empty : "<< (bool) hidden->isEmpty() << endl;
        if (visible_sample)
            cout << "visible_sample_empty : "<< (bool) visible_sample->isEmpty() << endl;
        if (hidden_sample)
            cout << "hidden_sample_empty : "<< (bool) hidden_sample->isEmpty() << endl;
        if (visible_expectation)
            cout << "visible_expectation_empty : "<< (bool) visible_expectation->isEmpty() << endl;

        */
        PLERROR("In RBMModule::fprop - Unknown port configuration for module %s", name.c_str());
    }

    checkProp(ports_value);

}

////////////////////
// bpropAccUpdate //
////////////////////
void RBMModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                               const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_value.length() == nPorts() );
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
    hidden_bias = ports_value[getPortIndex("hidden_bias")];
    Mat* contrastive_divergence_grad = NULL;
    Mat* contrastive_divergence = NULL;
    if (compute_contrastive_divergence)
        contrastive_divergence = ports_value[getPortIndex("contrastive_divergence")];
    bool computed_contrastive_divergence = compute_contrastive_divergence && 
        contrastive_divergence && !contrastive_divergence->isEmpty();

    // Ensure the gradient w.r.t. contrastive divergence is 1 (if provided).
    if (computed_contrastive_divergence) {
        contrastive_divergence_grad =
            ports_gradient[getPortIndex("contrastive_divergence")];
        if (contrastive_divergence_grad) {
            PLASSERT( !contrastive_divergence_grad->isEmpty() );
            PLASSERT( min(*contrastive_divergence_grad) >= 1 );
            PLASSERT( max(*contrastive_divergence_grad) <= 1 );
        }
    }

    if(reconstruction_connection)
        reconstruction_error_grad =
            ports_gradient[getPortIndex("reconstruction_error.state")];

    // Ensure the visible gradient is not provided as input. This is because we
    // accumulate more than once in 'visible_grad'.
    PLASSERT_MSG( !visible_grad || visible_grad->isEmpty(), "If visible gradient is desired "
                  " the corresponding matrix should have 0 length" );

    bool compute_visible_grad = visible_grad && visible_grad->isEmpty();
    bool compute_weights_grad = weights_grad && weights_grad->isEmpty();

    int mbs = (visible && !visible->isEmpty()) ? visible->length() : -1;

    if (hidden_grad && !hidden_grad->isEmpty())
    {
        // Note: the assert below is for behavior compatibility with previous
        // code. It might not be necessary, or might need to be modified.
        PLASSERT( visible && !visible->isEmpty() );

        // Note: we need to perform the following steps even if the gradient
        // learning rate is equal to 0. This is because we must propagate the
        // gradient to the visible layer, even though no update is required.
        setAllLearningRates(grad_learning_rate);
        PLASSERT_MSG( hidden && hidden_act , "To compute gradients in bprop, the hidden_activations.state port must have been filled during fprop");
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
        if (compute_visible_grad) {
            PLASSERT( visible_grad->width() == visible_layer->size );
            store_visible_grad = visible_grad;
        } else {
            // We do not actually need to store the gradient, but since it
            // is required in bpropUpdate, we provide a dummy matrix to
            // store it.
            store_visible_grad = &visible_exp_grad;
        }
        store_visible_grad->resize(mbs,visible_layer->size);

        if (weights)
        {
            int up = connection->up_size;
            int down = connection->down_size;
            PLASSERT( !weights->isEmpty() &&
                      weights_grad && weights_grad->isEmpty() &&
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

    if (cd_learning_rate > 0 && minimize_log_likelihood) {
        PLASSERT( visible && !visible->isEmpty() );
        PLASSERT( hidden && !hidden->isEmpty() );
        setAllLearningRates(cd_learning_rate);

        // positive phase
        visible_layer->accumulatePosStats(*visible);
        hidden_layer->accumulatePosStats(*hidden);
        connection->accumulatePosStats(*visible,*hidden);

        // negative phase
        PLASSERT_MSG(hidden_layer->size<32 || visible_layer->size<32,
                     "To minimize exact log-likelihood of an RBM, hidden_layer->size "
                     "or visible_layer->size must be <32");
        // gradient of partition function
        if (hidden_layer->size > visible_layer->size)
            // do it by summing over visible configurations
        {
            PLASSERT(visible_layer->classname()=="RBMBinomialLayer");
            // assuming a binary input we sum over all bit configurations
            int n_configurations = 1 << visible_layer->size; // = 2^{visible_layer->size}
            energy_inputs.resize(1, visible_layer->size);
            Vec input = energy_inputs(0);
            // COULD BE DONE MORE EFFICIENTLY BY DOING MANY CONFIGURATIONS
            // AT ONCE IN A 'MINIBATCH'
            for (int c=0;c<n_configurations;c++)
            {
                // convert integer c into a bit-wise visible representation
                int x=c;
                for (int i=0;i<visible_layer->size;i++)
                {
                    input[i]= x & 1; // take least significant bit
                    x >>= 1; // and shift right (divide by 2)
                }
                connection->setAsDownInput(input);
                hidden_layer->getAllActivations(connection,0,false);
                hidden_layer->computeExpectation();
                visible_layer->accumulateNegStats(input);
                hidden_layer->accumulateNegStats(hidden_layer->expectation);
                connection->accumulateNegStats(input,hidden_layer->expectation);
            }
        }
        else
        {
            PLASSERT(hidden_layer->classname()=="RBMBinomialLayer");
            // assuming a binary hidden we sum over all bit configurations
            int n_configurations = 1 << hidden_layer->size; // = 2^{hidden_layer->size}
            energy_inputs.resize(1, hidden_layer->size);
            Vec h = energy_inputs(0);
            for (int c=0;c<n_configurations;c++)
            {
                // convert integer c into a bit-wise hidden representation
                int x=c;
                for (int i=0;i<hidden_layer->size;i++)
                {
                    h[i]= x & 1; // take least significant bit
                    x >>= 1; // and shift right (divide by 2)
                }
                connection->setAsUpInput(h);
                visible_layer->getAllActivations(connection,0,false);
                visible_layer->computeExpectation();
                visible_layer->accumulateNegStats(visible_layer->expectation);
                hidden_layer->accumulateNegStats(h);
                connection->accumulateNegStats(visible_layer->expectation,h);
            }
        }
        // update
        visible_layer->update();
        hidden_layer->update();
        connection->update();
    }
    if (cd_learning_rate > 0 && !minimize_log_likelihood) {
        EXTREME_MODULE_LOG << "Performing contrastive divergence step in RBM '"
                           << name << "'" << endl;
        // Perform a step of contrastive divergence.
        PLASSERT( visible && !visible->isEmpty() );
        setAllLearningRates(cd_learning_rate);
        Mat* negative_phase_visible_samples =
            computed_contrastive_divergence?ports_value[getPortIndex("negative_phase_visible_samples.state")]:0;
        const Mat* negative_phase_hidden_expectations =
            computed_contrastive_divergence ?
                ports_value[getPortIndex("negative_phase_hidden_expectations.state")]
                : NULL;
        Mat* negative_phase_hidden_activations =
            computed_contrastive_divergence ?
                ports_value[getPortIndex("negative_phase_hidden_activations.state")]
                : NULL;

        PLASSERT( visible && hidden );
        PLASSERT( !negative_phase_visible_samples ||
                  !negative_phase_visible_samples->isEmpty() );
        if (!negative_phase_visible_samples)
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
            PLASSERT( !computed_contrastive_divergence );
            PLASSERT( !negative_phase_hidden_expectations );
            PLASSERT( !negative_phase_hidden_activations );
            negative_phase_hidden_expectations = &(hidden_layer->getExpectations());
            negative_phase_visible_samples = &(visible_layer->samples);
            negative_phase_hidden_activations = &(hidden_layer->activations);
        }
        PLASSERT( negative_phase_hidden_expectations &&
                  !negative_phase_hidden_expectations->isEmpty() );
        PLASSERT( negative_phase_hidden_activations &&
                  !negative_phase_hidden_activations->isEmpty() );

        // Perform update.
        visible_layer->update(*visible, *negative_phase_visible_samples);

        bool connection_update_is_done = false;
        if (compute_weights_grad) {
            // First resize the 'weights_grad' matrix.
            int up = connection->up_size;
            int down = connection->down_size;
            PLASSERT( weights && !weights->isEmpty() &&
                      weights_grad->width() == up * down );
            weights_grad->resize(mbs, up * down);

            if (standard_cd_weights_grad)
            {
                // Perform both computation of weights gradient and do update
                // at the same time.
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
                    connection_update_is_done = true;
                }
            }
        }
        if (!standard_cd_weights_grad || !standard_cd_grad) {
            // Compute 'true' gradient of contrastive divergence w.r.t.
            // the weights matrix.
            int up = connection->up_size;
            int down = connection->down_size;
            Mat* weights_g = weights_grad;
            if (!weights_g) {
                // We need to store the gradient in another matrix.
                store_weights_grad.resize(mbs, up * down);
                store_weights_grad.clear();
                weights_g = & store_weights_grad;
            }
            PLASSERT( connection->classname() == "RBMMatrixConnection" &&
                      visible_layer->classname() == "RBMBinomialLayer" &&
                      hidden_layer->classname() == "RBMBinomialLayer" );

            for (int k = 0; k < mbs; k++) {
                int idx = 0;
                for (int i = 0; i < up; i++) {
                    real p_i_p = (*hidden)(k, i);
                    real a_i_p = (*hidden_act)(k, i);
                    real p_i_n =
                        (*negative_phase_hidden_expectations)(k, i);
                    real a_i_n =
                        (*negative_phase_hidden_activations)(k, i);

                    real scale_p = 1 + (1 - p_i_p) * a_i_p;
                    real scale_n = 1 + (1 - p_i_n) * a_i_n;
                    for (int j = 0; j < down; j++, idx++) {
                        // Weight 'idx' is the (i,j)-th element in the
                        // 'weights' matrix.
                        real v_j_p = (*visible)(k, j);
                        real v_j_n =
                            (*negative_phase_visible_samples)(k, j);
                        (*weights_g)(k, idx) +=
                            p_i_n * v_j_n * scale_n     // Negative phase.
                            -(p_i_p * v_j_p * scale_p); // Positive phase.
                    }
                }
            }
            if (!standard_cd_grad) {
                // Update connection manually.
                Mat& weights = ((RBMMatrixConnection*)
                                get_pointer(connection))->weights;
                real lr = cd_learning_rate / mbs;
                for (int k = 0; k < mbs; k++) {
                    int idx = 0;
                    for (int i = 0; i < up; i++)
                        for (int j = 0; j < down; j++, idx++)
                            weights(i, j) -= lr * (*weights_g)(k, idx);
                }
                connection_update_is_done = true;
            }
        }
        if (!connection_update_is_done)
            // Perform standard update of the connection.
            connection->update(*visible, *hidden,
                    *negative_phase_visible_samples,
                    *negative_phase_hidden_expectations);

        Mat* hidden_bias_g = hidden_bias_grad;
        if (!standard_cd_grad && !hidden_bias_grad) {
            // We need to compute the CD gradient w.r.t. bias of hidden layer,
            // but there is no bias coming from the outside. Thus we need
            // another matrix to store this gradient.
            store_hidden_bias_grad.resize(mbs, hidden_layer->size);
            store_hidden_bias_grad.clear();
            hidden_bias_g = & store_hidden_bias_grad;
        }

        if (hidden_bias_g)
        {
            if (hidden_bias_g->isEmpty()) {
                PLASSERT(hidden_bias_g->width() == hidden_layer->size);
                hidden_bias_g->resize(mbs,hidden_layer->size);
            }
            PLASSERT_MSG( hidden_layer->classname() == "RBMBinomialLayer" &&
                          visible_layer->classname() == "RBMBinomialLayer",
                          "Only implemented for binomial layers" );
            // d(contrastive_divergence)/dhidden_bias
            for (int k = 0; k < hidden_bias_g->length(); k++) {
                for (int i = 0; i < hidden_bias_g->width(); i++) {
                    real p_i_p = (*hidden)(k, i);
                    real a_i_p = (*hidden_act)(k, i);
                    real p_i_n = (*negative_phase_hidden_expectations)(k, i);
                    real a_i_n = (*negative_phase_hidden_activations)(k, i);
                    (*hidden_bias_g)(k, i) +=
                        standard_cd_bias_grad ? p_i_n - p_i_p :
                        p_i_n * (1 - p_i_n) * a_i_n + p_i_n     // Neg. phase
                     -( p_i_p * (1 - p_i_p) * a_i_p + p_i_p );  // Pos. phase

                }
            }
        }

        if (standard_cd_grad) {
            hidden_layer->update(*hidden, *negative_phase_hidden_expectations);
        } else {
            PLASSERT( hidden_layer->classname() == "RBMBinomialLayer" );
            // Update hidden layer by hand.
            Vec& bias = hidden_layer->bias;
            real lr = cd_learning_rate / mbs;
            for (int i = 0; i < mbs; i++)
                bias -= lr * (*hidden_bias_g)(i);
        }


        partition_function_is_stale = true;
    } else {
        PLCHECK_MSG( !contrastive_divergence_grad ||
                     (!hidden_bias_grad && !weights_grad),
                "You currently cannot compute the "
                "gradient of contrastive divergence w.r.t. external ports "
                "when 'cd_learning_rate' is set to 0" );
    }

    if (reconstruction_error_grad && !reconstruction_error_grad->isEmpty()) {
        setAllLearningRates(grad_learning_rate);
        PLASSERT( reconstruction_connection != 0 );
        // Perform gradient descent on Autoassociator reconstruction cost
        Mat* visible_reconstruction = ports_value[getPortIndex("visible_reconstruction.state")];
        Mat* visible_reconstruction_activations = ports_value[getPortIndex("visible_reconstruction_activations.state")];
        Mat* reconstruction_error = ports_value[getPortIndex("reconstruction_error.state")];
        PLASSERT( hidden != 0 );
        PLASSERT( visible  && hidden_act &&
                  visible_reconstruction && visible_reconstruction_activations &&
                  reconstruction_error);
        //int mbs = reconstruction_error_grad->length();

        PLCHECK_MSG( !weights, "In RBMModule::bpropAccUpdate(): reconstruction cost "
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
        columnMean(visible_act_grad, visible_bias_grad);
        visible_layer->update(visible_bias_grad);

        // Reconstruction connection update
        hidden_exp_grad.resize(mbs, hidden_layer->size);
        hidden_exp_grad.clear();
        hidden_exp_grad.resize(0, hidden_layer->size);

        TVec<Mat*> rec_ports_value(2);
        rec_ports_value[0] = visible_reconstruction_activations;
        rec_ports_value[1] = hidden;
        TVec<Mat*> rec_ports_gradient(2);
        rec_ports_gradient[0] = &visible_act_grad;
        rec_ports_gradient[1] = &hidden_exp_grad;

        reconstruction_connection->bpropAccUpdate( rec_ports_value,
                                                   rec_ports_gradient );

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
        if(compute_visible_grad)
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

    // Explicit error message in the case of the 'visible' port.
    if (compute_visible_grad && visible_grad->isEmpty())
        PLERROR("In RBMModule::bpropAccUpdate - The gradient with respect "
                "to the 'visible' port was asked, but not computed");

    checkProp(ports_gradient);

    // Reset pointers to ensure we do not reuse them by mistake.
    hidden_act = NULL;
    weights = NULL;
    hidden_bias = NULL;
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
    hidden_layer->computeExpectations();
    hidden_layer->generateSamples();
}

//////////////////////////////
// sampleVisibleGivenHidden //
//////////////////////////////
void RBMModule::sampleVisibleGivenHidden(const Mat& hidden)
{
    computeVisibleActivations(hidden);
    visible_layer->computeExpectations();
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
