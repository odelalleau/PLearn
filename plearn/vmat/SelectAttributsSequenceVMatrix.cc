// -*- C++ -*-

// SelectAttributsSequenceVMatrix.cc
//
// Copyright (C) 2004 Hugo Larochelle 
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
   * $Id: SelectAttributsSequenceVMatrix.cc,v 1.5 2004/10/01 22:11:27 larocheh Exp $ 
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file SelectAttributsSequenceVMatrix.cc */


#include "SelectAttributsSequenceVMatrix.h"

namespace PLearn {
using namespace std;


SelectAttributsSequenceVMatrix::SelectAttributsSequenceVMatrix()
  :inherited(), n_left_context(2), n_right_context(2), conditions_offset(0), conditions_for_exclusion(1), full_context(true),
   target_is_last_element(true), use_last_context(true)
  /* ### Initialise all fields to their default value */
{
}
  
  PLEARN_IMPLEMENT_OBJECT(SelectAttributsSequenceVMatrix,
                          "This VMatrix takes a VMat of a sequence of elements (symbol and attributs) and constructs context rows.",
                          "The elements' symbol and attributs have to be coded using integers, and the source VMat string mapping\n"
                          "is also used with the functions getStringVal(...) and getValString(...).\n"
                          "The context rows can be of fixed length, or constrained by delimiter symbols.\n"
                          "Certain rows can be selected/excluded, and certain elements can be excluded of\n"
                          "a context according to some conditions on the symbol or its attributs.\n"
                          "The conditions are expressed as disjunctions of conjunctions. For example: \n\n"
                          "[ [ 0 : \"person\", 1 : \"NN\" ] , [ 0 : \"kind\", 2 : \"plural\" ] ] \n\n"
                          "is equivalent in C++ logic form to : \n\n"
                          "(fields[0]==\"person\" && fields[1]==\"NN\") || (fields[0]==\"kind\" && fields[2]==\"plural\").\n\n"
                          "Conditions can be expressed in string or int format. The string mapping is used to make the correspondance.\n"
    );
  
void SelectAttributsSequenceVMatrix::getNewRow(int i, const Vec& v) const
{
  if(i<0 || i>=length_) PLERROR("In SelectAttributSequenceVMatrix::getNewRow() : invalid row acces i=%d for VMatrix(%d,%d)",i,length_,width_);
  
  int target = indices[i];

  
  // If the target element is a delimiter, do nothing: Hugo: may not be a good thing!
  source->getRow(target,target_element);
  /*
  if(is_true(delimiters,target_element))
  {
    v.fill(MISSING_VALUE);
    if(use_last_context)
    {
      current_context_pos.clear();
      current_row_i.fill(MISSING_VALUE);
      current_target_pos = -1;
      lower_bound = 1;
      upper_bound = -1;
    }
    return;
  }
  */

  int left_added_elements = 0;
  int right_added_elements = 0;

  int left_pos = 1;
  int right_pos = 1;
  int p = 0;

  bool to_add = false;

  int this_lower_bound = target;
  int this_upper_bound = target;
  /*
  cout << "i=" << i << " target=" << target << endl;
  cout << "current_row_i=" << current_row_i << endl;
  cout << "current_target_pos=" << current_target_pos << endl;
  cout << "lower_bound=" << lower_bound << endl;
  cout << "upper_bound=" << upper_bound << endl;
  cout << "current_context_pos = " << endl;
  for(this_hash_map::iterator it = current_context_pos.begin(); it != current_context_pos.end(); it++)
    cout << it->first << " " << it->second << endl;
  cout << "-------------------------------------" << endl;
  */
  // Left context construction

  while(n_left_context < 0 || left_added_elements < n_left_context)
  {
    to_add = false;
    

    p = target-left_pos;

    if(p < 0 || p >= source->length())
      break;

    // Verifying if context can be found in last context
    if(use_last_context && lower_bound <= p && upper_bound >= p)
    {
      if(current_context_pos.find(p) != current_context_pos.end())
      {
        int position = current_context_pos[p];
        element = current_row_i.subVec(n_attributs*position,n_attributs);
        if(position != current_target_pos || !is_true(ignored_context,element))
        {
          to_add = true;
        }
      }
      else
      {
        left_pos++;
        continue;
      }
    }  // If not, fetch element in source VMat
    else
    {
      source->getRow(p,element);
      if(!is_true(ignored_context,element))
        to_add = true;  
    }
    
    if(is_true(delimiters,element)) break;

    if(to_add)
    {
      left_context.subVec(n_attributs*left_added_elements,n_attributs) << element;
      if(use_last_context) left_positions[left_added_elements] = p;
      this_lower_bound = p;
      left_added_elements++;
    }
    else
      if(!full_context)
      {
        left_context.subVec(n_attributs*left_added_elements,n_attributs).fill(MISSING_VALUE);
        if(use_last_context) left_positions[left_added_elements] = p;
        this_lower_bound = p;
        left_added_elements++;
      }
    
    left_pos++;
  }

  // Right context construction

  while(n_right_context < 0 || right_added_elements < n_right_context)
  {
    to_add = false;

    p = target+right_pos;

    if(p < 0 || p >= source->length())
      break;

    // Verifying if context can be found in last context
    if(use_last_context && lower_bound <= p && upper_bound >= p)
    {
      if(current_context_pos.find(p) != current_context_pos.end())
      {
        int position = current_context_pos[p];
        element = current_row_i.subVec(n_attributs*position,n_attributs);
        if(position != current_target_pos || !is_true(ignored_context,element))
        {
          to_add = true;
        }
      }
      else
      {
        right_pos++;
        continue;
      }
    }  // If not, fetch element in source VMat
    else
    {
      source->getRow(p,element);
      if(!is_true(ignored_context,element))
        to_add = true;  
    }
    
    if(is_true(delimiters,element)) break;

    if(to_add)
    {
      right_context.subVec(n_attributs*right_added_elements,n_attributs) << element;
      if(use_last_context) right_positions[right_added_elements] = p;
      this_upper_bound = p;
      right_added_elements++;
    }
    else
      if(!full_context)
      {
        right_context.subVec(n_attributs*right_added_elements,n_attributs).fill(MISSING_VALUE);
        if(use_last_context) right_positions[right_added_elements] = p;
        this_upper_bound = p;
        right_added_elements++;
      }
    
    right_pos++;
  }

  current_context_pos.clear();
  current_target_pos = -1;
  lower_bound = this_lower_bound;
  upper_bound = this_upper_bound;
  //current_row_i.fill(MISSING_VALUES);

  // Constructing complete row

  int cp = 0;

  if(fixed_context)  // Adding missing value, for the case where the context is of fixed length
    if(n_left_context-left_added_elements>0)
    {
      current_row_i.subVec(cp*n_attributs,n_attributs*(n_left_context-left_added_elements)).fill(MISSING_VALUE);
      cp = n_left_context-left_added_elements;
    }

  // adding left context

  int temp = left_added_elements;

  while(temp > 0)
  {
    temp--;
    current_row_i.subVec(cp*n_attributs,n_attributs) << left_context.subVec(temp*n_attributs,n_attributs);
    if(use_last_context) current_context_pos[(int)left_positions[temp]] = cp;
    cp++;
  }

  // adding target element

  current_row_i.subVec(cp*n_attributs,n_attributs) << target_element;
  if(use_last_context) 
  {
    current_context_pos[target] = cp;
    current_target_pos = cp;
  }
  cp++;

  // adding right context

  int r=0;
  while(r<right_added_elements)
  {
    current_row_i.subVec(cp*n_attributs,n_attributs) << right_context.subVec(r*n_attributs,n_attributs);
    if(use_last_context) current_context_pos[(int)right_positions[r]] = cp;
    r++;
    cp++;
  }

  if(current_row_i.length()-cp*n_attributs > 0)
    current_row_i.subVec(cp*n_attributs,current_row_i.length()-cp*n_attributs).fill(MISSING_VALUE);

  if(target_is_last_element)
  {
    if(fixed_context) 
    {
      left_added_elements = n_left_context;
      right_added_elements = n_right_context;
    }
    v.subVec(0,n_attributs*left_added_elements) << current_row_i.subVec(0,n_attributs*left_added_elements);
    v.subVec(n_attributs*left_added_elements,current_row_i.length() - n_attributs*(1+left_added_elements)) << 
      current_row_i.subVec(n_attributs*(1+left_added_elements),current_row_i.length() - n_attributs*(1+left_added_elements));
    v.subVec(v.length()-n_attributs,n_attributs) << current_row_i.subVec(n_attributs*left_added_elements,n_attributs);
  }
  else
    v << current_row_i;
  
  return;
}

void SelectAttributsSequenceVMatrix::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "n_left_context", &SelectAttributsSequenceVMatrix::n_left_context, OptionBase::buildoption,
                 "Number of elements at the left of the target element (if < 0, all elements to the left are included until a delimiter is met)");
  declareOption(ol, "n_right_context", &SelectAttributsSequenceVMatrix::n_right_context, OptionBase::buildoption,
                 "Number of elements at the right of the target element (if < 0, all elements to the right are included until a delimiter is met)");
  declareOption(ol, "conditions_offset", &SelectAttributsSequenceVMatrix::conditions_offset, OptionBase::buildoption,
                 "Offset for the position of the element on which conditions are tested (default = 0)");
  declareOption(ol, "conditions_for_exclusion", &SelectAttributsSequenceVMatrix::conditions_for_exclusion, OptionBase::buildoption,
                 "Indication that the specified conditions are for the exclusion (true) or inclusion (false) of elements in the VMatrix");
  declareOption(ol, "full_context", &SelectAttributsSequenceVMatrix::full_context, OptionBase::buildoption,
                 "Indication that ignored elements of context should be replaced by the next nearest valid element");
  declareOption(ol, "target_is_last_element", &SelectAttributsSequenceVMatrix::target_is_last_element, OptionBase::buildoption,
                 "Indication that the last element of the context should be the target element");
  declareOption(ol, "use_last_context", &SelectAttributsSequenceVMatrix::use_last_context, OptionBase::buildoption,
                 "Indication that the last accessed context should be put in a buffer.");
  declareOption(ol, "conditions", &SelectAttributsSequenceVMatrix::conditions, OptionBase::buildoption,
                 "Conditions to be satisfied for the exclusion or inclusion (see conditions_for_exclusion) of elements in the VMatrix");
  declareOption(ol, "string_conditions", &SelectAttributsSequenceVMatrix::string_conditions, OptionBase::buildoption,
                 "Conditions, in string format, to be satisfied for the exclusion or inclusion (see conditions_for_exclusion) of elements in the VMatrix");
  declareOption(ol, "delimiters", &SelectAttributsSequenceVMatrix::delimiters, OptionBase::buildoption,
                 "Delimiters of context");
  declareOption(ol, "string_delimiters", &SelectAttributsSequenceVMatrix::string_delimiters, OptionBase::buildoption,
                 "Delimiters, in string format, of context");
  declareOption(ol, "ignored_context", &SelectAttributsSequenceVMatrix::ignored_context, OptionBase::buildoption,
                 "Elements to be ignored in context");
  declareOption(ol, "string_ignored_context", &SelectAttributsSequenceVMatrix::string_ignored_context, OptionBase::buildoption,
                 "Elements, in string format, to be ignored in context");
  declareOption(ol, "source", &SelectAttributsSequenceVMatrix::source, OptionBase::buildoption,
                 "Source VMat, from which contexts are extracted");
  

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void SelectAttributsSequenceVMatrix::build_()
{
  
  if(!source) PLERROR("In SelectAttributSequenceVMatrix::build_() : no source defined");

  n_attributs = source->width();
  row.resize(n_attributs);
  element.resize(n_attributs);
  target_element.resize(n_attributs);

  fixed_context = n_left_context >=0 && n_right_context >= 0;

  // from string to int format on ...

  // conditions
  from_string_to_int_format(string_conditions, conditions);
  string_conditions.clear();

  // delimiters
  from_string_to_int_format(string_delimiters, delimiters);
  string_delimiters.clear();

  // ignored_context
  from_string_to_int_format(string_ignored_context, ignored_context);
  string_ignored_context.clear();

  // gathering information from source VMat

  indices.clear();
  int current_context_length = 0;
  ProgressBar *pb = new ProgressBar("Gathering information from source VMat of length " + tostring(source->length()), source->length());
  for(int i=0; i<source->length(); i++)
  {
    source->getRow(i,row);

    if(is_true(delimiters,row))
    {
      max_context_length = max_context_length < current_context_length ? current_context_length : max_context_length;
      current_context_length = 0;
    }
    else
    {
      if(!full_context || !is_true(ignored_context,row)) current_context_length++;
    }

    // Testing conditions for inclusion/exclusion

    if(i + conditions_offset >= 0 && i + conditions_offset < source->length())
    {
      source->getRow(i+conditions_offset,row);
      if(is_true(conditions,row))
      {
        if(!conditions_for_exclusion) indices.append(i);
      }
      else
      {
        if(conditions_for_exclusion) indices.append(i);
      }
    }
    pb->update(i+1);
  }

  max_context_length = max_context_length < current_context_length ? current_context_length : max_context_length;

  if(n_left_context >= 0 && n_right_context >= 0)
    max_context_length = 1 + n_left_context + n_right_context;

  length_ = indices.length();
  width_ = n_attributs*(max_context_length);

  current_row_i.resize(width_);
  current_target_pos = -1;
  current_context_pos.clear();
  lower_bound = 1;
  upper_bound = -1;

  if(n_left_context < 0) 
  {
    left_context.resize(width_);
    left_positions.resize(max_context_length);
  }
  else 
  {
    left_context.resize(n_attributs*n_left_context);
    left_positions.resize(n_left_context);
  }
  
  if(n_right_context < 0) 
  {
    right_context.resize(width_);
    right_positions.resize(max_context_length);
  }
  else 
  {
    right_context.resize(n_attributs*n_right_context);
    right_positions.resize(n_right_context);
  }
}

void SelectAttributsSequenceVMatrix::build()
{
  inherited::build();
  build_();
}

void SelectAttributsSequenceVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(conditions, copies);
  deepCopyField(delimiters, copies);
  deepCopyField(ignored_context, copies);
  deepCopyField(source, copies);
}

} // end of namespace PLearn

