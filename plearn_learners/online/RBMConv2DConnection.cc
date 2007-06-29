// -*- C++ -*-

// RBMConv2DConnection.cc
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

/*! \file RBMConv2DConnection.cc */

#define PL_LOG_MODULE_NAME "RBMConv2DConnection"
#include <plearn/io/pl_log.h>

#include "RBMConv2DConnection.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/math/convolutions.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMConv2DConnection,
    "Filter between two linear layers of a 2D convolutional RBM",
    "");

RBMConv2DConnection::RBMConv2DConnection( real the_learning_rate ) :
    inherited(the_learning_rate),
    down_image_length(-1),
    down_image_width(-1),
    up_image_length(-1),
    up_image_width(-1),
    kernel_step1(1),
    kernel_step2(1),
    kernel_length(-1),
    kernel_width(-1)
{
}

void RBMConv2DConnection::declareOptions(OptionList& ol)
{
    declareOption(ol, "down_image_length",
                  &RBMConv2DConnection::down_image_length,
                  OptionBase::buildoption,
                  "Length of the down image");

    declareOption(ol, "down_image_width",
                  &RBMConv2DConnection::down_image_width,
                  OptionBase::buildoption,
                  "Width of the down image");

    declareOption(ol, "up_image_length",
                  &RBMConv2DConnection::up_image_length,
                  OptionBase::buildoption,
                  "Length of the up image");

    declareOption(ol, "up_image_width",
                  &RBMConv2DConnection::up_image_width,
                  OptionBase::buildoption,
                  "Width of the up image");

    declareOption(ol, "kernel_step1", &RBMConv2DConnection::kernel_step1,
                  OptionBase::buildoption,
                  "\"Vertical\" step of the convolution");

    declareOption(ol, "kernel_step2", &RBMConv2DConnection::kernel_step2,
                  OptionBase::buildoption,
                  "\"Horizontal\" step of the convolution");

    declareOption(ol, "kernel", &RBMConv2DConnection::kernel,
                  OptionBase::learntoption,
                  "Matrix containing the convolution kernel (filter)");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "down_size",
                    &RBMConv2DConnection::down_size,
                    OptionBase::learntoption,
                    "Equals to down_image_length × down_image_width");

    redeclareOption(ol, "up_size",
                    &RBMConv2DConnection::up_size,
                    OptionBase::learntoption,
                    "Equals to up_image_length × up_image_width");
}

void RBMConv2DConnection::build_()
{
    MODULE_LOG << "build_() called" << endl;

    down_size = down_image_length * down_image_width;
    up_size = up_image_length * up_image_width;

    PLASSERT( down_image_length > 0 );
    PLASSERT( down_image_width > 0 );
    PLASSERT( down_image_length * down_image_width == down_size );
    PLASSERT( up_image_length > 0 );
    PLASSERT( up_image_width > 0 );
    PLASSERT( up_image_length * up_image_width == up_size );
    PLASSERT( kernel_step1 > 0 );
    PLASSERT( kernel_step2 > 0 );

    kernel_length = down_image_length - kernel_step1 * (up_image_length-1);
    PLASSERT( kernel_length > 0 );
    kernel_width = down_image_width - kernel_step2 * (up_image_width-1);
    PLASSERT( kernel_width > 0 );

    output_size = 0;
    bool needs_forget = false; // do we need to reinitialize the parameters?

    if( kernel.length() != kernel_length ||
        kernel.width() != kernel_width )
    {
        kernel.resize( kernel_length, kernel_width );
        needs_forget = true;
    }

    kernel_pos_stats.resize( kernel_length, kernel_width );
    kernel_neg_stats.resize( kernel_length, kernel_width );
    kernel_gradient.resize( kernel_length, kernel_width );

    if( momentum != 0. )
        kernel_inc.resize( kernel_length, kernel_width );

    if( needs_forget )
        forget();

    clearStats();
}

void RBMConv2DConnection::build()
{
    inherited::build();
    build_();
}


void RBMConv2DConnection::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(kernel, copies);
    deepCopyField(kernel_pos_stats, copies);
    deepCopyField(kernel_neg_stats, copies);
    deepCopyField(kernel_gradient, copies);
    deepCopyField(kernel_inc, copies);
    deepCopyField(down_image, copies);
    deepCopyField(up_image, copies);
    deepCopyField(down_image_gradient, copies);
    deepCopyField(up_image_gradient, copies);
}

