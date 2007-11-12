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
   * $Id$
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */

/* RandomVar.h

   Random Variables package:

   RandomVar class, helper classes such as RandomVariable,
   RVInstance, RVArray, RVInstanceArray, ConditionalExpression,
   and many subclasses of RandomVariable,
   plus helper global functions and operators.

 */

/* TUTORIAL ON THE RANDOMVAR PACKAGE

   Whereas a Var represents a variable in the mathematical sense, 
   a RandomVar represents a random variable in the mathematical sense. 
   A random variable is generally defined in terms of other random variables
   through deterministic transformations, or in terms of other random
   variables which are the parameters of a its distribution.
   A RandomVar represents a node in a graphical
   model, and its distribution is defined in terms
   of the values of its parents in the model.
   For example, its parents may be the parameter
   of its distribution or it may be the variables
   which when combined deterministically give rise
   to its value. The set of classes provided here
   allow to build such a network of random variables,
   and to make limited inferences, probability computations,
   gradient computations, and learning.

   Examples of use of RandomVars:

   Var u(1),lv(1); // constants, but changing their value will change
   Var w; // the distributions associated to the RandomVar's
   ...
   // things like *, +, log, tanh, etc... must do the proper thing for RandomVars
   RandomVar X = gamma(0.5)*log(normal(u,lv));
   RandomVar Y = tanh(product(w,X) + u);
   RandomVar LW(2); // unnormalized log-weights of the mixture
   RandomVar Z = mixture(Y&X,LW); 

   // see the comment on operator&, operator[] and mixtures below
   ...
   // (conditionned on the RHS of the |)
   Vec x,y; // put some value in x and y
   Vec z = sample(Z|(X==x && Y==y));
   // This is achieved by redefining operator==, operator&& and operator|
   // to represent the data structure which the function sample
   // takes as argument. In particular, note that == creates the
   // RVInstance data structure (which contains a RV and a Var instance),
   // while && or & makes an array of these structures (RVInstanceArray), and
   // the "|",  builds a ConditionalExpression, that contains a RVInstanceArray
   // for the left hand side and a RVInstanceArray for the right hand side. Note
   // the use of parentheses because of the default precedence of operators.
   ...
   // Using these operators, you can also express the computation of the probability of
   // a value for the random variable, or its conditional probability. In fact the
   // statement below defines a functional relationship (in the usual "Variable"
   // sense between the variable y and the variable p or log_p). Note that
   // no actual numerical value has yet been computed. 
   Var y(1);
   Var p = P(Y==y);
   Var log_p = logP(Y==y);
   // or
   Var y,x;
   Var log_p = logP((Y==y)|(X==x)); // note again ()'s for precedence
   // e.g. of use
   Vec actual_value_for_x_and_y = ...;
   Func f = log_p(x&y);
   real prob = f(actual_value_for_x_and_y)[0];
   cout << "log(P(Y|X))=" << prob << endl;
   ...
   // in the case of discrete distributions, the whole distribution can be returned
   // by P and logP, which are also defined on RandomVars.
   Vec p = P(Y);
   Vec log_p = logP(Y);
   ...
   // note that if Y has RandomVar parents, the above var can only be computed
   // if these parents are given a particular value or if
   // they are integrated over (with the function marginalize below).
   // This call will therefore automatically try to marginalize Y
   // (by integrating over the parents which are not observed).
   // To make a RandomVar observed, simply use the conditioning
   // notation V|(X==x && Y==y && Z==z), e.g. to condition V on X=x,Y=y,Z=z.
   ...
   // Similarly to P, logP, and sample, other functions of RVs are defined:
   // construct a Var that is functionally dependent on the Var x
   // and represents the expectation of Y given that X==x.
   Var e=E(Y|(X==x));
   // Similarly for covariance matrix:
   Var v=V(Y|(X==x && Z==z));
   // and for the cumulative distribution function
   // (which here depends on the Vars y and x)
   Var c=P((Y<y)|(X==x));
   // Note that derivatives through all these functional relationships
   // can automatically be computed. For example, to compute the
   // gradients of the log-probability wrt some parameters W & B & LogVariance
   RandomVar W,B,X,LogVariance; // all are "non-random", X is the input
   Var w,b,x,lv; // values that the above will take
   // e.g. to give values to these Vars
   w[0]=1; b[0]=0;x[0]=3;lv[0]=0;
   RandomVar Y=normal(W*X+B,LV); // the model (i.e. a regression)
   // establish the functional relationship of interest,
   // which goes from (y & x & w & b & lv) to logp:
   Var logp = logP((Y==y)|(X==x && W==w && B==b && LV==lv));
   // to actually compute logp, do
   logp->fprop_from_all_sources(); // a source is "constant" Var
   // to compute gradients, use the propagationPath function to find
   // the path of Vars from, say w&b, to logp:
   VarArray prop_path = propagationPath(w&b,logp);
   prop_path.bprop(); // compute dlogP/dparams in w->gradient and b->gradient
   ...
   // By default a RandomVar represents a "non-random" random variable 
   // (of class NonRandomVariable or a FunctionalRandomVariable which depends
   // only on NonRandomVariable). This is not the same as a "constant"
   // variable. It only means that its value is deterministic, but
   // its value may be a (Var) function of other Vars:
   RandomVar X;
   X->value = 1+exp(y+w*z); // y, w and z are Vars here
   ...
   // Parameters of the distributions that define the random variables can be
   // learned, most generally by optimizing a criterion to optimize, e.g.
   Mat observed_xy_matrix; // each row = concatenation of an X and a Y obs.
   Var cost =  -logP((Y==y)|(X==x && Params==params));
   // below, establish the functional relationship between x & y & params
   // and the "totalcost" which is the sum of "cost" when x & y are
   // "sampled" from the given empirical distribution.
   Var totalcost = meanOf(cost,x&y,VMat(observed_xy_matrix),
                          observed_xy_matrix.length(),
                          params);
   // construct an optimizer that optimizes totalcost by varying params
   ConjugateGradientOptimizer opt(params,totalcost, 1e-5, 1e-3, 100);
   // do the actual optimization
   real train_NLL = opt.optimize();
   // now we can test on a particular point setting values for
   // x and y and params and doing an fprop with
   propagationPath(x&y&params,cost).fprop();
   ...
   // Sometimes, the parameters can be estimated more efficiently
   // using an internal mechanism for estimation (usually the analytical 
   // solution of maximum likelihood or the EM algorithm):
   real avgNegLogLik=EM(Y|X,W&B&LV,VMat(observed_xy_matrix),
                         observed_xy_matrix.length(),4,0.001);
   // where the first argument specifies which conditional distribution
   // is of interest (ignoring the parameters), the second argument
   // gives the parameters to estimate, and the third one specifies
   // a training set. Note that the order of the variables in the 
   // observed_xy_matrix must be (1) inputs: all the variables on the RHS 
   // of the conditioning |, (2) outputs: all the variables on the LHS of the |.
   ...
   // Arrays of RVs can be formed with the operator &:
   RVArray a = X & Y & V;
   // and they can be automatically cast into JointRandomVariables
   // (whose value is the concatenation of the values of its parents)
   RandomVar Z = X & Y & V;
   ...
   // Marginals can sometimes be obtained (when X is discrete or 
   // the integral is feasible analytically and the code knows how to do it...).
   // For example, suppose X is one of the parents of Y. Then
   RandomVar mY = marginalize(Y,X);
   // is a random variable such that P(mY=y) = int_x P(Y=y|X=x)P(X=x) dx
   // this is obtained by summing over the values of X if it is discrete,
   // by doing the integral if we know how to do it, or otherwise, by the
   // Laplace approximation, or by some numerical integration method
   // such as Monte-Carlo.
   ...
   // The operator() is defined on RandomVar as follows:
   // If i is an integer, X(i) extracts a RandomVar that is scalar 
   // and corresponds to the i-th element of the vector random variable X 
   // (similarly, if the underlying Var is a Var, X(i,j), 
   // extracts the random element (i,j)). These two operators
   // are also defined for the case in which the index is a Var
   // (treated like the integer), and the case in which it is a RandomVar.
   // The last case, X(I), actually represents a mixture of the elements
   // of the vector X, with weights given by the parameters of I
   // (which must be discrete).
   ...
   // The operator[] is defined on RVArrays
   // and it allows to extract the i-th random variable in the array.
   // With I a RandomVar and A an RVArray, A[I], is very interesting because 
   // it represents the graphical model of a mixture in which I is the index,
   // and it is not (yet) integrated over. 
   RVArray A(3);
   A[0]=X; A[1]=Y; A[2]=Z;
   RandomVar XYZ(A); // joint distribution
   // or equivalently
   RandomVar XYZ = X & Y & Z;
   ...
   // A MultinomialRandomVariable is a subclass of RandomVariable
   // that represents discrete-valued variables in the range 0 ... N-1.
   RandomVar LW(3); // unnormalized log-probabilities of I
   RandomVar I = multinomial(LW); // N=3 possible values here
   // The parameters of a discrete random variable are the "log-probabilities" 
   // (more precisely the discrete probabilities are obtained with a softmax 
   // of the parameters, LW here). The discrete random variable will be 
   // conditional if LW is not a NonRandomVariable but rather depends
   // on some other RVs.
   ...
   // Let us consider a random variable that is obtained by selecting
   // one of several random variables (on the same space). We call
   // such a random variable an RVArrayRandomElementRandomVariable and it is 
   // obtained with the operator[] acting on a RVArray, 
   // with a discrete randomvariable as argument:
   RandomVar V = A[I]; // will take either the distribution of X, Y or Z according to value of I
   // therefore P(V=v|I=i)=P(A[i]=v)
   ...
   // A mixture is the marginalization of an IndexedRandomVariable with
   // respect to the random index:
   RandomVar LW(3); // unnormalized log-weights of the mixture
   RandomVar M = mixture(a,LW);
   // which is exactly the same thing as
   RandomVar I = multinomial(LW); 
   RandomVar M = marginalize(A[I],I);
   ...
   // Example of conditional mixture of n d-dimensional diagonal 
   // Gaussians with neural network expectations:
   // (1) define the neural network
   RandomVar X(n_inputs); // X is the network input RV
   Var x(n_inputs); // x will be its value
   int n_inputs=4, n_hidden=5,n_outputs=d*n;
   Var layer1W(n_hidden,n_inputs), layer2W(n_outputs,n_hidden);
   Var layer1bias(n_hidden), layer2bias(n_outputs);
   RandomVar NetOutput = layer2bias+layer2W*tanh(layer1bias+layer1W*X);
   // (2) define the gaussian mixture
   // (2.1) define the gaussians
   RVArray normals(n);
   RVArray mu(n),logsigma(n);
   for (int i=0;i<n;i++) {
     mu[i]=NetOutput.subVec(i*d,d); // extract subvector as i-th mean vector
     normals[i]=Normal(mu[i],logsigma[i]);
   }
   // (2.2) build the mixture itself
   Var lw(n);
   lw->value.fill(1.0/n);
   RandomVar Y = mixture(normals,lw); //the "target" output random variable
   Var y(d); // its value
   VarArray tunable_parameters = 
      lw & layer1W & layer2W & layer1bias & layer2bias;
   // each row of Mat is the concatenation of an x (input) and a y 
   Mat observed_xy_matrix; 
   // logP returns the path that goes from all "source" (constant) variables
   // into the computation of the given conditional probability
   Var cost =  -logP((Y==y)|(X==x)); 
   // note that we don't need to condition on the "non-random" parameters
   // such as the log-weights of the mixture (lw), but they will
   // occur as tunable parameters.
   // Below, the order of x and y in the observed_xy_matrix must
   // match their order in the second argument of meanOf.
   Var totalcost = meanOf(cost,x&y,VMat(observed_xy_matrix),
                          observed_xy_matrix.length(),tunable_parameters);
   ...
   // Example in which some parameters W & B have to be fitted
   // to some data, while the hyper-parameters gamma that control
   // the distribution of W & B should be fitted to maximize
   // the likelihood of the data.
   int npoints = 10; // there are 10 (x,y) pairs in each observation
   Var muW, logvarW, muB, logvarB; // parameters of the prior
   RandomVar W = normal(muW,logvarW); // prior on W
   RandomVar B = normal(muB,logvarB); // prior on B
   Var log_var(1); // log-variance of Y
   Var ones(npoints);
   ones->value.fill(1.0);
   Var log_vars = ones*log_var; // make vector of npoints copies of log_var
   Var x(npoints); // input
   Var muXint, logvXint; // parameters of Xinterval
   RandomVar Xinterval = normal(muXint,logvXint); // prior on Xinterval
   RandomVar Y = normal(tanh(W*x+B),log_vars); 
   Var cost = -logP(Y==y && Xinterval==vconcat(min(x) & max(x));
   // note that the above requires marginalizing over W & B
   VarArray gamma = muXint & logvXint & log_var & muW & logvarW & muB & logvarB;
   Var totalcost = meanOf(cost,x&y,VMat(observed_xy_matrix),
                          observed_xy_matrix.length(), gamma);
   ConjugateGradientOptimizer opt(gamma,totalcost, 1e-5, 1e-3, 100);
   real train_NLL = opt.optimize();
   // to obtain a fit of Theta for a particular value of x and y, optimize
   Var w,b;
   Var fitcost = 
     -logP((Y==y && Xinterval==vconcat(min(x) & max(x)))|(W==w && B==b));
   ConjugateGradientOptimizer fitopt(w & b,fitcost, 1e-5, 1e-3, 100);
   real fit_NLL = fitopt.optimize();
   // and the fitted parameters can be read in w->value and b->value.
*/


