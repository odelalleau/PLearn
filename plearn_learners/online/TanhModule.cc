// -*- C++ -*-

// TanhModule.cc
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
   * $Id: TanhModule.cc,v 1.2 2005/12/30 19:53:56 lamblinp Exp $
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file TanhModule.cc */


#include "TanhModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    TanhModule,
    "Propagates a (possibly scaled) Tanh function",
    "The output is ex_scale( in_scale * input ), for each coordinate.\n"
    "Usually in_scale = ex_scale = 1, but the values in_scale = 2/3 and\n"
    "ex_scale = 1.7159 are also used.\n"
    );

TanhModule::TanhModule() :
    in_scale(1),
    ex_scale(1)
{
}

// Applies tanh to input, and propagates to output
void TanhModule::fprop(const Vec& input, Vec& output) const
{
    int in_size = input.size();

    // size check
    if( in_size != input_size )
    {
        PLERROR("TanhModule::fprop: 'input.size()' should be equal\n"
                " to 'input_size' (%i != %i)\n", in_size, input_size);
    }

    output.resize( output_size );
    for( int i=0 ; i<output_size ; i++ )
    {
        output[i] = ex_scale * tanh( in_scale * input[i] );
    }
}

// Nothing to update
void TanhModule::bpropUpdate(const Vec& input, const Vec& output,
                             const Vec& output_gradient)
{
    int in_size = input.size();
    int out_size = output.size();
    int og_size = output_gradient.size();

    // size check
    if( in_size != input_size )
    {
        PLERROR("TanhModule::bpropUpdate: 'input.size()' should be equal\n"
                " to 'input_size' (%i != %i)\n", in_size, input_size);
    }
    if( out_size != output_size )
    {
        PLERROR("TanhModule::bpropUpdate: 'output.size()' should be equal\n"
                " to 'output_size' (%i != %i)\n", out_size, output_size);
    }
    if( og_size != output_size )
    {
        PLERROR("TanhModule::bpropUpdate: 'output_gradient.size()' should\n"
                  " be equal to 'output_size' (%i != %i)\n",
                  og_size, output_size);
    }
}

// Simply propagates output_gradient to input_gradient
void TanhModule::bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient, const Vec& output_gradient)
{
    int in_size = input.size();
    int out_size = output.size();
    int og_size = output_gradient.size();

    // size check
    if( in_size != input_size )
    {
        PLERROR("TanhModule::bpropUpdate: 'input.size()' should be equal\n"
                " to 'input_size' (%i != %i)\n", in_size, input_size);
    }
    if( out_size != output_size )
    {
        PLERROR("TanhModule::bpropUpdate: 'output.size()' should be equal\n"
                " to 'output_size' (%i != %i)\n", out_size, output_size);
    }
    if( og_size != output_size )
    {
        PLERROR("TanhModule::bpropUpdate: 'output_gradient.size()' should\n"
                " be equal to 'output_size' (%i != %i)\n",
                og_size, output_size);
    }

    input_gradient.resize( input_size );
    for( int i=0 ; i<input_size ; i++ )
    {
        real output_i = output[i];
        input_gradient[i] = in_scale *
            (ex_scale - output_i*output_i/ex_scale)*output_gradient[i];
    }

}

// Nothing to update
void TanhModule::bbpropUpdate(const Vec& input, const Vec& output,
                              const Vec& output_gradient,
                              const Vec& output_diag_hessian)
{
    int odh_size = output_diag_hessian.size();
    if( odh_size != output_size )
    {
        PLERROR("TanhModule::bbpropUpdate : 'output_diag_hessian.size()'\n"
                " should be equal to 'output_size' (%i != %i)\n",
                odh_size, output_size);
    }

    bpropUpdate( input, output, output_gradient );

}

// Propagates back output_gradient and output_diag_hessian
void TanhModule::bbpropUpdate(const Vec& input, const Vec& output,
                              Vec& input_gradient,
                              const Vec& output_gradient,
                              Vec& input_diag_hessian,
                              const Vec& output_diag_hessian)
{
    int odh_size = output_diag_hessian.size();

    // size check
    // others size checks will be done in bpropUpdate()
    if( odh_size != output_size )
    {
        PLERROR("TanhModule::bbpropUpdate : 'output_diag_hessian.size()'\n"
                " should be equal to 'output_size' (%i != %i)\n",
                odh_size, output_size);
    }


    bpropUpdate( input, output, input_gradient, output_gradient );

    input_diag_hessian.resize( input_size );
    for( int i=0 ; i<input_size ; i++ )
    {
        real output_i = output[i];
        real fprime_i = in_scale * (ex_scale-output_i*output_i / ex_scale);

        if( estimate_simpler_diag_hessian )
            input_diag_hessian[i] =
                fprime_i*fprime_i*output_diag_hessian[i];
        else
            input_diag_hessian[i] =
                fprime_i*fprime_i*output_diag_hessian[i]
                - 2*in_scale/ex_scale*fprime_i*output_i*output_gradient[i];
    }

}


// Nothing to forget
void TanhModule::forget()
{}


// ### Nothing to add here, simply calls build_
void TanhModule::build()
{
    inherited::build();
    build_();
}

void TanhModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

void TanhModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "in_scale", &TanhModule::in_scale,
                  OptionBase::buildoption,
                  "Inner scale");

    declareOption(ol, "ex_scale", &TanhModule::ex_scale,
                  OptionBase::buildoption,
                  "External scale");

    inherited::declareOptions(ol);

    redeclareOption(ol, "output_size", &TanhModule::output_size,
                    OptionBase::learntoption,
                    "output_size = input_size");

}

void TanhModule::build_()
{
    if( input_size < 0 )
        PLERROR("TanhModule::build_: 'input_size' (%d) < 0", input_size);

    output_size = input_size;
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
