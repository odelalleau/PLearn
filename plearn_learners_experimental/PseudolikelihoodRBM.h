// -*- C++ -*-

// PseudolikelihoodRBM.h
//
// Copyright (C) 2008 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file PseudolikelihoodRBM.h */

#ifndef PseudolikelihoodRBM_INC
#define PseudolikelihoodRBM_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/CostModule.h>
#include <plearn_learners/online/CrossEntropyCostModule.h>
#include <plearn_learners/online/NLLCostModule.h>
#include <plearn_learners/online/RBMClassificationModule.h>
#include <plearn_learners/online/RBMMatrixTransposeConnection.h>
#include <plearn_learners/online/RBMMultitaskClassificationModule.h>
#include <plearn_learners/online/RBMLayer.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMConnection.h>
#include <plearn/misc/PTimer.h>
#include <plearn/sys/Profiler.h>

namespace PLearn {
using namespace std;

/**
 * Restricted Boltzmann Machine trained by (generalized) pseudolikelihood
 */
class PseudolikelihoodRBM : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! The learning rate used for pseudolikelihood training
    real learning_rate;

    //! The decrease constant of the learning rate
    real decrease_ct;

    //! The learning rate used for contrastive divergence learning
    real cd_learning_rate;

    //! The decrease constant of the contrastive divergence learning rate
    real cd_decrease_ct;

    //! Number of negative phase gibbs sampling steps
    int cd_n_gibbs;

    //! Weight of Persistent Contrastive Divergence, i.e. weight of the
    //! prolonged gibbs chain
    real persistent_cd_weight;

    //! Number of gibbs chains maintained in parallel for 
    //! Persistent Contrastive Divergence
    int n_gibbs_chains;

    //! Indication that a mean-field version of Contrastive Divergence
    //! (MF-CD) should be used.
    bool use_mean_field_cd;

    //! The learning rate used for denoising autoencoder learning
    real denoising_learning_rate;

    //! The decrease constant of the denoising autoencoder learning rate
    real denoising_decrease_ct;

    //! Fraction of input components set to 0 for denoising autoencoder learning
    real fraction_of_masked_inputs;

    //! Number of classes in the training set (for supervised learning)
    int n_classes;
    
    //! Indication that the input space NLL should be computed
    //! during test
    bool compute_input_space_nll;

    //! Number of additional input variables chosen to form the joint
    //! condition likelihoods in generalized pseudolikelihood
    //! (default = 0, which corresponds to standard pseudolikelihood)
    int pseudolikelihood_context_size;

    //! Type of context for generalized pseudolikelihood:
    //! - "uniform_random": context elements are picked uniformly randomly
    //! 
    //! - "most_correlated": the most correlated (positively or negatively
    //!                      elemenst with the current input element are picked
    //!
    //! - "most_correlated_uniform_random": context elements are picked uniformly
    //!                                     among the k_most_correlated other input
    //!                                     elements, for each current input
    string pseudolikelihood_context_type;

    //! Number of most correlated input elements over which to sample
    real k_most_correlated;

    //! The binomial input layer of the RBM
    PP<RBMBinomialLayer> input_layer;

    //! The hidden layer of the RBM
    PP<RBMLayer> hidden_layer;

    //! The connection weights between the input and hidden layer
    PP<RBMMatrixConnection> connection;

    ////! Target weights' L1_penalty_factor
    //real target_weights_L1_penalty_factor;
    //
    ////! Target weights' L2_penalty_factor
    //real target_weights_L2_penalty_factor;

    //#####  Public Learnt Options  ###########################################
    //! The computed cost names
    TVec<string> cost_names;

    PP<RBMMatrixTransposeConnection> transpose_connection;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    PseudolikelihoodRBM();

    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    // (PLEASE IMPLEMENT IN .cc)
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void train();

    //! Computes the output from the input.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    // (PLEASE IMPLEMENT IN .cc)
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    // (PLEASE IMPLEMENT IN .cc)
    virtual TVec<std::string> getTrainCostNames() const;