/*! \file RandomVar.h */

#ifndef RANDOMVAR_INC
#define RANDOMVAR_INC

#include <plearn/opt/Optimizer.h>

//!  used only for implementing the sampling Var's in the setValueFromParentsValue 
//!  methods of StochasticRandomVariable's.
#include "SampleVariable.h"

//!  used only for defining RandomVarVMatrix
#include <plearn/vmat/VMat.h>

namespace PLearn {
using namespace std;


class RandomVariable;
class RVArray;
class RVInstance;
class RVInstanceArray;
class ConditionalExpression;

//!  we follow the same pattern as Var & Variable
class RandomVar: public PP<RandomVariable>
{
 public:
  RandomVar();
  RandomVar(int length, int width=1);
  RandomVar(RandomVariable* v);
  RandomVar(const RandomVar& other);

  RandomVar(const Vec& vec);
  RandomVar(const Mat& mat);
  RandomVar(const Var& var);

  RandomVar(const RVArray& vars); //!<  make joint distribution
 
  RandomVar operator[](RandomVar index); //!<  extract from joint or element of vector
#if 0
  RandomVar operator[](int i); //!<  same but index is fixed
#endif

  void operator=(const RVArray& vars);

  //!  these two only work with NonRandomVariables
  void operator=(real f); //!<  fill value to f
  void operator=(const Vec& v); //!<  copy v into value
  void operator=(const Mat& m);
  void operator=(const Var& v); //!<  assign value to v

  //!  associate a RV to an observation v,
  //!  as in expressions of the form (V==v)
  RVInstance operator==(const Var& v) const;

  //!  compare two RandomVars: == if same underlying RandomVariable (same address)
  bool operator==(const RandomVar& rv) const { return rv.ptr == this->ptr; }
  bool operator!=(const RandomVar& rv) const { return rv.ptr != this->ptr; }

