// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1999-2002 Yoshua Bengio
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


#include "SparseMatrix.h"

namespace PLearn {
using namespace std;

void SparseMatrix::resize(int nbrows,int n_columns,int n_non_zero)
{
  n_rows=nbrows;
  beginRow.resize(n_columns);
  endRow.resize(n_columns);
  row.resize(n_non_zero);
  values.resize(n_non_zero);
}

// load SparseMatrix from file in ascii Harwell-Boeing Fortran format:
// 4-line header, followed by beginRow, row, and values.
void SparseMatrix::loadFortran(const char* filename)
{
  FILE* fp=fopen(filename,"r");
  if (!fp)
    PLERROR("SparseMatrix::loadFortran, can't open file %s\n",filename);
  int n_cols,n_nonzero;
  fscanf(fp,"%*72c%*s%*s%*s%d%d%d%*d",&n_rows, &n_cols, &n_nonzero); 
  fscanf(fp,"%*s %*s %*s %*s"); // skip some format infos
  beginRow.resize(n_cols);
  endRow.resize(n_cols);
  values.resize(n_nonzero);
  row.resize(n_nonzero);
  real *brow = beginRow.data();
  real *erow = endRow.data();
  real *v = values.data();
  real *r = row.data();
  int i;
  for (i = 0; i < n_cols; i++) 
    { 
#ifdef USEFLOAT
      fscanf(fp, "%f", &brow[i]); 
#endif
#ifdef USEDOUBLE
      fscanf(fp, "%lf", &brow[i]); 
#endif
      brow[i]--; 
      if (i>0) erow[i-1]=brow[i]-1;
    }
  erow[n_cols-1]=n_nonzero-1;
  fscanf(fp,"%d",&i);
  if (i-1!=n_nonzero) 
    PLERROR("SparseMatrix::loadFortran, inconsistent nnz %d vs %d",
          n_nonzero,i);
  for (i=0;i<n_nonzero;i++)
    {
#ifdef USEFLOAT
      fscanf(fp, "%f", &r[i]); 
#endif
#ifdef USEDOUBLE
      fscanf(fp, "%lf", &r[i]); 
#endif
      r[i]--;
    }
  for (i=0;i<n_nonzero;i++)
#ifdef USEFLOAT
    fscanf(fp, "%f", &v[i]); 
#endif
#ifdef USEDOUBLE
    fscanf(fp, "%lf", &v[i]); 
#endif
}
// save SparseMatrix from file in ascii Harwell-Boeing Fortran format:
// 4-line header, followed by beginRow, row, and values.
void SparseMatrix::saveFortran(const char* filename)
{
  FILE* fp=fopen(filename,"w");
  if (!fp)
    PLERROR("SparseMatrix::saveFortran, can't open file %s\n",filename);
  int n_nonzero=values.length(), n_cols = endRow.length();
  fprintf(fp,"%72s%8s\n#\nrra %d %d %d 0\n","SparseMatrix         ",
          filename,
          n_rows, n_cols , n_nonzero);
  fprintf(fp,"          (10i8)          (10i8)            (8f10.3)            (8f10.3)\n");
  real *brow = beginRow.data();
  real *v = values.data();
  real *r = row.data();
  int i;
  for (i = 0; i < n_cols; i++) 
    //fprintf(fp, "%8d", (int)(brow[i]+1)); 
    fprintf(fp, "%7d ", (int)(brow[i]+1)); 
  fprintf(fp,"%8d\n",values.length()+1);
  for (i=0;i<n_nonzero;i++)
    fprintf(fp,"%7d ",(int)(r[i]+1));
  fprintf(fp,"\n");
  for (i=0;i<n_nonzero;i++)
    fprintf(fp,"%9f ",v[i]);
  fprintf(fp,"\n");
  fclose(fp);
}

// convert to the equivalent full matrix
Mat SparseMatrix::toMat()
{
  int n_cols = beginRow.length();
  Mat A(n_rows,n_cols);
  real* r=row.data();
  real* v=values.data();
  for (int c=0;c<n_cols;c++)
    {
      real* Ac = &A(0,c);
      int e = (int)endRow[c];
      for (int k=(int)beginRow[c];k<=e;k++)
        Ac[n_cols*(int)r[k]]=v[k];
    }
  return A;
}

SparseMatrix::SparseMatrix(Mat A)
  : n_rows(A.length()), beginRow(A.width()), endRow(A.width())
{
  int n_nonzero=0;
  for (int i=0;i<n_rows;i++)
    {
      real* Ai=A[i];
      for (int j=0;j<A.width();j++)
        if (Ai[j]!=0) n_nonzero++;
    }
  row.resize(n_nonzero);
  values.resize(n_nonzero);
  int mod = A.mod();
  int k=0;
  real* v=values.data();
  real* r=row.data();
  real* b=beginRow.data();
  real* e=endRow.data();
  for (int j=0;j<A.width();j++)
    {
      real* Aij = &A(0,j);
      b[j] = k;
      for (int i=0;i<n_rows;i++,Aij+=mod)
        if (*Aij!=0)
          {
            r[k] = i;
            v[k] = *Aij;
            k++;
          }
      e[j] = k-1;
    }
}

void SparseMatrix::product(const Vec& x, Vec& y)
{
  // y[i] = sum_j A[i,j] x[j]
  if (y.length()!=n_rows || x.length()!=beginRow.length())
    PLERROR("SparseMatrix(%d,%d)::product(x(%d) -> y(%d)): dimensions don't match",
          n_rows,beginRow.length(),x.length(),y.length());
  y.clear();
  real* y_=y.data();
  real* x_=x.data();
  real* A_=values.data();
  // loop over columns of A, accumulating in y
  for (int j=0;j<beginRow.length();j++)
  {
    real xj = x_[j];
    for (int k=(int)beginRow[j];k<=endRow[j];k++)
    {
      int i=(int)row[k];
      y_[i] += A_[k] * xj;
    }
  }
}

void SparseMatrix::diag(Vec& d)
{
  real* d_ = d.data();
  real* A_ = values.data();
  int k;
  for (int j=0;j<beginRow.length();j++)
  {
    int end=int(endRow[j]);
    for (k=(int)beginRow[j];k<=end && int(row[k])!=j;k++);
    if (k<=end)
      d_[j]=A_[k];
    else
      d_[j]=0;
  }
}

void SparseMatrix::diagonalOfSquare(Vec& d)
{
  real* d_ = d.data();
  real* A_ = values.data();
  int k;
  for (int j=0;j<beginRow.length();j++)
  {
    int end=int(endRow[j]);
    real sum2=0;
    for (k=(int)beginRow[j];k<=end;k++)
      sum2 += A_[k]*A_[k];
    d_[j]=sum2;
  }
}

// return dot product of i-th row with vector v
real SparseMatrix::dotRow(int i, Vec v)
{
  PLERROR("SparseMatrix is not appropriate to perform dotRow operations");
  return 0;
}

// return dot product of j-th column with vector v
real SparseMatrix::dotColumn(int j, Vec v)
{
#ifdef BOUNDCHECK
  if (v.length()!=length())
    PLERROR("SparseMatrix::dotColumn(%d,v), v.length_=%d != matrix length=%d",
          j,v.length(),length());
#endif
  real s=0;
  real* v_=v.data();
  real* A_=values.data();
  for (int k=int(beginRow[j]);k<=int(endRow[j]);k++)
    s += A_[k] * v_[int(row[k])];
  return s;
}

// add two sparse matrices (of same dimensions)
SparseMatrix operator+(const SparseMatrix& A, const SparseMatrix& B)
{
  int n_rows = A.n_rows;
  int n_columns = A.beginRow.length();
  if (n_rows != B.n_rows)
    PLERROR("SparseMatrix(%d,%d)+SparseMatrix(%d,%d): both should have same dimensions",
          n_rows,A.beginRow.length(),B.n_rows,B.beginRow.length());
  int n_non_zero = A.row.length()+B.row.length(); // THIS IS AN UPPER BOUND ON ACTUAL n_non_zero
  SparseMatrix C(n_rows,n_columns,n_non_zero);

  int n_actual_non_zero=0;
  // the data is stored column-wise
  Vec column(n_rows);
  real* v=column.data();
  for (int j=0;j<n_columns;j++)
    {
      column.clear();
      for (int i=(int)A.beginRow[j];i<=A.endRow[j];i++)
        v[(int)A.row[i]]=A.values[i];
      for (int i=(int)B.beginRow[j];i<=B.endRow[j];i++)
        v[(int)B.row[i]]+=B.values[i];
      C.beginRow[j]=n_actual_non_zero;
      for (int i=0;i<n_rows;i++)
        if (v[i]!=0)
          {
            C.row[n_actual_non_zero]=i;
            C.values[n_actual_non_zero]=v[i];
            n_actual_non_zero++;
          }
      C.endRow[j]=n_actual_non_zero-1;

    }
  C.row.resize(n_actual_non_zero);
  C.values.resize(n_actual_non_zero);
  return C;
}

// add a bunch of sparse matrices and return result
SparseMatrix add(Array<SparseMatrix>& matrices)
{
  int n_mat = matrices.size();
  if (n_mat<1) PLERROR("add(Array<SparseMatrix>) argument is empty!");
  int n_rows = matrices[0].n_rows;
  int n_columns = matrices[0].beginRow.length();
  for (int i=1;i<n_mat;i++)
    if (n_rows != matrices[i].n_rows)
    PLERROR("add(SparseMatrix(%d,%d)+SparseMatrix(%d,%d): both should have same dimensions",
          n_rows,n_columns,matrices[i].n_rows,matrices[i].beginRow.length());
  int n_non_zero = 0;
  for (int i=0;i<n_mat;i++)
    n_non_zero+=matrices[i].row.length(); // UPPER BOUND ON ACTUAL n_non_zero

  SparseMatrix C(n_rows,n_columns,n_non_zero);

  int n_actual_non_zero=0;
  // the data is stored column-wise
  Vec column(n_rows);
  real* v=column.data();
  for (int j=0;j<n_columns;j++)
    {
      column.clear();
      for (int k=0;k<n_mat;k++)
        for (int i=(int)matrices[k].beginRow[j];i<=(int)matrices[k].endRow[j];i++)
          v[(int)matrices[k].row[i]]+=matrices[k].values[i];
      C.beginRow[j]=n_actual_non_zero;
      for (int i=0;i<n_rows;i++)
        if (v[i]!=0)
          {
            C.row[n_actual_non_zero]=i;
            C.values[n_actual_non_zero]=v[i];
            n_actual_non_zero++;
          }
      C.endRow[j]=n_actual_non_zero-1;

    }
  C.row.resize(n_actual_non_zero);
  C.values.resize(n_actual_non_zero);
  return C;
}

} // end of namespace PLearn
