// -*- C++ -*-

// RBMMixedConnection.cc
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

/*! \file RBMMixedConnection.cc */



#include "RBMMixedConnection.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMMixedConnection,
    "Stores and learns the parameters between two linear layers of an RBM",
    "If a sub_connection is not present, it will be treated as a 0 matrix");

RBMMixedConnection::RBMMixedConnection()
{
}

void RBMMixedConnection::declareOptions(OptionList& ol)
{
    declareOption(ol, "sub_connections", &RBMMixedConnection::sub_connections,
                  OptionBase::buildoption,
                  "Matrix containing the sub-transformations (blocks).");

    declareOption(ol, "up_init_positions",
                  &RBMMixedConnection::up_init_positions,
                  OptionBase::learntoption,
                  "Initial vertical index of the blocks.");

    declareOption(ol, "down_init_positions",
                  &RBMMixedConnection::down_init_positions,
                  OptionBase::learntoption,
                  "Initial horizontal index of the blocks.");

    declareOption(ol, "n_up_blocks", &RBMMixedConnection::n_up_blocks,
                  OptionBase::learntoption,
                  "Length of the blocks matrix.");

    declareOption(ol, "n_down_blocks", &RBMMixedConnection::n_down_blocks,
                  OptionBase::learntoption,
                  "Width of the blocks matrix.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "down_size", &RBMMixedConnection::down_size,
                    OptionBase::learntoption,
                    "It is computed from the sizes of the sub-blocks.");

    redeclareOption(ol, "up_size", &RBMMixedConnection::up_size,
                    OptionBase::learntoption,
                    "It is computed from the sizes of the sub-blocks.");

    redeclareOption(ol, "initialization_method",
                    &RBMMixedConnection::initialization_method,
                    OptionBase::nosave,
                    "initialization_method is useless here.");
}

void RBMMixedConnection::build_()
{
    up_size = 0;
    down_size = 0;

    n_up_blocks = sub_connections.length();
    n_down_blocks = sub_connections.width();

    if( n_up_blocks == 0 || n_down_blocks == 0 )
        return;

    up_init_positions.resize( n_up_blocks );
    up_block_sizes.resize( n_up_blocks );
    down_init_positions.resize( n_down_blocks );
    down_block_sizes.resize( n_down_blocks );
    row_of.resize( 0 );
    col_of.resize( 0 );

    // size equality check
    for( int i=0 ; i<n_up_blocks ; i++ )
    {
        up_block_sizes[i] = 0;
        for( int j=0 ; j<n_down_blocks ; j++ )
        {
            if( sub_connections(i,j) )
            {
                if( up_block_sizes[i] == 0 ) // first non-null sub_connection
                    up_block_sizes[i] = sub_connections(i,j)->up_size;
                else
                    PLASSERT( sub_connections(i,j)->up_size ==
                            up_block_sizes[i] );
            }
        }
        up_init_positions[i] = up_size;
        up_size += up_block_sizes[i];
        row_of.append( TVec<int>( up_block_sizes[i], i ) );
    }

    for( int j=0 ; j<n_down_blocks ; j++ )
    {
        down_block_sizes[j] = 0;
        for( int i=0 ; i<n_up_blocks ; i++ )
        {
            if( sub_connections(i,j) )
            {
                if( down_block_sizes[j] == 0 ) // first non-null sub_connection
                    down_block_sizes[j] = sub_connections(i,j)->down_size;
                else
                    PLASSERT( sub_connections(i,j)->down_size ==
                            down_block_sizes[j] );
            }
        }

        down_init_positions[j] = down_size;
        down_size += down_block_sizes[j];
        col_of.append( TVec<int>( down_block_sizes[j], j ) );
    }

    // assign random generator, learning rate and momentum to sub_connections
    for( int i=0 ; i<n_up_blocks ; i++ )
        for( int j=0 ; j<n_down_blocks ; j++ )
        {
            if( sub_connections(i,j) )
            {
                if( learning_rate >= 0. )
                    sub_connections(i,j)->setLearningRate( learning_rate );

                if( momentum >= 0. )
                    sub_connections(i,j)->setMomentum( momentum );

                sub_connections(i,j)->random_gen = random_gen;
            }
        }

    // for OnlineLearningModule interface
    input_size = down_size;
    output_size = up_size;
}

