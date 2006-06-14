// -*- C++ -*-

// DeepFeatureExtractorNNet.h
//
// Copyright (C) 2006 Hugo Larochelle 
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file DeepFeatureExtractorNNet.h */


#ifndef DeepFeatureExtractorNNet_INC
#define DeepFeatureExtractorNNet_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/vmat/AppendNeighborsVMatrix.h>
#include <plearn/opt/Optimizer.h>
#include <plearn/var/VarArray.h>

namespace PLearn {

/**
 * Deep Neural Network that extracts features in an unsupervised way.
 */
class DeepFeatureExtractorNNet : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! Number of hidden units of each hidden layers to add
    TVec<int> nhidden_schedule;
    //! Optimizer of the neural network
    PP<Optimizer> optimizer;
    //! Optimizer of the supervised phase of the neural network.
    //! If not specified, then the same optimizer will always
    //! be used.
    PP<Optimizer> optimizer_supervised;
    //! Batch size
    int batch_size;
    //! Transfer function for the hidden nodes
    string hidden_transfer_func;
    //! Output transfer function, when all hidden layers are added 
    string output_transfer_func;
    //! Index of the layer that will be trained at the next call of train
    int nhidden_schedule_position;
    //! Cost function for the supervised phase
    TVec<string> cost_funcs;
    //! Weight decay for all weights
    real weight_decay; 
    //! Bias decay for all biases
    real bias_decay;   
    //! Penalty to use on the weights (for weight and bias decay)
    string penalty_type;
    //! Used only in the stable_cross_entropy cost function, to fight overfitting (0<=r<1)
    real classification_regularizer; 
    //! Used in the stable_cross_entropy cost function of the hidden activations, in the unsupervised stages (0<=r<1)
    real regularizer; 
    //! Margin requirement, used only with the margin_perceptron_cost cost function
    real margin;
    //! The method used to initialize the weights
    string initialization_method;
    //! Values of all parameters
    Vec paramsvalues; 
    //! Number of outputs for the neural network
    int noutputs;
    //! Use the same weights for the input and output weights for the autoassociators
    bool use_same_input_and_output_weights;
    //! Always use the reconstruction cost of the input, not of 
    //! the last layer. This option should be used if
    //! use_same_input_and_output_weights is true.
    bool always_reconstruct_input;
    //! Use the cubed value of the input of the activation functions
    bool use_activations_with_cubed_input;
    //! To simulate semi-supervised learning
    int use_n_first_as_supervised;
    //! Use only supervised part
    bool use_only_supervised_part;
    //! Always use supervised target
    bool always_use_supervised_target;
    //! Threshold on training set error relative improvement,
    //! before adding a new layer.
    //! If < 0, then the addition of layers must be done by
    //! the user.
    real relative_minimum_improvement;
    //! Input reconstruction error. The reconstruction error
    //! of the hidden layers will always be "cross_entropy".
    //! Choose among:
    //!   - "cross_entropy" (default, stable version)
    //!   - "mse"
    string input_reconstruction_error;
    //! Indication that the supervised phase
    //! should only train the last layer's parameters.
    bool dont_train_all_parameters;
    //! Indication that autoassociator regularisation cost should be used
    bool use_autoassociator_regularisation;
    //! Weight of autoassociator regularisation terms
    real autoassociator_regularisation_weight;
    //! Number of nearest neighbors 
    int k_nearest_neighbors_reconstruction;
    //! Weight of supervised signal used in addition
    //! to unsupervised signal in unsupervised phase.
    //! If <= 0, then supervised signal ignored.
    real supervised_signal_weight;
    //! Indication that the neighborhood of a point should
    //! be weighted based on the neighbor's local NLL
    bool use_neighborhood_weighting;
    //! Decay for neighborhood weighting computation
    real neighborhood_exponential_decay;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    DeepFeatureExtractorNNet();


    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of a
    //! fresh learner!).
    virtual void forget();
    
