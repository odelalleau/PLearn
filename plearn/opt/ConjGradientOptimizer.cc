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
   * $Id: ConjGradientOptimizer.cc,v 1.6 2003/04/15 21:44:08 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ConjGradientOptimizer.h"
#include "TMat_maths_impl.h"

namespace PLearn <%
using namespace std;

//
// Constructors
//
ConjGradientOptimizer::ConjGradientOptimizer(
    real the_starting_step_size, 
    real the_epsilon,
    int n_updates, const string& filename, 
    int every_iterations)
  :inherited(n_updates, filename, every_iterations),
  starting_step_size(the_starting_step_size),
  epsilon(the_epsilon) {}

ConjGradientOptimizer::ConjGradientOptimizer(
    VarArray the_params, 
    Var the_cost,
    real the_starting_step_size, 
    real the_epsilon,
    int n_updates, const string& filename, 
    int every_iterations)
  :inherited(the_params, the_cost, n_updates, filename, every_iterations),
  starting_step_size(the_starting_step_size),
  epsilon(the_epsilon) {}

ConjGradientOptimizer::ConjGradientOptimizer(
    VarArray the_params, 
    Var the_cost, 
    VarArray the_update_for_measure,
    real the_starting_step_size, 
    real the_epsilon,
    int n_updates, const string& filename, 
    int every_iterations)
  :inherited(the_params, the_cost, the_update_for_measure,
             n_updates, filename, every_iterations),
  starting_step_size(the_starting_step_size),
  epsilon(the_epsilon) {}
  
// 
// declareOptions
// 
void ConjGradientOptimizer::declareOptions(OptionList& ol)
{
    declareOption(ol, "starting_step_size", &ConjGradientOptimizer::starting_step_size, OptionBase::buildoption, 
                  "    the initial step size for the line search algorithm\n");

    declareOption(ol, "epsilon", &ConjGradientOptimizer::epsilon, OptionBase::buildoption, 
                  "    the gradient resolution\n");

    inherited::declareOptions(ol);
}

//
// oldwrite
//
void ConjGradientOptimizer::oldwrite(ostream& out) const
{
  writeHeader(out, "ConjGradientOptimizer", 0);
  inherited::write(out);  
  writeField(out, "starting_step_size", starting_step_size);
  writeField(out, "epsilon", epsilon);
  writeFooter(out, "ConjGradientOptimizer");
}

//
// oldread
//
void ConjGradientOptimizer::oldread(istream& in)
{
  int ver = readHeader(in, "ConjGradientOptimizer");
  if(ver!=0)
    PLERROR("In ConjGradientOptimizer::read version number %d not supported",ver);
  inherited::oldread(in);
  readField(in, "starting_step_size", starting_step_size);
  readField(in, "epsilon", epsilon);
  readFooter(in, "ConjGradientOptimizer");
}

//
// Implement name and deepCopy
//
IMPLEMENT_NAME_AND_DEEPCOPY(ConjGradientOptimizer);

/******************************
 * MAIN METHODS AND FUNCTIONS *
 ******************************/

/////////////////////////
// computeComposedGrad //
/////////////////////////
real ConjGradientOptimizer::computeComposedGrad(
    void (*grad)(VarArray, Var, VarArray, const Vec&),
    VarArray params,
    Var costs,
    VarArray proppath,
    Vec direction,
    Vec delta) {
  (*grad)(params, costs, proppath, delta);
  return dot(delta, direction);
}

/////////////////////
// computeGradient //
/////////////////////
void ConjGradientOptimizer::computeGradient(
    VarArray params,
    Var cost,
    VarArray proppath,
    const Vec& gradient) {
  // Clear all what's left from previous computations
  proppath.clearGradient();
  params.clearGradient();
  cost->gradient[0] = 1;
  proppath.fbprop();
  params.copyGradientTo(gradient);
}
  
/////////////////////////////
// computeOppositeGradient //
/////////////////////////////
void ConjGradientOptimizer::computeOppositeGradient(
    VarArray params,
    Var cost,
    VarArray proppath,
    const Vec& gradient) {
  // Clear all what's left from previous computations
  proppath.clearGradient();
  params.clearGradient();
  // We want the opposite of the gradient, thus the -1
  cost->gradient[0] = -1;
  proppath.fbprop();
  params.copyGradientTo(gradient);
}
  
///////////////
// conjpomdp //
///////////////
bool ConjGradientOptimizer::conjpomdp (
    void (*grad)(VarArray, Var, VarArray, const Vec&),
    VarArray params,
    Var costs,
    VarArray proppath,
    real epsilon,
    Vec g,
    Vec h,
    Vec delta) {

  int i;
  // delta = Gradient
  (*grad)(params, costs, proppath, delta);
  real norm_g = pownorm(g);
  // g <- delta - g
  for (i=0; i<g.length(); i++) {
    g[i] = delta[i] - g[i];
  }
  real gamma = dot(g, delta) / norm_g;
  // h <- delta + gamma * h
  for (i=0; i<h.length(); i++) {
    h[i] = delta[i] + gamma * h[i];
  }
  if (dot(h, delta) < 0) {
    // h <- delta
    h << delta;
  }
  // g <- delta
  g << delta;
  // We want to stop when the norm of the gradient is small enough
  return (pownorm(g) < epsilon);
};

///////////////////
// findDirection //
///////////////////
bool ConjGradientOptimizer::findDirection(
    void (*grad)(VarArray, Var, VarArray, const Vec&),
    VarArray params,
    Var costs,
    VarArray proppath,
    real starting_step_size,
    real epsilon,
    Vec g,
    Vec h,
    Vec delta,
    Vec tmp_storage) {
  bool isFinished = false;
  switch (isFinished) {
    case true:
      isFinished = conjpomdp(grad, params, costs, proppath,
                             epsilon, g, h, delta);
      break;
    case false:
      fletcherReeves(grad, params, costs, proppath,
                                  g, h, delta);
      break;
  }
  return isFinished;
}

////////////////////
// fletcherReeves //
////////////////////
void ConjGradientOptimizer::fletcherReeves (
    void (*grad)(VarArray, Var, VarArray, const Vec&),
    VarArray params,
    Var costs,
    VarArray proppath,
    Vec g,
    Vec h,
    Vec delta) {

  // delta = Gradient
  (*grad)(params, costs, proppath, delta);
  real gamma = pownorm(delta) / pownorm(g);
  for (int i=0; i<h.length(); i++) {
    h[i] = delta[i] + gamma * h[i];
  }
  g << delta;
}

////////////////////
// fletcherSearch //
////////////////////
real ConjGradientOptimizer::fletcherSearch (
    real (*f)(real),
    real (*g)(real),
    real sigma,
    real rho,
    real fmax,
    real tau1,
    real tau2,
    real tau3,
    real alpha1,
    real mu) {
  
  // Initialization
  if (mu == FLT_MAX)
    mu = (fmax - (*f)(0)) / (rho * (*g)(0));
  if (alpha1 == FLT_MAX)
    alpha1 = mu;
  real alpha0 = 0;
  real alpha2, f0, f1, g0, a1, a2, b1, b2;
  f0 = (*f)(0);
  g0 = (*g)(0);
  bool isBracketed = false;
  
  // Bracketing
  while (!isBracketed) {
    f1 = (*f)(alpha1);
    if (f1 <= fmax)
      return alpha1;
    if (f1 > f0 + alpha1 * g0 || f1 > (*f)(alpha0)) {
      a1 = alpha0;
      b1 = alpha1;
      isBracketed = true;
    } else {
      if (abs((*g)(alpha1)) < -sigma * g0)
        return alpha1;
      if ((*g)(alpha1) >= 0) {
        a1 = alpha1;
        b1 = alpha0;
        isBracketed = true;
      } else {
        if (mu <= 2*alpha1 - alpha0)
          alpha2 = mu;
        else
          // alpha2 = findMinWithInterpol(2*alpha1 - alpha0, min(mu, alpha1 + tau1 * (alpha1 - alpha0))); // TODO Remove comment when ready
          alpha2 = 0; // TODO Remove !
      }
    }
    alpha0 = alpha1;
    alpha1 = alpha2;
  }

  // Splitting
  while (true) {
    // alpha1 = findMinWithInterpol(a1 + tau2 * (b1-a1), b1 - tau3 * (b1-a1)); // TODO Remove comment when ready
    f1 = (*f)(alpha1);
    if ((a1 - alpha1) * (*g)(a1) <= epsilon)
      return a1;
    if (f1 > f0 + rho * alpha1 * g0 || f1 >= (*f)(a1)) {
     a2 = a1;
     b2 = alpha1;
    } else {
      if (abs((*g)(alpha1)) <= -sigma * g0)
        return alpha1;
      a2 = alpha1;
      if ((b1 - a1) * (*g)(alpha1) >= 0)
        b2 = a1;
      else
        b2 = b1;
    }
    a1 = a2;
    b1 = b2;
  }
}
  
/////////////
// gSearch //
/////////////
void ConjGradientOptimizer::gSearch (
    void (*grad)(VarArray, Var, VarArray, const Vec&),
    VarArray params,
    Var costs,
    VarArray proppath,
    Vec search_direction,
    real starting_step_size,
    real epsilon,
    Vec tmp_storage) {

  real step = starting_step_size;
  real sp, sm, pp, pm;

  // Backup the initial paremeters values
  params.copyTo(tmp_storage);

  params.update(step, search_direction);
  Vec delta(params.nelems());
  (*grad)(params, costs, proppath, delta);
  real prod = dot(delta, search_direction);

  if (prod < 0) {
    // Step back to bracket the maximum
    while (prod < -epsilon) {
      sp = step;
      pp = prod;
      step = step / 2;
      params.update(-step, search_direction);
      (*grad)(params, costs, proppath, delta);
      prod = dot(delta, search_direction);
    }
    sm = step;
    pm = prod;
  }
  else {
    // Step forward to bracket the maximum
    while (prod > epsilon) {
      sm = step;
      pm = prod;
      params.update(step, search_direction);
      (*grad)(params, costs, proppath, delta);
      prod = dot(delta, search_direction);
      step = step * 2;
    }
    sp = step;
    pp = prod;
  }

  if (pm > 0 && pp < 0)
    step = (sm*pp - sp*pm) / (pp - pm);
  else
    step = (sm+sp) / 2;

  params.copyFrom(tmp_storage);
  params.update(step, search_direction);
}

/////////////////////
// hestenesStiefel //
/////////////////////
void ConjGradientOptimizer::hestenesStiefel (
    void (*grad)(VarArray, Var, VarArray, const Vec&),
    VarArray params,
    Var costs,
    VarArray proppath,
    Vec g,
    Vec h,
    Vec delta) {

  int i;
  // delta = Gradient
  (*grad)(params, costs, proppath, delta);
  for (i=0; i<g.length(); i++) {
    g[i] = delta[i] - g[i];
  }
  real gamma = dot(delta, g) / dot(h, g);
  for (i=0; i<h.length(); i++) {
    h[i] = delta[i] + gamma * h[i];
  }
  g << delta;
}

////////////////
// lineSearch //
////////////////
void ConjGradientOptimizer::lineSearch(
    void (*grad)(VarArray, Var, VarArray, const Vec&),
    VarArray params,
    Var costs,
    VarArray proppath,
    Vec search_direction,
    real starting_step_size,
    real epsilon,
    Vec tmp_storage) {
  gSearch(grad, params, costs, proppath, search_direction,
          starting_step_size, epsilon, tmp_storage);
}

//////////////
// minCubic //
//////////////
real ConjGradientOptimizer::minCubic(real a, real b, real c,
    real mini, real maxi) {
  if (a == 0)
    return minQuadratic(b, c, mini, maxi);
  // f' = 3a.x^2 + 2b.x + c
  real aa = 3*a;
  real bb = 2*b;
  real d = bb*bb - 4 * aa * c;
  if (d <= 0) { // the function is monotonous
    if (a > 0)
      return mini;
    else
      return maxi;
  } else {  // the most usual case
    d = sqrt(d);
    real p2 = (-bb + d) / (2*aa);
    if (a > 0) {
      if (p2 < mini || mini == FLT_MIN)
        return mini;
      if (p2 > maxi) { // the minimum is beyond the range
        if (a*mini*mini*mini + b*mini*mini + c*mini > 
            a*maxi*maxi*maxi + b*maxi*maxi + c*maxi)
          return maxi;
        else
          return mini;
      }
      if (a*mini*mini*mini + b*mini*mini + c*mini >
          a*p2*p2*p2 + b*p2*p2 + c*p2)
        return p2;
      else
        return mini;
    } else {
      if (p2 > maxi || maxi == FLT_MAX)
        return maxi;
      if (p2 < mini) { // the minimum is before the range
        if (a*mini*mini*mini + b*mini*mini + c*mini > 
            a*maxi*maxi*maxi + b*maxi*maxi + c*maxi)
          return maxi;
        else
          return mini;
      }
      if (a*maxi*maxi*maxi + b*maxi*maxi + c*maxi >
          a*p2*p2*p2 + b*p2*p2 + c*p2)
        return p2;
      else
        return maxi;
    }
  }
}