  //!  make an array of RV's, as in expressions like X&Y
  RVArray operator&(const RandomVar& v) const;

/*!     This operator represents expressions of the form Y|(X&Z). It is
    almost the same as & (i.e. it also builds a RVArray), but it allows
    us to know that the LHS of the |
    will always be the first element of the array, so if the user types
    ((Y&Z)|(X&U))
    the result will be a 3-element array, because the first two-element
    array created by (Y&Z) is coerced into a JointRV, which will
    be the first element of the result.
*/
  ConditionalExpression operator|(RVArray rhs) const;

  //!  This operator represents expressions of the form Y|(X==x && Z==z)
  ConditionalExpression operator|(RVInstanceArray rhs) const;

#if 0
  //!  NOT IMPLEMENTED YET
  //!  take i-th row
  RandomVar operator[](RandomVar index);
  RandomVar operator[](int i);
  //!  take element i,j
  RandomVar operator()(RandomVar i, RandomVar j);
  RandomVar operator()(int i, int j);
#endif

};

typedef RandomVar MatRandomVar;


//!  An RVArray stores a table of RandomVar's
class RVArray: public Array<RandomVar>
{
 public:
  RVArray();
  RVArray(int n, int n_extra_allocated=0);
  RVArray(const Array<RandomVar>& va);
  RVArray(const RandomVar& v, int n_extra_allocated=0);
  RVArray(const RandomVar& v1, const RandomVar& v2, int n_extra_allocated=0);
  RVArray(const RandomVar& v1, const RandomVar& v2, const RandomVar& v3, 
          int n_extra_allocated=0);

  int length() const;

  //!  return the VarArray of values of the RV's
  VarArray values() const;

  //!  make a long array of RV's, as in expressions like X&Y&Z
  //!  RVArray operator&(const RandomVar& v); //!<  already built-in Arrays

  //!  Note that casting a RVArray to RandomVar makes a JointRandomVariable
  //!  because of RandomVar(RVAarray) constructor.

  //!  return a new RVArrayRandomElementRandomVariable
  RandomVar operator[](RandomVar index);

  RandomVar& operator[](int i)
    { return Array<RandomVar>::operator[](i); }

  const RandomVar& operator[](int i) const
    { return Array<RandomVar>::operator[](i); }

  static int compareRVnumbers(const RandomVar* v1, const RandomVar* v2);

  //!  sorts in-place the elements by rv_number (topological order of 
  //!  the graphical model) (in the order: ancestors -> descendants)
  void sort();
};


//!  RVInstance represents a RandomVariable V along with a "value" v
class RVInstance 
{
 public:
  RandomVar V;
  Var v;

  RVInstance(const RandomVar& VV, const Var& vv);
  RVInstance();

  RVInstanceArray operator&&(RVInstance rvi);

  ConditionalExpression operator|(RVInstanceArray a);

  //!  swap the v with the V->value
  void swap_v_and_Vvalue();

};

class RVInstanceArray: public Array<RVInstance>
{
 public:
  RVInstanceArray();
  RVInstanceArray(int n, int n_extra_allocated=0);
  RVInstanceArray(const Array<RVInstance>& a);
  RVInstanceArray(const RVInstance& v, int n_extra_allocated=0);
  RVInstanceArray(const RVInstance& v1, const RVInstance& v2,
                  int n_extra_allocated=0);
  RVInstanceArray(const RVInstance& v1, const RVInstance& v2, 
                  const RVInstance& v3, int n_extra_allocated=0);

  //!  total length of all the RV's in the array, this is generally != size()
  int length() const; 

  //!  Build a chain of (RV,value) pairs.
  //!  It is used to build expressions of the form (X==x && Y==y && Z==z).
  RVInstanceArray operator&&(RVInstance rhs);

/*!     This operator is used for expressions of the form (Y==y)|(X==x && Z==z)
    and it acts like &&. If the expression is 
       (Y==y && Z==z)|(X==x && U==u)
    then the LHS RVInstance list will be coerced into a single
    (RV,value) pair, with RV = Y&&Z (their joint) and 
    value = vconcat(y & z).
*/
  ConditionalExpression operator|(RVInstanceArray rhs);

  //!  put all the RVs in the list in a RVarray
  RVArray random_variables() const;

  //!  put all the values of the variables (array[i].V->value) in a VarArray
  VarArray values() const;
  //!  put all the instance values of the variables (array[i].v) in a VarArray
  VarArray instances() const;

  //!  swap the v's with the V->value's
  void swap_v_and_Vvalue()
    { for (int i=0;i<size();i++) (*this)[i].swap_v_and_Vvalue(); }

  static int compareRVnumbers(const RVInstance* rvi1, const RVInstance* rvi2);

  //!  sorts in-place the elements by V->rv_number (topological order of 
  //!  the graphical model) (in the order: ancestors -> descendants)
  void sort();

};

class ConditionalExpression
{
 public:
  RVInstance LHS;
  RVInstanceArray RHS;

  //!  build from both LHS and RHS
  ConditionalExpression(RVInstance lhs, RVInstanceArray rhs);
  //!  build from just LHS
  ConditionalExpression(RVInstance lhs);
  //!  build from just LHS's RV
  ConditionalExpression(RandomVar lhs);
  //!  build from multiple LHS RVInstances: make one RVInstance
  //!  from the joint of the RVs and the vconcat of the instances.
  ConditionalExpression(RVInstanceArray lhs);
};

class RandomVariable: public PPointable
{
  friend class RandomVar;
  friend class RVInstanceArray;
  friend class RVArray;

  static int rv_counter; //!<  used to assign a value to rv_number
                         //!  when a new RV is constructed

protected:

  //!  rv_number is used to sort RVs in topological order, knowing
  //!  that parents must be created before their descendents.
  const int rv_number; //!<  takes the value rv_counter upon construction

 public:
  const RVArray parents; //!<  other random variables, whose value give rise
/*!                       stochastically or deterministically to this value.
                     This field basically defines the NETWORK of the model.
                     Note that this oriented graphical model must not have 
                     cycles.
*/
  Var value; //!<  used either when marked, non-random, or to assess P(This=value)
  
 protected:
/*!     temporary used in various procedures that traverse the
    graphical model. For example, for logP it means 
    "conditionally non-random".
*/
  bool marked;

  bool EMmark; //!<  mark used by EM to avoid repeated calls to EMBprop/EMUpdate
  bool pmark; //!<  yet another mark 

  //!  learn_the_parameters[i] says if parent[i] should be learned
  //!  in the current call to EM (as a parameter of the distribution).
  bool* learn_the_parameters;

 public:
  //!  All these constructors give rise to a RV with no parents
  RandomVariable(int thelength, int thewidth=1);
  RandomVariable(const Vec& the_value);
  RandomVariable(const Mat& the_value);
  RandomVariable(const Var& the_value);
  //!  these constructors give rise to a RV with parents
  RandomVariable(const RVArray& parents, int thelength);
  RandomVariable(const RVArray& parents, int thelength, int thewidth);
  
  virtual char* classname() = 0;

  virtual int length() { return value->length(); }
  virtual int width() { return value->width(); }
  int nelems() { return value->nelems(); }
  bool isScalar() { return length()==1 && width()==1; }
  bool isVec() { return width()==1 || length()==1; }
  bool isColumnVec() { return width()==1; }
  bool isRowVec() { return length()==1; }

/*!     Random means that its value cannot be known deterministically.
    Note that StochasticRandomVariable's are never non-random,
    but a FunctionalRandomVariable is non-random if it has
    no parents (i.e. it is a NonRandomVariable) or if all its
    parents are non-random. 
*/
  virtual bool isNonRandom() = 0;
  //!  non-random is not the same thing as constant:
  //!  (a Var is constant if it does not depend on other vars)
  inline bool isConstant() { return isNonRandom() && value->isConstant(); }

