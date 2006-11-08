// -*- C++ -*-

// RBMClassificationModule.cc
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

/*! \file RBMClassificationModule.cc */


#define PL_LOG_MODULE_NAME "RBMClassificationModule"
#include <plearn/io/pl_log.h>

#include "RBMClassificationModule.h"

#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMClassificationModule,
    "Computes the undirected softmax used in deep belief nets",
    "This module contains, from bottom to top:\n"
    "  - an RBMConnection - previous_to_last,\n"
    "  - an RBMBinomialLayer - last_layer,\n"
    "  - an RBMMatrixConnection (transposed) - last_to_target,\n"
    "  - and an RBMMultinomialLayer - target_layer.\n"
    "The two RBMConnections are combined in joint_connection.\n");

RBMClassificationModule::RBMClassificationModule()
{
}

void RBMClassificationModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "previous_to_last",
                  &RBMClassificationModule::previous_to_last,
                  OptionBase::buildoption,
                  "Connection between the previous layer, and last_layer");

    declareOption(ol, "last_layer", &RBMClassificationModule::last_layer,
                  OptionBase::buildoption,
                  "Top-level layer (the one in the middle if we unfold)");

    declareOption(ol, "last_to_target",
                  &RBMClassificationModule::last_to_target,
                  OptionBase::buildoption,
                  "Connection between last_layer and target_layer");

    declareOption(ol, "target_layer", &RBMClassificationModule::target_layer,
                  OptionBase::buildoption,
                  "Layer containing the one-hot vector containing the target\n"
                  "(or its prediction).\n");

    declareOption(ol, "joint_connection",
                  &RBMClassificationModule::joint_connection,
                  OptionBase::learntoption,
                  "Connection grouping previous_to_last and last_to_target");

    declareOption(ol, "last_size", &RBMClassificationModule::last_size,
                  OptionBase::learntoption,
                  "Size of last_layer");
    /*
    declareOption(ol, "", &RBMClassificationModule::,
                  OptionBase::buildoption,
                  "");
     */

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMClassificationModule::build_()
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

    d_last_act.resize( last_size );
    d_target_act.resize( output_size );

    //! build joint_connection
    if( !joint_connection )
        joint_connection = new RBMMixedConnection();

    joint_connection->sub_connections.resize(1,2);
    joint_connection->sub_connections(0,0) = previous_to_last;
    joint_connection->sub_connections(0,1) = last_to_target;
    joint_connection->build();
}

// ### Nothing to add here, simply calls build_
void RBMClassificationModule::build()
{
    inherited::build();
    build_();
}


void RBMClassificationModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(previous_to_last, copies);
    deepCopyField(last_layer, copies);
    deepCopyField(last_to_target, copies);
    deepCopyField(target_layer, copies);
    deepCopyField(joint_connection, copies);
    deepCopyField(out_act, copies);
    deepCopyField(d_target_act, copies);
}

//! given the input, compute the output (possibly resize it  appropriately)
void RBMClassificationModule::fprop(const Vec& input, Vec& output) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    // input is supposed to be an expectation or sample from the previous layer
    previous_to_last->setAsDownInput( input );

    // last_layer->activation = bias + previous_to_last_weights * input
    last_layer->getAllActivations( previous_to_last );

    // target_layer->activation =
    //      bias - sum_j softplus(-(W_ji + last_layer->activation[j]))
    Vec target_act = target_layer->activation;
    for( int i=0 ; i<output_size ; i++ )
    {
        target_act[i] = target_layer->bias[i];
        real *w = &(last_to_target->weights(0,i));
        // step from one row to the next in weights matrix
        int m = last_to_target->weights.mod();

        Vec last_act = last_layer->activation;
        for( int j=0 ; j<last_size ; j++, w+=m )
        {
            // *w = weights(j,i)
            target_act[i] -= softplus( -(*w + last_act[j]) );
        }
    }

    target_layer->expectation_is_up_to_date = false;
    target_layer->computeExpectation();
    output << target_layer->expectation;
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
void RBMClassificationModule::bpropUpdate(const Vec& input, const Vec& output,
                               const Vec& output_gradient)
{
}
*/

//! this version allows to obtain the input gradient as well
void RBMClassificationModule::bpropUpdate(const Vec& input, const Vec& output,
                                          Vec& input_gradient,
                                          const Vec& output_gradient)
{
    // size checks
    PLASSERT( input.size() == input_size );
    PLASSERT( output.size() == output_size );
    PLASSERT( output_gradient.size() == output_size );
    input_gradient.resize( input_size );
    input_gradient.clear();

    // bpropUpdate in target_layer,
    // assuming target_layer->activation is up-to-date, but it should be the
    // case if fprop() has been called just before.
    target_layer->bpropUpdate( target_layer->activation, output,
                               d_target_act, output_gradient );

    // the tricky part is the backpropagation through last_to_target
    Vec last_act = last_layer->activation;
    for( int i=0 ; i<last_size ; i++ )
    {
        real* w = last_to_target->weights[i];
        d_last_act[i] = 0;
        for( int k=0 ; k<output_size ; k++ )
        {
            // dC/d( w_ik + target_act_i )
            real d_z = d_target_act[k]*(sigmoid(-w[k] - last_act[i]));
            w[k] -= last_to_target->learning_rate * d_z;

            d_last_act[i] += d_z;
        }
    }

    // don't use bpropUpdate(), because the function is different here
    // last_layer->bias -= learning_rate * d_last_act;
    multiplyAcc( last_layer->bias, d_last_act, -(last_layer->learning_rate) );

    // at this point, the gradient can be backpropagated through
    // previous_to_last the usual way (even if output is wrong)
    previous_to_last->bpropUpdate( input, last_act,
                                   input_gradient, d_last_act );

}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void RBMClassificationModule::forget()
{
    previous_to_last->forget();
    last_to_target->forget();
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
void RBMClassificationModule::bbpropUpdate(const Vec& input, const Vec& output,
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
void RBMClassificationModule::bbpropUpdate(const Vec& input, const Vec& output,
                                Vec& input_gradient,
                                const Vec& output_gradient,
                                Vec& input_diag_hessian,
                                const Vec& output_diag_hessian)
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