//////////////////
// minQuadratic //
//////////////////
real ConjGradientOptimizer::minQuadratic(real a, real b,
    real mini, real maxi) {
  if (a ==  0) {
    if (b > 0)
      return mini;
    else
      return maxi;
  }
  if (a < 0) {
    if (mini == -FLT_MAX)
      return -FLT_MAX;
    if (maxi == FLT_MAX)
      return FLT_MAX;
    if (mini*mini + mini * b / a > maxi*maxi + maxi * b / a)
      return mini;
    else
      return maxi;
  }
  // Now, the most usual case
  real the_min = -b / (2*a);
  if (the_min < mini)
    return mini;
  if (the_min > maxi)
    return maxi;
  return the_min;
}
  
//////////////
// optimize //
//////////////
real ConjGradientOptimizer::optimize()
{
  ofstream out;
  if (!filename.empty()) {
     out.open(filename.c_str());
//     out << " Stochastic! " << endl;  // TODO what's the purpose ?
  }
  Vec meancost(cost->size());
  Vec lastmeancost(cost->size());
  early_stop = false;

  // Initiliazation of the structures
  Vec g(params.nelems());
  Vec h(params.nelems());
  Vec delta(params.nelems());
  Vec tmp_storage(params.nelems());
  computeOppositeGradient(params, cost, proppath, h); 
  g << h;

  // Loop through the epochs
  for (int t=0; !early_stop && t<nupdates; t++) {

    cost->fprop(); // TODO Remove those 2 lines
    cout << "cost = " << cost->value[0] << " " << cost->value[1] << " " << cost->value[2] << endl;
    
    // Make a line search along the current search direction (h)
    lineSearch(computeOppositeGradient, params, cost, proppath, h, 
               starting_step_size, epsilon, tmp_storage);
    
    // Find the new search direction
    early_stop = findDirection(
        computeOppositeGradient, 
        params, 
        cost,
        proppath, 
        starting_step_size, 
        epsilon,
        g,
        h,
        delta,
        tmp_storage);

    // Display results TODO ugly copy/paste : to be cleaned ?
    meancost += cost->value;
    every = 2000; // TODO hack for test purpose
    if ((every!=0) && ((t+1)%every==0)) 
      // normally this is done every epoch
    { 
      //cerr << ">>>>>> nupdates= " << nupdates << "  every=" << every << "  sumofvar->nsamples=" << sumofvar->nsamples << endl;
      meancost /= real(every);
      //if (decrease_constant != 0)
      //  cout << "at t=" << t << ", learning rate = " << learning_rate << endl;
      cout << t+1 << ' ' << meancost << endl;
      if (out)
        out << t+1 << ' ' << meancost << endl;
      early_stop = early_stop || measure(t+1,meancost);
     // early_stop = measure(t+1,meancost); // TODO which is the best between this and the one above ?
      early_stop_i = (t+1)/every;
      lastmeancost << meancost;
      meancost.clear();
    }
  }
  return lastmeancost[0];
}

//////////////////
// polakRibiere //
//////////////////
void ConjGradientOptimizer::polakRibiere (
    void (*grad)(VarArray, Var, VarArray, const Vec&),
    VarArray params,
    Var costs,
    VarArray proppath,
    Vec g,
    Vec h,
    Vec delta) {

  int i;
  // delta = Gradient
  (*grad)(params, costs, proppath, delta);
  real normg = pownorm(g);
  for (i=0; i<h.length(); i++) {
    g[i] = delta[i] - g[i];
  }
  real gamma = dot(g,delta) / normg;
  for (int i=0; i<h.length(); i++) {
    h[i] = delta[i] + gamma * h[i];
  }
  g << delta;
}

%> // end of namespace PLearn
