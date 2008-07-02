// -*- C++ -*-

// AnalyzeFieldStats.cc
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

/*! \file AnalyzeFieldStats.cc */

#define PL_LOG_MODULE_NAME "AnalyzeFieldStats"

#include "AnalyzeFieldStats.h"
#include <plearn/io/pl_log.h>
#include <plearn/io/load_and_save.h>          //!<  For save
#include <plearn/io/fileutils.h>              //!<  For isfile()
#include <plearn/math/random.h>               //!<  For the seed stuff.
#include <plearn/vmat/ExplicitSplitter.h>     //!<  For the splitter stuff.

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    AnalyzeFieldStats,
    "Computes correlation coefficient between various discrete values and the target.",
    "name of the discrete variable, of the target and the values to check are options.\n"
);

/////////////////////////
// AnalyzeFieldStats //
/////////////////////////
AnalyzeFieldStats::AnalyzeFieldStats() :
  min_number_of_samples(5000),
  max_number_of_samples(50000)
{
}
    
////////////////////
// declareOptions //
////////////////////
void AnalyzeFieldStats::declareOptions(OptionList& ol)
{

    declareOption(ol, "min_number_of_samples", &AnalyzeFieldStats::min_number_of_samples,
                  OptionBase::buildoption,
                  "The minimum number of samples required to train the learner.");
    declareOption(ol, "max_number_of_samples", &AnalyzeFieldStats::max_number_of_samples,
                  OptionBase::buildoption,
                  "The maximum number of samples used to train the learner");
    declareOption(ol, "targeted_set", &AnalyzeFieldStats::targeted_set,
                  OptionBase::buildoption,
                  "The train and test data sets with the target field.");
    declareOption(ol, "cond_mean_template", &AnalyzeFieldStats::cond_mean_template,
                  OptionBase::buildoption,
                  "The template of the script to learn the conditional mean.");
    declareOption(ol, "fields", &AnalyzeFieldStats::fields,
                  OptionBase::buildoption,
                  "The vector of fields to consider by names.");

    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void AnalyzeFieldStats::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(min_number_of_samples, copies);
    deepCopyField(max_number_of_samples, copies);
    deepCopyField(targeted_set, copies);
    deepCopyField(cond_mean_template, copies);
    deepCopyField(fields, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);

}

///////////
// build //
///////////
void AnalyzeFieldStats::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void AnalyzeFieldStats::build_()
{
    MODULE_LOG << "build_() called" << endl;
    if (train_set)
    {
        for (int iteration = 1; iteration <= train_set->width(); iteration++)
        {
            cout << "In AnalyzeFieldStats, Iteration # " << iteration << endl;
            analyzeVariableStats();
            train();
        }
        PLERROR("AnalyzeFieldStats::build_() we are done here");
    }
}

