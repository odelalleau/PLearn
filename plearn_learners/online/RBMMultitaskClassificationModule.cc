// -*- C++ -*-

// RBMMultitaskClassificationModule.cc
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

/*! \file RBMMultitaskClassificationModule.cc */


#define PL_LOG_MODULE_NAME "RBMMultitaskClassificationModule"

#include "RBMMultitaskClassificationModule.h"
#include <plearn/io/pl_log.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMMultitaskClassificationModule,
    "Computes a mean-field approximate of p(y|x), with y a binary vector.",
    "This module contains, from bottom to top:\n"
    "  - an RBMConnection - previous_to_last,\n"
    "  - an RBMBinomialLayer - last_layer,\n"
    "  - an RBMMatrixConnection (transposed) - last_to_target,\n"
    "  - and an RBMBinomialLayer - target_layer.\n"
    "The two RBMConnections are combined in joint_connection.\n");

RBMMultitaskClassificationModule::RBMMultitaskClassificationModule():
    n_mean_field_iterations( 1 ),
    fprop_outputs_activation( false )
{
}

void RBMMultitaskClassificationModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "previous_to_last",
                  &RBMMultitaskClassificationModule::previous_to_last,
                  OptionBase::buildoption,
                  "Connection between the previous layer, and last_layer.\n");

    declareOption(ol, "last_layer", &RBMMultitaskClassificationModule::last_layer,
                  OptionBase::buildoption,
                  "Top-level layer (the one in the middle if we unfold).\n");

    declareOption(ol, "last_to_target",
                  &RBMMultitaskClassificationModule::last_to_target,
                  OptionBase::buildoption,
                  "Connection between last_layer and target_layer.\n");

    declareOption(ol, "target_layer", &RBMMultitaskClassificationModule::target_layer,
                  OptionBase::buildoption,
                  "Layer containing the one-hot vector containing the target\n"
                  "(or its prediction).\n");

    declareOption(ol, "joint_connection",
                  &RBMMultitaskClassificationModule::joint_connection,
                  OptionBase::learntoption,
                  "Connection grouping previous_to_last and last_to_target.\n");

    declareOption(ol, "n_mean_field_iterations",
                  &RBMMultitaskClassificationModule::n_mean_field_iterations,
                  OptionBase::buildoption,
                  "Number of mean-field iterations.\n");

    declareOption(ol, "fprop_outputs_activation",
                  &RBMMultitaskClassificationModule::fprop_outputs_activation,
                  OptionBase::buildoption,
                  "Indication that fprop should output the value of the "
                  "activation\n"
                  "before the squashing function and the application of the bias,\n"
                  "instead of the mean-field approximation.\n");

    declareOption(ol, "last_size", &RBMMultitaskClassificationModule::last_size,
                  OptionBase::learntoption,
                  "Size of last_layer.\n");
    /*
    declareOption(ol, "", &RBMMultitaskClassificationModule::,
                  OptionBase::buildoption,
                  "");
     */

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMMultitaskClassificationModule::build_()
{
    MODULE_LOG << "build_() called" << endl;

    if( !previous_to_last || !last_layer || !last_to_target || !target_layer )
    {
        MODULE_LOG << "build_() aborted because layers and connections were"
           " not set" << endl;
        return;
    }

    //! Check (and set) sizes
    input_size = previous_to_last->down_size;
    last_size = last_layer->size;
    output_size = target_layer->size;

    PLASSERT( previous_to_last->up_size == last_size );
    PLASSERT( last_to_target->up_size == last_size );
    PLASSERT( last_to_target->down_size == output_size );

    //! build joint_connection
    if( !joint_connection )
        joint_connection = new RBMMixedConnection();

    joint_connection->sub_connections.resize(1,2);
    joint_connection->sub_connections(0,0) = previous_to_last;
    joint_connection->sub_connections(0,1) = last_to_target;
    joint_connection->build();

    if( n_mean_field_iterations > 0 )
    {
        mean_field_activations_target.resize( n_mean_field_iterations );
        mean_field_approximations_target.resize( n_mean_field_iterations );
        mean_field_activations_hidden.resize( n_mean_field_iterations );
        mean_field_approximations_hidden.resize( n_mean_field_iterations );
        for( int i=0; i<n_mean_field_iterations; i++ )
        {
            mean_field_activations_target[i].resize( output_size );
            mean_field_approximations_target[i].resize( output_size );
            mean_field_activations_hidden[i].resize( last_size );
            mean_field_approximations_hidden[i].resize( last_size );
        }
        mean_field_activations_gradient_target.resize( output_size );
        mean_field_approximations_gradient_target.resize( output_size );
        mean_field_activations_gradient_hidden.resize( last_size );
        mean_field_approximations_gradient_hidden.resize( last_size );
    }
    else
        PLERROR("In RBMMultitaskClassificationModule::build_(): "
                "n_mean_field_iterations should be > 0\n");

    last_to_target_gradient.resize( last_to_target->up_size, 
                                    last_to_target->down_size );

    // If we have a random_gen, share it with the ones who do not
    if( random_gen )
    {
        if( !(previous_to_last->random_gen) )
        {
            previous_to_last->random_gen = random_gen;
            previous_to_last->forget();
        }
        if( !(last_layer->random_gen) )
        {
            last_layer->random_gen = random_gen;
            last_layer->forget();
        }
        if( !(last_to_target->random_gen) )
        {
            last_to_target->random_gen = random_gen;
            last_to_target->forget();
        }
        if( !(target_layer->random_gen) )
        {
            target_layer->random_gen = random_gen;
            target_layer->forget();
        }
        if( !(joint_connection->random_gen) )
        {
            joint_connection->random_gen = random_gen;
            joint_connection->forget();
        }
    }
}

