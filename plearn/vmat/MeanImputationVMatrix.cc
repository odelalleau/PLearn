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
 * $Id: MeanImputationVMatrix.cc 3658 2005-07-06 20:30:15  Godbout $
 ******************************************************************* */


#include "MeanImputationVMatrix.h"

namespace PLearn {
using namespace std;

/** MeanImputationVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
    MeanImputationVMatrix,
    "VMat class to impute the observed variable mean to replace missing values in the source matrix.",
    "This class will replace missing values in the underlyng dataset with the observed mean of a variable.\n"
    "If the number_of_train_samples option is different than zero, the mean is calculated only on that first portion of the underlying VMat.\n"
    "Otherwise, the mean is calculated on the entire dataset.\n"
    );

MeanImputationVMatrix::MeanImputationVMatrix()
    : obtained_inputsize_from_source(false),
      obtained_targetsize_from_source(false),
      obtained_weightsize_from_source(false),
      number_of_train_samples(0.0)
{
}

MeanImputationVMatrix::MeanImputationVMatrix(VMat the_source, real the_number_of_train_samples)
    : obtained_inputsize_from_source(false),
      obtained_targetsize_from_source(false),
      obtained_weightsize_from_source(false)
{
    source = the_source;
    number_of_train_samples = the_number_of_train_samples;
    build_();
}

MeanImputationVMatrix::~MeanImputationVMatrix()
{
}

void MeanImputationVMatrix::declareOptions(OptionList &ol)
{

    declareOption(ol, "number_of_train_samples", &MeanImputationVMatrix::number_of_train_samples, OptionBase::buildoption,
                  "If equal to zero, all the underlying dataset samples are used to calculated the variable means.\n"
                  "If it is a fraction between 0 and 1, this proportion of the samples will be used.\n"
                  "If greater or equal to 1, the integer portion will be interpreted as the number of samples to use.");

    declareOption(ol, "variable_mean", &MeanImputationVMatrix::variable_mean, OptionBase::learntoption,
                  "The vector of variable means observed from the source matrix.");

    declareOption(ol, "variable_present_count", &MeanImputationVMatrix::variable_present_count, OptionBase::learntoption,
                  "The vector of non missing variable counts from the source matrix.");

    declareOption(ol, "obtained_inputsize_from_source", &MeanImputationVMatrix::obtained_inputsize_from_source, OptionBase::learntoption,
                  "Set to 1 when the inputsize was obtained from the source matrix.");

    declareOption(ol, "obtained_targetsize_from_source", &MeanImputationVMatrix::obtained_targetsize_from_source, OptionBase::learntoption,
                  "Set to 1 when the targetsize was obtained from the source matrix.");

    declareOption(ol, "obtained_weightsize_from_source", &MeanImputationVMatrix::obtained_weightsize_from_source, OptionBase::learntoption,
                  "Set to 1 when the weightsize was obtained from the source matrix.");

    inherited::declareOptions(ol);
}

void MeanImputationVMatrix::build()
{
    inherited::build();
    build_();
}

void MeanImputationVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(variable_mean, copies);
    deepCopyField(variable_present_count, copies);
    deepCopyField(obtained_inputsize_from_source, copies);
    deepCopyField(obtained_targetsize_from_source, copies);
    deepCopyField(obtained_weightsize_from_source, copies);
}

void MeanImputationVMatrix::getExample(int i, Vec& input, Vec& target, real& weight)
{
    int col;
    source->getExample(i, input, target, weight);
    for (col = 0; col < input->length(); col++)
    {
        if (is_missing(input[col])) input[col] = variable_mean[col];
    }
}

real MeanImputationVMatrix::get(int i, int j) const
{
    real variable_value = source->get(i, j);
    if (is_missing(variable_value)) return variable_mean[j];
    else return variable_value;
}

void MeanImputationVMatrix::put(int i, int j, real value)
{
    PLERROR("In MeanImputationVMatrix::put not implemented");
}

void MeanImputationVMatrix::getSubRow(int i, int j, Vec v) const
{
    int col;
    source-> getSubRow(i, j, v);
    for (col = 0; col < v->length(); col++)
        if (is_missing(v[col])) v[col] = variable_mean[col + j];
}

void MeanImputationVMatrix::putSubRow(int i, int j, Vec v)
{
    PLERROR("In MeanImputationVMatrix::putSubRow not implemented");
}

void MeanImputationVMatrix::appendRow(Vec v)
{
    PLERROR("In MeanImputationVMatrix::appendRow not implemented");
}

void MeanImputationVMatrix::insertRow(int i, Vec v)
{
    PLERROR("In MeanImputationVMatrix::insertRow not implemented");
}

void MeanImputationVMatrix::getRow(int i, Vec v) const
{
    int col;
    source-> getRow(i, v);
    for (col = 0; col < v->length(); col++)
        if (is_missing(v[col])) v[col] = variable_mean[col];
}

void MeanImputationVMatrix::putRow(int i, Vec v)
{
    PLERROR("In MeanImputationVMatrix::putRow not implemented");
}

void MeanImputationVMatrix::getColumn(int i, Vec v) const
{
    int row;
    source-> getColumn(i, v);
    for (row = 0; row < v->length(); row++)
        if (is_missing(v[row])) v[row] = variable_mean[i];
}

void MeanImputationVMatrix::build_()
{
    if (source) {
        string error_msg =
            "In MeanImputationVMatrix::build_ - For safety reasons, it is forbidden to "
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
        computeMeanVector();
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

TVec<real> MeanImputationVMatrix::getMeanVector()
{
    return variable_mean;
}

void MeanImputationVMatrix::computeMeanVector()
{
    int length = length_;
    int width = width_;
    int row;
    int col;
    real variable_value;
    variable_mean.resize(width);
    variable_present_count.resize(width);
    if (number_of_train_samples > 0.0)
    {
        if (number_of_train_samples >= 1.0) length = (int) number_of_train_samples;
        else length = (int) ((double) length * number_of_train_samples);
        if (length < 1) length = 1;
        if (length > length_) length = length_;
    }
    for (col = 0; col < width; col++)
    {
        variable_mean[col] = 0.0;
        variable_present_count[col] = 0;
    }
    for (row = 0; row < length; row++)
    {
        for (col = 0; col < width; col++)
        {
            variable_value = source->get(row, col);
            if (!is_missing(variable_value))
            {
                variable_mean[col] += variable_value;
                variable_present_count[col] += 1;
            }
        }
    }
    for (col = 0; col < width; col++)
    {
        if (variable_present_count > 0) variable_mean[col] = variable_mean[col] / variable_present_count[col];
    }
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
