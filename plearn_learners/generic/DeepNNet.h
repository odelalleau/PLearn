// -*- C++ -*-

// DeepNNet.h
//
// Copyright (C) 2005 Yoshua Bengio 
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

/* *******************************************************      
   * $Id: DeepNNet.h,v 1.6 2005/01/25 14:52:29 yoshua Exp $ 
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file DeepNNet.h */


#ifndef DeepNNet_INC
#define DeepNNet_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

class DeepNNet: public PLearner
{

private:

  typedef PLearner inherited;
  
protected:

  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here

  TVec<TVec<TVec<int> > > sources; // at [l][i] indices of inputs to neuron i of layer l
  TVec<TVec<Vec > > weights; // at [l][i] input weight vector of neuron i of layer l
  TVec<Vec> biases; // at [l][i] bias of neuron i of layer l
  Vec layerwise_lr_factor; // at [l] the multiplicative factor on the weights
  
  // temporary 
  mutable TVec<Vec> activations; // at [l] output of non-linearity of layer l, including the input AND the output layer
  TVec<Vec> activations_gradients; // gradients of the above (for hidden and output layers, NOT the input layer)
  TVec<Mat> avg_weight_gradients; // at [l] average of norm of gradients on all existing and potential connections
  Vec layerwise_gradient_norm_ma; // at [l] moving average of the norm of the weight gradient on that layer
  Vec layerwise_gradient_norm; // at [l] sum of the weight gradient squared, on that layer
  TVec<int> n_weights_of_layer; // number of weights in layer l
  real learning_rate;
    
public:

  // ************************
  // * public build options *
  // ************************

  // ### declare public option fields (such as build options) here

  int n_layers; // this counts the output layer but not the input layer
  int n_outputs;
  int default_n_units_per_hidden_layer; // optionally to initialize n_units_per_layer
  TVec<int> n_units_per_layer; 
  real L1_regularizer; // amount of penalty on sum_{l,i,j} |weights[l][i][j]|
  real initial_learning_rate;
  real learning_rate_decay;
  real layerwise_learning_rate_adaptation; 
  string output_cost; // implies a non-linearity for outputs: "mse" -> linear, "nll" -> softmax
  bool add_connections; // if true, instanciate potential connections with greater
  // average gradient than the existing connections with the smallest average gradient
  bool remove_connections; // remove the weaker existing connections (smaller absolute value)
  real initial_sparsity; // initial fraction of weights that are 0
  int connections_adaptation_frequency; // after how many examples do we try to adapt connections? 0=train set size.

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  // (Make sure the implementation in the .cc
  // initializes all fields to reasonable default values)
  DeepNNet();


  // ********************
  // * PLearner methods *
  // ********************

private: 

  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  
  //! Declares this class' options.
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  // Declares other standard object methods.
  // If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT.
  PLEARN_DECLARE_OBJECT(DeepNNet);


  // **************************
  // **** PLearner methods ****
  // **************************

  //! Returns the size of this learner's output, (which typically
  //! may depend on its inputsize(), targetsize() and set options).
  // (PLEASE IMPLEMENT IN .cc)
  virtual int outputsize() const;

  //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0 (this is the stage of a fresh learner!).
  // (PLEASE IMPLEMENT IN .cc)
  virtual void forget();

    
  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  // (PLEASE IMPLEMENT IN .cc)
  virtual void train();


  //! Computes the output from the input.
  // (PLEASE IMPLEMENT IN .cc)
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Computes the costs from already computed output. 
  // (PLEASE IMPLEMENT IN .cc)
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;
                                

  //! Returns the names of the costs computed by computeCostsFromOutpus (and thus the test method).
  // (PLEASE IMPLEMENT IN .cc)
  virtual TVec<std::string> getTestCostNames() const;

  //! Returns the names of the objective costs that the train method computes and 
  //! for which it updates the VecStatsCollector train_stats.
  // (PLEASE IMPLEMENT IN .cc)
  virtual TVec<std::string> getTrainCostNames() const;


  // *** SUBCLASS WRITING: ***
  // While in general not necessary, in case of particular needs 
  // (efficiency concerns for ex) you may also want to overload
  // some of the following methods:
  // virtual void computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const;
  // virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;
  // virtual void test(VMat testset, PP<VecStatsCollector> test_stats, VMat testoutputs=0, VMat testcosts=0) const;
  // virtual int nTestCosts() const;
  // virtual int nTrainCosts() const;
  // virtual void resetInternalState();
  // virtual bool isStatefulLearner() const;

  // propagate activations from activations[0] to activations[n_layers]
  void fprop() const;

  // initialize with random connectivity and random weights and 0 biases
  void initializeParams(bool set_seed=true);
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(DeepNNet);
  
} // end of namespace PLearn

#endif
