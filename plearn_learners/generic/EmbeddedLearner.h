// -*- C++ -*-

// EmbeddedLearner.h
// 
// Copyright (C) 2002 Frederic Morin
// Copyright (C) 2003 Pascal Vincent
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
   * $Id: EmbeddedLearner.h,v 1.2 2003/02/28 03:21:28 plearner Exp $ 
   ******************************************************* */

/*! \file EmbeddedLearner.h */
#ifndef EmbeddedLearner_INC
#define EmbeddedLearner_INC

#include "Learner.h"

namespace PLearn <%
using namespace std;

// ###### EmbeddedLearner ######################################################

class EmbeddedLearner: public Learner
{
    typedef Learner inherited;
protected:
    // *********************
    // * protected options *
    // *********************
    
    // ### declare protected option fields (such as learnt parameters) here
    // ...
    
public:
    // ************************
    // * public build options *
    // ************************

    // ### declare public option fields (such as build options) here
    // ...

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    EmbeddedLearner(PP<Learner> the_learner = PP<Learner>());

  // ******************
  // * Object methods *
  // ******************

    void setLearner(PP<Learner> the_learner);

private: 
    //! This does the actual building. 
    // (Please implement in .cc)
    void build_();
protected: 
    //! Declares this class' options
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);
public:
    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Provides a help message describing this class
    virtual string help() const;

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

    //! Declares name and deepCopy methods
    DECLARE_NAME_AND_DEEPCOPY(EmbeddedLearner);

    // *******************
    // * Learner methods *
    // *******************
    
    // trains the model
    virtual void train(VMat training_set); 
    
    // computes the ouptu of a trained model
    virtual void use(const Vec& input, Vec& output);

protected:
    PP<Learner> learner;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(EmbeddedLearner);
  
%> // end of namespace PLearn

#endif
