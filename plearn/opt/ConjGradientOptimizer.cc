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
   * $Id: ConjGradientOptimizer.cc,v 1.12 2003/04/24 14:30:12 tihocan Exp $
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
    real the_sigma,
    real the_rho,
    real the_fmax,
    real the_stop_epsilon,
    real the_tau1,
    real the_tau2,
    real the_tau3,
    int n_updates, const string& filename, 
    int every_iterations)
  :inherited(n_updates, filename, every_iterations),
  starting_step_size(the_starting_step_size),
  epsilon(the_epsilon),
  sigma(the_sigma), rho(the_rho), fmax(the_fmax),
  stop_epsilon(the_stop_epsilon), tau1(the_tau1), tau2(the_tau2),
  tau3(the_tau3) {}

ConjGradientOptimizer::ConjGradientOptimizer(
    VarArray the_params, 
    Var the_cost,
    real the_starting_step_size, 
    real the_epsilon,
    real the_sigma,
    real the_rho,
    real the_fmax,
    real the_stop_epsilon,
    real the_tau1,
    real the_tau2,
    real the_tau3,
    int n_updates, const string& filename, 
    int every_iterations)
  :inherited(the_params, the_cost, n_updates, filename, every_iterations),
  starting_step_size(the_starting_step_size),
  epsilon(the_epsilon),
  sigma(the_sigma), rho(the_rho), fmax(the_fmax),
  stop_epsilon(the_stop_epsilon), tau1(the_tau1), tau2(the_tau2),
  tau3(the_tau3) {}

ConjGradientOptimizer::ConjGradientOptimizer(
    VarArray the_params, 
    Var the_cost, 
    VarArray the_update_for_measure,
    real the_starting_step_size, 
    real the_epsilon,
    real the_sigma,
    real the_rho,
    real the_fmax,
    real the_stop_epsilon,
    real the_tau1,
    real the_tau2,
    real the_tau3,
    int n_updates, const string& filename, 
    int every_iterations)
  :inherited(the_params, the_cost, the_update_for_measure,
             n_updates, filename, every_iterations),
  starting_step_size(the_starting_step_size),
  epsilon(the_epsilon),
  sigma(the_sigma), rho(the_rho), fmax(the_fmax),
  stop_epsilon(the_stop_epsilon), tau1(the_tau1), tau2(the_tau2),
  tau3(the_tau3) {}
  
// 
// declareOptions
// 
void ConjGradientOptimizer::declareOptions(OptionList& ol)
{
    declareOption(ol, "starting_step_size", &ConjGradientOptimizer::starting_step_size, OptionBase::buildoption, 
                  "    the initial step size for the line search algorithm\n");

    declareOption(ol, "epsilon", &ConjGradientOptimizer::epsilon, OptionBase::buildoption, 
                  "    the gradient resolution\n");

    declareOption(ol, "line_search_algo", &ConjGradientOptimizer::line_search_algo, OptionBase::buildoption, 
                  "    the line search algorithm\n");

    declareOption(ol, "find_new_direction_formula", &ConjGradientOptimizer::find_new_direction_formula, OptionBase::buildoption, 
                  "    the formula to find the new search direction\n");

    declareOption(ol, "sigma", &ConjGradientOptimizer::sigma, OptionBase::buildoption, 
                  "    sigma\n");

    declareOption(ol, "rho", &ConjGradientOptimizer::rho, OptionBase::buildoption, 
                  "    rho\n");

    declareOption(ol, "fmax", &ConjGradientOptimizer::fmax, OptionBase::buildoption, 
                  "    the value we will stop at if we manage to reach it\n");

    declareOption(ol, "stop_epsilon", &ConjGradientOptimizer::stop_epsilon, OptionBase::buildoption, 
                  "    the epsilon parameter to stop when we can't make any more progress\n");

    declareOption(ol, "tau1", &ConjGradientOptimizer::tau1, OptionBase::buildoption, 
                  "    tau1\n");

    declareOption(ol, "tau2", &ConjGradientOptimizer::tau2, OptionBase::buildoption, 
                  "    tau2\n");

    declareOption(ol, "tau3", &ConjGradientOptimizer::tau3, OptionBase::buildoption, 
                  "    tau3\n");

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
  writeField(out, "line_search_algo", line_search_algo);
  writeField(out, "find_new_direction_formula", find_new_direction_formula);
  writeField(out, "sigma", sigma);
  writeField(out, "rho", rho);
  writeField(out, "fmax", fmax);
  writeField(out, "stop_epsilon", stop_epsilon);
  writeField(out, "tau1", tau1);
  writeField(out, "tau2", tau2);
  writeField(out, "tau3", tau3);
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
  readField(in, "line_search_algo", line_search_algo);
  readField(in, "find_new_direction_formula", find_new_direction_formula);
  readField(in, "sigma", sigma);
  readField(in, "rho", rho);
  readField(in, "fmax", fmax);
  readField(in, "stop_epsilon", stop_epsilon);
  readField(in, "tau1", tau1);
  readField(in, "tau2", tau2);
  readField(in, "tau3", tau3);
  readFooter(in, "ConjGradientOptimizer");
}

//
// Implement name and deepCopy
//
IMPLEMENT_NAME_AND_DEEPCOPY(ConjGradientOptimizer);

/******************************
 * MAIN METHODS AND FUNCTIONS *
 ******************************/

//////////////////////
// computeCostValue //
//////////////////////
real ConjGradientOptimizer::computeCostValue(
    real alpha,
    ConjGradientOptimizer* opt) {
  if (alpha == 0)
    return opt->cost->value[0];
  opt->params.copyTo(opt->tmp_storage);
  opt->params.update(alpha, opt->search_direction);
  opt->proppath.fprop();
  real c = opt->cost->value[0];
  opt->params.copyFrom(opt->tmp_storage);
  return c;
}

///////////////////////
// computeDerivative //
///////////////////////
real ConjGradientOptimizer::computeDerivative(
    real alpha,
    ConjGradientOptimizer* opt) {
  if (alpha == 0)
    return -dot(opt->search_direction, opt->current_opp_gradient);
  opt->params.copyTo(opt->tmp_storage);
  opt->params.update(alpha, opt->search_direction);
  computeGradient(opt, opt->delta);
  opt->params.copyFrom(opt->tmp_storage);
  return dot(opt->search_direction, opt->delta);
}

/////////////////////
// computeGradient //
/////////////////////
void ConjGradientOptimizer::computeGradient(
    Optimizer* opt,
    const Vec& gradient) {
  // Clear all what's left from previous computations
  opt->proppath.clearGradient();
  opt->params.clearGradient();
  opt->cost->gradient[0] = 1;
  opt->proppath.fbprop();
  opt->params.copyGradientTo(gradient);
}
  
/////////////////////////////
// computeOppositeGradient //
/////////////////////////////
void ConjGradientOptimizer::computeOppositeGradient(
    Optimizer* opt,
    const Vec& gradient) {
  // Clear all what's left from previous computations
  opt->proppath.clearGradient();
  opt->params.clearGradient();
  // We want the opposite of the gradient, thus the -1
  opt->cost->gradient[0] = -1;
  opt->proppath.fbprop();
  opt->params.copyGradientTo(gradient);
}
  
///////////////
// conjpomdp //
///////////////
bool ConjGradientOptimizer::conjpomdp (
    void (*grad)(Optimizer*, const Vec& gradient),
    ConjGradientOptimizer* opt) {
  int i;
  // delta = Gradient
  (*grad)(opt, opt->delta);
  real norm_g = pownorm(opt->current_opp_gradient);
  // g <- delta - g (g = current_opp_gradient)
  for (i=0; i<opt->current_opp_gradient.length(); i++) {
    opt->current_opp_gradient[i] = opt->delta[i]-opt->current_opp_gradient[i];
  }
  real gamma = dot(opt->current_opp_gradient, opt->delta) / norm_g;
  // h <- delta + gamma * h (h = search_direction)
  for (i=0; i<opt->search_direction.length(); i++) {
    opt->search_direction[i] = opt->delta[i] + gamma * opt->search_direction[i];
  }
  if (dot(opt->search_direction, opt->delta) < 0) {
    // h <- delta
    opt->search_direction << opt->delta;
  }
  // g <- delta
  opt->current_opp_gradient << opt->delta;
  // We want to stop when the norm of the gradient is small enough
  return (pownorm(opt->current_opp_gradient) < opt->epsilon);
};

///////////////////
// cubicInterpol //
///////////////////
void ConjGradientOptimizer::cubicInterpol(
    real f0, real f1, real g0, real g1,
    real& a, real& b, real& c, real& d) {
  d = f0;
  c = g0;
  b = 3*(f1-f0) - 2*g0 - g1;
  a = g0 + g1 - 2*(f1 - f0);
}

/////////////
// dayYuan //
/////////////
void ConjGradientOptimizer::daiYuan (
    void (*grad)(Optimizer*, const Vec&),
    ConjGradientOptimizer* opt) {
  // delta = opposite gradient
  (*grad)(opt, opt->delta);
  real gamma = daiYuanMain(
      opt->delta, opt->current_opp_gradient, opt->search_direction,
      opt->tmp_storage);
  for (int i=0; i<opt->search_direction.length(); i++) {
    opt->search_direction[i] = 
      opt->delta[i] + gamma * opt->search_direction[i];
  }
  opt->current_opp_gradient << opt->delta;
}

/////////////////
// daiYuanMain //
/////////////////
real ConjGradientOptimizer::daiYuanMain (
    Vec new_gradient,
    Vec old_gradient,
    Vec old_search_direction,
    Vec tmp_storage) {
  real norm_grad = pownorm(new_gradient);
  for (int i=0; i<old_gradient.length(); i++) {
    tmp_storage[i] = -new_gradient[i] + old_gradient[i];
  }
  real gamma = norm_grad / dot(old_search_direction, tmp_storage);
  return gamma;
}

///////////////////
// findDirection //
///////////////////
bool ConjGradientOptimizer::findDirection() {
  bool isFinished = false;
  switch (find_new_direction_formula) {
    case 0:
      isFinished = conjpomdp(computeOppositeGradient, this);
      break;
    case 1:
      daiYuan(computeOppositeGradient, this);
      break;
    case 2:
      fletcherReeves(computeOppositeGradient, this);
      break;
    case 3:
      hestenesStiefel(computeOppositeGradient, this);
      break;
    case 4:
      polakRibiere(computeOppositeGradient, this);
      break;
  }
  return isFinished;
}

//////////////////////////////
// findMinWithCubicInterpol //
//////////////////////////////
real ConjGradientOptimizer::findMinWithCubicInterpol (
    real p1,
    real p2,
    real mini,
    real maxi,
    real f0,
    real f1,
    real g0,
    real g1) {
  // We prefer p2 > p1 and maxi > mini
  real tmp;
  if (p2 < p1) {
    tmp = p2;
    p2 = p1;
    p1 = tmp;
  }
  if (maxi < mini) {
    tmp = maxi;
    maxi = mini;
    mini = tmp;
  }
  // cout << "Finding min : p1 = " << p1 << " , p2 = " << p2 << " , mini = " << mini << " , maxi = " << maxi << endl;
  // The derivatives must be multiplied by (p2-p1), because we write :
  // x = p1 + z*(p2-p1)
  // with z in [0,1]  =>  df/dz = (p2-p1)*df/dx
  g0 = g0 * (p2-p1);
  g1 = g1 * (p2-p1);
  real a, b, c, d;
  // Store the interpolation coefficients in a,b,c,d
  cubicInterpol(f0, f1, g0, g1, a, b, c, d);
  // cout << "Interpol : a=" << a << " , b=" << b << " , c=" << c << " , d=" << d << endl;
  real mini_transformed = mini;
  real maxi_transformed = maxi;
  if (mini != -FLT_MAX)
    mini_transformed = (mini - p1) / (p2 - p1);
  if (maxi != FLT_MAX)
    maxi_transformed = (maxi - p1) / (p2 - p1);
  real xmin = minCubic(a, b, c, mini_transformed, maxi_transformed);
  if (xmin == -FLT_MAX || xmin == FLT_MAX)
    return xmin;
  // cout << "min is : xmin = " << p1 + xmin*(p2-p1) << endl;
  return p1 + xmin*(p2-p1);
}

////////////////////
// fletcherReeves //
////////////////////
void ConjGradientOptimizer::fletcherReeves (
    void (*grad)(Optimizer*, const Vec&),
    ConjGradientOptimizer* opt) {
  // delta = opposite gradient
  (*grad)(opt, opt->delta);
  real gamma = pownorm(opt->delta) / pownorm(opt->current_opp_gradient);
  for (int i=0; i<opt->search_direction.length(); i++) {
    opt->search_direction[i] = 
      opt->delta[i] + gamma * opt->search_direction[i];
  }
  opt->current_opp_gradient << opt->delta;
}

////////////////////
// fletcherSearch //
////////////////////
void ConjGradientOptimizer::fletcherSearch (real mu) {
  real alpha = fletcherSearchMain (
      computeCostValue,
      computeDerivative,
      this,
      sigma,
      rho,
      fmax,
      stop_epsilon,
      tau1,
      tau2,
      tau3,
      starting_step_size,
      mu);
  params.update(alpha, search_direction);
}

////////////////////////
// fletcherSearchMain //
////////////////////////
real ConjGradientOptimizer::fletcherSearchMain (
    real (*f)(real, ConjGradientOptimizer* opt),
    real (*g)(real, ConjGradientOptimizer* opt),
    ConjGradientOptimizer* opt,
    real sigma,
    real rho,
    real fmax,
    real epsilon,
    real tau1,
    real tau2,
    real tau3,
    real alpha1,
    real mu) {
  
  // Initialization
  real alpha0 = 0;
  // f0 = f(0), f_0 = f(alpha0), f_1 = f(alpha1)
  // g0 = g(0), g_0 = g(alpha0), g_1 = g(alaph1)
  // (for the bracketing phase)
  real alpha2, f0, f_1, f_0, g0, g_1, g_0, a1, a2, b1, b2;
  f0 = (*f)(0, opt);
  g0 = (*g)(0, opt);
  f_0 = f0;
  g_0 = g0;
  if (mu == FLT_MAX)
    mu = (fmax - f0) / (rho * g0);
  if (alpha1 == FLT_MAX)
    alpha1 = mu / 100; // My own heuristic
  if (g0 >= 0) {
    cout << "Warning : df/dx(0) >= 0 !" << endl;
    return 0;
  }
  bool isBracketed = false;
  
  // Bracketing
  while (!isBracketed) {
    // cout << "Bracketing : alpha1 = " << alpha1 << endl << "             alpha0 = " << alpha0 << endl;
    if (alpha1 == mu && alpha1 == alpha2) { // NB: Personal hack... hopefully that should not happen
      cout << "Warning : alpha1 == alpha2 == mu during bracketing" << endl;
      return alpha1;
    }
    f_1 = (*f)(alpha1, opt);
    if (f_1 <= fmax)
      return alpha1;
    if (f_1 > f0 + alpha1 * rho * g0 || f_1 > f_0) {
      // NB: in Fletcher's book, there is a typo in the test above
      a1 = alpha0;
      b1 = alpha1;
      isBracketed = true;
      // cout << "Bracketing done : f_1 = " << f_1 << " , alpha1 = " << alpha1 << " , f0 = " << f0 << " , g0 = " << g0 << endl;
    } else {
      g_1 = (*g)(alpha1, opt);
      if (abs(g_1) < -sigma * g0) {
        // cout << "Low gradient : g=" << abs(g1) << " < " << (-sigma * g0) << endl;
        return alpha1;
      }
      if (g_1 >= 0) {
        a1 = alpha1;
        b1 = alpha0;
        real tmp = f_0;
        f_0 = f_1;
        f_1 = tmp;
        tmp = g_0;
        g_0 = g_1;
        g_1 = tmp;
        isBracketed = true;
      } else {
        if (mu <= 2*alpha1 - alpha0)
          alpha2 = mu;
        else
          alpha2 = findMinWithCubicInterpol(
              alpha1, alpha0,
              2*alpha1 - alpha0, min(mu, alpha1 + tau1 * (alpha1 - alpha0)),
              f_1, f_0, g_1, g_0);
      }
    }
    if (!isBracketed) {
      alpha0 = alpha1;
      alpha1 = alpha2;
      f_0 = f_1;
      g_0 = g_1;
    }
  }

  // Splitting
  // NB: At this point, f_0 = f(a1), f_1 = f(b1) (and the same for g)
  //     and we'll keep it this way
  //     We then use f1 = f(alpha1) and g1 = g(alpha1)
  real f1,g1;
  while (true) {
    // cout << "Splitting : alpha1 = " << alpha1 << endl << "            a1 = " << a1 << endl << "            b1 = " << b1 << endl;
    // cout << "Interval : [" << a1 + tau2 * (b1-a1) << " , " << b1 - tau3 * (b1-a1) << "]" << endl;
    alpha1 = findMinWithCubicInterpol(
        a1, b1,
        a1 + tau2 * (b1-a1), b1 - tau3 * (b1-a1),
        f_0, f_1, g_0, g_1);
    f1 = (*f)(alpha1, opt);
    if ((a1 - alpha1) * g_0 <= epsilon) {
      // cout << "Early stop : a1 = " << a1 << " , alpha1 = " << alpha1 << " , g(a1) = " << g_0 << " , epsilon = " << epsilon << endl;
      return a1;
    }
    g1 = (*g)(alpha1, opt);
    if (f1 > f0 + rho * alpha1 * g0 || f1 >= f_0) {
     a2 = a1;
     b2 = alpha1;
     f_1 = f1; // to keep f_1 = f(b1) in the next iteration
     g_1 = g1; // to keep g_1 = g(b1) in the next iteration
    } else {
      if (abs(g1) <= -sigma * g0) {
        // cout << "Found what we were looking for : g(alpha1)=" << abs(g1) << " < " << -sigma * g0 << " with g0 = " << g0 << endl;
        return alpha1;
      }
      if ((b1 - a1) * g1 >= 0) {
        b2 = a1;
        f_1 = f_0; // to keep f_1 = f(b1) in the next iteration
        g_1 = g_0; // to keep g_1 = g(b1) in the next iteration
      } else
        b2 = b1;
      a2 = alpha1;
      f_0 = f1; // to keep f_0 = f(a1) in the next iteration
      g_0 = g1; // to keep g_0 = g(a1) in the next iteration
    }
    a1 = a2;
    b1 = b2;
  }
}