  //!  true if a discrete random variable, false otherwise.
  //!  Most of the RVs of interest here are continuous, so this is the default.
  virtual bool isDiscrete() = 0;

/*!     Return a new RandomVar which is obtained by extracting
    a sub-vector of length "length" from the value of this, starting at
    position "start".
*/
  RandomVar subVec(int start, int length);

  ////////////////////////////////////////////////////////!<  
/*!     ALL BELOW THIS IS NOT NECESSARY FOR ORDINARY USERS <  
    but may be necessary when writing subclasses. Note <  
    however that normally the subclasses should not be <  
    direct subclasses of RandomVariable but rather be  <  
    subclasses of StochasticRandomVariable and of      <  
    FunctionalRandomVariable.
*/
  ////////////////////////////////////////////////////////!<  

/*!     define the formula that gives a value to this RV given its parent's value
    (sets the value field). If the RV is stochastic, the formula
    may also be "stochastic" (using SampleVariable's to define the Var).
*/
  virtual void setValueFromParentsValue() = 0;

  void markRHSandSetKnownValues(const RVInstanceArray& RHS)
    {
      for (int i=0;i<RHS.size();i++)
        RHS[i].V->mark(RHS[i].v);
      setKnownValues();
    }
      
/*!     ************ EM STUFF **********
    propagate posterior information to parents in order
    to perform an EMupdate at the end of an EMEpoch.
    In the case of mixture-like RVs and their components, 
    the posterior is the probability of the component "this"
    given the observation "obs". 
*/
  virtual void EMBprop(const Vec obs, real posterior) = 0;

/*!     update the fixed (non-random) parameters
    using internal learning mechanism, at end of an EMEpoch.
    the default just propagates to the unmarked parents.
*/
  virtual void EMUpdate();

/*!     Has the distribution seen enough EM iterations
    to meaningfully stop EM iterative training?
    This is a way for a RandomVariable sub-class to force continuation
    of the EM iterations beyond the criteria given by the caller of EM.
    the default just propagates to the unmarked parents.
*/
  virtual bool canStopEM();

  //!  Initialization of EM training (before all the iterations start).
  //!  the default just propagates to the unmarked parents
  virtual void EMTrainingInitialize(const RVArray& parameters_to_learn);

  //!  Initialization of an individual EMEpoch.
  //!  the default just propagates to the unmarked parents
  virtual void EMEpochInitialize();

  ////////////////////////////////////////////////////////////////////////////!<  
  //!  ALL BELOW THIS IS NOT NECESSARY FOR ORDINARY USERS OR SUBCLASS WRITERS //!<  
  ////////////////////////////////////////////////////////////////////////////!<  

  virtual void mark(Var v) { marked = true; value = v; }
  virtual void mark() { marked = true; }
  virtual void unmark() { marked = false; }
  virtual void clearEMmarks();

  //!  clear not only the marked field but also that of parents
  virtual void unmarkAncestors();

  virtual bool isMarked() { return marked; }

/*!     traverse the graph of ancestors of this node
    and mark nodes which are deterministic descendents of marked nodes
    while setting their "value" field as a function of their parents.
*/
  virtual void setKnownValues();

/*!     Construct a Var that computes logP(This = obs | RHS ).
    This function SHOULD NOT be used directly, but is called
    by the global function logP (same argument), which does
    proper massaging of the network before and after this call.
*/
  virtual Var logP(const Var& obs, const RVInstanceArray& RHS,
      RVInstanceArray* parameters_to_learn=0) = 0;
  virtual Var P(const Var& obs, const RVInstanceArray& RHS);

/*!     the latter is like logP but it represents the expected log-probability
    of obs given the RHS, where the expectation is over the "hidden" random 
    variables of EM in mixtures, as a function of the values of 
    the parameters_to_learn.
*/
  virtual Var ElogP(const Var& obs, RVArray& parameters_to_learn, 
                    const RVInstanceArray& RHS);

/*!     Ordinary users don't need to use this method. Use
    the global function EM instead.
    Train with EM or analytical maximization of likelihood, and
    return training negative log-likelihood. The VMat provides
    the n_samples training examples, and EM is applied for
    at most max_n_iterations or until the relative improvement
    in neg-log-likelihood (improvement / previous_value) is
    less than relative_improvement_threshold. This method
    should in general NOT be redefined. Here prop_path
    contains the propagation path of Var's from the
    observedVars (sampled from VMat) to the negative-log-probability
    of the variable whose likelihood should be maximized.
*/
  ///////////////////////////////////////////////////////////!<  
/*!     NOTE NOTE NOTE:
    
    THE ORDER OF THE VALUES IN THE DISTRIBUTION MUST BE:
    (1) conditioning variables (RHS), (2) output variables
*/
  ///////////////////////////////////////////////////////////!<  
  virtual real EM(const RVArray& parameters_to_learn,
                   VarArray& prop_path, 
                   VarArray& observedVars, 
                   VMat distr, int n_samples, 
                   int max_n_iterations, 
                   real relative_improvement_threshold,
                   bool accept_worsening_likelihood=false);

/*!     Do one iteration of EM or compute the likelihood of the given data. 
    This method should not be redefined
    for most types of random variables, but it may be necessary
    for things like sequential data. This method can be used for
    doing one epoch of EM (do_EM_learning=true) or simply 
    for computing the likelihood, if do_EM_learning=false.
*/
  virtual real epoch(VarArray& prop_path, 
                      VarArray& observed_vars, const VMat& distr, 
                      int n_samples, 
                      bool do_EM_learning=true);

