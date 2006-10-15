// -*- C++ -*-

// ConcatDisjointFeatureSet.cc
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

// Authors: Hugo Larochelle

/*! \file ConcatDisjointFeatureSet.cc */


#include "ConcatDisjointFeatureSet.h"
#include <plearn/base/lexical_cast.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ConcatDisjointFeatureSet,
    "Feature set that is the concatenation of disjoint feature sets",
    "This class concatenates different feature sets but forces them\n"
    "to be disjoint. This means that, even though two feature sets\n"
    "to concatenate contain the same feature, a distinction will be\n"
    "made between the feature coming from the first and second set.\n"
    );

ConcatDisjointFeatureSet::ConcatDisjointFeatureSet()
{}

// ### Nothing to add here, simply calls build_
void ConcatDisjointFeatureSet::build()
{
    inherited::build();
    build_();
}

void ConcatDisjointFeatureSet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(feature_sets, copies);
    //PLERROR("ConcatDisjointFeatureSet::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void ConcatDisjointFeatureSet::declareOptions(OptionList& ol)
{
    // Now call the parent class' declareOptions
    declareOption(ol, "feature_sets", &ConcatDisjointFeatureSet::feature_sets,
                  OptionBase::buildoption,
                  "Feature sets to concatenate");

    inherited::declareOptions(ol);
}

void ConcatDisjointFeatureSet::build_()
{
    // Making sure subfeats has a non-null storage
    subfeats.resize(1);
    subfeats.resize(0);
}

void ConcatDisjointFeatureSet::getFeatures(string token, TVec<int>& feats)
{
    feats.resize(0);
    int s = 0;
    for(int i=0; i<feature_sets.length(); i++)
    {
        feature_sets[i]->getFeatures(token,subfeats);
        subfeats += s;
        feats.append(subfeats);
        s += feature_sets[i]->size();
    }
}

void ConcatDisjointFeatureSet::getNewFeaturesString(string token, TVec<string>& feats_str)
{
    feats_str.resize(0);
    for(int i=0; i<feature_sets.length(); i++)
    {
        feature_sets[i]->getNewFeaturesString(token, f_str);
        for(int j=0; j<f_str.length(); j++)
            f_str[j] = "f" + tostring(i) + ":" + f_str[j];
        feats_str.append(f_str);
    }
}

string ConcatDisjointFeatureSet::getStringFeature(int index)
{
#ifdef BOUNDCHECK
    if(index<0 || index >= size()) PLERROR("In ConcatDisjointFeatureSet::getStringFeature(): index %d is an invalid feature index", index);
#endif
    int sub_index = index;
    for(int i=0; i<feature_sets.length(); i++)
    {
        if(sub_index<feature_sets[i]->size()) return "f" + tostring(i) + ":" + feature_sets[i]->getStringFeature(sub_index);
        else sub_index -= feature_sets[i]->size();
    }
    PLERROR("In ConcatDisjointFeatureSet::getStringFeature(): index %d is an invalid feature index", index);
    return "";
}

int ConcatDisjointFeatureSet::getIndexFeature(string str)
{
    int fi = toint(tostring(str[1]));
    int s = 0;
#ifdef BOUNDCHECK
    if(fi >= feature_sets.length()) PLERROR("In ConcatDisjointFeatureSet::getIndexFeature(): string prefix %s of feature is invalid", str.substr(0,3).c_str());
#endif
    for(int i=0; i<fi; i++)
        s += feature_sets[i]->size();
    return feature_sets[fi]->getIndexFeature(str.substr(3,str.length()-3))+s;
}

int ConcatDisjointFeatureSet::size()
{
    int ret = 0;
    for(int i=0; i<feature_sets.length(); i++)
        ret += feature_sets[i]->size();
    return ret;
}

void ConcatDisjointFeatureSet::addFeatures(string token)
{
    for(int i=0; i<feature_sets.length(); i++)
        feature_sets[i]->addFeatures(token);
}

void ConcatDisjointFeatureSet::addFeatures(VMat tokens, int min_freq)
{
    for(int i=0; i<feature_sets.length(); i++)
        feature_sets[i]->addFeatures(tokens, min_freq);
}

void ConcatDisjointFeatureSet::clear()
{
    for(int i=0; i<feature_sets.length(); i++)
        feature_sets[i]->clear();
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
