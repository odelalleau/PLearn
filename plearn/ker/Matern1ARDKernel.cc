// -*- C++ -*-

// Matern1ARDKernel.cc
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

/*! \file Matern1ARDKernel.cc */


#include "Matern1ARDKernel.h"
#include <plearn/math/pl_math.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    Matern1ARDKernel,
    "Matern kernel with nu=1/2 that can be used for Automatic Relevance Determination.",
    "With nu=1/2, the Matern kernel corresponds to the Ornstein-Uhlenbeck\n"
    "process.  This function is specified as:\n"
    "\n"
    "  k(x,y) = (sf / (2*a)) * exp(-a sum_i |x_i - y_i|/w_i) * k_kron(x,y)\n"
    "\n"
    "where sf = softplus(isp_signal_sigma), a = softplus(isp_persistence), w_i =\n"
    "softplus(isp_global_sigma + isp_input_sigma[i]), and k_kron(x,y) is the\n"
    "result of the KroneckerBaseKernel evaluation, or 1.0 if there are no\n"
    "Kronecker terms.  Note that since the Kronecker terms are incorporated\n"
    "multiplicatively, the very presence of the term associated to this kernel\n"
    "can be gated by the value of some input variable(s) (that are incorporated\n"
    "within one or more Kronecker terms).\n"
    "\n"
    "Note that to make its operations more robust when used with unconstrained\n"
    "optimization of hyperparameters, all hyperparameters of this kernel are\n"
    "specified in the inverse softplus domain.  See IIDNoiseKernel for more\n"
    "explanations.\n"
    );


Matern1ARDKernel::Matern1ARDKernel()
    : m_isp_persistence(pl_log(exp(1.0) - 1.)) // inverse-softplus(1.0)
{ }


//#####  declareOptions  ######################################################

void Matern1ARDKernel::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "isp_persistence",
        &Matern1ARDKernel::m_isp_persistence,
        OptionBase::buildoption,
        "Inverse softplus of the O-U persistence parameter.  Default value =\n"
        "isp(1.0).");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//#####  build  ###############################################################

void Matern1ARDKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}


//#####  build_  ##############################################################

void Matern1ARDKernel::build_()
{
    // Ensure that we multiply in Kronecker terms
    inherited::m_default_value = 1.0;
}


//#####  evaluate  ############################################################

real Matern1ARDKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    PLASSERT( x1.size() == x2.size() );
    PLASSERT( !m_isp_input_sigma.size() || x1.size() == m_isp_input_sigma.size() );

    real gating_term = inherited::evaluate(x1,x2);
    if (fast_is_equal(gating_term, 0.0))
        return 0.0;
    
    if (x1.size() == 0)
        return softplus(m_isp_signal_sigma) /
            (2*softplus(m_isp_persistence)) * gating_term;
    
    const real* px1 = x1.data();
    const real* px2 = x2.data();
    real sf         = softplus(m_isp_signal_sigma);
    real persistence= softplus(m_isp_persistence);
    real expval     = 0.0;

    // Case where we have real ARD
    if (m_isp_input_sigma.size() > 0) {
        const real* pinpsig = m_isp_input_sigma.data();
        for (int i=0, n=x1.size() ; i<n ; ++i) {
            real diff    = *px1++ - *px2++;
            real absdiff = fabs(diff);
            expval      += absdiff / softplus(m_isp_global_sigma + *pinpsig++);
        }
    }
    // No ARD
    else {
        real global_sigma = softplus(m_isp_global_sigma);
        for (int i=0, n=x1.size() ; i<n ; ++i) {
            real diff    = *px1++ - *px2++;
            real absdiff = fabs(diff);
            expval      += absdiff / global_sigma;
        }
    }

    // Gate by Kronecker term
    return sf / (2. * persistence) * exp(-persistence * expval) * gating_term;
}


//#####  computeGramMatrix  ###################################################

