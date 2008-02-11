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
   * $Id: ConditionalMeanImputationVMatrix.cc 3658 2005-07-06 20:30:15  Godbout $
   ******************************************************************* */


#include "ConditionalMeanImputationVMatrix.h"
#include <plearn/io/fileutils.h>              //!<  For isfile()

namespace PLearn {
using namespace std;

/** ConditionalMeanImputationVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
  ConditionalMeanImputationVMatrix,
  "VMat class to impute the conditional mean to replace missing values in the source matrix.",
  "This class will replace missing values in the underlying dataset with the estimated values\n"
  "from a preceding machine learning step where each variable with missing value have to have.\n"
  "been considered as target in turns.\n"
  "The predictions are expected in the metadata directory of the data set.\n"
  );

ConditionalMeanImputationVMatrix::ConditionalMeanImputationVMatrix()
{
}

ConditionalMeanImputationVMatrix::~ConditionalMeanImputationVMatrix()
{
}

void ConditionalMeanImputationVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "condmean_dir", &ConditionalMeanImputationVMatrix::condmean_dir, OptionBase::buildoption, 
                "The directory in the source metadatadir housing the variable conditional mean files.\n");
  declareOption(ol, "condmean", &ConditionalMeanImputationVMatrix::condmean, OptionBase::learntoption, 
                "The matrix of conditional means.\n");
  declareOption(ol, "condmean_col_ref", &ConditionalMeanImputationVMatrix::condmean_col_ref, OptionBase::learntoption, 
                "The cross reference between columns of source and condmean.\n");
  inherited::declareOptions(ol);
}

void ConditionalMeanImputationVMatrix::build()
{
  inherited::build();
  build_();
}

void ConditionalMeanImputationVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  deepCopyField(condmean_dir, copies);
  deepCopyField(condmean, copies);
  deepCopyField(condmean_col_ref, copies);
  inherited::makeDeepCopyFromShallowCopy(copies);
}

void ConditionalMeanImputationVMatrix::getExample(int i, Vec& input, Vec& target, real& weight)
{
  source->getExample(i, input, target, weight);
  for (int source_col = 0; source_col < input->length(); source_col++)
    if (is_missing(input[source_col]) && condmean_col_ref[source_col] >= 0) input[source_col] = condmean(i, condmean_col_ref[source_col]);
    else if (is_missing(input[source_col])) PLERROR("In ConditionalMeanImputationVMatrix::getExample(%d,vec,vec,vec) we have a missing value in column %d that haven't been assigned a value",i,source_col);
}

real ConditionalMeanImputationVMatrix::get(int i, int j) const
{ 
  real variable_value = source->get(i, j);
  if (is_missing(variable_value) && condmean_col_ref[j] >= 0) return condmean(i, condmean_col_ref[j]);
  else if (is_missing(variable_value)) PLERROR("In ConditionalMeanImputationVMatrix::getExample(%d,%d) we have a missing value that haven't been assigned a value",i,j);
  return variable_value;
}

void ConditionalMeanImputationVMatrix::put(int i, int j, real value)
{
  PLERROR("In ConditionalMeanImputationVMatrix::put not implemented");
}

void ConditionalMeanImputationVMatrix::getSubRow(int i, int j, Vec v) const
{  
  source->getSubRow(i, j, v);
  for (int source_col = 0; source_col < v->length(); source_col++) 
    if (is_missing(v[source_col])) v[source_col] = condmean(i, condmean_col_ref[source_col + j]);
    else if (is_missing(v[source_col])) PLERROR("In ConditionalMeanImputationVMatrix::getSubRow(%d,%d,vec) we have a missing value in colomn %d that haven't been assigned a value",i,j,source_col);
}

void ConditionalMeanImputationVMatrix::putSubRow(int i, int j, Vec v)
{
  PLERROR("In ConditionalMeanImputationVMatrix::putSubRow not implemented");
}

void ConditionalMeanImputationVMatrix::appendRow(Vec v)
{
  PLERROR("In ConditionalMeanImputationVMatrix::appendRow not implemented");
}

void ConditionalMeanImputationVMatrix::insertRow(int i, Vec v)
{
  PLERROR("In ConditionalMeanImputationVMatrix::insertRow not implemented");
}

void ConditionalMeanImputationVMatrix::getRow(int i, Vec v) const
{  
  source-> getRow(i, v);
  for (int source_col = 0; source_col < v->length(); source_col++)
    if (is_missing(v[source_col]) && condmean_col_ref[source_col] >= 0) v[source_col] = condmean(i, condmean_col_ref[source_col]);
    else if (is_missing(v[source_col])) PLERROR("In ConditionalMeanImputationVMatrix::getRow(%d,vec) we have a missing value in column %d that haven't been assigned a value",i,source_col);
}

void ConditionalMeanImputationVMatrix::putRow(int i, Vec v)
{
  PLERROR("In ConditionalMeanImputationVMatrix::putRow not implemented");
}

void ConditionalMeanImputationVMatrix::getColumn(int i, Vec v) const
{  
  source-> getColumn(i, v);
  for (int source_row = 0; source_row < v->length(); source_row++)
    if (is_missing(v[source_row]) && condmean_col_ref[i] >= 0) v[source_row] = condmean(source_row, condmean_col_ref[i]);
    else if (is_missing(v[source_row])) PLERROR("In ConditionalMeanImputationVMatrix::getColumn(%d,vec) we have a missing value in row %d that haven't been assigned a value",i,source_row);
}



void ConditionalMeanImputationVMatrix::build_()
{
    if (!source) PLERROR("In ConditionalMeanImputationVMatrix::source vmat must be supplied");
    loadCondMeanMatrix(); 
    testResultantVMatrix();
}

void ConditionalMeanImputationVMatrix::loadCondMeanMatrix()
/*  
Imputation step:
  count the # of variables with missing values in the train and test datasets.
  create a matrix in memory with this number of columns and keep cross reference of columns.
  at the build stage, for each variable of train and test:
    if # of missing = 0 there is nothing to do.
    look for the (cond_mean_dir (/TreeCondMean/dir/) + field_name + /Split0/test1_outputs.pmat) file in the metadatadir;
    add its column 0 as a column of the matrix.
  then, if is_missing(source[i,j]) replace it with matrix[i, cross_reference[j]]
*/
{
    // initialize source dataset
    source_length = source->length();
    source_width = source->width();
    source_inputsize = source->inputsize();
    source_targetsize = source->targetsize();
    source_weightsize = source->weightsize();
    source_names.resize(source_width);
    source_names = source->fieldNames();
    source_metadata = source->getMetaDataDir();
    length_ = source_length;
    width_ = source_width;
    inputsize_ = source_inputsize;
    targetsize_ = source_targetsize;
    weightsize_ = source_weightsize;
    declareFieldNames(source_names);
    
    // count the # of variables with missing values in the source datasets.
    // create a matrix in memory with this number of columns and keep cross reference of columns.
    int count_variable_with_missing = 0;
    condmean_col_ref.resize(source_width);
    condmean_col_ref.fill(-1);
    for (source_col = 0; source_col < source_width; source_col++)
    {
        source_stats = source->getStats(source_col);
        if (source_stats.nmissing() <= 0) continue;
        condmean_col_ref[source_col] = count_variable_with_missing;
        count_variable_with_missing += 1;
    }
    condmean.resize(source_length, count_variable_with_missing);
    
    // for each variable with missing value, 
    // look for the (cond_mean_dir (/TreeCondMean/dir/) + field_name + /Split0/test1_outputs.pmat) file in the metadatadir;
    // add its column 0 as a column of the condmean matrix.
    for (source_col = 0; source_col < source_width; source_col++)
    {
        source_stats = source->getStats(source_col);
        if (source_stats.nmissing() <= 0) continue;
        int condmean_col = condmean_col_ref[source_col];
        PPath condmean_variable_file_name = source_metadata + "/" + condmean_dir + "/dir/" + source_names[source_col] + "/Split0/test1_outputs.pmat";
        if (!isfile(condmean_variable_file_name)) PLERROR("In ConditionalMeanImputationVMatrix::A conditional mean file(%s) was not found for variable %s",
                                                          condmean_variable_file_name.c_str(),source_names[source_col].c_str());
        VMat condmean_variable_file = new FileVMatrix(condmean_variable_file_name, false);
        if (condmean_variable_file->length() != source_length)
            PLERROR("In ConditionalMeanImputationVMatrix::Source and conditional mean file length are not equal for variable %s", source_names[source_col].c_str());
        for (source_row = 0; source_row < source_length; source_row++)
            condmean(source_row, condmean_col) = condmean_variable_file->get(source_row, 0);
    }
}

} // end of namespcae PLearn
