// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Olivier Delalleau
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
   * $Id: ConjGradientOptimizer.h,v 1.7 2003/04/22 20:55:17 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


#ifndef CONJGRADIENTOPTIMIZER_INC
#define CONJGRADIENTOPTIMIZER_INC

#include "Optimizer.h"

namespace PLearn <%
using namespace std;


/*
 * CLASS CONJGRADIENTOPTIMIZER
 *
 * An optimizer using the conjugate gradient method.
 * Currently only implements one method, but will hopefully evolve to be able
 * to use other algorithms.
 *
 */
class ConjGradientOptimizer : public Optimizer {

  typedef Optimizer inherited;

  // The various line search algorithms available
/*  enum LineSearchType {
    FletcherSearch,
    GSearch
  };

  // The various formulas available to find the new search direction
  enum FindNewDirectionType {
    CONJPOMDP,
    FletcherReeves,
    HestenesStiefel,
    PolakRibiere
  };*/ // TODO Use the enums types : but must work with the .plearn parser !

public:

  // General options (also available through setOption)
  int line_search_algo; // the line search algorithm used
  int find_new_direction_formula; // the formula used

  // GSearch specific options
  real starting_step_size;  // initial step for line search
  real epsilon;             // gradient resolution

  // FletcherSearch specific options
  real sigma; // in the constraint : abs(f'(m)) < -sigma.f'(0)
  real rho;   // in the constraint : f(m) < f(0) + m.rho.f'(0)
  real fmax;  // we stop if we reach cost <= fmax (usually fmax = 0)
  real stop_epsilon; // we stop when (a-alpha).f'(a) < stop_epsilon (Fletcher)
  real tau1, tau2, tau3; // bracketing parameters

private:

  // Internal data
  Vec search_direction;  // current search direction for the line search
  real last_improvement; // cost improvement during the last iteration
  Vec tmp_storage;       // used for temporary storage of data
  
public:

  // Constructors and other usual stuff
  ConjGradientOptimizer(
      real the_starting_step_size=0.01, 
      real the_epsilon=0.01,
      real the_sigma=0.01,
      real the_rho=0.005,
      real the_fmax=0,
      real the_stop_epsilon=0.0001,
      real the_tau1=9,
      real the_tau2=0.1,
      real the_tau3=0.5,
      int n_updates=1, const string& filename="", 
      int every_iterations=1);
  
  ConjGradientOptimizer(
      VarArray the_params, 
      Var the_cost,
      real the_starting_step_size=0.01, 
      real the_epsilon=0.01,
      real the_sigma=0.01,
      real the_rho=0.005,
      real the_fmax=0,
      real the_stop_epsilon=0.0001,
      real the_tau1=9,
      real the_tau2=0.1,
      real the_tau3=0.5,
      int n_updates=1, const string& filename="", 
      int every_iterations=1);

  ConjGradientOptimizer(
      VarArray the_params, 
      Var the_cost, 
      VarArray the_update_for_measure,
      real the_starting_step_size=0.01, 
      real the_epsilon=0.01,
      real the_sigma=0.01,
      real the_rho=0.005,
      real the_fmax=0,
      real the_stop_epsilon=0.0001,
      real the_tau1=9,
      real the_tau2=0.1,
      real the_tau3=0.5,
      int n_updates=1, const string& filename="", 
      int every_iterations=1);

  DECLARE_NAME_AND_DEEPCOPY(ConjGradientOptimizer);

  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies) { inherited::makeDeepCopyFromShallowCopy(copies); }

  virtual void build()
  {
    inherited::build();
    build_();
  }

private:

  void build_() {}
    
public:

  virtual void oldwrite(ostream& out) const;
  virtual void oldread(istream& in);
  virtual real optimize();

protected:
  static void declareOptions(OptionList& ol);