void Matern1ARDKernel::computeGramMatrix(Mat K) const
{
    PLASSERT( !m_isp_input_sigma.size() || dataInputsize() == m_isp_input_sigma.size() );
    PLASSERT( K.size() == 0 || m_data_cache.size() > 0 );  // Ensure data cached OK

    // Compute Kronecker gram matrix
    inherited::computeGramMatrix(K);

    // Precompute some terms. Make sure that the input sigmas don't get too
    // small
    real sf          = softplus(m_isp_signal_sigma);
    real persistence = softplus(m_isp_persistence);
    m_input_sigma.resize(dataInputsize());
    softplusFloor(m_isp_global_sigma, 1e-6);
    m_input_sigma.fill(m_isp_global_sigma);  // Still in ISP domain
    for (int i=0, n=m_input_sigma.size() ; i<n ; ++i) {
        if (m_isp_input_sigma.size() > 0) {
            softplusFloor(m_isp_input_sigma[i], 1e-6);
            m_input_sigma[i] += m_isp_input_sigma[i];
        }
        m_input_sigma[i] = softplus(m_input_sigma[i]);
    }

    // Compute Gram Matrix
    int  l = data->length();
    int  m = K.mod();
    int  n = dataInputsize();
    int  cache_mod = m_data_cache.mod();

    real *data_start = &m_data_cache(0,0);
    real *Ki = K[0];                         // Start of current row
    real *Kij;                               // Current element along row
    real *input_sigma_data = m_input_sigma.data();
    real *xi = data_start;
    
    for (int i=0 ; i<l ; ++i, xi += cache_mod, Ki+=m)
    {
        Kij = Ki;
        real *xj = data_start;

        for (int j=0; j<=i; ++j, xj += cache_mod) {
            // Kernel evaluation per se
            real *x1 = xi;
            real *x2 = xj;
            real *p_inpsigma = input_sigma_data;
            real sum_wt = 0.0;
            int  k = n;

            // Use Duff's device to unroll the following loop:
            //     while (k--) {
            //         real diff = *x1++ - *x2++;
            //         sum_wt += fabs(diff) / *p_inpsigma++;
            //     }
            real diff;
            switch (k % 8) {
            case 0: do { diff = *x1++ - *x2++; sum_wt += fabs(diff) / *p_inpsigma++;
            case 7:      diff = *x1++ - *x2++; sum_wt += fabs(diff) / *p_inpsigma++;
            case 6:      diff = *x1++ - *x2++; sum_wt += fabs(diff) / *p_inpsigma++;
            case 5:      diff = *x1++ - *x2++; sum_wt += fabs(diff) / *p_inpsigma++;
            case 4:      diff = *x1++ - *x2++; sum_wt += fabs(diff) / *p_inpsigma++;
            case 3:      diff = *x1++ - *x2++; sum_wt += fabs(diff) / *p_inpsigma++;
            case 2:      diff = *x1++ - *x2++; sum_wt += fabs(diff) / *p_inpsigma++;
            case 1:      diff = *x1++ - *x2++; sum_wt += fabs(diff) / *p_inpsigma++;
                       } while((k -= 8) > 0);
            }

            // Multiplicatively update kernel matrix (already pre-filled with
            // Kronecker terms, or 1.0 if no Kronecker terms, as per build_).
            real Kij_cur = *Kij * sf / (2.*persistence) * exp(-persistence * sum_wt);
            *Kij++ = Kij_cur;
        }
    }
    if (cache_gram_matrix) {
        gram_matrix.resize(l,l);
        gram_matrix << K;
        gram_matrix_is_cached = true;
    }
}


//#####  computeGramMatrixDerivative  #########################################

void Matern1ARDKernel::computeGramMatrixDerivative(
    Mat& KD, const string& kernel_param, real epsilon) const
{
    static const string ISS("isp_signal_sigma");
    static const string IGS("isp_global_sigma");
    static const string IIS("isp_input_sigma[");
    static const string IPe("isp_persistence");

    if (kernel_param == ISS) {
        computeGramMatrixDerivIspSignalSigma(KD);
        
        // computeGramMatrixDerivNV<
        //     Matern1ARDKernel,
        //     &Matern1ARDKernel::derivIspSignalSigma>(KD, this, -1);
    }
    /*
    else if (kernel_param == IGS) {
        computeGramMatrixDerivNV<
            Matern1ARDKernel,
            &Matern1ARDKernel::derivIspGlobalSigma>(KD, this, -1);
    }
    else if (string_begins_with(kernel_param, IIS) &&
             kernel_param[kernel_param.size()-1] == ']')
    {
        int arg = tolong(kernel_param.substr(
                             IIS.size(), kernel_param.size() - IIS.size() - 1));
        PLASSERT( arg < m_isp_input_sigma.size() );

        computeGramMatrixDerivIspInputSigma(KD, arg);

    }
    */
    else
        inherited::computeGramMatrixDerivative(KD, kernel_param, epsilon);
}


