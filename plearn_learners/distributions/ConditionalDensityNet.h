// -*- C++ -*-

// ConditionalDensityNet.h
//
// Copyright (C) 2003 *AUTHOR(S)* 
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
   * $Id: ConditionalDensityNet.h,v 1.6 2003/11/19 15:07:08 yoshua Exp $ 
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file ConditionalDensityNet.h */


#ifndef ConditionalDensityNet_INC
#define ConditionalDensityNet_INC

#include "PConditionalDistribution.h"
#include "Var_all.h"
#include "Optimizer.h"

namespace PLearn <%
using namespace std;

class ConditionalDensityNet: public PConditionalDistribution
{
public:
  typedef PConditionalDistribution inherited;  

protected:
  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
    Var input;  // Var(inputsize())
    Var target; // Var(targetsize()-weightsize())
    Var sampleweight; // Var(1) if train_set->hasWeights()
    Var w1; // bias and weights of first hidden layer
    Var w2; // bias and weights of second hidden layer
    Var wout; // bias and weights of output layer
    Var wdirect; // bias and weights for direct in-to-out connection

    Var output; // output layer contains the parameters of the distribution:
  Var a; // output parameter, scalar constant part
  Var b; // output parameters, step height parameters
  Var c; // output parameters, step smoothing parameters
  Var mu; // output parameters, step location parameters
  Var density;
  Var cumulative;
  Var expected_value;
  Vec target_dependent_outputs;
    VarArray costs; // all costs of interest
    VarArray penalties;
    Var training_cost; // weighted scalar costs[0] including penalties
    Var test_costs; // hconcat(costs)

    VarArray params;  // all arameter input vars

    Vec paramsvalues; // values of all parameters

public:

    mutable Func f; // input -> output
    mutable Func test_costf; // input & target -> output & test_costs
    mutable Func output_and_target_to_cost; // output & target -> cost

  // ************************
  // * public build options *
  // ************************

  // ***** OPTIONS PASTED FROM NNET ************** 

  // ### declare public option fields (such as build options) here
    int nhidden;    // number of hidden units in first hidden layer (default:0)
    int nhidden2;   // number of hidden units in second hidden layer (default:0)

    real weight_decay; // default: 0
    real bias_decay;   // default: 0 
    real layer1_weight_decay; // default: MISSING_VALUE
    real layer1_bias_decay;   // default: MISSING_VALUE
    real layer2_weight_decay; // default: MISSING_VALUE
    real layer2_bias_decay;   // default: MISSING_VALUE
    real output_layer_weight_decay; // default: MISSING_VALUE
    real output_layer_bias_decay;   // default: MISSING_VALUE
    real direct_in_to_out_weight_decay; // default: MISSING_VALUE

    bool L1_penalty; // default: false
    bool direct_in_to_out; // should we include direct input to output connecitons? default: false

    // Build options related to the optimization:
    PP<Optimizer> optimizer; // the optimizer to use (no default)

    int batch_size; // how many samples to use to estimate gradient before an update
                    // 0 means the whole training set (default: 1)

  // ***** OPTIONS SPECIFIC TO CONDITIONALDENSITYNET ************** 

  // maximum value that Y can take
  real maxY;

  // this weight between 0 and 1 controls the balance of the cost function
  // between minimizing the negative log-likelihood and minimizing squared error
  // if 1 then perform maximum likelihood, if 0 then perform least square optimization
  real log_likelihood_vs_squared_error_balance; 

  // number of terms in the output density function
  int n_output_density_terms;

  // the type of steps used to build the cumulative
  // allowed values are:
  //  - sigmoid_steps: g(y,theta,i) = sigmoid(s(c_i)*(y-mu_i))\n"
  //  - sloped_steps: g(y,theta,i) = s(s(c_i)*(mu_i-y))-s(s(c_i)*(mu_i-y))\n"
  string steps_type;

  // how to initialize the mu_i:
  //   - uniform: at regular intervals in [0,maxY]
  //   - log-scale: as the exponential of values at regular intervals in [0,log(1+maxY)], minus 1
  string centers_initialization;

  // approximate unconditional probability of Y=0 (mass point), used
  // to initialize the parameters
  real unconditional_p0;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  ConditionalDensityNet();


  // ******************
  // * PConditionalDistribution methods *
  // ******************

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declares other standard object methods
  //  If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
  PLEARN_DECLARE_OBJECT(ConditionalDensityNet);


  // **************************
  // **** PConditionalDistribution methods ****
  // **************************

  //! Set the input part before using the inherited methods
  virtual void setInput(const Vec& input) const;

  // **************************
  // **** PDistribution methods ****
  // **************************

  //! return log of probability density log(p(x))
  virtual double log_density(const Vec& x) const;

  //! return survival fn = P(X>x)
  virtual double survival_fn(const Vec& x) const;

  //! return survival fn = P(X<=x)
  virtual double cdf(const Vec& x) const;

  //! return E[X] 
  virtual void expectation(Vec& mu) const;

  //! return Var[X]
  virtual void variance(Mat& cov) const;

  //! Resets the random number generator used by generate using the given seed
  virtual void resetGenerator(long g_seed) const;

  //! return a pseudo-random sample generated from the distribution.
  virtual void generate(Vec& x) const;

  
  // **************************
  // **** Learner methods ****
  // **************************

  // Default version of inputsize returns learner->inputsize()
  // If this is not appropriate, you should uncomment this and define
  // it properly in the .cc
  // virtual int inputsize() const;

  //! (Re-)initializes the PConditionalDistribution in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
  //! You may remove this method if your distribution does not implement it
  virtual void forget();

    
  void initializeParams();

  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  //! You may remove this method if your distribution does not implement it
  virtual void train();

  virtual int outputsize() const;
  virtual TVec<string> getTrainCostNames() const;
  virtual TVec<string> getTestCostNames() const;
};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(ConditionalDensityNet);
  
%> // end of namespace PLearn

#endif
