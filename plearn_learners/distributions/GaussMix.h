

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
   * $Id: GaussMix.h,v 1.3 2003/06/17 02:10:01 jkeable Exp $ 
   ******************************************************* */

/*! \file GaussMix.h */
#ifndef GaussMix_INC
#define GaussMix_INC

#include "PDistribution.h"

namespace PLearn <%
using namespace std;

/*

This distribution implements a mixture of L multivariate normal gaussians in D dimensions

there are 3 ways to construct a GaussMix :
1: 
   setMixtureType(...)
   setGaussian(1,....)
   setGaussian(2,....)
   ...
   build();

2:
   setOption : alpha, mu, V, lambda 
   (e.g : mixtureType="General"; D=2;L=3;alpha = 3 [.33 .33 .33]; lambda = 6 [.2 .4 .2 .2 .2 .2]; mu = 3 2 [1 1 2 2 3 3]; V = 6 2  [1 0 0 1 1 0 0 1 1 0 0 1];  V_idx= 3 [0 2 4])
   build();

3: load a serialized object:
   PLearn::load(filename, GaussMixObject)

For the gaussians, you have 4 possible parametrizations (a.k.a types). Note that all gaussians of 
the mixture must be of the same type.

2 parameters are common to all 4 types :

alpha : the ponderation factor of that gaussian
mu : its center

then, accordingly to the chosen type, you will set :

** spherical
real sigma : gaussians will have a diagonal covariance matrix = sigma * I

** diagonal
Vec diag : gaussians will have a diagonal covariance matrix DIAG(diag)

** general

for the general case, the user provides K<=D orthonormal vectors defining the principal axis of the gaussian,
and their corresponding values (lambdas). The remaining unspecified D-K vectors, the complement of the orthogonal basis 
formed by the K vectors, if any, are assumed to have a constant value of lambda0

Mat eigenvecs : a K x D orthonormal matrix in which the *rows* are the eigenvectors
Vec lambda : the K factors of the vectors
real lambda0 : the factor for the D-K vector

** factor  ( ie. used to build a mixture of factor analyser )

-----> NOT TESTED YET

vecs : KxD matrix

suppose a variable x can be expressed as :

x = Vz + mu + psi, with

V, a DxK matrix
z, a K vector
mu, a D vector
psi, a D vector

and 

E(z) = 0, V(z) = I, E(psi)=0, C(psi_i, psi_j)=0, C(z, psi)=0,

then the factor analyzer model holds. 
If we have z and e multinormally distributed, we have :

p(z_k) = N(zk; 0,1)
p(x_n | z) = N(x_n; sum


**** to be continued ****

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

  //! a LxD matrix of diagonals (used if type is 'diagonal')
  Mat diags;

  //! a LxD matrix. Rows[l] is the center of the gaussians l
  Mat mu;  

  //! a sum_Ks x D matrix in which *rows* are components (only used for general and factor gaussians)
  Mat V;

  //! V_idx is a vector of length L
  //! V[ V_idx[l] ] up to V[ V_idx[l] + Ks[l] ] are rows of V describing the l-th gaussian
  TVec<int> V_idx;

  // a length=sum_Ks vector (only used for general gaussians)
  // **NOTE: it contains lambdas until build() is called, after, element 'i' contains ;
  Vec lambda;

  //! how much gaussians the mixture contains
  int L;

  //! the feature space dimension
  int D;

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

  //! set the type to spherical
  //! L : number of gaussians in mixture
  //! D : number of dimensions in feature space
  void setMixtureTypeSherical(int L, int D);

  void setMixtureTypeDiagonal(int L, int D);

  //! for 'general' and 'factor' mixtures :
  //! avg_K : the average number of dimensions of the gaussians (used to preallocate memory, so addGaussians calls are faster)
  void setMixtureTypeGeneral(int L, int avg_K, int D);

  void setMixtureTypeFactor(int L, int avg_K, int D);


  // spherical
  void setGaussian(int l, real alpha, Vec mu, real sigma);
  // diagonal
  void setGaussian(int l, real alpha, Vec mu, Vec diag);

  //! for general and factor : number of eigen vectors must not change if your updating a previoulsy set gaussian.
  // general
  void setGaussian(int l, real alpha, Vec mu, Vec _lambda, Mat eigenvecs, real _lambda0=0 );
  // factor
  void setGaussian(int l, real alpha, Vec mu, Mat vecs, Vec diag );

  virtual void generate(Vec& x) const;
  void generateSpherical(Vec &x) const;
  void generateDiagonal(Vec &x) const;
  void generateGeneral(Vec &x) const;
  void generateFactor(Vec &x) const;

  virtual void resetGenerator(long g_seed) const;

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

  //! length == L. the log of the constant part in the p(x) equation (1/sqrt(2*pi^D * Det(C)))
  Vec log_coef;

  //! precomputed (I+V(t)DV)^-1. Used for factor gaussians. 
  //! stored as a vector which is the flattened view of 'L' x 'Ks[i] x Ks[i]' matrices. 
  Vec ivtdv;

  //! A length==L vector. ivtdv_idx[l] is the index at which starts the flattened Ks[l] x Ks[l] matrix for the l-th gaussian in ivtdv
  TVec<int> ivtdv_idx; 
  
  //! the average number of dimensions of the gaussians (used to preallocate memory, so addGaussians calls are faster)
  int avg_K;

  //! a length==L vector. K[l] is the number of dimensions for the l-th gaussian 
  //! only used for general and factor mixtures
  TVec<int> Ks;

public:
  //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
  virtual void forget();


  virtual int inputsize() const {return D;}
  
  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Provides a help message describing this class
  static string help();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  DECLARE_NAME_AND_DEEPCOPY(GaussMix);

  // *******************
  // * Learner methods *
  // *******************

  //! trains the model
  virtual void train(VMat training_set); 

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
  
%> // end of namespace PLearn

#endif
