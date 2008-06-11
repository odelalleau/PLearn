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
    gibbs_ma_increment(0.1),
    gibbs_initial_ma_coefficient(0.1),
    L1_penalty_factor(0),
    L2_penalty_factor(0),
    L2_decrease_constant(0),
    L2_shift(100),
    L2_decrease_type("one_over_t"),
    L2_n_updates(0)
{
}

void RBMMatrixConnection::declareOptions(OptionList& ol)
{
    declareOption(ol, "weights", &RBMMatrixConnection::weights,
                  OptionBase::learntoption,
                  "Matrix containing unit-to-unit weights (up_size  x"
                  " down_size)");

    declareOption(ol, "gibbs_ma_schedule", &RBMMatrixConnection::gibbs_ma_schedule,
                  OptionBase::buildoption,
                  "Each element of this vector is a number of updates after which\n"
                  "the moving average coefficient is incremented (by incrementing\n"
                  "its inverse sigmoid by gibbs_ma_increment). After the last\n"
                  "increase has been made, the moving average coefficient stays constant.\n");

    declareOption(ol, "gibbs_ma_increment",
                  &RBMMatrixConnection::gibbs_ma_increment,
                  OptionBase::buildoption,
                  "The increment in the inverse sigmoid of the moving "
                  "average coefficient\n"
                  "to apply after the number of updates reaches an element "
                  "of the gibbs_ma_schedule.\n");

    declareOption(ol, "gibbs_initial_ma_coefficient",
                  &RBMMatrixConnection::gibbs_initial_ma_coefficient,
                  OptionBase::buildoption,
                  "Initial moving average coefficient for the negative phase "
                  "statistics in the Gibbs chain.\n");

    declareOption(ol, "L1_penalty_factor",
                  &RBMMatrixConnection::L1_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L1 regularization term, i.e.\n"
                  "minimize L1_penalty_factor * sum_{ij} |weights(i,j)| "
                  "during training.\n");

    declareOption(ol, "L2_penalty_factor",
                  &RBMMatrixConnection::L2_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L2 regularization term, i.e.\n"
                  "minimize 0.5 * L2_penalty_factor * sum_{ij} weights(i,j)^2 "
                  "during training.\n");

    declareOption(ol, "L2_decrease_constant",
                  &RBMMatrixConnection::L2_decrease_constant,
                  OptionBase::buildoption,
        "Parameter of the L2 penalty decrease (see L2_decrease_type).",
        OptionBase::advanced_level);

    declareOption(ol, "L2_shift",
                  &RBMMatrixConnection::L2_shift,
                  OptionBase::buildoption,
        "Parameter of the L2 penalty decrease (see L2_decrease_type).",
        OptionBase::advanced_level);

    declareOption(ol, "L2_decrease_type",
                  &RBMMatrixConnection::L2_decrease_type,
                  OptionBase::buildoption,
        "The kind of L2 decrease that is being applied. The decrease\n"
        "consists in scaling the L2 penalty by a factor that depends on the\n"
        "number 't' of times this penalty has been used to modify the\n"
        "weights of the connection. It can be one of:\n"
        " - 'one_over_t': 1 / (1 + t * L2_decrease_constant)\n"
        " - 'sigmoid_like': sigmoid((L2_shift - t) * L2_decrease_constant)",
        OptionBase::advanced_level);

    declareOption(ol, "L2_n_updates",
                  &RBMMatrixConnection::L2_n_updates,
                  OptionBase::learntoption,
        "Number of times that weights have been changed by the L2 penalty\n"
        "update rule.");



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

void RBMMatrixConnection::accumulatePosStats( const Mat& down_values,
                                              const Mat& up_values )
{
    int mbs=down_values.length();
    PLASSERT(up_values.length()==mbs);
    // weights_pos_stats += up_values * down_values'
    transposeProductAcc(weights_pos_stats, up_values, down_values);
    pos_count+=mbs;
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

void RBMMatrixConnection::accumulateNegStats( const Mat& down_values,
                                              const Mat& up_values )
{
    int mbs=down_values.length();
    PLASSERT(up_values.length()==mbs);
    // weights_neg_stats += up_values * down_values'
    transposeProductAcc(weights_neg_stats, up_values, down_values);
    neg_count+=mbs;
}

////////////
// update //
////////////
void RBMMatrixConnection::update()
{
    // updates parameters
    //weights += learning_rate * (weights_pos_stats/pos_count
    //                              - weights_neg_stats/neg_count)
    real pos_factor = learning_rate / pos_count;
    real neg_factor = -learning_rate / neg_count;

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

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();

    clearStats();
}

// Instead of using the statistics, we assume we have only one markov chain
// runned and we update the parameters from the first 4 values of the chain
void RBMMatrixConnection::update( const Vec& pos_down_values, // v_0
                                  const Vec& pos_up_values,   // h_0
                                  const Vec& neg_down_values, // v_1
                                  const Vec& neg_up_values )  // h_1
{
    // weights += learning_rate * ( h_0 v_0' - h_1 v_1' );
    // or:
    // weights[i][j] += learning_rate * (h_0[i] v_0[j] - h_1[i] v_1[j]);

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
                w_i[j] += learning_rate * (*puv_i * pdv[j] - *nuv_i * ndv[j]);
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
                    + learning_rate * (*puv_i * pdv[j] - *nuv_i * ndv[j]);
                w_i[j] += winc_i[j];
            }
    }

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();
}

