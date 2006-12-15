// -*- C++ -*-

// CachedFeatureSet.cc
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

/*! \file CachedFeatureSet.cc */


#include "CachedFeatureSet.h"
#include <plearn/vmat/VMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    CachedFeatureSet,
    "Feature set that maintains a cached mapping between tokens and their features",
    "This cache permits potentially faster access to a token's\n"
    "features using the getFeatures() function, when a call to\n"
    "to that function has been done previously for that given token.\n"
    "A source FeatureSet that provides the features most be given.\n"
    "It is assumed that adding features will not change the index\n"
    "assignements of the previously added features.\n"
    );

CachedFeatureSet::CachedFeatureSet()
    : report_progress(false)
{}

// ### Nothing to add here, simply calls build_
void CachedFeatureSet::build()
{
    inherited::build();
    build_();
}

void CachedFeatureSet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(source, copies);
    deepCopyField(token_to_features, copies);
    deepCopyField(f, copies);
}

void CachedFeatureSet::declareOptions(OptionList& ol)
{
    declareOption(ol, "source", &CachedFeatureSet::source,
                  OptionBase::buildoption,
                  "Source feature set, for which a buffer is maintained");    
    declareOption(ol, "token_to_features", &CachedFeatureSet::token_to_features,
                  OptionBase::learntoption,
                  "Token to features mapping");    
    declareOption(ol, "report_progress", &CachedFeatureSet::report_progress,
                  OptionBase::buildoption,
                  "Indication that progress should be reported in a ProgressBar\n"
                  "when adding features in the cache in addFeatures(VMat,int).\n");    

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void CachedFeatureSet::build_()
{}

void CachedFeatureSet::getFeatures(string token, TVec<int>& feats)
{
    if(token_to_features.find(token) != token_to_features.end())
    {
        f = token_to_features[token];
        feats.resize(f.length());
        feats << f;
        return;
    }
    source->getFeatures(token,feats);
    token_to_features[token] = feats.copy();
}

string CachedFeatureSet::getStringFeature(int index)
{
    return source->getStringFeature(index);
}

int CachedFeatureSet::getIndexFeature(string str)
{
    return source->getIndexFeature(str);
}

int CachedFeatureSet::size()
{
    return source->size();
}

void CachedFeatureSet::addFeatures(string token)
{ 
    source->addFeatures(token);
    // Insert these features in the cache
    getFeatures(token,f_temp);
    return;
}

void CachedFeatureSet::addFeatures(VMat tokens, int min_freq)
{
    source->addFeatures(tokens, min_freq);

    // Inserting these features in the cache
    string token;
    Vec row(tokens->width());

    PP<ProgressBar> pb;
    int id=0;

    if(report_progress)
        pb = new ProgressBar("Adding in cache the features of " + tostring(tokens->length()*tokens->width()) + " tokens", tokens->length()*tokens->width());

    for(int i=0; i<tokens->length(); i++)
    {
        tokens->getRow(i,row);
        for(int j=0; j<tokens->width(); j++)
        {
            token = tokens->getValString(j,row[j]);
            getFeatures(token,f_temp);
            if(report_progress) pb->update(++id);
        }
    }
    return;
}

void CachedFeatureSet::clear()
{
    source->clear();
    token_to_features.clear();
}

void CachedFeatureSet::getNewFeaturesString(string token, TVec<string>& feats_str)
{
    source->getNewFeaturesString(token,feats_str);
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
