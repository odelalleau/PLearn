// -*- C++ -*-

// UnconditionalDistribution.cc
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

/*! \file UnconditionalDistribution.cc */


#include "UnconditionalDistribution.h"

namespace PLearn {
using namespace std;

///////////////////////////////
// UnconditionalDistribution //
///////////////////////////////
UnconditionalDistribution::UnconditionalDistribution() 
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(UnconditionalDistribution,
                        "This class is a simplified version of PDistribution for unconditional distributions.",
                        "Its only goal is to hide the conditional side of PDistributions to make it simpler."
    );

////////////////////
// declareOptions //
////////////////////
void UnconditionalDistribution::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &UnconditionalDistribution::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // First call the parent class' declareOptions().
    inherited::declareOptions(ol);

    // And modify some options for unconditional distributions.

    redeclareOption(ol, "outputs_def", &PDistribution::outputs_def, OptionBase::buildoption,
                    "Defines what will be given in output. This is a string where the characters\n"
                    "have the following meaning:\n"
                    "'l'-> log_density, 'd' -> density, 'c' -> cdf, 's' -> survival_fn,\n"
                    "'e' -> expectation, 'v' -> variance.\n"
                    "In lower case they give the value associated with a given observation.\n"
                    "In upper case, a curve is evaluated at regular intervals and produced in\n"
                    "output (as a histogram), only for 'L', 'D', 'C' and 'S'.\n"
                    "The number of curve points is determined by the 'n_curve_points' option.\n"
                    "Note that the upper case letters only work for SCALAR variables."
        );

    /* TODO See what kind of option we must hide now (e.g. input_part?)
    redeclareOption(ol, "conditional_flags", &UnconditionalDistribution::conditional_flags, OptionBase::nosave,
                    "Unused in unconditional distributions.");

    redeclareOption(ol, "provide_input", &UnconditionalDistribution::provide_input, OptionBase::nosave,
                    "Unused in unconditional distributions.");

    redeclareOption(ol, "cond_sort",  &UnconditionalDistribution::cond_sort, OptionBase::nosave,
                    "Unused in unconditional distributions.");
                    */

    redeclareOption(ol, "n_input",  &UnconditionalDistribution::n_input, OptionBase::nosave,
                    "Unused in unconditional distributions.");

    redeclareOption(ol, "n_target",  &UnconditionalDistribution::n_target, OptionBase::nosave,
                    "Unused in unconditional distributions.");

    /*
    redeclareOption(ol, "n_margin",  &UnconditionalDistribution::n_margin, OptionBase::nosave,
                    "Unused in unconditional distributions.");
                    */

}

///////////
// build //
///////////
void UnconditionalDistribution::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void UnconditionalDistribution::build_()
{
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void UnconditionalDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

////////////////////
// setTrainingSet //
////////////////////
void UnconditionalDistribution::setTrainingSet(VMat training_set, bool call_forget) {
    PLearner::setTrainingSet(training_set, call_forget);
}

//////////////
// setInput //
//////////////
void UnconditionalDistribution::setInput(const Vec& input) const {
    PLERROR("setInput not implemented for UnconditionalDistribution");
}

//////////////////////////////////
// updateFromConditionalSorting //
//////////////////////////////////
void UnconditionalDistribution::updateFromConditionalSorting() const {
    PLERROR("updateFromConditionalSorting not implemented for UnconditionalDistribution");
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
