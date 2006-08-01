// -*- C++ -*-

// Convolution2DModule.cc
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

/*! \file Convolution2DModule.cc */


#define PL_LOG_MODULE_NAME "HintonDeepBeliefNet"
#include <plearn/io/pl_log.h>

#include "Convolution2DModule.h"
#include <plearn/math/Convolutions.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    Convolution2DModule,
    "Apply convolution filters on (possibly multiple) 2D inputs (images)",
    "");

Convolution2DModule::Convolution2DModule() :
    n_input_images(1),
    input_images_length(-1),
    input_images_width(-1),
    n_output_images(1),
    kernel_length(-1),
    kernel_width(-1),
    kernel_step1(1),
    kernel_step2(1),
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

void Convolution2DModule::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &Convolution2DModule::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");

    declareOption(ol, "n_input_images", &Convolution2DModule::n_input_images,
                  OptionBase::buildoption,
                  "Number of input images present at the same time in the"
                  " input vector");

    declareOption(ol, "input_images_length",
                  &Convolution2DModule::input_images_length,
                  OptionBase::buildoption,
                  "Length of each of the input images");

    declareOption(ol, "input_images_width",
                  &Convolution2DModule::input_images_width,
                  OptionBase::buildoption,
                  "Width of each of the input images");

    declareOption(ol, "n_output_images", &Convolution2DModule::n_output_images,
                  OptionBase::buildoption,
                  "Number of output images to put in the output vector");

    declareOption(ol, "kernel_length", &Convolution2DModule::kernel_length,
                  OptionBase::buildoption,
                  "Length of each filter (or kernel) applied on an input image"
                  );

    declareOption(ol, "kernel_width", &Convolution2DModule::kernel_width,
                  OptionBase::buildoption,
                  "Width of each filter (or kernel) applied on an input image"
                  );

    declareOption(ol, "kernel_step1", &Convolution2DModule::kernel_step1,
                  OptionBase::buildoption,
                  "Horizontal step of the kernels");

    declareOption(ol, "kernel_step2", &Convolution2DModule::kernel_step2,
                  OptionBase::buildoption,
                  "Vertical step of the kernels");

    declareOption(ol, "connection_matrix",
                  &Convolution2DModule::connection_matrix,
                  OptionBase::buildoption,
                  "Matrix of connections:\n"
                  "it has n_input_images rows and n_output_images columns,\n"
                  "each output image will only be connected to a subset of"
                  " the\n"
                  "input images, where a non-zero value is present in this"
                  " matrix.\n"
                  "If this matrix is not provided, it will be fully"
                  " connected.\n"
                  );

    declareOption(ol, "start_learning_rate",
                  &Convolution2DModule::start_learning_rate,
                  OptionBase::buildoption,
                  "Starting learning-rate, by which we multiply the gradient"
                  " step"
                  );

    declareOption(ol, "decrease_constant",
                  &Convolution2DModule::decrease_constant,
                  OptionBase::buildoption,
                  "learning_rate = start_learning_rate / (1 +"
                  " decrease_constant*t),\n"
                  "where t is the number of updates since the beginning\n"
                  );

    declareOption(ol, "output_images_length",
                  &Convolution2DModule::output_images_length,
                  OptionBase::learntoption,
                  "Length of the output images");

    declareOption(ol, "output_images_width",
                  &Convolution2DModule::output_images_width,
                  OptionBase::learntoption,
                  "Width of the output images");

    declareOption(ol, "kernels", &Convolution2DModule::kernels,
                  OptionBase::learntoption,
                  "Contains the kernels between input and output images");

    declareOption(ol, "bias", &Convolution2DModule::bias,
                  OptionBase::learntoption,
                  "Contains the bias of the output images");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Redeclare some of the parent's options as learntoptions
    redeclareOption(ol, "input_size", &Convolution2DModule::input_size,
                    OptionBase::learntoption,
                    "Size of the input, computed from n_input_images,\n"
                    "n_input_length and n_input_width.\n");

    redeclareOption(ol, "output_size", &Convolution2DModule::output_size,
                    OptionBase::learntoption,
                    "Size of the output, computed from n_output_images,\n"
                    "n_output_length and n_output_width.\n");
}

