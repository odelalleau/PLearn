// -*- C++ -*-

// CrossEntropyCostModule.cc
//
// Copyright (C) 2007 Pascal Lamblin, Dumitru Erhan and Hugo Larochelle
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

// Authors: Pascal Lamblin, Dumitru Erhan and Hugo Larochelle

/*! \file CrossEntropyCostModule.cc */



#include "CrossEntropyCostModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(CrossEntropyCostModule,
    "Computes the CrossEntropy, given two activation vectors",
    "and back-propagates the gradient\n");

CrossEntropyCostModule::CrossEntropyCostModule()
{
    output_size = 1;
}

void CrossEntropyCostModule::declareOptions(OptionList& ol)
{
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "target_size", &CrossEntropyCostModule::target_size,
                     OptionBase::nosave,
                     "equals to input_size");

}

void CrossEntropyCostModule::build_()
{
    target_size = input_size;
}

// ### Nothing to add here, simply calls build_
void CrossEntropyCostModule::build()
{
    inherited::build();
    build_();
}


void CrossEntropyCostModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


///////////
// fprop //
///////////
void CrossEntropyCostModule::fprop(const Vec& input, const Vec& target, real& cost) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    cost = 0;

    real target_i, activation_i;
    for( int i=0 ; i < target_size ; i++ )
    {
        // nll = - target*log(sigmoid(act)) -(1-target)*log(1-sigmoid(act))
        // but it is numerically unstable, so use instead the following:
        //     = target*softplus(-act) +(1-target)*(act+softplus(-act))
        //     = act + softplus(-act) - target*act
        //     = softplus(act) - target*act
        target_i = target[i];
        activation_i = input[i];
        cost += softplus(activation_i) - target_i * activation_i;
    }
}

void CrossEntropyCostModule::fprop(const Vec& input, const Vec& target, Vec& cost) const
{
    cost.resize( output_size );
    fprop( input, target, cost[0] );
}

void CrossEntropyCostModule::fprop(const Mat& inputs, const Mat& targets, Mat& costs) const
{
    costs.resize( inputs.length(), output_size );

    for (int i = 0; i < inputs.length(); i++)
        fprop(inputs(i), targets(i), costs(i,0));

}



/////////////////
// bpropUpdate //
/////////////////
void CrossEntropyCostModule::bpropUpdate(const Vec& input, const Vec& target,
                                         real cost, Vec& input_gradient,
                                         bool accumulate)
{
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    if (accumulate)
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
    {
        input_gradient.resize(input_size);
        input_gradient.clear();
    }

    for (int i=0; i < target_size; i++)
        input_gradient[i] += target[i] - sigmoid(-input[i]);
}

void CrossEntropyCostModule::bpropUpdate(const Mat& inputs, const Mat& targets,
                                         const Vec& costs,
                                         Mat& input_gradients, bool accumulate)
{
    PLASSERT( inputs.width() == input_size );
    PLASSERT( targets.width() == target_size );

    int batch_size = inputs.length();

    if (accumulate)
    {
        PLASSERT_MSG( input_gradients.width() == input_size &&
                      input_gradients.length() == batch_size,
                      "Cannot resize input_gradients AND accumulate into it" );
    }
    else
    {
        input_gradients.resize(batch_size, input_size);
        input_gradients.clear();
    }

    for (int i=0; i < batch_size; i++)
        for (int j=0; j < target_size; j++)
            input_gradients(i, j) += targets(i, j) - sigmoid(-inputs(i, j));
}

void CrossEntropyCostModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                   const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_value.length() == nPorts() );
    PLASSERT( ports_gradient.length() == nPorts() );

    Mat* prediction = ports_value[0];
    Mat* target = ports_value[1];
#ifndef NDEBUG
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

        PLASSERT( prediction->width() == getPortSizes()(0,1) );
        PLASSERT( target->width() == getPortSizes()(1,1) );
        PLASSERT( cost->width() == getPortSizes()(2,1) );
        PLASSERT( prediction_grad->width() == getPortSizes()(0,1) );
        PLASSERT( cost_grad->width() == getPortSizes()(2,1) );

        int batch_size = prediction->length();
        PLASSERT( target->length() == batch_size );
        PLASSERT( cost->length() == batch_size );
        PLASSERT( cost_grad->length() == batch_size );

        prediction_grad->resize(batch_size, getPortSizes()(0,1));

        for( int i=0; i < batch_size; i++ )
            for ( int j=0; j < target->width(); j++ )
                (*prediction_grad)(i, j) +=
                (*cost_grad)(i,0)*((*target)(i,j) - sigmoid(-(*prediction)(i,j) ));
    }

    else if( !prediction_grad && !target_grad &&
               (!cost_grad || !cost_grad->isEmpty()) )
        // We do not care about the gradient w.r.t prediction and target, and
        // either we do not care about the gradient w.r.t. cost or there is a
        // gradient provided (that we will not use).
        // In such situations, there is nothing to do.
        return;
    else if( !cost_grad && prediction_grad && prediction_grad->isEmpty() )
        PLERROR("In CrossEntropyCostModule::bpropAccUpdate - cost gradient is NULL,\n"
                "cannot compute prediction gradient. Maybe you should set\n"
                "\"propagate_gradient = 0\" on the incoming connection.\n");
    else
        PLERROR("In OnlineLearningModule::bpropAccUpdate - Port configuration "
                "not implemented for class '%s'", classname().c_str());
}

TVec<string> CrossEntropyCostModule::costNames()
{
    if (name == "" || name == classname())
        return TVec<string>(1, "CrossEntropy");
    else
        return TVec<string>(1, name + ".CrossEntropy");
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