  virtual ~RandomVariable();

};


//!  ********************** GLOBAL FUNCTIONS ********************** //!<  

/*!   Return a RandomVar that is the product of two RandomVar's.
  If a and b are matrices, this is a matrix product. If one
  of them is a vector it is interpreted as a column vector (nx1),
  but if both are vectors, this is a dot product (a is interpreted
  as a 1xn). The result contains a ProductRandomVariable, which
  can be "trained" by EM: if one of the two arguments is
  non-random and is considered to be a parameter, it can
  be learned (e.g. for implementing a linear regression).
*/
RandomVar operator*(RandomVar a, RandomVar b);

/*!   Return a RandomVar that is the element-by-element sum of two RandomVar's.
  The result contains a PlusRandomVariable, which
  can be "trained" by EM: if one of the two arguments is
  non-random and is considered to be a parameter, it can
  be learned (e.g. for implementing a linear regression).
*/
RandomVar operator+(RandomVar a, RandomVar b);

//!  Return a MatRandomVar that is the element-by-element difference of two RandomVar's.
//!  The result contains a MinusRandomVariable.
RandomVar operator-(RandomVar a, RandomVar b);

//!  Return a MatRandomVar that is the element-by-element ratio of two RandomVar's.
//!  The result contains a RandomVariable.
RandomVar operator/(RandomVar a, RandomVar b);

//!  exponential function applied element-by-element
RandomVar exp(RandomVar x);

//!  natural logarithm function applied element-by-element
RandomVar log(RandomVar x);

RandomVar extend(RandomVar v, real extension_value = 1.0, int n_extend = 1);

RandomVar hconcat(const RVArray& a);

/*!   Train a part of the graphical model given by the rv_expression
  (e.g. obtained with an expression of the form (Y==y)|(X1==x1 && X2==x2)
  with an analytical solution or the EM algorithm which maximize the
  likelihood of the LHS of the expression given the RHS. 
  The user must also specified the set of parameters over which
  EM whould be applied (parameters_to_learn).
  The observed Vars (e.g. x1, x2, y) in the conditional_expression will
  be sampled from the VMat n_samples times to form the training set.
  Note that a sample from VMat must contain the concatenation
  of the values for x1, x2 and y in that order (RHS in the given order,
  followed by LHS). At most max_n_iterations epoch of the EM algorithm
  will be performed, or until the relative improvement in log-likelihood
  is below the relative_improvement_threshold. 
*/
///////////////////////////////////////////////////////////!<  
/*!   NOTE NOTE NOTE:
  
  THE ORDER OF THE VALUES IN THE DISTRIBUTION MUST BE:
  (1) conditioning variables (RHS), (2) output variables
*/
///////////////////////////////////////////////////////////!<  
//real EMbyEMBprop(ConditionalExpression conditional_expression, 
real EM(ConditionalExpression conditional_expression, 
         RVArray parameters_to_learn,
         VMat distr, int n_samples, int max_n_iterations=1, 
         real relative_improvement_threshold=0.001,
         bool accept_worsening_likelihood=false,
         bool compute_final_train_NLL=true);

real oEM(ConditionalExpression conditional_expression,
         RVArray parameters_to_learn,
         VMat distr, int n_samples, int max_n_iterations, 
         real relative_improvement_threshold=0.001,
         bool compute_final_train_NLL=true);

real oEM(ConditionalExpression conditional_expression,
         RVArray parameters_to_learn,
         VMat distr, int n_samples, 
         Optimizer& MStepOptimizer,
         int max_n_iterations,
         real relative_improvement_threshold=0.001,
         bool compute_final_train_NLL=true);

/*!   Construct a Var that computes logP(RandomVariable == value | RHS )
  in terms of the value Var and the Vars in the RHS,
  where RHS is a list of the form (X1==x1 && X2==x2 && X3==x3)
  where Xi are RandomVar's and xi are Var's which represent the
  value that are given to the conditioning variables Xi.
  Normally the marks used to identify RVs which are deterministically
  determined from the RHS are cleared upon return (unless specified 
  with the optional 2nd argument).
*/
Var logP(ConditionalExpression conditional_expression, 
         bool clearMarksUponReturn=true,
         RVInstanceArray* parameters_to_learn=0);

/*!   Construct a Var that computes P(LHS == observation | RHS == values )
  in terms of the Var observation and the Vars in the RHS,
  where RHS is a RVArray such as (X1==x1 && X2==x2 && X3==x3)
  where Xi are RandomVar's and xi are Var's which represent the
  value that are given to the conditioning variables Xi.
  Normally the marks used to identify RVs which are deterministically
  determined from the RHS are cleared upon return (unless specified 
  with the optional 2nd argument).
*/
Var P(ConditionalExpression conditional_expression,
         bool clearMarksUponReturn=true);

/*!   This is like logP but it represents the expected log-probability
  of obs=conditional_expression.LHS.v given the RHS, where the expectation 
  is over the "hidden" random variables of EM in mixtures, as a function of the values of 
  the parameters_to_learn.
*/
Var ElogP(ConditionalExpression conditional_expression, 
    RVInstanceArray& parameters_to_learn,
    bool clearMarksUponReturn=true);

/*!   integrate the RV over the given hiddenRV
  and return the resulting new RandomVariable. This 
  may be difficult to do in general...
*/
RandomVar marginalize(const RandomVar& RV, const RandomVar& hiddenRV);
/*!   in practice, the user might want to specifify how
  to do the integrals, with an object yet to define, Marginalizer.
  RandomVar marginalize(const RandomVar& hiddenRV,Marginalizer& m)
   { PLERROR("marginalize not implemented yet..."); }
*/

/*!   Sample an instance from the given conditional expression,
  of the form (LHS|RHS) where LHS is a RandomVar and
  RHS is a RVInstanceArray, e.g. (X==x && Z==z && W==w).
*/
Vec sample(ConditionalExpression conditional_expression);

/*!   Return a Var which depends functionally on the RHS instances
  and the value of other RandomVars which are non-random and
  influence the LHS.
*/
Var Sample(ConditionalExpression conditional_expression);

/*!   Sample N instances from the given conditional expression,
  of the form (LHS|RHS) where LHS is a RandomVar and
  RHS is a RVInstanceArray, e.g. (X==x && Z==z && W==w).
  Put the N instances in the rows of the given Nxd matrix.
  THIS ALSO SHOWS HOW TO REPEATEDLY SAMPLE IN AN EFFICIENT
  MANNER (rather than call "Vec sample(ConditionalExpression)").
*/
void sample(ConditionalExpression conditional_expression,Mat& samples);

//!  Functions to build a normal distribution

/*!   multivariate d-dimensional diagonal normal with NON-RANDOM and CONSTANT 
  parameters (default means = 0, default standard deviations = 1)
  Actual variance is variance = minimum_variance + exp(log_variance)
*/
RandomVar normal(real mean=0, real standard_dev=1, int d=1,
                 real minimum_standard_deviation=1e-6);

/*!   diagonal normal with general parameters
  given by the provided RandomVar's.
  Actual variance is variance = minimum_variance + exp(log_variance)
*/
RandomVar normal(RandomVar mean, RandomVar log_variance,
                 real minimum_standard_deviation=1e-6);

/*!   A mixture of distributions, with the given components and
  the convex weights given by weights = softmax(log_weights). Note
  that the log_weights argument represents unnormalized log-probabilities
  (i.e normalization is automatically done inside the mixture).
*/
RandomVar mixture(RVArray components, RandomVar log_weights);

/*!   A discrete probability distribution which assigns
  probabilities[i] = softmax(log_probabilities)[i] to each
  of the discrete values i=0, 1, ... N-1. Note that
  the argument represents unnormalized log-probabilities
  (i.e normalization is automatically done inside the multinomial).
*/
RandomVar multinomial(RandomVar log_probabilities);

/*!   There are two main types of random variables:
  (1) StochasticRandomVariables: represents an unconditional
      "stochastic" distribution. e.g., NormalRV, ExponentialRV,
      UniformRV, etc... but that also includes such building
      bloks as the EmpiricalRV (whose density is a weighted sum of
      diracs). Uncertainty even parents are given ==> Stochastic.
  (2) FunctionalRandomVariables: represents a RV that is a deterministic
      function of other RVs, e.g., the sum, mixture, product, exponential,
      etc... of parent RVs. A FunctionalRandomVariable, unlike a
      StochasticRandomVariable, can be nonRandom() if its parents
      are non-random (or if it has no parents).
*/

/*!   A StochasticRandomVariable represents a
  "stochastic" distribution. Most "ordinary" distributions
  are of this type. Its parameters are its parents
  (which can be random or not). A StochasticRandomVariable
  can NEVER be random, even when its parents are non-random.
  Some sub-classes of StochasticRandomVariable can be
  defined which have no RandomVar parameters (i.e., no
  parents).
*/
class StochasticRandomVariable: public RandomVariable
{
 public:
  StochasticRandomVariable(int length=1);
  StochasticRandomVariable(const RVArray& params,int length);
  StochasticRandomVariable(const RVArray& params,int length, int width);

  //!  define RandomVariable functions

  //!  the only exception to this default would be a dirac distribution
  virtual bool isNonRandom() { return false; }

  //!  most common default
  virtual bool isDiscrete() { return false; }

  virtual void setKnownValues();

  /////////////////////////////////////////////////!<  
  //!  SUBCLASS WRITERS: IMPLEMENT FUNCTIONS BELOW //!<  
  /////////////////////////////////////////////////!<  
  
/*!     subclasses must also implement the following 
    methods from RandomVariable:
    
     void EMUpdate();
     void EMBprop(const Vec obs, real posterior);
     Var logP(const Var& obs, const RVInstanceArray& RHS,RVInstanceArray* parameters_to_learn);
     void setValueFromParentsValue(); <  put sampling algorithm in value
    
    and optionally:
    
     void EMTrainingInitialize(const RVArray& parameters_to_learn);
     void EMEpochInitialize();
     bool canStopEM();
     bool isDiscrete();
     void clearEMmarks();
     void unmarkAncestors();
    
*/
};

/*!   A FunctionalRandomVariable represents a RV that is a deterministic
  function of other RVs, e.g., the sum, mixture, product, exponential,
  etc... of parent RVs.
*/
class FunctionalRandomVariable: public RandomVariable {
 public:
  FunctionalRandomVariable(int length);
  FunctionalRandomVariable(int length, int width);
  FunctionalRandomVariable(const Vec& the_value);
  FunctionalRandomVariable(const Mat& the_value);
  FunctionalRandomVariable(const Var& the_value);
  FunctionalRandomVariable(const RVArray& parents,int length);
  FunctionalRandomVariable(const RVArray& parents,int length, int width);