void Convolution2DModule::build_()
{
    MODULE_LOG << "build_() called" << endl;

    // Verify the parameters
    if( n_input_images < 1 )
        PLERROR("Convolution2DModule::build_: 'n_input_images' < 1 (%i).\n",
                n_input_images);

    if( input_images_length < 0 )
        PLERROR("Convolution2DModule::build_: 'input_images_length'<0 (%i).\n",
                input_images_length);

    if( input_images_width < 0 )
        PLERROR("Convolution2DModule::build_: 'input_images_width'<0 (%i).\n",
                input_images_width);

    if( n_output_images < 1 )
        PLERROR("Convolution2DModule::build_: 'n_output_images' < 1 (%i).\n",
                n_input_images);

    if( kernel_length < 0 )
        PLERROR("Convolution2DModule::build_: 'kernel_length'<0 (%i).\n",
                kernel_length);

    if( kernel_width < 0 )
        PLERROR("Convolution2DModule::build_: 'kernel_width'<0 (%i).\n",
                kernel_width);

    if( kernel_step1 < 0 )
        PLERROR("Convolution2DModule::build_: 'kernel_step1'<0 (%i).\n",
                kernel_step1);

    if( kernel_step2 < 0 )
        PLERROR("Convolution2DModule::build_: 'kernel_step2'<0 (%i).\n",
                kernel_step2);

    if( (input_images_length - kernel_length) % kernel_step1 != 0 )
        PLERROR("Convolution2DModule::build_:\n"
                "the difference (input_images_length - kernel_length) (%i)\n"
                "should be a multiple of kernel_step1 (%i).\n",
                (input_images_length - kernel_length), kernel_step1);

    if( (input_images_width - kernel_width) % kernel_step2 != 0 )
        PLERROR("Convolution2DModule::build_:\n"
                "the difference (input_images_width - kernel_width) (%i)\n"
                "should be a multiple of kernel_step2 (%i).\n",
                (input_images_width - kernel_width), kernel_step2);

    // Build the learntoptions from the buildoptions
    input_images_size = input_images_length * input_images_width;
    input_size = n_input_images * input_size;

    output_images_length = (input_images_length-kernel_length)/kernel_step1+1;
    output_images_width = (input_images_width - kernel_width)/kernel_step2+1;
    output_images_size = output_images_length * output_images_width;

    kernel_size = kernel_length * kernel_width;

    bias.resize(n_output_images);

    // If connection_matrix was not specified, or inconsistently,
    // make it a matrix full of ones.
    if( connection_matrix.length() != n_input_images
        || connection_matrix.width() != n_output_images )
    {
        connection_matrix.resize(n_input_images, n_output_images);
        connection_matrix.fill(1);
    }

    if( !random_gen )
        random_gen = new PRandom();

    build_kernels();

    input_images.resize(n_input_images);
    output_images.resize(n_output_images);
    input_gradients.resize(n_input_images);
    output_gradients.resize(n_output_images);
    input_diag_hessians.resize(n_input_images);
    output_diag_hessians.resize(n_output_images);
}

void Convolution2DModule::build_kernels()
{
    // If kernels has the right size, for all i and j kernel(i,j) exists iff
    // connection_matrix(i,j) !=0, and has the appropriate size, then we don't
    // want to forget them.
    bool need_rebuild = false;
    if( kernels.length() != n_input_images
        || kernels.width() != n_output_images )
    {
        need_rebuild = true;
    }
    else
    {
        for( int i=0 ; i<n_input_images ; i++ )
            for( int j=0 ; j<n_output_images ; j++ )
            {
                if( connection_matrix(i,j) == 0 )
                {
                    if( kernels(i,j).size() != 0 )
                    {
                        need_rebuild = true;
                        break;
                    }
                }
                else if( kernels(i,j).length() != kernel_length
                         || kernels(i,j).width() != kernel_width )
                {
                    need_rebuild = true;
                    break;
                }
            }
    }

    if( need_rebuild )
        forget();

    kernel_gradient.resize(kernel_length, kernel_width);
    squared_kernel.resize(kernel_length, kernel_width);
}

void Convolution2DModule::build()
{
    inherited::build();
    build_();
}


void Convolution2DModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(connection_matrix, copies);
    deepCopyField(kernels, copies);
    deepCopyField(bias, copies);
    deepCopyField(input_images, copies);
    deepCopyField(output_images, copies);
    deepCopyField(input_gradients, copies);
    deepCopyField(output_gradients, copies);
    deepCopyField(input_diag_hessians, copies);
    deepCopyField(output_diag_hessians, copies);
    deepCopyField(kernel_gradient, copies);
    deepCopyField(squared_kernel, copies);

}

