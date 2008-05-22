// -*- C++ -*-

// StepwiseSelectionOracle.cc
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

/*! \file StepwiseSelectionOracle.cc */

#include <algorithm>
#include "StepwiseSelectionOracle.h"
#include <plearn/base/stringutils.h>
#include <plearn/io/openString.h>

namespace PLearn {
using namespace std;

StepwiseSelectionOracle::StepwiseSelectionOracle()
    : option_name("selected_inputs"),
      maxvars(-1),
      last_combination(false)
{ }

PLEARN_IMPLEMENT_OBJECT(
    StepwiseSelectionOracle,
    "This oracle implements a stepwise forward variable selection procedure.",
    "It supports the interface provided by SelectInputsLearner, wherein the\n"
    "variables to selected are specified by a vector of normalized\n"
    "'selected_inputs' ranging from 0 to n-1, where n is the maximum number\n"
    "of variables.  (The object SelectInputsLearner is then responsible for\n"
    "mapping back those normalized inputs to the real inputs of the\n"
    "learner).\n"
    "\n"
    "You simply specify to this oracle the maximum number of inputs that\n"
    "should be allowed.\n"
    "\n"
    "The current implementation is the simplest: it adds variables one at a\n"
    "time.  Look-ahead is not currently supported."
    );

void StepwiseSelectionOracle::declareOptions(OptionList& ol)
{
    declareOption(ol, "option_name", &StepwiseSelectionOracle::option_name,
                  OptionBase::buildoption,
                  "Name of option that should contain the returned list of selected\n"
                  "inputs  (default = 'selected_inputs')");

    declareOption(ol, "maxvars", &StepwiseSelectionOracle::maxvars,
                  OptionBase::buildoption,
                  "Maximum number of variables that should be permitted in the search");

    declareOption(ol, "current_indexes_searchset",
                  &StepwiseSelectionOracle:: current_indexes_searchset,
                  OptionBase::learntoption,
                  "Contains the remaining combinations to generate for the"
                  " current variable.");

    declareOption(ol, "combination_performance",
                  &StepwiseSelectionOracle:: combination_performance,
                  OptionBase::learntoption,
                  "This remembers the performance of each combination tried in the"
                  " current search set.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

TVec<string>  StepwiseSelectionOracle::getOptionNames() const
{
    return TVec<string>(1,option_name);
}

TVec<string> StepwiseSelectionOracle::generateNextTrial(
    const TVec<string>& older_trial, real obtained_objective)
{
    if (last_combination)
        return TVec<string>();
    else {
        if (older_trial.size() > 0) {
            PLASSERT(older_trial.size() == 1);
            // insert negative of objective since priority_queue computes
            // the maximum of its inserted elements
            combination_performance.push(make_pair(- obtained_objective,
                                                   older_trial[0]));
        }

        if (current_indexes_searchset.empty())
            generateNewSearchset();
        if (last_combination)
            return TVec<string>();

        PLASSERT(current_indexes_searchset.begin() !=
               current_indexes_searchset.end());

        string optionval = *current_indexes_searchset.begin();
        current_indexes_searchset.erase(current_indexes_searchset.begin());

        return TVec<string>(1,optionval);
    }
}

void StepwiseSelectionOracle::forget()
{
    base_selected_variables.resize(0);
    current_indexes_searchset.clear();
    combination_performance = priority_queue< pair<real,string> >();
    last_combination=false;
}

void StepwiseSelectionOracle::build_()
{
    forget();
}

// ### Nothing to add here, simply calls build_
void StepwiseSelectionOracle::build()
{
    inherited::build();
    build_();
}

void StepwiseSelectionOracle::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(base_selected_variables, copies);
}

void StepwiseSelectionOracle::generateNewSearchset()
{
    if (last_combination)
        return;

    // First find the best-performing combination in combination_performance
    // and convert the option string into a vector of ints to obtain the new
    // base_selected_variables
    if (combination_performance.size() > 0) {
        const pair<real,string>& min_objective = combination_performance.top();
        const string& option = min_objective.second;
        PStream option_stream = openString(option,PStream::plearn_ascii);
        option_stream >> base_selected_variables;
    }
    else {
        // There should have been nothing computed before...
        PLASSERT( base_selected_variables.size() == 0);
    }

    // Second, find the remaining variables (not in base_selected_variables)
    TVec<int> range(0,maxvars-1,1);
    set<int> remaining_vars(range.begin(), range.end());
    for(int i=0, n=base_selected_variables.size() ; i<n ; ++i)
        remaining_vars.erase(base_selected_variables[i]);

    // Third, make up a new current_indexes_searchset if we have not yet
    // reached maxvars
    current_indexes_searchset.clear();
    combination_performance = priority_queue< pair<real,string> >();
    int newsize = base_selected_variables.size() + 1;
    if (newsize > maxvars) {
        last_combination = true;
        return;
    }

    TVec<int> new_combination(newsize);
    new_combination.subVec(0, newsize-1) << base_selected_variables;
    for (set<int>::iterator it = remaining_vars.begin() ;
         it != remaining_vars.end() ; ++it) {
        new_combination[newsize-1] = *it;
        ostringstream newoption;
        PStream newoption_stream(&newoption, false /* don't own */);
        newoption_stream << new_combination;
        current_indexes_searchset.insert(newoption.str());
    }
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
