// -*- C++ -*-

// FeatureSetNaiveBayesClassifier.cc
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

/*! \file PLearnLibrary/PLearnAlgo/FeatureSetNaiveBayesClassifier.h */


#include "FeatureSetNaiveBayesClassifier.h"
#include <plearn/vmat/SubVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(FeatureSetNaiveBayesClassifier, "Naive Bayes classifier on a feature set space.", 
                        "This classifier is bases on the estimation of\n"
                        "P(y|x) where y is a class and x is the input.\n"
                        "In this naive bayes model, we have:\n"
                        "  P(y|x) = P(y) \\prod_ji P(f_i(x_j)|y)\n"
                        "where f_i(x_j) is the ith feature of the jth\n"
                        "component of x. P(y) and P(f_(x_j)|y) are\n"
                        "estimated by maximum likelihood, but a smoothing\n"
                        "additive constant to the observation counts\n"
                        "can be specified. Feature sets must also be\n"
                        "provided.");

FeatureSetNaiveBayesClassifier::FeatureSetNaiveBayesClassifier() // DEFAULT VALUES FOR ALL OPTIONS
    :
rgen(new PRandom()),
possible_targets_vary(0),
input_dependent_posterior_estimation(0),
smoothing_constant(0)
{}

FeatureSetNaiveBayesClassifier::~FeatureSetNaiveBayesClassifier()
{
}

void FeatureSetNaiveBayesClassifier::declareOptions(OptionList& ol)
{
    declareOption(ol, "possible_targets_vary", &FeatureSetNaiveBayesClassifier::possible_targets_vary, 
                  OptionBase::buildoption, 
                  "Indication that the set of possible targets vary from\n"
                  "one input vector to another.\n");

    declareOption(ol, "feat_sets", &FeatureSetNaiveBayesClassifier::feat_sets, 
                  OptionBase::buildoption, 
                  "FeatureSets to apply on input.\n");

    declareOption(ol, "input_dependent_posterior_estimation", &FeatureSetNaiveBayesClassifier::input_dependent_posterior_estimation, 
                  OptionBase::buildoption, 
                  "Indication that different estimations of\n"
                  "the posterior probability of a feature given a class\n"
                  "should be used for different inputs.\n");
    
    declareOption(ol, "smoothing_constant", &FeatureSetNaiveBayesClassifier::smoothing_constant, 
                  OptionBase::buildoption, 
                  "Add-delta smoothing constant.\n");
    
    declareOption(ol, "feature_class_counts", &FeatureSetNaiveBayesClassifier::feature_class_counts, 
                  OptionBase::learntoption, 
                  "Feature-class pair counts.\n");
    
    declareOption(ol, "sum_feature_class_counts", &FeatureSetNaiveBayesClassifier::sum_feature_class_counts, 
                  OptionBase::learntoption, 
                  "Sums of feature-class pair counts, over features.\n");
    
    declareOption(ol, "class_counts", &FeatureSetNaiveBayesClassifier::class_counts, 
                  OptionBase::learntoption, 
                  "Class counts.\n");
    
    inherited::declareOptions(ol);

}

///////////
// build //
///////////
void FeatureSetNaiveBayesClassifier::build()
{
    inherited::build();
    build_();
}


////////////
// build_ //
////////////
void FeatureSetNaiveBayesClassifier::build_()
{
    // Don't do anything if we don't have a train_set
    // It's the only one who knows the inputsize, targetsize and weightsize

    if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
    {
        if(targetsize_ != 1)
            PLERROR("In FeatureSetNaiveBayesClassifier::build_(): targetsize_ must be 1, not %d",targetsize_);

        if(weightsize_ > 0)
            PLERROR("In FeatureSetNaiveBayesClassifier::build_(): weightsize_ > 0 is not supported");

        n_feat_sets = feat_sets.length();
        if(n_feat_sets == 0)
            PLERROR("In FeatureSetNaiveBayesClassifier::build_(): at least one FeatureSet must be provided\n");

        if(inputsize_ % n_feat_sets != 0)
            PLERROR("In FeatureSetNaiveBayesClassifier::build_(): feat_sets.length() must be a divisor of inputsize()");

        PP<Dictionary> dict = train_set->getDictionary(inputsize_);
        total_output_size = dict->size() + (dict->oov_not_in_possible_values ? 0 : 1);

        total_feats_per_token = 0;
        for(int i=0; i<n_feat_sets; i++)
            total_feats_per_token += feat_sets[i]->size();

        if(stage <= 0)
        {
            if(input_dependent_posterior_estimation)
            {
                feature_class_counts.resize(inputsize_/n_feat_sets);
                sum_feature_class_counts.resize(inputsize_/n_feat_sets);
                for(int i=0; i<feature_class_counts.length(); i++)
                {
                    feature_class_counts[i].resize(total_output_size);
                    sum_feature_class_counts[i].resize(total_output_size);
                    for(int j=0; j<total_output_size; j++)
                    {
                        feature_class_counts[i][j].clear();
                        sum_feature_class_counts[i][j] = 0;
                    }
                }

                class_counts.resize(total_output_size);
                class_counts.fill(0);
            }
            else
            {
                feature_class_counts.resize(1);
                sum_feature_class_counts.resize(1);
                feature_class_counts[0].resize(total_output_size);
                sum_feature_class_counts[0].resize(total_output_size);
                for(int j=0; j<total_output_size; j++)
                {
                    feature_class_counts[0][j].clear();
                    sum_feature_class_counts[0][j] = 0;
                }
                
                class_counts.resize(total_output_size);
                class_counts.fill(0);
            }
        }
                
        output_comp.resize(total_output_size);
        row.resize(train_set->width());
        row.fill(MISSING_VALUE);
        feats.resize(inputsize_);
        // Making sure that all feats[i] have non null storage...
        for(int i=0; i<feats.length(); i++)
        {
            feats[i].resize(1);
            feats[i].resize(0);
        }
        val_string_reference_set = train_set;
        target_values_reference_set = train_set;

        if (seed_>=0)
            rgen->manual_seed(seed_);
    }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void FeatureSetNaiveBayesClassifier::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, 
                                   const Vec& targetv, Vec& costsv) const
{
    PLERROR("In FeatureSetNaiveBayesClassifier::computeCostsFromOutputs(): output is not enough to compute costs");
}

