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
   * $Id: DictionaryVMatrix.cc,v 1.10 2005/02/04 15:10:42 tihocan Exp $ 
   ******************************************************* */

// Authors: Christopher Kermorvant

/*! \file DictionaryVMatrix.cc */


#include "DictionaryVMatrix.h"
#include "DiskVMatrix.h"
#include "plearn/io/fileutils.h"

namespace PLearn {
using namespace std;


DictionaryVMatrix::DictionaryVMatrix()
  :inherited(),delimiters(" \t") 
  /* ### Initialise all fields to their default value */
{
  data=0;
  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

DictionaryVMatrix::DictionaryVMatrix(const string filename)
{
  load(filename);
}


PLEARN_IMPLEMENT_OBJECT(DictionaryVMatrix,
    "VMat encoded  with Dictionaries",
    "NO HELP"
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
  else return dictionaries[col]->getId(str);
}

string DictionaryVMatrix::getValString(int col, real val) const
{
  if(is_missing(val))return tostring(val);
  return dictionaries[col]->getSymbol((int)val);
}

int DictionaryVMatrix::getDimension(int row, int col) const
{
  if(row < 0 || row >= length_) PLERROR("In DictionaryVMatrix::getDimension() : invalid row %d, length()=%d", row, length_);
  if(col < 0 || col >= length_) PLERROR("In DictionaryVMatrix::getDimension() : invalid col %d, width()=%d", col, width_);
  TVec<int> options(option_fields[col].length());
  for(int i=0; i<options.length(); i++)
  {
    options[i] = (int)data(row,option_fields[col][i]);
  }
  return  dictionaries[col]->getDimension(options);
}

Vec DictionaryVMatrix::getValues(int row, int col) const
{
  if(row < 0 || row >= length_) PLERROR("In DictionaryVMatrix::getValues() : invalid row %d, length()=%d", row, length_);
  if(col < 0 || col >= length_) PLERROR("In DictionaryVMatrix::getValues() : invalid col %d, width()=%d", col, width_);
  TVec<int> options(option_fields[col].length());
  for(int i=0; i<options.length(); i++)
  {
    options[i] = (int)data(row,option_fields[col][i]);
  }
  return  dictionaries[col]->getValues(options);
}

Vec DictionaryVMatrix::getValues(const Vec& input, int col) const
{
  if(col < 0 || col >= length_) PLERROR("In DictionaryVMatrix::getValues() : invalid col %d, width()=%d", col, width_);
  TVec<int> options(option_fields[col].length());
  for(int i=0; i<options.length(); i++)
  {
    options[i] = (int)input[option_fields[col][i]];
  }
  return  dictionaries[col]->getValues(options);
}


void DictionaryVMatrix::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // ### ex:
  // declareOption(ol, "myoption", &DictionaryVMatrix::myoption, OptionBase::buildoption,
  //               "Help text describing this option");
  // ...
  declareOption(ol, "input_file", &DictionaryVMatrix::input_file, OptionBase::buildoption,
		  "The text file from which we create the VMat");
  declareOption(ol, "dictionaries", &DictionaryVMatrix::dictionaries, OptionBase::buildoption,
                "Vector of dictionaries\n");
  declareOption(ol, "option_fields", &DictionaryVMatrix::option_fields, OptionBase::buildoption,
                "Vector of the fields corresponding to the options of the Dictionary, for every Dictionary\n");
  declareOption(ol, "data", &DictionaryVMatrix::data, OptionBase::buildoption,
                "Encoded Matrix\n");
  declareOption(ol, "delimiters", &DictionaryVMatrix::delimiters, OptionBase::buildoption,
                "Delimiters for file fields (or attributs)\n");
  
  
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void DictionaryVMatrix::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.

  // Nothing to do if the VMatrix is reloaded...
  if(data.length()!=0)return;

  if(option_fields.length()==0) option_fields.resize(dictionaries.length());

  attributes_number = dictionaries.length();
  input_file = abspath(input_file);
  ifstream input_stream(input_file.c_str());
  if (!input_stream) PLERROR("DictionaryVMatrix: can't open input_file %s", input_file.c_str());
  // get file length
  int input_length = 0;
  string line = "";
  vector<string> tokens;
  // read first lines to set attributes_number value
  while (!input_stream.eof()){
    getline(input_stream, line, '\n');
    if (line == "" || line[0] == '#') continue;
    tokens = split(line, delimiters);
    if( (int)attributes_number != (int)tokens.size()) PLERROR("Number of attributs is different from number of dictionaries on line: %s", line.c_str());
    input_length++;
  }
  input_stream.close();

  ifstream input_stream2(input_file.c_str());
  data.resize(input_length,attributes_number);
  width_ = attributes_number;
  length_ = input_length;

  int i=0;
  while (!input_stream2.eof()){
    getline(input_stream2, line, '\n');
    if (line == "" || line[0] == '#') continue;
    tokens = split(line, delimiters);
    for(int j=0; j<(int)tokens.size(); j++)
    {
      TVec<string> options(option_fields[j].length());
      for(int k=0; k<options.length(); k++)
        options[k] = tokens[option_fields[j][k]];
      data(i,j) = dictionaries[j]->getId(tokens[j],options);
    }
    i++;
  }
  input_stream2.close();
  
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

  deepCopyField(dictionaries, copies);
  deepCopyField(option_fields, copies);
}

} // end of namespace PLearn

