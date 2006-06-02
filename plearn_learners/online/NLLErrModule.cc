// -*- C++ -*-

// NLLErrModule.cc
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
   * $Id: NLLErrModule.cc,v 1.5 2005/12/30 19:53:56 lamblinp Exp $
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file NLLErrModule.cc */


#include "NLLErrModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NLLErrModule,
    "NLLError Module",
    "This class computes the Negative Log-Likelihood of the input, given the\n"
    "desired 'target'. Also propagates gradient and diagonal of Hessian\n"
    "backwards.\n"
    );

NLLErrModule::NLLErrModule():
    target_size(1)
    /* ### Initialize all fields to their default value */
{
    output_size = 1;
}


// retrieve target from input vector
int NLLErrModule::getTarget(const Vec& input) const
{
    int t_size = input.size() - input_size;
    int target = -1;

    // size check
    if( t_size == 1 )
    {
        target = (int) input[ input_size ];
    }
    else if( t_size == input_size )
    {
        /*
        PLWARNING("NLLErrModule::getTarget: You're giving a target the same\n"
                  "size as the input, instead of an integer. Checking if\n"
                  "this is a one-hot vector from this integer.\n");
         */

        bool invalid_target = false;
        Vec the_target = input.subVec( input_size, t_size );
        // get position of '1'
        target = argmax( the_target );

#ifdef BOUNDCHECK
        // check if vector matches with a one-hot one
        assert( is_equal( the_target[target], 1. ) ) ;
        for( int i=0 ; i<input_size ; i++ )
            assert( is_equal( the_target[i], 0. ) || i == target );
#endif
    }
    else
    {
        PLERROR("NLLErrModule::getTarget: target.size() is %i,\n"
                " but should be 1. 'target' should contain an integer.\n",
                t_size);
    }

    if( target < 0 || target >= input_size )
        PLERROR("NLLErrModule::getTarget: target should be between 0 and"
                "input_size (%i).\n", input_size);

    return target;
}

// output = error = log(softmax(input))[target]
void NLLErrModule::fprop(const Vec& input, Vec& output) const
{
    int target = getTarget( input );
    // size check is done in getTarget()

    Vec input_ = input.subVec( 0, input_size );
    output.resize( output_size );
    output[0] = - pl_log( softmax( input_ )[target] );
}

// Don't modify class
void NLLErrModule::bpropUpdate(const Vec& input, const Vec& output,
                               const Vec& output_gradient)
{
    int out_size = output.size();
    int og_size = output_gradient.size();

    // for size check
    getTarget( input );

    // size check
    if( out_size != output_size )
    {
        PLWARNING("NLLErrModule::bpropUpdate: output.size()' should be\n"
                  " equal to 'output_size' (%i != %i)\n",
                  out_size, output_size);
    }
    if( og_size != output_size )
    {
        PLWARNING("NLLErrModule::bpropUpdate: 'output_gradient.size()'\n"
                  " should be equal to 'output_size' (%i != %i)\n",
                  og_size, output_size);
    }
}

// We don't care about output_gradient, we consider we're the last variable.
// So we compute the gradient of the error of this variable.
void NLLErrModule::bpropUpdate(const Vec& input, const Vec& output,
                               Vec& input_gradient,
                               const Vec& output_gradient)
{
    int out_size = output.size();
    int og_size = output_gradient.size();
    int target = getTarget( input );
    bool is_final_cost = false; // if yes, output_gradient is 1

    // size check
    if( out_size != output_size )
    {
        PLERROR("NLLErrModule::bpropUpdate: output.size()' should be\n"
                " equal to 'output_size' (%i != %i)\n",
                out_size, output_size);
    }
    if( og_size == 0 )
    {
        /*
        PLWARNING("NLLErrModule::bpropUpdate: you are not providing"
                  "output_gradient.\n"
                  "Assuming this is the final cost, and output_gradient=1.\n");
         */
        is_final_cost = true;
    }
    else if( og_size != output_size )
    {
        PLERROR("NLLErrModule::bpropUpdate: 'output_gradient.size()'\n"
                " should be equal to 'output_size' (%i != %i)\n",
                og_size, output_size);
    }

    Vec input_ = input.subVec( 0, input_size );

    // input_gradient[i] = output_gradient*( softmax(input)[i] ) if i!=target
    // input_gradient[target] = output_gradient*( softmax(input)[target] )
    input_gradient.resize( input_size );
    input_gradient << softmax( input_ );
    input_gradient[target] -= 1;
    if( !is_final_cost )
        input_gradient *= output_gradient[0];

}

