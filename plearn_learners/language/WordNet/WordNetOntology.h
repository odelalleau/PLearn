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
   * $Id: WordNetOntology.h,v 1.22 2004/04/02 19:27:57 kermorvc Exp $
   * AUTHORS: Christian Jauvin
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef WORD_NET_ONTOLOGY_H
#define WORD_NET_ONTOLOGY_H

#include "wn.h"
#include "general.h"
#include "ShellProgressBar.h"
#include "Set.h"

// #define NOUN_TYPE 1000
// #define VERB_TYPE 1001
// #define ADJ_TYPE 1002
// #define ADV_TYPE 1003
// #define UNDEFINED_TYPE 1004
// #define ALL_WN_TYPE 1005
// #define NUMERIC_TYPE 1006
// #define PROPER_NOUN_TYPE 1007

#define NOUN_TYPE 1
#define VERB_TYPE 2
#define ADJ_TYPE 3
#define ADV_TYPE 4
#define ADJ_SATELLITE_TYPE 5
#define ALL_WN_TYPE 6
#define UNDEFINED_TYPE 7
#define NUMERIC_TYPE 8
#define PROPER_NOUN_TYPE 9

#define SYNSETTAG_ID -2

#define UNDEFINED_SS_ID -1
#define ROOT_SS_ID 0
#define SUPER_UNKNOWN_SS_ID 1 // 'unknown' means "out of WordNet"
#define NOUN_SS_ID 2
#define VERB_SS_ID 3
#define ADJ_SS_ID 4
#define ADV_SS_ID 5
#define OOV_SS_ID 6 // out-of-vocabulary
#define PROPER_NOUN_SS_ID 7
#define NUMERIC_SS_ID 8
#define PUNCTUATION_SS_ID 9
#define STOP_SS_ID 10
#define BOS_SS_ID 11
#define EOS_SS_ID 12

#define UNDEFINED_OFFSET -1
#define ROOT_OFFSET -2
#define SUPER_UNKNOWN_OFFSET -3 // 'unknown' means "out of WordNet"
#define NOUN_OFFSET -4
#define VERB_OFFSET -5
#define ADJ_OFFSET -6
#define ADV_OFFSET -7
#define OOV_OFFSET -8 // out-of-vocabulary
#define PROPER_NOUN_OFFSET -9
#define NUMERIC_OFFSET -10
#define PUNCTUATION_OFFSET -11
#define STOP_OFFSET -12
#define BOS_OFFSET -13
#define EOS_OFFSET -14

#define SUPER_FNUM -1

#define NULL_TAG "<null>"

#define OOV_TAG "<oov>"
#define PROPER_NOUN_TAG "<proper_noun>"
#define NUMERIC_TAG "<numeric>"
#define PUNCTUATION_TAG "<punctuation>"
#define STOP_TAG "<stop>"
#define BOS_TAG "<s>"
#define EOS_TAG "</s>"

#define VERB_TAG "<verb>"
#define NOUN_TAG "<noun>"
#define ADJ_TAG "<adj>"
#define ADV_TAG "<adv>"
#define UNDEFINED_TAG "<undefined>"

#define WNO_ERROR -1000

#define WORD_COVERAGE_THRESHOLD 10

// terminology :
//     word
//     sense : word meaning
//     category : concepts (forming an ontology DAG)
//     synset : sense U category
//
namespace PLearn {

// utils
string trimWord(string word);
string stemWord(string& word); // call to WN morphword()
string stemWord(string& word, int wn_pos);
bool isLetter(char c);
bool isDigit(char c);
bool isAlpha(char c);
bool isLegalPunct(char c);
char* cstr(string& s);
void removeDelimiters(string& s, string delim, string replace);
bool startsWith(string& base, string s);
void replaceChars(string& str, string char_to_replace, string replacing_char);

// ontology DAG node

struct Node
{
  Node() { ss_id = UNDEFINED_SS_ID; is_unknown = true; visited = false; fnum = SUPER_FNUM; hereiam = 0;}
  Node(int id) { ss_id = id; is_unknown = true; visited = false; fnum = SUPER_FNUM; hereiam = 0;}
  int ss_id;
  Set types;
  string gloss;
  vector<string> syns;
  Set parents;
  Set children;
  bool is_unknown;
  //int level;
  bool visited;
  long hereiam;
  int fnum;
};

class WordNetOntology
{

protected:

