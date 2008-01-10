// -*- C++ -*-

// DiscriminativeRBM.h
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

/*! \file DiscriminativeRBM.h */

#ifndef DiscriminativeRBM_INC
#define DiscriminativeRBM_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/CostModule.h>
#include <plearn_learners/online/NLLCostModule.h>
#include <plearn_learners/online/RBMClassificationModule.h>
#include <plearn_learners/online/RBMLayer.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMConnection.h>
#include <plearn/misc/PTimer.h>
#include <plearn/sys/Profiler.h>

namespace PLearn {
using namespace std;

/**
 * Discriminative Restricted Boltzmann Machine classifier
 */
class DiscriminativeRBM : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! The learning rate used for discriminative learning
    real disc_learning_rate;

    //! The decrease constant of the discriminative learning rate
    real disc_decrease_ct;

    //! Indication that the exact gradient should be used for
    //! discriminative learning (instead of the CD gradient)
    bool use_exact_disc_gradient;

    //! The weight of the generative learning term, for
    //! hybrid discriminative/generative learning
    real gen_learning_weight;

    //! Indication that multi-conditional learning should
    //! be used instead of generative learning
    bool use_multi_conditional_learning;

    //! The weight of the semi-supervised learning term, for
    //! unsupervised learning on unlabeled data
    real semi_sup_learning_weight;

    //! Number of classes in the training set
    int n_classes;

    //! The input layer of the RBM
    PP<RBMLayer> input_layer;

    //! The hidden layer of the RBM
    PP<RBMBinomialLayer> hidden_layer;

    //! The connection weights between the input and hidden layer
    PP<RBMConnection> connection;

    //! Target weights' L1_penalty_factor
    real target_weights_L1_penalty_factor;

    //! Target weights' L2_penalty_factor
    real target_weights_L2_penalty_factor;

    //#####  Public Learnt Options  ###########################################
    //! The module computing the probabilities of the different classes.
    PP<RBMClassificationModule> classification_module;

    //! The computed cost names
    TVec<string> cost_names;

    //! The module computing the classification cost function (NLL) on top of
    //! classification_module.
    PP<NLLCostModule> classification_cost;

    //! Concatenation of input_layer and the target layer (that is
    //! inside classification_module)
    PP<RBMMixedLayer> joint_layer;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    DiscriminativeRBM();


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
    PLEARN_DECLARE_OBJECT(DiscriminativeRBM);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    //#####  Not Options  #####################################################

    //! Matrix connection weights between the hidden layer and the target layer
    //! (pointer to classification_module->last_to_target)
    PP<RBMMatrixConnection> last_to_target;

    //! Connection weights between the hidden layer and the target layer
    //! (pointer to classification_module->last_to_target)
    PP<RBMConnection> last_to_target_connection;

    //! Connection weights between the hidden layer and the visible layer
    //! (pointer to classification_module->joint_connection)
    PP<RBMConnection> joint_connection;

    //! Part of the RBM visible layer corresponding to the target
    //! (pointer to classification_module->target_layer)
    PP<RBMMultinomialLayer> target_layer;

    //! Temporary variables for Contrastive Divergence
    mutable Vec target_one_hot;

    mutable Vec disc_pos_down_val;
    mutable Vec disc_pos_up_val;
    mutable Vec disc_neg_down_val;
    mutable Vec disc_neg_up_val;

    mutable Vec gen_pos_down_val;
    mutable Vec gen_pos_up_val;
    mutable Vec gen_neg_down_val;
    mutable Vec gen_neg_up_val;

    mutable Vec semi_sup_pos_down_val;
    mutable Vec semi_sup_pos_up_val;
    mutable Vec semi_sup_neg_down_val;
    mutable Vec semi_sup_neg_up_val;  

    //! Temporary variables for gradient descent
    mutable Vec input_gradient;
    mutable Vec class_output;
    mutable Vec class_gradient;

    //! Keeps the index of the NLL cost in train_costs
    int nll_cost_index;

    //! Keeps the index of the class_error cost in train_costs
    int class_cost_index;

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

    void build_classification_cost();

    void setLearningRate( real the_learning_rate );

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(DiscriminativeRBM);

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
