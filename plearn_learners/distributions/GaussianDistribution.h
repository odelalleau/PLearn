// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// PLearn (A C++ Machine Learning Library)
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
   * $Id: GaussianDistribution.h,v 1.3 2003/05/14 21:15:32 jkeable Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnAlgo/GaussianDistribution.h */

#ifndef GaussianDistribution_INC
#define GaussianDistribution_INC

#include "Distribution.h"

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

  // This is a density estimation learner.
  // It uses a compact representation of a Gaussian, by keeping only the k 
  // top eigenvalues and associated eigenvectors of the covariance matrix.
  // All other eigenvalues are kept at the level of the k+1 th eigenvalue
  // Optionally, a constant sigma is first added to the diagonal of the covariance matrix.
  // The use(x,output) method returns log(p(x)) in output[0]

  class GaussianDistribution: public Distribution
  {
  protected:
    static Vec x_minus_mu;

    double compute_q(const Vec& x) const;
    void compact() { inv_lambda.compact(); V.compact(); } // compacify memory allocation

  protected:
    // "Learned" parameters
    Vec mu;
    Vec inv_lambda; // inverse of first k eigenvalues
    real inv_gamma; // inverse of all subsequent eigenvalues (0 indicates all subsequent eigenvalues were 0!)
    Mat V; // its *rows* are the orthonormal eigenvectors (this catually corresponds to V' in the above mathematical discussion)
    double logcoef; // log of normalization coeficient

    typedef Learner inherited;
    
  public:
    // Build options
    bool use_svd; // (default: false) should training use a SVD or eigendecomposition of the covariance matrix?
    int k; // maximum number of eigenvectors to keep
    double sigma; // (default 0) this is initially added to the diagonal of the empirical covariance matrix
    float ignore_weights_below; //!< When doing a weighted fitting (weightsize==1), points with a weight below this value will be ignored

    
  public:
    GaussianDistribution();

    DECLARE_NAME_AND_DEEPCOPY(GaussianDistribution);
    void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void train(VMat training_set);
    virtual double log_density(const Vec& x) const;

  protected:
    static void declareOptions(OptionList& ol);

  };

  DECLARE_OBJECT_PTR(GaussianDistribution);

%> // end of namespace PLearn

#endif
