// -*- C++ -*-

// RBMLateralBinomialLayer.cc
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

/*! \file RBMLateralBinomialLayer.cc */



#include "RBMLateralBinomialLayer.h"
#include <plearn/math/TMat_maths.h>
#include "RBMConnection.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMLateralBinomialLayer,
    "Layer in an RBM formed with binomial units and lateral connections.",
    "");

RBMLateralBinomialLayer::RBMLateralBinomialLayer( real the_learning_rate ) :
    inherited( the_learning_rate ),
    n_lateral_connections_passes( 1 ),
    dampening_factor( 0. ),
    mean_field_precision_threshold( 0. ),
    topographic_length( -1 ),
    topographic_width( -1 ),
    topographic_patch_vradius( 5 ),
    topographic_patch_hradius( 5 ),
    topographic_lateral_weights_init_value( 0. ),
    do_not_learn_topographic_lateral_weights( false ),
    use_parametric_mean_field( false )
{
}

void RBMLateralBinomialLayer::reset()
{
    inherited::reset();
    lateral_weights_inc.clear();
}

void RBMLateralBinomialLayer::clearStats()
{
    inherited::clearStats();
    lateral_weights_pos_stats.clear();
    lateral_weights_neg_stats.clear();
}

void RBMLateralBinomialLayer::forget()
{
    inherited::forget();
    //real bu;
    //for( int i=0; i<lateral_weights.length(); i++)
    //    for( int j=0; j<lateral_weights.width(); j++)
    //    {
    //        bu = random_gen->bounded_uniform(-1.0/size,1.0/size);
    //        lateral_weights(i,j) = bu;
    //        lateral_weights(j,i) = bu;
    //    }
    lateral_weights.clear();
    // Set diagonal to 0
    if( lateral_weights.length() != 0 )
    {
        real *d = lateral_weights.data();        
        for (int i=0; i<lateral_weights.length(); i++,d+=lateral_weights.mod()+1) 
            *d = 0;
    }

    for( int i=0; i<topographic_lateral_weights.length(); i++ )
        //topographic_lateral_weights[i].clear();
        topographic_lateral_weights[i].fill( topographic_lateral_weights_init_value );

    mean_field_output_weights.clear();
    for( int i=0; i<mean_field_output_weights.length(); i++ )
        mean_field_output_weights(i,i) = 1;
    for( int i=0; i<mean_field_output_bias.length(); i++ )
        mean_field_output_bias[i] = -0.5;
    
}

////////////////////
// generateSample //
////////////////////
void RBMLateralBinomialLayer::generateSample()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectation_is_up_to_date, "Expectation should be computed "
            "before calling generateSample()");

    for( int i=0 ; i<size ; i++ )
        sample[i] = random_gen->binomial_sample( expectation[i] );    
}

/////////////////////
// generateSamples //
/////////////////////
void RBMLateralBinomialLayer::generateSamples()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectations_are_up_to_date, "Expectations should be computed "
            "before calling generateSamples()");

    PLASSERT( samples.width() == size && samples.length() == batch_size );

    for (int k = 0; k < batch_size; k++) {
        for (int i=0 ; i<size ; i++)
            samples(k, i) = random_gen->binomial_sample( expectations(k, i) );
    }
}

////////////////////////
// computeExpectation //
////////////////////////
void RBMLateralBinomialLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    if( use_parametric_mean_field )
    {
        if (use_fast_approximations)
            for( int i=0 ; i<size ; i++ )
                mean_field_input[i] = fastsigmoid( activation[i] );
        else
            for( int i=0 ; i<size ; i++ )
                mean_field_input[i] = sigmoid( activation[i] );
        
        product(pre_sigmoid_mean_field_output, mean_field_output_weights, mean_field_input);
        pre_sigmoid_mean_field_output += mean_field_output_bias;

        if (use_fast_approximations)
            for( int i=0 ; i<size ; i++ )
                expectation[i] = fastsigmoid( pre_sigmoid_mean_field_output[i] );
        else
            for( int i=0 ; i<size ; i++ )
                expectation[i] = sigmoid( pre_sigmoid_mean_field_output[i] );

        // Update mean-field predictor, using KL-divergence gradient:
        //   dKL/dp_i = -activation[i] - \sum_{j \neq i} p_j + V_i h
        // where - V_i is the ith row of mean_field_output_weights
        //       - h is sigmoid(activation)

        real mean_field_i;
        product(temp_mean_field_gradient, lateral_weights, expectation);
        temp_mean_field_gradient += activation;
        for( int i=0 ; i<size ; i++ )
        {
            mean_field_i = expectation[i];
            temp_mean_field_gradient[i] = (pre_sigmoid_mean_field_output[i] 
                                           - temp_mean_field_gradient[i]) 
                * mean_field_i * (1 - mean_field_i);
        }

        externalProductScaleAcc( mean_field_output_weights, temp_mean_field_gradient, 
                                 mean_field_input, -learning_rate );
        multiplyScaledAdd( temp_mean_field_gradient, 1.0, -learning_rate, mean_field_output_bias);
    }
    else
    {        
        if( temp_output.length() != n_lateral_connections_passes+1 )
        {
            temp_output.resize(n_lateral_connections_passes+1);
            for( int i=0 ; i<n_lateral_connections_passes+1 ; i++ )
                temp_output[i].resize(size);
        }       
        
        current_temp_output = temp_output[0];
        temp_output.last() = expectation;
        
        if (use_fast_approximations)
            for( int i=0 ; i<size ; i++ )
                current_temp_output[i] = fastsigmoid( activation[i] );
        else
            for( int i=0 ; i<size ; i++ )
                current_temp_output[i] = sigmoid( activation[i] );
        
        for( int t=0; t<n_lateral_connections_passes; t++ )
        {
            previous_temp_output = current_temp_output;
            current_temp_output = temp_output[t+1];
            if( topographic_lateral_weights.length() == 0 )
                product(dampening_expectation, lateral_weights, previous_temp_output);
            else
                productTopoLateralWeights( dampening_expectation, previous_temp_output );
            dampening_expectation += activation;
            if (use_fast_approximations)
            {
                if( fast_exact_is_equal( dampening_factor, 0) )
                {
                    for( int i=0 ; i<size ; i++ )
                        current_temp_output[i] = fastsigmoid( dampening_expectation[i] );
                }
                else
                {
                    for( int i=0 ; i<size ; i++ )
                        current_temp_output[i] = 
                            (1-dampening_factor) * fastsigmoid( dampening_expectation[i] ) 
                            + dampening_factor * previous_temp_output[i];
                }
            }
            else
            {
                if( fast_exact_is_equal( dampening_factor, 0) )
                {
                    for( int i=0 ; i<size ; i++ )
                        current_temp_output[i] = sigmoid( dampening_expectation[i] );
                }
                else
                {
                    for( int i=0 ; i<size ; i++ )
                        current_temp_output[i] = 
                            (1-dampening_factor) * sigmoid( dampening_expectation[i] ) 
                            + dampening_factor * previous_temp_output[i];
                }
            }
            if( !fast_exact_is_equal(mean_field_precision_threshold, 0.) && 
                dist(current_temp_output, previous_temp_output,2)/size < mean_field_precision_threshold )
            {
                expectation << current_temp_output;
                break;
            }
            //cout << sqrt(max(square(current_temp_output-previous_temp_output))) << " ";
            //cout << dist(current_temp_output, previous_temp_output,2)/current_temp_output.length() << " ";
        }
        //cout << endl;
        //expectation << current_temp_output;
    }
    expectation_is_up_to_date = true;
}

