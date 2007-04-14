// -*- C++ -*-

// KroneckerBaseKernel.h
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

/*! \file KroneckerBaseKernel.h */


#ifndef KroneckerBaseKernel_INC
#define KroneckerBaseKernel_INC

#include <plearn/ker/MemoryCachedKernel.h>

namespace PLearn {

/**
 *  Base class for kernels that make use of Kronecker terms
 *
 *  This kernel allows the specification of product a of Kronecker delta terms
 *  when there is a match of VALUE in ONE DIMENSION.  (This may be generalized
 *  in the future to allow match according to a subset of the input variables,
 *  but is not currently done for performance reasons).  With these terms, the
 *  kernel function takes the form:
 *
 *    k(x,y) = \product_i delta_x[kr(i)],y[kr(i)]
 *
 *  where kr(i) is the i-th element of 'kronecker_indexes' (representing an
 *  index into the input vectors).  Derived classes can either integrate these
 *  terms additively (e.g. KroneckerBaseKernel) or multiplicatively
 *  (e.g. ARDBaseKernel and derived classes).  Note that this class does not
 *  provide any hyperparameter associated with this product; an hyperparameter
 *  may be included by derived classes as required.  (Currently, only
 *  IIDNoiseKernel needs one; in other kernels, this is absorbed by the global
 *  function noise hyperparameter).
 *
 *  The basic idea for Kronecker terms is to selectively build in parts of a
 *  covariance function based on matches in the value of some input variables.
 *  They are useful in conjunction with a "covariance function builder" such as
 *  SummationKernel.
 */
class KroneckerBaseKernel : public MemoryCachedKernel
{
    typedef MemoryCachedKernel inherited;

public:
    //#####  Public Build Options  ############################################

    //! Element index in the input vectors that should be subject to additional
    //! Kronecker delta terms
    TVec<int> m_kronecker_indexes;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    KroneckerBaseKernel();


    //#####  Kernel Member Functions  #########################################

    //! Compute K(x1,x2).
    virtual real evaluate(const Vec& x1, const Vec& x2) const;

    //! Compute the Gram Matrix.  Note that this version DOES NOT CACHE
    //! the results, since it is usually called by derived classes.
    virtual void computeGramMatrix(Mat K) const;
    

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(KroneckerBaseKernel);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    /**
     *  Value to be used for kernel evaluation if there are no kronecker terms.
     *  This is initialized to zero.  A derived class may set it to (e.g.) 1.0
     *  be be sure that the default value is filled to something that can be
     *  used multiplicatively, even when there are no Kronecker terms.
     */
    mutable real m_default_value;
    
protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    /**
     *  Utility function for derived classes: return the softplus of its
     *  argument, but if the softplus would fall below the given floor, then
     *  return the floor AND MODIFY the original argument to represent the
     *  inverse softplus of the floor.  This allows preventing some variables
     *  from getting too small during optimization.
     */
    static real softplusFloor(real& value, real floor=1e-6);

private:
    //! This does the actual building.
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(KroneckerBaseKernel);

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
