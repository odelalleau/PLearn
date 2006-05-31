// -*- C++ -*-

// RBMJointLLParameters.cc
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

/*! \file RBMJointLLParameters.cc */



#include "RBMJointLLParameters.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMJointLLParameters,
    "Parameters tying the last, previous and target layers of a DeepBeliefNet",
    "[to be completed]");

RBMJointLLParameters::RBMJointLLParameters( real the_learning_rate )
    : inherited(the_learning_rate)
{
}

RBMJointLLParameters::RBMJointLLParameters(
    PP<RBMLLParameters>& the_target_params,
    PP<RBMLLParameters>& the_cond_params,
    real the_learning_rate )
    : inherited( the_learning_rate ),
      target_params( the_target_params ),
      cond_params( the_cond_params )
{
    // We're not sure inherited::build() has been called
    build();
}


void RBMJointLLParameters::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "target_params",
                  &RBMJointLLParameters::target_params,
                  OptionBase::buildoption,
                  "RBMParameters between the target and the upper layer");

    declareOption(ol, "cond_params", &RBMJointLLParameters::cond_params,
                  OptionBase::buildoption,
                  "RBMParameters between the conditioning input and the upper"
                  " layer");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMJointLLParameters::build_units_types()
{
    if( !target_params || !cond_params )
        return;

    if( target_params->up_units_types != cond_params->up_units_types )
        PLERROR( "RBMJointLLParameters::build_units_types - \n"
                 "target_params->up_units_types should be equal to"
                 " cond_params->up_units_types\n"
                 "(\"%s\" != \"%s\").\n",
                 target_params->up_units_types.c_str(),
                 cond_params->up_units_types.c_str() );

    up_units_types = cond_params->up_units_types;
    down_units_types = target_params->down_units_types;
    down_units_types += cond_params->down_units_types;

    target_size = target_params->down_layer_size;
    cond_size = cond_params->down_layer_size;

    // to avoid "forget()" being called in RBMParameters::build_()
    weights.resize( up_units_types.length(), down_units_types.length() );
    out_act.resize( up_units_types.length() );
}

void RBMJointLLParameters::build_()
{
    // The first part of weights correspond to the weights between the target
    // part and the up layer
    weights.subMatColumns( 0, target_size ) << target_params->weights;
    target_params->weights = weights.subMatColumns( 0, target_size );

    // Second part correspond to the weights between the cond and up layer
    weights.subMatColumns( target_size, cond_size ) << cond_params->weights;
    cond_params->weights = weights.subMatColumns( target_size, cond_size );

    // same thing for the statistics
    target_params->weights_pos_stats =
        weights_pos_stats.subMatColumns( 0, target_size );

    cond_params->weights_pos_stats =
        weights_pos_stats.subMatColumns( target_size, cond_size );

    target_params->weights_neg_stats =
        weights_neg_stats.subMatColumns( 0, target_size );

    cond_params->weights_neg_stats =
        weights_neg_stats.subMatColumns( target_size, cond_size );

    // Same thing for down units bias
    down_units_bias = merge( target_params->down_units_bias,
                             cond_params->down_units_bias );

    target_params->down_units_bias_pos_stats =
        down_units_bias_pos_stats.subVec( 0, target_size );

    cond_params->down_units_bias_pos_stats =
        down_units_bias_pos_stats.subVec( target_size, cond_size );

    target_params->down_units_bias_neg_stats =
        down_units_bias_neg_stats.subVec( 0, target_size );

    cond_params->down_units_bias_neg_stats =
        down_units_bias_neg_stats.subVec( target_size, cond_size );

    // The up layer units parameters are shared between the three RBMParameters
    up_units_bias = cond_params->up_units_bias;
    target_params->up_units_bias = up_units_bias;

    target_params->up_units_bias_pos_stats = up_units_bias_pos_stats;
    cond_params->up_units_bias_pos_stats = up_units_bias_pos_stats;

    target_params->up_units_bias_neg_stats = up_units_bias_neg_stats;
    cond_params->up_units_bias_neg_stats = up_units_bias_neg_stats;

    // sizes for fprop() and all OnlineLearningModules methods
    input_size = cond_size;
    output_size = target_size;
}

