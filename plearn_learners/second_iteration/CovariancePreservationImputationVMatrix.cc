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
   * $Id: CovariancePreservationImputationVMatrix.cc 3658 2005-07-06 20:30:15  Godbout $
   ******************************************************************* */


#include "CovariancePreservationImputationVMatrix.h"

namespace PLearn {
using namespace std;

/** CovariancePreservationImputationVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
  CovariancePreservationImputationVMatrix,
  "VMat class to impute values preserving the observed relationships between variables on a global basis.",
  "This class will replace a missing value in the underlying dataset with a value computed to minimized\n"
  "the distance of the sample covariates with the global covariance vector of the observed data.\n"
  );

CovariancePreservationImputationVMatrix::CovariancePreservationImputationVMatrix()
{
}

CovariancePreservationImputationVMatrix::~CovariancePreservationImputationVMatrix()
{
}

void CovariancePreservationImputationVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "source", &CovariancePreservationImputationVMatrix::source, OptionBase::buildoption, 
                "The source VMatrix with missing values.\n");

  declareOption(ol, "train_set", &CovariancePreservationImputationVMatrix::train_set, OptionBase::buildoption, 
                "A referenced train set.\n"
                "The covariance imputation is computed with the observed values in this data set.\n");

  inherited::declareOptions(ol);
}

void CovariancePreservationImputationVMatrix::build()
{
  inherited::build();
  build_();
}

void CovariancePreservationImputationVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  deepCopyField(source, copies);
  deepCopyField(train_set, copies);
  inherited::makeDeepCopyFromShallowCopy(copies);
}

void CovariancePreservationImputationVMatrix::getExample(int i, Vec& input, Vec& target, real& weight)
{
  source->getExample(i, input, target, weight);
  for (int source_col = 0; source_col < input->length(); source_col++)
  {
    if (is_missing(input[source_col])) input[source_col] = computeImputation(i, source_col, input);
  }  
}

real CovariancePreservationImputationVMatrix::get(int i, int j) const
{ 
  real variable_value = source->get(i, j);
  if (is_missing(variable_value)) computeImputation(i, j);
  return variable_value;
}

void CovariancePreservationImputationVMatrix::put(int i, int j, real value)
{
  PLERROR("In CovariancePreservationImputationVMatrix::put not implemented");
}

void CovariancePreservationImputationVMatrix::getSubRow(int i, int j, Vec v) const
{  
  source->getSubRow(i, j, v);
  for (int source_col = 0; source_col < v->length(); source_col++) 
    if (is_missing(v[source_col])) v[source_col] = computeImputation(i, source_col + j);
}

void CovariancePreservationImputationVMatrix::putSubRow(int i, int j, Vec v)
{
  PLERROR("In CovariancePreservationImputationVMatrix::putSubRow not implemented");
}

void CovariancePreservationImputationVMatrix::appendRow(Vec v)
{
  PLERROR("In CovariancePreservationImputationVMatrix::appendRow not implemented");
}

void CovariancePreservationImputationVMatrix::insertRow(int i, Vec v)
{
  PLERROR("In CovariancePreservationImputationVMatrix::insertRow not implemented");
}

void CovariancePreservationImputationVMatrix::getRow(int i, Vec v) const
{  
  source-> getRow(i, v);
  for (int source_col = 0; source_col < v->length(); source_col++)
    if (is_missing(v[source_col])) v[source_col] = computeImputation(i, source_col, v);
}

void CovariancePreservationImputationVMatrix::putRow(int i, Vec v)
{
  PLERROR("In CovariancePreservationImputationVMatrix::putRow not implemented");
}

void CovariancePreservationImputationVMatrix::getColumn(int i, Vec v) const
{  
  source-> getColumn(i, v);
  for (int source_row = 0; source_row < v->length(); source_row++)
    if (is_missing(v[source_row])) v[source_row] = computeImputation(source_row, i);
}



void CovariancePreservationImputationVMatrix::build_()
{
    if (!train_set || !source) PLERROR("In CovariancePreservationImputationVMatrix::train set and source vmat must be supplied");
    train_length = train_set->length();
    if(train_length < 1) PLERROR("In CovariancePreservationImputationVMatrix::length of the number of train samples to use must be at least 1, got: %i", train_length);
    train_width = train_set->width();
    train_targetsize = train_set->targetsize();
    train_weightsize = train_set->weightsize();
    train_inputsize = train_set->inputsize();
    if(train_inputsize < 1) PLERROR("In CovariancePreservationImputationVMatrix::inputsize of the train vmat must be supplied, got : %i", train_inputsize);
    source_width = source->width();
    source_targetsize = source->targetsize();
    source_weightsize = source->weightsize();
    source_inputsize = source->inputsize();
    if (train_width != source_width) PLERROR("In CovariancePreservationImputationVMatrix::train set and source width must agree, got : %i, %i", train_width, source_width);
    if (train_targetsize != source_targetsize) PLERROR("In CovariancePreservationImputationVMatrix::train set and source targetsize must agree, got : %i, %i", train_targetsize, source_targetsize);
    if (train_weightsize != source_weightsize) PLERROR("In CovariancePreservationImputationVMatrix::train set and source weightsize must agree, got : %i, %i", train_weightsize, source_weightsize);
    if (train_inputsize != source_inputsize) PLERROR("In CovariancePreservationImputationVMatrix::train set and source inputsize must agree, got : %i, %i", train_inputsize, source_inputsize);
    train_field_names.resize(train_width);
    train_field_names = train_set->fieldNames();
    source_length = source->length();
    length_ = source_length;
    width_ = source_width;
    inputsize_ = source_inputsize;
    targetsize_ = source_targetsize;
    weightsize_ = source_weightsize;
    declareFieldNames(train_field_names);
    train_metadata = train_set->getMetaDataDir();
    covariance_file_name = train_metadata + "covariance_file.pmat";
    cov.resize(train_width, train_width);
    mu.resize(train_width);
    if (!isfile(covariance_file_name))
    {
        computeCovariances();
        createCovarianceFile();
    }
    else loadCovarianceFile();
}

void CovariancePreservationImputationVMatrix::createCovarianceFile()
{
    covariance_file = new FileVMatrix(covariance_file_name, train_width + 1, train_field_names);
    for (indj = 0; indj < train_width; indj++)
    {
        for (indk = 0; indk < train_width; indk++)
        {
            covariance_file->put(indj, indk, cov(indj, indk));
        }
    }
    for (indk = 0; indk < train_width; indk++)
    {
        covariance_file->put(train_width, indk, mu[indk]);
    }
}

void CovariancePreservationImputationVMatrix::loadCovarianceFile()
{
    covariance_file = new FileVMatrix(covariance_file_name);
    for (indj = 0; indj < train_width; indj++)
    {
        for (indk = 0; indk < train_width; indk++)
        {
            cov(indj, indk) = covariance_file->get(indj, indk);
        }
    }
    for (indk = 0; indk < train_width; indk++)
    {
        mu[indk] = covariance_file->get(train_width, indk);
    }
}

VMat CovariancePreservationImputationVMatrix::getCovarianceFile()
{
    return covariance_file;
}

void CovariancePreservationImputationVMatrix::computeCovariances()
{
/*
    We need to populate the matrix of COV for all combinations of input variables
    we need in one pass to populate 4 matrices of dxd:
    n(j,k) the number of samples where x(i, j) and x(i, k) are simultaneously observed.
    sum_x(j)(k) the sum of the x(i, j) values where x(i, j) and x(i, k) are simultaneously observed.
    sum_x(j)_x(k) the sum of the x(i, j)*x(i, k) values where x(i, j) and x(i, k) are simultaneously observed.
    we can the calculate mu(k) = sum_x(k, k)/n(k, k)
    COV(j, k) = (sum_x(j)_x(k) - sum_x(j)(k) * mu(k) - sum_x(k)(j) * mu(j) + mu(k) * mu(j)) (1 / n(j,k))
    All we need after is the COV matrix to impute values on missing values.
    
*/
    n_obs.resize(train_width, train_width);
    sum_xj.resize(train_width, train_width);
    sum_xj_xk.resize(train_width, train_width);
    train_input.resize(train_width);
    n_obs.clear();
    sum_xj.clear();
    sum_xj_xk.clear();
    mu.clear();
    cov.clear();
    ProgressBar* pb = 0;
    pb = new ProgressBar("Computing the covariance matrix", train_length);
    for (train_row = 0; train_row < train_length; train_row++)
    {
        train_set->getRow(train_row, train_input);
        for (indj = 0; indj < train_width; indj++)
        {
            for (indk = 0; indk < train_width; indk++)
            {
                if (is_missing(train_input[indj]) || is_missing(train_input[indk])) continue;
                n_obs(indj, indk) += 1.0;
                sum_xj(indj, indk) += train_input[indj];
                sum_xj_xk(indj, indk) += train_input[indj] * train_input[indk];
            }
        }
        pb->update( train_row ); 
    }
    delete pb;
    for (indj = 0; indj < train_width; indj++)
    {
        mu[indj] = sum_xj(indj, indj) / n_obs(indj, indj); 
    }
    for (indj = 0; indj < train_width; indj++)
    {
        for (indk = 0; indk < train_width; indk++)
        {
            cov(indj, indk) = sum_xj_xk(indj, indk) - sum_xj(indj, indk) * mu[indk] - sum_xj(indk, indj) * mu[indj];
            cov(indj, indk) = (cov(indj, indk) /  n_obs(indj, indk)) + mu[indk] * mu[indj];
        }
    }
}

real CovariancePreservationImputationVMatrix::computeImputation(int row, int col) const
{
    Vec input(source_width);
    source->getRow(row, input);
    return computeImputation(row, col, input);
}

real CovariancePreservationImputationVMatrix::computeImputation(int row, int col, Vec input) const
{
    real sum_cov_xl = 0;
    real sum_xl_square = 0;
    for (int indl = 0; indl < source_width; indl++)
    {
        if (is_missing(input[indl])) continue;
        sum_cov_xl += cov(indl, col) * (input[indl] - mu[indl]);
        sum_xl_square += (input[indl] - mu[indl]) * (input[indl] - mu[indl]);
    }
    if (sum_xl_square == 0.0) return mu[col];
    return mu[col] + sum_cov_xl / sum_xl_square;
}

} // end of namespcae PLearn
