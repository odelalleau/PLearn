// -*- C++ -*-

// RBMConv2DLLParameters.cc
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

/*! \file RBMConv2DLLParameters.cc */

#define PL_LOG_MODULE_NAME "RBMConv2DLLParameters"
#include <plearn/io/pl_log.h>

#include "RBMConv2DLLParameters.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/math/convolutions.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMConv2DLLParameters,
    "Filter between two linear layers of a 2D convolutional RBM",
    "");

RBMConv2DLLParameters::RBMConv2DLLParameters( real the_learning_rate ) :
    inherited(the_learning_rate),
    momentum(0.)
{
}

RBMConv2DLLParameters::RBMConv2DLLParameters( string down_types,
                                              string up_types,
                                              real the_learning_rate ) :
    inherited( down_types, up_types, the_learning_rate ),
    momentum(0.)
{
    // We're not sure inherited::build() has been called
    build();
}

void RBMConv2DLLParameters::declareOptions(OptionList& ol)
{
    declareOption(ol, "momentum", &RBMConv2DLLParameters::momentum,
                  OptionBase::buildoption,
                  "Momentum factor (should be between 0 and 1)");

    declareOption(ol, "down_image_length",
                  &RBMConv2DLLParameters::down_image_length,
                  OptionBase::buildoption,
                  "Length of the down image");

    declareOption(ol, "down_image_width",
                  &RBMConv2DLLParameters::down_image_width,
                  OptionBase::buildoption,
                  "Width of the down image");

    declareOption(ol, "up_image_length",
                  &RBMConv2DLLParameters::up_image_length,
                  OptionBase::buildoption,
                  "Length of the up image");

    declareOption(ol, "up_image_width",
                  &RBMConv2DLLParameters::up_image_width,
                  OptionBase::buildoption,
                  "Width of the up image");

    declareOption(ol, "kernel_step1", &RBMConv2DLLParameters::kernel_step1,
                  OptionBase::buildoption,
                  "\"Vertical\" step of the convolution");

    declareOption(ol, "kernel_step2", &RBMConv2DLLParameters::kernel_step2,
                  OptionBase::buildoption,
                  "\"Horizontal\" step of the convolution");

    declareOption(ol, "kernel", &RBMConv2DLLParameters::kernel,
                  OptionBase::learntoption,
                  "Matrix containing the convolution kernel (filter)");

    declareOption(ol, "up_units_bias",
                  &RBMConv2DLLParameters::up_units_bias,
                  OptionBase::learntoption,
                  "Element i contains the bias of up unit i");

    declareOption(ol, "down_units_bias",
                  &RBMConv2DLLParameters::down_units_bias,
                  OptionBase::learntoption,
                  "Element i contains the bias of down unit i");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMConv2DLLParameters::build_()
{
    MODULE_LOG << "build_() called" << endl;
    if( up_layer_size == 0 || down_layer_size == 0 )
    {
        MODULE_LOG << "build_() aborted" << endl;
        return;
    }

    assert( down_image_length > 0 );
    assert( down_image_width > 0 );
    assert( down_image_length * down_image_width == down_layer_size );
    assert( up_image_length > 0 );
    assert( up_image_width > 0 );
    assert( up_image_length * up_image_width == up_layer_size );
    assert( kernel_step1 > 0 );
    assert( kernel_step2 > 0 );

    kernel_length = down_image_length - kernel_step1 * (up_image_length-1);
    assert( kernel_length > 0 );
    kernel_width = down_image_width - kernel_step2 * (up_image_width-1);
    assert( kernel_width > 0 );

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

    down_units_bias.resize( down_layer_size );
    down_units_bias_pos_stats.resize( down_layer_size );
    down_units_bias_neg_stats.resize( down_layer_size );
    for( int i=0 ; i<down_layer_size ; i++ )
    {
        char dut_i = down_units_types[i];
        if( dut_i != 'l' ) // not linear activation unit
            PLERROR( "RBMConv2DLLParameters::build_() - value '%c' for"
                     " down_units_types[%d]\n"
                     "should be 'l'.\n",
                     dut_i, i );
    }

    up_units_bias.resize( up_layer_size );
    up_units_bias_pos_stats.resize( up_layer_size );
    up_units_bias_neg_stats.resize( up_layer_size );
    for( int i=0 ; i<up_layer_size ; i++ )
    {
        char uut_i = up_units_types[i];
        if( uut_i != 'l' ) // not linear activation unit
            PLERROR( "RBMConv2DLLParameters::build_() - value '%c' for"
                     " up_units_types[%d]\n"
                     "should be 'l'.\n",
                     uut_i, i );
    }

    if( momentum != 0. )
    {
        kernel_inc.resize( kernel_length, kernel_width );
        down_units_bias_inc.resize( down_layer_size );
        up_units_bias_inc.resize( up_layer_size );
    }

    if( needs_forget )
        forget();

    clearStats();
}

void RBMConv2DLLParameters::build()
{
    inherited::build();
    build_();
}


void RBMConv2DLLParameters::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(kernel, copies);
    deepCopyField(up_units_bias, copies);
    deepCopyField(down_units_bias, copies);
    deepCopyField(kernel_pos_stats, copies);
    deepCopyField(kernel_neg_stats, copies);
    deepCopyField(kernel_gradient, copies);
    deepCopyField(up_units_bias_pos_stats, copies);
    deepCopyField(up_units_bias_neg_stats, copies);
    deepCopyField(down_units_bias_pos_stats, copies);
    deepCopyField(down_units_bias_neg_stats, copies);
    deepCopyField(kernel_inc, copies);
    deepCopyField(up_units_bias_inc, copies);
    deepCopyField(down_units_bias_inc, copies);
    deepCopyField(down_image, copies);
    deepCopyField(up_image, copies);
    deepCopyField(down_image_gradient, copies);
    deepCopyField(up_image_gradient, copies);
}

