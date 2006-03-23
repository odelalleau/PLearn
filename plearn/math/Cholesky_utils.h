// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of
//                         Montreal, all rights reserved
// Copyright (C) 2006 Olivier Delalleau
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


#ifndef Cholesky_utils_INC
#define Cholesky_utils_INC

#include <plearn/math/Mat.h>

namespace PLearn {

//! Update the Cholesky decomposition of A = L L' when a new row is appended
//! to the matrix A. Since A is symmetric, this of course means that this
//! 'new_row' vector is also appended as a column in A.
//! The matrix L is modified as follows:
//!   - its dimensions (length and width) are increased by 1
//!   - its last row (except the bottom-right element) is the vector x such
//!     that Lx = new_row (obtained by back-substitution)
//!   - its bottom-right element is set to sqrt(new_row.last() - ||y||^2)
//! Computational cost: O(n^2)
void choleskyAppendDimension(Mat& L, const Vec& new_row);

//! Update the Cholesky decomposition of A = L L' when dimension i is removed
//! (i.e. row and column i are deleted in A).
void choleskyRemoveDimension(Mat& L, int i);

// L (n_active x n_active) is lower-diagonal and is such that A = L L' = lambda I + sum_t phi_t(x_t) phi_t(x_t)'
// where the sum runs until 'now' and phi_t is the 'current' active basis set. 
// In this function we update L so as to incorporate a new basis, i.e. n_active is incremented,
// using a new basis whose outputs on all the examples up to 'now' are given in new_basis_outputs.
// The dimensions of L are changed as a result. Note that L must have been pre-allocated
// with dimensions max_n_active x max_n_active. The matrix active_bases_outputs (n_active x n_examples)
// contains the outputs of the active bases that are in phi up to now.
// Computational cost is O(n_active * n_examples).
void choleskyInsertBasis(Mat& L, Mat active_bases_outputs, Vec new_basis_outputs, real lambda, real min_Lii=1e-10);

// Given a current Cholesky decomposition of A = L L' = sum_t v_t v_t'
// add an extra v_t v_t' term to the matrix A (i.e. upgrade lower-diagonal L accordingly).
// computational cost: O(n^2)
void choleskyUpgrade(Mat& L, Vec v);

/****** Utility functions ******/

//! From 'Matrix Algorithms, Vol1' by G. W. Stewart, p.272, 273, 335, 338.
void chol_dxch(Mat& R, int l, int m);
void chol_rotapp(real c, real s, const Vec& x, const Vec& y);
void chol_rotgen(real& a, real& b, real& c, real& s);

//! These two functions are variants of the above functions, where the R matrix
//! is given as its transpose (which is the case in the Cholesky
//! decomposition). This allows a more efficient implementation of
//! choleskyRemoveDimension(..).
void chol_dxch_tr(Mat& R_t, int l, int m);
void chol_rotapp_tr(real c, real s, const Mat& x, const Mat& y);


// debugging / test program for the above functions
void testCholeskyRoutines();


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
