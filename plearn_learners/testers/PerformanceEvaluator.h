// -*- C++ -*-

// PerformanceEvaluator.h
//
// Copyright (C) 2006 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file PerformanceEvaluator.h */


#ifndef PerformanceEvaluator_INC
#define PerformanceEvaluator_INC

#include <plearn/base/PP.h>
#include <plearn/base/Object.h>
#include <plearn/vmat/VMat.h>
#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

/**
 * Evaluates the performance of a learner given a testset VMat and the learner's corresponding output VMat.
 * Subclasses of this class are mainly designed to be plugged in a PTester, 
 * and allow to carry 'performance evaluations' that aren't just 
 * simple statistics of the test-costs computed by the learner. 
 * Indeed a PerformanceEvaluator has at its disposal the whole testset, 
 * and the whole corresponding learner output, and will also be passed a 
 * results directory in which it can create files such as performance curves... 
 * Since it also receives a pointer to the trained PLearner, a specialized 
 * PerformanceEvaluator may even inspect learner-specific options and call learner-specific methods 
 *
 */
class PerformanceEvaluator : public Object
{
    typedef Object inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    PerformanceEvaluator();

    //! Returns the name of the costs whose values are returned by the evaluatePerformance method
    virtual TVec<string> getCostNames() const = 0;

    //! Evaluates performance from the given testset and corresponding learner_output
    //! Performance curves and similar results may be saved in the resultsdir
    //! The call should return a Vec of costs (their names are given by getCostNames())
    virtual Vec evaluatePerformance(PP<PLearner> learner, VMat testset, VMat learner_output, PPath resultsdir) const = 0;

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_ABSTRACT_OBJECT(PerformanceEvaluator);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

protected:
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
DECLARE_OBJECT_PTR(PerformanceEvaluator);

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
