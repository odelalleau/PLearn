// -*- C++ -*-

// NeuralNet.h
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


#ifndef NeuralNet_INC
#define NeuralNet_INC

#include "Learner.h"
#include <plearn/opt/Optimizer.h>
//#include "Var_all.h"

namespace PLearn {
using namespace std;

/** @deprecated This class is DEPRECATED. Consider using NNet instead.
 */

class NeuralNet: public Learner
{
protected:
    Var input;  // Var(inputsize())
    Var target; // Var(targetsize()-weightsize())
    Var costweights; // Var(weightsize())
    Var target_and_weights;// hconcat(target&costweights)
    Var w1; // bias and weights of first hidden layer
    Var w2; // bias and weights of second hidden layer
    Var wout; // bias and weights of output layer
    Var wdirect; // bias and weights for direct in-to-out connection

    Var output;
    VarArray costs; // al costs of interest
    Var cost; // cost for one (input,target)

    VarArray params;  // all arameter input vars

    Vec paramsvalues; // values of all parameters
    Vec initial_paramsvalues;

public:
    Func f; // input -> output
    Func costf; // input & target -> output & cost
    Func output_and_target_to_cost; // output & target -> cost

public:

    typedef Learner inherited;

    // Build options inherited from learner:
    // inputsize, outputszie, targetsize, experiment_name, save_at_every_epoch 

    // Build options:
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

    bool global_weight_decay; // default: false
    bool direct_in_to_out; // should we include direct input to output connecitons? default: false
    string output_transfer_func; // tanh, sigmoid, softplus, softmax  (default: "" means no transfer function)
    int iseed; // seed for random nb generator, -1 = use time of day

    //! a list of cost functions to use in the form "[ cf1; cf2; cf3; ... ]"
    // where the cost functions can be one of mse, mse_onehot, NLL,
    // class_error or multiclass_error (no default), or semisupervised_prob_class
    // (if the latter, then the semisupervised_flatten_factor option must be set).
    Array<string> cost_funcs;  
    real semisupervised_flatten_factor; // see SemiSupervisedProbClassCostVariable for explanations
    Vec semisupervised_prior; // see SemiSupervisedProbClassCostVariable for explanations

    // Build options related to the optimization:
    PP<Optimizer> optimizer; // the optimizer to use (no default)

    int batch_size; // how many samples to use to estimate gradient before an update
                    // 0 means the whole training set (default: 1)

    int nepochs;   // how many times the optimizer gets to see the training set (default: 10000).

    string saveparams; // where to optionally save params after training

    Array<Vec> normalization;
private:
    void build_();

public:

    NeuralNet();
    virtual ~NeuralNet();
    PLEARN_DECLARE_OBJECT(NeuralNet);

    // Learner methods

    virtual void build();
    virtual void forget(); // simply calls initializeParams()
    void initializeParams();

    virtual void train(VMat training_set); 
    virtual void use(const Vec& inputvec, Vec& prediction); 

    virtual int costsize() const;
    virtual Array<string> costNames() const;
    virtual Array<string> testResultsNames() { return costNames(); }
    virtual void useAndCost(const Vec& inputvec, const Vec& targetvec, 
                            Vec outputvec, Vec costvec);
    void computeCost(const Vec& inputvec, const Vec& targetvec, 
                     const Vec& outputvec, const Vec& costvec);

    virtual void makeDeepCopyFromShallowCopy(CopiesMap &copies);
protected:
    static void declareOptions(OptionList& ol);

};

DECLARE_OBJECT_PTR(NeuralNet);

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
