// -*- C++ -*-

// Dictionary.cc
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
   * $Id: Dictionary.cc,v 1.4 2004/08/25 21:44:45 kermorvc Exp $ 
   ******************************************************* */

// Authors: Hugo Larochelle, Christopher Kermorvant

/*! \file Dictionary.cc */


#include "Dictionary.h"

namespace PLearn {
using namespace std;
  
  Dictionary::Dictionary()
    :
    dict_type(-1),
    update_mode(0), 
    stem_mode(0),
    file_name_dict(""),
    ontology_file_name(""),
    vector_dict(0)
  {
    // ### You may or may not want to call build_() to finish building the object
    // build_();
  }

  Dictionary::Dictionary(string file_name, bool up_mode)
  {
    setStemMode(NO_STEM);
    setUpdateMode(up_mode);
    setDictionaryType(FILE_DICTIONARY);
    file_name_dict=file_name;
  }

  Dictionary::Dictionary(TVec<string> symbols, bool up_mode)
  {
    setStemMode(NO_STEM);
    setUpdateMode(up_mode);
    setDictionaryType(VECTOR_DICTIONARY);
    vector_dict=symbols;
  }

  Dictionary::Dictionary(WordNetOntology *ont,string ontology_name,int ontology_type,bool up_mode, bool stem)
  {
    setStemMode(stem);
    setUpdateMode(up_mode);
    setDictionaryType(ontology_type);
    ontology_file_name=ontology_name;
    wno=ont;
  }

  

PLEARN_IMPLEMENT_OBJECT(Dictionary,
    "Mapping string->int and int->string",
    "Dictionary is a mapping between string and int. This mapping can be build automatically \n by the dictionary object (as it is done in StrTableVMatrix) but can also be specified.  \n It is usefull if you want several matrices which share the same mapping. There are 3    \n to specify a mapping as set by the dic_type parameter. with a file (file_name_dict),    \n with a vector (vector_dict) and with an ontology (ontology). \n The function getId  gives the id of a symbol in the dictionary. If the symbol is not in \n the dictionary and  update_mode = NO_UPDATE, it returns index of OOV_TAG. if the symbol \n is not update_mode = UPDATE, it inserts the new word and return its index.");

void Dictionary::declareOptions(OptionList& ol)
{
  declareOption(ol, "dict_type", &Dictionary::dict_type, OptionBase::buildoption, "type of the dictionary : \n\tVECTOR_DICTIONARY\t1\n\tFILE_DICTIONARY\t2\n\tWORDNET_WORD_DICTIONARY\t3\n\tWORDNET_SENSE_DICTIONARY\t4");
  declareOption(ol, "file_name_dict", &Dictionary::file_name_dict, OptionBase::buildoption, "file name for the dictionary");
  declareOption(ol, "ontology_file_name", &Dictionary::ontology_file_name, OptionBase::buildoption, "path to the ontology");
  declareOption(ol, "vector_dict", &Dictionary::vector_dict, OptionBase::buildoption, "vector for the dictionary");
  declareOption(ol, "update_mode", &Dictionary::update_mode, OptionBase::buildoption, "update_mode : 0(no_update)/1(update). Default is no_update");
  declareOption(ol, "string_to_int", &Dictionary::string_to_int, OptionBase::buildoption, "string to int mapping");
  declareOption(ol, "int_to_string", &Dictionary::int_to_string, OptionBase::buildoption, "int to string mapping");
  // Problem : WordNet onlogy is not a PLearn::Object. declareOption(ol, "ontology", &Dictionary::wno, OptionBase::nosave, "pointer to the ontology");
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
  
  //initial building
  if(string_to_int.size()==0){
    // save update mode for later
    int saved_up_mode=update_mode;
    // set the dictionary in update mode to insert the words
    update_mode =  UPDATE;
    string line;
    if(dict_type == FILE_DICTIONARY){
      ifstream ifs(file_name_dict.c_str());
      if (!ifs) PLERROR("Cannot open file %s", file_name_dict.c_str());
      while(!ifs.eof()){
	getline(ifs, line, '\n');
	if(line == "") continue;
	getId(line);
      }
      ifs.close();
      if(saved_up_mode==NO_UPDATE){
      // the dictionary must contain oov
	getId(OOV_TAG);
      }
    }else if(dict_type == VECTOR_DICTIONARY){
      for(int i=0; i<vector_dict.size(); i++){
	getId(vector_dict[i]);
      }
      if(saved_up_mode==NO_UPDATE){
	// the dictionary must contain oov
	getId(OOV_TAG);
      }
    }else  if(dict_type == WORDNET_WORD_DICTIONARY){
      // Add OOV if necessary
      if (update_mode==NO_UPDATE){
	if (!wno->containsWord(OOV_TAG)){
	  wno->extractWord(OOV_TAG, ALL_WN_TYPE, true, true, false);
	}
      }
    }else{
      PLERROR("Bad dictionary type %d",dict_type);
    }
    // restore update mode;
    update_mode=saved_up_mode;
  }
}

// ### Nothing to add here, simply calls build_
void Dictionary::build()
{
  inherited::build();
  build_();
}

int Dictionary::size()
{
  if(dict_type == VECTOR_DICTIONARY || dict_type == FILE_DICTIONARY){
    return int_to_string.size();
  }
    if(dict_type == WORDNET_WORD_DICTIONARY){
    return wno->getVocSize();
  }
    PLERROR("Dictionary is of incorrect type %d", dict_type);
  return -1;
}

void  Dictionary::setUpdateMode(bool up_mode)
{
  update_mode =up_mode;
}

void  Dictionary::setStemMode(bool stem)
{
  stem_mode =stem;
}

void  Dictionary::setDictionaryType(int type)
{
  dict_type=type;
}



int Dictionary::getId(string symbol)
{
  // Gives the id of a symbol in the dictionary
  // If the symbol is not in the dictionary, 
  // returns index of OOV_TAG if update_mode = NO_UPDATE
  // insert the new word otherwise and return its index
  int index;
  if(update_mode== UPDATE){
    if(dict_type == VECTOR_DICTIONARY || dict_type == FILE_DICTIONARY)
      {
	if(string_to_int.find(symbol) == string_to_int.end()){
	  // word not found, add it
	  index=string_to_int.size();
	  // update max_id if necessary
	  //if(index>max_id)max_id=index;
	  // set the maps
	  string_to_int[symbol] = index;
	  int_to_string[index] = symbol;
	}
	return string_to_int[symbol];
      }
    if(dict_type == WORDNET_WORD_DICTIONARY){
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
    }
    if(dict_type == WORDNET_SENSE_DICTIONARY){
      vector<string> tokens = split(symbol, "/");
      if(tokens.size()!=2)PLERROR("Badly formed word for sense extraction %s",symbol.c_str());
      //  word not found in the ontology, add it
      if(!wno->containsWord(tokens[0])){
	wno->extractWord(symbol, ALL_WN_TYPE, true, true, false);
      }
      if(string_to_int.find(symbol) == string_to_int.end()){
	// sense not found in the map, store it
	index =wno->getSynsetIDForSenseKey( wno->getWordId(tokens[0]),tokens[1]);
	string_to_int[symbol] = index;
	int_to_string[index] = symbol;
      }
      return index;
    }
    PLERROR(" Dictionary::getId : bad dictionary type %d",dict_type);
  }else{
    // NO update mode
    if(dict_type == VECTOR_DICTIONARY || dict_type == FILE_DICTIONARY){
      if(string_to_int.find(symbol) == string_to_int.end()){
	// word not found, return oov
	return string_to_int[OOV_TAG];
      }else{
	return string_to_int[symbol];
      }
    }
    if(dict_type == WORDNET_WORD_DICTIONARY){
      if(string_to_int.find(symbol) == string_to_int.end()){
	index = wno->getWordId(symbol);
      	// word not found in the map, store it
	string_to_int[symbol] = index;
	int_to_string[index] = symbol;
      }
      return string_to_int[symbol];
    }
    if(dict_type == WORDNET_SENSE_DICTIONARY){
      vector<string> tokens = split(symbol, "/");
      if(tokens.size()!=2)PLERROR("Badly formed word for sense extraction %s",symbol.c_str());
      if(string_to_int.find(symbol) == string_to_int.end()){
	// sense not found in the map, store it
	index =wno->getSynsetIDForSenseKey( wno->getWordId(tokens[0]),tokens[1]);
	string_to_int[symbol] = index;
	int_to_string[index] = symbol;
      }
      return index;
    }
    PLERROR(" Dictionary::getId : bad dictionary type %d",dict_type);
  }
  return 1;  
}

int Dictionary::getId(string symbol)const
{
  // Const version
  // Gives the id of a symbol in the dictionary
  // If the symbol is not in the dictionary, 
  // returns index of OOV_TAG
  if(dict_type == VECTOR_DICTIONARY || dict_type == FILE_DICTIONARY || dict_type == WORDNET_WORD_DICTIONARY || dict_type == WORDNET_WORD_DICTIONARY){
    if(string_to_int.find(symbol) == string_to_int.end()){
      // word not found, return oov
      if(string_to_int.find(OOV_TAG)==string_to_int.end()){ 
	PLERROR("Dictionary::getId can't find OOV_TAG and dictionary is const");
      }else{
	return (string_to_int.find(OOV_TAG))->second;
      }
    }else{
      return (string_to_int.find(symbol))->second;
    }
  }
  PLERROR(" Dictionary::getId : bad dictionary type %d",dict_type);
  return 1;  
}

string Dictionary::getSymbol(int id)const
{
  if(dict_type == VECTOR_DICTIONARY || dict_type == FILE_DICTIONARY)
  {
    if(id >= 0 && id < (int)int_to_string.size()){
      return (int_to_string.find(id))->second;
    }else{
      PLERROR("Entry id is doesn't satisfy 0 <= %d < %d", id, int_to_string.size());
    }
  }

  if(dict_type == WORDNET_WORD_DICTIONARY)
  {
    return wno->getWord(id);
  }

  PLERROR("Dictionary is of incorrect type %d", dict_type);
  return "";
}

void Dictionary::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(string_to_int, copies);
  deepCopyField(int_to_string, copies);
  deepCopyField(vector_dict, copies);
}

} // end of namespace PLearn
