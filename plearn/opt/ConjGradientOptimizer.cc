// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003,2006 Olivier Delalleau

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

///////////////////////////
// ConjGradientOptimizer //
///////////////////////////
ConjGradientOptimizer::ConjGradientOptimizer():
    constrain_limit(0.1),
    expected_red(1),
    max_eval_per_line_search(20),
    max_extrapolate(3),
    rho(1e-2),
    sigma(0.5),
    slope_ratio(100),
    verbosity(0)
{}

PLEARN_IMPLEMENT_OBJECT(ConjGradientOptimizer,
    "Optimizer based on the conjugate gradient method.",
    "The conjugate gradient algorithm is basically the following :\n"
    "- 0: initialize the search direction d = -gradient\n"
    "- 1: perform a line search along direction d for the minimum of the\n"
    "     function value\n"
    "- 2: move to this minimum, update the search direction d and go to\n"
    "     step 1\n"
    "The algorithm is inspired by Carl Rasmussen's Matlab algorithm from:\n"
    "http://www.kyb.tuebingen.mpg.de/bs/people/carl/code/minimize/minimize.m\n"
    "\n"
);

////////////////////
// declareOptions //
////////////////////
void ConjGradientOptimizer::declareOptions(OptionList& ol)
{
    declareOption(ol, "sigma", &ConjGradientOptimizer::sigma,
                                  OptionBase::buildoption, 
        "Constant in the Wolfe-Powell stopping conditions.");

     declareOption(ol, "rho", &ConjGradientOptimizer::rho,
                                  OptionBase::buildoption, 
        "Constant in the Wolfe-Powell stopping conditions.");

     declareOption(ol, "constrain_limit", &ConjGradientOptimizer::constrain_limit,
                                  OptionBase::buildoption, 
        "Multiplicative coefficient to constrain the evaluation bracket.");

     declareOption(ol, "max_extrapolate", &ConjGradientOptimizer::max_extrapolate,
                                  OptionBase::buildoption, 
        "Maximum coefficient for bracket extrapolation.");

     declareOption(ol, "max_eval_per_line_search", &ConjGradientOptimizer::max_eval_per_line_search,
                                  OptionBase::buildoption, 
        "Maximum number of function evalutions during line search.");

     declareOption(ol, "slope_ratio", &ConjGradientOptimizer::slope_ratio,
                                  OptionBase::buildoption, 
        "Maximum slope ratio.");

     declareOption(ol, "expected_red", &ConjGradientOptimizer::expected_red,
                                  OptionBase::buildoption, 
        "Expected function reduction at first step.");

    declareOption(ol, "verbosity", &ConjGradientOptimizer::verbosity,
                                   OptionBase::buildoption, 
        "Controls the amount of output.");

    inherited::declareOptions(ol);
}


////////////
// build_ //
////////////
void ConjGradientOptimizer::build_() {
    // Make sure the internal data have the right size.
    int n = params.nelems();
    current_opp_gradient.resize(n);
    search_direction.resize(n);
    tmp_storage.resize(n);
    delta.resize(n);
}

//////////////////////////////
// computeCostAndDerivative //
//////////////////////////////
void ConjGradientOptimizer::computeCostAndDerivative(
    real alpha, real& cost, real& derivative) {
    if (fast_exact_is_equal(alpha, 0)) {
        cost = this->last_cost;
        derivative = -dot(this->search_direction, this->current_opp_gradient);
    } else {
        this->params.copyTo(this->tmp_storage);
        this->params.update(alpha, this->search_direction);
        computeGradient(this->delta);
        cost = this->cost->value[0];
        derivative = dot(this->search_direction, this->delta);
        this->params.copyFrom(this->tmp_storage);
    }
}

//////////////////////
// computeCostValue //
//////////////////////
real ConjGradientOptimizer::computeCostValue(real alpha)
{
    if (fast_exact_is_equal(alpha, 0))
        return this->last_cost;
    this->params.copyTo(this->tmp_storage);
    this->params.update(alpha, this->search_direction);
    this->proppath.fprop();
    real c = this->cost->value[0];
    this->params.copyFrom(this->tmp_storage);
    return c;
}

///////////////////////
// computeDerivative //
///////////////////////
real ConjGradientOptimizer::computeDerivative(real alpha)
{
    if (fast_exact_is_equal(alpha, 0))
        return -dot(this->search_direction, this->current_opp_gradient);
    this->params.copyTo(this->tmp_storage);
    this->params.update(alpha, this->search_direction);
    computeGradient(this->delta);
    this->params.copyFrom(this->tmp_storage);
    return dot(this->search_direction, this->delta);
}

