// -*- C++ -*-

// OnlineLearningModule.cc
//
// Copyright (C) 2005 Yoshua Bengio
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

/* *******************************************************
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file OnlineLearningModule.cc */


#include "OnlineLearningModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    OnlineLearningModule,
    "Learn to map inputs to outputs, online, using caller-provided gradients.",
    "This pure virtual class (i.e. an interface) can basically do two things:\n"
    "  * map an input to an output\n"
    "  * modify itself when told in what direction the output should have \n"
    "    changed (i.e. output gradient),  while optionally giving back the \n"
    "    information about how the input should also have changed \n"
    "    (i.e. input gradient)\n"
    );

//////////////////////////
// OnlineLearningModule //
//////////////////////////
OnlineLearningModule::OnlineLearningModule(const string& the_name,
                                           bool call_build_):
    inherited(call_build_),
    input_size(-1),
    output_size(-1),
    name(the_name),
    estimate_simpler_diag_hessian(false)
{
    if (call_build_) {
        if (the_name.empty())
            PLERROR("In OnlineLearningModule::OnlineLearningModule - You "
                    "cannot create a new OnlineLearningModule with an empty "
                    "name and call build within the constructor itself");
        build_();
    }
}

///////////
// fprop //
///////////
void OnlineLearningModule::fprop(const Mat& inputs, Mat& outputs)
{
    PLERROR("In OnlineLearningModule::fprop - The mini-batch version of "
            "'fprop' for class '%s' is not implemented. Implementation is "
            "required out of safety, to ensure a subsequent call to "
            "'bpropUpdate' can use the correctly updated data",
            classname().c_str());
}

void OnlineLearningModule::fprop(const Vec& inputs, Vec& outputs) const
{
    PLERROR("In OnlineLearningModule::fprop - This variant is deprecated, use fprop(ports_value)\n");
}

void OnlineLearningModule::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == nPorts() );
    if (ports_value.length() == 2)
    {
        Mat* m1 = ports_value[0];
        Mat* m2 = ports_value[1];
        if (m1 && m2 && !m1->isEmpty() && m2->isEmpty()) {
            // We can re-use previous code for standard mini-batch fprop.
            fprop(*m1, *m2);
            checkProp(ports_value);
            return;
        }
    }
    PLERROR("In OnlineLearningModule::fprop - Port configuration not "
            "implemented for class '%s'", classname().c_str());
}

////////////////////
// bpropAccUpdate //
////////////////////
void OnlineLearningModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                          const TVec<Mat*>& ports_gradient)
{
    if (ports_gradient.length() == 2) {
        Mat* input_grad = ports_gradient[0];
        Mat* output_grad = ports_gradient[1];
        if (output_grad && !output_grad->isEmpty() &&
             (!input_grad || input_grad->isEmpty()))
        {
            // We can try to re-use the standard mini-batch bpropUpdate method.
            if (!input_grad) {
                // We are not interested in the input gradient: use a dummy
                // matrix to store it.
                input_grad = &tmpm_input_gradient;
            }
            Mat* input_val = ports_value[0];
            Mat* output_val = ports_value[1];
            PLASSERT( input_val && output_val );
            input_grad->resize(input_val->length(), input_val->width());
            bpropUpdate(*input_val, *output_val, *input_grad, *output_grad,
                        true);
            checkProp(ports_gradient);
            return;
        }
    }
    PLERROR("In OnlineLearningModule::bpropAccUpdate - Port configuration "
            "not implemented for class '%s'", classname().c_str());
}


/////////////////
// bpropUpdate //
/////////////////
void OnlineLearningModule::bpropUpdate(const Vec& input, const Vec& output,
                                       Vec& input_gradient,
                                       const Vec& output_gradient,
                                       bool accumulate)
{
    PLERROR("In OnlineLearningModule.cc: method 'bpropUpdate' not"
            " implemented.\n"
            "Please implement it in your derived class (%s) or do not call"
            " bpropUpdate.", classname().c_str());
}

void OnlineLearningModule::bpropUpdate(const Vec& input, const Vec& output,
                                       const Vec& output_gradient)
{
    bpropUpdate(input, output, tmp_input_gradient, output_gradient);
}

void OnlineLearningModule::bpropUpdate(const Mat& inputs, const Mat& outputs,
                                       Mat& input_gradients,
                                       const Mat& output_gradients,
                                       bool accumulate)
{
    PLERROR("In OnlineLearningModule::bpropUpdate - The mini-batch version of "
            "'bpropUpdate' for class '%s' is not implemented. Implementation "
            "is required since this method must be called immediately after "
            "a 'fprop'", classname().c_str());
}

void OnlineLearningModule::bpropUpdate(const Mat& inputs, const Mat& outputs,
                                       const Mat& output_gradients)
{
    bpropUpdate(inputs, outputs, tmpm_input_gradient, output_gradients);
}

void OnlineLearningModule::bpropUpdate(const TVec<Mat*>& ports_value,
                                       const TVec<Mat*>& ports_gradient)
{
    for (int i = 0; i < ports_gradient.length(); i++) {
        Mat* grad = ports_gradient[i];
        if (grad && grad->isEmpty()) {
            // This gradient must be computed (= cleared + accumulated).
            Mat* val = ports_value[i];
            if (!val)
                PLERROR("In OnlineLearningModule::bpropUpdate - Cannot compute"
                        " the gradient of a port whose value is not available,"
                        " since we cannot easily know its size");
            grad->resize(val->length(), val->width());
            grad->fill(0); // Clear the gradient.
            grad->resize(0, grad->width()); // So it is accumulated later.
        }
    }
    bpropAccUpdate(ports_value, ports_gradient);
}

//////////////////
// bbpropUpdate //
//////////////////
// Default implementations compile but crash at run-time if not implemented in
// sub-classes.
void OnlineLearningModule::bbpropUpdate(const Vec& input, const Vec& output,
                                        const Vec& output_gradient,
                                        const Vec& output_diag_hessian)
{
    bbpropUpdate(input, output, tmp_input_gradient, output_gradient,
                 tmp_input_diag_hessian, output_diag_hessian);
}

void OnlineLearningModule::bbpropUpdate(const Vec& input, const Vec& output,
                                        Vec& input_gradient,
                                        const Vec& output_gradient,
                                        Vec& input_diag_hessian,
                                        const Vec& output_diag_hessian,
                                        bool accumulate)
{
    PLERROR("In OnlineLearningModule.cc: method 'bbpropUpdate' not"
            "implemented.\n"
            "Please implement it in your derived class, or use"
            "'bpropUpdate'.\n");
}

/////////////////////
// setLearningRate //
/////////////////////
void OnlineLearningModule::setLearningRate( real dynamic_learning_rate )
{
    PLWARNING("In OnlineLearningModule::setLearningRate - The derived class "
            "(%s) does not have a learning rate that can be changed from "
            "outside. If it should have one, please implement setLearningRate "
            "in it", classname().c_str());
}


///////////
// build //
///////////
void OnlineLearningModule::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void OnlineLearningModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(random_gen,             copies);
    deepCopyField(port_sizes,             copies);
    deepCopyField(port_descriptions,      copies);
    deepCopyField(tmp_input_gradient,     copies);
    deepCopyField(tmpm_input_gradient,    copies);
    deepCopyField(tmp_input_diag_hessian, copies);
}

////////////////////
// declareOptions //
////////////////////
void OnlineLearningModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "input_size", &OnlineLearningModule::input_size,
                  OptionBase::buildoption,
                  "Size of the input");

    declareOption(ol, "output_size", &OnlineLearningModule::output_size,
                  OptionBase::buildoption,
                  "Size of the output");

    declareOption(ol, "name", &OnlineLearningModule::name,
                  OptionBase::buildoption,
        "Name of the module (if not provided, the class name is used).");

    declareOption(ol, "estimate_simpler_diag_hessian",
                  &OnlineLearningModule::estimate_simpler_diag_hessian,
                  OptionBase::buildoption,
                  "Should we compute a simpler diagonal estimation of the"
                  " input Hessian\n"
                  "matrix, using only the first (positive) term in:\n"
                  "  d²C/dx² ~= d²C/dy² (dy/dx)² [+ dC/dy d²y/dx²]\n");


    declareOption(ol, "expdir", &OnlineLearningModule::expdir,
                  OptionBase::buildoption,
                  "Path of the directory associated with this module,\n"
                  "in which it should save any file it wishes to create. \n"
                  "The directory will be created if it does not already"
                  " exist.\n"
                  "If expdir is the empty string (the default),\n"
                  "then the module should not create *any* file.\n");

    declareOption(ol, "random_gen",
                  &OnlineLearningModule::random_gen,
                  OptionBase::buildoption,
                  "Pointer to an optional random number generator,\n"
                  "e.g. for initializing parameters or any non-deterministic"
                  " operation\n"
                  "required by the module.\n");

    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void OnlineLearningModule::build_()
{
    if (name.empty())
        name = classname();
}

///////////////
// checkProp //
///////////////
void OnlineLearningModule::checkProp(const TVec<Mat*>& ports_data)
{
    for (int i = 0; i < ports_data.length(); i++) {
        PLCHECK( !ports_data[i] || !ports_data[i]->isEmpty() );
    }
}

////////////////////////
// getPortDescription //
////////////////////////
TVec<string> OnlineLearningModule::getPortDescription(const string& port)
{
    port_descriptions.resize(nPorts());
    int idx = getPortIndex(port);
    TVec<string>& descr = port_descriptions[idx];
    int desired_size = getPortWidth(port);
    PLCHECK_MSG( desired_size >= 0, "Port width must be defined" );
    if (descr.length() != desired_size) {
        descr.resize(desired_size);
        for (int i = 0; i < desired_size; i++)
            descr[i] = port + "_" + tostring(i);
    }
    return descr;
}

//////////////////
// getPortIndex //
//////////////////
int OnlineLearningModule::getPortIndex(const string& port)
{
    return getPorts().find(port);
}

/////////////////
// getPortName //
/////////////////
string OnlineLearningModule::getPortName(int i)
{
    return getPorts()[i];
}

//////////////
// getPorts //
//////////////
const TVec<string>& OnlineLearningModule::getPorts() {
    static TVec<string> default_ports;
    if (default_ports.isEmpty()) {
        default_ports.append("input");
        default_ports.append("output");
    }
    return default_ports;
}

//////////////////
// getPortSizes //
//////////////////
const TMat<int>& OnlineLearningModule::getPortSizes() {
    int n_ports = nPorts();
    if (port_sizes.length() != n_ports) {
        port_sizes.resize(n_ports, 2);
        port_sizes.fill(-1);
        if (n_ports >= 2) {
            port_sizes(0, 1) = input_size;
            port_sizes(1, 1) = output_size;
        }
    }
    return port_sizes;
}

///////////////////
// getPortLength //
///////////////////
int OnlineLearningModule::getPortLength(const string& port)
{
    PLASSERT( getPortIndex(port) >= 0 );
    return getPortSizes()(getPortIndex(port), 0);
}

//////////////////
// getPortWidth //
//////////////////
int OnlineLearningModule::getPortWidth(const string& port)
{
    PLASSERT( getPortIndex(port) >= 0 );
    return getPortSizes()(getPortIndex(port), 1);
}

////////////
// nPorts //
////////////
int OnlineLearningModule::nPorts()
{
    return getPorts().length();
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
