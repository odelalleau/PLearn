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


 

/* *******************************************************      
   * $Id: VMat.cc,v 1.1 2002/07/30 09:01:28 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "VMat.h"
#include "TMat_maths.h"
#include "Array.h"
#include "random.h"
#include "Kernel.h"
#include "Func.h"
#include "TmpFilenames.h"
#include "fileutils.h"
#include <vector>
#include "TopNI.h"
#include "BottomNI.h"
//#include "PreproVMat.h"
// #include "DisplayUtils.h"


namespace PLearn <%
using namespace std;

union short_and_twobytes 
{
  unsigned short us;
  unsigned char twobytes[2];
};

/** VMField **/

VMField::VMField(const string& the_name, FieldType the_fieldtype)
  : name(the_name), fieldtype(the_fieldtype) {}

void VMField::print(ostream& out) const
{
  out << name << "\t type: ";
  switch(fieldtype)
    {
      case VMField::UnknownType:
        out << "UnknownType\n";
        break;
      case VMField::Continuous:
        out << "Continuous\n";
        break;
      case VMField::DiscrGeneral:
        out << "DiscrGeneral\n";
        break;
      case VMField::DiscrMonotonic:
        out << "DiscrMonotonic\n";
        break;
      case VMField::DiscrFloat:
        out << "DiscrFloat\n";
        break;
      case VMField::Date:
        out << "Date\n";
        break;
      default:
        PLERROR("Can't write name of type");
    }  //writeField(out,"fieldtype", fieldtype);
}

void VMField::write(ostream& out) const
{
  writeHeader(out,"VMField");
  writeField(out,"name", name);
  //writeField(out,"fieldtype", fieldtype);
  //out << "fieldtype: " << fieldtype << endl;
  writeFooter(out,"VMField");
}

void VMField::read(istream& in)
{
  readHeader(in,"VMField");
  readField(in,"name", name);
  //readField(in,"fieldtype", fieldtype);
  readFooter(in,"VMField");
}

/** VMFieldStat **/

VMFieldStat::VMFieldStat(int the_maxndiscrete)
  : nmissing_(0), nnonmissing_(0), npositive_(0), nnegative_(0), sum_(0.),
  sumsquare_(0.), min_(FLT_MAX), max_(-FLT_MAX),
  maxndiscrete(the_maxndiscrete) {}

void VMFieldStat::update(real val)
{
  if(is_missing(val))
    nmissing_++;
  else
  {
    nnonmissing_++;
    sum_ += val;
    sumsquare_ += val*val;
    if(val>0.)
      npositive_++;
    else if(val<0.)
      nnegative_++;
    if(val<min_)
      min_ = val;
    if(val>max_)
      max_ = val;
    if(maxndiscrete>0)
    {
      if(int(counts.size())<maxndiscrete)
        counts[val]++;
      else // reached maxndiscrete. Stop counting and reset counts.
      {
        maxndiscrete = -1;
        counts.clear();
      }
    }          
  }
}                           

void VMFieldStat::write(ostream& out) const
{
  out << nmissing_ << ' ' 
    << nnonmissing_ << ' '
    << npositive_ << ' '
    << nnegative_ << ' '
    << sum_ << ' '
    << sumsquare_ << ' '
    << min_ << ' '
    << max_ << "    ";

  out << counts.size() << "  ";

  map<real,int>::const_iterator it = counts.begin();
  map<real,int>::const_iterator countsend = counts.end();
  while(it!=countsend)
  {
    out << it->first << ' ' << it->second << "  "; 
    ++it;
  }
}

void VMFieldStat::read(istream& in)
{
  in >> nmissing_ >> nnonmissing_ >> npositive_ >> nnegative_ 
    >> sum_ >> sumsquare_ >> min_ >> max_ ;
  
  int ndiscrete;
  real value;
  int count;
  counts.clear();
  in >> ndiscrete;
  for(int k=0; k<ndiscrete; k++)
  {
    in >> value >> count;
    counts[value] = count;
  }
}


/** VMat **/

VMat::VMat() {}
VMat::VMat(VMatrix* d): PP<VMatrix>(d) {}
VMat::VMat(const VMat& d) :PP<VMatrix>(d) {}
VMat::VMat(const Mat& datamat): PP<VMatrix>(new MemoryVMatrix(datamat)) {}
VMat::~VMat() {}

VMat VMat::rows(TVec<int> rows_indices) const
{ return new SelectRowsVMatrix(*this, rows_indices); }

VMat VMat::rows(Vec rows_indices) const
{ return new SelectRowsVMatrix(*this, rows_indices); }

VMat VMat::rows(const string& indexfile) const
{ return new SelectRowsFileIndexVMatrix(*this, indexfile); }

VMat VMat::columns(TVec<int> columns_indices) const
{ return new SelectColumnsVMatrix(*this, columns_indices); }

VMat VMat::columns(Vec columns_indices) const
{ return new SelectColumnsVMatrix(*this, columns_indices); }

void VMat::precompute()
{ *this = new MemoryVMatrix(Mat(*this)); }
  
void VMat::precompute(const string& pmatfile, bool use_existing_file)
{ 
  if(!use_existing_file || !file_exists(pmatfile))
    save(pmatfile); 
  *this = new FileVMatrix(pmatfile); 
}


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

void computeWeightedMeanAndCovar(Vec weights, VMat d, Vec& meanvec, Mat& covarmat)
{
  int w = d->width();
  int l = d->length();
  computeWeightedMean(weights, d, meanvec);
  covarmat.resize(w,w);
  covarmat.clear();
  Vec samplevec(w);
  Vec diffvec(w);
  double weight_sum = 0;
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


void linearRegression(VMat inputs, VMat outputs, real weight_decay, Mat theta_t, 
                      bool use_precomputed_XtX_XtY, Mat XtX, Mat XtY, int verbose_every)
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
      Vec x(X.width());
      Vec y(Y.width());
      int l=X.length();
      for(int i=0; i<l; i++)
      {
        X->getRow(i,x);
        Y->getRow(i,y);
        externalProductAcc(XtX, x,x);
        externalProductAcc(XtY, x,y);
      }
      // *************
    }

  // add weight_decay on the diagonal of XX' (except for the bias)
  for (int i=1; i<XtX.length(); i++)
    XtX(i,i) += weight_decay;

  // now solve by Cholesky decomposition
  solveLinearSystemByCholesky(XtX,XtY,theta_t);
}

void weightedLinearRegression(VMat inputs, VMat outputs, VMat gammas, real weight_decay, Mat theta_t, 
                              bool use_precomputed_XtX_XtY, Mat XtX, Mat XtY, int verbose_every)
{
  if (outputs.length()!=inputs.length())
    PLERROR("linearRegression: inputs.length()=%d while outputs.length()=%d",inputs.length(),outputs.length());
  if (theta_t.length()!=inputs.width()+1 || theta_t.width()!=outputs.width())
    PLERROR("linearRegression: theta_t(%d,%d) should be (%dx%d)",
          theta_t.length(),theta_t.width(),inputs.width()+1,outputs.width());

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
    int l=X.length();
    for(int i=0; i<l; i++)
    {
      X->getRow(i,x);
      Y->getRow(i,y);
      gamma_i = gammas(i,0);
      externalProductScaleAcc(XtX, x,x,gamma_i);
      externalProductScaleAcc(XtY, x,y,gamma_i);
    }
  }

  // add weight_decay on the diagonal of XX' (except for the bias)
  for (int i=1; i<XtX.length(); i++)
    XtX(i,i) += weight_decay;

  // now solve by Cholesky decomposition
  solveLinearSystemByCholesky(XtX,XtY,theta_t);
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
    string fname = save_indices ? filename : tmpfile.newFile();
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

void split(VMat d, real test_fraction, VMat& train, VMat& test, int i)
{
  int ntest = int( test_fraction>=1.0 ?test_fraction :test_fraction*d.length() );
  int ntrain_before_test = d.length()-(i+1)*ntest;
  int ntrain_after_test = i*ntest;

  test = d.subMatRows(ntrain_before_test, ntest);
  if(ntrain_after_test == 0)
    train = d.subMatRows(0,ntrain_before_test);
  else if(ntrain_before_test==0)
    train = d.subMatRows(ntest, ntrain_after_test);
  else
    train =  vconcat( d.subMatRows(0,ntrain_before_test), 
                      d.subMatRows(ntrain_before_test+ntest, ntrain_after_test) );
}

Vec randomSplit(VMat d, real test_fraction, VMat& train, VMat& test)
{
  int ntest = int( test_fraction>=1.0 ?test_fraction :test_fraction*d.length() );
  int ntrain = d.length()-ntest;
  Vec indices(0, d.length()-1, 1); // Range-vector
  shuffleElements(indices);
  train = d.rows(indices.subVec(0,ntrain));
  test = d.rows(indices.subVec(ntrain,ntest));
  return indices;
}

void split(VMat d, real validation_fraction, real test_fraction, VMat& train, VMat& valid, VMat& test,bool do_shuffle)
{
  int ntest = int( test_fraction>=1.0 ?test_fraction :test_fraction*d.length() );
  int nvalid = int( validation_fraction>=1.0 ?validation_fraction :validation_fraction*d.length() );
  int ntrain = d.length()-(ntest+nvalid);
  Vec indices(0, d.length()-1, 1); // Range-vector
  if (do_shuffle){
    cout<<"shuffle !"<<endl;
    shuffleElements(indices);
  }
  train = d.rows(indices.subVec(0,ntrain));
  valid = d.rows(indices.subVec(ntrain,nvalid));
  test = d.rows(indices.subVec(ntrain+nvalid,ntest));
  cout<<"n_train : "<<ntrain<<endl<<"n_valid : "<<nvalid<<endl<<"n_test : "<<(d.length()-ntrain+nvalid)<<endl;
}    

void randomSplit(VMat d, real validation_fraction, real test_fraction, VMat& train, VMat& valid, VMat& test)
{
  split(d,validation_fraction,test_fraction,train,valid,test,true);
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

/** VMatrix **/

IMPLEMENT_NAME_AND_DEEPCOPY(VMatrix);

void VMatrix::declareOptions(OptionList & ol)
{
//  declareOption(ol, "writable", &VMatrix::writable, OptionBase::buildoption, "Are write operations permitted?");
  declareOption(ol, "length", &VMatrix::length_, OptionBase::buildoption, "length of the matrix");
  declareOption(ol, "width", &VMatrix::width_, OptionBase::buildoption, "width of the matrix");
}

void VMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  deepCopyField(fieldinfos, copies);
  deepCopyField(fieldstats, copies);
}

Array<VMField>& VMatrix::getFieldInfos() const
{
  int w = width();
  int ninfos = fieldinfos.size();
  if(ninfos!=w)
    {
      fieldinfos.resize(w);
      for(int j=ninfos; j<w; j++)
        fieldinfos[j] = VMField(tostring(j));
    }
  return fieldinfos;
}

void VMatrix::unduplicateFieldNames()
{
  map<string,vector<int> > mp;
  for(int i=0;i<width();i++)
    mp[getFieldInfos(i).name].push_back(i);
  map<string,vector<int> >::iterator it;
  for(it=mp.begin();it!=mp.end();++it)
    if(it->second.size()!=1)
      {
        vector<int> v=it->second;
        for(unsigned int j=0;j<v.size();j++)
          fieldinfos[v[j]].name+="."+tostring(j);
      }
}

int VMatrix::fieldIndex(const string& fieldname) const
{
  Array<VMField>& infos = getFieldInfos(); 
  for(int i=0; i<width(); i++)
    if(infos[i].name==fieldname)
      return i;
  return -1;
}

void VMatrix::printFields(ostream& out) const
{ 
  for(int j=0; j<width(); j++)
  {
    out << "Field #" << j << ":  " << getFieldInfos(j);
    if(fieldstats.size()>0)
    {
      const VMFieldStat& s = fieldStat(j);
      out << "nmissing: " << s.nmissing() << '\n';
      out << "nnonmissing: " << s.nnonmissing() << '\n';
      out << "npositive: " << s.npositive() << '\n';
      out << "nzero: " << s.nzero() << '\n';
      out << "nnegative: " << s.nnegative() << '\n';
      out << "mean: " << s.mean() << '\n';
      out << "stddev: " << s.stddev() << '\n';
      out << "min: " << s.min() << '\n';
      out << "max: " << s.max() << '\n';
      if(!s.counts.empty())
      {
        out << "value:counts :   ";
        map<real,int>::const_iterator it = s.counts.begin();
        map<real,int>::const_iterator countsend = s.counts.end();
        while(it!=countsend)
        {
          out << it->first << ':' << it->second << "  "; 
          ++it;
        }
      }
      out << endl << endl;
    }
  }
}

void VMatrix::computeStats()
{
  fieldstats = Array<VMFieldStat>(width());
  Vec row(width());
  for(int i=0; i<length(); i++)
    {
      getRow(i,row);
      for(int j=0; j<width(); j++)
        fieldstats[j].update(row[j]);
    }
}

void VMatrix::writeStats(ostream& out) const
{
  out << fieldstats.size() << endl;
  for(int j=0; j<fieldstats.size(); j++)
  {
    fieldstats[j].write(out);
    out << endl;
  }
}

void VMatrix::readStats(istream& in)
{
  int nfields;
  in >> nfields;
  if(nfields!=width())
    PLWARNING("In VMatrix::loadStats nfields differes from VMat width");

  fieldstats.resize(nfields);
  for(int j=0; j<fieldstats.size(); j++)
    fieldstats[j].read(in);
}

void VMatrix::loadStats(const string& filename)
{
  ifstream in(filename.c_str());
  if(!in)
    PLERROR("In VMatrix::loadStats Couldn't open file %s for reading",filename.c_str());
  readStats(in);
}

void VMatrix::saveStats(const string& filename) const
{
  ofstream out(filename.c_str());
  if(!out)
    PLERROR("In VMatrix::saveStats Couldn't open file %s for writing",filename.c_str());
  writeStats(out);
}


string VMatrix::fieldheader(int elementcharwidth)
{
  // Implementation not done yet

  return "VMatrix::fieldheader NOT YET IMPLEMENTED";
}

void VMatrix::saveFieldInfos(const string& filename) const
{
  ofstream f(filename.c_str());
  if(!f)
    PLERROR("In VMatrix::saveFieldInfos Couldn't open file %s for writing",filename.c_str());    
  writeFieldInfos(f);
}
void VMatrix::loadFieldInfos(const string& filename)
{
  ifstream f(filename.c_str());
  if(!f)
    PLERROR("In VMatrix::loadFieldInfos Couldn't open file %s for reading",filename.c_str());    
  readFieldInfos(f);
}
void VMatrix::writeFieldInfos(ostream& out) const
{
  for(int i= 0; i < fieldinfos.length(); ++i)
    out << fieldinfos[i].name << '\t' << fieldinfos[i].fieldtype << endl;
}
void VMatrix::readFieldInfos(istream& in)
{
  for(int i= 0; i < width(); ++i)
    {
      vector<string> v(split(pgetline(in)));
      switch(v.size())
	{
	case 1: declareField(i, v[0]); break;
	case 2: declareField(i, v[0], VMField::FieldType(toint(v[1]))); break;
	default: PLERROR("In VMatrix::readFieldInfos Format not recognized.  Each line should be '<name> {<type>}'.");
	}
    }
}

string VMatrix::getValString(int col, real val) const
{ return ""; }

real VMatrix::getStringVal(int col,const string & str) const
{
  return MISSING_VALUE;
}

void VMatrix::setMetaDataDir(const string& the_metadatadir) 
{ 
  metadatadir = the_metadatadir; 
  if(!force_mkdir(metadatadir))
    PLERROR("In VMatrix::setMetadataDir could not create directory %s",metadatadir.c_str());
  metadatadir = abspath(metadatadir);
}

//! returns the unconditonal statistics for the given field
TVec<StatsCollector> VMatrix::getStats()
{
  if(!field_stats)
    {
      string statsfile = getMetaDataDir() + "/stats.psave";
      if(isfile(statsfile) && getMtime()<mtime(statsfile))
        PLearn::load(statsfile, field_stats);
      else
        {
          field_stats = PLearn::computeStats(this, 2000);
          PLearn::save(statsfile, field_stats);
        }
    }
  return field_stats;
}

TVec<RealMapping> VMatrix::getRanges()
{
  TVec<RealMapping> ranges;
  string rangefile = getMetaDataDir() + "/ranges.psave";
  if(isfile(rangefile))
    PLearn::load(rangefile, ranges);
  else
    {
      ranges = computeRanges(getStats(),std::max(10,length()/200),std::max(10,length()/100) );
      PLearn::save(rangefile, ranges);
    }
  return ranges;
}

//! returns the cooccurence statistics conditioned on the given field
PP<ConditionalStatsCollector> VMatrix::getConditionalStats(int condfield)
{
  PP<ConditionalStatsCollector> condst;
  TVec<RealMapping> ranges = getRanges();
  string condstatfile = getMetaDataDir() + "/stats" + tostring(condfield) + ".psave";      
  string rangefile = getMetaDataDir() + "/ranges.psave";
  cerr <<  "rangefile: " << mtime(rangefile) << " condstatfile: " << mtime(condstatfile) << endl;
  if(mtime(rangefile)>mtime(condstatfile))
    {
      cerr << ">> Computing conditional stats conditioned on field " << condfield << endl;
      cerr << "   (because file " << rangefile << " was more recent than cache file " << condstatfile << ")" << endl; 
      condst = computeConditionalStats(this, condfield, ranges);
      PLearn::save(condstatfile, *condst);
    }
  else
    PLearn::load(condstatfile, *condst);      
  return condst;
}

// Eventually to be changed to pure virtual, once get has been implemented in all subclasses
// calls to sample can then be replaced by getRow everywhere
real VMatrix::get(int i, int j) const
{
  PLERROR("get(i,j) method not implemented for this VMat (name=%s), please implement.",classname().c_str());
  return 0.0;
}

void VMatrix::put(int i, int j, real value)
{
  PLERROR("put(i,j,value) method not implemented for this VMat, please implement.");
}

void VMatrix::getColumn(int j, Vec v) const
{
#ifdef BOUNDCHECK
  if(v.length() != length())
    PLERROR("In VMatrix::getColumn v must have the same length as the VMatrix");
#endif
  for(int i=0; i<v.length(); i++)
    v[i] = get(i,j);
}

void VMatrix::getSubRow(int i, int j, Vec v) const
{
  for(int k=0; k<v.length(); k++)
    v[k] = get(i,j+k);
}

