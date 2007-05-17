// -*- C++ -*-

// NetworkModule.h
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

/*! \file NetworkModule.h */


#ifndef NetworkModule_INC
#define NetworkModule_INC

#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/NetworkConnection.h>

namespace PLearn {

class NetworkModule : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    TVec< PP<OnlineLearningModule> > modules;
    TVec< PP<NetworkConnection> > connections;

    TMat<string> ports;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    NetworkModule();

    //! Basic fprop (not implemented).
    virtual void fprop(const Vec& input, Vec& output) const;

    //! Compute the network's outputs by propagating each underlying module's
    //! computations through the network.
    virtual void fprop(const TVec<Mat*>& ports_value);

    //! Back-propagate gradient through the network, and update each module's
    //! internal parameters.
    virtual void bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient);

    //! Return the list of ports in the module.
    virtual const TVec<string>& getPorts();

    //! Return the size of each port.
    virtual const TMat<int>& getPortSizes();

    //! Return the description of a given port.
    virtual TVec<string> getPortDescription(const string& port);

    //! Reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(NetworkModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


protected:
    

    //! Ordered list of modules used when doing a fprop (the integer values
    //! correspond to indices in 'modules').
    TVec<int> fprop_path;

    //! Ordered list of modules used when doing a bprop (the integer values
    //! correspond to indices in 'modules').
    TVec<int> bprop_path;

    //! The i-th element is the list of Mat* pointers being provided to the
    //! i-th module in a fprop step.
    TVec< TVec<Mat*> > fprop_data;

    //! The i-th element is the list of matrices that need to be resized to
    //! empty matrices prior to calling fprop() on the i-th module in a fprop
    //! step.
    //! The resizing is needed to ensure we correctly compute the desired
    //! outputs.
    TVec< TVec<int> > fprop_toresize;

    //! The i-th element is the list of correspondances between ports in this
    //! NetworkModule and ports in the i-th module of the fprop path.
    //! The list is given by a matrix, where each row is a pair (p1, p2) with
    //! p1 the index of the port in this NetworkModule, and p2 the index of the
    //! corresponding port in the i-th module during fprop.
    TVec< TMat<int> > fprop_toplug;
    
    //! The i-th element is the list of Mat* pointers being provided to the
    //! i-th module in a bprop step.
    TVec< TVec<Mat*> > bprop_data;

    //! The i-th element is the list of matrices that need to be resized to
    //! empty matrices prior to calling bpropUpdate() on the i-th module in a
    //! bprop step.
    //! The resizing is needed to ensure we correctly compute the desired
    //! gradients.
    TVec< TVec<int> > bprop_toresize;

    //! A list of all matrices used to store the various computation results in
    //! the network (i.e. the outputs of each module).
    TVec<Mat> all_mats;

    //! The list of ports, computed from the 'ports' option.
    TVec<string> all_ports;

    //! The description of each port, obtained from the underlying modules.
    TVec< TVec<string> > port_descriptions;
    
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(NetworkModule);

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