///////////////////
// findDirection //
///////////////////
bool ConjGradientOptimizer::findDirection() {
    real gamma = polakRibiere();
    // It is suggested to keep gamma >= 0
    if (gamma < 0) {
        if (verbosity >= 2)
            pout << "gamma < 0 ! gamma = " << gamma << " ==> Restarting" << endl;

        // TODO Is this really needed / a good idea?
        // TODO PUT THAT AS AN OPTION!!
        gamma = 0;
    }
    /*
    else {
        real dp = dot(delta, current_opp_gradient);
        real delta_n = pownorm(delta);
        if (abs(dp) > restart_coeff *delta_n ) {
            if (verbosity >= 5)
                cout << "Restart triggered !" << endl;
            gamma = 0;
        }
    }
    */
    updateSearchDirection(gamma);
    /*
    // If the gradient is very small, we can stop !
//  isFinished = pownorm(current_opp_gradient) < 0.0000001;
    // TODO This may lead to an erroneous early stop. To investigate ?
    isFinished = false;
    if (isFinished && verbosity >= 2)
        cout << "Gradient is small enough, time to stop" << endl;
    return isFinished;
        */
    // TODO Is it necesary to return a boolean?
    return false;
}

/////////////////////
// rasmussenSearch //
/////////////////////
real ConjGradientOptimizer::rasmussenSearch()
{
    bool try_again = true;
    while (try_again) {
        try_again = false;
    // X0 = X; f0 = f1; df0 = df1; % make a copy of current values
    real fun_val0 = fun_val1;
    // real ras_df0_ = ras_df1_; Should not be needed TODO see
    // X = X + z1*s;  % begin line search
    // We don't do that explicitely
    // [f2 df2] = eval(argstr);
    // d2 = df2'*s;
    computeCostAndDerivative(step1, fun_val2, fun_deriv2);
    // i = i + (length<0);
    // We count epochs outside of this.
    // f3 = f1; d3 = d1; z3 = -z1; % initialize point 3 equal to point 1
    real fun_val3 = fun_val1;
    real fun_deriv3 = fun_deriv1;
    real step3 = - step1;
    fun_eval_count = max_eval_per_line_search;
    // success = 0; limit = -1;                     % initialize quanteties
    line_search_succeeded = false;
    bracket_limit = -1;
    // while 1
    while (true) {
        // while ((f2 > f1+z1*RHO*d1) | (d2 > -SIG*d1)) & (M > 0)
        while ( (fun_val2 > fun_val1 + step1 * rho * fun_deriv1 ||
                 fun_deriv2 > - sigma * fun_deriv1 ) &&
                fun_eval_count > 0 )
        {
            // limit = z1; % tighten the bracket
            bracket_limit = step1;
            // if f2 > f1
            // z2 = z3 - (0.5*d3*z3*z3)/(d3*z3+f2-f3);  % quadratic fit
            if (fun_val2 > fun_val1) {
                step2 = step3 -
                    (0.5*fun_deriv3*step3*step3) / 
                    (fun_deriv3*step3+fun_val2-fun_val3);
            } else {
                // A = 6*(f2-f3)/z3+3*(d2+d3); % cubic fit
                cubic_a = 6*(fun_val2-fun_val3)/step3+3*(fun_deriv2+fun_deriv3);
                // B = 3*(f3-f2)-z3*(d3+2*d2);
                cubic_b = 3*(fun_val3-fun_val2)-step3*(fun_deriv3+2*fun_deriv2);
                // z2 = (sqrt(B*B-A*d2*z3*z3)-B)/A;
                // % numerical error possible - ok!
                step2 = (sqrt(cubic_b*cubic_b-cubic_a*fun_deriv2*step3*step3)-cubic_b)/cubic_a;
            }
            if (isnan(step2) || isinf(step2))
                // z2 = z3/2;                  % if we had a numerical problem
                // then bisect
                step2 = step3/2;
            // z2 = max(min(z2, INT*z3),(1-INT)*z3);
            // % don't accept too close to limits
            step2 = max(min(step2, constrain_limit*step3),(1-constrain_limit)*step3);
            // z1 = z1 + z2; % update the step
            step1 = step1 + step2;
            //  X = X + z2*s;
            // [f2 df2] = eval(argstr);
            // d2 = df2'*s;
            computeCostAndDerivative(step1, fun_val2, fun_deriv2);
            // M = M - 1; i = i + (length<0); % count epochs?!
            fun_eval_count = fun_eval_count - 1;
            // z3 = z3-z2; % z3 is now relative to the location of z2
            step3 = step3 - step2;  
        }
        // if f2 > f1+z1*RHO*d1 | d2 > -SIG*d1
        //  break;  % this is a failure
        // elseif d2 > SIG*d1
        //  success = 1; break; % success
        // elseif M == 0
        //  break; % failure
        if (fun_val2 > fun_val1+step1*rho*fun_deriv1 ||
            fun_deriv2 > -sigma*fun_deriv1)
            break;
        else if (fun_deriv2 > sigma * fun_deriv1) {
            line_search_succeeded = true;
            break;
        } else if (fun_eval_count == 0)
            break;
        // A = 6*(f2-f3)/z3+3*(d2+d3); % make cubic extrapolation
        // B = 3*(f3-f2)-z3*(d3+2*d2);
        cubic_a = 6*(fun_val2-fun_val3)/step3+3*(fun_deriv2+fun_deriv3);
        cubic_b = 3*(fun_val3-fun_val2)-step3*(fun_deriv3+2*fun_deriv2);
        // z2 = -d2*z3*z3/(B+sqrt(B*B-A*d2*z3*z3));
        // % num. error possible - ok!
        step2 = -fun_deriv2*step3*step3/
            (cubic_b+sqrt(cubic_b*cubic_b-cubic_a*fun_deriv2*step3*step3));
        // if ~isreal(z2) | isnan(z2) | isinf(z2) | z2 < 0
        // % num prob or wrong sign?
        if (isnan(step2) || isinf(step2)) {
            // if limit < -0.5 % if we have no upper limit
            if (bracket_limit < -0.5)
                // z2 = z1 * (EXT-1); % the extrapolate the maximum amount
                step2 = step1 * (max_extrapolate - 1);
            else
                // z2 = (limit-z1)/2; % otherwise bisect
                step2 = (bracket_limit - step1) / 2;
        // elseif (limit > -0.5) & (z2+z1 > limit) 
        //  % extraplation beyond max?
        } else if (bracket_limit > -0.5 && (step2 + step1 > bracket_limit)) {
            step2 = (bracket_limit - step1) / 2; // bisect
        // elseif (limit < -0.5) & (z2+z1 > z1*EXT)
        // % extrapolation beyond limit
        } else if (bracket_limit < -0.5 && step2+step1 > step1 * max_extrapolate) {
            // z2 = z1*(EXT-1.0); % set to extrapolation limit
            step2 = step1 * (max_extrapolate - 1);
        // elseif z2 < -z3*INT
        } else if (step2 < - step3 * constrain_limit) {
            // z2 = -z3*INT;
            step2 = - step3 * constrain_limit;
        // elseif (limit > -0.5) & (z2 < (limit-z1)*(1.0-INT))
        // % too close to limit?
        } else if (bracket_limit > -0.5 &&
                   step2 < (bracket_limit - step1) * (1 - constrain_limit) ) {
            // z2 = (limit-z1)*(1.0-INT);
            step2 = (bracket_limit - step1) * (1 - constrain_limit);
        }
        // f3 = f2; d3 = d2; z3 = -z2;
        // % set point 3 equal to point 2
        fun_val3 = fun_val2;
        fun_deriv3 = fun_deriv2;
        step3 = - step2;
        // z1 = z1 + z2; X = X + z2*s;
        // % update current estimates
        step1 += step2;
        // [f2 df2] = eval(argstr);
        // d2 = df2'*s;
        computeCostAndDerivative(step1, fun_val2, fun_deriv2);
        // M = M - 1; i = i + (length<0);
        // % count epochs?!
        fun_eval_count--;
    }
    // if success % if line search succeeded
    if (line_search_succeeded) {
        fun_val1 = fun_val2;

        // ls_failed = 0; % this line search did not fail
        line_search_failed = false;
    } else {
        // X = X0; f1 = f0; df1 = df0;
        // % restore point from before failed line search
        fun_val1 = fun_val0;
        // if ls_failed | i > abs(length)
        // % line search failed twice in a row or we ran out of time, so we
        // give up
        if (line_search_failed)
            return 0;
        // tmp = df1; df1 = df2; df2 = tmp; % swap derivatives
        // We do not do that... it looks weird!
        // s = -df1; % try steepest
        // We will actually do s = -df0 as this seems more logical.
        // TODO Ask Rasmussen!
        // TODO So what do we do here?
        // d1 = -s'*s;
        fun_deriv1 = - pownorm(current_opp_gradient);
        // z1 = 1/(1-d1);
        step1 = 1 / (1 - fun_deriv1); // TODO What about expected_red or similar?
        // ls_failed = 1; % this line search failed
        line_search_failed = true;
        try_again = true;
    }
    }
    return step1;
}

