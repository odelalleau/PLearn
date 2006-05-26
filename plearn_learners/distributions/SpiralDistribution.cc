
// -*- C++ -*-

// SpiralDistribution.cc
//
// Copyright (C) 2003  Pascal Vincent
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
 ******************************************************* */

/*! \file SpiralDistribution.cc */
#include "SpiralDistribution.h"

namespace PLearn {
using namespace std;

SpiralDistribution::SpiralDistribution()
    : lambda(0.04),
      alpha(1),
      tmin(3),
      tmax(15),
      sigma(0.01),
      uniformity(1),
      include_t(false)
{
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(SpiralDistribution, "Generates samples drawn from a 2D spiral",
                        "SpiralDistribution is a generative model that generates 2D (x,y) samples in the following manner:\n"
                        " t ~ uniform([tmin, tmax])^uniformity \n"
                        " x = lambda*t*sin(alpha*t) + N(0,sigma) \n"
                        " y = lambda*t*cos(alpha*t) + N(0,sigma) \n");

void SpiralDistribution::declareOptions(OptionList& ol)
{
    declareOption(ol, "lambda", &SpiralDistribution::lambda, OptionBase::buildoption,"");
    declareOption(ol, "alpha", &SpiralDistribution::alpha, OptionBase::buildoption,"");
    declareOption(ol, "tmin", &SpiralDistribution::tmin, OptionBase::buildoption,"");
    declareOption(ol, "tmax", &SpiralDistribution::tmax, OptionBase::buildoption,"");
    declareOption(ol, "sigma", &SpiralDistribution::sigma, OptionBase::buildoption,"");
    declareOption(ol, "uniformity", &SpiralDistribution::uniformity, OptionBase::buildoption,"");
    declareOption(ol, "include_t", &SpiralDistribution::include_t, OptionBase::buildoption,
                  "If true, then t will be appended to the generated sample, along with x and y.");

    inherited::declareOptions(ol);
}

void SpiralDistribution::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
    
    // annoying stuff due to conditional distribution (silly, fix this some day)
    predicted_size = inputsize();
    inherited::build();
}

// ### Nothing to add here, simply calls build_
void SpiralDistribution::build()
{
    inherited::build();
    build_();
}

void SpiralDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("SpiralDistribution::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

real SpiralDistribution::log_density(const Vec& x) const
{ PLERROR("density not implemented for SpiralDistribution"); return 0; }

real SpiralDistribution::survival_fn(const Vec& x) const
{ PLERROR("survival_fn not implemented for SpiralDistribution"); return 0; }

real SpiralDistribution::cdf(const Vec& x) const
{ PLERROR("cdf not implemented for SpiralDistribution"); return 0; }

void SpiralDistribution::expectation(Vec& mu) const
{ PLERROR("expectation not implemented for SpiralDistribution"); }

void SpiralDistribution::variance(Mat& covar) const
{ PLERROR("variance not implemented for SpiralDistribution"); }

void SpiralDistribution::curve(real t, real& x, real& y) const
{
    x = lambda*t*sin(alpha*t);
    y = lambda*t*cos(alpha*t);
}

void SpiralDistribution::generate(Vec& v) const
{
    v.resize(inputsize());

    real x, y;
    real u =  random_gen->bounded_uniform(0,1);
    real t = (fast_is_equal(uniformity, 1))?u:pow(u,uniformity);
    t = tmin+(tmax-tmin)*t;
    curve(t,x,y);
    x += random_gen->gaussian_mu_sigma(0, sigma);
    y += random_gen->gaussian_mu_sigma(0, sigma);

    v[0] = x;
    v[1] = y;
    if(inputsize()==3)
        v[2] = t;
}


// Default version of inputsize returns learner->inputsize()
// If this is not appropriate, you should uncomment this and define
// it properly in the .cc
int SpiralDistribution::inputsize() const
{ return include_t ?3 :2; }

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
