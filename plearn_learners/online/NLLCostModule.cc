// -*- C++ -*-

// NLLCostModule.cc
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

/*! \file NLLCostModule.cc */



#include "NLLCostModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NLLCostModule,
    "Computes the NLL, given a probability vector and the true class.",
    "If input is the probability vector, and target the index of the true\n"
    "class, this module computes cost = -log( input[target] ), and\n"
    "back-propagates the gradient and diagonal of Hessian.\n");

NLLCostModule::NLLCostModule()
{
    output_size = 1;
    target_size = 1;
}

void NLLCostModule::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &NLLCostModule::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void NLLCostModule::build_()
{
}

// ### Nothing to add here, simply calls build_
void NLLCostModule::build()
{
    inherited::build();
    build_();
}


void NLLCostModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


///////////
// fprop //
///////////
void NLLCostModule::fprop(const Vec& input, const Vec& target, Vec& cost) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );
    cost.resize( output_size );

    int the_target = (int) round( target[0] );
    cost[0] = -pl_log( input[ the_target ] );
}

/////////////////
// bpropUpdate //
/////////////////
void NLLCostModule::bpropUpdate(const Vec& input, const Vec& target, real cost,
                                Vec& input_gradient, bool accumulate)
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

    int the_target = (int) round( target[0] );
    // input_gradient[ i ] = 0 if i != t,
    // input_gradient[ t ] = -1/x[t]
    input_gradient[ the_target ] -= 1. / input[ the_target ];
}

void NLLCostModule::bpropUpdate(const Mat& inputs, const Mat& targets,
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

    // input_gradient[ i ] = 0 if i != t,
    // input_gradient[ t ] = -1/x[t]
    for (int i = 0; i < inputs.length(); i++) {
        int the_target = (int) round( targets(i, 0) );
        input_gradients(i, the_target) -= 1. / inputs(i, the_target);
    }
}

void NLLCostModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                   const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_value.length() == nPorts() );
    PLASSERT( ports_gradient.length() == nPorts() );

    Mat* prediction = ports_value[0];
    Mat* target = ports_value[1];
    Mat* cost = ports_value[2];
    Mat* prediction_grad = ports_gradient[0];
    Mat* target_grad = ports_gradient[1];
    Mat* cost_grad = ports_gradient[2];

    // If we have cost_grad and we want prediction_grad
    if( prediction_grad && prediction_grad->isEmpty()
        && cost_grad && !cost_grad->isEmpty() )
    {
        PLASSERT( prediction );
        PLASSERT( target );
        PLASSERT( cost );
        PLASSERT( !target_grad );

        PLASSERT( prediction->width() == port_sizes(0,1) );
        PLASSERT( target->width() == port_sizes(1,1) );
        PLASSERT( cost->width() == port_sizes(2,1) );
        PLASSERT( prediction_grad->width() == port_sizes(0,1) );
        PLASSERT( cost_grad->width() == port_sizes(2,1) );

        int batch_size = prediction->length();
        PLASSERT( target->length() == batch_size );
        PLASSERT( cost->length() == batch_size );
        PLASSERT( cost_grad->length() == batch_size );

        prediction_grad->resize(batch_size, port_sizes(0,1));

        for( int k=0; k<batch_size; k++ )
        {
            // input_gradient[ i ] = 0 if i != t,
            // input_gradient[ t ] = -1/x[t]
            int target_k = (int) round((*target)(k, 0));
            (*prediction_grad)(k, target_k) -=
                (*cost_grad)(k, 0) / (*prediction)(k, target_k);
        }
    }
    else if( !prediction_grad && !target_grad && !cost_grad )
        return;
    else if( !cost_grad && prediction_grad && prediction_grad->isEmpty() )
        PLERROR("In NLLCostModule::bpropAccUpdate - cost gradient is NULL,\n"
                "cannot compute prediction gradient. Maybe you should set\n"
                "\"propagate_gradient = 0\" on the incoming connection.\n");
    else
        PLERROR("In OnlineLearningModule::bpropAccUpdate - Port configuration "
                "not implemented for class '%s'", classname().c_str());
}

void NLLCostModule::bbpropUpdate(const Vec& input, const Vec& target,
                                 real cost,
                                 Vec& input_gradient, Vec& input_diag_hessian,
                                 bool accumulate)
{
    if( accumulate )
    {
        PLASSERT_MSG( input_diag_hessian.size() == input_size,
                      "Cannot resize input_diag_hessian AND accumulate into it"
                    );
    }
    else
    {
        input_diag_hessian.resize( input_size );
        input_diag_hessian.clear();
    }

    // input_diag_hessian[ i ] = 0 if i!=t
    // input_diag_hessian[ t ] = 1/(x[t])^2
    int the_target = (int) round( target[0] );
    real input_t = input[ the_target ];
    input_diag_hessian[ the_target ] += 1. / (input_t * input_t);

    bpropUpdate( input, target, cost, input_gradient, accumulate );
}

TVec<string> NLLCostModule::name()
{
    return TVec<string>(1, "NLL");
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
