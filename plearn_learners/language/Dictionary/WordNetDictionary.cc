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
   * $Id: WordNetDictionary.cc,v 1.5 2004/10/01 20:07:05 kermorvc Exp $ 
   ******************************************************* */

// Authors: Hugo Larochelle, Christopher Kermorvant

/*! \file WordNetDictionary.cc */


#include "WordNetDictionary.h"

namespace PLearn {
using namespace std;
  
WordNetDictionary::WordNetDictionary()
  :
  inherited(),
  stem_mode(NO_STEM),
  ontology_file_name("default")
{
  // ### You may or may not want to call build_() to finish building the object
  // build_();
}
  
WordNetDictionary::~WordNetDictionary()
{
  if(wno)
    {
      wno->finalize();
      wno->save(ontology_file_name + ".voc");
      wno->save(ontology_file_name + ".synsets", ontology_file_name + ".ontology", ontology_file_name + ".sense_key");
    }
}

WordNetDictionary::WordNetDictionary(string ontology_name,bool up_mode, bool stem)
{
  setStemMode(stem);
  setUpdateMode(up_mode);
  ontology_file_name=abspath(ontology_name);
}

  

PLEARN_IMPLEMENT_OBJECT(WordNetDictionary,
    "Dictionary instantiation from WordNetOntology files",
    "Basically, this class gives a simpler interface to the WordNetOntology class.\n"
  "The symbols in the instantiated dictionary are words (not senses!).\n");

void WordNetDictionary::declareOptions(OptionList& ol)
{
  declareOption(ol, "ontology_file_name", &WordNetDictionary::ontology_file_name, OptionBase::buildoption, "path to the ontology");
  declareOption(ol, "stem_mode", &WordNetDictionary::stem_mode, OptionBase::buildoption, "indication that words should be stemmed");
  inherited::declareOptions(ol);
}

void WordNetDictionary::build_()
{
  // Loading ontology...
  string voc_file = ontology_file_name + ".voc";
  string synset_file = ontology_file_name + ".synsets";
  string ontology_file = ontology_file_name + ".ontology";
  string sense_key_file = ontology_file_name + ".sense_key";
  wno = new WordNetOntology(voc_file, synset_file, ontology_file, sense_key_file, false, false);
  wno->fillTempWordToSensesTVecMap();
  wno->getWordSenseUniqueIdSize();
  string_to_int = wno->getWordsId();
  int_to_string = wno->getWords();
  // save update mode for later
  int saved_up_mode=update_mode;
  // set the dictionary in update mode to insert the words
  update_mode =  UPDATE;
  
  // Add OOV if necessary  HUGO: I don't think it's necessary... CK: Yes, it is
  if (update_mode==NO_UPDATE){
    if (!wno->containsWord(OOV_TAG)){
      wno->extractWord(OOV_TAG, ALL_WN_TYPE, true, true, false);
    }
  }
  // restore update mode;
  update_mode=saved_up_mode;
}

// ### Nothing to add here, simply calls build_
void WordNetDictionary::build()
{
  inherited::build();
  build_();
}

void  WordNetDictionary::setStemMode(bool stem)
{
  stem_mode =stem;
}

int WordNetDictionary::getId(string symbol, TVec<string> options)
{
  // Gives the id of a symbol in the dictionary
  // If the symbol is not in the dictionary, 
  // returns index of OOV_TAG if update_mode = NO_UPDATE
  // insert the new word otherwise and return its index

  if(!wno) PLERROR("WordNetDictionary::getId : wno is not instantiated. build() should be called");
  
  if(stem_mode)
    symbol = stemWord(symbol);
  int oov_index = string_to_int[OOV_TAG];
  int index;
  if(update_mode== UPDATE){
    if(!wno->containsWord(symbol)){
      // word not found in the ontology, add it
      wno->extractWord(symbol, ALL_WN_TYPE, true, true, false);
    }
    if(string_to_int.find(symbol) == string_to_int.end()){
      index = wno->getWordId(symbol);
      // word not found in the map, store it
      string_to_int[symbol] = index;
      int_to_string[index] = symbol;
    }
    return string_to_int[symbol];
    
  }else{
    // NO update mode
    if(string_to_int.find(symbol) == string_to_int.end()){
      
      index = wno->getWordId(symbol);
      if(index!=oov_index){
	// word not found in the map, but found in wno, store it
	string_to_int[symbol] = index;
	int_to_string[index] = symbol;
      }
      return index;
    }
    return string_to_int[symbol];
  }
  return -1;  
}

int WordNetDictionary::getId(string symbol, TVec<string> options )const
{
  // Const version
  // Gives the id of a symbol in the dictionary
  // If the symbol is not in the dictionary, 
  // returns -1

  if(!wno) PLERROR("WordNetDictionary::getId : wno is not instantiated. build() should be called");

  if(stem_mode)
    symbol = stemWord(symbol);

  if(string_to_int.find(symbol) == string_to_int.end()){
    return -1;
  }
  return string_to_int.find(symbol)->second;  
}

string WordNetDictionary::getSymbol(int id, TVec<int>options )const
{
  if(!wno) PLERROR("WordNetDictionary::getId : wno is not instantiated. build() should be called");

  if(!wno->isWord(id)) return "";
  return wno->getWord(id);

}

void WordNetDictionary::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

} // end of namespace PLearn