/////////////////////////
// computeExpectations //
/////////////////////////
void RBMLateralBinomialLayer::computeExpectations()
{
    if( expectations_are_up_to_date )
        return;

    PLASSERT( expectations.width() == size
              && expectations.length() == batch_size );

    if( use_parametric_mean_field )
    {
        PLERROR("RBMLateralBinomialLayer::computeExpectations(): use_parametric_mean_field=true "
            "not implemented yet.");
    }
    else
    {
        dampening_expectations.resize( batch_size, size );
        
        if( temp_outputs.length() != n_lateral_connections_passes+1 )
        {
            temp_outputs.resize(n_lateral_connections_passes+1);
            for( int i=0 ; i<n_lateral_connections_passes+1 ; i++ )
                temp_outputs[i].resize( batch_size, size);
        }       
        
        current_temp_outputs = temp_outputs[0];
        temp_outputs.last() = expectations;
        
        if (use_fast_approximations)
            for (int k = 0; k < batch_size; k++)
                for (int i = 0 ; i < size ; i++)
                    current_temp_outputs(k, i) = fastsigmoid(activations(k, i));
        else
            for (int k = 0; k < batch_size; k++)
                for (int i = 0 ; i < size ; i++)
                    current_temp_outputs(k, i) = sigmoid(activations(k, i));

        for( int t=0; t<n_lateral_connections_passes; t++ )
        {
            previous_temp_outputs = current_temp_outputs;
            current_temp_outputs = temp_outputs[t+1];
            if( topographic_lateral_weights.length() == 0 )
                productTranspose(dampening_expectations, previous_temp_outputs, 
                                 lateral_weights);
            else
                for( int b = 0; b<dampening_expectations.length(); b++)
                    productTopoLateralWeights( dampening_expectations(b), 
                                               previous_temp_outputs(b) );

            dampening_expectations += activations;
            if (use_fast_approximations)
            {
                if( fast_exact_is_equal( dampening_factor, 0) )
                {
                    for(int k = 0; k < batch_size; k++)
                        for( int i=0 ; i<size ; i++ )
                            current_temp_outputs(k, i) = 
                                fastsigmoid( dampening_expectations(k, i) );
                }
                else
                {
                    for(int k = 0; k < batch_size; k++)
                        for( int i=0 ; i<size ; i++ )
                            current_temp_outputs(k, i) = (1-dampening_factor)
                                * fastsigmoid( dampening_expectations(k, i) ) 
                                + dampening_factor * previous_temp_outputs(k, i);
                }
            }
            else
            {
                if( fast_exact_is_equal( dampening_factor, 0) )
                {
                    for(int k = 0; k < batch_size; k++)
                        for( int i=0 ; i<size ; i++ )
                            current_temp_outputs(k, i) = 
                                sigmoid( dampening_expectations(k, i) );
                }
                else
                {
                    for(int k = 0; k < batch_size; k++)
                        for( int i=0 ; i<size ; i++ )
                            current_temp_outputs(k, i) = (1-dampening_factor) 
                                * sigmoid( dampening_expectations(k, i) ) 
                                + dampening_factor * previous_temp_outputs(k, i);
                }
            }
        }
        //expectations << current_temp_outputs;
    }
    expectations_are_up_to_date = true;
}

///////////
// fprop //
///////////
void RBMLateralBinomialLayer::fprop( const Vec& input, Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    add(bias, input, bias_plus_input);

    if( use_parametric_mean_field )
    {
        if (use_fast_approximations)
            for( int i=0 ; i<size ; i++ )
                mean_field_input[i] = fastsigmoid( bias_plus_input[i] );
        else
            for( int i=0 ; i<size ; i++ )
                mean_field_input[i] = sigmoid( bias_plus_input[i] );
        
        product(pre_sigmoid_mean_field_output, mean_field_output_weights, mean_field_input);
        pre_sigmoid_mean_field_output += mean_field_output_bias;

        if (use_fast_approximations)
            for( int i=0 ; i<size ; i++ )
                output[i] = fastsigmoid( pre_sigmoid_mean_field_output[i] );
        else
            for( int i=0 ; i<size ; i++ )
                output[i] = sigmoid( pre_sigmoid_mean_field_output[i] );
    }
    else
    {        

        if( temp_output.length() != n_lateral_connections_passes+1 )
        {
            temp_output.resize(n_lateral_connections_passes+1);
            for( int i=0 ; i<n_lateral_connections_passes+1 ; i++ )
                temp_output[i].resize(size);
        }       

        temp_output.last() = output;
        current_temp_output = temp_output[0];

        if (use_fast_approximations)
            for( int i=0 ; i<size ; i++ )
                current_temp_output[i] = fastsigmoid( bias_plus_input[i] );
        else
            for( int i=0 ; i<size ; i++ )
                current_temp_output[i] = sigmoid( bias_plus_input[i] );

        for( int t=0; t<n_lateral_connections_passes; t++ )
        {
            previous_temp_output = current_temp_output;
            current_temp_output = temp_output[t+1];
            if( topographic_lateral_weights.length() == 0 )
                product(dampening_expectation, lateral_weights, previous_temp_output);
            else
                productTopoLateralWeights( dampening_expectation, previous_temp_output );
            dampening_expectation += bias_plus_input;
            if (use_fast_approximations)
            {
                if( fast_exact_is_equal( dampening_factor, 0) )
                {
                    for( int i=0 ; i<size ; i++ )
                        current_temp_output[i] = fastsigmoid( dampening_expectation[i] );
                }
                else
                {
                    for( int i=0 ; i<size ; i++ )
                        current_temp_output[i] = 
                            (1-dampening_factor) * fastsigmoid( dampening_expectation[i] ) 
                            + dampening_factor * previous_temp_output[i];
                }
            }
            else
            {
                if( fast_exact_is_equal( dampening_factor, 0) )
                {
                    for( int i=0 ; i<size ; i++ )
                        current_temp_output[i] = sigmoid( dampening_expectation[i] );
                }
                else
                {
                    for( int i=0 ; i<size ; i++ )
                        current_temp_output[i] = 
                            (1-dampening_factor) * sigmoid( dampening_expectation[i] ) 
                            + dampening_factor * previous_temp_output[i];
                }
            }
        }
    }
}

void RBMLateralBinomialLayer::fprop( const Mat& inputs, Mat& outputs )
{
    int mbatch_size = inputs.length();
    PLASSERT( inputs.width() == size );
    outputs.resize( mbatch_size, size );

    dampening_expectations.resize( mbatch_size, size );

    if( use_parametric_mean_field )
    {
        PLERROR("RBMLateralBinomialLayer::fprop: use_parametric_mean_field = true "
            "not implemented yet for batch mode.");
    }
    else
    {
        if(bias_plus_inputs.length() != inputs.length() ||
           bias_plus_inputs.width() != inputs.width())
            bias_plus_inputs.resize(inputs.length(), inputs.width());
        bias_plus_inputs << inputs;
        bias_plus_inputs += bias;

        if( temp_outputs.length() != n_lateral_connections_passes+1 )
        {
            temp_outputs.resize(n_lateral_connections_passes+1);
            for( int i=0 ; i<n_lateral_connections_passes+1 ; i++ )
                temp_outputs[i].resize(mbatch_size,size);
        }       

        temp_outputs.last() = outputs;
        current_temp_outputs = temp_outputs[0];

        if (use_fast_approximations)
            for( int k = 0; k < mbatch_size; k++ )
                for( int i = 0; i < size; i++ )
                    current_temp_outputs(k,i) = fastsigmoid( bias_plus_inputs(k,i) );
        else
            for( int k = 0; k < mbatch_size; k++ )
                for( int i = 0; i < size; i++ )
                    current_temp_outputs(k,i) = sigmoid( bias_plus_inputs(k,i) );

        for( int t=0; t<n_lateral_connections_passes; t++ )
        {
            previous_temp_outputs = current_temp_outputs;
            current_temp_outputs = temp_outputs[t+1];
            if( topographic_lateral_weights.length() == 0 )
                productTranspose(dampening_expectations, previous_temp_outputs, 
                                 lateral_weights);
            else
                for( int b = 0; b<dampening_expectations.length(); b++)
                    productTopoLateralWeights( dampening_expectations(b), 
                                               previous_temp_outputs(b) );

            dampening_expectations += bias_plus_inputs;
            if (use_fast_approximations)
            {
                if( fast_exact_is_equal( dampening_factor, 0) )
                {
                    for(int k = 0; k < batch_size; k++)
                        for( int i=0 ; i<size ; i++ )
                            current_temp_outputs(k, i) = 
                                fastsigmoid( dampening_expectations(k, i) );
                }
                else
                {
                    for(int k = 0; k < batch_size; k++)
                        for( int i=0 ; i<size ; i++ )
                            current_temp_outputs(k, i) = (1-dampening_factor)
                                * fastsigmoid( dampening_expectations(k, i) ) 
                                + dampening_factor * previous_temp_outputs(k, i);
                }
            }
            else
            {
                if( fast_exact_is_equal( dampening_factor, 0) )
                {
                    for(int k = 0; k < batch_size; k++)
                        for( int i=0 ; i<size ; i++ )
                            current_temp_outputs(k, i) = 
                                sigmoid( dampening_expectations(k, i) );
                }
                else
                {
                    for(int k = 0; k < batch_size; k++)
                        for( int i=0 ; i<size ; i++ )
                            current_temp_outputs(k, i) = (1-dampening_factor)
                                * sigmoid( dampening_expectations(k, i) ) 
                                + dampening_factor * previous_temp_outputs(k, i);
                }
            }
        }
    }
}

