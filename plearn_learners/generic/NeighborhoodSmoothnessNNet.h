// -*- C++ -*-

// NeighborhoodSmoothnessNNet.h
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
 * $Id$
 ******************************************************* */

/*! \file PLearn/plearn_learners/generic/NeighborhoodSmoothnessNNet.cc */

#ifndef NeighborhoodSmoothnessNNet_INC
#define NeighborhoodSmoothnessNNet_INC

#include "PLearner.h"
#include <plearn/opt/Optimizer.h>
//#include "Var_all.h"

namespace PLearn {
using namespace std;

class NeighborhoodSmoothnessNNet: public PLearner
{

protected:

    Var input;  // Var(inputsize())
    Var target; // Var(targetsize()-weightsize())
    Var sampleweight; // Var(1) if train_set->hasWeights()
    Var w1; // bias and weights of first hidden layer
    Var w2; // bias and weights of second hidden layer
    Var wout; // bias and weights of output layer
    Var wdirect; // bias and weights for direct in-to-out connection
    Var last_hidden; // last hidden layer (the one to smooth)
    Var output; // output (P(y_i|x_i)) for a single bag element
    Var bag_size; // filled up by SumOverBagsVariable
    Var bag_inputs; // filled up by SumOverBagsVariable
    Var bag_output; // P(y=1|bag_inputs)
    Var bag_hidden; // The hidden layers of all inputs in a bag.
    mutable int test_bag_size; // BECAUSE OF UGLY HACK IN computOutputAndCost (look at it, it's worth it!)
    Func invars_to_training_cost; // (bag inputs and targets) -> training cost

    VarArray costs; // (negative log-likelihood, classification error) for the bag
    VarArray penalties;
    Var training_cost; // weighted cost + penalties
    Var test_costs; // hconcat(costs)
    VarArray invars;
    VarArray params;  // all arameter input vars
    Vec paramsvalues; // values of all parameters

    Var p_ij;       // The probabilities p_ij on the inputs.

public:

    mutable Func f; // input -> output
    Func f_input_to_hidden; // input -> hidden
    mutable Func test_costf; // input & target -> output & test_costs
    mutable Func output_and_target_to_cost; // output & target -> cost

public:
    
    typedef PLearner inherited;

    // Build options inherited from learner:
    // inputsize, outputszie, targetsize, experiment_name, save_at_every_epoch 

    // Build options:    
    int max_n_instances; // maximum number of instances (input vectors x_i) allowed

    int nhidden;    // number of hidden units in first hidden layer (default:0)
    int nhidden2;   // number of hidden units in second hidden layer (default:0)
    int noutputs;   // number of output units (outputsize)

    real sigma_hidden;
    real sne_weight;

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
    string output_transfer_func; // tanh, sigmoid, softplus, softmax  (default: "" means no transfer function)
    real interval_minval, interval_maxval; // if output_transfer_func = interval(minval,maxval), these are the interval bounds

    //! a list of cost functions to use in the form "[ cf1; cf2; cf3; ... ]"
    // where the cost functions can be one of mse, mse_onehot, NLL,
    // class_error or multiclass_error (no default)
    Array<string> cost_funcs;  

    // Build options related to the optimization:
    PP<Optimizer> optimizer; // the optimizer to use (no default)

    int batch_size; // how many samples to use to estimate gradient before an update
                    // 0 means the whole training set (default: 1)

private:

    void build_();

public:

    NeighborhoodSmoothnessNNet();
    virtual ~NeighborhoodSmoothnessNNet();
    PLEARN_DECLARE_OBJECT(NeighborhoodSmoothnessNNet);

    virtual void build();
    virtual void forget(); // simply calls initializeParams()

    virtual int outputsize() const;
    virtual TVec<string> getTrainCostNames() const;
    virtual TVec<string> getTestCostNames() const;

    virtual void train();

    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

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

DECLARE_OBJECT_PTR(NeighborhoodSmoothnessNNet);

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
