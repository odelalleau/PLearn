// -*- C++ -*-

// RBMSparse1DMatrixConnection.cc
//
// Copyright (C) 2008 Jerome Louradour
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

// Authors: Jerome Louradour

/*! \file RBMSparse1DMatrixConnection.cc */



#include "RBMSparse1DMatrixConnection.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMSparse1DMatrixConnection,
    "RBM connections with sparses weights, designed for 1D inputs.",
    "");

RBMSparse1DMatrixConnection::RBMSparse1DMatrixConnection( real the_learning_rate ) :
    filter_size(-1),
    enforce_positive_weights(false)
{
}

void RBMSparse1DMatrixConnection::declareOptions(OptionList& ol)
{
    declareOption(ol, "filter_size", &RBMSparse1DMatrixConnection::filter_size,
                  OptionBase::buildoption,
                  "Length of each filter. If -1 then input_size is taken (RBMMatrixConnection).");

    declareOption(ol, "enforce_positive_weights", &RBMSparse1DMatrixConnection::enforce_positive_weights,
                  OptionBase::buildoption,
                  "Whether or not to enforce having positive weights.");

    declareOption(ol, "step_size", &RBMSparse1DMatrixConnection::step_size,
                  OptionBase::learntoption,
                  "Step between each filter.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////////////
// declareMethods //
////////////////////
void RBMSparse1DMatrixConnection::declareMethods(RemoteMethodMap& rmm)
{
    // Insert a backpointer to remote methods; note that this is different from
    // declareOptions().
    rmm.inherited(inherited::_getRemoteMethodMap_());
    declareMethod(
        rmm, "getWeights", &RBMSparse1DMatrixConnection::getWeights,
        (BodyDoc("Returns the full weights (including 0s).\n"),
         RetDoc ("Matrix of weights (n_hidden x input_size)")));
}

void RBMSparse1DMatrixConnection::build_()
{
    if( up_size <= 0 || down_size <= 0 )
        return;

    if( filter_size < 0 )
        filter_size = down_size;
        
    step_size = (int)((real)(down_size-filter_size)/(real)(up_size-1));
        
    PLASSERT( filter_size <= down_size );

    bool needs_forget = false; // do we need to reinitialize the parameters?

    if( weights.length() != up_size ||
        weights.width() != filter_size )
    {
        weights.resize( up_size, filter_size );
        needs_forget = true;
    }

    weights_pos_stats.resize( up_size, filter_size );
    weights_neg_stats.resize( up_size, filter_size );

    if( momentum != 0. )
        weights_inc.resize( up_size, filter_size );

    if( needs_forget ) {
        forget();
    }
    
    clearStats();
}

void RBMSparse1DMatrixConnection::build()
{
    RBMConnection::build();
    build_();
}

int RBMSparse1DMatrixConnection::filterStart(int idx) const
{
    return step_size*idx;
}

int RBMSparse1DMatrixConnection::filterSize(int idx) const
{
    return filter_size;
}

Mat RBMSparse1DMatrixConnection::getWeights() const
{
    Mat w( up_size, down_size);
    w.clear();
    for ( int i=0; i<up_size; i++)
        w(i).subVec( filterStart(i), filterSize(i) ) << weights(i);
    return w;
}

////////////////////////
// accumulateStats //
////////////////////////
void RBMSparse1DMatrixConnection::accumulatePosStats( const Mat& down_values,
                                              const Mat& up_values )
{
    int mbs=down_values.length();
    PLASSERT(up_values.length()==mbs);
    // weights_pos_stats += up_values * down_values'
    for ( int i=0; i<up_size; i++)
        transposeProductAcc( weights_pos_stats(i),
                             down_values.subMatColumns( filterStart(i), filterSize(i) ),
                             up_values(i));
    pos_count+=mbs;
}

void RBMSparse1DMatrixConnection::accumulateNegStats( const Mat& down_values,
                                              const Mat& up_values )
{
    int mbs=down_values.length();
    PLASSERT(up_values.length()==mbs);
    // weights_neg_stats += up_values * down_values'
    for ( int i=0; i<up_size; i++)
        transposeProductAcc( weights_neg_stats(i),
                             down_values.subMatColumns( filterStart(i), filterSize(i) ),
                             up_values(i));
    neg_count+=mbs;
}

////////////////////
// computeProduct //
////////////////////
void RBMSparse1DMatrixConnection::computeProducts(int start, int length,
                                          Mat& activations,
                                          bool accumulate ) const
{
    PLASSERT( activations.width() == length );
    activations.resize(inputs_mat.length(), length);
    if( going_up )
    {
        PLASSERT( start+length <= up_size );
        // activations(k, i-start) += sum_j weights(i,j) inputs_mat(k, j)
        if( accumulate )
            for (int i=start; i<start+length; i++)
                productAcc( activations.column(i-start).toVec(),
                            inputs_mat.subMatColumns( filterStart(i), filterSize(i) ),
                            weights(i) );
        else
            for (int i=start; i<start+length; i++)
                product( activations.column(i-start).toVec(),
                         inputs_mat.subMatColumns( filterStart(i), filterSize(i) ),
                         weights(i) );
    }
    else
    {
        PLASSERT( start+length <= down_size );
        if( !accumulate )
            activations.clear();
        // activations(k, i-start) += sum_j weights(j,i) inputs_mat(k, j)
        Mat all_activations(inputs_mat.length(), down_size);
        all_activations.subMatColumns( start, length ) << activations;
        for (int i=0; i<up_size; i++)
        {
            externalProductAcc( all_activations.subMatColumns( filterStart(i), filterSize(i) ),
                                inputs_mat.column(i).toVec(),
                                weights(i) );
        }
        activations << all_activations.subMatColumns( start, length );
    }
}

///////////
// fprop //
///////////
void RBMSparse1DMatrixConnection::fprop(const Vec& input, const Mat& rbm_weights,
                          Vec& output) const
{
    PLERROR("RBMSparse1DMatrixConnection::fprop not implemented.");
}

/////////////////
// bpropUpdate //
/////////////////
void RBMSparse1DMatrixConnection::bpropUpdate(const Mat& inputs, const Mat& outputs,
                                      Mat& input_gradients,
                                      const Mat& output_gradients,
                                      bool accumulate)
{
    PLASSERT( inputs.width() == down_size );
    PLASSERT( outputs.width() == up_size );
    PLASSERT( output_gradients.width() == up_size );

    if( accumulate )
        PLASSERT_MSG( input_gradients.width() == down_size &&
                      input_gradients.length() == inputs.length(),
                      "Cannot resize input_gradients and accumulate into it" );
    else {
        input_gradients.resize(inputs.length(), down_size);
        input_gradients.clear();
    }  
        
    for (int i=0; i<up_size; i++) {
        int filter_start= filterStart(i), length= filterSize(i);
        
        // input_gradients = output_gradient * weights
        externalProductAcc( input_gradients.subMatColumns( filter_start, length ),
                            output_gradients.column(i).toVec(),
                            weights(i));

        // weights -= learning_rate/n * output_gradients' * inputs
        transposeProductScaleAcc( weights(i),
                                  inputs.subMatColumns( filter_start, length ),
                                  output_gradients.column(i).toVec(),
                                  -learning_rate / inputs.length(), real(1));

        if( enforce_positive_weights )
            for (int j=0; j<filter_size; j++)
                weights(i,j)= max( real(0), weights(i,j) );
   }

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0)) 
        applyWeightPenalty();
}



