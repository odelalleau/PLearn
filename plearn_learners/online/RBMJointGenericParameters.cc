// -*- C++ -*-

// RBMJointGenericParameters.cc
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

/*! \file RBMJointGenericParameters.cc */



#include "RBMJointGenericParameters.h"
#include <plearn/math/TMat_maths.h>
//#include "RBMLayer.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMJointGenericParameters,
    "Parameters tying the last, previous and target layers of a DeepBeliefNet",
    "[to be completed]");

RBMJointGenericParameters::RBMJointGenericParameters( real the_learning_rate )
    : inherited(the_learning_rate)
{
}

RBMJointGenericParameters::RBMJointGenericParameters(
    PP<RBMGenericParameters>& the_target_params,
    PP<RBMGenericParameters>& the_cond_params,
    real the_learning_rate )
    : inherited( the_learning_rate ),
      target_params( the_target_params ),
      cond_params( the_cond_params )
{
    // We're not sure inherited::build() has been called
    build();
}


void RBMJointGenericParameters::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "target_params",
                  &RBMJointGenericParameters::target_params,
                  OptionBase::buildoption,
                  "RBMParameters between the target and the upper layer");

    declareOption(ol, "cond_params", &RBMJointGenericParameters::cond_params,
                  OptionBase::buildoption,
                  "RBMParameters between the conditioning input and the upper"
                  " layer");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMJointGenericParameters::build_units_types()
{
    if( !target_params || !cond_params )
        return;

    if( target_params->up_units_types != cond_params->up_units_types )
        PLERROR( "RBMJointGenericParameters::build_units_types - \n"
                 "target_params->up_units_types should be equal to"
                 " cond_params->up_units_types\n"
                 "(\"%s\" != \"%s\").\n",
                 target_params->up_units_types.c_str(),
                 cond_params->up_units_types.c_str() );

    up_units_types = cond_params->up_units_types;
    down_units_types = target_params->down_units_types;

    target_size = target_params->down_layer_size;
    cond_size = cond_params->down_layer_size;

    for( int i=0 ; i<target_size ; i++ )
        if( down_units_types[i] != 'l' )
            PLERROR( "RBMJointGenericParameters::build_units_types - \n"
                     "target_params->down_units_types[%d] should be 'l', is"
                     " '%c'.\n", i, down_units_types[i] );

    down_units_types += cond_params->down_units_types;

    // to avoid "forget()" being called in RBMParameters::build_()
    weights.resize( up_units_types.length(), down_units_types.length() );
    out_act.resize( up_units_types.length() );
}

void RBMJointGenericParameters::build_()
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

    // Same thing for down units parameters
    down_units_params = merge( target_params->down_units_params,
                               cond_params->down_units_params );

    target_params->down_units_params_pos_stats =
        down_units_params_pos_stats.subVec( 0, target_size );

    cond_params->down_units_params_pos_stats =
        down_units_params_pos_stats.subVec( target_size, cond_size );

    target_params->down_units_params_neg_stats =
        down_units_params_neg_stats.subVec( 0, target_size );

    cond_params->down_units_params_neg_stats =
        down_units_params_neg_stats.subVec( target_size, cond_size );

    // The up layer units parameters are shared between the three RBMParameters
    up_units_params = cond_params->up_units_params;
    target_params->up_units_params = up_units_params;

    target_params->up_units_params_pos_stats = up_units_params_pos_stats;
    cond_params->up_units_params_pos_stats = up_units_params_pos_stats;

    target_params->up_units_params_neg_stats = up_units_params_neg_stats;
    cond_params->up_units_params_neg_stats = up_units_params_neg_stats;

    // sizes for fprop() and all OnlineLearningModules methods
    input_size = cond_size;
    output_size = target_size;
}

void RBMJointGenericParameters::build()
{
    // Begin by this, else inherited::build() will not work properly
    build_units_types();

    inherited::build();
    build_();
}


