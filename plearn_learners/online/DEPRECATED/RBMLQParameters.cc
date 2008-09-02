// -*- C++ -*-

// RBMLQParameters.cc
//
// Copyright (C) 2006 Dan Popovici
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

// Authors: Dan Popovici

/*! \file PLearn/plearn_learners/online/DEPRECATED/RBMLQParameters.cc */



#include "RBMLQParameters.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMLQParameters,
    "Stores and learns the parameters of an RBM between one quadratic layer at the bottom and one linear layer at the top",
    "");

RBMLQParameters::RBMLQParameters( real the_learning_rate ) :
    inherited(the_learning_rate)
{
}

RBMLQParameters::RBMLQParameters( string down_types, string up_types,
                                  real the_learning_rate ) :
    inherited( down_types, up_types, the_learning_rate )
{
    // We're not sure inherited::build() has been called
    build();
}

void RBMLQParameters::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "weights", &RBMLQParameters::weights,
                  OptionBase::learntoption,
                  "Matrix containing unit-to-unit weights (output_size ×"
                  " input_size)");

    declareOption(ol, "down_units_bias",
                  &RBMLQParameters::down_units_bias,
                  OptionBase::learntoption,
                  "Element i contains the bias of up unit i");

    declareOption(ol, "up_units_params",
                  &RBMLQParameters::up_units_params,
                  OptionBase::learntoption,
                  "Element 0,i contains the bias of down unit i. Element 1,i"
                  "contains the quadratic term of down unit i ");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMLQParameters::build_()
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
            PLERROR( "RBMLQParameters::build_() - value '%c' for"
                     " up_units_types[%d]\n"
                     "should be 'l'.\n",
                     dut_i, i );
    }
    
    up_units_params.resize( 2 ) ;
    up_units_params[0].resize( up_layer_size );
    up_units_params[1].resize( up_layer_size );

    up_units_params_pos_stats.resize( 2 );
    up_units_params_pos_stats[0].resize( up_layer_size );
    up_units_params_pos_stats[1].resize( up_layer_size );
    
    up_units_params_neg_stats.resize( 2 );
    up_units_params_neg_stats[0].resize( up_layer_size );
    up_units_params_neg_stats[1].resize( up_layer_size );

    for( int i=0 ; i<up_layer_size ; i++ )
    {
        char uut_i = up_units_types[i];
        if( uut_i != 'q' ) // not quadratic activation unit
            PLERROR( "RBMLQParameters::build_() - value '%c' for"
                     " down_units_types[%d]\n"
                     "should be 'q'.\n",
                     uut_i, i );
    }

    if( needs_forget )
        forget();

    clearStats();
}

void RBMLQParameters::build()
{
    inherited::build();
    build_();
}


void RBMLQParameters::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(weights, copies);
    deepCopyField(down_units_bias, copies);
    deepCopyField(up_units_params, copies);
    deepCopyField(weights_pos_stats, copies);
    deepCopyField(weights_neg_stats, copies);
    deepCopyField(down_units_bias_pos_stats, copies);
    deepCopyField(down_units_bias_neg_stats, copies);
    deepCopyField(up_units_params_pos_stats, copies);
    deepCopyField(up_units_params_neg_stats, copies);
}

void RBMLQParameters::accumulatePosStats( const Vec& down_values,
                                          const Vec& up_values )
{
    // weights_pos_stats += up_values * down_values'
    externalProductAcc( weights_pos_stats, up_values, down_values );

    down_units_bias_pos_stats += down_values;
    up_units_params_pos_stats[0] += up_values;

    for(int i=0 ; i<up_layer_size ; ++i) { 
        up_units_params_pos_stats[1][i] += 2 * up_units_params[1][i] *
            up_values[i] * up_values[i];
    }
    
    pos_count++;
}

void RBMLQParameters::accumulateNegStats( const Vec& down_values,
                                               const Vec& up_values )
{
    // weights_pos_stats += up_values * down_values'
    externalProductAcc( weights_neg_stats, up_values, down_values );

    down_units_bias_neg_stats += down_values;
    up_units_params_neg_stats[0] += up_values;

    for(int i=0 ; i<up_layer_size ; ++i) { 
        up_units_params_neg_stats[1][i] += 2 * up_units_params[1][i] *
            up_values[i] *up_values[i];
    }
    
    neg_count++;

    
}

void RBMLQParameters::update()
{
    // updates parameters
    //weights -= learning_rate * (weights_pos_stats/pos_count
    //                              - weights_neg_stats/neg_count)
    weights_pos_stats /= pos_count;
    weights_neg_stats /= neg_count;
    weights_pos_stats -= weights_neg_stats;
    weights_pos_stats *= learning_rate;
    weights -= weights_pos_stats;
    
    for( int i=0 ; i<up_layer_size ; i++ )
    {
        down_units_bias[i] -=
            learning_rate * (down_units_bias_pos_stats[i]/pos_count
                             - down_units_bias_neg_stats[i]/neg_count);
    }

    for( int i=0 ; i<down_layer_size ; i++ )
    {
        up_units_params[0][i] -=
            learning_rate * (up_units_params_pos_stats[0][i]/pos_count
                             - up_units_params_neg_stats[0][i]/neg_count);
        
        up_units_params[1][i] -=
            learning_rate * (up_units_params_pos_stats[1][i]/pos_count
                             - up_units_params_neg_stats[1][i]/neg_count);
    }

    clearStats();
}

