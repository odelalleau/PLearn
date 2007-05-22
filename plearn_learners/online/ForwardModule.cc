// -*- C++ -*-

// ForwardModule.cc
//
// Copyright (C) 2007 Jerome Louradour
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

// Authors: Jerome Louradour

/*! \file ForwardModule.cc */



#include "ForwardModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ForwardModule,
    "Module that simply forwards all calls to an underlying module.",
    "The underlying module may be chosen among a list of different modules,\n"
    "through the option 'forward_to'.\n"
);

///////////////////
// ForwardModule //
///////////////////
ForwardModule::ForwardModule(const string& the_name, bool call_build_):
    inherited(the_name.empty() && call_build_ ? classname() : the_name,
              call_build_),
    current(-1)
{
    if (call_build_)
        build_();
}

void ForwardModule::declareOptions(OptionList& ol)
{

    declareOption(ol, "modules", &ForwardModule::modules,
                  OptionBase::buildoption,
        "The list of modules that can be used to forward calls.");

    declareOption(ol, "forward_to", &ForwardModule::forward_to,
                  OptionBase::buildoption,
        "Name of the module currently being used. If empty, the first module\n"
        "in 'modules' will be used.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

}

////////////
// build_ //
////////////
void ForwardModule::build_()
{
    current = -1;
    if (forward_to.empty())
        current = 0;
    else {
        for (int i = 0; i < modules.length(); i++) {
            if (modules[i]->name == forward_to) {
                current = i;
                break;
            }
        }
    }
    PLCHECK( current >= 0 );
}

///////////
// build //
///////////
void ForwardModule::build()
{
    inherited::build();
    build_();
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ForwardModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(modules, copies);
}

///////////
// fprop //
///////////
void ForwardModule::fprop(const TVec<Mat*>& ports_value)
{
    modules[current]->fprop(ports_value);
}

////////////////////
// bpropAccUpdate //
////////////////////
void ForwardModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                   const TVec<Mat*>& ports_gradient)
{
    modules[current]->bpropAccUpdate(ports_value, ports_gradient);
}

////////////
// forget //
////////////
void ForwardModule::forget()
{
     modules[current]->forget();
}

//////////////
// finalize //
//////////////
void ForwardModule::finalize()
{
     modules[current]->finalize();
}

//////////////////////
// bpropDoesNothing //
//////////////////////
bool ForwardModule::bpropDoesNothing()
{
     return modules[current]->bpropDoesNothing();
}

/////////////////////
// setLearningRate //
/////////////////////
void ForwardModule::setLearningRate(real dynamic_learning_rate)
{
     modules[current]->setLearningRate(dynamic_learning_rate);
}

////////////////////////
// getPortDescription //
////////////////////////
TVec<string> ForwardModule::getPortDescription(const string& port)
{
    return modules[current]->getPortDescription(port);
} 

//////////////
// getPorts //
//////////////
const TVec<string>& ForwardModule::getPorts() {
      return modules[current]->getPorts();
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