//#####  evaluate_all_i_x  ####################################################

void Matern1ARDKernel::evaluate_all_i_x(const Vec& x, const Vec& k_xi_x,
                                        real squared_norm_of_x, int istart) const
{
    evaluateAllIXNV<Matern1ARDKernel>(x, k_xi_x, istart);
}


//#####  derivIspSignalSigma  #################################################

real Matern1ARDKernel::derivIspSignalSigma(int i, int j, int arg, real K) const
{
    // (No longer used; see computeGramMatrixDerivIspInputSigma below)
    return K*sigmoid(m_isp_signal_sigma)/softplus(m_isp_signal_sigma);
}


//#####  derivIspGlobalSigma  #################################################

real Matern1ARDKernel::derivIspGlobalSigma(int i, int j, int arg, real K) const
{
    if (fast_is_equal(K,0.))
        return 0.;

    // The norm term inside the exponential may be accessed as Log(K/sf)
    real inner = pl_log(K / softplus(m_isp_signal_sigma));
    return - K * inner * sigmoid(m_isp_global_sigma) / softplus(m_isp_global_sigma);

    // Note: in the above expression for 'inner' there is the implicit
    // assumption that the input_sigma[i] are zero, which allows the
    // sigmoid/softplus term to be factored out of the norm summation.
}


//#####  computeGramMatrixDerivIspSignalSigma  ################################

void Matern1ARDKernel::computeGramMatrixDerivIspSignalSigma(Mat& KD) const
{
    int l = data->length();
    KD.resize(l,l);
    PLASSERT_MSG(
        gram_matrix.width() == l && gram_matrix.length() == l,
        "To compute the derivative with respect to 'isp_signal_sigma', the\n"
        "Gram matrix must be precomputed and cached in Matern1ARDKernel.");
    
    KD << gram_matrix;
    KD *= sigmoid(m_isp_signal_sigma)/softplus(m_isp_signal_sigma);
}


//#####  computeGramMatrixDerivIspInputSigma  #################################

void Matern1ARDKernel::computeGramMatrixDerivIspInputSigma(Mat& KD,
                                                           int arg) const
{
    // Precompute some terms
    real input_sigma_arg = m_input_sigma[arg];
    real input_sigma_sq  = input_sigma_arg * input_sigma_arg;
    real input_sigmoid   = sigmoid(m_isp_global_sigma + m_isp_input_sigma[arg]);
    
    // Compute Gram Matrix derivative w.r.t. isp_input_sigma[arg]
    int  l = data->length();
    PLASSERT_MSG(
        gram_matrix.width() == l && gram_matrix.length() == l,
        "To compute the derivative with respect to 'isp_input_sigma[i]', the\n"
        "Gram matrix must be precomputed and cached in Matern1ARDKernel.");

    // Variables that walk over the data matrix
    int  cache_mod = m_data_cache.mod();
    real *data_start = &m_data_cache(0,0);
    real *xi = data_start+arg;               // Iterator on data rows

    // Variables that walk over the gram cache
    int   gram_cache_mod = gram_matrix.mod();
    real *gram_cache_row = gram_matrix.data();
    real *gram_cache_cur;
    
    // Variables that walk over the kernel derivative matrix (KD)
    KD.resize(l,l);
    real* KDi = KD.data();                   // Start of row i
    real* KDij;                              // Current element on row i
    int   KD_mod = KD.mod();

    // Iterate on rows of derivative matrix
    for (int i=0 ; i<l ; ++i, xi += cache_mod, KDi += KD_mod,
             gram_cache_row += gram_cache_mod)
    {
        KDij = KDi;
        real *xj  = data_start+arg;           // Inner iterator on data rows
        gram_cache_cur = gram_cache_row;

        // Iterate on columns of derivative matrix
        for (int j=0 ; j <= i
                 ; ++j, xj += cache_mod, ++gram_cache_cur)
        {
            real diff    = *xi - *xj;
            real sq_diff = diff * diff;
            real KD_cur  = 0.5 * *gram_cache_cur *
                           input_sigmoid * sq_diff / input_sigma_sq;

            // Set into derivative matrix
            *KDij++ = KD_cur;
        }
    }
}


//#####  makeDeepCopyFromShallowCopy  #########################################

void Matern1ARDKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
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
