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
    "This learner performs normalization of its input (subtracting the mean and dividing by stddev)",
    "NormalizationLearner produces as output a normalized version of its input\n"
    "obtained by subtracting the mean and dividing by the standard deviation.\n"
    "It also has a simple policy switch for the handling of missing values.\n"
    "It is typically used as an early preprocessing step in a ChainedLearner \n"
    "NOTE: you may also consider using PCA(normalize=1), if you wan to obtain \n"
    "decorrelated 'sphered' data." );

NormalizationLearner::NormalizationLearner()
    :min_allowed_stddev(1e-6),
     set_missing_to_zero(true)
{
}

void NormalizationLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "min_allowed_stddev", &NormalizationLearner::min_allowed_stddev,
                  OptionBase::buildoption,
                  "If the empirical standard deviation is lower than this, we'll use this value to \n"
                  "compute inv_stddev (this is to prevent getting too large or even infinite values for inv_stddev");

    declareOption(ol, "set_missing_to_zero", &NormalizationLearner::set_missing_to_zero,
                  OptionBase::buildoption,
                  "How to handle missing values: \n"
                  "  If true (the default), missing values will be replaced by \n"
                  "  the post-normalization mean, which is 0. \n"
                  "  If false missing values will be left as missing values. \n");

    declareOption(ol, "meanvec", &NormalizationLearner::meanvec,
                  OptionBase::learntoption,
                  "The empirical mean to subtract from the input\n");

    declareOption(ol, "inv_stddev", &NormalizationLearner::inv_stddev,
                  OptionBase::learntoption,
                  "The vector of factors by which to multiply (input-meanvec)\n");

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
    return meanvec.length();
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
        ProgressBar* pb = 0;
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
        if(pb)
            delete pb;

        st.getMean(meanvec);
        Vec stddev = st.getStdDev();
        inv_stddev.resize(n);
        for(int k=0; k<n; k++)
            inv_stddev[k] = 1.0/max(min_allowed_stddev, stddev[k]);
        ++stage;
        train_stats->finalize(); 
    }
}


void NormalizationLearner::computeOutput(const Vec& input, Vec& output) const
{
    int n = meanvec.length();
    if(input.length()!=n)
        PLERROR("length of input differs from length of meanvec!");
    output.resize(n);
    real* p_input = input.data();
    real* p_output = output.data();
    real* p_meanvec = meanvec.data();
    real* p_inv_stddev = inv_stddev.data();

    for(int k=0; k<n; k++)
    {
        real val = p_input[k];
        if(is_missing(val))
            p_output[k] = set_missing_to_zero?0.:val;
        else
            p_output[k] = p_inv_stddev[k]*(val - p_meanvec[k]);
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
    return inputnames;
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
