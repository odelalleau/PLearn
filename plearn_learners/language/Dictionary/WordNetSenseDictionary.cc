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

//! Returns a SynsetPtr from a synset key
SynsetPtr getSynsetPtr(string synset_key)
{
    vector<string> splits = split(synset_key,",:#");
    char* srch_str = cstr(splits[0]);
    string pos = tostring(splits[splits.size()-2]);
    int pos_int = getpos(&pos[0]);
    long hereiam = tolong(splits[splits.size()-1]);
    SynsetPtr ret = read_synset(pos_int,hereiam,srch_str);
    delete(srch_str);
    return ret;
}

void extractSenses(string word, int wn_pos, string symbol_type, TVec<string>& senses,TVec<int>& tagcnts, TVec< TVec<string> >& ancestors, bool extract_ancestors)
{
    char* cword = cstr(word);
    SynsetPtr ssp = NULL;
    IndexPtr idx = getindex(cword, wn_pos);
    ssp = findtheinfo_ds(cword, wn_pos, -HYPERPTR, ALLSENSES);
    
    if (ssp == NULL) 
    {
        if(idx) free_index(idx);
        delete cword;
        return;
    }
    
    int wnsn = 0;
    SynsetPtr head = ssp;
    // extract all senses for a given word
    while (ssp != NULL)
    {
        ++wnsn;
        if(symbol_type == "sense_key")
        {
            char *charsk = WNSnsToStr(idx, wnsn);
            senses.push_back(string(charsk));
            delete charsk;
        }
        else if(symbol_type == "synset_key")
        {
            senses.push_back(getSynsetKey(ssp));
        }
        else PLERROR("In extractSenses(): symbol_type %s not valid", symbol_type.c_str());
        if(extract_ancestors)
        {
            TVec< TVec<string> > this_anc_str;
            TVec< SynsetPtr > beg_anc(1);
            beg_anc[0] = ssp;
            if(wn_pos == NOUN)
                extractAncestors(beg_anc,this_anc_str, WN_NOUN_NODE);
            else if(wn_pos == VERB)
                extractAncestors(beg_anc,this_anc_str, WN_VERB_NODE);
            else if(wn_pos == ADJ)
                extractAncestors(beg_anc,this_anc_str, WN_ADJ_NODE);
            else if(wn_pos == ADV)
                extractAncestors(beg_anc,this_anc_str, WN_ADV_NODE);
            else
                PLERROR("In extractSenses(): wn_pos %d invalid", wn_pos);
            ancestors.append(this_anc_str);
        }
        tagcnts.push_back(GetTagcnt(idx,wnsn));
        ssp = ssp->nextss;
    }
    free_syns(head);
    free_index(idx);
    delete cword;
}

void extractAncestors(TVec<SynsetPtr> anc, TVec< TVec<string> >& anc_str, string root_node)
{
    SynsetPtr a;
    SynsetPtr extra_a;
    int i=0;
    anc_str.resize(anc.length());
    for(int k=0; k<anc_str.length(); k++)
        anc_str[k].resize(0);

    while(i<anc.length())
    {
        // Add sense
        a = anc[i];
        anc_str[i].push_back(getSynsetKey(a));
        // Go through one level and start looking for paths
        a = a->ptrlist; 
        while(a != NULL)
        {
            extra_a = a->nextss;
            while(extra_a != NULL)
            {
                anc.push_back(extra_a);
                anc_str.push_back(anc_str[i].copy());
                extra_a = extra_a->nextss;
            }
            anc_str[i].push_back(getSynsetKey(a));
            a = a->ptrlist; 
        }
        anc_str[i].push_back(root_node);
        i++;
    }
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

void stemsOfWord(string word, int wn_pos, TVec<string>& stems)
{
    stems.resize(0);
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
}

void stemsOfWord(string word, TVec<string>& stems)
{
    stems.resize(0);
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
}


WordNetSenseDictionary::WordNetSenseDictionary()    
    : options_stem_words(false), options_to_lower_case(false), symbol_type("sense_key"), use_wordnet_hierarchy(false)
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
    declareOption(ol, "possible_values_for_word", &WordNetSenseDictionary::possible_values_for_word, OptionBase::learntoption, "Mapping of the possible id values for a given word");
    declareOption(ol, "use_wordnet_hierarchy", &WordNetSenseDictionary::use_wordnet_hierarchy, OptionBase::buildoption, "Indication that the WordNet hierarchy will be used");
    declareOption(ol, "children", &WordNetSenseDictionary::children, OptionBase::learntoption, "Synset children mapping");
    declareOption(ol, "parents", &WordNetSenseDictionary::parents, OptionBase::learntoption, "Synset parents mapping");
    declareOption(ol, "sense_prior", &WordNetSenseDictionary::sense_prior, OptionBase::learntoption, "Prior distribution for p(sense|word) given by WordNet");

    inherited::declareOptions(ol);
}
  
