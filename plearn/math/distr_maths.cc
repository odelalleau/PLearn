// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// distr_maths.cc
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
   * $Id: distr_maths.cc,v 1.3 2003/12/05 22:13:43 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnAlgo/distr_maths.cc */


#include "distr_maths.h"
#include "TMat_maths.h"

namespace PLearn <%
using namespace std;



 /*
    // *****************************
    // * Using eigen decomposition *
    // *****************************

    X the input matrix
    Let C be the covariance of X:  C = (X-mu)'.(X-mu)

    The eigendecomposition: 
    C = VDV'    where the *columns* of V are the orthonormal eigenvectors and D is a diagonal matrix with the eigenvalues lambda_1 ... lambda_d 

    det(C) = det(D) = product of the eigenvalues
    log(det(C)) = \sum_{i=1}^d [ log(lambda_i) ]

    inv(C) = V.inv(D).V'    (where inv(D) is a diagonal with the inverse of each eigenvalue)

    For a gaussian where C is the covariance matrix, mu is the mean (column vector), and x is a column vector, we have
    gaussian(x; mu,C) = 1/sqrt((2PI)^d * det(C)) exp( -0.5 (x-mu)'.inv(C).(x-mu) )
                      = 1/sqrt((2PI)^d * det(D)) exp( -0.5 (V'(x-mu))'.inv(D).(V'(x-mu)) )
                      = exp [ -0.5( d*log(2PI) + log(det(D)) )  -0.5(V'(x-mu))'.inv(D).(V'(x-mu)) ] 
                               \_______________  ____________/       \____________  ____________/
                                               \/                                 \/
                                             logcoef                              q

    The expression q = (V'(x-mu))'.inv(D).(V'(x-mu)) can be understood as:
       a) projecting vector x-mu on the orthonormal basis V, 
          i.e. obtaining a transformed x that we shall call y:  y = V'(x-mu)
          (y corresponds to x, expressed in the coordinate system V)
          y_i = V'_i.(x-mu)

       b) computing the squared norm of y , after first rescaling each coordinate by a factor 1/sqrt(lambda_i)
          (i.e. differences in the directions with large lambda_i are given less importance)
          Giving  q = sum_i[ 1/lambda_i  y_i^2]

    If we only keep the first k eigenvalues, and replace the following d-k ones by the same value gamma
    i.e.  lambda_k+1 = ... = lambda_d = gamma
    
    Then q can be expressed as:
      q = \sum_{i=1}^k [ 1/lambda_i y_i^2 ]   +   1/gamma \sum_{i=k+1}^d [ y_i^2 ]

    But, as y is just x expressed in another orthonormal basis, we have |y|^2 = |x-mu|^2
    ( proof: |y|^2 = |V'(x-mu)|^2 = (V'(x-mu))'.(V'(x-mu)) = (x-mu)'.V.V'.(x-mu) = (x-mu)'(x-mu) = |x-mu|^2 )
    
    Thus, we know  \sum_{i=1}^d [ y_i^2 ] = |x-mu|^2
    Thus \sum_{i=k+1}^d [ y_i^2 ] = |x-mu|^2 - \sum_{i=1}^k [ y_i^2 ]

    Consequently: 
      q = \sum_{i=1}^k [ 1/lambda_i y_i^2 ]   +  1/gamma ( |x-mu|^2 - \sum_{i=1}^k [ y_i^2 ] )

      q = \sum_{i=1}^k [ (1/lambda_i - 1/gamma) y_i^2 ]  +  1/gamma  |x-mu|^2

      q = \sum_{i=1}^k [ (1/lambda_i - 1/gamma) (V'_i.(x-mu))^2 ]  +  1/gamma  |x-mu|^2

      This gives the efficient algorithm implemented below

   --------------------------------------------------------------------------------------

   Other possibility: direct computation:
   
   Let's note X~ = X-mu

   We have already seen that
    For a gaussian where C is the covariance matrix, mu is the mean (column vector), and x is a column vector, we have
    gaussian(x; mu,C) = 1/sqrt((2PI)^d * det(C)) exp( -0.5 (x-mu)'.inv(C).(x-mu) )
                      = 1/sqrt((2PI)^d * det(D)) exp( -0.5 (V'(x-mu))'.inv(D).(V'(x-mu)) )
                      = exp [ -0.5( d*log(2PI) + log(det(D)) )  -0.5 (x-mu)'.inv(C).(x-mu) ] 
                               \_______________  ____________/       \_________  ________/
                                               \/                              \/
                                             logcoef                           q
    Let z = inv(C).(x-mu)
    ==> z is the solution of C.z = x-mu
    And then we have q = (x-mu)'.z

    So computing q is simply a matter of solving this linear equation in z,
    and then computing q.

    From my old paper we had to solve for alpha in the old notation: 
      " (V'V + lambda.I) alpha = V' (x-N~) "
    Which in our current notation corresponds to:
       (C + lambda.I) alpha = X~' x~
     If we drop the + lambda.I for now:
        alpha = inv(C) X~' x~
     and the "hyperplane distance" is then given by
       hd = sqnorm(x~ - X~.alpha)
          = sqnorm(x~ - X~.inv(C).X~'.x~)
          = sqnorm(x~ - X~.inv(X~'X)


  */



