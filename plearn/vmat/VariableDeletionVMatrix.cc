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
    "This class will scan the VMat provided as the complete dataset and compute the percentage of non-missing values\n"
    "of each variables. It then only selects the columns with  a percentage of non-missing higher than the given\n"
    "threshod parameter.\n"
    "Optionnaly, variable with non-missing constant values will also be removed.\n"
    "Note that the ending targets and weight columns are always kept.\n"
    "The targetsize and weightsize of the underlying matrix are kept.\n"
    );

VariableDeletionVMatrix::VariableDeletionVMatrix() 
    : obtained_inputsize_from_source(false),
      obtained_targetsize_from_source(false),
      obtained_weightsize_from_source(false),
      deletion_threshold(0),
      remove_columns_with_constant_value(0),
      number_of_train_samples(0.0)
{
}

VariableDeletionVMatrix::VariableDeletionVMatrix(VMat the_complete_dataset, real the_threshold, bool the_remove_columns_with_constant_value, real the_number_of_train_samples)
    : obtained_inputsize_from_source(false),
      obtained_targetsize_from_source(false),
      obtained_weightsize_from_source(false)
{
    complete_dataset = the_complete_dataset;
    deletion_threshold = the_threshold;
    remove_columns_with_constant_value = the_remove_columns_with_constant_value;
    number_of_train_samples = the_number_of_train_samples;
    build();
}

void VariableDeletionVMatrix::declareOptions(OptionList &ol)
{

    declareOption(ol, "complete_dataset", &VariableDeletionVMatrix::complete_dataset, OptionBase::buildoption, 
                  "The data set with all variables to select the columns from.");

    declareOption(ol, "deletion_threshold", &VariableDeletionVMatrix::deletion_threshold, OptionBase::buildoption, 
                  "The percentage of non-missing values for a variable above which, the variable will be selected.");

    declareOption(ol, "remove_columns_with_constant_value", &VariableDeletionVMatrix::remove_columns_with_constant_value, OptionBase::buildoption,
                  "If set to 1, the columns with constant non-missing values will be removed.");
      
    declareOption(ol, "number_of_train_samples", &VariableDeletionVMatrix::number_of_train_samples, OptionBase::buildoption, 
                  "If equal to zero, all the underlying dataset samples are used to calculated the percentages and constant values.\n"
                  "If it is a fraction between 0 and 1, this proportion of the samples will be used.\n"
                  "If greater or equal to 1, the integer portion will be interpreted as the number of samples to use.");

    declareOption(ol, "obtained_inputsize_from_source", &VariableDeletionVMatrix::obtained_inputsize_from_source, OptionBase::learntoption, 
                  "Set to 1 when the inputsize was obtained from the source matrix.");

    declareOption(ol, "obtained_targetsize_from_source", &VariableDeletionVMatrix::obtained_targetsize_from_source, OptionBase::learntoption, 
                  "Set to 1 when the targetsize was obtained from the source matrix.");

    declareOption(ol, "obtained_weightsize_from_source", &VariableDeletionVMatrix::obtained_weightsize_from_source, OptionBase::learntoption, 
                  "Set to 1 when the weightsize was obtained from the source matrix.");

    inherited::declareOptions(ol);
}

void VariableDeletionVMatrix::build()
{
    buildIndices();
    inherited::build();
    build_();
}

void VariableDeletionVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(complete_dataset, copies);
    deepCopyField(deletion_threshold, copies);
    deepCopyField(remove_columns_with_constant_value, copies);
    deepCopyField(obtained_inputsize_from_source, copies);
    deepCopyField(obtained_targetsize_from_source, copies);
    deepCopyField(obtained_weightsize_from_source, copies);
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
    source-> getRow(i, v);
}

void VariableDeletionVMatrix::putRow(int i, Vec v)
{
    PLERROR("In VariableDeletionVMatrix::putRow not implemented");
}

void VariableDeletionVMatrix::getColumn(int i, Vec v) const
{  
    source-> getColumn(i, v);
}