void RBMMatrixConnection::update( const Mat& pos_down_values, // v_0
                                  const Mat& pos_up_values,   // h_0
                                  const Mat& neg_down_values, // v_1
                                  const Mat& neg_up_values )  // h_1
{
    // weights += learning_rate * ( h_0 v_0' - h_1 v_1' );
    // or:
    // weights[i][j] += learning_rate * (h_0[i] v_0[j] - h_1[i] v_1[j]);

    PLASSERT( pos_up_values.width() == weights.length() );
    PLASSERT( neg_up_values.width() == weights.length() );
    PLASSERT( pos_down_values.width() == weights.width() );
    PLASSERT( neg_down_values.width() == weights.width() );

    if( momentum == 0. )
    {
        // We use the average gradient over a mini-batch.
        real avg_lr = learning_rate / pos_down_values.length();

        transposeProductScaleAcc(weights, pos_up_values, pos_down_values,
                                 avg_lr, real(1));

        transposeProductScaleAcc(weights, neg_up_values, neg_down_values,
                                 -avg_lr, real(1));
    }
    else
    {
        PLERROR("RBMMatrixConnection::update minibatch with momentum - Not implemented");
        /*
        // ensure that weights_inc has the right size
        weights_inc.resize( l, w );

        // The update rule becomes:
        // weights_inc = momentum * weights_inc
        //               + learning_rate * ( h_0 v_0' - h_1 v_1' );
        // weights += weights_inc;

        real* winc_i = weights_inc.data();
        int winc_mod = weights_inc.mod();
        for( int i=0 ; i<l ; i++, w_i += w_mod, winc_i += winc_mod,
                             puv_i++, nuv_i++ )
            for( int j=0 ; j<w ; j++ )
            {
                winc_i[j] = momentum * winc_i[j]
                    + learning_rate * (*puv_i * pdv[j] - *nuv_i * ndv[j]);
                w_i[j] += winc_i[j];
            }
         */
    }

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();
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
        transposeProductScaleAcc(weights_neg_stats, gibbs_neg_up_values,
                                 gibbs_neg_down_values,
                                 normalize_factor, real(0));
    else
        transposeProductScaleAcc(weights_neg_stats,
                                 gibbs_neg_up_values,
                                 gibbs_neg_down_values,
                                 normalize_factor*(1-gibbs_ma_coefficient),
                                 gibbs_ma_coefficient);
    neg_count++;

    // delta w = lrate * ( pos_up_values'*pos_down_values
    //                   - ( background_gibbs_update_ratio*neg_stats
    //                      +(1-background_gibbs_update_ratio)
    //                       * cd_neg_up_values'*cd_neg_down_values/minibatch_size))
    transposeProductScaleAcc(weights, pos_up_values, pos_down_values,
                             learning_rate*normalize_factor, real(1));
    multiplyAcc(weights, weights_neg_stats,
                -learning_rate*background_gibbs_update_ratio);
    transposeProductScaleAcc(weights, cd_neg_up_values, cd_neg_down_values,
        -learning_rate*(1-background_gibbs_update_ratio)*normalize_factor,
        real(1));

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();
}

