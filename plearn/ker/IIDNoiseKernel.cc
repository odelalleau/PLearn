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

#include <plearn/base/lexical_cast.h>
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
    "In addition to comparing the complete x and y vectors, this kernel allows\n"
    "adding a Kronecker delta when there is a match in only ONE DIMENSION.  This\n"
    "may be generalized in the future to allow match according to a subset of\n"
    "the input variables (but is not currently done for performance reasons).\n"
    "With these terms, the kernel function takes the form:\n"
    "\n"
    "  k(x,y) = delta_x,y * sn2 + \\sum_i delta_x[kr(i)],y[kr(i)] * ks2[i]\n"
    "\n"
    "where kr(i) is the i-th element of 'kronecker_indexes' (representing an\n"
    "index into the input vectors), and ks2[i] is the exp of twice the value of\n"
    "the i-th element of the 'log_kronecker_sigma' option.\n"
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
        ol, "log_noise_sigma", &IIDNoiseKernel::m_log_noise_sigma,
        OptionBase::buildoption,
        "Log of the global noise variance.  Default value=0.0");

    declareOption(
        ol, "kronecker_indexes", &IIDNoiseKernel::m_kronecker_indexes,
        OptionBase::buildoption,
        "Element index in the input vectors that should be subject to additional\n"
        "Kronecker delta terms");

    declareOption(
        ol, "log_kronecker_sigma", &IIDNoiseKernel::m_log_kronecker_sigma,
        OptionBase::buildoption,
        "Log of the noise variance terms for the Kronecker deltas associated\n"
        "with kronecker_indexes");
    
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
{
    if (m_kronecker_indexes.size() != m_log_kronecker_sigma.size())
        PLERROR("IIDNoiseKernel::build_: size of 'kronecker_indexes' (%d) "
                "does not match that of 'log_kronecker_sigma' (%d)",
                m_kronecker_indexes.size(), m_log_kronecker_sigma.size());
}


//#####  evaluate  ############################################################

real IIDNoiseKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    // if (fast_is_equal(powdistance(x1,x2,2), 0.0))
    //     return exp(2*m_log_noise_sigma);
    // else
    //     return 0.0;

    real value = 0.0;
    if (x1 == x2)
        value += exp(2*m_log_noise_sigma);

    const int n = m_kronecker_indexes.size();
    if (n > 0) {
        int*  cur_index = m_kronecker_indexes.data();
        real* cur_sigma = m_log_kronecker_sigma.data();

        for (int i=0 ; i<n ; ++i, ++cur_index, ++cur_sigma)
            if (fast_is_equal(x1[*cur_index], x2[*cur_index]))
                value += exp(2 * *cur_sigma);
    }
    return value;
}


//#####  computeGramMatrix  ###################################################

void IIDNoiseKernel::computeGramMatrix(Mat K) const
{
    if (!data)
        PLERROR("Kernel::computeGramMatrix: setDataForKernelMatrix not yet called");
    if (!is_symmetric)
        PLERROR("Kernel::computeGramMatrix: not supported for non-symmetric kernels");
    if (K.length() != data.length() || K.width() != data.length())
        PLERROR("Kernel::computeGramMatrix: the argument matrix K should be\n"
                "of size %d x %d (currently of size %d x %d)",
                data.length(), data.length(), K.length(), K.width());
                
    PLASSERT( K.size() == 0 || m_data_cache.size() > 0 );  // Ensure data cached OK

    // Precompute some terms
    real noise_sigma  = exp(2 * m_log_noise_sigma);
    m_kronecker_sigma.resize(m_log_kronecker_sigma.size());
    m_kronecker_sigma << m_log_kronecker_sigma;
    m_kronecker_sigma *= 2.0;
    exp(m_kronecker_sigma, m_kronecker_sigma);

    // Prepare kronecker iteration
    int   kronecker_num     = m_kronecker_indexes.size();
    int*  kronecker_indexes = ( kronecker_num > 0?
                                m_kronecker_indexes.data() : 0 );
    real* kronecker_sigma   = ( kronecker_num > 0?
                                m_kronecker_sigma.data() : 0 );

    // Compute Gram Matrix
    int  l = data->length();
    int  m = K.mod();
    int  cache_mod = m_data_cache.mod();

    real *data_start = &m_data_cache(0,0);
    real Kij;
    real *Ki, *Kji;
    real *xi = data_start;
    
    for (int i=0 ; i<l ; ++i, xi += cache_mod) {
        Ki  = K[i];
        Kji = &K[0][i];
        real *xj = data_start;

        for (int j=0; j<=i; ++j, Kji += m, xj += cache_mod) {
            // Kernel evaluation per se
            if (i == j)
                Kij = noise_sigma;
            else
                Kij = 0.0;

            // Kronecker terms
            if (kronecker_num > 0) {
                int*  cur_index = kronecker_indexes;
                real* cur_sigma = kronecker_sigma;
                
                for (int k=0 ; k<kronecker_num ; ++k, ++cur_index, ++cur_sigma)
                    if (fast_is_equal(xi[*cur_index], xj[*cur_index]))
                        Kij += *cur_sigma;
            }
            
            // Fill upper triangle if not on diagonal
            *Ki++ = Kij;
            // if (j < i)
            //     *Kji = Kij;
        }
    }
}


