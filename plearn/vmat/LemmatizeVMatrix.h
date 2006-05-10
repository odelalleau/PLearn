// -*- C++ -*-

// LemmatizeVMatrix.h
//
// Copyright (C) 2005 Hugo Larochelle
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file LemmatizeVMatrix.h */


#ifndef LemmatizeVMatrix_INC
#define LemmatizeVMatrix_INC

#include <plearn/vmat/SourceVMatrix.h>
#include <plearn_learners/language/Dictionary/Dictionary.h>
#include <plearn_learners/language/Dictionary/WordNetSenseDictionary.h>

namespace PLearn {

/**
   Takes a VMatrix with a word and a POS field
   and adds a field consisting of the lemma
   form of the word. This field becomes the
   last input field. First, customized
   mapping tables (with different priorities) are used,
   and finally, if the previous step was unsuccesful,
   WordNet is used to obtain a lemma.
 */
class LemmatizeVMatrix : public SourceVMatrix
{
    typedef SourceVMatrix inherited;

public:
    //! Index (position) of word field.
    int word_field;
    //! Index (position) of POS field.
    int pos_field;
    //! Customized table that uses the POS tag to obtain a lemma
    //! It must have exactly two columns [POS lemma]. It has the third priority.
    TMat<string> pos_to_lemma_table;
    //! Customized table that uses the word to obtain a lemma
    //! It must have exactly two columns [word lemma]. It has the second priority.
    TMat<string> word_to_lemma_table;
    //! Customized table that uses the word and POS tag to obtain a lemma
    //! It must have exactly three columns [word POS lemma]. It has the first priority.
    TMat<string> word_pos_to_lemma_table;

public:
    //! Default constructor
    LemmatizeVMatrix();

    PLEARN_DECLARE_OBJECT(LemmatizeVMatrix);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! returns value associated with a string (or MISSING_VALUE if there's no association for this string)
    virtual real getStringVal(int col, const string & str) const;

    virtual string getValString(int col, real val) const;

    virtual Vec getValues(int row, int col) const;

    //! Gives the possible values of a certain field (column) given the input
    virtual Vec getValues(const Vec& input, int col) const;

    //! Get Dictionary from a certain column
    virtual PP<Dictionary> getDictionary(int col) const;


protected:
    //! Lemma Dictionary, constructed automatically
    PP<Dictionary> lemma_dict;
    //! Temporary variable for source row
    Vec src_row;

protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Fill the vector 'v' with the content of the i-th row.
    //! v is assumed to be the right size.
    virtual void getNewRow(int i, const Vec& v) const;

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

    //! Gives the lemma for a certain row
    string getLemma(int row) const;

private:
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(LemmatizeVMatrix);

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
