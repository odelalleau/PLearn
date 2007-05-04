// -*- C++ -*-

// Subsampling2DModule.cc
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

/*! \file Subsampling2DModule.cc */


#define PL_LOG_MODULE_NAME "Subsampling2DModule"
#include <plearn/io/pl_log.h>

#include "Subsampling2DModule.h"
#include <plearn/math/convolutions.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    Subsampling2DModule,
    "Apply convolution filters on (possibly multiple) 2D inputs (images)",
    "");

Subsampling2DModule::Subsampling2DModule() :
    n_input_images(1),
    input_images_length(-1),
    input_images_width(-1),
    kernel_length(-1),
    kernel_width(-1),
    start_learning_rate(0.),
    decrease_constant(0.),
    output_images_length(-1),
    output_images_width(-1),
    input_images_size(-1),
    output_images_size(-1),
    kernel_size(-1),
    learning_rate(0.),
    step_number(0)
{
}

void Subsampling2DModule::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &Subsampling2DModule::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");

    declareOption(ol, "n_input_images", &Subsampling2DModule::n_input_images,
                  OptionBase::buildoption,
                  "Number of input images present at the same time in the"
                  " input vector");

    declareOption(ol, "input_images_length",
                  &Subsampling2DModule::input_images_length,
                  OptionBase::buildoption,
                  "Length of each of the input images");

    declareOption(ol, "input_images_width",
                  &Subsampling2DModule::input_images_width,
                  OptionBase::buildoption,
                  "Width of each of the input images");

    declareOption(ol, "kernel_length", &Subsampling2DModule::kernel_length,
                  OptionBase::buildoption,
                  "Length of the areas to sum"
                  );

    declareOption(ol, "kernel_width", &Subsampling2DModule::kernel_width,
                  OptionBase::buildoption,
                  "Width of the areas to sum"
                  );

    declareOption(ol, "start_learning_rate",
                  &Subsampling2DModule::start_learning_rate,
                  OptionBase::buildoption,
                  "Starting learning-rate, by which we multiply the gradient"
                  " step"
                  );

    declareOption(ol, "decrease_constant",
                  &Subsampling2DModule::decrease_constant,
                  OptionBase::buildoption,
                  "learning_rate = start_learning_rate / (1 +"
                  " decrease_constant*t),\n"
                  "where t is the number of updates since the beginning\n"
                  );

    declareOption(ol, "output_images_length",
                  &Subsampling2DModule::output_images_length,
                  OptionBase::learntoption,
                  "Length of the output images");

    declareOption(ol, "output_images_width",
                  &Subsampling2DModule::output_images_width,
                  OptionBase::learntoption,
                  "Width of the output images");

    declareOption(ol, "scale", &Subsampling2DModule::scale,
                  OptionBase::learntoption,
                  "Contains the scale of the output images");

    declareOption(ol, "bias", &Subsampling2DModule::bias,
                  OptionBase::learntoption,
                  "Contains the bias of the output images");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Redeclare some of the parent's options as learntoptions
    redeclareOption(ol, "input_size", &Subsampling2DModule::input_size,
                    OptionBase::learntoption,
                    "Size of the input, computed from n_input_images,\n"
                    "n_input_length and n_input_width.\n");

    redeclareOption(ol, "output_size", &Subsampling2DModule::output_size,
                    OptionBase::learntoption,
                    "Size of the output, computed from n_output_images,\n"
                    "n_output_length and n_output_width.\n");
}

