// -*- C++ -*-

// SequentialValidation.h
//
// Copyright (C) 2003 Rejean Ducharme, Yoshua Bengio
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



#ifndef SEQUENTIAL_VALIDATION
#define SEQUENTIAL_VALIDATION

#include "Object.h"
#include "SequentialLearner.h"
#include "VMat.h"
#include "PP.h"

namespace PLearn <%
using namespace std;


class SequentialValidation: public Object
{
  public:

    typedef Object inherited;

    int init_train_size; // size of first training set
    VMat dataset; // the training/test set
    PP<SequentialLearner> learner; // the SequentialLearner that will be tested
    string expdir; // the directory where everything will be saved
    bool save_models;
    bool save_initial_models;
    bool save_test_outputs;
    bool save_test_costs;

  private:
    //! This does the actual building
    void build_();

  protected:
    //! Declare this class' options
    static void declareOptions(OptionList& ol);

  public:

    //! Default constructor
    SequentialValidation();

    //! Simply calls inherited::build() then build_()
    virtual void build();

    //! Provides a help message describing this class
    static string help();

    //! The main method;  runs the experiment
    virtual void run();
    
    //!  Does the necessary operations to transform a shallow copy (this)
    //!  into a deep copy by deep-copying all the members that need to be.
    DECLARE_NAME_AND_DEEPCOPY(SequentialValidation);
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(SequentialValidation);

%> // end of namespace PLearn

#endif
