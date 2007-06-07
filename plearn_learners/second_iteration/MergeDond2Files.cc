// -*- C++ -*-

// MergeDond2Files.cc
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

/*! \file MergeDond2Files.cc */

#define PL_LOG_MODULE_NAME "MergeDond2Files"
#include <plearn/io/pl_log.h>

#include "MergeDond2Files.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    MergeDond2Files,
    "Merge two files on specified keys.",
    "If more than 1 matching record is found in the secondary datatset,\n"
    "mean, mode or variable presence will be appended based on variable merge instruction.\n"
    "a variable from the secondary dataset may also be skipped.\n"
);

/////////////////////////
// MergeDond2Files //
/////////////////////////
MergeDond2Files::MergeDond2Files()
{
}
    
////////////////////
// declareOptions //
////////////////////
void MergeDond2Files::declareOptions(OptionList& ol)
{
    declareOption(ol, "external_dataset", &MergeDond2Files::external_dataset,
                  OptionBase::buildoption,
                  "The secondary dataset to merge with the main one.\n"
                  "The main one is provided as the train_set.");

    declareOption(ol, "missing_instructions", &MergeDond2Files::missing_instructions,
                  OptionBase::buildoption,
                  "The variable missing regeneration instructions in the form of pairs field : instruction.\n"
                  "Supported instructions are skip, as_is, zero_is_missing, 2436935_is_missing, present.");

    declareOption(ol, "merge_instructions", &MergeDond2Files::merge_instructions,
                  OptionBase::buildoption,
                  "The variable merge instructions in the form of pairs field : instruction.\n"
                  "Supported instructions are skip, mean, mode, present.");

    declareOption(ol, "merge_path", &MergeDond2Files::merge_path,
                  OptionBase::buildoption,
                  "The root name of merge files to be created.\n"
                  "3 files will be created: a root_train.pmat, a root_test.pmat and a root_uknown.pmat");

    declareOption(ol, "sec_key", &MergeDond2Files::sec_key,
                  OptionBase::buildoption,
                  "The column of the merge key in the secondary dataset.");

    declareOption(ol, "main_key", &MergeDond2Files::main_key,
                  OptionBase::buildoption,
                  "The column of the merge key in the main dataset.");

    declareOption(ol, "train_ind", &MergeDond2Files::train_ind,
                  OptionBase::buildoption,
                  "The column of the indicator of a train record in the main dataset.");

    declareOption(ol, "test_ind", &MergeDond2Files::test_ind,
                  OptionBase::buildoption,
                  "The column of the indicator of a test record in the main dataset.");

    declareOption(ol, "train_file", &MergeDond2Files::train_file,
                  OptionBase::learntoption,
                  "The train file created.");

    declareOption(ol, "test_file", &MergeDond2Files::test_file,
                  OptionBase::learntoption,
                  "The test file created.");

    declareOption(ol, "unknown_file", &MergeDond2Files::unknown_file,
                  OptionBase::learntoption,
                  "The unknown target file created.");


    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void MergeDond2Files::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(external_dataset, copies);
    deepCopyField(missing_instructions, copies);
    deepCopyField(merge_instructions, copies);
    deepCopyField(merge_path, copies);
    deepCopyField(sec_key, copies);
    deepCopyField(main_key, copies);
    deepCopyField(train_ind, copies);
    deepCopyField(test_ind, copies);
    deepCopyField(train_file, copies);
    deepCopyField(test_file, copies);
    deepCopyField(unknown_file, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);

}

///////////
// build //
///////////
void MergeDond2Files::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void MergeDond2Files::build_()
{
    MODULE_LOG << "build_() called" << endl;
    if (train_set)
    {
        mergeFiles();
        PLERROR("MergeDond2Files: we are done here");
    }
}

