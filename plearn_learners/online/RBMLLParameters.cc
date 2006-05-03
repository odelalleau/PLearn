// -*- C++ -*-

// RBMLLParameters.cc
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

/*! \file RBMLLParameters.cc */



#include "RBMLLParameters.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMLLParameters,
    "Stores and learns the parameters between two linear layers of an RBM",
    "");

RBMLLParameters::RBMLLParameters( real the_learning_rate )
    : inherited(),
      learning_rate(the_learning_rate)
{
}

RBMLLParameters::RBMLLParameters( string down_types, string up_types,
                                  real the_learning_rate )
    : inherited( down_types, up_types ),
      learning_rate( the_learning_rate )
{
    // We're not sure inherited::build() has been called
    build();
}

void RBMLLParameters::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "learning_rate", &RBMLLParameters::learning_rate,
                  OptionBase::buildoption,
                  "Learning rate");

    declareOption(ol, "weights", &RBMLLParameters::weights,
                  OptionBase::learntoption,
                  "Matrix containing unit-to-unit weights (output_size ×"
                  " input_size)");

    declareOption(ol, "up_units_bias",
                  &RBMLLParameters::up_units_bias,
                  OptionBase::learntoption,
                  "Element i contains the bias of up unit i");

    declareOption(ol, "down_units_bias",
                  &RBMLLParameters::down_units_bias,
                  OptionBase::learntoption,
                  "Element i contains the bias of down unit i");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMLLParameters::build_()
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

    down_units_bias.resize( down_layer_size );
    down_units_bias_pos_stats.resize( down_layer_size );
    down_units_bias_neg_stats.resize( down_layer_size );
    for( int i=0 ; i<down_layer_size ; i++ )
    {
        char dut_i = down_units_types[i];
        if( dut_i != 'l' ) // not linear activation unit
            PLERROR( "RBMLLParameters::build_() - value '%c' for"
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
            PLERROR( "RBMLLParameters::build_() - value '%c' for"
                     " up_units_types[%d]\n"
                     "should be 'l'.\n",
                     uut_i, i );
    }

    if( needs_forget )
        forget();

    clearStats();
}

void RBMLLParameters::build()
{
    inherited::build();
    build_();
}


void RBMLLParameters::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(weights, copies);
    deepCopyField(up_units_bias, copies);
    deepCopyField(down_units_bias, copies);
    deepCopyField(weights_pos_stats, copies);
    deepCopyField(weights_neg_stats, copies);
    deepCopyField(up_units_bias_pos_stats, copies);
    deepCopyField(up_units_bias_neg_stats, copies);
    deepCopyField(down_units_bias_pos_stats, copies);
    deepCopyField(down_units_bias_neg_stats, copies);
}

void RBMLLParameters::accumulatePosStats( const Vec& down_values,
                                          const Vec& up_values )
{
    // weights_pos_stats += up_values * down_values'
    externalProductAcc( weights_pos_stats, up_values, down_values );

    down_units_bias_pos_stats += down_values;
    up_units_bias_pos_stats += up_values;

    pos_count++;
}

void RBMLLParameters::accumulateNegStats( const Vec& down_values,
                                               const Vec& up_values )
{
    // weights_neg_stats += up_values * down_values'
    externalProductAcc( weights_neg_stats, up_values, down_values );

    down_units_bias_neg_stats += down_values;
    up_units_bias_neg_stats += up_values;

    neg_count++;
}

void RBMLLParameters::update()
{
    // updates parameters
    //weights -= learning_rate * (weights_pos_stats/pos_count
    //                              - weights_neg_stats/neg_count)
    weights_pos_stats /= pos_count;
    weights_neg_stats /= neg_count;
    weights_pos_stats -= weights_neg_stats;
    weights_pos_stats *= learning_rate;
    weights -= weights_pos_stats;

    // down_units_bias -= learning_rate * (down_units_bias_pos_stats/pos_count
    //                                    -down_units_bias_neg_stats/neg_count)
    down_units_bias_pos_stats /= pos_count;
    down_units_bias_neg_stats /= neg_count;
    down_units_bias_pos_stats -= down_units_bias_neg_stats;
    down_units_bias_pos_stats *= learning_rate;
    down_units_bias -= down_units_bias_pos_stats;

    // up_units_bias -= learning_rate * (up_units_bias_pos_stats/pos_count
    //                                   -up_units_bias_neg_stats/neg_count)
    up_units_bias_pos_stats /= pos_count;
    up_units_bias_neg_stats /= neg_count;
    up_units_bias_pos_stats -= up_units_bias_neg_stats;
    up_units_bias_pos_stats *= learning_rate;
    up_units_bias -= up_units_bias_pos_stats;

    clearStats();
}

void RBMLLParameters::clearStats()
{
    weights_pos_stats.clear();
    weights_neg_stats.clear();

    down_units_bias_pos_stats.clear();
    down_units_bias_neg_stats.clear();

    up_units_bias_pos_stats.clear();
    up_units_bias_neg_stats.clear();

    pos_count = 0;
    neg_count = 0;
}

void RBMLLParameters::computeUnitActivations
    ( int start, int length, const Vec& activations ) const
{
    assert( activations.length() == length );
    if( going_up )
    {
        assert( start+length <= up_layer_size );
        // activations[i-start] = sum_j weights(i,j) input_vec[j] + b[i]
        product( activations, weights.subMatRows(start, length), input_vec );
        activations += up_units_bias.subVec(start, length);
    }
    else
    {
        assert( start+length <= down_layer_size );
        // activations[i-start] = sum_j weights(j,i) input_vec[j] + b[i]
        transposeProduct( activations, weights.subMatColumns(start, length),
                          input_vec );
        activations += down_units_bias.subVec(start, length);
    }
}

//! this version allows to obtain the input gradient as well
void RBMLLParameters::bpropUpdate(const Vec& input, const Vec& output,
                                  Vec& input_gradient,
                                  const Vec& output_gradient)
{
    assert( input.size() == down_layer_size );
    assert( output.size() == up_layer_size );
    assert( output_gradient.size() == up_layer_size );
    input_gradient.resize( down_layer_size );

    // weights -= learning_rate * output_gradient * input'
    externalProductAcc( weights, (-learning_rate)*output_gradient, input );

    // (up) bias -= learning_rate * output_gradient
    multiplyAcc( up_units_bias, output_gradient, -learning_rate );

    // input_gradient = weights' * output_gradient
    transposeProduct( input_gradient, weights, output_gradient );
}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void RBMLLParameters::forget()
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

    down_units_bias.clear();
    up_units_bias.clear();

    clearStats();
}


/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void RBMLLParameters::finalize()
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
