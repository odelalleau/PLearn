// -*- C++ -*-

// MaxSubsampling2DModule.cc
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

/*! \file MaxSubsampling2DModule.cc */


#define PL_LOG_MODULE_NAME "MaxSubsampling2DModule"
#include <plearn/io/pl_log.h>

#include "MaxSubsampling2DModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    MaxSubsampling2DModule,
    "Reduce the size of the 2D images by taking the max value of nearby pixels",
    "");

MaxSubsampling2DModule::MaxSubsampling2DModule():
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
                  "Length of the areas to maximize over");

    declareOption(ol, "kernel_width", &MaxSubsampling2DModule::kernel_width,
                  OptionBase::buildoption,
                  "Width of the areas to maximize over");

    declareOption(ol, "output_images_length",
                  &MaxSubsampling2DModule::output_images_length,
                  OptionBase::learntoption,
                  "Length of the output images");

    declareOption(ol, "output_images_width",
                  &MaxSubsampling2DModule::output_images_width,
                  OptionBase::learntoption,
                  "Width of the output images");

    // declareOption(ol, "", &MaxSubsampling2DModule::,
    //               OptionBase::buildoption,
    //               "");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);


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

    // Build the learntoptions from the buildoptions
    input_images_size = input_images_length * input_images_width;
    input_size = n_input_images * input_images_size;

    PLCHECK( n_input_images > 0 );
    PLCHECK( input_images_length > 0 );
    PLCHECK( input_images_width > 0 );
    PLCHECK( kernel_length > 0 );
    PLCHECK( kernel_width > 0 );
    PLCHECK_MSG( input_images_length % kernel_length == 0,
                 "input_images_length should be a multiple of kernel_length" );
    PLCHECK_MSG( input_images_width % kernel_width == 0,
                 "input_images_width should be a multiple of kernel_width" );

    output_images_length = input_images_length / kernel_length;
    output_images_width = input_images_width / kernel_width;
    output_images_size = output_images_length * output_images_width;
    output_size = n_input_images * output_images_size;

    // build ports
    ports.resize(3);
    ports[0] = "input";
    ports[1] = "output";
    ports[2] = "argmax.state";

    // build port_sizes
    port_sizes.resize(nPorts(), 2);
    port_sizes.column(0).fill(-1);
    port_sizes(0, 1) = input_size;
    port_sizes(1, 1) = output_size;
    port_sizes(2, 1) = output_size;
}

// ### Nothing to add here, simply calls build_
void MaxSubsampling2DModule::build()
{
    inherited::build();
    build_();
}


void MaxSubsampling2DModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(ports, copies);
}

///////////
// fprop //
///////////
void MaxSubsampling2DModule::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == nPorts() );
    // check which ports are input
    // (ports_value[i] && !ports_value[i]->isEmpty())
    // which ports are output (ports_value[i] && ports_value[i]->isEmpty())
    // and which ports are ignored (!ports_value[i]).
    // If that combination of (input,output,ignored) is feasible by this class
    // then perform the corresponding computation. Otherwise launch the error
    // below. See the comment in the header file for more information.

    Mat* input = ports_value[0];
    Mat* output = ports_value[1];
    Mat* argmax = ports_value[2];

    if( input && !input->isEmpty()
        && output && output->isEmpty()
        && argmax && argmax->isEmpty() )
    {
        PLASSERT( input->width() == port_sizes(0,1) );

        int batch_size = input->length();
        output->resize(batch_size, port_sizes(1,1));
        argmax->resize(batch_size, port_sizes(2,1));

        for( int k=0; k<batch_size; k++ )
            for( int l=0; l<n_input_images; l++ )
            {
                Mat input_image_kl = (*input)(k)
                    .subVec(l*input_images_size, input_images_size)
                    .toMat(input_images_length, input_images_width);
                Mat output_image_kl = (*output)(k)
                    .subVec(l*output_images_size, output_images_size)
                    .toMat(output_images_length, output_images_width);
                Mat argmax_kl = (*argmax)(k)
                    .subVec(l*output_images_size, output_images_size)
                    .toMat(output_images_length, output_images_width);

                for( int i=0; i<output_images_length; i++ )
                    for( int j=0; j<output_images_width; j++ )
                    {
                        int argmax_i, argmax_j;
                        output_image_kl(i,j) = max(
                            input_image_kl.subMat(i*kernel_length,
                                                   j*kernel_width,
                                                   kernel_length,
                                                   kernel_width),
                            argmax_i, argmax_j );
                        argmax_kl(i,j) = argmax_i*input_images_width+argmax_j;
                    }
            }
    }
    else
        PLCHECK_MSG( false, "Unknown port configuration" );
}

