// -*- C++ -*-

// IIDNoiseKernel.h
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

/*! \file IIDNoiseKernel.h */


#ifndef IIDNoiseKernel_INC
#define IIDNoiseKernel_INC

#include <plearn/ker/KroneckerBaseKernel.h>

namespace PLearn {

/**
 *  Kernel representing independent and identically-distributed observation noise
 *
 *  This Kernel is typically used as a base class for covariance functions used
 *  in gaussian processes (see GaussianProcessRegressor).  It represents simple
 *  i.i.d. additive noise that applies to 'identical training cases' i and j:
 *
 *    k(D_i,D_j) = delta_i,j * sn
 *
 *  where D_i and D_j are elements from the current data set (established by
 *  the setDataForKernelMatrix function), delta_i,j is the Kronecker delta
 *  function, and sn is softplus(isp_noise_sigma), with softplus(x) =
 *  log(1+exp(x)).  Note that 'identity' is not equivalent to 'vector
 *  equality': in particular, at test-time, this noise is NEVER added.
 *  Currently, two vectors are considered identical if and only if they are the
 *  SAME ROW of the current data set, and hence the noise term is added only at
 *  TRAIN-TIME across the diagonal of the Gram matrix (when the
 *  computeGramMatrix() function is called).  This is why at test-time, no such
 *  noise term is added.  The idea (see the book "Gaussian Processes for
 *  Machine Learning" by Rasmussen and Williams for details) is that
 *  observation noise only applies when A SPECIFIC OBSERVATION is drawn from
 *  the GP distribution: if we sample a new point at the same x, we will get a
 *  different realization for the noise, and hence the correlation between the
 *  two noise realizations is zero.  This class can only be sure that two
 *  observations are "identical" when they are presented all at once through
 *  the data matrix.
 *
 *  The Kronecker terms computed by the base class are ADDDED to the noise 
 *  computed by this kernel (at test-time also).
 */
class IIDNoiseKernel : public KroneckerBaseKernel
{
    typedef KroneckerBaseKernel inherited;

public:
    //#####  Public Build Options  ############################################

    //! Inverse softplus of the global noise variance.  Default value=-100.0
    //! (very close to zero after we take softplus).
    real m_isp_noise_sigma;
    
    //! Inverse softplus of the noise variance term for the product of
    //! Kronecker deltas associated with kronecker_indexes, if specified.
    real m_isp_kronecker_sigma;
    
public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    IIDNoiseKernel();


    //#####  Kernel Member Functions  #########################################

    //! Compute K(x1,x2).  This DOES include noise if x1 == x2.
    virtual real evaluate(const Vec& x1, const Vec& x2) const;

    //! Always zero by independence
    virtual real evaluate_i_x(int i, const Vec& x, real) const;
    
    //! Compute the Gram Matrix.  Note that this version DOES NOT CACHE
    //! the results, since it is usually called by derived classes.
    virtual void computeGramMatrix(Mat K) const;
    
    //! Directly compute the derivative with respect to hyperparameters
    //! (Faster than finite differences...)
    virtual void computeGramMatrixDerivative(Mat& KD, const string& kernel_param,
                                             real epsilon=1e-6) const;

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(IIDNoiseKernel);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Compute the derivative of the Gram matrix with respect to the Kronecker
    //! sigma
    void computeGramMatrixDerivKronecker(Mat& KD) const;
    
private:
    //! This does the actual building.
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(IIDNoiseKernel);

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