void RBMLateralBinomialLayer::fprop( const Vec& input, const Vec& rbm_bias,
                              Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( rbm_bias.size() == input_size );
    output.resize( output_size );

    add(rbm_bias, input, bias_plus_input);

    if( use_parametric_mean_field )
    {
        PLERROR("RBMLateralBinomialLayer::fprop: use_parametric_mean_field = true "
            "not implemented yet for rbm_bias input.");
    }
    else
    {

        if( temp_output.length() != n_lateral_connections_passes+1 )
        {
            temp_output.resize(n_lateral_connections_passes+1);
            for( int i=0 ; i<n_lateral_connections_passes+1 ; i++ )
                temp_output[i].resize(size);
        }       

        temp_output.last() = output;
        current_temp_output = temp_output[0];

        if (use_fast_approximations)
            for( int i=0 ; i<size ; i++ )
                current_temp_output[i] = fastsigmoid( bias_plus_input[i] );
        else
            for( int i=0 ; i<size ; i++ )
                current_temp_output[i] = sigmoid( bias_plus_input[i] );

        for( int t=0; t<n_lateral_connections_passes; t++ )
        {
            previous_temp_output = current_temp_output;
            current_temp_output = temp_output[t+1];
            if( topographic_lateral_weights.length() == 0 )
                product(dampening_expectation, lateral_weights, previous_temp_output);
            else
                productTopoLateralWeights( dampening_expectation, previous_temp_output );
            dampening_expectation += bias_plus_input;
            if (use_fast_approximations)
            {
                if( fast_exact_is_equal( dampening_factor, 0) )
                {
                    for( int i=0 ; i<size ; i++ )
                        current_temp_output[i] = fastsigmoid( dampening_expectation[i] );
                }
                else
                {
                    for( int i=0 ; i<size ; i++ )
                        current_temp_output[i] = 
                            (1-dampening_factor) * fastsigmoid( dampening_expectation[i] ) 
                            + dampening_factor * previous_temp_output[i];
                }
            }
            else
            {
                if( fast_exact_is_equal( dampening_factor, 0) )
                {
                    for( int i=0 ; i<size ; i++ )
                        current_temp_output[i] = sigmoid( dampening_expectation[i] );
                }
                else
                {
                    for( int i=0 ; i<size ; i++ )
                        current_temp_output[i] = 
                            (1-dampening_factor) * sigmoid( dampening_expectation[i] ) 
                            + dampening_factor * previous_temp_output[i];
                }
            }
        }
    }
}

// HUGO: NO 0.5! Computes mat[i][j] += 0.5 * (v1[i] * v2[j] + v1[j] * v2[i])
// Computes mat[i][j] += (v1[i] * v2[j] + v1[j] * v2[i])
void RBMLateralBinomialLayer::externalSymetricProductAcc(const Mat& mat, const Vec& v1, const Vec& v2)
{
#ifdef BOUNDCHECK
    if (v1.length()!=mat.length() || mat.width()!=v2.length() 
        || v1.length() != v2.length())
        PLERROR("externalSymetricProductAcc(Mat,Vec,Vec), incompatible "
                "arguments sizes");
#endif

    real* v_1=v1.data();
    real* v_2=v2.data();
    real* mp = mat.data();
    int l = mat.length();
    int w = mat.width();

    if(mat.isCompact())
    {
        real* pv11 = v_1;
        real* pv21 = v_2;
        for(int i=0; i<l; i++)
        {
            real* pv22 = v_2;
            real* pv12 = v_1;
            real val1 = *pv11++;
            real val2 = *pv21++;
            for(int j=0; j<w; j++)
                //*mp++ += 0.5 * (val1 * *pv22++ + val2 * *pv12++) ;
                *mp++ += (val1 * *pv22++ + val2 * *pv12++) ;
        }
    }
    else
    {
        cerr << "!";
        for (int i=0;i<l;i++)
        {
            real* mi = mat[i];
            real v1i = v_1[i];
            real v2i = v_2[i];
            for (int j=0;j<w;j++)
                //mi[j] += 0.5 * ( v1i * v_2[j] + v2i * v_1[j]);
                mi[j] += ( v1i * v_2[j] + v2i * v_1[j]);
        }
    }
}

void RBMLateralBinomialLayer::productTopoLateralWeights(const Vec& result, 
                                                        const Vec& input ) const
{
    // Could be made faster, in terms of memory access
    result.clear();
    int connected_neuron;
    int wi;
    real* current_weights;
    int neuron_v, neuron_h;
    int vmin, vmax, hmin, hmax;
    for( int i=0; i<topographic_lateral_weights.length(); i++ )
    {
        neuron_v = i/topographic_width;
        neuron_h = i%topographic_width;
        wi = 0;
        current_weights = topographic_lateral_weights[i].data();
        
        vmin = neuron_v < topographic_patch_vradius ? 
            - neuron_v : - topographic_patch_vradius;
        vmax = topographic_length - neuron_v - 1 < topographic_patch_vradius ? 
            topographic_length - neuron_v - 1: topographic_patch_vradius;

        hmin = neuron_h < topographic_patch_hradius ? 
            - neuron_h : - topographic_patch_hradius;
        hmax = topographic_width - neuron_h - 1 < topographic_patch_hradius ? 
            topographic_width - neuron_h - 1: topographic_patch_hradius;

        for( int j = -1 * topographic_patch_vradius; 
             j <= topographic_patch_vradius ; j++ ) 
        {
            for( int k = -1 * topographic_patch_hradius; 
                 k <= topographic_patch_hradius; k++ )
            {
                connected_neuron = (i+j*topographic_width)+k;
                if( connected_neuron != i )
                {
                    if( j >= vmin && j <= vmax &&
                        k >= hmin && k <= hmax )
                        result[i] += input[connected_neuron]
                            * current_weights[wi];
                    wi++;
                }
            }
        }
    }
}

