// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: GaussianDistribution.cc,v 1.1 2002/10/22 05:00:19 plearner Exp $
   * AUTHORS: Pascal Vincent and Yoshua Bengio
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

real logOfCompactGaussian(const Vec& x, const Vec& mu, 
                          const Vec& eigenvalues, const Mat& eigenvectors, real gamma)
{
  // cerr << "logOfCompact: mu = " << mu << endl;

  int d = x.length();
  static Vec x_minus_mu;
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
    
  double log_det = 0.;
  double inv_gamma = 1./gamma;
  int kk = eigenvalues.length();
  double q = inv_gamma * sqnorm_xmu;
  int i;
  for(i=0; i<kk; i++)
    {
      double lambda_i = eigenvalues[i];
      if(lambda_i==0) // we've reached a 0 eigenvalue, stop computation here.
        break;
      log_det += log(lambda_i);
      q += (1./lambda_i - inv_gamma)*square(dot(eigenvectors(i),x_minus_mu));
    }
  if(kk<d)
    log_det += log(gamma)*(d-i);

  double logp = -0.5*( d*log(2*M_PI) + log_det + q);
  // cerr << "logOfCompactGaussian q=" << q << " log_det=" << log_det << " logp=" << logp << endl;
  // exit(0);
  return real(logp);
}

real logOfNormal(const Vec& x, const Vec& mu, const Mat& C)
{
  int n = x.length();
  static Vec x_mu;
  static Vec z;
  static Vec y;
  y.resize(n);
  z.resize(n);
  x_mu.resize(n);
  substract(x,mu,x_mu);

  static Mat L;
  // Perform Cholesky decomposition
  choleskyDecomposition(C, L);

  // get the log of the determinant: 
  // det(C) = square(product_i L_ii)
  double logdet = 0;
  for(int i=0; i<n; i++)
    logdet += log(L(i,i));
  logdet += logdet;

  // Finally find z, such that C.z = x-mu
  choleskySolve(L, x_mu, z, y);

  double q = dot(x_mu, z);
  double logp = -0.5*( n*log(2*M_PI) + logdet + q);
  // cerr << "logOfNormal q=" << q << " logdet=" << logdet << " logp=" << logp << endl;
  return real(logp);
}

real logPFittedGaussian(const Vec& x, const Mat& X, real lambda)
{
  static Mat C;
  static Vec mu;
  computeMeanAndCovar(X, mu, C);
  addToDiagonal(C, lambda);
  return logOfNormal(x, mu, C);
}

IMPLEMENT_NAME_AND_DEEPCOPY(GaussianDistribution);

void GaussianDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(mu, copies);
  deepCopyField(inv_lambda, copies);
  deepCopyField(V, copies);
}


Vec GaussianDistribution::x_minus_mu;

GaussianDistribution::GaussianDistribution(int the_inputsize)
  :Learner(the_inputsize,0,1), use_svd(true), k(1000), sigma(0), min_eigenval_ratio(0), min_eigenval(1e-5)
{
  setTestCostFunctions(neg_output_costfunc());
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
  /*
  declareOption(ol, "min_eigenval_ratio", &GaussianDistribution::min_eigenval_ratio, OptionBase::buildoption, 
                "only eigenvalues larger than min_eigenval_ratio*largest_eigenval will be kept");
  declareOption(ol, "min_eigenval", &GaussianDistribution::min_eigenval, OptionBase::buildoption, 
                "keep only eigenvalues above this value");
  */

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

  // these are used in SVD
  static Mat trainmat;
  static Mat U;

  // The maximum number of eigenvalues we want.
  int maxneigval = min(k+1, min(l,d));

  if(use_svd) // use svd
    {
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
      computeMeanAndCovar(training_set, mu, covarmat, vlog.rawout());

      /*
      string cachedir;
      string metadatadir = training_set->getMetaDataDir();
      if(!metadatadir.empty())
        {
          cachedir = metadatadir + "MODELCACHE/GaussianDistribution/";
          if(!force_mkdir(cachedir))
            {
              PLWARNING("Could not create directory %s",cachedir.c_str());
              cachedir = "";
            }
        }
      
      string covarfile;
      if(!cachedir.empty())
        covarfile = cachedir+"covar.cache";

      if(isfile(covarfile))
        {
          ifstream in(covarfile.c_str());
          if(!in)
            PLERROR("Could not open file %s for reading",covarfile.c_str());
          vlog << "(reading cached mean and covariance from file " << covarfile << ")" << endl;
          PLearn::binread(in,mu);
          PLearn::binread(in,covarmat);
        }
      else
        {
          computeMeanAndCovar(training_set, mu, covarmat, vlog);
          if(!covarfile.empty())
            {
              ofstream out(covarfile.c_str());
              if(!out)
                PLWARNING("Could not open file %s for writing",covarfile.c_str());
              else
                {
                  vlog << "(writing mean and covariance to cache file " << covarfile << ")" << endl;
                  PLearn::binwrite(out,mu);
                  PLearn::binwrite(out,covarmat);
                }
            }
        }
      */
      
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

double GaussianDistribution::log_p(const Vec& x) const 
{ 
  double q = compute_q(x);
  double logp = -logcoef - 0.5*q;
  // cerr << "GaussianDistribution q=" << q << " logcoef=" << logcoef << " logp=" << logp << endl;
  return logp;
}

void GaussianDistribution::use(const Vec& input, Vec& output)
{
#ifdef BOUNDCHECK
  if(input.length()!=inputsize() || output.length()!=1)
    PLERROR("In GaussianDistribution::use input.length() should be %d (currently %d) and output.length() should be 1 (currently %d)",inputsize(),input.length(),output.length());
#endif
  output[0] = log_p(input);
}
/*
void GaussianDistribution::write(ostream& out) const
{ PLearn::write(out, *this); }
*/

%> // end of namespace PLearn