void MergeDond2Files::mergeFiles()
{
    // initialization with merge instructions
    sec_row = 0;
    sec_col = 0;
    sec_length = external_dataset->length();
    sec_width = external_dataset->width();
    sec_names.resize(sec_width);
    sec_ins.resize(sec_width);
    sec_input.resize(sec_width);
    ins_col = 0;
    extension_width = 0;
    sec_names << external_dataset->fieldNames();
    for (sec_col = 0; sec_col < sec_width; sec_col++)
    {
        sec_ins[sec_col] = "mean";
    }
    for (ins_col = 0; ins_col < merge_instructions.size(); ins_col++)
    {
        for (sec_col = 0; sec_col < sec_width; sec_col++)
        {
            if (merge_instructions[ins_col].first == sec_names[sec_col]) break;
        }
        if (sec_col >= sec_width) PLERROR("In MergeDond2Files: no field with this name in external_dataset data set: %", (merge_instructions[ins_col].first).c_str());
        if (merge_instructions[ins_col].second == "skip") sec_ins[sec_col] = "skip";
        else if (merge_instructions[ins_col].second == "mean") sec_ins[sec_col] = "mean";
        else if (merge_instructions[ins_col].second == "mode") sec_ins[sec_col] = "mode";
        else if (merge_instructions[ins_col].second == "present") sec_ins[sec_col] = "present";
        else PLERROR("In MergeDond2Files: unsupported merge instruction: %", (merge_instructions[ins_col].second).c_str());
        if (sec_ins[sec_col] != "skip") extension_width += 1;
    }
    ext_col = 0;
    extension_pos.resize(sec_width);
    extension_names.resize(extension_width);
    for (sec_col = 0; sec_col < sec_width; sec_col++)
    {
        if (sec_ins[sec_col] == "skip")
        {
            extension_pos[sec_col] = -1; 
        }
        else
        {
            extension_pos[sec_col] = ext_col;
            extension_names[ext_col] = sec_names[sec_col];
            ext_col += 1;
        }
    }
    sec_values.resize(extension_width, 10);
    sec_value_cnt.resize(extension_width, 10);
    sec_value_ind.resize(extension_width);
    sec_values.clear();
    sec_value_cnt.clear();
    sec_value_ind.clear();
    external_dataset->getRow(sec_row, sec_input);
    
    // initialize primary dataset
    main_row = 0;
    main_col = 0;
    main_length = train_set->length();
    main_width = train_set->width();
    main_input.resize(main_width);
    main_names.resize(main_width);
    main_ins.resize(main_width);
    main_names << train_set->fieldNames();
    primary_width = 0;
    for (main_col = 0; main_col < main_width; main_col++)
    {
        main_ins[main_col] = "as_is";
    }
    for (ins_col = 0; ins_col < missing_instructions.size(); ins_col++)
    {
        for (main_col = 0; main_col < main_width; main_col++)
        {
            if (missing_instructions[ins_col].first == main_names[main_col]) break;
        }
        if (main_col >= main_width) PLERROR("In MergeDond2Files: no field with this name in external_dataset data set: %", (missing_instructions[ins_col].first).c_str());
        if (missing_instructions[ins_col].second == "skip") main_ins[main_col] = "skip";
        else if (missing_instructions[ins_col].second == "as_is") main_ins[main_col] = "as_is";
        else if (missing_instructions[ins_col].second == "zero_is_missing") main_ins[main_col] = "zero_is_missing";
        else if (missing_instructions[ins_col].second == "2436935_is_missing") main_ins[main_col] = "2436935_is_missing";
        else if (missing_instructions[ins_col].second == "present") main_ins[main_col] = "present";
        else PLERROR("In MergeDond2Files: unsupported merge instruction: %", (missing_instructions[ins_col].second).c_str());
        if (main_ins[main_col] != "skip") primary_width += 1;
    }
    prim_col = 0;
    primary_names.resize(primary_width);
    for (main_col = 0; main_col < main_width; main_col++)
    {
        if (main_ins[main_col] != "skip")
        {
            primary_names[prim_col] = main_names[main_col];
            prim_col += 1;
        }
    }
    
    // initialize output datasets
    merge_col = 0;
    merge_width = primary_width + extension_width;
    merge_output.resize(merge_width);
    merge_names.resize(merge_width);
    for (prim_col = 0; prim_col < primary_width; prim_col++)
    {
       merge_names[merge_col] = primary_names[prim_col];
       merge_col +=1;
    }
    for (ext_col = 0; ext_col < extension_width; ext_col++)
    {
       merge_names[merge_col] = extension_names[ext_col];
       merge_col +=1;
    }
    train_length = 0;
    test_length = 0;
    unknown_length = 0;
    ProgressBar* pb = 0;
    pb = new ProgressBar( "Counting the number of records in the train, test and unknown datasets", main_length);
    for (main_row = 0; main_row < main_length; main_row++)
    {
        train_set->getRow(main_row, main_input);
        if (main_input[train_ind] > 0.0) train_length += 1;
        else if (main_input[test_ind] > 0.0) test_length += 1;
        else unknown_length += 1;
        pb->update( main_row );
    }
    delete pb;
    train_file = new FileVMatrix(merge_path + "_train.pmat", train_length, merge_width);
    test_file = new FileVMatrix(merge_path + "_test.pmat", test_length, merge_width);
    unknown_file = new FileVMatrix(merge_path + "_unknown.pmat", unknown_length, merge_width);
    train_file->declareFieldNames(merge_names);
    test_file->declareFieldNames(merge_names);
    unknown_file->declareFieldNames(merge_names);
    train_row = 0;
    test_row = 0;
    unknown_row = 0;
    
    //Now, we can merge
    pb = new ProgressBar( "Merging primary and secondary datasets with instructions", main_length);
    for (main_row = 0; main_row < main_length; main_row++)
    {
        train_set->getRow(main_row, main_input);
        if (sec_row >= sec_length)
        {
            combineAndPut();
            continue;
        }
        while (sec_input[sec_key] < main_input[main_key])
        {
            sec_row += 1;
            if (sec_row >= sec_length) break;   
            external_dataset->getRow(sec_row, sec_input);
        }
        while (sec_input[sec_key] == main_input[main_key])
        {
            accumulateVec();
            sec_row += 1;
            if (sec_row >= sec_length) break;   
            external_dataset->getRow(sec_row, sec_input);
        }
        combineAndPut();
        pb->update( main_row );
    }
    delete pb;
}