//#####  makeDeepCopyFromShallowCopy  #########################################

void IIDNoiseKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_kronecker_indexes,   copies);
    deepCopyField(m_log_kronecker_sigma, copies);
    deepCopyField(m_kronecker_sigma,     copies);
}


//#####  computeGramMatrixDerivative  #########################################

void IIDNoiseKernel::computeGramMatrixDerivative(Mat& KD, const string& kernel_param,
                                                 real epsilon) const
{
    static const string LNS("log_noise_sigma");
    static const string LKS("log_kronecker_sigma[");
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
    else if (string_begins_with(kernel_param, LKS) &&
             kernel_param[kernel_param.size()-1] == ']')
    {
        int arg = tolong(kernel_param.substr(
                             LKS.size(), kernel_param.size() - LKS.size() - 1));
        PLASSERT( arg < m_kronecker_indexes.size() );

        computeGramMatrixDerivKronecker(KD, arg);
        
        // computeGramMatrixDerivNV<
        //     IIDNoiseKernel, &IIDNoiseKernel::derivKronecker>(KD, this, arg);
        
        // int W = nExamples();
        // KD.resize(W,W);
        // real deriv = 2*exp(2*m_log_kronecker_sigma[arg]);
        // int index  = m_kronecker_indexes[arg];
        // 
        // Vec row_i;
        // Vec row_j;
        // int m = KD.mod();
        // real* KDi;                           // Start of row i
        // real* KDji;                          // Start of column i
        // for (int i=0 ; i<W ; ++i) {
        //     KDi = KD[i];
        //     KDji = &KD[0][i];
        //     dataRow(i, row_i);
        //     real row_i_index = row_i[index];
        //     for (int j=0 ; j<=i ; ++j, KDji += m) {
        //         dataRow(j, row_j);
        //         real KDij;
        //         if (fast_is_equal(row_i_index, row_j[index]))
        //             KDij = deriv;
        //         else
        //             KDij = 0.0;
        // 
        //         *KDi++ = KDij;
        //         if (j < i)
        //             *KDji = KDij;
        //     }
        // }
    }
    else
        inherited::computeGramMatrixDerivative(KD, kernel_param, epsilon);
}


//#####  derivKronecker  ######################################################

real IIDNoiseKernel::derivKronecker(int i, int j, int arg, real K) const
{
    int index  = m_kronecker_indexes[arg];
    Vec& row_i = *dataRow(i);
    Vec& row_j = *dataRow(j);
    if (fast_is_equal(row_i[index], row_j[index]))
        return 2*exp(2*m_log_kronecker_sigma[arg]);
    else
        return 0.0;
}


//#####  computeGramMatrixDerivKronecker  #####################################

void IIDNoiseKernel::computeGramMatrixDerivKronecker(Mat& KD, int arg) const
{
    // Precompute some terms
    real kronecker_sigma_arg = 2. * exp(2. * m_log_kronecker_sigma[arg]);
    int index = m_kronecker_indexes[arg];
    
    // Compute Gram Matrix derivative w.r.t. log_kronecker_sigma[arg]
    int  l = data->length();

    // Variables that walk over the data matrix
    int  cache_mod = m_data_cache.mod();
    real *data_start = &m_data_cache(0,0);
    real *xi = data_start+index;             // Iterator on data rows

    // Variables that walk over the kernel derivative matrix (KD)
    KD.resize(l,l);
    real* KDi = KD.data();                   // Start of row i
    real* KDij;                              // Current element on row i
    int   KD_mod = KD.mod();

    // Iterate on rows of derivative matrix
    for (int i=0 ; i<l ; ++i, xi += cache_mod, KDi += KD_mod)
    {
        KDij = KDi;
        real xi_cur = *xi;
        real *xj  = data_start+index;        // Inner iterator on data rows

        // Iterate on columns of derivative matrix
        for (int j=0 ; j <= i ; ++j, xj += cache_mod)
        {
            // Set into derivative matrix
            *KDij++ = fast_is_equal(xi_cur, *xj)? kronecker_sigma_arg : 0.0;
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
