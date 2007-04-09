// -*- C++ -*-

// RationalQuadraticARDKernel.cc
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
    "  k(x,y) = sf * [1 + (sum_i (x_i - y_i)^2 / w_i)/(2*alpha)]^(-alpha) + k_iid(x,y)\n"
    "\n"
    "where sf is softplus(isp_signal_sigma), w_i is softplus(isp_global_sigma +\n"
    "isp_input_sigma[i]), and k_iid(x,y) is the result of IIDNoiseKernel kernel\n"
    "evaluation.\n"
    "\n"
    "Note that to make its operations more robust when used with unconstrained\n"
    "optimization of hyperparameters, all hyperparameters of this kernel are\n"
    "specified in the inverse softplus domain.  See IIDNoiseKernel for more\n"
    "explanations.\n"
    );


RationalQuadraticARDKernel::RationalQuadraticARDKernel()
    : m_isp_alpha(0.0)
{ }


//#####  declareOptions  ######################################################

void RationalQuadraticARDKernel::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "isp_alpha",
        &RationalQuadraticARDKernel::m_isp_alpha,
        OptionBase::buildoption,
        "Inverse softplus of the alpha parameter in the rational-quadratic kernel.\n"
        "Default value=0.0");

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


//#####  makeDeepCopyFromShallowCopy  #########################################

void RationalQuadraticARDKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_noise_gram_cache,        copies);
    deepCopyField(m_pow_minus_alpha_minus_1, copies);
}


//#####  evaluate  ############################################################

real RationalQuadraticARDKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    PLASSERT( x1.size() == x2.size() );
    PLASSERT( !m_isp_input_sigma.size() || x1.size() == m_isp_input_sigma.size() );

    if (x1.size() == 0)
        return softplus(m_isp_signal_sigma) + inherited::evaluate(x1,x2);
    
    const real* px1 = x1.data();
    const real* px2 = x2.data();
    real sf         = softplus(m_isp_signal_sigma);
    real alpha      = softplus(m_isp_alpha);
    real sum_wt     = 0.0;
    real sum_sqdiff = 0.0;
    
    if (m_isp_input_sigma.size() > 0) {
        const real* pinpsig = m_isp_input_sigma.data();
        for (int i=0, n=x1.size() ; i<n ; ++i) {
            real diff   = *px1++ - *px2++;
            real sqdiff = diff * diff;
            sum_sqdiff += sqdiff;
            sum_wt     += sqdiff / softplus(m_isp_global_sigma + *pinpsig++);
        }
    }
    else {
        real global_sigma = softplus(m_isp_global_sigma);
        for (int i=0, n=x1.size() ; i<n ; ++i) {
            real diff   = *px1++ - *px2++;
            real sqdiff = diff * diff;
            sum_sqdiff += sqdiff;
            sum_wt     += sqdiff / global_sigma;
        }
    }

    // We add the noise covariance as well
    real noise_cov = inherited::evaluate(x1,x2);
    return sf * pow(1 + sum_wt / (real(2.)*alpha), -alpha) + noise_cov;
}


//#####  computeGramMatrix  ###################################################

void RationalQuadraticARDKernel::computeGramMatrix(Mat K) const
{
    PLASSERT( !m_isp_input_sigma.size() || dataInputsize() == m_isp_input_sigma.size() );
    PLASSERT( K.size() == 0 || m_data_cache.size() > 0 );  // Ensure data cached OK

    // Compute IID noise gram matrix and save it
    inherited::computeGramMatrix(K);
    m_noise_gram_cache.resize(K.length(), K.width());
    m_noise_gram_cache << K;

    // Precompute some terms
    real sf    = softplus(m_isp_signal_sigma);
    real alpha = softplus(m_isp_alpha);
    m_input_sigma.resize(dataInputsize());
    m_input_sigma.fill(m_isp_global_sigma);
    if (m_isp_input_sigma.size() > 0)
        m_input_sigma += m_isp_input_sigma;
    for (int i=0, n=m_input_sigma.size() ; i<n ; ++i)
        m_input_sigma[i] = softplus(m_input_sigma[i]);

    // Prepare the cache for the pow terms
    m_pow_minus_alpha_minus_1.resize(K.length(), K.width());
    int   pow_cache_mod = m_pow_minus_alpha_minus_1.mod();
    real* pow_cache_row = m_pow_minus_alpha_minus_1.data();
    
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
    
    for (int i=0 ; i<l
             ; ++i, xi += cache_mod, pow_cache_row+=pow_cache_mod, Ki+=m)
    {
        Kij = Ki;
        real *xj = data_start;
        real *pow_cache_cur = pow_cache_row;

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

            real inner_pow   = 1 + sum_wt / (2.*alpha);
            real pow_alpha   = pow(inner_pow, -alpha);
            real Kij_cur     = sf * pow_alpha;
            *pow_cache_cur++ = Kij_cur / inner_pow;
            
            // Update kernel matrix (already pre-filled with IID noise terms)
            *Kij++ += Kij_cur;
        }
    }
    if (cache_gram_matrix) {
        gram_matrix.resize(l,l);
        gram_matrix << K;
        gram_matrix_is_cached = true;
    }
}