  //!  redefine RandomVariable functions

  virtual Var logP(const Var& obs, const RVInstanceArray& RHS,
      RVInstanceArray* parameters_to_learn);

  //!  a Functional RV may be non-random if all its ancestors are non-random
  bool isNonRandom();

  //!  default is that the RV is discrete if all its parents are discrete
  virtual bool isDiscrete();

  //!  functions specific to FunctionalRandomVariable's

  /////////////////////////////////////////////////!<  
  //!  SUBCLASS WRITERS: IMPLEMENT FUNCTIONS BELOW //!<  
  /////////////////////////////////////////////////!<  
  
/*!     check whether it is possible to invert the function
    which maps the given unobserved parents to the observed value of the RV (obs).
    If invertible, do the inversion, and set the value fields
    of the RVInstances to Var's which are functionally dependent
    on obs. If the absolute value of the Jacobian of the map from the 
    unobserved parents to this R.V.'s value is different from 1,
    then JacobianCorrection should point to a Var that is
    the logarithm of the determinant of this Jacobian 
    (first derivatives) matrix. If the function is not
    invertible but it is possible to write P(Y==obs | unobserved_parents)
    in terms of the unobserved_parents logP functions, then
    the sub-class writer should instead redefine the logP function
    appropriately.
*/
  virtual bool invertible(const Var& obs, 
                             RVInstanceArray& unobserved_parents,
                             Var** JacobianCorrection);

  //!  set the field value from the values of the parents
  virtual void setValueFromParentsValue() = 0;

  /////////////////////////////////////////////////!<  
  //!  SUBCLASS WRITERS: IMPLEMENT FUNCTIONS BELOW //!<  
  /////////////////////////////////////////////////!<  
  
/*!     subclasses must also implement the following 
    methods from RandomVariable:
    
     void EMBprop(const Vec obs, real posterior);
    
    and optionally 
    
     bool isDiscrete()
    
    and rarely, if some parents are viewed as learnable parameters
    
     void EMTrainingInitialize(const RVArray& parameters_to_learn);
     void EMEpochInitialize();
     void EMUpdate();
    
    even more rarely
    
     bool canStopEM();
     void clearEMmarks();
     void unmarkAncestors();
    
*/
};

/*!   A NonRandomVariable is actually just like a regular Var
  (it is a random variable whose value is not random). It has no 
  RV parents, but as a Var it may be functionally dependent
  on other Vars.
*/
class NonRandomVariable: public FunctionalRandomVariable
{
public:
  //!  the next 2 constructor build a really fixed NonRandomVariable
  //!  (the Var is a SourceVariable, which isConstant()).
  NonRandomVariable(int thelength);
  NonRandomVariable(int thelength, int thewidth);
/*!     whereas the next 2 allow to build from Var function relationships
    Note that a Vec or a Mat can also be passed, which will be cast to Var 
    (a Mat will be cast to Var and then Var).
*/
  NonRandomVariable(const Var& v);

  virtual char* classname() { return "NonRandomVariable"; }

  void setValueFromParentsValue() { }
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection) 
    { return true; }
  void EMBprop(const Vec obs, real post) { }
};

class JointRandomVariable: public FunctionalRandomVariable
{
 public:
  JointRandomVariable(const RVArray& variables);

  virtual char* classname() { return "JointRandomVariable"; }

  void setValueFromParentsValue();
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection);
  void EMBprop(const Vec obs, real post);
};

//!  RandomVariable that is the element of the first parent RandomVariable indexed 
//!  by the second parent RandomVariable 
class RandomElementOfRandomVariable: public FunctionalRandomVariable
{
public:
  RandomElementOfRandomVariable(const RandomVar& v, const RandomVar& index);

  virtual char* classname() { return "RandomElementOfRandomVariable"; }

  void setValueFromParentsValue();
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection);
  void EMBprop(const Vec obs, real post);

  //!  convenience inlines
  inline const RandomVar& v() { return parents[0]; }
  inline const RandomVar& index() { return parents[1]; }

};


/*!   RandomVariable that is the i-th element of its parent vector,
  where i is the value of the last parent. i.e.
   parents[0..N-1] is the table over which we select
   parents[N] is the index that makes the selection
*/
class RVArrayRandomElementRandomVariable: public FunctionalRandomVariable
{
 public:
  RVArrayRandomElementRandomVariable(const RVArray& table, const RandomVar& index);

  virtual char* classname() { return "RVArrayRandomElementRandomVariable"; }

  void setValueFromParentsValue();
  virtual Var logP(const Var& obs, const RVInstanceArray& RHS,
      RVInstanceArray* parameters_to_learn=0);
  void EMBprop(const Vec obs, real post);

  //!  convenience inlines
  inline const RandomVar& index() { return parents[parents.size()-1]; }

};

class NegRandomVariable: public FunctionalRandomVariable
{
 public:
  NegRandomVariable(RandomVariable* input);
  
  virtual char* classname() { return "NegRandomVariable"; }

  void setValueFromParentsValue();
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection);
  void EMBprop(const Vec obs, real post);
};

class ExpRandomVariable: public FunctionalRandomVariable
{
 public:
  ExpRandomVariable(RandomVar& input);
  
  virtual char* classname() { return "ExpRandomVariable"; }

  void setValueFromParentsValue();
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection);
  void EMBprop(const Vec obs, real post);
};


class LogRandomVariable: public FunctionalRandomVariable
{
 public:
  LogRandomVariable(RandomVar& input);
  
  virtual char* classname() { return "LogRandomVariable"; }

  void setValueFromParentsValue();
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection);
  void EMBprop(const Vec obs, real post);
};

class DiagonalNormalRandomVariable: public StochasticRandomVariable
{
/*!     mean = parents[0]
    log_variance = parents[1]
    variance = minimum_variance + exp(log_variance);
*/
 protected:
  real minimum_variance;
  real normfactor; //!<  normalization constant = dimension * log(2 PI)
  bool shared_variance; //!<  iff log_variance->length()==1

 public:
  DiagonalNormalRandomVariable(const RandomVar& mean, 
                               const RandomVar& log_variance,
                               real minimum_standard_deviation = 1e-10);

  virtual char* classname() { return "DiagonalNormalRandomVariable"; }

  Var logP(const Var& obs, const RVInstanceArray& RHS,
      RVInstanceArray* parameters_to_learn);
  void setValueFromParentsValue();
  void EMUpdate();
  void EMBprop(const Vec obs, real posterior);
  void EMEpochInitialize();

  //!  convenience inlines
  inline const RandomVar& mean() { return parents[0]; }
  inline const RandomVar& log_variance() { return parents[1]; }
  inline bool& learn_the_mean() { return learn_the_parameters[0]; }
  inline bool& learn_the_variance() { return learn_the_parameters[1]; }

 protected:
  //!  temporaries used during EM learning
  Vec mu_num; //!<  numerator for mean update in EM algorithm
  Vec sigma_num; //!<  numerator for variance update in EM algorithm
  real denom; //!<  denominator for mean and variance update in EM algorithm
};

class MixtureRandomVariable: public StochasticRandomVariable
{
 protected:
  //!   Note: THESE ARE NOT PARENTS IN THE GRAPHICAL MODEL.
  RVArray components; //!<  component distributions.

 public:
  MixtureRandomVariable(const RVArray& components,
                        const RandomVar& log_weights);

  virtual char* classname() { return "MixtureRandomVariable"; }

