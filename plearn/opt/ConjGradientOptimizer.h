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
   * $Id: ConjGradientOptimizer.h,v 1.11 2003/04/23 18:34:33 tihocan Exp $
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
 * It can be configured to use various algorithms.
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
  real starting_step_size;  // initial step for line search

  // GSearch specific options
  real epsilon;             // gradient resolution

  // FletcherSearch specific options
  real sigma; // in the constraint : abs(f'(m)) < -sigma.f'(0)
  real rho;   // in the constraint : f(m) < f(0) + m.rho.f'(0)
  real fmax;  // we stop if we reach cost <= fmax (usually fmax = 0)
  real stop_epsilon; // we stop when (a-alpha).f'(a) < stop_epsilon (Fletcher)
  real tau1, tau2, tau3; // bracketing parameters

private:

  // Internal data
  Vec current_opp_gradient;  // current opposite gradient value
  Vec search_direction;      // current search direction for the line search
  Vec tmp_storage;           // used for temporary storage of data
  Vec delta;                 // temporary storage of the gradient
  real last_improvement;     // cost improvement during the last iteration
  
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

  void build_() {
    // Make sure the internal datas have the right size
    current_opp_gradient.resize(params.nelems());
    search_direction.resize(params.nelems());
    tmp_storage.resize(params.nelems());
    delta.resize(params.nelems());
  }
    
public:

  virtual void oldwrite(ostream& out) const;
  virtual void oldread(istream& in);
  virtual real optimize();

protected:
  static void declareOptions(OptionList& ol);

private:

  // Find the new search direction for the line search algorithm
  bool findDirection();

  // Search the minimum in the current search direction
  // Return the value of the minimum found
  real lineSearch();

  //----------------------- CONJUGATE GRADIENT FORMULAS ---------------------
  //
  // A Conjugate Gradient formula finds the new search direction, given
  // the current gradient, the previous one, and the current search direction.
  // It updates "search_direction" and "current_opp_gradient".

  // The CONJPOMDP algorithm as described in
  // "Direct Gradient-Based Reinforcement Learning:
  // II. Gradient Ascent Algorithms and Experiments"
  // by J.Baxter, L. Weaver, P. Bartlett.
  // Actually this is almost the same as the Polak-Ribiere formula
  static bool conjpomdp (
      // The given grad function needs to compute the gradient
      // (or the opposite of the gradient if we need the minimum, as the
      // algorithm tries to find the maximum)
      void (*grad)(Optimizer*, const Vec& gradient),
      ConjGradientOptimizer* opt);

  // The Fletcher-Reeves formula used to find the new direction
  // h(n) = -g(n) + norm2(g(n)) / norm2(g(n-1)) * h(n-1)
  static void fletcherReeves (
      void (*grad)(Optimizer*, const Vec&),
      ConjGradientOptimizer* opt);

  // The Hestenes-Stiefel formula used to find the new direction
  // h(n) = -g(n) + dot(g(n), g(n)-g(n-1)) / dot(h(n-1), g(n)-g(n-1)) * h(n-1)
  static void hestenesStiefel (
      void (*grad)(Optimizer*, const Vec&),
      ConjGradientOptimizer* opt);

  // The Polak-Ribiere formula used to find the new direction
  // h(n) = -g(n) + dot(g(n), g(n)-g(n-1)) / norm2(g(n-1)) * h(n-1)
  static void polakRibiere (
      void (*grad)(Optimizer*, const Vec&),
      ConjGradientOptimizer* opt);

  //------------------------- LINE SEARCH ALGORITHMS -------------------------
  //
  // A line search algorithm moves "params" to the value minimizing "cost",
  // when moving in the direction "search_direction".
  // It must not update "current_opp_gradient" (that is done in the Conjugate
  // Gradient formulas).

  // The GSearch algorithm as described in
  // "Direct Gradient-Based Reinforcement Learning:
  // II. Gradient Ascent Algorithms and Experiments"
  // by J.Baxter, L. Weaver, P. Bartlett.
  void gSearch (void (*grad)(Optimizer*, const Vec&));

  // The line search algorithm described in
  // "Practical Methods of Optimization, 2nd Ed", by Fletcher (1987)
  // (this function actually just calls fletcherSearchMain)
  void fletcherSearch (real mu = FLT_MAX);

  
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

  // Given an optimizer, compute the gradient of the cost function and
  // store it in the "gradient" Vec
  static void computeGradient(
      Optimizer* opt,
      const Vec& gradient);

  // Given an optimizer, compute the opposite of the gradient of the cost
  // function and store it in the "gradient" Vec
  static void computeOppositeGradient(
      Optimizer* opt,
      const Vec& gradient);

  // Put in a, b, c, d the coefficients of the cubic interpolation
  // given values of f and g=df/dx in 2 points (0 and 1)
  static void cubicInterpol(
      real f0, real f1, real g0, real g1,
      real& a, real& b, real& c, real& d);


  // Find the minimum of the cubic interpolation of function f, with g=df/dx,
  // in the interval [mini, maxi], given values at points p1 and p2
  static real findMinWithCubicInterpol (
      real p1,
      real p2,
      real mini,
      real maxi,
      real f0,
      real f1,
      real g0,
      real g1);

  // The main function for Fletcher's line search algorithm
  // We keep all the parameters, so that it can be used separately
  // (without a real ConjGradientOptimizer object)
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