//! given the input, compute the output (possibly resize it  appropriately)
void Convolution2DModule::fprop(const Vec& input, Vec& output) const
{
    // Check size
    if( input.size() != input_size )
        PLERROR("Convolution2DModule::fprop: input.size() should be equal to\n"
                "input_size (%i != %i).\n", input.size(), input_size);
    output.resize(output_size);

    // Make input_images and output_images point to the right places
    for( int i=0 ; i<n_input_images ; i++ )
        input_images[i] =
            input.subVec(i*input_images_size, input_images_size)
                .toMat( input_images_length, input_images_width );

    for( int j=0 ; j<n_output_images ; j++ )
        output_images[j] =
            output.subVec(j*output_images_size, output_images_size)
                .toMat( output_images_length, output_images_width );

    // Compute the values of the output_images
    for( int j=0 ; j<n_output_images ; j++ )
    {
        output_images[j].fill( bias[j] );
        for( int i=0 ; i<n_input_images ; i++ )
        {
            if( connection_matrix(i,j) != 0 )
                convolve2D( input_images[i], kernels(i,j), output_images[j],
                            kernel_step1, kernel_step2, true );
        }
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
void Convolution2DModule::bpropUpdate(const Vec& input, const Vec& output,
                               const Vec& output_gradient)
{
}
*/

//! this version allows to obtain the input gradient as well
void Convolution2DModule::bpropUpdate(const Vec& input, const Vec& output,
                                      Vec& input_gradient,
                                      const Vec& output_gradient)
{
    // Check size
    if( input.size() != input_size )
        PLERROR("Convolution2DModule::bpropUpdate: input.size() should be\n"
                "equal to input_size (%i != %i).\n", input.size(), input_size);
    if( output.size() != output_size )
        PLERROR("Convolution2DModule::bpropUpdate: output.size() should be\n"
                "equal to output_size (%i != %i).\n",
                output.size(), output_size);
    if( output_gradient.size() != output_size )
        PLERROR("Convolution2DModule::bpropUpdate: output_gradient.size()"
                " should be\n"
                "equal to output_size (%i != %i).\n",
                output_gradient.size(), output_size);

    input_gradient.resize(input_size);

    // Since fprop() has just been called, we assume that input_images and
    // output_images are up-to-date
    // Make input_gradients and output_gradients point to the right places
    for( int i=0 ; i<n_input_images ; i++ )
        input_gradients[i] =
            input_gradient.subVec(i*input_images_size, input_images_size)
                .toMat( input_images_length, input_images_width );

    for( int j=0 ; j<n_output_images ; j++ )
        output_gradients[j] =
            output_gradient.subVec(j*output_images_size, output_images_size)
                .toMat( output_images_length, output_images_width );

    // Do the actual bprop and update
    learning_rate = start_learning_rate / (1+decrease_constant*step_number);
    for( int j=0 ; j<n_output_images ; j++ )
    {
        for( int i=0 ; i<n_input_images ; j++ )
            if( connection_matrix(i,j) != 0 )
            {
                convolve2Dbackprop( input_images[i], kernels(i,j),
                                    output_gradients[j],
                                    input_gradients[i], kernel_gradient,
                                    kernel_step1, kernel_step2, false );

                // kernel(i,j) -= learning_rate * kernel_gradient
                multiplyAcc( kernels(i,j), kernel_gradient, -learning_rate );
            }
        bias[j] -= learning_rate * sum( output_gradients[j] );
    }

}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void Convolution2DModule::forget()
{
    real scale_factor = 1./(kernel_length*kernel_width);
    for( int i=0 ; i<n_input_images ; i++ )
        for( int j=0 ; j<n_output_images ; j++ )
        {
            if( connection_matrix(i,j) == 0 )
                kernels(i,j).resize(0,0);
            else
            {
                kernels(i,j).resize(kernel_length, kernel_width);
                random_gen->fill_random_uniform( kernels(i,j),
                                                 -scale_factor,
                                                 scale_factor );
            }
        }
    bias.clear();
}

/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void Convolution2DModule::finalize()
{
}
*/

/* THIS METHOD IS OPTIONAL
//! in case bpropUpdate does not do anything, make it known
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS RETURNS false;
bool Convolution2DModule::bpropDoesNothing()
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
void Convolution2DModule::bbpropUpdate(const Vec& input, const Vec& output,
                                const Vec& output_gradient,
                                const Vec& output_diag_hessian)
{
}
*/

//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
void Convolution2DModule::bbpropUpdate(const Vec& input, const Vec& output,
                                       Vec& input_gradient,
                                       const Vec& output_gradient,
                                       Vec& input_diag_hessian,
                                       const Vec& output_diag_hessian)
{
    // This version forwards the second order information, but does not
    // actually use it for the update.

    // Check size
    if( output_diag_hessian.size() != output_size )
        PLERROR("Convolution2DModule::bbpropUpdate: output_diag_hessian.size()"
                "\n"
                "should be equal to output_size (%i != %i).\n",
                output_diag_hessian.size(), output_size);
    input_diag_hessian.resize(input_size);

    // Make input_diag_hessians and output_diag_hessians point to the right
    // places
    for( int i=0 ; i<n_input_images ; i++ )
        input_diag_hessians[i] =
            input_diag_hessian.subVec(i*input_images_size, input_images_size)
                .toMat( input_images_length, input_images_width );

    for( int j=0 ; j<n_output_images ; j++ )
        output_diag_hessians[j] =
            output_diag_hessian.subVec(j*output_images_size,output_images_size)
                .toMat( output_images_length, output_images_width );

    // Propagates to input_diag_hessian
    for( int j=0 ; j<n_output_images ; j++ )
        for( int i=0 ; j<n_input_images ; i++ )
            if( connection_matrix(i,j) != 0 )
            {
                squared_kernel << kernels(i,j);
                squared_kernel *= kernels(i,j); // term-to-term product

                backConvolve2D( input_diag_hessians[i], squared_kernel,
                                output_diag_hessians[j],
                                kernel_step1, kernel_step2, false );
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