  //!  convenience inline:
  //!  actual weights = softmax(log_weights)
  inline const RandomVar& log_weights() { return parents[0]; }
  inline bool& learn_the_weights() { return learn_the_parameters[0]; }

  virtual Var logP(const Var& obs, const RVInstanceArray& RHS,
      RVInstanceArray* parameters_to_learn);
  virtual Var ElogP(const Var& obs, RVInstanceArray& parameters_to_learn,
      const RVInstanceArray& RHS);

  virtual void setValueFromParentsValue();
  virtual void EMUpdate();
  virtual void EMBprop(const Vec obs, real posterior);
  virtual void EMEpochInitialize();
  virtual void EMTrainingInitialize(const RVArray& parameters_to_learn);
  virtual bool isDiscrete();
  virtual bool canStopEM();
  virtual void setKnownValues();
  virtual void unmarkAncestors();
  virtual void clearEMmarks();

 protected:
  //!  temporaries used for EM:
  Vec posteriors; //!<  P(i-th component | obs), used in EM
  Vec sum_posteriors; //!<  sum of posteriors over trianing data

  //!  used in logP:
  VarArray componentsLogP; //!<  result of logP call for each component
  Var lw; //!<  = log(softmax(log_weights()->value()))
  Var logp; //!<  result of last call to logP
};

/*!   Y = X0 + X1
  
  if both X0 and X1 are observed, and one of them is learnable,
  then it can be learned by EM.
  
*/
class PlusRandomVariable: public FunctionalRandomVariable
{
 public:
  PlusRandomVariable(RandomVar input1, RandomVar input2);

  virtual char* classname() { return "PlusRandomVariable"; }

  void setValueFromParentsValue();
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection);
  void EMBprop(const Vec obs, real post);
  void EMTrainingInitialize(const RVArray& parameters_to_learn);
  void EMEpochInitialize();
  void EMUpdate();

  //!  convenience inline's
  const RandomVar& X0() { return parents[0]; }
  const RandomVar& X1() { return parents[1]; }

  //!  stuff for EM
  bool learn_X0() { return learn_the_parameters[0]; }
  bool learn_X1() { return learn_the_parameters[1]; }
  bool learn_something;
  RandomVar parent_to_learn; //!<  the one of X0 or X1 that is to learn
  RandomVar other_parent; //!<  the other one
  Vec numerator;
  Vec difference;
  real denom;
};

/*!   Y = X0 - X1
  
  if both X0 and X1 are observed, and one of them is learnable,
  then it can be learned by EM.
  
*/
class MinusRandomVariable: public FunctionalRandomVariable
{
 public:
  MinusRandomVariable(RandomVar input1, RandomVar input2);

  virtual char* classname() { return "MinusRandomVariable"; }

  void setValueFromParentsValue();
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection);
  void EMBprop(const Vec obs, real post);
  void EMTrainingInitialize(const RVArray& parameters_to_learn);
  void EMEpochInitialize();
  void EMUpdate();

  //!  convenience inline's
  const RandomVar& X0() { return parents[0]; }
  const RandomVar& X1() { return parents[1]; }

  //!  stuff for EM
  bool learn_X0() { return learn_the_parameters[0]; }
  bool learn_X1() { return learn_the_parameters[1]; }
  bool learn_something;
  RandomVar parent_to_learn; //!<  the one of X0 or X1 that is to learn
  RandomVar other_parent; //!<  the other one
  Vec numerator;
  Vec difference;
  real denom;
};


/*!   Y = X0 / X1
  
  
*/
class ElementWiseDivisionRandomVariable: public FunctionalRandomVariable
{
 public:
  ElementWiseDivisionRandomVariable(RandomVar input1, RandomVar input2);

  virtual char* classname() { return "ElementWiseDivisionRandomVariable"; }

  void setValueFromParentsValue();
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection);
  void EMBprop(const Vec obs, real post);
  void EMTrainingInitialize(const RVArray& parameters_to_learn);
  void EMEpochInitialize();
  void EMUpdate();

  //!  convenience inline's
  const RandomVar& X0() { return parents[0]; }
  const RandomVar& X1() { return parents[1]; }

};


/*!   * X0 and X1 are Vec and product is element by element
   - X0 learned:
    X0 = (sum_t p_t x1_t .* y_t) ./ (sum_t p_t x1_t .* x1_t)
   - X1 learned:
    X1 = (sum_t p_t x0_t .* y_t) ./ (sum_t p_t x0_t .* x0_t)
   (where .* and ./ denote element wise multiplication and addition)
*/


/*!   Y = X0 * X1
  
  where Y is an mxl matrix (or a vector viewed as mx1 or 1xl),
  X0 is an mxn matrix, or a vector which must be 1xn or mx1,
  X1 is an nxl matrix, or a vector which must be nx1 or 1xl; this
  includes the special cases where one or the other is scalar
  (n=1 or l=1), or both are scalar (m=n=l=1).
  
  if both X0 and X1 are observed, and one of them is learnable,
  then it can be learned by EM (p_t denotes posterior for example t,
  and y_t the output observation for example t):
  
   - X0 learned:
    X0 = (sum_t p_t y_t x1_t') * inverse(sum_t p_t x1_t x1_t')
                    mxl lxn                        nxl  lxn
   - X1 learned (same stuff but everything is transposed):
    X1 = inverse(sum_t p_t x0_t' x0_t) * (sum_t p_t x0_t' y_t)
                           nxm   mxn                nxm  mxl
  
  Note that sometimes the nxn matrix to invert is a 1x1 scalar.
  
*/
class ProductRandomVariable: public FunctionalRandomVariable
{
 public:
  int m,n,l; //!<  dimensions of the matrices Y(mxl) = X0(mxn) * X1(nxl)

  ProductRandomVariable(MatRandomVar input1, MatRandomVar input2);

  virtual char* classname() { return "ProductRandomVariable"; }

  void setValueFromParentsValue();
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection);
  void EMBprop(const Vec obs, real post);
  void EMTrainingInitialize(const RVArray& parameters_to_learn);
  void EMEpochInitialize();
  void EMUpdate();

  //!  convenience inline's
  const RandomVar& X0() { return parents[0]; } //!<  viewed as mxn matrix
  const RandomVar& X1() { return parents[1]; } //!<  viewed as nx1 vector
  bool scalars; //!<  = (m==1 && n==1 && l==1);

  //!  stuff for EM
  bool learn_X0() { return learn_the_parameters[0]; }
  bool learn_X1() { return learn_the_parameters[1]; }
  bool learn_something;
  Mat X0numerator; //!<  mxn matrix, used in the case X0 is learned
  Mat X1numerator; //!<  nxl vector, used in the case X1 is learned
  Mat denom; //!<  nxn matrix denominator
  Mat tmp1; //!<  temporary matrices to avoid allocating in loops
  Mat tmp2; //!<  
  Mat tmp3; //!<  
  Vec vtmp3; //!<  Vec version of tmp3
  Vec tmp4;
};

//!  Y = sub-vector of X starting at position "start", of length "value->length()".
class SubVecRandomVariable: public FunctionalRandomVariable
{
 protected:
  int start;
 public:
  SubVecRandomVariable(const RandomVar& parent,int start, int length);
  virtual char* classname() { return "SubvecRandomVariable"; }
  void setValueFromParentsValue();
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection);
  void EMBprop(const Vec obs, real posterior);
};

/*!   Y is discrete and takes values 0 to N-1.
  This class represents P(Y=i) = p_i where p=softmax(log_probabilities)
  and log_probabilities is the unique parent (which can be
  learned with "EM" in one iteration).
*/
class MultinomialRandomVariable: public StochasticRandomVariable
{
 public:
  //!  the parameters are unnormalized log-probabilities 
  //!  associated to each of the possible values of the observation
  MultinomialRandomVariable(const RandomVar& log_probabilities);

  //!  convenience inline
  inline const RandomVar& log_probabilities() { return parents[0]; }
  inline bool learn_the_probabilities() { return learn_the_parameters[0]; }

