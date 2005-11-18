// -*- C++ -*-

// WordNetSenseDictionary.cc
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

/*! \file WordNetSenseDictionary.cc */


#include "WordNetSenseDictionary.h"
#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;

char* cstr(string& str)
{
    char* cstr = new char[str.size() + 1];
    for (unsigned int i = 0; i < str.size(); i++)
        *(cstr + i) = str[i];
    cstr[str.size()] = '\0';
    return cstr;
}

//! Returns a synset key from a SynsetPtr
string getSynsetKey(SynsetPtr ssp)
{
    if(!ssp) PLERROR("In getSynsetKey(): SynsetPtr is NULL");
    string ssk = tostring(ssp->words[0]);
    for(int i=1; i<ssp->wcount && i<3; i++)
        ssk = ssk + "," + tostring(ssp->words[i]);
    ssk = ssk + ":" + tostring(ssp->pos) + "#" + tostring(ssp->hereiam);
    return ssk;
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

        if(symbol_type == "sense_key")
        {
            char *charsk = WNSnsToStr(idx, ++wnsn);
            ret.push_back(string(charsk));
        }
        else if(symbol_type == "synset_key")
        {
            ret.push_back(getSynsetKey(ssp));
        }
        else PLERROR("In extractSenses(): symbol_type %s not valid", symbol_type.c_str());
        ssp = ssp->nextss;
    }
    free_syns(ssp);
    free_index(idx);
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
    else return removeblanks(tostring(lemma));
}

string stemWord(string word, int wn_pos)
{
    char* input_word = cstr(word);
    char* lemma = morphword(input_word, wn_pos);
    delete input_word;
    if (lemma == NULL)
        return word;
    else
        return removeblanks(tostring(lemma));
}

TVec<string> stemsOfWord(string word, int wn_pos)
{
    TVec<string> stems;
    stems.push_back(word);
    char* input_word = cstr(word);
    char* lemma = morphstr(input_word, wn_pos);
    string lemma_str;
    while(lemma)
    {
        lemma_str = removeblanks(tostring(lemma));
        if(lemma_str != word)
            stems.push_back(lemma_str);
        lemma = morphstr(NULL, wn_pos);
    }
    delete input_word;
    return stems;
}

TVec<string> stemsOfWord(string word)
{
    TVec<string> stems;
    stems.push_back(word);
    char* input_word = cstr(word);

    char* lemma = morphstr(input_word, NOUN);
    string lemma_str;
    while(lemma)
    {
        lemma_str = removeblanks(tostring(lemma));
        if(lemma_str != word)
            stems.push_back(lemma_str);
        lemma = morphstr(NULL, NOUN);
    }

    lemma = morphstr(input_word, VERB);
    while(lemma)
    {
        lemma_str = removeblanks(tostring(lemma));
        if(lemma_str != word)
            stems.push_back(lemma_str);
        lemma = morphstr(NULL, VERB);
    }

    lemma = morphstr(input_word, ADJ);
    while(lemma)
    {
        lemma_str = removeblanks(tostring(lemma));
        if(lemma_str != word)
            stems.push_back(lemma_str);
        lemma = morphstr(NULL, ADJ);
    }

    lemma = morphstr(input_word, ADV);
    while(lemma)
    {
        lemma_str = removeblanks(tostring(lemma));
        if(lemma_str != word)
            stems.push_back(lemma_str);
        lemma = morphstr(NULL, ADV);
    }

    delete input_word;
    return stems;
}


WordNetSenseDictionary::WordNetSenseDictionary()    
    : options_stem_words(false), options_to_lower_case(false), symbol_type("sense_key")
{
    if(wninit()<0)
        PLERROR("In WordNetSenseDictionary(): could not open WordNet database files");
}
  
PLEARN_IMPLEMENT_OBJECT(WordNetSenseDictionary,
                        "Dictionary of WordNet senses",
                        "The symbols in the instantiated dictionary are senses (not words!).\n");
  