void VMatrix::putSubRow(int i, int j, Vec v)
{
  for(int k=0; k<v.length(); k++)
    put(i, j+k, v[k]);
}

void VMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(v.length() != width())
    PLERROR("In VMatrix::getRow(i,v) length of v and width of VMatrix differ");
#endif
  getSubRow(i,0,v);
}

void VMatrix::putRow(int i, Vec v)
{
#ifdef BOUNDCHECK
  if(v.length() != width())
    PLERROR("In VMatrix::putRow(i,v) length of v and width of VMatrix differ");
#endif
  putSubRow(i,0,v);
}

void VMatrix::fill(real value)
{
  Vec v(width(), value);
  for (int i=0; i<length(); i++) putRow(i,v);
}

void VMatrix::appendRow(Vec v)
{
  PLERROR("This method (appendRow) not implemented by VMatrix subclass!");
}

void VMatrix::getMat(int i, int j, Mat m) const
{
#ifdef BOUNDCHECK
  if(i<0 || j<0 || i+m.length()>length() || j+m.width()>width())
    PLERROR("In VMatrix::getMat(i,j,m) OUT OF BOUNDS");
#endif
  for(int ii=0; ii<m.length(); ii++)
    {
      Vec v = m(ii);
      getSubRow(i+ii, j, v);
    }
}

void VMatrix::putMat(int i, int j, Mat m)
{
#ifdef BOUNDCHECK
  if(i<0 || j<0 || i+m.length()>length() || j+m.width()>width())
    PLERROR("In VMatrix::putMat(i,j,m) OUT OF BOUNDS");
#endif
  for(int ii=0; ii<m.length(); ii++)
    {
      Vec v = m(ii);
      putSubRow(i+ii, j, v);
    }
}

void VMatrix::compacify() {}


Mat VMatrix::toMat() const
{
  Mat m(length(),width());
  getMat(0,0,m);
  return m;
}

VMat VMatrix::subMat(int i, int j, int l, int w)
{ return new SubVMatrix(this,i,j,l,w); }

real VMatrix::dot(int i1, int i2, int inputsize) const
{ 
  real res = 0.;
  for(int k=0; k<inputsize; k++)
    res += get(i1,k)*get(i2,k);
  return res;
}

real VMatrix::dot(int i, const Vec& v) const
{
  real res = 0.;
  for(int k=0; k<v.length(); k++)
    res += get(i,k) * v[k];
  return res;
}

void VMatrix::getRow(int i, VarArray& inputs) const
{ 
  Vec v(width());
  getRow(i,v);
  inputs << v; 
}

void VMatrix::print(ostream& out) const
{
  Vec v(width());
  for(int i=0; i<length(); i++)
    {
      getRow(i,v);
      out << v << endl;
    }
}

void VMatrix::oldwrite(ostream& out) const
{
  writeHeader(out,"VMatrix");
  writeField(out,"length_", length_);
  writeField(out,"width_", width_);
  //writeField(out,"fieldinfos", fieldinfos);
  //writeField(out,"fieldstats", fieldstats);
  writeFooter(out,"VMatrix");
}

void VMatrix::oldread(istream& in)
{
  readHeader(in,"VMatrix");
  readField(in,"length_", length_);
  readField(in,"width_", width_);
  //readField(in,"fieldinfos", fieldinfos);
  //readField(in,"fieldstats", fieldstats);
  readFooter(in,"VMatrix");
}

VMatrix::~VMatrix()
{}

void VMatrix::save(const string& filename) const
{ savePMAT(filename); }

void VMatrix::savePMAT(const string& pmatfile) const
{
  if (width() == -1)
    PLERROR("In VMat::save Saving in a pmat file is only possible for constant width Distributions (where width()!=-1)");

  int nsamples = length();

  FileVMatrix m(pmatfile,nsamples,width());
  Vec tmpvec(width());

  ProgressBar pb(cout, "Saving to pmat", nsamples);

  for(int i=0; i<nsamples; i++)
    {
      getRow(i,tmpvec);
      m.putRow(i,tmpvec);
      pb(i);
    }
  string fieldinfosfname = pmatfile+".fieldnames";
  saveFieldInfos(fieldinfosfname);  
}

void VMatrix::saveDMAT(const string& dmatdir) const
{
  force_rmdir(dmatdir);  
  DiskVMatrix vm(dmatdir,width());
  Vec v(width());

  ProgressBar pb(cout, "Saving to dmat", length());

  for(int i=0;i<length();i++)
    {
      getRow(i,v);
      vm.appendRow(v);
      pb(i);
      //cerr<<i<<" "<<flush;
    }

  //save field names
  string fieldinfosfname = dmatdir+"/fieldnames";
  saveFieldInfos(fieldinfosfname);  
}

void VMatrix::saveAMAT(const string& amatfile) const
{
  ofstream out(amatfile.c_str());
  if (!out)
   PLERROR("In saveAscii could not open file %s for writing",amatfile.c_str());

  out << "#size: "<< length() << ' ' << width() << endl;
  out.precision(15);
  if(width()>0)
    {
      out << "#: ";
      for(int k=0; k<width(); k++)
	//there must not be any space in a field name...
        out << space_to_underscore(fieldName(k)) << ' ';
      out << "\n";
    }

  Vec v(width());

  ProgressBar pb(cout, "Saving to amat", length());

  for(int i=0;i<length();i++)
    {
      getRow(i,v);
      out << v << "\n";
      pb(i);
    }
}

    // This will compute for this vmat m a result vector (whose length must be tha same as m's)
    // s.t. result[i] = ker( m(i).subVec(v1_startcol,v1_ncols) , v2) 
    // i.e. the kernel value betweeen each (sub)row of m and v2
void VMatrix::evaluateKernel(Ker ker, int v1_startcol, int v1_ncols, 
                             const Vec& v2, const Vec& result, int startrow, int nrows) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  if(result.length() != endrow-startrow)
    PLERROR("In VMatrix::evaluateKernel length of result vector does not match the row range");

  Vec v1(v1_ncols);
  for(int i=startrow; i<endrow; i++)
  {
    getSubRow(i,v1_startcol,v1);
    result[i] = ker(v1,v2);
  }
}

    //  returns sum_i [ ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
real VMatrix::evaluateKernelSum(Ker ker, int v1_startcol, int v1_ncols, 
                                const Vec& v2, int startrow, int nrows, int ignore_this_row) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  double result = 0.;
  Vec v1(v1_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,v1_startcol,v1);
      result += ker(v1,v2);
    }
  return (real)result;
}
    
    // targetsum := sum_i [ m(i).subVec(t_startcol,t_ncols) * ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
    // and returns sum_i [ ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
real VMatrix::evaluateKernelWeightedTargetSum(Ker ker, int v1_startcol, int v1_ncols, const Vec& v2, 
                                                 int t_startcol, int t_ncols, Vec& targetsum, int startrow, int nrows, int ignore_this_row) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  targetsum.clear();
  double result = 0.;
  Vec v1(v1_ncols);
  Vec target(t_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,v1_startcol,v1);
      getSubRow(i,t_startcol,target);
      real kerval = ker(v1,v2);
      result += kerval;
      multiplyAcc(targetsum, target,kerval);
    }
  return (real)result;
}
  
TVec< pair<real,int> > VMatrix::evaluateKernelTopN(int N, Ker ker, int v1_startcol, int v1_ncols, 
                                                   const Vec& v2, int startrow, int nrows, int ignore_this_row) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  TopNI<real> extrema(N);
  Vec v1(v1_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,v1_startcol,v1);
      real kerval = ker(v1,v2);
      extrema.update(kerval,i);
    }
  extrema.sort();
  return extrema.getTopN();
}

TVec< pair<real,int> > VMatrix::evaluateKernelBottomN(int N, Ker ker, int v1_startcol, int v1_ncols, 
                                                      const Vec& v2, int startrow, int nrows, int ignore_this_row) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  BottomNI<real> extrema(N);
  Vec v1(v1_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,v1_startcol,v1);
      real kerval = ker(v1,v2);
      extrema.update(kerval,i);
    }
  extrema.sort();
  return extrema.getBottomN();
}

// result += transpose(X).Y
// Where X = this->subMatColumns(X_startcol,X_ncols)
// and   Y =  this->subMatColumns(Y_startcol,Y_ncols);
void VMatrix::accumulateXtY(int X_startcol, int X_ncols, int Y_startcol, int Y_ncols, 
                            Mat& result, int startrow, int nrows, int ignore_this_row) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  Vec x(X_ncols);
  Vec y(Y_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,X_startcol,x);
      getSubRow(i,Y_startcol,y);
      externalProductAcc(result, x,y);
    }
}

// result += transpose(X).Y
// Where X = this->subMatColumns(X_startcol,X_ncols)
void VMatrix::accumulateXtX(int X_startcol, int X_ncols, 
                            Mat& result, int startrow, int nrows, int ignore_this_row) const
{
  Vec x(X_ncols);
  int endrow = (nrows>0) ?startrow+nrows :length_;
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,X_startcol,x);
      externalProductAcc(result, x,x);
    }
}

void VMatrix::evaluateSumOfFprop(Func f, Vec& output_result, int nsamples)
{
  //if (f->outputs.size()!=1)
  //  PLERROR("In evaluateSumOfFprop: function must have a single variable output (maybe you can concat the vars into a single one, if this is really what you want)");
 
  static int curpos = 0;
  if (nsamples == -1) nsamples = length();
  Vec input_value(width());
  Vec output_value(output_result.length());

  f->recomputeParents();
  output_result.clear();
 
  for(int i=0; i<nsamples; i++)
  {
    getRow(curpos++, input_value);
    f->fprop(input_value, output_value);
    output_result += output_value;
    if(curpos == length()) curpos = 0;
  }
}

void VMatrix::evaluateSumOfFbprop(Func f, Vec& output_result, Vec& output_gradient, int nsamples)
{
//  if(f->outputs.size()!=1)
 //   PLERROR("In evaluateSumOfFprop: function must have a single variable output (maybe you can concat the vars into a single one, if this is really what you want)");
 
  static int curpos = 0;
  if (nsamples == -1) nsamples = length();
  Vec input_value(width());
  Vec input_gradient(width());
  Vec output_value(output_result.length());

  f->recomputeParents();
  output_result.clear();
 
  for(int i=0; i<nsamples; i++)
  {
    getRow(curpos++, input_value);
    f->fbprop(input_value, output_value, input_gradient, output_gradient);
    //displayFunction(f, true);
    output_result += output_value;
    if(curpos == length()) curpos = 0;
  }
}


/** RowBufferedVMatrix **/

IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY(RowBufferedVMatrix);

RowBufferedVMatrix::RowBufferedVMatrix(int the_length, int the_width)
  :VMatrix(the_length, the_width), 
  current_row_index(-1), current_row(the_width), 
  other_row_index(-1), other_row(the_width) 
{}

RowBufferedVMatrix::RowBufferedVMatrix()
  :current_row_index(-1), other_row_index(-1)
{}

real RowBufferedVMatrix::get(int i, int j) const
{
  if(current_row_index!=i)
    {
      if(current_row.length() != width())
        current_row.resize(width());
      getRow(i,current_row);
      current_row_index = i;
    }
  return current_row[j];
}

void RowBufferedVMatrix::getSubRow(int i, int j, Vec v) const
{
  if(current_row_index!=i)
    {
      if(current_row.length() != width())
        current_row.resize(width());
      getRow(i,current_row);
      current_row_index = i;
    }
  v << current_row.subVec(j,v.length());
}

real RowBufferedVMatrix::dot(int i1, int i2, int inputsize) const
{
  int w = width();
  if(current_row.length()!=w)
    current_row.resize(w);
  if(other_row.length()!=w)
    other_row.resize(w);

  if(i1==current_row_index)
    {
      if(i2==i1)
        return pownorm(current_row.subVec(0,inputsize));
      if(i2!=other_row_index)
        {
          getRow(i2,other_row);
          other_row_index = i2;
        }
    }
  else if(i1==other_row_index)
    {
      if(i2==i1)
        return pownorm(other_row.subVec(0,inputsize));
      if(i2!=current_row_index)
        {
          getRow(i2,current_row);
          current_row_index = i2;
        }
    }
  else // i1 not cached
    {
      if(i2==current_row_index)
        {
          getRow(i1,other_row);
          other_row_index = i1;
        }
      else if(i2==other_row_index)
        {
          getRow(i1,current_row);
          current_row_index = i1;
        }
      else // neither i1 nor i2 are cached
        {
          getRow(i1,current_row);
          if(i2==i1)
            getRow(i2,other_row);
          current_row_index = i1;
          other_row_index = i2;
        }
    }
  return PLearn::dot(current_row.subVec(0,inputsize), other_row.subVec(0,inputsize));
}
 
real RowBufferedVMatrix::dot(int i, const Vec& v) const
{
  int w = width();
  if(current_row.length()!=w)
    current_row.resize(w);

  if(i!=current_row_index)
    {
      getRow(i,current_row);
      i = current_row_index;
    }
  return PLearn::dot(current_row.subVec(0,v.length()),v);
}


/** ShiftAndRescaleVMatrix **/

ShiftAndRescaleVMatrix::
ShiftAndRescaleVMatrix(VMat underlying_distr, Vec the_shift, Vec the_scale)
  : VMatrix(underlying_distr->length(), underlying_distr->width()),
  distr(underlying_distr), shift(the_shift), scale(the_scale)
{
  fieldinfos = distr->getFieldInfos();
}


ShiftAndRescaleVMatrix::
ShiftAndRescaleVMatrix(VMat underlying_distr, int n_inputs)
  :VMatrix(underlying_distr->length(), underlying_distr->width()),
  distr(underlying_distr), shift(underlying_distr->width()),
   scale(underlying_distr->width())
{
  fieldinfos = distr->getFieldInfos();
  
  computeMeanAndStddev(underlying_distr, shift, scale);
  negateElements(shift);
  for (int i=0;i<scale.length();i++) 
    if (scale[i]==0)
      {
        PLWARNING("ShiftAndRescale: data column number %d is constant",i);
        scale[i]=1;
      }
  invertElements(scale);
  shift.subVec(n_inputs,shift.length()-n_inputs).fill(0);
  scale.subVec(n_inputs,shift.length()-n_inputs).fill(1);
}

ShiftAndRescaleVMatrix::
ShiftAndRescaleVMatrix(VMat underlying_distr, int n_inputs, int n_train)
  :VMatrix(underlying_distr->length(), underlying_distr->width()),
  distr(underlying_distr), shift(underlying_distr->width()),
   scale(underlying_distr->width())
{
  fieldinfos = distr->getFieldInfos();
  
  VMat train_distr = underlying_distr.subMatRows(0,n_train);
  computeMeanAndStddev(train_distr, shift, scale);
  shift.subVec(n_inputs,shift.length()-n_inputs).fill(0);
  scale.subVec(n_inputs,shift.length()-n_inputs).fill(1);
  // cout << "shift = " << shift << endl;
  // cout << "scale = " << scale << endl;
  negateElements(shift);
  invertElements(scale);
}
                                         
real ShiftAndRescaleVMatrix::get(int i, int j) const
{
  return (distr->get(i,j) + shift[j]) * scale[j];
}

void ShiftAndRescaleVMatrix::getSubRow(int i, int j, Vec v) const
{
  distr->getSubRow(i,j,v);
  for(int jj=0; jj<v.length(); jj++)
    v[jj] = (v[jj] + shift[j+jj]) * scale[j+jj];
}

/** ExtendedVMatrix **/

ExtendedVMatrix::ExtendedVMatrix(VMat the_distr, 
                                 int the_top_extent, int the_bottom_extent, 
                                 int the_left_extent, int the_right_extent, 
                                 real the_fill_value) :
  RowBufferedVMatrix(the_distr->length()+the_top_extent+the_bottom_extent,
          the_distr->width()+the_left_extent+the_right_extent),
  distr(the_distr), 
  top_extent(the_top_extent), bottom_extent(the_bottom_extent), 
  left_extent(the_left_extent), right_extent(the_right_extent),
  fill_value(the_fill_value)
{}


void ExtendedVMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In ExtendedVMatrix::getRow OUT OF BOUNDS");
  if(v.length() != width())
    PLERROR("In ExtendedVMatrix::getRow v.length() must be equal to the VMat's width");
#endif

  if(i<top_extent || i>=length()-bottom_extent)
    v.fill(fill_value);
  else
    {
      Vec subv = v.subVec(left_extent,distr->width());
      distr->getRow(i-top_extent,subv);
      if(left_extent>0)
        v.subVec(0,left_extent).fill(fill_value);
      if(right_extent>0)
        v.subVec(width()-right_extent,right_extent).fill(fill_value);
    }
}


/** VecExtendedVMatrix **/
VecExtendedVMatrix::VecExtendedVMatrix(VMat underlying, Vec extend_data)
  : inherited(underlying.length(), underlying.width() +
              extend_data.length()),
  underlying_(underlying), extend_data_(extend_data)
{
  fieldinfos = underlying_->getFieldInfos();
}

void VecExtendedVMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In VecExtendedVMatrix::getRow OUT OF BOUNDS");
  if(v.length() != width())
    PLERROR("In VecExtendedVMatrix::getRow v.length() must be equal to the VMat's width");
#endif

  Vec subv = v.subVec(0,underlying_->width());
  underlying_->getRow(i,subv);
  copy(extend_data_.begin(), extend_data_.end(),
       v.begin() + underlying_->width());
}


/** UniformizeVMatrix **/

UniformizeVMatrix::UniformizeVMatrix(VMat the_distr, Mat the_bins,
  Vec the_index, real the_a, real the_b)
  : RowBufferedVMatrix(the_distr->length(), the_distr->width()),
  distr(the_distr), bins(the_bins), index(the_index), a(the_a), b(the_b)
{
  fieldinfos = distr->getFieldInfos();
  
  if (a >= b)
    PLERROR("In UniformizeVMatrix: a (%f) must be strictly smaller than b (%f)", a, b);
  if (index.length() != bins.length())
    PLERROR("In UniformizeVMatrix: the number of elements in index (%d) must equal the number of rows in bins (%d)", index.length(), bins.length());
  if (min(index)<0 || max(index)>distr->length()-1)
    PLERROR("In UniformizeVMatrix: all values of index must be in range [0,%d]",
      distr->length()-1);
}

void UniformizeVMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In UniformizeVMatrix::getRow OUT OF BOUNDS");
  if(v.length() != width())
    PLERROR("In UniformizeVMatrix::getRow v.length() must be equal to the VMat's width");
