// -*- C++ -*-

// SquaredExponentialARDKernel.cc
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

/*! \file SquaredExponentialARDKernel.cc */


#include "SquaredExponentialARDKernel.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    SquaredExponentialARDKernel,
    "Squared-Exponential kernel that can be used for Automatic Relevance Determination",
    "This is a variant of the GaussianKernel (a.k.a. Radial Basis Function)\n"
    "that provides a different length-scale parameter for each input variable.\n"
    "When used in conjunction with GaussianProcessRegressor, this kernel may be\n"
    "used for Automatic Relevance Determination (ARD), a procedure wherein the\n"
    "significance of each input variable for the prediction task is found\n"
    "automatically through numerical optimization.\n"
    "\n"
    "Similar to C.E. Rasmussen's GPML code (see http://www.gaussianprocess.org),\n"
    "this kernel function is specified as:\n"
    "\n"
    "  k(x,y) = sf2 * exp(- 0.5 * (sum_i (x_i - y_i)^2 / w_i)) + delta_x,y*sn2\n"
    "\n"
    "where sf2 is the exp of the 'log_signal_sigma' option, sn2 is the exp of\n"
    "the 'log_noise_sigma' option (added only if x==y), and w_i is\n"
    "exp(log_global_sigma + log_input_sigma[i]).\n"
    "\n"
    "Note that to make its operations more robust when used with unconstrained\n"
    "optimizaiton of hyperparameters, all hyperparameters of this kernel are\n"
    "specified in the log-domain.\n"
    );


SquaredExponentialARDKernel::SquaredExponentialARDKernel()
    : m_log_signal_sigma(0.0),
      m_log_noise_sigma(0.0),
      m_log_global_sigma(0.0)
{ }


//#####  declareOptions  ######################################################

void SquaredExponentialARDKernel::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "log_signal_sigma",
        &SquaredExponentialARDKernel::m_log_signal_sigma,
        OptionBase::buildoption,
        "Log of the global signal variance.  Default value=0.0");

    declareOption(
        ol, "log_noise_sigma",
        &SquaredExponentialARDKernel::m_log_noise_sigma,
        OptionBase::buildoption,
        "Log of the global noise variance.  Default value=0.0");
    
    declareOption(
        ol, "log_global_sigma",
        &SquaredExponentialARDKernel::m_log_global_sigma,
        OptionBase::buildoption,
        "Log of the global length-scale.  Note that if ARD is performed on\n"
        "input-specific sigmas, this hyperparameter should have a fixed value\n"
        "(and not be varied during the optimization).  Default value=0.0.\n");

    declareOption(
        ol, "log_input_sigma",
        &SquaredExponentialARDKernel::m_log_input_sigma,
        OptionBase::buildoption,
        "If specified, contain input-specific length-scales that can be\n"
        "individually optimized for.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//#####  build  ###############################################################

void SquaredExponentialARDKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}


//#####  build_  ##############################################################

void SquaredExponentialARDKernel::build_()
{
}


//#####  evaluate  ############################################################

real SquaredExponentialARDKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    PLASSERT( x1.size() == x2.size() );
    PLASSERT( !m_log_input_sigma.size() || x1.size() == m_log_input_sigma.size() );

    if (x1.size() == 0)
        return exp(2*m_log_signal_sigma) + exp(2*m_log_noise_sigma);
    
    const real* px1 = x1.data();
    const real* px2 = x2.data();
    real expval = 0.0;
    real sum_sqdiff = 0.0;
    
    if (m_log_input_sigma.size() > 0) {
        const real* pinpsig = m_log_input_sigma.data();
        for (int i=0, n=x1.size() ; i<n ; ++i) {
            real diff   = *px1++ - *px2++;
            real sqdiff = diff * diff;
            sum_sqdiff += sqdiff;
            expval     += sqdiff / exp(2*(m_log_global_sigma + *pinpsig++));
        }
    }
    else {
        real global_sigma = exp(2*m_log_global_sigma);
        for (int i=0, n=x1.size() ; i<n ; ++i) {
            real diff   = *px1++ - *px2++;
            real sqdiff = diff * diff;
            sum_sqdiff += sqdiff;
            expval     += sqdiff / global_sigma;
        }
    }

    // We add a noise variance only if x and y are equal (within machine tolerance)
    real noise_cov = 0.0;
    if (is_equal(sum_sqdiff, 0))
        noise_cov = exp(2*m_log_noise_sigma);
    return exp(2*m_log_signal_sigma + -0.5 * expval) + noise_cov;
}


//#####  computeGramMatrixDerivative  #########################################

/**
void SquaredExponentialARDKernel::computeGramMatrixDerivative(
    Mat& KD, const string& kernel_param, real epsilon) const
{
    static const string LSV = "log_signal_sigma";
    static const string LNV = "log_noise_sigma";
    static const string LGS = "log_global_sigma";
    static const string LIS = "log_input_sigma[";

    const int W = nExamples();
    KD.resize(W,W);
    const int mod = KD.mod();
    real* KDij;
    real* KDji;

    // log_signal_sigma
    if (kernel_param == LSV) {

    }

    // log_noise_sigma
    else if (kernel_param == LNV) {
        for (int i=0 ; i<W ; ++i) {
            KDij = KD[i];
            KDji = &KD[0][i];
            for (int j=0 ; j<i ; ++j, Kji += m) {
                // Below main diagonal, we have to check if x_i == x_j
                
            }
        }
        
    }

    // log_global_sigma
    else if (kernel_param == LGS) {

    }

    // log_input_sigma
    else if (kernel_param.substr(0,16) == LIS) {

    }
    else
        PLERROR("SquaredExponentialARDKernel::computeGramMatrixDerivative: "
                "unknown hyperparameter '%s'", kernel_param.c_str());
}
*/


//#####  makeDeepCopyFromShallowCopy  #########################################

void SquaredExponentialARDKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