void RBMJointLLParameters::build()
{
    // Begin by this, else inherited::build() will not work properly
    build_units_types();

    inherited::build();
    build_();
}


void RBMJointLLParameters::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(target_params, copies);
    deepCopyField(cond_params, copies);
}

void RBMJointLLParameters::setAsUpInput( const Vec& input ) const
{
    target_given_cond = false;
    inherited::setAsUpInput( input );
}

void RBMJointLLParameters::setAsDownInput( const Vec& input ) const
{
    target_given_cond = false;
    inherited::setAsDownInput( input );
}

void RBMJointLLParameters::setAsCondInput( const Vec& input ) const
{
    assert( input.size() == cond_size );
    input_vec = input;
    target_given_cond = true;
    going_up = false;
}

void RBMJointLLParameters::fprop(const Vec& input, Vec& output) const
{
    // sets "input" as conditioning input, and compute "output"
    setAsCondInput( input );
    output.resize( output_size );
    computeUnitActivations( 0, output_size, output );
}


void RBMJointLLParameters::computeUnitActivations
    ( int start, int length, const Vec& activations ) const
{
    if( target_given_cond )
    {
        assert( activations.length() == length );
        assert( start+length <= target_size );

        out_act << up_units_bias;
        Mat V = weights.subMatColumns( target_size, cond_size );
        // out_act = up_units_bias + V * input_vec
        productAcc( out_act, V, input_vec );

        // actY_i = B_i - sum_j softplus(-(W_ji + C_j + sum_k V_jk p(P_k)))
        //        = B_i - sum_j softplus(-(W_ji + out_act[j]) )
        for( int i=start ; i<start+length ; i++ )
        {
            real somme = down_units_bias[i];
            real *w = &weights[0][i];
            // step from one row to the next in weights matrix
            int m = weights.mod();

            for( int j=0; j< weights.length() ; j++, w+=m )
            {
                // *w = weights(j,i)
                somme -= softplus( -(*w + out_act[j]));
            }
            activations[i-start] = somme;
        }
    }
    else
        inherited::computeUnitActivations( start, length, activations );
}

//! this version allows to obtain the input gradient as well
void RBMJointLLParameters::bpropUpdate(const Vec& input,
                                       const Vec& output,
                                       Vec& input_gradient,
                                       const Vec& output_gradient)
{
    assert( input.size() == cond_size );
    assert( output.size() == target_size );
    assert( output_gradient.size() == target_size );
    input_gradient.resize( cond_size );
    input_gradient.clear();

    //for( int k=0 ; k<target_size ; k++ )
    //    down_units_bias[k] -= learning_rate * output_gradient[k];
    multiplyAcc( down_units_bias.subVec(0, target_size),
                 output_gradient, -learning_rate );

    for( int i=0 ; i<up_layer_size ; i++ )
    {
        real* w = weights[i];
        real d_out_act = 0;
        for( int k=0 ; k<target_size ; k++ )
        {
            // dC/d(weights(i,k)+out_act[i])
            real d_z = output_gradient[k] * (-sigmoid(-w[k]-out_act[i]));
            w[k] -= learning_rate * d_z;

            d_out_act += d_z;
        }
        up_units_bias[i] -= learning_rate * d_out_act;

        for( int j=0 ; j<cond_size ; j++ )
        {
            real& w_ij = w[j+target_size];
            input_gradient[j] += d_out_act * w_ij;
            w_ij -= learning_rate * d_out_act * input[j];
        }
    }

}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void RBMJointLLParameters::forget()
{
    if( target_params )
        target_params->forget();

    if( cond_params )
        cond_params->forget();

    clearStats();
}

/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void RBMJointLLParameters::finalize()
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