  // main ontology structure access points
  map<int, Set> word_to_senses;
  map<int, Set> word_to_noun_senses;
  map<int, Set> word_to_verb_senses;
  map<int, Set> word_to_adj_senses;
  map<int, Set> word_to_adv_senses;
  map<int, Set> sense_to_words;
  map<int, Set> synset_to_ancestors;
  map<int, Set> word_to_ancestors;
  map<int, Set> synset_to_sense_descendants;
  map<int, Set> synset_to_word_descendants;
  map<int, Node*> synsets;
  map<int, string> words;
  map<string, int> words_id;
  map<int, vector<int> > word_to_noun_wnsn;
  map<int, vector<int> > word_to_verb_wnsn;
  map<int, vector<int> > word_to_adj_wnsn;
  map<int, vector<int> > word_to_adv_wnsn;
  map<int, int> word_to_predominent_pos;
  map<int, bool> word_is_in_wn;
  map<int, Set> word_to_high_level_senses;
  map<pair<int, int>, int> word_sense_to_unique_id;
  map<int, Set> word_to_under_target_level_high_level_senses; // BIG HACK!!!
  map< pair<int, string>,int> sense_key_to_ss_id;
  map<pair<int,int>, string> ws_id_to_sense_key;

  int word_index; // unique id for words
  int synset_index; // unique id for synsets
  int unknown_sense_index; // unique id for unknown senses
  
  // stats
  int noun_count;
  int verb_count;
  int adj_count ;
  int adv_count;
  
  int noun_sense_count;
  int verb_sense_count;
  int adj_sense_count;
  int adv_sense_count;
  
  int in_wn_word_count;
  int out_of_wn_word_count;

  // these flags are set to 'true' when the corresponding data is pre-computed
  bool are_ancestors_extracted;
  bool are_descendants_extracted;
  bool are_predominent_pos_extracted;
  bool are_word_high_level_senses_extracted;
  bool are_word_sense_unique_ids_computed;

  int n_word_high_level_senses;

  // If 'differentiate_unknown_words' is set to 'true', all the unknown words (words that are 
  // out of WordNet) will be mapped to DIFFERENT synsets (senses), that are all going to be linked 
  // to a SUPER-UNKNOWN synset node (a direct child of the ROOT synset). If it is set to 'false',
  // all the unknown words will be mapped to a single synset, SUPER-UNKNOWN, acting in this 
  // context as a SENSE, and a direct child of the ROOT synset.
  bool differentiate_unknown_words;

public:

  WordNetOntology();                                       // simply init the system, and load an ontology later

  WordNetOntology(string voc_file,                         // build a new ontology, given a voc file
                  bool differentiate_unknown_words,
                  bool pre_compute_ancestors,
                  bool pre_compute_descendants,
                  int wn_pos_type = ALL_WN_TYPE,
                  int word_coverage_threshold = -1);

  WordNetOntology(string voc_file,                         // init the system and load an ontology, 
                  string synset_file,                      // given a voc file, a synset file and an ontology file
                  string ontology_file,
                  bool pre_compute_ancestors,
                  bool pre_compute_descendants,
                  int word_coverage_threshold = -1);

  WordNetOntology(string voc_file,                         // init the system and load an ontology, 
                  string synset_file,                      // given a voc file, a synset file and an ontology file
                  string ontology_file,
                  string sense_key_file,
                  bool pre_compute_ancestors,
                  bool pre_compute_descendants,
                  int word_coverage_threshold = -1);

  void save(string synset_file, string ontology_file);
  void save(string voc_file);
  void saveVocInWordnet(string voc_file);
  void save(string synset_file, string ontology_file, string sense_key_file);
  void load(string voc_file, string synset_file, string ontology_file);
  void load(string voc_file, string synset_file, string ontology_file, string sense_key_file);
  void savePredominentSyntacticClasses(string file);
  void loadPredominentSyntacticClasses(string file);

