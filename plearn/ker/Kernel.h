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
   * $Id: Kernel.h,v 1.2 2002/08/08 22:53:14 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/Kernel.h */

#ifndef Kernel_INC
#define Kernel_INC

#include "Object.h"
#include "Mat.h"
#include "VMat.h"
#include "PLMPI.h"

namespace PLearn <%
using namespace std;

class Kernel: public Object
{
		typedef Object inherited;
		
protected:
  VMat data; //!<  data for kernel matrix, which will be used for calls to evaluate_i_j and the like 
  
public:
  bool is_symmetric;

  Kernel(bool is__symmetric = true) 
    : is_symmetric(is__symmetric) 
    {}

  DECLARE_ABSTRACT_NAME_AND_DEEPCOPY(Kernel);

  //!  ** Subclasses must overload this method **
  virtual real evaluate(const Vec& x1, const Vec& x2) const = 0; //!<  returns K(x1,x2) 

  //!  ** Subclasses may overload these methods to provide efficient kernel matrix access **

/*!     This method sets the data VMat that will be used to define the kernel
    matrix. It may precompute values from this that may later accelerate
    the evaluation of a kernel matrix element
*/
  virtual void setDataForKernelMatrix(VMat the_data);

  //!  returns evaluate(data(i),data(j))
  virtual real evaluate_i_j(int i, int j) const; 

/*!     returns evaluate(data(i),x)
    [squared_norm_of_x is just a hint that may
    allow to speed up computation if it is already known, 
    but it's optional]
*/
  virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const; 

  //!  returns evaluate(x,data(i)) [default version calls evaluate_i_x if
  //!  kernel is_symmetric]
  virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const; 

  //!  ** Subclasses may overload these methods ** 
  //!  They provide a generic way to set and retrieve kernel parameters
  virtual void setParameters(Vec paramvec); //!<  default version produces an error
  virtual Vec getParameters() const; //!<  default version returns an empty Vec

  //!  ** Subclasses should NOT overload the following methods. The default versions are fine. **

  void apply(VMat m1, VMat m2, Mat& result) const; //!<  result(i,j) = K(m1(i),m2(j))
  Mat apply(VMat m1, VMat m2) const; //!<  same as above, but returns the result mat instead
  void apply(VMat m, const Vec& x, Vec& result) const; //!<  result[i]=K(m[i],x)
  void apply(Vec x, VMat m, Vec& result) const; //!<  result[i]=K(x,m[i])

  inline real operator()(const Vec& x1, const Vec& x2) const
    { return evaluate(x1,x2); }

  //!  Returns a Mat m such that m(i,j) is the index of jth closest neighbour of input i, 
  //!  according to the "distance" measures given by D(i,j)
  static Mat computeNeighbourMatrixFromDistanceMatrix(const Mat& D, bool insure_self_first_neighbour=true);

  Mat estimateHistograms(VMat d, real sameness_threshold, real minval, real maxval, int nbins) const;
  Mat estimateHistograms(Mat input_and_class, real minval, real maxval, int nbins) const;
  real test(VMat d, real threshold, real sameness_below_threshold, real sameness_above_threshold) const;
  virtual void build() {}
	virtual void oldwrite(ostream& out) const;
	virtual void oldread(istream& in);
  virtual ~Kernel();
};


/*!   *******
  * Ker *
  *******
*/
class Ker: public PP<Kernel>
{
public:
  Ker() {}
  Ker(Kernel* v) :PP<Kernel>(v) {}
  Ker(const Ker& other) :PP<Kernel>(other) {}

  real operator()(const Vec& x1, const Vec& x2) const
    { return ptr->evaluate(x1,x2); }
};

DECLARE_OBJECT_PTR(Kernel);
DECLARE_TYPE_TRAITS(Ker);
DECLARE_OBJECT_PP(Ker, Kernel);

//! This class implements an Ln distance (defaults to L2 i.e. euclidean distance).
class DistanceKernel: public Kernel
{
		typedef Kernel inherited;
		
  protected:
    real n;  //!<  1 for L1, 2 for L2, etc...
  public:

    DistanceKernel(real the_Ln=2):
      n(the_Ln) {}

    DECLARE_NAME_AND_DEEPCOPY(DistanceKernel);
  virtual string info() const;
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
  virtual void oldread(istream& in);

protected:
  static void declareOptions(OptionList& ol);
};

DECLARE_OBJECT_PTR(DistanceKernel);

class PowDistanceKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
   PowDistanceKernel() : n() {}
 protected:
  real n;  //!<  1 for L1, 2 for L2, etc...
 public:
  PowDistanceKernel(real the_Ln):
    n(the_Ln) {}
  DECLARE_NAME_AND_DEEPCOPY(PowDistanceKernel);
  virtual string info() const;
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "n" 
  
};

DECLARE_OBJECT_PTR(PowDistanceKernel);

//!  returns <x1,x2>
class DotProductKernel: public Kernel
{
		typedef Kernel inherited;
		
 public:
  DotProductKernel() {}
  DECLARE_NAME_AND_DEEPCOPY(DotProductKernel);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;  
  virtual real evaluate_i_j(int i, int j) const; //!<  returns evaluate(data(i),data(j))
  virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const; //!<  returns evaluate(data(i),x)
  virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const; //!<  returns evaluate(x,data(i))
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
};

DECLARE_OBJECT_PTR(DotProductKernel);

/*!   behaves like PolynomialKernel except that the x1 and x2 vectors
  actually only contain INDICES of the rows of a CompactVMatrix,
  and the square difference is performed efficiently, taking
  advantage of the discrete nature of many fields.
*/
class CompactVMatrixPolynomialKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
  CompactVMatrixPolynomialKernel() : n(), beta() {}
  protected:
    int n; //!<  degree of polynomial
    real beta; //!<  a normalization constant for numerical stability
    PP<CompactVMatrix> m;
  public:
    CompactVMatrixPolynomialKernel(int degree, PP<CompactVMatrix>& vm, real the_beta=1.0)
      : n(degree), beta(the_beta), m(vm) {}
    DECLARE_NAME_AND_DEEPCOPY(CompactVMatrixPolynomialKernel);
    virtual real evaluate(const Vec& x1, const Vec& x2) const;    
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
    virtual void write(ostream& out) const;
    virtual void oldread(istream& in);
    //!  recognized options are "n"  and "beta"
    
};

DECLARE_OBJECT_PTR(CompactVMatrixPolynomialKernel);

//!  returns (beta*dot(x1,x2)+1)^n
class PolynomialKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
  PolynomialKernel() : n(), beta() {}
 public:
  int n; //!<  degree of polynomial
  real beta; //!<  a normalization constant for numerical stability
 public:
  PolynomialKernel(int degree, real the_beta=1.0)
    : n(degree), beta(the_beta) {}
  DECLARE_NAME_AND_DEEPCOPY(PolynomialKernel);

    inline real evaluateFromDot(real dot_product) const
    { return ipow(beta*dot_product+1.0, n); }

  virtual real evaluate(const Vec& x1, const Vec& x2) const; 
  virtual real evaluate_i_j(int i, int j) const; //!<  returns evaluate(data(i),data(j))
  virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const; //!<  returns evaluate(data(i),x)
  virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const; //!<  returns evaluate(x,data(i))
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized options are "n"  and "beta"
  
};

DECLARE_OBJECT_PTR(PolynomialKernel);

//!  returns sigmoid(c*x1.x2)
class SigmoidalKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
   SigmoidalKernel() : c() {}
 protected:
  real c; //!<  smoothing constant
 public:
  SigmoidalKernel(real the_c): c(the_c) {}
  DECLARE_NAME_AND_DEEPCOPY(SigmoidalKernel);
  virtual real evaluate(const Vec& x1, const Vec& x2) const; 
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "c"
  
};

DECLARE_OBJECT_PTR(SigmoidalKernel);

//!  returns log(1+exp(c*x1.x2)) = primitive of sigmoidal kernel
class SigmoidPrimitiveKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
   SigmoidPrimitiveKernel() : c() {}
 protected:
  real c; //!<  smoothing constant
 public:
  SigmoidPrimitiveKernel(real the_c): c(the_c) {}
  DECLARE_NAME_AND_DEEPCOPY(SigmoidPrimitiveKernel);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;  
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  //!  recognized option is "c"
  
};

DECLARE_OBJECT_PTR(SigmoidPrimitiveKernel);

//!  returns prod_i log(1+exp(c*(x1[i]-x2[i])))
//!  NOTE: IT IS NOT SYMMETRIC!
class ConvexBasisKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
   ConvexBasisKernel() : inherited(false), c() {}
 protected:
  real c; //!<  smoothing constant
 public:
  ConvexBasisKernel(real the_c): inherited(false), c(the_c) {}
  DECLARE_NAME_AND_DEEPCOPY(ConvexBasisKernel);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "c"
  
};

