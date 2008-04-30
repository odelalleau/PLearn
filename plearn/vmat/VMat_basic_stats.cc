// -*- C++ -*-

// VMat_basic_stats.cc
//
// Copyright (C) 2004 Pascal Vincent
// Copyright (C) 2005 University of Montreal
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

// Authors: Pascal Vincent

/*! \file VMat_basic_stats.cc */

#include <plearn/base/Object.h>
#include "VMat_basic_stats.h"
//#include "VMat.h"
#include "MemoryVMatrix.h"
#include "ShiftAndRescaleVMatrix.h"
//#include <plearn/math/TMat_maths.h>
#include <plearn/math/stats_utils.h>
#include <plearn/math/VecStatsCollector.h>
#include <plearn/math/TMat_maths.h>
//#include <plearn/sys/PLMPI.h>

namespace PLearn {
using namespace std;

/////////////////////////
// computeWeightedMean //
/////////////////////////
void computeWeightedMean(const Vec& weights, const VMat& d, Vec& meanvec)
{
    VecStatsCollector sc;
    int n = d->length();
    if (weights.length() != n)
        PLERROR("In computeWeightedMean - weights.length() != d->length()");
    Vec row(d->width());
    for (int i = 0; i < n; i++) {
        d->getRow(i, row);
        sc.update(row, weights[i]);
    }
    sc.getMean(meanvec);
}

//////////////////
// computeRange //
//////////////////
void computeRange(const VMat& d, Vec& minvec, Vec& maxvec)
{
    int n = d->length();
    int w = d->width();
    minvec.resize(w);
    maxvec.resize(w);
    VecStatsCollector sc;
    Vec row(w);
    for (int i = 0; i < n; i++) {
        d->getRow(i, row);
        sc.update(row);
    }
    for (int j = 0; j < w; j++) {
        minvec[j] = sc.getStats(j).min();
        maxvec[j] = sc.getStats(j).max();
    }
}

////////////////////
// computeRowMean //
////////////////////
void computeRowMean(const VMat& d, Vec& meanvec)
{
    int n = d->length();
    meanvec.resize(n);
    Vec samplevec(d->width());
    for(int i = 0; i < n; i++)
    {
        d->getRow(i,samplevec);
        meanvec[i] = mean(samplevec);
    }
}

/////////////////
// computeMean //
/////////////////
void computeMean(const VMat& d, Vec& meanvec)
{
    Vec constant_weight(d->length(), 1.0);
    computeWeightedMean(constant_weight, d, meanvec);
}

///////////////////////
// computeBasicStats //
///////////////////////
Mat computeBasicStats(const VMat& m)
{
    // TODO Use StatsCollector instead ?
    Vec v(m.width());
    real* vdata = v.data();
    Mat stats(10,m.width());
    Vec mean_row = stats(MEAN_ROW);
    Vec stddev_row = stats(STDDEV_ROW);
    Vec min_row = stats(MIN_ROW);
    Vec max_row = stats(MAX_ROW);
    Vec nmissing_row = stats(NMISSING_ROW);
    Vec nzero_row = stats(NZERO_ROW);
    Vec npositive_row = stats(NPOSITIVE_ROW);
    Vec nnegative_row = stats(NNEGATIVE_ROW);
    Vec meanpos_row = stats(MEANPOS_ROW);
    Vec stddevpos_row = stats(STDDEVPOS_ROW);
    min_row.fill(FLT_MAX);
    max_row.fill(-FLT_MAX);

    for(int i=0; i<m.length(); i++)
    {
        m->getRow(i,v);
        for(int j=0; j<v.length(); j++)
        {
            real val = vdata[j];
            if(is_missing(val))
                nmissing_row[j]++;
            else
            {
                if(val<min_row[j])
                    min_row[j] = val;
                if(val>max_row[j])
                    max_row[j] = val;

                if(fast_exact_is_equal(val, 0.))
                    nzero_row[j]++;
                else if(val>0.)
                {
                    npositive_row[j]++;
                    mean_row[j] += val;
                    stddev_row[j] += val*val;
                    meanpos_row[j] += val;
                    stddevpos_row[j] += val*val;
                }
                else // val < 0.
                {
                    nnegative_row[j]++;
                    mean_row[j] += val;
                    stddev_row[j] += val*val;
                }
            }
        }
    }
    for(int j=0; j<stats.width(); j++)
    {
        real nnonmissing = nzero_row[j]+nnegative_row[j]+npositive_row[j];
        mean_row[j] /= nnonmissing;
        meanpos_row[j] /= npositive_row[j];
        stddev_row[j] = sqrt(stddev_row[j]/nnonmissing - square(mean_row[j]));
        stddevpos_row[j] = sqrt(stddevpos_row[j]/npositive_row[j] - square(meanpos_row[j]));
    }
    return stats;
}

/////////////////////////////
// computeConditionalMeans //
/////////////////////////////
// mean = sum/n
// variance = (sumsquare-square(sum)/n)/(n-1)
// stddev_of_mean = sqrt(variance/n);
// mse = sumsquare/n - square(sum/n)
// stddev_of_mse = variance*sqrt(2./n);
TVec<Mat> computeConditionalMeans(const VMat& trainset, int targetsize, Mat& basic_stats)
{
    if(!basic_stats)
        basic_stats = computeBasicStats(trainset);

    int inputsize = trainset.width()-targetsize;
    TVec<Mat> a(inputsize);
    for(int j=0; j<inputsize; j++)
    {
        real minval = basic_stats(MIN_ROW,j);
        real maxval = basic_stats(MAX_ROW,j);
        if(is_integer(minval) && is_integer(maxval) && maxval-minval<400)
        {
            a[j] = Mat(int(maxval-minval+1),2+targetsize*4);
            for(int k=0; k<a[j].length(); k++)
                a[j](k,0) = minval+k;
        }
    }

    Vec row(trainset.width());
    Vec input = row.subVec(0,inputsize);
    Vec target = row.subVec(inputsize,targetsize);
    for(int i=0; i<trainset.length(); i++)
    {
        trainset->getRow(i,row);
        for(int j=0; j<inputsize; j++)
        {
            Mat& m = a[j];
            if(m.isNotEmpty())
            {
                int k = int(input[j]-basic_stats(MIN_ROW,j));
                Vec m_k = m(k);
                m_k[1]++;
                for(int l=0; l<targetsize; l++)
                {
                    real targetval = target[l];
                    m_k[2+4*l] += targetval;
                    m_k[3+4*l] += square(targetval);
                }
            }
        }
    }

    // postprocessing:
    for(int j=0; j<inputsize; j++)
    {
        Mat& m = a[j];
        if(m.isNotEmpty())
        {
            for(int k=0; k<m.length(); k++)
            {
                Vec m_k = m(k);
                real n = m_k[1];
                if(n>0.)
                {
                    // replace sum by mean and sumsquare by variance
                    for(int l=0; l<targetsize; l++)
                    {
                        real sum = m_k[2+4*l];
                        real sumsquare = m_k[3+4*l];
                        real mean = sum/n;
                        real variance = (sumsquare-square(sum)/n)/(n-1);
                        real mse = sumsquare/n - square(sum/n);
                        real stddev_of_mean = sqrt(variance/n);
                        real stddev_of_mse = variance*sqrt(2./n);
                        m_k[2+4*l] = mean;
                        m_k[3+4*l] = stddev_of_mean;
                        m_k[4+4*l] = mse;
                        m_k[5+4*l] = stddev_of_mse;
                    }
                }
            }
        }
    }

    return a;
}

////////////////////////////
// computeMeanAndVariance //
////////////////////////////
void computeMeanAndVariance(const VMat& d, Vec& meanvec, Vec& variancevec,
                            real epsilon)
{
    VecStatsCollector sc;
    sc.epsilon = epsilon;
    sc.build();
    int n = d->length();
    Vec row(d->width());
    for (int i = 0; i < n; i++) {
        d->getRow(i, row);
        sc.update(row);
    }
    sc.getMean(meanvec);
    variancevec.resize(d->width());
    variancevec << sc.getVariance();
}

//////////////////////
// computeInputMean //
//////////////////////
void computeInputMean(const VMat& d, Vec& meanvec)
{
    VecStatsCollector sc;
    int n = d->length();
    Vec input, target;
    real weight;
    for (int i = 0; i < n; i++) {
        d->getExample(i, input, target, weight);
        sc.update(input, weight);
    }
    sc.getMean(meanvec);
}

//////////////////////////////
// computeInputMeanAndCovar //
//////////////////////////////
void computeInputMeanAndCovar(const VMat& d, Vec& meanvec, Mat& covarmat,
                              real epsilon)
{
    PLASSERT( d->inputsize() >= 0 );
    VecStatsCollector sc;
    sc.compute_covariance = true;
    sc.epsilon = epsilon;
    sc.build();
    int n = d->length();
    Vec input, target;
    real weight;
    for (int i = 0; i < n; i++) {
        d->getExample(i, input, target, weight);
        sc.update(input, weight);
    }
    sc.getMean(meanvec);
    sc.getCovariance(covarmat);
}

/////////////////////////////////
// computeInputMeanAndVariance //
/////////////////////////////////
void computeInputMeanAndVariance(const VMat& d, Vec& meanvec, Vec& var,
                                 real epsilon)
{
    PLASSERT( d->inputsize() >= 0 );
    VecStatsCollector sc;
    sc.epsilon=epsilon;
    sc.build();
    int n = d->length();
    Vec input, target;
    real weight;
    for (int i = 0; i < n; i++) {
        d->getExample(i, input, target, weight);
        sc.update(input, weight);
    }
    sc.getMean(meanvec);
    var.resize(d->inputsize());
    var << sc.getVariance();
}

///////////////////////////////
// computeInputMeanAndStddev //
///////////////////////////////
void computeInputMeanAndStddev(const VMat& d, Vec& meanvec, Vec& stddev,
                               real epsilon)
{
    computeInputMeanAndVariance(d, meanvec, stddev, epsilon);
    for (int i = 0; i < stddev.length(); i++) {
#ifdef BOUNDCHECK
        if (stddev[i] < 0)
            PLERROR("In computeInputMeanAndStddev - The computed variance should be >= 0");
#endif
        stddev[i] = sqrt(stddev[i]);
    }
}

/////////////////////////////////
// computeWeightedMeanAndCovar //
/////////////////////////////////
void computeWeightedMeanAndCovar(const Vec& weights, const VMat& d, Vec& meanvec, Mat& covarmat,
                                 real epsilon)
{
    VecStatsCollector sc;
    sc.compute_covariance = true;
    sc.epsilon = epsilon;
    sc.build();
    int n = d->length();
    Vec row(d->width());
    for (int i = 0; i < n; i++) {
        d->getRow(i, row);
        sc.update(row, weights[i]);
    }
    sc.getMean(meanvec);
    sc.getCovariance(covarmat);
}

/////////////////////////
// computeMeanAndCovar //
/////////////////////////
void computeMeanAndCovar(const VMat& d, Vec& meanvec, Mat& covarmat, real epsilon)
{
    VecStatsCollector sc;
    sc.compute_covariance = true;
    sc.epsilon = epsilon;
    sc.build();
    int n = d->length();
    Vec row(d->width());
    for (int i = 0; i < n; i++) {
        d->getRow(i, row);
        sc.update(row);
    }
    sc.getMean(meanvec);
    sc.getCovariance(covarmat);

    /* Commented out old code that had an optimized MPI version, but was probably
       not used anymore.

       int w = m->width();
       int l = m->length();
       meanvec.resize(w);
       covarmat.resize(w,w);

       MemoryVMatrix* memvm = dynamic_cast<MemoryVMatrix*>((VMatrix*)m);
       if(memvm)
       computeMeanAndCovar(m->toMat(), meanvec, covarmat);
       else
       {
       meanvec.clear();
       covarmat.clear();
       Vec v(w);

       ProgressBar progbar("Computing covariance",l);

       if(USING_MPI && PLMPI::synchronized && PLMPI::size>1)
       { //!<  Parallel implementation
       #if USING_MPI
       PLMPI::synchronized = false;

       if(!covarmat.isCompact())
       PLERROR("In computeMeanAndCovar: MPI implementation cannot handle non-compact covariance matrices, please pass a compact matrix");

       // temporary storages for mpi
       Vec meanvec_b(meanvec.length());
       Mat covarmat_b(covarmat.length(),covarmat.width());

       for(int i=PLMPI::rank; i<l; i+=PLMPI::size)
       {
       m->getRow(i,v);
       meanvec_b += v;
       externalProductAcc(covarmat_b, v, v);
       progbar(i);
       }

       MPI_Reduce(meanvec_b.data(), meanvec.data(), meanvec.length(), PLMPI_REAL, MPI_SUM, 0, MPI_COMM_WORLD);
       MPI_Bcast(meanvec.data(), meanvec.length(), PLMPI_REAL, 0, MPI_COMM_WORLD);
       MPI_Reduce(covarmat_b.data(), covarmat.data(), covarmat.size(), PLMPI_REAL, MPI_SUM, 0, MPI_COMM_WORLD);
       MPI_Bcast(covarmat.data(), covarmat.size(), PLMPI_REAL, 0, MPI_COMM_WORLD);

       PLMPI::synchronized = true;
       #endif
       }
       else //!<  default sequential implementation
       {
       for(int i=0; i<l; i++)
       {
       m->getRow(i,v);
       meanvec += v;
       externalProductAcc(covarmat, v, v);
       progbar(i);
       }
       }

       // get the real averages and covariances, and priors
       meanvec /= real(l);
       covarmat /= real(l);
       externalProductScaleAcc(covarmat,meanvec,meanvec,real(-1.));
       }
    */
}

void computeCovar(const VMat& d, const Vec& mu, Mat& covarmat, real epsilon)
{
    int w = d->width();
    int l = d->length();
    covarmat.resize(w,w);
    covarmat.clear();
    Vec samplevec(w);
    Vec diffvec(w);
    Mat sqdiffmat(w,w);
    for(int i=0; i<l; i++)
    {
        d->getRow(i,samplevec);
        samplevec -= mu;
        externalProductAcc(covarmat, samplevec, samplevec);
    }
    covarmat /= l-1;
    addToDiagonal(covarmat, epsilon);
}

void computeInputCovar(const VMat& d, const Vec& mu, Mat& covarmat, real epsilon)
{
    PLASSERT( d->inputsize() >= 0 );
    int w = d->inputsize();
    int l = d->length();
    covarmat.resize(w,w);
    covarmat.clear();
    Vec input(w);
    Vec target;
    real weight;
    Vec diffvec(w);
    Mat sqdiffmat(w,w);
    real weightsum = 0;
    for(int i=0; i<l; i++)
    {
        d->getExample(i, input, target, weight);
        input -= mu;
        externalProductScaleAcc(covarmat, input, input, weight);
        weightsum += weight;
    }
    covarmat *= real(1./weightsum);
    addToDiagonal(covarmat, epsilon);
}


//////////////////////////
// computeMeanAndStddev //
//////////////////////////
void computeMeanAndStddev(const VMat& d, Vec& meanvec, Vec& stddevvec,
                          real epsilon)
{
    computeMeanAndVariance(d, meanvec, stddevvec, epsilon);
    for(int i=0; i<stddevvec.length(); i++)
        stddevvec[i] = sqrt(stddevvec[i]);
}

//////////////////////////////
// autocorrelation_function //
//////////////////////////////
void autocorrelation_function(const VMat& data, Mat& acf)
{
    int T = data.length();
    int N = data.width();
    acf.resize(T-2, N);

    for(int delta=0; delta < T-2; delta++)
    {
        Vec sumT(N);
        Vec sumD(N);
        TVec<Vec> products(N);

        // t = delta
        for(int k=0; k < N; k++)
        {
            real ts = data(delta, k);
            real ds = data(0, k);

            sumT[k] = ts;
            sumD[k] = ds;

            products[k].resize(3);
            products[k][0] = ts*ts;
            products[k][1] = ds*ds;
            products[k][2] = ts*ds;
        }

        for(int t=delta+1; t < T; t++)
        {
            for(int k=0; k < N; k++)
            {
                real ts = data(t, k);
                real ds = data(t-delta, k);

                sumT[k] += ts;
                sumD[k] += ds;

                products[k][0] += ts*ts;
                products[k][1] += ds*ds;
                products[k][2] += ts*ds;
            }
        }

        // Actual computation of the correlation
        for(int k=0; k < N; k++)
        {
            int count = T-delta;
            real multiplied_var_t = products[k][0] - square(sumT[k])/count;
            real multiplied_var_d = products[k][1] - square(sumD[k])/count;
            acf(delta, k) = (products[k][2] - sumT[k]*sumD[k]/count) / sqrt(multiplied_var_t * multiplied_var_d);
        }
    }
}


///////////////
// normalize //
///////////////
VMat normalize(const VMat& d, const Vec& meanvec, const Vec& stddevvec)
{
    int inputsize = meanvec.length();

    Vec shiftvec(d.width(), 0.0);
    shiftvec.subVec(0,inputsize) << meanvec;
    negateElements(shiftvec);

    Vec scalevec(d.width(), 1.0);
    scalevec.subVec(0,inputsize) << stddevvec;
    invertElements(scalevec);

    return new ShiftAndRescaleVMatrix(d, shiftvec, scalevec);
}

///////////////
// normalize //
///////////////
VMat normalize(const VMat& d, int inputsize, int ntrain)
{
    Vec meanvec(inputsize);
    Vec stddevvec(inputsize);
    computeMeanAndStddev(d.subMat(0,0,ntrain,inputsize), meanvec, stddevvec);
    return normalize(d, meanvec, stddevvec);
}

///////////////
// normalize //
///////////////
VMat normalize(VMat d, int inputsize)
{
    return normalize(d, inputsize, d.length());
}

//////////////////
// correlations //
//////////////////
void correlations(const VMat& x, const VMat& y, Mat& r, Mat& pvalues, bool ignore_missing)
{
    TMat<int> n_nonmissing; // Store the number of non-missing values for each pair.
    int n=x.length();
    if (n!=y.length())
        PLERROR("correlations: x and y must have the same length");
    int wx=x.width();
    int wy=y.width();
    r.resize(wx,wy);
    r.clear();
    Mat sxy(wx,wy);
    Vec sx2(wx);
    Vec sy2(wy);
    Vec sx(wx);
    Vec sy(wy);
    Vec xt(wx);
    Vec yt(wy);
    Mat sy_m, sx_m, sy2_m, sx2_m;
    if (ignore_missing) {
        n_nonmissing.resize(wx, wy);
        sy_m.resize(wx, wy);
        sy2_m.resize(wx, wy);
        sx_m.resize(wx, wy);
        sx2_m.resize(wx, wy);
        n_nonmissing.fill(0);
        sy_m.fill(0);
        sy2_m.fill(0);
        sx_m.fill(0);
        sx2_m.fill(0);
    }
    for (int t=0;t<n;t++)
    {
        x->getRow(t,xt);
        y->getRow(t,yt);
        for (int j=0;j<wy;j++)
        {
            real ytj = yt[j];
            if (!ignore_missing) {
#ifdef BOUNDCHECK
                if (is_missing(ytj))
                    PLWARNING("In correlations - You should not compute correlations "
                              "with missing values and 'ignore_ missing' set to false");
#endif
                sy[j] += ytj;
                sy2[j] += ytj*ytj;
            }
            for (int i=0;i<wx;i++)
            {
                real xti = xt[i];
                if (ignore_missing) {
                    if (!is_missing(ytj) && !is_missing(xti)) {
                        sy_m(i,j) += ytj;
                        sy2_m(i,j) += ytj * ytj;
                        sx_m(i,j) += xti;
                        sx2_m(i,j) += xti * xti;
                        sxy(i,j) += xti * ytj;
                        n_nonmissing(i,j)++;
                    }
                } else {
#ifdef BOUNDCHECK
                    if (is_missing(xti))
                        PLWARNING("In correlations - You should not compute correlations "
                                  "with missing values and 'ignore_ missing' set to false");
#endif
                    sxy(i,j) += xti*ytj;
                    sx[i] += xti;
                    sx2[i] += xti*xti;
                }
            }
        }
    }
    for (int i=0;i<wx;i++)
        for (int j=0;j<wy;j++)
        {
            real nv; // = n * variance of x
            if (ignore_missing) {
                nv = sx2_m(i,j) - sx_m(i,j) / real(n_nonmissing(i,j)) * sx_m(i,j);
            } else {
                nv = sx2[i] - sx[i]/real(n)*sx[i];
            }
            if (nv>0) // don't bother if variance is 0
                if (ignore_missing)
                    r(i,j) = (n_nonmissing(i,j)*sxy(i,j)-sx_m(i,j)*sy_m(i,j)) /
                        sqrt( (n_nonmissing(i,j)*sx2_m(i,j)-sx_m(i,j)*sx_m(i,j)) *
                              (n_nonmissing(i,j)*sy2_m(i,j)-sy_m(i,j)*sy_m(i,j)));
                else
                    r(i,j) = (n*sxy(i,j)-sx[i]*sy[j])/sqrt((n*sx2[i]-sx[i]*sx[i])*(n*sy2[j]-sy[j]*sy[j]));
            else
                r(i,j) = 0;
            if (r(i,j)<-1.01 || r(i,j)>1.01)
                PLWARNING("correlation: weird correlation coefficient, %f for %d-th input, %d-target",
                          r(i,j),i,j);
        }
    pvalues.resize(wx, wy);
    for (int i=0;i<wx;i++)
        for (int j=0;j<wy;j++)
            if (ignore_missing)
                pvalues(i,j) = testNoCorrelationAsymptotically(r(i,j),n_nonmissing(i,j));
            else
                pvalues(i,j) = testNoCorrelationAsymptotically(r(i,j),n);
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