void AnalyzeFieldStats::analyzeVariableStats()
{ 
    // initialize primary dataset
    int main_length = train_set->length();
    main_width = train_set->width();
    Vec main_input;
    main_input.resize(main_width);
    main_names.resize(main_width);
    main_names << train_set->fieldNames();
    main_metadata = train_set->getMetaDataDir();
    
    // validate the field instructions
    fields_width = fields.size();
    fields_selected.resize(main_width);
    fields_selected.clear();
    for (fields_col = 0; fields_col < fields_width; fields_col++)
    {
        int main_col;
        for (main_col = 0; main_col < main_width; main_col++)
        {
            if (fields[fields_col] == main_names[main_col]) break;
        }
        if (main_col >= main_width) 
            PLERROR("In AnalyzeFieldStats::analyzeVariableStats() no field with this name in input dataset: %s", (fields[fields_col]).c_str());
        fields_selected[main_col] = 1;
    }
    
    // initialize targeted datasets
    cout << "initialize train_test datasets" << endl;
    targeted_length = targeted_set->length();
    targeted_width = targeted_set->width();
    targeted_input.resize(targeted_width);
    targeted_names.resize(targeted_width);
    targeted_names << targeted_set->fieldNames();
    targeted_metadata = targeted_set->getMetaDataDir();
    
    // initialize the header file
    cout << "initialize the header file" << endl;
    train_set->lockMetaDataDir();
    header_record.resize(main_width);
    header_file_name = targeted_metadata + "/TreeCondMean/header.pmat";
    if (!isfile(header_file_name)) createHeaderFile();
    else getHeaderRecord();
    
    // choose variable to build a conditionnal function for
    cout << "choose variable to build a conditionnal function for" << endl;
    TVec<int> indices;
    to_deal_with_total = 0;
    to_deal_with_next = -1;
    for (int main_col = 0; main_col < main_width; main_col++)
    {
        if (header_record[main_col] != 2.0) continue;
        to_deal_with_total += 1;
        if (to_deal_with_next < 0) to_deal_with_next = main_col;
    }
    if (to_deal_with_next < 0)
    {
        train_set->unlockMetaDataDir();
        reviewGlobalStats();
        PLERROR("AnalyzeFieldStats::analyzeVariableStats() we are done here");
    }
    to_deal_with_name = main_names[to_deal_with_next];
    cout << "total number of variable left to deal with: " << to_deal_with_total << endl;
    cout << "next variable to deal with: " << main_names[to_deal_with_next] << endl;
    updateHeaderRecord(to_deal_with_next);
    train_set->unlockMetaDataDir();
    
    // find the available targeted records for this variable
    ProgressBar* pb = 0;
    main_stats = train_set->getStats(to_deal_with_next);
    main_total = main_stats.n();
    main_missing = main_stats.nmissing();
    main_present = main_total - main_missing;
    indices.resize((int) main_present);
    ind_next = 0;
    pb = new ProgressBar( "Building the indices for " + to_deal_with_name, main_length);
    for (int main_row = 0; main_row < main_length; main_row++)
    {
        to_deal_with_value = train_set->get(main_row, to_deal_with_next);
        if (is_missing(to_deal_with_value)) continue;
        if (ind_next >= indices.length()) 
            PLERROR("AnalyzeFieldStats::analyzeVariableStats() There seems to be more present values than indicated by the stats file");
        indices[ind_next] = main_row;
        ind_next += 1;
        pb->update( main_row );
    }
    delete pb;
    
    // shuffle the indices.
    manual_seed(123456);
    shuffleElements(indices);
    
    // initialize output datasets
    output_length = (int) main_present;
    if (output_length > max_number_of_samples) output_length = max_number_of_samples;
    output_width = 0;
    for (int main_col = 0; main_col < main_width; main_col++)
    {
        if (header_record[main_col] != 1) output_width += 1;
    }
    output_variable_src.resize(output_width);
    output_names.resize(output_width);
    output_vec.resize(output_width);
    output_path = main_metadata + "condmean_" + to_deal_with_name + ".pmat";
    output_col = 0;
    for (fields_col = 0; fields_col < fields_width; fields_col++)
    {
        int main_col;
        for (main_col = 0; main_col < main_width; main_col++)
        {
            if (fields[fields_col] == main_names[main_col]) break;
        }
        if (main_col >= main_width) 
            PLERROR("In AnalyzeFieldStats::analyzeVariableStats() no field with this name in input dataset: %s", (fields[fields_col]).c_str());
        if (fields_col != to_deal_with_next && header_record[main_col] != 1)
        {
            output_variable_src[output_col] = main_col;
            output_names[output_col] = fields[fields_col];
            output_col += 1;
        }
    }
    output_variable_src[output_col] = to_deal_with_next;
    output_names[output_col] = to_deal_with_name;
    output_file = new MemoryVMatrix(output_length, output_width);
    output_file->declareFieldNames(output_names);
    output_file->defineSizes(output_width - 1, 1, 0);
    
    //Now, we can build the training file
    pb = new ProgressBar( "Building the training file for " + to_deal_with_name, output_length);
    for (int main_row = 0; main_row < output_length; main_row++)
    {
        train_set->getRow(indices[main_row], main_input);
        for (output_col = 0; output_col < output_width; output_col++)
        {
            output_vec[output_col] = main_input[output_variable_src[output_col]];
        }
        output_file->putRow(main_row, output_vec);
        pb->update( main_row );
    }
    delete pb;
    
    // initialize train_test datasets
    train_test_length = targeted_length;
    train_test_variable_src.resize(output_width);
    train_test_path = targeted_metadata + "targeted_" + to_deal_with_name + ".pmat";
    output_col = 0;
    for (fields_col = 0; fields_col < fields_width; fields_col++)
    {
        int main_col;
        for (main_col = 0; main_col < targeted_width; main_col++)
        {
            if (fields[fields_col] == targeted_names[main_col]) break;
        }
        if (main_col >= targeted_width) 
            PLERROR("In AnalyzeFieldStats::analyzeVariableStats() no field with this name in targeted dataset: %s", (fields[fields_col]).c_str());
        if (fields_col != to_deal_with_next && header_record[main_col] != 1)
        {
            train_test_variable_src[output_col] = main_col;
            output_col += 1;
        }
    }
    train_test_variable_src[output_col] = to_deal_with_next;
    train_test_file = new MemoryVMatrix(train_test_length, output_width);
    train_test_file->declareFieldNames(output_names);
    train_test_file->defineSizes(output_width - 1, 1, 0);
    
    //Now, we can build the targeted file
    pb = new ProgressBar( "Building the targeted file for " + to_deal_with_name, train_test_length);
    for (int main_row = 0; main_row < train_test_length; main_row++)
    {
        targeted_set->getRow(main_row, targeted_input);
        for (output_col = 0; output_col < output_width; output_col++)
        {
            output_vec[output_col] = targeted_input[train_test_variable_src[output_col]];
        }
        train_test_file->putRow(main_row, output_vec);
        pb->update( main_row );
    }
    delete pb;
}

