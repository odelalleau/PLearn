// -*- C++ -*-

// MaxSubsampling2DModule.cc
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

/*! \file MaxSubsampling2DModule.cc */


#define PL_LOG_MODULE_NAME "MaxSubsampling2DModule"
#include <plearn/io/pl_log.h>

#include "MaxSubsampling2DModule.h"
#include <plearn/math/convolutions.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    MaxSubsampling2DModule,
    "Apply convolution filters on (possibly multiple) 2D inputs (images)",
    "");

MaxSubsampling2DModule::MaxSubsampling2DModule() :
    n_input_images(1),
    input_images_length(-1),
    input_images_width(-1),
    kernel_length(-1),
    kernel_width(-1),
    output_images_length(-1),
    output_images_width(-1),
    input_images_size(-1),
    output_images_size(-1)
{
}

void MaxSubsampling2DModule::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &MaxSubsampling2DModule::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");

    declareOption(ol, "n_input_images",
                  &MaxSubsampling2DModule::n_input_images,
                  OptionBase::buildoption,
                  "Number of input images present at the same time in the"
                  " input vector");

    declareOption(ol, "input_images_length",
                  &MaxSubsampling2DModule::input_images_length,
                  OptionBase::buildoption,
                  "Length of each of the input images");

    declareOption(ol, "input_images_width",
                  &MaxSubsampling2DModule::input_images_width,
                  OptionBase::buildoption,
                  "Width of each of the input images");

    declareOption(ol, "kernel_length", &MaxSubsampling2DModule::kernel_length,
                  OptionBase::buildoption,
                  "Length of the areas to maximize over"
                  );

    declareOption(ol, "kernel_width", &MaxSubsampling2DModule::kernel_width,
                  OptionBase::buildoption,
                  "Width of the areas to maximize over"
                  );

    declareOption(ol, "output_images_length",
                  &MaxSubsampling2DModule::output_images_length,
                  OptionBase::learntoption,
                  "Length of the output images");

    declareOption(ol, "output_images_width",
                  &MaxSubsampling2DModule::output_images_width,
                  OptionBase::learntoption,
                  "Width of the output images");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Redeclare some of the parent's options as learntoptions
    redeclareOption(ol, "input_size", &MaxSubsampling2DModule::input_size,
                    OptionBase::learntoption,
                    "Size of the input, computed from n_input_images,\n"
                    "input_images_length and input_images_width.\n");

    redeclareOption(ol, "output_size", &MaxSubsampling2DModule::output_size,
                    OptionBase::learntoption,
                    "Size of the output, computed from n_input_images,\n"
                    "output_images_length and output_images_width.\n");
}

void MaxSubsampling2DModule::build_()
{
    MODULE_LOG << "build_() called" << endl;

    // Verify the parameters
    if( n_input_images < 1 )
        PLERROR("MaxSubsampling2DModule::build_: 'n_input_images' < 1 (%i).\n",
                n_input_images);

    if( input_images_length < 0 )
        PLERROR("MaxSubsampling2DModule::build_: 'input_images_length'<0 (%i).\n",
                input_images_length);

    if( input_images_width < 0 )
        PLERROR("MaxSubsampling2DModule::build_: 'input_images_width'<0 (%i).\n",
                input_images_width);

    if( kernel_length < 0 )
        PLERROR("MaxSubsampling2DModule::build_: 'kernel_length'<0 (%i).\n",
                kernel_length);

    if( kernel_width < 0 )
        PLERROR("MaxSubsampling2DModule::build_: 'kernel_width'<0 (%i).\n",
                kernel_width);

    if( input_images_length % kernel_length != 0 )
        PLERROR("MaxSubsampling2DModule::build_: input_images_length (%i)\n"
                "should be a multiple of kernel_length (%i).\n",
                input_images_length, kernel_length);

    if( input_images_width % kernel_width != 0 )
        PLERROR("MaxSubsampling2DModule::build_: input_images_width (%i)\n"
                "should be a multiple of kernel_width (%i).\n",
                input_images_width, kernel_width);

    // Build the learntoptions from the buildoptions
    input_images_size = input_images_length * input_images_width;
    input_size = n_input_images * input_images_size;

    output_images_length = input_images_length / kernel_length;
    output_images_width = input_images_width / kernel_width;
    output_images_size = output_images_length * output_images_width;
    output_size = n_input_images * output_images_size;

    input_images.resize(n_input_images);
    output_images.resize(n_input_images);
    input_gradients.resize(n_input_images);
    output_gradients.resize(n_input_images);

    all_max_indices.resize(output_size);
    max_indices.resize(n_input_images);
    for( int i = 0; i < n_input_images; i++ )
        max_indices[i] =
            all_max_indices.subVec(i*output_images_size, output_images_size)
                .toMat(output_images_length, output_images_width);
}

