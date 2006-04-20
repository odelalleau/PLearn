// -*- C++ -*-

// RBMGenericParameters.cc
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

/*! \file RBMGenericParameters.cc */



#include "RBMGenericParameters.h"
#include <plearn/math/TMat_maths.h>
//#include "RBMLayer.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMGenericParameters,
    "Stores and learns the parameters between two layers of an RBM",
    "");

RBMGenericParameters::RBMGenericParameters( real the_learning_rate )
    : inherited(),
      learning_rate(the_learning_rate)
{
}

RBMGenericParameters::RBMGenericParameters( string down_types, string up_types,
                                            real the_learning_rate )
    : inherited( down_types, up_types ),
      learning_rate( the_learning_rate )
{
    // We're not sure inherited::build() has been called
    build();
}

/*
RBMGenericParameters::RBMGenericParameters( PP<RBMLayer> down, PP<RBMLayer> up,
                                            real the_learning_rate )
    : inherited( down, up ),
      learning_rate( the_learning_rate )
{
    // We're not sure inherited::build() has been called
    build();
}
// */

void RBMGenericParameters::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "learning_rate", &RBMGenericParameters::learning_rate,
                  OptionBase::buildoption,
                  "Learning rate");

    declareOption(ol, "weights", &RBMGenericParameters::weights,
                  OptionBase::learntoption,
                  "Matrix containing unit-to-unit weights (output_size ×"
                  " input_size)");

    declareOption(ol, "up_units_params",
                  &RBMGenericParameters::up_units_params,
                  OptionBase::learntoption,
                  "Element i contains inner parameters (like the bias) of up"
                  " unit i");

    declareOption(ol, "down_units_params",
                  &RBMGenericParameters::down_units_params,
                  OptionBase::learntoption,
                  "Element i contains inner parameters (like the bias) of down"
                  " unit i");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMGenericParameters::build_()
{
    if( up_layer_size == 0 || down_layer_size == 0 )
        return;

    output_size = 0;
    bool needs_forget = false; // do we need to reinitialize the parameters?

    if( weights.length() != up_layer_size ||
        weights.width() != down_layer_size )
    {
        weights.resize( up_layer_size, down_layer_size );
        needs_forget = true;
    }

    weights_pos_stats.resize( up_layer_size, down_layer_size );
    weights_neg_stats.resize( up_layer_size, down_layer_size );

    down_units_params.resize( down_layer_size );
    down_units_params_pos_stats.resize( down_layer_size );
    down_units_params_neg_stats.resize( down_layer_size );
    for( int i=0 ; i<down_layer_size ; i++ )
    {
        char dut_i = down_units_types[i];
        if( dut_i == 'l' ) // linear activation unit
        {
            down_units_params[i].resize(1);
            down_units_params_pos_stats[i].resize(1);
            down_units_params_neg_stats[i].resize(1);
        }
        else if( dut_i == 'q' ) // quadratic
        {
            down_units_params[i].resize(2);
            down_units_params_pos_stats[i].resize(2);
            down_units_params_neg_stats[i].resize(2);
        }
        else
            PLERROR( "RBMGenericParameters::build_() - value '%c' for"
                     " down_units_types[%d]\n"
                     "is unknown. Supported values are 'l' and 'q'.\n",
                     dut_i, i );
    }

    up_units_params.resize( up_layer_size );
    up_units_params_pos_stats.resize( up_layer_size );
    up_units_params_neg_stats.resize( up_layer_size );
    for( int i=0 ; i<up_layer_size ; i++ )
    {
        char uut_i = up_units_types[i];
        if( uut_i == 'l' ) // linear activation unit
        {
            up_units_params[i].resize(1);
            up_units_params_pos_stats[i].resize(1);
            up_units_params_neg_stats[i].resize(1);
            output_size += 1;
        }
        else if( uut_i == 'q' )
        {
            up_units_params[i].resize(2);
            up_units_params_pos_stats[i].resize(2);
            up_units_params_neg_stats[i].resize(2);
            output_size += 2;
        }
        else
            PLERROR( "RBMGenericParameters::build_() - value '%c' for"
                     " up_units_types[%d]\n"
                     "is unknown. Supported values are 'l' and 'q'.\n",
                     uut_i, i );
    }

    if( needs_forget )
        forget();

    clearStats();
}