void RBMLateralBinomialLayer::productTopoLateralWeightsGradients(
    const Vec& input,
    const Vec& input_gradient,
    const Vec& result_gradient,
    const TVec< Vec >& weights_gradient
    )
{
    // Could be made faster, in terms of memory access
    int connected_neuron;
    int wi;
    real* current_weights;
    real* current_weights_gradient;
    int neuron_v, neuron_h;
    int vmin, vmax, hmin, hmax;
    real result_gradient_i;
    real input_i;
    for( int i=0; i<topographic_lateral_weights.length(); i++ )
    {
        neuron_v = i/topographic_width;
        neuron_h = i%topographic_width;
        wi = 0;
        current_weights = topographic_lateral_weights[i].data();
        current_weights_gradient = weights_gradient[i].data();

        vmin = neuron_v < topographic_patch_vradius ? 
            - neuron_v : - topographic_patch_vradius;
        vmax = topographic_length - neuron_v - 1 < topographic_patch_vradius ? 
            topographic_length - neuron_v - 1: topographic_patch_vradius;

        hmin = neuron_h < topographic_patch_hradius ? 
            - neuron_h : - topographic_patch_hradius;
        hmax = topographic_width - neuron_h - 1 < topographic_patch_hradius ? 
            topographic_width - neuron_h - 1: topographic_patch_hradius;

        result_gradient_i = result_gradient[i];
        input_i = input[i];

        for( int j = -1 * topographic_patch_vradius; 
             j <= topographic_patch_vradius ; j++ )
        {
            for( int k = -1 * topographic_patch_hradius; 
                 k <= topographic_patch_hradius; k++ )
            {
                connected_neuron = (i+j*topographic_width)+k;
                if( connected_neuron != i )
                {
                    if( j >= vmin && j <= vmax &&
                        k >= hmin && k <= hmax )
                    {
                        input_gradient[connected_neuron] += 
                            result_gradient_i * current_weights[wi];
                        current_weights_gradient[wi] += 
                            //0.5 * ( result_gradient_i * input[connected_neuron] +
                            ( result_gradient_i * input[connected_neuron] +
                              input_i * result_gradient[connected_neuron] );
                    }
                    wi++;
                }
            }
        }
    }
}

void RBMLateralBinomialLayer::updateTopoLateralWeightsCD(
    const Vec& pos_values,
    const Vec& neg_values  )
{
    if( !do_not_learn_topographic_lateral_weights )
    {
        
        // Could be made faster, in terms of memory access
        int connected_neuron;
        int wi;
        int neuron_v, neuron_h;
        int vmin, vmax, hmin, hmax;
        real* current_weights;
        real pos_values_i;
        real neg_values_i;
        for( int i=0; i<topographic_lateral_weights.length(); i++ )
        {
            neuron_v = i/topographic_width;
            neuron_h = i%topographic_width;
            wi = 0;
            
            vmin = neuron_v < topographic_patch_vradius ? 
                - neuron_v : - topographic_patch_vradius;
            vmax = topographic_length - neuron_v - 1 < topographic_patch_vradius ? 
                topographic_length - neuron_v - 1: topographic_patch_vradius;
            
            hmin = neuron_h < topographic_patch_hradius ? 
                - neuron_h : - topographic_patch_hradius;
            hmax = topographic_width - neuron_h - 1 < topographic_patch_hradius ? 
                topographic_width - neuron_h - 1: topographic_patch_hradius;
            
            current_weights = topographic_lateral_weights[i].data();
            pos_values_i = pos_values[i];
            neg_values_i = neg_values[i];
            
            for( int j = - topographic_patch_vradius; 
                 j <= topographic_patch_vradius ; j++ )
            {
                for( int k = -topographic_patch_hradius; 
                     k <= topographic_patch_hradius; k++ )
                {
                    connected_neuron = (i+j*topographic_width)+k;
                    if( connected_neuron != i )
                    {
                        if( j >= vmin && j <= vmax &&
                            k >= hmin && k <= hmax )
                        {
                            current_weights[wi] += 
                                //learning_rate * 0.5 * ( 
                                learning_rate * ( 
                                    pos_values_i * pos_values[connected_neuron] -
                                    neg_values_i * neg_values[connected_neuron] );
                        }
                        wi++;
                    }
                }
            }
        }
    }
}

/////////////////
// bpropUpdate //
/////////////////
void RBMLateralBinomialLayer::bpropUpdate(const Vec& input, const Vec& output,
                                   Vec& input_gradient,
                                   const Vec& output_gradient,
                                   bool accumulate)
{
    PLASSERT( input.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );

    if( accumulate )
        PLASSERT_MSG( input_gradient.size() == size,
                      "Cannot resize input_gradient AND accumulate into it" );
    else
    {
        input_gradient.resize( size );
        input_gradient.clear();
    }

    //if( momentum != 0. )
    //    bias_inc.resize( size );

    if( use_parametric_mean_field )
    {
        real mean_field_i;
        for( int i=0 ; i<size ; i++ )
        {
            mean_field_i = output[i];
            temp_mean_field_gradient[i] = output_gradient[i] * mean_field_i * (1 - mean_field_i);
        }

        transposeProductAcc( input_gradient, mean_field_output_weights, temp_mean_field_gradient );

        externalProductScaleAcc( mean_field_output_weights, temp_mean_field_gradient, 
                                 mean_field_input, -learning_rate );
        multiplyScaledAdd( temp_mean_field_gradient, 1.0, -learning_rate, mean_field_output_bias);

        real input_mean_field_i;
        for( int i=0 ; i<size ; i++ )
        {
            input_mean_field_i = mean_field_input[i];
            input_gradient[i] = input_gradient[i] * input_mean_field_i * (1 - input_mean_field_i);
        }
    }
    else
    {
        temp_input_gradient.clear();
        temp_mean_field_gradient << output_gradient;
        current_temp_output = output;
        lateral_weights_gradient.clear();
        for( int i=0; i<topographic_lateral_weights_gradient.length(); i++)
            topographic_lateral_weights_gradient[i].clear();

        real output_i;
        for( int t=n_lateral_connections_passes-1 ; t>=0 ; t-- )
        {
            for( int i=0 ; i<size ; i++ )
            {
                output_i = current_temp_output[i];

                // Contribution from the mean field approximation
                temp_mean_field_gradient2[i] =  (1-dampening_factor)*
                    output_i * (1-output_i) * temp_mean_field_gradient[i];
            
                // Contribution from the dampening
                temp_mean_field_gradient[i] *= dampening_factor;
            }

            // Input gradient contribution
            temp_input_gradient += temp_mean_field_gradient2;

            // Lateral weights gradient contribution
            if( topographic_lateral_weights.length() == 0)
            {
                externalSymetricProductAcc( lateral_weights_gradient, 
                                            temp_mean_field_gradient2,
                                            temp_output[t] );
            
                transposeProductAcc(temp_mean_field_gradient, lateral_weights, 
                                    temp_mean_field_gradient2);
            }
            else
            {
                productTopoLateralWeightsGradients( 
                    temp_output[t],
                    temp_mean_field_gradient,
                    temp_mean_field_gradient2,
                    topographic_lateral_weights_gradient);
            }

            current_temp_output = temp_output[t];
        }
    
        for( int i=0 ; i<size ; i++ )
        {
            output_i = current_temp_output[i];
            temp_mean_field_gradient[i] *= output_i * (1-output_i);
        }

        temp_input_gradient += temp_mean_field_gradient;

        input_gradient += temp_input_gradient;

        // Update bias
        real in_grad_i;
        for( int i=0 ; i<size ; i++ )
        {
            in_grad_i = temp_input_gradient[i];
            if( momentum == 0. )
            {
                // update the bias: bias -= learning_rate * input_gradient
                bias[i] -= learning_rate * in_grad_i;
            }
            else
            {
                // The update rule becomes:
                // bias_inc = momentum * bias_inc - learning_rate * input_gradient
                // bias += bias_inc
                bias_inc[i] = momentum * bias_inc[i] - learning_rate * in_grad_i;
                bias[i] += bias_inc[i];
            }
        }

        if( topographic_lateral_weights.length() == 0)
        {
            if( momentum == 0. )
            {
                multiplyScaledAdd( lateral_weights_gradient, 1.0, -learning_rate,
                                   lateral_weights);
            }
            else
            {
                multiplyScaledAdd( lateral_weights_gradient, momentum, -learning_rate,
                                   lateral_weights_inc);
                lateral_weights += lateral_weights_inc;
            }
        }
        else
        {
            if( !do_not_learn_topographic_lateral_weights )
            {
                if( momentum == 0. )
                    for( int i=0; i<topographic_lateral_weights.length(); i++ )
                        multiplyScaledAdd( topographic_lateral_weights_gradient[i], 1.0, 
                                           -learning_rate,
                                           topographic_lateral_weights[i]);
            
                else
                    PLERROR("In RBMLateralBinomialLayer:bpropUpdate - Not implemented for "
                            "topographic weights");
            }
        }

        // Set diagonal to 0
        if( lateral_weights.length() != 0 )
        {
            real *d = lateral_weights.data();        
            for (int i=0; i<lateral_weights.length(); i++,d+=lateral_weights.mod()+1) 
                *d = 0;
        }
    }
}

