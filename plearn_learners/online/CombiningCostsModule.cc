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
    
    redeclareOption(ol, "input_size", &CombiningCostsModule::input_size,
                    OptionBase::learntoption,
                    "Is set to sub_costs[0]->input_size.");
    redeclareOption(ol, "target_size", &CombiningCostsModule::target_size,
                    OptionBase::learntoption,
                    "Is set to sub_costs[0]->target_size.");
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

    if(sub_costs.length() == 0)
        PLERROR( "CombiningCostsModule::build_(): sub_costs.length()\n"
                 "should be > 0.\n");                 

    input_size = sub_costs[0]->input_size;
    target_size = sub_costs[0]->target_size;
    for(int i=1; i<sub_costs.length(); i++)
    {
        if(sub_costs[i]->input_size != input_size)
            PLERROR( "CombiningCostsModule::build_(): sub_costs[%d]->input_size"
                     " (%d)\n"
                     "should be equal to %d.\n",
                     i,sub_costs[i]->input_size, input_size);  

        if(sub_costs[i]->target_size != target_size)
            PLERROR( "CombiningCostsModule::build_(): sub_costs[%d]->target_size"
                     " (%d)\n"
                     "should be equal to %d.\n",
                     i,sub_costs[i]->target_size, target_size);  
    }

    sub_costs_values.resize( n_sub_costs );
    output_size = n_sub_costs+1;
}

///////////
// build //
///////////
void CombiningCostsModule::build()
{
    inherited::build();
    build_();
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void CombiningCostsModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(sub_costs,                copies);
    deepCopyField(cost_weights,             copies);
    deepCopyField(sub_costs_values,         copies);
    deepCopyField(sub_costs_mbatch_values,  copies);
    deepCopyField(partial_gradient,         copies);
    deepCopyField(partial_gradients,        copies);
    deepCopyField(partial_diag_hessian,     copies);
}


///////////
// fprop //
///////////
void CombiningCostsModule::fprop(const Vec& input, const Vec& target,
                                 Vec& cost) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );
    cost.resize( output_size );

    for( int i=0 ; i<n_sub_costs ; i++ )
        sub_costs[i]->fprop( input, target, sub_costs_values[i] );

    cost[0] = dot( cost_weights, sub_costs_values );
    cost.subVec( 1, n_sub_costs ) << sub_costs_values;
}

void CombiningCostsModule::fprop(const Mat& inputs, const Mat& targets,
                                 Mat& costs) const
{
    PLASSERT( inputs.width() == input_size );
    PLASSERT( targets.width() == target_size );
    costs.resize(inputs.length(), output_size);

    Mat final_cost = costs.column(0);
    final_cost.fill(0);
    Mat other_costs = costs.subMatColumns(1, n_sub_costs);
    sub_costs_mbatch_values.resize(n_sub_costs, inputs.length());
    for( int i=0 ; i<n_sub_costs ; i++ ) {
        Vec sub_costs_i = sub_costs_mbatch_values(i);
        sub_costs[i]->fprop(inputs, targets, sub_costs_i);
        Mat first_sub_cost = sub_costs_i.toMat(sub_costs_i.length(), 1);

        // final_cost += weight_i * cost_i
        multiplyAcc(final_cost, first_sub_cost, cost_weights[i]);

        // Fill the rest of the costs matrix.
        other_costs.column(i) << first_sub_cost;
    }
}


/////////////////
// bpropUpdate //
/////////////////
void CombiningCostsModule::bpropUpdate(const Vec& input, const Vec& target,
                                       real cost, Vec& input_gradient,
                                       bool accumulate)
{
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
    {
        input_gradient.resize( input_size );
        input_gradient.clear();
    }

    for( int i=0 ; i<n_sub_costs ; i++ )
    {
        if( cost_weights[i] == 0. )
        {
            // Don't compute input_gradient
            sub_costs[i]->bpropUpdate( input, target, sub_costs_values[i] );
        }
        else if( cost_weights[i] == 1. )
        {
            // Accumulate directly into input_gradient
            sub_costs[i]->bpropUpdate( input, target, sub_costs_values[i],
                                       input_gradient, true );
        }
        else
        {
            // Put the result into partial_gradient, then accumulate into
            // input_gradient with the appropriate weight
            sub_costs[i]->bpropUpdate( input, target, sub_costs_values[i],
                                       partial_gradient, false );
            multiplyAcc( input_gradient, partial_gradient, cost_weights[i] );
        }
    }
}

