// -*- C++ -*-

// IIDNoiseKernel.cc
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
    "i.i.d. additive noise that applies to 'identical training cases' i and j:\n"
    "\n"
    "  k(D_i,D_j) = delta_i,j * sn\n"
    "\n"
    "where D_i and D_j are elements from the current data set (established by\n"
    "the setDataForKernelMatrix function), delta_i,j is the Kronecker delta\n"
    "function, and sn is softplus(isp_noise_sigma), with softplus(x) =\n"
    "log(1+exp(x)).  Note that 'identity' is not equivalent to 'vector\n"
    "equality': in particular, at test-time, this noise is NEVER added.\n"
    "Currently, two vectors are considered identical if and only if they are the\n"
    "SAME ROW of the current data set, and hence the noise term is added only at\n"
    "TRAIN-TIME across the diagonal of the Gram matrix (when the\n"
    "computeGramMatrix() function is called).  This is why at test-time, no such\n"
    "noise term is added.  The idea (see the book \"Gaussian Processes for\n"
    "Machine Learning\" by Rasmussen and Williams for details) is that\n"
    "observation noise only applies when A SPECIFIC OBSERVATION is drawn from\n"
    "the GP distribution: if we sample a new point at the same x, we will get a\n"
    "different realization for the noise, and hence the correlation between the\n"
    "two noise realizations is zero.  This class can only be sure that two\n"
    "observations are \"identical\" when they are presented all at once through\n"
    "the data matrix.\n"
    "\n"
    "The Kronecker terms computed by the base class are ADDDED to the noise\n"
    "computed by this kernel (at test-time also).\n"
    );


IIDNoiseKernel::IIDNoiseKernel()
    : m_isp_noise_sigma(-100.0), /* very close to zero... */
      m_isp_kronecker_sigma(-100.0)
{ }


//#####  declareOptions  ######################################################

void IIDNoiseKernel::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "isp_noise_sigma", &IIDNoiseKernel::m_isp_noise_sigma,
        OptionBase::buildoption,
        "Inverse softplus of the global noise variance.  Default value=-100.0\n"
        "(very close to zero after we take softplus).");

    declareOption(
        ol, "isp_kronecker_sigma", &IIDNoiseKernel::m_isp_kronecker_sigma,
        OptionBase::buildoption,
        "Inverse softplus of the noise variance term for the product of\n"
        "Kronecker deltas associated with kronecker_indexes, if specified.");
    
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
    // Assume that if x1 and x2 are identical, they are actually the same
    // instance of a data point.  This should not be called to compare a train
    // point against a test point (use evaluate_i_x for this purpose).
    return (x1 == x2? softplus(m_isp_noise_sigma) : 0.0) +
        softplus(m_isp_kronecker_sigma) * inherited::evaluate(x1,x2);
}


//#####  evaluate_i_x  ########################################################

real IIDNoiseKernel::evaluate_i_x(int i, const Vec& x, real) const
{
    // Noise component is ZERO between a test and any train example.
    // Just compute the Kronecker part is not necessarily zero
    Vec* train_row = dataRow(i);
    PLASSERT( train_row );
    return softplus(m_isp_kronecker_sigma) * inherited::evaluate(*train_row, x);
}


//#####  evaluate_all_i_x  ####################################################

void IIDNoiseKernel::evaluate_all_i_x(const Vec& x, const Vec& k_xi_x,
                                      real, int istart) const
{
    // Noise component is ZERO between a test and any train example
    k_xi_x.fill(0.0);
    real kronecker_sigma = softplus(m_isp_kronecker_sigma);
    int i_max = min(istart + k_xi_x.size(), data->length());
    int j = 0;
    for (int i=istart ; i<i_max ; ++i, ++j) {
        Vec* train_row = dataRow(i);
        k_xi_x[j] = kronecker_sigma * inherited::evaluate(*train_row, x);
    }
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
                
    // Compute Kronecker gram matrix. Multiply by kronecker sigma if there were
    // any Kronecker terms.
    inherited::computeGramMatrix(K);
    if (m_kronecker_indexes.size() > 0)
        K *= softplus(m_isp_kronecker_sigma);
    
    // Add iid noise contribution
    real noise_sigma = softplus(m_isp_noise_sigma);
    int  l   = data->length();
    int  m   = K.mod() + 1;               // Mind the +1 to go along diagonal
    real *Ki = K[0];
    
    for (int i=0 ; i<l ; ++i, Ki += m) {
        *Ki += noise_sigma;
    }

    if (cache_gram_matrix) {
        gram_matrix.resize(l,l);
        gram_matrix << K;
        gram_matrix_is_cached = true;
    }
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
    static const string INS("isp_noise_sigma");
    static const string IKS("isp_kronecker_sigma");

    if (kernel_param == INS) {
        if (!data)
            PLERROR("Kernel::computeGramMatrixDerivative should be called only after "
                    "setDataForKernelMatrix");

        int W = nExamples();
        KD.resize(W,W);
        KD.fill(0.0);
        real deriv = sigmoid(m_isp_noise_sigma);
        for (int i=0 ; i<W ; ++i)
            KD(i,i) = deriv;
    }
    else if (kernel_param == IKS) {
        computeGramMatrixDerivKronecker(KD);
    }
    else
        inherited::computeGramMatrixDerivative(KD, kernel_param, epsilon);
}


//#####  computeGramMatrixDerivKronecker  #####################################

void IIDNoiseKernel::computeGramMatrixDerivKronecker(Mat& KD) const
{
    // From the cached version of the Gram matrix, this function is easily
    // implemented: we first copy the Gram to the KD matrix, subtract the IID
    // noise contribution from the main diagonal, and multiply the remaining
    // matrix (made up of 0/1 elements) by the derivative of the kronecker
    // sigma hyperparameter.

    int l = data->length();
    KD.resize(l,l);
    PLASSERT_MSG(gram_matrix.width() == l && gram_matrix.length() == l,
                 "To compute the derivative with respect to 'isp_kronecker_sigma',\n"
                 "the Gram matrix must be precomputed and cached in IIDNoiseKernel.");
    
    KD << gram_matrix;
    real noise_sigma = softplus(m_isp_noise_sigma);
    for (int i=0 ; i<l ; ++i)
        KD(i,i) -= noise_sigma;

    KD *= sigmoid(m_isp_kronecker_sigma) / softplus(m_isp_kronecker_sigma);
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
