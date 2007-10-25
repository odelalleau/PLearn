// -*- C++ -*-

// StackedFocusedAutoassociatorsNet.h
//
// Copyright (C) 2007 Hugo Larochelle
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

/*! \file StackedFocusedAutoassociatorsNet.h */


#ifndef StackedFocusedAutoassociatorsNet_INC
#define StackedFocusedAutoassociatorsNet_INC

#include <plearn/vmat/ClassSubsetVMatrix.h>
#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/online/GradNNetLayerModule.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/CostModule.h>
#include <plearn_learners/online/ModuleStackModule.h>
#include <plearn_learners/online/NLLCostModule.h>
#include <plearn_learners/online/ClassErrorCostModule.h>
#include <plearn_learners/online/CombiningCostsModule.h>
#include <plearn_learners/online/RBMClassificationModule.h>
#include <plearn_learners/online/RBMLayer.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMConnection.h>
#include <plearn_learners/online/SoftmaxModule.h>
#include <plearn/misc/PTimer.h>

namespace PLearn {

/**
 * Neural net, trained layer-wise in a greedy but focused fashion 
 * using autoassociators/RBMs and a supervised non-parametric gradient.
 * It is highly inspired by the StackedAutoassociators class, 
 * and can use use the same RBMLayer and RBMConnection components.
 */
class StackedFocusedAutoassociatorsNet : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! Contrastive divergence learning rate
    real cd_learning_rate;
    
    //! Contrastive divergence decrease constant
    real cd_decrease_ct;

    //! The learning rate used during the autoassociator gradient descent training
    real greedy_learning_rate;

    //! The decrease constant of the learning rate used during the autoassociator
    //! gradient descent training. When a hidden layer has finished its training,
    //! the learning rate is reset to it's initial value.
    real greedy_decrease_ct;

    //! Supervised, non-parametric, greedy learning rate
    real supervised_greedy_learning_rate;

    //! Supervised, non-parametric, greedy decrease constant
    real supervised_greedy_decrease_ct;

    //! The learning rate used during the fine tuning gradient descent
    real fine_tuning_learning_rate;

    //! The decrease constant of the learning rate used during fine tuning
    //! gradient descent
    real fine_tuning_decrease_ct;

    //! Number of examples to use during each phase of greedy pre-training.
    //! The number of fine-tunig steps is defined by nstages.
    TVec<int> training_schedule;

    //! The layers of units in the network
    TVec< PP<RBMLayer> > layers;

    //! The weights of the connections between the layers
    TVec< PP<RBMConnection> > connections;

    //! The reconstruction weights of the autoassociators
    TVec< PP<RBMConnection> > reconstruction_connections;

    //! Additional units for greedy unsupervised learning
    TVec< PP<RBMLayer> > unsupervised_layers;

    //! Additional connections for greedy unsupervised learning
    TVec< PP<RBMConnection> > unsupervised_connections;

    //! Number of good nearest neighbors to attract and
    //! bad nearest neighbors to repel.
    int k_neighbors;

    //! Number of classes
    int n_classes;

    //! Use standard neural net architecture, not 
    //! the nearest neighbor model.
    bool do_not_use_knn_classifier;

    //! Output weights l1_penalty_factor
    real output_weights_l1_penalty_factor;

    //! Output weights l2_penalty_factor
    real output_weights_l2_penalty_factor;

    //#####  Public Learnt Options  ###########################################

    //! Number of layers
    int n_layers;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    StackedFocusedAutoassociatorsNet();

    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    virtual void train();

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    /**
     *  Precomputes the representations of the training set examples, 
     *  to speed up nearest neighbors searches in that space.
     */
    virtual void updateTrainSetRepresentations() const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;

    /**
     *  Declares the training set.  Then calls build() and forget() if
     *  necessary.  Also sets this learner's inputsize_ targetsize_ weightsize_
     *  from those of the training_set.  Note: You shouldn't have to override
     *  this in subclasses, except in maybe to forward the call to an
     *  underlying learner.
     */
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    void greedyStep( const Vec& input, const Vec& target, int index, 
                     Vec train_costs, int stage, Vec similar_example,
                     Vec dissimilar_example);