void RBMJointGenericParameters::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(target_params, copies);
    deepCopyField(cond_params, copies);
}

void RBMJointGenericParameters::setAsUpInput( const Vec& input ) const
{
    target_given_cond = false;
    inherited::setAsUpInput( input );
}

void RBMJointGenericParameters::setAsDownInput( const Vec& input ) const
{
    target_given_cond = false;
    inherited::setAsDownInput( input );
}

void RBMJointGenericParameters::setAsCondInput( const Vec& input ) const
{
    assert( input.size() == cond_size );
    input_vec = input;
    target_given_cond = true;
    going_up = false;
}

void RBMJointGenericParameters::computeLinearUnitActivations
    ( int i, const Vec& activations ) const
{
    if( target_given_cond )
    {
        assert( activations.length() == 1 );
        Mat V = weights.subMatColumns(target_size, cond_size);
        if (i==0)
            for (int j=0;j<weights.length(); j++)
                out_act[j] =
                    up_units_params[j][0] + matRowDotVec(V, j, input_vec);

        // actY_i = B_i - sum_j softplus(-(W_ij + C_j + sum_k V_jk p(P_k)))
        real somme = down_units_params[i][0];
        real* w = &weights[0][i];
        // step from one row to the next in weights matrix
        int m = weights.mod();

        for( int j=0; j< weights.length() ; j++, w+=m )
        {
            // *w = weights(j,i)
            somme -= softplus( -(*w + out_act[j]));
        }
        activations[0] = somme;
    }
    else
        inherited::computeLinearUnitActivations(i, activations);
}

void RBMJointGenericParameters::fprop(const Vec& input, Vec& output) const
{
    // sets "input" as conditioning input, and compute "output"
    setAsCondInput( input );
    output.resize( output_size );
    computeUnitActivations( 0, output_size, output );
}



void RBMJointGenericParameters::computeQuadraticUnitActivations
    ( int i, const Vec& activations ) const
{
    if( target_given_cond )
        PLERROR( "RBMJointGenericParameters::computeQuadraticUnitActivations"
                 " -\n"
                 "Cannot compute the activation of a quadratic unit in mode "
                 "\"target_given_cond\".\n" );
    else
        inherited::computeQuadraticUnitActivations(i, activations);
}


void RBMJointGenericParameters::computeUnitActivations
    ( int start, int length, const Vec& activations ) const
{
    string units_types;
    if( target_given_cond )
        units_types = down_units_types.substr(0, target_size);
    else if( going_up )
        units_types = up_units_types;
    else
        units_types = down_units_types;

    assert( start+length <= (int) units_types.length() );
    int cur_pos = 0; // position index inside activations

    for( int i=start ; i<start+length ; i++ )
    {
        char ut_i = units_types[i];
        if( ut_i == 'l' )
        {
            computeLinearUnitActivations( i, activations.subVec(cur_pos, 1) );
            cur_pos++;
        }
        else if( ut_i == 'q' )
        {
            computeQuadraticUnitActivations
                ( i, activations.subVec(cur_pos, 2) );
            cur_pos += 2;
        }
        else
            PLERROR( "RBMJointGenericParameters::computeUnitActivations():\n"
                     "value '%c' for units_types[%d] is unknown.\n"
                     "Supported values are 'l' and 'q'.\n", ut_i, i );
    }
}

//! this version allows to obtain the input gradient as well
void RBMJointGenericParameters::bpropUpdate(const Vec& input,
                                            const Vec& output,
                                            Vec& input_gradient,
                                            const Vec& output_gradient)
{
    PLERROR( "RBMJointGenericParameters::bpropUpdate() not implemented yet.\n"
             "If you only have linear units on up and down layer, you should\n"
             "consider using RBMJointLLParameters instead.\n" );
}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void RBMJointGenericParameters::forget()
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
void RBMJointGenericParameters::finalize()
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