DECLARE_OBJECT_PTR(ConvexBasisKernel);

//!  returns exp(-norm_2(x1-x2)^2/sigma^2)
class GaussianKernel: public Kernel
{
  typedef Kernel inherited;

protected:
  static void declareOptions(OptionList& ol);

public:
  real sigma;

protected:
  real minus_one_over_sigmasquare;
  Vec squarednorms; //!<  squarednorms of the rows of the data VMat (data is a member of Kernel)

private:
  void build_();
	 
 public:
  virtual void build();

  GaussianKernel(real the_sigma=1):
    sigma(the_sigma)
  { build_(); }

  DECLARE_NAME_AND_DEEPCOPY(GaussianKernel);
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  inline real evaluateFromSquaredNormOfDifference(real sqnorm_of_diff) const
  { return exp(sqnorm_of_diff*minus_one_over_sigmasquare); }

  inline real evaluateFromDotAndSquaredNorm(real sqnorm_x1, real dot_x1_x2, real sqnorm_x2) const
  { return evaluateFromSquaredNormOfDifference((sqnorm_x1+sqnorm_x2)-(dot_x1_x2+dot_x1_x2)); }

  //!  This method precomputes the squared norm for all the data to later speed up evaluate methods
  virtual void setDataForKernelMatrix(VMat the_data);

  virtual real evaluate(const Vec& x1, const Vec& x2) const; //!<  returns K(x1,x2) 
  virtual real evaluate_i_j(int i, int j) const; //!<  returns evaluate(data(i),data(j))
  virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const; //!<  returns evaluate(data(i),x)
  virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const; //!<  returns evaluate(x,data(i))

  virtual void setParameters(Vec paramvec);
  virtual void oldread(istream& in);
  
};

DECLARE_OBJECT_PTR(GaussianKernel);

//!  A kernel that precomputes the kernel matrix as soon as setDataForKernelMatrix is called.
//!  The kernel matrix is sotred internally as floats
class PrecomputedKernel: public Kernel
{
		typedef Kernel inherited;
		
protected:
  Ker ker; //!<  the real underlying kernel
  float* precomputedK; //!<  the precomputed kernel matrix

public:
  PrecomputedKernel():
    precomputedK(0) {}

  PrecomputedKernel(Ker the_ker): 
    ker(the_ker),
    precomputedK(0) {}

  virtual ~PrecomputedKernel();

  DECLARE_NAME_AND_DEEPCOPY(PrecomputedKernel);
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //!  This method precomputes and stores all kernel values 
  virtual void setDataForKernelMatrix(VMat the_data);

  virtual real evaluate(const Vec& x1, const Vec& x2) const; //!<  returns K(x1,x2) 
  virtual real evaluate_i_j(int i, int j) const; //!<  returns evaluate(data(i),data(j))
  virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const; //!<  returns evaluate(data(i),x)
  virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const; //!<  returns evaluate(x,data(i))

    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);

  //!  simply forwards to underlying kernel
    
};

DECLARE_OBJECT_PTR(PrecomputedKernel);

/*!   behaves like GaussianKernel except that the x1 and x2 vectors
  actually only contain INDICES of the rows of a CompactVMatrix,
  and the square difference is performed efficiently, taking
  advantage of the discrete nature of many fields.
*/
class CompactVMatrixGaussianKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
  CompactVMatrixGaussianKernel() : sigma(), m(0) {}
 protected:
  real sigma;
  PP<CompactVMatrix> m;
 public:
  CompactVMatrixGaussianKernel(real the_sigma,PP<CompactVMatrix>& vm)
    : sigma(the_sigma), m(vm) {}
  DECLARE_NAME_AND_DEEPCOPY(CompactVMatrixGaussianKernel);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
  virtual void setParameters(Vec paramvec);
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "sigma"
  
};

DECLARE_OBJECT_PTR(CompactVMatrixGaussianKernel);


//!  returns exp(-sum_i[(phi_i*(x1_i - x2_i))^2]/sigma^2)
class ScaledGaussianKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
  ScaledGaussianKernel() : sigma(), phi() {}
 protected:
  real sigma;
  Vec phi;
 public:
  ScaledGaussianKernel(real the_sigma, Vec the_phi)
    :sigma(the_sigma), phi(the_phi)
  {}

  DECLARE_NAME_AND_DEEPCOPY(ScaledGaussianKernel);
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "sigma"
  
  
};

DECLARE_OBJECT_PTR(ScaledGaussianKernel);