//#####  computeGramMatrixDerivative  #########################################

void RationalQuadraticARDKernel::computeGramMatrixDerivative(
    Mat& KD, const string& kernel_param, real epsilon) const
{
    static const string ISS("isp_signal_sigma");
    static const string IGS("isp_global_sigma");
    static const string IIS("isp_input_sigma[");
    static const string IAL("isp_alpha");

    if (kernel_param == ISS) {
        computeGramMatrixDerivNV<
            RationalQuadraticARDKernel,
            &RationalQuadraticARDKernel::derivIspSignalSigma>(KD, this, -1);
    }
    else if (kernel_param == IGS) {
        computeGramMatrixDerivNV<
            RationalQuadraticARDKernel,
            &RationalQuadraticARDKernel::derivIspGlobalSigma>(KD, this, -1);
    }
    else if (string_begins_with(kernel_param, IIS) &&
             kernel_param[kernel_param.size()-1] == ']')
    {
        int arg = tolong(kernel_param.substr(
                             IIS.size(), kernel_param.size() - IIS.size() - 1));
        PLASSERT( arg < m_isp_input_sigma.size() );

        computeGramMatrixDerivIspInputSigma(KD, arg);

        // computeGramMatrixDerivNV<
        //     RationalQuadraticARDKernel,
        //     &RationalQuadraticARDKernel::derivIspInputSigma>(KD, this, arg);
    }
    else if (kernel_param == IAL) {
        computeGramMatrixDerivIspAlpha(KD);

        // computeGramMatrixDerivNV<
        //     RationalQuadraticARDKernel,
        //     &RationalQuadraticARDKernel::derivIspAlpha>(KD, this, -1);
    }
    else
        inherited::computeGramMatrixDerivative(KD, kernel_param, epsilon);

    // Compare against finite differences
    // Mat KD1;
    // Kernel::computeGramMatrixDerivative(KD1, kernel_param, epsilon);
    // cerr << "Kernel hyperparameter: " << kernel_param << endl;
    // cerr << "Analytic derivative (200th row):" << endl
    //      << KD(200) << endl
    //      << "Finite differences:" << endl
    //      << KD1(200) << endl;
}


//#####  derivIspSignalSigma  #################################################

real RationalQuadraticARDKernel::derivIspSignalSigma(int i, int j, int arg, real K) const
{
    real noise = m_noise_gram_cache(i,j);
    return (K-noise)*sigmoid(m_isp_signal_sigma)/softplus(m_isp_signal_sigma);
}


//#####  derivIspGlobalSigma  #################################################

real RationalQuadraticARDKernel::derivIspGlobalSigma(int i, int j, int arg, real K) const
{
    // The rational quadratic gives us:
    //     K = s*k^(-alpha).
    // Rederive the value of k == (K/s)^(-1/alpha)
    real alpha = softplus(m_isp_alpha);
    real noise = m_noise_gram_cache(i,j);
    K -= noise;
    real k     = pow(K / softplus(m_isp_signal_sigma), real(-1.) / alpha);
    real inner = (k - 1) * alpha * sigmoid(m_isp_global_sigma) / softplus(m_isp_global_sigma);
    return (K / k) * inner;
}


//#####  derivIspInputSigma  ##################################################

// This function computes the derivative element-wise.  The function actually
// used now is computeGramMatrixDerivIspInputSigma, which computes the whole
// matrix much faster.
real RationalQuadraticARDKernel::derivIspInputSigma(int i, int j, int arg, real K) const
{
    // The rational quadratic gives us:
    //     K = s*k^(-alpha).
    // Rederive the value of k == (K/s)^(-1/alpha)
    real alpha   = softplus(m_isp_alpha);
    Vec& row_i   = *dataRow(i);
    Vec& row_j   = *dataRow(j);
    real noise   = m_noise_gram_cache(i,j);
    K -= noise;
    real k       = pow(K / softplus(m_isp_signal_sigma), real(-1.) / alpha);
    real diff    = row_i[arg] - row_j[arg];
    real sq_diff = diff * diff;
    real inner   = m_isp_global_sigma + m_isp_input_sigma[arg];
    real sig_inn = sigmoid(inner);
    real spl_inn = softplus(inner);
    return 0.5 * (K / k) * sig_inn * sq_diff / (spl_inn * spl_inn);
}