void WordNetSenseDictionary::build_()
{
    if(use_wordnet_hierarchy && symbol_type == "sense_key")
        PLERROR("In WordNetSenseDictionary::build_(): cannot use WordNet hierarchy with symbol type \"sense_key\"");
}

// ### Nothing to add here, simply calls build_
void WordNetSenseDictionary::build()
{
    inherited::build();
    build_();
}

void WordNetSenseDictionary::getSensesFromWordNet(TVec<string> options)
{
    string word = "";
    stems.resize(0);
    int wn_pos = -1;
    senses.resize(0);
    ancestors_vec.resize(0);
    tagcnts.resize(0);

    word = removeblanks(options[0]);
    if(options_to_lower_case) word = lowerstring(word);    
    
    if(wn_pos < 0)
    {
        // Extract senses for all possible POS

        // NOUN
        extractSenses(word,NOUN,symbol_type,senses, tagcnts, ancestors_vec, use_wordnet_hierarchy);
        if(options_stem_words) 
        {
            stemsOfWord(word,NOUN,stems);
            for(int i=0; i<stems.length(); i++)
                if(word != stems[i])
                    extractSenses(stems[i],NOUN,symbol_type,senses, tagcnts, ancestors_vec, use_wordnet_hierarchy);
        }

        // VERB
        extractSenses(word,VERB,symbol_type,senses, tagcnts, ancestors_vec, use_wordnet_hierarchy);
        if(options_stem_words) 
        {
            stemsOfWord(word,VERB,stems);
            for(int i=0; i<stems.length(); i++)
                if(word != stems[i])
                    extractSenses(stems[i],VERB,symbol_type,senses, tagcnts, ancestors_vec, use_wordnet_hierarchy);
        }
       
        // ADJ
        extractSenses(word,ADJ,symbol_type,senses, tagcnts, ancestors_vec, use_wordnet_hierarchy);
        if(options_stem_words) 
        {
            stemsOfWord(word,ADJ,stems);
            for(int i=0; i<stems.length(); i++)
                if(word != stems[i])
                    extractSenses(stems[i],ADJ,symbol_type,senses, tagcnts, ancestors_vec, use_wordnet_hierarchy);
        }
       
        // ADV
        extractSenses(word,ADV,symbol_type,senses, tagcnts, ancestors_vec, use_wordnet_hierarchy);
        if(options_stem_words) 
        {
            stemsOfWord(word,ADV,stems);
            for(int i=0; i<stems.length(); i++)
                if(word != stems[i])
                    extractSenses(stems[i],ADV,symbol_type,senses, tagcnts, ancestors_vec, use_wordnet_hierarchy);
        }

        if(tagcnts.length()>0)
        {
            word_sense = word + " " + senses[0];
            if(sense_prior.find(word_sense) == sense_prior.end())
            {
                int sum = 0;
                for(int i=0; i<tagcnts.length(); i++)
                    sum += tagcnts[i];
                for(int i=0; i<tagcnts.length();i++)
                {
                    word_sense = word + " " + senses[i];
                    sense_prior[word_sense] = ((real)tagcnts[i])/sum;
                }
            }
        }
    }
    else
    {
        // Extract senses only for wn_pos
        extractSenses(word,wn_pos,symbol_type,senses, tagcnts, ancestors_vec, use_wordnet_hierarchy);
        if(options_stem_words) 
        {
            stemsOfWord(word,wn_pos,stems);
            for(int i=0; i<stems.length(); i++)
                if(word != stems[i])
                    extractSenses(stems[i],wn_pos,symbol_type,senses, tagcnts, ancestors_vec, use_wordnet_hierarchy);
        }  

        PLERROR("In  WordNetSenseDictionary::getSensesFromWordNet(): producing sense_prior not implemented yet");
    }

    if(use_wordnet_hierarchy)
    {
        int n1,n2;
        for(int i=0; i<ancestors_vec.length(); i++)
        {            
            for(int j=0; j<ancestors_vec[i].length(); j++)
            {
                if(j != ancestors_vec[i].length()-1)
                {
                    n1 = inherited::getId(ancestors_vec[i][j]);
                    n2 = inherited::getId(ancestors_vec[i][j+1]);
                }
                else
                {
                    n1 = inherited::getId(ancestors_vec[i].last());
                    n2 = inherited::getId(WN_ROOT_NODE);
                }

                if(n1 == oov_tag_id || n2 == oov_tag_id)
                    continue;
                
                if(parents.find(n1) == parents.end())
                {
                    TVec<int> p(1);
                    p[0] = n2;
                    parents[n1] = p.copy();
                }
                else
                {
                    if(parents[n1].find(n2)>=0)
                        parents[n1].push_back(n2);
                }
                if(children.find(n2) == children.end())
                {
                    TVec<int> c(1);
                    c[0] = n1;
                    children[n2] = c.copy();
                }
                else
                {
                    if(children[n2].find(n1)>=0)
                        children[n2].push_back(n1);
                }
            }                        
        }
    }
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

    ret = inherited::getId(symbol,options);

    // call getValues, which fills possible_values and gets the ids for all those senses, to do a compatibility check, if necessary
    if(update_mode == UPDATE && options.length() != 0)
    {
        getValues(options);
        //getSensesFromWordNet(options); 
        if(possible_values.find(ret) < 0)
            PLWARNING("In WordNetSenseDictionary::getId(): sense %s is not among possible symbols",symbol.c_str());
    }

    /*
    // Extracting new senses. If dictionary can be updated and 
    // if symbol_type == "synset_key", then need to be sure that all 
    // synsets were inserted, even if the current synset is !!!
    if(update_mode == UPDATE && (symbol_type == "synset_key" || !isIn(symbol,options)) )
        for(int i=0; i<senses.length(); i++)
            inherited::getId(senses[i],options);         
    */

    return ret;
}