/*!   Like GaussianKernel but the result is properly normalized
  as a density.
  returns exp(-0.5*norm_2(x1-x2)^2/sigma^2)/(sigma*sqrt(2pi))^n
  where n=x1.length().
*/
class GaussianDensityKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
   GaussianDensityKernel() : sigma() {}
 protected:
  real sigma;
  static void declareOptions(OptionList& ol);
 public:
  GaussianDensityKernel(real the_sigma):sigma(the_sigma) {}
  DECLARE_NAME_AND_DEEPCOPY(GaussianDensityKernel);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
  virtual void oldread(istream& in);
};

DECLARE_OBJECT_PTR(GaussianDensityKernel);

/*! computes the log of a GaussianDensityKernel
  logp =  log[ exp(-0.5*norm_2(x1-x2)^2/sigma^2)/(sigma*sqrt(2pi))^n ]
       =  log[ exp(-0.5*norm_2(x1-x2)^2/sigma^2)/exp(log[(sigma*sqrt(2pi))^n]) ]
       =  log[ exp(-0.5*norm_2(x1-x2)^2/sigma^2)/exp(log[(2pi*sigma^2)^(n/2)]) ]
       = -0.5* [ norm_2(x1-x2)^2/sigma^2 + n*( log(2pi) + log(sigma^2) ) ]
  where n=x1.length().
*/
class LogOfGaussianDensityKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
   LogOfGaussianDensityKernel() : sigma() {}
 protected:
  real sigma;
  static void declareOptions(OptionList& ol);
 public:
  LogOfGaussianDensityKernel(real the_sigma):sigma(the_sigma) {}
  DECLARE_NAME_AND_DEEPCOPY(LogOfGaussianDensityKernel);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
};

DECLARE_OBJECT_PTR(LogOfGaussianDensityKernel);


//!  returns exp(-phi*(sum_i[abs(x1_i^a - x2_i^a)^b])^c)
class GeneralizedDistanceRBFKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
   GeneralizedDistanceRBFKernel() : phi(), a(), b(), c() {}
 protected:
  real phi, a, b, c;
 public:
  GeneralizedDistanceRBFKernel(real the_phi, real the_a, real the_b, real the_c)
    :phi(the_phi), a(the_a), b(the_b), c(the_c)
    {}

  DECLARE_NAME_AND_DEEPCOPY(GeneralizedDistanceRBFKernel);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized options are "phi", "a", "b" and "c"
  
};

DECLARE_OBJECT_PTR(GeneralizedDistanceRBFKernel);

//!  returns exp(-(sum_i phi_i*[abs(x1_i^a - x2_i^a)^b])^c)
class ScaledGeneralizedDistanceRBFKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
   ScaledGeneralizedDistanceRBFKernel() : b(), c(), phi(), a() {}
 protected:
  real b, c;
  Vec phi, a;
 public:
  ScaledGeneralizedDistanceRBFKernel(Vec the_phi, Vec the_a, real the_b, real the_c)
    : b(the_b), c(the_c), phi(the_phi), a(the_a)
    {}

  DECLARE_NAME_AND_DEEPCOPY(ScaledGeneralizedDistanceRBFKernel);
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized options are "b" and "c"
  
};

DECLARE_OBJECT_PTR(ScaledGeneralizedDistanceRBFKernel);

//!  returns exp(-phi*(sum_i[abs(x1_i - x2_i)]))
class LaplacianKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
   LaplacianKernel() : phi() {}
 protected:
  real phi;
 public:
  LaplacianKernel(real the_phi)
    :phi(the_phi)
    {}

  DECLARE_NAME_AND_DEEPCOPY(LaplacianKernel);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "phi"
  
};

DECLARE_OBJECT_PTR(LaplacianKernel);

//!  returns exp(-(sum_i[abs(x1_i - x2_i)*phi_i]))
class ScaledLaplacianKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
   ScaledLaplacianKernel() : phi() {}
 protected:
  Vec phi;
 public:
  ScaledLaplacianKernel(Vec the_phi)
    :phi(the_phi)
    {}

  DECLARE_NAME_AND_DEEPCOPY(ScaledLaplacianKernel);
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
};

DECLARE_OBJECT_PTR(ScaledLaplacianKernel);

//!  returns sum_i[x1_i-x2_i]
class DifferenceKernel: public Kernel
{
		typedef Kernel inherited;
		
public:
  DifferenceKernel() {}
  DECLARE_NAME_AND_DEEPCOPY(DifferenceKernel);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
};

DECLARE_OBJECT_PTR(DifferenceKernel);

class NegKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
  NegKernel() : ker() {}
 protected:
  Ker ker;
 public:
  NegKernel(const Ker& the_ker): ker(the_ker) {}
  DECLARE_NAME_AND_DEEPCOPY(NegKernel);
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
};

DECLARE_OBJECT_PTR(NegKernel);

class NormalizedDotProductKernel: public Kernel
{
		typedef Kernel inherited;
		
 protected:
  real norm_to_use;
 public:
  NormalizedDotProductKernel(real the_norm=2.0): norm_to_use(the_norm) {}
  DECLARE_NAME_AND_DEEPCOPY(NormalizedDotProductKernel);
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "norm_to_use"
  
};

DECLARE_OBJECT_PTR(NormalizedDotProductKernel);


//!  *********************************************************
//!  The following 'kernels' are rather used as cost functions


class SquaredErrorCostFunction: public Kernel
{
protected:
  //!  index in target vector of the target value to use to compute the squared error
  //!  (if -1, sum all the squared errors)
  int targetindex;
  bool classification;
  real hotvalue, coldvalue;

public:
  SquaredErrorCostFunction(int the_targetindex=-1)
    :targetindex(the_targetindex), classification(false), hotvalue(1), coldvalue(0) {};

  //!  Constructor for classification (target is interpreted as onehot)
  SquaredErrorCostFunction(real hot_value, real cold_value)
    : targetindex(-1), classification(true), hotvalue(hot_value), coldvalue(cold_value) {};

  DECLARE_NAME_AND_DEEPCOPY(SquaredErrorCostFunction);
  virtual string info() const;
  virtual real evaluate(const Vec& output, const Vec& target) const; 
    //virtual void readOptionVal(istream& in, const string& optionname);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "targetindex"
  
  static void declareOptions(OptionList &ol);
};

  DECLARE_OBJECT_PTR(SquaredErrorCostFunction);

//! This simply returns -output[0] (target should usually have a length of 0)
//! This is used for density estimators whose use(x) method typically computes log(p(x))
class NegOutputCostFunction: public Kernel
{
public:
  NegOutputCostFunction() {}
  DECLARE_NAME_AND_DEEPCOPY(NegOutputCostFunction);
  virtual real evaluate(const Vec& output, const Vec& target) const; 
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
};

DECLARE_OBJECT_PTR(NegOutputCostFunction);


/*!   * If output and target both have length 1, then binary classification 
    with targets -1 and +1 is assumed and the sign of the output is considered
  * If output has length>1 and target has length 1, then output is understood 
    as giving a score for each class while target is the index of the correct
    class (numbered from 0)
  * If both output and target have a length>1 then then output is understood 
    as giving a score for each class while the correct class is given by 
    argmax(target)
  In any case, evaluation returns 0 if classification was correct, 1 otherwise
*/
class ClassErrorCostFunction: public Kernel
{
  protected:
    bool output_is_classnum;

 public:
/*!       There are several cases:
      1) target is a single value +1 or -1 , and output is a single value whose sign stands for the class
      2) target is a single value in 0..n-1 indicating classnumber and output is a n-dimensional vector of scores
      3) target is a n-dimensional vector whose argmax indicates the class, and output is a n-dimensional vector of scores
      4) target is a single value indicating classnumber, and output is a single value indicating classnumber
      5) target is a single value 0 or 1 , and output is a single value with the threshold 0.5
      Cases 1,2,3 are handled correctly with the default output_is_classnum=false
      For case 4 and 5, you must specify output_is_classnum=true
*/
  ClassErrorCostFunction(bool the_output_is_classnum = false)
    :output_is_classnum(the_output_is_classnum) {}

  DECLARE_NAME_AND_DEEPCOPY(ClassErrorCostFunction);
  virtual string info() const;
  virtual real evaluate(const Vec& output, const Vec& target) const;
    //virtual void readOptionVal(istream& in, const string& optionname);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "output_is_classnum"
  
  static void declareOptions(OptionList &ol);
};

DECLARE_OBJECT_PTR(ClassErrorCostFunction);

/*!
  cost = sum_i class_error_i
*/

class MulticlassErrorCostFunction: public Kernel
{
 public:
  MulticlassErrorCostFunction() {}

  DECLARE_NAME_AND_DEEPCOPY(MulticlassErrorCostFunction);
  virtual string info() const;
  virtual real evaluate(const Vec& output, const Vec& target) const;
    //virtual void readOptionVal(istream& in, const string& optionname);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  
  //static void declareOptions(OptionList &ol);
};

