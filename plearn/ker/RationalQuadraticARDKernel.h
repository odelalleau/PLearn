// -*- C++ -*-

// RationalQuadraticARDKernel.h
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

/*! \file RationalQuadraticARDKernel.h */


#ifndef RationalQuadraticARDKernel_INC
#define RationalQuadraticARDKernel_INC

#include <plearn/ker/ARDBaseKernel.h>

namespace PLearn {

/**
 *  Rational-Quadratic kernel that can be used for Automatic Relevance
 *  Determination
 *
 *  This kernel can be interpreted as an infinite mixture of
 *  SquaredExponentialARDKernel (with different characteristic length-scales),
 *  allowing a greater variety of "interesting" functions to be generated.
 *  Similar to C.E. Rasmussen's GPML code (see http://www.gaussianprocess.org),
 *  this kernel is specified as:
 *
 *    k(x,y) = sf * [1 + (sum_i (x_i - y_i)^2 / w_i)/(2*alpha)]^(-alpha) * k_kron(x,y)
 *
 *  where sf is softplus(isp_signal_sigma), w_i is softplus(isp_global_sigma +
 *  isp_input_sigma[i]), and k_kron(x,y) is the result of the
 *  KroneckerBaseKernel evaluation, or 1.0 if there are no Kronecker terms.
 *  Note that since the Kronecker terms are incorporated multiplicatively, the
 *  very presence of the term associated to this kernel can be gated by the
 *  value of some input variable(s) (that are incorporated within one or more
 *  Kronecker terms).
 *
 *  Note that contrarily to previous versions that incorporated IID noise and
 *  Kronecker terms ADDITIVELY, this version does not add any noise at all (and
 *  as explained above incorporates the Kronecker terms multiplicatively).  For
 *  best results, especially with moderately noisy data, IT IS IMPERATIVE to
 *  use whis kernel within a SummationKernel in conjunction with an
 *  IIDNoiseKernel, as follows (e.g. within a GaussianProcessRegressor):
 *
 *      kernel = SummationKernel(terms = [ RationalQuadraticARDKernel(),
 *                                         IIDNoiseKernel() ] )
 *
 *  In order to make its operations more robust when used with unconstrained
 *  optimization of hyperparameters, all hyperparameters of this kernel are
 *  specified in the inverse softplus domain.  See IIDNoiseKernel for more
 *  explanations.
 */
class RationalQuadraticARDKernel : public ARDBaseKernel
{
    typedef ARDBaseKernel inherited;

public:
    //#####  Public Build Options  ############################################

    //! Inverse softplus of the alpha parameter in the rational-quadratic kernel.
    //! Default value=0.0
    mutable real m_isp_alpha;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RationalQuadraticARDKernel();


    //#####  Kernel Member Functions  #########################################

    //! Compute K(x1,x2).
    virtual real evaluate(const Vec& x1, const Vec& x2) const;

    //! Fill k_xi_x with K(x_i, x), for all i from
    //! istart to istart + k_xi_x.length() - 1.
    virtual void evaluate_all_i_x(const Vec& x, const Vec& k_xi_x,
                                  real squared_norm_of_x=-1, int istart = 0) const;

    //! Compute entire Gram matrix
    virtual void computeGramMatrix(Mat K) const;

    //! Compute the derivative of the Gram matrix with respect to one of the
    //! kernel's parameters.  Analytic derivatives are implemented for this
    //! kernel.
    virtual void computeGramMatrixDerivative(Mat& KD, const string& kernel_param,
                                             real epsilon=1e-6) const;
    

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(RationalQuadraticARDKernel);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Derivative function with respect to isp_signal_sigma
    real derivIspSignalSigma(int i, int j, int arg, real K) const;

    //! Derivative function with respect to isp_global_sigma
    real derivIspGlobalSigma(int i, int j, int arg, real K) const;
    
    //! Derivative function with respect to isp_input_sigma[arg]
    real derivIspInputSigma(int i, int j, int arg, real K) const;
    
    //! Derivative function with respect to isp_alpha
    real derivIspAlpha(int i, int j, int arg, real K) const;

    // Compute derivative w.r.t. isp_signal_sigma for WHOLE MATRIX
    void computeGramMatrixDerivIspSignalSigma(Mat& KD) const;
    
    // Compute derivative w.r.t. isp_input_sigma[arg] for WHOLE MATRIX
    void computeGramMatrixDerivIspInputSigma(Mat& KD, int arg) const;
    
    // Compute derivative w.r.t. isp_alpha for WHOLE MATRIX
    void computeGramMatrixDerivIspAlpha(Mat& KD) const;
    
protected:
    /**
     *  Cached version of the K / k terms, namely:
     *
     *      pow(1 + sum_wt / (2*alpha), -alpha-1) * sf * kron
     *
     *  This is useful for computing derivatives,
     */
    mutable Mat m_pow_minus_alpha_minus_1;

private:
    //! This does the actual building.
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RationalQuadraticARDKernel);

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
