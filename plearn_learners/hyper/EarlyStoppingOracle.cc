
// -*- C++ -*-

// EarlyStoppingOracle.cc
//
// Copyright (C) 2003-2004 ApSTAT Technologies Inc.
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

// Author: Pascal Vincent

/* *******************************************************
 * $Id$
 ******************************************************* */

/*! \file EarlyStoppingOracle.cc */
#include "EarlyStoppingOracle.h"
#include <plearn/base/stringutils.h>
#define PL_LOG_MODULE_NAME "EarlyStoppingOracle"
#include <plearn/io/pl_log.h>
#
namespace PLearn {
using namespace std;

EarlyStoppingOracle::EarlyStoppingOracle()
    : nreturned(0),
      previous_objective(REAL_MAX),
      best_objective(REAL_MAX),
      best_step(-1),
      met_early_stopping(false),
      min_value(-REAL_MAX),
      max_value(REAL_MAX),
      max_degradation(REAL_MAX),
      relative_max_degradation(-1),
      min_improvement(-REAL_MAX),
      relative_min_improvement(-1),
      max_degraded_steps(100),
      min_n_steps(0)
{
    //    build_();
}

PLEARN_IMPLEMENT_OBJECT(EarlyStoppingOracle, "ONE LINE DESCR", "NO HELP");

void EarlyStoppingOracle::declareOptions(OptionList& ol)
{
    declareOption(ol, "option", &EarlyStoppingOracle::option, OptionBase::buildoption,
                  "the name of the option to change");
    declareOption(ol, "values", &EarlyStoppingOracle::values, OptionBase::buildoption,
                  "a list of values to try in sequence ");
    declareOption(ol, "range", &EarlyStoppingOracle::range, OptionBase::buildoption,
                  "a numerical range of the form [ start, end ] or [ start, end, step ]\n"
                  "WARNING: end is not included!");

    declareOption(ol, "min_value", &EarlyStoppingOracle::min_value, OptionBase::buildoption,
                  "minimum allowed error beyond which we stop\n");

    declareOption(ol, "max_value", &EarlyStoppingOracle::max_value, OptionBase::buildoption,
                  "maximum allowed error beyond which we stop\n");

    declareOption(ol, "max_degradation", &EarlyStoppingOracle::max_degradation, OptionBase::buildoption,
                  "maximum allowed degradation from last best objective value\n");

    declareOption(ol, "relative_max_degradation", &EarlyStoppingOracle::relative_max_degradation, OptionBase::buildoption,
                  "maximum allowed degradation from last best objective, relative to abs(best_objective)\n"
                  "ex: 0.10 will allow a degradation of 10% the magnitude of best_objective\n"
                  "Will be ignored if negative}n");

    declareOption(ol, "min_improvement", &EarlyStoppingOracle::min_improvement, OptionBase::buildoption,
                  "minimum required improvement from previous objective value\n");

    declareOption(ol, "relative_min_improvement", &EarlyStoppingOracle::relative_min_improvement, OptionBase::buildoption,
                  "minimum required improvement from previous objective value, relative to it.\n"
                  "ex: 0.01 means we need an improvement of 0.01*abs(previous_objective)  i.e. 1%\n"
                  "Will be ignored if negative\n");

    declareOption(ol, "max_degraded_steps", &EarlyStoppingOracle::max_degraded_steps, OptionBase::buildoption,
                  "    ax. nb of steps beyond best found\n");

    declareOption(ol, "min_n_steps", &EarlyStoppingOracle::min_n_steps, OptionBase::buildoption,
                  "minimum required number of steps before allowing early stopping\n");

    //learnt option
    declareOption(ol, "nreturned", &EarlyStoppingOracle::nreturned,
                  OptionBase::learntoption,
                  "The number of returned option\n");

    declareOption(ol, "best_objective", &EarlyStoppingOracle::best_objective,
                  OptionBase::learntoption,
                  "The best objective see up to date\n");

    declareOption(ol, "best_step", &EarlyStoppingOracle::best_step,
                  OptionBase::learntoption,
                  "The step where we have see the best objective\n");

    declareOption(ol, "met_early_stopping", &EarlyStoppingOracle::met_early_stopping,
                  OptionBase::learntoption,
                  "True if we met the early stopping criterion\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void EarlyStoppingOracle::build_()
{
    if(values.length()>0)
        option_values = values;
    else if(range.length()>=2)
    {
        option_values.resize(0);
        real step = 1;
        if(range.length()==3){
            step = range[2];
            if(step<0 || is_equal(step,0))
                PLERROR("IN EarlyStoppingOracle::build_() - the"
                        " step(=%f) should be greater the 0!",step);
        }
        for(real x = range[0]; x<range[1]; x+=step)
            option_values.append(tostring(x));
        if(fast_exact_is_equal(range[0], range[1]))
            PLWARNING("In EarlyStoppingOracle::build_ - no range selected. "
                    "Maybe you forgot that the end part of the range is not "
                    "included!");
    }
    if(option_values.length()==0)
        PLWARNING("In EarlyStoppingOracle::build_ - no range selected.");
  
}

// ### Nothing to add here, simply calls build_
void EarlyStoppingOracle::build()
{
    inherited::build();
    build_();
}

TVec<string>  EarlyStoppingOracle::getOptionNames() const
{ return TVec<string>(1,option); }

TVec<string> EarlyStoppingOracle::generateNextTrial(const TVec<string>& older_trial, real obtained_objective)
{
    if(met_early_stopping)
        return TVec<string>();

    if(older_trial.length()>0)
    {
        real current_objective = obtained_objective;
        int current_step = nreturned;

        if(current_objective<best_objective && current_step >= min_n_steps)
        {
            best_objective = current_objective;
            best_step = current_step;
        }

        real improvement = previous_objective-current_objective;
        real degradation = current_objective-best_objective;

        int n_degraded_steps = current_step-best_step;

        // Check if early-stopping condition was met
        if (( (current_objective < min_value) ||
              (current_objective > max_value) ||
              (n_degraded_steps >= max_degraded_steps) ||
              (degradation > max_degradation) ||
              (relative_max_degradation>=0 && degradation > relative_max_degradation * abs(best_objective)) ||
              (improvement < min_improvement) ||
              (relative_min_improvement>=0 && improvement < relative_min_improvement * abs(previous_objective))
                ) && current_step >= min_n_steps){
            met_early_stopping = true;

            //print debug info
            if(current_objective < min_value){
                DBG_MODULE_LOG 
                    <<"stopping the learner as: current_objective "
                    <<current_objective
                    <<" < min_value "<<min_value
                    <<endl;
            }
            if(current_objective > max_value){
                DBG_MODULE_LOG
                    <<"stopping the learner as: current_objective ("<<current_objective
                    <<") < max_value ("<<max_value<<")"<<endl;
            }
            if(n_degraded_steps >= max_degraded_steps){
                DBG_MODULE_LOG
                    <<"stopping the learner as:n_degraded_steps("<<
                    n_degraded_steps<<") >= max_degraded_steps("<<
                    max_degraded_steps<<") "
                    <<endl;
            }
            if(degradation > max_degradation){
                DBG_MODULE_LOG
                    <<"stopping the learner as: degradation("<<degradation
                    <<") > max_degradation("<<max_degradation<<")"
                    <<endl;
            }
            if(relative_max_degradation>=0 
               && degradation > relative_max_degradation * abs(best_objective)){
                DBG_MODULE_LOG
                    <<"stopping the learner as: relative_max_degradation>=0 "
                    <<"&& degradation("<<degradation
                    <<") > relative_max_degradation("<<relative_max_degradation
                    <<") * abs(best_objective)("<<best_objective<<")"
                    <<endl;
            }
            if(improvement < min_improvement){
                DBG_MODULE_LOG
                    <<"stopping the learner as: improvement("<<improvement<<") < min_improvement("
                    <<min_improvement<<")"
                    <<endl;
            }
            if(relative_min_improvement>=0 
               && improvement < relative_min_improvement * abs(previous_objective)){
                DBG_MODULE_LOG
                    <<"stopping the learner as: relative_min_improvement("<<relative_min_improvement<<")>=0 "
                    <<endl
                    <<"&& improvement("<<improvement<<") < relative_min_improvement("<<relative_min_improvement<<") * abs(previous_objective"<<previous_objective<<")"
                    <<endl;
            }
        }

        previous_objective = current_objective;
    }

    if(met_early_stopping || nreturned >= option_values.length())
        return TVec<string>();
    else
        return TVec<string>(1, option_values[nreturned++]);
}

void EarlyStoppingOracle::forget()
{
    nreturned = 0;
    previous_objective = FLT_MAX;
    best_objective = FLT_MAX;
    best_step = -1;
    met_early_stopping = false;
}


void EarlyStoppingOracle::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(option_values, copies);
    deepCopyField(values, copies);
    deepCopyField(range, copies);
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