private:

  // Find the new search direction for the line search algorithm
  // and store it in h
  bool findDirection(
      void (*grad)(VarArray, Var, VarArray, const Vec&),
      VarArray params,
      Var costs,
      VarArray proppath,
      real starting_step_size,
      real epsilon,
      Vec g,
      Vec delta,
      Vec tmp_storage);

  // Search the minimum in the current search direction
  // Return the value of the minimum found
  real lineSearch(
      void (*grad)(VarArray, Var, VarArray, const Vec&),
      VarArray params,
      Var costs,
      VarArray proppath,
      real starting_step_size,
      real epsilon,
      Vec tmp_storage);

  //----------------------- CONJUGATE GRADIENT FORMULAS ---------------------

  // The CONJPOMDP algorithm as described in
  // "Direct Gradient-Based Reinforcement Learning:
  // II. Gradient Ascent Algorithms and Experiments"
  // by J.Baxter, L. Weaver, P. Bartlett.
  // This is just one single iteration of the algorithm.
  // Return true when the convergence is obtained.
  static bool conjpomdp (
      // the given grad function needs to compute the gradient
      void (*grad)(VarArray params, Var cost, VarArray proppath, const Vec& gradient),
      VarArray params,    // the current parameters of the model
      Var costs,          // the cost to optimize
      VarArray proppath,  // the propagation path from params to costs
      real epsilon,       // the gradient resolution
      Vec g,              // the gradient
      Vec h,              // the current search direction
      Vec delta);         // storage place, size of params.elems()

  // The Fletcher-Reeves formula used to find the new direction
  // h(n) = -g(n) + norm2(g(n)) / norm2(g(n-1)) * h(n-1)
  static void fletcherReeves (
      void (*grad)(VarArray, Var, VarArray, const Vec&),
      VarArray params,
      Var costs,
      VarArray proppath,
      Vec g,
      Vec h,
      Vec delta);

  // The Hestenes-Stiefel formula used to find the new direction
  // h(n) = -g(n) + dot(g(n), g(n)-g(n-1)) / dot(h(n-1), g(n)-g(n-1)) * h(n-1)
  static void hestenesStiefel (
      void (*grad)(VarArray, Var, VarArray, const Vec&),
      VarArray params,
      Var costs,
      VarArray proppath,
      Vec g,
      Vec h,
      Vec delta);

  // The Polak-Ribiere formula used to find the new direction
  // h(n) = -g(n) + dot(g(n), g(n)-g(n-1)) / norm2(g(n-1)) * h(n-1)
  static void polakRibiere (
      void (*grad)(VarArray, Var, VarArray, const Vec&),
      VarArray params,
      Var costs,
      VarArray proppath,
      Vec g,
      Vec h,
      Vec delta);

  //------------------------- LINE SEARCH ALGORITHMS -------------------------

  // The GSearch algorithm as described in
  // "Direct Gradient-Based Reinforcement Learning:
  // II. Gradient Ascent Algorithms and Experiments"
  // by J.Baxter, L. Weaver, P. Bartlett.
  // See conjpomdp for more explainations on the parameters.
  static void gSearch (
      void (*grad)(VarArray, Var, VarArray, const Vec&),
      VarArray params,
      Var costs,
      VarArray proppath,
      Vec search_direction, // current direction
      real starting_step_size,
      real epsilon,
      Vec tmp_storage);

  // The line search algorithm described in
  // "Practical Methods of Optimization, 2nd Ed", by Fletcher (1987)
  // (this function actually just calls fletcherSearchMain)
  real fletcherSearch (
      real alpha1 = FLT_MAX,
      real mu = FLT_MAX);

  
  //--------------------------- UTILITY FUNCTIONS ----------------------------
  
public:   // TODO For test purpose... remove later


  // Return cost->value() after an update of params with step size alpha
  // in the current search direction
  // ie : f(x) = cost(params + x*search_direction) in x = alpha
  static real computeCostValue(real alpha, ConjGradientOptimizer* opt);

  // Return the derivative of the function
  // f(x) = cost(params + x*search_direction)
  // in x = alpha
  static real computeDerivative(real alpha, ConjGradientOptimizer* opt);

  // Given a propagation path from params to costs,
  // return the derivative of the function :
  // f(a) = costs(params + a*direction)
  static real computeComposedGrad(
      void (*grad)(VarArray, Var, VarArray, const Vec&),
      VarArray params,
      Var costs,
      VarArray proppath,
      Vec direction,
      Vec delta);

  // Given a propagation path from params to costs,
  // compute the gradient and store it in the "gradient" Vec
  static void computeGradient(
      VarArray params,
      Var cost,
      VarArray proppath,
      const Vec& gradient);

  // Given a propagation path from params to costs,
  // compute the opposite of the gradient and store it in the "gradient" Vec
  static void computeOppositeGradient(
      VarArray params,
      Var cost,
      VarArray proppath,
      const Vec& gradient);

  // Put in a, b, c, d the coefficients of the cubic interpolation
  // given values of f and g=df/dx in 2 points (0 and 1)
  static void cubicInterpol(
      real f0, real f1, real g0, real g1,
      real& a, real& b, real& c, real& d);


  // Find the minimum of the cubic interpolation of function f, with g=df/dx,
  // in the interval [mini, maxi], given values at points p1 and p2
  static real findMinWithCubicInterpol (
      real (*f)(real, ConjGradientOptimizer* opt),
      real (*g)(real, ConjGradientOptimizer* opt),
      ConjGradientOptimizer* opt,
      real p1,
      real p2,
      real mini,
      real maxi);

  // The main function for Fletcher's line search algorithm
  static real fletcherSearchMain (
      real (*f)(real, ConjGradientOptimizer* opt),
      real (*g)(real, ConjGradientOptimizer* opt),
      ConjGradientOptimizer* opt,
      real sigma,
      real rho,
      real fmax,
      real epsilon,
      real tau1 = 9,
      real tau2 = 0.1,
      real tau3 = 0.5,
      real alpha1 = FLT_MAX, // if FLT_MAX, then let the algo find a value
      real mu = FLT_MAX);    // same remark as alpha1
  
  // Find the minimum of the cubic a.x^3 + b.x^2 + c.x
  // in the range [mini, maxi]
  static real minCubic(
      real a, real b, real c,
      real mini = -FLT_MAX, real maxi = FLT_MAX);

  // Find the minimum of the function a.x^2 + b.x
  // in the range [mini, maxi]
  static real minQuadratic(
      real a, real b,
      real mini = -FLT_MAX, real maxi = FLT_MAX);

  // Put in a, b, c the coefficients of the quadratic interpolation
  // given values of f in 2 points (0 and 1) and g=df/dx in 0
  static void quadraticInterpol(
      real f0, real f1, real g0,
      real& a, real& b, real& c);

};

%> // end of namespace PLearn

#endif
