// -*- C++ -*-

// DictionaryVMatrix.cc
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

/*! \file DictionaryVMatrix.cc */


#include "DictionaryVMatrix.h"
//#include "DiskVMatrix.h"
#include "plearn/io/openFile.h"
#include "plearn/io/fileutils.h"

namespace PLearn {
using namespace std;


DictionaryVMatrix::DictionaryVMatrix()
    :python(), delimiters(" \t"), code(""), remove_rows_with_oov(false)
    /* ### Initialise all fields to their default value */
{
    data=0;
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(DictionaryVMatrix,
                        "VMat of text files, encoded  with Dictionaries",
                        "The lines of the text files that are empty are ommited. If no Dictionary\n"
                        "objects are given by the user, then new Dictionary objects\n"
                        "are created and updated from the text files.\n"
                        "A Python script can be provided to preprocess each\n"
                        "row of all files. The script must define a function called\n"
                        "process_string_row(string_row), where string_row is a list of\n"
                        "strings corresponding to the symbolic fields of a row in\n"
                        "the input files. This function must return a list of processed\n"
                        "strings, which will consist of the actual data contained by\n"
                        "the VMatrix. Note that process_string_row can return a list\n"
                        "that has more or less strings then the input files has fields.\n"
                        "The length of the returned list will determine the width of\n"
                        "the VMatrix. Here is an example of a Python code that\n"
                        "puts the first field to lower case and does nothing to the second:\n"
                        "\n"
                        "\"def process_string_row(string_row):\\n"
                        "         ret = string_row[:]\\n"
                        "         ret[0] = string_row[0].lower()\\n"
                        "         ret[1] = string_row[1]\\n "
                        "         return ret \""
    );

void DictionaryVMatrix::getNewRow(int i, const Vec& v) const
{
    v << data(i);
}
//! returns value associated with a string (or MISSING_VALUE if there's no association for this string)
real DictionaryVMatrix::getStringVal(int col, const string & str) const
{
    int ret = dictionaries[col]->getId(str);
    if(ret == -1) return MISSING_VALUE;
    else return ret;
}

string DictionaryVMatrix::getValString(int col, real val) const
{
    if(is_missing(val))return tostring(val);
    return dictionaries[col]->getSymbol((int)val);
}

PP<Dictionary>  DictionaryVMatrix::getDictionary(int col) const
{
    if(col < 0 || col >= width_) PLERROR("In DictionaryVMatrix::getDictionary() : invalid col %d, width()=%d", col, width_);
    return  dictionaries[col];
}


void DictionaryVMatrix::getValues(int row, int col, Vec& values) const
{
    if(row < 0 || row >= length_) PLERROR("In DictionaryVMatrix::getValues() : invalid row %d, length()=%d", row, length_);
    if(col < 0 || col >= width_) PLERROR("In DictionaryVMatrix::getValues() : invalid col %d, width()=%d", col, width_);
    TVec<string> options(option_fields[col].length());
    for(int i=0; i<options.length(); i++)
    {
        options[i] = dictionaries[option_fields[col][i]]->getSymbol((int)data(row,option_fields[col][i]));
    }
    dictionaries[col]->getValues(options, values);
}

void DictionaryVMatrix::getValues(const Vec& input, int col, Vec& values) const
{
    if(col < 0 || col >= width_) PLERROR("In DictionaryVMatrix::getValues() : invalid col %d, width()=%d", col, width_);
    TVec<string> options(option_fields[col].length());
    for(int i=0; i<options.length(); i++)
    {
        options[i] = dictionaries[option_fields[col][i]]->getSymbol((int)input[option_fields[col][i]]);
    }
    dictionaries[col]->getValues(options, values);
}


void DictionaryVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "file_names", &DictionaryVMatrix::file_names, OptionBase::buildoption,
		  "The text files from which we create the VMat");
    declareOption(ol, "dictionaries", &DictionaryVMatrix::dictionaries, OptionBase::buildoption,
                  "Vector of dictionaries\n");
    declareOption(ol, "option_fields", &DictionaryVMatrix::option_fields, OptionBase::buildoption,
                  "Vector of the fields corresponding to the options of the Dictionary, for every Dictionary\n");
    declareOption(ol, "data", &DictionaryVMatrix::data, OptionBase::buildoption,
                  "Encoded Matrix\n");
    declareOption(ol, "delimiters", &DictionaryVMatrix::delimiters, OptionBase::buildoption,
                  "Delimiters for file fields (or attributs)\n");
    declareOption(ol, "code", &DictionaryVMatrix::code, OptionBase::buildoption,
                  "Snippet of python code that processes the text in the input files\n");
    declareOption(ol, "minimum_frequencies", &DictionaryVMatrix::minimum_frequencies, OptionBase::buildoption,
                  "Minimum frequency for a token to be added in a Dictionary, for the different fields\n");
    declareOption(ol, "symbols_to_ignore", &DictionaryVMatrix::symbols_to_ignore, OptionBase::buildoption,
                  "Symbols that are ignored, i.e. considered as \"<oov>\" a priori when reading\n"
                  "the text files. In NLP, this is usually called a stop word list.\n");
    declareOption(ol, "remove_rows_with_oov", &DictionaryVMatrix::remove_rows_with_oov, OptionBase::buildoption,
                  "Indication that a row that has a OOV symbol field should be ignored, i.e. removed\n"
                  "of the VMatrix.\n");
    declareOption(ol, "data", &DictionaryVMatrix::data, OptionBase::learntoption,
                  "Matrix containing the concatenated and encoded text files\n");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DictionaryVMatrix::build_()
{
    //if(data.length() != 0)
    //{
    //    width_ = data.width();
    //    length_ = data.length();
    //    if(option_fields.length()==0) option_fields.resize(width_);
    //    return;
    //}
    
    string line = "";
    vector<string> tokens;
    TVec<string> tokens_vec;
    TVec< hash_map<string,int> > frequencies;
    int it=0;
    int nlines = 0;
    bool row_has_oov = false;

    if (!python && code != "")
    {
        python = new PythonCodeSnippet(code);
        PLASSERT( python );
        python->build();
    }

    if(data.length()!=0) data.clear();

    length_ = 0;

    // Prepocessing of the data...
    for(int k=0; k<file_names.length(); k++)
    {
        PPath input_file = file_names[k];
        PStream input_stream = openFile(input_file, PStream::raw_ascii);
        updateMtime(input_file);
        input_stream.skipBlanks();
        while (!input_stream.eof()){
            input_stream.getline(line);
            input_stream.skipBlanks();
            tokens = split(line, delimiters);
            if(python)
            {
                tokens_vec.resize(int(tokens.size()));
                for(int i=0; i<tokens_vec.length(); i++)
                    tokens_vec[i] = tokens[i];
                tokens_vec = python->invoke("process_string_row",tokens_vec).as<TVec<string> >();
                tokens.resize(tokens_vec.length());
                for(int i=0; i<tokens_vec.length(); i++)
                    tokens[i] = tokens_vec[i];
            }

            // Set n_attributes
            if(it==0)
            {
                it++;
                n_attributes = int(tokens.size());
                if(minimum_frequencies.length() != 0 && minimum_frequencies.length() != n_attributes)
                    PLERROR("In DictionaryVMatrix::build_(): number of attributes (%d) and size of minimum_frequencies (%d) is different", n_attributes, minimum_frequencies.length());

                if(minimum_frequencies.length() == 0)
                {
                    minimum_frequencies.resize(n_attributes);
                    minimum_frequencies.fill(-1);
                }

                // If no dictionaries are specified, then create some
                if(dictionaries.length() == 0)
                {
                    dictionaries.resize(n_attributes);
                    for(int i=0; i<n_attributes; i++)
                    {
                        dictionaries[i] = new Dictionary();
                        dictionaries[i]->update_mode = UPDATE;
                        dictionaries[i]->build();
                    }

                }
                if(dictionaries.length() != n_attributes)
                    PLERROR("In DictionaryVMatrix::build_(): number of attributes (%d) and number of dictionaries (%d) is different", n_attributes, dictionaries.length());
                if(option_fields.length()==0) option_fields.resize(n_attributes);
                frequencies.resize(n_attributes);
            }

            if(symbols_to_ignore.length() != 0)
                for(int i=0; i<int(tokens.size()); i++)
                    if(symbols_to_ignore[i].find(tokens[i]) >= 0)
                        tokens[i] = dictionaries[i]->oov_symbol;

            row_has_oov = false;
            for(int i=0; i<int(tokens.size()); i++)
                if(tokens[i] == dictionaries[i]->oov_symbol)
                    row_has_oov = true;

            // Count frequencies...
            for(int j=0; j<n_attributes; j++)
            {
                if(tokens[j] != "nan") // Detect missing values
                {
                    if(frequencies[j].find(tokens[j]) == frequencies[j].end())
                        frequencies[j][tokens[j]] = 1;
                    else
                        frequencies[j][tokens[j]]++;
                }
            }

            if(!remove_rows_with_oov || !row_has_oov) length_++;
        }
    }

    it = 0;
    for(int k=0; k<file_names.length(); k++)
    {
        nlines = it;
        PPath input_file = file_names[k];
        PStream input_stream = openFile(input_file, PStream::raw_ascii);
        input_stream.skipBlanks();
        while (!input_stream.eof()){
            if(it==0) data.resize(length_,n_attributes);
            input_stream.getline(line);
            input_stream.skipBlanks();
            tokens = split(line, delimiters);

            if(python)
            {
                tokens_vec.resize(int(tokens.size()));
                for(int i=0; i<tokens_vec.length(); i++)
                    tokens_vec[i] = tokens[i];
                tokens_vec = python->invoke("process_string_row",tokens_vec).as<TVec<string> >();
                tokens.resize(tokens_vec.length());
                for(int i=0; i<tokens_vec.length(); i++)
                    tokens[i] = tokens_vec[i];
            }

            if(symbols_to_ignore.length() != 0)
                for(int i=0; i<int(tokens.size()); i++)
                    if(symbols_to_ignore[i].find(tokens[i]) >= 0)
                        tokens[i] = dictionaries[i]->oov_symbol;

            /*
            for(int i=0; i<to_lower_case.size(); i++)
                tokens[to_lower_case[i]] = lowerstring(tokens[to_lower_case[i]]);
            */

            if((int)tokens.size() != n_attributes)
                PLERROR("In DictionaryVMatrix::build_(): line %d (\"%s\") of file %s doesn't have %d attributes", it-nlines+1, line.c_str(), input_file.c_str(), n_attributes);

            // Insert symbols in dictionaries (if they can be updated)
            for(int j=0; j<n_attributes; j++)
            {
                if(tokens[j] == "nan") // Detect missing values
                    data(it,j) = MISSING_VALUE;
                else
                {
                    if(frequencies[j][tokens[j]] >= minimum_frequencies[j])
                    {
                        TVec<string> options(option_fields[j].length());
                        for(int k_it=0; k_it<options.length(); k_it++)
                            options[k_it] = tokens[option_fields[j][k_it]];
                        data(it,j) = dictionaries[j]->getId(tokens[j],options);
                    }
                    else
                        data(it,j) = dictionaries[j]->getId(dictionaries[j]->oov_symbol);
                }
            }
            row_has_oov = false;
            for(int j=0; j<n_attributes; j++)
                if(data(it,j) == dictionaries[j]->getId(dictionaries[j]->oov_symbol))
                    row_has_oov = true;

            if(!remove_rows_with_oov || !row_has_oov) it++;
        }
    }
    width_ = n_attributes;
    data.resize(it,n_attributes);
    length_ = it;

    // Making sure that the dictionaries cannot be
    // updated anymore, since they should contain
    // all the needed information about the data
    for(int i=0; i<dictionaries.length(); i++)
        dictionaries[i]->setUpdateMode(NO_UPDATE);
}

// ### Nothing to add here, simply calls build_
void DictionaryVMatrix::build()
{
    inherited::build();
    build_();
}

void DictionaryVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(data, copies);
    deepCopyField(file_names, copies);
    deepCopyField(dictionaries, copies);
    deepCopyField(option_fields, copies);
    deepCopyField(minimum_frequencies, copies);
    deepCopyField(symbols_to_ignore, copies);
    deepCopyField(python, copies);
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
