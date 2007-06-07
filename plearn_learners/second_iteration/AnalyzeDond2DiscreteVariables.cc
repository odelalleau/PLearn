// -*- C++ -*-

// AnalyzeDond2DiscreteVariables.cc
//
// Copyright (C) 2006 Dan Popovici, Pascal Lamblin
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

// Authors: Dan Popovici

/*! \file AnalyzeDond2DiscreteVariables.cc */

#define PL_LOG_MODULE_NAME "AnalyzeDond2DiscreteVariables"
#include <plearn/io/pl_log.h>

#include "AnalyzeDond2DiscreteVariables.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    AnalyzeDond2DiscreteVariables,
    "Computes correlation coefficient between various discrete values and the target.",
    "name of the discrete variable, of the target and the values to check are options.\n"
);

/////////////////////////
// AnalyzeDond2DiscreteVariables //
/////////////////////////
AnalyzeDond2DiscreteVariables::AnalyzeDond2DiscreteVariables()
{
}
    
////////////////////
// declareOptions //
////////////////////
void AnalyzeDond2DiscreteVariables::declareOptions(OptionList& ol)
{

    declareOption(ol, "variable_name", &AnalyzeDond2DiscreteVariables::variable_name,
                  OptionBase::buildoption,
                  "The field name of the variable to be analyzed.");

    declareOption(ol, "target_name", &AnalyzeDond2DiscreteVariables::target_name,
                  OptionBase::buildoption,
                  "The field name of the target.");

    declareOption(ol, "values_to_analyze", &AnalyzeDond2DiscreteVariables::values_to_analyze,
                  OptionBase::buildoption,
                  "The vector of values to check the correlation with the target.\n"
                  "The algorithm groups the values from, to of each pair specified.\n");

    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void AnalyzeDond2DiscreteVariables::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(values_to_analyze, copies);
    deepCopyField(variable_name, copies);
    deepCopyField(target_name, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);

}

///////////
// build //
///////////
void AnalyzeDond2DiscreteVariables::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void AnalyzeDond2DiscreteVariables::build_()
{
    MODULE_LOG << "build_() called" << endl;
    if (train_set)
    {
        analyzeDiscreteVariable();
        PLERROR("AnalyzeDond2DiscreteVariables: we are done here");
    }
}

void AnalyzeDond2DiscreteVariables::analyzeDiscreteVariable()
{    
    // initialize primary dataset
    main_row = 0;
    main_col = 0;
    main_length = train_set->length();
    main_width = train_set->width();
    main_input.resize(main_width);
    main_names.resize(main_width);
    main_names << train_set->fieldNames();
    
    // check for valid options
    number_of_values = values_to_analyze.size();
    variable_col = -1;
    target_col = -1;
    for (main_col = 0; main_col < main_width; main_col++)
    {
        if (variable_name == main_names[main_col]) variable_col = main_col;
        if (target_name == main_names[main_col]) target_col = main_col;
    }
    if (variable_col < 0) PLERROR("In AnalyzeDond2DiscreteVariables: variable name not found: %s", variable_name.c_str());
    if (target_col < 0) PLERROR("In AnalyzeDond2DiscreteVariables: target name not found: %s", target_name.c_str());
    if (number_of_values <= 0) PLERROR("In AnalyzeDond2DiscreteVariables: invalid values_to_analyze");
    
    // initialize working variables
    value_target_sum.resize(number_of_values);
    value_present_count.resize(number_of_values);
    value_target_sum.clear();
    value_present_count.clear();
    target_sum = 0.0;
    target_squared_sum = 0.0;
    variable_present_count = 0.0;
    
    //Now, we can process the discrete variable.
    ProgressBar* pb = 0;
    pb = new ProgressBar( "Analyzing discrete variable " + variable_name, main_length);
    for (main_row = 0; main_row < main_length; main_row++)
    {
        train_set->getRow(main_row, main_input);
        variable_value = main_input[variable_col];
        if (is_missing(variable_value)) continue;
        target_value = main_input[target_col];
        target_sum += target_value;
        target_squared_sum += target_value * target_value;
        variable_present_count += 1.0;
        for (value_col = 0; value_col < number_of_values; value_col++)
        {
            if (variable_value < values_to_analyze[value_col].first || variable_value > values_to_analyze[value_col].second) continue;
            value_target_sum[value_col] += target_value;
            value_present_count[value_col] += 1.0;
        }
        pb->update( main_row );
    }
    delete pb;
    if (variable_present_count <= 0.0)
    {
        cout << "In AnalyzeDond2DiscreteVariables: no value present for this variable" << endl;
        return;
    }
    target_mean = target_sum / variable_present_count;
    cout << "In AnalyzeDond2DiscreteVariables, for variable:  " << variable_name << endl;
    cout << variable_present_count << " values are present out of " << main_length << " samples." << endl;
    for (value_col = 0; value_col < number_of_values; value_col++)
    {
        ssxy = value_target_sum[value_col] - value_present_count[value_col] * target_mean;
        ss2xy = ssxy * ssxy;
        ssxx = value_present_count[value_col] * (1.0 -  value_present_count[value_col] / variable_present_count);
        ssyy = target_squared_sum - target_sum * target_mean;
        correlation_coefficient = ss2xy / (ssxx * ssyy);
        cout << "For value from: " << values_to_analyze[value_col].first << " to: " << values_to_analyze[value_col].second 
             << " occurence: " << value_present_count[value_col] << " correlation coefficient: " << correlation_coefficient << endl;
    }
}

int AnalyzeDond2DiscreteVariables::outputsize() const {return 0;}
void AnalyzeDond2DiscreteVariables::train() {}
void AnalyzeDond2DiscreteVariables::computeOutput(const Vec&, Vec&) const {}
void AnalyzeDond2DiscreteVariables::computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const {}
TVec<string> AnalyzeDond2DiscreteVariables::getTestCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
}
TVec<string> AnalyzeDond2DiscreteVariables::getTrainCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
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
