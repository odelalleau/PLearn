// -*- C++ -*-

// Dictionary.h
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
   * $Id: Dictionary.h,v 1.1 2004/08/03 21:17:33 larocheh Exp $ 
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file Dictionary.h */


#ifndef Dictionary_INC
#define Dictionary_INC

#include <plearn/base/Object.h>
#include <map>
#include <string>
#include <plearn_learners/language/WordNet/WordNetOntology.h>

#define VECTOR_DICTIONARY   1
#define FILE_DICTIONARY     2
#define WORDNET_DICTIONARY  3


namespace PLearn {
using namespace std;

class Dictionary: public Object
{

private:
  
  typedef Object inherited;

protected:
  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...
    
public:

  // ************************
  // * public build options *
  // ************************

  //! string to int mapping
  map<string,int> string_to_int;
  //! int to string mapping
  map<int,string> int_to_string;
  //! WordNet ontology of the dictionary
  WordNetOntology *wno;
  //! type of the dictionary
  int dict_type;
  //! path of voc file
  string voc_path;
  //! path of synset file
  string synset_path;
  //! path of sense key file
  string sense_key_path;
  //! path of ontology file
  string ontology_path;
  //! file_name of dictionary
  string file_name_dict;
  //! Vector of dictionary
  TVec<string> vector_dict;
  
  // ### declare public option fields (such as build options) here
  // ...

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  // ### Make sure the implementation in the .cc
  // ### initializes all fields to reasonable default values.
  Dictionary();


  //! Constructor
  /*!
    \param file_name file containing the symbols of the dictionary
   */
  Dictionary(string file_name);

  //! Constructor
  /*!
    \param symbols vector of the symbols of the dictionary
   */
  Dictionary(TVec<string> symbols);
  
  //! Constructor
  /*!
    \param voc_path path of voc file
    \param synset_path path of synset file
    \param ontology_path path of ontology file
    \param sense_key_path path of sense key file
   */
  Dictionary(string voc, string synset, string ontology, string sense_key);


  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  void build_();

protected: 
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:
  // Declares other standard object methods.
  // ### If your class is not instantiatable (it has pure virtual methods)
  // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
  PLEARN_DECLARE_OBJECT(Dictionary);

  //! Gives the size of the dictionary
  int size();

  //! Gives the id of a symbol in the dictionary
  int getId(string symbol);
  
  //! Gives the symbol from an id of the dictionary
  string getSymbol(int id);

  //! Sets the dictionary mapping and the type to FILE_DICTIONARY
  void setDictionary(string file_name);
  
  //! Sets the dictionary mapping and the type to VECTOR_DICTIONARY
  void setDictionary(TVec<string> symbols);
  
  //! Sets the dictionary mapping and the type to WORDNET_DICTIONARY
  void setDictionary(string voc, string synset, string ontology, string sense_key);


  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(Dictionary);
  
} // end of namespace PLearn

#endif
