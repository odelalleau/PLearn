// -*- C++ -*-

// GaussianProcessRegressor.cc
//
// Copyright (C) 2003 Yoshua Bengio
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
   * $Id: GaussianProcessRegressor.cc,v 1.11 2004/07/21 16:30:55 chrish42 Exp $
   ******************************************************* */

#include "GaussianProcessRegressor.h"
#include <plearn/math/pl_erf.h>
#include <plearn/math/plapack.h>

namespace PLearn {
using namespace std;

GaussianProcessRegressor::GaussianProcessRegressor() : 
  inherited(), Gram_matrix_normalization("none"), 
  max_nb_evectors(-1)
{}

PLEARN_IMPLEMENT_OBJECT(GaussianProcessRegressor, "Basic version of Gaussian Process regression.", "NO HELP");

void GaussianProcessRegressor::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
  deepCopyField(kernel, copies);
  deepCopyField(noise_sd, copies);
  deepCopyField(alpha, copies);
  deepCopyField(Kxxi, copies);
  deepCopyField(Kxx, copies);
  deepCopyField(K, copies);
  deepCopyField(eigenvectors, copies);
  deepCopyField(eigenvalues, copies);
  deepCopyField(meanK, copies);
}

void GaussianProcessRegressor::setInput(const Vec& input)
{
  setInput_const(input);
}
void GaussianProcessRegressor::setInput_const(const Vec& input) const
{
  // compute K(x,x_i)
  for (int i=0;i<Kxxi.length();i++)
    Kxxi[i]=kernel->evaluate_x_i(input,i);
  // compute K(x,x)
  Kxx = kernel->evaluate(input,input);
  // apply normalization
  if (Gram_matrix_normalization=="centering_a_dotproduct")
  {
    real kmean = mean(Kxxi);
    for (int i=0;i<Kxxi.length();i++)
      Kxxi[i] = Kxxi[i] - kmean - meanK[i] + mean_allK;
    Kxx = Kxx - kmean - kmean + mean_allK;
  } else if (Gram_matrix_normalization=="centering_a_distance")
  {
    real kmean = mean(Kxxi);
    for (int i=0;i<Kxxi.length();i++)
      Kxxi[i] = -0.5*(Kxxi[i] - kmean - meanK[i] + mean_allK);
    Kxx = -0.5*(Kxx - kmean - kmean + mean_allK);
  }
  else if (Gram_matrix_normalization=="divisive")
  {
    real kmean = mean(Kxxi);
    for (int i=0;i<Kxxi.length();i++)
      Kxxi[i] = Kxxi[i]/sqrt(kmean* meanK[i]);
    Kxx = Kxx/kmean;
  }
}


void GaussianProcessRegressor::declareOptions(OptionList& ol)
{
  declareOption(ol, "kernel", &GaussianProcessRegressor::kernel, OptionBase::buildoption, 
                "The kernel (seen as a symmetric, two-argument function of a pair of input points)\n"
                "that corresponds to the prior covariance on the function to be learned.\n");

  declareOption(ol, "noise_sd", &GaussianProcessRegressor::noise_sd, OptionBase::buildoption,
                "Output noise std. dev. (one element per output).\n");


  declareOption(ol, "max_nb_evectors", &GaussianProcessRegressor::max_nb_evectors, OptionBase::buildoption,
                "Maximum number of eigenvectors of the Gram matrix to compute (or -1 if all should be computed).\n");


  declareOption(ol, "Gram_matrix_normalization", &GaussianProcessRegressor::Gram_matrix_normalization, 
                OptionBase::buildoption,
                "normalization method to apply to Gram matrix. Expected values are:\n"
                "\"none\": no normalization\n"
                "\"centering_a_dot_product\": this is the kernel PCA centering\n"
                "     K_{ij} <-- K_{ij} - mean_i(K_ij) - mean_j(K_ij) + mean_{ij}(K_ij)\n"
                "\"centering_a_distance\": this is the MDS transformation of squared distances to dot products\n"
                "     K_{ij} <-- -0.5(K_{ij} - mean_i(K_ij) - mean_j(K_ij) + mean_{ij}(K_ij))\n"
                "\"divisive\": this is the spectral clustering and Laplacian eigenmaps normalization\n"
                "     K_{ij} <-- K_{ij}/sqrt(mean_i(K_ij) mean_j(K_ij))\n");


  inherited::declareOptions(ol);
}

