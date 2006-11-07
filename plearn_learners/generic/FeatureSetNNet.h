// -*- C++ -*-

// FeatureSetNNet.h
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

/*! \file PLearnLibrary/PLearnAlgo/FeatureSetNNet.h */

#ifndef FeatureSetNNet_INC
#define FeatureSetNNet_INC

#include "PLearner.h"
#include <plearn/math/PRandom.h>
#include <plearn/feat/FeatureSet.h>

namespace PLearn {
using namespace std;

/**
 * Feedforward Neural Network for symbolic data represented using features.
 * Inspired from the NNet class, FeatureSetNNet is simply an extension that deals with
 * feature representations of symbolic data. It can also learn distributed representations
 * for each symbolic input token. The possible targets are defined by the VMatrix's
 * getValues() function.
 * 
 */
class FeatureSetNNet: public PLearner
{

private:

    typedef PLearner inherited;
    
    //! Vector of possible target values
    mutable Vec target_values;
    //! Vector for output computations 
    mutable Vec output_comp;
    //! Row vector
    mutable Vec row;
    //! Last layer of network (pointer to either nnet_input,
    //! vnhidden or vnhidden2)
    mutable Vec last_layer;
    //! Gradient of last layer in back propagation
    mutable Vec gradient_last_layer;
    //! Features for each token
    mutable TVec< TVec<int> > feats;

    //! Temporary computations variable, used in fprop() and bprop()
    //! Care must be taken when using these variables,
    //! since they are used by many different functions
    mutable Vec gradient;
    mutable string str;
    mutable real * pval1, * pval2, * pval3, * pval4, * pval5;
    mutable real val, val2, grad;
    mutable int offset;
    mutable int ni,nj,nk,id,nfeats,ifeats;
    mutable int* f;

protected:

    //! Total output size
    int total_output_size;
    //! Total updates so far;
    int total_updates;
    //! Number of feature sets
    int n_feat_sets;
    //! Number of features per input token
    //! for which a distributed representation
    //! is computed.
    int total_feats_per_token;
    //! Reindexed target
    mutable int reind_target;
    //! Feature input;
    mutable Vec feat_input;
    //! Gradient on feature input (useless for now)
    Vec gradient_feat_input;
    //! Input vector to NNet (after mapping into distributed representations)
    Vec nnet_input;
    //! Gradient for vector to NNet
    Vec gradient_nnet_input;
    //! First hidden layer value
    Vec hiddenv;
    //! Gradient of first hidden layer
    Vec gradient_hiddenv;
    //! Gradient through first hidden layer activation
    Vec gradient_act_hiddenv;
    //! Second hidden layer value
    Vec hidden2v;
    //! Gradient of second hidden layer
    Vec gradient_hidden2v;
    //! Gradient through second hidden layer activation
    Vec gradient_act_hidden2v;
    //! Gradient on output
    Vec gradient_outputv;
    //! Gradient throught output layer activation
    Vec gradient_act_outputv;
    //! Random number generator for parameters initialization
    PP<PRandom> rgen;
    //! Features seen in input since last update
    Vec feats_since_last_update;
    //! Possible target values seen since last update
    Vec target_values_since_last_update;
    //! VMatrix used to get values to string mapping for input tokens
    mutable VMat val_string_reference_set;
    //! Possible target values mapping.
    mutable VMat target_values_reference_set;

public: 
    //! Weights of first hidden layer
    Mat w1;
    //! Gradient on weights of first hidden layer
    Mat gradient_w1;
    //! Bias of first hidden layer
    Vec b1;
    //! Gradient on bias of first hidden layer
    Vec gradient_b1;
    //! Weights of second hidden layer
    Mat w2;
    //! gradient on weights of second hidden layer
    Mat gradient_w2;
    //! Bias of second hidden layer
    Vec b2;
    //! Gradient on bias of second hidden layer
    Vec gradient_b2;
    //! Weights of output layer
    Mat wout;
    //! Gradient on weights of output layer
    Mat gradient_wout;
    //! Bias of output layer
    Vec bout;
    //! Gradient on bias of output layer
    Vec gradient_bout;
    //! Direct input to output weights
    Mat direct_wout;
    //! Gradient on direct input to output weights
    Mat gradient_direct_wout;
    //! Direct input to output bias (empty, since no bias is used)
    Vec direct_bout;
    //! Gradient on direct input to output bias (empty, since no bias is used)
    Vec gradient_direct_bout;
    //! Weights of output layer for distributed
    //! representation predictor
    Mat wout_dist_rep;
    //! Gradient on weights of output layer for distributed
    //! representation predictor
    Mat gradient_wout_dist_rep;
    //! Bias of output layer for distributed
    //! representation predictor
    Vec bout_dist_rep;
    //! Gradient on bias of output layer for distributed
    //! representation predictor
    Vec gradient_bout_dist_rep;

public:

    // Build options:

    //! Number of hidden nunits in first hidden layer (default:0)
    int nhidden;
    //! Number of hidden units in second hidden layer (default:0)
    int nhidden2; 
    //! Weight decay (default:0)
    real weight_decay; 
    //! Bias decay (default:0)
    real bias_decay; 
    //! Weight decay for weights from input layer to first hidden layer 
    //! (default:0)
    real layer1_weight_decay; 
    //! Bias decay for weights from input layer to first hidden layer 
    //! (default:0)
    real layer1_bias_decay;   
    //! Weight decay for weights from first hidden layer to second hidden
    //! layer (default:0)
    real layer2_weight_decay; 
    //! Bias decay for weights from first hidden layer to second hidden 
    //! layer (default:0)
    real layer2_bias_decay;   
    //! Weight decay for weights from last hidden layer to output layer 
    //! (default:0)
    real output_layer_weight_decay; 
    //! Bias decay for weights from last hidden layer to output layer 
    //! (default:0)
    real output_layer_bias_decay;
    //! Weight decay for weights from input directly to output layer 
    //! (default:0)
    real direct_in_to_out_weight_decay;
    //! Weight decay for weights from last hidden layer to output layer
    //! of distributed representation predictor (default:0)
    real output_layer_dist_rep_weight_decay; 
    //! Bias decay for weights from last hidden layer to output layer of 
    //! distributed representation predictor (default:0)
    real output_layer_dist_rep_bias_decay;
    //! Margin requirement, used only with the margin_perceptron_cost 
    //! cost function (default:1)
    real margin; 
    //! If true then the output weights are not learned. 
    //! They are initialized to +1 or -1 randomly (default:false)
    bool fixed_output_weights;
    //! If true then direct input to output weights will be added 
    //! (if nhidden > 0)
    bool direct_in_to_out;
    //! Penalty to use on the weights (for weight and bias decay) 
    //! (default:"L2_square")
    string penalty_type; 
    //! Transfer function to use for ouput layer (default:"")
    string output_transfer_func; 
    //! Transfer function to use for hidden units (default:"tanh")
    //! tanh, sigmoid, softplus, softmax, etc...  
    string hidden_transfer_func; 
    //! Cost functions.
    TVec<string> cost_funcs;  
    //! Start learning rate of gradient descent
    real start_learning_rate;
    //! Decrease constant of gradietn descent
    real decrease_constant;
    //! Number of samples to use to estimate gradient before an update.
    //! 0 means the whole training set (default: 1)
    int batch_size; 
    //! Indication that a trick to speedup stochastic gradient descent
    //! should be used. 
    bool stochastic_gradient_descent_speedup;
    //! Method of initialization for neural network's weights
    string initialization_method;
    //! Dimensionality (number of components) of distributed representations
    //! If <= 0, than distributed representations will not be used.
    int dist_rep_dim;
    //! Indication that the set of possible targets vary from
    //! one input vector to another.
    bool possible_targets_vary;
    //! FeatureSets to apply on input
    TVec<PP<FeatureSet> > feat_sets;
    //! Indication that the input IDs should be used as the feature ID.
    //! The ID/string mapping provided by the input VMatrix Dictionary
    //! objects is hence used.
    //! This implies that all VMatrices (even those at test time) that
    //! provide the input vectors should use the same Dictionary objects.
    bool use_input_as_feature;

private:
    void build_();

    //! Softmax vector y obtained on x
    //! This implementation is such that 
    //! compute_softmax(x,x) is such that x
    //! becomes its softmax value.
    void compute_softmax(const Vec& x, const Vec& y) const;

    //! Negative log-likelihood loss
    real nll(const Vec& outputv, int target) const;
    
    //! Classification loss
    real classification_loss(const Vec& outputv, int target) const;
    
    //! Argmax function that lets you define the default (first)
    //! component used for comparisons. This is useful to avoid bias in the prediction
    //! when the getValues() provides some information about
    //! the prior distribution of the targets (e.g. the first target given by 
    //! getValues() is the most likely) and the output of the model is
    //! the same for all targets.
    int my_argmax(const Vec& vec, int default_compare=0) const;

public:

    FeatureSetNNet();
    virtual ~FeatureSetNNet();
    PLEARN_DECLARE_OBJECT(FeatureSetNNet);

    virtual void build();
    virtual void forget(); // simply calls initializeParams()

    virtual int outputsize() const;
    virtual TVec<string> getTrainCostNames() const;
    virtual TVec<string> getTestCostNames() const;

    virtual void train();

    virtual void computeOutput(const Vec& input, Vec& output) const;

    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                       Vec& output, Vec& costs) const;

    virtual void computeCostsFromOutputs(const Vec& input, 
                                         const Vec& output, 
                                         const Vec& target, 
                                         Vec& costs) const;

    virtual void makeDeepCopyFromShallowCopy(CopiesMap &copies);

protected:
    static void declareOptions(OptionList& ol);

