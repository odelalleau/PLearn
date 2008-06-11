// -*- C++ -*-

// RBMDiagonalMatrixConnection.cc
//
// Copyright (C) 2006 Hugo Larochelle
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

/*! \file RBMDiagonalMatrixConnection.cc */



#include "RBMDiagonalMatrixConnection.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMDiagonalMatrixConnection,
    "Stores and learns the parameters between two linear layers of an RBM",
    "");

RBMDiagonalMatrixConnection::RBMDiagonalMatrixConnection( real the_learning_rate ) :
    inherited(the_learning_rate)
{
}

void RBMDiagonalMatrixConnection::declareOptions(OptionList& ol)
{
    declareOption(ol, "weights_diag", &RBMDiagonalMatrixConnection::weights_diag,
                  OptionBase::learntoption,
                  "Vector containing the diagonal of the weight matrix.\n");

    declareOption(ol, "L1_penalty_factor",
                  &RBMDiagonalMatrixConnection::L1_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L1 regularization term, i.e.\n"
                  "minimize L1_penalty_factor * sum_{ij} |weights(i,j)| "
                  "during training.\n");

    declareOption(ol, "L2_penalty_factor",
                  &RBMDiagonalMatrixConnection::L2_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L2 regularization term, i.e.\n"
                  "minimize 0.5 * L2_penalty_factor * sum_{ij} weights(i,j)^2 "
                  "during training.\n");

    declareOption(ol, "L2_decrease_constant",
                  &RBMDiagonalMatrixConnection::L2_decrease_constant,
                  OptionBase::buildoption,
        "Parameter of the L2 penalty decrease (see L2_decrease_type).",
        OptionBase::advanced_level);

    declareOption(ol, "L2_shift",
                  &RBMDiagonalMatrixConnection::L2_shift,
                  OptionBase::buildoption,
        "Parameter of the L2 penalty decrease (see L2_decrease_type).",
        OptionBase::advanced_level);

    declareOption(ol, "L2_decrease_type",
                  &RBMDiagonalMatrixConnection::L2_decrease_type,
                  OptionBase::buildoption,
        "The kind of L2 decrease that is being applied. The decrease\n"
        "consists in scaling the L2 penalty by a factor that depends on the\n"
        "number 't' of times this penalty has been used to modify the\n"
        "weights of the connection. It can be one of:\n"
        " - 'one_over_t': 1 / (1 + t * L2_decrease_constant)\n"
        " - 'sigmoid_like': sigmoid((L2_shift - t) * L2_decrease_constant)",
        OptionBase::advanced_level);

    declareOption(ol, "L2_n_updates",
                  &RBMDiagonalMatrixConnection::L2_n_updates,
                  OptionBase::learntoption,
        "Number of times that weights have been changed by the L2 penalty\n"
        "update rule.");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMDiagonalMatrixConnection::build_()
{
    if( up_size <= 0 || down_size <= 0 )
        return;

    if( up_size != down_size )
        PLERROR("In RBMDiagonalMatrixConnection::build_(): up_size should be "
            "equal to down_size");

    bool needs_forget = false; // do we need to reinitialize the parameters?

    if( weights_diag.length() != up_size )
    {
        weights_diag.resize( up_size );
        needs_forget = true;
    }

    weights_pos_stats.resize( up_size );
    weights_neg_stats.resize( up_size );

    if( momentum != 0. )
        weights_inc.resize( up_size );

    if( needs_forget )
        forget();

    clearStats();
}

void RBMDiagonalMatrixConnection::build()
{
    inherited::build();
    build_();
}


void RBMDiagonalMatrixConnection::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(weights_diag, copies);
    deepCopyField(weights_pos_stats, copies);
    deepCopyField(weights_neg_stats, copies);
    deepCopyField(weights_inc, copies);
}

void RBMDiagonalMatrixConnection::accumulatePosStats( const Vec& down_values,
                                              const Vec& up_values )
{
    real* wps = weights_pos_stats.data();
    real* uv = up_values.data();
    real* dv = down_values.data();
    for( int i=0; i<up_size; i++ )
        wps[i] += uv[i]*dv[i];

    pos_count++;
}

void RBMDiagonalMatrixConnection::accumulatePosStats( const Mat& down_values,
                                              const Mat& up_values )
{
    int mbs=down_values.length();
    PLASSERT(up_values.length()==mbs);

    real* wps;
    real* uv;
    real* dv;
    for( int t=0; t<mbs; t++ )
    {
        wps = weights_pos_stats.data();
        uv = up_values[t];
        dv = down_values[t];
        for( int i=0; i<up_size; i++ )
            wps[i] += uv[i]*dv[i];
    }
    pos_count+=mbs;
}

