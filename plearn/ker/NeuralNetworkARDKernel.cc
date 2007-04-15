// -*- C++ -*-

// NeuralNetworkARDKernel.cc
//
// Copyright (C) 2007 Nicolas Chapados
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

/*! \file NeuralNetworkARDKernel.cc */


#include "NeuralNetworkARDKernel.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NeuralNetworkARDKernel,
    "Neural network kernel that can be used for Automatic Relevance Determination",
    "This kernel is designed to be used within a GaussianProcessRegressor.  It\n"
    "is similar to the \"arcsin\" kernel of C.E. Rasmussen's GPML code (see\n"
    "http://www.gaussianprocess.org), but can be used with full Automatic\n"
    "Relevance Determination (ARD).  It takes the form:\n"
    "\n"
    "  k(x,y) = sf * asin(2*x*P*y / sqrt((1+2*x*P*x)*(1+2*y*P*y))) * k_kron(x,y)\n"
    "\n"
    "where sf is softplus(isp_signal_sigma), P is softplus(isp_global_sigma +\n"
    "isp_input_sigma[i])^-2 times the unit matrix, where the x and y vectors on\n"
    "the right-hand-side have an extra bias (1.0) added in front.  (Note that if\n"
    "ARD is desired, the number of elements provided for isp_input_sigma must be\n"
    "ONE MORE than the number of inputs, and the first element of the\n"
    "isp_input_sigma vector corresponds to this bias).  Also note that in\n"
    "keeping with Rasmussen and Williams, we raise these elements to the -2\n"
    "power, so these hyperparameters can be interpreted as true length-scales.\n"
    "The last factor k_kron(x,y) is the result of the KroneckerBaseKernel\n"
    "evaluation, or 1.0 if there are no Kronecker terms.  Note that since the\n"
    "Kronecker terms are incorporated multiplicatively, the very presence of the\n"
    "term associated to this kernel can be gated by the value of some input\n"
    "variable(s) (that are incorporated within one or more Kronecker terms).\n"
    "\n"
    "See SquaredExponentialARDKernel for more information about using this\n"
    "kernel within a SummationKernel in order to add IID noise to the examples.\n"
    "\n"
    "Note that to make its operations more robust when used with unconstrained\n"
    "optimization of hyperparameters, all hyperparameters of this kernel are\n"
    "specified in the inverse softplus domain.  See IIDNoiseKernel for more\n"
    "explanations.\n"
    );


NeuralNetworkARDKernel::NeuralNetworkARDKernel()
{ }


//#####  declareOptions  ######################################################

void NeuralNetworkARDKernel::declareOptions(OptionList& ol)
{
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//#####  build  ###############################################################

void NeuralNetworkARDKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}


//#####  build_  ##############################################################

void NeuralNetworkARDKernel::build_()
{
    // Ensure that we multiply in Kronecker terms
    inherited::m_default_value = 1.0;
}


//#####  evaluate  ############################################################

real NeuralNetworkARDKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    PLASSERT( x1.size() == x2.size() );
    PLASSERT( !m_isp_input_sigma.size() || x1.size()+1 == m_isp_input_sigma.size() );

    real gating_term = inherited::evaluate(x1,x2);
    if (fast_is_equal(gating_term, 0.0) || x1.size() == 0)
        return 0.0;
    
    const real* px1 = x1.data();
    const real* px2 = x2.data();
    real sf         = softplus(m_isp_signal_sigma);
    real dot_x1_x1;
    real dot_x2_x2;
    real dot_x1_x2;
    
    if (m_isp_input_sigma.size() > 0) {
        const real* pinpsig = m_isp_input_sigma.data();
        real sigma = softplus(*pinpsig++);
        sigma *= sigma;
        sigma  = 2. / sigma;

        // Handle bias
        dot_x1_x1 = dot_x2_x2 = dot_x1_x2 = sigma;
 
        for (int i=0, n=x1.size() ; i<n ; ++i, ++px1, ++px2) {
            sigma  = softplus(*pinpsig++);
            sigma *= sigma;
            sigma  = 2. / sigma;

            dot_x1_x2 += *px1 * *px2 * sigma;
            dot_x1_x1 += *px1 * *px1 * sigma;
            dot_x2_x2 += *px2 * *px2 * sigma;
        }
    }
    else {
        real global_sigma = softplus(m_isp_global_sigma);
        global_sigma *= global_sigma;
        global_sigma  = 2. / global_sigma;

        // Handle bias for x1 and x2
        dot_x1_x1 = dot_x2_x2 = dot_x1_x2 = 1;
        
        for (int i=0, n=x1.size() ; i<n ; ++i, ++px1, ++px2) {
            dot_x1_x2 += *px1 * *px2;
            dot_x1_x1 += *px1 * *px1;
            dot_x2_x2 += *px2 * *px2;
        }
        dot_x1_x2 *= global_sigma;
        dot_x1_x1 *= global_sigma;
        dot_x2_x2 *= global_sigma;
    }

    // Gate by Kronecker term
    return sf * asin(dot_x1_x2 / sqrt((1 + dot_x1_x1) * (1 + dot_x2_x2))) * gating_term;
}


