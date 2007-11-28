// -*- C++ -*-

// DistRepNNet.h
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
 * $Id: DistRepNNet.h 3994 2005-08-25 13:35:03Z chapados $
 ******************************************************* */


#ifndef DistRepNNet_INC
#define DistRepNNet_INC

#include "PLearner.h"
#include <plearn/opt/Optimizer.h>

namespace PLearn {
using namespace std;

class DistRepNNet: public PLearner
{

private:

    typedef PLearner inherited;
    
    //! Vector of possible target values
    mutable Vec target_values;
    //! Vector for output computations 
    mutable Vec output_comp;
    //! Row vector
    mutable Vec row;
    //! Token features vector
    mutable Vec tf;
    mutable TVec<string> options;

protected:

    //! Output of the neural network
    Var output;
    //! Costs of the neural network
    VarArray costs;
    //! Vars that use partial updates
    VarArray partial_update_vars;
    //! Penalties on the neural network's weights
    VarArray penalties;
    //! Training cost of the neural network
    //! It corresponds to costs[0] + weighted penalties
    Var training_cost;
    //! Training cost of the neural network for extra tasks
    VarArray training_cost_extra_tasks;
    //! Test costs
    Var test_costs; 
    //! Input variables for training cost Func
    VarArray invars;
    //! Input variables for extra tasks training cost Func
    TVec< VarArray > invars_extra_tasks;
    //! Parameters of the neural network
    VarArray params; 
    //! Weights of the activated features, in the 
    //! distributed representation predictor
    //VarArray activated_weights;
    //! Mapping between input and dictionaries
    TVec<int> input_to_dict_index;
    //! Target Dictionary index
    int target_dict_index;
    //! Temporary variables...
    real winputsparse_weight_decay;
    real winputsparse_bias_decay;

public: 
    //! Parameter values (linked to params in build() )
    //! It is put here to set these values instead of getting them by training
    Vec paramsvalues; // values of all parameters
    //! Input vector
    Var input;  
    //! Distributed representation input vector
    Var dp_input;  
    //! Target vector
    Var target; 
    //! Sample weight 
    Var sampleweight; 
    //! Bias and weights of first hidden layer
    Var w1;
    //! Bias and weights from input layer
    VarArray winputsparse;
    //! Weights of first hidden layer
    //Var w1target;
    //! Weights of first hidden layer of theta predictor
    Var w1theta;  
    //! Weights applied to the input of distributed representation predictor
    VarArray winputdistrep;
    //!  Bias and weights going to the output layer of distributed representation predictor
    Var woutdistrep;
    //! Bias and weights of second hidden layer
    Var w2; 
    //! bias and weights of output layer
    Var wout;
    //! Direct input to output weights
    Var direct_wout;
    //! Bias and weights of output layer, target part, when no hidden layers
    //Var wouttarget; 
    //! Bias and weights of output layer, for theta predictor
    Var wouttheta; 
    //! Bias used only if fixed_output_weights
    Var outbias;
    //! Distributed representation of all targets
    Var dp_all_targets;
    //! Features of the token
    Var token_features;
    //! Distributed representation
    Var dist_rep;
    //! Distributed representations 
    //VarArray dist_reps;    
    //! Dictionaries for the symbolic data
    TVec< PP<Dictionary> > dictionaries;
    //! Seen classes in training set
    //TVec<int> seen_target;
    //! Unseen classes in training set
    //oTVec<int> unseen_target;
    //! Function from input to output 
    mutable Func f;
    //! Function from input and target to output and test_costs
    mutable Func test_costf; 
    //! Function from token features to distributed representation
    mutable Func token_to_dist_rep;
    //! Function for training
    mutable Func paramf;

public:

    // Build options:

