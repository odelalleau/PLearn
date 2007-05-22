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
    "Module to refer to a module that may be used in different places of a network.",
    "Module to refer to a module that may be used in different places of a network.\n"
    "This wrapper allows to duplicate a same module in a network.");

///////////////////
// ForwardModule //
///////////////////
ForwardModule::ForwardModule(const string& the_name, bool call_build_):
               inherited(the_name.empty() && call_build_ ? classname() : the_name,
                         call_build_)
{
    if (call_build_)
        build_();
}

void ForwardModule::declareOptions(OptionList& ol)
{

    declareOption(ol, "module", &ForwardModule::module,
                  OptionBase::buildoption,
        "The module pointed by this module.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

}

void ForwardModule::build_()
{
    module->build();
}

// ### Nothing to add here, simply calls build_
void ForwardModule::build()
{
    inherited::build();
    build_();
}


void ForwardModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(module,	copies);
}

///////////
// fprop //
///////////
void ForwardModule::fprop(const TVec<Mat*>& ports_value)
{
    //PLASSERT( ports_value.length() == module->nPorts() );
 
    module->fprop(ports_value);
}

/////////////////
// bpropUpdate //
/////////////////


void ForwardModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                          const TVec<Mat*>& ports_gradient)
{
    //PLASSERT( ports_value.length() == module->nPorts() && ports_gradient.length() == module->nPorts());
	    
    module->bpropAccUpdate(ports_value, ports_gradient);
	    
}

// This version is similar to bpropAccUpdate but it does not accumulate
// in the input ports gradient 
void ForwardModule::bpropUpdate(const TVec<Mat*>& ports_value,
                                       const TVec<Mat*>& ports_gradient)
{
    module->bpropUpdate(ports_value, ports_gradient);
}


////////////
// forget //
////////////
void ForwardModule::forget()
{
     module->forget();
}

//////////////
// finalize //
//////////////
void ForwardModule::finalize()
{
     module->finalize();
}


//////////////////////
// bpropDoesNothing //
//////////////////////
// the default implementation returns false
bool ForwardModule::bpropDoesNothing()
{
     return module->bpropDoesNothing();
}


/////////////////////
// setLearningRate //
/////////////////////
// The default implementation raises a warning and does not do anything.
void ForwardModule::setLearningRate(real dynamic_learning_rate)
{
     module->setLearningRate(dynamic_learning_rate);
}


////////////////////////
// getPortDescription //
////////////////////////
// The default implementation is probably appropriate
TVec<string> ForwardModule::getPortDescription(const string& port)
{
    return module->getPortDescription(port);
} 

//////////////////
// getPortIndex //
//////////////////
// The default implementation is probably appropriate
int ForwardModule::getPortIndex(const string& port)
{
    return module->getPortIndex(port);
}


/////////////////
// getPortName //
/////////////////
// The default implementation is probably appropriate
string ForwardModule::getPortName(int i)
{
       return module->getPortName(i);
}


//////////////
// getPorts //
//////////////
const TVec<string>& ForwardModule::getPorts() {
      return module->getPorts();
}

//////////////////
// getPortSizes //
//////////////////
const TMat<int>& ForwardModule::getPortSizes() {
      return module->getPortSizes();
}
  

///////////////////
// getPortLength //
///////////////////
// The default implementation is probably appropriate
int ForwardModule::getPortLength(const string& port)
{
    return module->getPortLength(port);
}


//////////////////
// getPortWidth //
//////////////////
// The default implementation is probably appropriate
int ForwardModule::getPortWidth(const string& port)
{
    return module->getPortWidth(port);
}

////////////
// nPorts //
////////////
// The default implementation is probably appropriate
int ForwardModule::nPorts()
{
    return module->nPorts();
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