void RBMLateralBinomialLayer::bpropUpdate(const Mat& inputs, const Mat& outputs,
                                   Mat& input_gradients,
                                   const Mat& output_gradients,
                                   bool accumulate)
{
    PLASSERT( inputs.width() == size );
    PLASSERT( outputs.width() == size );
    PLASSERT( output_gradients.width() == size );

    int mbatch_size = inputs.length();
    PLASSERT( outputs.length() == mbatch_size );
    PLASSERT( output_gradients.length() == mbatch_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == size &&
                input_gradients.length() == mbatch_size,
                "Cannot resize input_gradients and accumulate into it" );
    }
    else
    {
        input_gradients.resize(mbatch_size, size);
        input_gradients.clear();
    }

    //if( momentum != 0. )
    //    bias_inc.resize( size );

    // TODO Can we do this more efficiently? (using BLAS)

    // We use the average gradient over the mini-batch.
    real avg_lr = learning_rate / inputs.length();

    if( use_parametric_mean_field )
    {
        PLERROR("RBMLateralBinomialLayer::bpropUpdate: use_parametric_mean_field=true "
            "not implemented yet for batch mode.");
    }
    else
    {
        lateral_weights_gradient.clear();
        real output_i;
        for (int j = 0; j < mbatch_size; j++)
        {
            temp_input_gradient.clear();
            temp_mean_field_gradient << output_gradients(j);
            current_temp_output = outputs(j);

            for( int t=n_lateral_connections_passes-1 ; t>=0 ; t-- )
            {

                for( int i=0 ; i<size ; i++ )
                {
                    output_i = current_temp_output[i];
                
                    // Contribution from the mean field approximation
                    temp_mean_field_gradient2[i] =  (1-dampening_factor)*
                        output_i * (1-output_i) * temp_mean_field_gradient[i];
                
                    // Contribution from the dampening
                    temp_mean_field_gradient[i] *= dampening_factor;
                }
            
                // Input gradient contribution
                temp_input_gradient += temp_mean_field_gradient2;
            
                // Lateral weights gradient contribution
                if( topographic_lateral_weights.length() == 0)
                {
                
                    externalSymetricProductAcc( lateral_weights_gradient, 
                                                temp_mean_field_gradient2,
                                                temp_outputs[t](j) );
                
                    transposeProductAcc(temp_mean_field_gradient, lateral_weights, 
                                        temp_mean_field_gradient2);
                }
                else
                {
                    productTopoLateralWeightsGradients( 
                        temp_outputs[t](j),
                        temp_mean_field_gradient,
                        temp_mean_field_gradient2,
                        topographic_lateral_weights_gradient);
                }

                current_temp_output = temp_outputs[t](j);
            }
    
            for( int i=0 ; i<size ; i++ )
            {
                output_i = current_temp_output[i];
                temp_mean_field_gradient[i] *= output_i * (1-output_i);
            }

            temp_input_gradient += temp_mean_field_gradient;
        
            input_gradients(j) += temp_input_gradient;

            // Update bias
            real in_grad_i;
            for( int i=0 ; i<size ; i++ )
            {
                in_grad_i = temp_input_gradient[i];
                if( momentum == 0. )
                {
                    // update the bias: bias -= learning_rate * input_gradient
                    bias[i] -= avg_lr * in_grad_i;
                }
                else
                    PLERROR("In RBMLateralBinomialLayer:bpropUpdate - Not implemented for "
                            "momentum with mini-batches");
            }        
        }

        if( topographic_lateral_weights.length() == 0)
        {
            if( momentum == 0. )
                multiplyScaledAdd( lateral_weights_gradient, 1.0, -learning_rate,
                                   lateral_weights);
            else
                PLERROR("In RBMLateralBinomialLayer:bpropUpdate - Not implemented for "
                        "momentum with mini-batches");
        }
        else
        {
            if( !do_not_learn_topographic_lateral_weights )
            {
                if( momentum == 0. )
                    for( int i=0; i<topographic_lateral_weights.length(); i++ )
                        multiplyScaledAdd( topographic_lateral_weights_gradient[i], 1.0, 
                                           -learning_rate,
                                           topographic_lateral_weights[i]);
            
                else
                    PLERROR("In RBMLateralBinomialLayer:bpropUpdate - Not implemented for "
                            "topographic weights");
            }

        }

        // Set diagonal to 0
        if( lateral_weights.length() != 0 )
        {
            real *d = lateral_weights.data();
            for (int i=0; i<lateral_weights.length(); i++,d+=lateral_weights.mod()+1) 
                *d = 0;
        }
    }
}


//! TODO: add "accumulate" here
void RBMLateralBinomialLayer::bpropUpdate(const Vec& input, const Vec& rbm_bias,
                                   const Vec& output,
                                   Vec& input_gradient, Vec& rbm_bias_gradient,
                                   const Vec& output_gradient)
{
    PLASSERT( input.size() == size );
    PLASSERT( rbm_bias.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );
    input_gradient.resize( size );
    rbm_bias_gradient.resize( size );

    if( use_parametric_mean_field )
    {
        PLERROR("RBMLateralBinomialLayer::bpropUpdate: use_parametric_mean_field=true "
                "not implemented yet for bias input.");
    }
    else
    {
        temp_input_gradient.clear();
        temp_mean_field_gradient << output_gradient;
        current_temp_output = output;
        lateral_weights_gradient.clear();

        real output_i;
        for( int t=n_lateral_connections_passes-1 ; t>=0 ; t-- )
        {

            for( int i=0 ; i<size ; i++ )
            {
                output_i = current_temp_output[i];

                // Contribution from the mean field approximation
                temp_mean_field_gradient2[i] =  (1-dampening_factor)*
                    output_i * (1-output_i) * temp_mean_field_gradient[i];
            
                // Contribution from the dampening
                temp_mean_field_gradient[i] *= dampening_factor;
            }

            // Input gradient contribution
            temp_input_gradient += temp_mean_field_gradient2;

            // Lateral weights gradient contribution
            if( topographic_lateral_weights.length() == 0)
            {

                externalSymetricProductAcc( lateral_weights_gradient, 
                                            temp_mean_field_gradient2,
                                            temp_output[t] );
            
                transposeProductAcc(temp_mean_field_gradient, lateral_weights, 
                                    temp_mean_field_gradient2);
            }
            else
            {
                productTopoLateralWeightsGradients( 
                    temp_output[t],
                    temp_mean_field_gradient,
                    temp_mean_field_gradient2,
                    topographic_lateral_weights_gradient);
            }

            current_temp_output = temp_output[t];
        }
    
        for( int i=0 ; i<size ; i++ )
        {
            output_i = current_temp_output[i];
            temp_mean_field_gradient[i] *= output_i * (1-output_i);
        }

        temp_input_gradient += temp_mean_field_gradient;

        input_gradient << temp_input_gradient;
        rbm_bias_gradient << temp_input_gradient;

        if( topographic_lateral_weights.length() == 0)
        {
            if( momentum == 0. )
            {
                multiplyScaledAdd( lateral_weights_gradient, 1.0, -learning_rate,
                                   lateral_weights);
            }
            else
            {
                multiplyScaledAdd( lateral_weights_gradient, momentum, -learning_rate,
                                   lateral_weights_inc);
                lateral_weights += lateral_weights_inc;
            }
        }
        else
        {
            if( !do_not_learn_topographic_lateral_weights )
            {
                if( momentum == 0. )
                    for( int i=0; i<topographic_lateral_weights.length(); i++ )
                        multiplyScaledAdd( topographic_lateral_weights_gradient[i], 1.0, 
                                           -learning_rate,
                                           topographic_lateral_weights[i]);
            
                else
                    PLERROR("In RBMLateralBinomialLayer:bpropUpdate - Not implemented for "
                            "topographic weights");
            }
        }
        
        // Set diagonal to 0
        if( lateral_weights.length() != 0 )
        {
            real *d = lateral_weights.data();
            for (int i=0; i<lateral_weights.length(); i++,d+=lateral_weights.mod()+1) 
                *d = 0;
        }
    }
}

