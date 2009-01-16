// -*- C++ -*-

// SoftmaxNLLCostModule.cc
//
// Copyright (C) 2008 Pascal Lamblin
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

/*! \file SoftmaxNLLCostModule.cc */



#include "SoftmaxNLLCostModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    SoftmaxNLLCostModule,
    "Like SoftmaxModule and NLLCostModule, with more precision.",
    "If target is the index of the true class, this module computes\n"
    "    cost = -log( softmax(input)[target] ),\n"
    "and back-propagates the gradient and diagonal of Hessian.\n");

SoftmaxNLLCostModule::SoftmaxNLLCostModule()
{
    output_size = 1;
    target_size = 1;
}

void SoftmaxNLLCostModule::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &SoftmaxNLLCostModule::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void SoftmaxNLLCostModule::build_()
{
}

// ### Nothing to add here, simply calls build_
void SoftmaxNLLCostModule::build()
{
    inherited::build();
    build_();
}


void SoftmaxNLLCostModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


///////////
// fprop //
///////////
void SoftmaxNLLCostModule::fprop(const Vec& input, const Vec& target, Vec& cost) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    tmp_vec.resize(input_size);
    cost.resize(output_size);

    if (input.hasMissing())
        cost[0] = MISSING_VALUE;
    else
    {
        int the_target = (int) round( target[0] );
        log_softmax(input, tmp_vec);
        cost[0] = - tmp_vec[the_target];
    }
}

void SoftmaxNLLCostModule::fprop(const Mat& inputs, const Mat& targets, Mat& costs)
    const
{
    PLASSERT( inputs.width() == input_size );
    PLASSERT( targets.width() == target_size );

    int batch_size = inputs.length();
    PLASSERT( inputs.length() == batch_size );
    PLASSERT( targets.length() == batch_size );

    tmp_vec.resize(input_size);
    costs.resize(batch_size, output_size);

    for( int k=0; k<batch_size; k++ )
    {
        if (inputs(k).hasMissing())
            costs(k, 0) = MISSING_VALUE;
        else
        {
            int target_k = (int) round( targets(k, 0) );
            log_softmax(inputs(k), tmp_vec);
            costs(k, 0) = - tmp_vec[target_k];
        }
    }
}

void SoftmaxNLLCostModule::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == nPorts() );

    Mat* prediction = ports_value[0];
    Mat* target = ports_value[1];
    Mat* cost = ports_value[2];

    // If we have prediction and target, and we want cost
    if( prediction && !prediction->isEmpty()
        && target && !target->isEmpty()
        && cost && cost->isEmpty() )

    {
        PLASSERT( prediction->width() == port_sizes(0, 1) );
        PLASSERT( target->width() == port_sizes(1, 1) );

        int batch_size = prediction->length();
        PLASSERT( target->length() == batch_size );

        cost->resize(batch_size, port_sizes(2, 1));


        for( int i=0; i<batch_size; i++ )
        {
            if( (*prediction)(i).hasMissing() || is_missing((*target)(i,0)) )
                (*cost)(i,0) = MISSING_VALUE;
            else
            {
                int target_i = (int) round( (*target)(i,0) );
                PLASSERT( is_equal( (*target)(i, 0), target_i ) );
                log_softmax( (*prediction)(i), tmp_vec );
                (*cost)(i,0) = - tmp_vec[target_i];
            }
        }
    }
    else if( !prediction && !target && !cost )
        return;
    else
        PLCHECK_MSG( false, "Unknown port configuration" );

    checkProp(ports_value);
}

/////////////////
// bpropUpdate //
/////////////////
void SoftmaxNLLCostModule::bpropUpdate(
        const Vec& input, const Vec& target, real cost,
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

    // input_gradient[ i ] = softmax(x)[i] if i != t,
    // input_gradient[ t ] = softmax(x)[t] - 1.
    softmax(input, input_gradient);
    input_gradient[ the_target ] -= 1.;
}

void SoftmaxNLLCostModule::bpropUpdate(
        const Mat& inputs, const Mat& targets, const Vec& costs,
        Mat& input_gradients, bool accumulate)
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

    // input_gradient[ i ] = softmax(x)[i] if i != t,
    // input_gradient[ t ] = softmax(x)[t] - 1.
    for (int i = 0; i < inputs.length(); i++) {
        int the_target = (int) round( targets(i, 0) );
        softmax(inputs(i), input_gradients(i));
        input_gradients(i, the_target) -= 1.;
    }
}

void SoftmaxNLLCostModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                          const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_value.length() == nPorts() );
    PLASSERT( ports_gradient.length() == nPorts() );

    Mat* prediction = ports_value[0];
    Mat* target = ports_value[1];
#ifdef BOUNDCHECK
    Mat* cost = ports_value[2];
#endif
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
        PLASSERT( cost_grad->width() == 1 );

        int batch_size = prediction->length();
        PLASSERT( target->length() == batch_size );
        PLASSERT( cost->length() == batch_size );
        PLASSERT( cost_grad->length() == batch_size );

        prediction_grad->resize(batch_size, port_sizes(0,1));

        for( int k=0; k<batch_size; k++ )
        {
            // input_gradient[ i ] = softmax(x)[i] if i != t,
            // input_gradient[ t ] = softmax(x)[t] - 1.
            int target_k = (int) round((*target)(k, 0));
            softmax((*prediction)(k), (*prediction_grad)(k));
            (*prediction_grad)(k, target_k) -= 1.;
        }
    }
    else if( !prediction_grad && !target_grad && !cost_grad )
        return;
    else if( !cost_grad && prediction_grad && prediction_grad->isEmpty() )
        PLERROR("In SoftmaxNLLCostModule::bpropAccUpdate - cost gradient is NULL,\n"
                "cannot compute prediction gradient. Maybe you should set\n"
                "\"propagate_gradient = 0\" on the incoming connection.\n");
    else
        PLERROR("In OnlineLearningModule::bpropAccUpdate - Port configuration "
                "not implemented for class '%s'", classname().c_str());

    checkProp(ports_value);
    checkProp(ports_gradient);
}

void SoftmaxNLLCostModule::bbpropUpdate(const Vec& input, const Vec& target,
                                 real cost,
                                 Vec& input_gradient, Vec& input_diag_hessian,
                                 bool accumulate)
{
    PLCHECK(false);
}

TVec<string> SoftmaxNLLCostModule::costNames()
{
    if (name == "" || name == classname())
        return TVec<string>(1, "NLL");
    else
        return TVec<string>(1, name + ".NLL");
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