void RBMConv2DLLParameters::accumulatePosStats( const Vec& down_values,
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

    down_units_bias_pos_stats += down_values;
    up_units_bias_pos_stats += up_values;

    pos_count++;
}

void RBMConv2DLLParameters::accumulateNegStats( const Vec& down_values,
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

    down_units_bias_neg_stats += down_values;
    up_units_bias_neg_stats += up_values;

    neg_count++;
}

void RBMConv2DLLParameters::update()
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

    // down_units_bias -= learning_rate * (down_units_bias_pos_stats/pos_count
    //                                    -down_units_bias_neg_stats/neg_count)
    real* dub = down_units_bias.data();
    real* dubps = down_units_bias_pos_stats.data();
    real* dubns = down_units_bias_neg_stats.data();

    if( momentum == 0. )
    {
        // no need to use down_units_bias_inc
        for( int i=0 ; i<down_layer_size ; i++ )
            dub[i] += pos_factor * dubps[i] + neg_factor * dubns[i];
    }
    else
    {
        // ensure that down_units_bias_inc has the right size
        down_units_bias_inc.resize( down_layer_size );

        // The update rule becomes:
        // down_units_bias_inc =
        //      momentum * down_units_bias_inc
        //      - learning_rate * (down_units_bias_pos_stats/pos_count
        //                         -down_units_bias_neg_stats/neg_count);
        // down_units_bias += down_units_bias_inc;
        real* dubinc = down_units_bias_inc.data();
        for( int i=0 ; i<down_layer_size ; i++ )
        {
            dubinc[i] = momentum * dubinc[i]
                + pos_factor * dubps[i] + neg_factor * dubns[i];
            dub[i] += dubinc[i];
        }
    }

    // up_units_bias -= learning_rate * (up_units_bias_pos_stats/pos_count
    //                                   -up_units_bias_neg_stats/neg_count)
    real* uub = up_units_bias.data();
    real* uubps = up_units_bias_pos_stats.data();
    real* uubns = up_units_bias_neg_stats.data();
    if( momentum == 0. )
    {
        // no need to use up_units_bias_inc
        for( int i=0 ; i<up_layer_size ; i++ )
            uub[i] += pos_factor * uubps[i] + neg_factor * uubns[i];
    }
    else
    {
        // ensure that up_units_bias_inc has the right size
        up_units_bias_inc.resize( up_layer_size );

        // The update rule becomes:
        // up_units_bias_inc =
        //      momentum * up_units_bias_inc
        //      - learning_rate * (up_units_bias_pos_stats/pos_count
        //                         -up_units_bias_neg_stats/neg_count);
        // up_units_bias += up_units_bias_inc;
        real* uubinc = up_units_bias_inc.data();
        for( int i=0 ; i<up_layer_size ; i++ )
        {
            uubinc[i] = momentum * uubinc[i]
                + pos_factor * uubps[i] + neg_factor * uubns[i];
            uub[i] += uubinc[i];
        }
    }

    clearStats();
}

