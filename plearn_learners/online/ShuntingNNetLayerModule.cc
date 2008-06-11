// -*- C++ -*-

// ShuntingNNetLayerModule.cc
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

/* *******************************************************
   * $Id: ShuntingNNetLayerModule.cc,v 1.3 2006/01/18 04:04:06 lamblinp Exp $
   ******************************************************* */

// Authors: Jerome Louradour

/*! \file ShuntingNNetLayerModule.cc */


#include "ShuntingNNetLayerModule.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ShuntingNNetLayerModule,
    "Affine transformation module, with stochastic gradient descent updates",
    "Neural Network layer, using stochastic gradient to update neuron weights\n"
    "       Output = weights * Input + bias\n"
    "Weights and bias are updated by online gradient descent, with learning\n"
    "rate possibly decreasing in 1/(1 + n_updates_done * decrease_constant).\n"
    "An L1 and L2 regularization penalty can be added to push weights to 0.\n"
    "Weights can be initialized to 0, to a given initial matrix, or randomly\n"
    "from a uniform distribution.\n"
    );

/////////////////////////
// ShuntingNNetLayerModule //
/////////////////////////
ShuntingNNetLayerModule::ShuntingNNetLayerModule():
    start_learning_rate( .001 ),
    decrease_constant( 0. ),
    init_weights_random_scale( 1. ),
    init_quad_weights_random_scale( 1. ),
    n_filters( 1 ),
    n_filters_inhib( -1 ),
    step_number( 0 )
{}

////////////////////
// declareOptions //
////////////////////

void ShuntingNNetLayerModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "start_learning_rate",
                  &ShuntingNNetLayerModule::start_learning_rate,
                  OptionBase::buildoption,
                  "Learning-rate of stochastic gradient optimization");

    declareOption(ol, "decrease_constant",
                  &ShuntingNNetLayerModule::decrease_constant,
                  OptionBase::buildoption,
                  "Decrease constant of stochastic gradient optimization");

    declareOption(ol, "init_weights_random_scale",
                  &ShuntingNNetLayerModule::init_weights_random_scale,
                  OptionBase::buildoption,
                  "Weights of the excitation (softplus part) are initialized randomly\n"
                  "from a uniform in [-r,r], with r = init_weights_random_scale/input_size.\n"
                  "To clear the weights initially, just set this option to 0.");
                  
    declareOption(ol, "init_quad_weights_random_scale",
                  &ShuntingNNetLayerModule::init_quad_weights_random_scale,
                  OptionBase::buildoption,
                  "Weights of the quadratic part (of excitation, as well as inhibition) are initialized randomly\n"
                  "from a uniform in [-r,r], with r = init_weights_random_scale/input_size.\n"
                  "To clear the weights initially, just set this option to 0.");
                  
    declareOption(ol, "n_filters",
                  &ShuntingNNetLayerModule::n_filters,
                  OptionBase::buildoption,
                  "Number of synapses per neuron for excitation.\n");

    declareOption(ol, "n_filters_inhib",
                  &ShuntingNNetLayerModule::n_filters_inhib,
                  OptionBase::buildoption,
                  "Number of synapses per neuron for inhibition.\n"
                  "Must be lower or equal to n_filters in the current implementation (!).\n"
                  "If -1, then it is taken equal to n_filters.");

    declareOption(ol, "excit_quad_weights", &ShuntingNNetLayerModule::excit_quad_weights,
                  OptionBase::learntoption,
                  "List of weights vectors of the neurons"
                  "contributing to the excitation -- quadratic part)");

    declareOption(ol, "inhib_quad_weights", &ShuntingNNetLayerModule::inhib_quad_weights,
                  OptionBase::learntoption,
                  "List of weights vectors of the neurons (inhibation -- quadratic part)\n");

    declareOption(ol, "excit_weights", &ShuntingNNetLayerModule::excit_weights,
                  OptionBase::learntoption,
                  "Input weights vectors of the neurons (excitation -- softplus part)\n");

    declareOption(ol, "bias", &ShuntingNNetLayerModule::bias,
                  OptionBase::learntoption,
                  "Bias of the neurons (in the softplus of the excitations)\n");

    declareOption(ol, "excit_num_coeff", &ShuntingNNetLayerModule::excit_num_coeff,
                  OptionBase::learntoption,
                  "Multiplicative Coefficient applied on the excitation\n"
                  "in the numerator of the activation closed form.\n");

    declareOption(ol, "inhib_num_coeff", &ShuntingNNetLayerModule::inhib_num_coeff,
                  OptionBase::learntoption,
                  "Multiplicative Coefficient applied on the inhibition\n"
                  "in the numerator of the activation closed form.\n");

    inherited::declareOptions(ol);
}
///////////
// build //
///////////

