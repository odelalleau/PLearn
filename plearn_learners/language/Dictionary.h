// -*- C++ -*-

// Dictionary.h
//
// Copyright (C) 2004 Hugo Larochelle Christopher Kermorvant
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
   * $Id: Dictionary.h,v 1.4 2004/08/25 21:44:45 kermorvc Exp $ 
   ******************************************************* */

// Authors: Hugo Larochelle, Christopher Kermorvant

/*! \file Dictionary.h */


#ifndef Dictionary_INC
#define Dictionary_INC
#include <plearn/base/stringutils.h>
#include <plearn/base/Object.h>
#include <map>
#include <string>
#include <plearn_learners/language/WordNet/WordNetOntology.h>

#define VECTOR_DICTIONARY   1
#define FILE_DICTIONARY     2
#define WORDNET_WORD_DICTIONARY  3
#define WORDNET_SENSE_DICTIONARY 4

#define NO_UPDATE 0
#define UPDATE 1
// Default mode for the dicionary
#define DEFAULT_UPDATE 0

// For words only
#define NO_STEM 0
#define STEM 1
#define OOV_TAG "<oov>"

//for WordNet senses only
// No sense exists for this word
#define NO_SENSE -1
// Sense exists but is hidden (un-known)
#define HIDDEN_SENSE 0

namespace PLearn {
using namespace std;

/*! A dictionary is a mapping between a string and and index (int).
  Depending on the update mode, the dictionay can include an unkown 
  word when asking for its Id with getId();
  if update_mode == UPDATE, add the word and return its Id
  if  update_mode == NO_UPDATE, return Id of OOV (out of vocabulary word)

  When using a WORDNET_WORD_DICTIONARY the word can be stemmed before
  including in the dictionary :  stem_mode == STEM

  The indices of the words are supposed to be consecutives : 
  string_to_int.size()==max_id-1
  
*/

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
  //! string to int mapping
  map<string,int> string_to_int;
  //! int to string mapping
  map<int,string> int_to_string;
  //! WordNet ontology of the dictionary
  WordNetOntology  *wno;

public:

  // ************************
  // * public build options *
  // ************************
  //! type of the dictionary
  int dict_type;
  //! update mode update/no update 
  int update_mode;
  //! Stem word before including in dictionary STEM/NO_STEM (ontology only)
  int stem_mode;
  //! file_name of dictionary
  string file_name_dict;
  //! path to ontology
  string ontology_file_name;
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
  Dictionary(string file_name,bool up_mode=DEFAULT_UPDATE);

  //! Constructor
  /*!
    \param symbols vector of the symbols of the dictionary
   */
  Dictionary(TVec<string> symbols,bool up_mode=DEFAULT_UPDATE);
  
  //! Constructor
  /*!
    \param up_mode update mode : NO_UPDATE, UPDATE
    \param stem use word stemming : NO_STEM/STEM 
   */
  Dictionary(WordNetOntology *ont,string ontology_name,int ontology_type,bool up_mode=DEFAULT_UPDATE, bool stem =NO_STEM);
    
  
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

  //! Set update dictionary mode : UPDATE/NO_UPDATE.
  void  setUpdateMode(bool up_mode);

  //! Set stem mode NO_STEM/STEM
  void  setStemMode(bool stem);
    
  //! Set the type of dictionary :  
  //! VECTOR_DICTIONARY,FILE_DICTIONARY,WORDNET_WORD_DICTIONARY,WORDNET_SENSE_DICTIONARY
  void  setDictionaryType(int type);

  //! Gives the id of a symbol in the dictionary
  //! If the symbol is not in the dictionary, 
  //! returns index of OOV_TAG if update_mode = NO_UPDATE
  //! insert the new word otherwise and return its index
  int getId(string symbol);

  //! Const version. Do not insert unknown words
  int getId(string symbol)const;
  
  //! Gives the symbol from an id of the dictionary
  string getSymbol(int id)const;
  
  //! Get dimension of the dictionaries (number of differents values in the dictionary)
  int getDimension(){return string_to_int.size();}

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(Dictionary);
  
} // end of namespace PLearn

#endif
