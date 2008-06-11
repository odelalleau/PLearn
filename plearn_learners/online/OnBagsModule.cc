// -*- C++ -*-

// OnBagsModule.cc
//
// Copyright (C) 2008 Jerome Louradour
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

// Author: Jerome Louradour

/*! \file OnBagsModule.cc */



#include "OnBagsModule.h"
#include <plearn/var/SumOverBagsVariable.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    OnBagsModule,
    "Module that process (variable-length) bags of vectors.\n",
    "One output vector is propagated at the end of the bag. Other outputs are MISSING_VALUE.\n"
    "The 'bagtarget' contains the bag information at the last position, has follows\n"
    "     bag info = 1. (01) beginning of bag\n"
    "              = 0. (00) middle of bag\n"
    "              = 2. (10) end of bag\n"
    "              = 3. (11) single-row bag (both beginning and end)\n");

OnBagsModule::OnBagsModule():
    bagtarget_size(1)
{
}

void OnBagsModule::declareOptions(OptionList& ol)
{
    inherited::declareOptions(ol);

    declareOption(ol, "bagtarget_size", &OnBagsModule::bagtarget_size,
                  OptionBase::buildoption,
                  "Size of the 'bagtarget' port (bag info is the last element).");
}

void OnBagsModule::build_()
{
    PLASSERT( bagtarget_size > 0 );
    PLASSERT( input_size > 0 );

    // The port story...
    ports.resize(0);
    portname_to_index.clear();
    addPortName("input");
    addPortName("output");
    addPortName("bagtarget");

    port_sizes.resize(nPorts(), 3);
    port_sizes.fill(-1);
    port_sizes(getPortIndex("input"), 1) = input_size;
    port_sizes(getPortIndex("output"), 1) = output_size;
    port_sizes(getPortIndex("bagtarget"), 1) = bagtarget_size;
}

void OnBagsModule::build()
{
    inherited::build();
    build_();
}

void OnBagsModule::forget()
{
    wait_new_bag = true;
}

void OnBagsModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(ports, copies);
}


///////////
// fprop //
///////////


void OnBagsModule::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == nPorts() );

    Mat* p_inputs = ports_value[getPortIndex("input")];
    Mat* p_bagtargets = ports_value[getPortIndex("bagtarget")];
    Mat* p_outputs = ports_value[getPortIndex("output")];

    if ( p_outputs && p_outputs->isEmpty() )
    {
        PLASSERT( p_inputs && !p_inputs->isEmpty() );
        PLASSERT( p_bagtargets && !p_bagtargets->isEmpty() );
        fprop(*p_inputs, *p_bagtargets, *p_outputs);
        checkProp(ports_value);
    }
    else
        PLERROR("In OnBagsModule::fprop - Port configuration not implemented.");
}

void OnBagsModule::fprop(const Mat& inputs, const Mat& bagtargets, Mat& outputs)
{
    int n_samples = inputs.length();
    outputs.resize( n_samples, output_size );
    //outputs.fill( MISSING_VALUE );
    for (int isample = 0; isample < n_samples; isample++)
    {
        Vec output;
        fprop( inputs(isample), bagtargets(isample), output );
        outputs.row(isample) << output;
    }
}

void OnBagsModule::fprop(const Vec& input, const Vec& bagtarget, Vec& output)
{
    PLASSERT( input.length() == input_size );
    PLASSERT( bagtarget.length() == bagtarget_size );
    output.resize( output_size );
    output.fill( MISSING_VALUE );
    int bag_info = int(round(bagtarget.lastElement()));
    if (bag_info & SumOverBagsVariable::TARGET_COLUMN_FIRST)
    {
        PLASSERT( wait_new_bag );
        fpropInit( input );
        wait_new_bag = false;
    }
    else
    {
        PLASSERT( !wait_new_bag );
        fpropAcc( input );
    }
    if (bag_info & SumOverBagsVariable::TARGET_COLUMN_LAST)
    {
        fpropOutput( output );
        wait_new_bag = true;
    }
}

// This function initializes internal statistics
// for the first sample of a bag
void OnBagsModule::fpropInit(const Vec& input)
{
    PLERROR("In OnBagsModule::fpropInit(input,output) - not implemented for class %s",classname().c_str());
}

// This function updates internal statistics given a new bag sample
void OnBagsModule::fpropAcc(const Vec& input)
{
    PLERROR("In OnBagsModule::fpropAcc(input,output) - not implemented for class %s",classname().c_str());
}

// This function fills the output from the internal statistics
// (once all the bag has been seen)
void OnBagsModule::fpropOutput(Vec& output)
{
    PLERROR("In OnBagsModule::fpropOutput(input,output) - not implemented for class %s",classname().c_str());
}

/////////////////
// bpropUpdate //
/////////////////

void OnBagsModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                  const TVec<Mat*>& ports_gradient)
{
    PLASSERT( input_size > 1 );
    PLASSERT( ports_value.length() == nPorts() );
    PLASSERT( ports_gradient.length() == nPorts() );

    const Mat* p_inputs = ports_value[getPortIndex("input")];
    const Mat* p_bagtargets = ports_value[getPortIndex("bagtarget")];
    const Mat* p_output_grad = ports_gradient[getPortIndex("output")];
    Mat* p_inputs_grad = ports_gradient[getPortIndex("input")];

    if( p_inputs_grad
        && p_output_grad && !p_output_grad->isEmpty() )
    {
        PLASSERT( p_inputs && !p_inputs->isEmpty());
        PLASSERT( p_bagtargets && !p_bagtargets->isEmpty());
        int n_samples = p_inputs->length();
        PLASSERT( p_output_grad->length() == n_samples );
        PLASSERT( p_output_grad->width() == output_size );
        if( p_inputs_grad->isEmpty() )
        {
            p_inputs_grad->resize( n_samples, input_size);
            p_inputs_grad->clear();
        }
        bpropUpdate( *p_inputs, *p_bagtargets,
                     *p_inputs_grad, *p_output_grad, true );
        checkProp(ports_gradient);
    }
    else if( !p_inputs_grad && !p_output_grad )
        return;
    else
        PLERROR("In OnBagsModule::bpropAccUpdate - Port configuration not implemented.");

}

void OnBagsModule::bpropUpdate( const Mat& inputs,
                                const Mat& bagtargets,
                                Mat& input_gradients,
                                const Mat& output_gradients,
                                bool accumulate)
{
    PLASSERT( inputs.width() == input_size );
    PLASSERT( bagtargets.width() == bagtarget_size );
    PLASSERT( output_gradients.width() == output_size );
    int n_samples = inputs.length();
    PLASSERT( n_samples == bagtargets.length() );
    PLASSERT( n_samples == output_gradients.length() );
    if( accumulate )
        PLASSERT_MSG( input_gradients.width() == input_size &&
                input_gradients.length() == n_samples,
                "Cannot resize input_gradients and accumulate into it." );
    else
    {
        input_gradients.resize(inputs.length(), input_size);
        input_gradients.fill(0);
    }
    bool bprop_wait_new_bag = true;
    int bag_start;
    for (int j = 0; j < inputs.length(); j++) {
        int bag_info = int(round(bagtargets(j).lastElement()));
        if (bag_info & SumOverBagsVariable::TARGET_COLUMN_FIRST)
        {
            PLASSERT( bprop_wait_new_bag );
            bprop_wait_new_bag = false;
            bag_start = j;
        }
        else
            PLASSERT( !bprop_wait_new_bag );
        if (bag_info & SumOverBagsVariable::TARGET_COLUMN_LAST)
        {
            PLASSERT( !is_missing(output_gradients(j,0)) );
            bprop_wait_new_bag = true;
            int bag_length = j - bag_start + 1;
            Mat baginputs_gradients;
            bprop( inputs.subMatRows(bag_start, bag_length),
                   output_gradients(j),
                   baginputs_gradients );
            input_gradients.subMatRows(bag_start, bag_length) << baginputs_gradients;
        }
        else
            PLASSERT( is_missing(output_gradients(j,0)) );
    }

    if (!bprop_wait_new_bag)
        PLERROR("In OnBagsModule::bpropUpdate - entire bags must be given."
                "If you use ModuleLearner, you should use the option operate_on_bags = True");
}

// This function computes the inputs gradients,
// given all inputs from a same bag,
//   and the gradient on the (single) output of the bag.
void OnBagsModule::bprop( const Mat& baginputs,
                          const Vec& bagoutput_gradient,
                          Mat& baginputs_gradients)
{
    PLERROR("In OnBagsModule::bprop() - not implemented for class %s",classname().c_str() );
}


//////////
// name //
//////////
TVec<string> OnBagsModule::name()
{
    return TVec<string>(1, OnlineLearningModule::name);
}

/////////////////
// addPortName //
/////////////////
void OnBagsModule::addPortName(const string& name)
{
    PLASSERT( portname_to_index.find(name) == portname_to_index.end() );
    portname_to_index[name] = ports.length();
    ports.append(name);
}

//////////////
// getPorts //
//////////////
const TVec<string>& OnBagsModule::getPorts()
{
    return ports;
}

///////////////////
// getPortsSizes //
///////////////////
const TMat<int>& OnBagsModule::getPortSizes()
{
    return port_sizes;
}

//////////////////
// getPortIndex //
//////////////////
int OnBagsModule::getPortIndex(const string& port)
{
    map<string, int>::const_iterator it = portname_to_index.find(port);
    if (it == portname_to_index.end())
        return -1;
    else
        return it->second;
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
