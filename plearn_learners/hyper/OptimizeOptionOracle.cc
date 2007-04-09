// -*- C++ -*-

// OptimizeOptionOracle.cc
//
// Copyright (C) 2004 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file OptimizeOptionOracle.cc */

#include "OptimizeOptionOracle.h"
#include <plearn/math/random.h>       //!< For uniform_sample.
#include <plearn/base/stringutils.h>  //!< For tostring.

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(OptimizeOptionOracle,
                        "This Oracle tries to optimize a given option (which must be a real number).",
                        "It starts from the value provided by the user, then tries higher and lower\n"
                        "values in order to try and find the minimum cost.\n"
                        "The algorithm employed is very basic, and subject to fall into local minima.\n"
    );

//////////////////////////
// OptimizeOptionOracle //
//////////////////////////
OptimizeOptionOracle::OptimizeOptionOracle()
    : current_direction("not_started"),
      lower_bound(1),
      n_steps(0),
      upper_bound(-1),
      factor(2),
      max_steps(10),
      max_value(REAL_MAX),
      min_value(-REAL_MAX),
      relative_precision(0.5),
      option(""),
      start_direction("random"),
      start_value(0)
{}

////////////////////
// declareOptions //
////////////////////
void OptimizeOptionOracle::declareOptions(OptionList& ol)
{
    declareOption(ol, "option", &OptimizeOptionOracle::option, OptionBase::buildoption,
                  "The name of the option to optimize.");

    declareOption(ol, "max_steps", &OptimizeOptionOracle::max_steps, OptionBase::buildoption,
                  "The maximum number of steps performed.");

    declareOption(ol, "start_value", &OptimizeOptionOracle::start_value, OptionBase::buildoption,
                  "The value we start from for this option.");

    declareOption(ol, "min_value", &OptimizeOptionOracle::min_value, OptionBase::buildoption,
                  "The minimum value that should be tried.");

    declareOption(ol, "max_value", &OptimizeOptionOracle::max_value, OptionBase::buildoption,
                  "The maximum value that should be tried.");

    declareOption(ol, "relative_precision", &OptimizeOptionOracle::relative_precision, OptionBase::buildoption,
                  "The precision to which we want to find the optimal option. For instance\n"
                  "0.1 would indicate we want the 'best' value to be found in an interval\n"
                  "of the kind [best - 10% * best, best + 10% * best].");

    declareOption(ol, "factor", &OptimizeOptionOracle::factor, OptionBase::buildoption,
                  "The factor by which we multiply / divide the current value after\n"
                  "we started from 'start_value'.");

    declareOption(ol, "start_direction", &OptimizeOptionOracle::start_direction, OptionBase::buildoption,
                  "The direction we start going to ('up', 'down' or 'random').");

    //***************

    declareOption(ol, "best", &OptimizeOptionOracle::best, OptionBase::learntoption,
                  "The best value found so far.");

    declareOption(ol, "best_objective", &OptimizeOptionOracle::best_objective, OptionBase::learntoption,
                  "The objective obtained with option = 'best'.");

    declareOption(ol, "current_direction", &OptimizeOptionOracle::current_direction, OptionBase::learntoption,
                  "The current direction we are going ('up', 'down' or 'not_started').");

    declareOption(ol, "lower_bound", &OptimizeOptionOracle::lower_bound, OptionBase::learntoption,
                  "The lower bound of the interval in which we working.");

    declareOption(ol, "n_steps", &OptimizeOptionOracle::n_steps, OptionBase::learntoption,
                  "The number of steps performed so far.");

    declareOption(ol, "upper_bound", &OptimizeOptionOracle::upper_bound, OptionBase::learntoption,
                  "The upper bound of the interval in which we working.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void OptimizeOptionOracle::build() {
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void OptimizeOptionOracle::build_()
{
    if (min_value > max_value) {
        PLERROR("In OptimizeOptionOracle::build_ - You specified a min_value higher than max_value");
    }
    if (lower_bound > upper_bound) {
        // Should only happen when this object is built for the first time.
        lower_bound = min_value;
        upper_bound = max_value;
    }
}

////////////
// forget //
////////////
void OptimizeOptionOracle::forget()
{
    current_direction = "not_started";
    lower_bound = min_value;
    upper_bound = max_value;
    n_steps = 0;
}

////////////////////
// getOptionNames //
////////////////////
TVec<string> OptimizeOptionOracle::getOptionNames() const {
    return TVec<string>(1,option);
}

///////////////////////
// generateNextTrial //
///////////////////////
TVec<string> OptimizeOptionOracle::generateNextTrial(const TVec<string>& older_trial, real obtained_objective)
{
    if (n_steps >= max_steps) {
        // Time to stop.
        TVec<string> empty(0);
        return empty;
    } else {
        n_steps++;
    }

    if (older_trial.length() == 0) {
        // This is the first try: we just start with the given start value.
        best = start_value;
        best_objective = REAL_MAX;
        return TVec<string>(1, tostring(start_value));
    }

    // We can stop if the interval is restrained enough.
    if (lower_bound >= best - (best * relative_precision) &&
        upper_bound <= best + (best * relative_precision)) {
        TVec<string> empty(0);
        return empty;
    }

    real last = toreal(older_trial[0]); // The last value tried
    bool improved = false;
    if (obtained_objective < best_objective) {
        improved = true;
        best_objective = obtained_objective;
    }

    if (current_direction == "not_started") {
        // This is the second try.
        if (start_direction == "random") {
            // Need to start with a random direction.
            real t = uniform_sample();
            if (t < 0.5) {
                current_direction = "up";
            } else {
                current_direction = "down";
            }
        } else {
            current_direction = start_direction;
        }
    } else {
        // Find out in which direction we should go now.
        if (improved) {
            // Going in the 'current_direction' helped, let's keep going.
            if (current_direction == "up") {
                lower_bound = best;
            } else {
                upper_bound = best;
            }
        } else {
            // Going in the 'current_direction' didn't help. We go the other way.
            if (current_direction == "up") {
                current_direction = "down";
                upper_bound = last;
            } else {
                current_direction = "up";
                lower_bound = last;
            }
        }
    }

    if (improved) {
        best = last;
    }

    real next; // The next value we are going to try.
    if (current_direction == "up") {
        if (best * factor < upper_bound*0.99) {
            // We can try much higher (the 0.99 is there to ensure we do not indefinitely
            // try the same 'upper_bound' value).
            next = best * factor;
        } else {
            // We take the middle point between 'best' and 'upper_bound'.
            next = (best + upper_bound) / 2;
        }
    } else if (current_direction == "down") {
        if (best / factor > lower_bound*1.01) {
            // We can try much lower.
            next = best / factor;
        } else {
            // We take the middle point between 'best' and 'lower_bound'.
            next = (best + lower_bound) / 2;
        }
    } else {
        PLERROR("In OptimizeOptionOracle::generateNextTrial - Wrong value for 'current_direction'");
    }
    return TVec<string>(1, tostring(next));
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void OptimizeOptionOracle::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("OptimizeOptionOracle::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
