// -*- C++ -*-

// SquaredErrorCostModule.cc
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

/*! \file SquaredErrorCostModule.cc */



#include "SquaredErrorCostModule.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    SquaredErrorCostModule,
    "Computes the sum of squared difference between input and target.",
    "");

SquaredErrorCostModule::SquaredErrorCostModule()
{
    output_size = 1;
}

void SquaredErrorCostModule::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &SquaredErrorCostModule::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void SquaredErrorCostModule::build_()
{
    target_size = input_size;
}

// ### Nothing to add here, simply calls build_
void SquaredErrorCostModule::build()
{
    inherited::build();
    build_();
}


void SquaredErrorCostModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


void SquaredErrorCostModule::fprop(const Vec& input_and_target, Vec& output)
    const
{
    fprop( input_and_target.subVec( 0, input_size ),
           input_and_target.subVec( input_size, target_size ),
           output );
}

void SquaredErrorCostModule::fprop(const Vec& input, const Vec& target,
                                   Vec& cost) const
{
    assert( input.size() == input_size );
    assert( target.size() == target_size );
    cost.resize( output_size );

    cost[0] = powdistance( input, target );
}


void SquaredErrorCostModule::bpropUpdate(const Vec& input, const Vec& target,
                                         real cost, Vec& input_gradient)
{
    assert( input.size() == input_size );
    assert( target.size() == target_size );
    input_gradient.resize( input_size );

    // input_gradient = 2*(input - target)
    substract( input, target, input_gradient );
    input_gradient *= 2;
}


void SquaredErrorCostModule::bbpropUpdate(const Vec& input, const Vec& target,
                                          real cost,
                                          Vec& input_gradient,
                                          Vec& input_diag_hessian)
{
    bpropUpdate( input, target, cost, input_gradient );

    input_diag_hessian.resize( input_size );
    input_diag_hessian.fill( 2 );
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
