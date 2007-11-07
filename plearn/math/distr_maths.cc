// -*- C++ -*-

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
   * $Id$
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearn/plearn/math/distr_maths.cc */


#include "distr_maths.h"
#include "random.h"
#include "TMat_maths.h"
#include "pl_erf.h" 
#include "plapack.h"

namespace PLearn {
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
                          const Vec& eigenvalues, const Mat& eigenvectors,
                          real gamma, bool add_gamma_to_eigenval)
{
    // cerr << "logOfCompact: mu = " << mu << endl;

    int i;
    int d = x.length();
    static Vec x_minus_mu;
    x_minus_mu.resize(d);
    real* px = x.data();
    real* pmu = mu.data();
    real* pxmu = x_minus_mu.data();
    
    real sqnorm_xmu = 0;
    for(i=0; i<d; i++)
    {
        real val = *px++ - *pmu++;
        sqnorm_xmu += val*val;
        *pxmu++ = val;
    }
    
    double log_det = 0.;
    double inv_gamma = 0;
    if(gamma>0)
        inv_gamma = 1./gamma;
    int kk = eigenvalues.length();
    double q = inv_gamma * sqnorm_xmu;
    for(i=0; i<kk; i++)
    {
        double lambda_i = eigenvalues[i];
        if(add_gamma_to_eigenval)
            lambda_i += gamma;
        if(lambda_i<=0) // we've reached a 0 eigenvalue, stop computation here.
            break;
        log_det += pl_log(lambda_i);
        q += (1./lambda_i - inv_gamma)*square(dot(eigenvectors(i),x_minus_mu));
    }
    if(kk<d)
        log_det += pl_log(gamma)*(d-i);

    static real log_2pi = pl_log(2*M_PI);
    double logp = -0.5*( d*log_2pi + log_det + q);
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
        logdet += pl_log(L(i,i));
    logdet += logdet;

    // Finally find z, such that C.z = x-mu
    choleskySolve(L, x_mu, z, y);

    double q = dot(x_mu, z);
    double logp = -0.5*( n*pl_log(2*M_PI) + logdet + q);
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

//! Returns the density of a proportion x under a Beta(alpha,beta) distribution,
//! equal to 
//!    x^{alpha-1} (1-x}^{beta-1} / Beta(a,b)
//! where
//!    Beta(a,b) = Gamma(a)Gamma(b)/Gamma(a+b)
real beta_density(real x, real alpha, real beta)
{
    return exp(log_beta_density(x,alpha,beta));
}
real log_beta_density(real x, real alpha, real beta)
{
    return (alpha-1)*safelog(x) + (beta-1)*safelog(1-x)  - log_beta(alpha,beta);
}

// return log of Normal(x;mu, sigma2*I), i.e. density of a spherical Gaussian
real log_of_normal_density(Vec x, Vec mu, real sigma2)
{
    real lp=0;
    for (int i=0;i<x.length();i++)
        lp += gauss_log_density_var(x[i],mu[i],sigma2);
    return lp;
}

real log_rbf(Vec x, Vec mu, real sigma2)
{
    real lp=0;
    real inv_s=1.0/sigma2;
    for (int i=0;i<x.length();i++)
    {
        real diff = x[i]-mu[i];
        lp += -0.5*diff*diff*inv_s;
    }
    return lp;
}


// return log of Normal(x;mu, diag(sigma2)), i.e. density of a diagonal Gaussian
real log_of_normal_density(Vec x, Vec mu, Vec sigma2)
{
    real lp=0;
    for (int i=0;i<x.length();i++)
        lp += gauss_log_density_var(x[i],mu[i],sigma2[i]);
    return lp;
}

// return log of Normal(x;mu, Sigma), i.e. density of a full Gaussian,
// where the covariance Sigma is
//    Sigma = remainder_evalue*I + sum_i max(0,evalues[i]-remainder_evalue)*evectors(i)*evectors(i)'
// The eigenvectors are in the ROWS of matrix evectors (because of easier row-wise access in Mat's).
real log_of_normal_density(Vec x, Vec mu, Mat evectors, Vec evalues, real remainder_evalue)
{
    return logOfCompactGaussian(x,mu,evalues,evectors,remainder_evalue,false);
/*
    static Vec centered_x;
    int d=x.length();
    centered_x.resize(d);
    int k=evectors.length();
    real lp = -0.5 * d * Log2Pi;
    real irev = 0;
    substract(x,mu,centered_x);
    if (remainder_evalue>0)
    {
        irev = 1 / remainder_evalue;
        lp -= 0.5 * ( (d-k)*pl_log(remainder_evalue)  +  pownorm(centered_x) * irev );
        if (k>=d)
            PLERROR("log_of_normal_density: when remainder_evalue>0, there should be less e-vectors (%d) than dimensions (%d)",
                    k,d);
    }
    for (int i=0;i<k;i++)
    {
        real ev = evalues[i];
        if (ev<remainder_evalue)
            lp -= 0.5 * pl_log(ev);
        else
        {
            real iv = 1/ev - irev;
            lp -= 0.5 * ( pl_log(ev) + iv * square(dot(evectors(i),centered_x)));
        }
    }
    return lp;
*/
}

// return log of Normal(x;mu, Sigma), i.e. density of a full Gaussian,
// where the covariance Sigma is
//    Sigma = remainder_evalue*I + sum_i max(0,evalues[i]-remainder_evalue)*evectors(i)*evectors(i)'
// The eigenvectors are in the ROWS of matrix evectors (because of easier row-wise access in Mat's).
real log_fullGaussianRBF(Vec x, Vec mu, Mat evectors, Vec evalues, real remainder_evalue)
{
    static Vec centered_x;
    int d=x.length();
    centered_x.resize(d);
    int k=evectors.length();
    real lp = 0;
    real irev = 0;
    substract(x,mu,centered_x);
    if (remainder_evalue>0)
    {
        irev = 1 / remainder_evalue;
        lp -= 0.5 * pownorm(centered_x) * irev;
        if (k>=d)
            PLERROR("log_of_normal_density: when remainder_evalue>0, there should be less e-vectors (%d) than dimensions (%d)",
                    k,d);
    }
    for (int i=0;i<k;i++)
    {
        real ev = evalues[i];
        if (ev>=remainder_evalue)
        {
            real iv = 1/ev - irev;
            lp -= 0.5 * iv * square(dot(evectors(i),centered_x));
        }
    }
    return lp;
}

void addEigenMatrices(Mat A_evec, Vec A_eval, Mat B_evec, Vec B_eval, Mat C_evec, Vec C_eval, bool inverses)
{
    static Mat C;
    int d=A_evec.length();
    C.resize(d,d);
    C.clear();
    if (inverses)
        for (int i=0;i<d;i++)
        {
            real ai = A_eval[i], bi = B_eval[i];
            externalProductScaleAcc(C,A_evec(i),A_evec(i),ai!=0 ?1/ai:0);
            externalProductScaleAcc(C,B_evec(i),B_evec(i),bi!=0 ?1/bi:0);
        }
    else
        for (int i=0;i<d;i++)
        {
            externalProductScaleAcc(C,A_evec(i),A_evec(i),A_eval[i]);
            externalProductScaleAcc(C,B_evec(i),B_evec(i),B_eval[i]);
        }
    eigenVecOfSymmMat(C,d,C_eval,C_evec,false);
    if (inverses)
        for (int i=0;i<d;i++)
        {
            real ci = C_eval[i];
            if (ci!=0) C_eval[i] = 1/ci;
        }
}

//! Given weighted statistics of order 0, 1 and 2, compute first and second moments of a Gaussian.
//! 0-th order statistic: sum_w = sum_i w_i 
//! 1-st order statistic: sum_wx = sum_i w_i x_i
//! 2-nd order statistic: sum_wx2 = sum_i w_i x_i x_i'
//! and put the results in 
//!   mu = sum_wx / sum_w
//!   (cov_evectors, cov_evalues) = eigen-decomposition of cov = sum_wx2 / sum_w - mu mu'
//! with eigenvectors in the ROWS of cov_evectors.
//! eigenvalues are replaced by max(eigenvalues,min_variance).
void sums2Gaussian(real sum_w, Vec sum_wx, Mat sum_wx2, Vec mu, Mat cov_evectors, Vec cov_evalues, real min_variance)
{
    if (sum_w>0)
    {
        real normf=1.0/sum_w;
        // mu = sum_x / sum_1
        multiply(sum_wx,normf,mu);
        // sigma = sum_x2 / sum_1  - mu mu'
        multiply(sum_wx2,sum_wx2,normf);
        externalProductScaleAcc(sum_wx2,mu,mu,-1.0);
        // perform eigendecoposition of the covariance matrix
        eigenVecOfSymmMat(sum_wx2,mu.length(),cov_evalues,cov_evectors,false);
    }
    else 
    {
        cov_evalues.fill(1.0);
        identityMatrix(cov_evectors);
    }
    for (int i=0;i<cov_evalues.length();i++)
        if (cov_evalues[i]<min_variance)
            cov_evalues[i]=min_variance;
}

} // end of namespace PLearn


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
