// -*- C++ -*-

// LinearARDKernel.cc
//
// Copyright (C) 2007-2009 Nicolas Chapados
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

/*! \file LinearARDKernel.cc */


#include "LinearARDKernel.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    LinearARDKernel,
    "Linear kernel that can be used for Automatic Relevance Determination",
    "This is a simple linear (dot-product) kernel that provides a different\n"
    "length-scale parameter for each input variable.  When used in conjunction\n"
    "with GaussianProcessRegressor it yields a Bayesian linear regression model\n"
    "with a non-isotropic prior.  (It is not a particularly efficient way of\n"
    "performing linear regression, but can be useful as a benchmark against\n"
    "other kernels).\n"
    "\n"
    "This kernel function is specified as:\n"
    "\n"
    "  k(x,y) = sf * (sum_i x_i * y_i / w_i) * k_kron(x,y)\n"
    "\n"
    "where sf is softplus(isp_signal_sigma), w_i is softplus(isp_global_sigma +\n"
    "isp_input_sigma[i]), and k_kron(x,y) is the result of the\n"
    "KroneckerBaseKernel evaluation, or 1.0 if there are no Kronecker terms.\n"
    "Note that since the Kronecker terms are incorporated multiplicatively, the\n"
    "very presence of the term associated to this kernel can be gated by the\n"
    "value of some input variable(s) (that are incorporated within one or more\n"
    "Kronecker terms).\n"
    "\n"
    "For best results, especially with moderately noisy data, IT IS IMPERATIVE\n"
    "to use whis kernel within a SummationKernel in conjunction with an\n"
    "IIDNoiseKernel, as follows (e.g. within a GaussianProcessRegressor):\n"
    "\n"
    "    kernel = SummationKernel(terms = [ LinearARDKernel(),\n"
    "                                       IIDNoiseKernel() ] )\n"
    "\n"
    "Note that to make its operations more robust when used with unconstrained\n"
    "optimization of hyperparameters, all hyperparameters of this kernel are\n"
    "specified in the inverse softplus domain.  See IIDNoiseKernel for more\n"
    "explanations.\n"
    );


LinearARDKernel::LinearARDKernel()
{ }


//#####  declareOptions  ######################################################

void LinearARDKernel::declareOptions(OptionList& ol)
{
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//#####  build  ###############################################################

void LinearARDKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}


//#####  build_  ##############################################################

void LinearARDKernel::build_()
{
    // Ensure that we multiply in Kronecker terms
    inherited::m_default_value = 1.0;
}


//#####  evaluate  ############################################################

real LinearARDKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    PLASSERT( x1.size() == x2.size() );
    PLASSERT( !m_isp_input_sigma.size() || x1.size() == m_isp_input_sigma.size() );

    real gating_term = inherited::evaluate(x1,x2);
    if (fast_is_equal(gating_term, 0.0) || x1.size() == 0)
        return 0.0;
    
    real the_dot    = 0.0;
    if (m_isp_input_sigma.size() > 0) {
        const real* px1 = x1.data();
        const real* px2 = x2.data();
        const real* pinpsig = m_isp_input_sigma.data();
        for (int i=0, n=x1.size() ; i<n ; ++i) {
            the_dot += (*px1++ * *px2++) / softplus(m_isp_global_sigma + *pinpsig++);
        }
    }
    else {
        real global_sigma = softplus(m_isp_global_sigma);
        the_dot = dot(x1, x2) / global_sigma;
    }

    // Gate by Kronecker term
    return softplus(m_isp_signal_sigma) * the_dot * gating_term;
}


//#####  computeGramMatrix  ###################################################