DECLARE_OBJECT_PTR(MulticlassErrorCostFunction);

/*!   * If output and target both have length 1, when binary classification 
    with targets -1 and +1 and the sign of the output is considered,
    then margin is output[0]*target[0]
    If binary targets is {0,1} and outputs>0, then the margin is:
    (output[0]+1)*(target[0]+1)/4
    However, if the flag output_is_positive is true then output is
    replaced by output[0]-0.5 in the above expressions.
  * If output has length>1 and target has length 1, then output is understood 
    as giving a score for each class while target is the index of the correct
    class (numbered from 0). Then margin is the difference between the score 
    of the correct class and the highest score among the other classes.
  * If both output and target have a length>1 then output is understood 
    as giving a score for each class while the correct class is given by 
    argmax(target). Then margin is the difference between the score 
    of the correct class and the highest score among the other classes.
  In all cases, as this is a cost function, we return -margin.
*/
class ClassMarginCostFunction: public Kernel
{
 public:
  bool binary_target_is_01;
  bool output_is_positive;
  ClassMarginCostFunction(bool the_binary_target_is_01=false, 
                          bool out_is_positive=false)
    : binary_target_is_01(the_binary_target_is_01), 
    output_is_positive(out_is_positive) {}
  DECLARE_NAME_AND_DEEPCOPY(ClassMarginCostFunction);
  virtual string info() const;
  virtual real evaluate(const Vec& output, const Vec& target) const;  
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "binary_target_is_01"
  
};

DECLARE_OBJECT_PTR(ClassMarginCostFunction);

/*!   This is very similar to ClassMarginCostFunction when output.length()>1,
  except than rather than return the negated difference between the score
  of the right class and the highest score among all other classes, we
  return:
  WARNING: This doesn't make much sense if scores have different signs (they should either be all positive or all negative)
*/

/*!   This is useful for multiclass problems when the scores represent
  negative distances to each class, It returns
  distance_to_right_class/(distance_to_right_class+distance_to_closest_other_classes)
*/
class ClassDistanceProportionCostFunction: public Kernel
{
public:
  ClassDistanceProportionCostFunction() {}
  DECLARE_NAME_AND_DEEPCOPY(ClassDistanceProportionCostFunction);
  virtual real evaluate(const Vec& output, const Vec& target) const;
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
};

DECLARE_OBJECT_PTR(ClassDistanceProportionCostFunction);

/*!   The target is an integer between 0 and n_classes-1 corresponding
  to a class Y, and the output is a vector of length n_classes 
  of non-negative estimated
  conditional probabilities P(Y=i|X) for class i (i=0 to n_classes-1),
  or if n_classes=2, the output could be a vector of length 1
  containing P(Y=1|X). The cost is
     - log P(Y=target|X).
*/

class NegLogProbCostFunction: public Kernel
{
 public:
  bool normalize;
  bool smooth_map_outputs;
#if USING_MPI
    int out_start;
    int out_end;
#endif
 public:
#if USING_MPI
  NegLogProbCostFunction(bool do_normalize=false, bool do_smooth_map_outputs=false,
                         int outstart=-1, int outend=-1)
    :normalize(do_normalize), smooth_map_outputs(do_smooth_map_outputs),
    out_start(outstart), out_end(outend) {}
#else
  NegLogProbCostFunction(bool do_normalize=false, bool do_smooth_map_outputs=false)
    :normalize(do_normalize), smooth_map_outputs(do_smooth_map_outputs) {}
#endif
  DECLARE_NAME_AND_DEEPCOPY(NegLogProbCostFunction);
  virtual string info() const;
  virtual real evaluate(const Vec& output, const Vec& target) const;
  //virtual void readOptionVal(istream& in, const string& optionname);
  static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized options are "normalize" and "smooth_map_outputs"
  
};

DECLARE_OBJECT_PTR(NegLogProbCostFunction);

//!  A costfunction that allows to reweight another costfunction (weight being last element of target)
//!  Returns target.lastElement() * costfunc(output,target.subVec(0,target.length()-1));
class WeightedCostFunction: public Kernel
{
 protected:
   WeightedCostFunction() : costfunc() {}
  protected:
    Ker costfunc;

  public:
    WeightedCostFunction(Ker the_costfunc)
      :costfunc(the_costfunc) {}

  DECLARE_NAME_AND_DEEPCOPY(WeightedCostFunction);
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual string info() const;
  virtual real evaluate(const Vec& output, const Vec& target) const;
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);   
};

