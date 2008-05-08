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
 * $Id$
 ******************************************************* */


#ifndef NNet_INC
#define NNet_INC

#include "PLearner.h"
#include <plearn/opt/Optimizer.h>

namespace PLearn {
using namespace std;

class NNet: public PLearner
{

private:

    typedef PLearner inherited;

protected:

    Var rbf_centers; // n_classes (or n_classes-1) x rbf_layer_size = mu_i of RBF gaussians
    Var rbf_sigmas; // n_classes (or n_classes-1) entries (sigma's of the RBFs)
    Var junk_prob; // scalar background (junk) probability, if first_class_is_junk
    Var alpha_adaboost;
    Var output;
    Var predicted_input;
    VarArray costs; // all costs of interest
    VarArray penalties;
    Var training_cost; // weighted scalar costs[0] including penalties
    Var test_costs; // hconcat(costs)
    VarArray invars;
    VarArray params;  // all arameter input vars

    //! Used to store the inputs in a bag when 'operate_on_bags' is true.
    Var bag_inputs;

    //! Used to store test inputs in a bag when 'operate_on_bags' is true.
    Mat store_bag_inputs;

    //! Used to store the size of a bag when 'operate_on_bags' is true.
    Var bag_size;

    //! Used to remember the size of a test bag.
    Vec store_bag_size;

    //! Number of bags in the training set.
    int n_training_bags;

// to put back later -- blip  Vec paramsvalues; // values of all parameters

public: // to set these values instead of getting them by training
    Vec paramsvalues; // values of all parameters
    Var input;  // Var(inputsize())
    Var target; // Var(targetsize()-weightsize())
    Var sampleweight; // Var(1) if train_set->hasWeights()

    Var w1; // bias and weights of first hidden layer
    Var w2; // bias and weights of second hidden layer
    Var wout; // bias and weights of output layer
    Var outbias; // bias used only if fixed_output_weights
    Var wdirect; // bias and weights for direct in-to-out connection
    Var wrec; // input reconstruction weights (optional), from hidden layer to predicted input

    //! Matrices used for the quadratic term in the 'ratio' transfer function.
    //! There is one matrix for each hidden neuron.
    VarArray v1, v2;

    // first hidden layer
    Var hidden_layer;

public:

    mutable Func input_to_output; // input -> output
    mutable Func test_costf; // input & target -> output & test_costs
    mutable Func output_and_target_to_cost; // output & target -> cost

public:

    // Build options inherited from learner:
    // inputsize, outputszie, targetsize, experiment_name, save_at_every_epoch 


    //#####  Public Build Options  ############################################

    /// number of hidden units in first hidden layer (default:0)
    int nhidden;

    /// number of hidden units in second hidden layer (default:0)
    int nhidden2;

    /**
     *  Number of output units. This gives this learner its outputsize.  It is
     *  typically of the same dimensionality as the target for regression
     *  problems.  But for classification problems where target is just the
     *  class number, noutputs is usually of dimensionality number of classes
     *  (as we want to output a score or probability vector, one per class).
     *
     *  The default value is 0, which is caught at build-time and gives an
     *  error.  If a value of -1 is put, noutputs is set from the targetsize of
     *  the trainingset the first time setTrainingSet() is called on the
     *  learner (appropriate for regression scenarios).  This allows using the
     *  learner as a 'template' without knowing in advance the number of
     *  outputs it should have to handle.  Future extensions will cover the
     *  case of automatically discovering the outputsize for classification.
     */
    int noutputs;

    bool operate_on_bags;
    int max_bag_size;

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

    string penalty_type; // default: "L2_square"
    bool L1_penalty; // default: false - deprecated, set "penalty_type" to "L1"

    real input_reconstruction_penalty; // default = 0
    bool direct_in_to_out; // should we include direct input to output connecitons? default: false
    string output_transfer_func; // tanh, sigmoid, softplus, softmax, etc...  (default: "" means no transfer function)
    string hidden_transfer_func; // tanh, sigmoid, softplus, softmax, etc...  (default: "tanh" means no transfer function)
    real interval_minval, interval_maxval; // if output_transfer_func = interval(minval,maxval), these are the interval bounds

    bool do_not_change_params;

    Var first_hidden_layer;
    bool first_hidden_layer_is_output;
    bool transpose_first_hidden_layer;
    int n_non_params_in_first_hidden_layer;

    //! Cost functions.
    TVec<string> cost_funcs;  

    // Build options related to the optimization:
    PP<Optimizer> optimizer; // the optimizer to use (no default)

    int batch_size; // how many samples to use to estimate gradient before an update
    // 0 means the whole training set (default: 1)

    string initialization_method;
    int ratio_rank;


private:
    void build_();

public:

    NNet();
    PLEARN_DECLARE_OBJECT(NNet);

    virtual void build();
    virtual void forget(); // simply calls initializeParams()

    /**
     *  Overridden to support the case where noutputs==-1, in which case
     *  noutputs is set automatically from the targetsize of the training set
     *  (correct for the regression case; should be extended to cover
     *  classification scenarios in the future as well.)
     */
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

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

    //! Methods to get the network's (learned) parameters.
    virtual Mat getW1() {return w1->matValue;}
    virtual Mat getW2() {return w2->matValue;}
    virtual Mat getWdirect() {return wdirect->matValue;}
    virtual Mat getWout() {return wout->matValue;}

protected:
    static void declareOptions(OptionList& ol);

    //! Initialize the parameters. If 'set_seed' is set to false, the seed
    //! will not be set in this method (it will be assumed to be already
    //! initialized according to the 'seed' option).
    virtual void initializeParams(bool set_seed = true);

    //! Return a variable that is the hidden layer corresponding to given
    //! input and weights. If the 'default' transfer_func is used, we use the
    //! hidden_transfer_func option.
    Var hiddenLayer(const Var& input, const Var& weights, string transfer_func = "default",
                    VarArray* ratio_quad_weights = NULL);

    //! Build the output of the neural network, from the given input.
    //! The hidden layer is also made available in the 'hidden_layer' parameter.
    //! The output before the transfer function is applied is also made
    //! available in the 'before_transfer_func' parameter.
    void buildOutputFromInput(const Var& the_input, Var& hidden_layer, Var& before_transfer_func);

    //! Build the output for a whole bag, from the network defined by the
    //! 'input' to 'before_transfer_func' variables.
    //! 'before_transfer_func' is modified so as to hold the activations for
    //! the bag rather than for individual samples.
    void buildBagOutputFromBagInputs(
        const Var& input, Var& before_transfer_func,
        const Var& bag_inputs, const Var& bag_size, Var& bag_output);

    //! Builds the target and sampleweight variables.
    void buildTargetAndWeight();

    //! Build the costs variable from other variables.
    void buildCosts(const Var& output, const Var& target, const Var& hidden_layer, const Var& before_transfer_func);

    //! Build the various functions used in the network.
    void buildFuncs(const Var& the_input, const Var& the_output, const Var& the_target, const Var& the_sampleweight, const Var& the_bag_size);

    //! Compute the final output from the activations of the output units.
    void applyTransferFunc(const Var& before_transfer_func, Var& output);

    //! Fill a matrix of weights according to the 'initialization_method' specified.
    //! The 'clear_first_row' boolean indicates whether we should fill the first
    //! row with zeros.
    void fillWeights(const Var& weights, bool clear_first_row);

    //! Fill the costs penalties.
    virtual void buildPenalties(const Var& hidden_layer);

};

DECLARE_OBJECT_PTR(NNet);

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
