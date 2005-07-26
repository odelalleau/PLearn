// -*- C++ -*-

// IncrementalNNet.h
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
   * $Id$ 
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file IncrementalNNet.h */


#ifndef IncrementalNNet_INC
#define IncrementalNNet_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

class IncrementalNNet: public PLearner
{

private:

  typedef PLearner inherited;
  
protected:

  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here

  Mat output_weights; // [hidden_unit, output] ** NOTE IT IS TRANSPOSED ** so can easily add hidden units
  Vec output_biases;
  Mat hidden_layer_weights; // [hidden_unit, input]
  Mat hidden_layer_weight_gradients; 
  Vec hidden_layer_biases; // [hidden_unit]
  TVec<Vec> internal_weights;    // among hidden units [to, from]. enabled by enable_internal_weights
  TVec<Vec> internal_weight_gradients;    
  Vec candidate_unit_weights;
  Vec candidate_unit_weight_gradients; 
  real candidate_unit_bias;
  Vec candidate_unit_output_weights;
  Vec candidate_unit_internal_weights;
  Vec candidate_unit_internal_weight_gradients;
  int n_examples_seen;
  real current_average_cost;
  real next_average_cost;
  int n_examples_training_candidate;
  int current_example;
    
public:

  // ************************
  // * public build options *
  // ************************

  // ### declare public option fields (such as build options) here

  int n_outputs;
  real output_weight_decay; // L1 regularizer's penalty factor on output weights
  bool online; // use online or batch version? if batch only consider adding a hidden unit after minibatch_size examples
               // Add a hidden unit only if it would reduce the average cost (including the L1 penalty).
               // This average is calculated either with a moving average over a moving target (online version)
               // or the algorithm proceeds in two phases (batch version): on even batches one improves the
               // tentative hidden unit, while on odd batches one evaluates its quality.
  int minibatch_size; // see above; 0 is a special value meaning minibatch_size == training set size
  string output_cost_type; // "squared_error", "hinge_loss", "discrete_log_likelihood" (for probabilistic classification)
  bool boosting; // use a boosting-like approach (only works with online=false and minimize_local_cost=true) and train the 
                 // new hidden unit  but not the previous ones; also descend not the actual cost but a weighted cost obtained
                 // from the gradient of the output cost on the hidden unit function (see minimize_local_cost option).
  bool minimize_local_cost; // if true then instead of minimize global cost sum_t Q(f(x_t),y_t),
                 // each hidden unit minimizes sum_t Q'(f(x_t),y_t) h(x_t)
                 // or some approximation of it if h is a hard threshold (weighted logistic regression cost
                 // with targets sign(Q'(f(x_t),y_t)) and weights |Q'(f(x_t),y_t)|),
                 // where Q is the output cost, f(x_t) is the current prediction, y_t the target, h(x_t) the
                 // output of the new hidden unit.
  bool hard_activation_function; // if true then h(x) = sign(w'x + b), else h(x) = tanh(w'x + b)
  bool use_hinge_loss_for_hard_activation; // use hinge loss or cross-entropy to train hidden units, when hard_activation_function
  real initial_learning_rate; // learning_rate = initial_learning_rate / (1 + n_examples_seen * decay_factor);
  real decay_factor;
  real max_n_epochs_to_fail; // Maximum number of epochs (not necessarily an integer) to try improving the new hidden unit
                             // before declaring failure to improve the regularized cost (and thus stopping training).
                             
  real rand_range;           // Interval of random numbers when initializing weights/biases: (-rand_range/2, rand_range/2).
  bool enable_internal_weights; // Network has a cascade topology if true, or one hidden layer if false (default).
  bool incremental_connections; // Add connections incrementally if true, or all at once if false (default). 
                                // This option is only supported with n_outputs == 1. 
  real connection_gradient_threshold;    // Threshold of gradient for connection to be added, when incremental_connections == true.
  bool residual_correlation_gradient;  // Use residual correlation gradient (ConvexNN) if true, or classical error 
                                       // back-propagation if false.
  
  // ** NON-OPTION FIELDS
  //
  Vec linear_output; // output before possible output-non-linearity = output_weights * h(x) + output_biases
  Vec act; // weighted sum of inputs on hidden units, before non-linearity
  Vec h; // output of hidden units after hidden unit non-linearity
  int cost_type; // 0 = squared_error, 1 = hinge_loss, 2 = discrete_log_likelihood

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  // (Make sure the implementation in the .cc
  // initializes all fields to reasonable default values)
  IncrementalNNet();


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
  PLEARN_DECLARE_OBJECT(IncrementalNNet);


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

  virtual real output_loss(const Vec& output,const Vec& target) const; // compute output loss, according to output_loss_type

  // compute doutput_loss/doutput in output_gradient
  virtual void output_loss_gradient(const Vec& output,const Vec& target,
                                    Vec output_gradient, real sampleweight) const;

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(IncrementalNNet);
  
} // end of namespace PLearn

#endif
