// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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


 
/*
* $Id: VMat_maths.cc,v 1.17 2004/05/04 14:45:38 yoshua Exp $
* This file is part of the PLearn library.
******************************************************* */
#include "VMat_maths.h"
#include "TMat_maths.h"
#include "IntVecFile.h"
#include "MemoryVMatrix.h"
#include "ShiftAndRescaleVMatrix.h"
#include "ExtendedVMatrix.h"
#include "Array.h"
#include "random.h"
#include "TmpFilenames.h"
#include "fileutils.h"
#include "PLMPI.h"
#include <vector>
#include "VecStatsCollector.h"
#include "ConditionalStatsCollector.h"
#include "stats_utils.h"

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

void computeStats(VMat m, VecStatsCollector& st)
{
  st.forget();
  st.setFieldNames(m->fieldNames());
  Vec v(m.width());
  int l = m.length();
  ProgressBar pbar(cerr, "computing statistics", l);
  for(int i=0; i<l; i++)
    {
      pbar(i);
      m->getRow(i,v);
      st.update(v);
    }
  st.finalize();
}

TVec<StatsCollector> computeStats(VMat m, int maxnvalues)
{
  int w = m.width();
  int l = m.length();
  TVec<StatsCollector> stats(w, StatsCollector(maxnvalues));
  Vec v(w);
  ProgressBar pbar(cerr, "computing statistics", l);
  for(int i=0; i<l; i++)
    {
      m->getRow(i,v);
      for(int j=0; j<w; j++)
        stats[j].update(v[j]);
      pbar(i);
    }
  return stats;
}

  //! returns the cooccurence statistics conditioned on the given field