int FeatureSetNaiveBayesClassifier::my_argmax(const Vec& vec, int default_compare) const
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN int argmax(const TVec<T>& vec) vec has zero length");
#endif
    real* v = vec.data();
    int indexmax = default_compare;
    real maxval = v[default_compare];
    for(int i=0; i<vec.length(); i++)
        if(v[i]>maxval)
        {
            maxval = v[i];
            indexmax = i;
        }
    return indexmax;
}


///////////////////
// computeOutput //
///////////////////
void FeatureSetNaiveBayesClassifier::computeOutput(const Vec& inputv, Vec& outputv) const
{
    getProbs(inputv,output_comp);
    if(possible_targets_vary)
        outputv[0] = target_values[my_argmax(output_comp,rgen->uniform_multinomial_sample(output_comp.length()))];
    else
        outputv[0] = argmax(output_comp);
}

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void FeatureSetNaiveBayesClassifier::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                 Vec& outputv, Vec& costsv) const
{
    getProbs(inputv,output_comp);
    if(possible_targets_vary)
        outputv[0] = target_values[my_argmax(output_comp,rgen->uniform_multinomial_sample(output_comp.length()))];
    else
        outputv[0] = argmax(output_comp);
    costsv[0] = (outputv[0] == targetv[0] ? 0 : 1);
}

