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
    inherited(the_learning_rate),
    delta_ma_param(0.1),
    stationarity_statistic_threshold(2),
    fast_mean(0), slow_mean(0),
    gibbs_fast_ma_coefficient(0), gibbs_fast_ma_param(0),
    gibbs_slow_ma_coefficient(0), gibbs_slow_ma_param(0),
    var_of_value(0),sum_fast2(0),sum_slow2(0),sum_slowfast(0)
{
}

void RBMMatrixConnection::declareOptions(OptionList& ol)
{
    declareOption(ol, "weights", &RBMMatrixConnection::weights,
                  OptionBase::learntoption,
                  "Matrix containing unit-to-unit weights (up_size ×"
                  " down_size)");

    declareOption(ol, "delta_ma_param", &RBMMatrixConnection::delta_ma_param,
                  OptionBase::buildoption,
                  "This option is used only when the background Gibbs is computed (i.e. update*Gibbs() is called).\n"
                  "delta_ma_param is the increment in the parameters that control the moving average coefficients\n"
                  "used to detect non-stationarity in the negative phase activity statistics.\n"
                  "The moving average coefficient are obtained by sigmoid(parameter).\n"
                  "Two moving averages are kept, a slow one and a fast one. When their\n"
                  "difference is statistically significant the moving averages are slowed down.\n");

    declareOption(ol, "stationarity_statistic_threshold", &RBMMatrixConnection::stationarity_statistic_threshold,
                  OptionBase::buildoption,
                  "This option is used only when the background Gibbs is computed (i.e. update*Gibbs() is called).\n"
                  "When the absolute value of the difference of the slow mean and the fast mean\n"
                  "divided by the standard deviation of this difference is above this threshold,\n"
                  "the moving averages are slowed down by increasing their parameter by delta_ma_param.\n"
                  "When this stastitic is less than half the threshold, the moving averages are accelerated\n"
                  "by decreasing their parameter by delta_ma_param.\n");

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

////////////////////////
// accumulateNegStats //
////////////////////////
void RBMMatrixConnection::accumulateNegStats( const Vec& down_values,
                                              const Vec& up_values )
{
    // weights_neg_stats += up_values * down_values'
    externalProductAcc( weights_neg_stats, up_values, down_values );

    neg_count++;
}

////////////
// update //
////////////
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

void RBMMatrixConnection::update( const Mat& pos_down_values, // v_0
                                  const Mat& pos_up_values,   // h_0
                                  const Mat& neg_down_values, // v_1
                                  const Mat& neg_up_values )  // h_1
{
    // weights -= learning_rate * ( h_0 v_0' - h_1 v_1' );
    // or:
    // weights[i][j] += learning_rate * (h_1[i] v_1[j] - h_0[i] v_0[j]);

    PLASSERT( pos_up_values.width() == weights.length() );
    PLASSERT( neg_up_values.width() == weights.length() );
    PLASSERT( pos_down_values.width() == weights.width() );
    PLASSERT( neg_down_values.width() == weights.width() );

    if( momentum == 0. )
    {
        // We use the average gradient over a mini-batch.
        real avg_lr = learning_rate / pos_down_values.length();

        productScaleAcc(weights, pos_up_values, true, pos_down_values, false,
                -avg_lr, 1.);

        productScaleAcc(weights, neg_up_values, true, neg_down_values, false,
                avg_lr, 1.);
    }
    else
    {
        PLERROR("RBMMatrixConnection::update minibatch with momentum - Not implemented");
        /*
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
         */
    }
}


void RBMMatrixConnection::updateCDandGibbs( const Mat& pos_down_values,
                                            const Mat& pos_up_values,
                                            const Mat& cd_neg_down_values,
                                            const Mat& cd_neg_up_values,
                                            const Mat& gibbs_neg_down_values,
                                            const Mat& gibbs_neg_up_values,
                                            real background_gibbs_update_ratio)
{
    real normalize_factor = 1.0/pos_down_values.length();
    // neg_stats <-- gibbs_chain_statistics_forgetting_factor * neg_stats
    //              +(1-gibbs_chain_statistics_forgetting_factor)
    //               * gibbs_neg_up_values'*gibbs_neg_down_values/minibatch_size
    if (neg_count==0)
        productScaleAcc(weights_neg_stats,
                        gibbs_neg_up_values,true,
                        gibbs_neg_down_values,false,normalize_factor,0);
    else
        productScaleAcc(weights_neg_stats,
                        gibbs_neg_up_values,true,
                        gibbs_neg_down_values,false,
                        normalize_factor*(1-gibbs_slow_ma_coefficient),
                        gibbs_slow_ma_coefficient);
    neg_count++;

    // delta w = -lrate * ( pos_up_values'*pos_down_values
    //                   - ( background_gibbs_update_ratio*neg_stats
    //                      +(1-background_gibbs_update_ratio)
    //                       * cd_neg_up_values'*cd_neg_down_values/minibatch_size))
    productScaleAcc(weights,
                    pos_up_values, true,
                    pos_down_values, false,-learning_rate*normalize_factor,1.);
    multiplyAcc(weights, weights_neg_stats,
                learning_rate*background_gibbs_update_ratio);
    productScaleAcc(weights,
                    cd_neg_up_values, true,
                    cd_neg_down_values, false,
                    learning_rate*(1-background_gibbs_update_ratio)*normalize_factor,1.);
}

void RBMMatrixConnection::updateGibbs( const Mat& pos_down_values,
                                       const Mat& pos_up_values,
                                       const Mat& gibbs_neg_down_values,
                                       const Mat& gibbs_neg_up_values)
{
    real normalize_factor = 1.0/pos_down_values.length();
    // neg_stats <-- gibbs_chain_statistics_forgetting_factor * neg_stats
    //              +(1-gibbs_chain_statistics_forgetting_factor)
    //               * gibbs_neg_up_values'*gibbs_neg_down_values
    static Mat tmp;
    tmp.resize(weights.length(),weights.width());
    productScaleAcc(tmp,gibbs_neg_up_values,true,gibbs_neg_down_values,false,1,0);
    if (neg_count==0)
        multiply(weights_neg_stats,tmp,normalize_factor);
    else
        multiplyScaledAdd(tmp,gibbs_slow_ma_coefficient,
                          normalize_factor*(1-gibbs_slow_ma_coefficient),
                          weights_neg_stats);

    neg_count++;

    // update gibbs chain statistics for checking when to change the ma_coefficient

    // this is the statistic that summarizes what is happening in one minibatch (we could use something else)
    real value = sum(tmp)*normalize_factor/tmp.size();
    // fast moving average of the value
    fast_mean = gibbs_fast_ma_coefficient*fast_mean + (1-gibbs_fast_ma_coefficient)*value;
    // slow moving average of the value
    slow_mean = gibbs_slow_ma_coefficient*fast_mean + (1-gibbs_slow_ma_coefficient)*value;
    // moving average estimator of the variance of the value
    var_of_value = gibbs_slow_ma_coefficient*var_of_value + (1-gibbs_slow_ma_coefficient)*(value-slow_mean)*(value-slow_mean);
    // now construct moving sums in order to compute Var(fast_mean-slow_mean)
    sum_fast2 = gibbs_fast_ma_coefficient*gibbs_fast_ma_coefficient*sum_fast2 + 
        (1-gibbs_fast_ma_coefficient)*(1-gibbs_fast_ma_coefficient);
    sum_slow2 = gibbs_slow_ma_coefficient*gibbs_slow_ma_coefficient*sum_slow2 + 
        (1-gibbs_slow_ma_coefficient)*(1-gibbs_slow_ma_coefficient);
    sum_slowfast = gibbs_slow_ma_coefficient*gibbs_fast_ma_coefficient*sum_slowfast + 
        (1-gibbs_slow_ma_coefficient)*(1-gibbs_fast_ma_coefficient);
    // this is Var(fast_mean-slow_mean):
    real var_of_mean_difference = var_of_value * (sum_fast2 + sum_slow2 - 2*sum_slowfast);
    if (var_of_mean_difference>0)
    {
        real stationarity_statistic = fabs(fast_mean - slow_mean)/sqrt(var_of_mean_difference);
        if (stationarity_statistic>stationarity_statistic_threshold) // things are changing too fast
        {
            gibbs_fast_ma_param-=delta_ma_param;
            gibbs_slow_ma_param-=2*delta_ma_param;
            gibbs_fast_ma_coefficient = sigmoid(gibbs_fast_ma_param);
            gibbs_slow_ma_coefficient = sigmoid(gibbs_slow_ma_param);
        }
        else if (stationarity_statistic<0.5*stationarity_statistic_threshold // things are not changing fast enough...
                 && gibbs_slow_ma_param <10) // but there is not point in going TOO slow
        {
            gibbs_fast_ma_param+=delta_ma_param;
            gibbs_slow_ma_param+=2*delta_ma_param;
            gibbs_fast_ma_coefficient = sigmoid(gibbs_fast_ma_param);
            gibbs_slow_ma_coefficient = sigmoid(gibbs_slow_ma_param);
        }
    }
    else PLWARNING("non-positive variance?");


    // delta w = -lrate * ( pos_up_values'*pos_down_values/minibatch_size - neg_stats )
    productScaleAcc(weights,
                    pos_up_values, true,
                    pos_down_values, false,-learning_rate*normalize_factor,1.);
    multiplyAcc(weights, weights_neg_stats,learning_rate);
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

    gibbs_fast_ma_param = -1;
    gibbs_slow_ma_param = 0;
    gibbs_fast_ma_coefficient = sigmoid(gibbs_fast_ma_param);
    gibbs_slow_ma_coefficient = sigmoid(gibbs_slow_ma_param);
    var_of_value = 0.25;
    sum_fast2 = (1 - gibbs_fast_ma_coefficient)*(1 - gibbs_fast_ma_coefficient);
    sum_slow2 = (1 - gibbs_slow_ma_coefficient)*(1 - gibbs_slow_ma_coefficient);
    sum_slowfast = (1 - gibbs_slow_ma_coefficient)*(1 - gibbs_fast_ma_coefficient);
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
            productAcc(activations,
                    inputs_mat,
                    weights.subMatColumns(start,length) );
        else
            product(activations,
                    inputs_mat,
                    weights.subMatColumns(start,length) );
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

void RBMMatrixConnection::bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate)
{
    PLASSERT( inputs.width() == down_size );
    PLASSERT( outputs.width() == up_size );
    PLASSERT( output_gradients.width() == up_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == down_size &&
                      input_gradients.length() == inputs.length(),
                      "Cannot resize input_gradients and accumulate into it" );

        // input_gradients += output_gradient * weights
        productAcc(input_gradients, output_gradients, weights);
    }
    else
    {
        input_gradients.resize(inputs.length(), down_size);
        // input_gradients = output_gradient * weights
        product(input_gradients, output_gradients, weights);
    }

    // weights -= learning_rate/n * output_gradients' * inputs
    productScaleAcc(weights, output_gradients, true, inputs, false,
            -learning_rate / inputs.length(), 1.);
}
 
////////////
// forget //
////////////
// Reset the parameters to the state they would be BEFORE starting training.
// Note that this method is necessarily called from build().
void RBMMatrixConnection::forget()
{
    clearStats();
    if( initialization_method == "zero" )
        weights.clear();
    else
    {
        if( !random_gen )
        {
            PLWARNING( "RBMMatrixConnection: cannot forget() without"
                       " random_gen" );
            return;
        }

        real d = 1. / max( down_size, up_size );
        if( initialization_method == "uniform_sqrt" )
            d = sqrt( d );

        random_gen->fill_random_uniform( weights, -d, d );
    }
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