real RBMLateralBinomialLayer::fpropNLL(const Vec& target)
{
    PLASSERT( target.size() == input_size );
    computeExpectation(); 

    real ret = 0;
    real target_i, expectation_i;
    for( int i=0 ; i<size ; i++ )
    {
        target_i = target[i];
        expectation_i = expectation[i];
        // TODO: implement more numerically stable version
        if(!fast_exact_is_equal(target_i,0.0))
            ret -= target_i*safeflog(expectation_i) ;
        if(!fast_exact_is_equal(target_i,1.0))
            ret -= (1-target_i)*safeflog(1-expectation_i);
    }
    return ret;
}

void RBMLateralBinomialLayer::fpropNLL(const Mat& targets, const Mat& costs_column)
{
    computeExpectations(); 

    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );

    for (int k=0;k<batch_size;k++) // loop over minibatch
    {
        real nll = 0;
        real* expectation = expectations[k];
        real* target = targets[k];
        for( int i=0 ; i<size ; i++ ) // loop over outputs
        {
            // TODO: implement more numerically stable version
            if(!fast_exact_is_equal(target[i],0.0))
                nll -= target[i]*safeflog(expectation[i]) ;
            if(!fast_exact_is_equal(target[i],1.0))
                nll -= (1-target[i])*safeflog(1-expectation[i]);
        }
        costs_column(k,0) = nll;
    }
}

void RBMLateralBinomialLayer::bpropNLL(const Vec& target, real nll, Vec& bias_gradient)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );
    bias_gradient.resize( size );
    bias_gradient.clear();

    if( use_parametric_mean_field )
    {
        PLERROR("RBMLateralBinomialLayer::bpropNLL: use_parametric_mean_field=true "
                "not implemented yet.");
    }
    else
    {
        // bias_gradient = expectation - target
        substract(expectation, target, temp_mean_field_gradient);

        current_temp_output = expectation;
        lateral_weights_gradient.clear();

        real output_i;
        for( int t=n_lateral_connections_passes-1 ; t>=0 ; t-- )
        {
            for( int i=0 ; i<size ; i++ )
            {
                output_i = current_temp_output[i];

                // Contribution from the mean field approximation
                temp_mean_field_gradient2[i] =  (1-dampening_factor)*
                    output_i * (1-output_i) * temp_mean_field_gradient[i];
            
                // Contribution from the dampening
                temp_mean_field_gradient[i] *= dampening_factor;
            }

            // Input gradient contribution
            bias_gradient += temp_mean_field_gradient2;

            // Lateral weights gradient contribution
            if( topographic_lateral_weights.length() == 0)
            {
                externalSymetricProductAcc( lateral_weights_gradient, 
                                            temp_mean_field_gradient2,
                                            temp_output[t] );
            
                transposeProductAcc(temp_mean_field_gradient, lateral_weights, 
                                    temp_mean_field_gradient2);
            }
            else
            {
                productTopoLateralWeightsGradients( 
                    temp_output[t],
                    temp_mean_field_gradient,
                    temp_mean_field_gradient2,
                    topographic_lateral_weights_gradient);
            }

            current_temp_output = temp_output[t];
        }
    
        for( int i=0 ; i<size ; i++ )
        {
            output_i = current_temp_output[i];
            temp_mean_field_gradient[i] *= output_i * (1-output_i);
        }

        bias_gradient += temp_mean_field_gradient;

        if( topographic_lateral_weights.length() == 0)
        {
            // Update lateral connections
            if( momentum == 0. )
            {
                multiplyScaledAdd( lateral_weights_gradient, 1.0, -learning_rate,
                                   lateral_weights);
            }
            else
            {
                multiplyScaledAdd( lateral_weights_gradient, momentum, -learning_rate,
                                   lateral_weights_inc);
                lateral_weights += lateral_weights_inc;
            }
        }
        else
        {
            if( !do_not_learn_topographic_lateral_weights )
            {
                if( momentum == 0. )
                    for( int i=0; i<topographic_lateral_weights.length(); i++ )
                        multiplyScaledAdd( topographic_lateral_weights_gradient[i], 1.0, 
                                           -learning_rate,
                                           topographic_lateral_weights[i]);
            
                else
                    PLERROR("In RBMLateralBinomialLayer:bpropNLL - Not implemented for "
                            "topographic weights");
            }
        }
        // Set diagonal to 0
        if( lateral_weights.length() != 0 )
        {
            real *d = lateral_weights.data();
            for (int i=0; i<lateral_weights.length(); i++,d+=lateral_weights.mod()+1) 
                *d = 0;
        }
    }
}

void RBMLateralBinomialLayer::bpropNLL(const Mat& targets, const Mat& costs_column,
                                Mat& bias_gradients)
{
    computeExpectations();

    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );
    bias_gradients.resize( batch_size, size );
    bias_gradients.clear();


    // TODO Can we do this more efficiently? (using BLAS)

    if( use_parametric_mean_field )
    {
        PLERROR("RBMLateralBinomialLayer::bpropNLL: use_parametric_mean_field=true "
                "not implemented yet.");
    }
    else
    {

        // We use the average gradient over the mini-batch.
        lateral_weights_gradient.clear();
        real output_i;
        for (int j = 0; j < batch_size; j++)
        {
            // top_gradient = expectations(j) - targets(j)
            substract(expectations(j), targets(j), temp_mean_field_gradient);
            current_temp_output = expectations(j);

            for( int t=n_lateral_connections_passes-1 ; t>=0 ; t-- )
            {
                for( int i=0 ; i<size ; i++ )
                {
                    output_i = current_temp_output[i];
                
                    // Contribution from the mean field approximation
                    temp_mean_field_gradient2[i] =  (1-dampening_factor)*
                        output_i * (1-output_i) * temp_mean_field_gradient[i];
                
                    // Contribution from the dampening
                    temp_mean_field_gradient[i] *= dampening_factor;
                }
            
                // Input gradient contribution
                bias_gradients(j) += temp_mean_field_gradient2;
            
                // Lateral weights gradient contribution
                if( topographic_lateral_weights.length() == 0)
                {

                    externalSymetricProductAcc( lateral_weights_gradient, 
                                                temp_mean_field_gradient2,
                                                temp_outputs[t](j) );
                
                    transposeProductAcc(temp_mean_field_gradient, lateral_weights, 
                                        temp_mean_field_gradient2);
                }
                else
                {
                    productTopoLateralWeightsGradients( 
                        temp_outputs[t](j),
                        temp_mean_field_gradient,
                        temp_mean_field_gradient2,
                        topographic_lateral_weights_gradient);
                }
                current_temp_output = temp_outputs[t](j);
            }
    
            for( int i=0 ; i<size ; i++ )
            {
                output_i = current_temp_output[i];
                temp_mean_field_gradient[i] *= output_i * (1-output_i);
            }

            bias_gradients(j) += temp_mean_field_gradient;
        }

        // Update lateral connections
        if( topographic_lateral_weights.length() == 0 )
        {
            if( momentum == 0. )
                multiplyScaledAdd( lateral_weights_gradient, 1.0, -learning_rate,
                                   lateral_weights);
            else
                PLERROR("In RBMLateralBinomialLayer:bpropUpdate - Not implemented for "
                        "momentum with mini-batches");
        }
        else
        {
            if( !do_not_learn_topographic_lateral_weights )
            {
                if( momentum == 0. )
                    for( int i=0; i<topographic_lateral_weights.length(); i++ )
                        multiplyScaledAdd( topographic_lateral_weights_gradient[i], 1.0, 
                                           -learning_rate,
                                           topographic_lateral_weights[i]);
            
                else
                    PLERROR("In RBMLateralBinomialLayer:bpropNLL - Not implemented for "
                            "topographic weights");
            }
        }

        // Set diagonal to 0
        if( lateral_weights.length() != 0 )
        {
            real *d = lateral_weights.data();
            for (int i=0; i<lateral_weights.length(); i++,d+=lateral_weights.mod()+1) 
                *d = 0;
        }
    }
}

