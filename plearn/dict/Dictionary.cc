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
 * $Id$ 
 ******************************************************* */

// Authors: Hugo Larochelle, Christopher Kermorvant

/*! \file Dictionary.cc */


#include "Dictionary.h"

namespace PLearn {
using namespace std;
  
Dictionary::Dictionary()
    :
    update_mode(UPDATE), 
    dont_insert_oov_symbol(false),
    oov_symbol("<oov>")
{
}

PLEARN_IMPLEMENT_OBJECT(Dictionary,
                        "Mapping string->int and int->string",
  "A dictionary is a mapping between a string and an index or ID (int).\n"
  "Depending on the update mode, the dictionay can include an unknown \n"
  "word when asking for its ID with getId();\n"
  "if update_mode == UPDATE, add the word and return its ID\n"
  "if update_mode == NO_UPDATE, return OOV symbol's ID (see below)\n"
  "\n"
  "Also, when asking for the symbol associated to an ID, no update is \n"
  "possible. If the ID has not been assigned by the dictionary, \n"
  "then the returned symbol is \"\".\n"
  "\n"
  "A Dictionary object is instantiated empty, and then symbols can \n"
  "be added in the dictionary by using getId() (with update_mode == UPDATE).\n"
  "By default, IDs are assigned by starting with the ID 0 and incrementing\n"
  "the assigned IDs as symbols are added in the dictionary. \n"
  "\n"
  "Also, a \"out-of-vocabulary\" (OOV) symbol is assigned an ID which \n"
  "is equal to the number of symbols (other than the OOV symbol) \n"
  "that are in the dictionary at the current state. By default, \n"
  "this symbol corresponds to the string \"<oov>\". \n"
  "\n"
  "The OOV symbol's ID is returned by the getId(symbol) function if\n"
  "\"symbol\" is not in the dictionary. Also by default, this OOV token is\n" 
  "considered as part of the dictionary, but this can be changed\n"
  "using the dont_insert_oov_symbol option. If dont_insert_oov_token\n"
  "is true, then the size(), isIn() and getValues() functions\n"
  "do not consider the oov symbol as part of the dictionary. Note that\n"
  "getId() will still return the oov symbol's ID for unknown symbols,\n"
  "and getSymbol() will still return the OOV symbol for its ID.\n"
);

void Dictionary::declareOptions(OptionList& ol)
{
    declareOption(ol, "update_mode", &Dictionary::update_mode, 
                  OptionBase::buildoption, 
                  "update_mode : 0(no_update)/1(update). Default is update");
    declareOption(ol, "oov_symbol", &Dictionary::oov_symbol, 
                  OptionBase::buildoption, 
                  "String symbol for \"out-of-vocabulary\" token.");
    declareOption(ol, "string_to_int", &Dictionary::string_to_int, 
                  OptionBase::learntoption, "string to int mapping");
    declareOption(ol, "int_to_string", &Dictionary::int_to_string, 
                  OptionBase::learntoption, "int to string mapping");
    // For backward compatibility...
    declareOption(ol, "oov_not_in_possible_values", 
                  &Dictionary::dont_insert_oov_symbol, 
                  OptionBase::buildoption,
                  "DEPRECATED, use dont_insert_oov_symbol instead: Indication that\n"
                  "the OOV symbol should not be considered as part of the dictionary"); 
    declareOption(ol, "dont_insert_oov_symbol", 
                  &Dictionary::dont_insert_oov_symbol, 
                  OptionBase::buildoption, 
                  "Indication that the OOV symbol should not be considered\n"
                  "as part of the dictionary"); 

    inherited::declareOptions(ol);
}

void Dictionary::build_(){
}

// ### Nothing to add here, simply calls build_
void Dictionary::build()
{
    inherited::build();
    build_();
}

void Dictionary::setUpdateMode(int up_mode)
{
    if(up_mode != UPDATE && up_mode != NO_UPDATE)
        PLERROR("In Dictionary::setUpdateMode(): incompatible update mode %d", up_mode);
    update_mode =up_mode;
}

int Dictionary::getId(string symbol, TVec<string> options)
{
    // Gives the id of a symbol in the dictionary
    // If the symbol is not in the dictionary, 
    // returns index of OOV symbol if update_mode = NO_UPDATE,
    // insert the new word otherwise and return its index

    if(symbol == oov_symbol) return string_to_int.size();

    int index;
    if(update_mode== UPDATE)
    {
        if(string_to_int.find(symbol) == string_to_int.end()){
            // word not found, add it
            index=int(string_to_int.size());
            string_to_int[symbol] = index;
            int_to_string[index] = symbol;
        }
        return string_to_int[symbol];
    }
    else
    {
        // No update mode
        if(string_to_int.find(symbol) == string_to_int.end()){
            // word not found, return OOV symbol's ID
            return string_to_int.size();
        }else{
            return string_to_int[symbol];
        }
    }
}

string Dictionary::getSymbol(int id, TVec<string>options)const
{
    if(int_to_string.find(id) == int_to_string.end())
    {
        if(id == ((int)string_to_int.size()))
            return oov_symbol;
        else
            return "";
    }
    else
        return int_to_string.find(id)->second;
}

int Dictionary::size(TVec<string> options){
    if(dont_insert_oov_symbol)
        return int(string_to_int.size());
    else
        return int(string_to_int.size())+1;
}

void Dictionary::getValues(TVec<string> options, Vec& values)
{ 
    values.resize(size());
    int i=0;
    for(map<int,string>::iterator it = int_to_string.begin(); it != int_to_string.end(); it++)
        values[i++] = it->first;
    if(!dont_insert_oov_symbol)
        values[i] = i; // OOV symbol's ID
}

bool Dictionary::isIn(string symbol, TVec<string> options){
    if(symbol == oov_symbol) return !dont_insert_oov_symbol;
    int last_update_mode = update_mode;
    update_mode = NO_UPDATE;
    int id = getId(symbol,options);
    update_mode = last_update_mode;
    return id != ((int)string_to_int.size());
}

bool Dictionary::isIn(int id, TVec<string> options) 
{ 
    return id < size() && id >= 0;
}

void Dictionary::clear()
{
    string_to_int.clear(); 
    int_to_string.clear();
}

void Dictionary::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(string_to_int, copies);
    deepCopyField(int_to_string, copies);
}

} // end of namespace PLearn


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
