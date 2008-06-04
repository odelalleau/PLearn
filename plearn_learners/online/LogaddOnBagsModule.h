// -*- C++ -*-

// LogaddOnBagsModule.h
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

/*! \file LogaddOnBagsModule.h */

#ifndef LogaddOnBagsModule_INC
#define LogaddOnBagsModule_INC

#include <plearn_learners/online/OnBagsModule.h>

#include <map>

namespace PLearn {

class LogaddOnBagsModule : public OnBagsModule
{
    typedef OnBagsModule inherited;

public:
    //#####  Public Build Options  ############################################

    //#####  Public Learnt Options  ###########################################


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    LogaddOnBagsModule();

    //! update internal statistics needed to compute the output of bag
    virtual void fpropAcc(const Vec& input);
    virtual void fpropInit(const Vec& input);
    virtual void fpropOutput(Vec& output);

    //! compute the gradient w.r.t bag inputs
    virtual void bprop( const Mat& baginputs,
                        const Vec& bagoutput_gradient,
                        Mat& baginputs_gradients);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(LogaddOnBagsModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    Vec accumulated_output;
    
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(LogaddOnBagsModule);

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