#endif

  distr->getRow(i, v);
  for(int j=0; j<v.length(); j++) {
    if (vec_find(index, (real)j) != -1) {
      Vec x_bin = bins(j);
      real xx = estimatedCumProb(v[j], x_bin);
      v[j] = xx*(b-a) - a;
    }
  }
}


/** MemoryVMatrix **/

IMPLEMENT_NAME_AND_DEEPCOPY(MemoryVMatrix);

void MemoryVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  deepCopyField(data, copies);
}

MemoryVMatrix::MemoryVMatrix(const Mat& the_data)
  :VMatrix(the_data.length(), the_data.width()), data(the_data)
{}

real MemoryVMatrix::get(int i, int j) const
{ return data(i,j); }

void MemoryVMatrix::put(int i, int j, real value)
{ data(i,j) = value; }

void MemoryVMatrix::getColumn(int i, Vec v) const
{ v << data.column(i); }

void MemoryVMatrix::getSubRow(int i, int j, Vec v) const
{ v << data(i).subVec(j,v.length()); }

void MemoryVMatrix::getRow(int i, Vec v) const
{ v << data(i); }

void MemoryVMatrix::getMat(int i, int j, Mat m) const
{ m << data.subMat(i,j,m.length(),m.width()); }

void MemoryVMatrix::putSubRow(int i, int j, Vec v)
{ data.subMat(i,j,1,v.length()) << v; }

void MemoryVMatrix::fill(real value)
{ data.fill(value); }

void MemoryVMatrix::putRow(int i, Vec v)
{ data(i) << v; }

void MemoryVMatrix::putMat(int i, int j, Mat m)
{ data.subMat(i,j,m.length(),m.width()) << m; }

void MemoryVMatrix::appendRow(Vec v)
{ 
  data.appendRow(v); 
  length_++;
}

void MemoryVMatrix::write(ostream& out) const
{
  writeHeader(out, "MemoryVMatrix");
  VMatrix::write(out);  // save higher-level stuff
  writeField(out, "data", data);
  writeFooter(out, "MemoryVMatrix");
}

void MemoryVMatrix::oldread(istream& in)
{
  readHeader(in, "MemoryVMatrix");
  VMatrix::oldread(in);  // read higher-level stuff
  readField(in, "data", data);
  readFooter(in, "MemoryVMatrix");
}

Mat MemoryVMatrix::toMat() const
{ return data; }

VMat MemoryVMatrix::subMat(int i, int j, int l, int w)
{ return new MemoryVMatrix(data.subMat(i,j,l,w)); }

real MemoryVMatrix::dot(int i1, int i2, int inputsize) const
{
#ifdef BOUNDCHECK
  if(inputsize>width())
    PLERROR("In MemoryVMatrix::dot inputsize>width()");
#endif
  real* v1 = data.rowdata(i1);
  real* v2 = data.rowdata(i2);
  real res = 0.;
  for(int k=0; k<inputsize; k++)
    res += v1[k]*v2[k];
  return res;
}
real MemoryVMatrix::dot(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
  if(v.length()>width())
    PLERROR("In MemoryVMatrix::dot length of vector v is greater than VMat's width");
#endif
  real* v1 = data.rowdata(i);
  real* v2 = v.data();
  real res = 0.;
  for(int k=0; k<v.length(); k++)
    res += v1[k]*v2[k];
  return res;
}

/** CompactVMatrix **/

static unsigned char n_bits_in_byte[256];
void set_n_bits_in_byte()
{
  if (n_bits_in_byte[255]!=8)
  {
    for (int i=0;i<256;i++)
    {
      int n=0;
      unsigned char byte=i;
      for (int j=0;j<8;j++)
      {
        n += byte & 1;
        byte >>= 1;
      }
      n_bits_in_byte[i]=n;
    }
  }
}

IMPLEMENT_NAME_AND_DEEPCOPY(CompactVMatrix);

CompactVMatrix::CompactVMatrix(int the_length, int nvariables, int n_binary, 
                               int n_nonbinary_discrete,
                               int n_fixed_point, TVec<int>& n_symbolvalues, 
                               Vec& fixed_point_min, Vec& fixed_point_max, 
                               bool onehot_encoding)
  : RowBufferedVMatrix(the_length,nvariables), n_bits(n_binary), 
    n_symbols(n_nonbinary_discrete), n_fixedpoint(n_fixed_point),
    n_variables(nvariables), one_hot_encoding(onehot_encoding), 
    n_symbol_values(n_symbolvalues),
    fixedpoint_min(fixed_point_min), fixedpoint_max(fixed_point_max),
    delta(n_fixed_point), variables_permutation(n_variables)
{
  normal_width=n_bits+n_fixed_point;
  for (int i=0;i<n_symbols;i++)
      normal_width += n_symbol_values[i];
  setOneHotMode(one_hot_encoding);
  for (int i=0;i<n_variables;i++) variables_permutation[i]=i;
  for (int i=0;i<n_symbols;i++)
    delta[i]=(fixedpoint_max[i]-fixedpoint_min[i])/USHRT_MAX;
  symbols_offset = (int)ceil(n_bits/8.0);
  fixedpoint_offset = symbols_offset + n_symbols;
  row_n_bytes =  fixedpoint_offset + sizeof(unsigned short)*n_fixed_point;
  data.resize(length_ * row_n_bytes);
  set_n_bits_in_byte();
}

CompactVMatrix::CompactVMatrix(VMat m, int keep_last_variables_last, bool onehot_encoding)
  : RowBufferedVMatrix(m->length(),m->width()), one_hot_encoding(onehot_encoding),
    n_symbol_values(m->width()), variables_permutation(m->width())
{
  if (!m->hasStats()) 
  {
    cout << "CompactVMatrix(VMat, int): VMat did not have stats. Computing them." << endl;
    m->computeStats();
  }
  // determine which variables are binary discrete, multi-class discrete, or continuous
  n_bits = n_symbols = n_fixedpoint = 0;
  TVec<int> bits_position(m->width());
  TVec<int> symbols_position(m->width());
  TVec<int> fp_position(m->width());
  fixedpoint_min.resize(m->width());
  fixedpoint_max.resize(m->width());
  delta.resize(m->width());
  for (int i=0;i<m->width();i++)
  {
    VMFieldStat& stat = m->fieldstats[i];
    int n_values = stat.counts.size(); // 0 means "continuous"
    bool counts_look_continuous = !isMapKeysAreInt(stat.counts);
    if (n_values == 0 || counts_look_continuous || i>=m->width()-keep_last_variables_last)
    {
      fixedpoint_min[n_fixedpoint]=stat.min();
      fixedpoint_max[n_fixedpoint]=stat.max();
      delta[n_fixedpoint]=(stat.max()-stat.min())/USHRT_MAX;
      fp_position[n_fixedpoint++]=i;
    }
    else 
    {
      if (stat.min()!=0 || (stat.max()-stat.min()+1)!=stat.counts.size())
        PLERROR("CompactVMatrix:: variable %d looks discrete but has zero-frequency intermediate values or min!=0",i);
      if (n_values==2)
        bits_position[n_bits++]=i;
      else if (n_values<=256)
      {
        symbols_position[n_symbols]=i;
        n_symbol_values[n_symbols++] = n_values;
      }
      else
      {
        fixedpoint_min[n_fixedpoint]=stat.min();
        fixedpoint_max[n_fixedpoint]=stat.max();
        delta[n_fixedpoint]=(stat.max()-stat.min())/USHRT_MAX;
        fp_position[n_fixedpoint++]=i;
      }
    }
  }
  fixedpoint_min.resize(n_fixedpoint);
  fixedpoint_max.resize(n_fixedpoint);
  delta.resize(n_fixedpoint);
  n_symbol_values.resize(n_symbols);
  n_variables = n_bits + n_symbols + n_fixedpoint;
  int j=0;
  for (int i=0;i<n_bits;i++,j++)
    variables_permutation[j]=bits_position[i];
  for (int i=0;i<n_symbols;i++,j++)
    variables_permutation[j]=symbols_position[i];
  for (int i=0;i<n_fixedpoint;i++,j++)
    variables_permutation[j]=fp_position[i];

  normal_width=n_bits+n_fixedpoint;
  for (int i=0;i<n_symbols;i++)
    normal_width += n_symbol_values[i];
  setOneHotMode(one_hot_encoding);
  symbols_offset = (int)ceil(n_bits/8.0);
  fixedpoint_offset = symbols_offset + n_symbols;
  row_n_bytes =  fixedpoint_offset + sizeof(unsigned short)*n_fixedpoint;
  data.resize(length_ * row_n_bytes);

  // copy the field infos and stats? not really useful with one-hot encoding
  // because of non-binary symbols being spread across many columns.
  if (!one_hot_encoding)
  {
    fieldinfos.resize(width_);
    fieldstats.resize(width_);
    for (int i=0;i<width_;i++) 
      {
        fieldinfos[i]=m->getFieldInfos()[variables_permutation[i]];
        fieldstats[i]=m->fieldstats[variables_permutation[i]];
      }
  }
  else
  {
    fieldinfos.resize(0);
    fieldstats.resize(0);
  }

  // copy the data
  Vec mrow(m->width());
  for (int t=0;t<length_;t++)
  {
    m->getRow(t,mrow);
    encodeAndPutRow(t,mrow);
  }
  set_n_bits_in_byte();
}


CompactVMatrix::CompactVMatrix(const string& filename, int nlast) : RowBufferedVMatrix(0,0) 
{ 
  load(filename); 
  n_last=nlast; 
  set_n_bits_in_byte();
}

CompactVMatrix::CompactVMatrix(CompactVMatrix* cvm, VMat m, bool rescale, bool check) : 
  RowBufferedVMatrix(m->length(),m->width())
{
  if (cvm->width() != m->width())
    PLERROR("CompactVMatrix::CompactVMatrix(CompactVMatrix* cvm, VMat m,...), args have widths %d!=%d",
          cvm->width(),m->width());
  // copy all the ordinary fields
  row_n_bytes = cvm->row_n_bytes;
  data.resize(length_*row_n_bytes);
  n_bits = cvm->n_bits;
  n_symbols = cvm->n_symbols;
  n_fixedpoint = cvm->n_fixedpoint;
  n_variables = cvm->n_variables;
  n_symbol_values = cvm->n_symbol_values;
  fixedpoint_min = cvm->fixedpoint_min.copy();
  fixedpoint_max = cvm->fixedpoint_max.copy();
  delta = cvm->delta.copy();
  variables_permutation = cvm->variables_permutation;
  n_last = cvm->n_last;
  normal_width = cvm->normal_width;
  symbols_offset = cvm->symbols_offset;
  fixedpoint_offset = cvm->fixedpoint_offset;

  setOneHotMode(cvm->one_hot_encoding);
  Vec row(width_);
  int offs=width_-n_fixedpoint;
  if (rescale)
  {
    for (int i=0;i<length_;i++)
    {
      m->getRow(i,row);
      for (int j=0;j<n_fixedpoint;j++)
      {
        real element=row[offs+j];
        if (element<fixedpoint_min[j])
          fixedpoint_min[j]=element;
        if (element>fixedpoint_max[j])
          fixedpoint_max[j]=element;
      }
    }
    for (int j=0;j<n_fixedpoint;j++)
      delta[j]=(fixedpoint_max[j]-fixedpoint_min[j])/USHRT_MAX;
  }
  for (int i=0;i<length_;i++)
  {
    m->getRow(i,row);
    if (!rescale && check) // check that range is OK
    {
      for (int j=0;j<n_fixedpoint;j++)
      {
        real element=row[offs+j];
        if (element<fixedpoint_min[j] ||
            element>fixedpoint_max[j])
          PLERROR("CompactVMatrix::CompactVMatrix(CompactVMatrix* cvm, VMat m,...) out-of-range element(%d,%d)=%g not in [%g,%g]",
                i,j,element,fixedpoint_min[j],fixedpoint_max[j]);
      }
    }
    putRow(i,row);
  }
}

void CompactVMatrix::setOneHotMode(bool on)
{
  one_hot_encoding=on;
  if (one_hot_encoding)
    width_ = normal_width;
  else
    width_ = n_variables;
}

void CompactVMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if (i<0 || i>=length_)
    PLERROR("CompactVMatrix::getRow, row %d out of bounds [0,%d]",i,length_-1);
  if (v.length()!=width_)
    PLERROR("CompactVMatrix::getRow, length of v (%d) should be equal to width of VMat (%d)",v.length(),width());
#endif
  unsigned char* encoded_row = &data.data[i*row_n_bytes];
  real* vp=v.data();
  int c=0;
  for (int b=0;b<symbols_offset;b++)
  {
    unsigned char byte=encoded_row[b];
    for (int j=0;j<8 && c<n_bits;j++,c++)
    {
      int bit = byte & 1;
      byte >>= 1; // shift right once
      vp[c]=bit;
    }
  }
  for (int b=0;b<n_symbols;b++)
  {
    int byte = encoded_row[symbols_offset+b];
    if (one_hot_encoding)
    {
      int n=n_symbol_values[b];
      for (int j=0;j<n;j++) vp[c+j]=0;
      vp[c+byte]=1;
      c+=n;
    }
    else vp[c++]=byte;
  }
  unsigned char* fixed_point_numbers = &encoded_row[fixedpoint_offset];
  for (int j=0;j<n_fixedpoint;j++,c++)
  {
    unsigned char *uc = &fixed_point_numbers[2*j];
    short_and_twobytes u;
    u.twobytes[0]=uc[0];
    u.twobytes[1]=uc[1];
    real decoded = u.us*delta[j]+fixedpoint_min[j];
    // correct rounding errors for integers, due to fixed-point low precision
    real rounded_decoded = rint(decoded);
    if (fabs(rounded_decoded-decoded)<1e-4) 
      decoded = rounded_decoded;
    vp[c]=decoded;
  }
}

//#define SANITYCHECK_CompactVMatrix
#define SANITYCHECK_CompactVMatrix_PRECISION 1e-5

real CompactVMatrix::dot(int i, int j, int inputsize) const
{
  if(inputsize!=width()-n_last)
    PLERROR("In CompactVMatrix::dot, in current implementation inputsize must be equal to width()-n_last");

  unsigned char* encoded_row_i = &data.data[i*row_n_bytes];
  unsigned char* encoded_row_j = &data.data[j*row_n_bytes];
  real dot_product=0.;
  int c=0;
  for (int b=0;b<symbols_offset;b++)
  {
    unsigned char byte_i=encoded_row_i[b];
    unsigned char byte_j=encoded_row_j[b];
    unsigned char byte_and = byte_i & byte_j;
#ifdef SANITYCHECK_CompactVMatrix
    real check=dot_product;
#endif
    // Here we want to count the number of ON bits in the byte_and
    // instead of looping through the bits, we look-up in a 
    // pre-computed table (n_bits_in_byte), which has been set-up by set_n_bits_in_byte().
    dot_product += n_bits_in_byte[byte_and];
#ifdef SANITYCHECK_CompactVMatrix
    for (int j=0;j<8 && c<n_bits;j++,c++)
    {
      check += byte_and & 1;
      byte_and >>= 1; // shift right once
    }
    if (check!=dot_product)
      PLERROR("logic error in n_bits_in_byte");
#else
    c+=8;
    if (c>n_bits) c=n_bits;
#endif
  }
  if (c>width_-n_last)
    PLERROR("CompactVMatrix: n_last should be among discrete non-binary or continuous variables");
  for (int b=0;b<n_symbols && c<width_-n_last;b++)
  {
    int byte_i = encoded_row_i[symbols_offset+b];
    int byte_j = encoded_row_j[symbols_offset+b];
    if (byte_i==byte_j) dot_product++;
    if (one_hot_encoding)
      c+=n_symbol_values[b];
    else
      c++;
  }
  unsigned char* fixed_point_numbers_i = &encoded_row_i[fixedpoint_offset];
  unsigned char* fixed_point_numbers_j = &encoded_row_j[fixedpoint_offset];
  for (int k=0;k<n_fixedpoint-n_last && c<width_-n_last;k++,c++)
  {
    unsigned char *uc = &fixed_point_numbers_i[2*k];
    short_and_twobytes u;
    u.twobytes[0]=uc[0];
    u.twobytes[1]=uc[1];
    real decoded_i = u.us*delta[k]+fixedpoint_min[k];
    uc = &fixed_point_numbers_j[2*k];
    u.twobytes[0]=uc[0];
    u.twobytes[1]=uc[1];
    real decoded_j = u.us*delta[k]+fixedpoint_min[k];
#ifdef SANITYCHECK_CompactVMatrix
    real rounded_decoded_i = rint(decoded_i);
    if (fabs(rounded_decoded_i-decoded_i)<1e-4)
      decoded_i = rounded_decoded_i;
    real rounded_decoded_j = rint(decoded_j);
    if (fabs(rounded_decoded_j-decoded_j)<1e-4)
      decoded_j = rounded_decoded_j;
#endif
    dot_product += decoded_i * decoded_j;
  }

  return dot_product;
}

// I used the code for getRow as a basis to implement this call (Pascal)
real CompactVMatrix::dot(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
  if (i<0 || i>=length_)
    PLERROR("CompactVMatrix::getRow, row %d out of bounds [0,%d]",i,length_-1);
#endif

  if(v.length()!=width()-n_last)
    PLERROR("In CompactVMatrix::dot, in current implementation v.length() must be equal to width()-n_last");

  real dot_product = 0.;

  unsigned char* encoded_row = &data.data[i*row_n_bytes];
  real* vp=v.data();
  int c=0;
  for (int b=0;b<symbols_offset;b++)
  {
    unsigned char byte=encoded_row[b];
    for (int j=0;j<8 && c<n_bits;j++,c++)
    {
      int bit = byte & 1;
      byte >>= 1; // shift right once
      if(bit)
        dot_product += vp[c];
    }
  }
  for (int b=0;b<n_symbols;b++)
  {
    int byte = encoded_row[symbols_offset+b];
    if (one_hot_encoding)
      {
        int n=n_symbol_values[b];
        dot_product += vp[c+byte];
        c += n;
      }
    else 
      dot_product += vp[c++]*byte;
  }
  // WARNING: COULD THIS CAUSE PROBLEMS IF fixedpoint_offset IS NOT A MULTIPLE OF 4
  // ON SOME MACHINES?
  unsigned char* fixed_point_numbers = &encoded_row[fixedpoint_offset];
  for (int j=0;j<n_fixedpoint-n_last && c<v.length();j++,c++)
  {
    unsigned char *uc = &fixed_point_numbers[2*j];
    short_and_twobytes u;
    u.twobytes[0]=uc[0];
    u.twobytes[1]=uc[1];
    real decoded = u.us*delta[j]+fixedpoint_min[j];
    // correct rounding errors for integers, due to fixed-point low precision
    real rounded_decoded = rint(decoded);
    if (fabs(rounded_decoded-decoded)<1e-4) 
      decoded = rounded_decoded;
    dot_product += vp[c] * decoded;
  }

  // Very Slow SANITY CHECK
#ifdef SANITYCHECK_CompactVMatrix
  Vec v_i(v.length());
  getRow(i,v_i);
  real dot_product2 = PLearn::dot(v_i.subVec(0,v.length()),v);
  real diff = fabs(dot_product-dot_product2)/fabs(dot_product2);
  if(diff>SANITYCHECK_CompactVMatrix_PRECISION)
    PLERROR("IN CompactVMatrix::dot(int i=%d, v) SANITY CHECK FAILED: difference=%g",i,diff);
#endif

  return dot_product;
}


