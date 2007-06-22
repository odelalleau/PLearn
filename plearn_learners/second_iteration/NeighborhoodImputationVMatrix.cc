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
  : count_missing_neighbors(0)
{
}

NeighborhoodImputationVMatrix::~NeighborhoodImputationVMatrix()
{
}

void NeighborhoodImputationVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "reference_index", &NeighborhoodImputationVMatrix::reference_index, OptionBase::buildoption, 
                "The set of pre-computed neighbors index.\n"
                "This can be done with BallTreeNearestNeighbors.\n");

  declareOption(ol, "reference_with_missing", &NeighborhoodImputationVMatrix::reference_with_missing, OptionBase::buildoption, 
                "The reference set corresponding to the pre-computed index with missing values.");
      
  declareOption(ol, "reference_with_covariance_preserved", &NeighborhoodImputationVMatrix::reference_with_covariance_preserved, OptionBase::buildoption, 
                "The reference set corresponding to the pre-computed index with the initial imputations.");

  declareOption(ol, "number_of_neighbors", &NeighborhoodImputationVMatrix::number_of_neighbors, OptionBase::buildoption,
                "This is usually called K, the number of neighbors to consider.\n"   
                "It must be less or equal than the with of the reference index.");

  declareOption(ol, "count_missing_neighbors", &NeighborhoodImputationVMatrix::count_missing_neighbors, OptionBase::buildoption,
                "0: (default) We do not count a neighbour with a missing value in the number of neighbors.\n"   
                "1: We count a neighbour with a missing value in the number of neighbors.\n");

  declareOption(ol, "imputation_spec", &NeighborhoodImputationVMatrix::imputation_spec, OptionBase::buildoption,
                "A vector that give for each variable the number of neighbors to use.\n"
                "If a variable don't have a value, we use the value of the varialbe: number_of_neighbors.\n"
                " Ex: [ varname1 : nbneighbors1, varname2 : nbneighbors2 ]\n");
  inherited::declareOptions(ol);
}

void NeighborhoodImputationVMatrix::build()
{
  inherited::build();
  build_();
  testResultantVMatrix();
}

void NeighborhoodImputationVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  deepCopyField(reference_index, copies);
  deepCopyField(reference_with_missing, copies);
  deepCopyField(reference_with_covariance_preserved, copies);
  deepCopyField(number_of_neighbors, copies);
  deepCopyField(count_missing_neighbors, copies);
  deepCopyField(imputation_spec, copies);
  inherited::makeDeepCopyFromShallowCopy(copies);
}

void NeighborhoodImputationVMatrix::getExample(int i, Vec& input, Vec& target, real& weight)
{
  source->getExample(i, input, target, weight);
  for (int source_col = 0; source_col < input->length(); source_col++)
  {
    if (is_missing(input[source_col])) input[source_col] = impute(i, source_col);
  }  
}

real NeighborhoodImputationVMatrix::get(int i, int j) const
{ 
  real variable_value = source->get(i, j);
  if (!is_missing(variable_value)) return variable_value;
  return impute(i, j);
}

void NeighborhoodImputationVMatrix::put(int i, int j, real value)
{
  PLERROR("In NeighborhoodImputationVMatrix::put not implemented");
}

void NeighborhoodImputationVMatrix::getSubRow(int i, int j, Vec v) const
{  
  source->getSubRow(i, j, v);
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
  source-> getRow(i, v);
  for (int source_col = 0; source_col < v->length(); source_col++)
    if (is_missing(v[source_col])) v[source_col] = impute(i, source_col); 
}

void NeighborhoodImputationVMatrix::putRow(int i, Vec v)
{
  PLERROR("In NeighborhoodImputationVMatrix::putRow not implemented");
}

void NeighborhoodImputationVMatrix::getColumn(int i, Vec v) const
{  
  source-> getColumn(i, v);
  for (int source_row = 0; source_row < v->length(); source_row++)
    if (is_missing(v[source_row])) v[source_row] = impute(source_row, i);
}

