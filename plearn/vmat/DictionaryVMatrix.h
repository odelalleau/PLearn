// -*- C++ -*-

// DictionaryVMatrix.h
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
   * $Id: DictionaryVMatrix.h,v 1.1 2004/08/25 14:44:39 kermorvc Exp $ 
   ******************************************************* */

// Authors: Christopher Kermorvant

/*! \file DictionaryVMatrix.h */


#ifndef DictionaryVMatrix_INC
#define DictionaryVMatrix_INC

#include <plearn/vmat/RowBufferedVMatrix.h>
#include <plearn/base/stringutils.h>
#include <plearn_learners/language/Dictionary.h>

// Attribute types
#define TEXT_FILE 0
#define WORD_ONTOLOGY 1
#define SENSE_ONTOLOGY 2

namespace PLearn {
using namespace std;

class DictionaryVMatrix: public RowBufferedVMatrix
{

private:
  //! The Wordnet ontology  used to extract word and sense attributes
  WordNetOntology *ontology;
  //! The dictionaries, one for each attributes
  TVec< Dictionary > dictionaries;
  //! Store the index of the word attribute. This is needed for word sense extraction 
  //! since the sense attribute depends on the word
  int word_attribute_index;
    //Mapping POS to WordNet POS index
  map<string, int> WNPosIndex;
  //! Number of attributes in the input text file (\t separated)
  int attributes_number;
  typedef RowBufferedVMatrix inherited;

  Mat data;
protected:

  // *********************
  // * protected options *
  // *********************

public:

  // ************************
  // * public build options *
  // ************************

  // ### declare public option fields (such as build options) here
  // ...
  //! The text input file which is processed with dictionaries 
  string input_file;
  //! The file which specifies the type and path to the dictionaries, one specification per line
  //! dic_type dic_path (tab separated)
  //! with dic_type is a value in : 
  //! TEXT_FILE : 0
  //! WORD_ONTOLOGY : 1
  //! SENSE_ONTOLOGY : 2
  string dic_specification_file;
  //! The dictionary for each attribute
  vector< pair<int,string> > dic_type;
  //! Use wordnet stemmer
  bool use_wordnet_stemmer;
  //! Transform word to lower case
  bool use_lower_case;
  //! Index of the pos attribute if exists in [0..attribute_number-1]
  int pos_attribute_index;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  DictionaryVMatrix();

  // ******************
  // * Object methods *
  // ******************
  DictionaryVMatrix(const string filename);
private: 

  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();
  void extractDicType();
protected: 

  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

  //! Fill the vector 'v' with the content of the i-th row.
  //! v is assumed to be the right size.
   virtual void getNewRow(int i, const Vec& v) const;

  //! returns value associated with a string (or MISSING_VALUE if there's no association for this string)
    virtual real getStringVal(int col, const string & str) const;
  //! returns element as a string, even if value doesn't map to a string, in which case tostring(value) is returned
  virtual string getString(int row, int col) const;      

  string getValString(int col, real val) const;

public:

  // Simply call inherited::build() then build_().
  virtual void build();

  //! Transform a shallow copy into a deep copy.
   virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  
  //  virtual void save(const string& filename) ;  
  //! Declare name and deepCopy methods.
  PLEARN_DECLARE_OBJECT(DictionaryVMatrix);

};

DECLARE_OBJECT_PTR(DictionaryVMatrix);

} // end of namespace PLearn
#endif