void RBMConv2DConnection::accumulatePosStats( const Vec& down_values,
                                              const Vec& up_values )
{
    down_image = down_values.toMat( down_image_length, down_image_width );
    up_image = up_values.toMat( up_image_length, up_image_width );

    /*  for i=0 to up_image_length:
     *   for j=0 to up_image_width:
     *     for l=0 to kernel_length:
     *       for m=0 to kernel_width:
     *         kernel_pos_stats(l,m) +=
     *           down_image(step1*i+l,step2*j+m) * up_image(i,j)
     */
    convolve2Dbackprop( down_image, up_image, kernel_pos_stats,
                        kernel_step1, kernel_step2, true );

    pos_count++;
}

void RBMConv2DConnection::accumulateNegStats( const Vec& down_values,
                                              const Vec& up_values )
{
    down_image = down_values.toMat( down_image_length, down_image_width );
    up_image = up_values.toMat( up_image_length, up_image_width );
    /*  for i=0 to up_image_length:
     *   for j=0 to up_image_width:
     *     for l=0 to kernel_length:
     *       for m=0 to kernel_width:
     *         kernel_neg_stats(l,m) +=
     *           down_image(step1*i+l,step2*j+m) * up_image(i,j)
     */
    convolve2Dbackprop( down_image, up_image, kernel_neg_stats,
                        kernel_step1, kernel_step2, true );

    neg_count++;
}

void RBMConv2DConnection::update()
{
    // updates parameters
    // kernel -= learning_rate * (kernel_pos_stats/pos_count
    //                              - kernel_neg_stats/neg_count)
    real pos_factor = -learning_rate / pos_count;
    real neg_factor = learning_rate / neg_count;

    real* k_i = kernel.data();
    real* kps_i = kernel_pos_stats.data();
    real* kns_i = kernel_neg_stats.data();
    int k_mod = kernel.mod();
    int kps_mod = kernel_pos_stats.mod();
    int kns_mod = kernel_neg_stats.mod();

    if( momentum == 0. )
    {
        // no need to use weights_inc
        for( int i=0 ; i<kernel_length ; i++, k_i+=k_mod,
                                         kps_i+=kps_mod, kns_i+=kns_mod )
            for( int j=0 ; j<kernel_width ; j++ )
                k_i[j] += pos_factor * kps_i[j] + neg_factor * kns_i[j];
    }
    else
    {
        // ensure that weights_inc has the right size
        kernel_inc.resize( kernel_length, kernel_width );

        // The update rule becomes:
        // kernel_inc = momentum * kernel_inc
        //               - learning_rate * (kernel_pos_stats/pos_count
        //                                  - kernel_neg_stats/neg_count);
        // kernel += kernel_inc;
        real* kinc_i = kernel_inc.data();
        int kinc_mod = kernel_inc.mod();
        for( int i=0 ; i<kernel_length ; i++, k_i += k_mod, kps_i += kps_mod,
                                         kns_i += kns_mod, kinc_i += kinc_mod )
            for( int j=0 ; j<kernel_width ; j++ )
            {
                kinc_i[j] = momentum * kinc_i[j]
                    + pos_factor * kps_i[j] + neg_factor * kns_i[j];
                k_i[j] += kinc_i[j];
            }
    }

    clearStats();
}

