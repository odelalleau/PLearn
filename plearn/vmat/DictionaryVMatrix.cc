// -*- C++ -*-

// DictionaryVMatrix.cc
//
// Copyright (C) 2004 Christopher Kermorvant 
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

// Authors: Christopher Kermorvant

/*! \file DictionaryVMatrix.cc */


#include "DictionaryVMatrix.h"
//#include "DiskVMatrix.h"
#include "plearn/io/openFile.h"
#include "plearn/io/fileutils.h"

namespace PLearn {
using namespace std;


DictionaryVMatrix::DictionaryVMatrix()
    :delimiters(" \t") 
    /* ### Initialise all fields to their default value */
{
    data=0;
    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(DictionaryVMatrix,
                        "VMat of text files, encoded  with Dictionaries",
                        "The lines of the text files that are empty or commented\n"
                        "out using the character # or ommited. If not Dictionary\n"
                        "objects are given by the user, then new Dictionary objects\n"
                        "are created and updated from the text files.\n"
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


Vec DictionaryVMatrix::getValues(int row, int col) const
{
    if(row < 0 || row >= length_) PLERROR("In DictionaryVMatrix::getValues() : invalid row %d, length()=%d", row, length_);
    if(col < 0 || col >= width_) PLERROR("In DictionaryVMatrix::getValues() : invalid col %d, width()=%d", col, width_);
    TVec<string> options(option_fields[col].length());
    for(int i=0; i<options.length(); i++)
    {
        options[i] = dictionaries[option_fields[col][i]]->getSymbol((int)data(row,option_fields[col][i]));
    }
    return  dictionaries[col]->getValues(options);
}

Vec DictionaryVMatrix::getValues(const Vec& input, int col) const
{
    if(col < 0 || col >= width_) PLERROR("In DictionaryVMatrix::getValues() : invalid col %d, width()=%d", col, width_);
    TVec<string> options(option_fields[col].length());
    for(int i=0; i<options.length(); i++)
    {
        options[i] = dictionaries[option_fields[col][i]]->getSymbol((int)input[option_fields[col][i]]);
    }
    return  dictionaries[col]->getValues(options);
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
    declareOption(ol, "data", &DictionaryVMatrix::data, OptionBase::buildoption,
                  "Matrix containing the concatenated and encoded text files\n");
  
  
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DictionaryVMatrix::build_()
{
    string line = "";
    vector<string> tokens;
    int it=0; 

    if(data.length()!=0) data.clear();
    
    length_ = 0;
    for(int k=0; k<file_names.length(); k++)
    {
        PPath input_file = file_names[k];
        int nlines = countNonBlankLinesOfFile(input_file);
        length_ += nlines;
        PStream input_stream = openFile(input_file, PStream::raw_ascii);
        if(k>0) data.resize(length_,n_attributes);
        while (!input_stream.eof()){
            getNextNonBlankLine(input_stream, line);
            tokens = split(line, delimiters);

            // Set n_attributes
            if(it==0)
            {
                n_attributes = int(tokens.size());
                data.resize(length_,n_attributes);
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
            }

            if((int)tokens.size() != n_attributes)
                PLERROR("In DictionaryVMatrix::build_(): %d th line \"%s\" of file %s doesn't have %d attributes", it+1-length_+nlines, line.c_str(), input_file.c_str(), n_attributes);
                
            // Insert symbols in dictionaries (if they can be updated)
            for(int j=0; j<n_attributes; j++)
            {
                if(tokens[j] == "nan") // Detect missing values
                    data(it,j) = MISSING_VALUE;
                else
                {
                    TVec<string> options(option_fields[j].length());
                    for(int k_it=0; k_it<options.length(); k_it++)
                        options[k_it] = tokens[option_fields[j][k_it]];
                    data(it,j) = dictionaries[j]->getId(tokens[j],options);
                }
            }
            it++;
        }       
    }
    width_ = n_attributes;

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