void VariableDeletionVMatrix::build_()
{
    if (source) {
        string error_msg =
            "In VariableDeletionVMatrix::build_ - For safety reasons, it is forbidden to "
            "re-use sizes obtained from a previous source VMatrix with a new source "
            "VMatrix having different sizes";
        length_ = source->length();
        width_ = source->width();
        if(inputsize_<0) {
            inputsize_ = source->inputsize();
            obtained_inputsize_from_source = true;
        } else if (obtained_inputsize_from_source && inputsize_ != source->inputsize())
            PLERROR(error_msg.c_str());
        if(targetsize_<0) {
            targetsize_ = source->targetsize();
            obtained_targetsize_from_source = true;
        } else if (obtained_targetsize_from_source && targetsize_ != source->targetsize())
            PLERROR(error_msg.c_str());
        if(weightsize_<0) {
            weightsize_ = source->weightsize();
            obtained_weightsize_from_source = true;
        } else if (obtained_weightsize_from_source && weightsize_ != source->weightsize())
            PLERROR(error_msg.c_str());
        fieldinfos = source->fieldinfos;
    } else {
        // Restore the original undefined sizes if the current one had been obtained
        // from the source VMatrix.
        if (obtained_inputsize_from_source) {
            inputsize_ = -1;
            obtained_inputsize_from_source = false;
        }
        if (obtained_targetsize_from_source) {
            targetsize_ = -1;
            obtained_targetsize_from_source = false;
        }
        if (obtained_weightsize_from_source) {
            weightsize_ = -1;
            obtained_weightsize_from_source = false;
        }
    }
}

void VariableDeletionVMatrix::buildIndices()
{
    int complete_dataset_length = complete_dataset->length();
    int complete_dataset_width = complete_dataset->width();
    int complete_dataset_targetsize = complete_dataset->targetsize();
    int complete_dataset_weightsize = complete_dataset->weightsize();
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
    for (col = 0; col < complete_dataset_width; col++)
    {
        variable_present_count[col] = 0;
        variable_constant[col] = true;
    }
    real variable_value;
    int scanned_length = complete_dataset_length;
    if (number_of_train_samples > 0.0)
    {
        if (number_of_train_samples >= 1.0) scanned_length = (int) number_of_train_samples;
        else scanned_length = (int) ((double) complete_dataset_length * number_of_train_samples);
        if (scanned_length < 1) scanned_length = 1;
        if (scanned_length > complete_dataset_length) scanned_length = complete_dataset_length;
    }
    for (row = 0; row < scanned_length; row++)
    {
        for (col = 0; col < complete_dataset_width; col++)
        {
            variable_value = complete_dataset->get(row, col);
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
    real adjusted_threshold = deletion_threshold * (real) complete_dataset_length / 100.0;
    if (complete_dataset_targetsize < 0) complete_dataset_targetsize = 0;
    if (complete_dataset_weightsize < 0) complete_dataset_weightsize = 0;
    int target_and_weight = complete_dataset_targetsize + complete_dataset_weightsize;
    int new_width = target_and_weight;
    for (col = 0; col < complete_dataset_width - target_and_weight; col++)
    {
        if ((real) variable_present_count[col] > adjusted_threshold && (!remove_columns_with_constant_value || !variable_constant[col])) new_width += 1;
    }
    selected_columns_indices.resize(new_width);  
    int selected_col = 0;
    for (col = 0; col < complete_dataset_width - target_and_weight; col++)
    {
        if ((real) variable_present_count[col] > adjusted_threshold && (!remove_columns_with_constant_value || !variable_constant[col]))
        {
            selected_columns_indices[selected_col] = col;
            selected_col += 1;
        }
    }
    if (target_and_weight > 0)
    {
        for (col = complete_dataset_width - target_and_weight; col < complete_dataset_width; col++)
        {
            selected_columns_indices[selected_col] = col;
            selected_col += 1;
        }
    }
    source = new SelectColumnsVMatrix(complete_dataset, selected_columns_indices);
    source->defineSizes(new_width - target_and_weight, complete_dataset_targetsize, complete_dataset_weightsize);
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
