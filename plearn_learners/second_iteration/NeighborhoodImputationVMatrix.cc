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
   * $Id: NeighborhoodImputationVMatrix.cc 3658 2005-07-06 20:30:15  Godbout $
   ******************************************************************* */


#include "NeighborhoodImputationVMatrix.h"

namespace PLearn {
using namespace std;

/** NeighborhoodImputationVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
  NeighborhoodImputationVMatrix,
  "VMat class to impute the observed variable mean to replace missing values in the source matrix.",
  "This class will replace missing values in the underlying dataset with the mean, median or mode observed on the train set.\n"
  "The imputed value is based on the imputation instruction option.\n"
  );

NeighborhoodImputationVMatrix::NeighborhoodImputationVMatrix()
{
}

NeighborhoodImputationVMatrix::~NeighborhoodImputationVMatrix()
{
}

void NeighborhoodImputationVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "source_with_missing", &NeighborhoodImputationVMatrix::source_with_missing, OptionBase::buildoption, 
                "The source VMatrix with missing values.\n");

  declareOption(ol, "reference_index", &NeighborhoodImputationVMatrix::reference_index, OptionBase::buildoption, 
                "The set of pre-computed neighbors index.\n"
                "his can be done with BallTreeNearestNeighbors.\n");

  declareOption(ol, "reference_with_missing", &NeighborhoodImputationVMatrix::reference_with_missing, OptionBase::buildoption, 
                "The reference set corresponding to the pre-computed index with missing values.");
      
  declareOption(ol, "reference_with_covariance_preserved", &NeighborhoodImputationVMatrix::reference_with_covariance_preserved, OptionBase::buildoption, 
                "The reference set corresponding to the pre-computed index with the initial imputations.");

  declareOption(ol, "number_of_neighbors", &NeighborhoodImputationVMatrix::number_of_neighbors, OptionBase::learntoption,
                "This is usually called K, the number of neighbors to consider.\n"   
                "It must be less or equal than the with of the reference index.");

  inherited::declareOptions(ol);
}

void NeighborhoodImputationVMatrix::build()
{
  inherited::build();
  build_();
}

void NeighborhoodImputationVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  deepCopyField(source_with_missing, copies);
  deepCopyField(reference_index, copies);
  deepCopyField(reference_with_missing, copies);
  deepCopyField(reference_with_covariance_preserved, copies);
  deepCopyField(number_of_neighbors, copies);
  inherited::makeDeepCopyFromShallowCopy(copies);
}

void NeighborhoodImputationVMatrix::getExample(int i, Vec& input, Vec& target, real& weight)
{
  source_with_missing->getExample(i, input, target, weight);
  for (int source_col = 0; source_col < input->length(); source_col++)
  {
    if (is_missing(input[source_col])) input[source_col] = impute(i, source_col);
  }  
}

real NeighborhoodImputationVMatrix::get(int i, int j) const
{ 
  real variable_value = source_with_missing->get(i, j);
  if (!is_missing(variable_value)) return variable_value;
  return impute(i, j);
}

void NeighborhoodImputationVMatrix::put(int i, int j, real value)
{
  PLERROR("In NeighborhoodImputationVMatrix::put not implemented");
}

void NeighborhoodImputationVMatrix::getSubRow(int i, int j, Vec v) const
{  
  source_with_missing->getSubRow(i, j, v);
  for (int source_col = 0; source_col < v->length(); source_col++) 
    if (is_missing(v[source_col])) v[source_col] = impute(i, source_col + j);
}

void NeighborhoodImputationVMatrix::putSubRow(int i, int j, Vec v)
{
  PLERROR("In NeighborhoodImputationVMatrix::putSubRow not implemented");
}

void NeighborhoodImputationVMatrix::appendRow(Vec v)
{
  PLERROR("In NeighborhoodImputationVMatrix::appendRow not implemented");
}

void NeighborhoodImputationVMatrix::insertRow(int i, Vec v)
{
  PLERROR("In NeighborhoodImputationVMatrix::insertRow not implemented");
}

void NeighborhoodImputationVMatrix::getRow(int i, Vec v) const
{  
  source_with_missing-> getRow(i, v);
  for (int source_col = 0; source_col < v->length(); source_col++)
    if (is_missing(v[source_col])) v[source_col] = impute(i, source_col); 
}

void NeighborhoodImputationVMatrix::putRow(int i, Vec v)
{
  PLERROR("In NeighborhoodImputationVMatrix::putRow not implemented");
}

void NeighborhoodImputationVMatrix::getColumn(int i, Vec v) const
{  
  source_with_missing-> getColumn(i, v);
  for (int source_row = 0; source_row < v->length(); source_row++)
    if (is_missing(v[source_row])) v[source_row] = impute(source_row, i);
}

void NeighborhoodImputationVMatrix::build_()
{
    if (!source_with_missing)                 PLERROR("In NeighborhoodImputationVMatrix::source with missing set must be supplied");
    if (!reference_index)                     PLERROR("In NeighborhoodImputationVMatrix::reference index set must be supplied");
    if (!reference_with_missing)              PLERROR("In NeighborhoodImputationVMatrix::reference with missing set must be supplied");
    if (!reference_with_covariance_preserved) PLERROR("In NeighborhoodImputationVMatrix::reference with covariance preserved must be supplied");
    src_length = source_with_missing->length();
    if (src_length != reference_index->length())
        PLERROR("In NeighborhoodImputationVMatrix::length of the source and its index must agree, got: %i - %i", src_length, reference_index->length());
    ref_length = reference_with_missing->length();
    if (ref_length != reference_with_covariance_preserved->length())
        PLERROR("In NeighborhoodImputationVMatrix::length of the reference set with missing and with covariance preserved must agree, got: %i - %i",
                ref_length, reference_with_covariance_preserved->length());
    src_width = source_with_missing->width();
    if (src_width != reference_with_missing->width())
        PLERROR("In NeighborhoodImputationVMatrix::width of the source and the reference with missing must agree, got: %i - %i",
                src_width, reference_with_missing->width());
    if (src_width != reference_with_covariance_preserved->width())
        PLERROR("In NeighborhoodImputationVMatrix::width of the source and the reference with missing must agree, got: %i - %i",
                src_width, reference_with_covariance_preserved->width());
    if (number_of_neighbors < 1)
        PLERROR("In NeighborhoodImputationVMatrix::the index must contains at least as many reference as the specified number of neighbors, got: %i - %i",
                number_of_neighbors, reference_index->width());
    if (number_of_neighbors > reference_index->width())
        PLERROR("In NeighborhoodImputationVMatrix::the index must contains at least as many reference as the specified number of neighbors, got: %i - %i",
                number_of_neighbors, reference_index->width());
    ref_idx.resize(reference_index->length(), reference_index->width());
    ref_idx = reference_index->toMat();
    ref_mis.resize(reference_with_missing->length(), reference_with_missing->width());
    ref_mis = reference_with_missing->toMat();
    ref_cov.resize(reference_with_covariance_preserved->length(), reference_with_covariance_preserved->width());
    ref_cov = reference_with_covariance_preserved->toMat();
    length_ = src_length;
    width_ = src_width;
    inputsize_ = source_with_missing->inputsize();
    targetsize_ = source_with_missing->targetsize();
    weightsize_ = source_with_missing->weightsize();
    declareFieldNames(source_with_missing->fieldNames());
}
real NeighborhoodImputationVMatrix::impute(int i, int j) const
{
    int ref_idx_col;
    int ref_row;
    real return_value = 0.0;
    real value_count = 0.0;
    for (ref_idx_col = 0; ref_idx_col < number_of_neighbors; ref_idx_col++)
    {
        ref_row = (int) ref_idx(i, ref_idx_col);
        if (ref_row < 0 || ref_row >= ref_length)
            PLERROR("In NeighborhoodImputationVMatrix::invalid index reference, got: %i for sample %i", ref_row, i);
        if (is_missing(ref_mis(ref_row, j))) continue;
        return_value += ref_mis(ref_row, j);
        value_count += 1.0;
    }
    if (value_count > 0) return return_value / value_count;
    return_value = 0.0;
    value_count = 0.0;
    for (ref_idx_col = 0; ref_idx_col < number_of_neighbors; ref_idx_col++)
    {
        ref_row = (int) ref_idx(i, ref_idx_col);
        if (is_missing(ref_cov(ref_row, j)))
            PLERROR("In NeighborhoodImputationVMatrix::missing value found in the reference with covariance preserved at: %i , %i", ref_row, j);
        return_value += ref_cov(ref_row, j);
        value_count += 1.0;
    }
    return return_value / value_count;
}

} // end of namespcae PLearn
