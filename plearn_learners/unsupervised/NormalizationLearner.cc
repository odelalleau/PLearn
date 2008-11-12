// -*- C++ -*-

// NormalizationLearner.cc
//
// Copyright (C) 2006 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file NormalizationLearner.cc */


#include "NormalizationLearner.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NormalizationLearner,
    "This learner can perform normalization of its input (subtracting the mean and dividing by stddev) "
    "and can also exclude input components that have too low standard deviation, or too many missing values.",
    "NormalizationLearner produces as output a possibly normalized version of its input\n"
    "obtained by subtracting the mean and dividing by the standard deviation.\n"
    "It can also exclude input components whose standard deviation is below a specified value,\n"
    "or whose missing values exceed a certain proportion of times."
    "It also has a simple policy switch for deciding to keep missing values as is or replace them by 0.\n"
    "It is typically used as an early preprocessing step in a ChainedLearner \n"
    "NOTE: you may also consider using PCA(normalize=1), if you wan to obtain \n"
    "decorrelated 'sphered' data." );

NormalizationLearner::NormalizationLearner()
    :min_allowed_stddev(1e-6),
     remove_components_with_stddev_smaller_than(-1),
     remove_components_whose_missing_proportion_exceeds(1),
     set_missing_to_zero(true),
     do_normalize(true)
{
}

void NormalizationLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "min_allowed_stddev", &NormalizationLearner::min_allowed_stddev,
                  OptionBase::buildoption,
                  "If the empirical standard deviation is lower than this, we'll use this value to \n"
                  "compute inv_stddev (this is to prevent getting too large or even infinite values for inv_stddev");

    declareOption(ol, "remove_components_with_stddev_smaller_than", &NormalizationLearner::remove_components_with_stddev_smaller_than,
                  OptionBase::buildoption,
                  "Components of the input whose stddev is strictly below that value will be excluded from the output\n");

    declareOption(ol, "remove_components_whose_missing_proportion_exceeds", &NormalizationLearner::remove_components_whose_missing_proportion_exceeds,
                  OptionBase::buildoption,
                  "Components of the input that are missing more than that given fraction of times will be excluded from the output\n"
                  "The default 1 means no component will be excluded for being missing.\n"
                  "At the other extreme 0 means any component that was missing at east once wil be excluded\n"
                  "0.75 would exclude components that are missing more than 75\% of the time\n");

    declareOption(ol, "do_normalize", &NormalizationLearner::do_normalize,
                  OptionBase::buildoption,
                  "If true (the default) then subtract mean and divide by stddev.\n"
                  "It can be useful to set this to false if all you want to do is remove components with small\n"
                  "stddev (see option remove_components with_small_stddev) but leave the others untouched.");

    declareOption(ol, "set_missing_to_zero", &NormalizationLearner::set_missing_to_zero,
                  OptionBase::buildoption,
                  "How to handle missing values: \n"
                  "  If true (the default), missing values will be replaced by 0\n"
                  "  (this corresponds to post-normalization mean if indeed we d_normakize) \n"
                  "  If false missing values will be left as missing values. \n");

    declareOption(ol, "meanvec", &NormalizationLearner::meanvec,
                  OptionBase::learntoption,
                  "The empirical mean to subtract from the input\n");

    declareOption(ol, "inv_stddev", &NormalizationLearner::inv_stddev,
                  OptionBase::learntoption,
                  "The vector of factors by which to multiply (input-meanvec)\n");

    declareOption(ol, "kept_components", &NormalizationLearner::kept_components,
                  OptionBase::learntoption,
                  "The indices of the input components kept in the final output\n");

    declareOption(ol, "inputnames", &NormalizationLearner::inputnames,
                  OptionBase::learntoption,
                  "We store the inputnames, which are also the outputnames\n");



    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void NormalizationLearner::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.

    int d = meanvec.length();
    if(d>0 && kept_components.length()==0) // fill uninitialized kept_components
    {
        kept_components.resize(d);
        for(int k=0; k<d; k++)
            kept_components[k] = k;
    }
}

// ### Nothing to add here, simply calls build_
void NormalizationLearner::build()
{
    inherited::build();
    build_();
}


void NormalizationLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("NormalizationLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int NormalizationLearner::outputsize() const
{
    return kept_components.length();
}

void NormalizationLearner::forget()
{
    inherited::forget();
    stage = 0;
}

void NormalizationLearner::train()
{
    if (!initTrain())
        return;

    if(stage<1)
    {
        inputnames = train_set->inputFieldNames();

        train_stats->forget();
        int l = train_set->length();
        int n = train_set->inputsize();
        Vec input;
        Vec target;
        real weight;

        VecStatsCollector st;
        PP<ProgressBar> pb;
        if(report_progress)
            pb = new ProgressBar("NormalizationLearner computing statistics ",l);

        for(int i=0; i<l; i++)
        {
            train_set->getExample(i, input, target, weight);
            st.update(input, weight);
            if(pb)
                pb->update(i);
        }
        st.finalize();

        st.getMean(meanvec);
        inv_stddev.resize(n);
        kept_components.resize(n);
        kept_components.resize(0);
        for(int k=0; k<n; k++)
        {
            const StatsCollector& stk = st.stats[k];
            real sd = stk.stddev();
            inv_stddev[k] = 1/max(min_allowed_stddev,sd);
            double missing_proportion = (double)stk.nmissing()/(double)l;
            if( (missing_proportion<=remove_components_whose_missing_proportion_exceeds)
                && (sd>=remove_components_with_stddev_smaller_than) )
                kept_components.append(k);
        }
        ++stage;
        train_stats->finalize(); 
    }
}


void NormalizationLearner::computeOutput(const Vec& input, Vec& output) const
{
    int n = meanvec.length();
    if(input.length()!=n)
        PLERROR("length of input differs from length of meanvec!");
    int n2 = kept_components.length();
    output.resize(n2);
    real* p_input = input.data();
    real* p_output = output.data();
    real* p_meanvec = meanvec.data();
    real* p_inv_stddev = inv_stddev.data();
    int*  p_kept_components = kept_components.data();

    for(int k=0; k<n2; k++)
    {
        int pos = p_kept_components[k];
        real val = p_input[pos];
        if(is_missing(val))
            p_output[k] = set_missing_to_zero?0.:val;
        else if(do_normalize)
            p_output[k] = p_inv_stddev[pos]*(val - p_meanvec[pos]);
    }
}

void NormalizationLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    costs.resize(0);
}

TVec<string> NormalizationLearner::getTestCostNames() const
{
    return TVec<string>();
}

TVec<string> NormalizationLearner::getTrainCostNames() const
{
    return TVec<string>();
}

TVec<string> NormalizationLearner::getOutputNames() const
{
    TVec<string> outnames;
    if(kept_components.length()==inputnames.length())
        outnames = inputnames;
    else
    {
        int n2 = kept_components.length();
        outnames.resize(n2);
        for(int k=0; k<n2; k++)
            outnames[k] = inputnames[kept_components[k]];
    }
    return outnames;
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
