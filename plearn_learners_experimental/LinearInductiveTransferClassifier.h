// -*- C++ -*-

// LinearInductiveTransferClassifier.h
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

/*! \file LinearInductiveTransferClassifier.h */


#ifndef LinearInductiveTransferClassifier_INC
#define LinearInductiveTransferClassifier_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/opt/Optimizer.h>
#include <plearn/var/VarArray.h>
#include <plearn_learners/online/RBMLayer.h>

namespace PLearn {

/**
 * Linear classifier that uses class representations in
 * order to make use of inductive transfer between classes.
 */
class LinearInductiveTransferClassifier : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! Class representations
    Mat class_reps;
    //! Optimizer of the neural network
    PP<Optimizer> optimizer;
    //! Batch size
    int batch_size;
    //! Weight decay for all weights
    real weight_decay; 
    //! Penalty to use on the weights for weight decay
    string penalty_type;
    //! The method used to initialize the weights
    string initialization_method;
    //! Model type. Choose between:
    //!  - "discriminative"             (multiclass classifier)
    //!  - "discriminative_1_vs_all"    (1 vs all multitask classier)
    //!  - "generative"                 (gaussian classifier)
    string model_type;
    //! Indication that the targets seen in the training set
    //! should not be considered when tagging a new set
    bool dont_consider_train_targets;
    //! Indication that a bias should be used for weights prediction
    bool use_bias_in_weights_prediction;
    //! Indication that the classifier works with multiple targets,
    //! possibly ON simulatneously
    bool multi_target_classifier;
    //! Minimum variance for all coordinates, which is added
    //! to the maximum likelihood estimates.
    real sigma_min;
    //! Number of hidden units for neural network
    int nhidden;
    //! Number of RBM training to initialize hidden layer weights
    int rbm_nstages;
    //! Learning rate for the RBM
    real rbm_learning_rate;
    //! Visible layer of the RBM
    PP<RBMLayer> visible_layer;
    //! Hidden layer of the RBM
    PP<RBMLayer> hidden_layer;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    LinearInductiveTransferClassifier();


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
    PLEARN_DECLARE_OBJECT(LinearInductiveTransferClassifier);

    // Simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    //! Number of outputs for the neural network
    int noutputs;
    //! Input variable
    Var input;
    //! Output variable
    Var output;
    //! Sup output variable
    Var sup_output;
    //! New output variable
    Var new_output;
    //! Target variable
    Var target;
    //! Sup target variable
    Var sup_target;
    //! New target variable
    Var new_target;
    //! Sample weight variable
    Var sampleweight;
    //! Linear classifier parameters
    Var A;
    //! Linear classifier scale parameter
    Var s;
    //! Class representations
    Var class_reps_var;
    //! Costs variables
    VarArray costs; 
    //! Costs variables for new tasks
    VarArray new_costs; 
    //! Parameters
    VarArray params;
    //! Parameters vec
    Vec paramsvalues;
    //! Penalties variables
    VarArray penalties;
    //! Training cost variable
    Var training_cost; 
    //! Test costs variable
    Var test_costs;
    //! Input variables
    VarArray invars;
    //! Vec of seen targets in the training set
    Vec seen_targets;
    //! Vec of unseen targets in the training set    
    Vec unseen_targets;

    //! Function: input -> output
    mutable Func f; 
    //! Function: input & target -> output & test_costs
    mutable Func test_costf; 
    //! Function: output & target -> cost
    mutable Func output_and_target_to_cost; 
    //! Function: input & target -> output & test_costs
    mutable Func sup_test_costf; 
    //! Function: output & target -> cost
    mutable Func sup_output_and_target_to_cost; 

    // Neural networks variables
    //! Input to hidden layer weights 
    Var W;
//    //! Parameters for hidden to output layer weights prediction
//    VarArray As;
//    //! Parameters for input to hidden layer weights prediction
//    VarArray Ws;
//    //! Scale parameter for input to hidden layer weights prediction
//    VarArray s_hids;
//    //! Hidden layer neurons
//    VarArray hidden_neurons;
    
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

    //! Build the various functions used in the network.
    void buildFuncs(const Var& the_input, const Var& the_output, const Var& the_target, const Var& the_sampleweight);

    //! Fill a matrix of weights according to the 'initialization_method' specified.
    //! The 'clear_first_row' boolean indicates whether we should fill the first
    //! row with zeros.
    void fillWeights(const Var& weights, bool zero_first_row, 
                     real scale_with_this=-1);

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
DECLARE_OBJECT_PTR(LinearInductiveTransferClassifier);
  
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
