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
 * $Id: GaussMix.cc,v 1.4 2003/06/16 15:05:55 jkeable Exp $ 
 ******************************************************* */

/*! \file GaussMix.cc */
#include "GaussMix.h"
#include "random.h"
#include "plapack.h"

namespace PLearn <%
using namespace std;

GaussMix::GaussMix() 
  :PDistribution()
/* ### Initialise all other fields here */
{
  // ### Possibly call setTestCostFunctions(...) to define the cost functions 
  // ### you are interested in (these are used by the default useAndCost() method,
  // ### which is called by the default test() method).
  // ### ex: 
  // setTestCostFunctions(squared_error());

  // ### You may also call setTestStatistics(...) if the Learner-default 'mean' and 'stderr' 
  // ### statistics are not appropriate...

  forget();

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

  declareOption(ol,"D", &GaussMix::D, OptionBase::buildoption,
                "number of dimensions in feature space. (a.k.a inputsize).");
    
  declareOption(ol,"L", &GaussMix::L, OptionBase::buildoption,
                "number of gaussians in mixture.");
  
  declareOption(ol,"mixtureType", &GaussMix::type, OptionBase::buildoption,
                "A string :  Unknown, Spherical, Diagonal, General, Factor");

  declareOption(ol, "alpha", &GaussMix::alpha, OptionBase::buildoption,
                "Coefficients of each gaussian. (E.g: 1/number_of_gaussians)");

  declareOption(ol, "mu", &GaussMix::mu, OptionBase::buildoption,
                "A LxD matrix containing the centers of The vertical concatenation of all the K[i] x D matrix, (each contains the K[i] vectors of gaussian i.) ");

  declareOption(ol, "sigma", &GaussMix::sigma, OptionBase::buildoption,
                "Spherical : the sigma for each gaussian\n"\
                "Diagonal : not used"\
                "General & Factor : for each gaussian, the lambda used for all D-K[l] dimensions");

  declareOption(ol, "diags", &GaussMix::diags, OptionBase::buildoption,
                "Only used in Diagonal : a L x D matrix where row 'l' is the diagonal of the covariance matrix of gaussian l");

  declareOption(ol, "lambda", &GaussMix::inv_lambda_minus_lambda0, OptionBase::buildoption,
                "The concatenation of all vectors of length K[l] containing the lambdas of the l-th gaussian.");

  declareOption(ol, "V", &GaussMix::V, OptionBase::buildoption,
                "The vertical concatenation of all the K[i] x D matrix, (each contains the K[i] vectors that define of gaussian i.) ");

  declareOption(ol, "V_idx", &GaussMix::V_idx, OptionBase::buildoption,
                "A vector of size L. Element 'l' is the index of the first vector of gaussian 'l' in the matrix 'V' (and in the vector 'lambda')");

   
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}


string GaussMix::help()
{
  // ### Provide some useful description of what the class is ...
  return 
    "GaussMix implements a mixture of gaussians, for which \n"\
    "L : number of gaussians\n"\
    "D : number of dimensions in the feature space\n"\
    "There are 4 possible parametrization types.:\n"\
    "Spherical : gaussians have covar matrix = diag(sigma). User provides sigma.\n"\
    "Diagonal : gaussians have covar matrix = diag(sigma_i). User provides a vector of D sigmas.\n"\
    "General : gaussians have an unconstrained covariance matrix. User provides the K<=D greatest eigenvalues\n"
    "          (thru parameter lambda) and their corresponding eigenvectors (thru KxD matrix V)\n"\
    "          of the covariance matrix. For the remaining D-K eigenvectors, a user-given eigenvalue (thru sigma) is assumed\n"\
    "Factor : as in the general case, the gaussians are defined with K<=D vectors (thru KxD matrix 'V'), but these need not be orthogonal.\n"\
    "         The covariance matrix used will be V(t)V + psi with psi a D-vector (provided thru diags).\n"\
    "2 parameters are common to all 4 types :\n"\
    "alpha : the ponderation factor of that gaussian\n"\
    "mu : its center\n" + optionHelp();
}

void GaussMix::setMixtureTypeSherical(int _L, int _D)
{
  L=_L;
  D=_D;
  avg_K=0;
  type="Spherical";
  initArrays();
}

void GaussMix::setMixtureTypeDiagonal(int _L, int _D)
{
  L=_L;
  D=_D;
  avg_K=0;
  type="Diagonal";
  initArrays();
}

void GaussMix::setMixtureTypeGeneral(int _L, int _avg_K, int _D)
{
  L=_L;
  D=_D;
  avg_K=_avg_K;
  type="General";
  initArrays();
}