    //! The role of the train method is to bring the learner up to stage==nstages,
    //! updating the train_stats collector with training costs measured on-line
    //! in the process.
    virtual void train();

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output. 
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;
    
    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method). 
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes and 
    //! for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;


    // *** SUBCLASS WRITING: ***
    // While in general not necessary, in case of particular needs 
    // (efficiency concerns for ex) you may also want to overload
    // some of the following methods:
    virtual void computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const;
    // virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;
    // virtual void test(VMat testset, PP<VecStatsCollector> test_stats, VMat testoutputs=0, VMat testcosts=0) const;
    // virtual int nTestCosts() const;
    // virtual int nTrainCosts() const;
    // virtual void resetInternalState();
    // virtual bool isStatefulLearner() const;

    
    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(DeepFeatureExtractorNNet);

    // Simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    //! Index of the layer that is being trained at the current state
    int nhidden_schedule_current_position;
    //! Parameter variables
    VarArray params;
    //! Parameter variables to train
    VarArray params_to_train;
    //! Weights
    VarArray weights;
    //! Reconstruction weights
    VarArray reconstruction_weights;
    //! Biases
    VarArray biases;
    //! Input variables
    VarArray invars;
    //! Input variable
    Var input;
    //! Output variable
    Var output;
    //! Feature vector output
    Var feature_vector;
    //! Hidden representation variable
    Var hidden_representation;
    //! Neighbor indices
    Var neighbor_indices;
    //! Target variable
    Var target;
    //! Unsupervised target variable
    Var unsupervised_target;
    //! Sample weight variable
    Var sampleweight;
    //! Costs variables
    VarArray costs; 
    //! Penalties variables
    VarArray penalties;
    //! Training cost variable
    Var training_cost; 
    //! Test costs variable
    Var test_costs;
    //! Fake supervised data;
    VMat sup_train_set;
    //! Unsupervised data when using nearest neighbors
    VMat unsup_train_set;
    //! Unsupervised data when using nearest neighbors
    PP<AppendNeighborsVMatrix> knn_train_set;

    //! Function: input -> output
    mutable Func f; 
    //! Function: input & target -> output & test_costs
    mutable Func test_costf; 
    //! Function: output & target -> cost
    mutable Func output_and_target_to_cost; 
    //! Function from input space to learned function space
    mutable Func to_feature_vector;
    //! Different training_costs 
    //! used for autoassociator regularisation 
    TVec<VarArray> autoassociator_params;
    //! Different training_costs 
    //! used for autoassociator regularisation 
    VarArray autoassociator_training_costs;

protected:
    //#####  Protected Member Functions  ######################################
    
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Return a variable that is the hidden layer corresponding to given
    //! input and weights. If the 'default' transfer_func is used, we use the
    //! hidden_transfer_func option.
    Var hiddenLayer(const Var& input, const Var& weights, string transfer_func, Var& before_transfer_function, bool use_cubed_value=false);

    //! Build the output of the neural network, from the given input.
    //! The hidden layer is also made available in the 'hidden_layer' parameter.
    //! The output before the transfer function is applied is also made
    //! available in the 'before_transfer_func' parameter.
    void buildOutputFromInput(const Var& the_input, Var& hidden_layer, Var& before_transfer_func);

    //! Builds the target and sampleweight variables.
    void buildTargetAndWeight();

    //! Build the costs variable from other variables.
    void buildCosts(const Var& output, const Var& target, const Var& unsupervised_target, const Var& before_transfer_func, const Var& output_sup);

    //! Build the various functions used in the network.
    void buildFuncs(const Var& the_input, const Var& the_output, const Var& the_target, const Var& the_sampleweight);

    //! Fill a matrix of weights according to the 'initialization_method' specified.
    //! The 'clear_first_row' boolean indicates whether we should fill the first
    //! row with zeros.
    void fillWeights(const Var& weights, bool fill_first_row, real fill_with_this=0);

    //! Fill the costs penalties.
    virtual void buildPenalties();

private: 
    //#####  Private Member Functions  ########################################

    //! This does the actual building. 
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(DeepFeatureExtractorNNet);
  
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