void Subsampling2DModule::build_()
{
    MODULE_LOG << "build_() called" << endl;

    // Verify the parameters
    if( n_input_images < 1 )
        PLERROR("Subsampling2DModule::build_: 'n_input_images' < 1 (%i).\n",
                n_input_images);

    if( input_images_length < 0 )
        PLERROR("Subsampling2DModule::build_: 'input_images_length'<0 (%i).\n",
                input_images_length);

    if( input_images_width < 0 )
        PLERROR("Subsampling2DModule::build_: 'input_images_width'<0 (%i).\n",
                input_images_width);

    if( kernel_length < 0 )
        PLERROR("Subsampling2DModule::build_: 'kernel_length'<0 (%i).\n",
                kernel_length);

    if( kernel_width < 0 )
        PLERROR("Subsampling2DModule::build_: 'kernel_width'<0 (%i).\n",
                kernel_width);

    if( input_images_length % kernel_length != 0 )
        PLERROR("Subsampling2DModule::build_: input_images_length (%i)\n"
                "should be a multiple of kernel_length (%i).\n",
                input_images_length, kernel_length);

    if( input_images_width % kernel_width != 0 )
        PLERROR("Subsampling2DModule::build_: input_images_width (%i)\n"
                "should be a multiple of kernel_width (%i).\n",
                input_images_width, kernel_width);

    // Build the learntoptions from the buildoptions
    input_images_size = input_images_length * input_images_width;
    input_size = n_input_images * input_images_size;

    output_images_length = input_images_length / kernel_length;
    output_images_width = input_images_width / kernel_width;
    output_images_size = output_images_length * output_images_width;
    output_size = n_input_images * output_images_size;

    kernel_size = kernel_length * kernel_width;

    scale.resize(n_input_images);
    bias.resize(n_input_images);

    input_images.resize(n_input_images);
    output_images.resize(n_input_images);
    kernel.resize(kernel_length, kernel_width);
    input_gradients.resize(n_input_images);
    output_gradients.resize(n_input_images);
    kernel_gradient.resize(kernel_length, kernel_width);
}

void Subsampling2DModule::build()
{
    inherited::build();
    build_();
}


void Subsampling2DModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(scale, copies);
    deepCopyField(bias, copies);
    deepCopyField(input_images, copies);
    deepCopyField(output_images, copies);
    deepCopyField(input_gradients, copies);
    deepCopyField(output_gradients, copies);
    deepCopyField(kernel, copies);
    deepCopyField(squared_kernel, copies);
    deepCopyField(kernel_gradient, copies);

}

//! given the input, compute the output (possibly resize it  appropriately)
void Subsampling2DModule::fprop(const Vec& input, Vec& output) const
{
    // Check size
    if( input.size() != input_size )
        PLERROR("Subsampling2DModule::fprop: input.size() should be equal to\n"
                "input_size (%i != %i).\n", input.size(), input_size);
    output.resize(output_size);

    // Make input_images and output_images point to the right places
    for( int i=0 ; i<n_input_images ; i++ )
    {
        input_images[i] =
            input.subVec(i*input_images_size, input_images_size)
                .toMat( input_images_length, input_images_width );

        output_images[i] =
            output.subVec(i*output_images_size, output_images_size)
                .toMat( output_images_length, output_images_width );
    }

    // Compute the values of the output_images
    for( int i=0 ; i<n_input_images ; i++ )
    {
        output_images[i].fill( bias[i] );
        kernel.fill( scale[i] );
        convolve2D( input_images[i], kernel, output_images[i],
                    kernel_length, kernel_width, true );
    }
}

/* THIS METHOD IS OPTIONAL
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
void Subsampling2DModule::bpropUpdate(const Vec& input, const Vec& output,
                               const Vec& output_gradient)
{
}
*/