////////////////////
// bpropAccUpdate //
////////////////////
void RBMSparse1DMatrixConnection::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                         const TVec<Mat*>& ports_gradient)
{
    //TODO: add weights as port?
    PLASSERT( ports_value.length() == nPorts()
              && ports_gradient.length() == nPorts() );

    Mat* down = ports_value[0];
    //Mat* up = ports_value[1];
    Mat* down_grad = ports_gradient[0];
    Mat* up_grad = ports_gradient[1];

    PLASSERT( down && !down->isEmpty() );
    PLASSERT( up && !up->isEmpty() );

    int batch_size = down->length();
    PLASSERT( up->length() == batch_size );

    // If we have up_grad
    if( up_grad && !up_grad->isEmpty() )
    {
        // down_grad should not be provided
        PLASSERT( !down_grad || down_grad->isEmpty() );
        PLASSERT( up_grad->length() == batch_size );
        PLASSERT( up_grad->width() == up_size );

        bool compute_down_grad = false;
        if( down_grad && down_grad->isEmpty() )
        {
            compute_down_grad = true;
            PLASSERT( down_grad->width() == down_size );
            down_grad->resize(batch_size, down_size);
        }
        
        for (int i=0; i<up_size; i++) {
            int filter_start= filterStart(i), length= filterSize(i);
            
            // propagate gradient
            // input_gradients = output_gradient * weights
            if( compute_down_grad )
                externalProductAcc( down_grad->subMatColumns( filter_start, length ),
                                    up_grad->column(i).toVec(),
                                    weights(i));

            // update weights
            // weights -= learning_rate/n * output_gradients' * inputs
            transposeProductScaleAcc( weights(i),
                                      down->subMatColumns( filter_start, length ),
                                      up_grad->column(i).toVec(),
                                      -learning_rate / batch_size, real(1));

            if( enforce_positive_weights )
                for (int j=0; j<filter_size; j++)
                    weights(i,j)= max( real(0), weights(i,j) );
       }
    }
    else if( down_grad && !down_grad->isEmpty() )
    {
        PLERROR("down-up gradient not implemented in RBMSparse1DMatrixConnection::bpropAccUpdate.");

        PLASSERT( down_grad->length() == batch_size );
        PLASSERT( down_grad->width() == down_size );
    }
    else
        PLCHECK_MSG( false,
                     "Unknown port configuration" );

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0)) 
        applyWeightPenalty();
}

