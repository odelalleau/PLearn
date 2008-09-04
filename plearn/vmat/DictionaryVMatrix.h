// -*- C++ -*-

// DictionaryVMatrix.h
//
// Copyright (C) 2004 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file DictionaryVMatrix.h */


#ifndef DictionaryVMatrix_INC
#define DictionaryVMatrix_INC

#include <plearn/python/PythonCodeSnippet.h>

#include <plearn/base/stringutils.h>
#include <plearn/vmat/RowBufferedVMatrix.h>
#include <plearn/dict/Dictionary.h>

// number of attributes of dictionary specifications
#define DIC_SPECIFICATION_SIZE 3

namespace PLearn {
using namespace std;

//! VMat of text files, encoded  with Dictionaries,
//! The lines of the text files that are empty are ommited. If no Dictionary
//! objects are given by the user, then new Dictionary objects
//! are created and updated from the text files.
//! A Python script can be provided to preprocess each
//! row of all files. The script must define a function called
//! process_string_row(string_row), where string_row is a list of
//! strings corresponding to the symbolic fields of a row in
//! the input files. This function must return a list of processed
//! strings, which will consist of the actual data contained by
//! the VMatrix. Note that process_string_row can return a list
//! that has more or less strings then the input files has fields.
//! The length of the returned list will determine the width of
//! the VMatrix. Here is an example of a Python code that
//! puts the first field to lower case and does nothing to the second:
//!
//! "def process_string_row(string_row):
//!          ret = string_row[:]
//!          ret[0] = string_row[0].lower()
//!          ret[1] = string_row[1]
//!          return ret "

class DictionaryVMatrix: public RowBufferedVMatrix
{

private:


    typedef RowBufferedVMatrix inherited;

    Mat data;

protected:

    // *********************
    // * protected options *
    // *********************

    //! Number of attributes in the input text file (\\t separated)
    int n_attributes;

    //! Python code snippet
    PP<PythonCodeSnippet> python;

public:

    // ************************
    // * public build options *
    // ************************

    //! The text input files which are processed with dictionaries
    TVec<PPath> file_names;

    //! The dictionaries, one for each attributes
    TVec< PP<Dictionary> > dictionaries;

    //! The options fields of every dictionaries
    TVec< TVec <int> > option_fields;

    //! String delimiters for input file fields
    string delimiters;

    //! Snippet of python code that processes the text in the input files
    string code;

    //! Minimum frequency for a token to be added in a Dictionary, for the
    //! different fields
    TVec<int> minimum_frequencies;

    //! Stop word list, mapped to OOV_TAG
    TVec< TVec<string> >symbols_to_ignore;

    //! Indication that a row that has a OOV_TAG field
    //! should be ignored, i.e. removed of the VMatrix
    bool remove_rows_with_oov;

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    DictionaryVMatrix();

private:

    //! This does the actual building.
    // (Please implement in .cc)
    void build_();

protected:

    //! Declares this class' options
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);

    //! Fill the vector 'v' with the content of the i-th row.
    //! v is assumed to be the right size.
    virtual void getNewRow(int i, const Vec& v) const;

public:

    //! returns value associated with a string (or MISSING_VALUE if there's no association for this string)
    virtual real getStringVal(int col, const string & str) const;

    virtual string getValString(int col, real val) const;

    virtual void getValues(int row, int col, Vec& values) const;

    //! Gives the possible values of a certain field (column) given the input
    virtual void getValues(const Vec& input, int col, Vec& values) const;

    // Simply call inherited::build() then build_().
    virtual void build();

    //! Transform a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Get Dictionary from a certain column
    virtual PP<Dictionary> getDictionary(int col) const;

    //  virtual void save(const string& filename) ;
    //! Declare name and deepCopy methods.
    PLEARN_DECLARE_OBJECT(DictionaryVMatrix);

};

DECLARE_OBJECT_PTR(DictionaryVMatrix);

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