void RBMLateralBinomialLayer::accumulatePosStats( const Vec& pos_values )
{
    inherited::accumulatePosStats( pos_values);
    externalProductAcc(lateral_weights_pos_stats, pos_values, pos_values);
}

void RBMLateralBinomialLayer::accumulatePosStats( const Mat& pos_values )
{
    inherited::accumulatePosStats( pos_values);
    transposeProductAcc(lateral_weights_pos_stats, pos_values, pos_values);
}

void RBMLateralBinomialLayer::accumulateNegStats( const Vec& neg_values )
{
    inherited::accumulateNegStats( neg_values);
    externalProductAcc(lateral_weights_neg_stats, neg_values, neg_values);
}

void RBMLateralBinomialLayer::accumulateNegStats( const Mat& neg_values )
{
    inherited::accumulateNegStats( neg_values);
    transposeProductAcc(lateral_weights_neg_stats, neg_values, neg_values);
}


void RBMLateralBinomialLayer::update()
{ 
    //real pos_factor = 0.5 * learning_rate / pos_count;
    //real neg_factor = - 0.5 * learning_rate / neg_count;
    real pos_factor = learning_rate / pos_count;
    real neg_factor = - learning_rate / neg_count;

    if( topographic_lateral_weights.length() != 0 )
        PLERROR("In RBMLateralBinomialLayer:update - Not implemented for "
                "topographic weights");

    // Update lateral connections
    if( momentum == 0. )
    {
        multiplyScaledAdd( lateral_weights_pos_stats, neg_factor, pos_factor,
                           lateral_weights_neg_stats);
        lateral_weights += lateral_weights_neg_stats; 
    }
    else
    {
        multiplyScaledAdd( lateral_weights_pos_stats, neg_factor, pos_factor,
                           lateral_weights_neg_stats);
        multiplyScaledAdd( lateral_weights_neg_stats, momentum, 1.0,
                           lateral_weights_inc);
        lateral_weights += lateral_weights_inc;
    }

    // Set diagonal to 0
    if( lateral_weights.length() != 0 )
    {
        real *d = lateral_weights.data();
        for (int i=0; i<lateral_weights.length(); i++,d+=lateral_weights.mod()+1) 
            *d = 0;
    }

    // Call to update() must be at the end, since update() calls clearStats()!
    inherited::update();
}

void RBMLateralBinomialLayer::update( const Vec& grad)
{
    inherited::update( grad );
    PLWARNING("RBMLateralBinomialLayer::update( grad ): does not update the\n"
        "lateral connections.");
}

void RBMLateralBinomialLayer::update( const Vec& pos_values, const Vec& neg_values )
{
    // Update lateral connections
    if( topographic_lateral_weights.length() == 0 )
    {
        if( momentum == 0. )
        {
            externalProductScaleAcc(lateral_weights, pos_values, pos_values,
                                    //0.5 * learning_rate);
                                    learning_rate);
            externalProductScaleAcc(lateral_weights, neg_values, neg_values,
                                    //- 0.5 * learning_rate);
                                    -learning_rate);
        }
        else
        {
            lateral_weights_inc *= momentum;
            externalProductScaleAcc(lateral_weights_inc, pos_values, pos_values,
                                    //0.5 * learning_rate);
                                    learning_rate);
            externalProductScaleAcc(lateral_weights_inc, neg_values, neg_values,
                                    //- 0.5 * learning_rate);
                                    - learning_rate);
            lateral_weights += lateral_weights_inc;
        }    

        // Set diagonal to 0
        if( lateral_weights.length() != 0 )
        {
            real *d = lateral_weights.data();
            for (int i=0; i<lateral_weights.length(); i++,d+=lateral_weights.mod()+1) 
                *d = 0;
        }
    }
    else
    {
        if( momentum == 0. )
            updateTopoLateralWeightsCD(pos_values, neg_values);
        else
            PLERROR("In RBMLateralBinomialLayer:bpropNLL - Not implemented for "
                    "topographic weights");
    }

    inherited::update( pos_values, neg_values );
}

void RBMLateralBinomialLayer::update( const Mat& pos_values, const Mat& neg_values )
{
    int n = pos_values.length();
    PLASSERT( neg_values.length() == n );

    // We take the average gradient over the mini-batch.
    //real avg_lr = 0.5 * learning_rate / n;
    real avg_lr = learning_rate / n;

    // Update lateral connections
    if( topographic_lateral_weights.length() == 0 )
    {
        if( momentum == 0. )
        {
            transposeProductScaleAcc(lateral_weights, pos_values, pos_values,
                                     avg_lr, 1);
            transposeProductScaleAcc(lateral_weights, neg_values, neg_values,
                                     -avg_lr, 1);
        }
        else
        {
            lateral_weights_inc *= momentum;
            transposeProductScaleAcc(lateral_weights_inc, pos_values, pos_values,
                                     avg_lr, 1);
            transposeProductScaleAcc(lateral_weights_inc, neg_values, neg_values,
                                     -avg_lr, 1);
            lateral_weights += lateral_weights_inc;
        }

        // Set diagonal to 0
        if( lateral_weights.length() != 0 )
        {
            real *d = lateral_weights.data();
            for (int i=0; i<lateral_weights.length(); i++,d+=lateral_weights.mod()+1) 
                *d = 0;
        }
    }
    else
    {
        if( momentum == 0. )
        {
            for(int b=0; b<pos_values.length(); b++)
                updateTopoLateralWeightsCD(pos_values(b), neg_values(b));
            
        }
        else
            PLERROR("In RBMLateralBinomialLayer:bpropNLL - Not implemented for "
                    "topographic weights");
    }

    inherited::update( pos_values, neg_values );
}

void RBMLateralBinomialLayer::updateCDandGibbs( const Mat& pos_values,
                                 const Mat& cd_neg_values,
                                 const Mat& gibbs_neg_values,
                                 real background_gibbs_update_ratio )
{
    inherited::updateCDandGibbs( pos_values, cd_neg_values,
                                 gibbs_neg_values, background_gibbs_update_ratio );
    PLERROR("In RBMLateralBinomialLayer::updateCDandGibbs(): not implemented yet.");
}

void RBMLateralBinomialLayer::updateGibbs( const Mat& pos_values,
                                           const Mat& gibbs_neg_values)
{
    inherited::updateGibbs( pos_values, gibbs_neg_values );
    PLERROR("In RBMLateralBinomialLayer::updateCDandGibbs(): not implemented yet.");
}

