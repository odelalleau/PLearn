// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// distr_math.h
// Copyright (C) 2002 Pascal Vincent
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


/* *******************************************************      
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file distr_maths.h */

#ifndef distr_maths_INC
#define distr_maths_INC

#include "TMat.h"

namespace PLearn {
using namespace std;

// return log of Normal(x;mu, sigma2*I), i.e. density of a spherical Gaussian
real log_of_normal_density(Vec x, Vec mu, real sigma2);
inline real normal_density(Vec x, Vec mu, real sigma2) { return safeexp(log_of_normal_density(x,mu,sigma2)); }
real log_rbf(Vec x, Vec mu, real sigma2);
inline real rbf(Vec x, Vec mu, real sigma2) { return safeexp(log_rbf(x,mu,sigma2)); }

// return log of Normal(x;mu, diag(sigma2)), i.e. density of a diagonal Gaussian
real log_of_normal_density(Vec x, Vec mu, Vec sigma2);
inline real normal_density(Vec x, Vec mu, Vec sigma2) { return safeexp(log_of_normal_density(x,mu,sigma2)); }

// return log of Normal(x;mu, Sigma), i.e. density of a full Gaussian,
// where the covariance Sigma is
//    Sigma = remainder_evalue*I + sum_i max(0,evalues[i]-remainder_evalue)*evectors(i)*evectors(i)'
// The eigenvectors are in the ROWS of matrix evectors (because of easier row-wise access in Mat's).
real log_of_normal_density(Vec x, Vec mu, Mat evectors, Vec evalues, real remainder_evalue=0);
inline real normal_density(Vec x, Vec mu, Mat evectors, Vec evalues, real remainder_evalue=0) 
{ return safeexp(log_of_normal_density(x,mu,evectors,evalues,remainder_evalue)); }
real log_fullGaussianRBF(Vec x, Vec mu, Mat evectors, Vec evalues, real remainder_evalue=0);
inline real fullGaussianRBF(Vec x, Vec mu, Mat evectors, Vec evalues, real remainder_evalue=0) 
{ return safeexp(log_fullGaussianRBF(x,mu,evectors,evalues,remainder_evalue)); }

real logOfNormal(const Vec& x, const Vec& mu, const Mat& C);

//! Fits a gaussian to the points in X (computing its mean and covariance
//! matrix, and adding lambda to the diagonal of that covariance matrix)
//! Then calls logOfNormal to return log(p(x | the_gaussian))
real logPFittedGaussian(const Vec& x, const Mat& X, real lambda);

//! returns log P(x|gaussian) with a gaussian represented compactly 
//! by the first few eigenvalues and eigenvectors of its covariance matrix.
//! gamma is the "variance" used for all other directions.
//! Eigenvalues need not be in decreasing order, but as soon as we meet a 0 eigenvalue,
//! this and all subsequent ones are considered to be equal to gamma.
//! In addition if add_gamma_to_eigenval is true, the used eigenvalues will be eigenvalues+gamma.
real logOfCompactGaussian(const Vec& x, const Vec& mu, 
                          const Vec& eigenvalues, const Mat& eigenvectors, 
                          real gamma=1e-6, bool add_gamma_to_eigenval=false);


//! Returns the density of a proportion x under a Beta(alpha,beta) distribution,
//! equal to 
//!  /f$  x^{alpha-1} (1-x}^{beta-1} / Beta(a,b) /f$
//! where
//!    Beta(a,b) = Gamma(a)Gamma(b)/Gamma(a+b)
real beta_density(real x, real alpha, real beta);
//! Log of the beta_density
real log_beta_density(real x, real alpha, real beta);

// if (inverses) compute the eigendecomposition of C = inv(inv(A) + inv(B)) from eigendecompositions of A and B 
// else compute the eigendecomposition of C = A + B from eigendecompositions of A and B
void addEigenMatrices(Mat A_evec, Vec A_eval, Mat B_evec, Vec B_eval, Mat C_evec, Vec C_eval, bool inverses=false);

//! Given weighted statistics of order 0, 1 and 2, compute first and second moments of a Gaussian.
//! 0-th order statistic: sum_w = sum_i w_i 
//! 1-st order statistic: sum_wx = sum_i w_i x_i
//! 2-nd order statistic: sum_wx2 = sum_i w_i x_i x_i'
//! and put the results in 
//!   mu = sum_wx / sum_w
//!   (cov_evectors, cov_evalues) = eigen-decomposition of cov = sum_wx2 / sum_w - mu mu'
//! with eigenvectors in the ROWS of cov_evectors.
//! eigenvalues are replaced by max(eigenvalues,min_variance).
void sums2Gaussian(real sum_w, Vec sum_wx, Mat sum_wx2, Vec mu, Mat cov_evectors, Vec cov_evalues, real min_variance);

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
