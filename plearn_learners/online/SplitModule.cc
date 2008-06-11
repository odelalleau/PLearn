// -*- C++ -*-

// SplitModule.cc
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

/*! \file SplitModule.cc */



#include "SplitModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    SplitModule,
    "Split a single down-port into multiple up-ports or concatenate the up-ports into the down-port",
    "There is a single down-port and multiple up-ports.\n"
    "After an fprop, we always have that the down-port\n"
    "is the horizontal concatenation of the up-ports.\n"
    "When the down-port is input and the up-ports are output\n"
    "the module SPLITS the input into its output. When the\n"
    "up-ports are input and the down-port is output, the module\n"
    "CONCATENATES the up-ports into the down-port.\n"
    "The user specifies names and sizes of each port.\n"
    "The port values are expected to be matrices of the\n"
    "same length. The splitting or concatenation operations\n"
    "can be seen as performed on each row of these matrices.\n"
    );

SplitModule::SplitModule()
    : down_port_name("down_port")
/* ### Initialize all fields to their default value here */
{
}

void SplitModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "down_port_name", &SplitModule::down_port_name,
                  OptionBase::buildoption,
                  "Name of the down-port. Default = 'down_port'.");

    declareOption(ol, "up_port_sizes", &SplitModule::up_port_sizes,
                  OptionBase::buildoption,
                  "Sizes of the up-ports. Mandatory.\n");

    declareOption(ol, "up_port_names", &SplitModule::up_port_names,
                  OptionBase::buildoption,
                  "Names of the up-ports. Default = ['up_port_1','up_port_2',...].");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "input_size", &SplitModule::input_size,
                    OptionBase::learntoption,
                    "input_size = down_port's size = sum(up_ports_size) but is set automatically.");

    redeclareOption(ol, "output_size", &SplitModule::output_size,
                    OptionBase::learntoption,
                    "output_size = sum(up_ports_size) but is set automatically.");
}

void SplitModule::build_()
{
    int n_up_ports = up_port_sizes.length();
    if (n_up_ports>0) // build cannot be completed until then
    {
        if (up_port_names.length()==0) // set default up_port_names
        {
            up_port_names.resize(n_up_ports);
            for (int i=0;i<n_up_ports;i++)
                up_port_names[i] = "up_port_" + tostring(i+1);
        }
        input_size = output_size = sum(up_port_sizes);
    }
}

// ### Nothing to add here, simply calls build_
void SplitModule::build()
{
    inherited::build();
    build_();
}


void SplitModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(up_port_sizes, copies);
    deepCopyField(up_port_names, copies);
}

///////////
// fprop //
///////////
void SplitModule::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == nPorts() );
    PLASSERT_MSG( up_port_sizes.length()>0, "SplitModule should have up_port_sizes specified with length>0");
    PLASSERT_MSG( ports_value[0], "SplitModule's down_port should always be connected.\n");
    if (!ports_value[0]->isEmpty()) // the down_port is an input, do a SPLIT
    {
        int start=0;
        Mat& input = *ports_value[0];
        for (int i=0;i<up_port_sizes.length();i++)
        {
            int width = up_port_sizes[i];
            if (ports_value[i+1])
            {
                PLASSERT_MSG(ports_value[i+1]->isEmpty(),"In SplitModule, when the down_port is an input, the up_ports should either be outputs or not connected.\n");
                ports_value[i+1]->resize(input.length(),width);
                *ports_value[i+1] << input.subMatColumns(start,width);
            }
            start += width;
        }
        return;
    } else // the down_port is an output, do a CONCATENATE
    {
        int start=0;
        Mat& output = *ports_value[0];
        // hacky check so that the resize does not fail
        PLASSERT_MSG(ports_value[1] && !ports_value[1]->isEmpty(), "In SplitModule, when the down_port is an output, all up_ports should be connected.\n");
        output.resize(ports_value[1]->length(),input_size);
        for (int i=0;i<up_port_sizes.length();i++)
        {
            int width = up_port_sizes[i];
            PLASSERT_MSG(ports_value[i+1] && !ports_value[i+1]->isEmpty(), "In SplitModule, when the down_port is an output, all up_ports should be connected.\n");
            output.subMatColumns(start,width) << *ports_value[i+1];
            start += width;
        }
        return;
    }
    PLERROR("In SplitModule::fprop - Not implemented for class "
            "'%s'", classname().c_str());
}

/////////////////
// bpropUpdate //
/////////////////


void SplitModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                 const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_value.length() == nPorts() && ports_gradient.length() == nPorts());
    PLASSERT_MSG( up_port_sizes.length()>0, "SplitModule should have up_port_sizes specified with length>0");
    if (ports_gradient[0] && ports_gradient[0]->isEmpty()) // the down_port is an input, do a backprop on SPLIT
    {
        int start=0;
        Mat& input_gradient = *ports_gradient[0];
        for (int i=0;i<up_port_sizes.length();i++)
        {
            int width = up_port_sizes[i];
            PLASSERT_MSG( ports_gradient[i+1] &&
                          !ports_gradient[i+1]->isEmpty(),
                          "In SplitModule::bpropAccUpdate - When the down_port"
                          " is an input and its gradient is required, one must"
                          " provide a gradient on the up_ports" );
            input_gradient.resize(ports_gradient[i+1]->length(),
                                  input_gradient.width());
            input_gradient.subMatColumns(start, width) += *ports_gradient[i+1];
            start += width;
        }
        return;
    }
    bool one_of_the_up_ports_wants_a_gradient = false;
    for (int i=0;i<up_port_sizes.length();i++)
        if (ports_gradient[i+1] && ports_gradient[i+1]->isEmpty())
        {
            one_of_the_up_ports_wants_a_gradient = true;
            break;
        }
    if (one_of_the_up_ports_wants_a_gradient)
    // the down_port is an output, do a CONCATENATE
    {
        int start=0;
        Mat& output_gradient = *ports_gradient[0];

        for (int i=0;i<up_port_sizes.length();i++)
        {
            int width = up_port_sizes[i];
            if (ports_gradient[i+1] && ports_gradient[i+1]->isEmpty())
            {
                ports_gradient[i+1]->resize(output_gradient.length(),width);
                (*ports_gradient[i+1]) += output_gradient.subMatColumns(start,width);
            }
            start += width;
        }
        return;
    }
    //PLERROR("In SplitModule::fprop - This configuration of ports not implemented for class "
    //       "'%s'", classname().c_str());
}

//////////////
// getPorts //
//////////////
const TVec<string>& SplitModule::getPorts() {
    if (port_names.isEmpty())
    {
        port_names.resize(nPorts());
        port_names[0] = down_port_name;
        port_names.subVec(1,nPorts()-1) << up_port_names;
    }
    return port_names;
}

//////////////////
// getPortSizes //
//////////////////
const TMat<int>& SplitModule::getPortSizes() {
    if (sizes.isEmpty())
    {
        sizes.resize(nPorts(),2);
        sizes.column(0).fill(-1);
        sizes(0,1) = input_size;
        sizes.subMat(1,1,nPorts()-1,1) << up_port_sizes.toMat(nPorts()-1,1);
    }
    return sizes;
}

////////////
// nPorts //
////////////
int SplitModule::nPorts()
{
    return 1+up_port_sizes.length();
}


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
