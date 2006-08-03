// -*- C++ -*-

// NnlmOnlineLearner.h
//
// Copyright (C) 2006 Pierre-Antoine Manzagol
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

// Authors: Pierre-Antoine Manzagol

/*! \file NnlmOnlineLearner.h */



#ifndef NnlmOnlineLearner_INC
#define NnlmOnlineLearner_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

class OnlineLearningModule;
class NnlmOutputLayer;
class NGramDistribution;

/**
 * Trains a Neural Network Language Model (NNLM).
 *
 * This learner is based upon the online module architecture.
 *
 * @todo
 * @deprecated
 * 
 */
class NnlmOnlineLearner : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! Defines which model is used
    string str_input_model;    // 'wrl' (default) or 'gnnl'
    string str_output_model;    // 'gaussian' (default) or 'softmax'


    //! --- Fixed (same in both models) part ----------------------------------

    //! Size of the real ditributed word representations
    int word_representation_size;

    //! Size of the semantic layer
    int semantic_layer_size;

    //! Neural part parameters
    real wrl_slr;
    real wrl_dc;
    real wrl_wd_l1;
    real wrl_wd_l2;
    real sl_slr;
    real sl_dc;
    real sl_wd_l1;
    real sl_wd_l2;

    //! --- Gaussian output model specific stuff ------------------------------

    //! Define behavior
    string str_gaussian_model_train_cost;
    string str_gaussian_model_learning;
    real gaussian_model_sigma2_min;
    real gaussian_model_dl_slr;

    //! Number of candidates to use from different sources in the gaussian model
    //! when we use the approx_discriminant cost
    // What to do if multiple sources suggest the same word? Do we have less candidates? Or compensate?
    int shared_candidates_size;    // frequent words on which we don't want to make mistakes
    int ngram_candidates_size;     // from a bigram
    int self_candidates_size;      // from the model itself. Should be used after training has gone a while
                                   // to keep ourselves on track (some words getting too high a score?)

    //! Used in determining the C sets of candidate words for normalization
    //! in the evaluated discriminant cost
    VMat ngram_train_set;

    //! --- Softmax output model specific stuff -------------------------------

    real sm_slr;
    real sm_dc;
    real sm_wd_l1;
    real sm_wd_l2;


    //#####  Public Learnt Options  ############################################

    //! Layers of the learner
    //! Separated between the fixed part which computes up to the "semantic layer"
    //! and the variable part (gaussian or softmax)
    TVec< PP<OnlineLearningModule> > modules;
    TVec< PP<OnlineLearningModule> > output_modules;

    //#####  Public NOT Options  ##############################################

    //! NNLM related - determined from train_set
    int vocabulary_size;
    int context_size;       // the train_set's input size -1 (because target is last input)

    //! --- Gaussian output model specific stuff ------------------------------

    // TODO THIS COULD BE A LEARNT OPTION
    //! Used in determining the C sets of candidate words for normalization
    //! in the evaluated discriminant cost
    PP<NGramDistribution> theNGram;

    // TODO THIS COULD BE A LEARNT OPTION
    //! Holds candidates
    TVec<int> shared_candidates;    // frequent (ie paying) words
    TVec< TVec<int> > candidates;   // context specific candidates


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    NnlmOnlineLearner();


    //#####  PLearner Member Functions  #######################################

    //! builds the layers, ie modules and output_modules
    void buildLayers();

    //! Specific to the gaussian model
    void buildCandidates();
    void reevaluateGaussianParameters() const;
    void evaluateGaussianCounts() const;

    //! Interfaces with the ProcessSymbolicSequenceVMatrix's getRow()
    void myGetExample(const VMat& example_set, int& sample, Vec& input, Vec& target, real& weight) const;

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    virtual void train();

    void test(VMat testset, PP<VecStatsCollector> test_stats, VMat testoutputs, VMat testcosts) const;

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    virtual void computeTrainCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;


    // *** SUBCLASS WRITING: ***
    // While in general not necessary, in case of particular needs
    // (efficiency concerns for ex) you may also want to overload
    // some of the following methods:
    // virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
    //                                    Vec& output, Vec& costs) const;
    // virtual void computeCostsOnly(const Vec& input, const Vec& target,
    //                               Vec& costs) const;
    // virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
    //                   VMat testoutputs=0, VMat testcosts=0) const;
    // virtual int nTestCosts() const;
    // virtual int nTrainCosts() const;
    // virtual void resetInternalState();
    // virtual bool isStatefulLearner() const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(NnlmOnlineLearner);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here


    //####  Not Options  ######################################################

    //! stores the input and output values of the functions
    TVec<Vec> values;
    //! stores the gradients
    TVec<Vec> gradients;

    //! for the second variable part of the model (starts from 'r',
    //! the semantic layer, on)
    TVec<Vec> output_values;
    TVec<Vec> output_gradients;


protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    //! Used for loops
    int nmodules;
    int output_nmodules;

    //! Holds model type
    int model_type;

    //! --- Gaussian output model specific stuff ------------------------------
    int gaussian_model_cost;
    int gaussian_model_learning;

    enum{MODEL_TYPE_GAUSSIAN, MODEL_TYPE_SOFTMAX};
    enum{GAUSSIAN_COST_DISCR=0, GAUSSIAN_COST_APPROX_DISCR=1, GAUSSIAN_COST_NON_DISCR=2};
    enum{GAUSSIAN_LEARNING_DISCR=0, GAUSSIAN_LEARNING_EMPIRICAL=1};
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(NnlmOnlineLearner);

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
