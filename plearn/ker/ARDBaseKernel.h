// -*- C++ -*-

// ARDBaseKernel.h
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

/*! \file ARDBaseKernel.h */


#ifndef ARDBaseKernel_INC
#define ARDBaseKernel_INC

#include <plearn/ker/KroneckerBaseKernel.h>

namespace PLearn {

/**
 *  Base class for kernels that carry out Automatic Relevance Determination (ARD)
 *
 *  The purpose of this Kernel is to introduce utility options that are
 *  generally shared by Kernels that perform Automatic Relevance Determination
 *  (ARD).  It does not introduce any specific behavior related to those
 *  options (since exactly where the ARD hyperparameters are used is very
 *  kernel-specific, this is left to derived classes).
 *
 *  Note that to make its operations more robust when used with unconstrained
 *  optimization of hyperparameters, all hyperparameters of this kernel are
 *  specified in the inverse softplus domain, hence the 'isp' prefix.  This is
 *  used in preference to the log-domain used by Rasmussen and Williams in
 *  their implementation of gaussian processes, due to numerical stability.
 *  (It may happen that the optimizer jumps 'too far' along one hyperparameter
 *  and this causes the Gram matrix to become extremely ill-conditioned.)
 */
class ARDBaseKernel : public KroneckerBaseKernel
{
    typedef KroneckerBaseKernel inherited;

public:
    //#####  Public Build Options  ############################################

    //! Inverse softplus of the global signal variance.  Default value=0.0
    real m_isp_signal_sigma;

    /**
     *  Inverse softplus of the global length-scale.  Note that if ARD is
     *  performed on input-specific sigmas, this hyperparameter should have a
     *  fixed value (and not be varied during the optimization).  Default
     *  value=0.0.
     */
    real m_isp_global_sigma;

    /**
     *  If specified, contain input-specific length-scales that can be
     *  individually optimized for (these are the ARD hyperparameters).
     */
    Vec m_isp_input_sigma;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    ARDBaseKernel();


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(ARDBaseKernel);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Buffer for final input sigma (add both global and per-input terms);
    //! Can be used by derived class.
    mutable Vec m_input_sigma;
    
private:
    //! This does the actual building.
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ARDBaseKernel);

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
