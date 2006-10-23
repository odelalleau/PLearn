// -*- C++ -*-

// ConditionalDictionary.h
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
 * $Id: ConditionalDictionary.h 4184 2005-10-04 14:02:12Z larocheh $ 
 ******************************************************* */

// Authors: Hugo Larochelle, Christopher Kermorvant

/*! \file ConditionalDictionary.h */


#ifndef ConditionalDictionary_INC
#define ConditionalDictionary_INC
#include "Dictionary.h"

namespace PLearn {
using namespace std;

/*! 
  This class implements a Dictionary where possible symbols
  depend on the option fields. It is defined simply by
  giving a mapping file, where each row has the form:
  
  OPTION_1 ... OPTION_M[\tab]TAG_1 TAG_2 TAG_3 ...

  where [\tab] is the tabulation character. For
  instance, a Part Of Speech Dictionary could be
  defined using lines like:

  pet[\tab]NN VB VBD

  When the option fields are not found in the mapping,
  then the possible values are simply the set of
  all possible symbols (TAG fields) of the Dictionary
*/

class ConditionalDictionary: public Dictionary
{

private:
  
    typedef Dictionary inherited;

    string tmp_str;
    TVec<int> tmp_sym;
    
protected:

    //! Option fields to possible symbols mapping
    map<string,Vec> options_to_symbols;

    //! possible values vector
    Vec possible_values;

public:

    // ************************
    // * public build options *
    // ************************

    //! Put options to lower case
    bool options_to_lower_case;

    //! Mapping file
    string mapping_file_path;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    ConditionalDictionary();

    // ******************
    // * Object methods *
    // ******************

private: 
    //! This does the actual building. 
    void build_();

    string get_option_string(TVec<string> options);

protected: 
    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

public:
    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(ConditionalDictionary);

    virtual void getValues(TVec<string> options, Vec& values);
    
    virtual int size(TVec<string> options=TVec<string>(0));

    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ConditionalDictionary);
  
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