real CompactVMatrix::dotProduct(int i, int j) const
{ return dot(i,j,width()-n_last); }

real CompactVMatrix::squareDifference(int i, int j)
{
  if (row_norms.length()==0)
    row_norms = Vec(length_,-1.0);
  real normi = row_norms[i];
  if (normi<0) normi=row_norms[i]=dotProduct(i,i);
  real normj = row_norms[j];
  if (normj<0) normj=row_norms[j]=dotProduct(j,j);
  return normi + normj - 2 * dotProduct(i,j);
}

void CompactVMatrix::encodeAndPutRow(int i, Vec v)
{
  unsigned char* encoded_row = &data.data[i*row_n_bytes];
  real* vp=v.data();
  int* perm=variables_permutation.data();
  int c=0;
  // 1 vector element ==> 1 bit
  for (int b=0;b<symbols_offset;b++)
  {
    unsigned char byte=0;
    for (int j=0;j<8 && c<n_bits;j++,c++)
      byte |= int(vp[perm[c]]) << j; // shift to right bit position
    encoded_row[b]=byte;
  }
  //    1 vector element (integer between 0 and n-1) ==> 1 byte
  for (int b=0;b<n_symbols;b++,c++)
  {
    real val = vp[perm[c]];
    int s = int(val);
    if (s!=val)
      PLERROR("CompactVMatrix::encodeAndPutRow(%d,v): v[%d]=%g not an integer",
            i,int(perm[c]),val);
    encoded_row[symbols_offset+b] = s; // ASSUMES THAT v IS NOT ONE-HOT ENCODED
    if (s<0 || s>=n_symbol_values[b])
      PLERROR("CompactVMatrix::encodeAndPutRow(%d,v): v[%d]=%d not in expected range (0,%d)",
            i,int(perm[c]),s,n_symbol_values[b]-1);
  }
  // WARNING: COULD THIS CAUSE PROBLEMS IF fixedpoint_offset IS NOT A MULTIPLE OF 4
  // ON SOME MACHINES?
  unsigned short* fixed_point_numbers = (unsigned short*)&encoded_row[fixedpoint_offset];
  for (int j=0;j<n_fixedpoint;j++,c++)
    fixed_point_numbers[j]=(unsigned short)((vp[perm[c]]-fixedpoint_min[j])/delta[j]);
}

void CompactVMatrix::putRow(int i, Vec v)
{
  putSubRow(i,0,v);
}

void CompactVMatrix::putSubRow(int i, int j, Vec v)
{
  unsigned char* encoded_row = &data.data[i*row_n_bytes];
  real* vp=v.data();
  int c=0;
  // 1 vector element ==> 1 bit
  for (int b=0;b<symbols_offset;b++)
  {
    unsigned char byte=0;
    for (int k=0;k<8 && c<n_bits;k++,c++)
      if (c>=j)
        byte |= int(vp[c-j]) << k; // shift to right bit position
    encoded_row[b]=byte;
  }
  // if (one_hot_encoding)
  //   n vector elements in one-hot-code ==> 1 byte
  // else
  //   1 vector element (integer between 0 and n-1) ==> 1 byte
  int n=0;
  if (one_hot_encoding)
    for (int b=0;b<n_symbols;b++,c+=n)
    {
      n=n_symbol_values[b];
      if (c>=j)
      {
        int pos=-1;
        for (int k=0;k<n;k++)
        {
          real vk=vp[c+k-j];
          if (vk!=0 && vk!=1)
            PLERROR("CompactVMatrix::putRow(%d,v): v[%d]=%g!=0 or 1 (not one-hot-code)",
                  i,c,vk);
          if (vk==1) 
          {
            if (pos<0) pos=k;
            else PLERROR("CompactVMatrix::putRow(%d,v): %d-th symbol not one-hot-encoded",
                       i,b);
          }
        }
        if (pos<0)
          PLERROR("CompactVMatrix::putRow(%d,v): %d-th symbol not one-hot-encoded",
                i,b);
        encoded_row[symbols_offset+b] = pos; 
      }
    }
  else
    for (int b=0;b<n_symbols;b++,c++)
      if (c>=j)
      {
        real val = vp[c-j];
        int s = int(val);
        if (s!=val)
          PLERROR("CompactVMatrix::encodeAndPutRow(%d,v): v[%d]=%g not an integer",
                i,c,val);
        encoded_row[symbols_offset+b] = s; // ASSUMES THAT v IS NOT ONE-HOT ENCODED
        if (s<0 || s>=n_symbol_values[b])
          PLERROR("CompactVMatrix::encodeAndPutRow(%d,v): v[%d]=%d not in expected range (0,%d)",
                i,c,s,n_symbol_values[b]-1);
      }

  // 1 vector element (real betweeen fixedpoint_min and fixedpoint_max) ==> 2 bytes
  //
  // WARNING: COULD THIS CAUSE PROBLEMS IF fixedpoint_offset IS NOT A MULTIPLE OF 4
  // ON SOME MACHINES?
  unsigned short* fixed_point_numbers = (unsigned short*)&encoded_row[fixedpoint_offset];
  for (int k=0;k<n_fixedpoint;k++,c++)
    if (c>=j)
      fixed_point_numbers[k]=(unsigned short)((vp[c-j]-fixedpoint_min[k])/delta[k]);
}

void CompactVMatrix::perturb(int i, Vec v, real noise_level, int n_last)
{
#ifdef BOUNDCHECK
  if (i<0 || i>=length_)
    PLERROR("CompactVMatrix::perturb, row %d out of bounds [0,%d]",i,length_-1);
  if (v.length()!=width_)
    PLERROR("CompactVMatrix::perturb, length of v (%d) should be equal to width of VMat (%d)",v.length(),width());
#endif
  if (fieldstats.size()!=n_variables)
    PLERROR("CompactVMatrix::perturb: stats not computed or wrong size");
  if (noise_level<0 || noise_level>1)
    PLERROR("CompactVMatrix::perturb: noise_level=%g, should be in [0,1]",noise_level);

  unsigned char* encoded_row = &data.data[i*row_n_bytes];
  real* vp=v.data();
  int c=0;
  int var=0;
  Vec probs(width_);
  for (int b=0;b<symbols_offset;b++)
  {
    unsigned char byte=encoded_row[b];
    for (int j=0;j<8 && c<n_bits;j++,c++,var++)
    {
      int bit = byte & 1;
      byte >>= 1; // shift right once
      vp[c]=binomial_sample((1-noise_level)*bit+noise_level*fieldstats[var].prob(1));
    }
  }
  for (int b=0;b<n_symbols;b++,var++)
  {
    int byte = encoded_row[symbols_offset+b];
    int nv=n_symbol_values[b];
    probs.resize(nv);
    VMFieldStat& stat=fieldstats[var];
    for (int val=0;val<nv;val++)
      if (val==byte)
        probs[val]=(1-noise_level)+noise_level*stat.prob(val);
      else
        probs[val]=noise_level*stat.prob(val);
    byte = multinomial_sample(probs);
    if (one_hot_encoding)
    {
      int n=n_symbol_values[b];
      for (int j=0;j<n;j++) vp[c+j]=0;
      vp[c+byte]=1;
      c+=n;
    }
    else vp[c++]=byte;
  }
  unsigned char* fixed_point_numbers = &encoded_row[fixedpoint_offset];
  for (int j=0;j<n_fixedpoint;j++,c++,var++)
  {
    unsigned char *uc = &fixed_point_numbers[2*j];
    short_and_twobytes u;
    u.twobytes[0]=uc[0];
    u.twobytes[1]=uc[1];
    real decoded = u.us*delta[j]+fixedpoint_min[j];
    // correct rounding errors for integers, due to fixed-point low precision
    real rounded_decoded = rint(decoded);
    if (fabs(rounded_decoded-decoded)<1e-4) 
      decoded = rounded_decoded;
    if (var<n_variables-n_last)
    {
      int ntry=0;
      do
      {
        vp[c]=decoded+noise_level*fieldstats[var].stddev()*normal_sample();
        ntry++;
        if (ntry>=100)
          PLERROR("CompactVMatrix::perturb:Something wrong in resampling, tried 100 times");
      }
      while (vp[c]<fixedpoint_min[j] || vp[c]>fixedpoint_max[j]);
    }
    else
      vp[c]=decoded;
  }
}

void CompactVMatrix::write(ostream& out) const
{
  writeHeader(out,"CompactVMatrix");
  writeField(out,"length",length_);
  writeField(out,"width",normal_width);
  writeField(out,"fieldinfos",fieldinfos);
  writeField(out,"fieldstats",fieldstats);
  writeField(out,"row_n_bytes",row_n_bytes);
  writeField(out,"n_bits",n_bits);
  writeField(out,"n_symbols",n_symbols);
  writeField(out,"n_fixedpoint",n_fixedpoint);
  writeField(out,"one_hot_encoding",one_hot_encoding);
  writeField(out,"n_symbol_values",n_symbol_values);
  writeField(out,"fixedpoint_min",fixedpoint_min);
  writeField(out,"fixedpoint_max",fixedpoint_max);
  writeField(out,"delta",delta);
  writeField(out,"variables_permutation",variables_permutation);
  writeField(out,"symbols_offset",symbols_offset);
  writeField(out,"fixedpoint_offset",fixedpoint_offset);
  out.write((char*)data.data,data.length()*sizeof(unsigned char));
  writeFooter(out,"CompactVMatrix");
}

void CompactVMatrix::oldread(istream& in)
{
  readHeader(in,"CompactVMatrix");
  readField(in,"length",length_);
  readField(in,"width",normal_width);
  readField(in,"fieldinfos",fieldinfos);
  fieldinfos.resize(0); // to fix current bug in setting fieldinfos
  readField(in,"fieldstats",fieldstats);
  readField(in,"row_n_bytes",row_n_bytes);
  readField(in,"n_bits",n_bits);
  readField(in,"n_symbols",n_symbols);
  readField(in,"n_fixedpoint",n_fixedpoint);
  n_variables = n_bits + n_symbols + n_fixedpoint;
  readField(in,"one_hot_encoding",one_hot_encoding);
  setOneHotMode(one_hot_encoding);
  readField(in,"n_symbol_values",n_symbol_values);
  readField(in,"fixedpoint_min",fixedpoint_min);
  readField(in,"fixedpoint_max",fixedpoint_max);
  readField(in,"delta",delta);
  readField(in,"variables_permutation",variables_permutation);
  readField(in,"symbols_offset",symbols_offset);
  readField(in,"fixedpoint_offset",fixedpoint_offset);
  data.resize(row_n_bytes*length_);
  in.read((char*)data.data,data.length()*sizeof(unsigned char));
  readFooter(in,"CompactVMatrix");
}

void CompactVMatrix::append(CompactVMatrix* vm)
{
  if (width_!=vm->width())
    PLERROR("CompactVMatrix::append, incompatible width %d vs %d",
          width_,vm->width());
  if (row_n_bytes!=vm->row_n_bytes)
    PLERROR("CompactVMatrix::append, incompatible row_n_bytes %d vs %d",
          row_n_bytes,vm->row_n_bytes);
  if (n_bits!=vm->n_bits)
    PLERROR("CompactVMatrix::append, incompatible n_bits %d vs %d",
          n_bits,vm->n_bits);
  if (n_symbols!=vm->n_symbols)
    PLERROR("CompactVMatrix::append, incompatible n_symbols %d vs %d",
          n_symbols,vm->n_symbols);
  if (n_fixedpoint!=vm->n_fixedpoint)
    PLERROR("CompactVMatrix::append, incompatible n_fixedpoint %d vs %d",
          n_fixedpoint,vm->n_fixedpoint);
  if (n_symbol_values!=vm->n_symbol_values)
    {
        //n_symbol_values.write(cerr); cerr << endl;
        //vm->n_symbol_values.write(cerr); cerr << endl;
        PLearn::write(cerr, n_symbol_values);
        cerr << endl;
        PLearn::write(cerr, vm->n_symbol_values);
        cerr << endl;
        PLERROR("CompactVMatrix::append, incompatible n_symbol_values");
    }
  bool rescale = false;
  for (int j=0;j<n_fixedpoint && !rescale;j++)
    if (fixedpoint_min[j]>vm->fixedpoint_min[j] ||
        fixedpoint_max[j]<vm->fixedpoint_max[j]) rescale=true;
  if (rescale)
  {
    cout << "The appended VMat has intervals that are wider than the current one." << endl;
    cout << "Start rescaling numeric variables fixed point representation." << endl;
    Vec new_min = fixedpoint_min.copy();
    Vec new_max = fixedpoint_max.copy();
    Vec new_delta = delta.copy();
    TVec<bool> change(n_fixedpoint);
    for (int j=0;j<n_fixedpoint;j++)
    {
      change[j]=false;
      if (fixedpoint_min[j]>vm->fixedpoint_min[j])
      {
        change[j]=true;
        new_min[j]=vm->fixedpoint_min[j];
      }
      if (fixedpoint_max[j]<vm->fixedpoint_max[j]) 
      {
        change[j]=true;
        new_max[j]=vm->fixedpoint_max[j];
      }
      if (change[j])
        new_delta[j]=(new_max[j]-new_min[j])/USHRT_MAX;
    }
    for (int r=0;r<length_;r++)
    {
      unsigned char* encoded_row = &data.data[r*row_n_bytes];
      unsigned char* fixed_point_numbers = &encoded_row[fixedpoint_offset];
      for (int j=0;j<n_fixedpoint;j++)
        if (change[j])
        {
          // DECODE using previous min/max
          unsigned char *uc = &fixed_point_numbers[2*j];
          short_and_twobytes u;
          u.twobytes[0]=uc[0];
          u.twobytes[1]=uc[1];
          real decoded = u.us*delta[j]+fixedpoint_min[j];
          // correct rounding errors for integers, due to fixed-point low precision
          real rounded_decoded = rint(decoded);
          if (fabs(rounded_decoded-decoded)<1e-4) 
            decoded = rounded_decoded;
          // ENCODE using new min/max
          fixed_point_numbers[j]=(unsigned short)((decoded-new_min[j])/new_delta[j]);
        }
    }
    cout << "DONE rescaling numeric variables fixed point representation." << endl;
    fixedpoint_min << new_min;
    fixedpoint_max << new_max;
    delta << new_delta;
  }
  int new_length=length_+vm->length();
  data.resize(row_n_bytes*new_length);
  // copy the new data
  Vec row(width_);
  bool old_vm_encoding = vm->one_hot_encoding;
  bool old_encoding = one_hot_encoding;
  vm->one_hot_encoding=false;
  setOneHotMode(false);
  int old_length=length_;
  length_=new_length;
  for (int r=0;r<vm->length();r++)
    {
      vm->getRow(r,row);
      putRow(old_length+r,row);
    }
  vm->one_hot_encoding=old_vm_encoding;
  setOneHotMode(old_encoding);
}

void CompactVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  deepCopyField(data, copies);
  deepCopyField(n_symbol_values, copies);
  deepCopyField(fixedpoint_min, copies);
  deepCopyField(fixedpoint_max, copies);
  deepCopyField(variables_permutation, copies);
}

/** SparseVMatrix **/

SparseVMatrix::SparseVMatrix(VMat m)
  :RowBufferedVMatrix(m.length(),m.width()),
   nelements(0),
   positions(0),
   values(0),
   rows(0)
{
  fieldinfos = m->getFieldInfos();                // Copy the field infos
  
  if(m.width()>USHRT_MAX)
    PLERROR("In SparseVMatrix constructor: m.width()=%d can't be greater than USHRT_MAX=%d",m.width(),USHRT_MAX);
  Vec v(m.width());
  real* vptr = v.data();

  // First count nelements
  nelements = 0;
  if(m->hasStats()) // use the stats!
    {
      for(int j=0; j<m.width(); j++)
        {
          const VMFieldStat& st = m->fieldStat(j);
          nelements += st.nmissing() + st.npositive() + st.nnegative();
        }
    }
  else // let's count them ourself
    {
      for(int i=0; i<m.length(); i++)
        {
          m->getRow(i,v);
          for(int j=0; j<v.length(); j++)
            if(vptr[j]!=0.)
              nelements++;
        }
    }
  
  // Now allocate space for those elements
  if(nelements>0)
    {
      positions = new unsigned short[nelements];
      values = new float[nelements];
      int l=length();
      rows = new SparseVMatrixRow[l];
      
      int pos = 0;
      // Fill the representation
      for(int i=0; i<m.length(); i++)
        {
          m->getRow(i,v);
          SparseVMatrixRow& r = rows[i];
          r.row_startpos = pos; 
          int nelem = 0;
          for(int j=0; j<v.length(); j++)
            if(vptr[j]!=0.)
              {
                positions[pos] = j;
                values[pos] = (float)vptr[j];
                pos++;
                nelem++;
              }
          r.nelements = nelem;
        }
    }
}
    
void SparseVMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In SparseVMatrix::getRow, row number i=%d OUT OF BOUNDS (matrix is %dx%d)",i,length(),width());
  if(v.length()!=width())
    PLERROR("In SparseVMatrix::getRow, length of v (%d) is different from width of VMatris (%d)",v.length(),width());
#endif

  if(nelements==0)
    v.clear();
  else
    {      
      SparseVMatrixRow row_i = rows[i];
      float* valueptr =  values + row_i.row_startpos;
      unsigned short* positionptr = positions + row_i.row_startpos;
      int n = row_i.nelements;
  
      real* vdata = v.data();
      
      int j = 0;
      while(n--)
        {
          int nextpos = (int) *positionptr++;
          real nextval = (real) *valueptr++;
          while(j<nextpos)
            vdata[j++] = 0.;
          vdata[j++] = nextval;
        }
      while(j<v.length())
        vdata[j++] = 0.;
    }
}