void ShuntingNNetLayerModule::build_()
{
    if( input_size < 0 ) // has not been initialized
        return;

    if( output_size < 0 )
        PLERROR("ShuntingNNetLayerModule::build_: 'output_size' is < 0 (%i),\n"
                " you should set it to a positive integer (the number of"
                " neurons).\n", output_size);

    if (n_filters_inhib < 0)
        n_filters_inhib= n_filters;
    PLASSERT( n_filters>0 );
    
    if(    excit_quad_weights.length() != n_filters
        || inhib_quad_weights.length() != n_filters_inhib
        || excit_weights.length() != output_size
        || excit_weights.width() != input_size
        || bias.size() != output_size )
    {
        forget();
    }
}
void ShuntingNNetLayerModule::build()
{
    inherited::build();
    build_();
}

////////////
// forget //
////////////

void ShuntingNNetLayerModule::forget()
{
    learning_rate = start_learning_rate;
    step_number = 0;

    bias.resize( output_size );
    bias.clear();
    
    excit_num_coeff.resize( output_size );
    inhib_num_coeff.resize( output_size );
    excit_num_coeff.fill(1.);
    inhib_num_coeff.fill(1.);

    excit_weights.resize( output_size, input_size );
    excit_quad_weights.resize( n_filters );
    PLASSERT( n_filters_inhib >= 0 && n_filters_inhib <= n_filters );
    inhib_quad_weights.resize( n_filters_inhib );
    
    if( !random_gen )
    {
        PLWARNING( "ShuntingNNetLayerModule: cannot forget() without random_gen" );
        return;
    }
    
    real r = init_weights_random_scale / (real)input_size;
    if( r > 0. )
        random_gen->fill_random_uniform(excit_weights, -r, r);
    else
        excit_weights.clear();
      
    r = init_quad_weights_random_scale / (real)input_size;    
    if( r > 0. )
        for( int k = 0; k < n_filters; k++ )
        {
            excit_quad_weights[k].resize( output_size, input_size );
            random_gen->fill_random_uniform(excit_quad_weights[k], -r, r);
            if ( k < n_filters_inhib ) {
                inhib_quad_weights[k].resize( output_size, input_size );
                random_gen->fill_random_uniform(inhib_quad_weights[k], -r, r);
            }
        }
    else
        for( int k = 0; k < n_filters; k++ )
        {
            excit_quad_weights[k].resize(output_size, input_size );
            excit_quad_weights[k].clear();
            if ( k < n_filters_inhib ) {
                inhib_quad_weights[k].resize(output_size, input_size );
                inhib_quad_weights[k].clear();
            }
        }
}

///////////
// fprop //
///////////

void ShuntingNNetLayerModule::fprop(const Vec& input, Vec& output) const
{
    PLASSERT_MSG( input.size() == input_size,
                  "input.size() should be equal to this->input_size" );

    output.resize( output_size );

    if( during_training )
    {
        batch_excitations.resize(1, output_size);
        batch_inhibitions.resize(1, output_size);
    }
//    if( use_fast_approximations )

        for( int i = 0; i < output_size; i++ )
        {
            real excitation = 0.;
            real inhibition = 0.;
            for ( int k=0; k < n_filters; k++ )
            {
                excitation += square( dot( excit_quad_weights[k](i), input ) );
                if ( k < n_filters_inhib )
                    inhibition += square( dot( inhib_quad_weights[k](i), input ) );
            }
            excitation = sqrt( excitation + tabulated_softplus( dot( excit_weights(i), input ) + bias[i] ) );
            inhibition = sqrt( inhibition );
            if( during_training )
            {
                    batch_excitations(0,i) = excitation;
                    batch_inhibitions(0,i) = inhibition;
            }

            output[i] = ( excit_num_coeff[i]* excitation - inhib_num_coeff[i]* inhibition ) /
                        (1. + excitation + inhibition );
        }
//    else
}