    void fineTuningStep( const Vec& input, const Vec& target,
                         Vec& train_costs, Vec similar_example, 
                         Vec dissimilar_example );

    void computeRepresentation( const Vec& input, 
                                Vec& representation, int layer) const;

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(StackedFocusedAutoassociatorsNet);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################

    //! Stores the activations of the input and hidden layers
    //! (at the input of the layers)
    mutable TVec<Vec> activations;

    //! Stores the expectations of the input and hidden layers
    //! (at the output of the layers)
    mutable TVec<Vec> expectations;

    //! Stores the gradient of the cost wrt the activations of 
    //! the input and hidden layers
    //! (at the input of the layers)
    mutable TVec<Vec> activation_gradients;

    //! Stores the gradient of the cost wrt the expectations of 
    //! the input and hidden layers
    //! (at the output of the layers)
    mutable TVec<Vec> expectation_gradients;

    //! Stores the activation of the trained hidden layer during a greedy step
    mutable Vec greedy_activation;

    //! Stores the expectation of the trained hidden layer during a greedy step
    mutable Vec greedy_expectation;

    //! Stores the activation gradient of the trained 
    //! hidden layer during a greedy step
    mutable Vec greedy_activation_gradient;

    //! Stores the expectation gradient of the trained 
    //! hidden layer during a greedy step
    mutable Vec greedy_expectation_gradient;

    //! Reconstruction activations
    mutable Vec reconstruction_activations;
    
    //! Reconstruction activation gradients
    mutable Vec reconstruction_activation_gradients;

    //! Reconstruction expectation gradients
    mutable Vec reconstruction_expectation_gradients;

    //! Layers used for greedy learning
    TVec< PP<RBMLayer> > greedy_layers;

    //! Connections used for greedy learning
    TVec< PP<RBMConnection> > greedy_connections;

    //! Similar example representation
    Vec similar_example_representation;

    //! Dissimilar example representation
    Vec dissimilar_example_representation;

    //! Example representation
    mutable Vec input_representation;

    //! Example representation at the previous layer, in a greedy step
    Vec previous_input_representation;

    //! Dissimilar gradient contribution
    Vec dissimilar_gradient_contribution;

    //! Positive down statistic
    Vec pos_down_val;
    //! Positive up statistic
    Vec pos_up_val;
    //! Negative down statistic
    Vec neg_down_val;
    //! Negative up statistic
    Vec neg_up_val;

    //! Input of cost function
    mutable Vec final_cost_input;
    //! Cost value
    mutable Vec final_cost_value;
    //! Cost gradient on output layer
    mutable Vec final_cost_gradient;

    //! Datasets for each class
    TVec< PP<ClassSubsetVMatrix> > class_datasets;

    //! Proportions of examples from the other classes (columns), for each
    //! class (rows)
    Mat other_classes_proportions;

    //! Nearest neighbors for each training example
    TMat<int> nearest_neighbors_indices;

    //! Nearest neighbors for each test example
    mutable TVec<int> test_nearest_neighbors_indices;

    //! Nearest neighbor votes for test example
    TVec<int> test_votes;

    //! Data set mapped to last hidden layer space
    mutable Mat train_set_representations;
    mutable VMat train_set_representations_vmat;
    mutable TVec<int> train_set_targets;

    //! Indication that train_set_representations is up to date
    mutable bool train_set_representations_up_to_date;

    //! Stages of the different greedy phases
    TVec<int> greedy_stages;

    //! Currently trained layer (1 means the first hidden layer,
    //! n_layers means the output layer)
    int currently_trained_layer;

    //! Output layer of neural net
    PP<OnlineLearningModule> final_module;

    //! Cost on output layer of neural net
    PP<CostModule> final_cost;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

    void build_layers_and_connections();

    void build_output_layer_and_cost();

    void setLearningRate( real the_learning_rate );

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here    
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(StackedFocusedAutoassociatorsNet);

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