void WordNetSenseDictionary::declareOptions(OptionList& ol)
{
    declareOption(ol, "options_stem_words", &WordNetSenseDictionary::options_stem_words, OptionBase::buildoption, "Indication that words given as options should be stemmed when looking at their possible senses");
    declareOption(ol, "options_to_lower_case", &WordNetSenseDictionary::options_to_lower_case, OptionBase::buildoption, "Indication that words given as options should be put to lower case");
    declareOption(ol, "symbol_type", &WordNetSenseDictionary::symbol_type, OptionBase::buildoption, "Type of representation (symbol) of the senses. The currently supported types are:\n"
        "   - \"sense_key\" (standard WordNet representation)\n"
        "   - \"synset_key\" (custom made representation, that consists of maximum three words from the synset, the POS category and the byte offset of the synset)\n"
        "Note that the Dictionary will always accept \"sense_key\" symbols as input, but will convert it automatically to\n"
        "the symbol_type representation and treat it as such.");
    inherited::declareOptions(ol);
}
  
void WordNetSenseDictionary::build_()
{}

// ### Nothing to add here, simply calls build_
void WordNetSenseDictionary::build()
{
    inherited::build();
    build_();
}

TVec<string> WordNetSenseDictionary::getSensesFromWordNet(TVec<string> options)
{
    string word = "";
    TVec<string> stemmed_words;
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
            TVec<string> stemmed_words = stemsOfWord(word,NOUN);
            for(int i=0; i<stemmed_words.length(); i++)
                if(word != stemmed_words[i])
                    ret.append(extractSenses(stemmed_words[i],NOUN,symbol_type));
        }

        // VERB
        ret.append(extractSenses(word,VERB,symbol_type));
        if(options_stem_words) 
        {
            TVec<string> stemmed_words = stemsOfWord(word,VERB);
            for(int i=0; i<stemmed_words.length(); i++)
                if(word != stemmed_words[i])
                    ret.append(extractSenses(stemmed_words[i],VERB,symbol_type));
        }
       
        // ADJ
        ret.append(extractSenses(word,ADJ,symbol_type));
        if(options_stem_words) 
        {
            TVec<string> stemmed_words = stemsOfWord(word,ADJ);
            for(int i=0; i<stemmed_words.length(); i++)
                if(word != stemmed_words[i])
                    ret.append(extractSenses(stemmed_words[i],ADJ,symbol_type));
        }
       
        // ADV
        ret.append(extractSenses(word,ADV,symbol_type));
        if(options_stem_words) 
        {
            TVec<string> stemmed_words = stemsOfWord(word,ADV);
            for(int i=0; i<stemmed_words.length(); i++)
                if(word != stemmed_words[i])
                    ret.append(extractSenses(stemmed_words[i],ADV,symbol_type));
        }           
    }
    else
    {
        // Extract senses only for wn_pos
        ret.append(extractSenses(word,wn_pos,symbol_type));
        if(options_stem_words) 
        {
            TVec<string> stemmed_words = stemsOfWord(word,wn_pos);
            for(int i=0; i<stemmed_words.length(); i++)
                if(word != stemmed_words[i])
                    ret.append(extractSenses(stemmed_words[i],wn_pos,symbol_type));
        }  
    }

    return ret;
}

int WordNetSenseDictionary::getId(string symbol, TVec<string> options)
{
    // This permits converting an input symbol as sense_key
    // to some other symbol_type...
    if(strchr(symbol.c_str(),'%') && symbol_type != "sense_key")
    {
        if(symbol_type == "synset_key")
        {
            char* symbol_cstr = cstr(symbol);
            symbol = getSynsetKey(GetSynsetForSense(symbol_cstr));
            delete symbol_cstr;
        }
        else PLERROR("In getId(): symbol_type %s not valid", symbol_type.c_str());
    }
         
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