Vec WordNetSenseDictionary::getValues(TVec<string> options)
{ 
    if(options.length() == 0)
        return inherited::getValues();
    else
    {
        refill_possible_values = 1;        
        if(options.length() > 1) PLERROR("In WordNetSenseDictionary::getSensesFromWordNet(): options.length()>1 not supported");
        if(possible_values_for_word.find(options[0]) == possible_values_for_word.end())
        {
            getSensesFromWordNet(options);
            possible_values.resize(senses.length());
            for(int i=0; i<senses.length(); i++)
                possible_values[i] = inherited::getId(senses[i]);
            possible_values_for_word[options[0]] = possible_values.copy();
            return possible_values;        
        }
        else
        {
            possible_values.resize(possible_values_for_word[options[0]].length());
            possible_values << possible_values_for_word[options[0]];
            return possible_values;        
        }
    }
}

int WordNetSenseDictionary::size(TVec<string> options){
    if(options.length() == 0)
        return inherited::size();
    else       
    {
        return getValues().length();
    }
}

void WordNetSenseDictionary::parentsOf(int sense, TVec<int>& the_parents){
    if(parents.find(sense) == parents.end())
        the_parents.resize(0);
    else
    {
        the_parents.resize(parents[sense].length());
        the_parents << parents[sense];
    }
}

void WordNetSenseDictionary::childrenOf(int sense, TVec<int>& the_children){
    if(children.find(sense) == children.end())
        the_children.resize(0);
    else
    {
        the_children.resize(children[sense].length());
        the_children << children[sense];
    }
}

int WordNetSenseDictionary::rootNode(){
    return inherited::getId(WN_ROOT_NODE);
}

real WordNetSenseDictionary::sensePrior(string word, string sense){
    word_sense = word + " " + sense;
    if(sense_prior.find(word_sense) == sense_prior.end())
        return 0;
    else
        return sense_prior[word_sense];
}

void WordNetSenseDictionary::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(stems, copies);
    deepCopyField(senses, copies);
    deepCopyField(ancestors_vec, copies);
    deepCopyField(tagcnts, copies);
    deepCopyField(possible_values_for_word, copies);
    deepCopyField(children, copies);
    deepCopyField(parents, copies);
    deepCopyField(sense_prior, copies);
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
