

// -*- C++ -*-

// GaussMix.h
// 
// Copyright (C) *YEAR* *AUTHOR(S)* 
// ...
// Copyright (C) *YEAR* *AUTHOR(S)* 
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
   * $Id: GaussMix.h,v 1.11 2004/05/06 13:07:11 yoshua Exp $ 
   ******************************************************* */

/*! \file GaussMix.h */
#ifndef GaussMix_INC
#define GaussMix_INC

#include "PDistribution.h"

namespace PLearn {
using namespace std;

/*

This distribution implements a mixture of L multivariate normal gaussians in D dimensions

there are 3 ways to construct a GaussMix :
1: 
   setMixtureType(...)
   setGaussianXXX(1, ...)
   setGaussianXXX(2, ...)
   ...
   build();

2: setOption : alpha, mu, V, lambda, etc.. 
   build();

3: load a serialized object:
   PLearn::load(filename, GaussMixObject)

For the gaussians, there are 4 possible parametrizations of the covariance matrix. All gaussians of the mixture must be of the same type. 
Note that depending on the chosen parametrization, members can have names that are irrelevent to the parameter they represent 
(i.e: sigma holds lambda0 when set to use the general gaussian type)

2 parameters are common to all 4 types :

alpha : the ponderation factor of that gaussian
mu : its center

then, accordingly to the chosen type, you will set for ...

** spherical (constant diagonal covar matrix)
real sigma : gaussians will have a diagonal covariance matrix = sigma * I

** diagonal (diagonal covar matrix)
Vec diag : gaussians will have a diagonal covariance matrix DIAG(diag)

** general (full-covariance matrix)

  For the general case, the user provides K<=D orthonormal vectors defining the principal axis of the gaussian (eigenvectors of its covar. matrix),
  and their corresponding values (lambdas). The remaining unspecified D-K vectors (that is, the complement of the orthogonal basis 
  formed by the K vectors), if any, are assumed to have a constant value of lambda0.
  
  Note: Each gaussian of the mixture can have a different K 

Mat V : a Ks[i] x D orthonormal matrix in which the *rows* are the eigenvectors (GaussMix::V is in reality the concatenation of all those Ks[i]xD matrices)
Vec lambda : the Ks[i] eigenvalues
real sigma : (lambda0) the eigenvalue used for the D-Ks[i] unspecified eigenvectors

** factor  ( ie. used to build a mixture of factor analyser )

Mat V : Ks[i]xD matrix which is the factor loading matrix (the Ks[i] vectors are not necessarily orthogonal neither normal)
        *** Note : In the litterature, V corresponds to "big lambda transposed". 
        (GaussMix::V is in reality the concatenation of all those Ks[i]xD matrices)
Vec diag : the noise on each dimension of the feature space (called psi in most litterature)

further notes on the FA model : 

suppose a random variable x can be expressed as :

x = Vz + mu + psi, with

V, a DxK matrix (factor loading matrix)
z, a K vector (values of the components)
mu, a D vector (mean of x)
psi, a D vector (random noise added on each dimension)

and 

E(z) = 0, V(z) = I, E(psi)=0, C(psi_i, psi_j)=0, C(z, psi)=0,

then the factor analyzer model holds. 

*/


class GaussMix: public PDistribution
{
protected:
  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...
    
public:

  typedef PDistribution inherited;

  // ************************
  // * public build options *
  // ************************


  //! length == L. the coefficients of the mixture
  Vec alpha;
  
  //! a length==L vector of sigmas (** or lambda0)
  Vec sigma;

  //! a LxD matrix of diagonals (type == 'diagonal' -> the diagonal of the covar matrix. type == 'factor' -> noise on each dimension of feature space)
  Mat diags;

  //! a LxD matrix. Rows[l] is the center of the gaussians l
  Mat mu;  

  //! a sum_Ks x D matrix in which *rows* are components (only used for general and factor gaussians)
  //! V[ V_idx[l] ] up to V[ V_idx[l] + Ks[l] ] are the rows of V describing the l-th gaussian
  Mat V;

  //! V_idx is a vector of length L
  TVec<int> V_idx;

  // a length=sum_Ks vector (only used for general gaussians) lambda[ V_idx[l] ] up to lambda[ V_idx[l] + Ks[l] ] are eigenvalus of the l-th gaussian
  Vec lambda;

  //! how many gaussians the mixture contains
  int L;

  //! the feature space dimension
  int D;

  // used if type=="General". Number of components and lambda0 used to perform EM training.
  int n_principal_components;
  real EM_lambda0;

  real relativ_change_stop_value;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  GaussMix();

  // ******************
  // * Object methods *
  // ******************

  // can be one of : 
  // Unknown, Spherical, Diagonal, General, Factor
  string type;

  // this function computes inv_ivtdv = (I + V(t) * psi^-1 * V) ^ -1 and logcoef
  void precomputeFAStuff(Mat V, Vec diag, real &log_coef, Vec inv_ivtdv);

  //! 4 next functions set the type of the parametrization of the gaussians covariance matrix

