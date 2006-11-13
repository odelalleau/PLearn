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
 * $Id$
 ******************************************************* */

#ifndef RANDOM_H
#define RANDOM_H

#include "TMat.h"

namespace PLearn {
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
//double incbet(double x, double y, double z);

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
real  gaussian_mixture_mu_sigma(Vec& w, const Vec& mu, const Vec& sigma);

/*!   returns a gamma distributed random number   */
real  gamdev(int ia);
/*!   returns a poisson random number with lambda = "xm"   */
real  poidev(real xm);
/*!   returns a binomial random number with probability = 'pp' and trials number = 'n'   */
real  bnldev(real pp, int n=1);
//!  alias
inline real binomial_sample(real prob1) { return bnldev(prob1); }

/*!   returns a random deviate from a discrete distribution given explicitely by 'distribution'   */
int  multinomial_sample(const Vec& distribution);

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

//!  sample each element from the given set
void fill_random_discrete(const Vec& dest, const Vec& set);

//!  sample each element from Normal(mean,sdev^2) distribution
void fill_random_normal(const Vec& dest, real mean=0, real stdev=1);

//!  sample each element from multivariate Normal(mean,diag(sdev^2)) distribution
void fill_random_normal(const Vec& dest, const Vec& mean, const Vec& stdev);

void fill_random_uniform(const Mat& dest, real minval=0, real maxval=1);
void fill_random_normal(const Mat& dest, real mean=0, real sdev=1);

//! Fill dest with dest.length() unique indices of entries in (0,1,...n-1), chosen uniformly
//! i.e. sample multinomially but without replacement, so that each entry in (0...n-1) can occur 0 or once.
//! This method is not very efficient as it performs memory allocation of size n.
void random_subset_indices(const TVec<int>& dest, int n);

//!  randomly shuffle the entries of the TVector
template<class T>
void shuffleElements(const TVec<T>& vec)
{
    if(vec.length() <= 0)
        return;//don't try to shuffle an empty vec.
    
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

//! For each column of 'mat', sort the elements and put in the 'ranks' matrix
//! (of the same dimensions) the rank of original elements. More precisely,
//! Let mat(i,j) be the k-th largest element of column j, than ranks(i,j) will be k.
//! If the boolean 'ignore_missing' is true, missing values are ignored in the ranking,
//! and will be assigned rank -1. The returned vector is also filled with the number
//! of non-missing values in each column.
//! If 'ignore_missing' is false, the returned vector is empty, and there should not
//! be missing values in the input matrices (this could lead to crash / NaN).
template<class T>
TVec<int> computeRanks(const TMat<T>& mat, TMat<T>& ranks, bool ignore_missing = false)
{
    TVec<int> result;
    int width=mat.width();
    int n=mat.length();
    ranks.resize(n,width);
    TVec<Mat> sorted(width); 
    // Sort all the y's.
    for (int j=0;j<width;j++)
        sorted[j].resize(n,2);
    if (ignore_missing) {
        // We do not know in advance how many non-missing values there are.
        for (int j = 0; j < width; j++)
            sorted[j].resize(0,2);
        Vec val(2);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < width; j++) {
                val[0] = mat(i,j);
                if (!is_missing(val[0])) {
                    val[1] = i;
                    sorted[j].appendRow(val);
                }
            }
        result.resize(width);
        for (int j = 0; j < width; j++)
            result[j] = sorted[j].length();
    } else {
        for (int i=0;i<n;i++)
        {
            for (int j=0;j<width;j++)
            {
                sorted[j](i,0)=mat(i,j);
#ifdef BOUNDCHECK
                if (is_missing(sorted[j](i,0)))
                    PLERROR("In computeRanks - Found a missing value, but 'ignore_missing' is false");
#endif
                sorted[j](i,1)=i;
            }
        }
    }
    for (int j=0;j<width;j++)
    {
        shuffleRows(sorted[j]); // To randomly permute the order of elements which have the same value, i.e. their rank within their category
        sortRows(sorted[j]);
    }
    // Compute the ranks.
    if (ignore_missing)
        ranks.fill(-1);
    for (int j=0;j<width;j++)
        for (int i=0;i<sorted[j].length();i++)
            ranks(int(sorted[j](i,1)),j) = i;
    return result;
}

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