void GaussMix::setMixtureTypeFactor(int _L, int _avg_K, int _D)
{
  L=_L;
  D=_D;
  avg_K=_avg_K;
  type="Factor";
  initArrays();
}

// spherical
void GaussMix::setGaussian(int l, real _alpha, Vec _mu, real _sigma)
{
  if(type!="Spherical")
    PLERROR("GaussMix::setGaussian : type is not Spherical");

  alpha[l]=_alpha;
  mu(l)<<_mu;
  sigma[l]=_sigma;
}

// diagonal
void GaussMix::setGaussian(int l, real _alpha, Vec _mu, Vec diag)
{
  if(type!="Diagonal")
    PLERROR("GaussMix::setGaussian : type is not Diagonal");

  alpha[l]=_alpha;
  mu(l)<<_mu;
  diags(l)<<diag;
}

//general
void GaussMix::setGaussian(int l, real _alpha, Vec _mu, Vec _lambda, Mat eigenvecs, real lambda0 )
{
  int num_lambda = _lambda.length();

  if(num_lambda==0)
    PLERROR("GaussMix::setGaussian : need at least 1 eigenvector");
  if(type!="General")
    PLERROR("GaussMix::setGaussian : type is not General");

  alpha[l]=_alpha;
  mu(l)<<_mu;
  sigma[l]=lambda0;

  if(Ks[l]==0)
  {
    // things to do only on the first time this gaussian is set
    Ks[l]=num_lambda;
    V_idx[l]=(l==0)?0:V_idx[l-1]+Ks[l];

    if(V.length()< V_idx[l] + Ks[l])
    {
      int newl = MAX(V_idx[l] + Ks[l], (int)ceil((double)V.length()*1.5));
      V.resize(V_idx[l] + Ks[l],V.width(), newl);
      inv_lambda_minus_lambda0.resize(newl);
    }
  }
  else 
  {
    if(Ks[l]!=num_lambda)
      PLERROR("GaussMix::setGaussian : for the same gaussian, the number of vectors (length of the 'eigenvecs' matrix) that "\
              "define a gaussian must not change between calls. ");
  }

  inv_lambda_minus_lambda0.subVec(V_idx[l],Ks[l])<<_lambda;
  V.subMatRows(V_idx[l],Ks[l])<<eigenvecs;

}

//factor
void GaussMix::setGaussian(int l, real _alpha, Vec _mu, Mat vecs, Vec diag )
{
  if(vecs.length()==0)
    PLERROR("GaussMix::setGaussian : need at least 1 eigenvector");
  if(type!="Factor")
    PLERROR("GaussMix::setGaussian : type is not General");
  alpha[l]=_alpha;
  mu(l)<<_mu;
  diags(l)<<diag ;

  // things to do only on the first time

  if(Ks[l]==0)
  {
    Ks[l]=vecs.length();
    V_idx[l]=(l==0)?0:V_idx[l-1]+Ks[l];

    if(V.length()< V_idx[l] + Ks[l])
    {
      int newl = MAX(V_idx[l] + Ks[l],(int)ceil((double)V.length()*1.5));
      V.resize(V_idx[l] + Ks[l], V.width(), newl);
    }
    
    if(ivtdv.length()< ivtdv_idx[l] + Ks[l]*Ks[l])
    {
      int newl = MAX(ivtdv_idx[l] + Ks[l]*Ks[l], (int)ceil((double)ivtdv.length()*1.5));
      ivtdv.resize(ivtdv_idx[l] + Ks[l]*Ks[l],newl);
    }
  }
  else if(Ks[l]!=vecs.length())
    PLERROR("GaussMix::setGaussian : for the same gaussian, the number of vectors (length of the 'eigenvecs' matrix) that "\
            "define a gaussian must not change between calls. ");
  
  V.subMatRows(V_idx[l],Ks[l])<<vecs;
  
}