// ### Nothing to add here, simply calls build_
void RBMMultitaskClassificationModule::build()
{
    inherited::build();
    build_();
}


void RBMMultitaskClassificationModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(previous_to_last, copies);
    deepCopyField(last_layer, copies);
    deepCopyField(last_to_target, copies);
    deepCopyField(target_layer, copies);
    deepCopyField(joint_connection, copies);
    deepCopyField(mean_field_activations_target, copies);
    deepCopyField(mean_field_approximations_target, copies);
    deepCopyField(mean_field_activations_hidden, copies);
    deepCopyField(mean_field_approximations_hidden, copies);
    deepCopyField(last_to_target_gradient, copies);
    deepCopyField(mean_field_activations_gradient_target, copies);
    deepCopyField(mean_field_approximations_gradient_target, copies);
    deepCopyField(mean_field_activations_gradient_hidden, copies);
    deepCopyField(mean_field_approximations_gradient_hidden, copies);
}

void RBMMultitaskClassificationModule::fprop(const Vec& input, Vec& output) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    previous_to_last->fprop( input, mean_field_activations_hidden[0] );
    last_layer->fprop( mean_field_activations_hidden[0], 
                       mean_field_approximations_hidden[0] );

    Mat weights = last_to_target->weights;
    for( int t=0; t<n_mean_field_iterations; t++ )
    {
        transposeProduct( mean_field_activations_target[t], weights, 
                          mean_field_approximations_hidden[t] );
        target_layer->fprop( mean_field_activations_target[t],
                             mean_field_approximations_target[t] );
        
        if( t != n_mean_field_iterations -1 )
        {
            product( mean_field_activations_hidden[t+1], weights, 
                     mean_field_approximations_target[t] );
            mean_field_activations_hidden[t+1] += mean_field_activations_hidden[0];
            last_layer->fprop( mean_field_activations_hidden[t+1],
                               mean_field_approximations_hidden[t+1] );
        }
    }
    
    if( fprop_outputs_activation )
    {
        output << mean_field_activations_target.last();
        //output += target_layer->bias;
    }
    else
        output << mean_field_approximations_target.last();
}

