// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
// Copyright (C) 2003 Olivier Delalleau
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


/* *******************************************************************    
   * $Id: MissingIndicatorVMatrix.cc 3658 2005-07-06 20:30:15  Godbout $
   ******************************************************************* */


#include "MissingIndicatorVMatrix.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

/** MissingIndicatorVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
  MissingIndicatorVMatrix,
  "VMatrix class to add a missing indicator for each variable.",
  "For each variable with a missing value in the referenced train set, an indicator is added.\n"
  "It is set to 1 if the value of the corresponding variable`in the underlying dataset is missing.\n"
  "It is set to 0 otherwise.\n"
  );

MissingIndicatorVMatrix::MissingIndicatorVMatrix()
: number_of_train_samples_to_use(0.0)
{
}

MissingIndicatorVMatrix::MissingIndicatorVMatrix(VMat the_source, VMat the_train_set, real the_number_of_train_samples_to_use)
{
  source = the_source;
  train_set = the_train_set;
  number_of_train_samples_to_use = the_number_of_train_samples_to_use;
}

MissingIndicatorVMatrix::~MissingIndicatorVMatrix()
{
}

void MissingIndicatorVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "source", &MissingIndicatorVMatrix::source, OptionBase::buildoption, 
                "The source VMatrix with missing values.\n");

  declareOption(ol, "train_set", &MissingIndicatorVMatrix::train_set, OptionBase::buildoption, 
                "A referenced train set.\n"
                "A missing indicator is added for variables with missing values in this data set.\n"
                "It is used in combination with the option number_of_train_samples_to_use\n");

  declareOption(ol, "number_of_train_samples_to_use", &MissingIndicatorVMatrix::number_of_train_samples_to_use, OptionBase::buildoption, 
                "The number of samples from the train set that will be examined to see\n"
                "if an indicator should be added for each variable\n");

  declareOption(ol, "fields", &MissingIndicatorVMatrix::fields, OptionBase::buildoption,
		"The names of the fields to extract if the train_set is not provided.");

  inherited::declareOptions(ol);
}

void MissingIndicatorVMatrix::build()
{
  inherited::build();
  build_();
}

void MissingIndicatorVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  deepCopyField(source, copies);
  deepCopyField(train_set, copies);
  deepCopyField(number_of_train_samples_to_use, copies);
  deepCopyField(source_rel_pos, copies);
  deepCopyField(fields, copies);

  inherited::makeDeepCopyFromShallowCopy(copies);
}

void MissingIndicatorVMatrix::getExample(int i, Vec& input, Vec& target, real& weight)
{
    source->getExample(i, source_input, target, weight);
    for (int source_col = 0, new_col = 0; source_col < source_inputsize;
	 source_col++)
    {
      input[new_col] = source_input[source_col];
      new_col += 1;
      if (source_rel_pos[source_col] < 0)
      {
          if (is_missing(source_input[source_col])) input[new_col] = 1.0;
          else input[new_col] = 0.0;
          new_col += 1;
      }
    }
}

real MissingIndicatorVMatrix::get(int i, int j) const
{
  if (source_rel_pos[j] < 0)
  {
    if (is_missing(source->get(i, source_rel_pos[j - 1]))) return 1.0;
    else return 0.0;
  }
  return source->get(i, source_rel_pos[j]);
}

void MissingIndicatorVMatrix::getColumn(int i, Vec v) const
{
  if (source_rel_pos[i] < 0) source->getColumn(source_rel_pos[i - 1], v);
  else source->getColumn(source_rel_pos[i], v);
  if (source_rel_pos[i] >= 0) return;
  for (int source_row = 0; source_row < v->length(); source_row++)
  {
    if (is_missing(v[source_row])) v[source_row] = 1.0;
    else v[source_row] = 0.0;
  } 
}

void MissingIndicatorVMatrix::build_()
{
    if (!source) PLERROR("In MissingIndicatorVMatrix:: source vmat must be supplied");
    if(source->inputsize()<=0)
      PLERROR("In MissingIndicatorVMatrix::build_() source must have an inputsize <= 0."
	      " inputsize = %d", source->inputsize());
    if(!train_set && !fields){
      train_set = source;
    }
    updateMtime(source);
    if(train_set)
      updateMtime(train_set);
    buildNewRecordFormat(); 
}

void MissingIndicatorVMatrix::buildNewRecordFormat()
{
    source_inputsize = source->inputsize();
    TVec<int> train_var_missing(source_inputsize);
    train_var_missing.clear();
    int source_width = source->width();

    if(train_set){
      int train_length = train_set->length();
      if (number_of_train_samples_to_use > 0.0){
        if (number_of_train_samples_to_use < 1.0) train_length = (int) (number_of_train_samples_to_use * (real) train_length);
        else train_length = (int) number_of_train_samples_to_use;
      }
      if (train_length > train_set->length()) train_length = train_set->length();

      int train_width = train_set->width();
      int train_inputsize = train_set->inputsize();

      if(train_length < 1) 
	PLERROR("In MissingIndicatorVMatrix::length of the number of train"
		" samples to use must be at least 1, got: %i", train_length);
      if(train_inputsize < 1) 
	PLERROR("In MissingIndicatorVMatrix::inputsize of the train vmat must"
		" be supplied, got : %i", train_inputsize);
      if (train_width != source_width) 
	PLERROR("In MissingIndicatorVMatrix::train set and source width must"
		" agree, got : %i, %i", train_width, source_width);
      if (train_set->targetsize() != source->targetsize())
	PLERROR("In MissingIndicatorVMatrix::train set and source targetsize"
		" must agree, got : %i, %i", train_set->targetsize(),
		source->targetsize());
      if (train_set->weightsize() != source->weightsize()) 
	PLERROR("In MissingIndicatorVMatrix::train set and source weightsize"
		" must agree, got : %i, %i", train_set->weightsize(),
		source->weightsize());
      if (train_inputsize != source_inputsize)
	PLERROR("In MissingIndicatorVMatrix::train set and source inputsize"
		" must agree, got : %i, %i", train_inputsize, source_inputsize);

      Vec train_input(train_width);

      for (int train_row = 0; train_row < train_length; train_row++)
	{
	  train_set->getRow(train_row, train_input);
	  for (int train_col = 0; train_col < train_inputsize; train_col++)
	    {
	      if (is_missing(train_input[train_col])) 
		train_var_missing[train_col] = 1;
	    }
	}
    }else if(fields.size()>0){
      TVec<string> source_field_names = source->fieldNames();
      for(int i=0;i<fields.size();i++)
	{
	  int index=source->getFieldIndex(fields[i]);
	  if(index<0)
	    PLERROR("In MissingIndicatorVMatrix::buildNewRecordFormat() - The fields '%s' is not in the source",
		    fields[i].c_str());
	  else
	    train_var_missing[index]=1;
 	}
      
    }else
       PLERROR("In MissingIndicatorVMatrix, the train_set or fields option should be provided.");

    int added_colomns = sum(train_var_missing);
    width_ = source_width + added_colomns;

    TVec<string> source_field_names(source_width);
    source_rel_pos.resize(width_);
    TVec<string> new_field_names(width_);
    source_field_names = source->fieldNames();
    int new_col = 0;
    for (int source_col = 0; source_col < source_inputsize; source_col++)
    {
      new_field_names[new_col] = source_field_names[source_col];
      source_rel_pos[new_col] = source_col;
      new_col += 1;
      if (train_var_missing[source_col] > 0)
      {
          new_field_names[new_col] = source_field_names[source_col] + "_MI";
          source_rel_pos[new_col] = -1;
          new_col += 1;
      }
    }
    for (int source_col = source_inputsize; source_col < source_width; source_col++)
    {
      new_field_names[new_col] = source_field_names[source_col];
      source_rel_pos[new_col] = source_col;
      new_col += 1;
    }
    length_ = source->length();
    inputsize_ = source_inputsize + added_colomns;
    targetsize_ = source->targetsize();
    weightsize_ = source->weightsize();
    source_input.resize(source_inputsize);
    declareFieldNames(new_field_names);
}

} // end of namespcae PLearn