void GaussMix::build_()
{
  if(type=="Unknown")
    return;
  
  if(alpha.length()!=L)
    PLERROR("You provided %i mixture coefficients. need %i",alpha.length(),L);
  if(mu.length()!=L)
    PLERROR("You provided %i rows in matrix mu. need %i",mu.length(),L);

  int sum_Ks=0;
  Ks.resize(L);
  if(sigma.size()==0)
  {
    sigma.resize(L);
    sigma.fill(0.0);
  }
  int l;
  for(l=0;l<L-1;l++)
    sum_Ks+=Ks[l]=V_idx[l+1]-V_idx[l];
  sum_Ks+=Ks[l]=V.length()-V_idx[L-1];

  log_coef.resize(L);
  mu.resize(L,D);
  V.resize(sum_Ks,D);
  inv_lambda_minus_lambda0.resize(sum_Ks);

  if(type=="Spherical")
    for(int l=0;l<L;l++)
      log_coef[l]=log(1/sqrt(pow(2*3.14159,D) * pow(sigma[l],D)));

  else if(type=="Diagonal")
    for(int l=0;l<L;l++)
      log_coef[l]=log(1/sqrt(pow(2*3.14159,D) * product(diags(l))));

  else if(type=="General")
    for(int l=0;l<L;l++)
    {
      Vec _lambda = inv_lambda_minus_lambda0.subVec(V_idx[l],Ks[l]);
      for(int i=0;i<Ks[l];i++)
        _lambda[i] = 1.0/MAX(0,_lambda[i] - sigma[l]);
  
      // compute determinant
      double log_det = sum_of_log(_lambda);
    
      if(D - _lambda.size() > 0)
        log_det+= log(sigma[l])*(D-_lambda.size());
    
      log_coef[l] = -0.5*( D*log(2*3.141549) + log_det );
    }
  
  else if(type=="Factor")

    for(int l=0;l<L;l++)
    {
      int K=Ks[l];

      // compute I + V(t) * psi * V
      
      Mat i_plus_vt_ipsi_v( K, K, 0.0 );
      Vec diag(diags(l));

      for(int i=0;i<K;i++)
      {
        Vec vi(V(V_idx[l]+i));
        Vec mati(i_plus_vt_ipsi_v(i));
        for(int j=0;j<K;j++)
        {
          Vec vj(V(V_idx[l]+j));
          for(int k=0;k < D;k++)
            // diags is psi
            mati[j]+=vi[k] * diag[k] * vj[k];
        }
      }
      addToDiagonal(i_plus_vt_ipsi_v,1.0);
      
      real determinant = det(i_plus_vt_ipsi_v);
      
      // see logDensityFactor for explanations on formulas
      log_coef = -D/2*log(2*3.14159) - 0.5 * log(determinant) - 0.5 * log(product(diag));
      
      Mat eigenvec(K,K);
      Vec eigenval(K);
      
      eigenVecOfSymmMat(i_plus_vt_ipsi_v, K, eigenval, eigenvec);
      
      // compute (I + V(t) * psi * V)^-1
      
      Mat inv(K,K);
      for(int i=0;i<K;i++)
      {
        Vec vi(eigenvec(i));
        Vec mati(inv(i));
        for(int j=0;j<K;j++)
        {
          Vec vj(eigenvec(j));
          for(int k=0;k < K;k++)
            mati[j]+=vi[k] * eigenval[k] * vj[k];
        }
      }

      if(l>0)
        ivtdv_idx[l] = ivtdv_idx[l-1] + K*K;
      
      ivtdv.subVec(ivtdv_idx[l], K*K) << inv.toVec();
      
    }
}

void GaussMix::initArrays()
{
  alpha.resize(L);
  log_coef.resize(L);
  sigma.resize(L);
  diags.resize(L,D);
  mu.resize(L,D);
    
  if(type=="General" || type=="Factor")
  {
    Ks.resize(L);
    Ks.fill(0);
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
    
  if(type=="Factor")
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


void GaussMix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  PLearner::makeDeepCopyFromShallowCopy(copies);

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
  if(type[0]=='S')
    generateSpherical(x);
  else if(type[0]=='E')
    generateDiagonal(x);
  else if(type[0]=='G')
    generateGeneral(x);
  else if(type[0]=='F')
    generateFactor(x);
  else if(type=="Unknown")
    PLERROR("You forgot to specify mixture type (Spherical, Diagonal, General, Factor).");
  else PLERROR("unknown mixtrure type");
  return;
}

void GaussMix::generateSpherical(Vec &x) const
{
  int l = multinomial_sample(alpha);
  fill_random_normal(x);
  x*=sigma[l];
  x += mu(l);
}

void GaussMix::generateDiagonal(Vec &x) const
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
  
  // the covariance matrix of the general gaussian type can be expressed as :
  // C = sum{i=1,K} [ lamda_i Vi Vi(t) ] + sum{i=K+1,D} [ lamda_0 Vi Vi(t) ]
  // Where Vi is the i-th eigenvector
  // this can also be reformulated as : 
  // C2 = sum{i=1,K} [ (lamda_i-lambda_0) Vi Vi(t) ] + diag(lambda_0) * I 
  // C2 = A + B
  // since C2 and C are diagonalizable and share the same eigenvectors/values, they are equal.
  // thus, to sample from a gaussian with covar. matrix C , we add two samples from a gaussian with covar. A and a one with covar. B

  // to sample from a gaussian with covar matrix A ( == diag(lambda) ),
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
  if(type[0]=='S')
    return logDensitySpherical(x);
  else if(type[0]=='E')
    return logDensityDiagonal(x);
  else if(type[0]=='G')
    return logDensityGeneral(x);
  else if(type[0]=='F')
    return logDensityFactor(x);
  else if(type=="Unknown")
    PLERROR("You forgot to specify mixture type (Spherical, Diagonal, General, Factor).");
  else PLERROR("unknown mixtrure type");
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
    // exponential part in multivariate normal density equation
    logs[idx++]+= -0.5 * dot(tmp, x_minus_mu);
  }
  
  return logadd(logs);
}
 
