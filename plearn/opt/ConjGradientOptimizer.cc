// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Olivier Delalleau
// Copyright (c) 1996-2001, Ian T. Nabney for functions minBrack, brentSearch

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
 * This file is part of the PLearn library.
 ******************************************************* */

#include "ConjGradientOptimizer.h"
#include <plearn/math/TMat_maths_impl.h>

namespace PLearn {
using namespace std;

//
// Constructors
//
ConjGradientOptimizer::ConjGradientOptimizer(
    real the_starting_step_size, 
    real the_restart_coeff,
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
     verbosity(2),
     compute_cost(1),
     line_search_algo(1),
     find_new_direction_formula(1),
     starting_step_size(the_starting_step_size), restart_coeff(the_restart_coeff),
     epsilon(the_epsilon),
     sigma(the_sigma), rho(the_rho), fmax(the_fmax),
     stop_epsilon(the_stop_epsilon), tau1(the_tau1), tau2(the_tau2),
     tau3(the_tau3), max_steps(5), initial_step(0.01), low_enough(1e-6),
     position_res(1e-4), value_res(1e-4), n_iterations(100)  {}

ConjGradientOptimizer::ConjGradientOptimizer(
    VarArray the_params, 
    Var the_cost,
    real the_starting_step_size, 
    real the_restart_coeff,
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
     verbosity(2),
     starting_step_size(the_starting_step_size), restart_coeff(the_restart_coeff),
     epsilon(the_epsilon),
     sigma(the_sigma), rho(the_rho), fmax(the_fmax),
     stop_epsilon(the_stop_epsilon), tau1(the_tau1), tau2(the_tau2),
     tau3(the_tau3)  {
    cout << "Warning: you should use the constructor ConjGradientOptimizer(), or some default options may not be set properly" << endl;
}

ConjGradientOptimizer::ConjGradientOptimizer(
    VarArray the_params, 
    Var the_cost, 
    VarArray the_update_for_measure,
    real the_starting_step_size, 
    real the_restart_coeff,
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
     verbosity(2),
     starting_step_size(the_starting_step_size), restart_coeff(the_restart_coeff),
     epsilon(the_epsilon),
     sigma(the_sigma), rho(the_rho), fmax(the_fmax),
     stop_epsilon(the_stop_epsilon), tau1(the_tau1), tau2(the_tau2),
     tau3(the_tau3) {
    cout << "Warning: you should use the constructor ConjGradientOptimizer(), or some default options may not be set properly" << endl;
}
  
PLEARN_IMPLEMENT_OBJECT(ConjGradientOptimizer,
                        "Optimizer based on the conjugate gradient method.",
                        "The conjugate gradient algorithm is basically the following :\n"
                        "- 0: initialize the search direction d = -gradient\n"
                        "- 1: perform a line search along direction d for the minimum of the\n"
                        "     gradient\n"
                        "- 2: move to this minimum, update the search direction d and go to\n"
                        "     step 1\n"
                        "There are various methods available through the options for both\n"
                        "steps 1 and 2.");

// 
// declareOptions
// 
void ConjGradientOptimizer::declareOptions(OptionList& ol)
{
    declareOption(ol, "starting_step_size",
                  &ConjGradientOptimizer::starting_step_size, OptionBase::buildoption, 
                  "The initial step size for the line search algorithm.\n"
                  "  This option has very little influence on the algorithm performance, \n"
                  "  since it is only used for the first iteration, and is set by the\n"
                  "  algorithm in the later stages.\n");

    declareOption(ol, "restart_coeff", &ConjGradientOptimizer::restart_coeff,
                  OptionBase::buildoption, 
                  "The restart coefficient.\n"
                  "  A restart is triggered when the following condition is met:\n"
                  "    abs(gradient(k-1).gradient(k)) >= restart_coeff * gradient(k)^2\n"
                  "  If no restart is wanted, any high value (e.g. 100) will prevent it.\n");

    declareOption(ol, "line_search_algo",
                  &ConjGradientOptimizer::line_search_algo, OptionBase::buildoption, 
                  "The kind of line search algorithm used :\n"
                  "  1. The line search algorithm described in :\n"
                  "     \"Practical Methods of Optimization, 2nd Ed\", by Fletcher (1987).\n"
                  "     This algorithm seeks a minimum m of f(x)=C(x0+x.d) satisfying the\n"
                  "     two constraints:\n"
                  "        i. abs(f'(m)) < -sigma.f'(0)\n"
                  "       ii. f(m) < f(0) + m.rho.f'(0)\n"
                  "     with rho < sigma <= 1.\n"
                  "  2. The GSearch algorithm, described in:\n"
                  "     \"Direct Gradient-Based Reinforcement Learning:\n"
                  "       II. Gradient Ascent Algorithms and Experiments\"\n"
                  "      by J.Baxter, L. Weaver, P. Bartlett (1999).\n"
                  "  3. A Newton line search algorithm for quadratic costs only.\n"
                  "  4. Brent's line search algorithm.\n");

    declareOption(ol, "find_new_direction_formula",
                  &ConjGradientOptimizer::find_new_direction_formula,
                  OptionBase::buildoption, 
                  "The kind of formula used in step 2 of the conjugate gradient\n"
                  "  algorithm to find the new search direction:\n"
                  "  1. ConjPOMPD: the formula associated with GSearch in the same paper.\n"
                  "     It is almost the same as the Polak-Ribiere formula.\n"
                  "  2. Dai - Yuan\n"
                  "  3. Fletcher - Reeves\n"
                  "  4. Hestenes - Stiefel\n"
                  "  5. Polak - Ribiere: this is probably the most commonly used.\n"
                  "  The value of this option (from 1 to 5) indicates the formula used.\n");

    declareOption(ol, "epsilon", &ConjGradientOptimizer::epsilon,
                  OptionBase::buildoption, 
                  "GSearch specific option: the gradient resolution.\n"
                  "  This small value is used in the GSearch algorithm instead of 0,\n"
                  "  to provide some robustness against errors in the estimate of the\n"
                  "  gradient, and to prevent the algorithm from looping indefinitely if\n"
                  "  there is no local minimum.\n"
                  "  Any small value should work, and the overall performance should\n"
                  "  not depend much on it.\n ");
    // TODO Check the last sentence is true !

    declareOption(ol, "sigma", &ConjGradientOptimizer::sigma,
                  OptionBase::buildoption, 
                  "Fletcher's line search specific option: constraint parameter in i.\n");

    declareOption(ol, "rho", &ConjGradientOptimizer::rho,
                  OptionBase::buildoption, 
                  "Fletcher's line search specific option : constraint parameter in ii.\n");

    declareOption(ol, "fmax", &ConjGradientOptimizer::fmax,
                  OptionBase::buildoption, 
                  "Fletcher's line search specific option : good enough minimum.\n"
                  "  If it finds a point n such that f(n) < fmax,\n"
                  "  then the line search returns n as minimum\n"
                  "  As a consequence, DO NOT USE 0 if f can be negative\n");

    declareOption(ol, "stop_epsilon", &ConjGradientOptimizer::stop_epsilon,
                  OptionBase::buildoption, 
                  "Fletcher's line search specific option : stopping criterion.\n"
                  "  This option allows the algorithm to detect when no improvement\n"
                  "  is possible along the search direction.\n"
                  "  It should be set to a small value, or we may stop too early.\n"
                  "IMPORTANT: this same option is also used to compute the next step size.\n"
                  "  If you get a NaN in the cost, try a higher stop_epsilon.\n");

    declareOption(ol, "tau1", &ConjGradientOptimizer::tau1,
                  OptionBase::buildoption, 
                  "Fletcher's line search specific option : bracketing parameter.\n"
                  "  This option controls how fast is augmenting the bracketing\n"
                  "  interval in the first phase of the line search algorithm.\n"
                  "  Fletcher reports good empirical results with tau1 = 9.\n");

    declareOption(ol, "tau2", &ConjGradientOptimizer::tau2,
                  OptionBase::buildoption, 
                  "Fletcher's line search specific option : bracketing parameter.\n"

                  "  This option controls how fast is augmenting the left side of the\n"
                  "  bracketing interval in the second phase of the line search algorithm.\n"
                  "  Fletcher reports good empirical results with tau2 = 0.1\n");

    declareOption(ol, "tau3", &ConjGradientOptimizer::tau3,
                  OptionBase::buildoption, 
                  "Fletcher's line search specific option : bracketing parameter.\n"
                  "  This option controls how fast is decreasing the right side of the\n"
                  "  bracketing interval in the second phase of the line search algorithm.\n"
                  "  Fletcher reports good empirical results with tau3 = 0.5\n");

    declareOption(ol, "max_steps", &ConjGradientOptimizer::max_steps,
                  OptionBase::buildoption, 
                  "Newton line search specific option:\n"
                  "  maximum number of steps at each iteration.\n"
                  "  This option defines how many steps will be performed to find the\n"
                  "  minimum, if no value < low_enough is found for the gradient (this\n"
                  "  can happen if the cost isn't perfectly quadratic).\n");

    declareOption(ol, "initial_step", &ConjGradientOptimizer::initial_step,
                  OptionBase::buildoption, 
                  "Newton line search specific option: value of the first step.\n"
                  "  This options controls the size of the first step made in the\n"
                  "  search direction.\n");

    declareOption(ol, "low_enough", &ConjGradientOptimizer::low_enough,
                  OptionBase::buildoption, 
                  "Newton line search specific option:\n"
                  "  stopping criterion for the gradient.\n"
                  "  We say the minimum has been found if we have\n"
                  "  abs(gradient) < low_enough.\n");

    declareOption(ol, "position_res", &ConjGradientOptimizer::position_res,
                  OptionBase::buildoption, 
                  "Brent line search specific option:\n"
                  "  resolution of the point coordinates.\n");

    declareOption(ol, "value_res", &ConjGradientOptimizer::value_res,
                  OptionBase::buildoption,
                  "Brent line search specific option:\n"
                  "  resolution of the point value.\n");

    declareOption(ol, "n_iterations", &ConjGradientOptimizer::n_iterations,
                  OptionBase::buildoption,
                  "Brent line search specific option: number of iterations.\n");

    declareOption(ol, "compute_cost", &ConjGradientOptimizer::compute_cost,
                  OptionBase::buildoption, 
                  "If set to 1, will compute and display the mean cost at each epoch.\n");

    declareOption(ol, "verbosity", &ConjGradientOptimizer::verbosity,
                  OptionBase::buildoption, 
                  "Controls the amount of output.\n");

    inherited::declareOptions(ol);
}


/******************************
 * MAIN METHODS AND FUNCTIONS *
 ******************************/

////////////
// build_ //
////////////
void ConjGradientOptimizer::build_() {
    // Make sure the internal data have the right size.
    int n = params.nelems();
    if (n > 0) {
        current_opp_gradient.resize(n);
        search_direction.resize(n);
        tmp_storage.resize(n);
        delta.resize(n);
        if (cost.length() > 0) {
            meancost.resize(cost->size());
        }
    }
}

//////////////////////////////
// computeCostAndDerivative //
//////////////////////////////
void ConjGradientOptimizer::computeCostAndDerivative(
    real alpha, ConjGradientOptimizer* opt, real& cost, real& derivative) {
    if (alpha == 0) {
        cost = opt->last_cost;
        derivative = -dot(opt->search_direction, opt->current_opp_gradient);
    } else {
        // TODO See why different from computeDerivative.
        opt->params.copyTo(opt->tmp_storage);
        opt->params.update(alpha, opt->search_direction);
        opt->proppath.clearGradient();
        opt->params.clearGradient();
        opt->cost->gradient[0] = 1;
        opt->proppath.fbprop();
        opt->params.copyGradientTo(opt->delta);
        cost = opt->cost->value[0];
        derivative = dot(opt->search_direction, opt->delta);
        opt->params.copyFrom(opt->tmp_storage);
    }
}

//////////////////////
// computeCostValue //
//////////////////////
real ConjGradientOptimizer::computeCostValue(
    real alpha,
    ConjGradientOptimizer* opt) {
    if (alpha == 0) {
        return opt->last_cost;
    }
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
    Optimizer::computeGradient(opt, opt->delta);
    opt->params.copyFrom(opt->tmp_storage);
    return dot(opt->search_direction, opt->delta);
}

///////////////
// conjpomdp //
///////////////
real ConjGradientOptimizer::conjpomdp (
    void (*grad)(Optimizer*, const Vec& gradient),
    ConjGradientOptimizer* opt) {
    int i;
    // delta = Gradient
    real norm_g = pownorm(opt->current_opp_gradient);
    // tmp_storage <- delta - g (g = current_opp_gradient)
    for (i=0; i<opt->current_opp_gradient.length(); i++) {
        opt->tmp_storage[i] = opt->delta[i]-opt->current_opp_gradient[i];
    }
    real gamma = dot(opt->tmp_storage, opt->delta) / norm_g;
    // h <- delta + gamma * h (h = search_direction)
    for (i=0; i<opt->search_direction.length(); i++) {
        opt->tmp_storage[i] = opt->delta[i] + gamma * opt->search_direction[i];
    }
    if (dot(opt->tmp_storage, opt->delta) < 0)
        return 0;
    else
        return gamma;
}

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
real ConjGradientOptimizer::daiYuan (
    void (*grad)(Optimizer*, const Vec&),
    ConjGradientOptimizer* opt) {
    real norm_grad = pownorm(opt->delta);
    for (int i=0; i<opt->current_opp_gradient.length(); i++) {
        opt->tmp_storage[i] = -opt->delta[i] + opt->current_opp_gradient[i];
    }
    real gamma = norm_grad / dot(opt->search_direction, opt->tmp_storage);
    return gamma;
}

///////////////////
// findDirection //
///////////////////
bool ConjGradientOptimizer::findDirection() {
    bool isFinished = false;
    real gamma = 0;
    switch (find_new_direction_formula) {
    case 1:
        gamma = conjpomdp(computeOppositeGradient, this);
        break;
    case 2:
        gamma = daiYuan(computeOppositeGradient, this);
        break;
    case 3:
        gamma = fletcherReeves(computeOppositeGradient, this);
        break;
    case 4:
        gamma = hestenesStiefel(computeOppositeGradient, this);
        break;
    case 5:
        gamma = polakRibiere(computeOppositeGradient, this);
        break;
    default:
        PLERROR("In ConjGradientOptimizer::findDirection - Invalid conjugate gradient formula !");
        break;
    }
    // It is suggested to keep gamma >= 0
    if (gamma < 0) {
        if (verbosity >= 2)
            cout << "gamma < 0 ! gamma = " << gamma << " ==> Restarting" << endl;
        gamma = 0;
    }
    else {
        real dp = dot(delta, current_opp_gradient);
        real delta_n = pownorm(delta);
        if (abs(dp) > restart_coeff *delta_n ) {
            if (verbosity >= 5)
                cout << "Restart triggered !" << endl;
            gamma = 0;
        }
    }
    updateSearchDirection(gamma);
    // If the gradient is very small, we can stop !
//  isFinished = pownorm(current_opp_gradient) < 0.0000001;
    // TODO This may lead to an erroneous early stop. To investigate ?
    isFinished = false;
    if (isFinished && verbosity >= 2)
        cout << "Gradient is small enough, time to stop" << endl;
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

/////////////////////////////
// findMinWithQuadInterpol //
/////////////////////////////
real ConjGradientOptimizer::findMinWithQuadInterpol(
    int q, real sum_x, real sum_x_2, real sum_x_3, real sum_x_4,
    real sum_c_x_2, real sum_g_x, real sum_c_x, real sum_c, real sum_g) {

    real q2 = q*q;
    real sum_x_2_quad = sum_x_2 * sum_x_2;
    real sum_x_quad = sum_x*sum_x;
    real sum_x_2_sum_x = sum_x_2*sum_x;
    real sum_x_2_sum_x_3 = sum_x_2*sum_x_3;
    real sum_x_3_sum_x = sum_x_3*sum_x;
    // TODO This could certainly be optimized more.
    real denom = 
        (-4*q2*sum_x_2 - 3*q*sum_x_2_quad + sum_x_2_quad*sum_x_2 + 
         q*sum_x_3*sum_x_3 - q2*sum_x_4 - q*sum_x_2*sum_x_4 + 4*q*sum_x_3_sum_x - 
         2*sum_x_2_sum_x_3*sum_x + 4*q*sum_x_quad + sum_x_4*sum_x_quad);

    real a =
        -(q2*sum_c_x_2 + 2*q2*sum_g_x - q*sum_c*sum_x_2 + 
          q*sum_c_x_2*sum_x_2 + 2*q*sum_g_x*sum_x_2 - sum_c*sum_x_2_quad - 
          q*sum_c_x*sum_x_3 - q*sum_g*sum_x_3 - 2*q*sum_c_x*sum_x - 
          2*q*sum_g*sum_x + sum_c_x*sum_x_2_sum_x + sum_g*sum_x_2_sum_x + 
          sum_c*sum_x_3_sum_x + 2*sum_c*sum_x_quad - sum_c_x_2*sum_x_quad - 
          2*sum_g_x*sum_x_quad) / denom;
  
    real b =
        -(4*q*sum_c_x*sum_x_2 + 4*q*sum_g*sum_x_2 - 
          sum_c_x*sum_x_2_quad - sum_g*sum_x_2_quad - q*sum_c_x_2*sum_x_3 - 
          2*q*sum_g_x*sum_x_3 + sum_c*sum_x_2_sum_x_3 + q*sum_c_x*sum_x_4 + 
          q*sum_g*sum_x_4 - 2*q*sum_c_x_2*sum_x - 4*q*sum_g_x*sum_x - 
          2*sum_c*sum_x_2_sum_x + sum_c_x_2*sum_x_2_sum_x + 
          2*sum_g_x*sum_x_2_sum_x - sum_c*sum_x_4*sum_x) / denom;

    real xmin = -b / (2*a);
    return xmin;

}

////////////////////
// fletcherReeves //
////////////////////
real ConjGradientOptimizer::fletcherReeves (
    void (*grad)(Optimizer*, const Vec&),
    ConjGradientOptimizer* opt) {
    // delta = opposite gradient
    real gamma = pownorm(opt->delta) / pownorm(opt->current_opp_gradient);
    return gamma;
}

////////////////////
// fletcherSearch //
////////////////////
real ConjGradientOptimizer::fletcherSearch (real mu) {
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
        current_step_size,
        mu);
    return alpha;
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
    // g0 = g(0), g_0 = g(alpha0), g_1 = g(alpha1)
    // (for the bracketing phase)
    real alpha2, f0, f_1=0, f_0, g0, g_1=0, g_0, a1=0, a2, b1=0, b2;
    g0 = (*g)(0, opt);
    f0 = (*f)(0, opt);
    f_0 = f0;
    g_0 = g0;
    if (mu == FLT_MAX)
        mu = (fmax - f0) / (rho * g0);
    if (alpha1 == FLT_MAX)
        alpha1 = mu / 100; // My own heuristic
    if (g0 >= 0) {
        if (opt->verbosity >= 2)
            cout << "Warning : df/dx(0) >= 0 !" << endl;
        return 0;
    }
    bool isBracketed = false;
    alpha2 = alpha1 + 1; // just to avoid a pathological initialization
  
    // Bracketing
    while (!isBracketed) {
        // cout << "Bracketing : alpha1 = " << alpha1 << endl << "             alpha0 = " << alpha0 << endl;
        if (alpha1 == mu && alpha1 == alpha2) { // NB: Personal hack... hopefully that should not happen
            if (opt->verbosity >= 2)
                cout << "Warning : alpha1 == alpha2 == mu during bracketing" << endl;
            return alpha1;
        }
        f_1 = (*f)(alpha1, opt);
        if (f_1 <= fmax) {
            if (opt->verbosity >= 2)
                cout << "fmax reached !" << endl;
            opt->early_stop = true; //added by dorionc
            return alpha1;
        }
        if (f_1 > f0 + alpha1 * rho * g0 || f_1 > f_0) {
            // NB: in Fletcher's book, there is a typo in the test above
            a1 = alpha0;
            b1 = alpha1;
            isBracketed = true;
            // cout << "Bracketing done : f_1 = " << f_1 << " , alpha1 = " << alpha1 << " , f0 = " << f0 << " , g0 = " << g0 << endl;
        } else {
            g_1 = (*g)(alpha1, opt);
            if (abs(g_1) < -sigma * g0) {
                // cout << "Low gradient : g=" << abs(g_1) << " < " << (-sigma * g0) << endl;
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
    bool repeated = false;
#ifdef __INTEL_COMPILER
#pragma warning(disable:279)  // Get rid of compiler warning.
#endif
    while (true) {
#ifdef __INTEL_COMPILER
#pragma warning(default:279)
#endif
        // cout << "Splitting : alpha1 = " << alpha1 << endl << "            a1 = " << a1 << endl << "            b1 = " << b1 << endl;
        // cout << "Interval : [" << a1 + tau2 * (b1-a1) << " , " << b1 - tau3 * (b1-a1) << "]" << endl;
        alpha1 = findMinWithCubicInterpol(
            a1, b1,
            a1 + tau2 * (b1-a1), b1 - tau3 * (b1-a1),
            f_0, f_1, g_0, g_1);
        g1 = (*g)(alpha1, opt);
        f1 = opt->cost->value[0]; // Shortcut.
        bool small= ((a1 - alpha1) * g_0 <= epsilon);
        if (small && (a1>0 || repeated)) {
            // cout << "Early stop : a1 = " << a1 << " , alpha1 = " << alpha1 << " , g(a1) = " << g_0 << " , epsilon = " << epsilon << endl;
            return a1;
        }
        if (small) repeated=true;
        //g1 = (*g)(alpha1, opt); // TODO See why this has been commented
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
real ConjGradientOptimizer::gSearch (void (*grad)(Optimizer*, const Vec&)) {

    real step = current_step_size;
    real sp, sm, pp, pm;

    // Backup the initial paremeters values
    params.copyTo(tmp_storage);

    params.update(step, search_direction);
    (*grad)(this, delta);
    real prod = dot(delta, search_direction);
    if (prod < 0) {
        // Step back to bracket the maximum
        do {
            sp = step;
            pp = prod;
            step /= 2.0;
            params.update(-step, search_direction);
            (*grad)(this, delta);
            prod = dot(delta, search_direction);
        } while (prod < -epsilon);
        sm = step;
        pm = prod;
    }
    else {
        // Step forward to bracket the maximum
        do {
            sm = step;
            pm = prod;
            params.update(step, search_direction);
            (*grad)(this, delta);
            prod = dot(delta, search_direction);
            step *= 2.0;
        } while (prod > epsilon);
        sp = step;
        pp = prod;
    }

    if (pm > 0 && pp < 0)
        step = (sm*pp - sp*pm) / (pp - pm);
    else
        step = (sm+sp) / 2;

    // Restore the original parameters value
    params.copyFrom(tmp_storage);
    return step;
}

/////////////////////
// hestenesStiefel //
/////////////////////
real ConjGradientOptimizer::hestenesStiefel (
    void (*grad)(Optimizer*, const Vec&),
    ConjGradientOptimizer* opt) {
    int i;
    // delta = opposite gradient
    //  (*grad)(opt, opt->delta); // TODO See why this line has been removed.
    for (i=0; i<opt->current_opp_gradient.length(); i++) {
        opt->tmp_storage[i] = opt->delta[i] - opt->current_opp_gradient[i];
    }
    real gamma = -dot(opt->delta, opt->tmp_storage) / dot(opt->search_direction, opt->tmp_storage);
    return gamma;
}

////////////////
// lineSearch //
////////////////
bool ConjGradientOptimizer::lineSearch() {
    real step;
    switch (line_search_algo) {
    case 1:
        step = fletcherSearch();
        break;
    case 2:
        step = gSearch(computeOppositeGradient);
        break;
    case 3:
        step = newtonSearch(max_steps, initial_step, low_enough);
        break;
    case 4:
        step = brentSearch();
        break;
    default:
        PLERROR("In ConjGradientOptimizer::lineSearch - Invalid conjugate gradient line search algorithm");
        step = 0;
        break;
    }
    if (step < 0)
        if (verbosity >= 1)
            cout << "Ouch, negative step !" << endl;
    if (step != 0) params.update(step, search_direction);
    if (step == 0)
        if (verbosity >= 2)
            cout << "No more progress made by the line search, stopping" << endl;
    return (step == 0);
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
                if ( (mini-maxi) * ( ( a * mini + b) * (mini + maxi) + a * maxi * maxi + c) > 0)
// is the same as: if (a*mini*mini*mini + b*mini*mini + c*mini >
//                      a*maxi*maxi*maxi + b*maxi*maxi + c*maxi )
                    return maxi;
                else
                    return mini;
            }
            if ((maxi-p2) * (( a * maxi + b) * (maxi + p2) + a * p2 * p2 + c) > 0)
// is the same as: if (a*maxi*maxi*maxi + b*maxi*maxi + c*maxi >
//                      a*p2*p2*p2 + b*p2*p2 + c*p2)
                return p2;
            else
                return mini;
        } else {
            if (p2 > maxi || maxi == FLT_MAX)
                return maxi;
            if (p2 < mini) { // the minimum is before the range
                if ((mini-maxi) * (( a * mini + b) * (mini + maxi) + a * maxi * maxi + c) > 0)
// is the same as: if (a*mini*mini*mini + b*mini*mini + c*mini >
// 		  	a*maxi*maxi*maxi + b*maxi*maxi + c*maxi )
                    return maxi;
                else
                    return mini;
            }
            if ((maxi-p2) * (( a * maxi + b) * (maxi + p2) + a * p2 * p2 + c) > 0)
// is the same as: if (a*maxi*maxi*maxi + b*maxi*maxi + c*maxi >
//          		a*p2*p2*p2 + b*p2*p2 + c*p2)
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
  
//////////////////
// newtonSearch //
//////////////////
real ConjGradientOptimizer::newtonSearch(
    int max_steps,
    real initial_step,
    real low_enough
    ) {
    Vec c(max_steps); // the cost
    Vec g(max_steps); // the gradient
    Vec x(max_steps); // the step
    computeCostAndDerivative(0, this, c[0], g[0]);
    x[0] = 0;
    real step = initial_step;
    real x_2 = x[0]*x[0];
    real x_3 = x[0] * x_2;
    real x_4 = x_2 * x_2;
    real sum_x = x[0];
    real sum_x_2 = x_2;
    real sum_x_3 = x_3;
    real sum_x_4 = x_4;
    real sum_c_x_2 = c[0] * x_2;
    real sum_g_x = g[0] * x[0];
    real sum_c_x = c[0] * x[0];
    real sum_c = c[0];
    real sum_g = g[0];
    for (int i=1; i<max_steps; i++) {
        computeCostAndDerivative(step, this, c[i], g[i]);
        x[i] = step;
        if (abs(g[i]) < low_enough) {
            // we have reached the minimum
            return step;
        }
        x_2 = x[i]*x[i];
        x_3 = x[i] * x_2;
        x_4 = x_2 * x_2;
        sum_x += x[i];
        sum_x_2 += x_2;
        sum_x_3 += x_3;
        sum_x_4 += x_4;
        sum_c_x_2 += c[i] * x_2;
        sum_g_x += g[i] * x[i];
        sum_c_x += c[i] * x[i];
        sum_c += c[i];
        sum_g += g[i];
        step = findMinWithQuadInterpol(i+1, sum_x, sum_x_2, sum_x_3, sum_x_4, sum_c_x_2, sum_g_x, sum_c_x, sum_c, sum_g);
    }
    if (verbosity >= 1)
        cout << "Warning : minimum not reached, is the cost really quadratic ?" << endl;
    return step;
}



//////////////
// optimize //
//////////////
real ConjGradientOptimizer::optimize()
{
    PLWARNING("In ConjGradientOptimizer::optimize This method is deprecated, use optimizeN instead");
    ofstream out;
    if (!filename.empty()) {
        out.open(filename.c_str());
    }
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
        //cout << "cost = " << cost->value[0] << " " << cost->value[1] << " " << cost->value[2] << endl;
    
        // Make a line search along the current search direction
        early_stop = lineSearch(); 
        if (early_stop && verbosity >= 2)
            cout << "Early stopping triggered by the line search" << endl;
        cost->fprop();
        current_cost = cost->value[0];
    
        // Find the new search direction
        bool early_stop_dir = findDirection();
        if (early_stop_dir && verbosity >= 2)
            cout << "Early stopping triggered by direction search" << endl;
        early_stop = early_stop || early_stop_dir;

        last_improvement = last_cost - current_cost;
        last_cost = current_cost;

        // This value of current_step_size is suggested by Fletcher
        df = max (last_improvement, 10*stop_epsilon);
        current_step_size = min(1.0, 2.0 * df / dot(search_direction, current_opp_gradient));
    
        // Display results TODO ugly copy/paste from GradientOptimizer: to be cleaned ?
        if (compute_cost) {
            meancost += cost->value;
        }
        if ((every!=0) && ((t+1)%every==0)) 
            // normally this is done every epoch
        { 
            //cerr << ">>>>>> nupdates= " << nupdates << "  every=" << every << "  sumofvar->nsamples=" << sumofvar->nsamples << endl;
            if (compute_cost) {
                meancost /= real(every);
            }
            //if (decrease_constant != 0)
            //  cout << "at t=" << t << ", learning rate = " << learning_rate << endl;
            if (compute_cost) {
                printStep(cerr, t+1, meancost[0]);
            }
            if (compute_cost && out) {
                printStep(out, t+1, meancost[0]);
            }
            bool early_stop_mesure = false;
            if (compute_cost) {
                early_stop_mesure = measure(t+1,meancost); 
            }
            if (early_stop_dir && verbosity >= 2)
                cout << "Early stopping triggered by the measurer" << endl;
            early_stop = early_stop || early_stop_mesure;
            // early_stop = measure(t+1,meancost); // TODO find which is the best between this and the one above
            // early_stop_i = (t+1)/every; TODO Remove, must be useless now
            if (compute_cost) {
                lastmeancost << meancost;
                meancost.clear();
            }
        }
    }
    return lastmeancost[0];
}


///////////////
// optimizeN //
///////////////
bool ConjGradientOptimizer::optimizeN(VecStatsCollector& stats_coll) {
    real df, current_cost;
    meancost.clear();
    int stage_max = stage + nstages; // the stage to reach

#ifdef BOUNDCHECK
    if (current_step_size <= 0 && line_search_algo <= 2) {
        PLERROR("In ConjGradientOptimizer::optimizeN - current_step_size <= 0, have you called reset() ?");
    }
#endif

    if (stage==0)
    {
        computeOppositeGradient(this, current_opp_gradient);
        search_direction <<  current_opp_gradient;  // first direction = -grad;
        last_cost = cost->value[0];
    }

    if (early_stop)
        stats_coll.update(cost->value);    

    for (; !early_stop && stage<stage_max; stage++) {

        // Make a line search along the current search direction
        early_stop = lineSearch();
        computeOppositeGradient(this, delta);
        current_cost = cost->value[0];
        if (compute_cost) {
            meancost += cost->value;
        }
        stats_coll.update(cost->value);
    
        // Find the new search direction
        early_stop = early_stop || findDirection();

        last_improvement = last_cost - current_cost;
        last_cost = current_cost;

        // This value of current_step_size is suggested by Fletcher
        df = max (last_improvement, 10*stop_epsilon);
        current_step_size = min(1.0, 2.0*df / dot(search_direction, current_opp_gradient));
    
    }

    if (compute_cost) {
        meancost /= real(nstages);
        printStep(cerr, stage, meancost[0]);
        early_stop = early_stop || measure(stage+1,meancost);
    }

    // TODO Call the Stats collector
    if (early_stop && verbosity >= 2)
        cout << "Early Stopping !" << endl;
    return early_stop;
}

//////////////////
// polakRibiere //
//////////////////
real ConjGradientOptimizer::polakRibiere (
    void (*grad)(Optimizer*, const Vec&),
    ConjGradientOptimizer* opt) {
    int i;
    // delta = Gradient
    real normg = pownorm(opt->current_opp_gradient);
    for (i=0; i<opt->current_opp_gradient.length(); i++) {
        opt->tmp_storage[i] = opt->delta[i] - opt->current_opp_gradient[i];
    }
    real gamma = dot(opt->tmp_storage,opt->delta) / normg;
    return gamma;
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

///////////
// reset //
///////////
void ConjGradientOptimizer::reset() {
    inherited::reset();
    early_stop = false;
    last_improvement = 0.1; // May only influence the speed of first iteration.
    current_step_size = starting_step_size;
}

///////////////////////////
// updateSearchDirection //
///////////////////////////
void ConjGradientOptimizer::updateSearchDirection(real gamma) {
    if (gamma==0)
        search_direction << delta;
    else
        for (int i=0; i<search_direction.length(); i++)
            search_direction[i] = delta[i] + gamma * search_direction[i];
    current_opp_gradient << delta;
}

//////////////
// minBrack //
//////////////
// Copyright (c) 1996-2001, Ian T. Nabney
void ConjGradientOptimizer::minBrack(real& br_min, real& br_max, real& br_mid)
{
    // Initialisation of the working parameters
    real a = br_min;
    real b = br_max;
    real c = br_mid;

    real fa = cost->value[0];
    real fb = computeCostValue( b, this );
    real fc;
    real fu = 0; // Initialization to make the compiler happy.

    // Value of golden section (1 + sqrt(5))/2.0
    real phi = 1.6180339887499;

    // A small non-zero number to avoid dividing by zero in quadratic
    // interpolation
    real TINY = 1.e-10;

    // Maximal proportional step to take: don't want to make this too big
    // as then spend a lot of time finding the minimum inside the bracket
    real max_step = 10.0;

    // Assume that we know going from a to b is downhill initially.
    // (usually because gradf(a) < 0).
    if( fb > fa )
    {
        // Minimum must lie between a and b: do golden section until we find point
        // low enough to be middle of bracket
        do
        {
            c = b;
            b = a + (c-a)/phi;
            fb = computeCostValue( b, this );
        }
        while( fb > fa );
    }
    else
    {
        // There is a valid bracket upper bound greater than b
        c = b + phi*(b-a);
        fc = computeCostValue( c, this );
        bool bracket_found = false;

        while( fb > fc )
        {
            // Do a quadratic interpolation (i.e. to minimum of quadratic)
            real r = (b-a)*(fb-fc);
            real q = (b-c)*(fb-fa);
            real q_r = q - r;
            if( fabs( q_r ) < TINY )
                q_r = sign( q_r ) * TINY;
            real u = b - ((b-c)*q - (b-a)*r)/(2.*q_r);
            real ulimit = b + max_step*(c-b);

            if( (b-u)*(u-c) > 0. )
            {
                // Interpolant lies between b and c
                fu = computeCostValue( u, this );
                if( fu < fc )
                {
                    // Have a minimum between b and c
                    br_min = b;
                    br_mid = u;
                    br_max = c;
                    return;
                }
                else if( fu > fb )
                {
                    // Have a minimum between a and u
                    br_min = a;
                    br_mid = c;
                    br_max = u;
                    return;
                }
                // Quadratic interpolation didn't give a bracket, so take a golden step
                u = c + phi*(c-b);
            }
            else if( (c-u)*(u-ulimit) > 0. )
            {
                // Interpolant lies between c and limit
                fu = computeCostValue( u, this );
                if( fu < fc )
                {
                    // Move bracket along, and then take a golden section step
                    b = c;
                    c = u;
                    u = c + phi*(c-b);
                }
                else
                {
                    bracket_found = true;
                }
            }
            else if( (u-ulimit)*(ulimit-c) >= 0. )
            {
                // Limit parabolic u to maximum value
                u = ulimit;
            }
            else
            {
                // Reject parabolic u and use golden section step
                u = c + phi*(c-b);
            }

            if( !bracket_found )
            {
                fu = computeCostValue( u, this );
            }

            a = b;
            b = c;
            c = u;
            fa = fb;
            fb = fc;
            fc = fu;
        }
    }

    br_mid = b;
    if( a < c )
    {
        br_min = a;
        br_max = c;
    }
    else
    {
        br_min = c;
        br_max = a;
    }
}


/////////////////
// brentSearch //
/////////////////
// Copyright (c) 1996-2001, Ian T. Nabney
real ConjGradientOptimizer::brentSearch()
{
    // Value of golden section (1 + sqrt(5))/2.0
    real phi = 1.6180339887499;
    real cphi = 2 - phi; // 1 - 1/phi
    real TOL = 2^(-26);  // Maximal fractional precision
    real TINY = 1.0e-10; // Can't use fractional precision when minimum is at 0

    // Bracket the minimum
    real br_min = 0.;
    real br_max = 1.;
    real br_mid;
    minBrack( br_min, br_max, br_mid );

    // Use Brent's algorithm to find minimum
    // Initialize the points and function values
    real w = br_mid;    // Where second from minimum is
    real v = br_mid;    // Previous value of w
    real x = v;         // Where current minimum is
    real e = 0.;        // Distance moved on step before last
    real d = MISSING_VALUE; // Initialization to make the compiler happy.
    real fx = computeCostValue( x, this );
    real fv = fx;
    real fw = fx;

    for( int i=0 ; i<n_iterations ; i++ )
    {
        real xm = 0.5*(br_min+br_max); // Middle of bracket
        // Make sure that tolerance is big enough
        real tol1 = TOL * fabs(x) + TINY;
        // Decide termination on absolute precision
        if( fabs(x - xm) <= value_res && (br_max-br_min) < 4*value_res )
        {
            return x;
        }

        // Check if step before last was big enough to try a parabolic step.
        // Note that this will fail on first iteration, which must be a
        // golden section step.
        if( fabs(e) > tol1 )
        {
            // Construct a trial parabolic fit through x, v and w
            real r = (fx - fv) * (x - w);
            real q = (fx - fw) * (x - v);
            real p = (x - v)*q - (x - w)*r;
            q = 2 * (q - r);
            if( q > 0 )
            {
                p = -p;
                q = -q;
            }

            // Test if the parabolic fit is OK
            if( fabs(p) >= fabs(0.5*q*e) || q <= q*(br_min-x) || p >= q*(br_max-x) )
            {
                // No it isn't, so take a golden section step
                if( x >= xm )
                    e = br_min - x;
                else
                    e = br_max - x;

                d = cphi*e;
            }
            else
            {
                // Yes it is, so take the parabolic step
                assert( !is_missing(d) );
                e = d;
                d = p/q;
                real u = x + d;
                if( u-br_min < 2*tol1 || br_max-u < 2*tol1 )
                    d = sign(xm - x)*tol1;
            }
        }
        else
        {
            // Step before last not big enough, so take a golden section step
            if( x >= xm )
                e = br_min - x;
            else
                e = br_max - x;

            d = cphi * e;
        }

        // Make sure that step is big enough
        real u;
        if( abs(d) >= tol1 )
            u = x + d;
        else
            u = x + sign(d)*tol1;

        // Evaluate function at u
        real fu = computeCostValue( u, this );

        // Reorganise bracket
        if( fu <= fx )
        {
            if( u >= x )
                br_min = x;
            else
                br_max = x;

            v = w;
            w = x;
            x = u;
            fv = fw;
            fw = fv;
            fx = fu;
        }
        else
        {
            if( u < x )
                br_min = u;
            else
                br_max = u;

            if( fu <= fw || w == x )
            {
                v = w;
                w = u;
                fv = fw;
                fw = fu;
            }
            else if( fu <= fv || v == x || v == w )
            {
                v = u;
                fv = fu;
            }
        }
    }
    return x;
}

} // end of namespace PLearn


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