void MergeDond2Files::accumulateVec()
{
    for (sec_col = 0; sec_col < sec_width; sec_col++)
    {
        if (is_missing(sec_input[sec_col])) continue;
        if (sec_ins[sec_col] == "skip") continue;
        ext_col = extension_pos[sec_col];
        if (sec_ins[sec_col] == "mean")
        {
            sec_values(ext_col, 0) += sec_input[sec_col];
            sec_value_cnt(ext_col, 0) += 1.0;
        }
        if (sec_ins[sec_col] == "mode")
        {
            sec_value_found = false;
            for (sec_value_col = 0; sec_value_col < sec_value_ind[sec_col]; sec_value_col++)
            {
                if (sec_values(ext_col, sec_value_col) == sec_input[sec_col])
                {
                    sec_value_found = true;
                    sec_value_cnt(ext_col, sec_value_col) += 1;
                    break;
                }
            }
            if (!sec_value_found)
            {
                if (sec_value_ind[sec_col] >= 10)
                {
                    cout << "MergeDond2Files: main file row: " << main_row << " external file row: " << sec_row << endl;
                    PLERROR("MergeDond2Files: we have exceeded the capacity of the value MAT.");
                }
                sec_values(ext_col, sec_value_ind[sec_col]) = sec_input[sec_col];
                sec_value_ind[sec_col] += 1;
            }
        }
        if (sec_ins[sec_col] == "present")
        {
            sec_value_cnt(ext_col, 0) = 1.0;
        }
    }
}

void MergeDond2Files::combineAndPut()
{
    merge_col = 0;
    for (main_col = 0; main_col < main_width; main_col++)
    {
        if (main_ins[main_col] == "skip") continue;
        if (main_ins[main_col] == "as_is")
        {
            merge_output[merge_col] = main_input[main_col];
            merge_col +=1;
            continue;
        }
        if (main_ins[main_col] == "zero_is_missing")
        {
            if (main_input[main_col] == 0.0) merge_output[merge_col] = MISSING_VALUE;
            else merge_output[merge_col] = main_input[main_col];
            merge_col +=1;
            continue;
        }
        if (main_ins[main_col] == "2436935_is_missing")
        {
            if (main_input[main_col] == 2436935.0) merge_output[merge_col] = MISSING_VALUE;
            else merge_output[merge_col] = main_input[main_col];
            merge_col +=1;
            continue;
        }
        if (main_ins[main_col] == "present")
        {
            if (is_missing(main_input[main_col])) merge_output[merge_col] = 0.0;
            else merge_output[merge_col] = 1.0;
            merge_col +=1;
            continue;
        }
    }
    for (sec_col = 0; sec_col < sec_width; sec_col++)
    {
        if (sec_ins[sec_col] == "skip") continue;
        ext_col = extension_pos[sec_col];
        if (sec_ins[sec_col] == "mean")
        {
            if (sec_value_cnt(ext_col, 0) <= 0.0)  merge_output[merge_col] = MISSING_VALUE;
            else merge_output[merge_col] = sec_values(ext_col, 0) / sec_value_cnt(ext_col, 0);
            merge_col +=1;
            continue;
        }
        if (sec_ins[sec_col] == "mode")
        {
            if (sec_value_ind[sec_col] <= 0.0)
            {
                merge_output[merge_col] = MISSING_VALUE;
                merge_col +=1;
                continue;
            }
            merge_output[merge_col] = sec_values(ext_col, 0);
            sec_value_count_max = sec_value_cnt(ext_col, 0);
            for (sec_value_col = 1; sec_value_col < sec_value_ind[sec_col]; sec_value_col++)
            {
                if (sec_value_cnt(ext_col, sec_value_col) >= sec_value_count_max)
                {
                    merge_output[merge_col] = sec_values(ext_col, sec_value_col);
                    sec_value_count_max = sec_value_cnt(ext_col, sec_value_col);
                }
            }
            merge_col +=1;
            continue;
        }
        if (sec_ins[sec_col] == "present")
        {
            merge_output[merge_col] = sec_value_cnt(ext_col, 0);
            merge_col +=1;
            continue;
        }
    }
    if (main_input[train_ind] > 0.0)
    {
        train_file->putRow(train_row, merge_output);
        train_row += 1;
    }
    else if (main_input[test_ind] > 0.0)
    {
        test_file->putRow(test_row, merge_output);
        test_row += 1;
    }
    else
    {
        unknown_file->putRow(unknown_row, merge_output);
        unknown_row += 1;
    }
    sec_values.clear();
    sec_value_cnt.clear();
    sec_value_ind.clear();
}

int MergeDond2Files::outputsize() const {return 0;}
void MergeDond2Files::train() {}
void MergeDond2Files::computeOutput(const Vec&, Vec&) const {}
void MergeDond2Files::computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const {}
TVec<string> MergeDond2Files::getTestCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
}
TVec<string> MergeDond2Files::getTrainCostNames() const
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
