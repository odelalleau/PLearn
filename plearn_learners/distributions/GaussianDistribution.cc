// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Pascal Vincent
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
   * $Id: GaussianDistribution.cc,v 1.2 2002/10/22 08:46:07 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnAlgo/GaussianDistribution.cc */

#include "GaussianDistribution.h"
#include "fileutils.h"
#include "plapack.h"
#include "VMat_maths.h"

namespace PLearn <%
using namespace std;

#define ZEROGAMMA

IMPLEMENT_NAME_AND_DEEPCOPY(GaussianDistribution);

void GaussianDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(mu, copies);
  deepCopyField(inv_lambda, copies);
  deepCopyField(V, copies);
}


Vec GaussianDistribution::x_minus_mu;

GaussianDistribution::GaussianDistribution()
  :use_svd(false), k(1000), sigma(0), ignore_weights_below(0)
{
}


void GaussianDistribution::declareOptions(OptionList& ol)
{
  // Build options
  declareOption(ol, "use_svd", &GaussianDistribution::use_svd, OptionBase::buildoption, 
                "If true, use SVD, if false, use eigendecomposition of covariance matrix.");
  declareOption(ol, "k", &GaussianDistribution::k, OptionBase::buildoption, 
                "number of eigenvectors to keep");
  declareOption(ol, "sigma", &GaussianDistribution::sigma, OptionBase::buildoption, 
                "what to add to diagonal of covariance matrices");
  declareOption(ol, "ignore_weights_below", &GaussianDistribution::ignore_weights_below, OptionBase::buildoption, 
                "When doing a weighted fitting (weightsize==1), points with a weight below this value will be ignored");

  // Learnt parameters
  declareOption(ol, "mu", &GaussianDistribution::mu, OptionBase::learntoption, "");
  declareOption(ol, "inv_lambda", &GaussianDistribution::inv_lambda, OptionBase::learntoption, "");
  declareOption(ol, "inv_gamma", &GaussianDistribution::inv_gamma, OptionBase::learntoption, "");
  declareOption(ol, "V", &GaussianDistribution::V, OptionBase::learntoption, "");
  declareOption(ol, "logcoef", &GaussianDistribution::logcoef, OptionBase::learntoption, "");

  inherited::declareOptions(ol);
}                