DECLARE_OBJECT_PTR(WeightedCostFunction);

//!  This allows to apply a costfunction on a single output element (and correponding target element)
//!  of a larger output vector, rather than on the whole vector
class SelectedOutputCostFunction: public Kernel
{
 protected:
   SelectedOutputCostFunction() : costfunc(), outputindex() {}
  protected:
    Ker costfunc;
    int outputindex;

  public:
    SelectedOutputCostFunction(Ker the_costfunc, int the_outputindex)
      :costfunc(the_costfunc), outputindex(the_outputindex) {}

  DECLARE_NAME_AND_DEEPCOPY(SelectedOutputCostFunction);
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual string info() const;
  virtual real evaluate(const Vec& output, const Vec& target) const; 
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "outputindex"
       
};

DECLARE_OBJECT_PTR(SelectedOutputCostFunction);

/*!   This class allows us to compute the lift (used by Bell Canada) of
  a binary classification problem.  Since we need the output AND the target
  to be represented by a single cost (the method evaluate returns only a
  real), we will use a trick.
  cost =  output[class#1], if target = 1
       = -output[class#1], if target = 0
  or, if the make_positive_output flag is true:
  cost =  sigmoid(output[class#1]), if target = 1
       = -sigmoid(output[class#1]), if target = 0
*/
class LiftBinaryCostFunction: public Kernel
{
  DECLARE_NAME_AND_DEEPCOPY(LiftBinaryCostFunction);
 protected:
  bool make_positive_output;
 public:
  LiftBinaryCostFunction(bool make_pos_output=false) : make_positive_output(make_pos_output) {}
  virtual string info() const;
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "make_positive_output"
           
  virtual real evaluate(const Vec& output, const Vec& target) const;

};

DECLARE_OBJECT_PTR(LiftBinaryCostFunction);

class DirectNegativeCostFunction: public Kernel
{
  DECLARE_NAME_AND_DEEPCOPY(DirectNegativeCostFunction);
 public:
  DirectNegativeCostFunction(){}
  virtual string info() const;
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);        
  virtual real evaluate(const Vec& output, const Vec& target) const;

};

/*!   ************
  * CostFunc *
  ************
*/

//!  a cost function maps (output,target) to a loss
typedef Ker CostFunc;

/*!   **********************************************************************
                             FINANCIAL STUFF
  **********************************************************************
*/

//!  a profit function maps (output,target) to a profit
typedef CostFunc ProfitFunc;

/*!   cost function that takes (output,target) as arguments, transforms
  them into a profit using a given profit_function,
  and returns the negative of quadratic utility = - profit + risk_aversion * profit * profit;
*/
class QuadraticUtilityCostFunction : public Kernel
{
  protected:
    real risk_aversion;
    ProfitFunc profit_function;
    QuadraticUtilityCostFunction(){}
  public:
    QuadraticUtilityCostFunction(real the_risk_aversion, CostFunc the_profit)
      : risk_aversion(the_risk_aversion), profit_function(the_profit)
    {}

    DECLARE_NAME_AND_DEEPCOPY(QuadraticUtilityCostFunction);
    
    virtual string info() const;
    virtual real evaluate(const Vec& output, const Vec& target) const; 
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
    virtual void write(ostream& out) const;
    virtual void oldread(istream& in);
/*!       Recognized option: "risk_aversion".
      ALSO: options of the form "profit_function.XXX" are passed to the
      profit_function kernel as "XXX".
*/
    
};

/*!   profit function that takes (output,target) as arguments
  and computes a profit. The output has at least 2 elements
  which represent the number of units of the security bought
  (or sold, if <0), and the cash change due to the transaction at time t1. The target vector
  contains at least one element which represents the price of
  the security at a later time t2.
  Profit is computed as follows:
    transaction_loss_t2 =  nb_units_transaction>0 ? 
                           additive_cost + multiplicative_cost * |nb_units_transaction| * price_t2 : 0
    profit = cash_earned_at_t1 + nb_units_transaction * price_t2 - transaction_loss_t2;
  
  where additive_cost, multiplicative_cost, per_unit_cost
  are user-specified parameters that control transaction losses
  
*/
class PricingTransactionPairProfitFunction : public Kernel
{
  protected:
    real multiplicative_cost; //!<  transaction loss
    real additive_cost; //!<  transaction loss
    real per_unit_cost; //!<  transaction loss
    PricingTransactionPairProfitFunction(){}
  public:
    PricingTransactionPairProfitFunction(real the_multiplicative_cost, 
                                         real the_additive_cost=0,
                                         real the_per_unit_cost=0) :
      multiplicative_cost(the_multiplicative_cost), additive_cost(the_additive_cost),
      per_unit_cost(the_per_unit_cost) {}
    
