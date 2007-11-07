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

/*! \file PLearn/plearn/math/pl_erf.h */

#ifndef PL_ERF_H
#define PL_ERF_H

#include <plearn/base/general.h>

namespace PLearn {
using namespace std;

//! function gamma returns log(Gamma(z)), where
//!  Gamma(z) = \int_0^infty t^{z-1}*e^{-t} dt
real pl_gammln(real z);

//! d(pl_gammln(z))/dz 
//! derivate of pl_gammln(z) = digamma function = d(log(gamma(z))/dz
real pl_dgammlndz(real z);

//!  returns the series value of
//!  the incomplete gamma function
real pl_gser(real a, real x);

//!  returns the continued fraction representation of
//!  the incomplete gamma function
real pl_gcf(real a, real x);

//!  returns the incomplete gamma function Q(a,x) = 1 - P(a,x)
//!  it either uses the series or the continued fraction formula
real pl_gammq(real a, real x);

//!  The error function
real pl_erf(real x);

//!  For X ~ Normal(0,1), cumulative probability function P(X<x)
real gauss_01_cum(real x);

inline real normal_cdf(real x) { return gauss_01_cum(x); }

inline real gauss_cum(real x, real mu, real sigma)
{ return gauss_01_cum((x-mu)/sigma); }

/*!   For X ~ Normal(0,1), inverse of cumulative probability function P(X<x)
  i.e. approximately gauss_01_quantile(gauss_01_cum(x)) ~=~ x
  (the inverse is computed with a binary search, the bisection method)
*/
real gauss_01_quantile(real q);

//!  for X ~ Normal(0,1), return density of X at x
real gauss_01_density(real x);
real gauss_01_log_density(real x);
real gauss_log_density_var(real x, real mu, real var);
real gauss_log_density_stddev(real x, real mu, real sigma);

real gauss_density_var(real x, real mu, real var);

inline real gauss_density_stddev(real x, real mu, real sigma)
{ return gauss_01_density((x-mu)/sigma); }

// returns p-value for mu under a normal distribution given the squared
// standard error v/n, where v is the variance of the observations and n is
// the number of observations.
real p_value(real mu, real vn);


} // end of namespace PLearn

#endif


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