// Does nothing (just checks and warns)
void NLLErrModule::bbpropUpdate(const Vec& input, const Vec& output,
                                const Vec& output_gradient,
                                const Vec& output_diag_hessian)
{
    int odh_size = output_diag_hessian.size();
    if( odh_size != output_size )
    {
        PLWARNING("NLLErrModule::bbpropUpdate:"
                  " 'output_diag_hessian.size()'\n"
                  " should be equal to 'output_size' (%i != %i)\n",
                  odh_size, output_size);
    }

    bpropUpdate( input, output, output_gradient );
}

// Propagates back output_gradient and output_diag_hessian
void NLLErrModule::bbpropUpdate(const Vec& input, const Vec& output,
                                Vec& input_gradient,
                                const Vec& output_gradient,
                                Vec& input_diag_hessian,
                                const Vec& output_diag_hessian)
{
    int odh_size = output_diag_hessian.size();
    int target = getTarget( input );
    bool is_final_cost = false; // if yes, output_diag_hessian is 0

    // size check
    // others size checks will be done in bpropUpdate()
    if( odh_size == 0 )
    {
        PLWARNING("NLLErrModule::bbpropUpdate: you are not providing"
                  " output_diag_hessian.\n"
                  "Assuming this is the final cost,"
                  " and output_diag_hessian=0.\n");
        is_final_cost = true;
    }
    else if( odh_size != output_size )
    {
        PLERROR("NLLErrModule::bbpropUpdate:"
                " 'output_diag_hessian.size()'\n"
                " should be equal to 'output_size' (%i != %i)\n",
                odh_size, output_size);
    }

    bpropUpdate( input, output, input_gradient, output_gradient );

    Vec input_ = input.subVec( 0, input_size );
    input_diag_hessian.resize( input_size );
    Vec softmax_in = softmax( input_ );

    // computation of term dC/dy d²y/dx²,
    // skipped if estimate_simpler_diag_hessian, unless it is final cost
    if( estimate_simpler_diag_hessian && !is_final_cost )
    {
        input_diag_hessian.clear();
    }
    else
    {
        for( int i=0 ; i<input_size ; i++ )
        {
            real sm_i = softmax_in[i];
            input_diag_hessian[i] = sm_i*( 1-sm_i);
        }

        if( !is_final_cost )
            input_diag_hessian *= output_gradient[0];

    }

    // computation of term d²C/dy² (dy/dx)²,
    // skipped if it is final cost, because then d²C/dy² == d²C/dC² == 0
    if( !is_final_cost )
    {
        Vec fprime = softmax_in;
        fprime[target] -= 1;
        fprime *= fprime;

        input_diag_hessian += output_diag_hessian[0] * fprime;
    }

}




//
void NLLErrModule::forget()
{
}


// ### Nothing to add here, simply calls build_
void NLLErrModule::build()
{
    inherited::build();
    build_();
}

void NLLErrModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

}

void NLLErrModule::declareOptions(OptionList& ol)
{
    inherited::declareOptions(ol);
}

void NLLErrModule::build_()
{
    if( input_size < 0 )
    {
        PLWARNING("NLLErrModule::build_: 'input_size' is < 0.\n"
                  "You should set it to a positive integer.\n"
                  "Defaulting to '1' (like a sigmoid function ?)\n");
        input_size = 1;
    }
    if( output_size != 1 )
    {
        PLWARNING("NLLErrModule::build_: 'output_size' (%i) should be 1.\n"
                  "Setting 'output_size' to 1.\n", output_size);
        output_size = 1;
    }

    target_size = 1;
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