void GaussianDistribution::train(VMat training_set)
{
  int l = training_set.length();
  int d = training_set.width();

  if(d!=inputsize()+weightsize())
    PLERROR("In GaussianDistribution::train width of training_set should be equal to inputsize()+weightsize()");

  // these are used in SVD
  static Mat trainmat;
  static Mat U;

  // The maximum number of eigenvalues we want.
  int maxneigval = min(k+1, min(l,d));

  if(use_svd) // use svd
    {
      if(weightsize()!=0)
        PLERROR("In GaussianDistribution::train use_svd not currently supported with weightsize>0");

      trainmat.resize(training_set.length(), training_set.width());
      trainmat << training_set;
      mu.resize(trainmat.width());
      columnMean(trainmat, mu);
      trainmat -= mu;
      
      SVD(trainmat, U, inv_lambda, V, 'S'); // singular values will be put in inv_lambda in decreasing order      
      // eigenvalues = square(singularvalues)
      squareElements(inv_lambda);
      // now we have the eigenvalues of (trainmat-mu)'(trainmat-mu), 
      // but we want those of (trainmat-mu)'(trainmat-mu)/l
      inv_lambda /= l;

      // add regularization
      inv_lambda += sigma;

      // cerr << "mu: " << mu << endl;
      // cerr << "svd lambda: " << inv_lambda << endl; 
      // cerr << "svd V: \n" << V << endl;
      
      if(inv_lambda.length()>maxneigval)
        inv_lambda.resize(maxneigval);
    }

  else  // use eigenvectors of covariance matrix
    {
      // First get mean and covariance
      // (declared static to avoid repeated dynamic memory allocation)
      static Mat covarmat;

      if(weightsize()==0)
        computeMeanAndCovar(training_set, mu, covarmat, vlog.rawout());
      else if(weightsize()==1)
        computeWeightedMeanAndCovar(training_set, mu, covarmat, ignore_weights_below);
      else
        PLERROR("In GaussianDistribution, weightsize() can only be 0 or 1");
      
      // Possibly, add sigma to the diagonal
      if(sigma!=0)
        addToDiagonal(covarmat, sigma);
      
      // if(k>l || k>d)
      //   PLERROR("In GaussianDistribution::train the number k=%d of eigenvectors you are attempting to get is greater than the smallest dimension of the data matrix %dx%d",k,l,d);

      // cerr << "maxneigval: " << maxneigval << " ";
      eigenVecOfSymmMat(covarmat, maxneigval, inv_lambda, V);
      // cerr << inv_lambda.length() << endl;

      // cerr << "eig lambda: " << inv_lambda << endl; 
      // cerr << "eig V: \n" << V << endl;
    }
  
  // ------------------
  // 
  
  /* 
     // Keep only eigenvalues that are above min_eigenval, and above min_eigenval_ratio*largest_eigenval
  int keep_n;
  real minval = max(min_eigenval, min_eigenval_ratio*inv_lambda[0]);
  for(keep_n=0; keep_n<inv_lambda.length(); keep_n++)
    if(inv_lambda[keep_n]<minval)
      break;
  inv_lambda.resize(keep_n);
  V.resize(keep_n,eigen_vectors.width());
  */

  int n_eval_found = inv_lambda.length();
  // compute determinant
  double log_det = sum_of_log(inv_lambda) + log(inv_lambda.back())*(d-n_eval_found);
  invertElements(inv_lambda);
  if(n_eval_found>k) // We got the k+1 eigenvalues we asked for
    { // but we want to keep only k eigen vectors
      inv_gamma = inv_lambda[k];
      inv_lambda.resize(k);
      V.resize(k,d);
    }
  else // We got less than k+1 eigenvalues, let's use the last one as gamma
    {
      inv_gamma = inv_lambda.back();
    }

#ifdef ZEROGAMMA
  inv_gamma = 1/sigma;
#endif

  // cerr << "gamma=" << 1./inv_gamma << endl;

  /*
    for(int j=0; j<inv_lambda.size(); j++)
    if(inv_lambda[j]<0)
      PLERROR("Problem in GaussianDistribution: eigenval#%d == %f",j,1./inv_lambda[j]);
  */

  // Compute the logcoef
  // 0.5( d*log(2PI) + log(det(D)) )
  logcoef = 0.5*( d*log(2*M_PI) + log_det );
  // cerr << "GaussianDistribution logdet=" << log_det << " ";

  vlog << "(retained " << inv_lambda.size() << " eigenvalues; largest=" << 1/inv_lambda.front() 
       << " smallest=" << 1/inv_lambda.back() << "   log of normalization = " << logcoef << " )" << endl;

  // measure(0,Vec(1));
}


double GaussianDistribution::compute_q(const Vec& x) const
{
  int d = x.length();
  x_minus_mu.resize(d);
  real* px = x.data();
  real* pmu = mu.data();
  real* pxmu = x_minus_mu.data();
    
  real sqnorm_xmu = 0;
  for(int i=0; i<d; i++)
    {
      real val = *px++ - *pmu++;
      sqnorm_xmu += val*val;
      *pxmu++ = val;
    }
    
  int kk = inv_lambda.length();
  real* pilambda = inv_lambda.data();
  double q = inv_gamma * sqnorm_xmu;
  for(int i=0; i<kk; i++)
    q += (*pilambda++ - inv_gamma)*square(dot(V(i),x_minus_mu));
  return q;
}  

double GaussianDistribution::log_density(const Vec& x) const 
{ 
  double q = compute_q(x);
  double logp = -logcoef - 0.5*q;
  // cerr << "GaussianDistribution q=" << q << " logcoef=" << logcoef << " logp=" << logp << endl;
  return logp;
}


%> // end of namespace PLearn
