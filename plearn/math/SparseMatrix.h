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

/*! \file PLearn/plearn/math/SparseMatrix.h */

#ifndef SPARSEMATRIX
#define SPARSEMATRIX

#include "Mat.h"
#include <plearn/base/Array.h>

namespace PLearn {
using namespace std;


/*!   
  Sparse matrices
  
  * beginRow(n_columns): beginning of block of (not necessarily contiguous) elements of column j in the values vector
  * endRow(n_columns): last element of block of (not necessarily contiguous) elements of column j in the values vector
  * row(n_non_zero_elements): row of a non-zero element
  * values(n_non_zero_elements): value of a non-zero element 
  so for example values[k] is the value of an element (i,j) of the matrix
  such that row[k]=i and beginRow[i]<=k<=endRow[i].
*/
class SparseMatrix {
public:
    int n_rows;  //!<  the dimensions of the matrix are: n_rows x beginRow.length()
    Vec beginRow;
    Vec endRow;
    Vec row;
    Vec values;

    int length() const { return n_rows; }
    int width() const { return beginRow.length(); }

    SparseMatrix() {}
    SparseMatrix(int nbrows,int n_columns,int n_non_zero) 
        : n_rows(nbrows), beginRow(n_columns), endRow(n_columns), 
          row(n_non_zero), values(n_non_zero) {}
    SparseMatrix(Vec bRow, Vec eRow, Vec Row, Vec Values, int nbrows)
        : n_rows(nbrows), beginRow(bRow), endRow(eRow),
          row(Row), values(Values) {}

    //!  convert Mat into SparseMatrix:
    SparseMatrix(Mat m);

    SparseMatrix(string filename) { loadFortran(filename.c_str()); }

    void resize(int nbrows,int n_columns,int n_non_zero);

    //!  load SparseMatrix from file in ascii Harwell-Boeing Fortran format:
    //!  4-line header, followed by beginRow, row, and values.
    void loadFortran(const char* filename);
    void saveFortran(const char* filename);

    //!  convert to the equivalent full matrix
    Mat toMat();

    //!  multiply a sparse matrix by a full vector and set resulting vector
    //!    y = matrix * x
    void product(const Vec& x, Vec& y);
    //!  extract the diagonal of the sparse matrix: d[i] = A[i,i]
    void diag(Vec& d);
    //!  d = diagonal(A'*A), i.e.
    //!  d[i] = |A[i]|^2 where A[i] is i-th column
    void diagonalOfSquare(Vec& d); 
    //!  return dot product of i-th row with vector v
    real dotRow(int i, Vec v);
    //!  return dot product of j-th column with vector v
    real dotColumn(int j, Vec v);
};

//!  add two sparse matrices (of same dimensions but with values
//!  in possibly different places)
SparseMatrix operator+(const SparseMatrix& A, const SparseMatrix& B);
//!  add a bunch of sparse matrices and return result
SparseMatrix add(Array<SparseMatrix>& matrices);


} // end of namespace PLearn

#endif


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
