// -*- C++ -*-

// CartesianProductOracle.cc
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

/* *******************************************************
 * $Id$
 ******************************************************* */

/*! \file CartesianProductOracle.cc */
#include "CartesianProductOracle.h"

namespace PLearn {
using namespace std;

CartesianProductOracle::CartesianProductOracle()
    :OptionsOracle(), last_combination(false)
/* ### Initialise all fields to their default value */
{
}

PLEARN_IMPLEMENT_OBJECT(
    CartesianProductOracle,
    "This OptionsOracle generates all combinations of values for a set of options",
    "This 'oracle' traverses a fixed list of option values, obtained from a set"
    "of values associated with each option of interest. That list is the cartesian"
    "product of those sets, i.e. it is the list of all combinations of values."
    "The user specifies the names of each option, and for each of them, a list"
    "of the values that should be tried."
    );

void CartesianProductOracle::declareOptions(OptionList& ol)
{
    declareOption(ol, "option_names", &CartesianProductOracle::option_names, OptionBase::buildoption,
                  "name of each of the options to optimize");

    declareOption(ol, "option_values", &CartesianProductOracle::option_values, OptionBase::buildoption,
                  "A list of lists of options: the top list must have as many elements as there are"
                  "options in the option_names field. Each sub-list contains the values to be tried"
                  "for the corresponding option."
        );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

TVec<string>  CartesianProductOracle::getOptionNames() const
{
    return option_names;
}

TVec<string> CartesianProductOracle::generateNextTrial(const TVec<string>& older_trial,
                                                       real obtained_objective)
{
    if (last_combination)
        return TVec<string>();
    else
    {
        int n=option_names.length();
        TVec<string> values(n);
        // copy current combination
        for (int i=0;i<n;i++)
            values[i]=option_values[i][option_values_indices[i]];
        // increment indices to next combination, in lexicographical order
        last_combination=true;
        for (int i=0;i<n;i++)
        {
            option_values_indices[i]++;
            if (option_values_indices[i]<option_values[i].length()) { last_combination=false; break; }
            else option_values_indices[i]=0;
        }
        return values;
    }
}

void CartesianProductOracle::forget()
{
    option_values_indices.clear();
    last_combination=false;
}

void CartesianProductOracle::build_()
{
    // Ensure consistency between option_names and option_values
    if (option_names.size() != option_values.size())
        PLERROR("CartesianProductOracle::build_: the 'option_names' and 'option_values'\n"
                "fields don't have the same length; len(option_names)=%d / len(option_values)=%d",
                option_names.size(), option_values.size());
    if (option_names.size() == 0 || option_values.size() == 0)
        PLWARNING("CartesianProductOracle::build_: either 'option_names' or 'option_values'\n"
                  "has size zero; is this what you want?");

    // Try to detect zero-length subarrays of options
    for (int i=0, n=option_values.size() ; i<n ; ++i)
        if (option_values[i].size() == 0)
            PLWARNING("CartesianProductOracle::build_: zero option values were specified\n"
                      "for option '%s'", option_names[i].c_str());

    int n=option_names.length();
    option_values_indices.resize(n);
    forget();
}

// ### Nothing to add here, simply calls build_
void CartesianProductOracle::build()
{
    inherited::build();
    build_();
}


void CartesianProductOracle::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(option_values_indices, copies);
    deepCopyField(option_values, copies);
    deepCopyField(option_names, copies);
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