    // *** SUBCLASS WRITING: ***
    // While in general not necessary, in case of particular needs
    // (efficiency concerns for ex) you may also want to overload
    // some of the following methods:
    // virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
    //                                    Vec& output, Vec& costs) const;
    // virtual void computeCostsOnly(const Vec& input, const Vec& target,
    //                               Vec& costs) const;
    // virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
    //                   VMat testoutputs=0, VMat testcosts=0) const;
    // virtual int nTestCosts() const;
    // virtual int nTrainCosts() const;
    // virtual void resetInternalState();
    // virtual bool isStatefulLearner() const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(PseudolikelihoodRBM);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    //#####  Not Options  #####################################################

    ////! Matrix connection weights between the hidden layer and the target layer
    ////! (pointer to classification_module->last_to_target)
    //PP<RBMMatrixConnection> last_to_target;
    //
    ////! Connection weights between the hidden layer and the target layer
    ////! (pointer to classification_module->last_to_target)
    //PP<RBMConnection> last_to_target_connection;
    //
    ////! Connection weights between the hidden layer and the visible layer
    ////! (pointer to classification_module->joint_connection)
    //PP<RBMConnection> joint_connection;
    //
    ////! Part of the RBM visible layer corresponding to the target
    ////! (pointer to classification_module->target_layer)
    //PP<RBMLayer> target_layer;

    //! Temporary variables for Contrastive Divergence
    mutable Vec target_one_hot;

    //! Temporary variables for RBM computations
    mutable Vec input_gradient;
    mutable Vec class_output;
    mutable Vec before_class_output;
    mutable Vec class_gradient;
    mutable Vec hidden_activation_pos_i;
    mutable Vec hidden_activation_neg_i;
    mutable Vec hidden_activation_gradient;
    mutable Vec hidden_activation_pos_i_gradient;
    mutable Vec hidden_activation_neg_i_gradient;
    mutable Mat connection_gradient;
    mutable TVec<int> context_indices;
    mutable TMat<int> context_indices_per_i;
    mutable Mat correlations_per_i;
    mutable TVec< TVec< int > > context_most_correlated;
    mutable Mat hidden_activations_context;
    mutable Vec hidden_activations_context_k_gradient;
    mutable Vec nums;
    mutable Vec nums_act;
    mutable Vec context_probs;
    mutable Vec gnums_act;
    mutable Vec conf;
    mutable Vec pos_input;
    mutable Vec pos_hidden;
    mutable Vec neg_input;
    mutable Vec neg_hidden;
    mutable Vec reconstruction_activation_gradient;
    mutable Vec hidden_layer_expectation_gradient;
    mutable Vec hidden_layer_activation_gradient;
    mutable Vec masked_autoencoder_input;
    mutable TVec<int> autoencoder_input_indices;
    mutable TVec<Vec> pers_cd_input;
    mutable TVec<Vec> pers_cd_hidden;

    //! Keeps the index of the NLL cost in train_costs
    int nll_cost_index;

    //! Keeps the index of the class_error cost in train_costs
    int class_cost_index;

    //! CPU time costs indices
    int training_cpu_time_cost_index;
    int cumulative_training_time_cost_index;
    //real cumulative_testing_time_cost_index;

    //! Cumulative CPU time costs
    real cumulative_training_time;
    //real cumulative_testing_time;
    
    //! Normalisation constant (on log scale)
    mutable real log_Z;

    //! Indication that the normalisation constant Z is up to date
    mutable bool Z_is_up_to_date;

    //! Indication that the prolonged gibbs chain for 
    //! Persistent Consistent Divergence is started, for each chain
    mutable TVec<bool> persistent_gibbs_chain_is_started;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

    void build_layers_and_connections();

    void build_costs();

    void setLearningRate( real the_learning_rate );

    void compute_Z() const;

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PseudolikelihoodRBM);

} // end of namespace PLearn

#endif


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
