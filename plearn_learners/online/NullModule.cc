// -*- C++ -*-

// NullModule.cc
//
// Copyright (C) 2007 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file NullModule.cc */



#include "NullModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NullModule,
    "An OnlineLearningModule that does nothing with its unique port.",
    "Such a module can be useful if a port of another module needs to be\n"
    "stored without its value being used outside of that module (e.g. to\n"
    "store the 'state' of the module): by adding a connection from that\n"
    "port to a NullModule, we ensure the value is stored, even if it is\n"
    "not actually used.\n"
);

////////////////
// NullModule //
////////////////
NullModule::NullModule(const string& name, bool call_build_):
    inherited(name.empty() ? classname() : name, call_build_)
{
    if (call_build_)
        build_();
}

void NullModule::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &NullModule::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void NullModule::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.
}

///////////
// build //
///////////
void NullModule::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void NullModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

///////////
// fprop //
///////////
void NullModule::fprop(const Vec& input, Vec& output) const
{
    // Nothing to do.
}

void NullModule::fprop(const TVec<Mat*>& ports_value)
{
    // Nothing to do.
    PLASSERT( ports_value.length() == nPorts() );
    // Ensure we are not asking for the value of the 'null' port.
    PLASSERT( !ports_value[0] || !ports_value[0]->isEmpty() );
}

////////////////////
// bpropAccUpdate //
////////////////////
void NullModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_gradient.length() == nPorts() );
    Mat* null_grad = ports_gradient[0];
    if (null_grad) {
        // Noone should ever provide us with a gradient w.r.t. 'null'.
        PLASSERT( null_grad->isEmpty() );
        // All we need to do is resize the gradient matrix.
        PLASSERT( ports_value[0] );
        PLASSERT( null_grad->width() == ports_value[0]->width() );
        null_grad->resize(ports_value[0]->length(), null_grad->width());
    }
}

//////////////
// getPorts //
//////////////
const TVec<string>& NullModule::getPorts() {
    static TVec<string> null_port;
    if (null_port.isEmpty())
        null_port.append("null");
    return null_port;
}

//////////////////
// getPortSizes //
//////////////////
const TMat<int>& NullModule::getPortSizes() {
    static TMat<int> null_size;
    if (null_size.isEmpty()) {
        null_size.resize(1, 2);
        null_size.fill(-1);
    }
    return null_size;
}

/////////////////
// bpropUpdate //
/////////////////
/* THIS METHOD IS OPTIONAL
void NullModule::bpropUpdate(const Vec& input, const Vec& output,
                               Vec& input_gradient,
                               const Vec& output_gradient,
                               bool accumulate)
{
}
*/

/* THIS METHOD IS OPTIONAL
void NullModule::bpropUpdate(const Vec& input, const Vec& output,
                               const Vec& output_gradient)
{
}
*/

//////////////////
// bbpropUpdate //
//////////////////
/* THIS METHOD IS OPTIONAL
void NullModule::bbpropUpdate(const Vec& input, const Vec& output,
                                Vec& input_gradient,
                                const Vec& output_gradient,
                                Vec& input_diag_hessian,
                                const Vec& output_diag_hessian,
                                bool accumulate)
{
}
*/

/* THIS METHOD IS OPTIONAL
void NullModule::bbpropUpdate(const Vec& input, const Vec& output,
                                const Vec& output_gradient,
                                const Vec& output_diag_hessian)
{
}
*/

////////////
// forget //
////////////
void NullModule::forget()
{
}

//////////////
// finalize //
//////////////
/* THIS METHOD IS OPTIONAL
void NullModule::finalize()
{
}
*/

//////////////////////
// bpropDoesNothing //
//////////////////////
/* THIS METHOD IS OPTIONAL
bool NullModule::bpropDoesNothing()
{
}
*/

/////////////////////
// setLearningRate //
/////////////////////
/* OPTIONAL
void NullModule::setLearningRate(real dynamic_learning_rate)
{
}
*/

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
