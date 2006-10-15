// -*- C++ -*-

// WordNetFeatureSet.h
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

/*! \file WordNetFeatureSet.h */


#ifndef WordNetFeatureSet_INC
#define WordNetFeatureSet_INC

#include <plearn/feat/HashMapFeatureSet.h>

namespace PLearn {

/**
 * FeatureSet with features from WordNet.
 * The feature of a token (word) corresponds to the set of its possible
 * WordNet senses or synsets. The hierarchy of WordNet can also be
 * used to obtain the synset ancestors of these senses and use
 * them as additional features. The input token can also be stemmed before
 * doing the WordNet searches.
 */
class WordNetFeatureSet : public HashMapFeatureSet
{
    typedef HashMapFeatureSet inherited;

public:
    //#####  Public Build Options  ############################################

    //! Indication that the input token should be stemmed
    bool stem_token;
    //! Indication that features should include information from the
    //! WordNet hierarchy, not just the token's possible senses
    bool use_wordnet_hierarchy;
    
public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    WordNetFeatureSet();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(WordNetFeatureSet);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Gives the possibly new features in string form for a token
    virtual void getNewFeaturesString(string token, TVec<string>& feats_str);

protected:
    //#####  Protected Options  ###############################################

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

    //! Temporary computations vectors
    TVec<string> f_str, s_str, stems;
    TVec<TVec<string> > anc_str;
    TVec<int> tagcnts;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(WordNetFeatureSet);

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
