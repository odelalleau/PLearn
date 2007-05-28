// -*- C++ -*-

// LinearCombinationModule.cc
//
// Copyright (C) 2007 Yoshua Bengio
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

// Authors: Yoshua Bengio

/*! \file LinearCombinationModule.cc */



#include "LinearCombinationModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    LinearCombinationModule,
    "Outputs a linear combination of the input ports\n",
    "   output = sum_i weights[i] input_i\n"
    "where input_i is the provided matrix for the i-th input port\n"
    "and output is the resulting matrix for the output port.\n"
    "Hence all the ports should have the same dimensions.\n"
    "The weights of the linear combination could either be learned or user-defined.\n"
    "The input ports are names 'in_1', 'in_2', ... and the output port is named 'output'.\n"
    );

LinearCombinationModule::LinearCombinationModule()
    : adaptive(false), learning_rate(0)
{
}

void LinearCombinationModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "weights", &LinearCombinationModule::weights,
                  OptionBase::buildoption,
                  "the weights of the linear combination: a vector with one element per input port\n");

    declareOption(ol, "adaptive", &LinearCombinationModule::adaptive,
                  OptionBase::buildoption,
                  "whether to adapt the weights, if true they are cleared upon initialization (forget()).\n");

    declareOption(ol, "learning_rate", &LinearCombinationModule::learning_rate,
                  OptionBase::buildoption,
                  "Learning rate to adapt the weights by online gradient descent.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void LinearCombinationModule::build_()
{
    PLASSERT(weights.length()==0 || port_names.length()==0 ||
             weights.length() + 1 ==port_names.length());
    int n_ports=0;
    if (weights.length()!=0 && port_names.length()==0)
        // weights provided but not ports: give default port names
    {
        n_ports = weights.length() + 1;
        port_names.resize(n_ports);
        for (int i=0;i<n_ports-1;i++)
            port_names[i]="in_" + tostring(i+1);
        port_names[n_ports-1]="output";
    }
    if (weights.length()==0 && port_names.length()!=0)
        // ports provided but not weights: initialize weights to 0
    {
        n_ports = port_names.length();
        weights.resize(n_ports - 1);
        weights.fill(0);
        if (!adaptive)
            PLWARNING("LinearCombinationModule::build: non-adaptive weights set to 0! the module will always output 0.");
    }

    PLCHECK( learning_rate >= 0 );
}

///////////
// build //
///////////
void LinearCombinationModule::build()
{
    inherited::build();
    build_();
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LinearCombinationModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(weights,    copies);
    deepCopyField(port_names, copies);
}

///////////
// fprop //
///////////
void LinearCombinationModule::fprop(const TVec<Mat*>& ports_value)
{
    int n_ports = weights.length() + 1;
    if ( n_ports < 2 )
        // has build completed? there should be at least one input port + the output port
        PLERROR("LinearCombinationModule should have at least 2 ports (one input port and one output port)\n");
    PLASSERT( ports_value.length() == n_ports ); // is the input coherent with expected nPorts
    
    const TVec<Mat*>& inputs = ports_value;
    Mat* output = ports_value[n_ports-1];
    if (output) {
        PLASSERT( output->isEmpty() );
        PLASSERT( inputs[0] );
        int mbs = inputs[0]->length();
        int width = inputs[0]->width();
        output->resize(mbs, width);
        output->clear();
        for (int i=0;i<n_ports-1;i++) {
            Mat* input_i = inputs[i];
            if (!input_i || input_i->isEmpty())
                PLERROR("In LinearCombinationModule::fprop - The %d-th input "
                        "port is missing or empty", i);
            multiplyAcc(*output, *input_i, weights[i]);
        }
    }

    // Ensure all required ports have been computed.
    checkProp(ports_value);
}

////////////////////
// bpropAccUpdate //
////////////////////
void LinearCombinationModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                             const TVec<Mat*>& ports_gradient)
{
    int n_ports = weights.length() + 1;
    PLASSERT( ports_value.length() == n_ports && ports_gradient.length() == n_ports);

    const TVec<Mat*>& input_grad = ports_gradient;
    Mat* output_grad = ports_gradient[n_ports-1];
    if (output_grad && !output_grad->isEmpty())
    {
        int mbs = output_grad->length();
        int width = output_grad->width();
        for (int i=0;i<n_ports-1;i++)
        {
            if (input_grad[i])
            {
                PLASSERT(input_grad[i]->isEmpty() &&
                         input_grad[i]->width() == width);
                input_grad[i]->resize(mbs,width);
                multiplyAcc(*input_grad[i],*output_grad,weights[i]);
            }
            if (adaptive && learning_rate > 0)
            {
                Mat* input_i = ports_value[i];
                PLASSERT(input_i);
                weights[i] -= learning_rate * dot(*output_grad,*input_i);
            }
        }
    }

    // Ensure all required gradients have been computed.
    checkProp(ports_gradient);
}

////////////
// forget //
////////////
void LinearCombinationModule::forget()
{
    if (adaptive)
        weights.clear();
}

//////////////
// finalize //
//////////////
/* THIS METHOD IS OPTIONAL
void LinearCombinationModule::finalize()
{
}
*/

//////////////////////
// bpropDoesNothing //
//////////////////////
/* THIS METHOD IS OPTIONAL
// the default implementation returns false
bool LinearCombinationModule::bpropDoesNothing()
{
}
*/

/////////////////////
// setLearningRate //
/////////////////////
/* OPTIONAL
// The default implementation raises a warning and does not do anything.
void LinearCombinationModule::setLearningRate(real dynamic_learning_rate)
{
}
*/

//////////////
// getPorts //
//////////////
const TVec<string>& LinearCombinationModule::getPorts() {
    return port_names;
}

//////////////////
// getPortSizes //
//////////////////
/* Optional
const TMat<int>& LinearCombinationModule::getPortSizes() {
}
*/

}
// end of namespace PLearn


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
