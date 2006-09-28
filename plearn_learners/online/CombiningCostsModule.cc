// -*- C++ -*-

// CombiningCostsModule.cc
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

/*! \file CombiningCostsModule.cc */



#include "CombiningCostsModule.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    CombiningCostsModule,
    "Combine several CostModules with the same input and target",
    "It is possible to assign a weight on each of the sub_costs, so the\n"
    "back-propagated gradient will be a weighted sum of the modules'"
    " gradients.\n"
    "The first output is the weighted sum of the cost, the following ones\n"
    "are the original costs.\n");

CombiningCostsModule::CombiningCostsModule() :
    n_sub_costs( 0 )
{
}

void CombiningCostsModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "sub_costs", &CombiningCostsModule::sub_costs,
                  OptionBase::buildoption,
                  "Vector containing the different sub_costs");

    declareOption(ol, "cost_weights", &CombiningCostsModule::cost_weights,
                  OptionBase::buildoption,
                  "The weights associated to each of the sub_costs");

    declareOption(ol, "n_sub_costs", &CombiningCostsModule::n_sub_costs,
                  OptionBase::learntoption,
                  "Number of sub_costs");

    // declareOption(ol, "", &CombiningCostsModule::,
    //               OptionBase::buildoption,
    //               "");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void CombiningCostsModule::build_()
{
    n_sub_costs = sub_costs.length();

    // Default value: sub_cost[0] has weight 1, the other ones have weight 0.
    if( cost_weights.length() == 0 )
    {
        cost_weights.resize( n_sub_costs );
        cost_weights.clear();
        cost_weights[0] = 1;
    }

    if( cost_weights.length() != n_sub_costs )
        PLERROR( "CombiningCostsModule::build_(): cost_weights.length()\n"
                 "should be equal to n_sub_costs (%d != %d).\n",
                 cost_weights.length(), n_sub_costs );

    output_size = n_sub_costs+1;
}

// ### Nothing to add here, simply calls build_
void CombiningCostsModule::build()
{
    inherited::build();
    build_();
}


void CombiningCostsModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(sub_costs, copies);
    deepCopyField(cost_weights, copies);
    deepCopyField(sub_costs_values, copies);
}


void CombiningCostsModule::fprop(const Vec& input, const Vec& target,
                                 Vec& cost) const
{
    assert( input.size() == input_size );
    assert( target.size() == target_size );
    cost.resize( output_size );

    for( int i=0 ; i<n_sub_costs ; i++ )
        sub_costs[i]->fprop( input, target, sub_costs_values[i] );

    cost[0] = dot( cost_weights, sub_costs_values );
    cost.subVec( 1, n_sub_costs ) << sub_costs_values;
}

void CombiningCostsModule::bpropUpdate(const Vec& input, const Vec& target,
                                       real cost, Vec& input_gradient)
{
    assert( input.size() == input_size );
    assert( target.size() == target_size );
    input_gradient.resize( input_size );

    Vec partial_gradient;
    for( int i=0 ; i<n_sub_costs ; i++ )
    {
        if( cost_weights[i] != 0. )
        {
            sub_costs[i]->bpropUpdate( input, target, sub_costs_values[i],
                                       partial_gradient );
            multiplyAcc( input_gradient, partial_gradient, cost_weights[i] );
        }
        else
            sub_costs[i]->bpropUpdate( input, target, sub_costs_values[i] );
    }
}

void CombiningCostsModule::bpropUpdate(const Vec& input, const Vec& target,
                                       real cost)
{
    assert( input.size() == input_size );
    assert( target.size() == target_size );

    for( int i=0 ; i<n_sub_costs ; i++ )
        sub_costs[i]->bpropUpdate( input, target, sub_costs_values[i] );
}

void CombiningCostsModule::bbpropUpdate(const Vec& input, const Vec& target,
                                        real cost,
                                        Vec& input_gradient,
                                        Vec& input_diag_hessian)
{
    assert( input.size() == input_size );
    assert( target.size() == target_size );
    input_gradient.resize( input_size );
    input_diag_hessian.resize( input_size );

    Vec partial_gradient;
    Vec partial_diag_hessian;
    for( int i=0 ; i<n_sub_costs ; i++ )
    {
        sub_costs[i]->bbpropUpdate( input, target, sub_costs_values[i],
                                    partial_gradient, partial_diag_hessian );
        multiplyAcc( input_gradient, partial_gradient, sub_costs_values[i] );
        multiplyAcc( input_diag_hessian, partial_diag_hessian,
                     sub_costs_values[i] );
    }
}

void CombiningCostsModule::bbpropUpdate(const Vec& input, const Vec& target,
                                        real cost)
{
    assert( input.size() == input_size );
    assert( target.size() == target_size );

    for( int i=0 ; i<n_sub_costs ; i++ )
        sub_costs[i]->bpropUpdate( input, target, sub_costs_values[i] );
}


//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void CombiningCostsModule::forget()
{
    for( int i=0 ; i<n_sub_costs ; i++ )
        sub_costs[i]->forget();
}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void CombiningCostsModule::finalize()
{
    for( int i=0 ; i<n_sub_costs ; i++ )
        sub_costs[i]->finalize();
}

//! in case bpropUpdate does not do anything, make it known
bool CombiningCostsModule::bpropDoesNothing()
{
    for( int i=0 ; i<n_sub_costs ; i++ )
        if( !(sub_costs[i]->bpropDoesNothing()) )
            return false;

    return true;
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
