// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
//
// This file is part of the PLearn Library. This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation, version 2.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this library; see the file GPL.txt  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// As a special exception, you may compile and link this library with files
// not covered by the GNU General Public License, and distribute the resulting
// executable file under the terms of your choice, without the requirement to
// distribute the complete corresponding source code, provided you have
// obtained explicit written permission to do so from Pascal Vincent (primary
// author of the library) or Yoshua Bengio or the University of Montreal.
// This exception does not however invalidate any other reasons why the
// executable file might be covered by the GNU General Public License.
//
// See the following URL for more information on PLearn:
// http://plearn.sourceforge.net 

 

/* *******************************************************      
   * $Id: WordNetOntology.cc,v 1.10 2002/12/10 06:35:25 jauvinc Exp $
   * AUTHORS: Christian Jauvin
   * This file is part of the PLearn library.
   ******************************************************* */

#include "WordNetOntology.h"

namespace PLearn {
  using namespace std;

WordNetOntology::WordNetOntology()
{
  init();
  createBaseSynsets();
}

WordNetOntology::WordNetOntology(string voc_file,
                                 bool differentiate_unknown_words,
                                 bool pre_compute_ancestors,
                                 bool pre_compute_descendants,
                                 int wn_pos_type,
                                 int word_coverage_threshold)
{
  init(differentiate_unknown_words);
  createBaseSynsets();
  extract(voc_file, wn_pos_type);
  if (pre_compute_descendants)
    extractDescendants();
  if (pre_compute_ancestors)
    extractAncestors(word_coverage_threshold);
}

WordNetOntology::WordNetOntology(string voc_file,
                                 string synset_file,
                                 string ontology_file,
                                 bool pre_compute_ancestors,
                                 bool pre_compute_descendants,
                                 int word_coverage_threshold)
{
  init();
  //createBaseSynsets();
  load(voc_file, synset_file, ontology_file);
  if (pre_compute_descendants)
    extractDescendants();
  if (pre_compute_ancestors)
    extractAncestors(word_coverage_threshold);
}

void WordNetOntology::init(bool the_differentiate_unknown_words)
{
  if (wninit() != 0)
    PLERROR("WordNet init error");

  noun_count = 0;
  verb_count = 0;
  adj_count = 0;
  adv_count = 0;

  synset_index = EOS_SS_ID + 1; // first synset id
  word_index = 0;
  unknown_sense_index = 0;

  noun_sense_count = 0;
  verb_sense_count = 0;
  adj_sense_count = 0;
  adv_sense_count = 0;
  
  in_wn_word_count = 0;
  out_of_wn_word_count = 0;

  are_ancestors_extracted = false;
  are_descendants_extracted = false;
  are_predominent_pos_extracted = false;
  are_word_high_level_senses_extracted = false;

  n_word_high_level_senses = 0;

  differentiate_unknown_words = the_differentiate_unknown_words;
}

void  WordNetOntology::createBaseSynsets()
{
  // create ROOT synset
  Node* root_node = new Node(ROOT_SS_ID);
  root_node->syns.push_back("ROOT");
  root_node->types.insert(UNDEFINED_TYPE);
  root_node->gloss = "(root concept)";
  synsets[ROOT_SS_ID] = root_node;
  //root_node->visited = true;

  // create SUPER-UNKNOWN synset
  Node* unk_node = new Node(SUPER_UNKNOWN_SS_ID);
  unk_node->syns.push_back("SUPER_UNKNOWN");
  unk_node->types.insert(UNDEFINED_TYPE);
  unk_node->gloss = "(super-unknown concept)";
  synsets[SUPER_UNKNOWN_SS_ID] = unk_node;
  //unk_node->visited = true;

  // link it <-> ROOT
  unk_node->parents.insert(ROOT_SS_ID);
  root_node->children.insert(SUPER_UNKNOWN_SS_ID);
  
  // create OOV (out-of-vocabulary) synset
  Node* oov_node = new Node(OOV_SS_ID);
  oov_node->syns.push_back("OOV");
  oov_node->types.insert(UNDEFINED_TYPE);
  oov_node->gloss = "(out-of-vocabulary)";
  synsets[OOV_SS_ID] = oov_node;
  //oov_node->visited = true;

  // link it <-> SUPER-UNKNOWN
  oov_node->parents.insert(SUPER_UNKNOWN_SS_ID);
  unk_node->children.insert(OOV_SS_ID);

  // create PROPER_NOUN, NUMERIC, PUNCTUATION, BOS, EOS and STOP synsets
  Node* proper_node = new Node(PROPER_NOUN_SS_ID);
  proper_node->syns.push_back("PROPER NOUN");
  proper_node->types.insert(UNDEFINED_TYPE);
  proper_node->gloss = "(proper noun)";
  synsets[PROPER_NOUN_SS_ID] = proper_node;
  //proper_node->visited = true;

  Node* num_node = new Node(NUMERIC_SS_ID);
  num_node->syns.push_back("NUMERIC");
  num_node->types.insert(UNDEFINED_TYPE);
  num_node->gloss = "(numeric)";
  synsets[NUMERIC_SS_ID] = num_node;
  //num_node->visited = true;

  Node* punct_node = new Node(PUNCTUATION_SS_ID);
  punct_node->syns.push_back("PUNCTUATION");
  punct_node->types.insert(UNDEFINED_TYPE);
  punct_node->gloss = "(punctuation)";
  synsets[PUNCTUATION_SS_ID] = punct_node;
  //punct_node->visited = true;

  Node* stop_node = new Node(STOP_SS_ID);
  stop_node->syns.push_back("STOP");
  stop_node->types.insert(UNDEFINED_TYPE);
  stop_node->gloss = "(stop)";
  synsets[STOP_SS_ID] = stop_node;

  Node* bos_node = new Node(BOS_SS_ID);
  bos_node->syns.push_back("BOS");
  bos_node->types.insert(UNDEFINED_TYPE);
  bos_node->gloss = "(BOS)";
  synsets[BOS_SS_ID] = bos_node;

  Node* eos_node = new Node(EOS_SS_ID);
  eos_node->syns.push_back("EOS");
  eos_node->types.insert(UNDEFINED_TYPE);
  eos_node->gloss = "(eos)";
  synsets[EOS_SS_ID] = eos_node;

  // link them <-> SUPER-UNKNOWN
  proper_node->parents.insert(SUPER_UNKNOWN_SS_ID);
  unk_node->children.insert(PROPER_NOUN_SS_ID);
  num_node->parents.insert(SUPER_UNKNOWN_SS_ID);
  unk_node->children.insert(NUMERIC_SS_ID);
  punct_node->parents.insert(SUPER_UNKNOWN_SS_ID);
  unk_node->children.insert(PUNCTUATION_SS_ID);
  stop_node->parents.insert(SUPER_UNKNOWN_SS_ID);
  unk_node->children.insert(STOP_SS_ID);
  bos_node->parents.insert(SUPER_UNKNOWN_SS_ID);
  unk_node->children.insert(BOS_SS_ID);
  eos_node->parents.insert(SUPER_UNKNOWN_SS_ID);
  unk_node->children.insert(EOS_SS_ID);

  // create NOUN, VERB, ADJECTIVE and ADVERB synsets
  Node* noun_node = new Node(NOUN_SS_ID);
  noun_node->syns.push_back("NOUN");
  noun_node->types.insert(UNDEFINED_TYPE);
  noun_node->gloss = "(noun concept)";
  synsets[NOUN_SS_ID] = noun_node;
  //noun_node->visited = true;

  Node* verb_node = new Node(VERB_SS_ID);
  verb_node->syns.push_back("VERB");
  verb_node->types.insert(UNDEFINED_TYPE);
  verb_node->gloss = "(verb concept)";
  synsets[VERB_SS_ID] = verb_node;
  //verb_node->visited = true;

  Node* adj_node = new Node(ADJ_SS_ID);
  adj_node->syns.push_back("ADJECTIVE");
  adj_node->types.insert(UNDEFINED_TYPE);
  adj_node->gloss = "(adjective concept)";
  synsets[ADJ_SS_ID] = adj_node;
  //adj_node->visited = true;
  
  Node* adv_node = new Node(ADV_SS_ID);
  adv_node->syns.push_back("ADVERB");
  adv_node->types.insert(UNDEFINED_TYPE);
  adv_node->gloss = "(adverb concept)";
  synsets[ADV_SS_ID] = adv_node;
  //adv_node->visited = true;
  
  // link them <-> ROOT
  noun_node->parents.insert(ROOT_SS_ID);
  root_node->children.insert(NOUN_SS_ID);
  verb_node->parents.insert(ROOT_SS_ID);
  root_node->children.insert(VERB_SS_ID);
  adj_node->parents.insert(ROOT_SS_ID);
  root_node->children.insert(ADJ_SS_ID);
  adv_node->parents.insert(ROOT_SS_ID);
  root_node->children.insert(ADV_SS_ID);

}

void WordNetOntology::extract(string voc_file, int wn_pos_type)
{
  int n_lines = ShellProgressBar::getAsciiFileLineCount(voc_file);
  ShellProgressBar progress(0, n_lines - 1, "extracting ontology", 50);
  progress.draw();
  ifstream input_if(voc_file.c_str());
  string word;
  while (!input_if.eof())
  {
    getline(input_if, word, '\n');
    if (word == "") continue;
    if (word[0] == '#' && word[1] == '#') continue;
    extractWord(word, wn_pos_type, true, true, false); 
    progress.update(word_index);
  }
  input_if.close();
  progress.done();
  finalize();
  input_if.close();
}

bool WordNetOntology::isInWordNet(string word, bool trim_word, bool stem_word, bool remove_undescores)
{
  if (trim_word)
    word = trimWord(word);

  if (remove_undescores)
    word = underscore_to_space(word);

  if (word == NULL_TAG)
  {
    return false;
  } else
  {
    bool found_noun = hasSenseInWordNet(word, NOUN_TYPE);
    bool found_verb = hasSenseInWordNet(word, VERB_TYPE);
    bool found_adj = hasSenseInWordNet(word, ADJ_TYPE);
    bool found_adv = hasSenseInWordNet(word, ADV_TYPE);
    bool found_stemmed_noun = false;
    bool found_stemmed_verb = false;
    bool found_stemmed_adj = false;
    bool found_stemmed_adv = false;
    
    if (stem_word)
    {
      string stemmed_word = stemWord(word, NOUN);
      if (stemmed_word != word)
        found_stemmed_noun = hasSenseInWordNet(stemmed_word, NOUN_TYPE);
      stemmed_word = stemWord(word, VERB);
      if (stemmed_word != word)
        found_stemmed_verb = hasSenseInWordNet(stemmed_word, VERB_TYPE);
      stemmed_word = stemWord(word, ADJ);
      if (stemmed_word != word)
        found_stemmed_adj = hasSenseInWordNet(stemmed_word, ADJ_TYPE);
      stemmed_word = stemWord(word, ADV);
      if (stemmed_word != word)
        found_stemmed_adv = hasSenseInWordNet(stemmed_word, ADV_TYPE);
    }
    
    if (found_noun || found_verb || found_adj || found_adv ||
        found_stemmed_noun || found_stemmed_verb || found_stemmed_adj || found_stemmed_adv)
    {
      return true;
    } else
    {
      return false;
    }
  }
}

bool WordNetOntology::hasSenseInWordNet(string word, int wn_pos_type)
{
  //char* cword = const_cast<char*>(word.c_str());
  char* cword = cstr(word);
  SynsetPtr ssp = NULL;
  
  switch (wn_pos_type)
  {
  case NOUN_TYPE:
    ssp = findtheinfo_ds(cword, NOUN, -HYPERPTR, ALLSENSES);
    break;
  case VERB_TYPE:
    ssp = findtheinfo_ds(cword, VERB, -HYPERPTR, ALLSENSES);
    break;
  case ADJ_TYPE:
    ssp = findtheinfo_ds(cword, ADJ, -HYPERPTR, ALLSENSES);
    break;
  case ADV_TYPE:
    ssp = findtheinfo_ds(cword, ADV, -HYPERPTR, ALLSENSES);
    break;
  }

  bool ssp_is_null = (ssp == NULL);

  delete(cword);
  free_syns(ssp);

  return !ssp_is_null;

}

void WordNetOntology::extractWord(string original_word, int wn_pos_type, bool trim_word, bool stem_word, bool remove_undescores)
{
  bool found_noun = false;
  bool found_verb = false;
  bool found_adj = false;
  bool found_adv = false;
  bool found_stemmed_noun = false;
  bool found_stemmed_verb = false;
  bool found_stemmed_adj = false;
  bool found_stemmed_adv = false;
  bool found = false;
  string processed_word = original_word;
  string stemmed_word;

  words[word_index] = original_word;
  words_id[original_word] = word_index;

  if (!catchSpecialTags(original_word))
  {
    if (trim_word)
      processed_word = trimWord(original_word);

    if (remove_undescores)
      processed_word = underscore_to_space(processed_word);

    if (processed_word == NULL_TAG)
    {
      out_of_wn_word_count++;
      processUnknownWord(word_index);
      word_is_in_wn[word_index] = false;
    } else
    {
      if (wn_pos_type == NOUN_TYPE || wn_pos_type == ALL_WN_TYPE)
        found_noun = extractSenses(original_word, processed_word, NOUN_TYPE);
      if (wn_pos_type == VERB_TYPE || wn_pos_type == ALL_WN_TYPE)
        found_verb = extractSenses(original_word, processed_word, VERB_TYPE);
      if (wn_pos_type == ADJ_TYPE || wn_pos_type == ALL_WN_TYPE)
        found_adj = extractSenses(original_word, processed_word, ADJ_TYPE);
      if (wn_pos_type == ADV_TYPE || wn_pos_type == ALL_WN_TYPE)
        found_adv = extractSenses(original_word, processed_word, ADV_TYPE);
    
      if (stem_word)
      {
        if (wn_pos_type == NOUN_TYPE || wn_pos_type == ALL_WN_TYPE)
        {
          stemmed_word = stemWord(processed_word, NOUN);
          if (stemmed_word != processed_word)
            found_stemmed_noun = extractSenses(original_word, stemmed_word, NOUN_TYPE);
        }
        if (wn_pos_type == VERB_TYPE || wn_pos_type == ALL_WN_TYPE)
        {
          stemmed_word = stemWord(processed_word, VERB);
          if (stemmed_word != processed_word)
            found_stemmed_verb = extractSenses(original_word, stemmed_word, VERB_TYPE);
        }
        if (wn_pos_type == ADJ_TYPE || wn_pos_type == ALL_WN_TYPE)
        {
          stemmed_word = stemWord(processed_word, ADJ);
          if (stemmed_word != processed_word)
            found_stemmed_adj = extractSenses(original_word, stemmed_word, ADJ_TYPE);
        }
        if (wn_pos_type == ADV_TYPE || wn_pos_type == ALL_WN_TYPE)
        {
          stemmed_word = stemWord(processed_word, ADV);
          if (stemmed_word != processed_word)
            found_stemmed_adv = extractSenses(original_word, stemmed_word, ADV_TYPE);
        }
      }
    
      found = (found_noun || found_verb || found_adj || found_adv ||
               found_stemmed_noun || found_stemmed_verb || found_stemmed_adj || found_stemmed_adv);
      if (found)
      {
        in_wn_word_count++;
        word_is_in_wn[word_index] = true;
      } else
      {
        out_of_wn_word_count++;
        processUnknownWord(word_index);
        word_is_in_wn[word_index] = false;
      }
    }
  } else // word is a "special tag" (<OOV>, etc...)
  {
    out_of_wn_word_count++;
    word_is_in_wn[word_index] = false;
  }
  if (word_to_senses[word_index].isEmpty())
    PLWARNING("word %d (%s) was not processed correctly (found = %d)", word_index, words[word_index].c_str(), found);
  word_index++;

}

bool WordNetOntology::extractSenses(string original_word, string processed_word, int wn_pos_type)
{
  //char* cword = const_cast<char*>(processed_word.c_str());
  char* cword = cstr(processed_word);
  SynsetPtr ssp = NULL;
  
  switch (wn_pos_type)
  {
  case NOUN_TYPE:
    ssp = findtheinfo_ds(cword, NOUN, -HYPERPTR, ALLSENSES);
    break;
  case VERB_TYPE:
    ssp = findtheinfo_ds(cword, VERB, -HYPERPTR, ALLSENSES);
    break;
  case ADJ_TYPE:
    ssp = findtheinfo_ds(cword, ADJ, -HYPERPTR, ALLSENSES);
    break;
  case ADV_TYPE:
    ssp = findtheinfo_ds(cword, ADV, -HYPERPTR, ALLSENSES);
    break;
  }

  if (ssp == NULL)
  {
    return false;
  } else
  {
    switch (wn_pos_type)
    {
    case NOUN_TYPE:
      noun_count++;
      break;
    case VERB_TYPE:
      verb_count++;
      break;
    case ADJ_TYPE:
      adj_count++;
      break;
    case ADV_TYPE:
      adv_count++;
      break;
    }

    // extract all senses for a given word
    while (ssp != NULL)
    {
    
      Node* node = checkForAlreadyExtractedSynset(ssp);

      if (node == NULL) // not found
      {

        switch (wn_pos_type)
        {
        case NOUN_TYPE:
          noun_sense_count++;
          break;
        case VERB_TYPE:
          verb_sense_count++;
          break;
        case ADJ_TYPE:
          adj_sense_count++;
          break;
        case ADV_TYPE:
          adv_sense_count++;
          break;
        }

        // create a new sense (1rst-level synset Node)
        node = extractOntology(ssp);
        
      }

      int word_id = words_id[original_word];
      node->types.insert(wn_pos_type);
      word_to_senses[word_id].insert(node->ss_id);
      sense_to_words[node->ss_id].insert(word_id);

      // warning : should check if inserting a given sense twice (vector)
      // (should not happen if vocabulary contains only unique values)
      switch(wn_pos_type)
      {
        case NOUN_TYPE:
          word_to_noun_wnsn[word_id].push_back(node->ss_id);
          word_to_noun_senses[word_id].insert(node->ss_id);
          break;
        case VERB_TYPE:
          word_to_verb_wnsn[word_id].push_back(node->ss_id);
          word_to_verb_senses[word_id].insert(node->ss_id);
          break;
        case ADJ_TYPE:
          word_to_adj_wnsn[word_id].push_back(node->ss_id);
          word_to_adj_senses[word_id].insert(node->ss_id);
          break;
        case ADV_TYPE:
          word_to_adv_wnsn[word_id].push_back(node->ss_id);
          word_to_adv_senses[word_id].insert(node->ss_id);
          break;
      }

      ssp = ssp->nextss;
    }
    free_syns(ssp);
    return true;
  }
}

Node* WordNetOntology::extractOntology(SynsetPtr ssp)
{
  Node* node = new Node(synset_index++); // increment synset counter
  node->syns = getSynsetWords(ssp);
  string defn = ssp->defn;
  removeDelimiters(defn, '*', '%');
  removeDelimiters(defn, '|', '/');
  node->gloss = defn;
  node->is_unknown = false;
  synsets[node->ss_id] = node;

  ssp = ssp->ptrlist;
  
  while (ssp != NULL)
  {
    Node* parent_node = checkForAlreadyExtractedSynset(ssp);
    
    if (parent_node == NULL) // create new synset Node
    {
      parent_node = extractOntology(ssp);
    }
    
    node->parents.insert(parent_node->ss_id);
    parent_node->children.insert(node->ss_id);
    
    ssp = ssp->nextss;
  }

  return node;

}

bool WordNetOntology::catchSpecialTags(string word)
{
  int word_id = words_id[word];
  if (word == OOV_TAG)
  {
    word_to_senses[word_id].insert(OOV_SS_ID);
    sense_to_words[OOV_SS_ID].insert(word_id);
    return true;
  } else if (word == PROPER_NOUN_TAG)
  {
    word_to_senses[word_id].insert(PROPER_NOUN_SS_ID);
    sense_to_words[PROPER_NOUN_SS_ID].insert(word_id);
    return true;
  } else if (word == NUMERIC_TAG)
  {
    word_to_senses[word_id].insert(NUMERIC_SS_ID);
    sense_to_words[NUMERIC_SS_ID].insert(word_id);
    return true;
  } else if (word == PUNCTUATION_TAG)
  {
    word_to_senses[word_id].insert(PUNCTUATION_SS_ID);
    sense_to_words[PUNCTUATION_SS_ID].insert(word_id);
    return true;
  } else if (word == STOP_TAG)
  {
    word_to_senses[word_id].insert(STOP_SS_ID);
    sense_to_words[STOP_SS_ID].insert(word_id);
    return true;
  } else if (word == BOS_TAG)
  {
    word_to_senses[word_id].insert(BOS_SS_ID);
    sense_to_words[BOS_SS_ID].insert(word_id);
    return true;
  } else if (word == EOS_TAG)
  {
    word_to_senses[word_id].insert(EOS_SS_ID);
    sense_to_words[EOS_SS_ID].insert(word_id);
    return true;
  }
  return false;
}

void WordNetOntology::lookForSpecialTags()
{
  if (!isSense(OOV_SS_ID))
    PLWARNING("no <oov> tag found");
  if (!isSense(PROPER_NOUN_SS_ID))
    PLWARNING("no <proper_noun> tag found");
  if (!isSense(NUMERIC_SS_ID))
    PLWARNING("no <numeric> tag found");
  if (!isSense(PUNCTUATION_SS_ID))
    PLWARNING("no <punctuation> tag found");
  if (!isSense(STOP_SS_ID))
    PLWARNING("no <stop> tag found");
}

void WordNetOntology::finalize()
{
  propagatePOSTypes();
  linkUpperCategories();
  //setLevels();
  removeNonReachableSynsets();
}

int WordNetOntology::getWordSenseIdForWnsn(string word, int wn_pos_type, int wnsn)
{
  if (!isWord(word))
  {
#ifndef NOWARNING
    PLWARNING("asking for a non-word (%s)", word.c_str());
#endif
    return WNO_ERROR;
  }

  int word_id = words_id[word];
  switch (wn_pos_type)
  {
  case NOUN_TYPE:
    if (wnsn > (int)word_to_noun_wnsn[word_id].size())
    {
#ifndef NOWARNING
      PLWARNING("invalid noun wnsn (%d)", wnsn);
#endif
      return WNO_ERROR;
    } else
      return word_to_noun_wnsn[word_id][wnsn - 1];
    break;
  case VERB_TYPE:
    if (wnsn > (int)word_to_verb_wnsn[word_id].size())
    {
#ifndef NOWARNING
      PLWARNING("invalid verb wnsn (%d)", wnsn);
#endif
      return WNO_ERROR;
    } else
      return word_to_verb_wnsn[word_id][wnsn - 1];
    break;
  case ADJ_TYPE:
    if (wnsn > (int)word_to_adj_wnsn[word_id].size())
    {
#ifndef NOWARNING
      PLWARNING("invalid adj wnsn (%d)", wnsn);
#endif
      return WNO_ERROR;
    } else
      return word_to_adj_wnsn[word_id][wnsn - 1];
    break;
  case ADV_TYPE:
    if (wnsn > (int)word_to_adv_wnsn[word_id].size())
    {
#ifndef NOWARNING
      PLWARNING("invalid adv wnsn (%d)", wnsn);
#endif
      return WNO_ERROR;
    } else
      return word_to_adv_wnsn[word_id][wnsn - 1];
    break;
  default:
#ifndef NOWARNING
      PLWARNING("undefined type");
#endif
    return WNO_ERROR;
  }
}

int WordNetOntology::getWordSenseIdForSenseKey(string lemma, string lexsn)
{
  string sense_key = lemma + "%" + lexsn;

  //cout << sense_key << endl;

  //char* csense_key = const_cast<char*>(sense_key.c_str());
  char* csense_key = cstr(sense_key);
  SynsetPtr ssp = GetSynsetForSense(csense_key);
  if (ssp != NULL)
  {
    vector<string> synset_words = getSynsetWords(ssp);
    int word_id = words_id[lemma];
    for (SetIterator it = word_to_senses[word_id].begin(); it != word_to_senses.end(); ++it)
    {
      Node* node = synsets[*it];
      if (node->syns == synset_words)
        return node->ss_id;
    }
  }
  return WNO_ERROR;
}

void WordNetOntology::processUnknownWord(int word_id)
{
  if (differentiate_unknown_words)
  {
    // create an UNKNOWN synset for a particular word
    Node* unk_node = new Node(synset_index++);
    int unknown_sense_id = unknown_sense_index++;
    unk_node->syns.push_back("UNKNOWN_SENSE_" + tostring(unknown_sense_id));
    unk_node->gloss = "(unknown sense " + tostring(unknown_sense_id) + ")";
    unk_node->types.insert(UNDEFINED_TYPE);
    synsets[unk_node->ss_id] = unk_node;
   
    // link UNKNOWN <-> SUPER-UNKNOWN
    unk_node->parents.insert(SUPER_UNKNOWN_SS_ID);
    synsets[SUPER_UNKNOWN_SS_ID]->children.insert(unk_node->ss_id);
  
    word_to_senses[word_id].insert(unk_node->ss_id);
    sense_to_words[unk_node->ss_id].insert(word_id);
  } else // all the unknown words are linked to SUPER-UNKNOWN 
  {      // (acting in this context as a sense)
    word_to_senses[word_id].insert(SUPER_UNKNOWN_SS_ID);
    sense_to_words[SUPER_UNKNOWN_SS_ID].insert(word_id);
  }

}

void WordNetOntology::propagatePOSTypes()
{
  for (map<int, Set>::iterator it = sense_to_words.begin(); it != sense_to_words.end(); ++it)
  {
    Node* node = synsets[it->first];
    propagatePOSTypes(node);
  }
}

void WordNetOntology::propagatePOSTypes(Node* node)
{
  for (SetIterator it = node->parents.begin(); it != node->parents.end(); ++it)
  {
    Node* parent_node = synsets[*it];
    for (SetIterator iit = node->types.begin(); iit != node->types.end(); ++iit)
    {
      parent_node->types.insert(*iit);
    }
    if (parent_node->types.size() > 1)
    {
#ifndef NOWARNING
      PLWARNING("a synset has more than 1 type");
#endif
    }
    propagatePOSTypes(parent_node);
  }
}

// link last-level nodes with the corresponding prior-to-ROOT POS super-category	
void WordNetOntology::linkUpperCategories()
{
  for (map<int, Node*>::iterator it = synsets.begin(); it != synsets.end(); ++it)
  {
    int ss_id = it->first;
    Node* node = it->second;
    if (node->parents.size() == 0 && ss_id != ROOT_SS_ID) 
    {
      bool link_directly_to_root = true;
      if (node->types.contains(NOUN_TYPE))
      {
        node->parents.insert(NOUN_SS_ID);
        synsets[NOUN_SS_ID]->children.insert(ss_id);
        link_directly_to_root = false;
      }
      if (node->types.contains(VERB_TYPE))
      {
        node->parents.insert(VERB_SS_ID);
        synsets[VERB_SS_ID]->children.insert(ss_id);
        link_directly_to_root = false;
      }
      if (node->types.contains(ADJ_TYPE))
      {
        node->parents.insert(ADJ_SS_ID);
        synsets[ADJ_SS_ID]->children.insert(ss_id);
        link_directly_to_root = false;
      }
      if (node->types.contains(ADV_TYPE))
      {
        node->parents.insert(ADV_SS_ID);
        synsets[ADV_SS_ID]->children.insert(ss_id);
        link_directly_to_root = false;
      }
      if (link_directly_to_root)
      {
        node->parents.insert(ROOT_SS_ID);
        synsets[ROOT_SS_ID]->children.insert(ss_id);
      }
    }
  }
}

// void WordNetOntology::setLevels()
// {
//   Node* root_node = synsets[ROOT_SS_ID];
//   root_node->level = 0;
//   for (SetIterator it = root_node->children.begin(); it != root_node->children.end(); ++it)
//     setLevels(*it, 1);
// }

// void WordNetOntology::setLevels(int ss_id, int level)
// {
//   Node* node = synsets[ss_id];
//   if (node->level != -1 && node->level != level)
//   {
//     PLWARNING("a synset is at 2 different levels (old level = %d, new level = %d)", node->level, level);
//     printSynset(ss_id, 1);
//   }
//   node->level = level;
//   for (SetIterator it = node->children.begin(); it != node->children.end(); ++it)
//     setLevels(*it, level + 1);
// }

// look for an identical, already extracted synset
Node* WordNetOntology::checkForAlreadyExtractedSynset(SynsetPtr ssp)
{
  vector<string> syns = getSynsetWords(ssp);
  string gloss = ssp->defn;
  for (map<int, Node*>::iterator it = synsets.begin(); it != synsets.end(); ++it)
  {
    Node* node = it->second;
    if (node->syns == syns && node->gloss == gloss)
    {
      return node;
    }
  }
  return NULL;

}

vector<string> WordNetOntology::getSynsetWords(SynsetPtr ssp)
{
  vector<string> syns;
  for (int i = 0; i < ssp->wcount; i++)
  {
    strsubst(ssp->words[i], '_', ' ');
    string word_i = ssp->words[i];
    removeDelimiters(word_i, '*', '%');
    removeDelimiters(word_i, '|', '/');
    syns.push_back(word_i);
  }
  return syns;
}

void WordNetOntology::print(bool print_ontology)
{
  for (map<int, Set>::iterator it = word_to_senses.begin(); it != word_to_senses.end(); ++it)
  {
    cout << words[it->first] << endl;
    for (SetIterator iit = it->second.begin(); iit != it->second.end(); ++iit)
    {
      printSynset(*iit, 1);
      if (print_ontology)
      {
        printOntology(synsets[*iit], 2);
      }
    }
  }
}

void WordNetOntology::printOntology(Node* node, int level)
{
  for (SetIterator it = node->parents.begin(); it != node->parents.end(); ++it)
  {
    printSynset(*it, level);
    printOntology(synsets[*it], level + 1);
  }
}

void WordNetOntology::printSynset(int ss_id, int indent_level)
{
  for (int i = 0; i < indent_level; i++) cout << "    "; // indent
  cout << "=> ";

  for (vector<string>::iterator it = synsets[ss_id]->syns.begin(); it != synsets[ss_id]->syns.end(); ++it)
  {
    cout << *it << ", ";
  }
  cout << " (" << ss_id << ")";
  //cout << " " << synsets[ss_id]->gloss;
  cout << " {";
  for (SetIterator it = synsets[ss_id]->types.begin(); it != synsets[ss_id]->types.end(); ++it)
  {
    int type = *it;
    switch (type)
    {
    case NOUN_TYPE:
      cout << "noun ";
      break;
    case VERB_TYPE:
      cout << "verb ";
      break;
    case ADJ_TYPE:
      cout << "adjective ";
      break;
    case ADV_TYPE:
      cout << "adverb ";
      break;
    case UNDEFINED_TYPE:
      cout << "undefined ";
      break;
    }
  }
  cout << "}" << endl;
}

void WordNetOntology::printStats()
{
/*
  cout << getSenseSize() << " senses (" << noun_sense_count << " nouns, " << verb_sense_count << " verbs, " 
       << adj_sense_count << " adjectives, " << adv_sense_count << " adverbs) for " << getVocSize() << " words" << endl;
  cout << out_of_wn_word_count << " out-of-wordnet words" << endl;
  cout << in_wn_word_count << " in-wordnet words" << endl;
  cout << noun_count << " nouns" << endl;
  cout << verb_count << " verbs" << endl;
  cout << adj_count << " adjectives" << endl;
  cout << adv_count << " adverbs" << endl;
  cout << (double)getSenseSize() / (double)getVocSize() << " senses per word on average" << endl;
  int all_classes = noun_count + verb_count + adj_count + adv_count;
  cout << (double)all_classes / (double)in_wn_count << " classes per word on average" << endl;
*/
  cout << getVocSize() << " words in vocabulary" << endl;
  cout << in_wn_word_count << " in WN words" << endl;
  cout << out_of_wn_word_count << " out of WN words" << endl;
  cout << getSenseSize() << " senses (" << (real)getSenseSize() / (real)getVocSize() << " senses per word on average)" << endl;
  cout << getSynsetSize() << " categories (ontology : sense + category, possible overlap)" << endl;
  if (are_word_high_level_senses_extracted)
  {
    cout << n_word_high_level_senses << " high-level senses (" << (real)n_word_high_level_senses / (real)getVocSize() << " high-level senses per word on average)" << endl;
  }
}

void WordNetOntology::save(string synset_file, string ontology_file)
{
  // synset
  ofstream of_synsets(synset_file.c_str());
  for (map<int, Node*>::iterator it = synsets.begin(); it != synsets.end(); ++it)
  {
    int ss_id = it->first;
    Node* node = it->second;
    of_synsets << ss_id << "*|";
    for (SetIterator it = node->types.begin(); it != node->types.end(); ++it)
    {
      of_synsets << *it << "|";
    }
    of_synsets << "*|";
    of_synsets << node->gloss << "|";
    for (vector<string>::iterator iit = node->syns.begin(); iit != node->syns.end(); ++iit)
    {
      of_synsets << *iit << "|";
    }
    of_synsets << endl;
  }
  of_synsets.close();

  // ontology
  ofstream of_ontology(ontology_file.c_str());
  for (map<int, Set>::iterator wit = word_to_senses.begin(); wit != word_to_senses.end(); ++wit)
  {
    int word_id = wit->first;
    of_ontology << "w " << word_id << " " << word_is_in_wn[word_id] << endl;
  }
  for (map<int, Node*>::iterator it = synsets.begin(); it != synsets.end(); ++it)
  {
    int id = it->first;
    Node* node = it->second;
    for(SetIterator iit = node->children.begin(); iit != node->children.end(); ++iit)
    {
      int child_id = *iit;
      of_ontology << "c " << id << " " << child_id << endl;
    }
    if (sense_to_words.find(id) != sense_to_words.end())
    {
      for (SetIterator iit = sense_to_words[id].begin(); iit != sense_to_words[id].end(); ++iit)
        of_ontology << "s " << id << " " << (*iit) << endl;
    }
  }

  of_ontology.close();
}

void WordNetOntology::save(string voc_file)
{
  ofstream of_voc(voc_file.c_str());
  for (map<int, string>::iterator it = words.begin(); it != words.end(); ++it)
  {
    of_voc << it->second << endl;
  }
  of_voc.close();
}

void WordNetOntology::load(string voc_file, string synset_file, string ontology_file)
{
  ifstream if_voc(voc_file.c_str());
  if (!if_voc) PLERROR("can't open %s", voc_file.c_str());
  ifstream if_synsets(synset_file.c_str());
  if (!if_synsets) PLERROR("can't open %s", synset_file.c_str());
  ifstream if_ontology(ontology_file.c_str());
  if (!if_ontology) PLERROR("can't open %s", ontology_file.c_str());

  string line;
  int counter = 0;
  while (!if_voc.eof()) // voc
  {
    getline(if_voc, line, '\n');
    if (line == "") continue;
    if (line[0] == '#' && line[1] == '#') continue;
    words_id[line] = counter;
    word_to_senses[counter] = Set();
    words[counter++] = line;
  }
  if_voc.close();

  while (!if_synsets.eof()) // synsets
  {
    getline(if_synsets, line, '\n');
    if (line == "") continue;
    if (line[0] == '#') continue;
    vector<string> tokens = split(line, "*");
    if (tokens.size() != 3)
    { 
      PLERROR("the synset file has not the expected format, line = '%s'", line.c_str());
    }
    int ss_id = toint(tokens[0]);
    vector<string> type_tokens = split(tokens[1], "|");
    vector<string> ss_tokens = split(tokens[2], "|");
    Node* node = new Node(ss_id);
    for (unsigned int i = 0; i < type_tokens.size(); i++)
    {
      node->types.insert(toint(type_tokens[i]));
    }
    node->gloss = ss_tokens[0];
    for (unsigned int i = 1; i < ss_tokens.size(); i++)
    {
      node->syns.push_back(ss_tokens[i]);
    }
    synsets[node->ss_id] = node;
  }
  if_synsets.close();
  int n_lines = ShellProgressBar::getAsciiFileLineCount(ontology_file);
  ShellProgressBar progress(0, n_lines - 1, "loading ontology", 50);
  progress.draw();
  counter = 0;
  while (!if_ontology.eof()) // ontology
  {
    getline(if_ontology, line, '\n');
    progress.update(counter++);
    if (line == "") continue;
    if (line[0] == '#') continue;
    vector<string> tokens = split(line);
    if (tokens.size() != 3) 
    {
      PLERROR("the ontology file has not the expected format");
    }
    int id = toint(tokens[1]);
    int child_id;

    if (tokens[0] == "w")
    {
      bool is_in_wn = tobool(tokens[2]);
      word_is_in_wn[id] = is_in_wn;
      if (is_in_wn)
        in_wn_word_count++;
      else
        out_of_wn_word_count++;
    } else if (tokens[0] == "s")
    {
      child_id = toint(tokens[2]);
      word_to_senses[child_id].insert(id);
      sense_to_words[id].insert(child_id);
      for (SetIterator tit = synsets[id]->types.begin(); tit != synsets[id]->types.end(); ++tit)
      {
        int type = *tit;
        switch (type)
        {
        case NOUN_TYPE:
          word_to_noun_senses[child_id].insert(id);
          break;
        case VERB_TYPE:
          word_to_verb_senses[child_id].insert(id);
          break;
        case ADJ_TYPE:
          word_to_adj_senses[child_id].insert(id);
          break;
        case ADV_TYPE:
          word_to_adv_senses[child_id].insert(id);
          break;          
        }
      }
    } else if (tokens[0] == "c")
    {
      child_id = toint(tokens[2]);
      synsets[child_id]->parents.insert(id);
      synsets[id]->children.insert(child_id);
    }
  }
  if_ontology.close();
  progress.done();
  if_voc.close();
  if_synsets.close();
  if_ontology.close();
}

void WordNetOntology::printNodes()
{
  for (map<int, Node*>::iterator it = synsets.begin(); it != synsets.end(); ++it)
  {
    Node* node = it->second;
    cout << "Node id = " << node->ss_id << " | parents = ";
    for (SetIterator pit = node->parents.begin(); pit != node->parents.end(); ++pit)
    {
      cout << *pit << " ";
    }
    cout << " | children = ";
    for (SetIterator cit = node->children.begin(); cit != node->children.end(); ++cit)
    {
      cout << *cit << " ";
    }
    cout << endl;
  }
}

void WordNetOntology::extractAncestors(int threshold, bool cut_with_word_coverage)
{
#ifdef VERBOSE
  cout << "extracting ancestors... ";
#endif

  // synsets -> ancestors
  int n_sense_ancestors = 0;
  for (map<int, Node*>::iterator it = synsets.begin(); it != synsets.end(); ++it)
  {
    Set ancestors;

    if (cut_with_word_coverage)
      extractAncestors(it->second, ancestors, threshold);
    else
      extractAncestors(it->second, ancestors, 1, threshold);
    ancestors.insert(it->first);
    synset_to_ancestors[it->first] = ancestors;
    n_sense_ancestors += ancestors.size();
  }

  are_ancestors_extracted = true;

  // words -> ancestors
  int n_word_ancestors = 0;
  for (map<int, Set>::iterator it = word_to_senses.begin(); it != word_to_senses.end(); ++it)
  {
    int word_id = it->first;
    Set senses = it->second;
    Set word_ancestors;
    for (SetIterator it = senses.begin(); it != senses.end(); it++)
    {
      int sense_id = *it;
      Set ancestors = getSynsetAncestors(sense_id);
      ancestors.insert(sense_id);
      word_ancestors.merge(ancestors);
    }
    word_to_ancestors[word_id] = word_ancestors;
    n_word_ancestors += word_ancestors.size();
  }

#ifdef VERBOSE
  cout << "(" << n_sense_ancestors << " sense ancestors, " << n_word_ancestors << " word ancestors)" << endl;
#endif

}

// "word coverage threshold" version
void WordNetOntology::extractAncestors(Node* node, Set ancestors, int word_coverage_threshold)
{
  int ss_id = node->ss_id;
  if (word_coverage_threshold == -1 || synset_to_word_descendants[ss_id].size() < word_coverage_threshold)
  {
    ancestors.insert(ss_id);
    for (SetIterator it = node->parents.begin(); it != node->parents.end(); ++it)
    {
      extractAncestors(synsets[*it], ancestors, word_coverage_threshold);
    }
  }

/*
  for (SetIterator it = node->parents.begin(); it != node->parents.end(); ++it)
  {
    int ss_id = *it;
    if (word_coverage_threshold == -1 || synset_to_word_descendants[ss_id].size() < word_coverage_threshold)
    {
      ancestors.insert(ss_id);
      extractAncestors(synsets[ss_id], ancestors, word_coverage_threshold);
    }
  }
*/
}

// "level threshold" version
void WordNetOntology::extractAncestors(Node* node, Set ancestors, int level, int level_threshold)
{
  for (SetIterator it = node->parents.begin(); it != node->parents.end(); ++it)
  {
    ancestors.insert(*it);
    if (level_threshold == -1 || level < level_threshold)
      extractAncestors(synsets[*it], ancestors, level + 1, level_threshold);
  }
}

Set WordNetOntology::getSynsetAncestors(int id, int max_level)
{
  if (are_ancestors_extracted)
  {
    if (!isSynset(id))
    {
#ifndef NOWARNING
      PLWARNING("asking for a non-synset id (%d)", id);
#endif
    }
    return synset_to_ancestors[id];
  } else
  {
    Set ancestors;
    if (isSynset(id))
    {
#ifndef NOWARNING
      PLWARNING("using non-pre-computed version");
#endif
      extractAncestors(synsets[id], ancestors, 1, max_level);
    } else
    {
#ifndef NOWARNING
      PLWARNING("asking for a non-synset id (%d)", id);
#endif
    }
    return ancestors;
  }
}

Set WordNetOntology::getWordAncestors(int id, int max_level)
{
  if (are_ancestors_extracted)
  {
    if (!isWord(id))
    {
#ifndef NOWARNING
      PLWARNING("asking for a non-word id (%d)", id);
#endif
    }
    return word_to_ancestors[id];
  } else
  {
    Set word_ancestors;
    if (isWord(id))
    {
#ifndef NOWARNING
      PLWARNING("using non-pre-computed version");
#endif
      for (SetIterator it = word_to_senses[id].begin(); it != word_to_senses[id].end(); ++it)
      {
        int sense_id = *it;
        word_ancestors.insert(sense_id);
        Set synset_ancestors = getSynsetAncestors(sense_id, max_level);
        word_ancestors.merge(synset_ancestors);
      }
    } else
    {
#ifndef NOWARNING
      PLWARNING("asking for a non-word id");
#endif
    }

    return word_ancestors;
  }
}

bool WordNetOntology::isInWordNet(int word_id)
{
#ifndef NOWARNING
  if (!isWord(word_id))
  {
    PLWARNING("asking for a non-word id (%d)", word_id);
    return false;
  }
#endif
  return word_is_in_wn[word_id];
}

int WordNetOntology::getWordId(string word)
{
#ifndef NOWARNING
  if (words_id.find(word) == words_id.end())
  {
    PLWARNING("asking for a non-word (%s)", word.c_str());
    return -1;
  }
#endif
  return words_id[word];
}

string WordNetOntology::getWord(int id)
{
#ifndef NOWARNING
  if (!isWord(id))
  {
    PLWARNING("asking for a non-word id (%d)", id);
    return NULL_TAG;
  }
#endif
  return words[id];
}

Set WordNetOntology::getWordSenses(int id)
{
#ifndef NOWARNING
  if (!isWord(id))
  {
    PLWARNING("asking for a non-word id (%d)", id);
    return Set();
  }
#endif
  return word_to_senses[id];
}

Set WordNetOntology::getWordHighLevelSenses(int id)
{
#ifndef NOWARNING
  if (!isWord(id))
  {
    PLWARNING("asking for a non-word id (%d)", id);
    return Set();
  }
#endif

  if (!are_word_high_level_senses_extracted)
    PLERROR("word high-level senses have not been extracted");

  return word_to_high_level_senses[id];
}

Set WordNetOntology::getWordNounSenses(int id)
{
#ifndef NOWARNING
  if (!isWord(id))
  {
    PLWARNING("asking for a non-word id (%d)", id);
    return Set();
  }
#endif
  return word_to_noun_senses[id];
}

Set WordNetOntology::getWordVerbSenses(int id)
{
#ifndef NOWARNING
  if (!isWord(id))
  {
    PLWARNING("asking for a non-word id (%d)", id);
    return Set();
  }
#endif
  return word_to_verb_senses[id];
}

Set WordNetOntology::getWordAdjSenses(int id)
{
#ifndef NOWARNING
  if (!isWord(id))
  {
    PLWARNING("asking for a non-word id (%d)", id);
    return Set();
  }
#endif
  return word_to_adj_senses[id];
}

Set WordNetOntology::getWordAdvSenses(int id)
{
#ifndef NOWARNING
  if (!isWord(id))
  {
    PLWARNING("asking for a non-word id (%d)", id);
    return Set();
  }
#endif
  return word_to_adv_senses[id];
}

Set WordNetOntology::getWordsForSense(int id)
{
#ifndef NOWARNING
  if (!isSense(id))
  {
    PLWARNING("asking for a non-sense id (%d)", id);
    return Set();
  }
#endif
  return sense_to_words[id];
}

Node* WordNetOntology::getSynset(int id)
{
#ifndef NOWARNING
  if (!isSynset(id))
  {
    PLWARNING("asking for a non-synset id (%d)", id);
    return NULL;
  }
#endif
  return synsets[id];
}

void WordNetOntology::printSynsetAncestors()
{
  if (!are_ancestors_extracted)
  {
    extractAncestors(WORD_COVERAGE_THRESHOLD);
  }
  for (map<int, Set>::iterator it = synset_to_ancestors.begin(); it != synset_to_ancestors.end(); ++it)
  {
    cout << it->first << " -> ";
    for (SetIterator iit = it->second.begin(); iit != it->second.end(); ++iit)
      cout << *iit << " ";
    cout << endl;
  }
}

void WordNetOntology::printWordAncestors()
{
  if (!are_ancestors_extracted)
  {
    extractAncestors(WORD_COVERAGE_THRESHOLD);
  }
  for (map<int, Set>::iterator it = word_to_senses.begin(); it != word_to_senses.end(); ++it)
  {
    int id = it->first;
    cout << id << " -> ";
    Set ancestors = getWordAncestors(id);
    for (SetIterator iit = ancestors.begin(); iit != ancestors.end(); ++iit)
    {
      cout << *iit << " ";
    }
    cout << endl;
  }
}

void WordNetOntology::extractDescendants()
{
#ifdef VERBOSE
  cout << "extracting descendants... ";
#endif

  int n_sense_descendants = 0;
  int n_word_descendants = 0;
  for (map<int, Node*>::iterator it = synsets.begin(); it != synsets.end(); ++it)
  {
    Set sense_descendants;
    Set word_descendants;
    extractDescendants(it->second, sense_descendants, word_descendants);
    synset_to_sense_descendants[it->first] = sense_descendants;
    synset_to_word_descendants[it->first] = word_descendants;
    n_sense_descendants += sense_descendants.size();
    n_word_descendants += word_descendants.size();
  }
  are_descendants_extracted = true;

#ifdef VERBOSE
  cout << "(" << n_sense_descendants << " senses, " << n_word_descendants << " words)" << endl;
#endif

}

void WordNetOntology::extractDescendants(Node* node, Set sense_descendants, Set word_descendants)
{
  int ss_id = node->ss_id;
  if (isSense(ss_id)) // is a sense
  {
    sense_descendants.insert(ss_id);
    for (SetIterator it = sense_to_words[ss_id].begin(); it != sense_to_words[ss_id].end(); ++it)
    {
      int word_id = *it;
      word_descendants.insert(word_id);
    }
  } 
  for (SetIterator it = node->children.begin(); it != node->children.end(); ++it)
  {
    extractDescendants(synsets[*it], sense_descendants, word_descendants);
  }
}

Set WordNetOntology::getSynsetSenseDescendants(int id)
{
  if (are_descendants_extracted)
  {
    if (!isSynset(id))
    {
#ifndef NOWARNING
      PLWARNING("asking for a non-synset id (%d)", id);
#endif
    }
    return synset_to_sense_descendants[id];
  }

  Set sense_descendants;
  if (isSynset(id))
  {
#ifndef NOWARNING
    PLWARNING("using non-pre-computed version");
#endif
    extractDescendants(synsets[id], sense_descendants, Set());
  } else
  {
#ifndef NOWARNING
    PLWARNING("asking for non-synset id (%d)", id);
#endif
  }
  return sense_descendants;
}

Set WordNetOntology::getSynsetWordDescendants(int id)
{
  if (are_descendants_extracted)
  {
    if (!isSynset(id))
    {
#ifndef NOWARNING
      PLWARNING("asking for a non-synset id (%d)", id);
#endif
    }
    return synset_to_word_descendants[id];
  }

  Set word_descendants;
  if (isSynset(id))
  {
#ifndef NOWARNING
    PLWARNING("using non-pre-computed version");
#endif
    extractDescendants(synsets[id], Set(), word_descendants);
  } else
  {
#ifndef NOWARNING
    PLWARNING("asking for non-synset id (%d)", id);
#endif
  }
  return word_descendants;
}

void WordNetOntology::printDescendants()
{
/*
  if (!are_descendants_extracted)
  {
    extractDescendants();
  }
  for (map<int, Set>::iterator it = synset_to_descendants.begin(); it != synset_to_descendants.end(); ++it)
  {
    cout << it->first << " -> ";
    for (SetIterator iit = it->second.begin(); iit != it->second.end(); ++iit)
      cout << *iit << " ";
    cout << endl;
  }
*/
}

bool WordNetOntology::isWord(int id)
{
  return (words.find(id) != words.end());
}

bool WordNetOntology:: isWord(string word)
{
  return (words_id.find(word) != words_id.end());
}

bool WordNetOntology::isSense(int id)
{
  return (sense_to_words.find(id) != sense_to_words.end());
}

bool WordNetOntology::isPureSense(int id)
{
  return (isSense(id) && synsets[id]->children.size() == 0);
}

bool WordNetOntology::isCategory(int id)
{
  return isSynset(id);
}

bool WordNetOntology::isPureCategory(int id)
{
  return (isCategory(id) && !isSense(id));
}

bool WordNetOntology::isSynset(int id)
{
  return (synsets.find(id) != synsets.end());
}

int WordNetOntology::overlappingSynsets(int ss_id1, int ss_id2)
{
  Set words1 = sense_to_words[ss_id1];
  Set words2 = sense_to_words[ss_id2];
  Set overlap;
  for (SetIterator it1=words1.begin();it1!=words1.end();++it1)
    if (words2.contains(*it1))
      overlap.insert(*it1);
  //for (set<int>::iterator it=overlap.begin();it!=overlap.end();++it)
  //  cout << words[*it] << endl;
  return overlap.size();
}

Set WordNetOntology::getAllWords()
{
  Set all_words;
  for (map<int, string>::iterator it = words.begin(); it != words.end(); ++it)
  {
    all_words.insert(it->first);
  }
  return all_words;
}

Set WordNetOntology::getAllSenses()
{
  Set senses;
  for (map<int, Set>::iterator it = sense_to_words.begin(); it != sense_to_words.end(); ++it)
  {
    senses.insert(it->first);
  }
  return senses;
}

Set WordNetOntology::getAllCategories()
{
  Set categories;
  for (map<int, Node*>::iterator it = synsets.begin(); it != synsets.end(); ++it)
  {
    categories.insert(it->first);
  }
  return categories;
}

void WordNetOntology::printWordOntology(int id)
{
  cout << words[id] << endl;
  for (SetIterator sit = word_to_senses[id].begin(); sit != word_to_senses[id].end(); ++sit)
  {
    int sense_id = *sit;
    printSynset(sense_id, 1);
    printOntology(synsets[sense_id], 2);    
  }
}

void WordNetOntology::printWordOntology(string word)
{
  printWordOntology(words_id[word]);
}

void WordNetOntology::printInvertedSynsetOntology(int id, int level)
{
  if (isSynset(id))
  {
    printSynset(id, level);
    for (SetIterator it = synsets[id]->children.begin(); it != synsets[id]->children.end(); ++it)
    {
      printInvertedSynsetOntology(*it, level + 1);
    }
  } else
  {
#ifndef NOWARNING
    PLWARNING("asking for a non-synset id (%d)", id);
#endif
  }
}

void WordNetOntology::intersectAncestorsAndSenses(Set categories, Set senses)
{
  // pour tous les mappings "mot -> ancetres", fait une intersection de "ancetres"
  // avec "categories"
  for (map<int, Set>::iterator it = word_to_ancestors.begin(); it != word_to_ancestors.end(); ++it)
  {
    it->second.intersection(categories);
  }

  // pour tous les mappings "synset -> ancetres" (ou "synset" = "sense" U "category")
  // enleve le mapping complet, si "synset" (la cle) n'intersecte pas avec "categories"
  Set keys_to_be_removed;
  for (map<int, Set>::iterator it = synset_to_ancestors.begin(); it != synset_to_ancestors.end(); ++it)
  {
    if (!categories.contains(it->first) && !senses.contains(it->first))
      keys_to_be_removed.insert(it->first);
  }
  // purge synset_to_ancestors
  for (SetIterator it = keys_to_be_removed.begin(); it != keys_to_be_removed.end(); ++it)
  {
    synset_to_ancestors.erase(*it);
    synsets.erase(*it);
  }

  // pour tous les mappings "synset -> ancetres" restants (ou "synset" = "sense" U "category")
  // fait une intersection de "ancetres" avec "categories"
  for (map<int, Set>::iterator it = synset_to_ancestors.begin(); it != synset_to_ancestors.end(); ++it)
  {
    it->second.intersection(categories); 
  }

  // pour tous les mappings "mot -> senses", fait une intersection de "senses"
  // avec "senses"
  for (map<int, Set>::iterator it = word_to_senses.begin(); it != word_to_senses.end(); ++it)
  {
    it->second.intersection(senses);
  }

  keys_to_be_removed->clear();
  for (map<int, Set>::iterator it = sense_to_words.begin(); it != sense_to_words.end(); ++it)
  {
    if (!senses.contains(it->first))
      keys_to_be_removed.insert(it->first);
  }

  for (SetIterator it = keys_to_be_removed.begin(); it != keys_to_be_removed.end(); ++it)
  {
    sense_to_words.erase(*it);
  }
}

bool WordNetOntology::isWordUnknown(string word)
{
  return isWordUnknown(words_id[word]);
}

bool WordNetOntology::isWordUnknown(int id)
{
  bool is_unknown = true;
  for (SetIterator it = word_to_senses[id].begin(); it != word_to_senses[id].end(); ++it)
  {
    if (!synsets[*it]->is_unknown)
      is_unknown = false;
  }
  return is_unknown;
}

bool WordNetOntology::isSynsetUnknown(int id)
{
  return synsets[id]->is_unknown;
}

// set<int> getWordCategoriesAtLevel(int id, int level)
// {
//   set<int> categories;
//   for (set<int>::iterator it = word_to_senses.begin(); it != word_to_senses.end(); ++it)
//   {
//     int sense_id = *it;
//     Node* node = sense_to_ontology[sense_id];
//     int cur_level = 0;
//     while (cur_level < level)
//     {
      
//     }
//   }
// }

// void WordNetOntology::getCategoriesAtLevel(int ss_id, int level, set<int>& categories)
// {
//   Node* node = synsets[ss_id];
//   if (node->level == level)
//   {
//     categories.insert(ss_id);
//   } else
//   {
//     for (SetIterator it = node->parents.begin(); it != node->parents.end(); ++it)
//     {
//       getCategoriesAtLevel(*it, level, categories);
//     }
//   }
// }

void WordNetOntology::getCategoriesAtLevel(int ss_id, int cur_level, int target_level, set<int>& categories)
{
  Node* node = synsets[ss_id];
  if (cur_level == target_level && !isTopLevelCategory(ss_id))
  {
    categories.insert(ss_id);
  } else
  {
    for (SetIterator it = node->parents.begin(); it != node->parents.end(); ++it)
    {
      getCategoriesAtLevel(*it, cur_level + 1, target_level, categories);
    }
  }  
}

void WordNetOntology::getCategoriesUnderLevel(int ss_id, int cur_level, int target_level, Set categories)
{
  Node* node = synsets[ss_id];
  if (!isTopLevelCategory(ss_id))
    categories.insert(ss_id);
  if (cur_level != target_level)
  {
    for (SetIterator it = node->parents.begin(); it != node->parents.end(); ++it)
      getCategoriesUnderLevel(*it, cur_level + 1, target_level, categories);
  }
}

void WordNetOntology::getDescendantCategoriesAtLevel(int ss_id, int cur_level, int target_level, Set categories)
{
  if (isSynset(ss_id))
  {
    Node* node = synsets[ss_id];
    if (cur_level < target_level && isSense(ss_id))
      categories.insert(ss_id);
    if (cur_level == target_level)
      categories.insert(ss_id);
    else
    {
      for (SetIterator it = node->children.begin(); it != node->children.end(); ++it)
        getDescendantCategoriesAtLevel(*it, cur_level + 1, target_level, categories);
    }  
  }
}

void WordNetOntology::reducePolysemy(int level)
{
  ShellProgressBar progress(0, words.size() - 1, "reducing polysemy", 50);
  progress.init();
  progress.draw();
  int count = 0;
  for (map<int, string>::iterator it = words.begin(); it != words.end(); ++it)
  {
    int word_id = it->first;
    //reduceWordPolysemy(word_id, level);
    reduceWordPolysemy_preserveSenseOverlapping(word_id, level);
    progress.update(count++);
  }
  progress.done();
  removeNonReachableSynsets();
}

// for every sense of a word, build a list of associated categories at the 
// nth ontological level, and use these to cluster the senses; we can keep only 
// a single sense in every clusters, thus performing a sense reduction
void WordNetOntology::reduceWordPolysemy(int word_id, int level)
{
  Set senses = word_to_senses[word_id];
  Set senses_to_be_removed;
  if (senses.size() > 1)
  {
    //SetsValuesSet svs;
    set<set<int> > ss;
    for (SetIterator it = senses.begin(); it != senses.end(); ++it)
    {
      int sense_id = *it;
      set<int> categories_at_level;
      getCategoriesAtLevel(sense_id, 0, level, categories_at_level);

//       cout << "sense_id = " << sense_id << ", categories_at_level[" << level << "] for word '" << words[word_id] << "' : ";
//       for (set<int>::iterator sit = categories_at_level.begin(); sit != categories_at_level.end(); ++sit)
//         cout << *sit << " ";

      if (categories_at_level.size() != 0)
      {
        // if a list of categories, for a given sense, is already extracted 
        // (through a different sense) mark the sense for deletion
        //bool already_there = !svs.insert(categories_at_level, sense_id);
        bool already_there = (ss.find(categories_at_level) != ss.end());
        if (already_there)
        {
          //cout << "*" << endl;
          senses_to_be_removed.insert(sense_id);
          sense_to_words[sense_id].remove(word_id);
          // if a sense doesn't point to any word anymore, erase it from the sense table
          if (sense_to_words[sense_id].isEmpty())
            sense_to_words.erase(sense_id);
        } else
        {
          ss.insert(categories_at_level);
          //cout << endl;
        }
      } else
      {
        //cout << endl;
      }
    }
    // erase the marked senses
    for (SetIterator it = senses_to_be_removed.begin(); it != senses_to_be_removed.end(); ++it)
    {
      int sense_id = *it;
      word_to_senses[word_id].remove(sense_id);
      word_to_noun_senses[word_id].remove(sense_id);
      word_to_verb_senses[word_id].remove(sense_id);
      word_to_adj_senses[word_id].remove(sense_id);
      word_to_adv_senses[word_id].remove(sense_id);
    }
  }
}

void WordNetOntology::reduceWordPolysemy_preserveSenseOverlapping(int word_id, int level)
{
  Set senses = word_to_senses[word_id];
  Set senses_to_be_removed;
  map<set<int>, Set> categories_to_senses;
  if (senses.size() > 1)
  {
    for (SetIterator it = senses.begin(); it != senses.end(); ++it)
    {
      int sense_id = *it;
      set<int> categories_at_level;
      getCategoriesAtLevel(sense_id, 0, level, categories_at_level);
      if (categories_at_level.size() != 0)
        categories_to_senses[categories_at_level].insert(sense_id);
     }

    for (map<set<int>, Set>::iterator it = categories_to_senses.begin(); it != categories_to_senses.end(); ++it)
    {
      Set sense_cluster = it->second;
      if (sense_cluster.size() > 1)
      {
        int sense_cluster_size = sense_cluster.size();
        int n_sense_removed = 0;
        for (SetIterator sit = sense_cluster.begin(); sit != sense_cluster.end(); ++sit)
        {
          int sense_id = *sit;
          if (sense_to_words[sense_id].size() < 2 && n_sense_removed < (sense_cluster_size - 1))
          {
            senses_to_be_removed.insert(sense_id);
            sense_to_words[sense_id].remove(word_id);
            // if a sense doesn't point to any word anymore, erase it from the sense table
            if (sense_to_words[sense_id].isEmpty())
              sense_to_words.erase(sense_id);
            n_sense_removed++;
          }
        }
      }
    }

      if (!senses_to_be_removed.isEmpty())
      {
        cout << words[word_id] << endl;
//       cout << "senses = " << senses; 
//       cout << ", senses_to_be_removed = " << senses_to_be_removed << endl;
      }

    // erase the marked senses
    for (SetIterator it = senses_to_be_removed.begin(); it != senses_to_be_removed.end(); ++it)
    {
      int sense_id = *it;

      printSynset(sense_id, 1);

      word_to_senses[word_id].remove(sense_id);
      word_to_noun_senses[word_id].remove(sense_id);
      word_to_verb_senses[word_id].remove(sense_id);
      word_to_adj_senses[word_id].remove(sense_id);
      word_to_adv_senses[word_id].remove(sense_id);
    }
  }
}

void WordNetOntology::reduceWordPolysemy_preserveSenseOverlapping2(int word_id, int level)
{
/*
  Set senses = word_to_senses[word_id];
  Set senses_to_be_removed;
  map<int, Set> sense_to_categories_under_level(senses.size());
  if (senses.size() > 1)
  {
    for (SetIterator it = senses.begin(); it != senses.end(); ++it)
    {
      int sense_id = *it;
      Set categories_under_level;
      getCategoriesUnderLevel(sense_id, 0, level, categories_under_level);
      sense_to_categories_under_level[sense_id] = categories_under_level;
    }
    
    if (!senses_to_be_removed.isEmpty())
    {
      //cout << words[word_id] << endl;
      //cout << "senses = " << senses; 
      //cout << ", senses_to_be_removed = " << senses_to_be_removed << endl;
    }
    // erase the marked senses
    for (SetIterator it = senses_to_be_removed.begin(); it != senses_to_be_removed.end(); ++it)
    {
      int sense_id = *it;

      printSynset(sense_id, 1);

      word_to_senses[word_id].remove(sense_id);
      word_to_noun_senses[word_id].remove(sense_id);
      word_to_verb_senses[word_id].remove(sense_id);
      word_to_adj_senses[word_id].remove(sense_id);
      word_to_adv_senses[word_id].remove(sense_id);
    }
  }
*/
}

// remove all the synsets that are not accessible through the word table
// direction is : words -> senses -> categories
void WordNetOntology::removeNonReachableSynsets()
{
  // visit the whole graph, beginning with words, and going upward, and marking the nodes
  for (map<int, Set>::iterator wit = word_to_senses.begin(); wit != word_to_senses.end(); ++wit)
  {
    Set senses = wit->second;
    for (SetIterator sit = senses.begin(); sit != senses.end(); ++sit)
    {
      int sense_id = *sit;
      visitUpward(synsets[sense_id]);
    }
  }
  // mark synsets that need to be removed
  Set synsets_to_be_removed;
  for (map<int, Node*>::iterator sit = synsets.begin(); sit != synsets.end(); ++sit)
  {
    int ss_id = sit->first;
    Node* node = sit->second;
    if (!node->visited)
    {
      synsets_to_be_removed.insert(ss_id);
    } else
    {
      // for a synset that does not need to be removed, check if there are child pointers
      // to a removed one (mark them for deletion if so)
      Set children_to_be_removed;
      for (SetIterator cit = node->children.begin(); cit != node->children.end(); ++cit)
      {
        int child_id = *cit;
        if (!synsets[child_id]->visited)
          children_to_be_removed.insert(child_id);
      }
      // remove the marked child pointers
      for (SetIterator rit = children_to_be_removed.begin(); rit != children_to_be_removed.end(); ++rit)
        node->children.remove(*rit);
    }
  }

  // remove the marked synsets
  for (SetIterator rit = synsets_to_be_removed.begin(); rit != synsets_to_be_removed.end(); ++rit)
  {
    int ss_id = *rit;
    delete(synsets[ss_id]);
    synsets.erase(ss_id);
  }
}

void WordNetOntology::visitUpward(Node* node)
{
  node->visited = true;
  for (SetIterator pit = node->parents.begin(); pit != node->parents.end(); ++pit)
  {
    int parent_id = *pit;
    if (!synsets[parent_id]->visited)
      visitUpward(synsets[parent_id]);
  }
}

void WordNetOntology::detectWordsWithoutOntology()
{
  for (map<int, Set>::iterator it = word_to_senses.begin(); it != word_to_senses.end(); ++it)
  {
    int word_id = it->first;
    Set senses = it->second;
    if (senses.isEmpty())
      PLWARNING("word %d (%s) has no attached ontology", word_id, words[word_id].c_str());
  }
}

int WordNetOntology::getMaxSynsetId()
{
  int max_id = -1;
  for (map<int, Node*>::iterator it = synsets.begin(); it != synsets.end(); ++it)
  {
    int id = it->first;
    if (id > max_id)
      max_id = id;
  }
  return max_id;
}

Set WordNetOntology::getSyntacticClassesForWord(int word_id)
{
#ifndef NOWARNING
  if (!isWord(word_id))
    PLWARNING("asking for a non-word id (%d)", word_id);
#endif
  Set syntactic_classes;
  Set senses = word_to_senses[word_id];
  for (SetIterator it = senses.begin(); it != senses.end(); ++it)
  {
    Node* node = synsets[*it];
    for (SetIterator tit = node->types.begin(); tit != node->types.end(); ++tit)
      syntactic_classes.insert(*tit);
  }
  return syntactic_classes;
}

int WordNetOntology::getSyntacticClassForSense(int sense_id)
{
#ifndef NOWARNING
  if (!isSense(sense_id))
    PLWARNING("asking for a non-sense id (%d)", sense_id);
#endif
  Node* sense = synsets[sense_id];
  if (sense->types.size() > 1)
    PLWARNING("a sense has more than 1 POS type");
  int type = *(sense->types.begin());
  return type;
}

int WordNetOntology::getPredominentSyntacticClassForWord(int word_id)
{
#ifndef NOWARNING
  if (!isWord(word_id))
    PLWARNING("asking for a non-word id (%d)", word_id);
#endif
  if (are_predominent_pos_extracted)
    return word_to_predominent_pos[word_id];
  int n_noun = 0;
  int n_verb = 0;
  int n_adj = 0;
  int n_adv = 0;
  Set senses = word_to_senses[word_id];
  for (SetIterator it = senses.begin(); it != senses.end(); ++it)
  {
    int sense_id = *it;
    int type = getSyntacticClassForSense(sense_id);
    switch (type)
    {
    case NOUN_TYPE:
      n_noun++;
      break;
    case VERB_TYPE:
      n_verb++;
      break;
    case ADJ_TYPE:
      n_adj++;
      break;
    case ADV_TYPE:
      n_adv++;
    }
  }
  if (n_noun == 0 && n_verb == 0 && n_adj == 0 && n_adv == 0)
    return UNDEFINED_TYPE;
  else if (n_noun >= n_verb && n_noun >= n_adj && n_noun >= n_adv)
    return NOUN_TYPE;
  else if (n_verb >= n_noun && n_verb >= n_adj && n_verb >= n_adv)
    return VERB_TYPE;
  else if (n_adj >= n_noun && n_adj >= n_verb && n_adj >= n_adv)
    return ADJ_TYPE;
  else 
    return ADV_TYPE;
}

void WordNetOntology::extractPredominentSyntacticClasses()
{
  for (map<int, Set>::iterator it = word_to_senses.begin(); it != word_to_senses.end(); ++it)
  {
    int word_id = it->first;
    word_to_predominent_pos[word_id] = getPredominentSyntacticClassForWord(word_id);
  }
  are_predominent_pos_extracted = true;
}

void WordNetOntology::savePredominentSyntacticClasses(string file)
{
  ofstream out_pos(file.c_str());
  for (map<int, Set>::iterator it = word_to_senses.begin(); it != word_to_senses.end(); ++it)
  {
    int word_id = it->first;
    out_pos << getPredominentSyntacticClassForWord(word_id) << endl;
    
  }
  out_pos.close();
}

void WordNetOntology::loadPredominentSyntacticClasses(string file)
{
  ifstream in_pos(file.c_str());
  int line_counter = 0;
  while (!in_pos.eof())
  {
    string line = pgetline(in_pos);
    if (line == "") continue;
    int pos = toint(line);
    word_to_predominent_pos[line_counter++] = pos;
  }
  in_pos.close();
  are_predominent_pos_extracted = true;
}

bool WordNetOntology::isTopLevelCategory(int ss_id)
{
  return (ss_id == ROOT_SS_ID || ss_id == SUPER_UNKNOWN_SS_ID || 
          ss_id == NOUN_SS_ID || ss_id == VERB_SS_ID || 
          ss_id == ADJ_SS_ID || ss_id == ADV_SS_ID ||
          ss_id == OOV_SS_ID || ss_id == PROPER_NOUN_SS_ID ||
          ss_id == NUMERIC_SS_ID || ss_id == PUNCTUATION_SS_ID ||
          ss_id == STOP_SS_ID || ss_id == UNDEFINED_SS_ID ||
          ss_id == BOS_SS_ID || ss_id == EOS_SS_ID);
}

int WordNetOntology::extractWordHighLevelSenses(int noun_depth, int verb_depth, int adj_depth, int adv_depth, int unk_depth)
{
  Set noun_categories;
  getDescendantCategoriesAtLevel(NOUN_SS_ID, 0, noun_depth, noun_categories);
  //cout << "|noun categories| = " << noun_categories.size() << endl;
  for (SetIterator sit = noun_categories.begin(); sit != noun_categories.end(); ++sit)
  {
    int ss_id = *sit;
    Set word_descendants = getSynsetWordDescendants(ss_id);
    for (SetIterator wit = word_descendants.begin(); wit != word_descendants.end(); ++wit)
    {
      int word_id = *wit;
      word_to_high_level_senses[word_id].insert(ss_id);
    }
  }
  Set verb_categories;
  getDescendantCategoriesAtLevel(VERB_SS_ID, 0, verb_depth, verb_categories);
  //cout << "|verb categories| = " << verb_categories.size() << endl;
  for (SetIterator sit = verb_categories.begin(); sit != verb_categories.end(); ++sit)
  {
    int ss_id = *sit;
    Set word_descendants = getSynsetWordDescendants(ss_id);
    for (SetIterator wit = word_descendants.begin(); wit != word_descendants.end(); ++wit)
    {
      int word_id = *wit;
      word_to_high_level_senses[word_id].insert(ss_id);
    }
  }
  Set adj_categories;
  getDescendantCategoriesAtLevel(ADJ_SS_ID, 0, adj_depth, adj_categories);
  //cout << "|verb categories| = " << verb_categories.size() << endl;
  for (SetIterator sit = adj_categories.begin(); sit != adj_categories.end(); ++sit)
  {
    int ss_id = *sit;
    Set word_descendants = getSynsetWordDescendants(ss_id);
    for (SetIterator wit = word_descendants.begin(); wit != word_descendants.end(); ++wit)
    {
      int word_id = *wit;
      word_to_high_level_senses[word_id].insert(ss_id);
    }
  }
  Set adv_categories;
  getDescendantCategoriesAtLevel(ADV_SS_ID, 0, adv_depth, adv_categories);
  //cout << "|verb categories| = " << verb_categories.size() << endl;
  for (SetIterator sit = adv_categories.begin(); sit != adv_categories.end(); ++sit)
  {
    int ss_id = *sit;
    Set word_descendants = getSynsetWordDescendants(ss_id);
    for (SetIterator wit = word_descendants.begin(); wit != word_descendants.end(); ++wit)
    {
      int word_id = *wit;
      word_to_high_level_senses[word_id].insert(ss_id);
    }
  }
  Set unk_categories;
  getDescendantCategoriesAtLevel(SUPER_UNKNOWN_SS_ID, 0, unk_depth, unk_categories);
  //cout << "|verb categories| = " << verb_categories.size() << endl;
  for (SetIterator sit = unk_categories.begin(); sit != unk_categories.end(); ++sit)
  {
    int ss_id = *sit;
    Set word_descendants = getSynsetWordDescendants(ss_id);
    for (SetIterator wit = word_descendants.begin(); wit != word_descendants.end(); ++wit)
    {
      int word_id = *wit;
      word_to_high_level_senses[word_id].insert(ss_id);
    }
  }
  
  // integrity verification : to each word should be assigned at least 1 category
  for (map<int, string>::iterator it = words.begin(); it != words.end(); ++it)
  {
    int word_id = it->first;
    if (word_to_high_level_senses[word_id].size() == 0)
      PLWARNING("word '%s' (%d) has no category", words[word_id].c_str(), word_id);
  }

  are_word_high_level_senses_extracted = true;

  n_word_high_level_senses = (noun_categories.size() + verb_categories.size() + adj_categories.size() + adv_categories.size() + unk_categories.size());

  return n_word_high_level_senses;

}

// {non-letters}word{non-letters} -> word
string trimWord(string word)
{
  // trim forward
  int index = 0;
  bool forward_trimmed = isLetter(word[index]) || isDigit(word[index]) || isLegalPunct(word[index]);
  while (!forward_trimmed)
  {
    index++;
    if (index > (int)word.size()) return NULL_TAG;
    forward_trimmed = isLetter(word[index]) || isDigit(word[index]) || isLegalPunct(word[index]);
  }

  word = word.substr(index, word.size());

  // trim backward
  index = word.size() - 1;
  bool backward_trimmed = isLetter(word[index]) || isDigit(word[index]) || isLegalPunct(word[index]);
  while (!backward_trimmed)
  {
    index--;
    if (index < 0) return NULL_TAG;
    backward_trimmed = isLetter(word[index]) || isDigit(word[index]) || isLegalPunct(word[index]);
  }

  string trimmed_word = word.substr(0, index + 1);

  if (trimmed_word == ".")
    return NULL_TAG;
  else
    return trimmed_word;
}

bool isLetter(char c)
{
  return (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
}

bool isDigit(char c)
{
  return (c >= 48 && c <= 57);
}

bool isAlpha(char c)
{
  return (isLetter(c) || isDigit(c));
}

bool isLegalPunct(char c)
{
  return (c == '.' || c == '_');
}

string stemWord(string& word)
{
  //char* input_word = const_cast<char*>(word.c_str());
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
  if (lemma == NULL)
  {
    return word;
  } else
  {
    //cout << word << " -> " << lemma << endl;
    return string(lemma);
  }
}

string stemWord(string& word, int wn_pos)
{
  //char* input_word = const_cast<char*>(word.c_str());
  char* input_word = cstr(word);
  char* lemma = morphword(input_word, wn_pos);
  if (lemma == NULL)
    return word;
  else
    return string(lemma);
}

char* cstr(string& str)
{
  char* cstr = new char[str.size() + 1];
  for (unsigned int i = 0; i < str.size(); i++)
    *(cstr + i) = str[i];
  cstr[str.size()] = '\0';
  return cstr;
}

void removeDelimiters(string& s, char delim, char replace)
{
  unsigned int pos = s.find(delim, 0);
  while (pos != string::npos)
  {
    s.replace(pos, 1, replace);
    pos = s.find(delim, pos + 1);
  }
}

}
