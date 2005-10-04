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
 * $Id$ 
 ******************************************************* */

// Authors: Hugo Larochelle, Christopher Kermorvant

/*! \file WordSenseNetDictionary.cc */


#include "WordNetSenseDictionary.h"
#include "wn.h"
#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;

// Tool functions

char* cstr(string& str)
{
    char* cstr = new char[str.size() + 1];
    for (unsigned int i = 0; i < str.size(); i++)
        *(cstr + i) = str[i];
    cstr[str.size()] = '\0';
    return cstr;
}

TVec<string> extractSenses(string word, int wn_pos, string symbol_type)
{
    char* cword = cstr(word);
    SynsetPtr ssp = NULL;
    IndexPtr idx = getindex(cword, wn_pos);
    ssp = findtheinfo_ds(cword, wn_pos, -HYPERPTR, ALLSENSES);
    
    TVec<string> ret(0);

    if (ssp == NULL) return ret;
    
    int wnsn = 0;
    // extract all senses for a given word
    while (ssp != NULL)
    {
        char *charsk = WNSnsToStr(idx, ++wnsn);
        if(symbol_type == "sense_key") ret.push_back(string(charsk));
        else PLERROR("In extractSenses(): symbol_type %s not valid", symbol_type.c_str());
        ssp = ssp->nextss;
    }
    free_syns(ssp);
    delete cword;
    return ret;
}

string stemWord(string word)
{
    char* input_word = cstr(word);
    char* lemma = morphword(input_word, NOUN);
    if (lemma == NULL)
    {
        lemma = morphword(input_word, VERB);
        if (lemma == NULL)
        {
            lemma = morphword(input_word, ADJ);
            if (lemma == NULL)
            {
                lemma = morphword(input_word, ADV);
            }
        }
    }
 
    delete input_word;
    if (lemma == NULL) return word;
    else return string(lemma);
}

string stemWord(string word, int wn_pos)
{
    char* input_word = cstr(word);
    char* lemma = morphword(input_word, wn_pos);
    delete input_word;
    if (lemma == NULL)
        return word;
    else
        return string(lemma);
}

  
WordNetSenseDictionary::WordNetSenseDictionary()
    : options_stem_words(false), symbol_type("sense_key")
{}
  
PLEARN_IMPLEMENT_OBJECT(WordNetSenseDictionary,
                        "Dictionary of WordNet senses",
                        "The symbols in the instantiated dictionary are senses (not words!).\n");
  
void WordNetSenseDictionary::declareOptions(OptionList& ol)
{
    declareOption(ol, "options_stem_words", &WordNetSenseDictionary::options_stem_words, OptionBase::buildoption, "Indication that words given as options should be stemmed when looking at their possible senses");
    declareOption(ol, "options_to_lower_case", &WordNetSenseDictionary::options_to_lower_case, OptionBase::buildoption, "Indication that words given as options should be put to lower case");
    declareOption(ol, "symbol_type", &WordNetSenseDictionary::symbol_type, OptionBase::buildoption, "Type of representation (symbol) of the senses. The currently supported types are:\n"
        "   - \"sense_key\" (standard WordNet representation)");
    inherited::declareOptions(ol);
}
  
void WordNetSenseDictionary::build_()
{
    wninit();
}

// ### Nothing to add here, simply calls build_
void WordNetSenseDictionary::build()
{
    inherited::build();
    build_();
}

TVec<string> WordNetSenseDictionary::getSensesFromWordNet(TVec<string> options)
{
    string word = "";
    string stemmed_word = "";
    int wn_pos = -1;
    TVec<string> ret(0);

    // Do nothing if no options are specified
    if(options.length() == 0) return ret;

    if(options.length() == 1) 
    {
        word = removeblanks(options[0]);
        if(options_to_lower_case) word = lowerstring(word);
    }
        
    if(wn_pos < 0)
    {
        // Extract senses for all possible POS

        // NOUN
        ret.append(extractSenses(word,NOUN,symbol_type));
        if(options_stem_words) 
        {
            stemmed_word = stemWord(word,NOUN);
            if(word != stemmed_word)
                ret.append(extractSenses(stemmed_word,NOUN,symbol_type));
        }

        // VERB
        ret.append(extractSenses(word,VERB,symbol_type));
        if(options_stem_words) 
        {
            stemmed_word = stemWord(word,VERB);
            if(word != stemmed_word)
                ret.append(extractSenses(stemmed_word,VERB,symbol_type));
        }
       
        // ADJ
        ret.append(extractSenses(word,ADJ,symbol_type));
        if(options_stem_words) 
        {
            stemmed_word = stemWord(word,ADJ);
            if(word != stemmed_word)
                ret.append(extractSenses(stemmed_word,ADJ,symbol_type));
        }
       
        // ADV
        ret.append(extractSenses(word,ADV,symbol_type));
        if(options_stem_words) 
        {
            stemmed_word = stemWord(word,ADV);
            if(word != stemmed_word)
                ret.append(extractSenses(stemmed_word,ADV,symbol_type));
        }           
    }
    else
    {
        // Extract senses only for wn_pos
        ret.append(extractSenses(word,wn_pos,symbol_type));
        if(options_stem_words) 
        {
            stemmed_word = stemWord(word,wn_pos);
            if(word != stemmed_word)
                ret.append(extractSenses(stemmed_word,wn_pos,symbol_type));
        }  
    }

    return ret;
}

int WordNetSenseDictionary::getId(string symbol, TVec<string> options)
{
    if(update_mode == UPDATE && !isIn(symbol,options))
    {
        TVec<string> sret = getSensesFromWordNet(options);        
        for(int i=0; i<sret.length(); i++)
            getId(sret[i]);
        if(options.length() != 0 && sret.find(symbol) < 0)
            PLWARNING("In WordNetSenseDictionary::getId(): sense %s is not among possible symbols",symbol.c_str());
        
        return inherited::getId(symbol,options);
    }
    else return inherited::getId(symbol,options);    
}

Vec WordNetSenseDictionary::getValues(TVec<string> options)
{ 
    Vec ret;
    TVec<string> sret = getSensesFromWordNet(options);
    ret.resize(sret.length());
    for(int i=0; i<sret.length(); i++)
        ret[i] = getId(sret[i]);
    return ret;
}

int WordNetSenseDictionary::size(TVec<string> options){
    if(options.length() == 0)
        return inherited::size();
    else         
        return getSensesFromWordNet(options).length();            
}

void WordNetSenseDictionary::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
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
