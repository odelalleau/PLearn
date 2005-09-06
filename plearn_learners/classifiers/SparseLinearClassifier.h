// -*- C++ -*-

// SparseLinearClassifier.h
//
// Copyright (C) 2004 Yoshua Bengio & Hugo Larochelle
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
   * $Id: SparseLinearClassifier.h,v 1.7 2005/06/17 16:45:39 larocheh Exp $
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file SparseLinearClassifier.h */


#ifndef SparseLinearClassifier_INC
#define SparseLinearClassifier_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/var/Func.h>
#include <plearn/opt/Optimizer.h>
#include <plearn/var/SparseIncrementalAffineTransformVariable.h>

namespace PLearn {
using namespace std;

class SparseLinearClassifier: public PLearner
{

private:

  typedef PLearner inherited;
  
protected:
  // NON-OPTION FIELDS

  // *********************
  // * protected options *
  // *********************



  //! Input dimension
  int n;
  //! input vector
  Var x; 
  //! Target vector
  Var target;
  //! Output vector
  Var output;
  //! Sample weight
  Var sampleweight;
  //! Weights of the classifier
  Var W; 
  //! Sparse affine transform
  PP<SparseIncrementalAffineTransformVariable> afft;
  //! Sparse affine transform
  Var afft_var;
  //! Costs
  VarArray costs;
  //! Classification function
  Func classify;
  //! Training costs;
  Var training_cost;
  //! Test costs
  Var test_costs;
  //! Input variables of cost function
  VarArray invars;
  //! ComputeCostsFromOutputs function 
  Func output_and_target_to_cost;
  //! Learnt parameters
  VarArray parameters;
  //! Penalty variables
  VarArray penalties;
  //! Number of weights
  int n_weights;

public:

  
  // ************************
  // * public build options *
  // ************************
  
  // ### declare public option fields (such as build options) here

  //! Optimizer of the neural network
  PP<Optimizer> optimizer; 
  //! Maximum number of weights
  int max_n_weights;
  //! Number of stages before adding a weight
  int add_weight_every_n_epochs;
  //! Number of weights to add at a time
  int add_n_weights;
  //! Batch size of the optimization
  int batch_size;
  //! Output size
  int noutputs;
  //! Output transfer function
  string output_transfer_func;
  //! Cost functions
  Array<string> cost_funcs;
  //! Classification regularizer for stable cross entropy
  real classification_regularizer; 
  //! Weight decay
  real weight_decay;
  //! Weight decay penalty type
  string penalty_type;
  //! Margin for margin perceptron cost
  real margin;   
  //! Indication that the last outputsize() elements of the input are the mulitarget weights.
  bool weights_are_in_input;


  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  SparseLinearClassifier();


  // ********************
  // * PLearner methods *
  // ********************

private: 

  //! This does the actual building. 
  void build_();

protected: 
  
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

  //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0 (this is the stage of a fresh learner!).
  virtual void forget();
  virtual void initializeParams();

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
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS.
  PLEARN_DECLARE_OBJECT(SparseLinearClassifier);

  // *******************************
  // **** PDistribution methods ****
  // *******************************

  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  virtual void train();

  //! Produce outputs according to what is specified in outputs_def.
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Returns the size of this learner's output, (which typically
  //! may depend on its inputsize(), targetsize() and set options).
  virtual int outputsize() const;

  //! Computes the costs from already computed output. 
  // (PLEASE IMPLEMENT IN .cc)
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;

  //! Returns the names of the costs computed by computeCostsFromOutpus (and thus the test method).
  // (PLEASE IMPLEMENT IN .cc)
  virtual TVec<string> getTestCostNames() const;

  //! Returns the names of the objective costs that the train method computes and 
  //! for which it updates the VecStatsCollector train_stats.
  // (PLEASE IMPLEMENT IN .cc)
  virtual TVec<string> getTrainCostNames() const;


  // *** SUBCLASS WRITING: ***
  // While in general not necessary, in case of particular needs 
  // (efficiency concerns for ex) you may also want to overload
  // some of the following methods:
  // virtual void computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const;
  // virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;
  // virtual void test(VMat testset, PP<VecStatsCollector> test_stats, VMat testoutputs=0, VMat testcosts=0) const;
  // virtual int nTestCosts() const;
  // virtual int nTrainCosts() const;
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(SparseLinearClassifier);
  
} // end of namespace PLearn

#endif