void RBMSparse1DMatrixConnection::update( const Mat& pos_down_values, // v_0
                                  const Mat& pos_up_values,   // h_0
                                  const Mat& neg_down_values, // v_1
                                  const Mat& neg_up_values )  // h_1
{
    // weights += learning_rate * ( h_0 v_0' - h_1 v_1' );
    // or:
    // weights[i][j] += learning_rate * (h_0[i] v_0[j] - h_1[i] v_1[j]);

    PLASSERT( pos_up_values.width() == weights.length() );
    PLASSERT( neg_up_values.width() == weights.length() );
    PLASSERT( pos_down_values.width() == down_size );
    PLASSERT( neg_down_values.width() == down_size );

    if( momentum == 0. )
    {
        // We use the average gradient over a mini-batch.
        real avg_lr = learning_rate / pos_down_values.length();

        for (int i=0; i<up_size; i++) {
            int filter_start= filterStart(i), length= filterSize(i);

            transposeProductScaleAcc( weights(i),
                                      pos_down_values.subMatColumns( filter_start, length ),
                                      pos_up_values.column(i).toVec(), 
                                      avg_lr, real(1));

            transposeProductScaleAcc( weights(i),
                                      neg_down_values.subMatColumns( filter_start, length ),
                                      neg_up_values.column(i).toVec(),
                                      -avg_lr, real(1));

            if( enforce_positive_weights )
                for (int j=0; j<filter_size; j++)
                    weights(i,j)= max( real(0), weights(i,j) );
        }
    }
    else
        PLERROR("RBMSparse1DMatrixConnection::update minibatch with momentum - Not implemented");

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0)) 
        applyWeightPenalty();
}

////////////
// forget //
////////////
// Reset the parameters to the state they would be BEFORE starting training.
void RBMSparse1DMatrixConnection::forget()
{
    clearStats();
    if( initialization_method == "zero" )
        weights.clear();
    else
    {
        if( !random_gen ) {
            PLWARNING( "RBMSparse1DMatrixConnection: cannot forget() without"
                       " random_gen" );
            return;
        }
        real d = 1. / max( filter_size, up_size );
        if( initialization_method == "uniform_sqrt" )
            d = sqrt( d );

        if( enforce_positive_weights )
            random_gen->fill_random_uniform( weights, real(0), d );
        else
            random_gen->fill_random_uniform( weights, -d, d );
    }
    L2_n_updates = 0;
}

//! return the number of parameters
int RBMSparse1DMatrixConnection::nParameters() const
{
    return weights.size();
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
