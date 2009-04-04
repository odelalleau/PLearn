// -*- C++ -*-

// Matern1ARDKernel.h
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

/*! \file Matern1ARDKernel.h */


#ifndef MATERN1ARDKERNEL_INC
#define MATERN1ARDKERNEL_INC

#include <plearn/ker/ARDBaseKernel.h>

namespace PLearn {

/**
 *  Matern kernel with nu=1/2 that can be used for Automatic Relevance
 *  Determination.
 *
 *  With nu=1/2, the Matern kernel corresponds to the Ornstein-Uhlenbeck
 *  process.  This function is specified as:
 *
 *    k(x,y) = (sf / (2*a)) * exp(-a sum_i |x_i - y_i|/w_i) * k_kron(x,y)
 *
 *  where sf = softplus(isp_signal_sigma), a = softplus(isp_persistence), w_i =
 *  softplus(isp_global_sigma + isp_input_sigma[i]), and k_kron(x,y) is the
 *  result of the KroneckerBaseKernel evaluation, or 1.0 if there are no
 *  Kronecker terms.  Note that since the Kronecker terms are incorporated
 *  multiplicatively, the very presence of the term associated to this kernel
 *  can be gated by the value of some input variable(s) (that are incorporated
 *  within one or more Kronecker terms).
 *
 *  Note that to make its operations more robust when used with unconstrained
 *  optimization of hyperparameters, all hyperparameters of this kernel are
 *  specified in the inverse softplus domain.  See IIDNoiseKernel for more
 *  explanations.
 */
class Matern1ARDKernel : public ARDBaseKernel
{
    typedef ARDBaseKernel inherited;

public:
    //#####  Public Build Options  ############################################

    /// Inverse softplus of the O-U persistence parameter.  Default value =
    /// isp(1.0).
    mutable real m_isp_persistence;
    
public:
    //#####  Public Member Functions  #########################################

    /// Default constructor
    Matern1ARDKernel();


    //#####  Kernel Member Functions  #########################################

    /// Compute K(x1,x2).
    virtual real evaluate(const Vec& x1, const Vec& x2) const;

    /// Compute the Gram Matrix.
    virtual void computeGramMatrix(Mat K) const;
    
    /// Directly compute the derivative with respect to hyperparameters
    /// (Faster than finite differences...)
    virtual void computeGramMatrixDerivative(Mat& KD, const string& kernel_param,
                                             real epsilon=1e-6) const;
    
    /// Fill k_xi_x with K(x_i, x), for all i from istart to istart + k_xi_x.length() - 1.
    virtual void evaluate_all_i_x(const Vec& x, const Vec& k_xi_x,
                                  real squared_norm_of_x=-1, int istart = 0) const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(Matern1ARDKernel);

    // Simply calls inherited::build() then build_()
    virtual void build();

    /// Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    /// Declares the class options.
    static void declareOptions(OptionList& ol);

    /// Derivative function with respect to isp_signal_sigma
    real derivIspSignalSigma(int i, int j, int arg, real K) const;

    /// Derivative function with respect to isp_global_sigma
    real derivIspGlobalSigma(int i, int j, int arg, real K) const;
    
    /// Compute derivative w.r.t. isp_persistence
    void derivIspPersistence(int i, int j, int arg, real K) const;
    
    /// Compute derivative w.r.t. isp_signal_sigma for WHOLE MATRIX
    void computeGramMatrixDerivIspSignalSigma(Mat& KD) const;
    
    /// Compute derivative w.r.t. isp_input_sigma[arg] for WHOLE MATRIX
    void computeGramMatrixDerivIspInputSigma(Mat& KD, int arg) const;

private:
    /// This does the actual building.
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(Matern1ARDKernel);

} // end of namespace PLearn

#endif


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