void NeighborhoodImputationVMatrix::build_()
{
    if (!source)                 PLERROR("In NeighborhoodImputationVMatrix::source with missing set must be supplied");
    if (!reference_index)                     PLERROR("In NeighborhoodImputationVMatrix::reference index set must be supplied");
    if (!reference_with_missing)              PLERROR("In NeighborhoodImputationVMatrix::reference with missing set must be supplied");
    if (!reference_with_covariance_preserved) PLERROR("In NeighborhoodImputationVMatrix::reference with covariance preserved must be supplied");
    src_length = source->length();
    if (src_length != reference_index->length())
        PLERROR("In NeighborhoodImputationVMatrix::length of the source and its index must agree, got: %i - %i", src_length, reference_index->length());
    ref_length = reference_with_missing->length();
    if (ref_length != reference_with_covariance_preserved->length())
        PLERROR("In NeighborhoodImputationVMatrix::length of the reference set with missing and with covariance preserved must agree, got: %i - %i",
                ref_length, reference_with_covariance_preserved->length());
    src_width = source->width();
    if (src_width != reference_with_missing->width())
        PLERROR("In NeighborhoodImputationVMatrix::width of the source and the reference with missing must agree, got: %i - %i",
                src_width, reference_with_missing->width());
    if (src_width != reference_with_covariance_preserved->width())
        PLERROR("In NeighborhoodImputationVMatrix::width of the source and the reference with covariance preserved must agree, got: %i - %i",
                src_width, reference_with_covariance_preserved->width());
    if (number_of_neighbors < 1)
      PLERROR("In NeighborhoodImputationVMatrix::there must be at least 1 neighbors, got: %d",number_of_neighbors);
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
    inputsize_ = source->inputsize();
    targetsize_ = source->targetsize();
    weightsize_ = source->weightsize();
    declareFieldNames(source->fieldNames());

    nb_neighbors.resize(source->inputsize());
    nb_neighbors.fill(number_of_neighbors);
    for (int spec_col = 0; spec_col < imputation_spec.size(); spec_col++)
      {
        int source_col = fieldIndex(imputation_spec[spec_col].first);
        if (source_col < 0) 
          PLERROR("In NeighborhoodImputationVMatrix::build_() no field with this name in source data set: %s", (imputation_spec[spec_col].first).c_str());
        nb_neighbors[source_col] = imputation_spec[spec_col].second;
      }
}
real NeighborhoodImputationVMatrix::impute(int i, int j) const
{
    int ref_row;
    real return_value = 0.0;
    int value_count = 0;
    int neighbors_count = 0;
    for (int ref_idx_col = 0; ref_idx_col < ref_idx.width() &&
           neighbors_count < nb_neighbors[j]; ref_idx_col++)
    {
        ref_row = (int) ref_idx(i, ref_idx_col);
        if (ref_row < 0 || ref_row >= ref_length)
            PLERROR("In NeighborhoodImputationVMatrix::invalid index reference, got: %i for sample %i", ref_row, i);
        if (is_missing(ref_mis(ref_row, j))){
          if(count_missing_neighbors)
            neighbors_count++;
          continue;
        }
        return_value += ref_mis(ref_row, j);
        value_count++;
        neighbors_count++;
    }
    if (value_count > 0) return return_value / value_count;
    //if all neighbors have missing value we use the inputed version
    //TODO put a warning
    return_value = 0.0;
    value_count = 0;
    for (int ref_idx_col = 0; ref_idx_col < number_of_neighbors; ref_idx_col++)
    {
        ref_row = (int) ref_idx(i, ref_idx_col);
        if (is_missing(ref_cov(ref_row, j)))
            PLERROR("In NeighborhoodImputationVMatrix::missing value found in the reference with covariance preserved at: %i , %i", ref_row, j);
        return_value += ref_cov(ref_row, j);
        value_count += 1;
    }
    return return_value / value_count;
}

} // end of namespcae PLearn
