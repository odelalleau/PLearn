// -*- C++ -*-

// StructuralLearner.h
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

/* *******************************************************      
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Pierre-Antoine Manzagol

/*! \file StructuralLearner.h */


#ifndef StructuralLearner_INC
#define StructuralLearner_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

/**
   Putain de code fait à la va-vite pour ICML
 */
class StructuralLearner : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    real start_learning_rate, decrease_constant;
    VMat auxiliary_task_train_set;
    real lambda;
    real abstention_threshold;
    real epsilon;
    int index_O;
    int nhidden;
    int n_auxiliary_wordproblems;
    bool separate_features;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    StructuralLearner();


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

    virtual void computeOutputWithFeatures(TVec<TVec<unsigned int> >& feats, Vec& output, bool use_theta=true, int begin_class = -1, int end_class = -1) const;

    void computeFeatures(const Vec& input, const Vec& target, int data_set, int
index, TVec< TVec<unsigned int> >& theFeatureGroups, char featureMask = 0x1F) const;

void updateFeatures(const Vec& input, const Vec& target,  TVec< TVec<unsigned int> >& theFeatureGroups, char
featureMask = 0x1F);


    //virtual void updateDynamicFeatures(hash_map<int, TVec<bool> > token_prediction, int token, int prediction);
        
    virtual void test(VMat testset, PP<VecStatsCollector> test_stats, VMat testoutputs=0, VMat testcosts=0) const;

    // *** SUBCLASS WRITING: ***
    // While in general not necessary, in case of particular needs 
    // (efficiency concerns for ex) you may also want to overload
    // some of the following methods:
    // virtual void computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const;
    // virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;
    // virtual int nTestCosts() const;
    // virtual int nTrainCosts() const;
    // virtual void resetInternalState();
    // virtual bool isStatefulLearner() const;

    
    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
    PLEARN_DECLARE_OBJECT(StructuralLearner);

    // Simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    //int ninputs_onehot; // 

    // For the model
    mutable TVec<Mat> ws, vs, thetas;
    mutable TVec<Mat> whids;

    // Features for an example
    mutable TVec< TVec<unsigned int> > feats;
    mutable unsigned int *current_features;

    //hash_map<int,TVec<bool> > token_prediction_train;
    //TVec<hash_map<int,TVec<bool> > > token_prediction_test;

    mutable Mat thetas_times_x;
    mutable Mat activations;

    // Bag of words features, over window of chunks, precomputed for
    // the training set
    //TVec< TVec<unsigned int> > bag_of_words_over_chunks;

    // For examples
    mutable Vec input, target, before_softmax, output, costs;
    mutable real weight;
    real learning_rate;

    // Temporary files for computeFeatures
    mutable TVec<unsigned int> currentFeatureGroup;
    mutable bool tag;
    mutable int size;
    mutable unsigned int fl;
    mutable std::string symbol;

    // Feature dimensions
    mutable TVec<unsigned int> fls;

    // Indices of auxiliary examples effectively used and their target ("most frequent word"-tag encoded, ie between 0 and 999)
    TMat< unsigned int > auxiliary_indices_current;
    TMat< unsigned int > auxiliary_indices_left;

    mutable std::map<int, int> plcw_bigram_mapping;   // maps "previous label - current word" bigrams seen in train_set to an index


protected:
    //#####  Protected Member Functions  ######################################
    
    //! Declares the class options.
    // (PLEASE IMPLEMENT IN .cc)
    static void declareOptions(OptionList& ol);

    //! Initialize the parameters. If 'set_seed' is set to false, the seed
    //! will not be set in this method (it will be assumed to be already
    //! initialized according to the 'seed' option).
    virtual void initializeParams(bool set_seed = true);

    //! Determines the most frequent words on the auxiliary example set and fills 
    //! auxiliary_indices_current, auxiliary_indices_left accordingly
    void initWordProblemsStructures();

    //! Build a map of "previous label - current word" bigrams seen in train_set to an index
    void initPreviousLabelCurrentWordBigramMapping();

    void buildTasksParameters(int nout, TVec<unsigned int> feat_lengths);
    void buildThetaParameters(TVec<unsigned int> feat_lengths);


private: 
    //#####  Private Member Functions  ########################################

    //! This does the actual building. 
    // (PLEASE IMPLEMENT IN .cc)
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(StructuralLearner);
  
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