void ShuntingNNetLayerModule::fprop(const Mat& inputs, Mat& outputs)
{
    PLASSERT( inputs.width() == input_size );
    int n = inputs.length();
    outputs.resize(n, output_size);
    

    Mat excitations_part2(n, output_size);
    excitations_part2.clear();
    productTranspose(excitations_part2, inputs, excit_weights);
    resizeOnes(n);
    externalProductAcc(excitations_part2, ones, bias);

    Mat excitations(n, output_size), inhibitions(n, output_size);
    excitations.clear();
    inhibitions.clear();

        for ( int k=0; k < n_filters; k++ )
        {
            Mat tmp_sample_output(n, output_size);

            tmp_sample_output.clear();
            productTranspose(tmp_sample_output, inputs, excit_quad_weights[k]);
            squareElements(tmp_sample_output);
            multiplyAcc(excitations, tmp_sample_output, 1.);

            if ( k < n_filters_inhib ) {
                tmp_sample_output.clear();
                productTranspose(tmp_sample_output, inputs, inhib_quad_weights[k]);
                squareElements(tmp_sample_output);
                multiplyAcc(inhibitions, tmp_sample_output, 1.);
            }
        }
        for( int i_sample = 0; i_sample < n; i_sample ++)
        {
            for( int i = 0; i < output_size; i++ )
            {
                excitations(i_sample,i) = sqrt( excitations(i_sample,i) + tabulated_softplus( excitations_part2(i_sample,i) ) );
                inhibitions(i_sample,i) = sqrt( inhibitions(i_sample,i) );

                real E = excitations(i_sample,i);
                real S = inhibitions(i_sample,i);
                    
                outputs(i_sample,i) = ( excit_num_coeff[i]* E - inhib_num_coeff[i]* S ) /
                                       (1. + E + S );
            }
        }

    if( during_training )
    {
        batch_excitations.resize(n, output_size);
        batch_inhibitions.resize(n, output_size);
        batch_excitations << excitations;
        batch_inhibitions << inhibitions;
    }
}

/////////////////
// bpropUpdate //
/////////////////

void ShuntingNNetLayerModule::bpropUpdate(const Vec& input, const Vec& output,
                                      const Vec& output_gradient)
{
    learning_rate = start_learning_rate / (1+decrease_constant*step_number);

    for( int i=0; i<output_size; i++ )
    {
        real tmp = square(1 + batch_excitations(0,i) + batch_inhibitions(0,i) );
        
        real Dactivation_Dexcit =   ( excit_num_coeff[i]  +  batch_inhibitions(0,i)*(excit_num_coeff[i] + inhib_num_coeff[i]) ) / tmp;
        real Dactivation_Dinhib = - ( inhib_num_coeff[i]  +  batch_excitations(0,i)*(excit_num_coeff[i] + inhib_num_coeff[i]) ) / tmp;

        real lr_og_excit = learning_rate * output_gradient[i];
        PLASSERT( batch_excitations(0,i)>0. );
        PLASSERT( batch_inhibitions(0,i)>0. );
        real lr_og_inhib = lr_og_excit * Dactivation_Dinhib / batch_inhibitions(0,i);
        lr_og_excit *= Dactivation_Dexcit / batch_excitations(0,i);
        
        tmp = lr_og_excit * sigmoid( dot( excit_weights(i), input ) + bias[i] ) * .5;

        bias[i] -= tmp;
        multiplyAcc( excit_weights(i), input, -tmp);

        for( int k = 0; k < n_filters; k++ )
        {
            real tmp_excit2 = lr_og_excit * dot( excit_quad_weights[k](i), input );
            real tmp_inhib2 = 0;
            if (k < n_filters_inhib)
                tmp_inhib2 = lr_og_inhib * dot( inhib_quad_weights[k](i), input );
            for( int j=0; j<input_size; j++ )
            {
                excit_quad_weights[k](i,j) -= tmp_excit2 * input[j];
                if (k < n_filters_inhib)
                    inhib_quad_weights[k](i,j) -= tmp_inhib2 * input[j];
            }   
        }
    }

    step_number++;
}

