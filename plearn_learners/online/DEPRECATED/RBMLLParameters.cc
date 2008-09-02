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

/*! \file PLearn/plearn_learners/online/DEPRECATED/RBMLLParameters.cc */



#include "RBMLLParameters.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMLLParameters,
    "Stores and learns the parameters between two linear layers of an RBM",
    "");

RBMLLParameters::RBMLLParameters( real the_learning_rate ) :
    inherited(the_learning_rate),
    momentum(0.)
{
}

RBMLLParameters::RBMLLParameters( string down_types, string up_types,
                                  real the_learning_rate ) :
    inherited( down_types, up_types, the_learning_rate ),
    momentum(0.)
{
    // We're not sure inherited::build() has been called
    build();
}

void RBMLLParameters::declareOptions(OptionList& ol)
{
    declareOption(ol, "momentum", &RBMLLParameters::momentum,
                  OptionBase::buildoption,
                  "Momentum factor (should be between 0 and 1)");

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

    if( momentum != 0. )
    {
        weights_inc.resize( up_layer_size, down_layer_size );
        down_units_bias_inc.resize( down_layer_size );
        up_units_bias_inc.resize( up_layer_size );
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
    deepCopyField(weights_inc, copies);
    deepCopyField(up_units_bias_inc, copies);
    deepCopyField(down_units_bias_inc, copies);
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
    real pos_factor = -learning_rate / pos_count;
    real neg_factor = learning_rate / neg_count;

    int l = weights.length();
    int w = weights.width();

    real* w_i = weights.data();
    real* wps_i = weights_pos_stats.data();
    real* wns_i = weights_neg_stats.data();
    int w_mod = weights.mod();
    int wps_mod = weights_pos_stats.mod();
    int wns_mod = weights_neg_stats.mod();

    if( momentum == 0. )
    {
        // no need to use weights_inc
        for( int i=0 ; i<l ; i++, w_i+=w_mod, wps_i+=wps_mod, wns_i+=wns_mod )
            for( int j=0 ; j<w ; j++ )
                w_i[j] += pos_factor * wps_i[j] + neg_factor * wns_i[j];
    }
    else
    {
        // ensure that weights_inc has the right size
        weights_inc.resize( l, w );

        // The update rule becomes:
        // weights_inc = momentum * weights_inc
        //               - learning_rate * (weights_pos_stats/pos_count
        //                                  - weights_neg_stats/neg_count);
        // weights += weights_inc;
        real* winc_i = weights_inc.data();
        int winc_mod = weights_inc.mod();
        for( int i=0 ; i<l ; i++, w_i += w_mod, wps_i += wps_mod,
                             wns_i += wns_mod, winc_i += winc_mod )
            for( int j=0 ; j<w ; j++ )
            {
                winc_i[j] = momentum * winc_i[j]
                    + pos_factor * wps_i[j] + neg_factor * wns_i[j];
                w_i[j] += winc_i[j];
            }
    }

    // down_units_bias -= learning_rate * (down_units_bias_pos_stats/pos_count
    //                                    -down_units_bias_neg_stats/neg_count)
    l = down_units_bias.length();
    real* dub = down_units_bias.data();
    real* dubps = down_units_bias_pos_stats.data();
    real* dubns = down_units_bias_neg_stats.data();

    if( momentum == 0. )
    {
        // no need to use down_units_bias_inc
        for( int i=0 ; i<l ; i++ )
            dub[i] += pos_factor * dubps[i] + neg_factor * dubns[i];
    }
    else
    {
        // ensure that down_units_bias_inc has the right size
        down_units_bias_inc.resize( l );

        // The update rule becomes:
        // down_units_bias_inc =
        //      momentum * down_units_bias_inc
        //      - learning_rate * (down_units_bias_pos_stats/pos_count
        //                         -down_units_bias_neg_stats/neg_count);
        // down_units_bias += down_units_bias_inc;
        real* dubinc = down_units_bias_inc.data();
        for( int i=0 ; i<l ; i++ )
        {
            dubinc[i] = momentum * dubinc[i]
                + pos_factor * dubps[i] + neg_factor * dubns[i];
            dub[i] += dubinc[i];
        }
    }

    // up_units_bias -= learning_rate * (up_units_bias_pos_stats/pos_count
    //                                   -up_units_bias_neg_stats/neg_count)
    l = up_units_bias.length();
    real* uub = up_units_bias.data();
    real* uubps = up_units_bias_pos_stats.data();
    real* uubns = up_units_bias_neg_stats.data();
    if( momentum == 0. )
    {
        // no need to use up_units_bias_inc
        for( int i=0 ; i<l ; i++ )
            uub[i] += pos_factor * uubps[i] + neg_factor * uubns[i];
    }
    else
    {
        // ensure that up_units_bias_inc has the right size
        up_units_bias_inc.resize( l );

        // The update rule becomes:
        // up_units_bias_inc =
        //      momentum * up_units_bias_inc
        //      - learning_rate * (up_units_bias_pos_stats/pos_count
        //                         -up_units_bias_neg_stats/neg_count);
        // up_units_bias += up_units_bias_inc;
        real* uubinc = up_units_bias_inc.data();
        for( int i=0 ; i<l ; i++ )
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
void RBMLLParameters::update( const Vec& pos_down_values, // v_0
                              const Vec& pos_up_values,   // h_0
                              const Vec& neg_down_values, // v_1
                              const Vec& neg_up_values )  // h_1
{
    // weights -= learning_rate * ( h_0 v_0' - h_1 v_1' );
    // or:
    // weights[i][j] += learning_rate * (h_1[i] v_1[j] - h_0[i] v_0[j]);

    int l = weights.length();
    int w = weights.width();
    PLASSERT( pos_up_values.length() == l );
    PLASSERT( neg_up_values.length() == l );
    PLASSERT( pos_down_values.length() == w );
    PLASSERT( neg_down_values.length() == w );

    real* w_i = weights.data();
    real* puv_i = pos_up_values.data();
    real* nuv_i = neg_up_values.data();
    real* pdv = pos_down_values.data();
    real* ndv = neg_down_values.data();
    int w_mod = weights.mod();

    if( momentum == 0. )
    {
        for( int i=0 ; i<l ; i++, w_i += w_mod, puv_i++, nuv_i++ )
            for( int j=0 ; j<w ; j++ )
                w_i[j] += learning_rate * (*nuv_i * ndv[j] - *puv_i * pdv[j]);
    }
    else
    {
        // ensure that weights_inc has the right size
        weights_inc.resize( l, w );

        // The update rule becomes:
        // weights_inc = momentum * weights_inc
        //               - learning_rate * ( h_0 v_0' - h_1 v_1' );
        // weights += weights_inc;

        real* winc_i = weights_inc.data();
        int winc_mod = weights_inc.mod();
        for( int i=0 ; i<l ; i++, w_i += w_mod, winc_i += winc_mod,
                             puv_i++, nuv_i++ )
            for( int j=0 ; j<w ; j++ )
            {
                winc_i[j] = momentum * winc_i[j]
                    + learning_rate * (*nuv_i * ndv[j] - *puv_i * pdv[j]);
                w_i[j] += winc_i[j];
            }
    }

    // down_units_bias -= learning_rate * ( v_0 - v_1 )

    real* dub = down_units_bias.data();
    // pdv and ndv didn't change since last time
    // real* pdv = pos_down_values.data();
    // real* ndv = neg_down_values.data();

    if( momentum == 0. )
    {
        // no need to use down_units_bias_inc
        for( int j=0 ; j<w ; j++ )
            dub[j] += learning_rate * ( ndv[j] - pdv[j] );
    }
    else
    {
        // ensure that down_units_bias_inc has the right size
        down_units_bias_inc.resize( w );

        // The update rule becomes:
        // down_units_bias_inc = momentum * down_units_bias_inc
        //                       - learning_rate * ( v_0 - v_1 )
        // down_units_bias += down_units_bias_inc;

        real* dubinc = down_units_bias_inc.data();
        for( int j=0 ; j<w ; j++ )
        {
            dubinc[j] = momentum * dubinc[j]
                + learning_rate * ( ndv[j] - pdv[j] );
            dub[j] += dubinc[j];
        }
    }

    // up_units_bias -= learning_rate * ( h_0 - h_1 )
    real* uub = up_units_bias.data();
    real* puv = pos_up_values.data();
    real* nuv = neg_up_values.data();

    if( momentum == 0. )
    {
        // no need to use up_units_bias_inc
        for( int i=0 ; i<l ; i++ )
            uub[i] += learning_rate * (nuv[i] - puv[i] );
    }
    else
    {
        // ensure that up_units_bias_inc has the right size
        up_units_bias_inc.resize( l );

        // The update rule becomes:
        // up_units_bias_inc =
        //      momentum * up_units_bias_inc
        //      - learning_rate * (up_units_bias_pos_stats/pos_count
        //                         -up_units_bias_neg_stats/neg_count);
        // up_units_bias += up_units_bias_inc;
        real* uubinc = up_units_bias_inc.data();
        for( int i=0 ; i<l ; i++ )
        {
            uubinc[i] = momentum * uubinc[i]
                + learning_rate * ( nuv[i] - puv[i] );
            uub[i] += uubinc[i];
        }
    }
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
    PLASSERT( activations.length() == length );
    if( going_up )
    {
        PLASSERT( start+length <= up_layer_size );
        // activations[i-start] = sum_j weights(i,j) input_vec[j] + b[i]
        product( activations, weights.subMatRows(start, length), input_vec );
        activations += up_units_bias.subVec(start, length);
    }
    else
    {
        PLASSERT( start+length <= down_layer_size );
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
    PLASSERT( input.size() == down_layer_size );
    PLASSERT( output.size() == up_layer_size );
    PLASSERT( output_gradient.size() == up_layer_size );
    input_gradient.resize( down_layer_size );

    // input_gradient = weights' * output_gradient
    transposeProduct( input_gradient, weights, output_gradient );

    // weights -= learning_rate * output_gradient * input'
    externalProductScaleAcc( weights, output_gradient, input, -learning_rate );

    // (up) bias -= learning_rate * output_gradient
    multiplyAcc( up_units_bias, output_gradient, -learning_rate );

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

//! return the number of parameters
int RBMLLParameters::nParameters(bool share_up_params, bool share_down_params) const
{
    return weights.size() + (share_up_params?up_units_bias.size():0) + 
        (share_down_params?down_units_bias.size():0);
}

//! Make the parameters data be sub-vectors of the given global_parameters.
//! The argument should have size >= nParameters. The result is a Vec
//! that starts just after this object's parameters end, i.e.
//!    result = global_parameters.subVec(nParameters(),global_parameters.size()-nParameters());
//! This allows to easily chain calls of this method on multiple RBMParameters.
Vec RBMLLParameters::makeParametersPointHere(const Vec& global_parameters, bool share_up_params, bool share_down_params)
{
    int n1=weights.size();
    int n2=up_units_bias.size();
    int n3=down_units_bias.size();
    int n = n1+(share_up_params?n2:0)+(share_down_params?n3:0); // should be = nParameters()
    int m = global_parameters.size();
    if (m<n)
        PLERROR("RBMLLParameters::makeParametersPointHere: argument has length %d, should be longer than nParameters()=%d",m,n);
    real* p = global_parameters.data();
    weights.makeSharedValue(p,n1);
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