  //! L : number of gaussians in mixture
  //! D : number of dimensions in feature space
  void setMixtureTypeSpherical(int L, int D);

  void setMixtureTypeDiagonal(int L, int D);

  //! for 'general' and 'factor' mixtures :
  //! avg_K : the average number of dimensions of the gaussians (used to preallocate memory, so setGaussianXXX calls are faster)
  void setMixtureTypeGeneral(int L, int avg_K, int D);

  void setMixtureTypeFactor(int L, int avg_K, int D);

  //! 4 next functions provide an interface to set the gaussians manually

  // spherical
  void setGaussianSpherical(int l, real alpha, Vec mu, real sigma);
  // diagonal
  void setGaussianDiagonal(int l, real alpha, Vec mu, Vec diag);

  //! for general and factor : number of eigen vectors must not change if your updating a previoulsy set gaussian.
  // general
  // note: this function sets every _lambda[i] = MAX(lambda0,_lambda[i])
  void setGaussianGeneral(int l, real alpha, Vec mu, Vec _lambda, Mat V, real lambda0=0 );
  // factor
  void setGaussianFactor(int l, real alpha, Vec mu, Mat V, Vec diag );

  // set gaussian l to fit a group of samples (optionnaly weigthed)

  // spherical : sigma = average of variance on all dimensions
  void setGaussianSphericalWSamples(int l, real _alpha, VMat samples);
  // diagonal : sigma[i] = variance on i-th dimension 
  void setGaussianDiagonalWSamples(int l, real _alpha, VMat samples);
  // general : weighted covariance matrix is used
  void setGaussianGeneralWSamples(int l, real _alpha, real _sigma, int ncomponents, VMat samples);
  // factor : gaussian in set with EM algorithm
  void setGaussianFactorWSamples(int l, real _alpha, VMat samples);

  // if the argument given_gaussian is provided then a sample from
  // the specified gaussian is generated, otherwise from the mixture
  virtual void generate(Vec& x, int given_gaussian=-1) const;
  void generateSpherical(Vec &x, int given_gaussian=-1) const;
  void generateDiagonal(Vec &x, int given_gaussian=-1) const;
  void generateGeneral(Vec &x, int given_gaussian=-1) const;
  void generateFactor(Vec &x, int given_gaussian=-1) const;

  virtual void resetGenerator(long g_seed) const;


  void EMFactorAnalyser(VMat samples, real relativ_change_stop_value = 0.005);

  // train with EM on (possibly weighted) samples,
  // and stopping when NLL(t)/NLL(t-1) < 1 - relativ_change_stop_value.
  void EM(VMat samples, real relativ_change_stop_value = 0.005);

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

  // resizes arrays with respect to the dimensions given to setMixtureType
  void initArrays();

protected: 
  //! Declares this class' options
  //! (Please implement in .cc)
  static void declareOptions(OptionList& ol);

  void kmeans(VMat samples, int nclust, TVec<int> & clust_idx, Mat & clust, int maxit=9999);

  //! length == L. the log of the constant part in the p(x) equation : log(1/sqrt(2*pi^D * Det(C)))
  Vec log_coef;


  //! precomputed (I+V(t)DV)^-1. Used for factor gaussians. 
  //! stored as a vector which is the flattened view of all L 'Ks[i] x Ks[i]' matrices. 
  Vec inv_ivtdv;

  //! A length==L vector. inv_ivtdv_idx[l] is the index at which starts the flattened Ks[l] x Ks[l] matrix for the l-th gaussian in inv_ivtdv
  TVec<int> inv_ivtdv_idx; 
  
  //! the average number of dimensions of the gaussians (used to preallocate memory, so addGaussians calls are faster)
  int avg_K;

  //! a length==L vector. K[l] is the number of dimensions for the l-th gaussian 
  //! only used for general and factor mixtures
  TVec<int> Ks;

  // a work vector
  Vec tmpvec,tmpvec2;

public:
  //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
  virtual void forget();

  virtual int inputsize() const {return D;}
  
  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(GaussMix);

  // *******************
  // * Learner methods *
  // *******************

  //! trains the model
  virtual void train();

  real NLL(VMat set);

  // ************************
  // * Distribution methods *
  // ************************

  //! return density p(x)
  virtual double log_density(const Vec& x) const;

  // set num to 'i' if you want to get density as if mixture contained only i-th gaussian (with a coefficient of 1)
  double logDensitySpherical(const Vec& x, int num=-1) const;
  double logDensityDiagonal(const Vec& x, int num=-1) const;
  double logDensityGeneral(const Vec& x, int num=-1) const;
  double logDensityFactor(const Vec& x, int num=-1) const;


  //! return survival fn = P(X>x)
  virtual double survival_fn(const Vec& x) const;

  //! return survival fn = P(X<x)
  virtual double cdf(const Vec& x) const;

  //! return E[X] 
  virtual Vec expectation() const;

  //! return Var[X]
  virtual Mat variance() const;
   
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(GaussMix);
  
} // end of namespace PLearn

#endif