void GaussianProcessRegressor::build_()
{
  if(expdir!="")
    {
      if(!force_mkdir(expdir))
        PLERROR("In GaussianProcessRegressor Could not create experiment directory %s",expdir.c_str());
      expdir = abspath(expdir);
    }
  
  if (train_set)
  {
    K.resize(train_set->length(),train_set->length());
    Kxxi.resize(train_set->length());
    alpha.resize(outputsize(),train_set->length());
    meanK.resize(train_set->length());
    n_outputs = train_set->targetsize();
  }
}

int GaussianProcessRegressor::outputsize() const
{ 
  int output_size=0;
  if (outputs_def.find("e") != string::npos)
    output_size+=n_outputs;
  if (outputs_def.find("v") != string::npos)
    // we only compute a diagonal output variance
    output_size+=n_outputs;
  return output_size;
}

void GaussianProcessRegressor::build()
{
  inherited::build();
  build_();
}

void GaussianProcessRegressor::forget()
{
  stage = 0;
}

GaussianProcessRegressor::~GaussianProcessRegressor()
{
}

TVec<string> GaussianProcessRegressor::getTrainCostNames() const
{
  TVec<string> names(2);
  names[0]="log-likelihood";
  names[1]="mse";
  return names;
}

TVec<string> GaussianProcessRegressor::getTestCostNames() const
{ return getTrainCostNames(); }

int GaussianProcessRegressor::getTestCostIndex(const string& costname) const
{
  TVec<string> costnames = getTestCostNames();
  for(int i=0; i<costnames.length(); i++)
    if(costnames[i]==costname)
      return i;
  return -1;
}

int GaussianProcessRegressor::getTrainCostIndex(const string& costname) const
{
  TVec<string> costnames = getTrainCostNames();
  for(int i=0; i<costnames.length(); i++)
    if(costnames[i]==costname)
      return i;
  return -1;
}

//! return log of probability density log(p(x))
double GaussianProcessRegressor::log_density(const Vec& y) const
{
  PLERROR("GaussianProcessRegressor::log_density not implemented yet");
  return 0;
}

//! return E[X] 
void GaussianProcessRegressor::expectation(Vec expected_y) const
{
  for (int i=0;i<n_outputs;i++)
    expected_y[i] = dot(Kxxi,alpha(i));
}

//! return E[X] 
Vec GaussianProcessRegressor::expectation() const
{
  static Vec expected_target;
  expected_target.resize(n_outputs);
  expectation(expected_target);
  return expected_target;
}

void GaussianProcessRegressor::variance(Vec diag_variances) const
{
  for (int i=0;i<n_outputs;i++)
  {
    real v = Kxx;
    v -= QFormInverse(noise_sd[i]*noise_sd[i],Kxxi);
    diag_variances[i] = v;
  }
}

//! return Var[X]
Mat GaussianProcessRegressor::variance() const
{
  static Mat var;
  if (var.length()!=n_outputs)
  {
    var.resize(n_outputs,n_outputs);
    var.clear();
  }
  for (int i=0;i<n_outputs;i++)
  {
    real v = Kxx;
    v -= QFormInverse(noise_sd[i]*noise_sd[i],Kxxi);
    var(i,i) = v;
  }
  return var;
}
                                
void GaussianProcessRegressor::computeOutput(const Vec& input, Vec& output) const
{
  setInput_const(input);
  int i0=0;
  if (outputs_def.find("e") != string::npos)
  {
    expectation(output.subVec(i0,n_outputs));
    i0+=n_outputs;
  }
  if (outputs_def.find("v") != string::npos)
  {
    variance(output.subVec(i0,n_outputs));
    i0+=n_outputs;
  }
}

