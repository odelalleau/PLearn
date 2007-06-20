// -*- C++ -*-

// RBMMatrixTransposeConnection.cc
//
// Copyright (C) 2007 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file RBMMatrixTransposeConnection.cc */

#include "RBMMatrixTransposeConnection.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMMatrixTransposeConnection,
    "RBMConnection which uses the tranpose of some other "
    "RBMMatrixConnection's weights",
    "");

RBMMatrixTransposeConnection::RBMMatrixTransposeConnection( 
    PP<RBMMatrixConnection> the_rbm_matrix_connection,
    real the_learning_rate ) :
    inherited(the_learning_rate), 
    rbm_matrix_connection(the_rbm_matrix_connection)
{}

void RBMMatrixTransposeConnection::declareOptions(OptionList& ol)
{
    declareOption(ol, "rbm_matrix_connection", 
                  &RBMMatrixTransposeConnection::rbm_matrix_connection,
                  OptionBase::buildoption,
                  "RBMMatrixConnection from which the weights are taken");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "up_size", &RBMConnection::up_size,
                    OptionBase::learntoption,
                    "Is set to rbm_matrix_connection->down_size.");
    redeclareOption(ol, "down_size", &RBMConnection::down_size,
                    OptionBase::learntoption,
                    "Is set to rbm_matrix_connection->up_size.");
}

void RBMMatrixTransposeConnection::build_()
{
    if( !rbm_matrix_connection )
        return;

    // If we have a random_gen and rbm_matrix_connection does not, share it
    if( random_gen && !(rbm_matrix_connection->random_gen) )
    {
        rbm_matrix_connection->random_gen = random_gen;
        rbm_matrix_connection->forget();
    }
    weights = rbm_matrix_connection->weights;
    down_size = rbm_matrix_connection->up_size;
    up_size = rbm_matrix_connection->down_size;

    // For compatibility with OnlineLearningModule inherited functions
    input_size = down_size;
    output_size = up_size;


    weights_pos_stats.resize( down_size, up_size );
    weights_neg_stats.resize( down_size, up_size );

    if( momentum != 0. )
        weights_inc.resize( down_size, up_size );

    clearStats();
}

void RBMMatrixTransposeConnection::build()
{
    inherited::build();
    build_();
}


void RBMMatrixTransposeConnection::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(weights, copies);
    deepCopyField(rbm_matrix_connection, copies);
    deepCopyField(weights_pos_stats, copies);
    deepCopyField(weights_neg_stats, copies);
    deepCopyField(weights_inc, copies);
}

void RBMMatrixTransposeConnection::accumulatePosStats( const Vec& down_values,
                                              const Vec& up_values )
{
    // weights_pos_stats += down_values * up_values'
    externalProductAcc( weights_pos_stats, down_values, up_values );

    pos_count++;
}

void RBMMatrixTransposeConnection::accumulateNegStats( const Vec& down_values,
                                              const Vec& up_values )
{
    // weights_neg_stats += down_values * up_values'
    externalProductAcc( weights_neg_stats, down_values, up_values );

    neg_count++;
}