void RBMLateralBinomialLayer::declareOptions(OptionList& ol)
{
    declareOption(ol, "n_lateral_connections_passes", 
                  &RBMLateralBinomialLayer::n_lateral_connections_passes,
                  OptionBase::buildoption,
                  "Number of passes through the lateral connections.\n");

    declareOption(ol, "dampening_factor", 
                  &RBMLateralBinomialLayer::dampening_factor,
                  OptionBase::buildoption,
                  "Dampening factor ( expectation_t = (1-df) * currrent mean field"
                  " + df * expectation_{t-1}).\n");

    declareOption(ol, "mean_field_precision_threshold", 
                  &RBMLateralBinomialLayer::mean_field_precision_threshold,
                  OptionBase::buildoption,
                  "Mean-field precision threshold that, once reached, stops the mean-field\n"
                  "expectation approximation computation. Used only in computeExpectation().\n"
                  "Precision is computed as:\n"
                  "  dist(last_mean_field, current_mean_field) / size\n");

    declareOption(ol, "topographic_length", 
                  &RBMLateralBinomialLayer::topographic_length,
                  OptionBase::buildoption,
                  "Length of the topographic map.\n");

    declareOption(ol, "topographic_width", 
                  &RBMLateralBinomialLayer::topographic_width,
                  OptionBase::buildoption,
                  "Width of the topographic map.\n");

    declareOption(ol, "topographic_patch_vradius", 
                  &RBMLateralBinomialLayer::topographic_patch_vradius,
                  OptionBase::buildoption,
                  "Vertical radius of the topographic local weight patches.\n");

    declareOption(ol, "topographic_patch_hradius", 
                  &RBMLateralBinomialLayer::topographic_patch_hradius,
                  OptionBase::buildoption,
                  "Horizontal radius of the topographic local weight patches.\n");

    declareOption(ol, "topographic_lateral_weights_init_value", 
                  &RBMLateralBinomialLayer::topographic_lateral_weights_init_value,
                  OptionBase::buildoption,
                  "Initial value for the topographic_lateral_weights.\n");

    declareOption(ol, "do_not_learn_topographic_lateral_weights", 
                  &RBMLateralBinomialLayer::do_not_learn_topographic_lateral_weights,
                  OptionBase::buildoption,
                  "Indication that the topographic_lateral_weights should\n"
                  "be fixed at their initial value.\n");

    declareOption(ol, "lateral_weights", 
                  &RBMLateralBinomialLayer::lateral_weights,
                  OptionBase::learntoption,
                  "Lateral connections.\n");

    declareOption(ol, "topographic_lateral_weights", 
                  &RBMLateralBinomialLayer::topographic_lateral_weights,
                  OptionBase::learntoption,
                  "Local topographic lateral connections.\n");

    declareOption(ol, "use_parametric_mean_field", 
                  &RBMLateralBinomialLayer::use_parametric_mean_field,
                  OptionBase::buildoption,
                  "Indication that a parametric predictor of the mean-field\n"
                  "approximation of the hidden layer conditional distribution.\n");

    declareOption(ol, "mean_field_output_weights", 
                  &RBMLateralBinomialLayer::mean_field_output_weights,
                  OptionBase::learntoption,
                  "Output weights of the mean field predictor.\n");

    declareOption(ol, "mean_field_output_bias", 
                  &RBMLateralBinomialLayer::mean_field_output_bias,
                  OptionBase::learntoption,
                  "Output bias of the mean field predictor.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMLateralBinomialLayer::build_()
{
    if( n_lateral_connections_passes == 0 &&
        !fast_exact_is_equal(dampening_factor, 0) )
        PLERROR("In RBMLateralBinomialLayer::build_(): when not using the lateral\n"
                "connections, dampening_factor should be 0.");

    if( dampening_factor < 0 || dampening_factor > 1)
        PLERROR("In RBMLateralBinomialLayer::build_(): dampening_factor should be\n"
                "in [0,1].");

    if( n_lateral_connections_passes < 0 )
        PLERROR("In RBMLateralBinomialLayer::build_(): n_lateral_connections_passes\n"
                " should be >= 0.");
 
    if( use_parametric_mean_field && topographic_length > 0 && topographic_width > 0 )
        PLERROR("RBMLateralBinomialLayer::build_(): can't use parametric mean field "
            "and topographic lateral connections.");
    
    if( use_parametric_mean_field )
    {
        mean_field_output_weights.resize(size,size);
        mean_field_output_bias.resize(size);
        mean_field_input.resize(size);
        pre_sigmoid_mean_field_output.resize(size);
    }

    if( topographic_length <= 0 || topographic_width <= 0)
    {
        lateral_weights.resize(size,size);

        lateral_weights_gradient.resize(size,size);
        lateral_weights_pos_stats.resize(size,size);
        lateral_weights_neg_stats.resize(size,size);
        if( momentum != 0. )
        {
            bias_inc.resize( size );
            lateral_weights_inc.resize(size,size);
        }   
    }
    else
    {
        if( size != topographic_length * topographic_width )
            PLERROR( "In RBMLateralBinomialLayer::build_(): size != "
                     "topographic_length * topographic_width.\n" );

        if( topographic_length-1 <= 2*topographic_patch_vradius )
            PLERROR( "In RBMLateralBinomialLayer::build_(): "
                     "topographic_patch_vradius is too large.\n" );

        if( topographic_width-1 <= 2*topographic_patch_hradius )
            PLERROR( "In RBMLateralBinomialLayer::build_(): "
                     "topographic_patch_hradius is too large.\n" );

        topographic_lateral_weights.resize(size);
        topographic_lateral_weights_gradient.resize(size);
        for( int i=0; i<size; i++ )
        {
            topographic_lateral_weights[i].resize( 
                ( 2 * topographic_patch_hradius + 1 ) *
                ( 2 * topographic_patch_vradius + 1 ) - 1 );
            topographic_lateral_weights_gradient[i].resize( 
                ( 2 * topographic_patch_hradius + 1 ) *
                ( 2 * topographic_patch_vradius + 1 ) - 1 );
        }

        // Should probably have separate lateral_weights_*_stats
    }

    // Resizing temporary variables
    dampening_expectation.resize(size);
    temp_input_gradient.resize(size);
    temp_mean_field_gradient.resize(size);
    temp_mean_field_gradient2.resize(size);
}

void RBMLateralBinomialLayer::build()
{
    inherited::build();
    build_();
}


void RBMLateralBinomialLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(lateral_weights,copies);
    deepCopyField(topographic_lateral_weights,copies);
    deepCopyField(lateral_weights_pos_stats,copies);
    deepCopyField(lateral_weights_neg_stats,copies);
    deepCopyField(dampening_expectation,copies);
    deepCopyField(dampening_expectations,copies);
    deepCopyField(mean_field_input,copies);
    deepCopyField(pre_sigmoid_mean_field_output,copies);
    deepCopyField(temp_output,copies);
    deepCopyField(temp_outputs,copies);
    deepCopyField(current_temp_output,copies);
    deepCopyField(previous_temp_output,copies);
    deepCopyField(current_temp_outputs,copies);
    deepCopyField(previous_temp_outputs,copies);
    deepCopyField(bias_plus_input,copies);
    deepCopyField(bias_plus_inputs,copies);
    deepCopyField(temp_input_gradient,copies);
    deepCopyField(temp_mean_field_gradient,copies);
    deepCopyField(temp_mean_field_gradient2,copies);
    deepCopyField(lateral_weights_gradient,copies);
    deepCopyField(lateral_weights_inc,copies);
    deepCopyField(topographic_lateral_weights_gradient,copies);
    deepCopyField(mean_field_output_weights,copies);
    deepCopyField(mean_field_output_bias,copies);
}

real RBMLateralBinomialLayer::energy(const Vec& unit_values) const
{
    if( topographic_lateral_weights.length() == 0 )
        product(dampening_expectation, lateral_weights, unit_values);
    else
        productTopoLateralWeights( dampening_expectation, unit_values );
    return -dot(unit_values, bias) - 0.5 * dot(unit_values, dampening_expectation);
}

real RBMLateralBinomialLayer::freeEnergyContribution(const Vec& unit_activations)
    const
{
    PLERROR(
        "In RBMLateralBinomialLayer::freeEnergyContribution(): not implemented.");
    return -1;
}

int RBMLateralBinomialLayer::getConfigurationCount()
{
    return size < 31 ? 1<<size : INFINITE_CONFIGURATIONS;
}

void RBMLateralBinomialLayer::getConfiguration(int conf_index, Vec& output)
{
    PLASSERT( output.length() == size );
    PLASSERT( conf_index >= 0 && conf_index < getConfigurationCount() );

    for ( int i = 0; i < size; ++i ) {
        output[i] = conf_index & 1;
        conf_index >>= 1;
    }
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
