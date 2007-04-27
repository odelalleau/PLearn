// -*- C++ -*-

// SummationKernel.h
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

/*! \file SummationKernel.h */


#ifndef SummationKernel_INC
#define SummationKernel_INC

#include <plearn/ker/Kernel.h>

namespace PLearn {

/**
 *  Kernel computing the sum of other kernels
 *
 *  This kernel computes the summation of several subkernel objects.  It can
 *  also chop up parts of its input vector and send it to each kernel (so that
 *  each kernel can operate on a subset of the variables).
 */
class SummationKernel : public Kernel
{
    typedef Kernel inherited;

public:
    //#####  Public Build Options  ############################################

    /**
     *  Individual kernels to add to produce the final result.  The
     *  hyperparameters of kernel i can be accesed under the option names
     *  'terms[i].hyperparam' for, e.g. GaussianProcessRegressor.
     */
    TVec<Ker> m_terms;

    /**
     *  Optionally, one can specify which of individual input variables should
     *  be routed to each kernel.  The format is as a vector of vectors: for
     *  each kernel in 'terms', one must list the INDEXES in the original input
     *  vector(zero-based) that should be passed to that kernel.  If a list of
     *  indexes is empty for a given kernel, it means that the COMPLETE input
     *  vector should be passed to the kernel.
     */
    TVec< TVec<int> > m_input_indexes;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    SummationKernel();


    //#####  Kernel Member Functions  #########################################

    //! Distribute to terms (sub-kernels) in the summation, subsetting if required
    virtual void setDataForKernelMatrix(VMat the_data);

    //! Distribute to terms (sub-kernels) in the summation, subsetting if required
    virtual void addDataForKernelMatrix(const Vec& newRow);

    //! Compute K(x1,x2).
    virtual real evaluate(const Vec& x1, const Vec& x2) const;

    //! Evaluate a test example x against a train example given by its index
    virtual real evaluate_i_x(int i, const Vec& x, real) const;
    
    //! Fill k_xi_x with K(x_i, x), for all i from
    //! istart to istart + k_xi_x.length() - 1.
    virtual void evaluate_all_i_x(const Vec& x, const Vec& k_xi_x,
                                  real squared_norm_of_x=-1, int istart = 0) const;

    //! Compute the Gram Matrix by calling subkernels computeGramMatrix
    virtual void computeGramMatrix(Mat K) const;
    
    //! Directly compute the derivative with respect to hyperparameters
    //! (Faster than finite differences...)
    virtual void computeGramMatrixDerivative(Mat& KD, const string& kernel_param,
                                             real epsilon=1e-6) const;
    

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(SummationKernel);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //! Input buffers for kernel evaluation in cases where subsetting is needed
    TVec<Vec> m_input_buf1;
    TVec<Vec> m_input_buf2;

    //! Temporary buffer for kernel evaluation on all training dataset
    mutable Vec m_eval_buf;
    
    //! Temporary buffer for Gram matrix accumulation
    mutable Mat m_gram_buf;

protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //! This does the actual building.
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(SummationKernel);

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
