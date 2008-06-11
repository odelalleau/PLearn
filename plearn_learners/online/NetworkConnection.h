// -*- C++ -*-

// NetworkConnection.h
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

/*! \file NetworkConnection.h */


#ifndef NetworkConnection_INC
#define NetworkConnection_INC

#include <plearn/base/Object.h>
#include <plearn_learners/online/OnlineLearningModule.h>

namespace PLearn {

class NetworkConnection : public Object
{
    typedef Object inherited;

public:

    //#####  Public Build Options  ############################################

    string source;
    string destination;
    bool propagate_gradient;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor.
    NetworkConnection();

    //! Convenience constructors.
    NetworkConnection(const string& the_source, const string& the_destination,
                      bool the_propagate_gradient, bool call_build_ = true);

    NetworkConnection(PP<OnlineLearningModule> the_src_module,
                      const string& the_src_port,
                      PP<OnlineLearningModule> the_dst_module,
                      const string& the_dst_port,
                      bool the_propagate_gradient, bool call_build_ = true);

    //! Initialize the connection using the list of modules provided as a map
    //! from modules' names to pointers to the modules themselves.
    //! The goal of the initialization is to properly set the 'src_module',
    //! 'src_port', 'dst_module' and 'dst_port' fields.
    void initialize(map<string, PP<OnlineLearningModule> >& modules);

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Return the source module.
    PP<OnlineLearningModule> getSourceModule();

    //! Return the source port.
    const string& getSourcePort();

    //! Return the destination module.
    PP<OnlineLearningModule> getDestinationModule();

    //! Return the destination port.
    const string& getDestinationPort();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(NetworkConnection);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    //! Source module.
    PP<OnlineLearningModule> src_module;

    //! Source port.
    string src_port;

    //! Destination module.
    PP<OnlineLearningModule> dst_module;

    //! Destination port.
    string dst_port;

    //#####  Protected Options  ###############################################

protected:
    //#####  Protected Member Functions  ######################################

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(NetworkConnection);

} // end of namespace PLearn

#endif


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
