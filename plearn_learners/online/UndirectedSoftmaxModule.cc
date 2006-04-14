// -*- C++ -*-

// UndirectedSoftmaxModule.cc
//
// Copyright (C) 2005 Pascal Lamblin 
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
   * $Id$
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file UndirectedSoftmaxModule.cc */


#include "UndirectedSoftmaxModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    UndirectedSoftmaxModule,
    "Softmax output layer in an undirected multi-layer graphical model, using stochastic gradient to update neuron weights",
    "There is one output unit per class. The model estimates P(Y|X) where Y is the output class and X is the input\n"
    "of the module. The input X can be interpreted as the linear output of binary stochastic neurons H at a previous layer,\n"
    "i.e. these input neurons fire with probability sigmoid(X + weights'*T), where T_i = 1_{Y=i}.\n"
    "Output units fire with probability proportional to exp(biases + weights*H),where H is the vector of binary values of the\n"
    "hidden (or input) neurons whose activations are in X.\n"
    "The output probabilities are computed as follows:\n"
    "      P(Y=i|X) = exp(-biases[i] + sum(softplus(-(X + weights[i])))) / Z\n"
    "where Z normalizes over classes and softplus(a)=log(1+exp(a)).\n"
    "This formula can be derived by considering that X,H, and T are binary random variables\n"
    "following the Boltzmann distribution with energy\n"
    "  energy(H,T,X) = biases'T + T' weights H + H' X.\n"
    "During training, both X and T are observed, so that E is linear in H, i.e. P(H|X,T) is\n"
    "a product of P(H_i|X,T), i.e. the H_i are conditionally independent given X and T.\n"
    "This corresponds to an undirected graphical model with full connectivity between each H_i\n"
    "and each T_j (and similarly between H_i and the inputs of the previous layer, if there is one),\n"
    "but no connection among the H_i or among the T_j's. Because of this factorization we obtain that\n"
    "   P(Y|X) = sum_H exp(-energy(H,T,X)) / Z\n"
    "and\n"
    "   sum_H exp(-energy(H,T,X)) = exp(-biases'T) prod_i (exp(-energy_i(1,T,X)) + exp(-energy_i(0,T,X)))\n"
    "where energy_i(h,T,X) = the term in H_i=h in the energy = h(T' weights[:,i] + X_i).\n"
    "Since energy_i(0,T,X) = 0, we obtain that\n"
    "   sum_H exp(-energy(H,T,X)) = exp(-biases'T) exp(sum_i log(1+exp(-energy_i(1,T,X))))\n"
    "                             = exp(-biases'T + sum(softplus(-T'weights + X')))\n"
    "which gives the above formula for P(Y|X).\n"
    "\n"
    "Weights and biases are updated by online gradient with learning rate possibly decreasing\n"
    "in 1/(1 + n_updates_done_up_to_now * decrease_constant).\n"
    "An L1 and/or L2 regularization penalty can be added to push weights to 0.\n"
    "Weights can be initialized to 0, to a given initial matrix, or randomly\n"
    "from a uniform distribution. Biases can be initialized to 0 or from a user-provided vector.\n"
    );

UndirectedSoftmaxModule::UndirectedSoftmaxModule():
    start_learning_rate( .001 ),
    decrease_constant( 0 ),
    init_weights_random_scale( 1. ),
    L1_penalty_factor( 0. ),
    L2_penalty_factor( 0. ),
    step_number( 0 )
    /* ### Initialize all fields to their default value */
{
}

// Applies linear transformation
void UndirectedSoftmaxModule::fprop(const Vec& input, Vec& output) const
{
    int in_size = input.size();

    // size check
    if( in_size != input_size )
    {
        PLERROR("UndirectedSoftmaxModule::fprop: 'input.size()' should be equal\n"
                " to 'input_size' (%i != %i)\n", in_size, input_size);
    }

    

}

void UndirectedSoftmaxModule::bpropUpdate(const Vec& input, const Vec& output,
                                      const Vec& output_gradient)
{
    int in_size = input.size();
    int out_size = output.size();
    int og_size = output_gradient.size();

    // size check
    if( in_size != input_size )
    {
        PLERROR("UndirectedSoftmaxModule::bpropUpdate: 'input.size()' should be"
                " equal\n"
                " to 'input_size' (%i != %i)\n", in_size, input_size);
    }
    if( out_size != output_size )
    {
        PLERROR("UndirectedSoftmaxModule::bpropUpdate: 'output.size()' should be"
                " equal\n"
                " to 'output_size' (%i != %i)\n", out_size, output_size);
    }
    if( og_size != output_size )
    {
        PLERROR("UndirectedSoftmaxModule::bpropUpdate: 'output_gradient.size()'"
                " should\n"
                " be equal to 'output_size' (%i != %i)\n",
                og_size, output_size);
    }

    learning_rate = start_learning_rate / ( 1+decrease_constant*step_number);

    if (L2_penalty_factor==0)
    {
    }
    else
    {
    }

    if (L1_penalty_factor!=0)
    {
        real delta = learning_rate * L1_penalty_factor;
        for (int i=0;i<output_size;i++)
        {
            real* Wi = weights[i]; // don't apply penalty on bias
            for (int j=0;j<input_size;j++)
            {
                real Wij =  Wi[j];
                if (Wij>delta)
                    Wi[j] -=delta;
                else if (Wij<-delta)
                    Wi[j] +=delta;
                else 
                    Wi[j]=0;
            }
        }
    }
    if (L2_penalty_factor!=0)
    {
        real delta = learning_rate*L2_penalty_factor;
        if (delta>1)
            PLWARNING("UndirectedSoftmaxModule::bpropUpdate: learning rate = %f is too large!",learning_rate);
        weights *= 1 - delta;
    }

    step_number++;

}