/* THIS METHOD IS OPTIONAL
//! Adapt based on the output gradient: this method should only
//! be called just after a corresponding fprop; it should be
//! called with the same arguments as fprop for the first two arguments
//! (and output should not have been modified since then).
//! Since sub-classes are supposed to learn ONLINE, the object
//! is 'ready-to-be-used' just after any bpropUpdate.
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! JUST CALLS
//!     bpropUpdate(input, output, input_gradient, output_gradient)
//! AND IGNORES INPUT GRADIENT.
void RBMMultitaskClassificationModule::bpropUpdate(const Vec& input, const Vec& output,
                               const Vec& output_gradient)
{
}
*/

//! this version allows to obtain the input gradient as well
void RBMMultitaskClassificationModule::bpropUpdate(const Vec& input, const Vec& output,
                                          Vec& input_gradient,
                                          const Vec& output_gradient,
                                          bool accumulate)
{
    // size checks
    PLASSERT( input.size() == input_size );
    PLASSERT( output.size() == output_size );
    PLASSERT( output_gradient.size() == output_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }

    last_to_target_gradient.clear();
    Mat weights = last_to_target->weights;
    if( fprop_outputs_activation )
        mean_field_activations_gradient_target << output_gradient;
    else
        mean_field_approximations_gradient_target << output_gradient;

    for( int t=n_mean_field_iterations-1; t>=0; t-- )
    {
        if( t != n_mean_field_iterations-1 || !fprop_outputs_activation )
            target_layer->bpropUpdate( mean_field_activations_target[t],
                                       mean_field_approximations_target[t],
                                       mean_field_activations_gradient_target,
                                       mean_field_approximations_gradient_target
                );

        externalProductAcc( last_to_target_gradient,
                            mean_field_approximations_hidden[t],
                            mean_field_activations_gradient_target);

        product( mean_field_approximations_gradient_hidden, weights, 
                          mean_field_activations_gradient_target);
        
        if( t != 0 )
        {
            last_layer->bpropUpdate( mean_field_activations_hidden[t],
                                       mean_field_approximations_hidden[t],
                                       mean_field_activations_gradient_hidden,
                                       mean_field_approximations_gradient_hidden
                );

            externalProductAcc( last_to_target_gradient,
                                mean_field_activations_gradient_hidden,
                                mean_field_approximations_target[t-1]
                                );

            transposeProduct( mean_field_approximations_gradient_target, weights, 
                              mean_field_activations_gradient_hidden);
        }
    }

    last_layer->bpropUpdate( mean_field_activations_hidden[0],
                             mean_field_approximations_hidden[0],
                             mean_field_activations_gradient_hidden,
                             mean_field_approximations_gradient_hidden
        );

    previous_to_last->bpropUpdate( input, mean_field_activations_hidden[0],
                                   input_gradient, 
                                   mean_field_activations_gradient_hidden,
                                   accumulate);

    multiplyAcc( weights, last_to_target_gradient, 
                 - (last_to_target->learning_rate) );
}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void RBMMultitaskClassificationModule::forget()
{
    if( !random_gen )
    {
        PLWARNING("RBMMultitaskClassificationModule: cannot forget() without"
                  " random_gen");
        return;
    }

    if( !(previous_to_last->random_gen) )
        previous_to_last->random_gen = random_gen;
    previous_to_last->forget();
    if( !(last_to_target->random_gen) )
        last_to_target->random_gen = random_gen;
    last_to_target->forget();
    if( !(joint_connection->random_gen) )
        joint_connection->random_gen = random_gen;
    joint_connection->forget();
}

/* THIS METHOD IS OPTIONAL
//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! JUST CALLS
//!     bbpropUpdate(input, output, input_gradient, output_gradient,
//!                  in_hess, out_hess)
//! AND IGNORES INPUT HESSIAN AND INPUT GRADIENT.
void RBMMultitaskClassificationModule::bbpropUpdate(const Vec& input, const Vec& output,
                                           const Vec& output_gradient,
                                           const Vec& output_diag_hessian)
{
}
*/

/* THIS METHOD IS OPTIONAL
//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! RAISES A PLERROR.
void RBMMultitaskClassificationModule::bbpropUpdate(const Vec& input, const Vec& output,
                                           Vec& input_gradient,
                                           const Vec& output_gradient,
                                           Vec& input_diag_hessian,
                                           const Vec& output_diag_hessian,
                                           bool accumulate)
{
}
*/


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