void RBMMatrixTransposeConnection::update()
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
void RBMMatrixTransposeConnection::update( const Vec& pos_down_values, // v_0
                                           const Vec& pos_up_values,   // h_0
                                           const Vec& neg_down_values, // v_1
                                           const Vec& neg_up_values )  // h_1
{
    PLASSERT_MSG( rbm_matrix_connection, "RBMMatrixTransposeConnection must be given an rbm_matrix_connection.\n");
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

void RBMMatrixTransposeConnection::clearStats()
{
    weights_pos_stats.clear();
    weights_neg_stats.clear();

    pos_count = 0;
    neg_count = 0;
}

void RBMMatrixTransposeConnection::computeProduct( int start, int length,
                                          const Vec& activations,
                                          bool accumulate ) const
{
    PLASSERT( activations.length() == length );
    PLASSERT_MSG( rbm_matrix_connection, "RBMMatrixTransposeConnection must be given an rbm_matrix_connection.\n");

    if( going_up )
    {
        PLASSERT( start+length <= up_size );
        // activations[i-start] += sum_j weights(i,j) input_vec[j]

        if( accumulate )
            transposeProductAcc( activations,
                                 weights.subMatColumns(start,length),
                                 input_vec );
        else
            transposeProduct( activations,
                              weights.subMatColumns(start,length),
                              input_vec );
    }
    else
    {
        PLASSERT( start+length <= down_size );
        // activations[i-start] += sum_j weights(j,i) input_vec[j]
        if( accumulate )
            productAcc( activations,
                        weights.subMatRows(start,length),
                        input_vec );
        else
            product( activations,
                     weights.subMatRows(start,length),
                     input_vec );
    }
}

void RBMMatrixTransposeConnection::computeProducts(int start, int length,
                                          Mat& activations,
                                          bool accumulate ) const
{
    PLASSERT_MSG( rbm_matrix_connection, "RBMMatrixTransposeConnection must be given an rbm_matrix_connection.\n");
    activations.resize(inputs_mat.length(), length);
    if( going_up )
    {
        PLASSERT( start+length <= up_size );
        // activations(k, i-start) += sum_j weights(i,j) inputs_mat(k, j)

        if( accumulate )
            productAcc(activations,
                    inputs_mat,
                    weights.subMatColumns(start,length));
        else
            product(activations,
                    inputs_mat,
                    weights.subMatColumns(start,length));
    }
    else
    {
        PLASSERT( start+length <= down_size );
        // activations(k, i-start) += sum_j weights(j,i) inputs_mat(k, j)
        if( accumulate )
            productTransposeAcc(activations,
                    inputs_mat,
                    weights.subMatRows(start,length) );
        else
            productTranspose(activations,
                    inputs_mat,
                    weights.subMatRows(start,length) );
    }
}

//! this version allows to obtain the input gradient as well
void RBMMatrixTransposeConnection::bpropUpdate(const Vec& input, 
                                               const Vec& output,
                                               Vec& input_gradient,
                                               const Vec& output_gradient,
                                               bool accumulate)
{
    PLASSERT( input.size() == down_size );
    PLASSERT( output.size() == up_size );
    PLASSERT( output_gradient.size() == up_size );
    PLASSERT_MSG( rbm_matrix_connection, "RBMMatrixTransposeConnection must be given an rbm_matrix_connection.\n");

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == down_size,
                      "Cannot resize input_gradient AND accumulate into it" );

        // input_gradient += weights' * output_gradient
        productAcc( input_gradient, weights, output_gradient );
    }
    else
    {
        input_gradient.resize( down_size );
        
        // input_gradient = weights' * output_gradient
        product( input_gradient, weights, output_gradient );
    }
    
    // weights -= learning_rate * output_gradient * input'
    externalProductScaleAcc( weights, input, output_gradient, -learning_rate );
}

void RBMMatrixTransposeConnection::bpropUpdate(const Mat& inputs, const Mat& outputs,
                                       Mat& input_gradients,
                                       const Mat& output_gradients,
                                       bool accumulate)
{
    PLASSERT( inputs.width() == down_size );
    PLASSERT( outputs.width() == up_size );
    PLASSERT( output_gradients.width() == up_size );
    PLASSERT_MSG( rbm_matrix_connection, "RBMMatrixTransposeConnection must be given an rbm_matrix_connection.\n");

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == down_size &&
                      input_gradients.length() == inputs.length(),
                      "Cannot resize input_gradients and accumulate into it" );

        // input_gradients += output_gradient * weights
        productTransposeAcc(input_gradients, output_gradients, weights);
    }
    else
    {
        input_gradients.resize(inputs.length(), down_size);
        // input_gradients = output_gradient * weights
        productTranspose(input_gradients, output_gradients, weights);
    }

    // weights -= learning_rate/n * output_gradients' * inputs
    transposeProductScaleAcc(weights, inputs, output_gradients,
                             -learning_rate / inputs.length(), real(1));
}


//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void RBMMatrixTransposeConnection::forget()
{
    PLASSERT_MSG( rbm_matrix_connection, "RBMMatrixTransposeConnection must be given an rbm_matrix_connection.\n");
    clearStats();
    if( !random_gen )
    {
        PLWARNING("RBMMatrixTransposeConnection: cannot forget() without"
                  " random_gen");
        return;
    }
    if( !(rbm_matrix_connection->random_gen) )
        rbm_matrix_connection->random_gen = random_gen;
    rbm_matrix_connection->forget();
}


/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void RBMMatrixTransposeConnection::finalize()
{
}
*/

//! return the number of parameters
int RBMMatrixTransposeConnection::nParameters() const
{
    return weights.size();
}

//! Make the parameters data be sub-vectors of the given global_parameters.
//! The argument should have size >= nParameters. The result is a Vec
//! that starts just after this object's parameters end, i.e.
//!    result = global_parameters.subVec(nParameters(),global_parameters.size()-nParameters());
//! This allows to easily chain calls of this method on multiple RBMParameters.
Vec RBMMatrixTransposeConnection::makeParametersPointHere(const Vec& global_parameters)
{
    PLASSERT_MSG( rbm_matrix_connection, "RBMMatrixTransposeConnection must be given an rbm_matrix_connection.\n");
    Vec ret = rbm_matrix_connection->makeParametersPointHere(global_parameters);
    weights = rbm_matrix_connection->weights;
    return ret;
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