////////////
// forget //
////////////
void FeatureSetNaiveBayesClassifier::forget()
{
    stage = 0;
    if (train_set) build();
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> FeatureSetNaiveBayesClassifier::getTrainCostNames() const
{
    TVec<string> ret(1);
    ret[0] = "class_error";
    return ret;
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> FeatureSetNaiveBayesClassifier::getTestCostNames() const
{ 
    return getTrainCostNames();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void FeatureSetNaiveBayesClassifier::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // Private variables
    deepCopyField(target_values,copies);
    deepCopyField(output_comp,copies);
    deepCopyField(row,copies);
    deepCopyField(feats,copies);

    // Protected variables
    deepCopyField(val_string_reference_set,copies);
    deepCopyField(target_values_reference_set,copies);
    deepCopyField(rgen,copies);

    // Public variables
    deepCopyField(feature_class_counts,copies);
    deepCopyField(sum_feature_class_counts,copies);
    deepCopyField(class_counts,copies);

    // Public build options
    deepCopyField(feat_sets,copies);
}

////////////////
// outputsize //
////////////////
int FeatureSetNaiveBayesClassifier::outputsize() const {
    return targetsize_;
}

///////////
// train //
///////////
void FeatureSetNaiveBayesClassifier::train()
{
    if(!train_set)
        PLERROR("In FeatureSetNaiveBayesClassifier::train, you did not setTrainingSet");

    //if(!train_stats)
    //    PLERROR("In FeatureSetNaiveBayesClassifier::train, you did not setTrainStatsCollector");
 
    Vec outputv(outputsize());
    Vec costsv(getTrainCostNames().length());
    Vec inputv(train_set->inputsize());
    Vec targetv(train_set->targetsize());
    real sample_weight=1;
    int l = train_set->length();  

    if(stage == 0)
    {
        ProgressBar* pb = 0;
        if(report_progress)
            pb = new ProgressBar("Training " + classname() 
                                 + " from stage 0 to " + tostring(l), l);
        int id = 0;
        for(int t=0; t<l;t++)
        {
            train_set->getExample(t,inputv,targetv,sample_weight);

            // Get possible target values
            if(possible_targets_vary) 
            {
                row.subVec(0,inputsize_) << inputv;
                train_set->getValues(row,inputsize_,
                                     target_values);
                output_comp.resize(target_values.length());
            }
            
            // Get features
            nfeats = 0;
            for(int i=0; i<inputsize_; i++)
            {
                str = train_set->getValString(i,inputv[i]);
                feat_sets[i%n_feat_sets]->getFeatures(str,feats[i]);
                nfeats += feats[i].length();
            }

            for(int i=0; i<inputsize_; i++)
            {
                for(int j=0; j<feats[i].length(); j++)
                {
                    if(input_dependent_posterior_estimation)
                        id = i/n_feat_sets;
                    else
                        id = 0;

                    if(feature_class_counts[id][(int)targetv[0]].find(feats[i][j]) == feature_class_counts[id][(int)targetv[0]].end())
                        feature_class_counts[id][(int)targetv[0]][feats[i][j]] = 1;
                    else
                        feature_class_counts[id][(int)targetv[0]][feats[i][j]] += 1;

                    sum_feature_class_counts[id][(int)targetv[0]] += 1;
                }
            }

            class_counts[(int)targetv[0]] += 1;

            computeOutputAndCosts(inputv, targetv, outputv, costsv);
            train_stats->update(costsv);

            if(pb) pb->update(t);
        }
        if(pb) delete pb;
        stage = 1;
        train_stats->finalize();
        if(verbosity>1)
            cout << "Epoch " << stage << " train objective: " 
                 << train_stats->getMean() << endl;
    }
}

void FeatureSetNaiveBayesClassifier::getProbs(const Vec& inputv, Vec& outputv) const
{
    // Get possible target values
    if(possible_targets_vary) 
    {
        row.subVec(0,inputsize_) << inputv;
        target_values_reference_set->getValues(row,inputsize_,
                                               target_values);
        outputv.resize(target_values.length());
    }
    
    // Get features
    nfeats = 0;
    for(int i=0; i<inputsize_; i++)
    {
        str = val_string_reference_set->getValString(i,inputv[i]);
        feat_sets[i%n_feat_sets]->getFeatures(str,feats[i]);
        nfeats += feats[i].length();
    }
    int id=0;
    
    if(possible_targets_vary)
    {
        for(int i=0; i<target_values.length(); i++)
        {
            outputv[i] = safeflog(class_counts[(int)target_values[i]]);
            for(int k=0; k<inputsize_; k++)
            {
                if(input_dependent_posterior_estimation)
                    id = k/n_feat_sets;
                else
                    id = 0;
                
                for(int j=0; j<feats[k].length(); j++)
                {
                    outputv[i] -= safeflog(sum_feature_class_counts[id][(int)target_values[i]] + smoothing_constant*total_feats_per_token);
                    if(feature_class_counts[id][(int)target_values[i]].find(feats[k][j]) == feature_class_counts[id][(int)target_values[i]].end())
                        outputv[i] += safeflog(smoothing_constant);
                    else
                        outputv[i] += safeflog(feature_class_counts[id][(int)target_values[i]][feats[k][j]]+smoothing_constant);
                }
            }
        }
    }
    else
    {
        for(int i=0; i<total_output_size; i++)
        {
            outputv[i] = safeflog(class_counts[i]);
            for(int k=0; k<inputsize_; k++)
            {
                if(input_dependent_posterior_estimation)
                    id = k/n_feat_sets;
                else
                    id = 0;
                
                for(int j=0; j<feats[k].length(); j++)
                {
                    outputv[i] -= safeflog(sum_feature_class_counts[id][i] + smoothing_constant*total_feats_per_token);
                    if(feature_class_counts[id][i].find(feats[k][j]) == feature_class_counts[id][i].end())
                        outputv[i] += safeflog(smoothing_constant);
                    else
                        outputv[i] += safeflog(feature_class_counts[id][i][feats[k][j]]+smoothing_constant);
                }
            }
        }
    }
}

void FeatureSetNaiveBayesClassifier::batchComputeOutputAndConfidence(VMat inputs, real probability,
                                         VMat outputs_and_confidence) const
{
    val_string_reference_set = inputs;
    inherited::batchComputeOutputAndConfidence(inputs,probability,outputs_and_confidence);
    val_string_reference_set = train_set;
}

void FeatureSetNaiveBayesClassifier::use(VMat testset, VMat outputs) const
{
    val_string_reference_set = testset;
    if(testset->width() > train_set->inputsize())
        target_values_reference_set = testset;
    target_values_reference_set = testset;
    inherited::use(testset,outputs);
    val_string_reference_set = train_set;
    if(testset->width() > train_set->inputsize())
        target_values_reference_set = train_set;
}

void FeatureSetNaiveBayesClassifier::test(VMat testset, PP<VecStatsCollector> test_stats, 
                      VMat testoutputs, VMat testcosts) const
{
    val_string_reference_set = testset;
    target_values_reference_set = testset;
    inherited::test(testset,test_stats,testoutputs,testcosts);
    val_string_reference_set = train_set;
    target_values_reference_set = train_set;
}

VMat FeatureSetNaiveBayesClassifier::processDataSet(VMat dataset) const
{
    VMat ret;
    val_string_reference_set = dataset;
    // Assumes it contains the target part information
    if(dataset->width() > train_set->inputsize())
        target_values_reference_set = dataset;
    ret = inherited::processDataSet(dataset);
    val_string_reference_set = train_set;
    if(dataset->width() > train_set->inputsize())
        target_values_reference_set = train_set;
    return ret;
}

} // end of namespace PLearn


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