void RBMMixedConnection::build()
{
    inherited::build();
    build_();
}


void RBMMixedConnection::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(sub_connections, copies);
    deepCopyField(up_init_positions, copies);
    deepCopyField(down_init_positions, copies);
    deepCopyField(row_of, copies);
    deepCopyField(col_of, copies);
}


void RBMMixedConnection::setLearningRate( real the_learning_rate )
{
    inherited::setLearningRate( the_learning_rate );

    for( int i=0 ; i<n_up_blocks ; i++ )
        for( int j=0 ; j<n_down_blocks ; j++ )
            if( sub_connections(i,j) )
                sub_connections(i,j)->setLearningRate( the_learning_rate );
}

void RBMMixedConnection::setMomentum( real the_momentum )
{
    inherited::setMomentum( the_momentum );

    for( int i=0 ; i<n_up_blocks ; i++ )
        for( int j=0 ; j<n_down_blocks ; j++ )
            if( sub_connections(i,j) )
                sub_connections(i,j)->setMomentum( the_momentum );
}

void RBMMixedConnection::setAsUpInput( const Vec& input ) const
{
    inherited::setAsUpInput( input );

    for( int i=0 ; i<n_up_blocks ; i++ )
    {
        Vec sub_input = input.subVec( up_init_positions[i],
                                      up_block_sizes[i] );

        for( int j=0 ; j<n_down_blocks ; j++ )
            if( sub_connections(i,j) )
                sub_connections(i,j)->setAsUpInput( sub_input );
    }
}

void RBMMixedConnection::setAsDownInput( const Vec& input ) const
{
    inherited::setAsDownInput( input );

    for( int j=0 ; j<n_down_blocks ; j++ )
    {
        Vec sub_input = input.subVec( down_init_positions[j],
                                      down_block_sizes[j] );

        for( int i=0 ; i<n_up_blocks ; i++ )
            if( sub_connections(i,j) )
                sub_connections(i,j)->setAsDownInput( sub_input );
    }
}

void RBMMixedConnection::accumulatePosStats( const Vec& down_values,
                                             const Vec& up_values )
{
    for( int i=0 ; i<n_up_blocks ; i++ )
    {
        Vec sub_up_values = up_values.subVec( up_init_positions[i],
                                              up_block_sizes[i] );

        for( int j=0 ; j<n_down_blocks ; j++ )
        {
            if( sub_connections(i,j) )
            {
                Vec sub_down_values =
                    down_values.subVec( down_init_positions[j],
                                        sub_connections(i,j)->down_size );

                sub_connections(i,j)->accumulatePosStats( sub_down_values,
                                                          sub_up_values );
            }
        }
    }

    pos_count++;
}

void RBMMixedConnection::accumulateNegStats( const Vec& down_values,
                                             const Vec& up_values )
{
    for( int i=0 ; i<n_up_blocks ; i++ )
    {
        Vec sub_up_values = up_values.subVec( up_init_positions[i],
                                              up_block_sizes[i] );

        for( int j=0 ; j<n_down_blocks ; j++ )
        {
            if( sub_connections(i,j) )
            {
                Vec sub_down_values =
                    down_values.subVec( down_init_positions[j],
                                        sub_connections(i,j)->down_size );

                sub_connections(i,j)->accumulateNegStats( sub_down_values,
                                                          sub_up_values );
            }
        }
    }

    neg_count++;
}

void RBMMixedConnection::update()
{
    for( int i=0 ; i<n_up_blocks ; i++ )
        for( int j=0 ; j<n_down_blocks ; j++ )
            if( sub_connections(i,j) )
                sub_connections(i,j)->update();

    clearStats();
}