  virtual char* classname() { return "MultinomialRandomVariable"; }

  Var logP(const Var& obs, const RVInstanceArray& RHS, 
      RVInstanceArray* parameters_to_learn);
  void setValueFromParentsValue(); //!<  sampling algorithm
  void EMUpdate();
  void EMBprop(const Vec obs, real posterior);
  void EMEpochInitialize();
  bool isDiscrete();

 protected:
  //!  temporaries used for EM:
  Vec sum_posteriors; //!<  sum of posteriors over training data
};


/*!   If the RV is a vector (i.e., a column vector), extend it
  with n_extend fill_value; if it is a matrix (MatRandomVar), then
  extend the matrix with n_extend rows with value "fill_value".
  The "default" extension is with n_extend=1 and fill_value=1
  (useful for extending a vector before an "affine" dot product).
*/
class ExtendedRandomVariable: public FunctionalRandomVariable
{
 protected:
  int n_extend;
  real fill_value;
 public:
  ExtendedRandomVariable(const RandomVar& parent, real fill_value=1.0,int n_extend=1);
  virtual char* classname() { return "ExtendedRandomVariable"; }
  void setValueFromParentsValue();
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection);
  void EMBprop(const Vec obs, real posterior);
};

//!  concatenate the columns of the matrix arguments, just
//!  like the hconcat function (PLearn.h) on Vars.
class ConcatColumnsRandomVariable: public FunctionalRandomVariable
{
 public:
  ConcatColumnsRandomVariable(const RVArray& vars);
  virtual char* classname() { return "ConcatColumnsRandomVariable"; }
  void setValueFromParentsValue();
  bool invertible(const Var& obs, RVInstanceArray& unobserved_parents,
                     Var** JacobianCorrection);
  void EMBprop(const Vec obs, real posterior);
};

/*!   ** WARNING **
  DEPRECATED: this class should be rewritten entirely or erased.
  It probably won't work in its current state.
*/

//!  This is a convenient wrapping around the required data
//!  structures for efficient repeated sampling from a RandomVar.
class RandomVarVMatrix: public VMatrix
{
 protected:
  RandomVar rv;
  Var instance;
  VarArray prop_path;

 public:
  RandomVarVMatrix(ConditionalExpression conditional_expression);
  virtual int nVars() { return instance->length(); }
  virtual Vec sample()
    {
      prop_path.fprop();
      return instance->value;
    }
};

//!   template <> void deepCopyField(RandomVar& field, CopiesMap& copies);

} // end of namespace PLearn

#endif

/*! THINGS THAT REMAIN TO DO:

 - check gradient through transposeProduct

 - implement a "Var" version of EM that
   allows each RandomVar to return E_{latent}[log P({observed},{latent}|{inputs}) | observed, previous_params]
   as a function of next_params, i.e., a weighted sum of log-probabilities where the weights
   are "constant" posteriors computed based on the current values of the parameters:
   PROBLEM = THE VARIANCES OF NORMAL DISTRIBUTIONS ARE NOT PARAMETERS OF A QUADRATIC FORM FOR logP
   (even though it turns out that there is an analytical solution)

 - implement Var::symbolicBprop, which allows to symbolically
   compute the bprop through a bunch of Vars: add a "Var dCdvalue" 
   field to Var's and a method symbolicBprop
   which fills it in a way akin to bprop.

 - add utilities to estimate second derivatives matrix S = d2Cdtheta2
    - based on symbolicBprop (analytic solution)
     * if Var dCdtheta = symbolicBprop of dC/dtheta, vector fn of theta, then
     * for (i=0;i<theta->length();i++)
         dCdtheta->gradient.clear();
         dCdtheta->gradient[i]=1;
         dCdtheta->bprop()
         S(i).copyFrom(theta->gradient)
    - based on the Fisher Information trick:
        E_{X|theta}[ d2logP(X|theta)/dtheta ] = 
        - E_{X|theta}[ (dlogP(X|theta)/dtheta)^2 ]
      so if C = logP(X|theta) (e.g. integrate P(X|theta)P(theta)dtheta
      with a uniform prior), we have
      * if P(X|theta) is approximately normal, then second
        derivative of logP(X|theta) does not depend on X, so
        S = - E_{X|theta}[ (dlogP(X|theta)/dtheta)^2 ]
        The expectation can be approximated by an average when
        X is a set of iid examples. Or it can be obtained 
        through sampling of P(X|theta) by Monte-Carlo.
      * if P(theta) is not uniform, we obtain the relation
        E_{X|theta}[ d2logP(X|theta)/dtheta ] = 
         d2logP(theta)/dtheta + (dlogP(theta)/dtheta)^2 
        - E_{X|theta}[ (dlogP(X|theta)/dtheta)^2 ]
        If S does not depend too much on X around theta0, then
        the RHS evaluated at theta0 gives us a method to compute S.
        Again, if X is a set of iid examples, then just use the
        average to approximate the expectation, otherwise use
        Monte-Carlo sampling of P(X|theta0).

 - built-in regularization: smoother for multinomial and * and Affine
   ==> can be done by the user by adding a penalty

 - do all the functional RVs corresponding to existing Variable subclasses

 - RandomVariable::operator[] --> access scalar sub-elements 
 - RandomVariable::operator() --> access sub-var of jointRV

 - expectation all the way down

 - variance all the way down

 - cumulative distribution all the way down

 - FullNormalRandomVariable

 - GeometricRandomVariable

 - PoissonRandomVariable

 - ChiSquareRandomVariable

 - StudentRandomVariable

 - ExponentialRandomVariable

 - Handling of time-series: this requires a change in the Variable objects
   that includes 
    - "delays" (from parent's value)
    - a special type of VarArray, SeqVarArray which does the unfolding
      based on the delays and the connectivity: this requires buffering
      the values and gradients somewhere => in the Var or in the SeqVarArray?
    - a representation of sequences as matrices 
      (row = value at time t, for efficiency, although mathematically
       col = value at time t would make more sense)
    - consider the generalization to other less linear structures (e.g.
      asynchronous linear structures, tree-like structures, others which
      are basically directed graphs with "sharing").
    - Maybe it would be simpler to wrap the temporal or other structure
      around Vars by creating copies of the Vars with the appropriate
      connectivity built-in.
    - How would things like HMMs be implemented using this framework?
      maybe the ElogP method of an HMM RandomVar would take a matrix-observation
      (representing a sequence of observations) and return
      a SeqVarArray.

 - some feasible cases of marginalization
   e.g. approximation of the posterior by a Normal
   Laplace approximation:
     I = int f(theta) dtheta  is approximated
     when f(theta) is peaked around theta0 = argmax f(theta), by
     I = f(theta0) * (2pi)^{n/2} * sqrt(det(d2(log f(theta))/dtheta^2))
     where the second derivative is evaluated at theta0

 - implement matrix pseudo-inverse, inverse and and backprop through them

 - setup comparative tests of correctedness and speed with AD

 - add a generateCode methods to Var, which spits out strings on a stream,
   representing "compiled" fprop. We need two methods, I think:
    void generateInitializationCode(const iostream& s, char* suffix); 
    void generateFpropCode(const iostream& s, char* suffix);
   The suffix is for guaranteeing uniqueness of names of variables
   in the generated code. It will be provided by the VarArray::generateCode
   method, which chooses to give a "name" (maybe just an ascii integer)
   to each Var in the array.
   Note that we don't need a generateBpropCode if the symbolicBprop
   is implemented. The Initialization method spits out declarations
   on the stream. The simplest is to use Vec's and Mat's to do the
   job, but it might even be possible to do everything in real's
   with fixed-size arrays (e.g. real x[33]) since the sizes are
   known at compile time (no memory management is needed). However,
   this requires also having a compiler for the Vec/Mat operations.
   
 */


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
