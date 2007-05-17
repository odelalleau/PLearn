// -*- C++ -*-

// NetworkConnection.cc
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

/*! \file NetworkConnection.cc */


#include "NetworkConnection.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NetworkConnection,
    "A connexion between modules in a LearningNetwork.",
    "A connexion indicates links between ports between two modules\n"
    "inheriting from OnlineLearningModule.\n"
);

///////////////////////
// NetworkConnection //
///////////////////////
NetworkConnection::NetworkConnection():
    propagate_gradient(true)
{}

///////////////////////
// NetworkConnection //
///////////////////////
NetworkConnection::NetworkConnection(const string& the_source,
                                     const string& the_destination,
                                     bool the_propagate_gradient,
                                     bool call_build_):
    inherited(call_build_),
    source(the_source),
    destination(the_destination),
    propagate_gradient(the_propagate_gradient)
{
    if (call_build_)
        build_();
}

NetworkConnection::NetworkConnection(
                      PP<OnlineLearningModule> the_src_module,
                      const string& the_src_port,
                      PP<OnlineLearningModule> the_dst_module,
                      const string& the_dst_port,
                      bool the_propagate_gradient, bool call_build_):
    inherited(call_build_),
    propagate_gradient(the_propagate_gradient),
    src_module(the_src_module),
    src_port(the_src_port),
    dst_module(the_dst_module),
    dst_port(the_dst_port)
{
    PLASSERT( the_src_module && the_dst_module );
    source = the_src_module->name + "." + the_src_port;
    destination = the_dst_module->name + "." + the_dst_port;
    if (call_build_)
        build_();
}

///////////
// build //
///////////
void NetworkConnection::build()
{
    inherited::build();
    build_();
}

////////////////
// initialize //
////////////////
void NetworkConnection::initialize(map<string, PP<OnlineLearningModule> >& modules)
{
    TVec<string> specs;
    specs.append(source);
    specs.append(destination);
    for (int i = 0; i < specs.length(); i++) {
        const string& spec = specs[i];
        size_t dot_pos = spec.find('.');
        if (dot_pos == string::npos)
            PLERROR("In NetworkConnection::initialize - Could not find a dot "
                    "in the port specification '%s'", spec.c_str());
        string module_name = spec.substr(0, dot_pos);
        if (modules.find(module_name) == modules.end())
            PLERROR("In NetworkConnection::initialize - Could not find a "
                    "module named '%s'", module_name.c_str());
        PP<OnlineLearningModule> module = modules[module_name];
        string port = spec.substr(dot_pos + 1);
        if (i == 0) {
            src_module = module;
            src_port = port;
        } else {
            PLASSERT( i == 1 );
            dst_module = module;
            dst_port = port;
        }
    }

}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void NetworkConnection::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("NetworkConnection::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void NetworkConnection::declareOptions(OptionList& ol)
{
    declareOption(ol, "source", &NetworkConnection::source,
                  OptionBase::buildoption,
        "Source of the connection (of the form 'module.port').");

    declareOption(ol, "destination", &NetworkConnection::destination,
                  OptionBase::buildoption,
        "Destination of the connection (of the form 'module.port').");

    declareOption(ol, "propagate_gradient",
                  &NetworkConnection::propagate_gradient,
                  OptionBase::buildoption,
        "Whether or not the destination should propagate its gradient to the\n"
        "source.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void NetworkConnection::build_()
{
}

//////////////////////////
// getDestinationModule //
//////////////////////////
PP<OnlineLearningModule> NetworkConnection::getDestinationModule()
{
    PLASSERT_MSG( dst_module, "getDestinationModule() cannot be called before "
            "the connection is initialized");
    return dst_module;
}

////////////////////////
// getDestinationPort //
////////////////////////
const string& NetworkConnection::getDestinationPort()
{
    PLASSERT_MSG( !dst_port.empty(), "getDestinationPort() cannot be called "
            "before the connection is initialized");
    return dst_port;
}

/////////////////////
// getSourceModule //
/////////////////////
PP<OnlineLearningModule> NetworkConnection::getSourceModule()
{
    PLASSERT_MSG( src_module, "getSourceModule() cannot be called before the "
            "connection is initialized");
    return src_module;
}

///////////////////
// getSourcePort //
///////////////////
const string& NetworkConnection::getSourcePort()
{
    PLASSERT_MSG( !src_port.empty(), "getSourcePort() cannot be called before "
            "the connection is initialized");
    return src_port;
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
