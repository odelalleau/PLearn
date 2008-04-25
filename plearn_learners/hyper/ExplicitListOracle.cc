
// -*- C++ -*-

// ExplicitListOracle.cc
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

/*! \file ExplicitListOracle.cc */
#include "ExplicitListOracle.h"

namespace PLearn {
using namespace std;

ExplicitListOracle::ExplicitListOracle()
    :OptionsOracle(),
     nreturned(0)
/* ### Initialise all fields to their default value */
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(ExplicitListOracle, "ONE LINE DESCR", "NO HELP");

void ExplicitListOracle::declareOptions(OptionList& ol)
{
    declareOption(ol, "option_names", &ExplicitListOracle::option_names, OptionBase::buildoption,
                  "name of options");

    declareOption(ol, "option_values", &ExplicitListOracle::option_values, OptionBase::buildoption,
                  "A matrix with as many columns as there are options, giving their values");

    declareOption(ol, "nreturned", &ExplicitListOracle::nreturned,
                  OptionBase::learntoption,
                  "The number of returned option");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

TVec<string>  ExplicitListOracle::getOptionNames() const
{ return option_names; }

TVec<string> ExplicitListOracle::generateNextTrial(const TVec<string>& older_trial, real obtained_objective)
{
    if(nreturned < option_values.length())
        return option_values(nreturned++);
    else
        return TVec<string>();
}

void ExplicitListOracle::forget()
{
    nreturned = 0;
}



void ExplicitListOracle::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

// ### Nothing to add here, simply calls build_
void ExplicitListOracle::build()
{
    inherited::build();
    build_();
}


void ExplicitListOracle::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(option_names, copies);
    deepCopyField(option_values, copies);
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
