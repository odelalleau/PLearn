// -*- C++ -*-

// VMat_basic_stats.cc
//
// Copyright (C) 2004 Pascal Vincent 
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
   * $Id: VMat_basic_stats.cc,v 1.1 2004/09/27 20:19:28 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file VMat_basic_stats.cc */


#include "VMat_basic_stats.h"
#include "VMat.h"
#include "MemoryVMatrix.h"
#include "ShiftAndRescaleVMatrix.h"
#include <plearn/math/TMat_maths_impl.h>
#include <plearn/math/stats_utils.h>
#include <plearn/sys/PLMPI.h>

namespace PLearn {
using namespace std;

/** Statistics functions **/
void computeWeightedMean(Vec weights, VMat d, Vec& meanvec)
{
  int w = d->width();
  int l = d->length();
  if(weights.length() != l) {
    PLERROR("In VMat.cc, method computeMean: weights.length() != d->length()\n");
  }
  meanvec.resize(w);
  meanvec.clear();
  Vec samplevec(w);
  for(int i=0; i<l; i++)
  {
      d->getRow(i,samplevec);
      meanvec += weights[i]*samplevec;
  }
  meanvec /= sum(weights);
}


void computeRange(VMat d, Vec& minvec, Vec& maxvec)
{
  int l = d.length();
  int w = d.width();
  minvec.resize(w);
  maxvec.resize(w);
  minvec.fill(FLT_MAX);
  maxvec.fill(-FLT_MAX);

  Vec v(w);
  for(int i=0; i<l; i++)
    {
      d->getRow(i,v);
      for(int j=0; j<w; j++)
        {
          minvec[j] = min(v[j],minvec[j]);
          maxvec[j] = max(v[j],maxvec[j]);
        }
    }
}

void computeRowMean(VMat d, Vec& meanvec)
{
  meanvec.resize(d->length());
  Vec samplevec(d->width());
  for(int i=0; i<d->length(); i++)
  {
    d->getRow(i,samplevec);
    meanvec[i] = mean(samplevec);
  }
}

void computeMean(VMat d, Vec& meanvec)
{
  meanvec.resize(d->width());
  meanvec.clear();
  Vec samplevec(d->width());
  for(int i=0; i<d->length(); i++)
    {
      d->getRow(i,samplevec);
      meanvec += samplevec;
    }
  meanvec /= real(d->length());
}

Mat computeBasicStats(VMat m)
{
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

        if(val==0.) 
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

// mean = sum/n
// variance = (sumsquare-square(sum)/n)/(n-1)
// stddev_of_mean = sqrt(variance/n);
// mse = sumsquare/n - square(sum/n)
// stddev_of_mse = variance*sqrt(2./n); 

Array<Mat> computeConditionalMeans(VMat trainset, int targetsize, Mat& basic_stats)
{
  if(!basic_stats)
    basic_stats = computeBasicStats(trainset);

  int inputsize = trainset.width()-targetsize;
  Array<Mat> a(inputsize);
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

void computeMeanAndVariance(VMat d, Vec& meanvec, Vec& variancevec)
{
  computeMean(d, meanvec);
  variancevec.resize(d->width());
  variancevec.clear();
  int w = d->width();
  int l = d->length();
  Vec samplevec(w);
  Vec diffvec(w);
  Vec sqdiffvec(w);
  for(int i=0; i<l; i++)
    {
      d->getRow(i,samplevec);
      //variancevec += powdistance(samplevec, meanvec, 2.0);
      substract(samplevec,meanvec,diffvec);
      multiply(diffvec,diffvec,sqdiffvec);
      variancevec += sqdiffvec;
    }
  variancevec /= real(l-1);
  
}

void computeInputMean(VMat d, Vec& meanvec)
{
  Vec input;
  Vec target;
  real weight;
  int l = d->length();
  int n = d->inputsize();
  real weightsum = 0;
  meanvec.resize(n);  
  meanvec.clear();
  for(int i=0; i<l; i++)
  {
    d->getExample(i,input,target,weight);
    weightsum += weight;
    multiplyAcc(meanvec,input,weight);
  }
  meanvec /= weightsum;
}

void computeInputMeanAndCovar(VMat d, Vec& meanvec, Mat& covarmat)
{
  Vec input;
  Vec target;
  Vec offset;
  real weight;
  int l = d->length();
  int n = d->inputsize();
  real weightsum = 0;
  meanvec.resize(n);  
  meanvec.clear();
  covarmat.resize(n,n);
  covarmat.clear();
  offset.resize(n);
  for(int i=0; i<l; i++)
  {
    d->getExample(i,input,target,weight);
    weightsum += weight;
    multiplyAcc(meanvec,input,weight);
    if (i==0) offset << input;
    input-=offset;
    externalProductScaleAcc(covarmat, input, input, weight);
  }
  meanvec *= 1/weightsum;
  covarmat *= 1/weightsum;
  offset-=meanvec;
  externalProductScaleAcc(covarmat, offset, offset, real(-1));
}

void computeInputMeanAndVariance(VMat d, Vec& meanvec, Vec& var)
{
  Vec input;
  Vec target;
  Vec offset;
  real weight;
  int l = d->length();
  int n = d->inputsize();
  real weightsum = 0;
  meanvec.resize(n);  
  meanvec.clear();
  var.resize(n);
  var.clear();
  offset.resize(n);
  for(int i=0; i<l; i++)
  {
    d->getExample(i,input,target,weight);
    if (i==0) offset<<input;
    weightsum+=weight;
    for(int j=0;j<input.size();j++)
    {
      real xj = input[j]-offset[j];
      var[j]+=weight*xj*xj;
    }
    multiplyAcc(meanvec,input,weight);
  }
  meanvec /= weightsum;
  var /= weightsum;
  for(int i=0;i<input.size();i++)
  {
    real mu=meanvec[i]-offset[i];
    var[i]-=mu*mu;
    if (var[i] < 0) {
      // This can happen because of numerical imprecisions.
      var[i] = 0;
    }
  }
}


void computeWeightedMeanAndCovar(Vec weights, VMat d, Vec& meanvec, Mat& covarmat)
{
  int w = d->width();
  int l = d->length();
  computeWeightedMean(weights, d, meanvec);
  covarmat.resize(w,w);
  covarmat.clear();
  Vec samplevec(w);
  Vec diffvec(w);
  real weight_sum = 0;
  for(int i=0; i<l; i++)
    {
      d->getRow(i,samplevec);
      substract(samplevec,meanvec,diffvec);
      real weight = weights[i];
      externalProductScaleAcc(covarmat, diffvec,diffvec, weight);
      weight_sum += weight;
    }
  covarmat *= real(1./weight_sum);
}

//! Last column of d is supposed to contain the weight for each sample
//! Samples with a weight less or equal to threshold will be ignored
//! (returns the sum of all weights actually used)
real computeWeightedMeanAndCovar(VMat d, Vec& meanvec, Mat& covarmat, real threshold)
{ 
  Vec samplevec;
  Vec diffvec;
  int w = d->width()-1;
  int l = d->length();
  samplevec.resize(w+1);
  diffvec.resize(w);
  Vec input = samplevec.subVec(0,w);
  real& weight = samplevec[w];

  real weightsum = 0;

  // Compute weighted mean
  meanvec.resize(w);
  meanvec.clear();
  for(int i=0; i<l; i++)
    {
      d->getRow(i,samplevec);
      if(weight>threshold)
        {
          multiplyAcc(meanvec, input, weight);
          weightsum += weight;
        }
    }

  meanvec *= real(1./weightsum);

  // Compute weighted covariance matrix
  covarmat.resize(w,w);
  covarmat.clear();
  for(int i=0; i<l; i++)
    {
      d->getRow(i,samplevec);
      if(weight>threshold)
        {
          substract(input,meanvec,diffvec);
          externalProductScaleAcc(covarmat, diffvec,diffvec, weight);
        }
    }

  covarmat *= real(1./weightsum);

  return weightsum;
}


//! computes empirical mean and covariance in a single pass
void computeMeanAndCovar(VMat m, Vec& meanvec, Mat& covarmat, ostream& logstream)
{
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

      ProgressBar progbar(logstream,"Computing covariance",l);

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
}

/* // two pass version
void computeMeanAndCovar(VMat d, Vec& meanvec, Mat& covarmat)
{
  int w = d->width();
  int l = d->length();
  computeMean(d, meanvec);
  covarmat.resize(w,w);
  covarmat.clear();
  Vec samplevec(w);
  Vec diffvec(w);
  Mat sqdiffmat(w,w);
  for(int i=0; i<l; i++)
    {
      d->getRow(i,samplevec);
      substract(samplevec,meanvec,diffvec);
      externalProduct(sqdiffmat, diffvec,diffvec);
      covarmat += sqdiffmat;
    }
  covarmat /= l-1;
}
*/

void computeMeanAndStddev(VMat d, Vec& meanvec, Vec& stddevvec)
{
  computeMeanAndVariance(d,meanvec,stddevvec);
  for(int i=0; i<stddevvec.length(); i++)
    stddevvec[i] = sqrt(stddevvec[i]);
}

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


VMat normalize(VMat d, Vec meanvec, Vec stddevvec)
{
  int inputsize = meanvec.length();

  Vec shiftvec(d.width(),0.0);
  shiftvec.subVec(0,inputsize) << meanvec;
  negateElements(shiftvec);

  Vec scalevec(d.width(),1.0);
  scalevec.subVec(0,inputsize) << stddevvec;
  invertElements(scalevec);
  
  return new ShiftAndRescaleVMatrix(d, shiftvec, scalevec);
}

VMat normalize(VMat d, int inputsize, int ntrain)
{
  Vec meanvec(inputsize);
  Vec stddevvec(inputsize);
  computeMeanAndStddev(d.subMat(0,0,ntrain,inputsize), meanvec, stddevvec);
  return normalize(d, meanvec, stddevvec);
}

VMat normalize(VMat d, int inputsize) 
{ return normalize(d,inputsize,d.length()); }

//! Compute the correlations between each of the columns of x and each of the 
//! columns of y. The results are in the x.width() by y.width() matrix r.
//! The p-values of the corresponding test (no correlation) are stored 
//! in the same-sized matrix pvalues.
void correlations(const VMat& x, const VMat& y, Mat& r, Mat& pvalues)
{
  int n=x.length();
  if (n!=y.length())
    PLERROR("correlations: x and y must have the same length");
  int wx=x.width();
  int wy=y.width();
  r.resize(wx,wy);
  r.clear();
  Mat sxy(wx,wy);
  Vec sx2(wx);
  Vec sy2(wx);
  Vec sx(wx);
  Vec sy(wx);
  Vec xt(wx);
  Vec yt(wy);
  for (int t=0;t<n;t++)
  {
    x->getRow(t,xt);
    y->getRow(t,yt);
    for (int j=0;j<wy;j++)
    {
      real ytj = yt[j];
      sy[j] += ytj;
      sy2[j] += ytj*ytj;
      for (int i=0;i<wx;i++)
      {
        real xti = xt[i];
        sxy(i,j) += xti*ytj;
        sx[i] += xti;
        sx2[i] += xti*xti;
      }
    }
  }
  for (int i=0;i<wx;i++)
    for (int j=0;j<wy;j++)
    {
      real nv = (sx2[i] - sx[i]/n*sx[i]); // = n * variance of x
      if (nv>0) // don't bother if variance is 0
        r(i,j) = (n*sxy(i,j)-sx[i]*sy[j])/sqrt((n*sx2[i]-sx[i]*sx[i])*(n*sy2[j]-sy[j]*sy[j]));
      else
        r(i,j) = 0;
      if (r(i,j)<-1.01 || r(i,j)>1.01)
        PLWARNING("correlation: weird correlation coefficient, %f for %d-th input, %d-target",
                  r(i,j),i,j);
    }
  pvalues.resize(r.length(),r.width());
  for (int i=0;i<r.length();i++)
    for (int j=0;j<r.width();j++)
      pvalues(i,j) = testNoCorrelationAsymptotically(r(i,j),n);

}

} // end of namespace PLearn
