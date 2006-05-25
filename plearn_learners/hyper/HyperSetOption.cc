// -*- C++ -*-

// HyperSetOption.cc
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

/*! \file HyperSetOption.cc */

#include "HyperSetOption.h"
#include "HyperLearner.h"

namespace PLearn {
using namespace std;

HyperSetOption::HyperSetOption()
{
}

PLEARN_IMPLEMENT_OBJECT(
    HyperSetOption,
    "HyperCommand to set an object option during HyperOptimization",
    "");

void HyperSetOption::declareOptions(OptionList& ol)
{
    declareOption(ol, "option_name", &HyperSetOption::option_name,
                  OptionBase::buildoption,
                  "Name of single option to set");

    declareOption(ol, "option_value", &HyperSetOption::option_value,
                  OptionBase::buildoption,
                  "Value of option to set");

    declareOption(ol, "options", &HyperSetOption::options,
                  OptionBase::buildoption,
                  "List of pairs  \"optionname\":\"optionvalue\"  to set");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void HyperSetOption::build_()
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
void HyperSetOption::build()
{
    inherited::build();
    build_();
}

void HyperSetOption::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(options, copies);
}

Vec HyperSetOption::optimize()
{
    TVec<string> names, values;
    if (option_name != "") {
        names.push_back(option_name);
        values.push_back(option_value);
    }
    for (int i=0; i<options.size(); ++i) {
        names.push_back(options[i].first);
        values.push_back(options[i].second);
    }
    hlearner->setLearnerOptions(names, values);
    return Vec();
}

TVec<string> HyperSetOption::getResultNames() const
{
    return TVec<string>();
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