// Instead of using the statistics, we assume we have only one markov chain
// runned and we update the parameters from the first 4 values of the chain
void RBMConv2DConnection::update( const Vec& pos_down_values, // v_0
                                  const Vec& pos_up_values,   // h_0
                                  const Vec& neg_down_values, // v_1
                                  const Vec& neg_up_values )  // h_1
{
    PLASSERT( pos_up_values.length() == up_size );
    PLASSERT( neg_up_values.length() == up_size );
    PLASSERT( pos_down_values.length() == down_size );
    PLASSERT( neg_down_values.length() == down_size );

    /*  for i=0 to up_image_length:
     *   for j=0 to up_image_width:
     *     for l=0 to kernel_length:
     *       for m=0 to kernel_width:
     *         kernel_neg_stats(l,m) -= learning_rate *
     *           ( pos_down_image(step1*i+l,step2*j+m) * pos_up_image(i,j)
     *             - neg_down_image(step1*i+l,step2*j+m) * neg_up_image(i,j) )
     */

    real* puv = pos_up_values.data();
    real* nuv = neg_up_values.data();
    real* pdv = pos_down_values.data();
    real* ndv = neg_down_values.data();
    int k_mod = kernel.mod();

    if( momentum == 0. )
    {
        for( int i=0; i<up_image_length; i++,
                                         puv+=up_image_width,
                                         nuv+=up_image_width,
                                         pdv+=kernel_step1*down_image_width,
                                         ndv+=kernel_step1*down_image_width )
        {
            // copies to iterate over columns
            real* pdv1 = pdv;
            real* ndv1 = ndv;
            for( int j=0; j<up_image_width; j++,
                                            pdv1+=kernel_step2,
                                            ndv1+=kernel_step2 )
            {
                real* k = kernel.data();
                real* pdv2 = pdv1; // copy to iterate over sub-rows
                real* ndv2 = ndv1;
                real puv_ij = puv[j];
                real nuv_ij = nuv[j];
                for( int l=0; l<kernel_length; l++, k+=k_mod,
                                               pdv2+=down_image_width,
                                               ndv2+=down_image_width )
                    for( int m=0; m<kernel_width; m++ )
                        k[m] += learning_rate *
                            (ndv2[m] * nuv_ij - pdv2[m] * puv_ij);
            }
        }
    }
    else
    {
        // ensure that weights_inc has the right size
        kernel_inc.resize( kernel_length, kernel_width );
        kernel_inc *= momentum;

        int kinc_mod = kernel_inc.mod();
        for( int i=0; i<down_image_length; i++,
                                           puv+=up_image_width,
                                           nuv+=up_image_width,
                                           pdv+=kernel_step1*down_image_width,
                                           ndv+=kernel_step1*down_image_width )
        {
            // copies to iterate over columns
            real* pdv1 = pdv;
            real* ndv1 = ndv;
            for( int j=0; j<down_image_width; j++,
                                              pdv1+=kernel_step2,
                                              ndv1+=kernel_step2 )
            {
                real* kinc = kernel_inc.data();
                real* pdv2 = pdv1; // copy to iterate over sub-rows
                real* ndv2 = ndv1;
                real puv_ij = puv[j];
                real nuv_ij = nuv[j];
                for( int l=0; l<kernel_length; l++, kinc+=kinc_mod,
                                               pdv2+=down_image_width,
                                               ndv2+=down_image_width )
                    for( int m=0; m<kernel_width; m++ )
                        kinc[m] += ndv2[m] * nuv_ij - pdv2[m] * puv_ij;
            }
        }
        multiplyAcc( kernel, kernel_inc, learning_rate );
    }
}

void RBMConv2DConnection::update( const Mat& pos_down_values, // v_0
                                  const Mat& pos_up_values,   // h_0
                                  const Mat& neg_down_values, // v_1
                                  const Mat& neg_up_values )  // h_1
{
    PLASSERT( pos_up_values.width() == up_size );
    PLASSERT( neg_up_values.width() == up_size );
    PLASSERT( pos_down_values.width() == down_size );
    PLASSERT( neg_down_values.width() == down_size );

    int batch_size = pos_down_values.length();
    PLASSERT( pos_up_values.length() == batch_size );
    PLASSERT( neg_down_values.length() == batch_size );
    PLASSERT( neg_up_values.length() == batch_size );

    real norm_lr = learning_rate / batch_size;

    /*  for i=0 to up_image_length:
     *   for j=0 to up_image_width:
     *     for l=0 to kernel_length:
     *       for m=0 to kernel_width:
     *         kernel_neg_stats(l,m) -= learning_rate *
     *           ( pos_down_image(step1*i+l,step2*j+m) * pos_up_image(i,j)
     *             - neg_down_image(step1*i+l,step2*j+m) * neg_up_image(i,j) )
     */

    if( momentum == 0. )
    {
        for( int b=0; b<batch_size; b++ )
        {
            real* puv = pos_up_values(b).data();
            real* nuv = neg_up_values(b).data();
            real* pdv = pos_down_values(b).data();
            real* ndv = neg_down_values(b).data();
            int k_mod = kernel.mod();

            for( int i=0; i<up_image_length;
                 i++,
                 puv+=up_image_width,
                 nuv+=up_image_width,
                 pdv+=kernel_step1*down_image_width,
                 ndv+=kernel_step1*down_image_width )
            {
                // copies to iterate over columns
                real* pdv1 = pdv;
                real* ndv1 = ndv;
                for( int j=0; j<up_image_width; j++,
                                                pdv1+=kernel_step2,
                                                ndv1+=kernel_step2 )
                {
                    real* k = kernel.data();
                    real* pdv2 = pdv1; // copy to iterate over sub-rows
                    real* ndv2 = ndv1;
                    real puv_ij = puv[j];
                    real nuv_ij = nuv[j];
                    for( int l=0; l<kernel_length; l++, k+=k_mod,
                                                   pdv2+=down_image_width,
                                                   ndv2+=down_image_width )
                        for( int m=0; m<kernel_width; m++ )
                            k[m] += norm_lr *
                                (ndv2[m] * nuv_ij - pdv2[m] * puv_ij);
                }
            }
        }
    }
    else
        PLCHECK_MSG(false,
                    "mini-batch and momentum don't work together yet");
}

