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

NetworkConnection::NetworkConnection():
    propagate_gradient(true)
{}

///////////////////////
// NetworkConnection //
///////////////////////
NetworkConnection::NetworkConnection(
        PP<OnlineLearningModule> the_src_module,
        const string& the_src_port,
        PP<OnlineLearningModule> the_dest_module,
        const string& the_dest_port,
        bool the_propagate_gradient,
        bool call_build_):

    inherited(call_build_),
    src_module(the_src_module),
    src_port(the_src_port),
    dest_module(the_dest_module),
    dest_port(the_dest_port),
    propagate_gradient(the_propagate_gradient)
{
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
    declareOption(ol, "src_module", &NetworkConnection::src_module,
                  OptionBase::buildoption,
        "Source module.");

    declareOption(ol, "src_port", &NetworkConnection::src_port,
                  OptionBase::buildoption,
        "Source module's port.");

    declareOption(ol, "dest_module", &NetworkConnection::dest_module,
                  OptionBase::buildoption,
        "Destination module.");

    declareOption(ol, "dest_port", &NetworkConnection::dest_port,
                  OptionBase::buildoption,
        "Destination module's port.");

    declareOption(ol, "propagate_gradient",
                  &NetworkConnection::propagate_gradient,
                  OptionBase::buildoption,
        "Whether or not the destination should propagate its gradient to the\n"
        "source.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void NetworkConnection::build_()
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
