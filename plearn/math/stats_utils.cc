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
   * $Id: stats_utils.cc,v 1.2 2003/03/01 16:00:46 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/stats_utils.h */

#include "stats_utils.h"
#include "TMat_maths.h"

namespace PLearn <%
using namespace std;

//! Returns the max of the difference between the empirical cdf of 2 series of values
real max_cdf_diff(Vec& v1, Vec& v2)
{  
  int n1 = v1.length();
  int n2 = v2.length();
  real inv_n1 = 1./n1;
  real inv_n2 = 1./n2;
  sortElements(v1);
  sortElements(v2);
  int i1=0;
  int i2=0;
  real maxdiff = 0;

  for(;;)
    {
      if(v1[i1]<v2[i2])
        {
          i1++;
          if(i1==n1)
            break;
        }
      else
        {
          i2++;
          if(i2==n2)
            break;
        }
      real F1 = inv_n1*i1;
      real F2 = inv_n2*i2;
      real diff = fabs(F1-F2);
      if(diff>maxdiff)
        maxdiff = diff;

      // cerr << "v1[" << i1 << "]=" << v1[i1] << "; v2[" << i2 << "]=" << v2[i2] << "; F1=" << F1 << "; F2=" << F2 << "; diff=" << diff << endl;
    } 

  return maxdiff;  
}


/*************************************************************/
/* 
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
real KS_test(real D, real N, int conv)
 {
  int k;
  real res = 0.0;
  real sn = sqrt((double)N);
  real ks = D*(sn+0.12+0.11/sn);
  real ks2 = ks*ks;
  for (k=1;k<=conv;k++) {
     real x = ((k % 2) ? 1 : -1) * exp( -2 * ks2 * k * k );
     if (k==conv)
       res += 0.5*x;
     else 
       res += x;
  }
  return 2 * res;
}

void KS_test(Vec& v1, Vec& v2, int conv, real& D, real& ks_stat)
{
  int n1 = v1.length();
  int n2 = v2.length();
  real N = (n1/real(n1+n2))*n2;
  D = max_cdf_diff(v1, v2);
  ks_stat = KS_test(D,N,conv);
}

real KS_test(Vec& v1, Vec& v2, int conv)
{
  real D, ks_stat;
  KS_test(v1,v2,conv,D, ks_stat);
  return ks_stat;
}

%> // end of namespace PLearn


/* 

// Test code...

#include "random.h"
#include "Gnuplot.h"

using namespace PLearn;

// should plot a uniform distribution
void verify_ks(int n1=1000, int n2=1000, int k=100)
{
  Vec v1(n1);
  Vec v2(n2);
  Vec ks(k);

  for(int i=0; i<k; i++)
    {
      fill_random_normal(v1, 0, 1);
      fill_random_normal(v2, 0.1, 1);
      // fill_random_uniform(v2, -0.5, 0.5);

      ks[i] = KS_test(v1,v2);
      cerr << '.';
    }

  Gnuplot gp;
  gp.plotcdf(ks);
  char s[100];
  cin.getline(s, 100);
}

int main()
{
  Vec v1(5);
  v1 << "1 2 5 9 14";

  Vec v2(6);
  v2 << "-1 4 12 14 25 3";

  real md = max_cdf_diff(v1,v2);

  int n1 = v1.length();
  int n2 = v2.length();

  cout << md << endl;
  cout << KS_test(md, n1*n2/real(n1+n2)) << endl; 

  verify_ks();

  return 0;
}

*/

