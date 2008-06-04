// -*- C++ -*-

// StackedAutoassociatorsNet.h
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

// Authors: Hugo Larochelle

/*! \file StackedAutoassociatorsNet.h */


#ifndef StackedAutoassociatorsNet_INC
#define StackedAutoassociatorsNet_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/CostModule.h>
#include <plearn_learners/online/NLLCostModule.h>
#include <plearn_learners/online/RBMClassificationModule.h>
#include <plearn_learners/online/RBMLayer.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMConnection.h>
#include <plearn/misc/PTimer.h>

namespace PLearn {

/**
 * Neural net, trained layer-wise in a greedy fashion using autoassociators.
 * It is highly inspired by the DeepBeliefNet class, and can use use the
 * same RBMLayer and RBMConnection components.
 */
class StackedAutoassociatorsNet : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! The learning rate used during the autoassociator gradient descent training
    real greedy_learning_rate;

    //! The decrease constant of the learning rate used during the autoassociator
    //! gradient descent training. When a hidden layer has finished its training,
    //! the learning rate is reset to it's initial value.
    real greedy_decrease_ct;

    //! The learning rate used during the fine tuning gradient descent
    real fine_tuning_learning_rate;

    //! The decrease constant of the learning rate used during fine tuning
    //! gradient descent
    real fine_tuning_decrease_ct;

    //! L1 penalty weight on the hidden layers, to encourage sparsity during
    //! the greedy unsupervised phases
    real l1_neuron_decay;

    //! Value around which the L1 penalty should be centered, i.e.
    //!    L1(h) = | h - l1_neuron_decay_center |
    //! where h is the value of the neurons.
    real l1_neuron_decay_center;

    //! Number of examples to use during each phase of greedy pre-training.
    //! The number of fine-tunig steps is defined by nstages.
    TVec<int> training_schedule;

    //! Whether to do things by stages, including fine-tuning, or on-line
    bool online;

    //! The layers of units in the network
    TVec< PP<RBMLayer> > layers;

    //! The weights of the connections between the layers
    TVec< PP<RBMConnection> > connections;

    //! The weights of the reconstruction connections between the layers
    TVec< PP<RBMConnection> > reconstruction_connections;

    //! Optional weights to capture correlation and anti-correlation
    //! in the hidden layers. They must have the same input and
    //! output sizes, compatible with their corresponding layers.
    TVec< PP<RBMConnection> > correlation_connections;

    //! Optional weights from each inputs to all other inputs'
    //! reconstruction, which can capture simple (linear or log-linear)
    //! correlations between inputs.
    mutable TVec< PP<RBMConnection> > direct_connections;

    //! Module that takes as input the output of the last layer
    //! (layers[n_layers-1), and feeds its output to final_cost
    //! which defines the fine-tuning criteria.
    PP<OnlineLearningModule> final_module;

    //! The cost function to be applied on top of the neural network
    //! (i.e. at the output of final_module). Its gradients will be 
    //! backpropagated to final_module and then backpropagated to
    //! the layers.
    PP<CostModule> final_cost;

    //! Corresponding additional supervised cost function to be applied on 
    //! top of each hidden layer during the autoassociator training stages. 
    //! The gradient for these costs are not backpropagated to previous layers.
    TVec< PP<CostModule> > partial_costs;

    //! Relative weights of the partial costs. If not defined,
    //! weights of 1 will be assumed for all partial costs.
    Vec partial_costs_weights;

    //! Indication that, at test time, all costs for all
    //! layers (up to the currently trained layer) should be computed.
    bool compute_all_test_costs;

    //! Indication that the autoassociators are also trained to
    //! reconstruct their hidden layers (inspired from CD1 in an RBM)
    bool reconstruct_hidden;

    //! Random fraction of the autoassociators' input components that
    //! masked, i.e. unsused to reconstruct the input.
    real fraction_of_masked_inputs;

    //! Number of samples to use for unsupervised fine-tuning
    int unsupervised_nstages;

    //! The learning rate used during the unsupervised fine tuning gradient descent
    real unsupervised_fine_tuning_learning_rate;

    //! The decrease constant of the learning rate used during 
    //! unsupervised fine tuning gradient descent
    real unsupervised_fine_tuning_decrease_ct;

    //! Indication that only the input layer should be masked 
    //! during unsupervised fine-tuning
    bool mask_input_layer_only_in_unsupervised_fine_tuning;