// Simply updates and propagates back gradient
void UndirectedSoftmaxModule::bpropUpdate(const Vec& input, const Vec& output,
                                      Vec& input_gradient,
                                      const Vec& output_gradient)
{
    // compute input_gradient from initial weights
    input_gradient = transposeProduct( weights, output_gradient
                                     ).subVec( 1, input_size );

    // do the update (and size check)
    bpropUpdate( input, output, output_gradient);


}

// Update
void UndirectedSoftmaxModule::bbpropUpdate(const Vec& input, const Vec& output,
                                       const Vec& output_gradient,
                                       const Vec& output_diag_hessian)
{
    PLWARNING("UndirectedSoftmaxModule::bbpropUpdate: You're providing\n"
              "'output_diag_hessian', but it will not be used.\n");

    int odh_size = output_diag_hessian.size();
    if( odh_size != output_size )
    {
        PLERROR("UndirectedSoftmaxModule::bbpropUpdate:"
                " 'output_diag_hessian.size()'\n"
                " should be equal to 'output_size' (%i != %i)\n",
                odh_size, output_size);
    }

    bpropUpdate( input, output, output_gradient );

}

// Propagates back output_gradient and output_diag_hessian
void UndirectedSoftmaxModule::bbpropUpdate(const Vec& input, const Vec& output,
                              Vec&  input_gradient,
                              const Vec& output_gradient,
                              Vec&  input_diag_hessian,
                              const Vec& output_diag_hessian)
{
    bpropUpdate( input, output, input_gradient, output_gradient );
}


// Nothing to forget
void UndirectedSoftmaxModule::forget()
{
    resetWeights();

    if( init_weights.size() !=0 )
        weights << init_weights;
    else if (init_weights_random_scale!=0)
    {
        real r = init_weights_random_scale / input_size;
        random_generator->fill_random_uniform(weights,-r,r);
    }
    if( init_biases.size() !=0 )
        biases << init_biases;
    else
        biases.clear();

    learning_rate = start_learning_rate;
    step_number = 0;
}



void UndirectedSoftmaxModule::resetWeights()
{
    weights.resize( output_size, input_size );
    biases.resize(output_size);
    weights.fill( 0 );
}


// ### Nothing to add here, simply calls build_
void UndirectedSoftmaxModule::build()
{
    inherited::build();
    build_();
}

void UndirectedSoftmaxModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(init_weights, copies);
    deepCopyField(init_biases, copies);
    deepCopyField(weights, copies);
    deepCopyField(biases, copies);
}

void UndirectedSoftmaxModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "start_learning_rate",
                  &UndirectedSoftmaxModule::start_learning_rate,
                  OptionBase::buildoption,
                  "Learning-rate of stochastic gradient optimization");

    declareOption(ol, "decrease_constant",
                  &UndirectedSoftmaxModule::decrease_constant,
                  OptionBase::buildoption,
                  "Decrease constant of stochastic gradient optimization");

    declareOption(ol, "init_weights", &UndirectedSoftmaxModule::init_weights,
                  OptionBase::buildoption,
                  "Optional initial weights of the neurons (one row per output).\n"
                  "If not provided then weights are initialized according\n"
                  "to a uniform distribution (see init_weights_random_scale)\n"
                  "and biases are initialized to 0.\n");

    declareOption(ol, "init_biases", &UndirectedSoftmaxModule::init_biases,
                  OptionBase::buildoption,
                  "Optional initial biases (one per output neuron). If not provided\n"
                  "then biases are initialized to 0.\n");
    
    declareOption(ol, "init_weights_random_scale", &UndirectedSoftmaxModule::init_weights_random_scale,
                  OptionBase::buildoption,
                  "If init_weights is not provided, the weights are initialized randomly by\n"
                  "from a uniform in [-r,r], with r = init_weights_random_scale/input_size.\n"
                  "To clear the weights initially, just set this option to 0.");

    declareOption(ol, "L1_penalty_factor", &UndirectedSoftmaxModule::L1_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L1 regularization term, i.e.\n"
                  "minimize L1_penalty_factor * sum_{ij} |weights(i,j)| during training.\n");
    
    declareOption(ol, "L2_penalty_factor", &UndirectedSoftmaxModule::L2_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L2 regularization term, i.e.\n"
                  "minimize 0.5 * L2_penalty_factor * sum_{ij} weights(i,j)^2 during training.");
    

    declareOption(ol, "weights", &UndirectedSoftmaxModule::weights,
                  OptionBase::learntoption,
                  "Input weights of the output neurons (one row per output neuron)." );

    declareOption(ol, "biases", &UndirectedSoftmaxModule::biases,
                  OptionBase::learntoption,
                  "Biases of the output neurons.");

    inherited::declareOptions(ol);
}

void UndirectedSoftmaxModule::build_()
{
    if( input_size < 0 ) // has not been initialized
    {
        PLERROR("UndirectedSoftmaxModule::build_: 'input_size' < 0 (%i).\n"
                "You should set it to a positive integer.\n", input_size);
    }
    else if( output_size < 0 ) // default to 1 neuron
    {
        PLWARNING("UndirectedSoftmaxModule::build_: 'output_size' is < 0 (%i),\n"
                  " you should set it to a positive integer (the number of"
                  " neurons).\n"
                  " Defaulting to 1.\n", output_size);
        output_size = 1;
    }

    if( weights.size() == 0 )
    {
        resetWeights();
    }

    if (init_weights.size()==0 && init_weights_random_scale!=0 && !random_generator)
        random_generator = new PRandom();
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
