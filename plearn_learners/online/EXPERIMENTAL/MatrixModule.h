// -*- C++ -*-

// MatrixModule.h
//
// Copyright (C) 2007 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file MatrixModule.h */


#ifndef MatrixModule_INC
#define MatrixModule_INC

#include <plearn_learners/online/OnlineLearningModule.h>

namespace PLearn {

/**
 * The first sentence should be a BRIEF DESCRIPTION of what the class does.
 * Place the rest of the class programmer documentation here.  Doxygen supports
 * Javadoc-style comments.  See http://www.doxygen.org/manual.html
 *
 * @todo Write class to-do's here if there are any.
 *
 * @deprecated Write deprecated stuff here if there is any.  Indicate what else
 * should be used instead.
 */
class MatrixModule : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    Mat data;
    Mat data_gradient;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    MatrixModule(bool call_build_ = false);

    // Your other public member functions go here

    //! Make this module see the provided 'the_data' matrix. Note that no copy
    //! is made, so 'the_data' should not be modified afterwards.
    void setData(const Mat& the_data);

    //! Return the current matrix viewed by this module.
    Mat& getData();

    //! given the input, compute the output (possibly resize it  appropriately)
    virtual void fprop(const Vec& input, Vec& output) const;


    /* Optional
       THE DEFAULT IMPLEMENTATION IN SUPER-CLASS JUST RAISES A PLERROR.
    //! Adapt based on the output gradient, and obtain the input gradient.
    //! The flag indicates wether the input_gradient is accumulated or set.
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
    */

    /* Optional
       A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
       JUST CALLS
            bpropUpdate(input, output, input_gradient, output_gradient)
       AND IGNORES INPUT GRADIENT.
    //! This version does not obtain the input gradient.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             const Vec& output_gradient);
    */

    /* Optional
       N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
       RAISES A PLERROR.
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
    */

    /* Optional
       N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS,
       WHICH JUST CALLS
            bbpropUpdate(input, output, input_gradient, output_gradient,
                         out_hess, in_hess)
       AND IGNORES INPUT HESSIAN AND INPUT GRADIENT.
    //! This version does not obtain the input gradient and diag_hessian.
    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              const Vec& output_gradient,
                              const Vec& output_diag_hessian);
    */


    //! Reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();


    /* Optional
       THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT
       DO ANYTHING.
    //! Perform some processing after training, or after a series of
    //! fprop/bpropUpdate calls to prepare the model for truly out-of-sample
    //! operation.
    virtual void finalize();
    */

    /* Optional
       THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS RETURNS false
    //! In case bpropUpdate does not do anything, make it known
    virtual bool bpropDoesNothing();
    */

    /* Optional
       Default implementation prints a warning and does nothing
    //! If this class has a learning rate (or something close to it), set it.
    //! If not, you can redefine this method to get rid of the warning.
    virtual void setLearningRate(real dynamic_learning_rate);
    */

    //! If provided, the matrix value is used to fill 'data'.
    //! If required, it is filled with 'data'.
    virtual void fprop(const TVec<Mat*>& ports_value);

    //! If provided, the gradient is used to update 'data' (and fill
    //! 'data_gradient').
    //! If required, it is filled with 'gradient'.
    virtual void bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient);

    //! Overridden.
    virtual const TVec<string>& getPorts();

    //! Overridden.
    virtual const TMat<int>& getPortSizes();

    //! Set the gradient w.r.t. 'data' to the provided constant value 'g'.
    //! The result is that 'data_gradient' is resized to the same size as 'data'
    //! and filled with 'g'.
    void setGradientTo(real g);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(MatrixModule);

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
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(MatrixModule);

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
