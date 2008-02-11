// -*- C++ -*-

// DichotomizeDond2DiscreteVariables.cc
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

/*! \file DichotomizeDond2DiscreteVariables.cc */

#define PL_LOG_MODULE_NAME "DichotomizeDond2DiscreteVariables"

#include "DichotomizeDond2DiscreteVariables.h"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DichotomizeDond2DiscreteVariables,
    "Dichotomize variables with discrete values.",
    "Instructions are provided with the discrete_variable_instructions option.\n"
    "DEPRECATED use DichotomizeVMatrix.cc instead"
);

/////////////////////////
// DichotomizeDond2DiscreteVariables //
/////////////////////////
DichotomizeDond2DiscreteVariables::DichotomizeDond2DiscreteVariables()
{
}
    
////////////////////
// declareOptions //
////////////////////
void DichotomizeDond2DiscreteVariables::declareOptions(OptionList& ol)
{
    declareOption(ol, "discrete_variable_instructions", &DichotomizeDond2DiscreteVariables::discrete_variable_instructions,
                  OptionBase::buildoption,
                  "The instructions to dichotomize the variables in the form of field_name : TVec<pair>.\n"
                  "The pairs are values from : to, each creating a 0, 1 variable.\n"
                  "Variables with no specification will be kept as_is.\n");

    declareOption(ol, "output_path", &DichotomizeDond2DiscreteVariables::output_path,
                  OptionBase::buildoption,
                  "The file path for the fixed output file.");

    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void DichotomizeDond2DiscreteVariables::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(discrete_variable_instructions, copies);
    deepCopyField(output_path, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);

}

///////////
// build //
///////////
void DichotomizeDond2DiscreteVariables::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void DichotomizeDond2DiscreteVariables::build_()
{
    MODULE_LOG << "build_() called" << endl;
    if (train_set)
    {
        dichotomizeDiscreteVariables();
    }
}

void DichotomizeDond2DiscreteVariables::dichotomizeDiscreteVariables()
{    
    // initialize primary dataset
    int main_length = train_set->length();
    int main_width = train_set->width();
    Vec main_input(main_width);
    TVec<string> main_names(main_width);
    TVec<int> main_ins(main_width);
    main_ins.fill(-1);
    int ins_width = discrete_variable_instructions.size();
    main_names << train_set->fieldNames();
    for (int ins_col = 0; ins_col < ins_width; ins_col++)
    {
        int main_col;
        for (main_col = 0; main_col < main_width; main_col++)
        {
            if (discrete_variable_instructions[ins_col].first == main_names[main_col]) break;
        }
        if (main_col >= main_width) PLERROR("In DichotomizeDond2DiscreteVariables: no field with this name in data set: %s", (discrete_variable_instructions[ins_col].first).c_str());
        else main_ins[main_col] = ins_col;
    }
    
    // initialize output datasets
    int output_length = main_length;
    int output_width = 0;
    for (int main_col = 0; main_col < main_width; main_col++)
    {
        if (main_ins[main_col] < 0) output_width += 1;
        else
        {
            TVec<pair<real, real> > instruction_ptr = discrete_variable_instructions[main_ins[main_col]].second;
            output_width += instruction_ptr.size();
        }
    }
    TVec<string> output_names(output_width);
    int output_col = 0;
    for (int main_col = 0; main_col < main_width; main_col++)
    {
        if (main_ins[main_col] < 0)
        {
            output_names[output_col] = main_names[main_col];
            output_col += 1;
        }
        else
        {
           TVec<pair<real, real> > instruction_ptr = discrete_variable_instructions[main_ins[main_col]].second;
           if (instruction_ptr.size() == 0) continue;
           for (int ins_col = 0; ins_col < instruction_ptr.size(); ins_col++)
           {
               output_names[output_col] = main_names[main_col] + "_"
                                        + tostring(instruction_ptr[ins_col].first) + "_" 
                                        + tostring(instruction_ptr[ins_col].second);
               output_col += 1;
           }
        }    
    }
    output_file = new FileVMatrix(output_path + ".pmat", output_length, output_names);
    output_file->defineSizes(output_width, 0, 0);
    
    //Now, we can process the discrete variables.
    ProgressBar* pb = 0;
    pb = new ProgressBar( "Dichotomizing the discrete variables", main_length);
    Vec output_record(output_width);

    for (int main_row = 0; main_row < main_length; main_row++)
    {
        train_set->getRow(main_row, main_input);
        output_col = 0;
        for (int main_col = 0; main_col < main_width; main_col++)
        {
            if (main_ins[main_col] < 0)
            {
                output_record[output_col] = main_input[main_col];
                output_col += 1;
            }
            else
            {
               TVec<pair<real, real> > instruction_ptr = discrete_variable_instructions[main_ins[main_col]].second;
               if (instruction_ptr.size() == 0) continue;
               for (int ins_col = 0; ins_col < instruction_ptr.size(); ins_col++)
               {
                   if (is_missing(main_input[main_col])) output_record[output_col] = MISSING_VALUE;
                   else if (main_input[main_col] < instruction_ptr[ins_col].first || main_input[main_col] > instruction_ptr[ins_col].second) output_record[output_col] = 0.0;
                   else output_record[output_col] = 1.0;
                   output_col += 1;
               }
            }    
        }
        output_file->putRow(main_row, output_record);
        pb->update( main_row );
    }
    delete pb;
}

VMat DichotomizeDond2DiscreteVariables::getOutputFile()
{
    return output_file;
}

int DichotomizeDond2DiscreteVariables::outputsize() const {return 0;}
void DichotomizeDond2DiscreteVariables::train()
{
        PLERROR("DichotomizeDond2DiscreteVariables: we are done here");
}
void DichotomizeDond2DiscreteVariables::computeOutput(const Vec&, Vec&) const {}
void DichotomizeDond2DiscreteVariables::computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const {}
TVec<string> DichotomizeDond2DiscreteVariables::getTestCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
}
TVec<string> DichotomizeDond2DiscreteVariables::getTrainCostNames() const
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