// Instead of using the statistics, we assume we have only one markov chain
// runned and we update the parameters from the first 4 values of the chain
void RBMConv2DLLParameters::update( const Vec& pos_down_values, // v_0
                                    const Vec& pos_up_values,   // h_0
                                    const Vec& neg_down_values, // v_1
                                    const Vec& neg_up_values )  // h_1
{
    assert( pos_up_values.length() == up_layer_size );
    assert( neg_up_values.length() == up_layer_size );
    assert( pos_down_values.length() == down_layer_size );
    assert( neg_down_values.length() == down_layer_size );

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

    // down_units_bias -= learning_rate * ( v_0 - v_1 )

    real* dub = down_units_bias.data();
    // pdv and ndv didn't change since last time
    // real* pdv = pos_down_values.data();
    // real* ndv = neg_down_values.data();

    if( momentum == 0. )
    {
        // no need to use down_units_bias_inc
        for( int j=0 ; j<down_layer_size ; j++ )
            dub[j] += learning_rate * ( ndv[j] - pdv[j] );
    }
    else
    {
        // ensure that down_units_bias_inc has the right size
        down_units_bias_inc.resize( down_layer_size );

        // The update rule becomes:
        // down_units_bias_inc = momentum * down_units_bias_inc
        //                       - learning_rate * ( v_0 - v_1 )
        // down_units_bias += down_units_bias_inc;

        real* dubinc = down_units_bias_inc.data();
        for( int j=0 ; j<down_layer_size ; j++ )
        {
            dubinc[j] = momentum * dubinc[j]
                + learning_rate * ( ndv[j] - pdv[j] );
            dub[j] += dubinc[j];
        }
    }

    // up_units_bias -= learning_rate * ( h_0 - h_1 )
    real* uub = up_units_bias.data();
    puv = pos_up_values.data();
    nuv = neg_up_values.data();

    if( momentum == 0. )
    {
        // no need to use up_units_bias_inc
        for( int i=0 ; i<up_layer_size ; i++ )
            uub[i] += learning_rate * (nuv[i] - puv[i] );
    }
    else
    {
        // ensure that up_units_bias_inc has the right size
        up_units_bias_inc.resize( up_layer_size );

        // The update rule becomes:
        // up_units_bias_inc =
        //      momentum * up_units_bias_inc
        //      - learning_rate * (up_units_bias_pos_stats/pos_count
        //                         -up_units_bias_neg_stats/neg_count);
        // up_units_bias += up_units_bias_inc;
        real* uubinc = up_units_bias_inc.data();
        for( int i=0 ; i<up_layer_size ; i++ )
        {
            uubinc[i] = momentum * uubinc[i]
                + learning_rate * ( nuv[i] - puv[i] );
            uub[i] += uubinc[i];
        }
    }
}

void RBMConv2DLLParameters::clearStats()
{
    kernel_pos_stats.clear();
    kernel_neg_stats.clear();

    down_units_bias_pos_stats.clear();
    down_units_bias_neg_stats.clear();

    up_units_bias_pos_stats.clear();
    up_units_bias_neg_stats.clear();

    pos_count = 0;
    neg_count = 0;
}

