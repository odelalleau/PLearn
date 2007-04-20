// -*- C++ -*-

// RBMMatrixConnection.cc
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

/*! \file RBMMatrixConnection.cc */



#include "RBMMatrixConnection.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMMatrixConnection,
    "Stores and learns the parameters between two linear layers of an RBM",
    "");

RBMMatrixConnection::RBMMatrixConnection( real the_learning_rate ) :
    inherited(the_learning_rate)
{
}

void RBMMatrixConnection::declareOptions(OptionList& ol)
{
    declareOption(ol, "weights", &RBMMatrixConnection::weights,
                  OptionBase::learntoption,
                  "Matrix containing unit-to-unit weights (up_size ×"
                  " down_size)");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMMatrixConnection::build_()
{
    if( up_size <= 0 || down_size <= 0 )
        return;

    bool needs_forget = false; // do we need to reinitialize the parameters?

    if( weights.length() != up_size ||
        weights.width() != down_size )
    {
        weights.resize( up_size, down_size );
        needs_forget = true;
    }

    weights_pos_stats.resize( up_size, down_size );
    weights_neg_stats.resize( up_size, down_size );

    if( momentum != 0. )
        weights_inc.resize( up_size, down_size );

    if( needs_forget )
        forget();

    clearStats();
}

void RBMMatrixConnection::build()
{
    inherited::build();
    build_();
}


void RBMMatrixConnection::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(weights, copies);
    deepCopyField(weights_pos_stats, copies);
    deepCopyField(weights_neg_stats, copies);
    deepCopyField(weights_inc, copies);
}

void RBMMatrixConnection::accumulatePosStats( const Vec& down_values,
                                              const Vec& up_values )
{
    // weights_pos_stats += up_values * down_values'
    externalProductAcc( weights_pos_stats, up_values, down_values );

    pos_count++;
}

void RBMMatrixConnection::accumulateNegStats( const Vec& down_values,
                                              const Vec& up_values )
{
    // weights_neg_stats += up_values * down_values'
    externalProductAcc( weights_neg_stats, up_values, down_values );

    neg_count++;
}

void RBMMatrixConnection::update()
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

    clearStats();
}

// Instead of using the statistics, we assume we have only one markov chain
// runned and we update the parameters from the first 4 values of the chain
void RBMMatrixConnection::update( const Vec& pos_down_values, // v_0
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
}

////////////////
// clearStats //
////////////////
void RBMMatrixConnection::clearStats()
{
    weights_pos_stats.clear();
    weights_neg_stats.clear();

    pos_count = 0;
    neg_count = 0;
}

////////////////////
// computeProduct //
////////////////////
void RBMMatrixConnection::computeProduct( int start, int length,
                                          const Vec& activations,
                                          bool accumulate ) const
{
    PLASSERT( activations.length() == length );
    if( going_up )
    {
        PLASSERT( start+length <= up_size );
        // activations[i-start] += sum_j weights(i,j) input_vec[j]

        if( accumulate )
            productAcc( activations,
                        weights.subMatRows(start,length),
                        input_vec );
        else
            product( activations,
                     weights.subMatRows(start,length),
                     input_vec );
    }
    else
    {
        PLASSERT( start+length <= down_size );
        // activations[i-start] += sum_j weights(j,i) input_vec[j]
        if( accumulate )
            transposeProductAcc( activations,
                                 weights.subMatColumns(start,length),
                                 input_vec );
        else
            transposeProduct( activations,
                              weights.subMatColumns(start,length),
                              input_vec );
    }
}

/////////////////////
// computeProducts //
/////////////////////
void RBMMatrixConnection::computeProducts(int start, int length,
                                          Mat& activations,
                                          bool accumulate ) const
{
    activations.resize(inputs_mat.length(), length);
    if( going_up )
    {
        PLASSERT( start+length <= up_size );
        // activations(k, i-start) += sum_j weights(i,j) inputs_mat(k, j)

        if( accumulate )
            productTransposeAcc(activations,
                    inputs_mat,
                    weights.subMatRows(start,length));
        else
            productTranspose(activations,
                    inputs_mat,
                    weights.subMatRows(start,length));
    }
    else
    {
        PLASSERT( start+length <= down_size );
        // activations(k, i-start) += sum_j weights(j,i) inputs_mat(k, j)
        if( accumulate )
            transposeProductAcc( activations,
                                 weights.subMatColumns(start,length),
                                 inputs_mat );
        else
            transposeProduct( activations,
                              weights.subMatColumns(start,length),
                              inputs_mat );
    }
}

/////////////////
// bpropUpdate //
/////////////////
void RBMMatrixConnection::bpropUpdate(const Vec& input, const Vec& output,
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

        // input_gradient += weights' * output_gradient
        transposeProductAcc( input_gradient, weights, output_gradient );
    }
    else
    {
        input_gradient.resize( down_size );

        // input_gradient = weights' * output_gradient
        transposeProduct( input_gradient, weights, output_gradient );
    }

    // weights -= learning_rate * output_gradient * input'
    externalProductScaleAcc( weights, output_gradient, input, -learning_rate );
}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void RBMMatrixConnection::forget()
{
    if( initialization_method == "zero" )
        weights.clear();
    else
    {
        if( !random_gen )
            random_gen = new PRandom();

        real d = 1. / max( down_size, up_size );
        if( initialization_method == "uniform_sqrt" )
            d = sqrt( d );

        random_gen->fill_random_uniform( weights, -d, d );
    }
    clearStats();
}


/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void RBMMatrixConnection::finalize()
{
}
*/

//! return the number of parameters
int RBMMatrixConnection::nParameters() const
{
    return weights.size();
}

//! Make the parameters data be sub-vectors of the given global_parameters.
//! The argument should have size >= nParameters. The result is a Vec
//! that starts just after this object's parameters end, i.e.
//!    result = global_parameters.subVec(nParameters(),global_parameters.size()-nParameters());
//! This allows to easily chain calls of this method on multiple RBMParameters.
Vec RBMMatrixConnection::makeParametersPointHere(const Vec& global_parameters)
{
    int n=weights.size();
    int m = global_parameters.size();
    if (m<n)
        PLERROR("RBMMatrixConnection::makeParametersPointHere: argument has length %d, should be longer than nParameters()=%d",m,n);
    real* p = global_parameters.data();
    weights.makeSharedValue(p,n);

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
