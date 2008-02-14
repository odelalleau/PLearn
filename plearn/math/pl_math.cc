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
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearn/plearn/math/pl_math.cc */

#include "pl_math.h"

namespace PLearn {
using namespace std;


# ifdef BIGENDIAN
_plearn_nan_type plearn_nan = { {0x7f, 0xc0, 0, 0} };
# endif
# ifdef LITTLEENDIAN
_plearn_nan_type plearn_nan = { {0, 0, 0xc0, 0x7f} };
# endif

float tanhtable[TANHTABLESIZE];

PLMathInitializer::PLMathInitializer()
{
    //! Fill the tanh table
    real scaling = MAXTANHX/(TANHTABLESIZE-1);
    for(int i=0; i<TANHTABLESIZE; i++)
        tanhtable[i] = (float) tanh(i*scaling);
}
  
PLMathInitializer::~PLMathInitializer() 
{}
  
PLMathInitializer pl_math_initializer;

//////////////
// is_equal //
//////////////
bool is_equal(real a, real b, real absolute_tolerance_threshold, 
              real absolute_tolerance,
              real relative_tolerance)
{
    if (isnan(a))
        if (isnan(b))
            return true;
        else
            return false;
    if (isnan(b))
        return false;
    if (int inf_a = isinf(a))
        return inf_a == isinf(b);
    if (isinf(b))
        return false;
    return fast_is_equal(a, b, absolute_tolerance_threshold, absolute_tolerance, relative_tolerance);
}

real safeflog(real a)
{
    if (a < 0.0)
        PLERROR("safeflog: negative argument (%f)", a);
    if (a < 1e-25)
        return -57.5;
    else return (real)pl_log((double)a);
}

real safeexp(real a)
{
#ifdef USEDOUBLE
    if (a < -300) return 0;
    if (a > 300) return 1e38;
#else
    if (a < -87) return 0;
    if (a > 43) return 5e18;
#endif
    return exp(a);
}

real log(real base, real a)
{
    return pl_log(a) / pl_log(base);
}

real logtwo(real a)
{
    return pl_log(a) / LOG_2;
}

real safeflog(real base, real a)
{
    return safeflog(a) / safeflog(base);
}

real safeflog2(real a)
{
    return safeflog(a) / LOG_2;
}

real tabulated_softplus_primitive(real x) {
    static const int n_softplus_primitive_values = 10000;
    static const real min_softplus_primitive_arg = -20;
    static const real max_softplus_primitive_arg = 10;
    static const real max_offset = max_softplus_primitive_arg*max_softplus_primitive_arg*0.5;
    static const real softplus_primitive_delta = (n_softplus_primitive_values-1)/(max_softplus_primitive_arg-min_softplus_primitive_arg);
    static real softplus_primitive_values[n_softplus_primitive_values];
    static bool computed_softplus_primitive_table = false;
    if (!computed_softplus_primitive_table)
    {
        real y=min_softplus_primitive_arg;
        real dy=1.0/softplus_primitive_delta;
        for (int i=0;i<n_softplus_primitive_values;i++,y+=dy)
            softplus_primitive_values[i] = softplus_primitive(y);
        computed_softplus_primitive_table=true;
    }
    if (x<min_softplus_primitive_arg) return 0;
    if (x>max_softplus_primitive_arg) return softplus_primitive_values[n_softplus_primitive_values-1]+x*x*0.5 - max_offset;
    int bin = int(rint((x-min_softplus_primitive_arg)*softplus_primitive_delta));
    return softplus_primitive_values[bin];
}

// compute log(exp(log_a)+exp(log_b)) without losing too much precision
real logadd(double log_a, double log_b)
{
    if (log_a < log_b)
    { // swap them
        double tmp = log_a;
        log_a = log_b;
        log_b = tmp;
    } else if (fast_exact_is_equal(log_a, log_b)) {
        // Special case when log_a == log_b. In particular this works when both
        // log_a and log_b are (+-) INFINITY: it will return (+-) INFINITY
        // instead of NaN.
        return LOG_2 + log_a;
    }
    double negative_absolute_difference = log_b - log_a;
    if (negative_absolute_difference < MINUS_LOG_THRESHOLD)
        return real(log_a);
    return (real)(log_a + log1p(exp(negative_absolute_difference)));
}

real square_f(real x)
{ return x*x; }

// compute log(exp(log_a)-exp(log_b)) without losing too much precision
real logsub(real log_a, real log_b)
{
    if (log_a < log_b)
        PLERROR("log_sub: log_a (%f) should be greater than log_b (%f)", log_a, log_b);
 
    real negative_absolute_difference = log_b - log_a;
 
    // We specify an absolute 1e-5 threshold to have the same behavior as with
    // the old FEQUAL macro.
    if (fast_is_equal(log_a, log_b, REAL_MAX, 1e-5))
        return -REAL_MAX;
    else if (negative_absolute_difference < MINUS_LOG_THRESHOLD)
        return log_a;
    else
        return log_a + log1p(-exp(negative_absolute_difference));
}

real small_dilogarithm(real x)
{
    // TODO Deal with x == 0.
    real somme = x;
    real prod = x;
    int i=2;
    for (;i<=999;i++)
    {
        real coef = (i-1.0)/i;
        prod *= x*coef*coef;
        somme += prod;
        if (fabs(prod/somme)<1e-16) break; // tolerance
    }
    static bool warning_was_raised=false;
    if (i==1000 && !warning_was_raised) 
    {
        warning_was_raised=true;
        PLWARNING("dilogarithm (%f): insufficient precision", x);
    }
    return somme;
}

real positive_dilogarithm(real x)
{
    if (x<0.5)
        return small_dilogarithm(x);
    else if (x<1.0)
        return Pi*Pi/6.0 - small_dilogarithm(1.0-x) - pl_log(x)*pl_log(1-x);
    else if (fast_exact_is_equal(x, 1.0))
        return Pi*Pi/6.0;
    else if (x<=1.01)
    {
        real delta=x-1.0;
        real log_delta=pl_log(delta);
        return Pi*Pi/6.0 + delta*(1-log_delta+delta*
                                  ((2*log_delta-1)/4 + delta*
                                   ((1-3*log_delta)/9 + delta*
                                    ((4*log_delta-1)/16 + delta*
                                     ((1-5*log_delta)/25 + delta*
                                      ((6*log_delta-1)/36 + delta*
                                       ((1-7*log_delta)/49 + delta*
                                        (8*log_delta-1)/64)))))));
    }
    else if (x<=2.0)
    {
        real logx = pl_log(x);
        return Pi*Pi/6.0 + small_dilogarithm(1.0-1.0/x) - logx*(0.5*logx+pl_log(1-1/x));
    } else 
    {
        real logx = pl_log(x);
        return Pi*Pi/3.0 - small_dilogarithm(1.0/x) - 0.5*logx*logx;
    }
}

real dilogarithm(real x)
{
    if (is_missing(x))
    {
#ifdef BOUNDCHECK
        PLWARNING("Dilogarithm taking NaN as input");
#endif
        return MISSING_VALUE;
    }
    if (x<0)
        return -positive_dilogarithm(-x) + 0.5*positive_dilogarithm(x*x);
    else 
        if (fast_exact_is_equal(x, 0)) return 0;
        else
            return positive_dilogarithm(x);
}

real hard_slope_integral(real l, real r, real a, real b)
{
    if (b<l) return 0;
    if (b<r)
    {
        if (a<l) 
            return 0.5*(b-l)*(b-l)/(r-l);
        else // a>=l
            return 0.5*((b-l)*(b-l)-(a-l)*(a-l))/(r-l);
    }
    else // b>=r
    {
        if (a<l)
            return 0.5*(r-l)+(b-r);
        else if (a<r) // l<a<r
            return 0.5*((r-l) - (a-l)*(a-l)/(r-l)) + (b-r);
        else // a>r
            return b-a;
    }
}

real soft_slope_integral(real smoothness, real left, real right, real a, real b)
{
    if (fast_exact_is_equal(smoothness, 0))
        return 0.5*(b-a);
    if (smoothness<100)
        return 
            (b - a) + (softplus_primitive(-smoothness*(b-right)) - softplus_primitive(-smoothness*(b-left))
                       -softplus_primitive(-smoothness*(a-right)) + softplus_primitive(-smoothness*(a-left)))/
            (smoothness*smoothness*(right-left));
    // else do the integral of the hard slope function
    return hard_slope_integral(left,right,a,b);
}

real tabulated_soft_slope_integral(real smoothness, real left, real right, real a, real b)
{
    if (fast_exact_is_equal(smoothness, 0))
        return 0.5*(b-a);
    if (smoothness<100)
        return 
            (b - a) + (tabulated_softplus_primitive(-smoothness*(b-right)) - tabulated_softplus_primitive(-smoothness*(b-left))
                       -tabulated_softplus_primitive(-smoothness*(a-right)) + tabulated_softplus_primitive(-smoothness*(a-left)))/
            (smoothness*smoothness*(right-left));
    // else do the integral of the hard slope function
    return hard_slope_integral(left,right,a,b);
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