void RBMGenericParameters::build()
{
    inherited::build();
    build_();
}


void RBMGenericParameters::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.

    deepCopyField(weights, copies);
    deepCopyField(up_units_params, copies);
    deepCopyField(down_units_params, copies);
    deepCopyField(weights_pos_stats, copies);
    deepCopyField(weights_neg_stats, copies);
    deepCopyField(up_units_params_pos_stats, copies);
    deepCopyField(up_units_params_neg_stats, copies);
    deepCopyField(down_units_params_pos_stats, copies);
    deepCopyField(down_units_params_neg_stats, copies);
}

void RBMGenericParameters::accumulatePosStats( const Vec& down_values,
                                               const Vec& up_values )
{
    // weights_pos_stats += up_values * down_values'
    externalProductAcc( weights_pos_stats, up_values, down_values );

    for( int i=0 ; i<down_layer_size ; i++ )
    {
        // the bias is updated the same way for 'l' and 'g' units
        down_units_params_pos_stats[i][0] += down_values[i];

        // update also 'g' units' quadratic term
        if( down_units_types[i] == 'g' )
            down_units_params_pos_stats[i][1] +=
                2 * down_units_params[i][1] * down_values[i] * down_values[i];
    }

    for( int i=0 ; i<up_layer_size ; i++ )
    {
        // the bias is updated the same way for 'l' and 'g' units
        up_units_params_pos_stats[i][0] += up_values[i];

        // update also 'g' units' quadratic term
        if( up_units_types[i] == 'g' )
            up_units_params_pos_stats[i][1] +=
                2 * up_units_params[i][1] * up_values[i] * up_values[i];
    }

    pos_count++;
}

void RBMGenericParameters::accumulateNegStats( const Vec& down_values,
                                               const Vec& up_values )
{
    // weights_neg_stats += up_values * down_values'
    externalProductAcc( weights_neg_stats, up_values, down_values );

    for( int i=0 ; i<down_layer_size ; i++ )
    {
        // the bias is updated the same way for 'l' and 'g' units
        down_units_params_neg_stats[i][0] += down_values[i];

        // update also 'g' units' quadratic term
        if( down_units_types[i] == 'g' )
            down_units_params_neg_stats[i][1] +=
                2 * down_units_params[i][1] * down_values[i] * down_values[i];
    }

    for( int i=0 ; i<up_layer_size ; i++ )
    {
        // the bias is updated the same way for 'l' and 'g' units
        up_units_params_neg_stats[i][0] += up_values[i];

        // update also 'g' units' quadratic term
        if( up_units_types[i] == 'g' )
            up_units_params_neg_stats[i][1] +=
                2 * up_units_params[i][1] * up_values[i] * up_values[i];
    }

    neg_count++;
}

void RBMGenericParameters::update()
{
    real p_count = pos_count;
    real n_count = neg_count;
    // updates parameters
    weights -= real(learning_rate) * (weights_pos_stats/p_count
                                 - weights_neg_stats/n_count);

    for( int i=0 ; i<up_layer_size ; i++ )
    {
        up_units_params[i] -=
            learning_rate * (up_units_params_pos_stats[i]/p_count
                             - up_units_params_neg_stats[i]/n_count);
    }

    for( int i=0 ; i<down_layer_size ; i++ )
    {
        down_units_params[i] -=
            learning_rate * (down_units_params_pos_stats[i]/p_count
                             - down_units_params_neg_stats[i]/n_count);
    }

    clearStats();
}

void RBMGenericParameters::clearStats()
{
    weights_pos_stats.clear();
    weights_neg_stats.clear();
    for( int i=0 ; i<down_layer_size ; i++ )
    {
        down_units_params_pos_stats[i].clear();
        down_units_params_neg_stats[i].clear();
    }
    for( int i=0 ; i<up_layer_size ; i++ )
    {
        up_units_params_pos_stats[i].clear();
        up_units_params_neg_stats[i].clear();
    }
    pos_count = 0;
    neg_count = 0;
}