//#####  computeGramMatrix  ###################################################

#define DUFF_DOTLOOP                            \
        sigma = *p_inpsigma++;                  \
        dot_x1_x2 += *x1 * *x2 * sigma;         \
        dot_x1_x1 += *x1 * *x1 * sigma;         \
        dot_x2_x2 += *x2 * *x2 * sigma;         \
        ++x1;                                   \
        ++x2;

void NeuralNetworkARDKernel::computeGramMatrix(Mat K) const
{
    PLASSERT( !m_isp_input_sigma.size() || dataInputsize()+1 == m_isp_input_sigma.size() );
    PLASSERT( K.size() == 0 || m_data_cache.size() > 0 );  // Ensure data cached OK

    // Compute Kronecker gram matrix
    inherited::computeGramMatrix(K);

    // Precompute some terms. Make sure that the input sigmas don't get too
    // small
    real sf = softplus(m_isp_signal_sigma);
    m_input_sigma.resize(dataInputsize() + 1);
    softplusFloor(m_isp_global_sigma, 1e-6);
    m_input_sigma.fill(m_isp_global_sigma);  // Still in ISP domain
    for (int i=0, n=m_input_sigma.size() ; i<n ; ++i) {
        if (m_isp_input_sigma.size() > 0) {
            softplusFloor(m_isp_input_sigma[i], 1e-6);
            m_input_sigma[i] += m_isp_input_sigma[i];
        }
        m_input_sigma[i]  = softplus(m_input_sigma[i]);
        m_input_sigma[i] *= m_input_sigma[i];
        m_input_sigma[i]  = 2. / m_input_sigma[i];
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
            int  k = n;

            // Handle the bias for x1 and x2
            real sigma     = *p_inpsigma++;
            real dot_x1_x1 = sigma;
            real dot_x2_x2 = sigma;
            real dot_x1_x2 = sigma;

            switch (k % 8) {
            case 0: do {  DUFF_DOTLOOP
            case 7:       DUFF_DOTLOOP
            case 6:       DUFF_DOTLOOP
            case 5:       DUFF_DOTLOOP
            case 4:       DUFF_DOTLOOP
            case 3:       DUFF_DOTLOOP
            case 2:       DUFF_DOTLOOP
            case 1:       DUFF_DOTLOOP  } while((k -= 8) > 0);
            }

            // Multiplicatively update kernel matrix (already pre-filled with
            // Kronecker terms, or 1.0 if no Kronecker terms, as per build_).
            real Kij_cur = *Kij * sf * asin(dot_x1_x2 / sqrt((1 + dot_x1_x1) * (1 + dot_x2_x2)));
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

void NeuralNetworkARDKernel::computeGramMatrixDerivative(
    Mat& KD, const string& kernel_param, real epsilon) const
{
    static const string ISS("isp_signal_sigma");
    static const string IGS("isp_global_sigma");
    static const string IIS("isp_input_sigma[");

    if (kernel_param == ISS) {
        computeGramMatrixDerivIspSignalSigma(KD);
    }
    // else if (kernel_param == IGS) {
    //     computeGramMatrixDerivNV<
    //         NeuralNetworkARDKernel,
    //         &NeuralNetworkARDKernel::derivIspGlobalSigma>(KD, this, -1);
    // }
    // else if (string_begins_with(kernel_param, IIS) &&
    //          kernel_param[kernel_param.size()-1] == ']')
    // {
    //     int arg = tolong(kernel_param.substr(
    //                          IIS.size(), kernel_param.size() - IIS.size() - 1));
    //     PLASSERT( arg < m_isp_input_sigma.size() );
    // 
    //     computeGramMatrixDerivIspInputSigma(KD, arg);
    // 
    // }
    else
        inherited::computeGramMatrixDerivative(KD, kernel_param, epsilon);
}


//#####  derivIspGlobalSigma  #################################################

real NeuralNetworkARDKernel::derivIspGlobalSigma(int i, int j, int arg, real K) const
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

void NeuralNetworkARDKernel::computeGramMatrixDerivIspSignalSigma(Mat& KD) const
{
    int l = data->length();
    KD.resize(l,l);
    PLASSERT_MSG(
        gram_matrix.width() == l && gram_matrix.length() == l,
        "To compute the derivative with respect to 'isp_signal_sigma', the\n"
        "Gram matrix must be precomputed and cached in NeuralNetworkARDKernel.");
    
    KD << gram_matrix;
    KD *= sigmoid(m_isp_signal_sigma)/softplus(m_isp_signal_sigma);
}


//#####  computeGramMatrixDerivIspInputSigma  #################################

void NeuralNetworkARDKernel::computeGramMatrixDerivIspInputSigma(Mat& KD,
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
        "Gram matrix must be precomputed and cached in NeuralNetworkARDKernel.");

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

void NeuralNetworkARDKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