// Instead of using the statistics, we assume we have only one markov chain
// runned and we update the parameters from the first 4 values of the chain
void RBMMixedConnection::update( const Vec& pos_down_values, // v_0
                                 const Vec& pos_up_values,   // h_0
                                 const Vec& neg_down_values, // v_1
                                 const Vec& neg_up_values )  // h_1
{
    for( int i=0 ; i<n_up_blocks ; i++ )
    {
        int up_begin = up_init_positions[i];
        int up_length = up_block_sizes[i];
        Vec sub_pos_up_values = pos_up_values.subVec( up_begin, up_length );
        Vec sub_neg_up_values = neg_up_values.subVec( up_begin, up_length );
        for( int j=0 ; j<n_down_blocks ; j++ )
        {
            if( sub_connections(i,j) )
            {
                int down_begin = down_init_positions[j];
                int down_length = sub_connections(i,j)->down_size;
                Vec sub_pos_down_values = pos_down_values.subVec( down_begin,
                                                                  down_length );
                Vec sub_neg_down_values = neg_down_values.subVec( down_begin,
                                                                  down_length );

                sub_connections(i,j)->update( sub_pos_down_values,
                                              sub_pos_up_values,
                                              sub_neg_down_values,
                                              sub_neg_up_values );
            }
        }
    }
}

void RBMMixedConnection::clearStats()
{
    for( int i=0 ; i<n_up_blocks ; i++ )
        for( int j=0 ; j<n_down_blocks ; j++ )
            if( sub_connections(i,j) )
                sub_connections(i,j)->clearStats();

    pos_count = 0;
    neg_count = 0;
}

void RBMMixedConnection::computeProduct( int start, int length,
                                         const Vec& activations,
                                         bool accumulate ) const
{
    PLASSERT( activations.length() == length );

    if( !accumulate )
        activations.subVec( start, length ).fill( 0. );

    if( going_up )
    {
        PLASSERT( start+length <= up_size );

        int init_row = row_of[start];
        int end_row = row_of[start+length-1];

        if( init_row == end_row )
        {
            int start_in_row = start - up_init_positions[init_row];

            for( int j=0 ; j<n_down_blocks ; j++ )
            {
                if( sub_connections(init_row,j) )
                {
                    sub_connections(init_row,j)->computeProduct(
                        start_in_row, length, activations, true );
                }
            }
        }
        else
        {
            // partial computation on init_row
            int start_in_init_row = start - up_init_positions[init_row];
            int len_in_init_row = up_init_positions[init_row+1]
                                    - start_in_init_row;
            int cur_pos = 0;

            Vec sub_activations = activations.subVec( cur_pos,
                                                      len_in_init_row );
            cur_pos += len_in_init_row;
            for( int j=0 ; j<n_down_blocks ; j++ )
            {
                if( sub_connections(init_row,j) )
                {
                    sub_connections(init_row,j)->computeProduct(
                        start_in_init_row, len_in_init_row,
                        sub_activations, true );
                }
            }

            // full computation for init_row < i < end_row
            for( int i=init_row+1 ; i<end_row ; i++ )
            {
                int up_size_i = up_block_sizes[i];
                sub_activations = activations.subVec( cur_pos, up_size_i );
                cur_pos += up_size_i;
                for( int j=0 ; j<n_down_blocks ; j++ )
                {
                    if( sub_connections(i,j) )
                    {
                        sub_connections(i,j)->computeProduct(
                            0, up_size_i, sub_activations, true );
                    }
                }

            }

            // partial computation on end_row
            int len_in_end_row = start+length - up_init_positions[end_row];
            sub_activations = activations.subVec( cur_pos, len_in_end_row );
            cur_pos += len_in_end_row;
            for( int j=0 ; j<n_down_blocks ; j++ )
            {
                if( sub_connections(end_row,j) )
                {
                    sub_connections(end_row,j)->computeProduct(
                        0, len_in_end_row, sub_activations, true );
                }
            }
        }
    }
    else
    {
        PLASSERT( start+length <= down_size );

        int init_col = col_of[start];
        int end_col = col_of[start+length-1];

        if( init_col == end_col )
        {
            int start_in_col = start - down_init_positions[init_col];

            for( int i=0 ; i<n_up_blocks ; i++ )
            {
                if( sub_connections(i,init_col) )
                {
                    sub_connections(i,init_col)->computeProduct(
                        start_in_col, length, activations, true );
                }
            }
        }
        else
        {
            // partial computation on init_col
            int start_in_init_col = start - down_init_positions[init_col];
            int len_in_init_col = down_init_positions[init_col+1]
                                    - start_in_init_col;
            int cur_pos = 0;

            Vec sub_activations = activations.subVec( cur_pos,
                                                      len_in_init_col );
            cur_pos += len_in_init_col;
            for( int i=0 ; i<n_up_blocks ; i++ )
            {
                if( sub_connections(i,init_col) )
                {
                    sub_connections(i,init_col)->computeProduct(
                        start_in_init_col, len_in_init_col,
                        sub_activations, true );
                }
            }

            // full computation for init_col < j < end_col
            for( int j=init_col+1 ; j<end_col ; j++ )
            {
                int down_size_j = down_block_sizes[j];
                sub_activations = activations.subVec( cur_pos, down_size_j );
                cur_pos += down_size_j;
                for( int i=0 ; i<n_up_blocks ; i++ )
                {
                    if( sub_connections(i,j) )
                    {
                        sub_connections(i,j)->computeProduct(
                            0, down_size_j, sub_activations, true );
                    }
                }

            }

            // partial computation on end_row
            int len_in_end_col = start+length - down_init_positions[end_col];
            sub_activations = activations.subVec( cur_pos, len_in_end_col );
            cur_pos += len_in_end_col;
            for( int i=0 ; i<n_up_blocks ; i++ )
            {
                if( sub_connections(i,end_col) )
                {
                    sub_connections(i,end_col)->computeProduct(
                        0, len_in_end_col, sub_activations, true );
                }
            }
        }
    }
}

