// -*- C++ -*-

// CostModule.cc
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

/*! \file CostModule.cc */



#include "CostModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    CostModule,
    "General class representing a cost function module",
    "It usually takes an input and a target, and outputs one cost.\n"
    "It can also output more costs, in that case the first one will be the\n"
    "objective function to be decreased.\n");

CostModule::CostModule() :
    target_size(-1)
{
}

void CostModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "target_size", &CostModule::target_size,
                  OptionBase::buildoption,
                  "Size of the target vectors.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "output_size", &CostModule::output_size,
                    OptionBase::buildoption,
                    "Number of costs (outputs).");
}

void CostModule::build_()
{
}

// ### Nothing to add here, simply calls build_
void CostModule::build()
{
    inherited::build();
    build_();
}


void CostModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


void CostModule::fprop(const Vec& input, const Vec& target, Vec& cost) const
{
    PLERROR("CostModule::fprop(const Vec& input, const Vec& target, Vec& cost)"
            "\n"
            "is not implemented. You have to implement it in your class.\n");
}

//! keeps only the first cost
void CostModule::fprop(const Vec& input, const Vec& target, real& cost) const
{
    Vec costs;
    fprop( input, target, costs );

    cost = costs[0];
}

//! for compatibility with OnlineLearningModule interface
void CostModule::fprop(const Vec& input_and_target, Vec& output) const
{
    PLASSERT( input_and_target.size() == input_size + target_size );
    fprop( input_and_target.subVec( 0, input_size ),
           input_and_target.subVec( input_size, target_size ),
           output );
}


void CostModule::bpropUpdate(const Vec& input, const Vec& target, real cost,
                             Vec& input_gradient)
{
    // default version, calling the bpropUpdate with inherited prototype
    Vec input_and_target( input_size + target_size );
    input_and_target.subVec( 0, input_size ) << input;
    input_and_target.subVec( input_size, target_size ) << target;
    Vec input_and_target_gradient( input_size + target_size );
    Vec the_cost( 1, cost );
    Vec one( 1, 1 );

    bpropUpdate( input_and_target, the_cost, input_and_target_gradient, one );

    input_gradient = input_and_target_gradient.subVec( 0, input_size );
}

void CostModule::bpropUpdate(const Vec& input, const Vec& target, real cost)
{
    Vec input_gradient;
    bpropUpdate( input, target, cost, input_gradient );
}

void CostModule::bpropUpdate(const Vec& input_and_target, const Vec& output,
                             Vec& input_and_target_gradient,
                             const Vec& output_gradient,
                             bool accumulate)
{
    PLASSERT_MSG(!accumulate,"Implementation of bpropUpdate cannot yet handle accumulate=false");

    inherited::bpropUpdate( input_and_target, output,
                            input_and_target_gradient, output_gradient );
}


void CostModule::bbpropUpdate(const Vec& input, const Vec& target, real cost,
                              Vec& input_gradient, Vec& input_diag_hessian)
{
    // default version, calling the bpropUpdate with inherited prototype
    Vec input_and_target( input_size + target_size );
    input_and_target.subVec( 0, input_size ) << input;
    input_and_target.subVec( input_size, target_size ) << target;
    Vec input_and_target_gradient( input_size + target_size );
    Vec input_and_target_diag_hessian( input_size + target_size );
    Vec the_cost( 1, cost );
    Vec one( 1, 1 );
    Vec zero( 1 );

    bbpropUpdate( input_and_target, the_cost,
                  input_and_target_gradient, one,
                  input_and_target_diag_hessian, zero );

    input_gradient = input_and_target_gradient.subVec( 0, input_size );
    input_diag_hessian = input_and_target_diag_hessian.subVec( 0, input_size );
}

void CostModule::bbpropUpdate(const Vec& input, const Vec& target, real cost)
{
    Vec input_gradient;
    Vec input_diag_hessian;
    bbpropUpdate( input, target, cost, input_gradient, input_diag_hessian );
}

void CostModule::bbpropUpdate(const Vec& input_and_target, const Vec& output,
                              Vec& input_and_target_gradient,
                              const Vec& output_gradient,
                              Vec& input_and_target_diag_hessian,
                              const Vec& output_diag_hessian,
                              bool accumulate)
{
    PLASSERT_MSG(!accumulate,"Implementation of bbpropUpdate cannot yet handle accumulate=false");
    inherited::bbpropUpdate( input_and_target, output,
                             input_and_target_gradient,
                             output_gradient,
                             input_and_target_diag_hessian,
                             output_diag_hessian );
}

void CostModule::forget()
{
}

TVec<string> CostModule::name()
{
    return TVec<string>();
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
