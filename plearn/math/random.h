// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal, all rights reserved
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
   * $Id: random.h,v 1.2 2002/12/06 19:06:40 yoshua Exp $
   ******************************************************* */

#ifndef RANDOM_H
#define RANDOM_H

#include "TMat.h"

namespace PLearn <%
using namespace std;

  /*

  Special functions.
  -----------------
  */

/*!   returns the natural logarithm of the gamma function   */
real  log_gamma(real x);

/*! returns the natural logarithm of the beta function */
real log_beta(real x, real y);

/*! returns the incomplete beta function B_z(x,y)  (BUGGED?)*/
real incomplete_beta(real z, real x, real y);
/*! returns the incomplete beta function B_z(x,y)  */
double incbet(double x, double y, double z);

/*! Student-t cumulative distribution function */
real student_t_cdf(real t, int nb_degrees_of_freedom);

  /*!   
  Utilities for random numbers generation. 
  ---------------------------------------
  */

  /*!   initializes the random number generator with the cpu time   */
  void  seed();
  /*!   initialzes the random number generator with the given long "x"   */
  void  manual_seed(long x);
  /*!   returns the current seed used by the random number generator   */
  long  get_seed();

  /*!   returns a random number uniformly distributed between 0 and 1   */
  real  uniform_sample();
  /*!   returns a random number uniformly distributed between a and b   */
  real  bounded_uniform(real a,real b);

  /*!   returns an exponential distributed random number   */
  real  expdev();
  /*!   returns a random number gaussian with mean 0 and standard deviation 1   */
  real  gaussian_01();
  inline real normal_sample() { return gaussian_01(); }

  /*!   returns a random number gaussian with mean "mu" and standard dev "sigma"   */
  real  gaussian_mu_sigma(real mu, real sigma);

  /*!   returns a random number with mixture of gaussians, "w" is the mixture
     (must be positive numbers summing to 1), "mu" and "sigma" are the vectors
     of means and standard deviations for each gaussian   */
  real  gaussian_mixture_mu_sigma(Vec& w, Vec& mu, Vec& sigma);

  /*!   returns a gamma distributed random number   */
  real  gamdev(int ia);
  /*!   returns a poisson random number with lambda = "xm"   */
  real  poidev(real xm);
  /*!   returns a binomial random number with probability = 'pp' and trials number = 'n'   */
  real  bnldev(real pp, int n=1);
  //!  alias
  inline real binomial_sample(real prob1) { return bnldev(prob1); }

  /*!   returns a random deviate from a discrete distribution given explicitely by 'distribution'   */
  int  multinomial_sample(Vec& distribution);

  //! return an integer between 0 and N-1 with equal probabilities 
  int uniform_multinomial_sample(int N);

  //! sample with replacement the rows of source and put them in destination.
  template <class T>
  void bootstrap_rows(const TMat<T>& source, TMat<T> destination)
  {
    int N=source.length();
    destination.resize(N,source.width());
    for (int i=0;i<N;i++)
    {
      int j = uniform_multinomial_sample(N);
      destination(i) << source(j);
    }
  }

  //!  sample each element from uniform distribution U[minval,maxval]
  void fill_random_uniform(const Vec& dest, real minval=0, real maxval=1);

  //!  sample each element from Normal(mean,sdev^2) distribution
  void fill_random_normal(const Vec& dest, real mean=0, real stdev=1);

  //!  sample each element from multivariate Normal(mean,diag(sdev^2)) distribution
  void fill_random_normal(const Vec& dest, const Vec& mean, const Vec& stdev);

  void fill_random_uniform(const Mat& dest, real minval=0, real maxval=1);
  void fill_random_normal(const Mat& dest, real mean=0, real sdev=1);


//!  randomly shuffle the entries of the TVector
template<class T>
void shuffleElements(const TVec<T>& vec)
{
  T* v = vec.data();
  for(int i=0; i<vec.length(); i++)
    {
      int j = i+(int)(uniform_sample()*(vec.length()-i));
      // int j=(int)floor(i+uniform_sample()*(length()-i-1e-5));
      if(j!=i)
        {
          T tmp = v[i];
          v[i] = v[j];
          v[j] = tmp;
        }
    }
}


// Performs a random permutation of all the rows of this Mat
template<class T>
void shuffleRows(const TMat<T>& mat)
{
  for(int i=0; i<mat.length(); i++)
    {
      int j = i+int(uniform_sample()*(mat.length()-i));
      mat.swapRows(i,j);
    }
}


%> // end of namespace PLearn

#endif







