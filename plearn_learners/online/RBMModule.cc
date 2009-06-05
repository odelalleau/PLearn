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

// Authors: Olivier Delalleau, Yoshua Bengio

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
    "  - 'visible_activation' : activation of visible units (output port ONLY)\n"
    "  - 'hidden_sample' : random sample obtained on hidden units\n"
    "  - 'energy' : energy of the joint (visible,hidden) pair or free-energy\n"
    "               of the visible (if given) or of the hidden (if given).\n"
    "  - 'hidden_bias' : externally controlled bias on the hidden units,\n"
    "                    used to implement conditional RBMs\n"
    "  - 'neg_log_likelihood' : USE WITH CARE, this is the exact negative log-likelihood\n"
    "    of the RBM. Computing it requires re-computing the partition function (which must\n"
    "    be recomputed if the parameters have changed) and takes O(2^{min(n_hidden,n_visible)})\n"
    "    computations of the free-energy.\n"
    "  - 'neg_log_phidden' : use as an optional input port when asking for an output on\n"
    "    the 'neg_log_pvisible_given_phidden' port. It is a a column matrix with one element\n"
    "    -log w_h for each row h of the input 'hidden.state'. The w_h could be interpreted as\n"
    "    probabilities, e.g. w_h = P(h) according to some prior probability P, and sum_w w_h=1\n"
    "    over the set of h's provided in the 'hidden.state' port.\n"
    "  - 'neg_log_pvisible_given_phidden' : this output port is used to ask the module to compute\n"
    "    a column matrix with entries = -log( sum_h P(x|h) w_h ) for each row x in the input\n"
    "    'visible' port. This quantity would be a valid - log P(x) if sum_h w_h = 1, under the\n"
    "    joint model P(x,h) = P(x|h) P(h), with P(h)=w_h.\n"
    "\n"
    "An RBM also has other ports that exist only if some options are set.\n"
    "If reconstruction_connection is given, then it has\n"
    "  - 'visible_reconstruction_activations.state' : the deterministic reconstruction of the\n"
    "     visible activations through the conditional expectations of the hidden given the visible.\n"
    "  - 'visible_reconstruction.state' : the deterministic reconstruction of the visible\n"
    "     values (expectations) through the conditional expectations of hidden | visible.\n"
    "  - 'reconstruction_error.state' : the auto-associator reconstruction error (NLL)\n"
    "    obtained by matching the visible_reconstruction with the given visible.\n"
    "Note that the above deterministic reconstruction may be made stochastic\n"
    "by using the advanced option 'stochastic_reconstruction'.\n"
    "If compute_contrastive_divergence is true, then the RBM also has these ports\n"
    "  - 'contrastive_divergence' : the quantity minimized by contrastive-divergence training.\n"
    "  - 'negative_phase_visible_samples.state' : the negative phase stochastic reconstruction\n"
    "    of the visible units, only provided to avoid recomputing them in bpropUpdate.\n"
    "  - 'negative_phase_hidden_expectations.state' : the negative phase hidden units\n"
    "    expected values, only provided to avoid recomputing them in bpropUpdate.\n"
    "The following ports are filled only in test mode when the option\n"
    "'compare_true_gradient_with_cd' is true:\n"
    "   - 'median_reldiff_cd_nll': median relative difference between the CD\n"
    "     update and the true NLL gradient. Here, the CD update is not\n"
    "     stochastic, but is computed exactly as the truncation of the log-\n"
    "     likelihood expansion. This port has size 'n_steps_compare': there\n"
    "     is one value for each step of the CD.\n"
    "   - 'mean_diff_cd_nll': mean of the absolute difference between the CD\n"
    "     and NLL gradient updates.\n"
    "   - 'agreement_cd_nll': fraction of weights for which the CD and NLL\n"
    "     gradient updates agree on the sign, followed by the fraction of\n"
    "     weights for which the CD update has same sign as the difference\n"
    "     between the NLL gradient and the CD update.\n"
    "   - 'agreement_stoch': same as the first half of above, except that\n"
    "     it is for the stochastic CD update rather than its expected value.\n"
    "   - 'bound_cd_nll': bound on the difference between the CD and NLL\n"
    "     gradient updates, as computed in (Bengio & Delalleau, 2008)\n"
    "   - 'weights_stats': first element is the median of the absolute value\n"
    "     of all weights and biases, second element is the mean, third\n"
    "     element is the maximum sum of weights and biases (in absolute\n"
    "     values) over columns of the weight matrix, and third element is\n"
    "     the same over rows.\n"
    "   - 'ratio_cd_leftout': median ratio between the absolute value of the\n"
    "     CD update and the absolute value of the term left out in CD (i.e.\n"
    "     the difference between NLL gradient and CD).\n"
    "   - 'abs_cd': average absolute value of the CD update. First for the\n"
    "     expected CD update, then its stochastic (sampled) version.\n"
    "   - 'nll_grad': NLL gradient.\n"
    "    \n"
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
    tied_connection_weights(false),
    compute_contrastive_divergence(false),
    compare_true_gradient_with_cd(false),
    n_steps_compare(1),
    n_Gibbs_steps_CD(1),
    min_n_Gibbs_steps(1),
    n_Gibbs_steps_per_generated_sample(-1),
    compute_log_likelihood(false),
    minimize_log_likelihood(false),
    Gibbs_step(0),
    log_partition_function(0),
    partition_function_is_stale(true),
    deterministic_reconstruction_in_cd(false),
    stochastic_reconstruction(false),
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
    // Build options.

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

    declareOption(ol, "stochastic_reconstruction",
                  &RBMModule::stochastic_reconstruction,
                  OptionBase::buildoption,
        "If set to true, then reconstruction is not deterministic. Instead,\n"
        "we sample a hidden vector given the visible input, then use the\n"
        "visible layer's expectation given this sample as reconstruction.",
                  OptionBase::advanced_level);

    declareOption(ol, "grad_learning_rate", &RBMModule::grad_learning_rate,
                  OptionBase::buildoption,
        "Learning rate for the gradient descent step.");

    declareOption(ol, "cd_learning_rate", &RBMModule::cd_learning_rate,
                  OptionBase::buildoption,
        "Learning rate for the constrastive divergence step. Note that when\n"
        "set to 0, the gradient of the contrastive divergence will not be\n"
        "computed at all.");

    declareOption(ol, "tied_connection_weights", &RBMModule::tied_connection_weights,
                  OptionBase::buildoption,
        "Whether to keep fixed the connection weights during learning.");

    declareOption(ol, "compute_contrastive_divergence", &RBMModule::compute_contrastive_divergence,
                  OptionBase::buildoption,
        "Compute the constrastive divergence in an output port.");

    declareOption(ol, "deterministic_reconstruction_in_cd",
                  &RBMModule::deterministic_reconstruction_in_cd,
                  OptionBase::buildoption,
        "Whether to use the expectation of the visible (given a hidden sample)\n"
        "or a sample of the visible in the contrastive divergence learning.\n"
        "In other words, instead of the classical Gibbs sampling\n"
        "   v_0 --> h_0 ~ p(h|v_0) --> v_1 ~ p(v|h_0) -->  p(h|v_1)\n"
        "we will have by setting 'deterministic_reconstruction_in_cd=1'\n"
        "   v_0 --> h_0 ~ p(h|v_0) --> v_1 = E(v|h_0) -->  p(h|E(v|h_0)).");

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

    declareOption(ol, "compare_true_gradient_with_cd",
                  &RBMModule::compare_true_gradient_with_cd,
                  OptionBase::buildoption,
        "If true, then will compute the true gradient (of the NLL) as well\n"
        "as the exact non-stochastic CD update, and compare them.",
                  OptionBase::advanced_level);

    declareOption(ol, "n_steps_compare",
                  &RBMModule::n_steps_compare,
                  OptionBase::buildoption,
        "Number of steps for which we want to compare CD with the true\n"
        "gradient (when 'compare_true_gradient_with_cd' is true). This will\n"
        "compute P(x_t|x) for t from 1 to 'n_steps_compare'.",
                  OptionBase::advanced_level);

    // Learnt options.

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

