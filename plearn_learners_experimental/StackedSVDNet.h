// -*- C++ -*-

// StackedSVDNet.h
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

/*! \file StackedSVDNet.h */


#ifndef StackedSVDNet_INC
#define StackedSVDNet_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/CostModule.h>
#include <plearn_learners/online/RBMLayer.h>
#include <plearn_learners/online/RBMConnection.h>
#include <plearn_learners/online/RBMMatrixConnection.h>
#include <plearn/misc/PTimer.h>

namespace PLearn {

/**
 * Neural net, initialized with SVDs of logistic auto-regressions.
 */
class StackedSVDNet : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! The learning rate used during the logistic auto-regression 
    //! gradient descent training
    real greedy_learning_rate;

    //! The decrease constant of the learning rate used during the 
    //! logistic auto-regression gradient descent training. 
    real greedy_decrease_ct;

    //! The learning rate used during the fine tuning gradient descent
    real fine_tuning_learning_rate;

    //! The decrease constant of the learning rate used during fine tuning
    //! gradient descent
    real fine_tuning_decrease_ct;

    //! Size of mini-batch for gradient descent
    int minibatch_size;

    //! Indication that the output layer (given by the final module)
    //! should have as input all units of the network (including the input units)
    bool global_output_layer;

    //! Indication that the zero diagonal of the weight matrix after
    //! logistic auto-regression should be filled with the
    //! maximum absolute value of each corresponding row.
    bool fill_in_null_diagonal;
        
    //! Number of examples to use during each phase of learning:
    //! first the greedy phases, and then the fine-tuning phase.
    //! However, the learning will stop as soon as we reach nstages.
    //! For example for 2 hidden layers, with 1000 examples in each
    //! greedy phase, and 500 in the fine-tuning phase, this option
    //! should be [1000 1000 500], and nstages should be at least 2500.
    //! When online = true, this vector is ignored and should be empty.
    TVec<int> training_schedule;

    //! The layers of units in the network
    TVec< PP<RBMLayer> > layers;

    //! Module that takes as input the output of the last layer
    //! (layers[n_layers-1), and feeds its output to final_cost
    //! which defines the fine-tuning criteria.
    PP<OnlineLearningModule> final_module;

    //! The cost function to be applied on top of the neural network
    //! (i.e. at the output of final_module). Its gradients will be 
    //! backpropagated to final_module and then backpropagated to
    //! the layers.
    PP<CostModule> final_cost;

    //#####  Public Learnt Options  ###########################################

    //! The weights of the connections between the layers
    TVec< PP<RBMMatrixConnection> > connections;

    //! View of connections as RBMConnection pointers (for compatibility
    //! with RBM function calls)
    TVec< PP<RBMConnection> > rbm_connections;

    //! Number of layers
    int n_layers;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    StackedSVDNet();


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

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;


    void greedyStep( const Mat& inputs, const Mat& targets, int index, 
                     Vec train_costs );

    void fineTuningStep( const Mat& inputs, const Mat& targets,
                         Vec& train_costs );

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(StackedSVDNet);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################

    //! Stores the gradient of the cost wrt the activations of 
    //! the input and hidden layers
    //! (at the input of the layers)
    mutable TVec<Mat> activation_gradients;

    //! Stores the gradient of the cost wrt the expectations of 
    //! the input and hidden layers
    //! (at the output of the layers)
    mutable TVec<Mat> expectation_gradients;

    //! Reconstruction layer
    mutable PP<RBMLayer> reconstruction_layer;
    
    //! Reconstruction target
    mutable Mat reconstruction_targets;

    //! Reconstruction costs
    mutable Mat reconstruction_costs;

    //! Reconstruction costs (when using computeOutput and computeCostsFromOutput)
    mutable Vec reconstruction_test_costs;

    //! Reconstruction activation gradient
    mutable Vec reconstruction_activation_gradient;

    //! Reconstruction activation gradients
    mutable Mat reconstruction_activation_gradients;
    
    //! Reconstruction activations
    mutable Mat reconstruction_input_gradients;

    //! Global output layer input
    mutable Vec global_output_layer_input;

    //! Global output layer inputs
    mutable Mat global_output_layer_inputs;

    //! Global output layer input gradients
    mutable Mat global_output_layer_input_gradients;

    //! Inputs of the final_cost
    mutable Mat final_cost_inputs;

    //! Cost value of final_cost
    mutable Vec final_cost_value;

    //! Cost values of final_cost
    mutable Mat final_cost_values;

    //! Stores the gradients of the cost at the inputs of final_cost
    mutable Mat final_cost_gradients;

    //! Cumulative training schedule
    TVec<int> cumulative_schedule;


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

    void build_costs();

    void setLearningRate( real the_learning_rate );

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here    
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(StackedSVDNet);

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