    //! Forward propagation in the network
    void fprop(const Vec& inputv, Vec& outputv, const Vec& targetv, Vec& costsv, real sampleweight=1) const;

    //! Forward propagation to compute the output
    void fpropOutput(const Vec& inputv, Vec& outputv) const;

    //! Forward propagation to compute the costs from the output
    void fpropCostsFromOutput(const Vec& inputv, const Vec& outputv, const Vec& targetv, Vec& costsv, real sampleweight=1) const;

    //! Backward propagation in the network,
    //! which assumes that a forward propagation has been done
    //! before.
    //! A learning rate needs to be provided because it is
    //! -learning_rate * gradient that is propagated, not just the gradient.
    void bprop(Vec& inputv, Vec& outputv, Vec& targetv, Vec& costsv, real learning_rate, real sampleweight=1);

    //! Update network's parameters
    void update();

    //! Update affine transformation's parameters
    void update_affine_transform(Vec input, Mat weights, Vec bias,
                                 Mat gweights, Vec gbias,
                                 bool input_is_sparse, bool output_is_sparse,
                                 Vec output_indices);
    
    //! Clear network's propagation path gradient fields
    //! Assumes fprop and bprop have been called before.
    void clearProppathGradient();

    //! Initialize the parameters. If 'set_seed' is set to false, the seed
    //! will not be set in this method (it will be assumed to be already
    //! initialized according to the 'seed' option).
    //! The index of the extra task (-1 if main task) also needs to be
    //! provided.
    virtual void initializeParams(bool set_seed = true);

    //! Computes the result of the application of the
    //! given transfer function on the input vector
    void add_transfer_func(const Vec& input, 
                          string transfer_func = "default") const;

    //! Computes the gradient through the given activation function,
    //! the output value and the initial gradient on that output
    //! (i.e. before the activation function).
    //! After calling this function, gradient_act_output corresponds
    //! the gradient after the activation function.
    //! nll_softmax_speed_up_target is to speed up the gradient computation
    //! for the output layer using the softmax transfert function
    //! and the NLL cost function is applied.
    void gradient_transfer_func(Vec& output, Vec& gradient_input, 
                                Vec& gradient_output,                   
                                string transfer_func = "default",
                                int nll_softmax_speed_up_target=-1);

    //! Applies affine transform on input using provided weights
    //! and bias. Information about the nature of the input and output
    //! need to be provided.
    //! If bias.length() == 0, then output initial value is used as the bias.
    void add_affine_transform(Vec input, Mat weights, Vec bias, Vec output, 
                              bool input_is_sparse, bool output_is_sparse,
                              Vec output_indices = Vec(0)) const;

    //! Propagate gradient through affine transform on input using provided weights
    //! and bias. Information about the nature of the input and output
    //! need to be provided. 
    //! If bias.length() == 0, then no backprop is made to bias.
    void gradient_affine_transform(Vec input, Mat weights, Vec bias, 
                                   Vec ginput, Mat gweights, Vec gbias, Vec goutput, 
                                   bool input_is_sparse, bool output_is_sparse,
                                   real learning_rate,
                                   real weight_decay, real bias_decay,
                                   Vec output_indices = Vec(0));

    //! Propagate penalty gradient through weights and bias, 
    //! scaled by -learning rate.
    void gradient_penalty(Vec input, Mat weights, Vec bias, 
                          Mat gweights, Vec gbias,  
                          bool input_is_sparse, bool output_is_sparse,
                          real learning_rate,
                          real weight_decay, real bias_decay,
                          Vec output_indices = Vec(0));
    
    //! Fill a matrix of weights according to the 'initialization_method' 
    //! specified. 
    void fillWeights(const Mat& weights);

    //! Verify gradient of propagation path
    void verify_gradient(Vec& input, Vec target, real step);

    //! Verify gradient of affine_transform parameters
    void verify_gradient_affine_transform(
        Vec global_input, Vec& global_output, Vec& global_targetv, 
        Vec& global_costs, real sampleweight,
        Vec input, Mat weights, Vec bias,
        Mat est_gweights, Vec est_gbias, 
        bool input_is_sparse, bool output_is_sparse,
        real step,
        Vec output_indices = Vec(0)) const;
    
    void output_gradient_verification(Vec grad, Vec est_grad);

    //! Changes the reference_set and then calls the parent's class method
    void batchComputeOutputAndConfidence(VMat inputs, real probability,
                                         VMat outputs_and_confidence) const;
    //! Changes the reference_set and then calls the parent's class method
    virtual void use(VMat testset, VMat outputs) const;
    //! Changes the reference_set and then calls the parent's class method
    virtual void test(VMat testset, PP<VecStatsCollector> test_stats, 
                      VMat testoutputs=0, VMat testcosts=0) const;
    //! Changes the reference_set and then calls the parent's class method
    virtual VMat processDataSet(VMat dataset) const;
        
};

DECLARE_OBJECT_PTR(FeatureSetNNet);

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
