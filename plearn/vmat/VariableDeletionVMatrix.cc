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


/* *********************************************************************
 * $Id: VariableDeletionVMatrix.cc 3658 2005-07-06 20:30:15  Godbout $
 ********************************************************************* */

#include "VariableDeletionVMatrix.h"

namespace PLearn {
using namespace std;

/** VariableDeletionVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
    VariableDeletionVMatrix,
    "VMat class to select columns from a source VMat based on a given threshold percentage of non-missing variables.",
    "This class will scan the VMat provided as the train set and compute the percentage of non-missing values\n"
    "of each variables. It then only selects the columns with  a percentage of non-missing higher than the given\n"
    "threshod parameter from the complete dataset.\n"
    "Optionnaly, variable with non-missing constant values will also be removed.\n"
    "Note that the ending targets and weight columns are always kept.\n"
    "The targetsize and weightsize of the underlying matrix are kept.\n"
    );

VariableDeletionVMatrix::VariableDeletionVMatrix()
    : deletion_threshold(0),
      remove_columns_with_constant_value(0),
      number_of_train_samples(0.0),
      start_row(0)
{
}

void VariableDeletionVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "complete_dataset", &VariableDeletionVMatrix::complete_dataset, OptionBase::buildoption,
                  "The data set with all variables to select the columns from.");

    declareOption(ol, "train_set", &VariableDeletionVMatrix::train_set, OptionBase::buildoption,
                  "The train set in which to compute the percentage of missing values.");

    declareOption(ol, "deletion_threshold", &VariableDeletionVMatrix::deletion_threshold, OptionBase::buildoption,
                  "The percentage of non-missing values for a variable above which, the variable will be selected.");

    declareOption(ol, "remove_columns_with_constant_value", &VariableDeletionVMatrix::remove_columns_with_constant_value, OptionBase::buildoption,
                  "If set to 1, the columns with constant non-missing values will be removed.");

    declareOption(ol, "number_of_train_samples", &VariableDeletionVMatrix::number_of_train_samples, OptionBase::buildoption,
                  "If equal to zero, all the train samples are used to calculated the percentages and constant values.\n"
                  "If it is a fraction between 0 and 1, this proportion of the samples will be used.\n"
                  "If greater or equal to 1, the integer portion will be interpreted as the number of samples to use.");

    declareOption(ol, "start_row", &VariableDeletionVMatrix::start_row, OptionBase::buildoption,
                  "The row at which, to start to calculate the percentages and constant values.");

    declareOption(ol, "source", &VariableDeletionVMatrix::source, OptionBase::learntoption,
                  "The resulting data set.");

    inherited::declareOptions(ol);
}

void VariableDeletionVMatrix::build()
{
    inherited::build();
    build_();
}

void VariableDeletionVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(complete_dataset, copies);
    deepCopyField(train_set, copies);
    deepCopyField(deletion_threshold, copies);
    deepCopyField(remove_columns_with_constant_value, copies);
    deepCopyField(number_of_train_samples, copies);
    deepCopyField(start_row, copies);
}

void VariableDeletionVMatrix::getExample(int i, Vec& input, Vec& target, real& weight)
{
    source->getExample(i, input, target, weight);
}

real VariableDeletionVMatrix::get(int i, int j) const
{
    return source->get(i, j);
}

void VariableDeletionVMatrix::put(int i, int j, real value)
{
    PLERROR("In VariableDeletionVMatrix::put not implemented");
}

void VariableDeletionVMatrix::getSubRow(int i, int j, Vec v) const
{
    source-> getSubRow(i, j, v);
}

void VariableDeletionVMatrix::putSubRow(int i, int j, Vec v)
{
    PLERROR("In VariableDeletionVMatrix::putSubRow not implemented");
}

void VariableDeletionVMatrix::appendRow(Vec v)
{
    PLERROR("In VariableDeletionVMatrix::appendRow not implemented");
}

void VariableDeletionVMatrix::insertRow(int i, Vec v)
{
    PLERROR("In VariableDeletionVMatrix::insertRow not implemented");
}

void VariableDeletionVMatrix::getRow(int i, Vec v) const
{
    source->getRow(i, v);
}

void VariableDeletionVMatrix::putRow(int i, Vec v)
{
    PLERROR("In VariableDeletionVMatrix::putRow not implemented");
}

void VariableDeletionVMatrix::getColumn(int i, Vec v) const
{
    source->getColumn(i, v);
}

void VariableDeletionVMatrix::build_()
{
    if (!train_set || !complete_dataset) PLERROR("In VariableDeletionVMatrix::train set and complete_dataset vmat must be supplied");
    buildIndices();
}

void VariableDeletionVMatrix::buildIndices()
{
    int train_set_length = train_set->length();
    if(train_set_length < 1) PLERROR("In VariableDeletionVMatrix::length of the number of train samples to use must be at least 1, got: %i", train_set_length);
    int train_set_width = train_set->width();
    int train_set_inputsize = train_set->inputsize();
    if(train_set_inputsize < 1) PLERROR("In VariableDeletionVMatrix::inputsize of the train vmat must be supplied, got : %i", train_set_inputsize);
    int train_set_targetsize = train_set->targetsize();
    int train_set_weightsize = train_set->weightsize();
    int complete_dataset_length = complete_dataset->length();
    int complete_dataset_width = complete_dataset->width();
    int complete_dataset_inputsize = complete_dataset->inputsize();
    int complete_dataset_targetsize = complete_dataset->targetsize();
    int complete_dataset_weightsize = complete_dataset->weightsize();
    if (train_set_width != complete_dataset_width)
        PLERROR("In VariableDeletionVMatrix::train set and complete_dataset width must agree, got : %i, %i", train_set_width, complete_dataset_width);
    if (train_set_inputsize != complete_dataset_inputsize)
        PLERROR("In VariableDeletionVMatrix::train set and complete_dataset inputsize must agree, got : %i, %i", train_set_inputsize, complete_dataset_inputsize);
    if (train_set_targetsize != complete_dataset_targetsize)
        PLERROR("In VariableDeletionVMatrix::train set and complete_dataset targetsize must agree, got : %i, %i", train_set_targetsize, complete_dataset_targetsize);
    if (train_set_weightsize != complete_dataset_weightsize)
        PLERROR("In VariableDeletionVMatrix::train set and complete_dataset weightsize must agree, got : %i, %i", train_set_weightsize, complete_dataset_weightsize);
    int row;
    int col;
    TVec<int>  selected_columns_indices;
    TVec<int>  variable_present_count;
    TVec<real> variable_last_value;
    TVec<bool> variable_observed;
    TVec<bool> variable_constant;
    variable_present_count.resize(complete_dataset_width);
    variable_last_value.resize(complete_dataset_width);
    variable_constant.resize(complete_dataset_width);
    variable_present_count.clear();
    variable_constant.fill(true);
    real variable_value;
    int scanned_length = train_set_length;
    if (number_of_train_samples > 0.0)
    {
        if (number_of_train_samples >= 1.0) scanned_length = (int) number_of_train_samples;
        else scanned_length = (int) ((double) train_set_length * number_of_train_samples);
        if (scanned_length < 1) scanned_length = 1;
    }
    if (start_row + scanned_length > train_set_length)        
        PLERROR("In VariableDeletionVMatrix: start_row + number_of_train_samples must be less or equal to the train set length");
    for (row = start_row; row < start_row + scanned_length; row++)
    {
        for (col = 0; col < train_set_width; col++)
        {
            variable_value = train_set->get(row, col);
            if (!is_missing(variable_value))
            {
                if (variable_present_count[col] > 0)
                {
                    if (variable_value != variable_last_value[col]) variable_constant[col] = false;
                }
                variable_present_count[col] += 1;
                variable_last_value[col] = variable_value;
            }
        }
    }
    real adjusted_threshold = deletion_threshold * (real) scanned_length;
    if (train_set_targetsize < 0) train_set_targetsize = 0;
    if (train_set_weightsize < 0) train_set_weightsize = 0;
    int target_and_weight = train_set_targetsize + train_set_weightsize;
    int new_width = target_and_weight;
    for (col = 0; col < train_set_width - target_and_weight; col++)
    {
        if ((real) variable_present_count[col] > adjusted_threshold && (!remove_columns_with_constant_value || !variable_constant[col])) new_width += 1;
    }
    selected_columns_indices.resize(new_width);
    TVec<string> complete_dataset_names(complete_dataset_width);
    TVec<string> new_names(new_width);
    complete_dataset_names = complete_dataset->fieldNames();
    int selected_col = 0;
    for (col = 0; col < train_set_width - target_and_weight; col++)
    {
        if ((real) variable_present_count[col] > adjusted_threshold && (!remove_columns_with_constant_value || !variable_constant[col]))
        {
            selected_columns_indices[selected_col] = col;
            selected_col += 1;
        }
    }
    if (target_and_weight > 0)
    {
        for (col = train_set_width - target_and_weight; col < train_set_width; col++)
        {
            selected_columns_indices[selected_col] = col;
            selected_col += 1;
        }
    }
    for (col = 0; col < new_width; col++)
    {
        new_names[col] = complete_dataset_names[selected_columns_indices[col]];
    }
    source = new SelectColumnsVMatrix(complete_dataset, selected_columns_indices);
    length_ = complete_dataset_length;
    width_ = new_width;
    inputsize_ = new_width - target_and_weight;
    targetsize_ = complete_dataset_targetsize;
    weightsize_ = complete_dataset_weightsize;
    declareFieldNames(new_names);
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