////////////////////////
// accumulateNegStats //
////////////////////////
void RBMDiagonalMatrixConnection::accumulateNegStats( const Vec& down_values,
                                              const Vec& up_values )
{
    real* wns = weights_neg_stats.data();
    real* uv = up_values.data();
    real* dv = down_values.data();
    for( int i=0; i<up_size; i++ )
        wns[i] += uv[i]*dv[i];
    neg_count++;
}

void RBMDiagonalMatrixConnection::accumulateNegStats( const Mat& down_values,
                                              const Mat& up_values )
{
    int mbs=down_values.length();
    PLASSERT(up_values.length()==mbs);

    real* wns;
    real* uv;
    real* dv;
    for( int t=0; t<mbs; t++ )
    {
        wns = weights_neg_stats.data();
        uv = up_values[t];
        dv = down_values[t];
        for( int i=0; i<up_size; i++ )
            wns[i] += uv[i]*dv[i];
    }
    neg_count+=mbs;
}

////////////
// update //
////////////
void RBMDiagonalMatrixConnection::update()
{
    // updates parameters
    //weights += learning_rate * (weights_pos_stats/pos_count
    //                              - weights_neg_stats/neg_count)
    real pos_factor = learning_rate / pos_count;
    real neg_factor = -learning_rate / neg_count;

    int l = weights_diag.length();

    real* w_i = weights_diag.data();
    real* wps_i = weights_pos_stats.data();
    real* wns_i = weights_neg_stats.data();

    if( momentum == 0. )
    {
        // no need to use weights_inc
        for( int i=0 ; i<l ; i++ )
            w_i[i] += pos_factor * wps_i[i] + neg_factor * wns_i[i];
    }
    else
    {
        // ensure that weights_inc has the right size
        weights_inc.resize( l );

        // The update rule becomes:
        // weights_inc = momentum * weights_inc
        //               - learning_rate * (weights_pos_stats/pos_count
        //                                  - weights_neg_stats/neg_count);
        // weights += weights_inc;
        real* winc_i = weights_inc.data();
        for( int i=0 ; i<l ; i++ )
        {
            winc_i[i] = momentum * winc_i[i]
                + pos_factor * wps_i[i] + neg_factor * wns_i[i];
            w_i[i] += winc_i[i];
        }
    }

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();

    clearStats();
}

// Instead of using the statistics, we assume we have only one markov chain
// runned and we update the parameters from the first 4 values of the chain
void RBMDiagonalMatrixConnection::update( const Vec& pos_down_values, // v_0
                                  const Vec& pos_up_values,   // h_0
                                  const Vec& neg_down_values, // v_1
                                  const Vec& neg_up_values )  // h_1
{
    int l = weights_diag.length();
    PLASSERT( pos_up_values.length() == l );
    PLASSERT( neg_up_values.length() == l );
    PLASSERT( pos_down_values.length() == l );
    PLASSERT( neg_down_values.length() == l );

    real* w_i = weights_diag.data();
    real* pdv = pos_down_values.data();
    real* puv = pos_up_values.data();
    real* ndv = neg_down_values.data();
    real* nuv = neg_up_values.data();

    if( momentum == 0. )
    {
        for( int i=0 ; i<l ; i++)
            w_i[i] += learning_rate * (puv[i] * pdv[i] - nuv[i] * ndv[i]);
    }
    else
    {
        // ensure that weights_inc has the right size
        weights_inc.resize( l );

        real* winc_i = weights_inc.data();
        for( int i=0 ; i<l ; i++ )
        {
            winc_i[i] = momentum * winc_i[i]
                + learning_rate * (puv[i] * pdv[i] - nuv[i] * ndv[i]);
            w_i[i] += winc_i[i];
        }
    }

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();
}

