// -*- C++ -*-

// HashMapFeatureSet.cc
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

/*! \file HashMapFeatureSet.cc */


#include "HashMapFeatureSet.h"
#include <plearn/vmat/VMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    HashMapFeatureSet,
    "Base class for feature sets that maintains an explicit mapping between index and string form features",
    "This class facilitates the conception of FeatureSet objects.\n"
    "Classes that inherits from this class only need to define the\n"
    "getNewFeaturesString() function, which is called when adding new features\n"
    "or when asking for the features of a token (in index or string format).\n"
    "This function should return the list of features in string format\n"
    "for a given token. It is assumed that no feature is present more than once\n"
    "in the output of that function, for any input token.\n"
    "This base class maintains two hash_maps between the index and string forms\n"
    "of the features. These mappings contain all the necessary\n"
    "information to define the different functions of a FeatureSet object.\n"
    );

HashMapFeatureSet::HashMapFeatureSet()
    : report_progress(false)
{}

// ### Nothing to add here, simply calls build_
void HashMapFeatureSet::build()
{
    inherited::build();
    build_();
}

void HashMapFeatureSet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(index_to_string_feats, copies);
    deepCopyField(string_to_index_feats, copies);

    //PLERROR("HashMapFeatureSet::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void HashMapFeatureSet::declareOptions(OptionList& ol)
{
    declareOption(ol, "report_progress", &HashMapFeatureSet::report_progress,
                  OptionBase::buildoption,
                  "Indication that progress should be reported in a ProgressBar\n"
                  "when filling the feature set.\n");    
    declareOption(ol, "index_to_string_feats", &HashMapFeatureSet::index_to_string_feats,
                  OptionBase::learntoption,
                  "Index feature to string feature mapping");    
    declareOption(ol, "string_to_index_feats", &HashMapFeatureSet::string_to_index_feats,
                  OptionBase::learntoption,
                  "String feature to index feature mapping");    

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void HashMapFeatureSet::build_()
{}

void HashMapFeatureSet::getFeatures(string token, TVec<int>& feats)
{
    int index;
    string str;
    getNewFeaturesString(token,f_str);
    feats.resize(0);
    
    for(int t=0; t<f_str.length(); t++)
    {
        str = f_str[t];
        if(string_to_index_feats.find(str) != string_to_index_feats.end())
        {
            index = string_to_index_feats[str];
            feats.push_back(index);
        }
    }
}

string HashMapFeatureSet::getStringFeature(int index)
{
#ifdef BOUNDCHECK
    if(index_to_string_feats.find(index) == index_to_string_feats.end())
        PLERROR("In HashMapFeatureSet::getStringFeature(): index %d is an invalid feature index", index);
#endif
    return index_to_string_feats[index];
    
}

int HashMapFeatureSet::getIndexFeature(string str)
{
    if(string_to_index_feats.find(str) == string_to_index_feats.end())
        PLERROR("In HashMapFeatureSet::getIndexFeature(): string %s is an invalid feature", str.c_str());

    return string_to_index_feats[str];
}

int HashMapFeatureSet::size()
{
    return int(index_to_string_feats.size());
}

void HashMapFeatureSet::addFeatures(string token)
{
    string str;
    int index;
    
    getNewFeaturesString(token,f_str);
    for(int t=0; t<f_str.length(); t++)
    {
        str = f_str[t];
        if(string_to_index_feats.find(str) == string_to_index_feats.end()){
            index = int(string_to_index_feats.size());
            string_to_index_feats[str] = index;
            index_to_string_feats[index] = str;
        }
    }

}

void HashMapFeatureSet::addFeatures(VMat tokens, int min_freq)
{
    string str, token;
    int index;
    map<string,int> frequencies;
    map<string,TVec<string> > token_features;
    map<string,bool> token_feat_inserted;
    Vec row(tokens->width());
    TVec<string> f_str_temp;

    ProgressBar* pb=0;
    int id=0;

    if(min_freq > 1)
    {
        if(report_progress)
            pb = new ProgressBar("Counting frequencies for the features of " + tostring(tokens->length()*tokens->width()) + " tokens", tokens->length()*tokens->width());
        
        for(int i=0; i<tokens->length(); i++)
        {
            tokens->getRow(i,row);
            for(int j=0; j<tokens->width(); j++)
            {
                token = tokens->getValString(j,row[j]);
                if(token_features.find(token) == token_features.end())
                {
                    getNewFeaturesString(token,f_str);
                    token_features[token] = f_str.copy();
                }
                else
                {
                    f_str_temp = token_features[token];
                    f_str.resize(f_str_temp.length());
                    f_str << f_str_temp;
                }

                for(int t=0; t<f_str.length(); t++)
                {
                    str = f_str[t];
                    if(frequencies.find(str) == frequencies.end())
                        frequencies[str] = 1;
                    else
                        frequencies[str]++;
                }
                
                if(report_progress) pb->update(++id);
            }        
        }
        token_features.clear();
    }

    id=0;
    if(report_progress)
        pb = new ProgressBar("Filling FeatureSet with the features of " + tostring(tokens->length()*tokens->width()) + " tokens", tokens->length()*tokens->width());


    for(int i=0; i<tokens->length(); i++)
    {
        tokens->getRow(i,row);
        for(int j=0; j<tokens->width(); j++)
        {
            token = tokens->getValString(j,row[j]);
            if(token_feat_inserted.find(token) == token_feat_inserted.end())
            {
                getNewFeaturesString(token,f_str);
                for(int t=0; t<f_str.length(); t++)
                {
                    str = f_str[t];
                    if(string_to_index_feats.find(str) == string_to_index_feats.end()
                       && (min_freq <=1 || frequencies[str] >= min_freq)){
                        index = int(string_to_index_feats.size());
                        string_to_index_feats[str] = index;
                        index_to_string_feats[index] = str;
                    }
                }
                token_feat_inserted[token] = true;
            }
            if(report_progress) pb->update(++id);
        }
    }
    if(report_progress) delete(pb);
}

void HashMapFeatureSet::clear()
{
    index_to_string_feats.clear();
    string_to_index_feats.clear();
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
