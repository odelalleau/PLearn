// -*- C++ -*-

// IIDNoiseKernel.h
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

/*! \file IIDNoiseKernel.h */


#ifndef IIDNoiseKernel_INC
#define IIDNoiseKernel_INC

#include <plearn/ker/MemoryCachedKernel.h>

namespace PLearn {

/**
 *  Kernel representing independent and identically-distributed observation noise
 *
 *  This Kernel is typically used as a base class for covariance functions used
 *  in gaussian processes (see GaussianProcessRegressor).  It represents simple
 *  i.i.d. additive noise:
 *
 *    k(x,y) = delta_x,y * sn2
 *
 *  where delta_x,y is the Kronecker delta function, and sn2 is the exp of
 *  twice the 'log_noise_sigma' option.
 *
 *  In addition to comparing the complete x and y vectors, this kernel allows
 *  adding a Kronecker delta when there is a match in only ONE DIMENSION.  This
 *  may be generalized in the future to allow match according to a subset of
 *  the input variables (but is not currently done for performance reasons).
 *  With these terms, the kernel function takes the form:
 *
 *    k(x,y) = delta_x,y * sn2 + \sum_i delta_x[kr(i)],y[kr(i)] * ks2[i]
 *
 *  where kr(i) is the i-th element of 'kronecker_indexes' (representing an
 *  index into the input vectors), and ks2[i] is the exp of twice the value of
 *  the i-th element of the 'log_kronecker_sigma' option.
 *
 *  Note that to make its operations more robust when used with unconstrained
 *  optimization of hyperparameters, all hyperparameters of this kernel are
 *  specified in the log-domain.
 */
class IIDNoiseKernel : public MemoryCachedKernel
{
    typedef MemoryCachedKernel inherited;

public:
    //#####  Public Build Options  ############################################

    //! Log of the global noise variance.  Default value=0.0
    real m_log_noise_sigma;

    //! Element index in the input vectors that should be subject to additional
    //! Kronecker delta terms
    TVec<int> m_kronecker_indexes;

    //! Log of the noise variance terms for the Kronecker deltas associated
    //! with kronecker_indexes
    Vec m_log_kronecker_sigma;
    
public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    IIDNoiseKernel();


    //#####  Kernel Member Functions  #########################################

    //! Compute K(x1,x2).
    virtual real evaluate(const Vec& x1, const Vec& x2) const;

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

    //! Derivative function with respect to kronecker_indexes[arg] hyperparameter
    real derivKronecker(int i, int j, int arg, real K) const;

    //! Derivative w.r.t kronecker_indexes[arg] for WHOLE MATRIX
    void computeGramMatrixDerivKronecker(Mat& KD, int arg) const;
    
protected:
    //! Buffer for exponential of m_log_kronecker_sigma
    mutable Vec m_kronecker_sigma;
    
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