////////////////
// lineSearch //
////////////////
bool ConjGradientOptimizer::lineSearch() {
    real step = rasmussenSearch();
    if (step < 0)
        PLWARNING("Negative step!");
    if (!fast_exact_is_equal(step, 0))
        params.update(step, search_direction);
    else
        if (verbosity >= 2)
            pout << "No more progress made by the line search, stopping" << endl;
    return fast_exact_is_equal(step, 0);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ConjGradientOptimizer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    PLERROR("In ConjGradientOptimizer::makeDeepCopyFromShallowCopy - Not "
            "implementented");
}

///////////////
// optimizeN //
///////////////
bool ConjGradientOptimizer::optimizeN(VecStatsCollector& stats_coll) {
    /*
    real df, current_cost;
    meancost.clear();
    */
    int stage_max = stage + nstages; // the stage to reach

    /*
#ifdef BOUNDCHECK
    if (current_step_size <= 0 && line_search_algo <= 2) {
        PLERROR("In ConjGradientOptimizer::optimizeN - current_step_size <= 0, have you called reset() ?");
    }
#endif
*/

    if (stage==0)
    {
        computeOppositeGradient(current_opp_gradient);
        search_direction <<  current_opp_gradient;  // first direction = -grad;
        last_cost = cost->value[0];
        expected_red = 1; // TODO Find the best value here!
        //if (line_search_algo == 5) {
            fun_val1 = last_cost;
            fun_deriv1 = - pownorm(search_direction);
            step1 = expected_red / ( 1 - fun_deriv1 );
        //}
        rho = 0.01;
        sigma = 0.5;
        constrain_limit = 0.1;
        max_extrapolate = 3.0;
        max_eval_per_line_search = 20;
        slope_ratio = 100;
    }

    /*
    if (early_stop)
        stats_coll.update(cost->value);    
        */

    for (; !early_stop && stage<stage_max; stage++) {

        // Make a line search along the current search direction.
        early_stop = lineSearch();
        computeOppositeGradient(delta); // TODO Why this?
        // current_cost = cost->value[0];
        if (verbosity >= 2)
            pout << "ConjGradientOptimizer - stage " << stage << ": "
                 << cost->value[0] << endl;
        stats_coll.update(cost->value);
    
        // Find the new search direction.
        early_stop = early_stop || findDirection();

   
    }


    // TODO Call the Stats collector
    if (early_stop && verbosity >= 2)
        pout << "Early Stopping !" << endl;

    return early_stop;
}

