// -*- C++ -*-

// GaussMix.cc
// 
// Copyright (C) 2003 Julien Keable
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
 * $Id: GaussMix.cc,v 1.1 2003/06/02 20:59:54 jkeable Exp $ 
 ******************************************************* */

/*! \file GaussMix.cc */
#include "GaussMix.h"
#include "random.h"
#include "plapack.h"

namespace PLearn <%
using namespace std;

GaussMix::GaussMix() 
  :Distribution(), type(Spherical)
/* ### Initialise all other fields here */
{
  // ### Possibly call setTestCostFunctions(...) to define the cost functions 
  // ### you are interested in (these are used by the default useAndCost() method,
  // ### which is called by the default test() method).
  // ### ex: 
  // setTestCostFunctions(squared_error());

  // ### You may also call setTestStatistics(...) if the Learner-default 'mean' and 'stderr' 
  // ### statistics are not appropriate...

  L=D=0;
  avg_K=0;
  // ### You may or may not want to call build_() to finish building the object
  build_();

  // ### You may want to set the Distribution::use_returns_what parameter
}


IMPLEMENT_NAME_AND_DEEPCOPY(GaussMix);

void GaussMix::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // ### ex:
  // declareOption(ol, "myoption", &GaussMix::myoption, OptionBase::buildoption,
  //               "Help text describing this option");
  // ...

   
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}


string GaussMix::help()
{
  // ### Provide some useful description of what the class is ...
  return 
    "GaussMix implements a mixture of gaussians. That's about it for now." 
    + optionHelp();
}

void GaussMix::setMixtureTypeSherical(int _L, int _D)
{
  L=_L;
  D=_D;
  avg_K=0;
  type=Spherical;
  build_();
}

void GaussMix::setMixtureTypeElliptic(int _L, int _D)
{
  L=_L;
  D=_D;
  avg_K=0;
  type=Elliptic;
  build_();
}

void GaussMix::setMixtureTypeGeneral(int _L, int _avg_K, int _D)
{
  L=_L;
  D=_D;
  avg_K=_avg_K;
  type=General;
  build_();
}

void GaussMix::setMixtureTypeFactor(int _L, int _avg_K, int _D)
{
  L=_L;
  D=_D;
  avg_K=_avg_K;
  type=Factor;
  build_();
}

// spherical
void GaussMix::setGaussian(int l, real _alpha, Vec _mu, real _sigma)
{
  if(type!=Spherical)
    PLERROR("GaussMix::setGaussian : type is not Spherical");

  alpha[l]=_alpha;
  mu(l)<<_mu;
  sigma[l]=_sigma;

  log_coef[l]=log(1/sqrt(pow(2*3.14159,D) * pow(_sigma,D)));
}

// elliptic
void GaussMix::setGaussian(int l, real _alpha, Vec _mu, Vec diag)
{
  if(type!=Elliptic)
    PLERROR("GaussMix::setGaussian : type is not Elliptic");

  alpha[l]=_alpha;
  mu(l)<<_mu;
  diags(l)<<diag;

  log_coef[l]=log(1/sqrt(pow(2*3.14159,D) * product(diag)));
}

//general
void GaussMix::setGaussian(int l, real _alpha, Vec _mu, Vec _lambda, Mat eigenvecs, real lambda0 )
{
  int num_lambda = _lambda.length();

  if(num_lambda==0)
    PLERROR("GaussMix::setGaussian : need at least 1 eigenvector");
  if(type!=General)
    PLERROR("GaussMix::setGaussian : type is not General");

  alpha[l]=_alpha;
  mu(l)<<_mu;
  sigma[l]=lambda0;

  if(Ks[l]==0)
  {
    // things to do only on the first time this gaussian is set
    V_idx[l]=sum_Ks;
    Ks[l]=num_lambda;
    sum_Ks+=Ks[l];

    if(V.length()< V_idx[l] + Ks[l])
    {
      int newl = MAX(V_idx[l] + Ks[l], (int)ceil((double)V.length()*1.5));
      V.resize(newl,V.width());
      inv_lambda_minus_lambda0.resize(newl);
    }
  }
  else if(Ks[l]!=num_lambda)
    PLERROR("GaussMix::setGaussian : for the same gaussian, the amount of vectors (length of the 'eigenvecs' matrix) that "\
            "define a gaussian must not change between calls. ");
  
  V.subMatRows(V_idx[l],Ks[l])<<eigenvecs;
  for(int i=0;i<Ks[l];i++)
    inv_lambda_minus_lambda0[V_idx[l] + i] = 1.0/MAX(0,_lambda[i] - lambda0);
  

  // compute determinant
  double log_det = sum_of_log(_lambda);

  if(D - num_lambda > 0)
    log_det+= log(lambda0)*(D-num_lambda);

  log_coef[l] = -0.5*( D*log(2*3.141549) + log_det );
}

//factor
void GaussMix::setGaussian(int l, real _alpha, Vec _mu, Mat vecs, Vec diag )
{
  if(vecs.length()==0)
    PLERROR("GaussMix::setGaussian : need at least 1 eigenvector");
  if(type!=Factor)
    PLERROR("GaussMix::setGaussian : type is not General");
  alpha[l]=_alpha;
  mu(l)<<_mu;
  diags(l)<<diag ;

  // things to do only on the first time

  if(Ks[l]==0)
  {
    V_idx[l]=sum_Ks;
    Ks[l]=vecs.length();
    sum_Ks+=Ks[l];

    if(V.length()< V_idx[l] + Ks[l])
    {
      int newl = (int)ceil((double)V.length()*1.5);
      V.resize(newl,V.width());
    }
    
    if(ivtdv.length()< ivtdv_idx[l] + Ks[l]*Ks[l])
    {
      int newl = MAX(ivtdv_idx[l] + Ks[l]*Ks[l], (int)ceil((double)ivtdv.length()*1.5));
      ivtdv.resize(newl);
    }
  }
  else if(Ks[l]!=vecs.length())
    PLERROR("GaussMix::setGaussian : for the same gaussian, the amount of vectors (length of the 'eigenvecs' matrix) that "\
            "define a gaussian must not change between calls. ");
  
  V.subMatRows(V_idx[l],Ks[l])<<transpose(vecs);
  
  // compute I + V(t) * psi * V

  Mat i_plus_vt_ipsi_v( Ks[l], Ks[l], 0.0 );
  
  for(int i=0;i<Ks[l];i++)
  {
    Vec vi(vecs(i));
    Vec mati(i_plus_vt_ipsi_v(i));
    for(int j=0;j<Ks[l];j++)
    {
      Vec vj(vecs(j));
      for(int k=0;k < D;k++)
        mati[j]+=vi[k] * diag[k] * vj[k];
    }
  }
  addToDiagonal(i_plus_vt_ipsi_v,1.0);

  real determinant = det(i_plus_vt_ipsi_v);
  
  log_coef = -D/2*log(2*3.14159) - 0.5 * log(determinant) - 0.5 * log(product(diag));

  Mat eigenvec(Ks[l],Ks[l]);
  Vec eigenval(Ks[l]);
  
  eigenVecOfSymmMat(i_plus_vt_ipsi_v, Ks[l], eigenval, eigenvec);

  Mat inv(Ks[l],Ks[l]);
  for(int i=0;i<Ks[l];i++)
  {
    Vec vi(eigenvec(i));
    Vec mati(inv(i));
    for(int j=0;j<Ks[l];j++)
    {
      Vec vj(eigenvec(j));
      for(int k=0;k < Ks[l];k++)
        mati[j]+=vi[k] * eigenval[k] * vj[k];
    }
  }

  if(l>0)
    ivtdv_idx[l] = ivtdv_idx[l-1] + Ks[l]*Ks[l];
  
  ivtdv.subVec(ivtdv_idx[l], Ks[l]*Ks[l]) << inv.toVec();
  
}

void GaussMix::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.

  sum_Ks=0;
  alpha.resize(L);
  log_coef.resize(L);
  sigma.resize(L);
  diags.resize(L,D);
  mu.resize(L,D);
    
  if(type==General || type==Factor)
  {
    Ks.resize(L);
    V_idx.resize(L);
    V.resize(L*avg_K,D);
    inv_lambda_minus_lambda0.resize(L*avg_K,D);
  }
  else
  {
    Ks.resize(0);
    V_idx.resize(0);
    V.resize(0,0);
    inv_lambda_minus_lambda0.resize(0,0);
  }
    
  if(type==Factor)
  {
    ivtdv_idx.resize(L);
    ivtdv.resize(L*avg_K*avg_K);
  }

}


// ### Nothing to add here, simply calls build_
void GaussMix::build()
{
  inherited::build();
  build_();
}

