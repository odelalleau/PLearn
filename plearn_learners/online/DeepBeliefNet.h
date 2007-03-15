// -*- C++ -*-

// DeepBeliefNet.h
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

/*! \file DeepBeliefNet.h */


#ifndef DeepBeliefNet_INC
#define DeepBeliefNet_INC

#include <plearn_learners/generic/PLearner.h>
#include "OnlineLearningModule.h"
#include "CostModule.h"
#include "NLLCostModule.h"
#include "RBMClassificationModule.h"
#include "RBMLayer.h"
#include "RBMMixedLayer.h"
#include "RBMConnection.h"
#include <plearn/misc/PTimer.h>

namespace PLearn {

/**
 * Neural net, learned layer-wise in a greedy fashion.
 * This version support different unit types, different connection types,
 * and different cost functions, including the NLL in classification.
 */
class DeepBeliefNet : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! The learning rate used during contrastive divergence learning
    real cd_learning_rate;

    //! The learning rate used during the gradient descent
    real grad_learning_rate;

    //! The decrease constant of the learning rate used during gradient descent
    real grad_decrease_ct;

    /* NOT IMPLEMENTED YET
    //! The weight decay used during the gradient descent
    real grad_weight_decay;
    */

    //! Number of classes in the training set
    //!   - 0 means we are doing regression,
    //!   - 1 means we have two classes, but only one output,
    //!   - 2 means we also have two classes, but two outputs summing to 1,
    //!   - >2 is the usual multiclass case.
    int n_classes;

    //! Number of examples to use during each phase of learning:
    //! first the greedy phases, and then the gradient descent
    TVec<int> training_schedule;

    //! If the first cost function is the NLL in classification,
    //! pre-trained with CD, and using the last *two* layers to get a better
    //! approximation (undirected softmax) than layer-wise mean-field.
    bool use_classification_cost;

    //! Minimize reconstruction error of each layer as an auto-encoder.
    //! This is done using cross-entropy between actual and reconstructed.
    //! This option automatically adds the following cost names:
    //! layerwise_reconstruction_error (sum over all layers)
    //!   layer1_reconstruction_error (only layer 1)
    //!   layer2_reconstruction_error (only layer 2)
    //!   etc.
    bool reconstruct_layerwise;

    //! The layers of units in the network
    TVec< PP<RBMLayer> > layers;

    //! The weights of the connections between the layers
    TVec< PP<RBMConnection> > connections;

    //! Optional module that takes as input the output of the last layer
    //! (layers[n_layers-1), and its output is fed to final_cost, and
    //! concatenated with the one of classification_cost (if present) as output
    //! of the learner.
    //! If it is not provided, then the last layer will directly be put as
    //! input of final_cost.
    PP<OnlineLearningModule> final_module;

    //! The cost function to be applied on top of the DBN (or of final_module
    //! if provided). Its gradients will be backpropagated to final_module,
    //! then combined with the one of classification_cost and backpropagated to
    //! the layers.
    PP<CostModule> final_cost;

    //! The different cost function to be applied on top of each layer
    //! (except the first one, which has to be null) of the RBM. These
    //! costs are not back-propagated to previous layers.
    TVec< PP<CostModule> > partial_costs;

    //#####  Public Learnt Options  ###########################################
    //! The module computing the probabilities of the different classes.
    PP<RBMClassificationModule> classification_module;

    //! Number of layers
    int n_layers;

    //! whether to do things by stages, including fine-tuning, or on-line
    bool online;

    //! Wether we do a step of joint contrastive divergence on top-layer
    //! Only used if online for the moment
    bool top_layer_joint_cd;

    //#####  Not Options  #####################################################

    //! Timer for monitoring the speed
    PP<PTimer> timer;

    //! The module computing the classification cost function (NLL) on top of
    //! classification_module.
    PP<NLLCostModule> classification_cost;

    //! Concatenation of layers[n_layers-2] and the target layer (that is
    //! inside classification_module), if use_classification_cost
    PP<RBMMixedLayer> joint_layer;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    DeepBeliefNet();


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


    void onlineStep( const Vec& input, const Vec& target, Vec& train_costs );

    void greedyStep( const Vec& input, const Vec& target, int index );

    void jointGreedyStep( const Vec& input, const Vec& target );

    void fineTuningStep( const Vec& input, const Vec& target,
                         Vec& train_costs );

    void contrastiveDivergenceStep( const PP<RBMLayer>& down_layer,
                                    const PP<RBMConnection>& connection,
                                    const PP<RBMLayer>& up_layer,
                                    bool nofprop=false);


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
    PLEARN_DECLARE_OBJECT(DeepBeliefNet);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################

    //! Stores the gradient of the cost wrt the activations
    //! (at the input of the layers)
    mutable TVec<Vec> activation_gradients;

    //! Stores the gradient of the cost wrt the expectations
    //! (at the output of the layers)
    mutable TVec<Vec> expectation_gradients;

    //!
    mutable Vec final_cost_input;

    mutable Vec final_cost_value;

    mutable Vec final_cost_output;
    //!
    mutable Vec class_output;

    mutable Vec class_gradient;

    mutable Vec class_input_gradient;

    //! Stores the gradient of the cost at the input of final_cost
    mutable Vec final_cost_gradient;

    //! buffers bottom layer activation during onlineStep 
    mutable Vec save_layer_activation;

    //! buffers bottom layer expectation during onlineStep 
    mutable Vec save_layer_expectation;

    //! Does final_module exist and have a "learning_rate" option
    bool final_module_has_learning_rate;

    //! Does final_cost exist and have a "learning_rate" option
    bool final_cost_has_learning_rate;

    //! Store a copy of the positive phase values
    mutable Vec pos_down_values;
    mutable Vec pos_up_values;

    //! Keeps the index of the NLL cost in train_costs
    int nll_cost_index;

    //! Keeps the index of the CLASS cost in train_costs
    int class_cost_index;

    //! Keeps the indices of the final costs in train_costs
    TVec<int> final_cost_indices;

    //! Keeps the index of the reconstruction cost in train_costs
    int recons_cost_index;

    //! indices of the partial costs in train_costs
    TVec<TVec<int> > partial_cost_indices;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

    void build_layers_and_connections();

    void build_classification_cost();

    void build_final_cost();

    void setLearningRate( real the_learning_rate );

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(DeepBeliefNet);

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
