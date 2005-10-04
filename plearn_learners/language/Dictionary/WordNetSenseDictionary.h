// -*- C++ -*-

// WordNetDictionary.h
//
// Copyright (C) 2004 Hugo Larochelle Christopher Kermorvant
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

/* *******************************************************      
 * $Id$ 
 ******************************************************* */

// Authors: Hugo Larochelle, Christopher Kermorvant

/*! \file WordNetSenseDictionary.h */


#ifndef WordNetSenseDictionary_INC
#define WordNetSenseDictionary_INC
#include "Dictionary.h"

namespace PLearn {
using namespace std;

/*! This class implements a Dictionary for WordNet senses.
  The symbols in the instantiated dictionary are senses (not words!).
*/

class WordNetSenseDictionary: public Dictionary
{

private:
  
    typedef Dictionary inherited;

protected:

public:

    // ************************
    // * public build options *
    // ************************

    //! Stem word before including in dictionary STEM/NO_STEM (ontology only)
    bool options_stem_words;
    
    //! Put words to lower case
    bool options_to_lower_case;

    //! Type of representation (symbol) of the senses
    string symbol_type;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    WordNetSenseDictionary();

    // ******************
    // * Object methods *
    // ******************

private: 
    //! This does the actual building. 
    void build_();

protected: 
    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

    TVec<string> getSensesFromWordNet(TVec<string> options);

public:
    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(WordNetSenseDictionary);

    virtual int getId(string symbol, TVec<string> options = TVec<string>(0));

    virtual Vec getValues(TVec<string> options=TVec<string>(0));
    
    virtual int size(TVec<string> options=TVec<string>(0));

    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(WordNetSenseDictionary);
  
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