real SparseVMatrix::dot(int i1, int i2, int inputsize) const
{
#ifdef BOUNDCHECK
  if(i1<0 || i1>=length() || i2<0 || i2>=length() || inputsize>width())
    PLERROR("IN SparseVMatrix::dot OUT OF BOUNDS");
#endif

  if(nelements==0)
    return 0.;
  
  SparseVMatrixRow row_1 = rows[i1];
  float* valueptr_1 =  values + row_1.row_startpos;
  unsigned short* positionptr_1 = positions + row_1.row_startpos;
  int n_1 = row_1.nelements;

  SparseVMatrixRow row_2 = rows[i2];
  float* valueptr_2 =  values + row_2.row_startpos;
  unsigned short* positionptr_2 = positions + row_2.row_startpos;
  int n_2 = row_2.nelements;

  real res = 0.;

  while(n_1 && n_2)
    {
      if(*positionptr_1>=inputsize)
        break;
      if(*positionptr_1==*positionptr_2)
        {
          res += (*valueptr_1)*(*valueptr_2);
          positionptr_1++;
          valueptr_1++;
          n_1--;
          positionptr_2++;
          valueptr_2++;
          n_2--;
        }
      else if(*positionptr_1<*positionptr_2)
        {
          positionptr_1++;
          valueptr_1++;
          n_1--;
        }
      else 
        {
          positionptr_2++;
          valueptr_2++;
          n_2--;
        }
    }

  return res;
}

real SparseVMatrix::dot(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || v.length()>width())
    PLERROR("IN SparseVMatrix::dot OUT OF BOUNDS");
#endif

  if(nelements==0)
    return 0.;

  SparseVMatrixRow row_i = rows[i];
  float* valueptr =  values + row_i.row_startpos;
  unsigned short* positionptr = positions + row_i.row_startpos;
  int n = row_i.nelements;
  
  real* vdata = v.data();
  real res = 0.;

  while(n--)
    {
      int nextpos = (int) *positionptr++;
      real nextval = (real) *valueptr++;
      if(nextpos>=v.length())
        break;
      res += nextval*vdata[nextpos];
    }
  return res;
}

void SparseVMatrix::write(ostream& out) const
{
  writeHeader(out,"SparseVMatrix");
  writeField(out,"length",length_);
  writeField(out,"width",width_);
  writeField(out,"fieldinfos",fieldinfos);
  writeField(out,"fieldstats",fieldstats);
  writeField(out,"nelements",nelements);
  write_ushort(out,positions,nelements,false);
  write_float(out,values,nelements,false);
  for(int i=0; i<length(); i++)
    {
      write_int(out,rows[i].nelements);
      write_int(out,rows[i].row_startpos);
    }
  writeFooter(out,"SparseVMatrix");
}

void SparseVMatrix::oldread(istream& in)
{
  readHeader(in,"SparseVMatrix");
  readField(in,"length",length_);
  readField(in,"width",width_);
  readField(in,"fieldinfos",fieldinfos);
  fieldinfos.resize(0); // to fix current bug in setting fieldinfos
  readField(in,"fieldstats",fieldstats);
  
  if(nelements>0)
    {
      delete[] positions;
      delete[] values;
      delete[] rows;
    }
  readField(in,"nelements",nelements);
  positions = new unsigned short[nelements];
  values = new float[nelements];
  rows = new SparseVMatrixRow[length()];
  
  read_ushort(in,positions,nelements,false);
  read_float(in,values,nelements,false);
  for(int i=0; i<length(); i++)
    {
      rows[i].nelements = read_int(in);
      rows[i].row_startpos = read_int(in);
    }
  readFooter(in,"SparseVMatrix");
}

SparseVMatrix::~SparseVMatrix()
{
  if(nelements>0)
    {
      delete[] positions;
      delete[] values;
      delete[] rows;
    }
}


/** SubVMatrix **/

IMPLEMENT_NAME_AND_DEEPCOPY(SubVMatrix);

SubVMatrix::SubVMatrix(VMat the_parent, int the_istart, int the_jstart, int the_length, int the_width)
  :VMatrix(the_length, the_width), parent(the_parent), istart(the_istart), jstart(the_jstart)
{
  build_();
}

void SubVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "parent", &SubVMatrix::parent, OptionBase::buildoption, "Source VMatrix");
  declareOption(ol, "istart", &SubVMatrix::istart, OptionBase::buildoption, "Start i coordinate");
  declareOption(ol, "jstart", &SubVMatrix::jstart, OptionBase::buildoption, "Start j coordinate");
  inherited::declareOptions(ol);
}

void SubVMatrix::build()
{
  inherited::build();
  build_();
}

void SubVMatrix::build_()
{
  if(istart+length()>parent->length() || jstart+width()>parent->width())
    PLERROR("In SubVMatrix constructor OUT OF BOUNDS of parent VMatrix");

  // Copy the parent field names
  fieldinfos.resize(width_);
  if (parent->getFieldInfos().size() > 0)
    for(int j=0; j<width_; j++)
      fieldinfos[j] = parent->getFieldInfos()[jstart+j];
}

void SubVMatrix::reset_dimensions() 
{ 
  int delta_length = parent->length()-length_;
  int delta_width = 0; // parent->width()-width_; HACK
  parent->reset_dimensions(); 
  length_=parent->length()-delta_length; 
  width_=parent->width()-delta_width; 
}

real SubVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In SubVMatrix::get(i,j) OUT OF BOUND access");
#endif
  return parent->get(i+istart,j+jstart);
}
void SubVMatrix::getSubRow(int i, int j, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In SubVMatrix::getSubRow(i,j,v) OUT OF BOUND access");
#endif
  parent->getSubRow(i+istart,j+jstart,v);
}

void SubVMatrix::getMat(int i, int j, Mat m) const
{
#ifdef BOUNDCHECK
  if(i<0 || i+m.length()>length() || j<0 || j+m.width()>width())
    PLERROR("In SubVMatrix::getMat OUT OF BOUND access");
#endif
  parent->getMat(i+istart, j+jstart, m);
}

void SubVMatrix::put(int i, int j, real value)
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In SubVMatrix::put(i,j,value) OUT OF BOUND access");
#endif
  return parent->put(i+istart,j+jstart,value);
}
void SubVMatrix::putSubRow(int i, int j, Vec v)
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In SubVMatrix::putSubRow(i,j,v) OUT OF BOUND access");
#endif
  parent->putSubRow(i+istart,j+jstart,v);
}

void SubVMatrix::putMat(int i, int j, Mat m)
{
#ifdef BOUNDCHECK
  if(i<0 || i+m.length()>length() || j<0 || j+m.width()>width())
    PLERROR("In SubVMatrix::putMat(i,j,m) OUT OF BOUND access");
#endif
  parent->putMat(i+istart, j+jstart, m);
}

VMat SubVMatrix::subMat(int i, int j, int l, int w)
{
  return parent->subMat(istart+i,jstart+j,l,w);
}

real SubVMatrix::dot(int i1, int i2, int inputsize) const
{
  if(jstart==0)
    return parent->dot(istart+i1, istart+i2, inputsize);
  else 
    return VMatrix::dot(i1,i2,inputsize);
}

real SubVMatrix::dot(int i, const Vec& v) const
{
  if(jstart==0)
    return parent->dot(istart+i,v);
  else
    return VMatrix::dot(i,v);
}

/** FileVMatrix **/

IMPLEMENT_NAME_AND_DEEPCOPY(FileVMatrix);

FileVMatrix::FileVMatrix(const string& filename)
  :filename_(abspath(filename))
{
  build_();
}

FileVMatrix::FileVMatrix(const string& filename, int the_length, int the_width)
  :VMatrix(the_length, the_width), filename_(abspath(filename))
{
  f = fopen(filename.c_str(),"w+b");
  if (!f)
    PLERROR("In FileVMatrix constructor, could not open file %s for read/write",filename.c_str());

  setMtime(mtime(filename_));

  char header[DATAFILE_HEADERLENGTH]; 

#ifdef USEFLOAT
  file_is_float = true;
#ifdef LITTLEENDIAN
 file_is_bigendian = false; 
 sprintf(header,"MATRIX %d %d FLOAT LITTLE_ENDIAN", length_, width_);
#endif
#ifdef BIGENDIAN
 file_is_bigendian = true; 
 sprintf(header,"MATRIX %d %d FLOAT BIG_ENDIAN", length_, width_);
#endif
#endif
#ifdef USEDOUBLE
 file_is_float = false;
#ifdef LITTLEENDIAN
 file_is_bigendian = false; 
 sprintf(header,"MATRIX %d %d DOUBLE LITTLE_ENDIAN", length_, width_);
#endif
#ifdef BIGENDIAN
 file_is_bigendian = true; 
 sprintf(header,"MATRIX %d %d DOUBLE BIG_ENDIAN", length_, width_);
#endif
#endif

  // Pad the header with whites and terminate it with '\n'
  for(int pos=strlen(header); pos<DATAFILE_HEADERLENGTH; pos++)
    header[pos] = ' ';
  header[DATAFILE_HEADERLENGTH-1] = '\n';

  // write the header to the file
  fwrite(header,DATAFILE_HEADERLENGTH,1,f);

  if(length_ > 0 && width_ > 0) //ensure we can allocate enough space... if len>0, to ensure
    {             // that the header ends with a '\n'.
      if( fseek(f, DATAFILE_HEADERLENGTH+length_*width_*sizeof(real)-1, SEEK_SET) <0 )
	{
	  perror("");
	  PLERROR("In FileVMatrix constructor Could not fseek to last byte");
	}
      fputc('\0',f);
    }

  string fieldinfosfname = filename+".fieldnames";
  if(isfile(fieldinfosfname))
    loadFieldInfos(fieldinfosfname);
}

void FileVMatrix::build()
{
  inherited::build();
  build_();
}

void FileVMatrix::build_()
{
  char header[DATAFILE_HEADERLENGTH];
  char matorvec[20];
  char datatype[20];
  char endiantype[20];

  setMtime(mtime(filename_));

  if (writable)
    f = fopen(filename_.c_str(), "rw");
  else
    f = fopen(filename_.c_str(), "r");

  if (!f)
    PLERROR("In FileVMatrix constructor, could not open file %s for reading",filename_.c_str());
  fread(header,DATAFILE_HEADERLENGTH,1,f);
  if(header[DATAFILE_HEADERLENGTH-1]!='\n')
    PLERROR("In FileVMatrix constructor, wrong header for PLearn binary matrix format. Please use checkheader (in PLearn/Scripts) to check the file.(0)");
  sscanf(header,"%s%d%d%s%s",matorvec,&length_,&width_,datatype,endiantype);
  if (strcmp(matorvec,"MATRIX")!=0)
    PLERROR("In FileVMatrix constructor, wrong header for PLearn binary matrix format. Please use checkheader (in PLearn/Scripts) to check the file.(1)");

  if (strcmp(endiantype,"LITTLE_ENDIAN")==0)
    file_is_bigendian = false;
  else if (strcmp(endiantype,"BIG_ENDIAN")==0)
    file_is_bigendian = true;
  else
    PLERROR("In FileVMatrix constructor, wrong header for PLearn binary matrix format. Please use checkheader (in PLearn/Scripts) to check the file.(2)");

  if (strcmp(datatype,"FLOAT")==0)
    file_is_float = true;
  else if (strcmp(datatype,"DOUBLE")==0)
    file_is_float = false;
  else
    PLERROR("In FileVMatrix constructor, wrong header for PLearn binary matrix format. Please use checkheader (in PLearn/Scripts) to check the file.(3)");

  string fieldinfosfname = filename_+".fieldnames";
  if(isfile(fieldinfosfname))
    loadFieldInfos(fieldinfosfname);
}

void FileVMatrix::declareOptions(OptionList & ol)
{
  declareOption(ol, "filename", &FileVMatrix::filename_, OptionBase::buildoption, "Filename of the matrix");
  inherited::declareOptions(ol);
}

FileVMatrix::~FileVMatrix()
{ fclose(f); }

real FileVMatrix::get(int i, int j) const
{
  if(file_is_float)
    {
      fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(float), SEEK_SET);
      return (real) fread_float(f,file_is_bigendian);
    }
  else
    {
      fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(double), SEEK_SET);
      return (real) fread_double(f,file_is_bigendian);
    }
}

void FileVMatrix::getSubRow(int i, int j, Vec v) const
{
  if(file_is_float)
    {
      fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(float), SEEK_SET);
      fread_float(f, v.data(), v.length(), file_is_bigendian);
    }
  else
    {
      fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(double), SEEK_SET);
      fread_double(f, v.data(), v.length(), file_is_bigendian);
    }  
}

void FileVMatrix::putSubRow(int i, int j, Vec v)
{
  if(file_is_float)
    {
      fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(float), SEEK_SET);
      fwrite_float(f, v.data(), v.length(), file_is_bigendian);
    }
  else
    {
      fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(double), SEEK_SET);
      fwrite_double(f, v.data(), v.length(), file_is_bigendian);
    }  
}

void FileVMatrix::put(int i, int j, real value)
{
  if(file_is_float)
    {
      fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(float), SEEK_SET);
      fwrite_float(f,float(value),file_is_bigendian);
    }
  else
    {
      fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(double), SEEK_SET);
      fwrite_double(f,double(value),file_is_bigendian);
    }
}

void FileVMatrix::appendRow(Vec v)
{
  if(file_is_float)
    {
      fseek(f,DATAFILE_HEADERLENGTH+length_*width_*sizeof(float), SEEK_SET);
      fwrite_float(f, v.data(), v.length(), file_is_bigendian);
    }
  else
    {
      fseek(f,DATAFILE_HEADERLENGTH+length_*width_*sizeof(double), SEEK_SET);
      fwrite_double(f, v.data(), v.length(), file_is_bigendian);
    }
  length_++;

  char header[DATAFILE_HEADERLENGTH]; 

#ifdef USEFLOAT
#ifdef LITTLEENDIAN
 sprintf(header,"MATRIX %d %d FLOAT LITTLE_ENDIAN", length_, width_);
#endif
#ifdef BIGENDIAN
 sprintf(header,"MATRIX %d %d FLOAT BIG_ENDIAN", length_, width_);
#endif
#endif
#ifdef USEDOUBLE
#ifdef LITTLEENDIAN
 sprintf(header,"MATRIX %d %d DOUBLE LITTLE_ENDIAN", length_, width_);
#endif
#ifdef BIGENDIAN
 sprintf(header,"MATRIX %d %d DOUBLE BIG_ENDIAN", length_, width_);
#endif
#endif

  for(int pos=strlen(header); pos<DATAFILE_HEADERLENGTH; pos++)
    header[pos] = ' ';
  header[DATAFILE_HEADERLENGTH-1] = '\n';

  // write the header to the file
  fseek(f,0,SEEK_SET);
  fwrite(header,DATAFILE_HEADERLENGTH,1,f);
}

  // *******************
  // ** VecCompressor **

signed char* VecCompressor::compressVec(const Vec& v, signed char* data)
{
  real* vdata = v.data();
  signed char* ptr = data;

  // mode can be '0' for zeroes, 'F' for floats, 'I' for small integers (signed chars)
  // If mode is '0' abs(count) indicates the number of zeroes, 
  //                a positive sign indicates switch to 'F' mode
  //                a negative sign indicates switch to 'I' mode
  //                a 0 count means insert 127 zeros and stay in zero mode
  // If mode is 'F' abs(count) indicates the number of floats that follow 
  //                a positive sign indicates switch to 'I' mode
  //                a negative sign indicates switch to '0' mode
  //                a 0 count means insert 127 floats and stay in float mode
  // If mode is 'I' abs(count) indicates the number of small integers that follow 
  //                a positive sign indicates switch to 'F' mode
  //                a negative sign indicates switch to '0' mode
  //                a 0 count means insert 127 small integers and stay in 'I' mode

  int l = v.length();

  int i=0;
  real val = vdata[i];

  signed char mode = 'F';
  if(val==0.)
    mode = '0';
  else if(issmallint(val))
    mode = 'I';
  // else 'F'

  int count = 0;
  int istart = 0;
  float fval = 0.;
  signed char* pfval = (signed char*)&fval;

  *ptr++ = mode;

  while(i<l)
    {
      switch(mode)
        {
        case '0':
          istart = i;
          while(i<l && is0(vdata[i]))
              i++;
          count = i - istart;
          while(count>127)
            {
              *ptr++ = 0;
              count -= 127;
            }
          if(i>=l || issmallint(vdata[i]))
            {
              *ptr++ = (signed char)(-count);
              mode = 'I';
            }
          else
            {
              *ptr++ = (signed char)count;
              mode = 'F';
            }
          break;

        case 'I':
          istart = i;
          while(i<l && isI(vdata[i]))
              i++;
          count = i - istart;
          while(count>127)
            {
              *ptr++ = 0;
              int n = 127;
              while(n--)
                *ptr++ = (signed char)vdata[istart++];
              count -= 127;
            }
          if(i>=l || is0(vdata[i]))
            {
              *ptr++ = (signed char)(-count);
              mode = '0';
            }
          else // next value is a floating point
            {
              *ptr++ = (signed char)count;
              mode = 'F';
            }
          while(count--)
            *ptr++ = (signed char)vdata[istart++];                
          break;

        case 'F':
          istart = i;
          val = vdata[i];
          while(i<l && isF(vdata[i]))
            i++;
          count = i - istart;
          while(count>127)
            {
              *ptr++ = 0;
              int n = 127;
              while(n--)
                {
                  fval = (float)vdata[istart++];
                  *ptr++ = pfval[0];
                  *ptr++ = pfval[1];
                  *ptr++ = pfval[2];
                  *ptr++ = pfval[3];
                }
              count -= 127;
            }
          if(i>=l || is0(vdata[i]))
            {
              *ptr++ = (signed char)(-count);
              mode = '0';
            }
          else
            {
              *ptr++ = (signed char)count;
              mode = 'I';
            }
          while(count--)
            {
              fval = (float)vdata[istart++];
              *ptr++ = pfval[0];
              *ptr++ = pfval[1];
              *ptr++ = pfval[2];
              *ptr++ = pfval[3];
            }
        }
    }
  return ptr;
}

