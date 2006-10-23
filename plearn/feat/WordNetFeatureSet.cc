// -*- C++ -*-

// WordNetFeatureSet.cc
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

/*! \file WordNetFeatureSet.cc */


#include "WordNetFeatureSet.h"
#include <plearn/dict/WordNetSenseDictionary.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    WordNetFeatureSet,
    "FeatureSet with features from WordNet.",
    "The feature of a token (word) corresponds to the set of its possible\n"
    "WordNet senses or synsets. The hierarchy of WordNet can also be\n"
    "used to obtain the synset ancestors of these senses and use\n"
    "them as additional features. The input token can also be stemmed before\n"
    "doing the WordNet searches.\n"
    );

WordNetFeatureSet::WordNetFeatureSet()
    : stem_token(true), use_wordnet_hierarchy(true)
{
    if(wninit()<0)
        PLERROR("In WordNetFeatureSet(): could not open WordNet database files");
}

// ### Nothing to add here, simply calls build_
void WordNetFeatureSet::build()
{
    inherited::build();
    build_();
}

void WordNetFeatureSet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    //PLERROR("WordNetFeatureSet::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void WordNetFeatureSet::declareOptions(OptionList& ol)
{
    declareOption(ol, "stem_token", &WordNetFeatureSet::stem_token, 
                  OptionBase::buildoption, 
                  "Indication that the input token should be stemmed");
    declareOption(ol, "use_wordnet_hierarchy", 
                  &WordNetFeatureSet::use_wordnet_hierarchy, 
                  OptionBase::buildoption, 
                  "Indication that features should include information from the\n"
                  "WordNet hierarchy, not just the token's possible senses.\n");
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void WordNetFeatureSet::build_()
{}

void WordNetFeatureSet::getNewFeaturesString(string token, TVec<string>& feats_str)
{
    feats_str.resize(0);
    s_str.resize(0);
    anc_str.resize(0);
    // Extract senses for all possible POS

    // NOUN
    extractSenses(token,NOUN,"synset_key",s_str, tagcnts, anc_str, 
                  use_wordnet_hierarchy);

    if(stem_token) 
    {
        stemsOfWord(token,NOUN,stems);
        for(int i=0; i<stems.length(); i++)
        {
            if(token != stems[i])
                extractSenses(stems[i],NOUN,"synset_key",s_str, tagcnts, 
                              anc_str, use_wordnet_hierarchy);
        }
    }

    // VERB
    extractSenses(token,VERB,"synset_key",s_str, tagcnts, anc_str, 
                  use_wordnet_hierarchy);

    if(stem_token) 
    {
        stemsOfWord(token,VERB,stems);
        for(int i=0; i<stems.length(); i++)
            if(token != stems[i])
                extractSenses(stems[i],VERB,"synset_key",s_str, tagcnts, 
                              anc_str, use_wordnet_hierarchy);
    }
       
    // ADJ
    extractSenses(token,ADJ,"synset_key",s_str, tagcnts, anc_str, 
                  use_wordnet_hierarchy);
    
    if(stem_token) 
    {
        stemsOfWord(token,ADJ,stems);
        for(int i=0; i<stems.length(); i++)
            if(token != stems[i])
                extractSenses(stems[i],ADJ,"synset_key",s_str, tagcnts, 
                              anc_str, use_wordnet_hierarchy);
    }
       
    // ADV
    extractSenses(token,ADV,"synset_key",s_str, tagcnts, anc_str, 
                  use_wordnet_hierarchy);

    if(stem_token) 
    {
        stemsOfWord(token,ADV,stems);
        for(int i=0; i<stems.length(); i++)
            if(token != stems[i])
                extractSenses(stems[i],ADV,"synset_key",s_str, tagcnts, 
                              anc_str, use_wordnet_hierarchy);
    }

    if(use_wordnet_hierarchy)
        for(int t=0; t<anc_str.length(); t++)
            for(int s=0; s<anc_str[t].length(); s++)
                feats_str.appendIfNotThereAlready(anc_str[t][s]);
    else
        for(int t=0; t<s_str.length(); t++)
            feats_str.appendIfNotThereAlready(s_str[t]);
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