void RBMMatrixConnection::updateGibbs( const Mat& pos_down_values,
                                       const Mat& pos_up_values,
                                       const Mat& gibbs_neg_down_values,
                                       const Mat& gibbs_neg_up_values)
{
    int minibatch_size = pos_down_values.length();
    real normalize_factor = 1.0/minibatch_size;
    // neg_stats <-- gibbs_chain_statistics_forgetting_factor * neg_stats
    //              +(1-gibbs_chain_statistics_forgetting_factor)
    //               * gibbs_neg_up_values'*gibbs_neg_down_values
    static Mat tmp;
    tmp.resize(weights.length(),weights.width());
    transposeProduct(tmp, gibbs_neg_up_values, gibbs_neg_down_values);

    if (neg_count==0)
        multiply(weights_neg_stats,tmp,normalize_factor);
    else
        multiplyScaledAdd(tmp,gibbs_ma_coefficient,
                          normalize_factor*(1-gibbs_ma_coefficient),
                          weights_neg_stats);

    neg_count++;

    bool increase_ma=false;
    for (int i=0;i<gibbs_ma_schedule.length();i++)
        if (gibbs_ma_schedule[i]==neg_count*minibatch_size)
        {
            increase_ma=true;
            break;
        }
    if (increase_ma)
    {
        gibbs_ma_coefficient = sigmoid(gibbs_ma_increment + inverse_sigmoid(gibbs_ma_coefficient));
        cout << "new coefficient = " << gibbs_ma_coefficient << " at example " << neg_count*minibatch_size << endl;
    }

    // delta w = lrate * ( pos_up_values'*pos_down_values/minibatch_size - neg_stats )
    transposeProductScaleAcc(weights, pos_up_values, pos_down_values,
                             learning_rate*normalize_factor, real(1));
    multiplyAcc(weights, weights_neg_stats, -learning_rate);

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();
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

    gibbs_ma_coefficient = gibbs_initial_ma_coefficient;
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
    PLASSERT( activations.width() == length );
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

///////////
// fprop //
///////////
void RBMMatrixConnection::fprop(const Vec& input, const Mat& rbm_weights,
                          Vec& output) const
{
    product( output, rbm_weights, input );
}

///////////////////
// getAllWeights //
///////////////////
void RBMMatrixConnection::getAllWeights(Mat& rbm_weights) const
{
    rbm_weights = weights;
}

///////////////////
// setAllWeights //
///////////////////
void RBMMatrixConnection::setAllWeights(const Mat& rbm_weights)
{
    weights = rbm_weights;
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

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();
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
    transposeProductScaleAcc(weights, output_gradients, inputs,
                             -learning_rate / inputs.length(), real(1));

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();
}

void RBMMatrixConnection::petiteCulotteOlivierUpdate(
    const Vec& input, const Mat& rbm_weights,
    const Vec& output,
    Vec& input_gradient,
    Mat& rbm_weights_gradient,
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

        // input_gradient += rbm_weights' * output_gradient
        transposeProductAcc( input_gradient, rbm_weights, output_gradient );

        // rbm_weights_gradient += output_gradient' * input
        externalProductAcc( rbm_weights_gradient, output_gradient,
                            input);

    }
    else
    {
        input_gradient.resize( down_size );

        // input_gradient = rbm_weights' * output_gradient
        transposeProduct( input_gradient, rbm_weights, output_gradient );

        // rbm_weights_gradient = output_gradient' * input
        externalProduct( rbm_weights_gradient, output_gradient,
                         input);
    }

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        addWeightPenalty(rbm_weights, rbm_weights_gradient);
}


////////////////////
// bpropAccUpdate //
////////////////////
void RBMMatrixConnection::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                         const TVec<Mat*>& ports_gradient)
{
    //TODO: add weights as port?
    PLASSERT( ports_value.length() == nPorts()
              && ports_gradient.length() == nPorts() );

    Mat* down = ports_value[0];
    Mat* up = ports_value[1];
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

        // If we want down_grad
        if( down_grad && down_grad->isEmpty() )
        {
            PLASSERT( down_grad->width() == down_size );
            down_grad->resize(batch_size, down_size);

            // down_grad = up_grad * weights
            product(*down_grad, *up_grad, weights);
        }

        // weights -= learning_rate/n * up_grad' * down
        transposeProductScaleAcc(weights, *up_grad, *down,
                                 -learning_rate/batch_size, real(1));
    }
    else if( down_grad && !down_grad->isEmpty() )
    {
        PLASSERT( down_grad->length() == batch_size );
        PLASSERT( down_grad->width() == down_size );

        // If we wand up_grad
        if( up_grad && up_grad->isEmpty() )
        {
            PLASSERT( up_grad->width() == up_size );
            up_grad->resize(batch_size, up_size);

            // up_grad = down_grad * weights'
            productTranspose(*up_grad, *down_grad, weights);
        }

        // weights = -learning_rate/n * up' * down_grad
        transposeProductScaleAcc(weights, *up, *down_grad,
                                 -learning_rate/batch_size, real(1));
    }
    else
        PLCHECK_MSG( false,
                     "Unknown port configuration" );

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();
}


/////////////
// bpropCD //
/////////////
void RBMMatrixConnection::petiteCulotteOlivierCD(Mat& weights_gradient,
                                                 bool accumulate)
{
    int l = weights_gradient.length();
    int w = weights_gradient.width();

    real* w_i = weights_gradient.data();
    real* wps_i = weights_pos_stats.data();
    real* wns_i = weights_neg_stats.data();
    int w_mod = weights_gradient.mod();
    int wps_mod = weights_pos_stats.mod();
    int wns_mod = weights_neg_stats.mod();

    if(accumulate)
    {
        for( int i=0 ; i<l ; i++, w_i+=w_mod, wps_i+=wps_mod, wns_i+=wns_mod )
            for( int j=0 ; j<w ; j++ )
                w_i[j] += wns_i[j]/pos_count - wps_i[j]/neg_count;
    }
    else
    {
        for( int i=0 ; i<l ; i++, w_i+=w_mod, wps_i+=wps_mod, wns_i+=wns_mod )
            for( int j=0 ; j<w ; j++ )
                w_i[j] = wns_i[j]/pos_count - wps_i[j]/neg_count;
    }

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        addWeightPenalty(weights, weights_gradient);
}