void RBMDiagonalMatrixConnection::update( const Mat& pos_down_values, // v_0
                                  const Mat& pos_up_values,   // h_0
                                  const Mat& neg_down_values, // v_1
                                  const Mat& neg_up_values )  // h_1
{
    // weights += learning_rate * ( h_0 v_0' - h_1 v_1' );
    // or:
    // weights[i][j] += learning_rate * (h_0[i] v_0[j] - h_1[i] v_1[j]);

    int l = weights_diag.length();

    PLASSERT( pos_up_values.width() == l );
    PLASSERT( neg_up_values.width() == l );
    PLASSERT( pos_down_values.width() == l );
    PLASSERT( neg_down_values.width() == l );

    real* w_i = weights_diag.data();
    real* pdv;
    real* puv;
    real* ndv;
    real* nuv;

    if( momentum == 0. )
    {
        // We use the average gradient over a mini-batch.
        real avg_lr = learning_rate / pos_down_values.length();

        for( int t=0; t<pos_up_values.length(); t++ )
        {
            pdv = pos_down_values[t];
            puv = pos_up_values[t];
            ndv = neg_down_values[t];
            nuv = neg_up_values[t];
            for( int i=0 ; i<l ; i++)
                w_i[i] += avg_lr * (puv[i] * pdv[i] - nuv[i] * ndv[i]);
        }
    }
    else
    {
        PLERROR("RBMDiagonalMatrixConnection::update minibatch with momentum - Not implemented");
    }

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();
}

////////////////
// clearStats //
////////////////
void RBMDiagonalMatrixConnection::clearStats()
{
    weights_pos_stats.clear();
    weights_neg_stats.clear();

    pos_count = 0;
    neg_count = 0;
}

////////////////////
// computeProduct //
////////////////////
void RBMDiagonalMatrixConnection::computeProduct( int start, int length,
                                          const Vec& activations,
                                          bool accumulate ) const
{
    PLASSERT( activations.length() == length );
    PLASSERT( start+length <= up_size );
    real* act = activations.data();
    real* w = weights_diag.data();
    real* iv = input_vec.data();
    if( accumulate )
        for( int i=0; i<length; i++ )
            act[i] += w[i+start] * iv[i+start];
    else
        for( int i=0; i<length; i++ )
            act[i] = w[i+start] * iv[i+start];
}

/////////////////////
// computeProducts //
/////////////////////
void RBMDiagonalMatrixConnection::computeProducts(int start, int length,
                                          Mat& activations,
                                          bool accumulate ) const
{
    PLASSERT( activations.width() == length );
    activations.resize(inputs_mat.length(), length);
    real* act;
    real* w = weights_diag.data();
    real* iv;
    if( accumulate )
        for( int t=0; t<inputs_mat.length(); t++ )
        {
            act = activations[t];
            iv = inputs_mat[t];
            for( int i=0; i<length; i++ )
                act[i] += w[i+start] * iv[i+start];
        }
    else
        for( int t=0; t<inputs_mat.length(); t++ )
        {
            act = activations[t];
            iv = inputs_mat[t];
            for( int i=0; i<length; i++ )
                act[i] = w[i+start] * iv[i+start];
        }
}

/////////////////
// bpropUpdate //
/////////////////
void RBMDiagonalMatrixConnection::bpropUpdate(const Vec& input, const Vec& output,
                                      Vec& input_gradient,
                                      const Vec& output_gradient,
                                      bool accumulate)
{
    PLASSERT( input.size() == down_size );
    PLASSERT( output.size() == up_size );
    PLASSERT( output_gradient.size() == up_size );

    real* w = weights_diag.data();
    real* in = input.data();
    real* ing = input_gradient.data();
    real* outg = output_gradient.data();
    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == down_size,
                      "Cannot resize input_gradient AND accumulate into it" );

        for( int i=0; i<down_size; i++ )
        {
            ing[i] += outg[i]*w[i];
            w[i] -= learning_rate * in[i] * outg[i];
        }
    }
    else
    {
        input_gradient.resize( down_size );
        for( int i=0; i<down_size; i++ )
        {
            ing[i] = outg[i]*w[i];
            w[i] -= learning_rate * in[i] * outg[i];
        }
    }

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();
}

void RBMDiagonalMatrixConnection::bpropUpdate(const Mat& inputs, const Mat& outputs,
                                      Mat& input_gradients,
                                      const Mat& output_gradients,
                                      bool accumulate)
{
    PLASSERT( inputs.width() == down_size );
    PLASSERT( outputs.width() == up_size );
    PLASSERT( output_gradients.width() == up_size );

    int mbatch = inputs.length();

    real* w = weights_diag.data();
    real* in;
    real* ing;
    real* outg;
    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == down_size &&
                      input_gradients.length() == inputs.length(),
                      "Cannot resize input_gradients and accumulate into it" );

        for( int t=0; t<mbatch; t++ )
        {
            ing = input_gradients[t];
            outg = output_gradients[t];
            for( int i=0; i<down_size; i++ )
                ing[i] += outg[i]*w[i];
        }
    }
    else
    {
        input_gradients.resize(inputs.length(), down_size);
        for( int t=0; t<mbatch; t++ )
        {
            ing = input_gradients[t];
            outg = output_gradients[t];
            for( int i=0; i<down_size; i++ )
                ing[i] = outg[i]*w[i];
        }
    }

    real avg_lr = learning_rate / mbatch;
    for( int t=0; t<mbatch; t++ )
    {
        in = inputs[t];
        outg = output_gradients[t];
        for( int i=0; i<down_size; i++ )
            w[i] -= avg_lr * in[i] * outg[i];
    }

    if(!fast_exact_is_equal(L1_penalty_factor,0) || !fast_exact_is_equal(L2_penalty_factor,0))
        applyWeightPenalty();
}

