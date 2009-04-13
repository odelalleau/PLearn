// -*- C++ -*-

// PLearnerDiagonalKernel.cc
//
// Copyright (C) 2009 Nicolas Chapados
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

/*! \file PLearnerDiagonalKernel.cc */


#include "PLearnerDiagonalKernel.h"
#include <plearn/math/pl_math.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PLearnerDiagonalKernel,
    "Diagonal kernel from the output of a PLearner.",
    "The output of this kernel is given by:\n"
    "\n"
    "  k(x,x) = isp_signal_sigma * exp(learner->computeOutput(x))\n"
    "\n"
    "and is 0 for x != y.\n"
    "\n"
    "This is useful for representing heteroscedastic noise in Gaussian\n"
    "Processes, where the log-noise process is the output of another learner\n"
    "(e.g. another Gaussian Process).\n"
    );


PLearnerDiagonalKernel::PLearnerDiagonalKernel()
    : m_isp_signal_sigma(0.)
{ }


//#####  declareOptions  ######################################################

void PLearnerDiagonalKernel::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "learner",
        &PLearnerDiagonalKernel::m_learner,
        OptionBase::buildoption,
        "Learner we are taking output from.");
    
    declareOption(
        ol, "isp_signal_sigma",
        &PLearnerDiagonalKernel::m_isp_signal_sigma,
        OptionBase::buildoption,
        "Inverse softplus of the global noise variance.  Default value = 0.0.");
        
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//#####  build  ###############################################################

void PLearnerDiagonalKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}


//#####  build_  ##############################################################

void PLearnerDiagonalKernel::build_()
{
    if (m_learner.isNull())
        PLERROR("PLearnerDiagonalKernel::build: the option 'learner' must be specified");

    // At build-time, we don't yet know the learner outputsize
    // if (m_learner->outputsize() != 1)
    //     PLERROR("PLearnerDiagonalKernel::build: the learner must have an outputsize of 1; "
    //             "current outputsize is %d", m_learner->outputsize());
    
    // Ensure that we multiply in Kronecker terms
    inherited::m_default_value = 1.0;
}


//#####  evaluate  ############################################################

real PLearnerDiagonalKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    PLASSERT( x1.size() == x2.size() );
    PLASSERT( ! m_learner.isNull() && m_learner->outputsize() == 1);

    m_output_buffer.resize(m_learner->outputsize());
    
    if (x1 == x2) {
        real gating_term = inherited::evaluate(x1,x2);
        real sigma = softplus(m_isp_signal_sigma);
        m_learner->computeOutput(x1, m_output_buffer);
        real diag_term = exp(m_output_buffer[0]);
        return sigma * gating_term * diag_term;
    }
    else
        return 0.0;
}


//#####  computeGramMatrix  ###################################################

void PLearnerDiagonalKernel::computeGramMatrix(Mat K) const
{
    PLASSERT( K.size() == 0 || m_data_cache.size() > 0 );  // Ensure data cached OK
    PLASSERT( ! m_learner.isNull() && m_learner->outputsize() == 1);

    m_output_buffer.resize(m_learner->outputsize());
    
    // Most elements are zero, except for the diagonal
    K.fill(0.0);

    real sigma = softplus(m_isp_signal_sigma);
    int  n = m_data_cache.length();
    
    PLASSERT( K.length() == n && K.width() == n );
    
    for (int i=0 ; i<n ; ++i) {
        real gating_term = inherited::evaluate_i_j(i, i);
        Vec input_i = m_data_cache(i);
        m_learner->computeOutput(input_i, m_output_buffer);
        real diag_term = exp(m_output_buffer[0]);
        K(i,i) = sigma * gating_term * diag_term;
    }

    if (cache_gram_matrix) {
        gram_matrix.resize(n,n);
        gram_matrix << K;
        gram_matrix_is_cached = true;
    }
}


//#####  computeGramMatrixDerivative  #########################################

void PLearnerDiagonalKernel::computeGramMatrixDerivative(
    Mat& KD, const string& kernel_param, real epsilon) const
{
    static const string ISS("isp_signal_sigma");

    if (kernel_param == ISS) {
        computeGramMatrixDerivIspSignalSigma(KD);
        
        // computeGramMatrixDerivNV<
        //     PLearnerDiagonalKernel,
        //     &PLearnerDiagonalKernel::derivIspSignalSigma>(KD, this, -1);
    }
    else
        inherited::computeGramMatrixDerivative(KD, kernel_param, epsilon);
}


//#####  evaluate_all_i_x  ####################################################

void PLearnerDiagonalKernel::evaluate_all_i_x(const Vec& x, const Vec& k_xi_x,
                                              real squared_norm_of_x, int istart) const
{
    evaluateAllIXNV<PLearnerDiagonalKernel>(x, k_xi_x, istart);
}


//#####  computeGramMatrixDerivIspSignalSigma  ################################

void PLearnerDiagonalKernel::computeGramMatrixDerivIspSignalSigma(Mat& KD) const
{
    int l = data->length();
    KD.resize(l,l);
    PLASSERT_MSG(
        gram_matrix.width() == l && gram_matrix.length() == l,
        "To compute the derivative with respect to 'isp_signal_sigma', the\n"
        "Gram matrix must be precomputed and cached in PLearnerDiagonalKernel.");
    
    KD << gram_matrix;
    KD *= sigmoid(m_isp_signal_sigma)/softplus(m_isp_signal_sigma);
}

//#####  makeDeepCopyFromShallowCopy  #########################################

void PLearnerDiagonalKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(m_learner, copies);
    deepCopyField(m_output_buffer, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);
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