void RBMLQParameters::clearStats()
{
    weights_pos_stats.clear();
    weights_neg_stats.clear();

    up_units_params_pos_stats[0].clear();
    up_units_params_pos_stats[1].clear();
    
    up_units_params_neg_stats[0].clear();
    up_units_params_neg_stats[1].clear();

    down_units_bias_pos_stats.clear();
    down_units_bias_neg_stats.clear();

    pos_count = 0;
    neg_count = 0;
}

void RBMLQParameters::computeUnitActivations
    ( int start, int length, const Vec& activations ) const
{
    //activations[2 * i] = mu of unit (i - start)
    //activations[2 * i + 1] = sigma of unit (i - start)
    if( going_up )
    {

        // TODO: change it to work with start and length
        PLASSERT( start+length <= down_layer_size );
        Mat activations_mat = activations.toMat( activations.length()/2 , 2);
        Mat mu = activations_mat.column(0) ; 
        Mat sigma = activations_mat.column(1) ; 
        
        product( mu , weights , input_vec.toMat(input_vec.length(),1) );


        // activations[i-start] = sum_j weights(j,i) input_vec[j] + b[i]
        for(int i=0 ; i<length ; ++i) { 
            real a_i = up_units_params[1][i] ; 
            mu[i][0] = - (mu[i][0] + up_units_params[0][i]) / (2 * a_i * a_i)     ; 
            sigma[i][0] = 1 / (2. * a_i * a_i) ; 
        }
        
    }
    else
    {
        PLASSERT( activations.length() == length );
        PLASSERT( start+length <= up_layer_size );
        // mu = activations[i] = -(sum_j weights(i,j) input_vec[j] + b[i])
        //                    / (2 * up_units_params[i][1]^2)
//        product( weights, input_vec , activations) ;
        transposeProduct( activations , weights, input_vec ) ;
        activations += down_units_bias ; 
    }
}

inline double cube(double x){
    return x*x*x ; 
}
//! this version allows to obtain the input gradient as well
void RBMLQParameters::bpropUpdate(const Vec& input, const Vec& output,
                                  Vec& input_gradient,
                                  const Vec& output_gradient)
{
    //TODO: clean up the code a bit
    PLASSERT( input.size() == down_layer_size );
    PLASSERT( output.size() == 2 * up_layer_size );
    PLASSERT( output_gradient.size() == 2 * up_layer_size );
    input_gradient.resize( down_layer_size );

    // weights -= learning_rate * output_gradient * input'
//    externalProductAcc( weights, (-learning_rate)*output_gradient, input );

    Vec scaled_out_grad(up_layer_size) ;  
    
    Vec prod_w_input( up_layer_size ) ; 
    
    for(int i=0 ; i<up_layer_size ; ++i) 
    {
        real a_i_square = up_units_params[1][i] * up_units_params[1][i] ; 
        
        scaled_out_grad[i] = -0.5 * output_gradient[2 * i] / a_i_square ; 
        
//        up_units_params[0][i] -= learning_rate * ( -0.5 / a_i_square ) *
//                                 output_gradient[2*i] ; 
        
        for(int j=0 ; j < down_layer_size ; ++j) {             
            prod_w_input[i] += weights[i][j] * input[j] ; 
        }
    }

    // input_gradient = weights' * output_gradient
    transposeProduct( input_gradient, weights, scaled_out_grad );

    externalProductAcc( weights, (-learning_rate)*scaled_out_grad, input );
    
    // (up) bias -= learning_rate * output_gradient
    multiplyAcc( up_units_params[0], scaled_out_grad, -learning_rate );

    for(int i=0 ; i<up_layer_size ; ++i) { 
      up_units_params[1][i] -= learning_rate * ( up_units_params[0][i] +
                prod_w_input[i] ) / (cube(up_units_params[1][i])) *
          output_gradient[2*i] ; 
    }

    // (up) bias -= learning_rate * output_gradient
//    multiplyAcc( up_units_params[0], output_gradient, -learning_rate );

}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void RBMLQParameters::forget()
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

    up_units_params[0].clear();    
    up_units_params[1].fill(1.);
    
    down_units_bias.clear();

    clearStats();
}


/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void RBMLQParameters::finalize()
{
}
*/
//! return the number of parameters
int RBMLQParameters::nParameters(bool share_up_params, bool share_down_params) const
{
    int m = weights.size() +  (share_down_params?down_units_bias.size():0);
    if (share_up_params)
        for (int i=0;i<up_units_params.length();i++)
            m += up_units_params[i].size();
    return m;
}

//! Make the parameters data be sub-vectors of the given global_parameters.
//! The argument should have size >= nParameters. The result is a Vec
//! that starts just after this object's parameters end, i.e.
//!    result = global_parameters.subVec(nParameters(),global_parameters.size()-nParameters());
//! This allows to easily chain calls of this method on multiple RBMParameters.
Vec RBMLQParameters::makeParametersPointHere(const Vec& global_parameters, bool share_up_params, bool share_down_params)
{
    int n = nParameters(share_up_params,share_down_params);
    int m = global_parameters.size();
    if (m<n)
        PLERROR("RBMLLParameters::makeParametersPointHere: argument has length %d, should be longer than nParameters()=%d",m,n);
    real* p = global_parameters.data();
    weights.makeSharedValue(p,weights.size());
    p+=weights.size();
    if (share_down_params)
    {
        down_units_bias.makeSharedValue(p,down_units_bias.size());
        p+=down_units_bias.size();
    }
    if (share_up_params)
        for (int i=0;i<up_units_params.length();i++)
        {
            up_units_params[i].makeSharedValue(p,up_units_params[i].size());
            p+=up_units_params[i].size();
        }
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
