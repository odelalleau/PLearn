// -*- C++ -*-

// WordNetDictionary.h
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
   * $Id: WordNetDictionary.h,v 1.3 2004/09/14 18:52:56 kermorvc Exp $ 
   ******************************************************* */

// Authors: Hugo Larochelle, Christopher Kermorvant

/*! \file WordNetDictionary.h */


#ifndef WordNetDictionary_INC
#define WordNetDictionary_INC
#include "Dictionary.h"
#include <plearn_learners/language/WordNet/WordNetOntology.h>

#define NO_STEM 0
#define STEM 1

namespace PLearn {
using namespace std;

/*! This class implements a Dictionary instantiated from WordNetOntology files.
  Basically, this class gives a simpler interface to the WordNetOntology class.
  The symbols in the instantiated dictionary are words (not senses!).
*/

class WordNetDictionary: public Dictionary
{

private:
  
  typedef Dictionary inherited;

protected:
  // *********************
  // * protected options *
  // *********************

  //! WordNet ontology of the dictionary
  WordNetOntology *wno;

public:

  // ************************
  // * public build options *
  // ************************
  //! Stem word before including in dictionary STEM/NO_STEM (ontology only)
  bool stem_mode;
  //! path to ontology
  string ontology_file_name;
  
  // ### declare public option fields (such as build options) here
  // ...

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  WordNetDictionary();

  //! Destructor
  ~WordNetDictionary();

  //! Constructor
  /*!
    \param 
    \param up_mode update mode : NO_UPDATE, UPDATE
    \param stem use word stemming : NO_STEM/STEM 
   */
  WordNetDictionary(string ontology_name,bool up_mode=DEFAULT_UPDATE,bool stem= NO_STEM);
    
  
  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  void build_();

  //! Set stem mode NO_STEM/STEM
  void setStemMode(bool stem);


protected: 
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:
  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(WordNetDictionary);

  //! Gives the id of a symbol in the dictionary
  //! If the symbol is not in the dictionary, 
  //! returns index of OOV_TAG if update_mode = NO_UPDATE
  //! insert the new word otherwise and return its index
  int getId(string symbol, TVec<string> options = TVec<string>(0));

  //! Const version. Do not insert unknown words
  int getId(string symbol, TVec<string> options = TVec<string>(0))const;
  
  //! Gives the symbol from an id of the dictionary
  string getSymbol(int id, TVec<int> options = TVec<int>(0))const;

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(WordNetDictionary);
  
} // end of namespace PLearn

#endif
