// -*- C++ -*-

// SquaredErrModule.cc
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
   * $Id: SquaredErrModule.cc,v 1.2 2005/12/30 19:53:56 lamblinp Exp $
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file PLearn/plearn_learners/online/DEPRECATED/SquaredErrModule.cc */


#include "SquaredErrModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    SquaredErrModule,
    "SquaredError Module",
    ""
    );

SquaredErrModule::SquaredErrModule() :
    /* ### Initialize all fields to their default value */
    target_size( 0 )
{
    output_size = 1;
}


// output = error = sum of squared difference between input and target
void SquaredErrModule::fprop(const Vec& input, Vec& output) const
{
    int in_size = input.size();
    // size check
    if( in_size != input_size+target_size )
    {
        PLERROR("SquaredErrModule::fprop: 'input.size()' should be equal\n"
                " to 'input_size' + 'target_size' (%i != %i + %i)\n",
                in_size, input_size, target_size);
    }

    Vec target = input.subVec( input_size, target_size );
    Vec input_ = input.subVec( 0, input_size );
    output.resize( output_size );
    output[0] = sumsquare( input_ - target );
}

// Don't modify class
void SquaredErrModule::bpropUpdate(const Vec& input, const Vec& output,
                                   const Vec& output_gradient)
{
    int in_size = input.size();
    int out_size = output.size();
    int og_size = output_gradient.size();

    // size check
    if( in_size != input_size + target_size )
    {
        PLWARNING("SquaredErrModule::bpropUpdate: 'input.size()' should be\n"
                  " equal to 'input_size' + 'target_size' (%i != %i + %i)\n",
                  in_size, input_size, target_size);
    }
    if( out_size != output_size )
    {
        PLWARNING("SquaredErrModule::bpropUpdate: output.size()' should be\n"
                  " equal to 'output_size' (%i != %i)\n",
                  out_size, output_size);
    }
    if( og_size != output_size )
    {
        PLWARNING("SquaredErrModule::bpropUpdate: 'output_gradient.size()'\n"
                  " should be equal to 'output_size' (%i != %i)\n",
                  og_size, output_size);
    }
}

// We don't care about output_gradient, we consider we're the last variable.
// So we compute the gradient of the error of this variable.
void SquaredErrModule::bpropUpdate(const Vec& input, const Vec& output,
                                   Vec& input_gradient,
                                   const Vec& output_gradient)
{
    int in_size = input.size();
    int out_size = output.size();
    int og_size = output_gradient.size();
    bool is_final_cost = false; // if yes, output_gradient is 1

    // size check
    if( in_size != input_size + target_size )
    {
        PLERROR("SquaredErrModule::bpropUpdate: 'input.size()' should be\n"
                " equal to 'input_size' + 'target_size' (%i != %i + %i)\n",
                in_size, input_size, target_size);
    }
    if( out_size != output_size )
    {
        PLERROR("SquaredErrModule::bpropUpdate: output.size()' should be\n"
                " equal to 'output_size' (%i != %i)\n",
                out_size, output_size);
    }
    if( og_size == 0 )
    {
        /*
        PLWARNING("SquaredErrModule::bpropUpdate: you are not providing"
                  "output_gradient.\n"
                  "Assuming this is the final cost, and output_gradient=1.\n");
         */
        is_final_cost = true;
    }
    else if( og_size != output_size )
    {
        PLERROR("SquaredErrModule::bpropUpdate: 'output_gradient.size()'\n"
                " should be equal to 'output_size' (%i != %i)\n",
                og_size, output_size);
    }

    Vec input_ = input.subVec( 0, input_size );
    Vec target = input.subVec( input_size, target_size );
    input_gradient.resize( input_size );
    for( int i=0 ; i<input_size ; i++ )
    {
        if( is_final_cost )
            input_gradient[i] = 2*( input_[i] - target[i] );
        else
            input_gradient[i] = 2*( input_[i] - target[i] )*output_gradient[0];
    }
}

// Does nothing (just checks and warns)
void SquaredErrModule::bbpropUpdate(const Vec& input, const Vec& output,
                                    const Vec& output_gradient,
                                    const Vec& output_diag_hessian)
{
    int odh_size = output_diag_hessian.size();
    if( odh_size != output_size )
    {
        PLWARNING("SquaredErrModule::bbpropUpdate:"
                  " 'output_diag_hessian.size()'\n"
                  " should be equal to 'output_size' (%i != %i)\n",
                  odh_size, output_size);
    }

    bpropUpdate( input, output, output_gradient );
}

// Propagates back output_gradient and output_diag_hessian
void SquaredErrModule::bbpropUpdate(const Vec& input, const Vec& output,
                                    Vec& input_gradient,
                                    const Vec& output_gradient,
                                    Vec& input_diag_hessian,
                                    const Vec& output_diag_hessian)
{
    int odh_size = output_diag_hessian.size();
    bool is_final_cost = false; // if yes, output_diag_hessian is 0

    // size check
    // others size checks will be done in bpropUpdate()
    if( odh_size == 0 )
    {
        PLWARNING("SquaredErrModule::bbpropUpdate: you are not providing"
                  " output_diag_hessian.\n"
                  "Assuming this is the final cost,"
                  " and output_diag_hessian=0.\n");
        is_final_cost = true;
    }
    else if( odh_size != output_size )
    {
        PLERROR("SquaredErrModule::bbpropUpdate:"
                " 'output_diag_hessian.size()'\n"
                " should be equal to 'output_size' (%i != %i)\n",
                odh_size, output_size);
    }

    bpropUpdate( input, output, input_gradient, output_gradient );

    Vec input_ = input.subVec( 0, input_size );
    Vec target = input.subVec( input_size, target_size );
    input_diag_hessian.resize( input_size );

    // computation of term dC/dy d²y/dx²,
    // skipped if estimate_simpler_diag_hessian, unless it is final cost
    if( estimate_simpler_diag_hessian && !is_final_cost )
    {
        input_diag_hessian.clear();
    }
    else
    {
        Vec idh( input_size, 2 );
        input_diag_hessian << idh;

        if( !is_final_cost )
            input_diag_hessian *= output_gradient[0];
    }

    // computation of term d²C/dy² (dy/dx)²,
    // skipped if it is final cost, because then d²C/dy² == d²C/dC² == 0
    if( !is_final_cost )
    {
        for( int i=0 ; i<input_size ; i++ )
        {
            real fprime_i = 2*(input_[i] - target[i]);
            input_diag_hessian[i] += (fprime_i*fprime_i)
                                       * output_diag_hessian[0];
        }
    }

}




//
void SquaredErrModule::forget()
{
//    target = Vec( input_size );
}


// ### Nothing to add here, simply calls build_
void SquaredErrModule::build()
{
    inherited::build();
    build_();
}

void SquaredErrModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

//    deepCopyField(target, copies);
}

void SquaredErrModule::declareOptions(OptionList& ol)
{
    inherited::declareOptions(ol);
}

void SquaredErrModule::build_()
{
    if( input_size < 0 )
    {
        PLWARNING("SquaredErrModule::build_: 'input_size' is < 0.\n"
                  "You should set it to a positive integer.\n"
                  "Defaulting to '1' (scalar version).");
        input_size = 1;
    }
    if( output_size != 1 )
    {
        PLWARNING("SquaredErrModule::build_: 'output_size' (%i) should be 1.\n"
                  "Setting 'output_size' to 1.\n", output_size);
        output_size = 1;
    }

    target_size = input_size;
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