void MaxSubsampling2DModule::build()
{
    inherited::build();
    build_();
}


void MaxSubsampling2DModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(input_images, copies);
    deepCopyField(output_images, copies);
    deepCopyField(input_gradients, copies);
    deepCopyField(output_gradients, copies);
    deepCopyField(all_max_indices, copies);
    deepCopyField(max_indices, copies);
}

//! given the input, compute the output (possibly resize it  appropriately)
void MaxSubsampling2DModule::fprop(const Vec& input, Vec& output) const
{
    // Check size
    if( input.size() != input_size )
        PLERROR("MaxSubsampling2DModule::fprop: input.size() should be equal to\n"
                "input_size (%i != %i).\n", input.size(), input_size);
    output.resize(output_size);

    // Make input_images and output_images point to the right places
    for( int l=0 ; l<n_input_images ; l++ )
    {
        input_images[l] =
            input.subVec(l*input_images_size, input_images_size)
                .toMat( input_images_length, input_images_width );

        output_images[l] =
            output.subVec(l*output_images_size, output_images_size)
                .toMat( output_images_length, output_images_width );
    }

    // Compute the values of the output_images
    for( int l=0 ; l<n_input_images ; l++ )
        for( int i=0; i<output_images_length; i++ )
            for( int j=0; j<output_images_width; j++ )
            {
                int min_i, min_j;
                output_images[l](i,j) = max(
                    input_images[l].subMat(i*kernel_length, j*kernel_width,
                                           kernel_length, kernel_width),
                    min_i, min_j );
                max_indices[l](i,j) = min_i*input_images_width + min_j;
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
void MaxSubsampling2DModule::bpropUpdate(const Vec& input, const Vec& output,
                               const Vec& output_gradient)
{
}
*/

//! this version allows to obtain the input gradient as well
void MaxSubsampling2DModule::bpropUpdate(const Vec& input, const Vec& output,
                                         Vec& input_gradient,
                                         const Vec& output_gradient,
                                         bool accumulate)
{
    // Check size
    if( input.size() != input_size )
        PLERROR("MaxSubsampling2DModule::bpropUpdate: input.size() should be\n"
                "equal to input_size (%i != %i).\n", input.size(), input_size);
    if( output.size() != output_size )
        PLERROR("MaxSubsampling2DModule::bpropUpdate: output.size() should be\n"
                "equal to output_size (%i != %i).\n",
                output.size(), output_size);
    if( output_gradient.size() != output_size )
        PLERROR("MaxSubsampling2DModule::bpropUpdate: output_gradient.size()"
                " should be\n"
                "equal to output_size (%i != %i).\n",
                output_gradient.size(), output_size);

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
    {
        input_gradient.resize(input_size);
        input_gradient.clear();
    }

    // Since fprop() has just been called, we assume that input_images,
    // output_images and gradient are up-to-date
    // Make input_gradients and output_gradients point to the right places
    for( int l=0 ; l<n_input_images ; l++ )
    {
        input_gradients[l] =
            input_gradient.subVec(l*input_images_size, input_images_size)
                .toMat( input_images_length, input_images_width );

        output_gradients[l] =
            output_gradient.subVec(l*output_images_size, output_images_size)
                .toMat( output_images_length, output_images_width );
    }

    // Do the actual bprop and update
    for( int l=0 ; l<n_input_images ; l++ )
        for( int i=0; i<output_images_length; i++ )
            for( int j=0; j<output_images_width; j++ )
            {
                Mat input_grad_zone =
                    input_gradients[l].subMat(i*kernel_length, j*kernel_width,
                                              kernel_length, kernel_width);
                input_grad_zone.data()[ max_indices[l](i,j) ] =
                    output_gradients[l](i,j);
            }
}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void MaxSubsampling2DModule::forget()
{
    all_max_indices.clear();
}

/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void MaxSubsampling2DModule::finalize()
{
}
*/

/* THIS METHOD IS OPTIONAL
//! in case bpropUpdate does not do anything, make it known
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS RETURNS false;
bool MaxSubsampling2DModule::bpropDoesNothing()
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
void MaxSubsampling2DModule::bbpropUpdate(const Vec& input, const Vec& output,
                                const Vec& output_gradient,
                                const Vec& output_diag_hessian)
{
}
*/

/* NOT IMPLEMENTED
//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
void MaxSubsampling2DModule::bbpropUpdate(const Vec& input, const Vec& output,
                                       Vec& input_gradient,
                                       const Vec& output_gradient,
                                       Vec& input_diag_hessian,
                                       const Vec& output_diag_hessian,
                                       bool accumulate)
{
}
*/


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
