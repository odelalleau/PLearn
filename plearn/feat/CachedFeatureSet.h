// -*- C++ -*-

// CachedFeatureSet.h
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

/*! \file CachedFeatureSet.h */


#ifndef CachedFeatureSet_INC
#define CachedFeatureSet_INC

#include <plearn/feat/FeatureSet.h>

namespace PLearn {

/**
 * Feature set that maintains a cached mapping between tokens and their features.
 * This cache permits potentially faster access to a token's
 * features using the getFeatures() function, when a call to
 * to that function has been done previously for that given token.
 * A source FeatureSet that provides the features most be given.
 * It is assumed that adding features will not change the index
 * assignements of the previously added features.
 */

class CachedFeatureSet : public FeatureSet
{
    typedef FeatureSet inherited;

public:
    //#####  Public Build Options  ############################################

    //! Source feature set, for which a buffer is maintained
    PP<FeatureSet> source;
    //! Indication that progress should be reported in a ProgressBar
    //! when adding features in the cache in addFeatures(VMat,int).
    bool report_progress;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    CachedFeatureSet();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(CachedFeatureSet);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Gives features of token in index form
    virtual void getFeatures(string token, TVec<int>& feats);
    
    //! Gives string form of a feature in index form
    virtual string getStringFeature(int index);
    
    //! Gives index form of a feature in string form
    virtual int getIndexFeature(string str);
    
    //! Gives the number of features in the set
    virtual int size();

    //! Adds the features for a given token in the set
    virtual void addFeatures(string token);
    
    //! Adds the features for the tokens contained in the set.
    //! Features with frequency smaller than min_freq are not added.
    virtual void addFeatures(VMat tokens, int min_freq=-1);

    //! Clears all features from the feature set
    virtual void clear();

    //! Gives the possibly new features in string form for a token
    virtual void getNewFeaturesString(string token, TVec<string>& feats_str);

protected:
    //#####  Protected Options  ###############################################

    //! Token to features mapping
    hash_map<string, TVec<int> > token_to_features;
    
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
    TVec<int> f, f_temp;
    int index;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(CachedFeatureSet);

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
