// -*- C++ -*-

// SquaredExponentialARDKernel.cc
//
// Copyright (C) 2006-2007 Nicolas Chapados
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
    "  k(x,y) = sf * exp(- 0.5 * (sum_i (x_i - y_i)^2 / w_i)) * k_kron(x,y)\n"
    "\n"
    "where sf is softplus(isp_signal_sigma), w_i is softplus(isp_global_sigma +\n"
    "isp_input_sigma[i]), and k_kron(x,y) is the result of the\n"
    "KroneckerBaseKernel evaluation, or 1.0 if there are no Kronecker terms.\n"
    "Note that since the Kronecker terms are incorporated multiplicatively, the\n"
    "very presence of the term associated to this kernel can be gated by the\n"
    "value of some input variable(s) (that are incorporated within one or more\n"
    "Kronecker terms).\n"
    "\n"
    "The current version of this class DOES NOT ALLOW differentiating the Kernel\n"
    "matrix with respect to the Kronecker hyperparameters.  These parameters are\n"
    "redundant due to the presence of the global sf above; they should be set to\n"
    "1.0 and left untouched by hyperoptimization.\n"
    "\n"
    "Note that contrarily to previous versions that incorporated IID noise and\n"
    "Kronecker terms ADDITIVELY, this version does not add any noise at all (and\n"
    "as explained above incorporates the Kronecker terms multiplicatively).  For\n"
    "best results, especially with moderately noisy data, IT IS IMPERATIVE to\n"
    "use whis kernel within a SummationKernel in conjunction with an\n"
    "IIDNoiseKernel, as follows (e.g. within a GaussianProcessRegressor):\n"
    "\n"
    "    kernel = SummationKernel(terms = [ SquaredExponentialARDKernel(),\n"
    "                                       IIDNoiseKernel() ] )\n"
    "\n"
    "Note that to make its operations more robust when used with unconstrained\n"
    "optimization of hyperparameters, all hyperparameters of this kernel are\n"
    "specified in the inverse softplus domain.  See IIDNoiseKernel for more\n"
    "explanations.\n"
    );


SquaredExponentialARDKernel::SquaredExponentialARDKernel()
{ }


//#####  declareOptions  ######################################################

void SquaredExponentialARDKernel::declareOptions(OptionList& ol)
{
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
    // Ensure that we multiply in Kronecker terms
    inherited::m_default_value = 1.0;
}


//#####  evaluate  ############################################################

real SquaredExponentialARDKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    PLASSERT( x1.size() == x2.size() );
    PLASSERT( !m_isp_input_sigma.size() || x1.size() == m_isp_input_sigma.size() );

    real gating_term = inherited::evaluate(x1,x2);
    if (fast_is_equal(gating_term, 0.0))
        return 0.0;
    
    if (x1.size() == 0)
        return softplus(m_isp_signal_sigma) * gating_term;
    
    const real* px1 = x1.data();
    const real* px2 = x2.data();
    real sf         = softplus(m_isp_signal_sigma);
    real expval     = 0.0;
    
    if (m_isp_input_sigma.size() > 0) {
        const real* pinpsig = m_isp_input_sigma.data();
        for (int i=0, n=x1.size() ; i<n ; ++i) {
            real diff   = *px1++ - *px2++;
            real sqdiff = diff * diff;
            expval     += sqdiff / softplus(m_isp_global_sigma + *pinpsig++);
        }
    }
    else {
        real global_sigma = softplus(m_isp_global_sigma);
        for (int i=0, n=x1.size() ; i<n ; ++i) {
            real diff   = *px1++ - *px2++;
            real sqdiff = diff * diff;
            expval     += sqdiff / global_sigma;
        }
    }

    // Gate by Kronecker term
    return sf * exp(-0.5 * expval) * gating_term;
}


//#####  computeGramMatrix  ###################################################

