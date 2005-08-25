// -*- C++ -*-

// SelectInputSubsetLearner.h
//
// Copyright (C) 2004 Yoshua Bengio 
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
 * $Id$
 ******************************************************* */

// Authors: Yoshua Bengio

/*! \file SelectInputSubsetLearner.h */


#ifndef SelectInputSubsetLearner_INC
#define SelectInputSubsetLearner_INC

#include "EmbeddedLearner.h"

namespace PLearn {
using namespace std;

class SelectInputSubsetLearner: public EmbeddedLearner
{

private:

    typedef EmbeddedLearner inherited;
  
protected:

    // *********************
    // * protected options *
    // *********************
    Vec learner_inputs;
    TVec<int> all_indices; // indices of selected inputs + indices of the target and weight and other columns

    // ### declare protected option fields (such as learnt parameters) here
    
public:

    // ************************
    // * public build options *
    // ************************

    // ### declare public option fields (such as build options) here
  
    // the inputs can be selected explicitly
    TVec<int> selected_inputs;
    // or they can be selected automatically and randomly
    // by taking a random subset from the original, if random_fraction>0
    // (that subset will be of size int(random_fraction*inputsize()).
    real random_fraction;

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    SelectInputSubsetLearner();


    // ********************
    // * PLearner methods *
    // ********************

private: 

    //! This does the actual building. 
    // (Please implement in .cc)
    void build_();

protected: 
  
    //! Declares this class' options.
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods.
    // If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS.
    PLEARN_DECLARE_OBJECT(SelectInputSubsetLearner);


    // **************************
    // **** PLearner methods ****
    // **************************

    virtual int inputsize() const;

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output. 
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;
                                

    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                       Vec& output, Vec& costs) const;

    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(SelectInputSubsetLearner);
  
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