void LinearARDKernel::computeGramMatrix(Mat K) const
{
    PLASSERT( !m_isp_input_sigma.size() || dataInputsize() == m_isp_input_sigma.size() );
    PLASSERT( K.size() == 0 || m_data_cache.size() > 0 );  // Ensure data cached OK

    // Compute Kronecker gram matrix
    inherited::computeGramMatrix(K);

    // Precompute some terms. Make sure that the input sigmas don't get too
    // small
    real sf = softplus(m_isp_signal_sigma);
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
            real the_dot = 0.0;
            int  k = n;

            // Use Duff's device to unroll the following loop:
            //     while (k--) {
            //         the_dot += (*x1++ * *x2++) / *p_inpsigma++;
            //     }
            switch (k % 8) {
            case 0: do { the_dot += (*x1++ * *x2++) / *p_inpsigma++;
            case 7:      the_dot += (*x1++ * *x2++) / *p_inpsigma++;
            case 6:      the_dot += (*x1++ * *x2++) / *p_inpsigma++;
            case 5:      the_dot += (*x1++ * *x2++) / *p_inpsigma++;
            case 4:      the_dot += (*x1++ * *x2++) / *p_inpsigma++;
            case 3:      the_dot += (*x1++ * *x2++) / *p_inpsigma++;
            case 2:      the_dot += (*x1++ * *x2++) / *p_inpsigma++;
            case 1:      the_dot += (*x1++ * *x2++) / *p_inpsigma++;
                       } while((k -= 8) > 0);
            }

            // Multiplicatively update kernel matrix (already pre-filled with
            // Kronecker terms, or 1.0 if no Kronecker terms, as per build_).
            real Kij_cur = *Kij * sf * the_dot;
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

void LinearARDKernel::computeGramMatrixDerivative(
    Mat& KD, const string& kernel_param, real epsilon) const
{
    static const string ISS("isp_signal_sigma");
    static const string IGS("isp_global_sigma");
    static const string IIS("isp_input_sigma[");

    if (kernel_param == ISS) {
        computeGramMatrixDerivIspSignalSigma(KD);
        
        // computeGramMatrixDerivNV<
        //     LinearARDKernel,
        //     &LinearARDKernel::derivIspSignalSigma>(KD, this, -1);
    }
    else if (kernel_param == IGS) {
        computeGramMatrixDerivNV<
            LinearARDKernel,
            &LinearARDKernel::derivIspGlobalSigma>(KD, this, -1);
    }
    else if (string_begins_with(kernel_param, IIS) &&
             kernel_param[kernel_param.size()-1] == ']')
    {
        int arg = tolong(kernel_param.substr(
                             IIS.size(), kernel_param.size() - IIS.size() - 1));
        PLASSERT( arg < m_isp_input_sigma.size() );

        computeGramMatrixDerivIspInputSigma(KD, arg);

    }
    else
        inherited::computeGramMatrixDerivative(KD, kernel_param, epsilon);
}


//#####  evaluate_all_i_x  ####################################################

void LinearARDKernel::evaluate_all_i_x(const Vec& x, const Vec& k_xi_x,
                                       real squared_norm_of_x, int istart) const
{
    evaluateAllIXNV<LinearARDKernel>(x, k_xi_x, istart);
}



//#####  derivIspSignalSigma  #################################################

real LinearARDKernel::derivIspSignalSigma(int i, int j, int arg, real K) const
{
    // (No longer used; see computeGramMatrixDerivIspInputSigma below)
    return K*sigmoid(m_isp_signal_sigma)/softplus(m_isp_signal_sigma);
}


//#####  derivIspGlobalSigma  #################################################

real LinearARDKernel::derivIspGlobalSigma(int i, int j, int arg, real K) const
{
    if (fast_is_equal(K,0.))
        return 0.;

    return - K * sigmoid(m_isp_global_sigma) / softplus(m_isp_global_sigma);
}


//#####  computeGramMatrixDerivIspSignalSigma  ################################

void LinearARDKernel::computeGramMatrixDerivIspSignalSigma(Mat& KD) const
{
    int l = data->length();
    KD.resize(l,l);
    PLASSERT_MSG(
        gram_matrix.width() == l && gram_matrix.length() == l,
        "To compute the derivative with respect to 'isp_signal_sigma', the\n"
        "Gram matrix must be precomputed and cached in LinearARDKernel.");
    
    KD << gram_matrix;
    KD *= sigmoid(m_isp_signal_sigma)/softplus(m_isp_signal_sigma);
}


//#####  computeGramMatrixDerivIspInputSigma  #################################

void LinearARDKernel::computeGramMatrixDerivIspInputSigma(Mat& KD, int arg) const
{
    // Precompute some terms
    real signal_sigma    = softplus(m_isp_signal_sigma);
    real input_sigma_arg = m_input_sigma[arg];
    real input_sigma_sq  = input_sigma_arg * input_sigma_arg;
    real input_sigmoid   = sigmoid(m_isp_global_sigma + m_isp_input_sigma[arg]);
    
    // Compute Gram Matrix derivative w.r.t. isp_input_sigma[arg]
    int  l = data->length();
    PLASSERT_MSG(
        gram_matrix.width() == l && gram_matrix.length() == l,
        "To compute the derivative with respect to 'isp_input_sigma[i]', the\n"
        "Gram matrix must be precomputed and cached in LinearARDKernel.");

    // Variables that walk over the data matrix
    int  cache_mod = m_data_cache.mod();
    real *data_start = &m_data_cache(0,0);
    real *xi = data_start+arg;               // Iterator on data rows

    // Variables that walk over the kernel derivative matrix (KD)
    KD.resize(l,l);
    real* KDi = KD.data();                   // Start of row i
    real* KDij;                              // Current element on row i
    int   KD_mod = KD.mod();

    // Iterate on rows of derivative matrix
    for (int i=0 ; i<l ; ++i, xi += cache_mod, KDi += KD_mod)
    {
        KDij = KDi;
        real *xj  = data_start+arg;           // Inner iterator on data rows

        // Iterate on columns of derivative matrix
        for (int j=0 ; j <= i ; ++j, xj += cache_mod)
        {
            // Set into derivative matrix
            *KDij++ = - signal_sigma * (*xi * *xj) * input_sigmoid / input_sigma_sq;
        }
    }
}


//#####  makeDeepCopyFromShallowCopy  #########################################

void LinearARDKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