    //! Extra tasks
    TVec<VMat> extra_tasks;
    //! Number of hidden nunits in first hidden layer (default:0)
    int nhidden;
    //! Number of hidden units in second hidden layer (default:0)
    int nhidden2; 
    //! Number of hidden nunits in first hidden layer for extra tasks
    TVec<int> nhidden_extra_tasks;
    //! Number of hidden units in second hidden layer for extra tasks
    TVec<int> nhidden2_extra_tasks; 
    //! Number of hidden units of the neural network predictor for the hidden to output weights
    int nhidden_theta_predictor; 
    //! Number of hidden units of the neural network predictor for the distributed representation 
    int nhidden_dist_rep_predictor; 
    //! Weight decay (default:0)
    real weight_decay; 
    //! Bias decay (default:0)
    real bias_decay; 
    //! Weight decay for weights going from input layer
    real input_dist_rep_predictor_bias_decay;
    //! Weight decay for weights going from hidden layer
    real output_dist_rep_predictor_bias_decay;
    //! Weight decay for weights going from input layer
    real input_dist_rep_predictor_weight_decay;
    //! Weight decay for weights going from hidden layer
    real output_dist_rep_predictor_weight_decay;
    //! Weight decay for weights from input layer to first hidden layer (default:0)
    real layer1_weight_decay; 
    //! Bias decay for weights from input layer to first hidden layer (default:0)
    real layer1_bias_decay;   
    //! Weight decay for weights from input layer to first hidden layer of the theta-predictor (default:0)
    real layer1_theta_predictor_weight_decay; 
    //! Bias decay for weights from input layer to first hidden layer of the theta-predictor (default:0)
    real layer1_theta_predictor_bias_decay;   
    //! Weight decay for weights from first hidden layer to second hidden layer (default:0)
    real layer2_weight_decay; 
    //! Bias decay for weights from first hidden layer to second hidden layer (default:0)
    real layer2_bias_decay;   
    //! Weight decay for weights from last hidden layer to output layer (default:0)
    real output_layer_weight_decay; 
    //! Bias decay for weights from last hidden layer to output layer (default:0)
    real output_layer_bias_decay;
    //! Weight decay for weights from last hidden layer to output layer of the theta-predictor (default:0)
    real output_layer_theta_predictor_weight_decay; 
    //! Bias decay for weights from last hidden layer to output layer of the theta-predictor (default:0)
    real output_layer_theta_predictor_bias_decay;
    //! Weight decay for weights from input directly to output layer (default:0)
    real direct_in_to_out_weight_decay;
    //! Bias decay for weights from input directly to output layer (default:0)
    real direct_in_to_out_bias_decay;
    //! Margin requirement, used only with the margin_perceptron_cost cost function (default:1)
    real margin; 
    //! If true then the output weights are not learned. They are initialized to +1 or -1 randomly (default:false)
    bool fixed_output_weights;
    //! If true then direct input to output weights will be added (if nhidden > 0)
    bool direct_in_to_out;
    //! Penalty to use on the weights (for weight and bias decay) (default:"L2_square")
    string penalty_type; 
    //! Transfer function to use for ouput layer (default:"")
    string output_transfer_func; 
    //! Transfer function to use for hidden units (default:"tanh")
    string hidden_transfer_func; // tanh, sigmoid, softplus, softmax, etc...  (default: "tanh" means no transfer function)
    //! If set to 1, the weights won't be loaded nor initialized at build time (default:false)
    bool do_not_change_params;
    //! Cost functions.
    TVec<string> cost_funcs;  
    //! Build options related to the optimization:
    PP<Optimizer> optimizer; 
    //! Optimizers for extra tasks
    TVec< PP<Optimizer> > optimizer_extra_tasks;
    //! Number of samples to use to estimate gradient before an update.
    //! 0 means the whole training set (default: 1)
    int batch_size; 
    //! Method of initialization for neural network's weights
    string initialization_method;
    //! Dimensionality (number of components) of distributed representations
    TVec<int> dist_rep_dim;
    //! Number of tokens, for which to predict a distributed representation
    int ntokens;
    //! Number of tokens, for which to predict a distributed representation,
    //! for extra tasks
    TVec<int> ntokens_extra_tasks;
    //! Number of features per token
    int nfeatures_per_token;
    //! Number of features for each token
    //! (nfeatures_per_token is used if nfeatures_for_each_token.length()==0)
    TVec<int> nfeatures_for_each_token;
    //! Target dictionary
    PP<Dictionary> target_dictionary;
    //! Target distributed representations    
    Mat target_dist_rep;
    //! Indication that distributed representations should be used
    bool use_dist_reps;
    //! Indication that bases for output weights should be used
    bool use_output_weights_bases;
    //! Indication that the possible targets varies from
    //! one input vector to another
    //bool possible_targets_varies;
    //! Indication that the extra tasks will only 
    //! be used at the first epoch
    bool use_extra_tasks_only_on_first_epoch;
    //! Indication that the parameters on the sparse input
    //! should be initialized to zero
    bool initialize_sparse_params_to_zero;
    //! Architecture of the neural network
    //string nnet_architecture;

private:
    void build_();

public:

    DistRepNNet();
    virtual ~DistRepNNet();
    PLEARN_DECLARE_OBJECT(DistRepNNet);

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

    //! Gives distributed representation for token features.
    //! If nfeatures_for_each_token is defined, then token_features
    //! should contain the features for all tokens, and distributed
    //! representations of all the tokens are concatenated.
    void getTokenDistRep(TVec<string>& token_features, Vec& dist_rep);

    //! Methods to get the network's (learned) parameters.
    //virtual Mat getW1() {return w1->matValue;}
    //virtual Mat getW2() {return w2->matValue;}
    //virtual Mat getWout() {return wout->matValue;}

protected:
    static void declareOptions(OptionList& ol);

    //! Builds a sparse affine transformation from a set of weights and an input variable.
    Var buildSparseAffineTransform(VarArray weights, Var input, TVec<int> input_to_dict_index, int begin);

    //! Builds a sparse affine transformation weight penalty from a set of weights and an input variable.
    Var buildSparseAffineTransformWeightPenalty(VarArray weights, Var input, TVec<int> input_to_dict_index, int begin, real weight_decay, real bias_decay=0, string penalty_type="L2_square");

    //! Builds a var graph from a task, specified by task_index
    void buildVarGraph(int task_index);

    //! Initialize the parameters. If 'set_seed' is set to false, the seed
    //! will not be set in this method (it will be assumed to be already
    //! initialized according to the 'seed' option).
    //! The index of the extra task (-1 if main task) also needs to be
    //! provided.
    virtual void initializeParams(bool set_seed = true, int task_index = -1);

    //! Return a variable that is the result of the application of the
    //! given transfer function on the input variable
    Var add_transfer_func(const Var& input, string transfer_func = "default", VarArray mus=0, Var sigma=0);

    //! Build the output of the neural network, from the given input.
    void buildOutputFromInput(int task_index);

    //! Build the costs variable from other variables.
    void buildCosts(const Var& output, const Var& target, int task_index = -1);

    //! Build the various functions used in the network.
    void buildFuncs(VarArray& invars);

    //! Fill a matrix of weights according to the 'initialization_method' specified.
    //! The 'clear_first_row' boolean indicates whether we should fill the first
    //! row with zeros.
    void fillWeights(const Var& weights, bool clear_first_row, int use_this_to_scale=-1);

    //! Fill the costs penalties.
    virtual void buildPenalties(int this_ntokens);

};

DECLARE_OBJECT_PTR(DistRepNNet);

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