void CombiningCostsModule::bpropUpdate(const Mat& inputs, const Mat& targets,
        const Vec& costs, Mat& input_gradients, bool accumulate)
{
    PLASSERT( inputs.width() == input_size );
    PLASSERT( targets.width() == target_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == input_size &&
                      input_gradients.length() == inputs.length(),
                      "Cannot resize input_gradients and accumulate into it" );
    }
    else
    {
        input_gradients.resize(inputs.length(), input_size );
        input_gradients.clear();
    }


    Vec sub;
    for( int i=0 ; i<n_sub_costs ; i++ )
    {
        sub = sub_costs_mbatch_values(i);
        if( cost_weights[i] == 0. )
        {
            // Do not compute input_gradients.
            sub_costs[i]->bpropUpdate( inputs, targets, sub );
        }
        else if( cost_weights[i] == 1. )
        {
            // Accumulate directly into input_gradients.

            sub_costs[i]->bpropUpdate( inputs, targets, sub, input_gradients,
                    true );
        }
        else
        {
            // Put the result into partial_gradients, then accumulate into
            // input_gradients with the appropriate weight.
            sub_costs[i]->bpropUpdate( inputs, targets, sub, partial_gradients,
                    false);
            multiplyAcc( input_gradients, partial_gradients, cost_weights[i] );
        }
    }
}


void CombiningCostsModule::bpropUpdate(const Vec& input, const Vec& target,
                                       real cost)
{
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    for( int i=0 ; i<n_sub_costs ; i++ )
        sub_costs[i]->bpropUpdate( input, target, sub_costs_values[i] );
}

void CombiningCostsModule::bbpropUpdate(const Vec& input, const Vec& target,
                                        real cost,
                                        Vec& input_gradient,
                                        Vec& input_diag_hessian,
                                        bool accumulate)
{
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
        PLASSERT_MSG( input_diag_hessian.size() == input_size,
                      "Cannot resize input_diag_hessian AND accumulate into it"
                    );
    }
    else
    {
        input_gradient.resize( input_size );
        input_gradient.clear();
        input_diag_hessian.resize( input_size );
        input_diag_hessian.clear();
    }

    for( int i=0 ; i<n_sub_costs ; i++ )
    {
        if( cost_weights[i] == 0. )
        {
            // Don't compute input_gradient nor input_diag_hessian
            sub_costs[i]->bbpropUpdate( input, target, sub_costs_values[i] );
        }
        else if( cost_weights[i] == 1. )
        {
            // Accumulate directly into input_gradient and input_diag_hessian
            sub_costs[i]->bbpropUpdate( input, target, sub_costs_values[i],
                                        input_gradient, input_diag_hessian,
                                        true );
        }
        else
        {
            // Put temporary results into partial_*, then multiply and add to
            // input_*
            sub_costs[i]->bbpropUpdate( input, target, sub_costs_values[i],
                                        partial_gradient, partial_diag_hessian,
                                        false );
            multiplyAcc( input_gradient, partial_gradient, cost_weights[i] );
            multiplyAcc( input_diag_hessian, partial_diag_hessian,
                         cost_weights[i] );
        }
    }
}

void CombiningCostsModule::bbpropUpdate(const Vec& input, const Vec& target,
                                        real cost)
{
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    for( int i=0 ; i<n_sub_costs ; i++ )
        sub_costs[i]->bbpropUpdate( input, target, sub_costs_values[i] );
}


//! Reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void CombiningCostsModule::forget()
{
    for( int i=0 ; i<n_sub_costs ; i++ )
        sub_costs[i]->forget();
}

//! Sets the sub_costs' learning rates
void CombiningCostsModule::setLearningRate(real dynamic_learning_rate)
{
    for( int i=0 ; i<n_sub_costs ; i++ )
        sub_costs[i]->setLearningRate(dynamic_learning_rate);
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

//! Indicates the name of the computed costs
TVec<string> CombiningCostsModule::name()
{
    TVec<string> names(1, "combined_cost");
    for( int i=0 ; i<n_sub_costs ; i++ )
        names.append( sub_costs[i]->name() );

    return names;
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
