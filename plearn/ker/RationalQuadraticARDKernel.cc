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
    "  k(x,y) = sf2 * [1 + (sum_i (x_i - y_i)^2 / w_i)/(2*alpha)]^(-alpha) + k_iid(x,y)\n"
    "\n"
    "where sf2 is the exp of twice the 'log_signal_sigma' option, w_i is\n"
    "exp(2*log_global_sigma + 2*log_input_sigma[i]), and k_iid(x,y) is the\n"
    "result of IIDNoiseKernel kernel evaluation.\n"
    "\n"
    "Note that to make its operations more robust when used with unconstrained\n"
    "optimizaiton of hyperparameters, all hyperparameters of this kernel are\n"
    "specified in the log-domain.\n"
    );


RationalQuadraticARDKernel::RationalQuadraticARDKernel()
    : m_log_alpha(0.0)
{ }


//#####  declareOptions  ######################################################

void RationalQuadraticARDKernel::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "log_alpha",
        &RationalQuadraticARDKernel::m_log_alpha,
        OptionBase::buildoption,
        "Log of the alpha parameter in the rational-quadratic kernel.\n"
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

    deepCopyField(m_noise_gram_cache, copies);
}


//#####  evaluate  ############################################################

real RationalQuadraticARDKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    PLASSERT( x1.size() == x2.size() );
    PLASSERT( !m_log_input_sigma.size() || x1.size() == m_log_input_sigma.size() );

    if (x1.size() == 0)
        return exp(2*m_log_signal_sigma) + inherited::evaluate(x1,x2);
    
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

    // We add the noise covariance as well
    real noise_cov = inherited::evaluate(x1,x2);
    return sf2 * pow(1 + sum_wt / (2.*alpha), -alpha) + noise_cov;
}


//#####  computeGramMatrix  ###################################################