void RBMConv2DConnection::clearStats()
{
    kernel_pos_stats.clear();
    kernel_neg_stats.clear();

    pos_count = 0;
    neg_count = 0;
}

void RBMConv2DConnection::computeProduct
    ( int start, int length, const Vec& activations, bool accumulate ) const
{
    // Unoptimized way, that computes all the activations and return a subvec
    PLASSERT( activations.length() == length );
    if( going_up )
    {
        PLASSERT( start+length <= up_size );
        down_image = input_vec.toMat( down_image_length, down_image_width );

        // special cases:
        if( length == 1 )
        {
            real act = 0;
            real* k = kernel.data();
            real* di = down_image.data()
                        + kernel_step1*(start / down_image_width)
                        + kernel_step2*(start % down_image_width);
            for( int l=0; l<kernel_length; l++, di+=down_image_width )
                for( int m=0; m<kernel_width; m++ )
                    act += di[m] * k[m];
            if( accumulate )
                activations[0] += act;
            else
                activations[0] = act;
        }
        else if( start == 0 && length == up_size )
        {
            up_image = activations.toMat( up_image_length, up_image_width );
            convolve2D( down_image, kernel, up_image,
                        kernel_step1, kernel_step2, accumulate );
        }
        else
        {
            up_image = Mat( up_image_length, up_image_width );
            convolve2D( down_image, kernel, up_image,
                        kernel_step1, kernel_step2, false );
            if( accumulate )
                activations += up_image.toVec().subVec( start, length );
            else
                activations << up_image.toVec().subVec( start, length );
        }
    }
    else
    {
        PLASSERT( start+length <= down_size );
        up_image = input_vec.toMat( up_image_length, up_image_width );

        // special cases
        if( start == 0 && length == down_size )
        {
            down_image = activations.toMat( down_image_length,
                                            down_image_width );
            backConvolve2D( down_image, kernel, up_image,
                            kernel_step1, kernel_step2, accumulate );
        }
        else
        {
            down_image = Mat( down_image_length, down_image_width );
            backConvolve2D( down_image, kernel, up_image,
                            kernel_step1, kernel_step2, false );
            if( accumulate )
                activations += down_image.toVec().subVec( start, length );
            else
                activations << down_image.toVec().subVec( start, length );
        }
    }
}

void RBMConv2DConnection::computeProducts(int start, int length,
                                          Mat& activations,
                                          bool accumulate) const
{
    PLASSERT( activations.width() == length );
    int batch_size = inputs_mat.length();
    activations.resize( batch_size, length);
    if( going_up )
    {
        PLASSERT( start+length <= up_size );
        // usual case
        if( start == 0 && length == up_size )
            for( int k=0; k<batch_size; k++ )
            {
                up_image = activations(k)
                    .toMat(up_image_length, up_image_width);
                down_image = inputs_mat(k)
                    .toMat(down_image_length, down_image_width);

                convolve2D(down_image, kernel, up_image,
                           kernel_step1, kernel_step2, accumulate);
            }
        else
            PLCHECK_MSG(false,
                        "Unusual case of use (start!=0 or length!=up_size)\n"
                        "not implemented yet.");
    }
    else
    {
        PLASSERT( start+length <= down_size );
        // usual case
        if( start == 0 && length == down_size )
            for( int k=0; k<batch_size; k++ )
            {
                up_image = inputs_mat(k)
                    .toMat(up_image_length, up_image_width);
                down_image = activations(k)
                    .toMat(down_image_length, down_image_width);

                backConvolve2D(down_image, kernel, up_image,
                               kernel_step1, kernel_step2, accumulate);
            }
        else
            PLCHECK_MSG(false,
                        "Unusual case of use (start!=0 or length!=down_size)\n"
                        "not implemented yet.");
    }
}