    //#####  Public Learnt Options  ###########################################

    //! Number of layers
    int n_layers;

    //! Number of samples visited so far during unsupervised fine-tuning
    int unsupervised_stage;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    StackedAutoassociatorsNet();


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
    virtual void computeOutputWithoutCorrelationConnections(const Vec& input, 
                                                        Vec& output) const;

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;


    void greedyStep( const Vec& input, const Vec& target, int index, 
                     Vec train_costs );

    void unsupervisedFineTuningStep( const Vec& input, const Vec& target,
                                     Vec& train_costs );

    void fineTuningStep( const Vec& input, const Vec& target,
                         Vec& train_costs );

    void onlineStep( const Vec& input, const Vec& target,
                         Vec& train_costs );

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(StackedAutoassociatorsNet);

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

    //! Reconstruction activations
    mutable Vec reconstruction_activations;
    
    //! Reconstruction activation gradients
    mutable Vec reconstruction_activation_gradients;

    //! Reconstruction expectation gradients
    mutable Vec reconstruction_expectation_gradients;

    //! Unsupervised fine-tuning reconstruction activations
    TVec< Vec > fine_tuning_reconstruction_activations;
    
    //! Unsupervised fine-tuning reconstruction expectations
    TVec< Vec > fine_tuning_reconstruction_expectations;

    //! Unsupervised fine-tuning reconstruction activations gradients
    TVec< Vec > fine_tuning_reconstruction_activation_gradients;
    
    //! Unsupervised fine-tuning reconstruction expectations gradients
    TVec< Vec > fine_tuning_reconstruction_expectation_gradients;

    //! Reconstruction activation gradients coming from hidden reconstruction
    mutable Vec reconstruction_activation_gradients_from_hid_rec;
    
    //! Reconstruction expectation gradients coming from hidden reconstruction
    mutable Vec reconstruction_expectation_gradients_from_hid_rec;

    //! Hidden reconstruction activations
    mutable Vec hidden_reconstruction_activations;
    
    //! Hidden reconstruction activation gradients
    mutable Vec hidden_reconstruction_activation_gradients;
    
    //! Activations before the correlation layer
    mutable TVec<Vec> correlation_activations;
    
    //! Expectations before the correlation layer
    mutable TVec<Vec> correlation_expectations;
    
    //! Gradients of activations before the correlation layer
    mutable TVec<Vec> correlation_activation_gradients;
    
    //! Gradients of expectations before the correlation layer
    mutable TVec<Vec> correlation_expectation_gradients;

    //! Hidden layers for the correlation connections
    mutable TVec< PP<RBMLayer> > correlation_layers;

    //! Activations from the direct connections
    mutable Vec direct_activations;

    //! Sum of activations from the direct and reconstruction connections
    mutable Vec direct_and_reconstruction_activations;

    //! Gradient of sum of activations from the direct and reconstruction connections
    mutable Vec direct_and_reconstruction_activation_gradients;

    //! Position in the total cost vector of the different partial costs
    mutable TVec<int> partial_costs_positions;
    
    //! Cost value of partial_costs
    mutable Vec partial_cost_value;

    //! Input of the final_cost
    mutable Vec final_cost_input;

    //! Cost value of final_cost
    mutable Vec final_cost_value;

    //! Stores the gradient of the cost at the input of final_cost
    mutable Vec final_cost_gradient;

    //! Input of autoassociator where some of the components
    //! have been masked (set to 0) randomly.
    Vec masked_autoassociator_input;

    //! Layers randomly masked, for unsupervised fine-tuning.
    TVec< Vec > masked_autoassociator_expectations;

    //! Indices of the input components
    TVec<int> autoassociator_input_indices;

    //! Indices of the expectation components
    TVec< TVec<int> > autoassociator_expectation_indices;

    //! Stages of the different greedy phases
    TVec<int> greedy_stages;

    //! Currently trained layer (1 means the first hidden layer,
    //! n_layers means the output layer)
    int currently_trained_layer;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Declare the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

    void build_layers_and_connections();

    void build_classification_cost();

    void build_costs();

    void setLearningRate( real the_learning_rate );

    // List of remote methods
    
    Vec remote_computeOutputWithoutCorrelationConnections(const Vec& input) const;

    //! Global storage to save memory allocations.
    mutable Vec tmp_output;


private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here    
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(StackedAutoassociatorsNet);

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