void VecCompressor::uncompressVec(signed char* data, const Vec& v)
{
  // mode can be '0' for zeroes, 'F' for floats, 'I' for small integers (signed chars)
  // If mode is '0' abs(count) indicates the number of zeroes, 
  //                a positive sign indicates switch to 'F' mode
  //                a negative sign indicates switch to 'I' mode
  //                a 0 count means insert 127 zeros and stay in zero mode
  // If mode is 'F' abs(count) indicates the number of floats that follow 
  //                a positive sign indicates switch to 'I' mode
  //                a negative sign indicates switch to '0' mode
  //                a 0 count means insert 127 floats and stay in float mode
  // If mode is 'I' abs(count) indicates the number of small integers that follow 
  //                a positive sign indicates switch to 'F' mode
  //                a negative sign indicates switch to '0' mode
  //                a 0 count means insert 127 small integers and stay in 'I' mode

  real* vptr = v.data();
  real* vptrend = vptr+v.length();
  signed char* ptr = data;
  signed char mode = *ptr++;
  float fval = 0.;
  signed char* pfval = (signed char*)&fval;
  signed char count;
  while(vptr!=vptrend)
    {
      count = *ptr++;
      switch(mode)
        {
        case '0':
          if(count<0)
            {
              mode = 'I';              
              count = -count;
            }
          else if(count>0)
            mode = 'F';
          else
            count = 127;

          while(count--)
            *vptr++ = 0.;
          break;

        case 'I':
          if(count<0)
            {
              mode = '0';
              count = -count;
            }
          else if(count>0)
            mode = 'F';
          else 
            count = 127;

          while(count--)
            *vptr++ = real(*ptr++);
          break;
          
        case 'F':
          if(count<0)
            {
              mode = '0';
              count = -count;
            }
          else if(count>0)
            mode = 'I';
          else 
            count = 127;

          while(count--)
            {
              pfval[0] = *ptr++;
              pfval[1] = *ptr++;
              pfval[2] = *ptr++;
              pfval[3] = *ptr++;
              *vptr++ = real(fval);
            }
          break;

        default:
          PLERROR("Problem in VecCompressor::uncompressVec this should not happen!!! (wrong data format?)");
        }
    }
}

void VecCompressor::writeCompressedVec(ostream& out, const Vec& v)
{
  real* vdata = v.data();

  // mode can be '0' for zeroes, 'F' for floats, 'I' for small integers (signed chars)
  // If mode is '0' abs(count) indicates the number of zeroes, 
  //                a positive sign indicates switch to 'F' mode
  //                a negative sign indicates switch to 'I' mode
  //                a 0 count means insert 127 zeros and stay in zero mode
  // If mode is 'F' abs(count) indicates the number of floats that follow 
  //                a positive sign indicates switch to 'I' mode
  //                a negative sign indicates switch to '0' mode
  //                a 0 count means insert 127 floats and stay in float mode
  // If mode is 'I' abs(count) indicates the number of small integers that follow 
  //                a positive sign indicates switch to 'F' mode
  //                a negative sign indicates switch to '0' mode
  //                a 0 count means insert 127 small integers and stay in 'I' mode

  int l = v.length();

  int i=0;
  real val = vdata[i];

  signed char mode = 'F';
  if(val==0.)
    mode = '0';
  else if(issmallint(val))
    mode = 'I';
  // else 'F'

  int count = 0;
  int istart = 0;
  float fval = 0.;

  write_sbyte(out,mode);

  while(i<l)
    {
      switch(mode)
        {
        case '0':
          istart = i;
          while(i<l && is0(vdata[i]))
              i++;
          count = i - istart;
          while(count>127)
            {
              write_sbyte(out,0);
              count -= 127;
            }
          if(i>=l || issmallint(vdata[i]))
            {
              write_sbyte(out,-count);
              mode = 'I';
            }
          else
            {
              write_sbyte(out,count);
              mode = 'F';
            }
          break;

        case 'I':
          istart = i;
          while(i<l && isI(vdata[i]))
              i++;
          count = i - istart;
          while(count>127)
            {
              write_sbyte(out,0);
              int n = 127;
              while(n--)
                write_sbyte(out,(signed char)vdata[istart++]);
              count -= 127;
            }
          if(i>=l || is0(vdata[i]))
            {
              write_sbyte(out,-count);
              mode = '0';
            }
          else // next value is a floating point
            {
              write_sbyte(out,count);
              mode = 'F';
            }
          while(count--)
            write_sbyte(out,(signed char)vdata[istart++]);                
          break;

        case 'F':
          istart = i;
          while(i<l && isF(vdata[i]))
            i++;
          count = i - istart;
          while(count>127)
            {
              write_sbyte(out,0);
              int n = 127;
              while(n--)
                {
                  fval = (float)vdata[istart++];
                  out.write((char*)&fval,4);
                }
              count -= 127;
            }
          if(i>=l || is0(vdata[i]))
            {
              write_sbyte(out,-count);
              mode = '0';
            }
          else
            {
              write_sbyte(out,count);
              mode = 'I';
            }
          while(count--)
            {
              fval = (float)vdata[istart++];
              out.write((char*)&fval,4);
            }
        }
    }
}

void VecCompressor::readCompressedVec(istream& in, const Vec& v)
{
  // mode can be '0' for zeroes, 'F' for floats, 'I' for small integers (signed chars)
  // If mode is '0' abs(count) indicates the number of zeroes, 
  //                a positive sign indicates switch to 'F' mode
  //                a negative sign indicates switch to 'I' mode
  //                a 0 count means insert 127 zeros and stay in zero mode
  // If mode is 'F' abs(count) indicates the number of floats that follow 
  //                a positive sign indicates switch to 'I' mode
  //                a negative sign indicates switch to '0' mode
  //                a 0 count means insert 127 floats and stay in float mode
  // If mode is 'I' abs(count) indicates the number of small integers that follow 
  //                a positive sign indicates switch to 'F' mode
  //                a negative sign indicates switch to '0' mode
  //                a 0 count means insert 127 small integers and stay in 'I' mode

  real* vptr = v.data();
  real* vptrend = vptr+v.length();
  signed char mode = read_sbyte(in);
  float fval = 0.;
  signed char count;

  while(vptr!=vptrend)
    {
      count = read_sbyte(in);
      // cerr << int(count) <<  ' ';
      switch(mode)
        {
        case '0':
          if(count<0)
            {
              mode = 'I';              
              count = -count;
            }
          else if(count>0)
            mode = 'F';
          else
            count = 127;

          while(count--)
            *vptr++ = 0.;
          break;

        case 'I':
          if(count<0)
            {
              mode = '0';
              count = -count;
            }
          else if(count>0)
            mode = 'F';
          else 
            count = 127;

          while(count--)
            *vptr++ = real(read_sbyte(in));
          break;
          
        case 'F':
          if(count<0)
            {
              mode = '0';
              count = -count;
            }
          else if(count>0)
            mode = 'I';
          else 
            count = 127;

          while(count--)
            {
              in.read((char*)&fval,4);
              *vptr++ = real(fval);
            }
          break;

        default:
          PLERROR("Problem in VecCompressor::readCompressedVec this should not happen!!! (wrong data format?)");
        }
    }
  // cerr << endl;
}


// ************************
// ** CompressedVMatrix **

void CompressedVMatrix::init(int the_max_length, int the_width, size_t memory_alloc)
{
  length_ = 0;
  width_ = the_width;
  max_length = the_max_length;
  data = new signed char[memory_alloc];
  dataend = data+memory_alloc;
  curpos = data;
  rowstarts = new signed char*[max_length];
}

CompressedVMatrix::CompressedVMatrix(VMat m, size_t memory_alloc)
{
  if(memory_alloc==0)
    init(m.length(), m.width(), m.length()*VecCompressor::worstCaseSize(m.width()));
  Vec v(m.width());
  for(int i=0; i<m.length(); i++)
    {
      m->getRow(i,v);
      appendRow(v);
    }
}

void CompressedVMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(v.length() != width_)
    PLERROR("In CompressedVMatrix::getRow length of v and width of matrix do not match");
  if(i<0 || i>=length_)
    PLERROR("In CompressedVMatrix::getRow OUT OF BOUNDS row index");
#endif
  VecCompressor::uncompressVec(rowstarts[i],v);
}

void CompressedVMatrix::appendRow(Vec v)
{
  if(length_>=max_length)
    PLERROR("In CompressedVMatrix::appendRow, max_length exceeded");
  rowstarts[length_] = curpos;
  curpos = VecCompressor::compressVec(v,curpos);
  if(curpos>dataend)
    PLERROR("In CompressedVMatrix::appendRow not enough space reserved for data");
  ++length_;
}

void CompressedVMatrix::compacify()
{
  size_t datasize = curpos-data;
  signed char* old_data = data;
  signed char** old_rowstarts = rowstarts;
  data = new signed char[datasize];
  dataend = data+datasize;
  curpos = dataend;
  rowstarts = new signed char*[length_];
  memcpy(data, old_data, datasize);

  for(int i=0; i<length_; i++)
    rowstarts[i] = data + (old_rowstarts[i]-old_data);
  max_length = length_;
  delete[] old_data;
  delete[] old_rowstarts;
}

CompressedVMatrix::~CompressedVMatrix()
{
  if(data)
    delete[] data;
  if(rowstarts)
    delete[] rowstarts;
}


/** DiskVMatrix **/

DiskVMatrix::DiskVMatrix(const string& the_dirname, bool readwrite)
  : readwritemode(readwrite),freshnewfile(false),
    dirname(remove_trailing_slash(the_dirname))
{
  build_();
}

DiskVMatrix::DiskVMatrix(const string& the_dirname, int the_width, bool write_double_as_float)  
  : RowBufferedVMatrix(0,the_width),readwritemode(true), 
  freshnewfile(true),dirname(remove_trailing_slash(the_dirname))
{
  build_();
}


void DiskVMatrix::build()
{
  inherited::build();
  build_();
}

void DiskVMatrix::build_()
{
  if(!freshnewfile)
  {
    if(!isdir(dirname))
      PLERROR("In DiskVMatrix constructor, directory %s could not be found",dirname.c_str());
    setMetaDataDir(dirname + ".metadata"); 
    setMtime(mtime(append_slash(dirname)+"indexfile"));
    ios::openmode omode;
    if(readwritemode)
      omode = ios::in | ios::out | ios::binary;
    else // read-only
      omode = ios::in | ios::binary;

    string indexfname = dirname+"/indexfile";
    indexf = new fstream();
    indexf->open(indexfname.c_str(), omode);
    if(!*indexf)
      PLERROR("In DiskVMatrix constructor, could not open file %s in specified mode", indexfname.c_str());
  
    int header;
    indexf->read((char*)&header,sizeof(int));
    indexf->read((char*)&length_,sizeof(int));
    indexf->read((char*)&width_,sizeof(int));

    int k=0;
    string fname = dirname+"/"+tostring(k)+".data";
    while(isfile(fname))
    {
      fstream* f = new fstream();
      f->open(fname.c_str(), omode);
      if(!(*f))
        PLERROR("In DiskVMatrix constructor, could not open file %s in specified mode", fname.c_str());
      dataf.append(f);
      fname = dirname+"/"+tostring(++k)+".data";
    }
    // Stuff related to RowBufferedVMatrix, for consistency
    current_row_index = -1;
    current_row.resize(width_);
    other_row_index = -1;
    other_row.resize(width_);

    string fieldinfosfname = dirname+"/fieldnames";
    if(isfile(fieldinfosfname))
      loadFieldInfos(fieldinfosfname);
  }
  else
  {
    if(isdir(dirname))
      PLERROR("In DiskVMatrix constructor (with specified width), directory %s already exists",dirname.c_str());
    setMetaDataDir(dirname + ".metadata");
    setMtime(mtime(append_slash(dirname)+"indexfile"));

    //ios::openmode omode;
    if(isfile(dirname)) // patch for running mkstemp (TmpFilenames)
      unlink(dirname.c_str());
    if(!force_mkdir(dirname)) // force directory creation 
      PLERROR("In DiskVMatrix constructor (with specified width), could not create directory %s  Error was: %s",dirname.c_str(), strerror(errno));

    string indexfname = dirname + "/indexfile";
    indexf = new fstream();
    indexf->open(indexfname.c_str(),ios::in | ios::out | ios::trunc | ios::binary);

    int header = 123408; 
    indexf->write((char*)&header,sizeof(int));
    indexf->write((char*)&length_,sizeof(int));
    indexf->write((char*)&width_,sizeof(int));
  
    string fname = dirname + "/0.data";
    // These two line don't work (core dump!) with our actual libraries (sigh!)
    fstream* f = new fstream();
    f->open(fname.c_str(), ios::in | ios::out | ios::trunc | ios::binary);
    dataf.append(f);
  }
  freshnewfile=false;
}

void DiskVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "dirname", &DiskVMatrix::dirname, OptionBase::buildoption, "Directory name of the.dmat");
  inherited::declareOptions(ol);
}

void DiskVMatrix::getRow(int i, Vec v) const
{ 
#ifdef BOUNDCHECK
  if(i<0 || i>length())
    PLERROR("In DiskVMatrix::getRow, bad row number %d",i);
  if(v.length() != width())
    PLERROR("In DiskVMatrix::getRow, length of v (%d) does not match matrix width (%d)",v.length(),width());
#endif

  unsigned char filenum;
  unsigned int position;
  indexf->seekg(3*sizeof(int) + i*(sizeof(unsigned char)+sizeof(unsigned int)));
  indexf->get((char&)filenum);
  indexf->read((char*)&position,sizeof(unsigned int));
  fstream* f = dataf[int(filenum)];
  f->seekg(position);
  binread_compressed(*f,v.data(),v.length());
}

void DiskVMatrix::putRow(int i, Vec v)
{ 
#ifdef BOUNDCHECK
  if(i<0 || i>length())
    PLERROR("In DiskVMatrix::putRow, bad row number %d",i);
  if(v.length() != width())
    PLERROR("In DiskVMatrix::putRow, length of v (%d) does not match matrix width (%d)",v.length(),width());
#endif

  unsigned char filenum;
  unsigned int position;
  indexf->seekg(3*sizeof(int) + i*(sizeof(unsigned char)+sizeof(unsigned int)));
  indexf->get((char&)filenum);
  indexf->read((char*)&position,sizeof(unsigned int));
  fstream* f = dataf[int(filenum)];
  f->seekp(position);
  binwrite_compressed(*f,v.data(), v.length());
}

void DiskVMatrix::appendRow(Vec v)
{
  if(!readwritemode)
    PLERROR("In DiskVMatrix::appendRow cannot append row in read only mode, set readwrite parameter to true when calling the constructor");
  if(v.length() != width())
    PLERROR("In DiskVMatrix::appendRow, length of v (%d) does not match matrix width (%d)",v.length(),width());

  int filenum = dataf.size()-1;
  fstream* f = dataf[filenum];
  f->seekp(0,ios::end);
  unsigned int position = f->tellp();
  if(position>500000000L)
    {
      filenum++;
      string filename = dirname + "/" + tostring(filenum) + ".data";
      f = new fstream();
      f->open(filename.c_str(), ios::in | ios::out);
      dataf.append(f);
      position = 0;
    }
  binwrite_compressed(*f,v.data(),v.length());
  indexf->seekp(0,ios::end);
  indexf->put((unsigned char)filenum);
  indexf->write((char*)&position,sizeof(unsigned int));
  length_++;
  indexf->seekp(sizeof(int),ios::beg);
  indexf->write((char*)&length_,sizeof(int));
  //  indexf.flush();
}

DiskVMatrix::~DiskVMatrix()
{
  for(int i=0; i<dataf.size(); i++)
    delete dataf[i];
  delete indexf;
}

IMPLEMENT_NAME_AND_DEEPCOPY(DiskVMatrix);

/** UpsideDownVMatrix **/

UpsideDownVMatrix::UpsideDownVMatrix(VMat the_distr):
  VMatrix(the_distr->length(), the_distr->width()),
  distr(the_distr)
{
  fieldinfos = distr->getFieldInfos();
}
  
real UpsideDownVMatrix::get(int i, int j) const
{ return distr->get(length()-i-1,j); }

void UpsideDownVMatrix::getSubRow(int i, int j, Vec v) const
{ distr->getSubRow(length()-i-1,j,v); }

void UpsideDownVMatrix::put(int i, int j, real value)
{ distr->put(length()-i-1,j,value); }

void UpsideDownVMatrix::putSubRow(int i, int j, Vec v)
{ distr->putSubRow(length()-i-1,j,v); }


/** SelectRowsVMatrix **/

real SelectRowsVMatrix::get(int i, int j) const
{ return distr->get(indices[i], j); }

void SelectRowsVMatrix::getSubRow(int i, int j, Vec v) const
{ distr->getSubRow(indices[i], j, v); }

real SelectRowsVMatrix::dot(int i1, int i2, int inputsize) const
{ return distr->dot(int(indices[i1]), int(indices[i2]), inputsize); }

real SelectRowsVMatrix::dot(int i, const Vec& v) const
{ return distr->dot(indices[i],v); }

real SelectRowsVMatrix::getStringVal(int col, const string & str) const
{ return distr->getStringVal(col, str); }

string SelectRowsVMatrix::getValString(int col, real val) const
{ return distr->getValString(col,val); }


/** RemoveRowsVMatrix **/

int RemoveRowsVMatrix::getrownum(int i) const
{
  int k=0;
  while(k<indices.length() && indices[k]<=i)
  {
    i++;
    k++;
  }
  return i;
}

real RemoveRowsVMatrix::get(int i, int j) const
{ return distr->get(getrownum(i), j); }

void RemoveRowsVMatrix::getSubRow(int i, int j, Vec v) const
{ distr->getSubRow(getrownum(i), j, v); }

real RemoveRowsVMatrix::dot(int i1, int i2, int inputsize) const
{ return distr->dot(getrownum(i1),getrownum(i2),inputsize); }

real RemoveRowsVMatrix::dot(int i, const Vec& v) const
{ return distr->dot(getrownum(i),v); }

/** SelectRowsFileIndexVMatrix **/

real SelectRowsFileIndexVMatrix::get(int i, int j) const
{ return distr->get(indices[i], j); }

void SelectRowsFileIndexVMatrix::getSubRow(int i, int j, Vec v) const
{ distr->getSubRow(indices[i], j, v); }

void SelectRowsFileIndexVMatrix::getRow(int i, Vec v) const
{ distr->getRow(indices[i], v); }

real SelectRowsFileIndexVMatrix::dot(int i1, int i2, int inputsize) const
{ return distr->dot(indices[i1],indices[i2],inputsize); }

real SelectRowsFileIndexVMatrix::dot(int i, const Vec& v) const
{ return distr->dot(indices[i],v); }