PP<ConditionalStatsCollector> computeConditionalStats(VMat m, int condfield, TVec<RealMapping> ranges)
{
  PP<ConditionalStatsCollector> condst = new ConditionalStatsCollector;
  condst->setBinMappingsAndCondvar(ranges, condfield);
  int l = m.length();
  int w = m.width();
  Vec v(w);
  for(int i=0; i<l; i++)
    {
      if(i%10000==0)
        cerr << "computeConditionalStats: row " << i << " of " << l << endl;
      m->getRow(i,v);
      condst->update(v);
    }
  return condst;
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
      if(m) 
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
    if(m)
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
  static Vec input;
  static Vec target;
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
  static Vec input;
  static Vec target;
  static Vec offset;
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
  static Vec input;
  static Vec target;
  static Vec offset;
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
    for(int i=0;i<input.size();i++)
    {
      real xi = input[i]-offset[i];
      var[i]+=weight*xi*xi;
    }
    multiplyAcc(meanvec,input,weight);
  }
  meanvec /= weightsum;
  var /= weightsum;
  for(int i=0;i<input.size();i++)
  {
    real mu=meanvec[i]-offset[i];
    var[i]-=mu*mu;
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
  static Vec samplevec;
  static Vec diffvec;
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

VMat grep(VMat d, int col, Vec values, bool exclude)
{
  Vec indices(d.length());
  int nrows = 0;
  for(int i=0; i<d.length(); i++)
    {
      bool contains = values.contains(d(i,col));
      if( (!exclude && contains) || (exclude && !contains) )
        indices[nrows++] = i;
    }
  indices = indices.subVec(0,nrows);
  return d.rows(indices.copy());
}

//! returns a map mapping all different values appearing in column col to their number of occurences
map<real, int> countOccurencesInColumn(VMat m, int col)
{
  map<real, int> counts; // result we will return
  map<real, int>::iterator found;
  int l = m.length();
  for(int i=0; i<l; i++)
    {
      real val = m(i,col);
      found = counts.find(val);
      if(found==counts.end())
        counts[val] = 1;
      else
        found->second++;
    }
  return counts;
}

//! returns a map mapping all different values appearing in column col to a vector of the corresponding row indices in the VMat
map<real, TVec<int> > indicesOfOccurencesInColumn(VMat m, int col)
{
  map< real, TVec<int> > indices; // result we will return
  map<real, int> counts = countOccurencesInColumn(m,col);
  map<real, int>::iterator it = counts.begin();  
  map<real, int>::iterator itend = counts.end();
  for(; it!=itend; ++it)
    {
      indices[it->first].resize(it->second); // allocate the exact amount of memory
      indices[it->first].resize(0); // reset the size to 0 so we can do appends...
    }
  int l = m.length();
  for(int i=0; i<l; i++)
    indices[m(i,col)].push_back(i);
  return indices;
}

VMat grep(VMat d, int col, Vec values, const string& indexfile, bool exclude)
{
  if(!file_exists(indexfile))
    {
      IntVecFile indices(indexfile,true);
      for(int i=0; i<d.length(); i++)
        {
          bool contains = values.contains(d(i,col));
          if( (!exclude && contains) || (exclude && !contains) )
            indices.append(i);
        }
    }
  return d.rows(indexfile);
}

VMat filter(VMat d, const string& indexfile)
{
  if(!file_exists(indexfile) || file_size(indexfile)==0)
  {
    IntVecFile indices(indexfile,true);
    Vec v(d.width());
    for(int i=0; i<d.length(); i++)
    {
      d->getRow(i,v);
      if(!v.hasMissing())
        indices.append(i);
    }
  }
  return d.rows(indexfile);
}


VMat shuffle(VMat d)
{
  Vec indices(0, d.length()-1, 1); // Range-vector
  shuffleElements(indices);
  return d.rows(indices);
}

VMat bootstrap(VMat d, bool reorder, bool norepeat)
{
  Vec indices;
  if (norepeat)
  {
    indices = Vec(0, d.length()-1, 1); // Range-vector
    shuffleElements(indices);
    indices = indices.subVec(0,int(0.667 * d.length()));
    if (reorder)
      sortElements(indices);
    return d.rows(indices);
  }
  else
  {
    indices.resize(d.length());
    for (int i=0;i<d.length();i++)
      indices[i] = uniform_multinomial_sample(d.length());
  }
  if (reorder)
    sortElements(indices);
  return d.rows(indices);
}

Mat transposeProduct(VMat m)
{
  Mat result(m.width(),m.width());
  
  Vec v(m.width());
  Mat vrowmat = rowmatrix(v);

  for(int i=0; i<m.length(); i++)
    {
      m->getRow(i,v);
      transposeProductAcc(result, vrowmat,vrowmat);
    }
  return result;
}

Mat transposeProduct(VMat m1, VMat m2)
{
  if(m1.length()!=m2.length())
    PLERROR("in Mat transposeProduct(VMat m1, VMat m2) arguments have incompatible dimensions");
  
  Mat result(m1.width(),m2.width());
  
  Vec v1(m1.width());
  Vec v2(m2.width());
  Mat v1rowmat = rowmatrix(v1);
  Mat v2rowmat = rowmatrix(v2);

  for(int i=0; i<m1.length(); i++)
    {
      m1->getRow(i,v1);
      m2->getRow(i,v2);
      transposeProductAcc(result, v1rowmat,v2rowmat);
    }
  return result;
}

Vec transposeProduct(VMat m1, Vec v2)
{
  if(m1.length()!=v2.length())
    PLERROR("in Mat transposeProduct(VMat m1, Vec v2) arguments have incompatible dimensions");
  
  Vec result(m1.width(),1);
  result.clear();
  
  Vec v1(m1.width());
  for(int i=0; i<m1.length(); i++)
  {
    m1->getRow(i,v1);
    result += v1 * v2[i];
  }
  return result;
}

Mat productTranspose(VMat m1, VMat m2)
{
  if(m1.width()!=m2.width())
    PLERROR("in Mat productTranspose(VMat m1, VMat m2) arguments have incompatible dimensions");

  int m1l = (m1.length());
  int m2l = (m2.length());
  int w = (m1.width());  
  Mat result(m1l,m2l);
  
  Vec v1(w);
  Vec v2(w);

  for(int i=0; i<m1l; i++)
  {
    m1->getRow(i,v1);
    for(int j=0; j<m2l; j++)
    {
      m2->getRow(j,v2);
      result(i,j) = dot(v1,v2);
    }
  }
  return result;
}

Mat product(Mat m1, VMat m2)
{
  if(m1.width()!=m2.length())
    PLERROR("in Mat product(VMat m1, VMat m2) arguments have incompatible dimensions");
  
  Mat result(m1.length(),m2.width());
  result.clear();
  
  Vec v2(m2.width());
  Mat v2rowmat = rowmatrix(v2);

  for(int i=0; i<m1.width(); i++)
  {
    m2->getRow(i,v2);
    productAcc(result, m1.column(i), v2rowmat);
  }
  return result;
}

VMat transpose(VMat m1)
{
  return VMat(transpose(m1.toMat()));
}

real linearRegression(VMat inputs, VMat outputs, real weight_decay, Mat theta_t, 
                      bool use_precomputed_XtX_XtY, Mat XtX, Mat XtY, real& sum_squared_Y,
                      bool return_squared_loss, int verbose_every)
{
  if (outputs.length()!=inputs.length())
    PLERROR("linearRegression: inputs.length()=%d while outputs.length()=%d",inputs.length(),outputs.length());
  if (theta_t.length()!=inputs.width()+1 || theta_t.width()!=outputs.width())
    PLERROR("linearRegression: theta_t(%d,%d) should be (%dx%d)",
          theta_t.length(),theta_t.width(),inputs.width()+1,outputs.width());

  int inputsize = inputs.width();
  int targetsize = outputs.width();

  if(XtX.length()!=inputsize+1 || XtX.width()!=inputsize+1)
    PLERROR("In linearRegression: XtX should have dimensions %dx%d (inputs.width()+1)x(inputs.width()+1)",inputsize+1,inputsize+1);
  if(XtY.length()!=inputsize+1 || XtY.width()!=targetsize)
    PLERROR("In linearRegression: XtY should have dimensions %dx%d (inputs.width()+1)x(outputs.width())",inputsize+1,targetsize);

  if(!use_precomputed_XtX_XtY) // then compute them
    {
      VMat X = new ExtendedVMatrix(inputs,0,0,1,0,1.0); // prepend a first column of ones
      VMat Y = outputs;
      // *************
      // Do efficiently the following:
      // XtX << transposeProduct(X); // '<<' to copy elements (as transposeProduct returns a new matrix)
      // XtY << transposeProduct(X,Y); // same thing (remember '=' for Mat never copies elements)
      XtX.clear();
      XtY.clear();
      sum_squared_Y=0;
      Vec x(X.width());
      Vec y(Y.width());
      int l=X.length();
      for(int i=0; i<l; i++)
      {
        X->getRow(i,x);
        Y->getRow(i,y);
        externalProductAcc(XtX, x,x);
        externalProductAcc(XtY, x,y);
        sum_squared_Y += dot(y,y);
      }
      // *************
    }

  // add weight_decay on the diagonal of XX' (except for the bias)
  for (int i=1; i<XtX.length(); i++)
    XtX(i,i) += weight_decay;

  // now solve by Cholesky decomposition
  solveLinearSystemByCholesky(XtX,XtY,theta_t);

  real squared_loss=0;
  if (return_squared_loss)
  {
    // squared loss = sum_{ij} theta_{ij} (X'W X theta')_{ij} + sum_{t,i} Y_{ti}^2 - 2 sum_{ij} theta_{ij} (X'W Y)_{ij}
    Mat M(inputsize+1,targetsize);
    product(M,XtX,theta_t);
    squared_loss += dot(M,theta_t); // 
    squared_loss += sum_squared_Y;
    squared_loss -= 2*dot(XtY,theta_t);
  }
  return squared_loss;
}

Mat linearRegression(VMat inputs, VMat outputs, real weight_decay)
{
  int n = inputs.width()+1;
  int n_outputs = outputs.width();
  Mat XtX(n,n);
  Mat XtY(n,n_outputs);
  Mat theta_t(n,n_outputs);
  real sy=0;
  linearRegression(inputs, outputs, weight_decay, theta_t, false, XtX, XtY,sy);
  return theta_t;
}


real weightedLinearRegression(VMat inputs, VMat outputs, VMat gammas, real weight_decay, Mat theta_t, 
                              bool use_precomputed_XtX_XtY, Mat XtX, Mat XtY, real& sum_squared_Y,
                              real& sum_gammas, bool return_squared_loss, int verbose_every)
{
  int inputsize = inputs.width();
  int targetsize = outputs.width();
  if (outputs.length()!=inputs.length())
    PLERROR("linearRegression: inputs.length()=%d while outputs.length()=%d",inputs.length(),outputs.length());
  if (theta_t.length()!=inputsize+1 || theta_t.width()!=targetsize)
    PLERROR("linearRegression: theta_t(%d,%d) should be (%dx%d)",
            theta_t.length(),theta_t.width(),inputsize+1,targetsize);

  int l=inputs.length();
  if(!use_precomputed_XtX_XtY) // then compute them
  {
    XtX.clear();
    XtY.clear();
    VMat X = new ExtendedVMatrix(inputs,0,0,1,0,1.0); // prepend a first column of ones
    VMat Y = outputs;
    
    // Prepare to comnpute weighted XtX and XtY
    Vec x(X.width());
    Vec y(Y.width());
    real gamma_i;
    for(int i=0; i<l; i++)
    {
      X->getRow(i,x);
      Y->getRow(i,y);
      gamma_i = gammas(i,0);
      externalProductScaleAcc(XtX, x,x,gamma_i);
      externalProductScaleAcc(XtY, x,y,gamma_i);
      sum_squared_Y += gamma_i * dot(y,y);
      sum_gammas += gamma_i;
    }
  }

  // add weight_decay on the diagonal of XX' (except for the bias)
  for (int i=1; i<XtX.length(); i++)
    XtX(i,i) += weight_decay;

  // now solve by Cholesky decomposition
  solveLinearSystemByCholesky(XtX,XtY,theta_t);

  real squared_loss=0;
  if (return_squared_loss)
  {
    // squared loss = sum_{ij} theta_{ij} (X'W X theta')_{ij} + sum_{t,i} gamma_t*Y_{ti}^2 - 2 sum_{ij} theta_{ij} (X'W Y)_{ij}
    Mat M(inputsize+1,targetsize);
    product(M,XtX,theta_t);
    squared_loss += dot(M,theta_t); // 
    squared_loss += sum_squared_Y;
    squared_loss -= 2*dot(XtY,theta_t);
  }
  return squared_loss/l;
}

//!  Version that does all the memory allocations of XtX, XtY and theta_t. Returns theta_t
Mat weightedLinearRegression(VMat inputs, VMat outputs, VMat gammas, real weight_decay)
{
  int n = inputs.width()+1;
  int n_outputs = outputs.width();
  Mat XtX(n,n);
  Mat XtY(n,n_outputs);
  Mat theta_t(n,n_outputs);
  real sy=0;
  real sg=0;
  weightedLinearRegression(inputs, outputs, gammas, weight_decay, theta_t, false, XtX, XtY,sy,sg);
  return theta_t;
}


VMat rebalanceNClasses(VMat inputs, int nclasses, const string& filename)
{
  if (!file_exists(filename))
  {
    IntVecFile indices(filename, true);
    Vec last = inputs.lastColumn()->toMat().toVecCopy();
    const int len = last.length();
    Vec capacity(nclasses);
    Array<Vec> index(nclasses);
    Array<Vec> index_used(nclasses);
    for (int i=0; i<nclasses; i++) index[i].resize(len);
    for (int i=0; i<nclasses; i++) index_used[i].resize(len);
    real** p_index;
    p_index = new real*[nclasses];
    for (int i=0; i<nclasses; i++) p_index[i] = index[i].data();
    for (int i=0; i<nclasses; i++) index_used[i].clear();
    for (int i=0; i<len; i++)
    {
      int class_i = int(last[i]);
      *p_index[class_i]++ = i;
      capacity[class_i]++;
    }
    for (int i=0; i<nclasses; i++) index[i].resize(int(capacity[i]));
    for (int i=0; i<nclasses; i++) index_used[i].resize(int(capacity[i]));

    Mat class_length(nclasses,2);
    for (int i=0; i<nclasses; i++)
    {
      class_length(i,0) = capacity[i];
      class_length(i,1) = i;
    }
    sortRows(class_length);
    Vec remap = class_length.column(1).toVecCopy();

    vector<int> n(nclasses,0);
    int new_index = -1;
    for (int i=0; i<len; i++)
    {
      int c = i%nclasses;
      int c_map = int(remap[c]);
      if (c == 0)
      {
        if (n[0] == capacity[c_map]) n[0] = 0;
        new_index = int(index[c_map][n[0]++]);
      }
      else
      {
        if (n[c] == capacity[c_map])
        {
          n[c] = 0;
          index_used[c_map].clear();
        }
        bool index_found = false;
        int start_pos = binary_search(index[c_map], real(new_index));
        for (int j=start_pos+1; j<capacity[c_map]; j++)
        {
          if (index_used[c_map][j] == 0)
          {
            index_used[c_map][j] = 1;
            new_index = int(index[c_map][j]);
            index_found = true;
            n[c]++;
            break;
          }
        }
        if (!index_found)
        {
          for (int j=0; j<start_pos; j++)
          {
            if (index_used[c_map][j] == 0)
            {
              index_used[c_map][j] = 1;
              new_index = int(index[c_map][j]);
              index_found = true;
              n[c]++;
              break;
            }
          }
        }
        if (!index_found)
          PLERROR("In rebalanceNClasses:  something got wrong!"); 
      }
      indices.put(i, new_index);
    }

    delete[] p_index;
  }
  return inputs.rows(filename);
}

void fullyRebalance2Classes(VMat inputs, const string& filename, bool save_indices)
{
  if (!file_exists(filename))
  {
    int len = inputs.length();

    int n_zeros = 0;
    int n_ones = 0;
    Vec zeros(len);
    Vec ones(len);

    Vec last = inputs.lastColumn()->toMat().toVecCopy();
    for (int i=0; i<len;i++)
    {
      if (last[i] == 0)
        zeros[n_zeros++] = i;
      else
        ones[n_ones++] = i;
    }
    zeros.resize(n_zeros);
    ones.resize(n_ones);

    TmpFilenames tmpfile(1);
    string fname = save_indices ? filename : tmpfile.addFilename();
    IntVecFile indices(fname, true);
    int max_symbols = MAX(n_zeros, n_ones);
    for (int i=0; i<max_symbols; i++)
    {
      indices.put(2*i, int(zeros[i%n_zeros]));
      indices.put(2*i+1, int(ones[i%n_ones]));
    }
    if (!save_indices)
    {
      VMat vm = inputs.rows(fname);
      vm.save(filename);
    }
  }
}

VMat temporalThreshold(VMat distr, int threshold_date, bool is_before,
    int yyyymmdd_col)
{
  Vec indices(distr->length());
  int n_data = 0;
  for (int i=0; i<distr->length(); i++)
  {
    int reference_date = (int)distr(i, yyyymmdd_col);
    if (is_before ? reference_date<=threshold_date : reference_date>=threshold_date)
      indices[n_data++] = i;
  }
  indices.resize(n_data);

  return distr.rows(indices);
}

VMat temporalThreshold(VMat distr, int threshold_date, bool is_before,
    int yyyy_col, int mm_col, int dd_col)
{
  Vec indices(distr->length());
  int n_data = 0;
  for (int i=0; i<distr->length(); i++)
  {
    int reference_date = 10000*(int)distr(i, yyyy_col) + 100*(int)distr(i, mm_col) + (int)distr(i, dd_col);
    if (is_before ? reference_date<=threshold_date : reference_date>=threshold_date)
      indices[n_data++] = i;
  }
  indices.resize(n_data);

  return distr.rows(indices);
}

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
