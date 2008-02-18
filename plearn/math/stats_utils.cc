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


/*! \file PLearn/plearn/math/stats_utils.cc */

#include "stats_utils.h"
#include "TMat_maths.h"
#include "pl_erf.h"
#include "random.h"

namespace PLearn {
using namespace std;

/////////////////////////////
// SpearmanRankCorrelation //
/////////////////////////////
TMat<int> SpearmanRankCorrelation(const VMat &x, const VMat& y, Mat& r, bool ignore_missing)
{
    TMat<int> result;
    int n=x.length();
    if (n!=y.length())
        PLERROR("SpearmanRankCorrelation: x and y must have the same length");
    int wx=x.width();
    int wy=y.width();
    r.resize(wx,wy);
    r.clear();
    Mat y_ranks;
    TVec<int> n_ynonmissing = computeRanks(y.toMat(),y_ranks, ignore_missing);
    Mat x_rank(n,1);
    //real rank_normalization = sqrt(1.0/(n*n-1.0));
    real rank_normalization = 12.0/(n*(n-1.0)*(n-2.0));
    real half = n*0.5;
    // Vectors used only when 'ignore_missing' is true.
    Vec rank_normalization_miss;
    Vec half_miss;
    Mat y_copy, y_rankj, xi_placeholder;
    if (ignore_missing) {
        rank_normalization_miss.resize(wy);
        half_miss.resize(wy);
        result.resize(wx, wy);
        y_copy.resize(n, 1);
        y_rankj.resize(n, 1);
        xi_placeholder.resize(x.length(), 1);
    }
    ProgressBar pb("Computing Spearman rank correlation", wx);
    for (int i=0;i<wx; pb.update(++i))
    {
        Mat xi = x.column(i).toMat();
        const Vec& r_i = r(i);

        if (ignore_missing) {
            // Ignoring missing values is more complex, because we need to ignore them
            // both in x and y.
            for (int j = 0; j < wy; j++) {
                // We replace values in x_i associated to missing values in y by missing
                // values, so that they are not taken into account when computing ranks.
                // We need to make a copy to ensure we do not destroy any data.
                if (n_ynonmissing[j] < y.length()) {
                    Mat xi_back = xi; // Backup of original xi.
                    xi = xi_placeholder;
                    xi << xi_back;
                    for (int k = 0; k < y_ranks.length(); k++)
                        if (fast_exact_is_equal(y_ranks(k,j), -1)) // -1 rank <-> missing value
                            xi(k,0) = MISSING_VALUE;
                }
                TVec<int> n_nonmissing = computeRanks(xi, x_rank, ignore_missing);
                n = n_nonmissing[0];
                result(i,j) = n;
                rank_normalization = 12.0/(n*(n-1.0)*(n-2.0));
                half = n*0.5;
                y_rankj << y_ranks.column(j);
                // We need to recompute y's ranks if there were missing values in x.
                if (n < n_ynonmissing[j]) {
                    y_copy << y.column(j).toMat();
                    for (int k = 0; k < x_rank.length(); k++)
                        if (fast_exact_is_equal(x_rank(k,0), -1))
                            y_copy(k,0) = MISSING_VALUE;
                    computeRanks(y_copy, y_rankj, ignore_missing);
                } else
                    y_rankj << y_ranks.column(j);
                for (int k = 0; k < x_rank.length(); k++) {
                    real x_r = x_rank(k,0);
                    real y_r = y_rankj(k,0);
                    if (!fast_exact_is_equal(x_r, -1)) // -1 rank <-> missing value
                        r_i[j] += (x_r - half) * (y_r - half) * rank_normalization;
                }
            }
        } else {
            // Compute the rank of the i-th column of x
            computeRanks(xi,x_rank, ignore_missing);
            // Compute the Spearman rank correlation coefficient:
            for (int k=0;k<n;k++)
                for (int j=0;j<wy;j++)
                {
                    //real delta = (x_rank(k,0) - y_ranks(k,j))*rank_normalization;
                    // r_i[j] += delta*delta;
                    r_i[j] += (x_rank(k,0) - half) * (y_ranks(k,j)-half) * rank_normalization;
                }
        }
        for (int j=0;j<wy;j++)
            if (r_i[j]<-1.01 || r_i[j]>1.01)
                PLWARNING("SpearmanRankCorrelation: weird correlation coefficient, %f for %d-th input, %d-target",
                          r_i[j],i,j);
    }
    return result;
}

//! Return P(|R|>|r|) two-sided p-value for the null-hypothesis that
//! there is no monotonic dependency, with r the observed Spearman Rank 
//! correlation between two paired samples of length n. The p-value
//! is computed by taking advantage of the fact that under the null
//! hypothesis r*sqrt(n-1) converges to a Normal(0,1), if n is LARGE ENOUGH (approx. > 30).
real testNoCorrelationAsymptotically(real r, int n)
{
    real fz = fabs(r)*sqrt(n-1.0);
    return (1-gauss_01_cum(fz)) + gauss_01_cum(-fz);
}

void testSpearmanRankCorrelationPValues(const VMat &x, const VMat& y, Mat& pvalues, bool ignore_missing)
{
    Mat r;
    testSpearmanRankCorrelation(x,y,r,pvalues, ignore_missing);
}

void testSpearmanRankCorrelation(const VMat &x, const VMat& y, Mat& r, Mat& pvalues, bool ignore_missing)
{
    int n = x.length();
    TMat<int> n_mat = SpearmanRankCorrelation(x,y,r, ignore_missing);
    pvalues.resize(r.length(),r.width());
    for (int i=0;i<r.length();i++)
        for (int j=0;j<r.width();j++)
            pvalues(i,j) = testNoCorrelationAsymptotically(r(i,j), ignore_missing ? n_mat(i,j) : n);
}


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