void ShuntingNNetLayerModule::bpropUpdate(const Mat& inputs, const Mat& outputs,
        Mat& input_gradients,
        const Mat& output_gradients,
        bool accumulate)
{
    PLASSERT( inputs.width() == input_size );
    PLASSERT( outputs.width() == output_size );
    PLASSERT( output_gradients.width() == output_size );

    //fprop(inputs);

    int n = inputs.length();

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == input_size &&
                input_gradients.length() == n,
                "Cannot resize input_gradients and accumulate into it" );
    }
    else
    {
        input_gradients.resize(n, input_size);
        input_gradients.fill(0);
    }

    learning_rate = start_learning_rate / (1+decrease_constant*step_number);
    real avg_lr = learning_rate / n; // To obtain an average on a mini-batch.

    if ( avg_lr == 0. )
        return ; 

        Mat tmp(n, output_size);
        // tmp = (1 + E + S ).^2;
        tmp.fill(1.);
        multiplyAcc(tmp, batch_excitations, 1);
        multiplyAcc(tmp, batch_inhibitions, 1);
        squareElements(tmp);
        
        Vec bias_updates(output_size);
        Mat excit_weights_updates( output_size, input_size);
        TVec<Mat> excit_quad_weights_updates(n_filters);
        TVec<Mat> inhib_quad_weights_updates(n_filters_inhib);
        // Initialisation 
        bias_updates.clear();
        excit_weights_updates.clear();
        for( int k=0; k < n_filters; k++ )
        {
            excit_quad_weights_updates[k].resize( output_size, input_size);
            excit_quad_weights_updates[k].clear();
            if (k < n_filters_inhib) {
                inhib_quad_weights_updates[k].resize( output_size, input_size);
                inhib_quad_weights_updates[k].clear();
            }
        }

        for( int i_sample = 0; i_sample < n; i_sample++ )
        for( int i=0; i<output_size; i++ )
        {
            real Dactivation_Dexcit =   ( excit_num_coeff[i]  +  batch_inhibitions(i_sample,i)*(excit_num_coeff[i] + inhib_num_coeff[i]) ) / tmp(i_sample,i);
            real Dactivation_Dinhib = - ( inhib_num_coeff[i]  +  batch_excitations(i_sample,i)*(excit_num_coeff[i] + inhib_num_coeff[i]) ) / tmp(i_sample,i);
            
            real lr_og_excit = avg_lr * output_gradients(i_sample,i);
            PLASSERT( batch_excitations(i_sample,i)>0. );
            PLASSERT( n_filters_inhib==0 || batch_inhibitions(i_sample,i)>0. );
            real lr_og_inhib = lr_og_excit * Dactivation_Dinhib / batch_inhibitions(i_sample,i);
            lr_og_excit *= Dactivation_Dexcit / batch_excitations(i_sample,i);
                
            real tmp2 = lr_og_excit * sigmoid( dot( excit_weights(i), inputs(i_sample) ) + bias[i] ) * .5;

            bias_updates[i] -= tmp2;
            multiplyAcc( excit_weights_updates(i), inputs(i_sample), -tmp2);

            for( int k = 0; k < n_filters; k++ )
            {
                real tmp_excit2 = lr_og_excit   * dot( excit_quad_weights[k](i), inputs(i_sample) );
                real tmp_inhib2 = 0;
                if (k < n_filters_inhib)
                    tmp_inhib2 = lr_og_inhib   * dot( inhib_quad_weights[k](i), inputs(i_sample) );
                //for( int j=0; j<input_size; j++ )
                //{
                //    excit_quad_weights_updates[k](i,j) -= tmp_excit2 * inputs(i_sample,j);
                //    if (k < n_filters_inhib)
                //        inhib_quad_weights_updates[k](i,j) -= tmp_inhib2 * inputs(i_sample,j);
                //}
                multiplyAcc( excit_quad_weights_updates[k](i), inputs(i_sample), -tmp_excit2);
                if (k < n_filters_inhib)
                    multiplyAcc( inhib_quad_weights_updates[k](i), inputs(i_sample), -tmp_inhib2);
            }
        }

        multiplyAcc( bias, bias_updates, 1.);
        multiplyAcc( excit_weights, excit_weights_updates, 1.);
        for( int k = 0; k < n_filters; k++ )
        {
            multiplyAcc( excit_quad_weights[k], excit_quad_weights_updates[k], 1.);
            if (k < n_filters_inhib)
                multiplyAcc( inhib_quad_weights[k], inhib_quad_weights_updates[k], 1.);
        }
        batch_excitations.clear();
        batch_inhibitions.clear();

    step_number += n;
}




void ShuntingNNetLayerModule::setLearningRate( real dynamic_learning_rate )
{
    start_learning_rate = dynamic_learning_rate;
    step_number = 0;
    // learning_rate will automatically be set in bpropUpdate()
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////

void ShuntingNNetLayerModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(excit_weights,            copies);
    deepCopyField(excit_quad_weights,       copies);
    deepCopyField(inhib_quad_weights,       copies);
    deepCopyField(bias,                     copies);
    deepCopyField(excit_num_coeff,          copies);
    deepCopyField(inhib_num_coeff,          copies);
    deepCopyField(ones,                     copies);
}




////////////////
// resizeOnes //
////////////////
void ShuntingNNetLayerModule::resizeOnes(int n) const
{
    if (ones.length() < n) {
        ones.resize(n);
        ones.fill(1);
    } else if (ones.length() > n)
        ones.resize(n);
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