void RBMGenericParameters::computeLinearUnitActivations
    ( int i, const Vec& activations ) const
{
    assert( activations.length() == 1 );

    if( going_up )
    {
        assert( up_units_types[i] == 'l' );

        // activations[0] = sum_j weights(i,j) input_vec[j] + b[i]
        product( activations, weights.subMatRows(i,1), input_vec );
        activations[0] += up_units_params[i][0];
    }
    else
    {
        assert( down_units_types[i] == 'l' );

        // activations[0] = sum_j weights(j,i) input_vec[j] + b[i]
        transposeProduct( activations, weights.subMatColumns(i,1), input_vec );
        activations[0] += down_units_params[i][0];
    }
}

void RBMGenericParameters::computeQuadraticUnitActivations
    ( int i, const Vec& activations ) const
{
    assert( activations.length() == 2 );

    if( going_up )
    {
        assert( up_units_types[i] == 'q' );

        // activations[0] = -(sum_j weights(i,j) input_vec[j] + b[i])
        //                    / (2 * up_units_params[i][1]^2)
        product( activations, weights.subMatRows(i,1), input_vec );
        real a_i = up_units_params[i][1];
        activations[0] = -(activations[0] + up_units_params[i][0])
                           / (2 * a_i * a_i);

        // activations[1] = 1 / (2 * up_units_params[i][1]^2)
        activations[1] = 1. / (2. * a_i * a_i);
    }
    else
    {
        assert( down_units_types[i] == 'q' );

        // activations[0] = -(sum_j weights(j,i) input_vec[j] + b[i])
        //                    / (2 * down_units_params[i][1]^2)
        transposeProduct( activations, weights.subMatColumns(i,1), input_vec );
        real a_i = down_units_params[i][1];
        activations[0] = -(activations[0] + down_units_params[i][0])
                           / (2 * a_i * a_i);

        // activations[1] = 1 / (2 * down_units_params[i][1]^2)
        activations[1] = 1. / (2. * a_i * a_i);
    }
}


void RBMGenericParameters::computeUnitActivations
    ( int start, int length, const Vec& activations ) const
{
    string units_types;
    if( going_up )
        units_types = up_units_types;
    else
        units_types = down_units_types;

    assert( start+length <= (int) units_types.length() );
    int cur_pos; // position index inside activations

    for( int i=start ; i<start+length ; i++ )
    {
        char ut_i = units_types[i];
        if( ut_i == 'l' )
        {
            computeLinearUnitActivations( i, activations.subVec(cur_pos, 2) );
            cur_pos++;
        }
        else if( ut_i == 'q' )
        {
            computeQuadraticUnitActivations( i,
                                             activations.subVec(cur_pos, 2) );
            cur_pos += 2;
        }
        else
            PLERROR( "RBMGenericParameters::computeUnitActivations():\n"
                     "value '%c' for units_types[%d] is unknown.\n"
                     "Supported values are 'l' and 'q'.\n", ut_i, i );
    }
}

//! this version allows to obtain the input gradient as well
//! N.B. THE DEFAULT IMPLEMENTATION IN SUPER-CLASS JUST RAISES A PLERROR.
void RBMGenericParameters::bpropUpdate(const Vec& input, const Vec& output,
                                       Vec& input_gradient,
                                       const Vec& output_gradient)
{
    PLERROR( "not implemented yet" );
}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void RBMGenericParameters::forget()
{
    if( initialization_method == "zero" )
        weights.clear();
    else
    {
        if( !random_gen )
            random_gen = new PRandom();

        real d = 1. / max( down_layer_size, up_layer_size );
        if( initialization_method == "uniform_sqrt" )
            d = sqrt( d );

        random_gen->fill_random_uniform( weights, -d, d );
    }

    for( int i=0 ; i<down_layer_size ; i++ )
        down_units_params[i].clear();

    for( int i=0 ; i<up_layer_size ; i++ )
        up_units_params[i].clear();

    clearStats();
}

/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void RBMGenericParameters::finalize()
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
