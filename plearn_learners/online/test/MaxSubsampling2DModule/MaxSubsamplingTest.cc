// -*- C++ -*-

// MaxSubsamplingTest.cc
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

/*! \file MaxSubsamplingTest.cc */


#include "MaxSubsamplingTest.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    MaxSubsamplingTest,
    "Tests MaxSubsampling2DModule",
    ""
);

//////////////////
// MaxSubsamplingTest //
//////////////////
MaxSubsamplingTest::MaxSubsamplingTest()
{
}

///////////
// build //
///////////
void MaxSubsamplingTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void MaxSubsamplingTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(max_module, copies);
    deepCopyField(random_gen, copies);
}

////////////////////
// declareOptions //
////////////////////
void MaxSubsamplingTest::declareOptions(OptionList& ol)
{
    declareOption(ol, "batch_size", &MaxSubsamplingTest::batch_size,
                  OptionBase::buildoption,
                  "Size of batch");

    declareOption(ol, "n_input_images", &MaxSubsamplingTest::n_input_images,
                  OptionBase::buildoption,
                  "Number of images in each input vector");

    declareOption(ol, "input_images_length",
                  &MaxSubsamplingTest::input_images_length,
                  OptionBase::buildoption,
                  "Length of input images");

    declareOption(ol, "input_images_width",
                  &MaxSubsamplingTest::input_images_width,
                  OptionBase::buildoption,
                  "Width of input images");

    declareOption(ol, "kernel_length", &MaxSubsamplingTest::kernel_length,
                  OptionBase::buildoption,
                  "Length of the subsampling zone");

    declareOption(ol, "kernel_width", &MaxSubsamplingTest::kernel_width,
                  OptionBase::buildoption,
                  "Width of the subsampling zone");

    declareOption(ol, "output_images_length",
                  &MaxSubsamplingTest::output_images_length,
                  OptionBase::learntoption,
                  "Length of output images");

    declareOption(ol, "output_images_width",
                  &MaxSubsamplingTest::output_images_width,
                  OptionBase::learntoption,
                  "Width of output images");

    declareOption(ol, "input_images_size",
                  &MaxSubsamplingTest::input_images_size,
                  OptionBase::learntoption,
                  "input_images_length*input_images_width");

    declareOption(ol, "output_images_size",
                  &MaxSubsamplingTest::output_images_size,
                  OptionBase::learntoption,
                  "output_images_length*output_images_width");

    declareOption(ol, "input_size",
                  &MaxSubsamplingTest::input_size,
                  OptionBase::learntoption,
                  "n_input_images*input_images_size");

    declareOption(ol, "output_size",
                  &MaxSubsamplingTest::output_size,
                  OptionBase::learntoption,
                  "n_input_images*output_images_size");

    declareOption(ol, "max_module", &MaxSubsamplingTest::max_module,
                  OptionBase::learntoption,
                  "The MaxSubsampling2DModule we build and test");
    /*
    declareOption(ol, "", &MaxSubsamplingTest::,
                  OptionBase::buildoption,
                  "");
    */

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void MaxSubsamplingTest::build_()
{
    PLCHECK( input_images_length % kernel_length == 0 );
    PLCHECK( input_images_width % kernel_width == 0 );

    output_images_length = input_images_length / kernel_length;
    output_images_width = input_images_width / kernel_width;

    input_images_size = input_images_length * input_images_width;
    output_images_size = output_images_length * output_images_width;

    input_size = n_input_images * input_images_size;
    output_size = n_input_images * output_images_size;

    if( !max_module )
    {
        max_module = new MaxSubsampling2DModule();
        max_module->n_input_images = n_input_images;
        max_module->input_images_length = input_images_length;
        max_module->input_images_width = input_images_width;
        max_module->kernel_length = kernel_length;
        max_module->kernel_width = kernel_width;
        max_module->build();
    }

    if (!random_gen)
        random_gen = new PRandom();
    random_gen->manual_seed(42);
}

/////////////
// perform //
/////////////
void MaxSubsamplingTest::perform()
{
    Mat in_(batch_size, input_size);
    Mat out_(batch_size, output_size);
    Mat argmax_(batch_size, output_size);

    TMat<Mat> in(batch_size, n_input_images);
    TMat<Mat> out(batch_size, n_input_images);
    TMat<Mat> argmax(batch_size, n_input_images);

    for( int k=0; k<batch_size; k++ )
        for( int i=0; i<n_input_images; i++ )
        {
            in(k,i) = in_(k).subVec(i*input_images_size, input_images_size)
                .toMat(input_images_length, input_images_width);
            random_gen->fill_random_uniform(in(k,i), -1, 1);

            out(k,i) = out_(k)
                .subVec(i*output_images_size, output_images_size)
                .toMat(output_images_length, output_images_width);

            argmax(k,i) = argmax_(k)
                .subVec( i*output_images_size, output_images_size)
                .toMat(output_images_length, output_images_width);
        }

    TVec<Mat*> ports_values(3);
    ports_values[0] = &in_;
    ports_values[1] = &out_;
    ports_values[2] = &argmax_;

    out_.resize(0, output_size);
    argmax_.resize(0, output_size);
    max_module->fprop(ports_values);


    Mat out_grad_(batch_size, output_size, 1.);
    Mat in_grad_(batch_size, input_size);
    TMat<Mat> in_grad(batch_size, n_input_images);

    for( int k=0; k<batch_size; k++ )
        for( int i=0; i<n_input_images; i++ )
        {
            in_grad(k,i) = in_grad_(k)
                .subVec(i*input_images_size, input_images_size)
                .toMat(input_images_length, input_images_width);
        }

    TVec<Mat*> ports_grad(3);
    ports_grad[0] = &in_grad_;
    ports_grad[1] = &out_grad_;
    ports_grad[2] = NULL;

    in_grad_.resize(0, input_size);
    max_module->bpropAccUpdate( ports_values, ports_grad );

    for( int k=0; k<batch_size; k++ )
        for( int i=0; i<n_input_images; i++ )
        {
            pout << "in("<<k<<","<<i<<") = " << endl << in(k,i) << endl;
            pout << "out("<<k<<","<<i<<") = " << endl << out(k,i) << endl;
            pout << "argmax("<<k<<","<<i<<") = " << endl
                << argmax(k,i) << endl;
            pout << "in_grad("<<k<<","<<i<<") = " << endl
                << in_grad(k,i) << endl;
        }

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