void AnalyzeFieldStats::createHeaderFile()
{ 
    for (int main_col = 0; main_col < main_width; main_col++)
    {
        targeted_stats = targeted_set->getStats(main_col);
        targeted_missing = targeted_stats.nmissing();
        main_stats = train_set->getStats(main_col);
        main_total = main_stats.n();
        main_missing = main_stats.nmissing();
        main_present = main_total - main_missing;
        if (fields_selected[main_col] < 1) header_record[main_col] = 1;                  // delete column, field not selected
        else if (targeted_missing <= 0) header_record[main_col] = 0;                     // nothing to do
        else if (main_present < min_number_of_samples) header_record[main_col] = 1;      // delete column
        else header_record[main_col] = 2;                                                // build tree
    }
    header_file = new FileVMatrix(header_file_name, 1, main_names);
    header_file->putRow(0, header_record);
}

void AnalyzeFieldStats::getHeaderRecord()
{ 
    header_file = new FileVMatrix(header_file_name, true);
    header_file->getRow(0, header_record);
    for (int main_col = 0; main_col < main_width; main_col++)
    {
        if (header_record[main_col] == 0) continue;
        if (header_record[main_col] == 2) continue;
        if (header_record[main_col] == 1 && fields_selected[main_col] < 1) continue;
        if (header_record[main_col] == 1)
        {
            main_stats = train_set->getStats(main_col);
            main_total = main_stats.n();
            main_missing = main_stats.nmissing();
            main_present = main_total - main_missing;
            if (main_present >= min_number_of_samples) header_record[main_col] = 2;
            continue;
        }
    }
}

void AnalyzeFieldStats::updateHeaderRecord(int var_col)
{ 
    header_file->put(0, var_col, 3.0);
}

void AnalyzeFieldStats::reviewGlobalStats()
{ 
    cout << "There is no more variable to deal with." << endl;
    for (int main_col = 0; main_col < main_width; main_col++)
    {
        if (header_record[main_col] == 0)
        { 
            cout << setiosflags(ios::left) << setw(30) << main_names[main_col];
            cout << " : no missing values for this variable in the targeted files." << endl;
            continue;
        }
        if (header_record[main_col] == 1 && fields_selected[main_col] < 1)
        {
            cout << setiosflags(ios::left) << setw(30) << main_names[main_col];
            cout << " : field not selected." << endl;
            continue;
        }
        if (header_record[main_col] == 1)
        {
            main_stats = train_set->getStats(main_col);
            main_total = main_stats.n();
            main_missing = main_stats.nmissing();
            main_present = main_total - main_missing;
            cout << setiosflags(ios::left) << setw(30) << main_names[main_col];
            cout << " : field deleted, only " << setw(6) << main_present << " records to train with." << endl;
            continue;
        }
        results_file_name = targeted_metadata + "/TreeCondMean/dir/" + main_names[main_col] + "/Split0/LearnerExpdir/Strat0results.pmat";
        if (!isfile(results_file_name))
        {
            header_file->put(0, main_col, 2.0);
            cout << setiosflags(ios::left) << setw(30) << main_names[main_col];
            cout << " : missing results file." << endl;
            continue;
        }
        test_output_file_name = targeted_metadata + "/TreeCondMean/dir/" + main_names[main_col] + "/Split0/test1_outputs.pmat";
        if (!isfile(test_output_file_name))
        {
            header_file->put(0, main_col, 2.0);
            cout << setiosflags(ios::left) << setw(30) << main_names[main_col];
            cout << " : missing test output file." << endl;
            continue;
        }
        results_file = new FileVMatrix(results_file_name);
        results_length = results_file->length();
        results_nstages = results_file->get(results_length - 1, 2);
        results_mse = results_file->get(results_length - 1, 6);
        results_std_err = results_file->get(results_length - 1, 7);
        test_output_file = new FileVMatrix(test_output_file_name);
        test_output_length = test_output_file->length();
        cout << setiosflags(ios::left) << setw(30) << main_names[main_col];
        cout << " : tree built with " << setw(2) << (int) results_nstages << " leaves, "
             << setw(6) << test_output_length << " test output records found, "
             << "performance: " << setiosflags(ios::fixed) << setprecision(4) << results_mse
             << " +/- " << setiosflags(ios::fixed) << setprecision(4) << results_std_err << endl;
    }
}

void AnalyzeFieldStats::train()
{
    PP<ExplicitSplitter> explicit_splitter = new ExplicitSplitter();
    explicit_splitter->splitsets.resize(1,2);
    explicit_splitter->splitsets(0,0) = output_file;
    explicit_splitter->splitsets(0,1) = train_test_file;
    PP<PTester> cond_mean = ::PLearn::deepCopy(cond_mean_template);
    cond_mean->setOption("expdir", targeted_metadata + "/TreeCondMean/dir/" + to_deal_with_name);
    cond_mean->splitter = new ExplicitSplitter();
    cond_mean->splitter = explicit_splitter;
    cond_mean->build();
    Vec results = cond_mean->perform(true);
}

int AnalyzeFieldStats::outputsize() const {return 0;}
void AnalyzeFieldStats::computeOutput(const Vec&, Vec&) const {}
void AnalyzeFieldStats::computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const {}
TVec<string> AnalyzeFieldStats::getTestCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
}
TVec<string> AnalyzeFieldStats::getTrainCostNames() const
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