//! this version allows to obtain the input gradient as well
void RBMMixedConnection::bpropUpdate(const Vec& input, const Vec& output,
                                      Vec& input_gradient,
                                      const Vec& output_gradient)
{
    PLASSERT( input.size() == down_size );
    PLASSERT( output.size() == up_size );
    PLASSERT( output_gradient.size() == up_size );
    input_gradient.resize( down_size );
    input_gradient.clear();

    for( int j=0 ; j<n_down_blocks ; j++ )
    {
        int init_j = down_init_positions[j];
        int down_size_j = down_block_sizes[j];
        Vec sub_input = input.subVec( init_j, down_size_j );
        Vec sub_input_gradient = input_gradient.subVec( init_j, down_size_j );
        Vec part_input_gradient( down_size_j );

        for( int i=0 ; i<n_up_blocks ; i++ )
        {
            if( sub_connections(i,j) )
            {
                int init_i = up_init_positions[i];
                int up_size_i = up_block_sizes[i];
                Vec sub_output = output.subVec( init_i, up_size_i );
                Vec sub_output_gradient = output_gradient.subVec( init_i,
                                                                  up_size_i );
                sub_connections(i,j)->bpropUpdate( sub_input, sub_output,
                                                   part_input_gradient,
                                                   sub_output_gradient );

                sub_input_gradient += part_input_gradient;
            }
        }
    }
}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void RBMMixedConnection::forget()
{
    for( int i=0 ; i<n_up_blocks ; i++ )
        for( int j=0 ; j<n_down_blocks ; j++ )
            if( sub_connections(i,j) )
                sub_connections(i,j)->forget();

    clearStats();
}


/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void RBMMixedConnection::finalize()
{
}
*/

//! return the number of parameters
int RBMMixedConnection::nParameters() const
{
    return 0;
}

//! Make the parameters data be sub-vectors of the given global_parameters.
//! The argument should have size >= nParameters. The result is a Vec
//! that starts just after this object's parameters end, i.e.
//!    result = global_parameters.subVec(nParameters(),global_parameters.size()-nParameters());
//! This allows to easily chain calls of this method on multiple RBMParameters.
Vec RBMMixedConnection::makeParametersPointHere(const Vec& global_parameters)
{
    return global_parameters;
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