// prediction = E[E[y|x]|training_set] = E[y|x,training_set]
// prediction[j] = sum_i alpha_{ji} K(x,x_i)
//               = (K(x,x_i))_i' inv(K+sigma^2[j] I) targets
//
// Var[y[j]|x,training_set] = Var[E[y[j]|x]|training_set] + E[Var[y[j]|x]|training_set]
//  where
//  Var[E[y[j]|x]|training_set] = K(x,x)- (K(x,x_i))_i' inv(K+sigma^2[j]) (K(x,x_i))_i
//  and
//  E[Var[y[j]|x]|training_set] = Var[y[j]|x] = sigma^2[j] = noise
//
// costs:
//   MSE = sum_j (y[j] - prediction[j])^2
//   NLL = sum_j log Normal(y[j];prediction[j],Var[y[j]|x,training_set])
//
void GaussianProcessRegressor::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                       const Vec& target, Vec& costs) const
{
  Vec mu;
  static Vec var;
  int i0=0;
  if (outputs_def.find("e")!=string::npos)
  {
    mu = output.subVec(i0,n_outputs);
    i0+=n_outputs;
  }
  else
    mu = expectation();
  if (outputs_def.find("v")!=string::npos)
  {
    var = output.subVec(i0,n_outputs);
    i0+=n_outputs;
  }
  else
  {
    var.resize(n_outputs);
    variance(var);
  }
  real mse = 0;
  real logdensity = 0;
  for (int i=0;i<n_outputs;i++)
  {
    real diff=mu[i] - target[i];
    mse += diff*diff;
    logdensity += gauss_log_density_var(target[i],mu[i],var[i]+noise_sd[i]*noise_sd[i]);
  }
  costs[0]=mse;
  costs[1]=logdensity;
}

void GaussianProcessRegressor::computeOutputAndCosts(const Vec& input, const Vec& target, 
                                                     Vec& output, Vec& costs) const
{
  computeOutput(input, output);
  computeCostsFromOutputs(input, output, target, costs);
}

void GaussianProcessRegressor::computeCostsOnly(const Vec& input, const Vec& target,  
                                Vec& costs) const
{
  static Vec tmp_output;
  tmp_output.resize(outputsize());
  computeOutputAndCosts(input, target, tmp_output, costs);
}

void GaussianProcessRegressor::train()
{
  // compute Gram matrix K
  int l=K.length();
  VMat input_rows = train_set.subMatColumns(0,inputsize());
  VMat target_rows = train_set.subMatColumns(inputsize(),targetsize());
  kernel->setDataForKernelMatrix(input_rows);
  kernel->computeGramMatrix(K);

  // SHOULD WE ADD THE NOISE VARIANCE BEFORE NORMALIZATION?

  // optionally "normalize" the gram matrix
  if (Gram_matrix_normalization=="centering_a_dotproduct")
  {
    columnMean(K,meanK);
    mean_allK = mean(meanK);
    int m=K.mod();
    real mean_allK = mean(meanK);
    for (int i=0;i<l;i++)
    {
      real* Ki = K[i];
      real* Kji_ = &K[0][i];
      for (int j=0;j<=i;j++,Kji_+=m)
      {
        real Kij = Ki[j] - meanK[i] - meanK[j] + mean_allK;
        Ki[j]=Kij;
        if (j<i)
          *Kji_ =Kij;
      }
    }
  }
  else if (Gram_matrix_normalization=="centering_a_distance")
  {
    columnMean(K,meanK);
    mean_allK = mean(meanK);
    int m=K.mod();
    real mean_allK = mean(meanK);
    for (int i=0;i<l;i++)
    {
      real* Ki = K[i];
      real* Kji_ = &K[0][i];
      for (int j=0;j<=i;j++,Kji_+=m)
      {
        real Kij = -0.5*(Ki[j] - meanK[i] - meanK[j] + mean_allK);
        Ki[j]=Kij;
        if (j<i)
          *Kji_ =Kij;
      }
    }
  }
  else if (Gram_matrix_normalization=="divisive")
  {
    columnMean(K,meanK);
    int m=K.mod();
    for (int i=0;i<l;i++)
    {
      real* Ki = K[i];
      real* Kji_ = &K[0][i];
      for (int j=0;j<=i;j++,Kji_+=m)
      {
        real Kij = Ki[j] / sqrt(meanK[i]*meanK[j]);
        Ki[j]=Kij;
        if (j<i)
          *Kji_ =Kij;
      }
    }
  }
  // compute principal eigenvectors
  int n_components = max_nb_evectors<0 || max_nb_evectors>l ? l : max_nb_evectors;
  eigenVecOfSymmMat(K,n_components,eigenvalues,eigenvectors);
  // pre-compute alpha[i]=(K+noise_sd[i]^2 I)^{-1}*targets  for regression
  for (int i=0;i<n_outputs;i++)
  {
    VMat target_column = target_rows.subMatColumns(i,1);
    inverseCovTimesVec(noise_sd[i]*noise_sd[i],target_column.toMat().toVec(),alpha(i));
  }

}