void SquaredExponentialARDKernel::computeGramMatrix(Mat K) const
{
    PLASSERT( !m_isp_input_sigma.size() || dataInputsize() == m_isp_input_sigma.size() );
    PLASSERT( K.size() == 0 || m_data_cache.size() > 0 );  // Ensure data cached OK

    // Compute Kronecker gram matrix
    inherited::computeGramMatrix(K);

    // Precompute some terms. Make sure that the input sigmas don't get too
    // small
    real sf    = softplus(m_isp_signal_sigma);
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
            //         sum_wt += (diff * diff) / *p_inpsigma++;
            //     }
            real diff;
            switch (k % 8) {
            case 0: do { diff = *x1++ - *x2++; sum_wt += (diff*diff) / *p_inpsigma++;
            case 7:      diff = *x1++ - *x2++; sum_wt += (diff*diff) / *p_inpsigma++;
            case 6:      diff = *x1++ - *x2++; sum_wt += (diff*diff) / *p_inpsigma++;
            case 5:      diff = *x1++ - *x2++; sum_wt += (diff*diff) / *p_inpsigma++;
            case 4:      diff = *x1++ - *x2++; sum_wt += (diff*diff) / *p_inpsigma++;
            case 3:      diff = *x1++ - *x2++; sum_wt += (diff*diff) / *p_inpsigma++;
            case 2:      diff = *x1++ - *x2++; sum_wt += (diff*diff) / *p_inpsigma++;
            case 1:      diff = *x1++ - *x2++; sum_wt += (diff*diff) / *p_inpsigma++;
                       } while((k -= 8) > 0);
            }

            // Multiplicatively update kernel matrix (already pre-filled with
            // Kronecker terms, or 1.0 if no Kronecker terms, as per build_).
            real Kij_cur = *Kij * sf * exp(-0.5 * sum_wt);
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

void SquaredExponentialARDKernel::computeGramMatrixDerivative(
    Mat& KD, const string& kernel_param, real epsilon) const
{
    static const string ISS("isp_signal_sigma");
    static const string IGS("isp_global_sigma");
    static const string IIS("isp_input_sigma[");

    if (kernel_param == ISS) {
        computeGramMatrixDerivIspSignalSigma(KD);
        
        // computeGramMatrixDerivNV<
        //     SquaredExponentialARDKernel,
        //     &SquaredExponentialARDKernel::derivIspSignalSigma>(KD, this, -1);
    }
    else if (kernel_param == IGS) {
        computeGramMatrixDerivNV<
            SquaredExponentialARDKernel,
            &SquaredExponentialARDKernel::derivIspGlobalSigma>(KD, this, -1);
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


//#####  derivIspSignalSigma  #################################################

real SquaredExponentialARDKernel::derivIspSignalSigma(int i, int j, int arg, real K) const
{
    // (No longer used; see computeGramMatrixDerivIspInputSigma below)
    return K*sigmoid(m_isp_signal_sigma)/softplus(m_isp_signal_sigma);
}


//#####  derivIspGlobalSigma  #################################################

real SquaredExponentialARDKernel::derivIspGlobalSigma(int i, int j, int arg, real K) const
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

void SquaredExponentialARDKernel::computeGramMatrixDerivIspSignalSigma(Mat& KD) const
{
    int l = data->length();
    KD.resize(l,l);
    PLASSERT_MSG(
        gram_matrix.width() == l && gram_matrix.length() == l,
        "To compute the derivative with respect to 'isp_signal_sigma', the\n"
        "Gram matrix must be precomputed and cached in SquaredExponentialARDKernel.");
    
    KD << gram_matrix;
    KD *= sigmoid(m_isp_signal_sigma)/softplus(m_isp_signal_sigma);
}


//#####  computeGramMatrixDerivIspInputSigma  #################################

void SquaredExponentialARDKernel::computeGramMatrixDerivIspInputSigma(Mat& KD,
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
        "Gram matrix must be precomputed and cached in SquaredExponentialARDKernel.");

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

void SquaredExponentialARDKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