void GaussMix::train(VMat training_set)
{ 
  if(training_set->width() != inputsize()+targetsize())
    PLERROR("In GaussMix::train(VMat training_set) training_set->width() != inputsize()+targetsize()");

  setTrainingSet(training_set);

  // ### Please implement the actual training of the model.
  // ### For models with incremental training, to benefit 
  // ### from the "testing during training" and early-stopping 
  // ### mechanisms, you should make sure to call measure at 
  // ### every "epoch" (whatever epoch means for your algorithm).
  // ### ex:
  // if(measure(epoch,costvec)) 
  //     break; // exit training loop because early-stopping contditions were met
}

void GaussMix::use(const Vec& input, Vec& output)
{
  // ### You should redefine this method to compute the output
  // ### corresponfding to a new test input.
}

void GaussMix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Learner::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("GaussMix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void GaussMix::generate(Vec& x) const
{
 switch(type)
  {
  case Spherical:
    generateSpherical(x);
    return;
    break;
  case Elliptic:
    generateElliptic(x);
    return;
    break;
  case General:
    generateGeneral(x);
    return;
    break;
  case Factor:
    generateFactor(x);
    return;
    break;
  }
 PLERROR("unknown mixture type.");
}

void GaussMix::generateSpherical(Vec &x) const
{
  int l = multinomial_sample(alpha);
  fill_random_normal(x);
  x*=sigma[l];
  x += mu(l);
}

void GaussMix::generateElliptic(Vec &x) const
{
  int l = multinomial_sample(alpha);
  Vec lambda(diags(l));
  fill_random_normal(x);
  x *= lambda;
  x += mu(l);
}

void GaussMix::generateGeneral(Vec &x) const
{
  int l = multinomial_sample(alpha);
  x=0;
  
  // the covariance can be expressed as :
  // C = sum{i=1,K} [ lamda_i Vi Vi(t) ] + sum{i=K+1,D} [ lamda_0 Vi Vi(t) ]
  // this can also be reformulated as : 
  // C' = sum{i=1,K} [ (lamda_i-lambda_0) Vi Vi(t) ] + diag(lambda_0) * I 
  // C' = A + B
  // since C' and C are diagonalizable and share the same eigenvectors/values, they are equal.
  // thus, to sample from a gaussian with covar. matrix C , we add two samples from a gaussian with covar. A and a one with covar. B

  // to sample from a gaussian with covar matrix A,
  // we use x = diag(lambda) * V * z, with z sampled from N(0,1) 
  
  Vec norm(Ks[l]);
  fill_random_normal(norm);

  for(int i=0;i<Ks[l];i++)
  {
    Vec vi = V(V_idx[l]+i);
    real val = 1/sqrt(inv_lambda_minus_lambda0[V_idx[l]+i]) * norm[i];
    for(int j=0;j<D;j++)
      x[j]+= vi[j] * val;
  }

  // now add sample from N( 0,diag(lambda0) )
  norm.resize(D);
  fill_random_normal(norm);
  x += norm*sigma[l]; // in this case, vector sigma stocks lambda0s 
  x += mu(l);
}

void GaussMix::generateFactor(Vec &x) const
{
  int l = multinomial_sample(alpha);
  x=0;
  Vec norm(Ks[l]);
  fill_random_normal(norm);
  
  for(int i=0;i<Ks[l];i++)
  {
    Vec vi = V(V_idx[l]+i);
    for(int j=0;j<D;j++)
      x[j]+= vi[j] * norm[i];
  }
  norm.resize(D);
  fill_random_normal(norm);
  for(int i=0;i<D;i++)
    x[i] += norm[i]*diags(l)[i];
  x += mu(l);
}

double GaussMix::log_density(const Vec& x) const
{ 
  switch(type)
  {
  case Spherical:
    return logDensitySpherical(x);
    break;
  case Elliptic:
    return logDensityElliptic(x);
    break;
  case General:
    return logDensityGeneral(x);
    break;
  case Factor:
    return logDensityFactor(x);
    break;
  }
  PLERROR("unknown mixture type.");
  return 0.0;
}

double GaussMix::logDensitySpherical(const Vec& x, int num) const
{
  Vec x_minus_mu(x.length()), tmp(x.length());

  int begin= (num==-1?0:num);
  int end= (num==-1?L-1:num);

  Vec logs(end-begin+1);
  int idx=0;

  for(int l=begin;l<=end;l++)
  {
    logs[idx]=0;
    x_minus_mu =x-mu(l);
    logs[idx] += log(alpha[l]) + log_coef[l];
    transposeProduct(tmp, diagonalmatrix(Vec(D,1/sigma[l])), x_minus_mu); 
    // this is the exponential part in multivariate normal density equation
    logs[idx++]+= -0.5 * dot(tmp, x_minus_mu);
  }
  
  return logadd(logs);
}
 
double GaussMix::logDensityElliptic(const Vec& x, int num) const
{
  Vec x_minus_mu(x.length()), tmp(x.length()),tmp2(x.length());

  int begin= (num==-1?0:num);
  int end= (num==-1?L-1:num);

  Vec logs(end-begin+1);
  int idx=0;

  for(int l=begin;l<=end;l++)
  {
    logs[idx]=0;
    x_minus_mu =x-mu(l);
    logs[idx] += log(alpha[l]) + log_coef[l];
    tmp2<<diags(l);
    invertElements(tmp2);
    transposeProduct(tmp, diagonalmatrix(tmp2), x_minus_mu); 
    // this is the exponential part in multivariate normal density equation
    logs[idx++]+= -0.5 * dot(tmp, x_minus_mu);
  }
 
  return logadd(logs);
}
 
// return log(p(x)) =  logadd {1..l} ( log(alpha[l]) + log_coeff[l] + q[l] )
double GaussMix::logDensityGeneral(const Vec& x, int num) const
{
  Vec x_minus_mu(x.length());

  int begin= (num==-1?0:num);
  int end= (num==-1?L-1:num);

  Vec logs(end-begin+1);
  int idx=0;
  for(int l=begin;l<=end;l++)
  {
    bool galette = Ks[l]<D;
    x_minus_mu = x-mu(l);
    logs[idx]=log(alpha[l]);
    logs[idx]+= log_coef[l];

    // compute q
    Mat subV = V.subMatRows(V_idx[l],Ks[l]);

    real* ptr_inv_lambda_minus_lambda0 = inv_lambda_minus_lambda0.data() + V_idx[l];

    if(galette)
    {
      real* px = x.data();
      real* pmu = mu(l).data();
      real* pxmu = x_minus_mu.data();
      
      real sqnorm_xmu = 0;
      for(int i=0; i<D; i++)
      {
        real val = *px++ - *pmu++;
        sqnorm_xmu += val * val;
        *pxmu++ = val;
      }
      
      logs[idx] -= 0.5/sigma[l] * sqnorm_xmu;
    }
    
    for(int i=0; i<Ks[l]; i++)
      logs[idx] -= 0.5*(*ptr_inv_lambda_minus_lambda0++) *square(dot(subV(i),x_minus_mu));
    
    idx++;
  }
  return logadd(logs);
}
 
double GaussMix::logDensityFactor(const Vec& x, int num) const
{
  double sum=0;
  for(int l=0;l<L;l++)
    sum+=log(alpha[l])+ 0;
  return sum;
}

double GaussMix::survival_fn(const Vec& x) const
{ 
  PLERROR("survival_fn not implemented for GaussMix"); return 0.0; 
}

double GaussMix::cdf(const Vec& x) const
{ 
  PLERROR("cdf not implemented for GaussMix"); return 0.0; 
}

Vec GaussMix::expectation() const
{ 
  PLERROR("expectation not implemented for GaussMix"); return Vec(); 
}

Mat GaussMix::variance() const
{ 
  PLERROR("variance not implemented for GaussMix"); return Mat(); 
}

void GaussMix::printInfo()
{
  switch(type)
  {
  case Spherical:
    cout<<"type: Spherical"<<endl;
    cout<<"n_gaussians : "<<L<<endl;;
    for(int i=0;i<L;i++)
    {
      cout<<" gauss "<<i<<":"<<endl;
      cout<<"  alpha:"<<alpha[i]<<endl;
      cout<<"  sigma:"<<sigma[i]<<endl;
      cout<<"  mu   :"<<mu(i)<<endl;
    }
    return;
    break;
  case Elliptic:
    cout<<"type: Elliptic"<<endl;
    cout<<"n_gaussians : "<<L<<endl;;
    for(int i=0;i<L;i++)
    {
      cout<<" gauss "<<i<<":"<<endl;
      cout<<"  alpha:"<<alpha[i]<<endl;
      cout<<"  sigmas:"<<diags(i)<<endl;
      cout<<"  mu   :"<<mu(i)<<endl;
    }
    return;
    break;

  }
  PLERROR("GaussMix::printInfo not implemented for this type yet.");
}
  

%> // end of namespace PLearn