// Instead of using the statistics, we assume we have only one markov chain
// runned and we update the parameters from the first 4 values of the chain
void RBMMatrixConnection::petiteCulotteOlivierCD(
    const Vec& pos_down_values, // v_0
    const Vec& pos_up_values,   // h_0
    const Vec& neg_down_values, // v_1
    const Vec& neg_up_values, // h_1
    Mat& weights_gradient,
    bool accumulate)
{
    int l = weights.length();
    int w = weights.width();
    PLASSERT( pos_up_values.length() == l );
    PLASSERT( neg_up_values.length() == l );
    PLASSERT( pos_down_values.length() == w );
    PLASSERT( neg_down_values.length() == w );

    real* w_i = weights_gradient.data();
    real* puv_i = pos_up_values.data();
    real* nuv_i = neg_up_values.data();
    real* pdv = pos_down_values.data();
    real* ndv = neg_down_values.data();
    int w_mod = weights_gradient.mod();

    if(accumulate)
    {
        for( int i=0 ; i<l ; i++, w_i += w_mod, puv_i++, nuv_i++ )
            for( int j=0 ; j<w ; j++ )
                w_i[j] +=  *nuv_i * ndv[j] - *puv_i * pdv[j] ;
    }
    else
    {
        for( int i=0 ; i<l ; i++, w_i += w_mod, puv_i++, nuv_i++ )
            for( int j=0 ; j<w ; j++ )
                w_i[j] =  *nuv_i * ndv[j] - *puv_i * pdv[j] ;
    }

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        addWeightPenalty(weights, weights_gradient);
}

////////////////////////
// applyWeightPenalty //
////////////////////////
void RBMMatrixConnection::applyWeightPenalty()
{
    // Apply penalty (decay) on weights.
    real delta_L1 = learning_rate * L1_penalty_factor;
    real delta_L2 = learning_rate * L2_penalty_factor;
    if (L2_decrease_type == "one_over_t")
        delta_L2 /= (1 + L2_decrease_constant * L2_n_updates);
    else if (L2_decrease_type == "sigmoid_like")
        delta_L2 *= sigmoid((L2_shift - L2_n_updates) * L2_decrease_constant);
    else
        PLERROR("In RBMMatrixConnection::applyWeightPenalty - Invalid value "
                "for L2_decrease_type: %s", L2_decrease_type.c_str());
    for( int i=0; i<up_size; i++)
    {
        real* w_ = weights[i];
        for( int j=0; j<down_size; j++ )
        {
            if( delta_L2 != 0. )
                w_[j] *= (1 - delta_L2);

            if( delta_L1 != 0. )
            {
                if( w_[j] > delta_L1 )
                    w_[j] -= delta_L1;
                else if( w_[j] < -delta_L1 )
                    w_[j] += delta_L1;
                else
                    w_[j] = 0.;
            }
        }
    }
    if (delta_L2 > 0)
        L2_n_updates++;
}

//////////////////////
// addWeightPenalty //
//////////////////////
void RBMMatrixConnection::addWeightPenalty(Mat weights, Mat weight_gradients)
{
    // Add penalty (decay) gradient.
    real delta_L1 = L1_penalty_factor;
    real delta_L2 = L2_penalty_factor;
    PLASSERT_MSG( is_equal(L2_decrease_constant, 0) && is_equal(L2_shift, 100),
                  "L2 decrease not implemented in this method" );
    for( int i=0; i<weights.length(); i++)
    {
        real* w_ = weights[i];
        real* gw_ = weight_gradients[i];
        for( int j=0; j<weights.width(); j++ )
        {
            if( delta_L2 != 0. )
                gw_[j] += delta_L2*w_[j];

            if( delta_L1 != 0. )
            {
                if( w_[j] > 0 )
                    gw_[j] += delta_L1;
                else if( w_[j] < 0 )
                    gw_[j] -= delta_L1;
            }
        }
    }
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

        //random_gen->manual_seed(1827);

        real d = 1. / max( down_size, up_size );
        if( initialization_method == "uniform_sqrt" )
            d = sqrt( d );

        random_gen->fill_random_uniform( weights, -d, d );
    }
    L2_n_updates = 0;
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