////////////////////////
// applyWeightPenalty //
////////////////////////
void RBMDiagonalMatrixConnection::applyWeightPenalty()
{
    // Apply penalty (decay) on weights.
    real delta_L1 = learning_rate * L1_penalty_factor;
    real delta_L2 = learning_rate * L2_penalty_factor;
    if (L2_decrease_type == "one_over_t")
        delta_L2 /= (1 + L2_decrease_constant * L2_n_updates);
    else if (L2_decrease_type == "sigmoid_like")
        delta_L2 *= sigmoid((L2_shift - L2_n_updates) * L2_decrease_constant);
    else
        PLERROR("In RBMDiagonalMatrixConnection::applyWeightPenalty - Invalid value "
                "for L2_decrease_type: %s", L2_decrease_type.c_str());
    real* w_ = weights_diag.data();
    for( int i=0; i<down_size; i++ )
    {
        if( delta_L2 != 0. )
            w_[i] *= (1 - delta_L2);

        if( delta_L1 != 0. )
        {
            if( w_[i] > delta_L1 )
                w_[i] -= delta_L1;
            else if( w_[i] < -delta_L1 )
                w_[i] += delta_L1;
            else
                w_[i] = 0.;
        }
    }

    if (delta_L2 > 0)
        L2_n_updates++;
}

//////////////////////
// addWeightPenalty //
//////////////////////
void RBMDiagonalMatrixConnection::addWeightPenalty(Vec weights_diag, Vec weight_diag_gradients)
{
    // Add penalty (decay) gradient.
    real delta_L1 = L1_penalty_factor;
    real delta_L2 = L2_penalty_factor;
    PLASSERT_MSG( is_equal(L2_decrease_constant, 0) && is_equal(L2_shift, 100),
                  "L2 decrease not implemented in this method" );
    real* w_ = weights_diag.data();
    real* gw_ = weight_diag_gradients.data();
    for( int i=0; i<down_size; i++ )
    {
        if( delta_L2 != 0. )
            gw_[i] += delta_L2*w_[i];

        if( delta_L1 != 0. )
        {
            if( w_[i] > 0 )
                gw_[i] += delta_L1;
            else if( w_[i] < 0 )
                gw_[i] -= delta_L1;
        }
    }
}

////////////
// forget //
////////////
// Reset the parameters to the state they would be BEFORE starting training.
// Note that this method is necessarily called from build().
void RBMDiagonalMatrixConnection::forget()
{
    clearStats();
    if( initialization_method == "zero" )
        weights_diag.clear();
    else
    {
        if( !random_gen )
        {
            PLWARNING( "RBMDiagonalMatrixConnection: cannot forget() without"
                       " random_gen" );
            return;
        }

        //random_gen->manual_seed(1827);

        real d = 1. / max( down_size, up_size );
        if( initialization_method == "uniform_sqrt" )
            d = sqrt( d );

        random_gen->fill_random_uniform( weights_diag, -d, d );
    }
    L2_n_updates = 0;
}


/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void RBMDiagonalMatrixConnection::finalize()
{
}
*/

//! return the number of parameters
int RBMDiagonalMatrixConnection::nParameters() const
{
    return weights_diag.size();
}

//! Make the parameters data be sub-vectors of the given global_parameters.
//! The argument should have size >= nParameters. The result is a Vec
//! that starts just after this object's parameters end, i.e.
//!    result = global_parameters.subVec(nParameters(),global_parameters.size()-nParameters());
//! This allows to easily chain calls of this method on multiple RBMParameters.
Vec RBMDiagonalMatrixConnection::makeParametersPointHere(const Vec& global_parameters)
{
    int n=weights_diag.size();
    int m = global_parameters.size();
    if (m<n)
        PLERROR("RBMDiagonalMatrixConnection::makeParametersPointHere: argument has length %d, should be longer than nParameters()=%d",m,n);
    real* p = global_parameters.data();
    weights_diag.makeSharedValue(p,n);

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
