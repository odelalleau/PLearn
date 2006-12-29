// -*- C++ -*-

// IIDNoiseKernel.cc
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

/*! \file IIDNoiseKernel.cc */

#include <plearn/math/TMat_maths.h>
#include "IIDNoiseKernel.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    IIDNoiseKernel,
    "Kernel representing independent and identically-distributed observation noise",
    "This Kernel is typically used as a base class for covariance functions used\n"
    "in gaussian processes (see GaussianProcessRegressor).  It represents simple\n"
    "i.i.d. additive noise:\n"
    "\n"
    "  k(x,y) = delta_x,y * sn2\n"
    "\n"
    "where delta_x,y is the Kronecker delta function, and sn2 is the exp of\n"
    "twice the 'log_noise_sigma' option.\n"
    "\n"
    "Note that to make its operations more robust when used with unconstrained\n"
    "optimization of hyperparameters, all hyperparameters of this kernel are\n"
    "specified in the log-domain.\n"
    );


IIDNoiseKernel::IIDNoiseKernel()
    : m_log_noise_sigma(0.0)
{ }


//#####  declareOptions  ######################################################

void IIDNoiseKernel::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "log_noise_sigma",
        &IIDNoiseKernel::m_log_noise_sigma,
        OptionBase::buildoption,
        "Log of the global noise variance.  Default value=0.0");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//#####  build  ###############################################################

void IIDNoiseKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}


//#####  build_  ##############################################################

void IIDNoiseKernel::build_()
{ }


//#####  evaluate  ############################################################

real IIDNoiseKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    if (fast_is_equal(powdistance(x1,x2,2), 0.0))
        return exp(2*m_log_noise_sigma);
    else
        return 0.0;
}


//#####  makeDeepCopyFromShallowCopy  #########################################

void IIDNoiseKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


//#####  computeGramMatrixDerivative  #########################################

void IIDNoiseKernel::computeGramMatrixDerivative(Mat& KD, const string& kernel_param,
                                                 real epsilon) const
{
    static const string LNS("log_noise_sigma");
    if (kernel_param == LNS) {
        if (!data)
            PLERROR("Kernel::computeGramMatrixDerivative should be called only after "
                    "setDataForKernelMatrix");

        // For efficiency reasons, we only accumulate a derivative on the
        // diagonal of the kernel matrix, even if two training examples happen
        // to be EXACTLY identical.  (May change in the future if this turns
        // out to be a problem).
        int W = nExamples();
        KD.resize(W,W);
        KD.fill(0.0);
        real deriv = 2*exp(2*m_log_noise_sigma);
        for (int i=0 ; i<W ; ++i)
            KD(i,i) = deriv;
    }
    else
        inherited::computeGramMatrixDerivative(KD, kernel_param, epsilon);
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