double GaussMix::logDensityDiagonal(const Vec& x, int num) const
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
    // exponential part in multivariate normal density equation
    logs[idx++]+= -0.5 * dot(tmp, x_minus_mu);
  }
 
  return logadd(logs);
}

// return log(p(x)) =  logadd {1..l} ( log(alpha[l]) + log_coeff[l] + q[l] )
// with q[l] = -0.5 * (x-mu[l])'C-1(x-mu[l])
//     The expression q = (V'(x-mu))'.inv(D).(V'(x-mu)) can be understood as:
//        a) projecting vector x-mu on the orthonormal basis V, 
//           i.e. obtaining a transformed x that we shall call y:  y = V'(x-mu)
//           (y corresponds to x, expressed in the coordinate system V)
//           y_i = V'_i.(x-mu)
//
//        b) computing the squared norm of y , after first rescaling each coordinate by a factor 1/sqrt(lambda_i)
//           (i.e. differences in the directions with large lambda_i are given less importance)
//           Giving  q = sum_i[ 1/lambda_i  y_i^2]
//
//     If we only keep the first k eigenvalues, and replace the following d-k ones by the same value gamma
//     i.e.  lambda_k+1 = ... = lambda_d = gamma
//    
//     Then q can be expressed as:
//       q = \sum_{i=1}^k [ 1/lambda_i y_i^2 ]   +   1/gamma \sum_{i=k+1}^d [ y_i^2 ]
//
//     But, as y is just x expressed in another orthonormal basis, we have |y|^2 = |x-mu|^2
//     ( proof: |y|^2 = |V'(x-mu)|^2 = (V'(x-mu))'.(V'(x-mu)) = (x-mu)'.V.V'.(x-mu) = (x-mu)'(x-mu) = |x-mu|^2 )
//    
//     Thus, we know  \sum_{i=1}^d [ y_i^2 ] = |x-mu|^2
//     Thus \sum_{i=k+1}^d [ y_i^2 ] = |x-mu|^2 - \sum_{i=1}^k [ y_i^2 ]
//
//     Consequently: 
//       q = \sum_{i=1}^k [ 1/lambda_i y_i^2 ]   +  1/gamma ( |x-mu|^2 - \sum_{i=1}^k [ y_i^2 ] )
//
//       q = \sum_{i=1}^k [ (1/lambda_i - 1/gamma) y_i^2 ]  +  1/gamma  |x-mu|^2
//
//       q = \sum_{i=1}^k [ (1/lambda_i - 1/gamma) (V'_i.(x-mu))^2 ]  +  1/gamma  |x-mu|^2
//
//       This gives the efficient algorithm implemented below
double GaussMix::logDensityGeneral(const Vec& x, int num) const
{
  Vec x_minus_mu(x.length());

  int begin= (num==-1?0:num);
  int end= (num==-1?L-1:num);

  Vec logs(end-begin+1);
  int idx=0;
  for(int l=begin;l<=end;l++)
  {
//  cout<<"gaussian ="<<l<<endl;

    bool galette = Ks[l]<D;
    x_minus_mu = x-mu(l);

/// cout<<" x-mu:"<<x_minus_mu<<endl;
    
    logs[idx]=log(alpha[l]);

//  cout<<"logcoef :"<<log_coef[l]<<endl;
    
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

void GaussMix::forget()
{
  type="Unknown";
  L=D=0;
  avg_K=0;
  initArrays();
}


void GaussMix::resetGenerator(long g_seed) const
{ 
  manual_seed(g_seed);  
}


%> // end of namespace PLearn