  // main access methods
  string getSenseKey(int word_id, int ss_id);
  int getSynsetIDForSenseKey(int word_id, string sense_key);
  int getWordId(string word);
  string getWord(int id);
  int getWordSenseIdForWnsn(string word, int wn_pos_type, int wnsn);
  int getWordSenseIdForSenseKey(string lemma, string lexsn, string word);
  int getWordSenseUniqueId(int word, int sense);
  int getWordSenseUniqueIdSize();
  Set getWordSenses(int id);
  Set getWordHighLevelSenses(int id);
  Set getWordNounSenses(int id);
  Set getWordVerbSenses(int id);
  Set getWordAdjSenses(int id);
  Set getWordAdvSenses(int id);
  Set getWordsForSense(int id);
  Set getSynsetAncestors(int id, int max_level = -1);
  Set getSynsetParents(int id);
  Set getWordAncestors(int id, int max_level = -1);
  Set getSynsetSenseDescendants(int id);
  Set getSynsetWordDescendants(int id);
  Node* getSynset(int id);
  Node* getRootSynset() { return synsets[ROOT_SS_ID]; }
  Set getAllWords();
  Set getAllSenses();
  Set getAllCategories();
  int getVocSize() { return words.size(); }
  int getSenseSize() { return sense_to_words.size(); }
  int getSynsetSize() { return synsets.size(); }
  int getMaxSynsetId();
  Set getSyntacticClassesForWord(int word_id);
  int getSyntacticClassForSense(int sense_id);
  int getPredominentSyntacticClassForWord(int word_id);
  void getDescendantCategoriesAtLevel(int ss_id, int cur_level, int target_level, Set categories);
  void getDownToUpParentCategoriesAtLevel(int ss_id, int target_level, Set categories, int cur_level = 0);
  
  
  bool isWord(int id);
  bool isWord(string word);
  bool isSense(int id); // is a correct sens id
  bool isPureSense(int id);// is a synset but not a category
  bool isCategory(int id); // = isSynset
  bool isPureCategory(int id);// is a synset but not a sense 
  bool isSynset(int id); // is a synset (sense or category)
  bool isWordUnknown(string word);
  bool isWordUnknown(int id);
  bool isSynsetUnknown(int id);
  bool isInWordNet(string word, bool trim_word = true, bool stem_word = true, bool remove_undescores = false);
  bool isInWordNet(int word_id);
  bool hasSenseInWordNet(string word, int wn_pos_type);
  bool isTopLevelCategory(int ss_id);
  bool containsWord(string word) { return (words_id.find(word) != words_id.end()); }
  bool containsWordId(int id) { return (words.find(id) != words.end()); }

  Node *findSynsetFromSynsAndGloss(const vector<string> &syns, const string &gloss, const long offset, const int fnum);
  void removeNonReachableSynsets();
  void removeWord(int id);

  void print(bool print_ontology = true);
  void printSynset(int ss_id, int indent_level = 0);
  void printSynset(int ss_id, ostream& sout, int indent_level = 0);
  void printStats();
  void printSynsetAncestors();
  void printWordAncestors();
  void printDescendants();
  void printNodes();
  void printWordOntology(int id);
  void printWordOntology(string word);
  void printInvertedSynsetOntology(int id, int level = 0);

  int overlappingSynsets(int ss_id1, int ss_id2);
  bool areOverlappingSynsets(int ss_id1, int ss_id2) { return (overlappingSynsets(ss_id1, ss_id2) > 1); }
  void intersectAncestorsAndSenses(Set categories, Set senses);
  void reducePolysemy(int level);
  void extractPredominentSyntacticClasses();
  void extractWordHighLevelSenses(int noun_depth, int verb_depth, int adj_depth, int adv_depth, int unk_depth);
  void extractWordNounAndVerbHighLevelSenses(int noun_depth, int verb_depth);
  
  // integrity verifications
  void detectWordsWithoutOntology();
  void lookForSpecialTags();

  void extract(string voc_file, int wn_pos_type);
  void extractWord(string original_word, int wn_pos_type, bool trim_word, bool stem_word, bool remove_underscores);
  bool extractSenses(string original_word, string processed_word, int wn_pos_type);
  void extractTaggedWordFrequencies(map<int, map<int, int> > &word_senses_to_tagged_frequencies);
//int extractFrequencies(string word, int sense, int wn_pos_type);
  Node* extractOntology(SynsetPtr ssp);
  void extractAncestors(int threshold, bool cut_with_word_coverage, bool exclude_itself);
  void extractAncestors(Node* node, Set ancestors, int level, int level_threshold);
  void extractAncestors(Node* node, Set ancestors, int word_coverage_threshold);
  void extractDescendants(Node* node, Set sense_descendants, Set word_descendants);
  void extractStrictDescendants(Node* node, Set sense_descendants, Set word_descendants);
  void extractDescendants();
  void computeWordSenseUniqueIds();
  void init(bool differentiate_unknown_words = true);
  void createBaseSynsets();
  void processUnknownWord(int word_id);
  void finalize();
  void propagatePOSTypes();
  void propagatePOSTypes(Node* node);
  void linkUpperCategories();
  //void setLevels();
  //void setLevels(int ss_id, int level);
  Node* checkForAlreadyExtractedSynset(SynsetPtr ssp);
  vector<string> getSynsetWords(SynsetPtr ssp);
  bool catchSpecialTags(string word);
  void reduceWordPolysemy(int word_id, int level);
  void reduceWordPolysemy_preserveSenseOverlapping(int word_id, int level);
  void reduceWordPolysemy_preserveSenseOverlapping2(int word_id, int level);
  //void getCategoriesAtLevel(int ss_id, int level, set<int>& categories);
  void getCategoriesAtLevel(int ss_id, int cur_level, int target_level, set<int>& categories);
  void getCategoriesUnderLevel(int ss_id, int cur_level, int target_level, Set categories);
  void visitUpward(Node* node);
  void unvisitDownward(Node *node);
  void unvisitAll();
  void printOntology(Node* node, int level = 0);

