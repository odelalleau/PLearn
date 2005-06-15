// -*- C++ -*-

// MultiInstanceNNet.h
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
   * $Id: MultiInstanceNNet.h,v 1.15 2005/06/15 14:40:38 lamblin Exp $
   ******************************************************* */

/*! \file PLearn/plearn_learners/classifiers/MultiInstanceNNet.h */

#ifndef MultiInstanceNNet_INC
#define MultiInstanceNNet_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/opt/Optimizer.h>

namespace PLearn {
using namespace std;

class MultiInstanceNNet: public PLearner
{

private:

  typedef PLearner inherited;

  //! Used to store data between calls to computeCostsFromOutput.
  mutable Vec instance_logP0;

  protected:

    Var input;  // Var(inputsize())
    Var target; // Var(targetsize()-weightsize())
    Var sampleweight; // Var(1) if train_set->hasWeights()
    Var w1; // bias and weights of first hidden layer
    Var w2; // bias and weights of second hidden layer
    Var wout; // bias and weights of output layer
    Var wdirect; // bias and weights for direct in-to-out connection

    Var output; // output (P(y_i|x_i)) for a single bag element
    Var bag_size; // filled up by SumOverBagsVariable
    Var bag_inputs; // filled up by SumOverBagsVariable
    Var bag_output; // P(y=1|bag_inputs)

    Func inputs_and_targets_to_test_costs; // (bag inputs and targets) -> (bag NLL, bag class. err, lift_output)
    Func inputs_and_targets_to_training_costs; // (bag inputs and targets) -> (bag NLL + penalty, bag NLL, bag class. err, lift_output)
    Func input_to_logP0; // single input x -> log P(y=0|x)
    Var nll;

    VarArray costs; // (negative log-likelihood, classification error) for the bag
    VarArray penalties;
    Var training_cost; // weighted cost + penalties
    Var test_costs; // hconcat(costs)
    VarArray invars;
    VarArray params;  // all arameter input vars

    Vec paramsvalues; // values of all parameters

    int optstage_per_lstage; // number of bags in training set / batch_size (in nb of bags)
    bool training_set_has_changed; // if so, must count nb of bags in training set

  public:
    mutable Func f; // input -> output
    mutable Func test_costf; // input & target -> output & test_costs
    mutable Func output_and_target_to_cost; // output & target -> cost

  public:
    
    // Build options inherited from learner:
    // inputsize, outputsize, targetsize, experiment_name, save_at_every_epoch 

    // Build options:    
    int max_n_instances; // maximum number of instances (input vectors x_i) allowed

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
    real classification_regularizer; // default: 0

    string penalty_type; // default: "L2_square"
    bool L1_penalty; // default: false - deprecated, set "penalty_type" to "L1"
    bool direct_in_to_out; // should we include direct input to output connecitons? default: false
    real interval_minval, interval_maxval; // if output_transfer_func = interval(minval,maxval), these are the interval bounds
    mutable int test_bag_size; // counting the number of instances in a test bag

    // Build options related to the optimization:
    PP<Optimizer> optimizer; // the optimizer to use (no default)

    int batch_size; // how many samples to use to estimate gradient before an update
                    // 0 means the whole training set (default: 1)

  private:
    void build_();

  public:

    MultiInstanceNNet();
    virtual ~MultiInstanceNNet();
    PLEARN_DECLARE_OBJECT(MultiInstanceNNet);

    // PLearner methods

    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

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
    void initializeParams();

  };

  DECLARE_OBJECT_PTR(MultiInstanceNNet);

} // end of namespace PLearn

#endif

