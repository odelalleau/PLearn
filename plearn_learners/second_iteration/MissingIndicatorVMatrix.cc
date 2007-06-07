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

namespace PLearn {
using namespace std;

/** MissingIndicatorVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
  MissingIndicatorVMatrix,
  "VMat class to add a missing indicator for each variable.",
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
  inherited::makeDeepCopyFromShallowCopy(copies);
}

void MissingIndicatorVMatrix::getExample(int i, Vec& input, Vec& target, real& weight)
{
    source->getExample(i, source_input, target, weight);
    new_col = 0;
    for (int source_col = 0; source_col < source_inputsize; source_col++)
    {
      input[new_col] = source_input[source_col];
      new_col += 1;
      if (train_var_missing[source_col] > 0)
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

void MissingIndicatorVMatrix::put(int i, int j, real value)
{
  PLERROR("In MissingIndicatorVMatrix::put not implemented");
}

void MissingIndicatorVMatrix::getSubRow(int i, int j, Vec v) const
{  
  for (int source_col = j; source_col < j + v.length(); source_col++) v[source_col] = get(i, source_col);
}

void MissingIndicatorVMatrix::putSubRow(int i, int j, Vec v)
{
  PLERROR("In MissingIndicatorVMatrix::putSubRow not implemented");
}

void MissingIndicatorVMatrix::appendRow(Vec v)
{
  PLERROR("In MissingIndicatorVMatrix::appendRow not implemented");
}

void MissingIndicatorVMatrix::insertRow(int i, Vec v)
{
  PLERROR("In MissingIndicatorVMatrix::insertRow not implemented");
}

void MissingIndicatorVMatrix::getRow(int i, Vec v) const
{  
  for (int source_col = 0; source_col < width_; source_col++) v[source_col] = get(i, source_col); 
}

void MissingIndicatorVMatrix::putRow(int i, Vec v)
{
  PLERROR("In MissingIndicatorVMatrix::putRow not implemented");
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
    if (!train_set || !source) PLERROR("In MissingIndicatorVMatrix::train set and source vmat must be supplied");
    buildNewRecordFormat(); 
}

void MissingIndicatorVMatrix::buildNewRecordFormat()
{
    train_length = train_set->length();
    if (number_of_train_samples_to_use > 0.0)
        if (number_of_train_samples_to_use < 1.0) train_length = (int) (number_of_train_samples_to_use * (real) train_length);
        else train_length = (int) number_of_train_samples_to_use;
    if (train_length > train_set->length()) train_length = train_set->length();
    if(train_length < 1) PLERROR("In MissingIndicatorVMatrix::length of the number of train samples to use must be at least 1, got: %i", train_length);
    train_width = train_set->width();
    train_targetsize = train_set->targetsize();
    train_weightsize = train_set->weightsize();
    train_inputsize = train_set->inputsize();
    if(train_inputsize < 1) PLERROR("In MissingIndicatorVMatrix::inputsize of the train vmat must be supplied, got : %i", train_inputsize);
    source_width = source->width();
    source_targetsize = source->targetsize();
    source_weightsize = source->weightsize();
    source_inputsize = source->inputsize();
    if (train_width != source_width) PLERROR("In MissingIndicatorVMatrix::train set and source width must agree, got : %i, %i", train_width, source_width);
    if (train_targetsize != source_targetsize) PLERROR("In MissingIndicatorVMatrix::train set and source targetsize must agree, got : %i, %i", train_targetsize, source_targetsize);
    if (train_weightsize != source_weightsize) PLERROR("In MissingIndicatorVMatrix::train set and source weightsize must agree, got : %i, %i", train_weightsize, source_weightsize);
    if (train_inputsize != source_inputsize) PLERROR("In MissingIndicatorVMatrix::train set and source inputsize must agree, got : %i, %i", train_inputsize, source_inputsize);
    train_input.resize(train_width);
    train_var_missing.resize(train_inputsize);
    train_var_missing.clear();
    for (train_row = 0; train_row < train_length; train_row++)
    {
        train_set->getRow(train_row, train_input);
        for (train_col = 0; train_col < train_inputsize; train_col++)
        {
            if (is_missing(train_input[train_col])) train_var_missing[train_col] = 1;
        }
    }
    new_width = train_width;
    new_inputsize = train_inputsize;
    for (train_col = 0; train_col < train_inputsize; train_col++)
    {
        new_width += train_var_missing[train_col];
        new_inputsize += train_var_missing[train_col];
    }
    train_field_names.resize(train_width);
    source_rel_pos.resize(new_width);
    new_field_names.resize(new_width);
    train_field_names = train_set->fieldNames();
    new_col = 0;
    for (train_col = 0; train_col < train_inputsize; train_col++)
    {
      new_field_names[new_col] = train_field_names[train_col];
      source_rel_pos[new_col] = train_col;
      new_col += 1;
      if (train_var_missing[train_col] > 0)
      {
          new_field_names[new_col] = train_field_names[train_col] + "_MI";
          source_rel_pos[new_col] = -1;
          new_col += 1;
      }
    }
    for (train_col = train_inputsize; train_col < train_width; train_col++)
    {
      new_field_names[new_col] = train_field_names[train_col];
      source_rel_pos[new_col] = train_col;
      new_col += 1;
    }
    source_length = source->length();
    length_ = source_length;
    width_ = new_width;
    inputsize_ = new_inputsize;
    targetsize_ = source_targetsize;
    weightsize_ = train_weightsize;
    source_input.resize(source_inputsize);
    declareFieldNames(new_field_names);
}

} // end of namespcae PLearn