//! this version allows to obtain the input gradient as well
void Subsampling2DModule::bpropUpdate(const Vec& input, const Vec& output,
                                      Vec& input_gradient,
                                      const Vec& output_gradient,
                                      bool accumulate)
{
    // Check size
    if( input.size() != input_size )
        PLERROR("Subsampling2DModule::bpropUpdate: input.size() should be\n"
                "equal to input_size (%i != %i).\n", input.size(), input_size);
    if( output.size() != output_size )
        PLERROR("Subsampling2DModule::bpropUpdate: output.size() should be\n"
                "equal to output_size (%i != %i).\n",
                output.size(), output_size);
    if( output_gradient.size() != output_size )
        PLERROR("Subsampling2DModule::bpropUpdate: output_gradient.size()"
                " should be\n"
                "equal to output_size (%i != %i).\n",
                output_gradient.size(), output_size);

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
        input_gradient.resize(input_size);

    // Since fprop() has just been called, we assume that input_images,
    // output_images and gradient are up-to-date
    // Make input_gradients and output_gradients point to the right places
    for( int i=0 ; i<n_input_images ; i++ )
    {
        input_gradients[i] =
            input_gradient.subVec(i*input_images_size, input_images_size)
                .toMat( input_images_length, input_images_width );

        output_gradients[i] =
            output_gradient.subVec(i*output_images_size, output_images_size)
                .toMat( output_images_length, output_images_width );
    }

    // Do the actual bprop and update
    learning_rate = start_learning_rate / (1+decrease_constant*step_number);
    for( int i=0 ; i<n_input_images ; i++ )
    {
        kernel.fill( scale[i] );
        convolve2Dbackprop( input_images[i], kernel,
                            output_gradients[i],
                            input_gradients[i], kernel_gradient,
                            kernel_length, kernel_width, accumulate );

        // The scale's gradient is the sum of contributions to kernel_gradient
        scale[i] -= learning_rate * sum( kernel_gradient );
        bias[i] -= learning_rate * sum( output_gradients[i] );
    }
}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void Subsampling2DModule::forget()
{
    bias.clear();

    if( !random_gen )
    {
        PLWARNING( "Subsampling2DModule: cannot forget() without random_gen" );
        return;
    }
    real scale_factor = 1./(kernel_length*kernel_width);
    random_gen->fill_random_uniform( scale, -scale_factor, scale_factor );
}

/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void Subsampling2DModule::finalize()
{
}
*/

/* THIS METHOD IS OPTIONAL
//! in case bpropUpdate does not do anything, make it known
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS RETURNS false;
bool Subsampling2DModule::bpropDoesNothing()
{
}
*/

/* THIS METHOD IS OPTIONAL
//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! JUST CALLS
//!     bbpropUpdate(input, output, input_gradient, output_gradient,
//!                  in_hess, out_hess)
//! AND IGNORES INPUT HESSIAN AND INPUT GRADIENT.
void Subsampling2DModule::bbpropUpdate(const Vec& input, const Vec& output,
                                const Vec& output_gradient,
                                const Vec& output_diag_hessian)
{
}
*/

//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
void Subsampling2DModule::bbpropUpdate(const Vec& input, const Vec& output,
                                       Vec& input_gradient,
                                       const Vec& output_gradient,
                                       Vec& input_diag_hessian,
                                       const Vec& output_diag_hessian,
                                       bool accumulate)
{
    // This version forwards the second order information, but does not
    // actually use it for the update.

    // Check size
    if( output_diag_hessian.size() != output_size )
        PLERROR("Subsampling2DModule::bbpropUpdate: output_diag_hessian.size()"
                "\n"
                "should be equal to output_size (%i != %i).\n",
                output_diag_hessian.size(), output_size);

    if( accumulate )
    {
        PLASSERT_MSG( input_diag_hessian.size() == input_size,
                      "Cannot resize input_diag_hessian AND accumulate into it"
                    );
    }
    else
        input_diag_hessian.resize(input_size);

    // Make input_diag_hessians and output_diag_hessians point to the right
    // places
    for( int i=0 ; i<n_input_images ; i++ )
    {
        input_diag_hessians[i] =
            input_diag_hessian.subVec(i*input_images_size, input_images_size)
                .toMat( input_images_length, input_images_width );

        output_diag_hessians[i] =
            output_diag_hessian.subVec(i*output_images_size,output_images_size)
                .toMat( output_images_length, output_images_width );
    }

    // Propagates to input_diag_hessian
    for( int i=0 ; i<n_input_images ; i++ )
    {
        kernel.fill( scale[i] );
        squared_kernel.fill( scale[i]*scale[i] );
        backConvolve2D( input_diag_hessians[i], squared_kernel,
                        output_diag_hessians[i],
                        kernel_length, kernel_width, accumulate );
    }

    // Call bpropUpdate()
    bpropUpdate( input, output, input_gradient, output_gradient );
}


} // end of namespace PLearn


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
