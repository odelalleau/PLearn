// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1999-2002 Christian Jauvin
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

/*! \file ProbSparseMatrix.cc */

#include "ProbSparseMatrix.h"

namespace PLearn <%

ProbSparseMatrix::ProbSparseMatrix(int n_rows, int n_cols, string name, int mode, bool double_access) : DoubleAccessSparseMatrix<real>(n_rows, n_cols, name, mode, double_access)
{
}

void ProbSparseMatrix::incr(int i, int j, real inc, bool warning)
{
  if (inc <= 0.0 && warning)
    PLWARNING("incrementing value by <= 0.0");
  DoubleAccessSparseMatrix<real>::incr(i, j, inc);
}

void ProbSparseMatrix::set(int i, int j, real value, bool warning)
{
  if (value <= 0.0 && warning)
    PLWARNING("setting value <= 0.0");
  DoubleAccessSparseMatrix<real>::set(i, j, value);
}

bool ProbSparseMatrix::checkCondProbIntegrity()
{
  real sum = 0.0;
  if (mode == ROW_WISE)
  {
    for (int i = 0; i < height; i++)
    {
      map<int, real>& row_i = rows[i];
      sum = 0.0;
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
        sum += it->second;
      if (fabs(sum - 1.0) > 1e-4 && (sum > 0.0))
        return false;
    }
    return true;
  } else
  {
    for (int j = 0; j < width; j++)
    {
      map<int, real>& col_j = cols[j];
      sum = 0.0;
      for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it)
        sum += it->second;
      if (fabs(sum - 1.0) > 1e-4 && (sum > 0.0))
        return false;
    }
    return true;
  }
}

bool ProbSparseMatrix::checkJointProbIntegrity()
{
  return (fabs(sumOfElements() - 1.0) > 1e-4);
}

void ProbSparseMatrix::normalizeCond(ProbSparseMatrix& nXY, bool clear_nXY_on_the_fly)
{
  if (mode == ROW_WISE && (nXY.getMode() == ROW_WISE || nXY.isDoubleAccessible()))
  {
    clear();
    int nXY_height = nXY.getHeight();
    for (int i = 0; i < nXY_height; i++)
    {
      real sum_row_i = nXY.sumRow(i);
      map<int, real>& row_i = nXY(i);
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
      {
        int j = it->first;
        real nij = it->second;
        real pij = nij / sum_row_i;
        if (pij > 0.0)
          set(i, j, pij);
      }
      if (clear_nXY_on_the_fly)
        nXY.clearRow(i);
    }
  } else if (mode == COLUMN_WISE && (nXY.getMode() == COLUMN_WISE || nXY.isDoubleAccessible()))
  {
    clear();
    int nXY_width = nXY.getWidth();
    for (int j = 0; j < nXY_width; j++)
    {
      real sum_col_j = nXY.sumCol(j);
      map<int, real>& col_j = nXY(j);
      for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it)
      {
        int i = it->first;
        real nij = it->second;
        real pij = nij / sum_col_j;
        if (pij > 0.0)
          set(i, j, pij);
      }
      if (clear_nXY_on_the_fly)
        nXY.clearCol(j);
    }
  } else
  {
    PLERROR("pXY and nXY modes must match");
  }
  if (clear_nXY_on_the_fly)
    nXY.clear();
}

void ProbSparseMatrix::normalizeCond()
{
  if (mode == ROW_WISE)
  {
    for (int i = 0; i < height; i++)
    {
      real sum_row_i = sumRow(i);
      map<int, real>& row_i = rows[i];
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
      {
        int j = it->first;
        real nij = it->second;
        real pij = nij / sum_row_i;
        if (pij > 0.0)
          set(i, j, pij);
      }
    }
  } else
  {
    for (int j = 0; j < width; j++)
    {
      real sum_col_j = sumCol(j);
      map<int, real>& col_j = cols[j];
      for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it)
      {
        int i = it->first;
        real nij = it->second;
        real pij = nij / sum_col_j;
        if (pij > 0.0)
          set(i, j, pij);
      }
    }
  }
}

void ProbSparseMatrix::normalizeJoint(ProbSparseMatrix& nXY, bool clear_nXY)
{
  clear();
  real sum_nXY = nXY.sumOfElements();
  if (nXY.getMode() == ROW_WISE)
  {
    int nXY_height = nXY.getHeight();
    for (int i = 0; i < nXY_height; i++)
    {
      map<int, real>& row_i = nXY(i);
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
      {
        int j = it->first;
        real nij = it->second;
        real pij = nij / sum_nXY;
        if (pij > 0.0)
          set(i, j, pij);
      }
    }
  } else if (nXY.getMode() == COLUMN_WISE)
  {
    int nXY_width = nXY.getWidth();
    for (int j = 0; j < nXY_width; j++)
    {
      map<int, real>& col_j = nXY(j);
      for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it)
      {
        int i = it->first;
        real nij = it->second;
        real pij = nij / sum_nXY;
        if (pij > 0.0)
          set(i, j, pij);
      }
    }
  }
  if (clear_nXY)
    nXY.clear();
}

void ProbSparseMatrix::normalizeJoint()
{
  if (mode == ROW_WISE)
  {
    for (int i = 0; i < height; i++)
    {
      map<int, real>& row_i = rows[i];
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
      {
        int j = it->first;
        real nij = it->second;
        real pij = nij / sumOfElements();
        if (pij > 0.0)
          set(i, j, pij);
      }
    }
  } else
  {
    for (int j = 0; j < width; j++)
    {
      map<int, real>& col_j = cols[j];
      for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it)
      {
        int i = it->first;
        real nij = it->second;
        real pij = nij / sumOfElements();
        if (pij > 0.0)
          set(i, j, pij);
      }
    }
  }
}

%>