    DECLARE_NAME_AND_DEEPCOPY(PricingTransactionPairProfitFunction);
    
    virtual string info() const;
    virtual real evaluate(const Vec& output, const Vec& target) const; 
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
    virtual void write(ostream& out) const;
    virtual void oldread(istream& in);
    //!  Recognized options: all 3 fields.
    
};

//!  **********************************************************************

inline CostFunc squared_error(int singleoutputindex=-1)
{ 
  if(singleoutputindex>=0)
    return new SelectedOutputCostFunction(new SquaredErrorCostFunction(),singleoutputindex); 
  else
    return new SquaredErrorCostFunction();
}

inline CostFunc squared_classification_error(real hot_value=0.8, real cold_value=0.2)
{
  return new SquaredErrorCostFunction(hot_value, cold_value);
}

inline CostFunc absolute_deviation(int singleoutputindex=-1) 
{ 
  if(singleoutputindex>=0)
    return new SelectedOutputCostFunction(new DistanceKernel(1.0),singleoutputindex); 
  else
    return new DistanceKernel(1.0); 
}

inline CostFunc output_minus_target(int singleoutputindex=-1)
{
  if(singleoutputindex>=0)
    return new SelectedOutputCostFunction(new DifferenceKernel(),singleoutputindex); 
  else
    return new DifferenceKernel(); 
}

inline CostFunc quadratic_risk(real risk_aversion, CostFunc profit_function)
{
  return new QuadraticUtilityCostFunction(risk_aversion, profit_function);
}

//!  classification error
inline CostFunc class_error(bool output_is_classnum=false) { return new ClassErrorCostFunction(output_is_classnum); } 
//!  difference between correct class score and max of other class' scores
inline CostFunc class_margin(bool binary_target_is_01=false, 
                             bool output_is_positive=false) 
{ return new ClassMarginCostFunction(binary_target_is_01,output_is_positive); }
//!  if outputs are neg distances to each class: dist_to_correct_class/(dist_to_correct_class+dist_to_closest_other_class)
inline CostFunc class_distance_proportion() { return new ClassDistanceProportionCostFunction(); }

inline CostFunc class_lift(bool make_positive=false) 
{ return new LiftBinaryCostFunction(make_positive); }

//!  negative log conditional probability 
#if USING_MPI
inline CostFunc condprob_cost(bool normalize=false, bool smooth_map_outputs=false,
                              int outstart=-1, int outend=-1)
{ return new NegLogProbCostFunction(normalize,smooth_map_outputs,outstart,outend); }
#else
inline CostFunc condprob_cost(bool normalize=false, bool smooth_map_outputs=false)
{ return new NegLogProbCostFunction(normalize,smooth_map_outputs); }
#endif

//! returns -output[0]. This is for density estimators whose use(x) method typically computes log(p(x))
inline CostFunc neg_output_costfunc()
{ return new NegOutputCostFunction(); }

//!  reweighting
inline CostFunc weighted_costfunc(CostFunc costfunc)
{ return new WeightedCostFunction(costfunc); }

inline CostFunc directnegative_costfunc()
{ return new DirectNegativeCostFunction(); }

//!  ********************
//!  inline Ker operators

inline Ker operator-(const Ker& k)
{ return new NegKernel(k); }

inline Array<Ker> operator&(const Ker& k1, const Ker& k2)
{ return Array<Ker>(k1,k2); }


/*!   *****************
  * KernelVMatrix *
  *****************
*/

/*!   This VMat is built from 2 other (possibly identical)
  VMats and a Kernel K.
  The resulting VMat takes every sample (x_i,y_i) 
*/

class KernelVMatrix: public VMatrix
{
 //protected:
 //!   KernelVMatrix() : d1(), d2(), ker(), input1(), input2() {}
protected:
  VMat d1;
  VMat d2;
  Ker ker;

  Vec input1;
  Vec input2;

public:
  KernelVMatrix(VMat data1, VMat data2, Ker the_ker);

  //DECLARE_NAME_AND_DEEPCOPY(KernelVMatrix);
  //virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
};

Mat findClosestPairsOfDifferentClass(int k, VMat data, Ker dist);

template <> void deepCopyField(Ker& field, CopiesMap& copies);

%> // end of namespace PLearn

#endif

