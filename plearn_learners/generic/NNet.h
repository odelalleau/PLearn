// -*- C++ -*-

// NNet.h
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
   * $Id: NNet.h,v 1.20 2004/07/21 16:30:56 chrish42 Exp $
   ******************************************************* */

/*! \file PLearnLibrary/PLearnAlgo/NNet.h */

#ifndef NNet_INC
#define NNet_INC

#include "PLearner.h"
#include <plearn/opt/Optimizer.h>
//#include "Var_all.h"

namespace PLearn {
using namespace std;

  class NNet: public PLearner
  {
  protected:
    Var input;  // Var(inputsize())
    Var target; // Var(targetsize()-weightsize())
    Var sampleweight; // Var(1) if train_set->hasWeights()
    Var w1; // bias and weights of first hidden layer
    Var w2; // bias and weights of second hidden layer
    Var wout; // bias and weights of output layer
    Var outbias; // bias used only if fixed_output_weights
    Var wdirect; // bias and weights for direct in-to-out connection
    Var wrec; // input reconstruction weights (optional), from hidden layer to predicted input
    Var rbf_centers; // n_classes (or n_classes-1) x rbf_layer_size = mu_i of RBF gaussians
    Var rbf_sigmas; // n_classes (or n_classes-1) entries (sigma's of the RBFs)
    Var junk_prob; // scalar background (junk) probability, if first_class_is_junk
    Var output;
    Var predicted_input;
    VarArray costs; // all costs of interest
    VarArray penalties;
    Var training_cost; // weighted scalar costs[0] including penalties
    Var test_costs; // hconcat(costs)
    VarArray invars;
    VarArray params;  // all arameter input vars

    Vec paramsvalues; // values of all parameters

  public:
    mutable Func f; // input -> output
    mutable Func test_costf; // input & target -> output & test_costs
    mutable Func output_and_target_to_cost; // output & target -> cost

  public:

    typedef PLearner inherited;

    // Build options inherited from learner:
    // inputsize, outputszie, targetsize, experiment_name, save_at_every_epoch 

    // Build options:    
    int nhidden;    // number of hidden units in first hidden layer (default:0)
    int nhidden2;   // number of hidden units in second hidden layer (default:0)
    int noutputs;   // number of output units (outputsize)

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
    real margin; // default: 1, used with margin_perceptron_cost
    bool fixed_output_weights;

    int rbf_layer_size; // number of representation units when adding an rbf layer in output
    bool first_class_is_junk;

    bool L1_penalty; // default: false
    real input_reconstruction_penalty; // default = 0
    bool direct_in_to_out; // should we include direct input to output connecitons? default: false
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

    NNet();
    virtual ~NNet();
    PLEARN_DECLARE_OBJECT(NNet);

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

  DECLARE_OBJECT_PTR(NNet);

} // end of namespace PLearn

#endif