  // ATTENTION: il y a un systeme de mapping word->senses temporaire
  // et base sur TVec<int>, qui sert uniquement dans le contexte de la WSD 
  // (SemiSupervisedSparseDataNeuralNet). Le but a moyen terme est de remplacer 
  // tous les mappings bases sur Set dans WordNetOntology pour les faire reposer 
  // sur TVec<int>, plus PLearn-compliant.
  map<int, TVec<int> > temp_word_to_senses;
  map<int, TVec<int> > temp_word_to_noun_senses;
  map<int, TVec<int> > temp_word_to_verb_senses;
  map<int, TVec<int> > temp_word_to_adj_senses;
  map<int, TVec<int> > temp_word_to_adv_senses;
  map<int, TVec<int> > temp_word_to_high_level_senses;

  void fillTempWordToSensesTVecMap()
  {
    for (map<int, Set>::iterator it = word_to_senses.begin(); it != word_to_senses.end(); ++it)
    {
      int w = it->first;
      Set senses = it->second;
      for (SetIterator sit = senses.begin(); sit != senses.end(); ++sit)
        temp_word_to_senses[w].push_back(*sit);
    }

    for (map<int, Set>::iterator it = word_to_noun_senses.begin(); it != word_to_noun_senses.end(); ++it)
    {
      int w = it->first;
      Set senses = it->second;
      for (SetIterator sit = senses.begin(); sit != senses.end(); ++sit)
        temp_word_to_noun_senses[w].push_back(*sit);
    }

    for (map<int, Set>::iterator it = word_to_verb_senses.begin(); it != word_to_verb_senses.end(); ++it)
    {
      int w = it->first;
      Set senses = it->second;
      for (SetIterator sit = senses.begin(); sit != senses.end(); ++sit)
        temp_word_to_verb_senses[w].push_back(*sit);
    }

    for (map<int, Set>::iterator it = word_to_adj_senses.begin(); it != word_to_adj_senses.end(); ++it)
    {
      int w = it->first;
      Set senses = it->second;
      for (SetIterator sit = senses.begin(); sit != senses.end(); ++sit)
        temp_word_to_adj_senses[w].push_back(*sit);
    }

    for (map<int, Set>::iterator it = word_to_adv_senses.begin(); it != word_to_adv_senses.end(); ++it)
    {
      int w = it->first;
      Set senses = it->second;
      for (SetIterator sit = senses.begin(); sit != senses.end(); ++sit)
        temp_word_to_adv_senses[w].push_back(*sit);
    }
  }

  TVec<int> getSensesForWord(int w) { return temp_word_to_senses[w]; }

  void fillTempWordToHighLevelSensesTVecMap()
  {
    for (map<int, string>::iterator it = words.begin(); it != words.end(); ++it)
    {
      int w = it->first;
      Set hl_senses = getWordHighLevelSenses(w);
      for (SetIterator sit = hl_senses.begin(); sit != hl_senses.end(); ++sit)
        temp_word_to_high_level_senses[w].push_back(*sit);
    }
  }
  TVec<int> getHighLevelSensesForWord(int w) { return temp_word_to_high_level_senses[w]; }

  TVec<int> getSecondLevelSensesForWord(int w)
  {
    Set sl_senses;
    Set senses = word_to_senses[w];
    for (SetIterator sit = senses.begin(); sit != senses.end(); ++sit)
    {
      int s = *sit;
      Node* node = synsets[s];
      for (SetIterator ssit = node->parents.begin(); ssit != node->parents.end(); ++ssit)
      {
        sl_senses.insert(*ssit);
      }
    }
    TVec<int> sl_senses_vec;
    for (SetIterator slit = sl_senses.begin(); slit != sl_senses.end(); ++slit)
      sl_senses_vec.push_back(*slit);
    return sl_senses_vec;
  }

  TVec<int> getThirdLevelSensesForWord(int w) 
  {
    Set tl_senses;
    Set senses = word_to_senses[w];
    for (SetIterator sit = senses.begin(); sit != senses.end(); ++sit)
    {
      int s = *sit;
      Node* node = synsets[s];
      for (SetIterator slit = node->parents.begin(); slit != node->parents.end(); ++slit)
      {
        int sl_sense = *slit;
        Node* node = synsets[sl_sense];
        for (SetIterator tlit = node->parents.begin(); tlit != node->parents.end(); ++tlit)
        {
          tl_senses.insert(*tlit);
        }
      }
    }
    TVec<int> tl_senses_vec;
    for (SetIterator tlit = tl_senses.begin(); tlit != tl_senses.end(); ++tlit)
      tl_senses_vec.push_back(*tlit);
    return tl_senses_vec;
  }

};

}

#endif