    if(n1==0 && n2==0)
    {
        PLWARNING("In max_cdf_diff(Vec, Vec) - both vector are empty!");
        return 0;
    }
    else if (n1==0 || n2==0)
        return 1;

    for(;;)
    {

        if(v1[i1]<v2[i2])
        {
            i1++;
            if(i1+1==n1)
                break;
        }
        else if(fast_exact_is_equal(v1[i1],v2[i2]))
        {
            i1++;i2++;
            if(i2==n2)
                break;
            else if(i1+1==n1)
                break;
            continue;
        }
        else
        {
            i2++;
            if(i2==n2)
                break;
        }

        if ((i1>0 && fast_exact_is_equal(v1[i1], v1[i1-1])) ||
            (i2>0 && fast_exact_is_equal(v2[i2], v2[i2-1])) ||
            (v1[i1]<v2[i2] && v1[i1+1]<v2[i2]))
            continue; // to deal with discrete-valued variables: only look at "changing-value" places

        real F1 = inv_n1*i1;
        real F2 = inv_n2*i2;
        real diff = fabs(F1-F2);
        if(diff>maxdiff)
            maxdiff = diff;

        // perr << "v1[" << i1 << "]=" << v1[i1] << "; v2[" << i2 << "]=" << v2[i2] << "; F1=" << F1 << "; F2=" << F2 << "; diff=" << diff << endl;
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

void KS_test(Vec& v1, Vec& v2, int conv, real& D, real& p_value)
{
    int n1 = v1.length();
    int n2 = v2.length();
    real N = (n1/real(n1+n2))*n2;
    D = max_cdf_diff(v1, v2);
    p_value = KS_test(D,N,conv);
}

//! This version work with nan value
void KS_test(const VMat& m1, const VMat& m2, const int conv, Vec& Ds, Vec& p_values,
             const bool report_progress)
{
    m1->compatibleSizeError(m2);
    Ds.resize(m1->width());
    p_values.resize(m1->width());
    PP<ProgressBar> pbar;
    if (report_progress)
        pbar = new ProgressBar("Computing Kologorov Smirnow two sample test",
                               m1->width());

#pragma omp parallel default(none) shared(pbar)
    {
        Vec row1(m1->length());
        Vec row2(m2->length());
#pragma omp for
    for(int col = 0;col<m1->width();col++)
    {
        row1->resize(m1->length());
        row2->resize(m2->length());
        m1->getColumn(col,row1);
        m2->getColumn(col,row2);
        remove_missing_inplace(row1);
        remove_missing_inplace(row2);
        real D;
        real p_value;
        KS_test(row1,row2,conv,D,p_value);
        Ds[col]=D;
        p_values[col]=p_value;
        if (report_progress)
#pragma omp critical
            pbar->updateone();

    }
    }
}
real KS_test(Vec& v1, Vec& v2, int conv)
{
    real D, ks_stat;
    KS_test(v1,v2,conv,D, ks_stat);
    return ks_stat;
}

real paired_t_test(Vec u, Vec v)
{
    int n = u.length();
    if( v.length() != n )
    {
        PLWARNING("paired_t_test:  "
                  "Can't make a paired t-test on to unequally lengthed vectors (%d != %d).",
                  n, v.length());
        return MISSING_VALUE;
    }

    real ubar = mean(u);
    real vbar = mean(v);
    Vec u2 = u - ubar;
    Vec v2 = v - vbar;

    return (ubar - vbar) * sqrt( n*(n-1) / sumsquare(u2-v2));
}

/**
 * Estimate the parameters of a Dirichlet by maximum-likelihood
 *
 * p(t,i) are the observed probabilities (all between 0 and 1)
 * for t-th case, i ranging from 0 to N-1.
 * alphal[i] are the resulting parameters of the Dirichlet,
 * with i ranging from 0 to N-1.
 */
void DirichletEstimatorMMoments(const Mat& p, Vec& alpha)
{
    static Vec mean_p, mean_p2, var_p; // NON-REENTRANT CODE
    int N=p.width();
    alpha.resize(N);
    mean_p.resize(N);
    mean_p2.resize(N);
    var_p.resize(N);
    columnMean(p, mean_p);
    columnSumOfSquares(p, mean_p2);
    mean_p2 *= real(1.0/N);
    columnVariance(p, var_p, mean_p);
    real log_sum_alpha = 0;
    for (int i=0;i<N;i++)
        log_sum_alpha += safeflog(mean_p[i]*(1-mean_p[i])/var_p[i]-1);
    log_sum_alpha /= (N-1);
    multiply(mean_p, exp(log_sum_alpha), alpha);
}

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
void DirichletEstimatorMaxLik(const Mat& p, Vec alpha)
{
    DirichletEstimatorMMoments(p,alpha);
    // int N=alpha.length(); Commented out to remove compiler warning.
    PLERROR("In DirichletEstimatorMaxLik - Not implemented yet");
    // Have a look at Tom Minka's paper on estimating Dirichlet parameters...
    // TO BE IMPLEMENTED
}


} // end of namespace PLearn

/* 

// Test code...

#include "random.h"
#include <plearn/display/Gnuplot.h>

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
    perr << '.';
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

    pout << md << endl;
    pout << KS_test(md, n1*n2/real(n1+n2)) << endl; 

    verify_ks();

    return 0;
}

*/


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
