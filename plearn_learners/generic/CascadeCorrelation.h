// -*- C++ -*-

// CascadeCorrelation.h
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio and University of Montreal
// Copyright (c) 2002 Jean-Sebastien Senecal, Xavier Saint-Mleux, Rejean Ducharme
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
   * $Id: CascadeCorrelation.h,v 1.2 2004/11/17 16:04:25 larocheh Exp $
   ******************************************************* */

/*! \file PLearnLibrary/PLearnAlgo/CascadeCorrelation.h */

#ifndef CascadeCorrelation_INC
#define CascadeCorrelation_INC

#include "PLearner.h"
#include <plearn/opt/Optimizer.h>

namespace PLearn {
using namespace std;

class CascadeCorrelation: public PLearner
{

private:

  typedef PLearner inherited;

protected:

  Var input;  // Var(inputsize())
  Var target; // Var(targetsize()-weightsize())
  Var sampleweight; // Var(1) if train_set->hasWeights()
  Var w_in_to_out;
  VarArray hidden_nodes;
  VarArray w;
  VarArray wout; // weights of output layer
  
  Var wrec;
  Var candidate_node;
  Var output;
  Var identity;
  Var before_transfer_func;
  Var predicted_input;
  Var residual_error;
  VarArray costs; // all costs of interest
  VarArray penalties;
  Var training_cost; // weighted scalar costs[0] including penalties
  Var test_costs; // hconcat(costs)
  VarArray invars;
  VarArray params;  // all parameter input vars

  Vec paramsvalues; // values of all parameters

public:

  mutable Func f; // input -> output
  mutable Func test_costf; // input & target -> output & test_costs
  mutable Func output_and_target_to_cost; // output & target -> cost

public:

  // Build options inherited from learner:
  // inputsize, outputszie, targetsize, experiment_name, save_at_every_epoch 

  // Build options:    
  int n_opt_iterations;
  int noutputs;   // number of output units (outputsize)

  real weight_decay; // default: 0
  real bias_decay;   // default: 0 
  real hidden_nodes_weight_decay; // default: MISSING_VALUE
  real hidden_nodes_bias_decay;   // default: MISSING_VALUE
  real output_layer_weight_decay; // default: MISSING_VALUE
  real output_layer_bias_decay;   // default: MISSING_VALUE
  real classification_regularizer; // default: 0
  real margin; // default: 1, used with margin_perceptron_cost
  real early_stop_threshold; // default: 0.00001
  real min_iteration; // default: 10
  real correlation_early_stop_threshold; //default: 0.001
  bool use_correlation_optimization; //default true

  bool L1_penalty; // default: false
  string output_transfer_func; // tanh, sigmoid, softplus, softmax, etc...  (default: "" means no transfer function)
  string hidden_transfer_func; // tanh, sigmoid, softplus, softmax, etc...  (default: "tanh" means no transfer function)
  real interval_minval, interval_maxval; // if output_transfer_func = interval(minval,maxval), these are the interval bounds

  //! a list of cost functions to use in the form "[ cf1; cf2; cf3; ... ]"
  // where the cost functions can be one of mse, mse_onehot, NLL,
  // class_error or multiclass_error (no default)
  Array<string> cost_funcs;  

  // Build options related to the optimization:
  PP<Optimizer> optimizer; // the optimizer to use (no default)

  int batch_size; // how many samples to use to estimate gradient before an update
  // 0 means the whole training set (default: 1)

  string initialization_method;


private:
  void build_();

public:

  CascadeCorrelation();
  virtual ~CascadeCorrelation();
  PLEARN_DECLARE_OBJECT(CascadeCorrelation);

  virtual void build();
  virtual void forget(); // simply calls initializeParams()

  virtual int outputsize() const;
  virtual TVec<string> getTrainCostNames() const;
  virtual TVec<string> getTestCostNames() const;

  virtual void train();

  virtual void computeOutput(const Vec& input, Vec& output) const;

  virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                     Vec& output, Vec& costs) const;

  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;

  virtual void makeDeepCopyFromShallowCopy(CopiesMap &copies);

protected:
  static void declareOptions(OptionList& ol);

  //! Initialize the parameters. If 'set_seed' is set to false, the seed
  //! will not be set in this method (it will be assumed to be already
  //! initialized according to the 'seed' option).
  virtual void initializeParams(bool set_seed = true);

  //! Return a variable that is the hidden layer corresponding to given
  //! input and weights. If the 'default' transfer_func is used, we use the
  //! hidden_transfer_func option.
  Var hiddenLayer(const Var& input, const Var& weights, string transfer_func = "default");

  //! Build the output of the neural network from the input.
  void buildOutputFromInput();

  //! Update the output of the neural network from the input at stage i.
  void updateOutputFromInput(int i);

  //! Builds the target and sampleweight variables.
  void buildTargetAndWeight();

  //! Build the costs variable from other variables.
  void buildCosts();

  //! Update the costs variable from other variables at stage i.
  void updateCosts(int i);

  //! Build the various functions used in the network.
  void buildFuncs();

  //! Update the various functions used in the network.
  void updateFuncs();

  //! Fill a matrix of weights according to the 'initialization_method' specified.
  //! The 'clear_first_row' boolean indicates whether we should fill the first
  //! row with zeros.
  void fillWeights(const Var& weights, bool clear_first_row, int the_length = -1);

  //! Fill the costs penalties.
  virtual void buildPenalties();

  //! Update the costs penalties at stage i.
  virtual void updatePenalties(int i);

  //! Add a new candidate node at stage i
  void addCandidateNode(int i);
};

DECLARE_OBJECT_PTR(CascadeCorrelation);

} // end of namespace PLearn

#endif

