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
 * $Id$ 
 ******************************************************* */

// Authors: Hugo Larochelle, Christopher Kermorvant

/*! \file Dictionary.h */


#ifndef Dictionary_INC
#define Dictionary_INC
#include <plearn/base/stringutils.h>
#include <plearn/base/Object.h>
#include <map>
#include <string>

#define NO_UPDATE 0
#define UPDATE 1

namespace PLearn {
using namespace std;

/*! A dictionary is a mapping between a string and an index or ID (int).
  Depending on the update mode, the dictionay can include an unknown 
  word when asking for its ID with getId();
  if update_mode == UPDATE, add the word and return its ID
  if update_mode == NO_UPDATE, return OOV symbol's ID (see below)
  
  Also, when asking for the symbol associated to an ID, no update is 
  possible. If the ID has not been assigned by the dictionary, 
  then the returned symbol is "".

  A Dictionary object is instantiated empty, and then symbols can 
  be added in the dictionary by using getId() (with update_mode == UPDATE).
  By default, IDs are assigned by starting with the ID 0 and incrementing
  the assigned IDs as symbols are added in the dictionary. 

  Also, a "out-of-vocabulary" (OOV) symbol is assigned an ID which 
  is equal to the number of symbols (other than the OOV symbol) 
  that are in the dictionary at the current state. By default, 
  this symbol corresponds to the string "<oov>". 

  The OOV symbol's ID is returned by the getId(symbol) function if
  "symbol" is not in the dictionary. Also by default, this OOV token is 
  considered as part of the dictionary, but this can be changed
  using the dont_insert_oov_symbol option. If dont_insert_oov_token
  is true, then the size(), isIn() and getValues() functions
  do not consider the oov symbol as part of the dictionary. Note that
  getId() will still return the oov symbol's ID for unknown symbols,
  and getSymbol() will still return the OOV symbol for its ID.
*/

class Dictionary: public Object
{

private:
  
    typedef Object inherited;

protected:
    // *********************
    // * protected options *
    // *********************

    //! string to int mapping
    map<string,int> string_to_int;
    //! int to string mapping
    map<int,string> int_to_string;

public:

    // ************************
    // * public build options *
    // ************************
    //! update mode update/no update 
    int update_mode;
    //! Indication that "oov" should not be considered as part of the
    //! dictionary
    bool dont_insert_oov_symbol;
    //! Symbol for "out-of-vocabulary" token
    string oov_symbol;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    Dictionary();

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

    PLEARN_DECLARE_OBJECT(Dictionary);

    //! Set update dictionary mode : UPDATE/NO_UPDATE.
    virtual void setUpdateMode(int up_mode);

    //! Gives the ID of a symbol in the dictionary
    //! If the symbol is not in the dictionary, 
    //! returns the index of oov token if update_mode = NO_UPDATE.
    //! Insert the new word otherwise and return its index
    //! When a symbol is added to the dictionary, the following fields
    //! are updated: string_to_int, int_to_string, values
    //! Options can be specified ...
    virtual  int getId(string symbol, TVec<string> options = TVec<string>(0));

    //! Gives the symbol from an id of the dictionary
    //! If the id is invalid, the string returned is ""
    //! Options can be specified ...
    virtual  string getSymbol(int id, TVec<string> options = TVec<string>(0))const;
  
    //! Get size of the dictionary (number of symbols in the dictionary)
    //! Options can be specified to restrict the number of possible values. 
    virtual int size(TVec<string> options=TVec<string>(0));

    //! Fills a Vec containing every possible ID values of the Dictionary
    //! Options can be specified to restrict the number of possible values.
    //! A Vec instead of a TVec<int> is required, for compatibility with
    //! the getValues() function of VMatrix objects.
    virtual void getValues(TVec<string> options, Vec& values);

    //! Indicates if a symbol is in the dictionary
    //! The OOV symbol is considered as in the dictionary if 
    //! dont_insert_oov_symbol is false.
    //! Options can be specified ...
    virtual bool isIn(string symbol, TVec<string> options=TVec<string>(0));

    //! Indicates if an id is in the dictionary
    //! The OOV symbol's ID is considered as in the dictionary if 
    //! dont_insert_oov_symbol is false.
    //! Options can be specified ...
    virtual bool isIn(int id, TVec<string> options=TVec<string>(0));

    virtual void clear();

    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(Dictionary);
  
} // end of namespace PLearn

#endif


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
