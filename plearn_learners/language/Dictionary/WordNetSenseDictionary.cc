// -*- C++ -*-

// WordNetDictionary.cc
//
// Copyright (C) 2004 Hugo Larochelle, Christopher Kermorvant
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
   * $Id: WordNetSenseDictionary.cc,v 1.3 2004/09/14 18:52:56 kermorvc Exp $ 
   ******************************************************* */

// Authors: Hugo Larochelle, Christopher Kermorvant

/*! \file WordSenseNetDictionary.cc */


#include "WordNetSenseDictionary.h"

namespace PLearn {
using namespace std;
  
WordNetSenseDictionary::WordNetSenseDictionary()
    :
    inherited(),
    ontology_file_name("default")
  {}

WordNetSenseDictionary::~WordNetSenseDictionary()
{
    wno->finalize();
    wno->save(ontology_file_name + ".voc");
    wno->save(ontology_file_name + ".synsets", ontology_file_name + ".ontology", ontology_file_name + ".sense_key");
}
  
WordNetSenseDictionary::WordNetSenseDictionary(string ontology_name,bool up_mode)
{
  setUpdateMode(up_mode);
  ontology_file_name=ontology_name;
}

  
  
PLEARN_IMPLEMENT_OBJECT(WordNetSenseDictionary,
			  "Dictionary instantiation from WordNetOntology files",
			  "Basically, this class gives a simpler interface to the WordNetOntology class.\n"
			  "The symbols in the instantiated dictionary are senses (not words!).\n");
  
void WordNetSenseDictionary::declareOptions(OptionList& ol)
  {
    declareOption(ol, "ontology_file_name", &WordNetSenseDictionary::ontology_file_name, OptionBase::buildoption, "path to the ontology");
    inherited::declareOptions(ol);
  }
  
void WordNetSenseDictionary::build_()
{
  // Loading ontology...
  string voc_file = ontology_file_name + ".voc";
  string synset_file = ontology_file_name + ".synsets";
  string ontology_file = ontology_file_name + ".ontology";
  string sense_key_file = ontology_file_name + ".sense_key";
  wno = new WordNetOntology(voc_file, synset_file, ontology_file, sense_key_file, false, false);
  wno->fillTempWordToSensesTVecMap();
  wno->getWordSenseUniqueIdSize();
  
  // Add NO_SENSE if necessary
  if (update_mode==NO_UPDATE){
    string_to_int[NO_SENSE_TAG] = NO_SENSE;
    int_to_string[NO_SENSE] = NO_SENSE_TAG;
  }
}

// ### Nothing to add here, simply calls build_
void WordNetSenseDictionary::build()
{
  inherited::build();
  build_();
}


int WordNetSenseDictionary::getId(string symbol, TVec<string> options)
{
  // Gives the id of a symbol in the dictionary
  // If the symbol is not in the dictionary, 
  // returns index of OOV_TAG if update_mode = NO_UPDATE
  // insert the new word otherwise and return its index

  if(!wno) PLERROR("WordNetSenseDictionary::getId : wno is not instantiated. build() should be called");
  int index;
  if(update_mode== UPDATE){
    if(options.length()!=1)PLERROR("WordNetSenseDictionary: need word to get id of a sense - You must specify the position of the word attribute with option_fields ");
    //  word not found in the ontology, add it
    if(!wno->containsWord(options[0])){
      wno->extractWord(options[0], ALL_WN_TYPE, true, true, false);
    }
    if(string_to_int.find(symbol) == string_to_int.end()){
      // sense not found in the map, store it
      index =wno->getSynsetIDForSenseKey(wno->getWordId(options[0]),symbol);
      string_to_int[symbol] = index;
      int_to_string[index] = symbol;
      // TODO : values -----
      
    }
    return index;
    
  }else{
    if(options.length()!=1)PLERROR("WordNetSenseDictionary: need word to get id of a sense");
    if(string_to_int.find(symbol) == string_to_int.end()){
      // sense not found in the map, store it
      index =wno->getSynsetIDForSenseKey( wno->getWordId(options[0]),symbol);
      if(index!=NO_SENSE){
	string_to_int[symbol] = index;
	int_to_string[index] = symbol;
      }
      return index;
    }
    return string_to_int[symbol];
  }
  return -1;  
}

int WordNetSenseDictionary::getId(string symbol, TVec<string> options)const
{
  return -1;
}

string WordNetSenseDictionary::getSymbol(int id, TVec<string> options)const
{
  if(!wno) PLERROR("WordNetSenseDictionary::getId : wno is not instantiated. build() should be called");
  if(int_to_string.find(id)==int_to_string.end()){
    PLERROR("Entry %d  doesn't exist in mapping ", id);
  }else{
    return int_to_string.find(id)->second;
  }
  return "";
}

void WordNetSenseDictionary::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

} // end of namespace PLearn