/////////////
// gSearch //
/////////////
void ConjGradientOptimizer::gSearch (void (*grad)(Optimizer*, const Vec&)) {

  real step = starting_step_size;
  real sp, sm, pp, pm;

  // Backup the initial paremeters values
  params.copyTo(tmp_storage);

  params.update(step, search_direction);
  (*grad)(this, delta);
  real prod = dot(delta, search_direction);

  if (prod < 0) {
    // Step back to bracket the maximum
    while (prod < -epsilon) {
      sp = step;
      pp = prod;
      step = step / 2;
      params.update(-step, search_direction);
      (*grad)(this, delta);
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
      (*grad)(this, delta);
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
    void (*grad)(Optimizer*, const Vec&),
    ConjGradientOptimizer* opt) {
  int i;
  // delta = opposite gradient
  (*grad)(opt, opt->delta);
  for (i=0; i<opt->current_opp_gradient.length(); i++) {
    opt->current_opp_gradient[i] = opt->delta[i] - opt->current_opp_gradient[i];
  }
  real gamma = -dot(opt->delta, opt->current_opp_gradient) / dot(opt->search_direction, opt->current_opp_gradient);
  for (i=0; i<opt->search_direction.length(); i++) {
    opt->search_direction[i] = opt->delta[i] + gamma * opt->search_direction[i];
  }
  opt->current_opp_gradient << opt->delta;
}

////////////////
// lineSearch //
////////////////
real ConjGradientOptimizer::lineSearch() {
  switch (line_search_algo) {
    case 0:
      fletcherSearch();
      break;
    case 1:
      gSearch(computeOppositeGradient);
      break;
  }
  return cost->value[0];
}

//////////////
// minCubic //
//////////////
real ConjGradientOptimizer::minCubic(
    real a, real b, real c,
    real mini, real maxi) {
  if (a == 0 || (b != 0 && abs(a/b) < 0.0001)) // heuristic value for a == 0
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
      if (p2 < mini || mini == -FLT_MAX)
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
real ConjGradientOptimizer::minQuadratic(
    real a, real b,
    real mini, real maxi) {
  if (a == 0 || (b != 0 && abs(a/b) < 0.0001)) { // heuristic for a == 0
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
  computeOppositeGradient(this, current_opp_gradient);
  search_direction <<  current_opp_gradient;  // first direction = -grad;
  last_improvement = 0.1; // why not ?
  real last_cost, current_cost;
  cost->fprop();
  last_cost = cost->value[0];
  real df;

  // Loop through the epochs
  for (int t=0; !early_stop && t<nupdates; t++) {

    // TODO Remove this line later
    cout << "cost = " << cost->value[0] << " " << cost->value[1] << " " << cost->value[2] << endl;
    
    // Make a line search along the current search direction
    current_cost = lineSearch();
    
    // Find the new search direction
    early_stop = findDirection();

    last_improvement = last_cost - current_cost;
    last_cost = current_cost;

    // This value of starting_step_size is suggested by Fletcher
    df = max (last_improvement, 10*stop_epsilon);
    starting_step_size = 2*df / dot(search_direction, current_opp_gradient);
    
    // Display results TODO ugly copy/paste from GradientOptimizer: to be cleaned ?
    meancost += cost->value;
    every = 2000; // TODO Remove later, this is for test purpose
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
     // early_stop = measure(t+1,meancost); // TODO find which is the best between this and the one above
      early_stop_i = (t+1)/every;
      lastmeancost << meancost;
      meancost.clear();
    }
  }
  if (early_stop)
    cout << "Early Stopping !" << endl;
  return lastmeancost[0];
}

//////////////////
// polakRibiere //
//////////////////
void ConjGradientOptimizer::polakRibiere (
    void (*grad)(Optimizer*, const Vec&),
    ConjGradientOptimizer* opt) {
  int i;
  // delta = Gradient
  (*grad)(opt, opt->delta);
  real normg = pownorm(opt->current_opp_gradient);
  for (i=0; i<opt->current_opp_gradient.length(); i++) {
    opt->current_opp_gradient[i] = opt->delta[i] - opt->current_opp_gradient[i];
  }
  real gamma = dot(opt->current_opp_gradient,opt->delta) / normg;
  for (int i=0; i<opt->search_direction.length(); i++) {
    opt->search_direction[i] = opt->delta[i] + gamma * opt->search_direction[i];
  }
  opt->current_opp_gradient << opt->delta;
}

///////////////////////
// quadraticInterpol //
///////////////////////
void ConjGradientOptimizer::quadraticInterpol(
    real f0, real f1, real g0,
    real& a, real& b, real& c) {
  c = f0;
  b = g0;
  a = f1 - f0 - g0;
}

%> // end of namespace PLearn