void RBMConv2DLLParameters::computeUnitActivations
    ( int start, int length, const Vec& activations ) const
{
    // Unoptimized way, that computes all the activations and return a subvec
    assert( activations.length() == length );
    if( going_up )
    {
        assert( start+length <= up_layer_size );
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
            activations[0] = act;
        }
        else if( start == 0 && length == up_layer_size )
        {
            up_image = activations.toMat( up_image_length, up_image_width );
            convolve2D( down_image, kernel, up_image,
                        kernel_step1, kernel_step2, false );
        }
        else
        {
            up_image = Mat( up_image_length, up_image_width );
            convolve2D( down_image, kernel, up_image,
                        kernel_step1, kernel_step2, false );
            activations << up_image.toVec().subVec( start, length );
        }
        activations += up_units_bias.subVec(start, length);
    }
    else
    {
        assert( start+length <= down_layer_size );
        up_image = input_vec.toMat( up_image_length, up_image_width );

        // special cases
        if( start == 0 && length == down_layer_size )
        {
            down_image = activations.toMat( down_image_length,
                                            down_image_width );
            backConvolve2D( down_image, kernel, up_image,
                            kernel_step1, kernel_step2, false );
        }
        else
        {
            down_image = Mat( down_image_length, down_image_width );
            backConvolve2D( down_image, kernel, up_image,
                            kernel_step1, kernel_step2, false );
            activations << down_image.toVec().subVec( start, length );
        }
        activations += down_units_bias.subVec(start, length);
    }
}

//! this version allows to obtain the input gradient as well
void RBMConv2DLLParameters::bpropUpdate(const Vec& input, const Vec& output,
                                        Vec& input_gradient,
                                        const Vec& output_gradient)
{
    assert( input.size() == down_layer_size );
    assert( output.size() == up_layer_size );
    assert( output_gradient.size() == up_layer_size );
    input_gradient.resize( down_layer_size );

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
                        kernel_step1, kernel_step2, false );

    // kernel -= learning_rate * kernel_gradient
    multiplyAcc( kernel, kernel_gradient, -learning_rate );

    // (up) bias -= learning_rate * output_gradient
    multiplyAcc( up_units_bias, output_gradient, -learning_rate );

}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void RBMConv2DLLParameters::forget()
{
    if( initialization_method == "zero" )
        kernel.clear();
    else
    {
        if( !random_gen )
            random_gen = new PRandom();

        real d = 1. / max( down_layer_size, up_layer_size );
        if( initialization_method == "uniform_sqrt" )
            d = sqrt( d );

        random_gen->fill_random_uniform( kernel, -d, d );
    }

    down_units_bias.clear();
    up_units_bias.clear();

    clearStats();
}


/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void RBMConv2DLLParameters::finalize()
{
}
*/

//! return the number of parameters
int RBMConv2DLLParameters::nParameters(bool share_up_params, bool share_down_params) const
{
    return kernel.size() + (share_up_params?up_units_bias.size():0) + 
        (share_down_params?down_units_bias.size():0);
}

//! Make the parameters data be sub-vectors of the given global_parameters.
//! The argument should have size >= nParameters. The result is a Vec
//! that starts just after this object's parameters end, i.e.
//!    result = global_parameters.subVec(nParameters(),global_parameters.size()-nParameters());
//! This allows to easily chain calls of this method on multiple RBMParameters.
Vec RBMConv2DLLParameters::makeParametersPointHere(const Vec& global_parameters, bool share_up_params, bool share_down_params)
{
    int n1=kernel.size();
    int n2=up_units_bias.size();
    int n3=down_units_bias.size();
    int n = n1+(share_up_params?n2:0)+(share_down_params?n3:0); // should be = nParameters()
    int m = global_parameters.size();
    if (m<n)
        PLERROR("RBMConv2DLLParameters::makeParametersPointHere: argument has length %d, should be longer than nParameters()=%d",m,n);
    real* p = global_parameters.data();
    kernel.makeSharedValue(p,n1);
    p+=n1;
    if (share_up_params)
    {
        up_units_bias.makeSharedValue(p,n2);
        p+=n2;
    }
    if (share_down_params)
        down_units_bias.makeSharedValue(p,n3);
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