//////////////////
// polakRibiere //
//////////////////
real ConjGradientOptimizer::polakRibiere()
{
    real normg = pownorm(this->current_opp_gradient);
    // At this point, delta = gradient at new point.
    this->tmp_storage << this->delta;
    this->tmp_storage -= this->current_opp_gradient;
    return dot(this->tmp_storage, this->delta) / normg;
}

///////////
// reset //
///////////
void ConjGradientOptimizer::reset() {
    inherited::reset();
    early_stop = false;
}

///////////////////////////
// updateSearchDirection //
///////////////////////////
void ConjGradientOptimizer::updateSearchDirection(real gamma) {
    if (fast_exact_is_equal(gamma, 0))
        search_direction << delta;
    else
        for (int i=0; i<search_direction.length(); i++)
            search_direction[i] = delta[i] + gamma * search_direction[i];
    // Update 'current_opp_gradient' for the new current point.
    current_opp_gradient << delta;
    // if (line_search_algo == 5) {
        // d2 = df1'*s;
        // if d2 > 0               % new slope must be negative
        //   s = -df1;             % otherwise use steepest direction
        //   d2 = -s'*s;    
        fun_deriv2 = - dot(current_opp_gradient, search_direction);
        if (fun_deriv2 > 0) {
            search_direction << current_opp_gradient;
            fun_deriv2 = - pownorm(search_direction);
        }
        // z1 = z1 * min(RATIO, d1/(d2-realmin));
        // % slope ratio but max RATIO
        step1 = step1 * min(slope_ratio, fun_deriv1/(fun_deriv2-REAL_EPSILON));
        // d1 = d2;
        fun_deriv1 = fun_deriv2;
    //}
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