//! this version allows to obtain the input gradient as well
void RBMConv2DConnection::bpropUpdate(const Vec& input, const Vec& output,
                                      Vec& input_gradient,
                                      const Vec& output_gradient,
                                      bool accumulate)
{
    PLASSERT( input.size() == down_size );
    PLASSERT( output.size() == up_size );
    PLASSERT( output_gradient.size() == up_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == down_size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
        input_gradient.resize( down_size );

    down_image = input.toMat( down_image_length, down_image_width );
    up_image = output.toMat( up_image_length, up_image_width );
    down_image_gradient = input_gradient.toMat( down_image_length,
                                                down_image_width );
    up_image_gradient = output_gradient.toMat( up_image_length,
                                               up_image_width );

    // update input_gradient and kernel_gradient
    convolve2Dbackprop( down_image, kernel,
                        up_image_gradient, down_image_gradient,
                        kernel_gradient,
                        kernel_step1, kernel_step2, accumulate );

    // kernel -= learning_rate * kernel_gradient
    multiplyAcc( kernel, kernel_gradient, -learning_rate );
}

void RBMConv2DConnection::bpropUpdate(const Mat& inputs, const Mat& outputs,
                                      Mat& input_gradients,
                                      const Mat& output_gradients,
                                      bool accumulate)
{
    PLASSERT( inputs.width() == down_size );
    PLASSERT( outputs.width() == up_size );
    PLASSERT( output_gradients.width() == up_size );

    int batch_size = inputs.length();
    PLASSERT( outputs.length() == batch_size );
    PLASSERT( output_gradients.length() == batch_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == down_size &&
                      input_gradients.length() == batch_size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
    {
        input_gradients.resize(batch_size, down_size);
        input_gradients.clear();
    }

    kernel_gradient.clear();
    for( int k=0; k<batch_size; k++ )
    {
        down_image = inputs(k).toMat( down_image_length, down_image_width );
        up_image = outputs(k).toMat( up_image_length, up_image_width );
        down_image_gradient = input_gradients(k)
            .toMat( down_image_length, down_image_width );
        up_image_gradient = output_gradients(k)
            .toMat( up_image_length, up_image_width );

        // update input_gradient and kernel_gradient
        convolve2Dbackprop( down_image, kernel,
                            up_image_gradient, down_image_gradient,
                            kernel_gradient,
                            kernel_step1, kernel_step2, true );
    }

    // kernel -= learning_rate/n * kernel_gradient
    multiplyAcc( kernel, kernel_gradient, -learning_rate/batch_size );
}
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void RBMConv2DConnection::forget()
{
    clearStats();
    if( initialization_method == "zero" )
        kernel.clear();
    else
    {
        if( !random_gen )
        {
            PLWARNING( "RBMConv2DConnection: cannot forget() without"
                       " random_gen" );
            return;
        }

        real d = 1. / max( down_size, up_size );
        if( initialization_method == "uniform_sqrt" )
            d = sqrt( d );

        random_gen->fill_random_uniform( kernel, -d, d );
    }
}


/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void RBMConv2DConnection::finalize()
{
}
*/

//! return the number of parameters
int RBMConv2DConnection::nParameters() const
{
    return kernel.size();
}

//! Make the parameters data be sub-vectors of the given global_parameters.
//! The argument should have size >= nParameters. The result is a Vec
//! that starts just after this object's parameters end, i.e.
//!    result = global_parameters.subVec(nParameters(),global_parameters.size()-nParameters());
//! This allows to easily chain calls of this method on multiple RBMParameters.
Vec RBMConv2DConnection::makeParametersPointHere(const Vec& global_parameters)
{
    int n=kernel.size();
    int m = global_parameters.size();
    if (m<n)
        PLERROR("RBMConv2DConnection::makeParametersPointHere: argument has length %d, should be longer than nParameters()=%d",m,n);
    real* p = global_parameters.data();
    kernel.makeSharedValue(p,n);
    return global_parameters.subVec(n,m-n);
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
