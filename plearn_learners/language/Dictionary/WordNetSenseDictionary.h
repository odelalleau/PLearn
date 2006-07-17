// -*- C++ -*-

// WordNetSenseDictionary.h
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

/*! \file WordNetSenseDictionary.h */


#ifndef WordNetSenseDictionary_INC
#define WordNetSenseDictionary_INC
#include "Dictionary.h"
#include "wn.h"

#define WN_ROOT_NODE "wn_root_node"
#define WN_NOUN_NODE "wn_noun_node"
#define WN_VERB_NODE "wn_verb_node"
#define WN_ADJ_NODE "wn_adj_node"
#define WN_ADV_NODE "wn_adv_node"

namespace PLearn {
using namespace std;

// Tool functions

//! Takes a string and returns a char array of that string.
//! Helps, when a char array representation that doesn't
//! need to be constant (const) is needed.
char* cstr(string& str);
//! Returns a synset key from a SynsetPtr
string getSynsetKey(SynsetPtr ssp);
//! Returns a SynsetPtr from a synset key
SynsetPtr getSynsetPtr(string synset_key);
//! Extract senses for a word and a certain POS tag, as a certain
//! symbol type. Appends the extracted senses to the TVec senses
void extractSenses(string word, int wn_pos, string symbol_type, TVec<string>& senses,TVec<int>& tagcnts, TVec< TVec<string> >& ancestors, bool extract_ancestors=false);
//! Extracts synset
void extractAncestors(TVec<SynsetPtr> anc, TVec< TVec<string> >& anc_str, string root_node=WN_ROOT_NODE);
//! Stems a word
string stemWord(string word);
//! Stems a word, according to a POS
string stemWord(string word, int wn_pos);
//! Lists the possible stemmed variation of a word
void stemsOfWord(string word, int wn_pos, TVec<string>& stems);
//! Lists the possible stemmed variation of a word using its POS
void stemsOfWord(string word, TVec<string>& stems);

/*! This class implements a Dictionary for WordNet senses.
  The symbols in the instantiated dictionary are senses (not words!).
*/

class WordNetSenseDictionary: public Dictionary
{

private:
  
    typedef Dictionary inherited;

protected:

public:

    // ************************
    // * public build options *
    // ************************

    //! Stem word before including in dictionary STEM/NO_STEM (ontology only)
    bool options_stem_words;
    
    //! Put words to lower case
    bool options_to_lower_case;

    //! Type of representation (symbol) of the senses
    string symbol_type;

    //! Possible values for a certain word
    hash_map<string,Vec > possible_values_for_word;

    // Hierarchy information:

    //! Indication that the WordNet hierarchy will be used
    bool use_wordnet_hierarchy;

    //! Synset children mapping
    hash_map<int, TVec<int> > children;

    //! Synset parents mapping
    hash_map<int, TVec<int> > parents;

    //! Prior distribution p(sense|word) given by WordNet
    hash_map<string, real> sense_prior;

protected:

    //! Stems of words (temporary computation field)
    TVec<string> stems;
    
    //! Senses of words (temporary computation field)
    TVec<string> senses;

    //! Senses of words (temporary computation field)
    TVec< TVec<string> > ancestors_vec;

    //! Tag counts for senses (temporary computation field)
    TVec<int> tagcnts;

    //! Temporary variable when returning an int;
    int ret;

    //! Temporary variable for sense_prior computation
    string word_sense;


    // ******************
    // * Object methods *
    // ******************

private: 
    //! This does the actual building. 
    void build_();

protected: 
    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

    void getSensesFromWordNet(TVec<string> options);

public:

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    WordNetSenseDictionary();

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(WordNetSenseDictionary);

    virtual int getId(string symbol, TVec<string> options = TVec<string>(0));

    virtual Vec getValues(TVec<string> options=TVec<string>(0));
    
    virtual int size(TVec<string> options=TVec<string>(0));

    //! Returns the parents of a sense (synset)
    //! Will give an empty vector is sense id incorrect
    //! or use_wordnet_hierarchy is false.
    virtual void parentsOf(int sense, TVec<int>& the_parents);

    //! Returns the children of a sense (synset)
    //! Will give an empty vector is sense id incorrect
    //! or use_wordnet_hierarchy is false.
    virtual void childrenOf(int sense, TVec<int>& the_children);

    //! Return the id of WN_ROOT_NODE
    virtual int rootNode();

    //! Prior distribution over senses of a word, given by WordNet
    virtual real sensePrior(string word, string sense);

    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(WordNetSenseDictionary);
  
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