void RBMModule::declareMethods(RemoteMethodMap& rmm)
{
    // Make sure that inherited methods are declared
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(rmm, "CDUpdate", &RBMModule::CDUpdate,
                  (BodyDoc("Perform one CD_k update"),
                   ArgDoc ("v_0", "Positive phase statistics on visible layer"),
                   ArgDoc ("h_0", "Positive phase statistics on hidden layer"),
                   ArgDoc ("v_k", "Negative phase statistics on visible layer"),
                   ArgDoc ("h_k", "Negative phase statistics on hidden layer")
                  ));

    declareMethod(rmm, "computePartitionFunction",
        &RBMModule::computePartitionFunction,
        (BodyDoc("Compute the log partition function (will be stored within "
                 "the 'log_partition_function' field)")));

    declareMethod(rmm, "computeLogLikelihoodOfVisible",
        &RBMModule::computeLogLikelihoodOfVisible,
        (BodyDoc("Compute log-likehood"),
         ArgDoc("visible", "Matrix of visible inputs"),
         RetDoc("A vector with the log-likelihood of each input")));
}

void RBMModule::CDUpdate(const Mat& v_0, const Mat& h_0,
                         const Mat& v_k, const Mat& h_k)
{
    visible_layer->update(v_0, v_k);
    hidden_layer->update(h_0, h_k);
    connection->update(v_0, h_0, v_k, h_k);
    partition_function_is_stale = true;
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
    addPortName("visible_activations.state");
    addPortName("hidden_sample");
    addPortName("energy");
    addPortName("hidden_bias");
    addPortName("weights");
    addPortName("neg_log_likelihood");
    // a column matrix with one element -log P(h) for each row h of "hidden",
    // used as an input port, with neg_log_pvisible_given_phidden as output
    addPortName("neg_log_phidden");
    // compute column matrix with one entry -log P(x) = -log( sum_h P(x|h) P(h) ) for
    // each row x of "visible", and where {P(h)}_h is provided
    // in "neg_log_phidden" for the set of h's in "hidden".
    addPortName("neg_log_pvisible_given_phidden");
    addPortName("median_reldiff_cd_nll");
    addPortName("mean_diff_cd_nll");
    addPortName("agreement_cd_nll");
    addPortName("agreement_stoch");
    addPortName("bound_cd_nll");
    addPortName("weights_stats");
    addPortName("ratio_cd_leftout");
    addPortName("abs_cd");
    addPortName("nll_grad");
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
        port_sizes(getPortIndex("visible_activations.state"), 1) = visible_layer->size;
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
    port_sizes(getPortIndex("neg_log_phidden"),1) = 1;
    port_sizes(getPortIndex("neg_log_pvisible_given_phidden"),1) = 1;
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
// or more robustly,
//  = hidden_layer->energy(h)
//    + visible_layer->freeEnergyContribution(visible_layer->activation)
void RBMModule::computeFreeEnergyOfHidden(const Mat& hidden, Mat& energy)
{
    int mbs=hidden.length();
    if (energy.isEmpty())
        energy.resize(mbs,1);
    else {
        PLASSERT( energy.length() == mbs && energy.width() == 1 );
    }

    computeVisibleActivations(hidden, false);
    for (int i=0;i<mbs;i++)
    {
        energy(i,0) = hidden_layer->energy(hidden(i))
            + visible_layer->freeEnergyContribution(
                visible_layer->activations(i));
    }
}

////////////////////////////////
// computeFreeEnergyOfVisible //
////////////////////////////////
// FREE-ENERGY(visible) CASE
// we know x:
// free energy = -log sum_h e^{-energy(h,x)}
// or more robustly,
//  = visible_layer->energy(x)
//    + hidden_layer->freeEnergyContribution(hidden_layer->activation)
void RBMModule::computeFreeEnergyOfVisible(const Mat& visible, Mat& energy,
                                           bool positive_phase)
{
    int mbs=visible.length();
    if (energy.isEmpty())
        energy.resize(mbs,1);
    else {
        PLASSERT( energy.length() == mbs && energy.width() == 1 );
    }

    Mat* hidden_activations = NULL;
    if (positive_phase && hidden_act) {
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
        energy(i,0) = visible_layer->energy(visible(i))
            + hidden_layer->freeEnergyContribution((*hidden_activations)(i));
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

///////////////////////////////////
// computeLogLikelihoodOfVisible //
///////////////////////////////////
Vec RBMModule::computeLogLikelihoodOfVisible(const Mat& visible)
{
    Mat energy;
    computePartitionFunction();
    computeFreeEnergyOfVisible(visible, energy, false);
    negateElements(energy);
    for (int i = 0; i < energy.length(); i++)
        energy(i, 0) -= log_partition_function;
    return energy.toVec();
}

///////////////////////////////////
// computeAllHiddenProbabilities //
///////////////////////////////////
void RBMModule::computeAllHiddenProbabilities(const Mat& visible,
                                              const Mat& p_hidden)
{
    Vec hidden(hidden_layer->size);
    computeHiddenActivations(visible);
    int n_conf = hidden_layer->getConfigurationCount();
    for (int i = 0; i < n_conf; i++) {
        hidden_layer->getConfiguration(i, hidden);
        for (int j = 0; j < visible.length(); j++) {
            hidden_layer->activation = hidden_layer->activations(j);
            real neg_log_p_h_given_v = hidden_layer->fpropNLL(hidden);
            p_hidden(i, j) = exp(-neg_log_p_h_given_v);
        }
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

    deepCopyField(all_p_visible,            copies);
    deepCopyField(all_hidden_cond_prob,     copies);
    deepCopyField(all_visible_cond_prob,    copies);
    deepCopyField(p_ht_given_x,             copies);
    deepCopyField(p_xt_given_x,             copies);
}

///////////
// fprop //
///////////
void RBMModule::fprop(const Vec& input, Vec& output) const
{
    PLERROR("In RBMModule::fprop - Not implemented");
}

//////////////////////////////
// computePartitionFunction //
//////////////////////////////
void RBMModule::computePartitionFunction()
{
    int hidden_configurations = hidden_layer->getConfigurationCount();
    int visible_configurations = visible_layer->getConfigurationCount();

    PLASSERT_MSG(hidden_configurations != RBMLayer::INFINITE_CONFIGURATIONS ||
                 visible_configurations != RBMLayer::INFINITE_CONFIGURATIONS,
                 "To compute exact log-likelihood of an RBM maximum configurations of hidden "
                 "or visible layer must be less than 2^31.");

    // Compute partition function
    if (hidden_configurations > visible_configurations ||
        compare_true_gradient_with_cd)
        // do it by log-summing minus-free-energy of visible configurations
    {
        if (compare_true_gradient_with_cd) {
            all_p_visible.resize(visible_configurations);
            all_visible_cond_prob.resize(visible_configurations,
                                         hidden_configurations);
            all_hidden_cond_prob.resize(hidden_configurations,
                                        visible_configurations);
        }
        energy_inputs.resize(1, visible_layer->size);
        Vec input = energy_inputs(0);
        // COULD BE DONE MORE EFFICIENTLY BY DOING MANY CONFIGURATIONS
        // AT ONCE IN A 'MINIBATCH'
        Mat free_energy(1, 1);
        log_partition_function = 0;
        PP<ProgressBar> pb;
        if (verbosity >= 2)
            pb = new ProgressBar("Computing partition function",\
                                 visible_configurations);
        for (int c = 0; c < visible_configurations; c++)
        {
            visible_layer->getConfiguration(c, input);
            computeFreeEnergyOfVisible(energy_inputs, free_energy, false);
            real fe = free_energy(0,0);
            if (c==0)
                log_partition_function = -fe;
            else
                log_partition_function = logadd(log_partition_function, -fe);
            if (compare_true_gradient_with_cd) {
                all_p_visible[c] = -fe;
                // Compute P(visible | hidden) and P(hidden | visible) for all
                // values of hidden.
                computeAllHiddenProbabilities(input.toMat(1, input.length()),
                                              all_hidden_cond_prob.column(c));
                Vec hidden(hidden_layer->size);
                for (int d = 0; d < hidden_configurations; d++) {
                    hidden_layer->getConfiguration(d, hidden);
                    computeVisibleActivations(hidden.toMat(1, hidden.length()),
                                              false);
                    visible_layer->activation = visible_layer->activations(0);
                    real neg_log_p_v_given_h = visible_layer->fpropNLL(input);
                    all_visible_cond_prob(c, d) = exp(-neg_log_p_v_given_h);
                }
            }
            if (pb)
                pb->update(c + 1);
        }
        pb = NULL;
        hidden_activations_are_computed = false;
        if (compare_true_gradient_with_cd) {
            // Normalize probabilities.
            for (int i = 0; i < all_p_visible.length(); i++)
                all_p_visible[i] =
                    exp(all_p_visible[i] - log_partition_function);
            //pout << "All P(x): " << all_p_visible << endl;
            //pout << "Sum_x P(x) = " << sum(all_p_visible) << endl;
            if (!is_equal(sum(all_p_visible), 1)) {
                PLWARNING("The sum of all probability is not 1: %f",
                        sum(all_p_visible));
                // Renormalize.
                all_p_visible /= sum(all_p_visible);
            }
            PLCHECK( is_equal(sum(all_p_visible), 1) );
        }
    }
    else
        // do it by summing free-energy of hidden configurations
    {
        PLASSERT( !compare_true_gradient_with_cd );
        energy_inputs.resize(1, hidden_layer->size);
        Vec input = energy_inputs(0);
        // COULD BE DONE MORE EFFICIENTLY BY DOING MANY CONFIGURATIONS
        // AT ONCE IN A 'MINIBATCH'
        Mat free_energy(1, 1);
        log_partition_function = 0;
        for (int c = 0; c < hidden_configurations; c++)
        {
            hidden_layer->getConfiguration(c, input);
            //pout << "Input = " << input << endl;
            computeFreeEnergyOfHidden(energy_inputs, free_energy);
            //pout << "FE = " << free_energy(0, 0) << endl;
            real fe = free_energy(0,0);
            if (c==0)
                log_partition_function = -fe;
            else
                log_partition_function = logadd(log_partition_function, -fe);
        }
    }
    if (false)
        pout << "Log Z(" << name << ") = " << log_partition_function << endl;
}

///////////
// fprop //
///////////
void RBMModule::fprop(const TVec<Mat*>& ports_value)
{

    PLASSERT( ports_value.length() == nPorts() );
    PLASSERT( visible_layer );
    PLASSERT( hidden_layer );
    PLASSERT( connection );

    Mat* visible = ports_value[getPortIndex("visible")];
    bool visible_is_output = visible && visible->isEmpty();
    Mat* hidden = ports_value[getPortIndex("hidden.state")];
    // hidden_is_output is needed in BPROP, which is VERY BAD, VIOLATING OUR DESIGN ASSUMPTIONS
    hidden_is_output = hidden && hidden->isEmpty();
    hidden_act = ports_value[getPortIndex("hidden_activations.state")];
    bool hidden_act_is_output = hidden_act && hidden_act->isEmpty();
    Mat* visible_sample = ports_value[getPortIndex("visible_sample")];
    bool visible_sample_is_output = visible_sample && visible_sample->isEmpty();
    Mat* visible_expectation = ports_value[getPortIndex("visible_expectation")];
    bool visible_expectation_is_output = visible_expectation && visible_expectation->isEmpty();
    Mat* visible_activation = ports_value[getPortIndex("visible_activations.state")];
    bool visible_activation_is_output = visible_activation && visible_activation->isEmpty();
    Mat* hidden_sample = ports_value[getPortIndex("hidden_sample")];
    bool hidden_sample_is_output = hidden_sample && hidden_sample->isEmpty();
    Mat* energy = ports_value[getPortIndex("energy")];
    bool energy_is_output = energy && energy->isEmpty();
    Mat* neg_log_likelihood = ports_value[getPortIndex("neg_log_likelihood")];
    bool neg_log_likelihood_is_output = neg_log_likelihood && neg_log_likelihood->isEmpty();
    Mat* neg_log_phidden = ports_value[getPortIndex("neg_log_phidden")];
    bool neg_log_phidden_is_output = neg_log_phidden && neg_log_phidden->isEmpty();
    Mat* neg_log_pvisible_given_phidden = ports_value[getPortIndex("neg_log_pvisible_given_phidden")];
    bool neg_log_pvisible_given_phidden_is_output = neg_log_pvisible_given_phidden && neg_log_pvisible_given_phidden->isEmpty();
    Mat* median_reldiff_cd_nll = ports_value[getPortIndex("median_reldiff_cd_nll")];
    bool median_reldiff_cd_nll_is_output = median_reldiff_cd_nll && median_reldiff_cd_nll->isEmpty();
    Mat* mean_diff_cd_nll = ports_value[getPortIndex("mean_diff_cd_nll")];
    bool mean_diff_cd_nll_is_output = mean_diff_cd_nll && mean_diff_cd_nll->isEmpty();
    Mat* agreement_cd_nll = ports_value[getPortIndex("agreement_cd_nll")];
    bool agreement_cd_nll_is_output = agreement_cd_nll && agreement_cd_nll->isEmpty();
    Mat* agreement_stoch = ports_value[getPortIndex("agreement_stoch")];
    bool agreement_stoch_is_output = agreement_stoch && agreement_stoch->isEmpty();
    Mat* bound_cd_nll = ports_value[getPortIndex("bound_cd_nll")];
    bool bound_cd_nll_is_output = bound_cd_nll && bound_cd_nll->isEmpty();
    Mat* weights_stats = ports_value[getPortIndex("weights_stats")];
    bool weights_stats_is_output = weights_stats && weights_stats->isEmpty();
    Mat* ratio_cd_leftout = ports_value[getPortIndex("ratio_cd_leftout")];
    bool ratio_cd_leftout_is_output = ratio_cd_leftout && ratio_cd_leftout->isEmpty();
    Mat* abs_cd = ports_value[getPortIndex("abs_cd")];
    bool abs_cd_is_output = abs_cd && abs_cd->isEmpty();
    Mat* nll_grad = ports_value[getPortIndex("nll_grad")];
    bool nll_grad_is_output = nll_grad && nll_grad->isEmpty();
    hidden_bias = ports_value[getPortIndex("hidden_bias")];
    //bool hidden_bias_is_output = hidden_bias && hidden_bias->isEmpty();
    weights = ports_value[getPortIndex("weights")];
    //bool weights_is_output = weights && weights->isEmpty();
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
    bool visible_reconstruction_is_output = visible_reconstruction && visible_reconstruction->isEmpty();
    bool visible_reconstruction_activations_is_output = visible_reconstruction_activations && visible_reconstruction_activations->isEmpty();
    bool reconstruction_error_is_output = reconstruction_error && reconstruction_error->isEmpty();
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
    bool contrastive_divergence_is_output = contrastive_divergence && contrastive_divergence->isEmpty();
    //bool negative_phase_visible_samples_is_output = negative_phase_visible_samples && negative_phase_visible_samples->isEmpty();
    bool negative_phase_hidden_expectations_is_output = negative_phase_hidden_expectations && negative_phase_hidden_expectations->isEmpty();
    bool negative_phase_hidden_activations_is_output = negative_phase_hidden_activations && negative_phase_hidden_activations->isEmpty();

    bool hidden_expectations_are_computed = false;
    hidden_activations_are_computed = false;
    bool found_a_valid_configuration = false;

    if (visible && !visible_is_output)
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
        PLASSERT_MSG( energy_is_output,
                      "RBMModule: the energy port can only be an output port\n" );
        if (visible && !visible_is_output
            && hidden && !hidden_is_output)
        {
            computeEnergy(*visible, *hidden, *energy);
        }
        else if (visible && !visible_is_output)
        {
            computeFreeEnergyOfVisible(*visible,*energy);
        }
        else if (hidden && !hidden_is_output)
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


    // COMPUTE UNSUPERVISED NLL
    if (neg_log_likelihood && neg_log_likelihood_is_output && compute_log_likelihood)
    {
        if (partition_function_is_stale && !during_training)
        {
            // Save layers' state
            Mat visible_activations = visible_layer->activations.copy();
            Mat visible_expectations = visible_layer->getExpectations().copy();
            Mat visible_samples = visible_layer->samples.copy();

            Mat hidden_activations = hidden_layer->activations.copy();
            Mat hidden_expectations = hidden_layer->getExpectations().copy();
            Mat hidden_samples = hidden_layer->samples.copy();

            computePartitionFunction();

            // Restore layers' state
            visible_layer->activations.resize(visible_activations.length(),
                                              visible_activations.width());
            visible_layer->activations << visible_activations;

            visible_layer->setExpectations(visible_expectations);

            visible_layer->samples.resize(visible_samples.length(),
                                          visible_samples.width());
            visible_layer->samples << visible_samples;

            hidden_layer->activations.resize(hidden_activations.length(),
                                              hidden_activations.width());
            hidden_layer->activations << hidden_activations;

            hidden_layer->setExpectations(hidden_expectations);

            hidden_layer->samples.resize(hidden_samples.length(),
                                          hidden_samples.width());
            hidden_layer->samples << hidden_samples;

            partition_function_is_stale=false;
        }
        if (visible && !visible_is_output
            && hidden && !hidden_is_output)
        {
            // neg-log-likelihood(visible,hidden) = energy(visible,hidden) + log(partition_function)
            computeEnergy(*visible,*hidden,*neg_log_likelihood);
            *neg_log_likelihood += log_partition_function;
        }
        else if (visible && !visible_is_output)
        {
            // neg-log-likelihood(visible) = free_energy(visible) + log(partition_function)
            computeFreeEnergyOfVisible(*visible,*neg_log_likelihood,hidden_act);
            *neg_log_likelihood += log_partition_function;
        }
        else if (hidden && !hidden_is_output)
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
    if ( visible && !visible_is_output &&
         hidden && hidden_is_output )
    {
        computePositivePhaseHiddenActivations(*visible);
        PLCHECK_MSG( !hidden_layer->expectations_are_up_to_date, "Safety "
                     "check: how were expectations computed previously?" );
        hidden_layer->computeExpectations();
        hidden_expectations_are_computed=true;
        const Mat& hidden_out = hidden_layer->getExpectations();
        hidden->resize(hidden_out.length(), hidden_out.width());
        *hidden << hidden_out;

        // Since we return below, the other ports must be unused.
        //PLASSERT( !visible_sample && !hidden_sample );
        found_a_valid_configuration = true;
    }

    // DOWNWARD FPROP
    // we are given hidden  and we want to compute the visible or visible_activation
    if ( hidden && !hidden_is_output && visible && visible_is_output)
    {
        computeVisibleActivations(*hidden,true);
        if (visible_activation)
        {
            PLASSERT_MSG(visible_activation_is_output,"visible_activation should be an output");
            visible_activation->resize(visible_layer->activations.length(),
                                       visible_layer->size);
            *visible_activation << visible_layer->activations;
        }
        if (visible)
        {
            PLASSERT_MSG(visible_is_output,"visible should be an output");
            visible_layer->computeExpectations();
            const Mat expectations=visible_layer->getExpectations();
            visible->resize(expectations.length(),visible_layer->size);
            *visible << expectations;
        }
        if (hidden_act && hidden_act_is_output)
        {
            // THIS IS STUPID CODE TO HANDLE THE BAD state SYSTEM AND AVOID AN UNNECESSARY ERROR MESSAGE
            // (hidden_act is a "state" port that must always be produced, even if we don't compute it!)
            hidden_act->resize(hidden_layer->samples.length(),
                               hidden_layer->samples.width());
        }
        found_a_valid_configuration = true;
    }

    // COMPUTE AUTOASSOCIATOR RECONSTRUCTION ERROR
    if ( visible && !visible_is_output &&
         ( ( visible_reconstruction && visible_reconstruction_is_output ) ||
           ( visible_reconstruction_activations &&
             visible_reconstruction_activations_is_output ) ||
           ( reconstruction_error && reconstruction_error_is_output ) ) )
    {
        // Autoassociator reconstruction cost
        PLASSERT( ports_value.length() == nPorts() );

        Mat h;
        if (hidden && !hidden_is_output) {
            h = *hidden;
            PLASSERT(!stochastic_reconstruction);
        } else {
            if(!hidden_expectations_are_computed)
            {
                computePositivePhaseHiddenActivations(*visible);
                hidden_layer->computeExpectations();
                hidden_expectations_are_computed=true;
            }
            if (stochastic_reconstruction) {
                hidden_layer->generateSamples();
                h = hidden_layer->samples;
            } else
                h = hidden_layer->getExpectations();
        }

        // Don't need to verify if they are asked in a port, this was done previously

        computeVisibleActivations(h, true);
        if(visible_reconstruction_activations)
        {
            PLASSERT( visible_reconstruction_activations_is_output );
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
                PLASSERT( visible_reconstruction_is_output );
                const Mat& to_store = visible_layer->getExpectations();
                visible_reconstruction->resize(to_store.length(),
                                               to_store.width());
                *visible_reconstruction << to_store;
            }
            if(reconstruction_error)
            {
                PLASSERT( reconstruction_error_is_output );
                reconstruction_error->resize(visible->length(),1);
                visible_layer->setBatchSize( visible->length() );
                visible_layer->fpropNLL(*visible,
                                        *reconstruction_error);
            }
        }
        found_a_valid_configuration = true;
    }
    // COMPUTE VISIBLE GIVEN HIDDEN
    else if ( visible_reconstruction && visible_reconstruction_is_output
         && hidden && !hidden_is_output)
    {
        PLASSERT_MSG(!stochastic_reconstruction,
                     "Not yet implemented");
        // Don't need to verify if they are asked in a port, this was done previously
        computeVisibleActivations(*hidden,true);
        if(visible_reconstruction_activations)
        {
            PLASSERT( visible_reconstruction_activations_is_output );
            const Mat& to_store = visible_layer->activations;
            visible_reconstruction_activations->resize(to_store.length(),
                                                       to_store.width());
            *visible_reconstruction_activations << to_store;
        }
        visible_layer->computeExpectations();
        PLASSERT( visible_reconstruction_is_output );
        const Mat& to_store = visible_layer->getExpectations();
        visible_reconstruction->resize(to_store.length(),
                                       to_store.width());
        *visible_reconstruction << to_store;
        found_a_valid_configuration = true;
    }

    // Compute column matrix with one entry:
    //      -log P(x) = -log( sum_h P(x|h) P(h) )
    // for each row x of "visible", and where {P(h)}_h is provided
    // in "neg_log_phidden" for the set of h's in "hidden".
    //
    // neg_log_phidden is an optional column matrix with one element:
    //      -log P(h)
    // for each row h of "hidden", used as an input port,
    // with neg_log_pvisible_given_phidden as output.
    //
    // If neg_log_phidden is provided, it is assumed to be
    // 1/n_h (n_h=h->length()).
    if (neg_log_pvisible_given_phidden
        && neg_log_pvisible_given_phidden_is_output
        && hidden && !hidden_is_output
        && visible && !visible_is_output)
    {
        // estimate P(x) by sum_h P(x|h) P(h) where P(h) is either constant
        // or provided by neg_log_phidden
        if (neg_log_phidden)
        {
            PLASSERT_MSG(!neg_log_phidden_is_output,
                         "If neg_log_phidden is provided, it must be an input");
            PLASSERT_MSG(neg_log_phidden->length()==hidden->length(),
                        "If neg_log_phidden is provided, it must have the same"
                        " length as hidden.state");
            PLASSERT_MSG(neg_log_phidden->width()==1,
                         "neg_log_phidden must have width 1 (single column)");
        }
        computeNegLogPVisibleGivenPHidden(*visible,
                                          *hidden,
                                          neg_log_phidden,
                                          *neg_log_pvisible_given_phidden);
        found_a_valid_configuration = true;
    }

    // SAMPLING
    if ((visible_sample && visible_sample_is_output)
            // is asked to sample visible units (discrete)
        || (visible_expectation && visible_expectation_is_output)
            //              "                   (continous)
        || (hidden_sample && hidden_sample_is_output)
            // or to sample hidden units
        )
    {
        if (hidden_sample && !hidden_sample_is_output)
            // sample visible conditionally on hidden
        {
            sampleVisibleGivenHidden(*hidden_sample);
            Gibbs_step=0;
            //cout << "sampling visible from hidden" << endl;
        }
        else if (visible_sample && !visible_sample_is_output)
            // if an input is provided, sample hidden conditionally
        {
            sampleHiddenGivenVisible(*visible_sample);
            hidden_activations_are_computed = false;
            Gibbs_step = 0;
            //cout << "sampling hidden from visible" << endl;
        }
        else if (visible_expectation && !visible_expectation_is_output)
        {
             PLERROR("In RBMModule::fprop visible_expectation can only be an output port (use visible as input port");
        }
        else // sample unconditionally: Gibbs sample after k steps
        {
            // Find out how many samples we want.
            // TODO: check if this code is OK.
            int n_samples = -1;
            if (visible_sample_is_output)
            {
                // Not exactly sure of where to pick the sizes from
                visible_sample->resize(visible_layer->samples.length(),
                                       visible_layer->samples.width());
                n_samples = visible_sample->length();
            }
            if (visible_expectation_is_output)
            {
                // Not exactly sure of where to pick the sizes from
                visible_expectation->resize(visible_layer->samples.length(),
                                            visible_layer->samples.width());
                PLASSERT( n_samples == -1 ||
                          n_samples == visible_expectation->length() );
                n_samples = visible_expectation->length();
            }
            if (hidden_sample_is_output)
            {
                // Not exactly sure of where to pick the sizes from
                hidden_sample->resize(hidden_layer->samples.length(),
                                      hidden_layer->samples.width());

                PLASSERT( n_samples == -1 ||
                          n_samples == hidden_sample->length() );
                n_samples = hidden_sample->length();
            }
            PLCHECK( n_samples > 0 );

            // the visible_layer->expectations contain the "state" from which we
            // start or continue the chain
            if (visible_layer->samples.isEmpty())
            {
                // There are no samples already available to continue the
                // chain: we restart it.
                Gibbs_step = 0;
                if (visible && !visible_is_output)
                    visible_layer->samples << *visible;
                else if (!visible_layer->getExpectations().isEmpty())
                    visible_layer->samples << visible_layer->getExpectations();
                else if (!hidden_layer->samples.isEmpty())
                    sampleVisibleGivenHidden(hidden_layer->samples);
                else if (!hidden_layer->getExpectations().isEmpty())
                    sampleVisibleGivenHidden(hidden_layer->getExpectations());
                else {
                    // There is no available data to initialize the chain: we
                    // initialize it with a zero vector.
                    Mat& zero_vector = visible_layer->samples;
                    PLASSERT( zero_vector.width() > 0 );
                    zero_vector.resize(1, zero_vector.width());
                    zero_vector.clear();
                }
            }
            int min_n = max(Gibbs_step+n_Gibbs_steps_per_generated_sample,
                            min_n_Gibbs_steps);
            //cout << "Gibbs sampling " << Gibbs_step+1;
            PP<ProgressBar> pb =
                verbosity >= 2 ? new ProgressBar("Gibbs sampling",
                                                 min_n - Gibbs_step)
                               : NULL;
            int start = Gibbs_step;
            for (;Gibbs_step<min_n;Gibbs_step++)
            {
                sampleHiddenGivenVisible(visible_layer->samples);
                sampleVisibleGivenHidden(hidden_layer->samples);
                if (pb)
                    pb->update(Gibbs_step - start);
            }
            if (pb)
                pb = NULL;
            hidden_activations_are_computed = false;
            //cout << " -> " << Gibbs_step << endl;
        }

        if ( hidden && hidden_is_output)
            // fill hidden.state with expectations
        {
              const Mat& hidden_expect = hidden_layer->getExpectations();
              hidden->resize(hidden_expect.length(), hidden_expect.width());
              *hidden << hidden_expect;
        }
        if (visible_sample && visible_sample_is_output)
            // provide sample of the visible units
        {
            visible_sample->resize(visible_layer->samples.length(),
                                   visible_layer->samples.width());
            PLASSERT( visible_sample->length() ==
                      visible_layer->samples.length() );
            *visible_sample << visible_layer->samples;
        }
        if (hidden_sample && hidden_sample_is_output)
            // provide sample of the hidden units
        {
            hidden_sample->resize(hidden_layer->samples.length(),
                                  hidden_layer->samples.width());
            PLASSERT( hidden_sample->length() ==
                      hidden_layer->samples.length() );
            *hidden_sample << hidden_layer->samples;
        }
        if (visible_expectation && visible_expectation_is_output)
            // provide expectation of the visible units
        {
            const Mat& to_store = visible_layer->getExpectations();
            visible_expectation->resize(to_store.length(),
                                        to_store.width());
            PLASSERT( visible_expectation->length() == to_store.length() );
            *visible_expectation << to_store;
        }
        if (hidden && hidden_is_output)
        {
            hidden->resize(hidden_layer->getExpectations().length(),
                           hidden_layer->getExpectations().width());
            PLASSERT( hidden->length() ==
                      hidden_layer->getExpectations().length() );
            *hidden << hidden_layer->getExpectations();
        }
        if (hidden_act && hidden_act_is_output)
        {
            hidden_act->resize(hidden_layer->activations.length(),
                               hidden_layer->activations.width());
            PLASSERT( hidden_act->length() ==
                      hidden_layer->activations.length() );
            *hidden_act << hidden_layer->activations;
        }
        found_a_valid_configuration = true;
    }// END SAMPLING

    // COMPUTE CONTRASTIVE DIVERGENCE CRITERION
    if (contrastive_divergence)
    {
        PLASSERT_MSG( contrastive_divergence_is_output,
                      "RBMModule: the contrastive_divergence port can only be an output port\n" );
        if (visible && !visible_is_output)
        {
            int mbs = visible->length();
            const Mat& hidden_expectations = hidden_layer->getExpectations();
            Mat* h=0;
            Mat* h_act=0;
            if (!hidden_activations_are_computed)
                // it must be because neither hidden nor hidden_act were asked
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
                // hidden_act must have been computed above if they were
                // requested on port
                PLASSERT(hidden_act && !hidden_act->isEmpty());
                h_act = hidden_act;
            }
            if (!hidden_expectations_are_computed)
                // it must be because hidden outputs were not asked
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
                // hidden exp. must have been computed above if they were
                // requested on port
                PLASSERT(hidden && !hidden->isEmpty());
                h = hidden;
            }
            // perform negative phase
            for( int i=0; i<n_Gibbs_steps_CD; i++)
            {
                hidden_layer->generateSamples();
                if (deterministic_reconstruction_in_cd)
                {
                   // (Negative phase) compute visible expectations
                   computeVisibleActivations(hidden_layer->samples);
                   visible_layer->computeExpectations();
                   // compute corresponding hidden expectations.
                   computeHiddenActivations(visible_layer->getExpectations());
                }
                else
                {
                   // (Negative phase) Generate visible samples.
                   sampleVisibleGivenHidden(hidden_layer->samples);
                   // compute corresponding hidden expectations.
                   computeHiddenActivations(visible_layer->samples);
                }
                hidden_activations_are_computed = false;
                hidden_layer->computeExpectations();
            }
            PLASSERT(negative_phase_visible_samples);
            PLASSERT(negative_phase_hidden_expectations &&
                     negative_phase_hidden_expectations_is_output);
            PLASSERT(negative_phase_hidden_activations &&
                     negative_phase_hidden_activations_is_output);
            negative_phase_visible_samples->resize(mbs,visible_layer->size);
            if (deterministic_reconstruction_in_cd)
               *negative_phase_visible_samples <<
                   visible_layer->getExpectations();
            else
               *negative_phase_visible_samples << visible_layer->samples;

            negative_phase_hidden_expectations->resize(
                hidden_expectations.length(),
                hidden_expectations.width());
            *negative_phase_hidden_expectations << hidden_expectations;
            const Mat& neg_hidden_act = hidden_layer->activations;
            negative_phase_hidden_activations->resize(neg_hidden_act.length(),
                                                      neg_hidden_act.width());
            *negative_phase_hidden_activations << neg_hidden_act;

            contrastive_divergence->resize(hidden_expectations.length(),1);
            // compute contrastive divergence itself
            for (int i=0;i<mbs;i++)
            {
                // + Free energy of positive example
                // - free energy of negative example
                (*contrastive_divergence)(i,0) =
                    visible_layer->energy((*visible)(i))
                  + hidden_layer->freeEnergyContribution((*h_act)(i))
                  - visible_layer->energy(visible_layer->samples(i))
                  - hidden_layer->freeEnergyContribution(hidden_layer->activations(i));
            }
        }
        else
            PLERROR("RBMModule: unknown configuration to compute contrastive_divergence (currently\n"
                    "only possible if only visible are provided in input).\n");
        found_a_valid_configuration = true;
    }

    if (compare_true_gradient_with_cd) {
        PLCHECK_MSG(!partition_function_is_stale,
                "The partition function must be computed for the comparison "
                "between true gradient and contrastive divergence to work.");
        PLCHECK_MSG(visible && !visible_is_output, "Visible must be as input");
        // Compute P(x_t|x) for all t and inputs x.
        int n_visible_conf = visible_layer->getConfigurationCount();
        int n_hidden_conf = hidden_layer->getConfigurationCount();
        p_xt_given_x.resize(n_visible_conf, visible->length());
        p_ht_given_x.resize(n_hidden_conf, visible->length());
        Vec input(visible_layer->size);
        Mat input_mat = input.toMat(1, input.length());
        Mat grad_nll(hidden_layer->size, visible_layer->size);
        Mat grad_cd(hidden_layer->size, visible_layer->size);
        Mat grad_stoch_cd(hidden_layer->size, visible_layer->size);
        Mat grad_first_term(hidden_layer->size, visible_layer->size);
        grad_nll.fill(0);
        if (median_reldiff_cd_nll_is_output)
            median_reldiff_cd_nll->resize(visible->length(), n_steps_compare);
        if (mean_diff_cd_nll_is_output)
            mean_diff_cd_nll->resize(visible->length(), n_steps_compare);
        if (agreement_cd_nll_is_output)
            agreement_cd_nll->resize(visible->length(), 2 * n_steps_compare);
        if (agreement_stoch_is_output)
            agreement_stoch->resize(visible->length(), n_steps_compare);
        real bound_coeff = MISSING_VALUE;
        if (bound_cd_nll_is_output || weights_stats_is_output) {
            if (bound_cd_nll_is_output)
                bound_cd_nll->resize(visible->length(), n_steps_compare);
            if (weights_stats_is_output)
                weights_stats->resize(visible->length(), 4);
            if (ratio_cd_leftout_is_output)
                ratio_cd_leftout->resize(visible->length(), n_steps_compare);
            if (abs_cd_is_output)
                abs_cd->resize(visible->length(), 2 * n_steps_compare);
            if (nll_grad_is_output)
                nll_grad->resize(visible->length(),
                        visible_layer->size * hidden_layer->size);
            // Compute main bound coefficient:
            // (1 - N_x N_h sigm(-alpha)^d_x sigm(-beta)^d_h).
            PP<RBMMatrixConnection> matrix_conn =
                (RBMMatrixConnection*) get_pointer(connection);
            PLCHECK(matrix_conn);
            Vec all_abs_weights_and_biases;
            // Compute alpha.
            real alpha = 0;
            for (int j = 0; j < hidden_layer->size; j++) {
                real alpha_j = abs(hidden_layer->bias[j]);
                all_abs_weights_and_biases.append(alpha_j);
                for (int i = 0; i < visible_layer->size; i++) {
                    real abs_w_ij = abs(matrix_conn->weights(j, i));
                    alpha_j += abs_w_ij;
                    all_abs_weights_and_biases.append(abs_w_ij);
                }
                if (alpha_j > alpha)
                    alpha = alpha_j;
            }
            // Compute beta.
            real beta = 0;
            for (int i = 0; i < visible_layer->size; i++) {
                real beta_i = abs(visible_layer->bias[i]);
                all_abs_weights_and_biases.append(beta_i);
                for (int j = 0; j < hidden_layer->size; j++)
                    beta_i += abs(matrix_conn->weights(j, i));
                if (beta_i > beta)
                    beta = beta_i;
            }
            bound_coeff = 1 -
                (visible_layer->getConfigurationCount() *
                    ipow(sigmoid(-alpha), visible_layer->size)) *
                (hidden_layer->getConfigurationCount() *
                    ipow(sigmoid(-beta), hidden_layer->size));
            //pout << "bound_coeff = " << bound_coeff << endl;
            if (weights_stats_is_output) {
                real med_weight = median(all_abs_weights_and_biases);
                real mean_weight = mean(all_abs_weights_and_biases);
                for (int i = 0; i < visible->length(); i++) {
                    (*weights_stats)(i, 0) = med_weight;
                    (*weights_stats)(i, 1) = mean_weight;
                    (*weights_stats)(i, 2) = alpha;
                    (*weights_stats)(i, 3) = beta;
                }
            }
        }
        for (int i = 0; i < visible->length(); i++) {
            // Compute dF(visible)/dWij.
            PLASSERT_MSG( visible->length() == 1, "The comparison can "
                    "currently be made only with one input example at a "
                    "time" );
            computeHiddenActivations(*visible);
            hidden_layer->computeExpectations();
            transposeProduct(grad_first_term,
                    hidden_layer->getExpectations(),
                    *visible);
            // First compute P(h|x) for inputs x.
            computeAllHiddenProbabilities(*visible, p_ht_given_x);
            for (int t = 0; t < n_steps_compare; t++) {
                // Compute P(x_t|x).
                product(p_xt_given_x, all_visible_cond_prob, p_ht_given_x);
                /*
                pout << "P(x_" << (t + 1) << "|x) = " << endl << p_xt_given_x
                     << endl;
                     */
                Vec colsum(p_xt_given_x.width());
                columnSum(p_xt_given_x, colsum);
                for (int j = 0; j < colsum.length(); j++) {
                    PLCHECK( is_equal(colsum[j], 1) );
                }
                //pout << "Sum = " << endl << colsum << endl;
                int best_idx = argmax(p_xt_given_x.column(0).toVecCopy());
                Vec tmp(visible_layer->size);
                visible_layer->getConfiguration(best_idx, tmp);
                /*
                pout << "Best (P = " << p_xt_given_x.column(0)(best_idx, 0) <<
                    ") for x = " << (*visible)(0) << ":" <<
                    endl << tmp << endl;
                */
                int stoch_idx = -1;
                if (abs_cd_is_output) {
                    grad_stoch_cd.fill(0);
                    // Pick a random X_t drawn from X_t | x.
                    stoch_idx = random_gen->multinomial_sample(
                            p_xt_given_x.toVecCopy());
                }
                // Compute E_{X_t}[dF(X_t)/dWij | x].
                grad_cd.fill(0);
                for (int k = 0; k < n_visible_conf; k++) {
                    visible_layer->getConfiguration(k, input);
                    computeHiddenActivations(input_mat);
                    hidden_layer->computeExpectations();
                    transposeProductScaleAcc(grad_cd,
                                             hidden_layer->getExpectations(),
                                             input_mat,
                                             -p_xt_given_x(k, 0),
                                             real(1));
                    if (t == 0) {
                        // Also compute the gradient for the NLL.
                        transposeProductScaleAcc(
                                grad_nll,
                                hidden_layer->getExpectations(),
                                input_mat,
                                -all_p_visible[k],
                                real(1));
                    }
                    if (k == stoch_idx) {
                        transposeProduct(grad_stoch_cd,
                                hidden_layer->getExpectations(),
                                input_mat);
                        negateElements(grad_stoch_cd);
                    }
                }
                // Compute difference between CD and NLL updates.
                Mat diff = grad_nll.copy();
                diff -= grad_cd;
                grad_cd += grad_first_term;
                if (abs_cd_is_output) {
                    grad_stoch_cd += grad_first_term;
                }
                //pout << "Grad_CD_" << t+1 << "=" << endl << grad_cd << endl;
                //pout << "Diff =" << endl << diff << endl;
                // Compute average relative difference.
                Vec all_relative_diffs;
                Vec all_abs_diffs;
                Vec all_ratios;
                for (int p = 0; p < diff.length(); p++)
                    for (int q = 0; q < diff.width(); q++) {
                        all_abs_diffs.append(abs(diff(p, q)));
                        if (!fast_exact_is_equal(grad_nll(p, q), 0))
                            all_relative_diffs.append(abs(diff(p, q) / grad_nll(p, q)));
                        if (!fast_exact_is_equal(diff(p, q), 0))
                            all_ratios.append(abs(grad_cd(p, q) / diff(p, q)));
                    }
                //pout << "All relative diffs: " << all_relative_diffs << endl;
                (*median_reldiff_cd_nll)(i, t) = median(all_relative_diffs);
                (*mean_diff_cd_nll)(i, t) = mean(all_abs_diffs);
                // Compute the fraction of parameters for which both updates
                // agree.
                int agree = 0;
                int agree2 = 0;
                int agree_stoch = 0;
                real mean_abs_updates = 0;
                real mean_abs_stoch_updates = 0;
                for (int p = 0; p < grad_cd.length(); p++)
                    for (int q = 0; q < grad_cd.width(); q++) {
                        if (grad_cd(p, q) *
                                (grad_first_term(p, q) + grad_nll(p, q)) >= 0)
                        {
                            agree++;
                        }
                        if (grad_cd(p, q) * diff(p, q) >= 0)
                            agree2++;
                        if (abs_cd_is_output) {
                            mean_abs_updates += abs(grad_cd(p, q));
                            mean_abs_stoch_updates += abs(grad_stoch_cd(p, q));
                        }
                        if (agreement_stoch_is_output &&
                                grad_stoch_cd(p, q) *
                                (grad_first_term(p, q) + grad_nll(p, q)) >= 0)
                        {
                            agree_stoch++;
                        }
                    }
                mean_abs_updates /= real(grad_cd.size());
                mean_abs_stoch_updates /= real(grad_cd.size());
                if (agreement_cd_nll_is_output) {
                    (*agreement_cd_nll)(i, t) = agree / real(grad_cd.size());
                    (*agreement_cd_nll)(i, t + n_steps_compare) =
                        agree2 / real(grad_cd.size());
                }
                if (agreement_stoch_is_output)
                    (*agreement_stoch)(i, t) = agree_stoch / real(grad_cd.size());
                if (bound_cd_nll_is_output)
                    (*bound_cd_nll)(i, t) =
                        visible_layer->getConfigurationCount() *
                        ipow(bound_coeff, t + 1);
                if (ratio_cd_leftout_is_output) {
                    if (all_ratios.isEmpty())
                        (*ratio_cd_leftout)(i, t) = MISSING_VALUE;
                    else
                        (*ratio_cd_leftout)(i, t) = median(all_ratios);
                }
                if (abs_cd_is_output) {
                    (*abs_cd)(i, t) = mean_abs_updates;
                    (*abs_cd)(i, t + n_steps_compare) = mean_abs_stoch_updates;
                }
                /*
                pout << "Median relative difference: "
                    << median(all_relative_diffs) << endl;
                pout << "Mean relative difference: "
                    << mean(all_relative_diffs) << endl;
                    */
                // If it is not the last step, update P(h_t|x).
                if (t < n_steps_compare - 1)
                    product(p_ht_given_x, all_hidden_cond_prob, p_xt_given_x);
            }
            //pout << "P(x)=" << endl << all_p_visible << endl;
            grad_nll += grad_first_term;
            if (nll_grad_is_output) {
                //real mean_nll_grad = 0;
                int idx = 0;
                for (int p = 0; p < grad_nll.length(); p++)
                    for (int q = 0; q < grad_nll.width(); q++, idx++)
                        (*nll_grad)(i, idx) = grad_nll(p, q);
                        //mean_nll_grad += abs(grad_nll(p, q));
                //mean_nll_grad /= real(grad_nll.size());
                //(*nll_grad)(i, 0) = mean_nll_grad;
            }
            //pout << "Grad_NLL=" << endl << grad_nll << endl;
            //pout << "Grad first term=" << endl << grad_first_term << endl;
        }
    }

    // Fill ports that are skipped during training with missing values.
    if (median_reldiff_cd_nll_is_output && median_reldiff_cd_nll->isEmpty()) {
        PLASSERT( during_training );
        median_reldiff_cd_nll->resize(visible->length(), n_steps_compare);
        median_reldiff_cd_nll->fill(MISSING_VALUE);
    }
    if (mean_diff_cd_nll_is_output && mean_diff_cd_nll->isEmpty()) {
        PLASSERT( during_training );
        mean_diff_cd_nll->resize(visible->length(), n_steps_compare);
        mean_diff_cd_nll->fill(MISSING_VALUE);
    }
    if (agreement_cd_nll_is_output && agreement_cd_nll->isEmpty()) {
        PLASSERT( during_training );
        agreement_cd_nll->resize(visible->length(), 2 * n_steps_compare);
        agreement_cd_nll->fill(MISSING_VALUE);
    }
    if (agreement_stoch_is_output && agreement_stoch->isEmpty()) {
        PLASSERT( during_training );
        agreement_stoch->resize(visible->length(), n_steps_compare);
        agreement_stoch->fill(MISSING_VALUE);
    }
    if (bound_cd_nll_is_output && bound_cd_nll->isEmpty()) {
        PLASSERT( during_training );
        bound_cd_nll->resize(visible->length(), n_steps_compare);
        bound_cd_nll->fill(MISSING_VALUE);
    }
    if (weights_stats_is_output && weights_stats->isEmpty()) {
        PLASSERT( during_training );
        weights_stats->resize(visible->length(), 4);
        weights_stats->fill(MISSING_VALUE);
    }
    if (ratio_cd_leftout_is_output && ratio_cd_leftout->isEmpty()) {
        PLASSERT( during_training );
        ratio_cd_leftout->resize(visible->length(), n_steps_compare);
        ratio_cd_leftout->fill(MISSING_VALUE);
    }
    if (abs_cd_is_output && abs_cd->isEmpty()) {
        PLASSERT( during_training );
        abs_cd->resize(visible->length(), 2 * n_steps_compare);
        abs_cd->fill(MISSING_VALUE);
    }
    if (nll_grad_is_output && nll_grad->isEmpty()) {
        PLASSERT( during_training );
        nll_grad->resize(visible->length(),
                         visible_layer->size * hidden_layer->size);
        nll_grad->fill(MISSING_VALUE);
    }

    // UGLY HACK TO DEAL WITH THE PROBLEM THAT XXX.state MAY NOT BE NEEDED
    // BUT IS ALWAYS EXPECTED BECAUSE IT IS A STATE (!@#$%!!!)
    if (hidden_act && hidden_act->isEmpty())
        hidden_act->resize(1,1);
    if (visible_activation && visible_activation->isEmpty())
        visible_activation->resize(1,1);
    if (hidden && hidden->isEmpty())
        hidden->resize(1,1);
    if (visible_reconstruction && visible_reconstruction->isEmpty())
        visible_reconstruction->resize(1,1);
    if (visible_reconstruction_activations && visible_reconstruction_activations->isEmpty())
        visible_reconstruction_activations->resize(1,1);
    if (reconstruction_error && reconstruction_error->isEmpty())
        reconstruction_error->resize(1,1);
    if (negative_phase_visible_samples && negative_phase_visible_samples->isEmpty())
        negative_phase_visible_samples->resize(1,1);
    if (negative_phase_hidden_expectations && negative_phase_hidden_expectations->isEmpty())
        negative_phase_hidden_expectations->resize(1,1);
    if (negative_phase_hidden_activations && negative_phase_hidden_activations->isEmpty())
        negative_phase_hidden_activations->resize(1,1);

    // Reset some class fields to ensure they are not reused by mistake.
    hidden_act = NULL;
    hidden_bias = NULL;
    weights = NULL;
    hidden_activations_are_computed = false;


    if (!found_a_valid_configuration)
    {
        PLERROR("In RBMModule::fprop - Unknown port configuration for module %s", name.c_str());
    }

    checkProp(ports_value);

}

void RBMModule::computeNegLogPVisibleGivenPHidden(Mat visible, Mat hidden, Mat* neg_log_phidden, Mat& neg_log_pvisible_given_phidden)
{
    computeVisibleActivations(hidden,true);
    int n_h = hidden.length();
    int T = visible.length();
    real default_neg_log_ph = safelog(real(n_h)); // default P(h)=1/Nh: -log(1/Nh) = log(Nh)
    Vec old_act = visible_layer->activation;
    neg_log_pvisible_given_phidden.resize(T,1);
    for (int t=0;t<T;t++)
    {
        Vec x_t = visible(t);
        real log_p_xt=0;
        for (int i=0;i<n_h;i++)
        {
            visible_layer->activation = visible_layer->activations(i);
            real neg_log_p_xt_given_hi = visible_layer->fpropNLL(x_t);
            real neg_log_p_hi = neg_log_phidden?(*neg_log_phidden)(i,0):default_neg_log_ph;
            if (i==0)
                log_p_xt = -(neg_log_p_xt_given_hi + neg_log_p_hi);
            else
                log_p_xt = logadd(log_p_xt, -(neg_log_p_xt_given_hi + neg_log_p_hi));
        }
        neg_log_pvisible_given_phidden(t,0) = -log_p_xt;
    }
    visible_layer->activation = old_act;
}

////////////////////
// bpropAccUpdate //
////////////////////
void RBMModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                               const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_value.length() == nPorts() );
    PLASSERT( ports_gradient.length() == nPorts() );
    Mat* visible = ports_value[getPortIndex("visible")];
    Mat* visible_grad = ports_gradient[getPortIndex("visible")];
    Mat* hidden_grad = ports_gradient[getPortIndex("hidden.state")];
    Mat* hidden_activations_grad =
        ports_gradient[getPortIndex("hidden_activations.state")];
    Mat* hidden = ports_value[getPortIndex("hidden.state")];
    hidden_act = ports_value[getPortIndex("hidden_activations.state")];
    Mat* visible_activations = ports_value[getPortIndex("visible_activations.state")];
    Mat* reconstruction_error_grad = 0;
    Mat* hidden_bias_grad = ports_gradient[getPortIndex("hidden_bias")];
    weights = ports_value[getPortIndex("weights")];
    Mat* weights_grad = ports_gradient[getPortIndex("weights")];
    hidden_bias = ports_value[getPortIndex("hidden_bias")];
    Mat* energy_grad = ports_gradient[getPortIndex("energy")];
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
//    PLASSERT_MSG( !visible_grad || visible_grad->isEmpty(), "If visible gradient is desired "
//                  " the corresponding matrix should have 0 length" );

    bool compute_visible_grad = visible_grad && visible_grad->isEmpty();
    bool compute_hidden_grad = hidden_grad && hidden_grad->isEmpty();
    bool compute_weights_grad = weights_grad && weights_grad->isEmpty();
    bool provided_hidden_grad = hidden_grad && !hidden_grad->isEmpty();
    bool provided_hidden_act_grad = hidden_activations_grad &&
                                    !hidden_activations_grad->isEmpty();

    int mbs = (visible && !visible->isEmpty()) ? visible->length() : -1;

    // BPROP of UPWARD FPROP
    if (provided_hidden_grad || provided_hidden_act_grad)
    {
        // Note: the assert below is for behavior compatibility with previous
        // code. It might not be necessary, or might need to be modified.
        PLASSERT( visible && !visible->isEmpty() );

        // Note: we need to perform the following steps even if the gradient
        // learning rate is equal to 0. This is because we must propagate the
        // gradient to the visible layer, even though no update is required.
        if (tied_connection_weights)
           setLearningRatesOnlyForLayers(grad_learning_rate);
        else
           setAllLearningRates(grad_learning_rate);

        PLASSERT_MSG( hidden && hidden_act ,
                      "To compute gradients in bprop, the "
                      "hidden_activations.state port must have been filled "
                      "during fprop" );

        // Compute gradient w.r.t. activations of the hidden layer.
        if (provided_hidden_grad)
            hidden_layer->bpropUpdate(
                    *hidden_act, *hidden, hidden_act_grad, *hidden_grad,
                    false);
        if (provided_hidden_act_grad) {
            if (!provided_hidden_grad) {
                // 'hidden_act_grad' will not have been resized nor filled yet,
                // so we need to do it now.
                hidden_act_grad.resize(hidden_activations_grad->length(),
                                       hidden_activations_grad->width());
                hidden_act_grad.clear();
            }
            hidden_act_grad += *hidden_activations_grad;
        }

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

    // BPROP of DOWNWARD FPROP
    if (compute_hidden_grad && visible_grad && !compute_visible_grad)
    {
        PLASSERT(visible && !visible->isEmpty());
        PLASSERT(visible_activations && !visible_activations->isEmpty());
        PLASSERT(hidden && !hidden->isEmpty());
        setAllLearningRates(grad_learning_rate);
        visible_layer->bpropUpdate(*visible_activations,
                                   *visible, visible_act_grad, *visible_grad,
                                   false);

//        PLASSERT_MSG(!visible_bias_grad,"back-prop into visible bias  not implemented for downward fprop");
//        PLASSERT_MSG(!weights_grad,"back-prop into weights  not implemented for downward fprop");
//        hidden_grad->resize(mbs,hidden_layer->size);
        TVec<Mat*> ports_value(2);
        TVec<Mat*> ports_gradient(2);
        ports_value[0] = visible_activations;
        ports_value[1] = hidden;
        ports_gradient[0] = &visible_act_grad;
        ports_gradient[1] = hidden_grad;
        connection->bpropAccUpdate(ports_value,ports_gradient);
    }

    if (cd_learning_rate > 0 && minimize_log_likelihood) {
        PLASSERT( visible && !visible->isEmpty() );
        PLASSERT( hidden && !hidden->isEmpty() );
        if (tied_connection_weights)
           setLearningRatesOnlyForLayers(cd_learning_rate);
        else
           setAllLearningRates(cd_learning_rate);

        // positive phase
        visible_layer->accumulatePosStats(*visible);
        hidden_layer->accumulatePosStats(*hidden);
        connection->accumulatePosStats(*visible,*hidden);

        // negative phase
        PLCHECK_MSG(hidden_layer->size<32 || visible_layer->size<32,
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
        if (tied_connection_weights)
           setLearningRatesOnlyForLayers(cd_learning_rate);
        else
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

        Mat vis_expect_ptr;
        if (!negative_phase_visible_samples)
        {
            // Generate hidden samples.
            hidden_layer->setExpectations(*hidden);
            for( int i=0; i<n_Gibbs_steps_CD; i++)
            {
                hidden_layer->generateSamples();
                if (deterministic_reconstruction_in_cd)
                {
                   // (Negative phase) compute visible expectations
                   computeVisibleActivations(hidden_layer->samples);
                   visible_layer->computeExpectations();
                   // compute corresponding hidden expectations.
                   computeHiddenActivations(visible_layer->getExpectations());
                }
                else // classical CD learning
                {
                   // (Negative phase) Generate visible samples.
                   sampleVisibleGivenHidden(hidden_layer->samples);
                   // compute corresponding hidden expectations.
                   computeHiddenActivations(visible_layer->samples);
                }
                hidden_layer->computeExpectations();
            }
            PLASSERT( !computed_contrastive_divergence );
            PLASSERT( !negative_phase_hidden_expectations );
            PLASSERT( !negative_phase_hidden_activations );
            if (deterministic_reconstruction_in_cd) {
                vis_expect_ptr = visible_layer->getExpectations();
                negative_phase_visible_samples = &vis_expect_ptr;
            }
            else // classical CD learning
               negative_phase_visible_samples = &(visible_layer->samples);
            negative_phase_hidden_activations = &(hidden_layer->activations);
            negative_phase_hidden_expectations = &(hidden_layer->getExpectations());
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
            if (!standard_cd_grad && !tied_connection_weights) {
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
        if (tied_connection_weights)
           setLearningRatesOnlyForLayers(grad_learning_rate);
        else
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

        // UGLY HACK WHICH BREAKS THE RULE THAT RBMMODULE CAN BE CALLED IN DIFFERENT CONTEXTS AND fprop/bprop ORDERS
        // BUT NECESSARY WHEN hidden WAS AN INPUT
        if (hidden_is_output)
        {
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
                          (visible_grad->length() == 0 ||
                           visible_grad->length() == mbs) );
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
        }
        else if (hidden_grad && hidden_grad->isEmpty()) // copy the hidden gradient
        {
            hidden_grad->resize(mbs,hidden_layer->size);
            *hidden_grad << hidden_exp_grad;
        }

        partition_function_is_stale = true;
    }

    if (energy_grad && !energy_grad->isEmpty() &&
        visible_grad && visible_grad->isEmpty())
        // compute the gradient of the free-energy wrt input
    {
        // very cheap shot, specializing to the common case...
        PLASSERT(hidden_layer->classname()=="RBMBinomialLayer");
        PLASSERT(visible_layer->classname()=="RBMBinomialLayer" ||
                 visible_layer->classname()=="RBMGaussianlLayer");
        PLASSERT(connection->classname()=="RBMMatrixConnection");
        PLASSERT(hidden && !hidden->isEmpty());
        // FE(x) = -b'x - sum_i softplus(hidden_layer->activation[i])
        // dFE(x)/dx = -b - sum_i sigmoid(hidden_layer->activation[i]) W_i
        // dC/dxt = -b dC/dFE - dC/dFE sum_i p_ti W_i
        int mbs=energy_grad->length();
        visible_grad->resize(mbs,visible_layer->size);
        Mat& weights = ((RBMMatrixConnection*)
                        get_pointer(connection))->weights;
        bool same_dC_dFE=true;
        real dC_dFE=(*energy_grad)(0,0);
        const Mat& p = *hidden;
        for (int t=0;t<mbs;t++)
        {
            real new_dC_dFE=(*energy_grad)(t,0);
            if (new_dC_dFE!=dC_dFE)
                same_dC_dFE=false;
            dC_dFE = new_dC_dFE;
            multiplyAcc((*visible_grad)(t),visible_layer->bias,-dC_dFE);
        }
        if (same_dC_dFE)
            productScaleAcc(*visible_grad, p, false, weights, false, -dC_dFE,
                            real(1));
        else
            for (int t=0;t<mbs;t++)
                productScaleAcc((*visible_grad)(t), weights, true, p(t),
                        -(*energy_grad)(t, 0), real(1));
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
    if (reconstruction_connection && reconstruction_connection != connection)
        // We avoid to call forget() twice if the connections are the same.
        reconstruction_connection->forget();
    Gibbs_step = 0;
    partition_function_is_stale = true;
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

void RBMModule::setLearningRatesOnlyForLayers(real lr)
{
    hidden_layer->setLearningRate(lr);
    visible_layer->setLearningRate(lr);
    connection->setLearningRate(0.);
    if(reconstruction_connection)
        reconstruction_connection->setLearningRate(0.);
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
