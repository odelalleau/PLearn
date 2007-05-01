// -*- C++ -*-

// ModuleStackModule.h
//
// Copyright (C) 2007 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file ModuleStackModule.h */


#ifndef ModuleStackModule_INC
#define ModuleStackModule_INC

#include <plearn_learners/online/OnlineLearningModule.h>

namespace PLearn {

/**
 * Wraps a stack of layered OnlineLearningModule into a single one.
 *
 * The OnlineLearningModule's are disposed like superposed layers:
 * outputs of module i are the inputs of module (i+1), the last layer is
 * the output layer.
 *
 */
class ModuleStackModule : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    //! The underlying modules
    TVec< PP<OnlineLearningModule> > modules;

    //! The number of modules
    int n_modules;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    ModuleStackModule();

    // Your other public member functions go here

    //! given the input, compute the output (possibly resize it  appropriately)
    virtual void fprop(const Vec& input, Vec& output) const;

    //! Overridden.
    virtual void fprop(const Mat& inputs, Mat& outputs);

    //! Adapt based on the output gradient, and obtain the input gradient.
    //! This method should only be called just after a corresponding
    //! fprop; it should be called with the same arguments as fprop
    //! for the first two arguments (and output should not have been
    //! modified since then).
    //! Since sub-classes are supposed to learn ONLINE, the object
    //! is 'ready-to-be-used' just after any bpropUpdate.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient,
                             const Vec& output_gradient,
                             bool accumulate=false);

    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate = false);
    
    //! This version does not obtain the input gradient.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             const Vec& output_gradient);

    //! Similar to bpropUpdate, but adapt based also on the estimation
    //! of the diagonal of the Hessian matrix, and propagates this
    //! back. If these methods are defined, you can use them INSTEAD of
    //! bpropUpdate(...)
    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              Vec& input_gradient,
                              const Vec& output_gradient,
                              Vec& input_diag_hessian,
                              const Vec& output_diag_hessian,
                              bool accumulate=false);

    //! This version does not obtain the input gradient and diag_hessian.
    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              const Vec& output_gradient,
                              const Vec& output_diag_hessian);


    //! Reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();


    //! Perform some processing after training, or after a series of
    //! fprop/bpropUpdate calls to prepare the model for truly out-of-sample
    //! operation.
    virtual void finalize();

    //! In case bpropUpdate does not do anything, make it known
    virtual bool bpropDoesNothing();

    //! If this class has a learning rate (or something close to it), set it
    virtual void setLearningRate(real dynamic_learning_rate);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(ModuleStackModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


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

    //! values[i] represents the value of the output of module i and the input
    //! of module i+1. No need for values[n_modules-1] because it's the output.
    //! gradients[i] and diag_hessians[i] works just the same, and there is no
    //! need for gradients[-1] because it is input_gradient.
    mutable TVec<Vec> values;
    mutable TVec<Vec> gradients;
    mutable TVec<Vec> diag_hessians;

    //! Mini-batch versions.
    TVec<Mat> values_m;
    TVec<Mat> gradients_m;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ModuleStackModule);

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
