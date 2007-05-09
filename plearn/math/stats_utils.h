// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Pascal Vincent, Yoshua Bengio

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


/*! \file PLearnLibrary/PLearnCore/stats_utils.h */

#ifndef stats_utils_INC
#define stats_utils_INC

#include <plearn/vmat/VMat.h>
#include "Mat.h"

namespace PLearn {
using namespace std;

//! Compute the Spearman Rank correlation statistic. It measures
//! how much of a monotonic dependency there is between two variables x and y
//! (column matrices). The statistic is computed as follows:
//!    r = 1 - 6 (sum_{i=1}^n (rx_i - ry_i)^2) / (n(n^2-1))
//! If x and y are column matrices than r is a 1x1 matrix.
//! If x and y have width wx and wy respectively than the
//! statistic is computed for each pair of column (the first
//! taken from x and the second from y) and r will be a symmetric
//! matrix size wx by wy upon return. N.B. If x holds in memory
//! than copying it to a matrix (toMat()) before calling this function will
//! speed up computation significantly.
//! If 'ignore_missing' is set to true, rows for which either x or y is missing
//! will be ignored.
//! The returned matrix is empty if 'ignore_missing' is false, otherwise it
//! contains the number of non-missing values for each pair of variables in x and y.
TMat<int> SpearmanRankCorrelation(const VMat &x, const VMat& y, Mat& r, bool ignore_missing = false);

//! Return P(|R|>|r|) two-sided p-value for the null-hypothesis that
//! there is no monotonic dependency, with r the observed 
//! correlation between two paired samples of length n. The p-value
//! is computed by taking advantage of the fact that under the null 
//! hypothesis (true corr=0), r*sqrt(n-1) converges to a Normal(0,1), if n is LARGE ENOUGH.
real testNoCorrelationAsymptotically(real r, int n);

//! Compute P(|R|>|r|) two-sided p-value for the null-hypothesis that
//! there is no monotonic dependency, with r the observed Spearman Rank 
//! correlation between two paired samples x and y of length n (column
//! matrices). The p-value is computed by taking advantage of the fact 
//! that under the null hypothesis r*sqrt(n-1) is Normal(0,1).
//! If x and y have width wx and wy respectively than the
//! statistic is computed for each pair of column (the first
//! taken from x and the second from y) and pvalues will be a symmetric
//! matrix size wx by wy upon return. N.B. If x holds in memory
//! than copying it to a matrix (toMat()) before calling this function will
//! speed up computation significantly.
//! See 'SpearmanRankCorrelation' for the meaning of 'ignore_missing'.
void testSpearmanRankCorrelationPValues(const VMat &x, const VMat& y, Mat& pvalues, bool ignore_missing = false);
//! same as above but return also in r the rank correlations
void testSpearmanRankCorrelation(const VMat &x, const VMat& y, Mat& r, Mat& pvalues, bool ignore_missing = false);

//! Returns the max of the difference between the empirical cdf of 2 series of values
//! Side-effect: the call sorts v1 and v2.
real max_cdf_diff(Vec& v1, Vec& v2);


/*! 
  Return the probability that the Kolmogorov-Smirnov statistic
  D takes the observed value or greater, given the null hypothesis
  that the distributions  that are compared are
  really identical. N is the effective number of samples 
  used for comparing the distributions. 
  The argument conv gives the precision with which
  this probability is computed. A value above 10 does not bring
  much improvement. Note that the statistic D can
  be obtained as follows:

  Comparing two empirical distributions from data sets D_1 and D_2:
  Let F_1(x) the empirical cumulative distribution of D_1 of size N_1, and
  let F_2(x) the empirical cumulative distribution of D_2 of size N_2. Then

  D = max_x | F_1(x) - F_2(x) |

  and the effective N is N_1 N_2 / (N_1 + N_2).

  Comparing a theoretical distribution F and a data set D of size N with 
  empirical cumulative distribution F_N:
 
  D  = max_x | F(x) - F_N(x) |

  This function returns the following

  P(D > observed d | same distributions) estimated by
  2 sum_{k=1}^{infty} (-1)^{k-1} exp(-2k^2 a^2)

  where a = sqrt(D*(sqrt(N)+0.12+0.11/sqrt(N)))

  Ref: Stephens, M.A. (1970), Journal of the Royal Statistical Society B, vol. 32, pp. 115-122.
*/
real KS_test(real D, real N, int conv=10);

/**
 * Kolmogorov-Smirnov test. Computes D (the max abs dfference between the 2 cdfs)
 * and p_value P(random variable D > observed D|no difference in true prob)
 * A reasonable value for D is 10. The call sorts v1 and v2.
 */
void KS_test(Vec& v1, Vec& v2, int conv, real& D, real& p_value);

/** 
 * Returns result of Kolmogorov-Smirnov test between 2 samples
 * The call sorts v1 and v2.
 */
real KS_test(Vec& v1, Vec& v2, int conv=10);

/**
 * Given two paired sets u and v of n measured values, the paired t-test 
 * determines whether they differ from each other in a significant way under 
 * the assumptions that the paired differences are independent and identically 
 * normally distributed.
 */
real paired_t_test(Vec u, Vec v);


/**
 * Estimate the parameters of a Dirichlet by maximum-likelihood
 *
 * p(t,i) are the observed probabilities (all between 0 and 1)
 * for t-th case, i ranging from 0 to N-1.
 * alphal[i] are the resulting parameters of the Dirichlet,
 * with i ranging from 0 to N-1.
 */
void DirichletEstimatorMMoments(const Mat& p, Vec& alpha);

/**
 * Estimate the parameters of a Dirichlet by maximum-likelihood
 *
 * p(t,i) are the observed probabilities (all between 0 and 1)
 * for t-th case, i ranging from 0 to N-1.
 * alphal[i] are the resulting parameters of the Dirichlet,
 * with i ranging from 0 to N-1.
 *
 * The method of moments is used to initialize the estimator,
 * and a globally convergent iteration is then followed.
 *
 */
void DirichletEstimatorMaxLik(const Mat& p, Vec& alpha);

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
