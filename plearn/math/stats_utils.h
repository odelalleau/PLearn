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
   * $Id: stats_utils.h,v 1.1 2003/02/28 22:47:57 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/stats_utils.h */

#ifndef stats_utils_INC
#define stats_utils_INC

#include "Mat.h"

namespace PLearn <%
using namespace std;

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

       D = max_x | F_1(x) - F_2 |

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

//! Kolmogorov-Smirnov test. Computes D (the max abs dfference between the 2 cdfs) and the ks statistic.
//! A reasonable value for D is 10. The call sorts v1 and v2.
void KS_test(Vec& v1, Vec& v2, int conv, real& D, real& ks_stat);

//! Returns result of Kolmogorov-Smirnov test between 2 samples
//! The call sorts v1 and v2.
real KS_test(Vec& v1, Vec& v2, int conv=10);

%> // end of namespace PLearn


#endif