void RationalQuadraticARDKernel::computeGramMatrix(Mat K) const
{
    PLASSERT( !m_log_input_sigma.size() || dataInputsize() == m_log_input_sigma.size() );
    PLASSERT( K.size() == 0 || m_data_cache.size() > 0 );  // Ensure data cached OK

    // Compute IID noise gram matrix and save it
    inherited::computeGramMatrix(K);
    m_noise_gram_cache.resize(K.length(), K.width());
    m_noise_gram_cache << K;

    // Precompute some terms
    real sf2   = exp(2*m_log_signal_sigma);
    real alpha = exp(m_log_alpha);
    m_input_sigma.resize(dataInputsize());
    m_input_sigma.fill(m_log_global_sigma);
    if (m_log_input_sigma.size() > 0)
        m_input_sigma += m_log_input_sigma;
    m_input_sigma *= 2.0;
    exp(m_input_sigma, m_input_sigma);
    
    // Compute Gram Matrix
    int  l = data->length();
    int  m = K.mod();
    int  n = dataInputsize();
    int  cache_mod = m_data_cache.mod();

    real *data_start = &m_data_cache(0,0);
    real Kij;
    real *Ki, *Kji, *x1, *x2;
    real *input_sigma_data = m_input_sigma.data();
    real *xi = data_start;
    
    for (int i=0 ; i<l ; ++i, xi += cache_mod) {
        Ki  = K[i];
        Kji = &K[0][i];
        real *xj = data_start;

        for (int j=0; j<=i; ++j, Kji += m, xj += cache_mod) {
            // Kernel evaluation per se
            x1 = xi;
            x2 = xj;
            real* p_inpsigma = input_sigma_data;
            real sum_wt = 0.0;
            int k = n;

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
            
            Kij = sf2 * pow(1 + sum_wt / (2.*alpha), -alpha);
            
            // Update kernel matrix (already pre-filled with IID noise terms)
            *Ki++ += Kij;
            if (j < i)
                *Kji += Kij;
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
    static const string LSS("log_signal_sigma");
    static const string LGS("log_global_sigma");
    static const string LIS("log_input_sigma[");
    static const string LAL("log_alpha");

    if (kernel_param == LSS) {
        computeGramMatrixDerivNV<
            RationalQuadraticARDKernel,
            &RationalQuadraticARDKernel::derivLogSignalSigma>(KD, this, -1);
    }
    else if (kernel_param == LGS) {
        computeGramMatrixDerivNV<
            RationalQuadraticARDKernel,
            &RationalQuadraticARDKernel::derivLogGlobalSigma>(KD, this, -1);
    }
    else if (string_begins_with(kernel_param, LIS) &&
             kernel_param[kernel_param.size()-1] == ']')
    {
        int arg = tolong(kernel_param.substr(
                             LIS.size(), kernel_param.size() - LIS.size() - 1));
        PLASSERT( arg < m_log_input_sigma.size() );

        computeGramMatrixDerivLogInputSigma(KD, arg);

        // computeGramMatrixDerivNV<
        //     RationalQuadraticARDKernel,
        //     &RationalQuadraticARDKernel::derivLogInputSigma>(KD, this, arg);
    }
    else if (kernel_param == LAL) {
        computeGramMatrixDerivNV<
            RationalQuadraticARDKernel,
            &RationalQuadraticARDKernel::derivLogAlpha>(KD, this, -1);
    }
    else
        inherited::computeGramMatrixDerivative(KD, kernel_param, epsilon);

    // Compare against finite differences
    // Mat KD1;
    // Kernel::computeGramMatrixDerivative(KD1, kernel_param, epsilon);
    // cerr << "Kernel hyperparameter: " << kernel_param << endl;
    // cerr << "Analytic derivative (1st row):" << endl
    //      << KD(0) << endl
    //      << "Finite differences:" << endl
    //      << KD1(0) << endl;
}


//#####  derivLogSignalSigma  #################################################

real RationalQuadraticARDKernel::derivLogSignalSigma(int i, int j, int arg, real K) const
{
    real noise = m_noise_gram_cache(i,j);
    return 2*(K-noise);
}


//#####  derivLogGlobalSigma  #################################################

real RationalQuadraticARDKernel::derivLogGlobalSigma(int i, int j, int arg, real K) const
{
    // The rational quadratic gives us:
    //     K = exp(2*s)*k^(-alpha).
    // Rederive the value of k
    real alpha = exp(m_log_alpha);
    real noise = m_noise_gram_cache(i,j);
    K -= noise;
    real k     = exp(- (pl_log(K) - 2*m_log_signal_sigma) / alpha);
    real inner = - (k - 1) * alpha;
    return -0.5 * (K / k) * inner;
}


//#####  derivLogInputSigma  ##################################################

// This function computes the derivative element-wise.  The function actually
// used now is computeGramMatrixDerivLogInputSigma, which computes the whole
// matrix much faster.
real RationalQuadraticARDKernel::derivLogInputSigma(int i, int j, int arg, real K) const
{
    // The rational quadratic gives us:
    //     K = exp(2*s)*k^(-alpha).
    // Rederive the value of k
    Vec& row_i   = *dataRow(i);
    Vec& row_j   = *dataRow(j);
    real alpha   = exp(m_log_alpha);
    real noise   = m_noise_gram_cache(i,j);
    K -= noise;
    real k       = exp(- (pl_log(K) - 2*m_log_signal_sigma) / alpha);
    real diff    = row_i[arg] - row_j[arg];
    real sq_diff = diff * diff;
    return (K / k) * exp(-2 * (m_log_global_sigma + m_log_input_sigma[arg])) * sq_diff;
}


//#####  derivLogAlpha  #######################################################

real RationalQuadraticARDKernel::derivLogAlpha(int i, int j, int arg, real K) const
{
    real alpha = exp(m_log_alpha);
    real noise = m_noise_gram_cache(i,j);
    K -= noise;
    real k     = exp(- (pl_log(K) - 2*m_log_signal_sigma) / alpha);
    real left  = - alpha * pl_log(k);
    real num   = (k - 1) * 2 * alpha;
    real denum = 2 * k;
    return K * (left + num / denum);
}


//#####  computeGramMatrixDerivLogInputSigma  #################################

void RationalQuadraticARDKernel::computeGramMatrixDerivLogInputSigma(Mat& KD,
                                                                     int arg) const
{
    // Precompute some terms
    real alpha = exp(m_log_alpha);
    real twice_log_signal_sigma = 2.*m_log_signal_sigma;
    
    // Compute Gram Matrix derivative w.r.t. log_input_sigma[arg]
    int  l = data->length();
    int  k_mod     = gram_matrix.mod();
    int  cache_mod = m_data_cache.mod();

    // Variables that walk over the pre-computed kernel (K) and data matrices
    real *input_sigma_data = m_input_sigma.data();
    real *data_start = &m_data_cache(0,0);
    real *xi = data_start;                   // Iterator on data rows
    real *Ki = &gram_matrix(0,0);            // Current row of kernel matrix
    real *Kij;                               // Current element of kernel matrix

    // Variables that walk over the noise cache
    real *noise_start_row = m_noise_gram_cache.data();
    real *cur_noise;                         // Current element of noise matrix
    int  noise_mod = m_noise_gram_cache.mod();
    
    // Variables that walk over the kernel derivative matrix (KD)
    KD.resize(l,l);
    real* KDi = KD.data();                   // Start of row i
    real* KDj = KD.data();                   // Start of column j
    real* KDij;                              // Current element on row i
    real* KDji;                              // Current element on column j
    int   KD_mod = KD.mod();

    // Iterate on rows of derivative matrix
    for (int i=0 ; i<l ; ++i, xi += cache_mod, Ki += k_mod,
             KDi += KD_mod, ++KDj, noise_start_row += noise_mod)
    {
        Kij  = Ki;
        KDij = KDi;
        KDji = KDj;
        real *xj  = data_start;              // Inner iterator on data rows
        cur_noise = noise_start_row;

        // Iterate on columns of derivative matrix
        for (int j=0 ; j <= i
                 ; ++j, ++Kij, KDji+=KD_mod, xj += cache_mod, ++cur_noise)
        {
            real K       = *Kij - *cur_noise;
            real k       = exp(- (pl_log(K) - twice_log_signal_sigma) / alpha);
            real diff    = xi[arg] - xj[arg];
            real sq_diff = diff * diff;
            real KD_cur  = (K / k) * sq_diff / input_sigma_data[arg];
            
            // Set into derivative matrix
            *KDij++ = KD_cur;
            if (j < i)
                *KDji = KD_cur;
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