real GaussianProcessRegressor::BayesianCost()
{
  //! compute the "training cost" = negative log-likelihood of the training data
  //!  = 0.5*sum_i (log det(K+sigma[i]^2 I) + y' inv(K+sigma[i]^2 I) y + l log(2 pi))
  int l=K.length();
  int m=eigenvectors.length();
  real nll = l*n_outputs*Log2Pi;
  for (int i=0;i<n_outputs;i++)
  {
    real sigma2_i=noise_sd[i]*noise_sd[i];
    //nll += QFormInverse(sigma2_i,targets); // y'*inv(C)*y 
    // add the log det(K+sigma_i^2 I) contribution
    if (m<l)
      // the last l-m eigenvalues are sigma_i^2
      nll += (l-m)*safeflog(sigma2_i); 
    // while the first m ones are lambda_i + sigma_i^2
    for (int j=0;j<m;j++)
      nll += safeflog(eigenvalues[i]+sigma2_i);
  }
  nll *= 0.5;
  return nll;
}

// multiply (K+sigma^2 I)^{-1} by vector v, put result in Cinv_v
// TRICK USING PRINCIPAL E-VECTORS OF K:
//   Let C = sum_{i=1}^m lambda_i v_i v_i' + sigma^2 I
//   with v_i orthonormal eigenvectors. Then, it can also be written
//       C = sum_{i=1}^m (lambda_i +sigma^2) v_i v_i' + sum_{i=m+1}^n sigma^2 v_i v_i'
//   whose inverse is simply
//       inverse(C) = sum_{i=1}^m 1/(lambda_i +sigma^2) v_i v_i' + sum_{i=m+1}^n 1/sigma^2 v_i v_i'
//                  = sum_{i=1}^m (1/(lambda_i +sigma^2) - 1/sigma^2) v_i v_i' + 1/sigma^2 I
//   so 
//    inverse(C) * u = u/sigma + sum_{i=1}^m (1/(lambda_i+sigma^2) - 1/sigma^2) v_i v_i.u
void GaussianProcessRegressor::inverseCovTimesVec(real sigma2, Vec u, Vec Cinv_u) const
{
  int m=eigenvectors.length();
  real one_over_sigma2 = 1.0/sigma2;
  multiply(u,one_over_sigma2,Cinv_u);
  for (int i=0;i<m;i++)
  {
    Vec v_i = eigenvectors(i);
    real proj = dot(v_i,u);
    multiplyAdd(Cinv_u, v_i, proj*(1.0/(eigenvalues[i]+sigma2)-one_over_sigma2), Cinv_u);
  }
}

real GaussianProcessRegressor::QFormInverse(real sigma2, Vec u) const
{
  int m=eigenvectors.length();
  real one_over_sigma2 = 1.0/sigma2;
  real qf = norm(u)*one_over_sigma2;
  for (int i=0;i<m;i++)
  {
    Vec v_i = eigenvectors(i);
    real proj = dot(v_i,u);
    qf += (1.0/(eigenvalues[i]+sigma2)-one_over_sigma2) * proj*proj;
  }
  return qf;
}


} // end of namespace PLearn

