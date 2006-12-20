// -*- C++ -*-

// OnlineLearningModule.h
//
// Copyright (C) 2005 Yoshua Bengio
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

/* *******************************************************
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file OnlineLearningModule.h */


#ifndef OnlineLearningModule_INC
#define OnlineLearningModule_INC

#include <plearn/base/Object.h>
#include <plearn/math/PRandom.h>

namespace PLearn {

/**
 *  Learn to map inputs to outputs, online, using caller-provided gradients.
 *
 *  This pure virtual class (i.e. an interface) can basically do two things:
 *    * map an input to an output
 *    * modify itself when told in what direction the output should have
 *      changed (i.e. output gradient),  while optionally giving back the
 *      information about how the input should also have changed
 *      (i.e. input gradient)
 *
 * @todo write all Object methods, compile, and test somehow
 *
 * @deprecated
 */
class OnlineLearningModule : public Object
{
    typedef Object inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    //! input size
    int input_size;

    //! output size
    int output_size;

    //! compute simpler estimation of diagonal of the input Hessian matrix,
    //! using only the first (positive) part in:
    //! d²C/dx² ~= d²C/dy² (dy/dx)² [+ dC/dy d²y/dx²]
    bool estimate_simpler_diag_hessian;

    /**
     * Path of the directory associated with this module, in which it should
     * save any file it wishes to create.  The directory will be created if
     * it does not already exist.  If expdir is the empty string (the
     * default), then the module should not create *any* file.
     */
    PPath expdir;

    //! optional random generator, possibly shared among several modules
    PP<PRandom> random_gen;

public:
    //#####  Public Member Fun ctions  #########################################

    //! Default constructor
    OnlineLearningModule();

    // Your other public member functions go here

    //! given the input, compute the output (possibly resize it  appropriately)
    virtual void fprop(const Vec& input, Vec& output) const = 0;

    //! Adapt based on the output gradient: this method should only
    //! be called just after a corresponding fprop; it should be
    //! called with the same arguments as fprop for the first two arguments
    //! (and output should not have been modified since then).
    //! Since sub-classes are supposed to learn ONLINE, the object
    //! is 'ready-to-be-used' just after any bpropUpdate.
    //! N.B. The DEFAULT IMPLEMENTATION JUST CALLS
    //!   bpropUpdate(input, output, input_gradient, output_gradient)
    //! AND IGNORES INPUT GRADIENT.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             const Vec& output_gradient);

    //! this version allows to obtain the input gradient as well
    //! N.B. THE DEFAULT IMPLEMENTATION JUST RAISES A PLERROR.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient, const Vec& output_gradient);

    //! Similar to bpropUpdate, but adapt based also on the estimation
    //! of the diagonal of the Hessian matrix, and propagates this
    //! back. If these methods are defined, you can use them INSTEAD of
    //! bpropUpdate(...)
    //! THE DEFAULT IMPLEMENTATION PROVIDED HERE JUST CALLS
    //!   bbpropUpdate(input, output,
    //!                input_gradient, output_gradient,
    //!                in_hess, out_hess)
    //! AND IGNORES INPUT HESSIAN AND INPUT GRADIENT
    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              const Vec& output_gradient,
                              const Vec& output_diag_hessian);

    //! this version allows to obtain the input gradient and diag_hessian
    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              Vec& input_gradient,
                              const Vec& output_gradient,
                              Vec& input_diag_hessian,
                              const Vec& output_diag_hessian);

    //! reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget() = 0;


    //! optionally perform some processing after training, or after a
    //! series of fprop/bpropUpdate calls to prepare the model for truly
    //! out-of-sample operation
    virtual void finalize() {}

    // in case bpropUpdate does not do anything, make it known
    virtual bool bpropDoesNothing() { return false; }

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_ABSTRACT_OBJECT(OnlineLearningModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void setLearningRate(real& dynamic_learning_rate);

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(OnlineLearningModule);

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