real SelectRowsFileIndexVMatrix::getStringVal(int col, const string & str) const
{ return distr->getStringVal(col, str); }

string SelectRowsFileIndexVMatrix::getValString(int col, real val) const
{ return distr->getValString(col,val); }

/** SelectColumnsVMatrix **/

SelectColumnsVMatrix::SelectColumnsVMatrix(VMat the_distr, TVec<int> the_indices)
  : VMatrix(the_distr->length(), the_indices.length()),
    distr(the_distr), indices(the_indices)
{
  //! Copy the appropriate VMFields
  int len = the_indices.length();
  fieldinfos.resize(len);
  if (distr->getFieldInfos().size() > 0)
    for (int i=0; i<len; ++i)
      fieldinfos[i] = distr->getFieldInfos()[indices[i]];
}

SelectColumnsVMatrix::SelectColumnsVMatrix(VMat the_distr, Vec the_indices)
  : VMatrix(the_distr->length(), the_indices.length()),
    distr(the_distr), indices(the_indices.length())
{
  // copy the real the_indices into the integer indices
  indices << the_indices;

  //! Copy the appropriate VMFields
  int len = the_indices.length();
  fieldinfos.resize(len);
  if (distr->getFieldInfos().size() > 0)
    for (int i=0; i<len; ++i)
      fieldinfos[i] = distr->getFieldInfos()[indices[i]];
}

real SelectColumnsVMatrix::get(int i, int j) const
{ return distr->get(i, indices[j]); }

void SelectColumnsVMatrix::getSubRow(int i, int j, Vec v) const
{
  for(int jj=0; jj<v.length(); jj++)
    v[jj] = distr->get(i, indices[j+jj]); 
}

/** Uniform VMatrix **/

UniformVMatrix::UniformVMatrix(real the_minval, real the_maxval, int the_width)
  :VMatrix(-1,the_width),minval(the_minval), maxval(the_maxval)
{}

real UniformVMatrix::get(int i, int j) const
{
  double scale = maxval-minval;
  return uniform_sample()*scale+minval;
}

void UniformVMatrix::getSubRow(int i, int j, Vec v) const
{
  double scale = maxval-minval;
  for(int k=0; k<v.length(); k++)
    v[k] = uniform_sample()*scale+minval;
}

/** Range VMatrix **/

RangeVMatrix::RangeVMatrix(real the_start, real the_end, real the_step)
  :start(the_start), end(the_end), step(the_step)
{
  width_=1;
  length_ = (int)((end-start)/step); 
  if (length_*step==(end-start))
    length_++;
}

real RangeVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
  if(j!=0 || i<0 || i>=length())
    PLERROR("In RangeVMatrix::get OUT OF BOUNDS");
#endif
  return start+i*step;
}

/** ConcatColumnsVMatrix **/

IMPLEMENT_NAME_AND_DEEPCOPY(ConcatColumnsVMatrix);

void ConcatColumnsVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "array", &ConcatColumnsVMatrix::array, OptionBase::buildoption, "Array of VMatrices");
  inherited::declareOptions(ol);
}

void ConcatColumnsVMatrix::build()
{
  inherited::build();
  build_();
}

void ConcatColumnsVMatrix::build_()
{
  length_ = width_ = 0;
  if(array.size())
    length_ = array[0]->length();
  else
    PLERROR("ConcatRowsVMatrix expects >= 1 underlying-array, got 0");

  for(int i=0; i<array.size(); i++)
    {
      if(array[i]->length()!=length_)
        PLERROR("ConcatColumnsVMatrix: Problem concatenating to VMatrices with differnet lengths");
      if(array[i]->width() == -1)
        PLERROR("In ConcatColumnsVMatrix constructor. Non-fixed width distribution not supported");
      width_ += array[i]->width();
    }

  // Copy the original fieldinfos.  Be careful if only some of the
  // matrices have fieldinfos
  fieldinfos.resize(width_);
  int fieldindex = 0;
  for (int i=0; i<array.size(); ++i) 
    {
      int len = array[i]->getFieldInfos().size();
      if (len > 0) // infos exist for this VMat
        {
          for (int j=0; j<len; ++j)
            fieldinfos[fieldindex++] = array[i]->getFieldInfos()[j];
        }
      else // infos don't exist for this VMat, use the index as the name for those fields.
        {
          len = array[i]->width();
          for(int j=0; j<len; ++j)
            fieldinfos[fieldindex++] = VMField(tostring(fieldindex));
        }
    }
}

void ConcatColumnsVMatrix::getRow(int i, Vec samplevec) const
{
  if (length_==-1)
    PLERROR("In ConcatColumnsVMatrix::getRow(int i, Vec samplevec) not supported for distributions with different (or infinite) lengths\nCall sample without index instead");
  samplevec.resize(width_);
  int pos = 0;
  for(int n=0; n<array.size(); n++)
    {
      int nvars = array[n]->width();
      Vec samplesubvec = samplevec.subVec(pos, nvars);
      array[n]->getRow(i,samplesubvec);
      pos += nvars;
    }
}

real ConcatColumnsVMatrix::getStringVal(int col, const string & str) const
{
  if(col>=width_)
    PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
  int pos=0,k=0;
  while(col>=pos+array[k]->width())
    {
      pos += array[k]->width();
      k++;
    }
  return array[k]->getStringVal(pos+col,str);
}

string ConcatColumnsVMatrix::getValString(int col, real val) const
{
  if(col>=width_)
    PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
  int pos=0,k=0;
  while(col>=pos+array[k]->width())
    {
      pos += array[k]->width();
      k++;
    }
  return array[k]->getValString(pos+col,val);
}


real ConcatColumnsVMatrix::dot(int i1, int i2, int inputsize) const
{
  real res = 0.;
  for(int k=0; ;k++)
    {
      const VMat& vm = array[k];
      int vmwidth = vm.width();
      if(inputsize<=vmwidth)
        {
          res += vm->dot(i1,i2,inputsize);
          break;
        }
      else
        {
          res += vm->dot(i1,i2,vmwidth);
          inputsize -= vmwidth;
        }
    }
  return res;
}

real ConcatColumnsVMatrix::dot(int i, const Vec& v) const
{
  if (length_==-1)
    PLERROR("In ConcatColumnsVMatrix::getRow(int i, Vec samplevec) not supported for distributions with different (or infinite) lengths\nCall sample without index instead");

  real res = 0.;
  int pos = 0;
  for(int n=0; n<array.size(); n++)
    {
      int nvars = std::min(array[n]->width(),v.length()-pos);
      if(nvars<=0)
        break;
      Vec subv = v.subVec(pos, nvars);
      res += array[n]->dot(i,subv);
      pos += nvars;
    }
  return res;
}


/** ByteMemoryVMatrix **/

ByteMemoryVMatrix::
ByteMemoryVMatrix(unsigned char* the_data,int the_length,int the_width, Vec the_scale)
  :VMatrix(the_length, the_width), data(the_data), 
   scale(the_scale), offset_(the_scale.length())
{
  if (the_scale.length() != width_)
    PLERROR("ByteMemoryVMatrix: inconsistent arguments (scale(%d),n_col(%d))",
          the_scale.length(), width_);
}

ByteMemoryVMatrix::
ByteMemoryVMatrix(unsigned char* the_data,int the_length,int the_width, 
                          Vec the_scale, Vec the_offset)
  :VMatrix(the_length, the_width), data(the_data), 
   scale(the_scale), offset_(the_offset)
{
  if (the_scale.length() != width_ || the_offset.length()!=width_)
    PLERROR("ByteMemoryVMatrix: inconsistent arguments (scale(%d),offset(%d),n_col(%d))",
          the_scale.length(), the_offset.length(), width_);
}

ByteMemoryVMatrix::ByteMemoryVMatrix(unsigned char* the_data,int the_length,int the_width,
                            double the_scaling_factor,double the_offset)
  :VMatrix(the_length, the_width), data(the_data), 
   scale(the_width, the_scaling_factor), offset_(the_width, the_offset)
{}

real ByteMemoryVMatrix::get(int i, int j) const
{
  return ( data[i*width()+j] + offset_[j] ) * scale[j];
}

void ByteMemoryVMatrix::getSubRow(int i, int j, Vec samplevec) const
{
  unsigned char* p = &data[i*width_];
  real *v = samplevec.data();
  real *s = scale.data();
  real *o = offset_.data();
  for (int jj=0; jj<samplevec.length(); jj++)
    v[jj] = s[j+jj] * (p[j+jj] + o[j+jj]);
}

/** PairsVMatrix **/

PairsVMatrix::PairsVMatrix(Mat the_data1, Mat the_data2)
  : RowBufferedVMatrix(data1.width()+data2.width(), data1.length()*data2.length()), 
    data1(the_data1), data2(the_data2)
{}

void PairsVMatrix::getRow(int ij, Vec samplevec) const
{
  //ij = ij%length_;
  ij %= length_;
  samplevec.resize(width_);
  real* data = samplevec.data();
  real* data_i = data1[ij/data2.length()];
  real* data_j = data2[ij%data2.length()];
  int kk=0;
  for (int k=0;k<data1.width();k++)
    data[kk++] = data_i[k];
  for (int k=0;k<data2.width();k++)
    data[kk++] = data_j[k];
}

/** ConcatRowsVMatrix **/

IMPLEMENT_NAME_AND_DEEPCOPY(ConcatRowsVMatrix);

void ConcatRowsVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "array", &ConcatRowsVMatrix::array, OptionBase::buildoption, "Array of VMatrices");
  inherited::declareOptions(ol);
}

void ConcatRowsVMatrix::build()
{
  inherited::build();
  build_();
}

void ConcatRowsVMatrix::build_()
{
  int n = array.size();
  if (n < 1)
    PLERROR("ConcatRowsVMatrix expects >= 1 underlying-array, got 0");

  // Copy the field names
  fieldinfos = array[0]->getFieldInfos();
  
  width_ = array[0]->width();
  length_ = 0;
  for (int i=0;i<n;i++)
  {
    if (array[i]->width() != width_)
      PLERROR("ConcatRowsVMatrix: underlying-array %d has %d width, while 0-th has %d",i,array[i]->width(),width_);
    length_ += array[i]->length();
  }
}

//! Warning : the string map used is the one from the first of the concatenated matrices
real ConcatRowsVMatrix::getStringVal(int col, const string & str) const
{
  if(col>=width_)
    PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
  return array[0]->getStringVal(col,str);
}


void ConcatRowsVMatrix::getpositions(int i, int& whichvm, int& rowofvm) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In ConcatRowsVMatrix::getpositions OUT OF BOUNDS");
#endif

  int pos = 0;
  int k=0;
  while(i>=pos+array[k]->length())
    {
      pos += array[k]->length();
      k++;
    }

  whichvm = k;
  rowofvm = i-pos;
}

real ConcatRowsVMatrix::get(int i, int j) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  return array[whichvm]->get(rowofvm,j);
}

void ConcatRowsVMatrix::getSubRow(int i, int j, Vec v) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  array[whichvm]->getSubRow(rowofvm, j, v);
}

real ConcatRowsVMatrix::dot(int i1, int i2, int inputsize) const
{
  int whichvm1, rowofvm1;
  getpositions(i1,whichvm1,rowofvm1);
  int whichvm2, rowofvm2;
  getpositions(i2,whichvm2,rowofvm2);
  if(whichvm1==whichvm2)
    return array[whichvm1]->dot(rowofvm1, rowofvm2, inputsize);
  else
    return VMatrix::dot(i1,i2,inputsize);
}

real ConcatRowsVMatrix::dot(int i, const Vec& v) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  return array[whichvm]->dot(rowofvm,v);
}


/** ConcatRowsSubVMatrix **/

ConcatRowsSubVMatrix::ConcatRowsSubVMatrix
(VMat the_distr, TVec<int>& the_start, TVec<int>& the_len) :
  VMatrix(-1,the_distr->width()),
  distr(the_distr), start(the_start), len(the_len)
{
  //! Copy parent field names
  fieldinfos = the_distr->getFieldInfos();
  
  check();
}

ConcatRowsSubVMatrix::ConcatRowsSubVMatrix
(VMat the_distr, int start1, int len1, int start2, int len2) :
  VMatrix(-1,the_distr->width()),
  distr(the_distr), start(2), len(2)
{
  //! Copy parent field names
  fieldinfos = the_distr->getFieldInfos();
  
  start[0]=start1;
  start[1]=start2;
  len[0]=len1;
  len[1]=len2;
  check();
}

void ConcatRowsSubVMatrix::check()
{
  length_=0;
  for (int i=0;i<start.length();i++)
  {
    if (start[i]<0 || start[i]+len[i]>distr->length())
      PLERROR("ConcatRowsSubVMatrix: out-of-range specs for sub-distr %d, "
            "start=%d, len=%d, underlying distr length=%d",i,start[i],len[i],
            distr->length());
    length_ += len[i];
  }
}


void ConcatRowsSubVMatrix::getpositions(int i, int& whichvm, int& rowofvm) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In ConcatRowsSubVMatrix::getpositions OUT OF BOUNDS");
#endif

  int pos = 0;
  int k=0;
  while(i>=pos+len[k])
    {
      pos += len[k];
      k++;
    }

  whichvm = k;
  rowofvm = i-pos;
}

real ConcatRowsSubVMatrix::get(int i, int j) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  return distr->get(start[whichvm]+rowofvm,j);
}

void ConcatRowsSubVMatrix::getSubRow(int i, int j, Vec v) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  distr->getSubRow(start[whichvm]+rowofvm, j, v);
}

real ConcatRowsSubVMatrix::dot(int i1, int i2, int inputsize) const
{
  int whichvm1, rowofvm1;
  getpositions(i1,whichvm1,rowofvm1);
  int whichvm2, rowofvm2;
  getpositions(i2,whichvm2,rowofvm2);
  return distr->dot(start[whichvm1]+rowofvm1, start[whichvm2]+rowofvm2, inputsize);
}

real ConcatRowsSubVMatrix::dot(int i, const Vec& v) const
{
  int whichvm, rowofvm;
  getpositions(i,whichvm,rowofvm);
  return distr->dot(start[whichvm]+rowofvm,v);
}


/** InterleaveVMatrix **/

InterleaveVMatrix::InterleaveVMatrix(Array<VMat> the_vm)
  :vm(the_vm)
{
  int n=vm.size();
  if (n<1) 
    PLERROR("InterleaveVMatrix expects >= 1 underlying-distribution, got %d",n);

  // Copy the parent fields
  fieldinfos = vm[0]->getFieldInfos();
  
  width_ = vm[0]->width();
  int maxl = 0;
  for (int i=0;i<n;i++)
    {
      if (vm[i]->width() != width_)
        PLERROR("InterleaveVMatrix: underlying-distr %d has %d width, while 0-th has %d",i,vm[i]->width(),width_);
      int l=vm[i]->length();
      if (l>maxl) maxl=l;
    }
  length_ = n*maxl;
}

InterleaveVMatrix::InterleaveVMatrix(VMat d1, VMat d2)
  :vm(d1,d2)
{
  int n=vm.size();
  if (n<1) 
    PLERROR("InterleaveVMatrix expects >= 1 underlying-distribution, got %d",n);

  // Copy the parent fields
  fieldinfos = vm[0]->getFieldInfos();
  
  width_ = vm[0]->width();
  int maxl = 0;
  for (int i=0;i<n;i++)
    {
      if (vm[i]->width() != width_)
        PLERROR("InterleaveVMatrix: underlying-distr %d has %d width, while 0-th has %d",i,vm[i]->width(),width_);
      int l=vm[i]->length();
      if (l>maxl) maxl=l;
    }
  length_ = n*maxl;
}

real InterleaveVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In InterleaveVMatrix::get OUT OF BOUNDS");
#endif
  int n=vm.size();
  int m = i%n; // which VM 
  int pos = int(i/n) % vm[m].length(); // position within vm[m]
  return vm[m]->get(pos,j);
}

void InterleaveVMatrix::getSubRow(int i, int j, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j+v.length()>width())
    PLERROR("In InterleaveVMatrix::getRow OUT OF BOUNDS");
#endif
  int n=vm.size();
  int m = i%n; // which VM 
  int pos = int(i/n) % vm[m].length(); // position within vm[m]
  vm[m]->getSubRow(pos, j, v);
}

/** OneHotVMatrix **/

OneHotVMatrix::OneHotVMatrix(VMat the_underlying_distr, int the_nclasses, real the_cold_value, real the_host_value)
  :RowBufferedVMatrix(the_underlying_distr->length(), the_underlying_distr->width()+the_nclasses-1),
   underlying_distr(the_underlying_distr), nclasses(the_nclasses), cold_value(the_cold_value), hot_value(the_host_value)
{}

void OneHotVMatrix::getRow(int i, Vec samplevec) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In OneHotVMatrix::getRow OUT OF BOUNDS");
  if(samplevec.length()!=width())
    PLERROR("In OneHotVMatrix::getRow samplevec.length() must be equal to the VMat's width");
#endif
  Vec input = samplevec.subVec(0,width()-nclasses);
  Vec target = samplevec.subVec(width()-nclasses,nclasses);
  underlying_distr->getSubRow(i,0,input);
  int classnum = int(underlying_distr->get(i,underlying_distr->width()-1));
  fill_one_hot(target,classnum,cold_value,hot_value);
}

real OneHotVMatrix::dot(int i1, int i2, int inputsize) const
{
  return underlying_distr->dot(i1,i2,inputsize);
}

real OneHotVMatrix::dot(int i, const Vec& v) const
{
  return underlying_distr->dot(i,v);
}

/** FileVMatrix **/


/** GeneralizedOneHotVMatrix **/

GeneralizedOneHotVMatrix::GeneralizedOneHotVMatrix(VMat the_distr,
  Vec the_index, Vec the_nclasses, Vec the_cold_value, Vec the_host_value)
  : RowBufferedVMatrix(the_distr->length(), the_distr->width()+(int)sum(the_nclasses)-the_nclasses.length()),
   distr(the_distr), index(the_index), nclasses(the_nclasses),
   cold_value(the_cold_value), hot_value(the_host_value)
{
  if (min(index)<0 || max(index)>distr->length()-1)
    PLERROR("In GeneralizedOneHotVMatrix: all values of index must be in range [0,%d]",
      distr->length()-1);
  if (index.length()!=nclasses.length() || cold_value.length()!=hot_value.length() || index.length()!=hot_value.length())
    PLERROR("In GeneralizedOneHotVMatrix: index, nclasses, cold_value and hot_value must have same length");
}

void GeneralizedOneHotVMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In OneHotVMatrix::getRow OUT OF BOUNDS");
  if(v.length()!=width())
    PLERROR("In GeneralizedOneHotVMatrix::getRow v.length() must be equal to the VMat's width");
#endif

  Vec input(distr->width());
  distr->getRow(i, input);
  int v_pos = 0;
  for (int j=0; j<input.length(); j++) {
    const int index_pos = vec_find(index, (real)j);
    if (index_pos == -1)
      v[v_pos++] = input[j];
    else {
      const int nb_class = (int)nclasses[index_pos];
      Vec target = v.subVec(v_pos, nb_class);
      const real cold = cold_value[index_pos];
      const real hot = hot_value[index_pos];
      const int classnum = int(distr->get(i,j));
      fill_one_hot(target, classnum, cold, hot);
      v_pos += nb_class;
    }
  }
}


/** RemapLastColumnVMatrix **/

RemapLastColumnVMatrix::RemapLastColumnVMatrix(VMat the_underlying_distr, Mat the_mapping)
  :RowBufferedVMatrix(the_underlying_distr->length(), the_underlying_distr->width()+the_mapping.width()-2),
   underlying_distr(the_underlying_distr), mapping(the_mapping)
{}

RemapLastColumnVMatrix::RemapLastColumnVMatrix(VMat the_underlying_distr, real if_equals_value, real then_value, real else_value)
  :RowBufferedVMatrix(the_underlying_distr->length(), the_underlying_distr->width()),
   underlying_distr(the_underlying_distr), 
   if_equals_val(if_equals_value),
   then_val(then_value),
   else_val(else_value)
{}

void RemapLastColumnVMatrix::getRow(int i, Vec samplevec) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In RemapLastColumnVMatrix::getRow OUT OF BOUNDS");
  if(samplevec.length()!=width())
    PLERROR("In RemapLastColumnVMatrix::getRow samplevec.length() must be equal to the VMat's width");
#endif
  if(mapping.isEmpty()) // use if-then-else mapping
  {
    underlying_distr->getRow(i,samplevec);
    real& lastelem = samplevec.lastElement();
    if(lastelem==if_equals_val)
      lastelem = then_val;
    else
      lastelem = else_val;
  }
  else // use mapping matrix
  {
    int underlying_width = underlying_distr->width();
    int replacement_width = mapping.width()-1;
    underlying_distr->getRow(i,samplevec.subVec(0,underlying_width));
    real val = samplevec[underlying_width-1];
    int k;
    for(k=0; k<mapping.length(); k++)
    {
      if(mapping(k,0)==val)
      {
        samplevec.subVec(underlying_width-1,replacement_width) << mapping(k).subVec(1,replacement_width);
        break;
      }
    }
    if(k>=mapping.length())
      PLERROR("In RemapLastColumnVMatrix::getRow there is a value in the last column that does not have any defined mapping");
  }
}

/** CrossReferenceVMatrix **/

CrossReferenceVMatrix::CrossReferenceVMatrix(VMat v1, int c1, VMat v2)
 : VMatrix(v1.length(), v1.width()+v2.width()-1), vm1(v1), col1(c1), vm2(v2)
{
  fieldinfos = v1->getFieldInfos();
  fieldinfos &= v2->getFieldInfos();
}


void CrossReferenceVMatrix::getRow(int i, Vec samplevec) const
{
#ifdef BOUNDCHECK
  if (i<0 || i>=length() || samplevec.length()!=width())
    PLERROR("In CrossReferenceVMatrix::getRow OUT OF BOUNDS");
#endif

  Vec v1(vm1.width());
  Vec v2(vm2.width());
  vm1->getRow(i, v1);
  int index = (int)v1[col1];
  vm2->getRow(index, v2);

  for (int j=0; j<col1; j++) samplevec[j] = v1[j];
  for (int j=col1+1; j<v1.length(); j++) samplevec[j-1] = v1[j];
  for (int j=0; j<v2.length(); j++) samplevec[j+v1.length()-1] = v2[j];
}

real CrossReferenceVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In CrossReferenceVMatrix::get OUT OF BOUNDS");
#endif

  if (j < col1)
    return vm1->get(i,j);
  else if (j < vm1.width()-1)
    return vm1->get(i,j+1);
  else {
    int ii = (int)vm1->get(i,col1);
    int jj = j - vm1.width() + 1;
    return vm2->get(ii,jj);
  }
}

YMDDatedVMatrix::YMDDatedVMatrix(VMat data_, Mat years_, Mat months_, Mat days_)
    : DatedVMatrix(data_->length(),data_->width()),data(data_), years(years_), 
      months(months_), days(days_), day_of_ith_pos(days_.length())
{
  init();
}

YMDDatedVMatrix::YMDDatedVMatrix(Mat& YMD_and_data)
  : DatedVMatrix(YMD_and_data.length(),YMD_and_data.width()-3),
    data(YMD_and_data.subMatColumns(3,YMD_and_data.width()-3)), 
    years(YMD_and_data.subMatColumns(0,1)),
    months(YMD_and_data.subMatColumns(1,1)),
    days(YMD_and_data.subMatColumns(2,1)), day_of_ith_pos(YMD_and_data.length())
{
  init();
}

void YMDDatedVMatrix::init()
{
  // check that the dates are in increasing chronological order and
  // compute the pos_of_ith_{year,month,day} vectors
  if (years.length()!=data->length() ||
      months.length()!=data->length() ||
      days.length()!=data->length())
    PLERROR("YMDDatedVMatrix: arguments should all have the same length");

  // build pos_of_date
  int first_year = (int)years(0,0);
  int last_year = (int)years(data->length()-1,0);
  int ny=last_year-first_year+1;
  pos_of_date.resize(ny);
  for (int y=0;y<ny;y++)
    {
      pos_of_date[y].resize(12,31);
      for (int m=0;m<12;m++)
        for (int d=0;d<31;d++)
          pos_of_date[y](m,d)= -1; // -1 will mean unseen date
    }

  int n_different_years=1;
  int n_different_months=1;
  int n_different_days=1;
  for (int i=1;i<years.length();i++)
    {
      if (years(i,0)>years(i-1,0)) 
        {
          n_different_years++;
          n_different_months++;
          n_different_days++;
          pos_of_date[int(years(i,0)-first_year)](int(months(i,0)-1), int(days(i,0)-1)) = i;
        }
      else
        {
          if (years(i,0)<years(i-1,0)) 
            PLERROR("YMDDatedVMatrix: %d-th year = %d < %d-th year= %d",
                  i,(int)years(i,0),i-1,(int)years(i-1,0));
          if (months(i,0)>months(i-1,0)) 
            {
              n_different_months++;
              n_different_days++;
              pos_of_date[int(years(i,0)-first_year)](int(months(i,0)-1),int(days(i,0)-1)) = i;
            }
          else
            {
              if (months(i,0)<months(i-1,0)) 
                PLERROR("YMDDatedVMatrix: %d-th month = %d < %d-th month= %d",
                      i,(int)months(i,0),i-1,(int)months(i-1,0));
              if (days(i,0)>days(i-1,0)) 
                {
                  n_different_days++;
                  pos_of_date[int(years(i,0)-first_year)](int(months(i,0)-1),int(days(i,0)-1)) = i;
                }
              else  if (days(i,0)<days(i-1,0)) 
                PLERROR("YMDDatedVMatrix: %d-th day = %d < %d-th day= %d",
                      i,(int)days(i,0),i-1,(int)days(i-1,0));
            }
        }
    }
  pos_of_ith_year.resize(n_different_years+1);
  pos_of_ith_month.resize(n_different_months+1);
  pos_of_ith_day.resize(n_different_days+1);
  int y=1;
  int m=1;
  int d=1;
  day_of_ith_pos[0]=0;
  for (int i=1;i<years.length();i++)
    {
      if (years(i,0)>years(i-1,0)) 
        {
          pos_of_ith_year[y++]=i;
          pos_of_ith_month[m++]=i;
          pos_of_ith_day[d++]=i;
        }
      else
        {
          if (years(i,0)<years(i-1,0)) 
            PLERROR("YMDDatedVMatrix: %d-th year = %d < %d-th year= %d",
                  i,(int)years(i,0),i-1,(int)years(i-1,0));
          if (months(i,0)>months(i-1,0)) 
            {
              pos_of_ith_month[m++]=i;
              pos_of_ith_day[d++]=i;
            }
          else
            {
              if (months(i,0)<months(i-1,0)) 
                PLERROR("YMDDatedVMatrix: %d-th month = %d < %d-th month= %d",
                      i,(int)months(i,0),i-1,(int)months(i-1,0));
              if (days(i,0)>days(i-1,0)) 
                pos_of_ith_day[d++]=i;
              else  if (days(i,0)<days(i-1,0)) 
                PLERROR("YMDDatedVMatrix: %d-th day = %d < %d-th day= %d",
                      i,(int)days(i,0),i-1,(int)days(i-1,0));
            }
        }
      day_of_ith_pos[i]=d-1;
    }
  pos_of_ith_year[y]=data->length();
  pos_of_ith_month[m]=data->length();
  pos_of_ith_day[d]=data->length();
}

VMat YMDDatedVMatrix::subDistrRelativeYears(int first_relative_year, int n_years)
{
  if (first_relative_year<0 || n_years<0 ||
      first_relative_year+n_years >=pos_of_ith_year.length())
    PLERROR("YMDDatedVMatrix::subDistrRelativeYears(%d,%d) : incorrect arguments",
          first_relative_year, n_years);
  return data.subMatRows(int(pos_of_ith_year[first_relative_year]),
                         int(pos_of_ith_year[first_relative_year+n_years]-
                         pos_of_ith_year[first_relative_year]));
}

VMat YMDDatedVMatrix::subDistrRelativeMonths(int first_relative_month, int n_months)
{
  if (first_relative_month<0 || n_months<0 ||
      first_relative_month+n_months >=pos_of_ith_month.length())
    PLERROR("YMDDatedVMatrix::subDistrRelativeMonths(%d,%d) : incorrect arguments",
          first_relative_month, n_months);
  return data.subMatRows(int(pos_of_ith_month[first_relative_month]),
                         int(pos_of_ith_month[first_relative_month+n_months]-
                             pos_of_ith_month[first_relative_month]));
}

VMat YMDDatedVMatrix::subDistrRelativeDays(int first_relative_day, int n_days)
{
  if (first_relative_day<0 || n_days<0 ||
      first_relative_day+n_days >=pos_of_ith_day.length())
    PLERROR("YMDDatedVMatrix::subDistrRelativeDays(%d,%d) : incorrect arguments",
          first_relative_day, n_days);
  return data.subMatRows(int(pos_of_ith_day[first_relative_day]),
                         int(pos_of_ith_day[first_relative_day+n_days]-
                             pos_of_ith_day[first_relative_day]));
} 

int YMDDatedVMatrix::positionOfDate(int year, int month, int day)
{
  if (year<years(0,0))
    PLERROR("YMDDatedVMatrix::positionOfDate(%d,%d,%d): year %d < first year %d",
          year,month,day,year,(int)years(0,0));
  if (year>years(years.length()-1,0))
    return years.length();
  if (month<1 || month>12)
    PLERROR("YMDDatedVMatrix::positionOfDate(%d,%d,%d): month %d should be in [1,12]",
          year,month,day,month);
  if (day<1 || day>31)
    PLERROR("YMDDatedVMatrix::positionOfDate(%d,%d,%d): day %d should be in [1,31]",
          year,month,day,day);
  return int(pos_of_date[year-int(years(0,0))](month-1,day-1));
}

VMat YMDDatedVMatrix::subDistrAbsoluteYears(int year, int month, int day, int n_years)
{
  int first_pos = positionOfDate(year,month,day);
  int last_pos = positionOfDate(year+n_years,month,day);
  return data.subMatRows(first_pos,last_pos-first_pos);
}

VMat YMDDatedVMatrix::subDistrAbsoluteMonths(int year, int month, int day, int n_months)
{
  int first_pos = positionOfDate(year,month,day);
  int extra_years = (month + n_months) / 12;
  int new_month = (month + n_months) - extra_years*12;
  int last_pos = positionOfDate(year+extra_years,new_month,day);
  return data.subMatRows(first_pos,last_pos-first_pos);
}

VMat YMDDatedVMatrix::subDistrAbsoluteDays(int year, int month, int day, int n_days)
{
  int first_pos = positionOfDate(year,month,day);
  int nthday = (int)day_of_ith_pos[first_pos];
  int last_pos = (int)pos_of_ith_day[nthday+n_days];
  return data.subMatRows(first_pos,last_pos-first_pos);
}

VMat YMDDatedVMatrix::subDistrAbsoluteUnits(int year, int month, int day, int n_units, const string& units)
{
  if (units[0]=='y' || units[0]=='Y')
    return subDistrAbsoluteYears(year,month,day,n_units);
  if (units[0]=='m' || units[0]=='M')
    return subDistrAbsoluteMonths(year,month,day,n_units);
  if (units[0]=='d' || units[0]=='D')
    return subDistrAbsoluteDays(year,month,day,n_units);
  else
    PLERROR("YMDDatedVMatrix::subDistrAbsoluteUnits, unknown units, expected years,months or days!",
          units.c_str());
  Mat m;
  return VMat(m);
}

VMat YMDDatedVMatrix::subDistrRelativeDates(int first, int n, const string& units)
{
  if (units[0]=='y' || units[0]=='Y')
    return subDistrRelativeYears(first,n);
  if (units[0]=='m' || units[0]=='M')
    return subDistrRelativeMonths(first,n);
  if (units[0]=='d' || units[0]=='D')
    return subDistrRelativeDays(first,n);
  else
    PLERROR("YMDDatedVMatrix::subDistrRelativeDates(%d,%d,%s), unknown units, expected years,months or days!",
          first,n,units.c_str());
  return VMat(Mat());
}

int YMDDatedVMatrix::lengthInDates(const string& units)
{
  if (units[0]=='y' || units[0]=='Y')
    return pos_of_ith_year.length()-1;
  if (units[0]=='m' || units[0]=='M')
    return pos_of_ith_month.length()-1;
  if (units[0]=='d' || units[0]=='D')
    return pos_of_ith_day.length()-1;
  else
    PLERROR("YMDDatedVMatrix::lengthInDates(%s), unknown units, expected years,months or days!",
          units.c_str());
  return 0;
}

int YMDDatedVMatrix::positionOfRelativeDate(int first, const string& units)
{
  if (units[0]=='y' || units[0]=='Y')
    return (int)pos_of_ith_year[first];
  if (units[0]=='m' || units[0]=='M')
    return (int)pos_of_ith_month[first];
  if (units[0]=='d' || units[0]=='D')
    return (int)pos_of_ith_day[first];
  else
    PLERROR("YMDDatedVMatrix::positionOfRelativeDate(%s), unknown units, expected years,months or days!",
          units.c_str());
  return 0;
}

void YMDDatedVMatrix::copyDatesOfRows(int from_row, int n_rows, Mat& dates)
{
  dates.resize(n_rows,3);
  for (int i=from_row;i<from_row+n_rows;i++)
    {
      dates(i-from_row,0)=years(i,0);
      dates(i-from_row,1)=months(i,0);
      dates(i-from_row,2)=days(i,0);
    }
}

Vec YMDDatedVMatrix::copyRowDataAndDate(int row, int &year, int &month, int &day)
{
  year = (int)years(row,0);
  month = (int)months(row,0);
  day = (int)days(row,0);
  return data(row);
}
void YMDDatedVMatrix::copyDateOfRow(int row, int &year, int &month, int &day)
{
  year = (int)years(row,0);
  month = (int)months(row,0);
  day = (int)days(row,0);
}

/** SentencesBlocks **/

SentencesBlocks::SentencesBlocks(int n_blocks, VMat d, Vec separator)
  : Array<VMat>(n_blocks)
{
  if (n_blocks==1) 
    {
      (*this)[0]=d;
      return;
    }
  int total_size = d->length();
  if (total_size < n_blocks * 2)
    PLERROR("SentencesBlocks: can't have blocks of size < 2 in average");
  Vec v(d->width());
  int b=0;
  int previous_previous_block=0,previous_beginning_of_block = 0, previous_beginning_of_sentence=0;
  int next_target = (int)(total_size / (real)n_blocks );
  for (int i=0;i<total_size && b<n_blocks-1;i++)
    {
      d->getRow(i,v);
      if (v==separator)
        {
          if (i>=next_target)
            {
              int cut=0;
              if (i-next_target < next_target-previous_beginning_of_sentence ||
                  previous_beginning_of_sentence < previous_beginning_of_block)
                cut=i+1;
              else
                {
                  cut=previous_beginning_of_sentence;
                  previous_beginning_of_sentence = i+1;
                }
              (*this)[b++] = d.subMatRows(previous_beginning_of_block,
                                             cut-previous_beginning_of_block);
              previous_previous_block = previous_beginning_of_block;
              previous_beginning_of_block=cut;
              if (b<n_blocks)
                {
                  if (b>n_blocks-3)
                    next_target = (int)((total_size - cut) / (real)(n_blocks-b));
                  else
                    next_target = (int)(total_size * (real)(b+1.0) / n_blocks);
                }
            }
          else
            previous_beginning_of_sentence=i+1;
        }
    }
  if (b==n_blocks-1)
    (*this)[b++] = d.subMatRows(previous_beginning_of_block,
                                   total_size-previous_beginning_of_block);
  if (b<n_blocks-1) // we have to backtrack, split previous block in two
    {
      if (b<n_blocks-2)
        PLERROR("SentencesBlocks: blocks are too small!");
      if (previous_beginning_of_sentence<previous_beginning_of_block)
        PLERROR("SentencesBlocks: Blocks are too small!");
      int cut = previous_beginning_of_sentence;
      (*this)[b++] = d.subMatRows(previous_beginning_of_block,
                                     cut-previous_beginning_of_block);
      previous_beginning_of_block=cut;
      (*this)[b++] = d.subMatRows(previous_beginning_of_block,
                                     total_size-previous_beginning_of_block);
    }
}

template <>
void deepCopyField(VMat& field, CopiesMap& copies)
{
  if (field)
    field = static_cast<VMatrix*>(field->deepCopy(copies));
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

VMat loadAsciiAsVMat(const string& filename)
{
  Mat m;
  TVec<string> fn;
  loadAscii(filename,m,fn);
  VMat vm= new MemoryVMatrix(m);
  for(int i=0;i<fn.size();i++)
    vm->declareField(i, fn[i]);
  return vm;
}

%> // end of namespace PLearn