//#####  derivIspAlpha  #######################################################

real RationalQuadraticARDKernel::derivIspAlpha(int i, int j, int arg, real K) const
{
    real alpha = softplus(m_isp_alpha);
    real noise = m_noise_gram_cache(i,j);
    K         -= noise;
    real k     = pow(K / softplus(m_isp_signal_sigma), real(-1.) / alpha);
    return sigmoid(m_isp_alpha) * K * (1 - pl_log(k) - 1 / k);
}


//#####  computeGramMatrixDerivIspInputSigma  #################################

void RationalQuadraticARDKernel::computeGramMatrixDerivIspInputSigma(Mat& KD,
                                                                     int arg) const
{
    // Precompute some terms
    real input_sigma_arg = m_input_sigma[arg];
    real input_sigma_sq  = input_sigma_arg * input_sigma_arg;
    real input_sigmoid   = sigmoid(m_isp_global_sigma + m_isp_input_sigma[arg]);
    
    // Compute Gram Matrix derivative w.r.t. isp_input_sigma[arg]
    int  l = data->length();

    // Variables that walk over the data matrix
    int  cache_mod = m_data_cache.mod();
    real *data_start = &m_data_cache(0,0);
    real *xi = data_start+arg;               // Iterator on data rows

    // Variables that walk over the pow cache
    int   pow_cache_mod = m_pow_minus_alpha_minus_1.mod();
    real *pow_cache_row = m_pow_minus_alpha_minus_1.data();
    real *pow_cache_cur;
    
    // Variables that walk over the kernel derivative matrix (KD)
    KD.resize(l,l);
    real* KDi = KD.data();                   // Start of row i
    real* KDij;                              // Current element on row i
    int   KD_mod = KD.mod();

    // Iterate on rows of derivative matrix
    for (int i=0 ; i<l ; ++i, xi += cache_mod, KDi += KD_mod,
             pow_cache_row += pow_cache_mod)
    {
        KDij = KDi;
        real *xj  = data_start+arg;           // Inner iterator on data rows
        pow_cache_cur = pow_cache_row;

        // Iterate on columns of derivative matrix
        for (int j=0 ; j <= i
                 ; ++j, xj += cache_mod, ++pow_cache_cur)
        {
            real diff    = *xi - *xj;
            real sq_diff = diff * diff;
            real KD_cur  = 0.5 * *pow_cache_cur *
                           input_sigmoid * sq_diff / input_sigma_sq;

            // Set into derivative matrix
            *KDij++ = KD_cur;
        }
    }
}


//#####  computeGramMatrixDerivIspAlpha  ######################################

void RationalQuadraticARDKernel::computeGramMatrixDerivIspAlpha(Mat& KD) const
{
    // Precompute some terms
    real alpha_sigmoid = sigmoid(m_isp_alpha);
    
    // Compute Gram Matrix derivative w.r.t. isp_alpha
    int  l     = data->length();
    int  k_mod = gram_matrix.mod();

    // Variables that walk over the pre-computed kernel matrix (K) 
    real *Ki = &gram_matrix(0,0);            // Current row of kernel matrix
    real *Kij;                               // Current element of kernel matrix

    // Variables that walk over the pow cache
    int   pow_cache_mod = m_pow_minus_alpha_minus_1.mod();
    real *pow_cache_row = m_pow_minus_alpha_minus_1.data();
    real *pow_cache_cur;

    // Variables that walk over the noise cache
    int   noise_cache_mod = m_noise_gram_cache.mod();
    real *noise_cache_row = m_noise_gram_cache[0];
    real *noise_cache_cur;
    
    // Variables that walk over the kernel derivative matrix (KD)
    KD.resize(l,l);
    real* KDi = KD.data();                   // Start of row i
    real* KDij;                              // Current element on row i
    int   KD_mod = KD.mod();

    // Iterate on rows of derivative matrix
    for (int i=0 ; i<l ; ++i, Ki += k_mod,
             KDi += KD_mod, pow_cache_row += pow_cache_mod,
             noise_cache_row += noise_cache_mod)
    {
        Kij  = Ki;
        KDij = KDi;
        pow_cache_cur   = pow_cache_row;
        noise_cache_cur = noise_cache_row;

        // Iterate on columns of derivative matrix
        for (int j=0 ; j <= i
                 ; ++j, ++Kij, ++noise_cache_cur, ++pow_cache_cur)
        {
            real K      = *Kij - *noise_cache_cur;
            real k      = K / *pow_cache_cur;
            real KD_cur = alpha_sigmoid * K * (1 - pl_log(k) - 1/k);
            
            // Set into derivative matrix
            *KDij++ = KD_cur;
        }
    }
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
