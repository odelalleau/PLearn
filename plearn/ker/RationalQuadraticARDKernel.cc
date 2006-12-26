// -*- C++ -*-

// RationalQuadraticARDKernel.cc
//
// Copyright (C) 2006 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file RationalQuadraticARDKernel.cc */


#include "RationalQuadraticARDKernel.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RationalQuadraticARDKernel,
    "Rational-Quadratic kernel that can be used for Automatic Relevance Determination",
    "This kernel can be interpreted as an infinite mixture of\n"
    "SquaredExponentialARDKernel (with different characteristic length-scales),\n"
    "allowing a greater variety of \"interesting\" functions to be generated.\n"
    "Similar to C.E. Rasmussen's GPML code (see http://www.gaussianprocess.org),\n"
    "this kernel is specified as:\n"
    "\n"
    "  k(x,y) = sf2 * [1 + (sum_i (x_i - y_i)^2 / w_i)/(2*alpha)]^(-alpha) + delta_x,y*sn2\n"
    "\n"
    "where sf2 is the exp of twice the 'log_signal_sigma' option, sn2 is the\n"
    "exp of twice the 'log_noise_sigma' option (added only if x==y), and w_i\n"
    "is exp(2*log_global_sigma + 2*log_input_sigma[i]).\n"
    "\n"
    "Note that to make its operations more robust when used with unconstrained\n"
    "optimizaiton of hyperparameters, all hyperparameters of this kernel are\n"
    "specified in the log-domain.\n"
    );


RationalQuadraticARDKernel::RationalQuadraticARDKernel()
    : m_log_signal_sigma(0.0),
      m_log_noise_sigma(0.0),
      m_log_alpha(0.0),
      m_log_global_sigma(0.0)
{ }


//#####  declareOptions  ######################################################

void RationalQuadraticARDKernel::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "log_signal_sigma",
        &RationalQuadraticARDKernel::m_log_signal_sigma,
        OptionBase::buildoption,
        "Log of the global signal variance.  Default value=0.0");

    declareOption(
        ol, "log_noise_sigma",
        &RationalQuadraticARDKernel::m_log_noise_sigma,
        OptionBase::buildoption,
        "Log of the global noise variance.  Default value=0.0");

    declareOption(
        ol, "log_alpha",
        &RationalQuadraticARDKernel::m_log_alpha,
        OptionBase::buildoption,
        "Log of the alpha parameter in the rational-quadratic kernel.\n"
        "Default value=0.0");
    
    declareOption(
        ol, "log_global_sigma",
        &RationalQuadraticARDKernel::m_log_global_sigma,
        OptionBase::buildoption,
        "Log of the global length-scale.  Note that if ARD is performed on\n"
        "input-specific sigmas, this hyperparameter should have a fixed value\n"
        "(and not be varied during the optimization).  Default value=0.0.\n");

    declareOption(
        ol, "log_input_sigma",
        &RationalQuadraticARDKernel::m_log_input_sigma,
        OptionBase::buildoption,
        "If specified, contain input-specific length-scales that can be\n"
        "individually optimized for.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//#####  build  ###############################################################

void RationalQuadraticARDKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}


//#####  build_  ##############################################################

void RationalQuadraticARDKernel::build_()
{ }


//#####  evaluate  ############################################################

real RationalQuadraticARDKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    PLASSERT( x1.size() == x2.size() );
    PLASSERT( !m_log_input_sigma.size() || x1.size() == m_log_input_sigma.size() );

    if (x1.size() == 0)
        return exp(2*m_log_signal_sigma) + exp(2*m_log_noise_sigma);
    
    const real* px1 = x1.data();
    const real* px2 = x2.data();
    real sf2        = exp(2*m_log_signal_sigma);
    real alpha      = exp(m_log_alpha);
    real sum_wt     = 0.0;
    real sum_sqdiff = 0.0;
    
    if (m_log_input_sigma.size() > 0) {
        const real* pinpsig = m_log_input_sigma.data();
        for (int i=0, n=x1.size() ; i<n ; ++i) {
            real diff   = *px1++ - *px2++;
            real sqdiff = diff * diff;
            sum_sqdiff += sqdiff;
            sum_wt     += sqdiff / exp(2*(m_log_global_sigma + *pinpsig++));
        }
    }
    else {
        real global_sigma = exp(2*m_log_global_sigma);
        for (int i=0, n=x1.size() ; i<n ; ++i) {
            real diff   = *px1++ - *px2++;
            real sqdiff = diff * diff;
            sum_sqdiff += sqdiff;
            sum_wt     += sqdiff / global_sigma;
        }
    }

    // We add a noise variance only if x and y are equal (within machine tolerance)
    real noise_cov = 0.0;
    if (is_equal(sum_sqdiff, 0))
        noise_cov = exp(2*m_log_noise_sigma);
    return sf2 * pow(1 + sum_wt / (2.*alpha), -alpha) + noise_cov;
}


//#####  makeDeepCopyFromShallowCopy  #########################################

void RationalQuadraticARDKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_log_input_sigma, copies);
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