/////////////////
// bpropUpdate //
/////////////////


void MaxSubsampling2DModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                            const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_value.length() == nPorts()
              && ports_gradient.length() == nPorts());
    // check which ports are input
    // (ports_value[i] && !ports_value[i]->isEmpty())
    // which ports are output (ports_value[i] && ports_value[i]->isEmpty())
    // and which ports are ignored (!ports_value[i]).
    // A similar logic applies to ports_gradients (to know whether gradient
    // is coming into the module of coming from the module through a given
    // ports_gradient[i]).
    // An input port_value should correspond to an outgoing port_gradient,
    // an output port_value could either correspond to an incoming
    // port_gradient (when that gradient is to be propagated inside and to the
    // input ports) or it should be null (no gradient is propagated from that
    // output port).

    Mat* input = ports_value[0];
    Mat* output = ports_value[1];
    Mat* argmax = ports_value[2];
    Mat* input_grad = ports_gradient[0];
    Mat* output_grad = ports_gradient[1];
    Mat* argmax_grad = ports_gradient[2];

    // If we want input_grad and we have output_grad
    if( input_grad && input_grad->isEmpty()
        && output_grad && !output_grad->isEmpty() )
    {
        PLASSERT( input );
        PLASSERT( output );
        PLASSERT( argmax );
        PLASSERT( !argmax_grad );

        PLASSERT( input->width() == port_sizes(0,1) );
        PLASSERT( output->width() == port_sizes(1,1) );
        PLASSERT( argmax->width() == port_sizes(2,1) );
        PLASSERT( input_grad->width() == port_sizes(0,1) );
        PLASSERT( output_grad->width() == port_sizes(1,1) );

        int batch_size = input->length();
        PLASSERT( output->length() == batch_size );
        PLASSERT( argmax->length() == batch_size );
        PLASSERT( output_grad->length() == batch_size );

        input_grad->resize(batch_size, port_sizes(0,1));

        for( int k=0; k<batch_size; k++ )
            for( int l=0; l<n_input_images; l++ )
            {
                Mat input_grad_image_kl = (*input_grad)(k)
                    .subVec(l*input_images_size, input_images_size)
                    .toMat(input_images_length, input_images_width);
                Mat output_grad_image_kl = (*output_grad)(k)
                    .subVec(l*output_images_size, output_images_size)
                    .toMat(output_images_length, output_images_width);
                Mat argmax_kl = (*argmax)(k)
                    .subVec(l*output_images_size, output_images_size)
                    .toMat(output_images_length, output_images_width);

                for( int i=0; i<output_images_length; i++ )
                    for( int j=0; j<output_images_width; j++ )
                    {
                        Mat input_grad_zone = input_grad_image_kl
                            .subMat(i*kernel_length, j*kernel_width,
                                    kernel_length, kernel_width);

                        int argmax = (int) round(argmax_kl(i,j));
                        input_grad_zone.data()[argmax] =
                            output_grad_image_kl(i,j);
                    }
            }
    }
    else
        PLERROR("In MaxSubsampling2DModule::bpropAccUpdate - this configuration of ports not implemented for class "
            "'%s'", classname().c_str());
}


////////////
// forget //
////////////
void MaxSubsampling2DModule::forget()
{
}

//////////////
// finalize //
//////////////
/* THIS METHOD IS OPTIONAL
void MaxSubsampling2DModule::finalize()
{
}
*/

//////////////////////
// bpropDoesNothing //
//////////////////////
/* THIS METHOD IS OPTIONAL
// the default implementation returns false
bool MaxSubsampling2DModule::bpropDoesNothing()
{
}
*/

/////////////////////
// setLearningRate //
/////////////////////
void MaxSubsampling2DModule::setLearningRate(real dynamic_learning_rate)
{
    // Do nothing.
}

//////////////
// getPorts //
//////////////
const TVec<string>& MaxSubsampling2DModule::getPorts()
{
    return ports;
}

//////////////////
// getPortSizes //
//////////////////
const TMat<int>& MaxSubsampling2DModule::getPortSizes()
{
    return port_sizes;
}

}
// end of namespace PLearn


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
