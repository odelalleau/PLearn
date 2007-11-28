// -*- C++ -*-

// FeatureSetNaiveBayesClassifier.h
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


#ifndef FeatureSetNaiveBayesClassifier_INC
#define FeatureSetNaiveBayesClassifier_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/math/PRandom.h>
#include <plearn/feat/FeatureSet.h>

namespace PLearn {
using namespace std;

/**
 * Naive Bayes classifier on a feature set space.
 */
class FeatureSetNaiveBayesClassifier: public PLearner
{

private:

    typedef PLearner inherited;
    
    //! Vector of possible target values
    mutable Vec target_values;
    //! Vector for output computations 
    mutable Vec output_comp;
    //! Row vector
    mutable Vec row;
    //! Features for each token
    mutable TVec< TVec<int> > feats;

    //! Temporary computations variable, used in fprop() and bprop()
    //! Care must be taken when using these variables,
    //! since they are used by many different functions
    mutable string str;
    mutable int nfeats;

protected:

    //! Total output size
    int total_output_size;
    //! Number of features per input token
    //! for which a distributed representation
    //! is computed.
    int total_feats_per_token;
    //! Number of feature sets
    int n_feat_sets;
    //! Feature input;
    mutable Vec feat_input;
    //! VMatrix used to get values to string mapping for input tokens
    mutable VMat val_string_reference_set;
    //! Possible target values mapping.
    mutable VMat target_values_reference_set;
    //! Random number generator for parameters initialization
    PP<PRandom> rgen;

public: 
    //! Feature-class pair counts
    TVec< TVec< hash_map<int,int> > > feature_class_counts;
    //! Sums of feature-class pair counts, over features
    TVec< TVec<int> > sum_feature_class_counts;
    //! Class counts
    TVec<int> class_counts;

public:

    // Build options:

    //! Indication that the set of possible targets vary from
    //! one input vector to another.
    bool possible_targets_vary;
    //! FeatureSets to apply on input
    TVec<PP<FeatureSet> > feat_sets;
    //! Indication that different estimations of 
    //! the posterior probability of a feature given a class
    //! should be used for different inputs.
    bool input_dependent_posterior_estimation;
    //! Add-delta smoothing constant
    real smoothing_constant;

private:
    void build_();

    int my_argmax(const Vec& vec, int default_compare=0) const;

public:

    FeatureSetNaiveBayesClassifier();
    virtual ~FeatureSetNaiveBayesClassifier();
    PLEARN_DECLARE_OBJECT(FeatureSetNaiveBayesClassifier);

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

    void getProbs(const Vec& inputv, Vec& outputv) const;

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

DECLARE_OBJECT_PTR(FeatureSetNaiveBayesClassifier);

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
