// -*- C++ -*-

// Subsampling2DModule.h
//
// Copyright (C) 2006 Pascal Lamblin
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

/*! \file Subsampling2DModule.h */


#ifndef Subsampling2DModule_INC
#define Subsampling2DModule_INC

#include <plearn_learners/online/OnlineLearningModule.h>

namespace PLearn {

/**
 * Reduce the size of the 2D images by adding the values of nearby pixels.
 *
 */
class Subsampling2DModule : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    //! Number of input images present at the same time in the input vector
    int n_input_images;

    //! Length of each of the input images
    int input_images_length;

    //! Width of each of the input images
    int input_images_width;

    //! Length of the areas to sum
    int kernel_length;

    //! Width of the areas to sum
    int kernel_width;

    //! Starting learning-rate, by which we multiply the gradient step
    real start_learning_rate;

    //! learning_rate = start_learning_rate / (1 + decrease_constant*t),
    //! where t is the number of updates since the beginning
    real decrease_constant;


    //#####  Learnt options, that we would want to access  ####################
    //! Scale applied to the sum (for each image)
    Vec scale;

    //! Bias added after the scaling
    Vec bias;

    //#####  Not options  #####################################################
    //! Length of the output images
    int output_images_length;

    //! Width of the output images
    int output_images_width;

    //! Size of the input images (length * width)
    int input_images_size;

    //! Size of the input images (length * width)
    int output_images_size;

    //! Size of the input images (length * width)
    int kernel_size;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    Subsampling2DModule();

    // Your other public member functions go here

    //! given the input, compute the output (possibly resize it  appropriately)
    virtual void fprop(const Vec& input, Vec& output) const;

    //! Adapt based on the output gradient: this method should only
    //! be called just after a corresponding fprop; it should be
    //! called with the same arguments as fprop for the first two arguments
    //! (and output should not have been modified since then).
    //! Since sub-classes are supposed to learn ONLINE, the object
    //! is 'ready-to-be-used' just after any bpropUpdate.
    //! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
    //! JUST CALLS
    //!     bpropUpdate(input, output, input_gradient, output_gradient)
    //! AND IGNORES INPUT GRADIENT.
    // virtual void bpropUpdate(const Vec& input, const Vec& output,
    //                          const Vec& output_gradient);

    //! this version allows to obtain the input gradient as well
    //! N.B. THE DEFAULT IMPLEMENTATION IN SUPER-CLASS JUST RAISES A PLERROR.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient,
                             const Vec& output_gradient);

    //! Similar to bpropUpdate, but adapt based also on the estimation
    //! of the diagonal of the Hessian matrix, and propagates this
    //! back. If these methods are defined, you can use them INSTEAD of
    //! bpropUpdate(...)
    //! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS,
    //! WHICH JUST CALLS
    //!     bbpropUpdate(input, output, input_gradient, output_gradient,
    //!                  out_hess, in_hess)
    //! AND IGNORES INPUT HESSIAN AND INPUT GRADIENT.
    // virtual void bbpropUpdate(const Vec& input, const Vec& output,
    //                           const Vec& output_gradient,
    //                           const Vec& output_diag_hessian);

    //! this version allows to obtain the input gradient and diag_hessian
    //! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
    //! RAISES A PLERROR.
    // virtual void bbpropUpdate(const Vec& input, const Vec& output,
    //                           Vec& input_gradient,
    //                           const Vec& output_gradient,
    //                           Vec& input_diag_hessian,
    //                           const Vec& output_diag_hessian);

    //! reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();


    //! optionally perform some processing after training, or after a
    //! series of fprop/bpropUpdate calls to prepare the model for truly
    //! out-of-sample operation.  THE DEFAULT IMPLEMENTATION PROVIDED IN
    //! THE SUPER-CLASS DOES NOT DO ANYTHING.
    // virtual void finalize();

    //! in case bpropUpdate does not do anything, make it known
    //! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS RETURNS false;
    // virtual bool bpropDoesNothing();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(Subsampling2DModule);

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
    real learning_rate;
    int step_number;

    // The Mat they contain will point to sub-parts of input and output vectors
    // and gradients, for more convenience
    TVec<Mat> input_images;
    TVec<Mat> output_images;
    TVec<Mat> input_gradients;
    TVec<Mat> output_gradients;

    // Will store the temporary kernels
    mutable Mat kernel;
    // Will store gradients wrt kernels
    mutable Mat kernel_gradient;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(Subsampling2DModule);

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
