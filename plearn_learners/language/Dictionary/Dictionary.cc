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
    refill_possible_values(true), update_mode(UPDATE), oov_not_in_possible_values(false)
{
}

PLEARN_IMPLEMENT_OBJECT(Dictionary,
                        "Mapping string->int and int->string",
                        "A dictionary is a mapping between a string and and index (int).\n"
                        "Depending on the update mode, the dictionay can include an unknown \n"
                        "word when asking for its Id with getId();\n"
                        "if update_mode == UPDATE, add the word and return its Id\n"
                        "if  update_mode == NO_UPDATE, return -1\n"
                        "\n"
                        "Also, when asking for the symbol associated to an Id, no update is possible.\n"
                        "If the id is not in the dictionary, than the symbol is ""\n"
                        "\n"                     
                        "An object from Dictionary is instantiated empty, and then symbols can be added in the\n"
                        "dictionary by using getId() (with update_mode == UPDATE). Subclasses will normaly \n"
                        "permit more sophisticated instantiation.\n");

void Dictionary::declareOptions(OptionList& ol)
{
    declareOption(ol, "update_mode", &Dictionary::update_mode, OptionBase::buildoption, "update_mode : 0(no_update)/1(update). Default is update");
    declareOption(ol, "string_to_int", &Dictionary::string_to_int, OptionBase::learntoption, "string to int mapping");
    declareOption(ol, "int_to_string", &Dictionary::int_to_string, OptionBase::learntoption, "int to string mapping");
    declareOption(ol, "oov_tag_id", &Dictionary::oov_tag_id, OptionBase::learntoption, "id of the OOV_TAG");
    declareOption(ol, "oov_not_in_possible_values", &Dictionary::oov_not_in_possible_values, OptionBase::learntoption, "Indication that \"oov\" should not be part of the possible values");
    inherited::declareOptions(ol);
}

void Dictionary::build_(){
    int last_update_mode = update_mode;
    update_mode = UPDATE;
    // the dictionary must contain oov
    oov_tag_id = getId(OOV_TAG);
    update_mode = last_update_mode;
}

// ### Nothing to add here, simply calls build_
void Dictionary::build()
{
    inherited::build();
    build_();
}

void  Dictionary::setUpdateMode(int up_mode)
{
    update_mode =up_mode;
}

int Dictionary::getId(string symbol, TVec<string> options)
{
    // Gives the id of a symbol in the dictionary
    // If the symbol is not in the dictionary, 
    // returns index of OOV_TAG if update_mode = NO_UPDATE,
    // insert the new word otherwise and return its index
    int index;
    if(update_mode== UPDATE)
    {
        if(string_to_int.find(symbol) == string_to_int.end()){
            // word not found, add it
            index=int(string_to_int.size());
            string_to_int[symbol] = index;
            int_to_string[index] = symbol;
            refill_possible_values = true;
        }
        return string_to_int[symbol];
    }
    else
    {
        // NO update mode
        if(string_to_int.find(symbol) == string_to_int.end()){
            // word not found, return oov
            return string_to_int[OOV_TAG];
        }else{
            return string_to_int[symbol];
        }
    }
}

string Dictionary::getSymbol(int id, TVec<string>options)const
{
    if(int_to_string.find(id) == int_to_string.end())
        return OOV_TAG;
    else
        return int_to_string.find(id)->second;
}

int Dictionary::size(TVec<string> options){
    if(string_to_int.size() < 1)
        PLERROR("In Dictionary::size(): string mapping should at least contain the OOV_TAG (did you forget to call build()?)");
    // By definition, OOV_TAG is not in the Dictionary (but of course
    // must be in the string mapping...
    return int(string_to_int.size())-1;
}

Vec Dictionary::getValues(TVec<string> options)
{ 
    if(refill_possible_values || last_oov_not_in_possible_values != oov_not_in_possible_values)
    {
        last_oov_not_in_possible_values = oov_not_in_possible_values;
        possible_values.resize((int) string_to_int.size() - (oov_not_in_possible_values ? 1 : 0));
        int i=0;
        for(map<string,int>::iterator it = string_to_int.begin(); it != string_to_int.end(); it++)
            if(!oov_not_in_possible_values || oov_tag_id != it->second)
                possible_values[i++] = it->second;
        refill_possible_values = false;
    }
    return possible_values;
}

bool Dictionary::isIn(string symbol, TVec<string> options){
    int last_update_mode = update_mode;
    update_mode = NO_UPDATE;
    int id = getId(symbol,options);
    update_mode = last_update_mode;
    return id != oov_tag_id;
}

void Dictionary::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(string_to_int, copies);
    deepCopyField(int_to_string, copies);
    deepCopyField(possible_values, copies);
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
