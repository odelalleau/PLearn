// -*- C++ -*-

// FixDond2BinaryVariables.cc
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

/*! \file FixDond2BinaryVariables.cc */

#define PL_LOG_MODULE_NAME "FixDond2BinaryVariables"
#include <plearn/io/pl_log.h>

#include "FixDond2BinaryVariables.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    FixDond2BinaryVariables,
    "Fix binary variables with values 0, 1.0 or possibly is_missing.",
    "Instructions are provided with the binary_variable_instructions option.\n"
);

/////////////////////////
// FixDond2BinaryVariables //
/////////////////////////
FixDond2BinaryVariables::FixDond2BinaryVariables()
{
}
    
////////////////////
// declareOptions //
////////////////////
void FixDond2BinaryVariables::declareOptions(OptionList& ol)
{
    declareOption(ol, "binary_variable_instructions", &FixDond2BinaryVariables::binary_variable_instructions,
                  OptionBase::buildoption,
                  "The instructions to fix the binary variables in the form of field_name : instruction.\n"
                  "Supported instructions are 9_is_one, not_0_is_one, not_missing_is_one, not_1000_is_one.\n"
                  "Variables with no specification will be kept as_is.\n");

    declareOption(ol, "output_path", &FixDond2BinaryVariables::output_path,
                  OptionBase::buildoption,
                  "The file path for the fixed output file.");

    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void FixDond2BinaryVariables::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(binary_variable_instructions, copies);
    deepCopyField(output_path, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);

}

///////////
// build //
///////////
void FixDond2BinaryVariables::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void FixDond2BinaryVariables::build_()
{
    MODULE_LOG << "build_() called" << endl;
    if (train_set)
    {
        fixBinaryVariables();
    }
}

void FixDond2BinaryVariables::fixBinaryVariables()
{    
    // initialize primary dataset
    main_row = 0;
    main_col = 0;
    main_length = train_set->length();
    main_width = train_set->width();
    main_input.resize(main_width);
    main_names.resize(main_width);
    main_ins.resize(main_width);
    ins_width = binary_variable_instructions.size();
    main_names << train_set->fieldNames();
    for (main_col = 0; main_col < main_width; main_col++)
    {
        main_ins[main_col] = "as_is";
    }
    for (ins_col = 0; ins_col < ins_width; ins_col++)
    {
        for (main_col = 0; main_col < main_width; main_col++)
        {
            if (binary_variable_instructions[ins_col].first == main_names[main_col]) break;
        }
        if (main_col >= main_width) PLERROR("In FixDond2BinaryVariables: no field with this name in train data set: %s", (binary_variable_instructions[ins_col].first).c_str());
        if (binary_variable_instructions[ins_col].second == "9_is_one") main_ins[main_col] = "9_is_one";
        else if (binary_variable_instructions[ins_col].second == "not_0_is_one") main_ins[main_col] = "not_0_is_one";
        else if (binary_variable_instructions[ins_col].second == "not_missing_is_one") main_ins[main_col] = "not_missing_is_one";
        else if (binary_variable_instructions[ins_col].second == "not_1000_is_one") main_ins[main_col] = "not_1000_is_one";
        else PLERROR("In FixDond2BinaryVariables: unsupported instruction: %s", (binary_variable_instructions[ins_col].second).c_str());
    }
    
    // initialize output datasets
    output_file = new FileVMatrix(output_path + ".pmat", main_length, main_names);
    output_file->defineSizes(main_width, 0, 0);
    
    //Now, we can process the binary variables.
    ProgressBar* pb = 0;
    pb = new ProgressBar( "Fixing the binary variables", main_length);
    for (main_row = 0; main_row < main_length; main_row++)
    {
        train_set->getRow(main_row, main_input);
        for (main_col = 0; main_col < main_width; main_col++)
        {
            if (main_ins[main_col] == "not_missing_is_one")
            {
                if (is_missing(main_input[main_col])) main_input[main_col] = 0.0;
                else  main_input[main_col] = 1.0;
            }
            else if (main_ins[main_col] == "not_0_is_one")
            {
                if (is_missing(main_input[main_col])) continue;
                if (main_input[main_col] != 0.0)  main_input[main_col] = 1.0;
                else  main_input[main_col] = 0.0;
            }
            else if (main_ins[main_col] == "9_is_one")
            {
                if (is_missing(main_input[main_col])) continue;
                if (main_input[main_col] == 9.0)  main_input[main_col] = 1.0;
                else  main_input[main_col] = 0.0;
            }
            else if (main_ins[main_col] == "not_1000_is_one")
            {
                if (is_missing(main_input[main_col])) continue;
                if (main_input[main_col] != -1000.0)  main_input[main_col] = 1.0;
                else  main_input[main_col] = 0.0;
            }
        }
        output_file->putRow(main_row, main_input);
        pb->update( main_row );
    }
    delete pb;
}

VMat FixDond2BinaryVariables::getOutputFile()
{
    return output_file;
}

int FixDond2BinaryVariables::outputsize() const {return 0;}
void FixDond2BinaryVariables::train()
{
    PLERROR("FixDond2BinaryVariables: we are done here");
}
void FixDond2BinaryVariables::computeOutput(const Vec&, Vec&) const {}
void FixDond2BinaryVariables::computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const {}
TVec<string> FixDond2BinaryVariables::getTestCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
}
TVec<string> FixDond2BinaryVariables::getTrainCostNames() const
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