//! Computes and returns log( Normal(x; mu,C) )
//! where mu is the normal's mean and C its covariance matrix.
/*! For numerical stability, you may consider adding some lambda to the diagonal of C

    Normal(x; mu,C) = 1/sqrt((2PI)^d * det(C)) exp( -0.5 (x-mu)'.inv(C).(x-mu) )
                    = exp [ -0.5( d*log(2PI) + log(det(D)) )  -0.5 (x-mu)'.inv(C).(x-mu) ] 
                            \_______________  ____________/       \_________  ________/
                                            \/                              \/
                                          logcoef                           q

    Let z = inv(C).(x-mu)
    ==> z is the solution of C.z = x-mu
    And then we have q = (x-mu)'.z

    So computing q is simply a matter of solving this linear equation in z,
    and then computing q.

    
*/


real logOfCompactGaussian(const Vec& x, const Vec& mu, 
                          const Vec& eigenvalues, const Mat& eigenvectors, real gamma, bool add_gamma_to_eigenval)
{
  // cerr << "logOfCompact: mu = " << mu << endl;

  int d = x.length();
  static Vec x_minus_mu;
  x_minus_mu.resize(d);
  real* px = x.data();
  real* pmu = mu.data();
  real* pxmu = x_minus_mu.data();
    
  real sqnorm_xmu = 0;
  for(int i=0; i<d; i++)
    {
      real val = *px++ - *pmu++;
      sqnorm_xmu += val*val;
      *pxmu++ = val;
    }
    
  double log_det = 0.;
  double inv_gamma = 1./gamma;
  int kk = eigenvalues.length();
  double q = inv_gamma * sqnorm_xmu;
  int i;
  for(i=0; i<kk; i++)
    {
      double lambda_i = eigenvalues[i];
      if(add_gamma_to_eigenval)
        lambda_i += gamma;
      if(lambda_i<=0) // we've reached a 0 eigenvalue, stop computation here.
        break;
      log_det += log(lambda_i);
      q += (1./lambda_i - inv_gamma)*square(dot(eigenvectors(i),x_minus_mu));
    }
  if(kk<d)
    log_det += log(gamma)*(d-i);

  double logp = -0.5*( d*log(2*M_PI) + log_det + q);
  // cerr << "logOfCompactGaussian q=" << q << " log_det=" << log_det << " logp=" << logp << endl;
  // exit(0);
  return real(logp);
}

real logOfNormal(const Vec& x, const Vec& mu, const Mat& C)
{
  int n = x.length();
  static Vec x_mu;
  static Vec z;
  static Vec y;
  y.resize(n);
  z.resize(n);
  x_mu.resize(n);
  substract(x,mu,x_mu);

  static Mat L;
  // Perform Cholesky decomposition
  choleskyDecomposition(C, L);

  // get the log of the determinant: 
  // det(C) = square(product_i L_ii)
  double logdet = 0;
  for(int i=0; i<n; i++)
    logdet += log(L(i,i));
  logdet += logdet;

  // Finally find z, such that C.z = x-mu
  choleskySolve(L, x_mu, z, y);

  double q = dot(x_mu, z);
  double logp = -0.5*( n*log(2*M_PI) + logdet + q);
  // cerr << "logOfNormal q=" << q << " logdet=" << logdet << " logp=" << logp << endl;
  return real(logp);
}

real logPFittedGaussian(const Vec& x, const Mat& X, real lambda)
{
  static Mat C;
  static Vec mu;
  computeMeanAndCovar(X, mu, C);
  addToDiagonal(C, lambda);
  return logOfNormal(x, mu, C);
}



%> // end of namespace PLearn

