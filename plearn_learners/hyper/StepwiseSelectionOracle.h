// -*- C++ -*-

// StepwiseSelectionOracle.h
//
// Copyright (C) 2004 ApSTAT Technologies Inc.
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

/*! \file StepwiseSelectionOracle.h */
#ifndef StepwiseSelectionOracle_INC
#define StepwiseSelectionOracle_INC

// From C++ stdlib
#include <map>
#include <queue>

// From PLearn
#include "OptionsOracle.h"

namespace PLearn {
using namespace std;

/**
 *  This oracle implements a stepwise forward variable selection procedure.
 *  It supports the interface provided by SelectInputsLearner, wherein the
 *  variables to selected are specified by a vector of normalized
 *  'selected_inputs' ranging from 0 to n-1, where n is the maximum number
 *  of variables.  (The object SelectInputsLearner is then responsible for
 *  mapping back those normalized inputs to the real inputs of the
 *  learner).
 *
 *  You simply specify to this oracle the maximum number of inputs that
 *  should be allowed.
 *
 *  The current implementation is the simplest: it adds variables one at a
 *  time.  Look-ahead is not currently supported.
 */
class StepwiseSelectionOracle: public OptionsOracle
{
    typedef OptionsOracle inherited;

public:
    //#####  Options  #########################################################

    //! Name of option that should contain the returned list of selected
    //! inputs  (default = 'selected_inputs')
    string option_name;

    //! Maximum number of variables that should be permitted in the search
    int maxvars;

protected:
    //! This remembers the current BASE set of selected indexes,
    //! EXCLUDING the variable we are currently iterating upon
    TVec<int> base_selected_variables;

    //! This list contains the remaining combinations to generate for the
    //! current variable.  It consists of a set of strings of FULLY-FORMED
    //! options, starting with the current base_selected_variables and
    //! ending with each allowable index for the current variable
    set<string> current_indexes_searchset;

    //! This remembers the performance of each combination tried in the
    //! current search set.
    priority_queue< pair<real,string> > combination_performance;

    //! To know that we have gone through all combinations
    bool last_combination;

public:
    //#####  Object Methods  ##################################################

    // Default constructor
    StepwiseSelectionOracle();

    // simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(StepwiseSelectionOracle);

protected:
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

private:
    //! This does the actual building.
    void build_();

public:
    //#####  Oracle Methods  ###################################################

    //! Returns the set of names of options this generator generates
    virtual TVec<string> getOptionNames() const;

    //! Given the objective value returned for a previous trial, generate
    //! a new trial
    virtual TVec<string> generateNextTrial(const TVec<string>& older_trial,
                                           real obtained_objective);

    //! Reset the generator's internal state (as having no info about
    //! previous trials).
    virtual void forget();

protected:
    //! Generate a new searchset from base_selected_variables and
    //! combination_performance
    void generateNewSearchset();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(StepwiseSelectionOracle);

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
