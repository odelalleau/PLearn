// -*- C++ -*-

// Dictionary.cc
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
   * $Id: Dictionary.cc,v 1.1 2004/08/03 21:17:33 larocheh Exp $ 
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file Dictionary.cc */


#include "Dictionary.h"

namespace PLearn {
using namespace std;
  
  Dictionary::Dictionary()
    :dict_type(-1)
  {
    // ### You may or may not want to call build_() to finish building the object
    // build_();
  }

  Dictionary::Dictionary(string file_name)
  {
    setDictionary(file_name);
  }

  Dictionary::Dictionary(TVec<string> symbols)
  {
    setDictionary(symbols);
  }

  Dictionary::Dictionary(string voc, string synset, string ontology, string sense_key)
  {
    setDictionary(voc, synset, ontology, sense_key);
  }

  

PLEARN_IMPLEMENT_OBJECT(Dictionary,
    "ONE LINE DESCRIPTION",
    "MULTI LINE\nHELP"
);

void Dictionary::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // ### ex:
  // declareOption(ol, "myoption", &Dictionary::myoption, OptionBase::buildoption,
  //               "Help text describing this option");
  // ...

  // Now call the parent class' declareOptions

  declareOption(ol, "voc_path", &Dictionary::voc_path, OptionBase::buildoption, "path of voc file");
  declareOption(ol, "synset_path", &Dictionary::synset_path, OptionBase::buildoption, "path of synset file");
  declareOption(ol, "ontology_path", &Dictionary::ontology_path, OptionBase::buildoption, "path of ontology file");
  declareOption(ol, "sense_key_path", &Dictionary::sense_key_path, OptionBase::buildoption, "path of sense key file");
  declareOption(ol, "dict_type", &Dictionary::dict_type, OptionBase::buildoption, "type of the dictionary");
  declareOption(ol, "file_name_dict", &Dictionary::file_name_dict, OptionBase::buildoption, "file name for the dictionary");
  declareOption(ol, "vector_dict", &Dictionary::vector_dict, OptionBase::buildoption, "vector for the dictionary");
  

  inherited::declareOptions(ol);
}

void Dictionary::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
  
  if(dict_type == FILE_DICTIONARY)
  {
    setDictionary(file_name_dict);
    return;
  }
  
  if(dict_type == VECTOR_DICTIONARY)
  {
    setDictionary(vector_dict);
    return;
  }

  if(dict_type == WORDNET_DICTIONARY)
  {
    setDictionary(voc_path, synset_path, ontology_path, sense_key_path);
    return;
  }
  
  PLERROR("Dictionary is of incorrect type %d", dict_type);
}

// ### Nothing to add here, simply calls build_
void Dictionary::build()
{
  inherited::build();
  build_();
}

int Dictionary::size()
{
  if(dict_type == VECTOR_DICTIONARY || dict_type == FILE_DICTIONARY)
  {
    return int_to_string.size();
  }

  if(dict_type == WORDNET_DICTIONARY)
  {
    return wno->getVocSize();
  }

  PLERROR("Dictionary is of incorrect type %d", dict_type);
  return -1;
}

int Dictionary::getId(string symbol)
{
  if(dict_type == VECTOR_DICTIONARY || dict_type == FILE_DICTIONARY)
  {
    if(string_to_int.find(symbol) == string_to_int.end())
      PLERROR("Could not find symbol %c", symbol.c_str());
    return string_to_int[symbol];
  }

  if(dict_type == WORDNET_DICTIONARY)
  {
    return wno->getWordId(symbol);
  }

  PLERROR("Dictionary is of incorrect type %d", dict_type);
  return -1;
}

string Dictionary::getSymbol(int id)
{
  if(dict_type == VECTOR_DICTIONARY || dict_type == FILE_DICTIONARY)
  {
    if(id >= 0 && id < int_to_string.size())
      return int_to_string[id];
    else
      PLERROR("Entry id is doesn't satisfy 0 <= %d < %d", id, int_to_string.size());
  }

  if(dict_type == WORDNET_DICTIONARY)
  {
    return wno->getWord(id);
  }

  PLERROR("Dictionary is of incorrect type %d", dict_type);
  return "";
}

void Dictionary::setDictionary(string file_name)
{
  dict_type = FILE_DICTIONARY;
  file_name_dict = file_name;

  int i=0;
  string line;

  ifstream ifs(file_name.c_str());
  while(!ifs.eof())
  {
    getline(ifs, line, '\n');
    if(line == "") continue;
    string_to_int[line] = i;
    int_to_string[i] = line;
    i++;
  }

  if(string_to_int.size() != int_to_string.size()) PLERROR("There is a symbol that occurs more than one time in the dictionary");
  ifs.close();
}
  
void Dictionary::setDictionary(TVec<string> symbols)
{
  vector_dict = symbols;
  dict_type = VECTOR_DICTIONARY;

  for(int i=0; i<symbols.size(); i++)
  {
    string_to_int[symbols[i]] = i;
    int_to_string[i] = symbols[i];
  }

  if(string_to_int.size() != int_to_string.size()) PLERROR("There is a symbol that occurs more than one time in the dictionary");
  
}

void Dictionary::setDictionary(string voc, string synset, string ontology, string sense_key)
{
  dict_type = WORDNET_DICTIONARY;

  voc_path = voc;
  synset_path = synset;
  ontology_path = ontology;
  sense_key_path = sense_key;

  wno = new WordNetOntology(voc_path, synset_path, ontology_path, sense_key_path, false, false);
  wno->fillTempWordToSensesTVecMap();
  wno->getWordSenseUniqueIdSize();
}

void Dictionary::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  //PLERROR("Dictionary::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn
