/*! \file TextSenseSequenceVMatrix.cc */

#include "TextSenseSequenceVMatrix.h"

namespace PLearn {
using namespace std;


TextSenseSequenceVMatrix::TextSenseSequenceVMatrix()
  :inherited(),window_size(0), is_supervised_data(true), res_pos(TVec<int>(0)), rand_syn(false), wno(NULL)
  /* ### Initialise all fields to their default value */
{

  // ### You may or may not want to call build_() to finish building the object
  //build_();
}

PLEARN_IMPLEMENT_OBJECT(TextSenseSequenceVMatrix,
                        "VMat class that takes another VMat which contains a sequence (rows) "
                        "of words/sense/POS triplets extracted from a corpus and implements a "
                        "representation of a target word and its context.",
                        "");

void TextSenseSequenceVMatrix::getNewRow(int i, Vec& v) const
{

  if(res_pos.size() != 0)
  {
    getRestrictedRow(i,v);
    return;
  }

  if(i >= dvm->length() || i < 0)
    PLERROR("In TextSenseSequenceVMatrix: requesting %dth row of matrix of length %d", i, dvm->length());
  if(v.length() != 3*(window_size+1))
    PLERROR("In TextSenseSequenceVMatrix: getNewRow v.length() must be equal to VMat's width");

  // Fetch context already in memory

  if(i == my_current_row_index)
  {
    for(int j=0; j<my_current_row.size(); j++)
      v[j] = my_current_row[j];
    if(dvm->width() == 3 && rand_syn)
      permute(v);
    for(int j=0; j<my_current_row.size(); j++)
      my_current_row[j] = v[j];
    return;
  }

  // Fetch context not found in memory

  int context_dist = -(window_size/2);
  int context_count = 0;
  while(context_dist <= window_size/2)
  {
    int context_dist_i = context_dist+i;
    int dist_my_current_row_index = (context_dist_i) - my_current_row_index;
    if(my_current_row_index != -1 && dist_my_current_row_index >= - window_size/2 && dist_my_current_row_index <= window_size/2)
    {
      int index = -1;
      if(dist_my_current_row_index == 0)
        index = window_size;
      if(dist_my_current_row_index < 0)
        index = window_size/2 + dist_my_current_row_index;
      if(dist_my_current_row_index > 0)
        index = window_size/2 + dist_my_current_row_index - 1;
      
      if(context_dist != 0)
      {
        v[3*context_count] = my_current_row[3*index];
        v[3*context_count+1] = my_current_row[3*index+1];
        v[3*context_count+2] = my_current_row[3*index+2];
        context_count++;
      }
      else
      {
        v[3*window_size] = my_current_row[3*index];
        v[3*window_size+1] = my_current_row[3*index+1];
        v[3*window_size+2] = my_current_row[3*index+2];
      }
    }
    else
      if(context_dist_i < 0 || context_dist_i >= dvm->length())
      {
        v[3*context_count] = 0; //oov_tag_id : should'nt be handcoded;              
        v[3*context_count+1] = UNDEFINED_SS_ID;
        v[3*context_count+2] = UNDEFINED_TYPE;
        context_count++;
      }
      else
      {
        if(context_dist == 0)
        {
          if(is_supervised_data)
          {
            Vec temp(3);
            dvm->getRow(i, temp);
            if(temp[0] == SYNSETTAG_ID)
            {
              temp[0] = 0; //oov_tag_id : should'nt be handcoded;
              temp[1] = UNDEFINED_SS_ID;
              temp[2] = UNDEFINED_TYPE;
            }
            v[3*window_size] = temp[0];                
            v[3*window_size+1] = temp[1];
            v[3*window_size+2] = temp[2];
          }
          else
          {
            Vec temp(2);
            dvm->getRow(i, temp);
            if(SYNSETTAG_ID == temp[0])
            {
              temp[0] = 0; //oov_tag_id : should'nt be handcoded;
              temp[1] = UNDEFINED_TYPE;
            }
            v[3*window_size] = temp[0];
            v[3*window_size+1] = UNDEFINED_SS_ID;
            v[3*window_size+2] = temp[1];
          }
          context_dist++;
          continue;
        }

        if(is_supervised_data)
        {
          Vec temp(3);
          dvm->getRow(context_dist_i, temp);
          if(temp[0] == SYNSETTAG_ID)
          {
            temp[0] = 0; //oov_tag_id : should'nt be handcoded;
            temp[1] = UNDEFINED_SS_ID;
            temp[2] = UNDEFINED_TYPE;
          }
          v[3*context_count] = temp[0];               
          v[3*context_count+1] = temp[1];
          v[3*context_count+2] = temp[2];
        }
        else
        {
          Vec temp(2);
          dvm->getRow(context_dist_i, temp);
          if(SYNSETTAG_ID == temp[0])
          {
            temp[0] = 0; //oov_tag_id : should'nt be handcoded;
            temp[1] = UNDEFINED_TYPE;
          }
          v[3*context_count] = temp[0];
          v[3*context_count+1] = UNDEFINED_SS_ID;
          v[3*context_count+2] = temp[1];
        }
        context_count++;
      }
    context_dist++;
  }
  
  if(context_count != window_size)
    PLERROR("What the hell!!!");

  if(dvm->width() == 3 && rand_syn)
    permute(v);
  
  my_current_row_index = i;
  for(int j=0; j<my_current_row.size(); j++)
    my_current_row[j] = v[j];
}

int TextSenseSequenceVMatrix::getRestrictedRow(const int i, Vec v) const
{

  if(i >= dvm->length() || i < 0)
    PLERROR("In TextSenseSequenceVMatrix: requesting %dth row of matrix of length %d", i, dvm.length());
  if(v.length() != 3*(window_size+1))
    PLERROR("In TextSenseSequenceVMatrix: getRestrictedRow v.length() must be equal to VMat's width");
  
  // Initialization of context
  
  for(int j=0; j<window_size; j++)
  {
    v[3*j] = 0; //oov_tag_id : should'nt be handcoded;
    v[3*j+1] = UNDEFINED_SS_ID;
    v[3*j+2] = UNDEFINED_TYPE;
  }

  // Fetch target word

  if(is_supervised_data)
  {
    Vec temp(3);
    dvm->getRow(i, temp);
    if(SYNSETTAG_ID == temp[0])
    {
      temp[0] = 0; //oov_tag_id : should'nt be handcoded;
      temp[1] = UNDEFINED_SS_ID;
      temp[2] = UNDEFINED_TYPE;
    }
    v[3*window_size] = temp[0];
    v[3*window_size+1] = temp[1];
    v[3*window_size+2] = temp[2]; 
  }
  else
  {
    Vec temp(2);
    dvm->getRow(i, temp);
    if(SYNSETTAG_ID == temp[0])
    {
      temp[0] = 0; //oov_tag_id : should'nt be handcoded;
      temp[1] = UNDEFINED_TYPE;
    }
    v[3*window_size] = temp[0];
    v[3*window_size+1] = UNDEFINED_SS_ID;
    v[3*window_size+2] = temp[1];
  }
  

  // Fetch words to the left

  int context_dist = -1;
  int context_found = 0;

  while(context_found != window_size/2 && context_dist+i >=0)
  {
    if(is_supervised_data)
    {
      Vec temp(3);
      dvm->getRow(context_dist+i, temp);
      if(temp[0] == SYNSETTAG_ID)
        break;
      if(!res_pos.contains((int)temp[2]))
      {
        context_found++;
        int index = window_size/2 - context_found;
        v[3*index] = temp[0];
        v[3*index+1] = temp[1];
        v[3*index+2] = temp[2];
      }
    }
    else
    {
      Vec temp(2);
      dvm->getRow(context_dist+i, temp);
      if(temp[0] == SYNSETTAG_ID)
        break;
      if(!res_pos.contains((int)temp[1]))
      {
        context_found++;
        int index = window_size/2 - context_found;
        v[3*index] = temp[0];
        v[3*index+1] = UNDEFINED_SS_ID;;
        v[3*index+2] = temp[1];
      }
    }
    context_dist--;
  }
  
  // Fetch words to the right

  context_dist = 1;
  context_found = window_size/2;

  while(context_found != window_size && context_dist+i < dvm->length())
  {
    if(is_supervised_data)
    {
      Vec temp(3);
      dvm->getRow(context_dist+i, temp);
      if(temp[0] == SYNSETTAG_ID)
        break;
      if(!res_pos.contains((int)temp[2]))
      {
        int index = context_found;
        context_found++;
        v[3*index] = temp[0];
        v[3*index+1] = temp[1];
        v[3*index+2] = temp[2];
      }
    }
    else
    {
      Vec temp(2);
      dvm->getRow(context_dist+i, temp);
      if(temp[0] == SYNSETTAG_ID)
        break;
      if(!res_pos.contains((int)temp[1]))
      {
        int index = context_found;
        context_found++;
        v[3*index] = temp[0];
        v[3*index+1] = UNDEFINED_SS_ID;;
        v[3*index+2] = temp[1];
      }
    }
    context_dist++;
  }

  // Looking for next non-overlapping context

  context_found = 0; 
  while(context_found != window_size/2+1 && context_dist+i < dvm->length())
  {
    if(is_supervised_data)
    {
      Vec temp(3);
      dvm->getRow(context_dist+i, temp);
      if(temp[0] == SYNSETTAG_ID)
      {
        context_dist++;
        continue;
      }
      if(!res_pos.contains((int)temp[2]))
        context_found++;
    }
    else
    {
      Vec temp(2);
      dvm->getRow(context_dist+i, temp);
      if(temp[0] == SYNSETTAG_ID)
      {
        context_dist++;
        continue;
      }
      if(!res_pos.contains((int)temp[1]))
        context_found++;
    }
    context_dist++;
  }
  

  if(dvm->width() == 3 && rand_syn)
    permute(v);

  my_current_row_index = i;
  for(int j=0; j<my_current_row.size(); j++)
    my_current_row[j] = v[j];

  return context_dist+i == dvm->length() ? context_dist+i : context_dist+i-1;
}

void TextSenseSequenceVMatrix::permute(Vec v) const
{
  for(int i=0; i<window_size+1; i++)
  {
    int pos = (int)v[3*i+2];
    if(pos == NOUN_TYPE || pos == VERB_TYPE || pos == ADJ_TYPE || pos == ADV_TYPE)
    {
      real rand = uniform_sample();
      real sum = 0;
      int j=0;
      int sense = (int)v[3*i+1];
      int word_id = (int)v[3*i];
      if(sense >= 0 && word_id >= 0)
      {
        for(; j<word_given_sense_priors[sense].size(); j++)
        {
          if(rand < sum + word_given_sense_priors[sense][j].second)
            break;
          sum += word_given_sense_priors[sense][j].second;
        }
        string word = wno->getWord(word_given_sense_priors[sense][j].first);
        string stemmed_syn = stemWord(word, pos);
        int syn_word_id = wno->getWordId(stemmed_syn);
        if(syn_word_id != -1)
        {
          TVec<int> senses_of_target_word;
          switch (pos)
          {
          case NOUN_TYPE:
            senses_of_target_word = wno->temp_word_to_noun_senses[syn_word_id];
            break;
          case VERB_TYPE:
            senses_of_target_word = wno->temp_word_to_verb_senses[syn_word_id];
            break;
          case ADJ_TYPE:
            senses_of_target_word = wno->temp_word_to_adj_senses[syn_word_id];
            break;
          case ADV_TYPE:
            senses_of_target_word = wno->temp_word_to_adv_senses[syn_word_id];
            break;
          case UNDEFINED_TYPE:
            senses_of_target_word = wno->getSensesForWord(syn_word_id);
            break;
          default:
            //PLERROR("weird in train, target_pos = %d", target_pos);
            senses_of_target_word = wno->getSensesForWord(syn_word_id);
          }
          
        int k=0;
        while(k<senses_of_target_word.size())
        {
          if(senses_of_target_word[k] == (int)v[3*i+1])
            break;
          k++;
        }
        if(k != senses_of_target_word.size())
          v[3*i] = syn_word_id;
        }
      }
    }
  }
}

void TextSenseSequenceVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "window_size", &TextSenseSequenceVMatrix::window_size, OptionBase::buildoption,"Size of the context window");
  declareOption(ol, "is_supervised_data", &TextSenseSequenceVMatrix::is_supervised_data, OptionBase::buildoption,"Data of VMatrix is supervised");
  declareOption(ol, "res_pos", &TextSenseSequenceVMatrix::res_pos, OptionBase::buildoption,"TVec<int> containing the POSs of the words which should not be included in the target word context");
  declareOption(ol, "dvm", &TextSenseSequenceVMatrix::dvm, OptionBase::buildoption,"VMatrix that contains the triplets word/sense/POS of a corpus");
  declareOption(ol, "rand_syn", &TextSenseSequenceVMatrix::rand_syn, OptionBase::buildoption,"Use same-sense random permutation of words");
  inherited::declareOptions(ol);
}

void TextSenseSequenceVMatrix::build_()
{
  if(window_size%2 != 0)
    PLERROR("In TextSenseSequenceVMatrix: window_size must be even number");
  if(window_size < 0)
    PLERROR("In TextSenseSequenceVMatrix: window_size must be non negative");
  if(dvm->width() != 2 && dvm->width() != 3)
    PLERROR("In TextSenseSequenceVMatrix: VMat that_dvm should have width equal to 2 or 3");
  
  width_ = 3*(window_size+1);
  length_ = dvm->length();
  fieldinfos.resize(width_);
  //To do: Field Infos ?

  //oov_tag_id = wno->getWordId(OOV_TAG);

  if(dvm->width() == 2 && rand_syn)
    PLWARNING("In TextSenseSequenceVMatrix: cannot use permutation of same-sense words with unsupervised data");

  if(dvm.isNull())
    PLERROR("In TextSenseSequenceVMatrix: dvm (data of the matrix) is not defined");
  if(dvm->width() == 3 && rand_syn)
  {
    if(wno == NULL)
      PLERROR("In TextSenseSequence: there is no WordNetOntology defined");
    word_given_sense_priors.resize(wno->getSenseSize());
  
    for(int i=0; i<word_given_sense_priors.size(); i++)
    {
      Set words_for_sense = wno->getWordsForSense(i);
      int n_words_for_sense = words_for_sense.size();
      word_given_sense_priors[i]->resize(n_words_for_sense, 1);
      int j=0;
      for(SetIterator sit = words_for_sense.begin(); sit != words_for_sense.end(); sit++,j++)
      {
        word_given_sense_priors[i][j].first = *sit;
        word_given_sense_priors[i][j].second = 1;
      }
    }

    if(dvm.isNull())
      PLERROR("In TextSenseSequenceVMatrix: dvm (data of the matrix) is not defined");
    Vec triplet(3);
    for(int i=0; i<dvm.length(); i++)
    {
      dvm->getRow(i, triplet);
      int sense = (int)triplet[1];
      int word = (int )triplet[0];
      if(sense >= 0 && word >= 0)
      {
        int size = word_given_sense_priors[sense].size();
        for(int j=0; j<size;j++)
          if(word == word_given_sense_priors[sense][j].first)
          {
            word_given_sense_priors[sense][j].second += word_given_sense_priors[sense][j].second == 1 ? 1 : 2;
            break;
          }
      }
    }

    //Normalization
    for(int i=0; i<word_given_sense_priors.size(); i++)
    {
      real sum = 0;
      for(int j=0; j<word_given_sense_priors[i].size(); j++)
        sum += word_given_sense_priors[i][j].second;
      if(sum != 0)
        for(int j=0; j<word_given_sense_priors[i].size(); j++)
          word_given_sense_priors[i][j].second /= sum;
    }
  }
}

void TextSenseSequenceVMatrix::build()
{
  inherited::build();
  build_();
}

void TextSenseSequenceVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(dvm, copies);
  deepCopyField(res_pos,copies);
  // ### Remove this line when you have fully implemented this method.
  //PLERROR("TextSenseSequenceVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn

