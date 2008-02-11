// -*- C++ -*-

// Experimentation.cc
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

/*! \file Experimentation.cc */

#define PL_LOG_MODULE_NAME "Experimentation"
#include <plearn/io/pl_log.h>

#include "Experimentation.h"
#include <plearn/io/load_and_save.h>                 //!<  For save
#include <plearn/io/fileutils.h>                     //!<  For isfile()
#include <plearn/math/random.h>                      //!<  For the seed stuff.
#include <plearn/vmat/ExplicitSplitter.h>            //!<  For the splitter stuff.
#include <plearn/vmat/VariableDeletionVMatrix.h>     //!<  For the new_set stuff.

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    Experimentation,
    "Computes correlation coefficient between various discrete values and the target.",
    "name of the discrete variable, of the target and the values to check are options.\n"
);

/////////////////////////
// Experimentation //
/////////////////////////
Experimentation::Experimentation()
{
}
    
////////////////////
// declareOptions //
////////////////////
void Experimentation::declareOptions(OptionList& ol)
{
    declareOption(ol, "save_files", &Experimentation::save_files,
                  OptionBase::buildoption,
                  "If set to 1, save the built train and test files instead of running the experiment.");
    declareOption(ol, "experiment_without_missing_indicator", &Experimentation::experiment_without_missing_indicator,
                  OptionBase::buildoption,
                  "If set to 1, the missing_indicator_field_names will be excluded from the built training files.");
    declareOption(ol, "target_field_name", &Experimentation::target_field_name,
                  OptionBase::buildoption,
                  "The name of the field to select from the target_set as target for the built training files.");
    declareOption(ol, "missing_indicator_field_names", &Experimentation::missing_indicator_field_names,
                  OptionBase::buildoption,
                  "The field names of the missing indicators to exclude when we experiment without them.");
    declareOption(ol, "experiment_name", &Experimentation::experiment_name,
                  OptionBase::buildoption,
                  "The name of the group of experiments to conduct.");
    declareOption(ol, "number_of_test_samples", &Experimentation::number_of_test_samples,
                  OptionBase::buildoption,
                  "The number of test samples at the beginning of the train set.");
    declareOption(ol, "number_of_train_samples", &Experimentation::number_of_train_samples,
                  OptionBase::buildoption,
                  "The number of train samples in the reference set to compute the % of missing.");
    declareOption(ol, "reference_train_set", &Experimentation::reference_train_set,
                  OptionBase::buildoption,
                  "The train and valid set with missing values to compute the % of missing.");
    declareOption(ol, "target_set", &Experimentation::target_set,
                  OptionBase::buildoption,
                  "The data set with the targets corresponding to the train set.");
    declareOption(ol, "deletion_thresholds", &Experimentation::deletion_thresholds,
                  OptionBase::buildoption,
                  "The vector of the various deletion threshold to run this experiment with.");
    declareOption(ol, "experiment_directory", &Experimentation::experiment_directory,
                  OptionBase::buildoption,
                  "The path in which to build the directories for the experiment's results.");
    declareOption(ol, "experiment_template", &Experimentation::experiment_template,
                  OptionBase::buildoption,
                  "The template of the script to conduct the experiment.");

    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void Experimentation::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(experiment_name, copies);
    deepCopyField(number_of_test_samples, copies);
    deepCopyField(number_of_train_samples, copies);
    deepCopyField(reference_train_set, copies);
    deepCopyField(target_set, copies);
    deepCopyField(deletion_thresholds, copies);
    deepCopyField(experiment_directory, copies);
    deepCopyField(experiment_template, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);

}

///////////
// build //
///////////
void Experimentation::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void Experimentation::build_()
{
    MODULE_LOG << "build_() called" << endl;
    if (train_set)
    {
        for (int iteration = 1; iteration <= train_set->width(); iteration++)
        {
            cout << "In Experimentation, Iteration # " << iteration << endl;
            experimentSetUp();
            train();
            ::PLearn::save(header_expdir + "/" + deletion_threshold_str + "/source_names.psave", source_names);
        }
        PLERROR("In Experimentation::build_() we are done here");
    }
}

void Experimentation::experimentSetUp()
{ 
    // initialize primary dataset
    main_row = 0;
    main_col = 0;
    main_length = train_set->length();
    main_width = train_set->width();
    main_names.resize(main_width);
    main_names << train_set->fieldNames();
    if (train_set->hasMetaDataDir()) main_metadata = train_set->getMetaDataDir();
    else if (experiment_directory == "") PLERROR("In Experimentation::experimentSetUp() we need one of experiment_directory or train_set->metadatadir");
         else main_metadata = experiment_directory;
    if (experiment_without_missing_indicator > 0)
    {
        fields_width = missing_indicator_field_names.size();
        main_fields_selected.resize(main_width - fields_width);
        for (fields_col = 0; fields_col < fields_width; fields_col++)
        {
            for (main_col = 0; main_col < main_width; main_col++)
            {
                if (missing_indicator_field_names[fields_col] == main_names[main_col]) break;
            }
            if (main_col >= main_width) PLERROR("In Experimentation::experimentSetUp() no field with this name in input dataset: %", (missing_indicator_field_names[fields_col]).c_str());
        }
        main_fields_selected_col = 0;
        for (main_col = 0; main_col < main_width; main_col++)
        {
            for (fields_col = 0; fields_col < fields_width; fields_col++)
            {
                if (missing_indicator_field_names[fields_col] == main_names[main_col]) break;
            }
            if (fields_col < fields_width) continue;
            main_fields_selected[main_fields_selected_col] = main_names[main_col];
            main_fields_selected_col += 1;
        }
    }
    
    // initialize target dataset
    target_row = 0;
    target_col = 0;
    target_length = target_set->length();
    target_width = target_set->width();
    target_names.resize(target_width);
    target_names << target_set->fieldNames();
    if (target_length != main_length) PLERROR("In Experimentation::experimentSetUp() target and main train datasets should have equal length");
    for (target_col = 0; target_col < target_width; target_col++)
    {
        if (target_field_name == target_names[target_col]) break;
    }
    if (target_col >= target_width) PLERROR("In Experimentation::experimentSetUp() no field with this name in target dataset: %", target_field_name.c_str());
    
    // initialize the header file
    cout << "initialize the header file" << endl;
    reference_train_set->lockMetaDataDir();
    if (experiment_directory == "") header_expdir = main_metadata + "/Experiment/" + experiment_name;
    else header_expdir = experiment_directory;
    header_expdir += "/" + target_field_name;
    if (experiment_without_missing_indicator > 0) header_expdir += "/no_ind/";
    else header_expdir += "/ind/";
    header_file_name = header_expdir + "header.pmat";
    if (deletion_thresholds.length() <= 0)
    {
        deletion_thresholds.resize(20);
        for (int header_col = 0; header_col < 20; header_col++) deletion_thresholds[header_col] = (real) to_deal_with_next / 20.0;
    } 
    header_width = deletion_thresholds.length();
    header_record.resize(header_width);
    if (!isfile(header_file_name)) createHeaderFile();
    else getHeaderRecord();
    
    // choose deletion threshold to experiment with
    cout << "choose deletion threshold to experiment with" << endl;
    to_deal_with_total = 0;
    to_deal_with_next = -1;
    for (int header_col = 0; header_col < header_width; header_col++)
    {
        if (header_record[header_col] != 0.0) continue;
        to_deal_with_total += 1;
        if (to_deal_with_next < 0) to_deal_with_next = header_col;
    }
    if (to_deal_with_next < 0)
    {
        reference_train_set->unlockMetaDataDir();
        reviewGlobalStats();
        PLERROR("In Experimentation::experimentSetUp() we are done here");
    }
    deletion_threshold = deletion_thresholds[to_deal_with_next];
    deletion_threshold_str = tostring(deletion_threshold + 0.005).substr(0,4);
    cout << "total number of thresholds left to deal with: " << to_deal_with_total << endl;
    cout << "next thresholds to deal with: " << deletion_threshold << endl;
    updateHeaderRecord(to_deal_with_next);
    reference_train_set->unlockMetaDataDir();
    
    // build the train and test sets
    setSourceDataset();
    cout << "source data set width: " << source_set->width() << endl;
    main_input.resize(source_set->width());
    source_names.resize(source_set->width());
    source_names << source_set->fieldNames();
    source_names.resize(source_set->width() + 1);
    source_names[source_set->width()] = target_field_name;
    
    // load test data set
    ProgressBar* pb = 0;
    test_length = number_of_test_samples;
    test_width = source_set->width() + 1;
    test_file = new MemoryVMatrix(test_length, test_width);
    test_file->defineSizes(test_width - 1, 1, 0);
    test_record.resize(test_width);
    pb = new ProgressBar( "loading the test file for threshold: " + deletion_threshold_str, test_length);
    for (main_row = 0; main_row < test_length; main_row++)
    {
        source_set->getRow(main_row, main_input);
        for (main_col = 0; main_col < source_set->width(); main_col++) test_record[main_col] = main_input[main_col];
        test_record[source_set->width()] = target_set->get(main_row, target_col);
        test_file->putRow(main_row, test_record);
        pb->update( main_row );
    }
    delete pb;
    
    // load training and validation data set
    train_valid_length = main_length - test_length;
    train_valid_width = source_set->width() + 1;
    train_valid_file = new MemoryVMatrix(train_valid_length, train_valid_width);
    train_valid_file->defineSizes(train_valid_width - 1, 1, 0);
    train_valid_record.resize(train_valid_width);
    pb = new ProgressBar( "loading the training and validation file for threshold: " + deletion_threshold_str, train_valid_length);
    for (main_row = test_length; main_row < main_length; main_row++)
    {
        source_set->getRow(main_row, main_input);
        for (main_col = 0; main_col < source_set->width(); main_col++) train_valid_record[main_col] = main_input[main_col];
        train_valid_record[source_set->width()] = target_set->get(main_row, target_col);
        train_valid_file->putRow(main_row - test_length, train_valid_record);
        pb->update( main_row - test_length );
    }
    delete pb;
    
    // save files if requested
    if (save_files <= 0) return;
    VMat save_test = new FileVMatrix(header_expdir + "/" + deletion_threshold_str + "/test.pmat", test_length, test_width);
    save_test->declareFieldNames(source_names);
    pb = new ProgressBar( "saving the test file for threshold: " + deletion_threshold_str, test_length);
    for (main_row = 0; main_row < test_length; main_row++)
    {
        test_file->getRow(main_row, test_record);
        save_test->putRow(main_row, test_record);
        pb->update( main_row );
    }
    delete pb;
    VMat save_train_valid = new FileVMatrix(header_expdir + "/" + deletion_threshold_str + "/train_valid.pmat", train_valid_length, train_valid_width);
    save_train_valid->declareFieldNames(source_names);
    pb = new ProgressBar( "saving the training and validation file for threshold: " + deletion_threshold_str, train_valid_length);
    for (main_row = 0; main_row < train_valid_length; main_row++)
    {
        train_valid_file->getRow(main_row, train_valid_record);
        save_train_valid->putRow(main_row, train_valid_record);
        pb->update( main_row );
    }
    delete pb;
    PLERROR("In Experimentation::experimentSetUp() we are done here");
}

void Experimentation::createHeaderFile()
{ 
    header_record.clear();
    header_names.resize(header_width);
    for (int header_col = 0; header_col < header_width; header_col++) 
        header_names[header_col] = tostring(deletion_thresholds[header_col] + 0.005).substr(0,4);
    header_file = new FileVMatrix(header_file_name, 1, header_names);
    header_file->putRow(0, header_record);
}

void Experimentation::getHeaderRecord()
{ 
    header_file = new FileVMatrix(header_file_name, true);
    if (header_width != header_file->width()) 
        PLERROR("In Experimentation::getHeaderRecord() the existing header file does not match the deletion_thresholds width)");
    header_names = header_file->fieldNames();
    for (int header_col = 0; header_col < header_width; header_col++) 
        if (header_names[header_col] != tostring(deletion_thresholds[header_col] + 0.005).substr(0,4))
            PLERROR("In Experimentation::getHeaderRecord() the existing header file names does not match the deletion_thresholds values)");;
    header_file->getRow(0, header_record);
}

void Experimentation::updateHeaderRecord(int var_col)
{ 
    header_file->put(0, var_col, 1.0);
    header_file->flush();
}

void Experimentation::setSourceDataset()
{
    VMat selected_train_set = train_set;
    VMat selected_reference_set= reference_train_set;
    if (experiment_without_missing_indicator > 0)
    {
            SelectColumnsVMatrix* new_train_set = new SelectColumnsVMatrix();
            new_train_set->source = train_set;
            new_train_set->fields = main_fields_selected;
            selected_train_set = new_train_set;
            selected_train_set->build();
            selected_train_set->defineSizes(selected_train_set->width(), 0, 0);
            SelectColumnsVMatrix* new_reference_set = new SelectColumnsVMatrix();
            new_reference_set->source = reference_train_set;
            new_reference_set->fields = main_fields_selected;
            selected_reference_set = new_reference_set;
            selected_reference_set->build();
            selected_reference_set->defineSizes(selected_reference_set->width(), 0, 0);
    }
    if (deletion_threshold <= 0.0)
    {
        source_set = selected_train_set;
        return;
    }
    VariableDeletionVMatrix* new_set = new VariableDeletionVMatrix();
    // VMat: The data set with all variables to select the columns from.
    new_set->complete_dataset = selected_train_set;
    // VMat: The train set in which to compute the percentage of missing values.
    new_set->train_set = selected_reference_set;
    // double: The percentage of non-missing values for a variable above which, the variable will be selected.
    new_set->deletion_threshold = deletion_threshold;
    // bool: If set to 1, the columns with constant non-missing values will be removed.
    new_set->remove_columns_with_constant_value = 1;
    // double: If equal to zero, all the train samples are used to calculated the percentages and constant values.
    // If it is a fraction between 0 and 1, this proportion of the samples will be used.
    // If greater or equal to 1, the integer portion will be interpreted as the number of samples to use.
    new_set->number_of_train_samples = number_of_train_samples;
    // int: The row at which, to start to calculate the percentages and constant values.
    new_set->start_row = 0;
    source_set = new_set;
    source_set->build();
}

void Experimentation::reviewGlobalStats()
{
    cout << "There is no more variable to deal with." << endl;
    bool missing_results_file = false;
    for (int header_col = 0; header_col < header_width; header_col++)
    {
        deletion_threshold = deletion_thresholds[header_col];
        deletion_threshold_str = tostring(deletion_threshold + 0.005).substr(0,4);
        PPath expdir = header_expdir + "/" + deletion_threshold_str;
        PPath train_valid_results_file_name = expdir + "/Split0/LearnerExpdir/Strat0results.pmat";
        PPath test_results_file_name = expdir + "/global_stats.pmat";
        PPath source_names_file_name = expdir + "/source_names.psave";
        if (!isfile(train_valid_results_file_name))
        {
            cout << "Missing training and validation results for threshold " << deletion_threshold_str << endl;
            missing_results_file = true;
        }
        if (!isfile(test_results_file_name))
        {
            cout << "Missing test results for threshold " << deletion_threshold_str << endl;
            missing_results_file = true;
        }
        if (!isfile(source_names_file_name))
        {
            cout << "Missing variable selected saved file for threshold " << deletion_threshold_str << endl;
            missing_results_file = true;
        }
    }
    if (missing_results_file) return;
    cout << endl << endl;
    cout << "Results for experiment " << experiment_name << endl;
    cout << "       The file used for this experiment was " << main_metadata << endl;
    cout << "       The target used was " << target_field_name << endl;
    if (experiment_without_missing_indicator > 0) cout << "       The experiment was carried without missing indicators" << endl;
    else cout << "       The experiment was carried with missing indicators" << endl;
    cout << endl << endl;
    cout << "           number                                                                  " << endl;
    cout << "             of                                                                    " << endl;
    cout << " deletion variable   weigth    train    valid     test     test       std      test" << endl;
    cout << "threshold selected    decay     mse      mse      mse      cse       error     cle " << endl;
    cout << endl;
    cout << fixed << showpoint;
    real best_valid_mse_threshold = -1.0;
    real best_valid_mse_value;
    for (int header_col = 0; header_col < header_width; header_col++)
    {
        deletion_threshold = deletion_thresholds[header_col];
        deletion_threshold_str = tostring(deletion_threshold + 0.005).substr(0,4);
        PPath expdir = header_expdir + "/" + deletion_threshold_str;
        PPath train_valid_results_file_name = expdir + "/Split0/LearnerExpdir/Strat0results.pmat";
        PPath test_results_file_name = expdir + "/global_stats.pmat";
        PPath source_names_file_name = expdir + "/source_names.psave";
        ::PLearn::load(source_names_file_name, source_names);
        VMat train_valid_results_file = new FileVMatrix(train_valid_results_file_name);
        VMat test_results_file = new FileVMatrix(test_results_file_name);
        int train_valid_last_row = train_valid_results_file->length() - 1;
        real weight_decay = train_valid_results_file->get(train_valid_last_row, 2);
        real train_mse =    train_valid_results_file->get(train_valid_last_row, 3);
        real valid_mse =    train_valid_results_file->get(train_valid_last_row, 4);
        if (best_valid_mse_threshold < 0.0)
        {
            best_valid_mse_threshold = deletion_threshold;
            best_valid_mse_value = valid_mse;
        }
        else if (valid_mse < best_valid_mse_value)
        {
            best_valid_mse_threshold = deletion_threshold;
            best_valid_mse_value = valid_mse;
        }
        real test_mse =     test_results_file->get(1, 0);
        real test_cse =     test_results_file->get(1, 2);
        real test_cse_std = test_results_file->get(1, 3);
        real test_cle =     test_results_file->get(1, 4);
        cout << setiosflags(ios::right) << setw(9) << deletion_threshold_str << "   "
             << setw(4) << source_names.size() << "    "
             << setw(6) << weight_decay << " "
             << setw(6) << train_mse << " "
             << setw(6) << valid_mse << " "
             << setw(6) << test_mse << " "
             << setw(6) << test_cse << "+/-"
             << setw(6) << test_cse_std << " "
             << setw(6) << test_cle << endl;
    }
    cout << endl << endl;
    cout << "       Based on the validation mse, the suggested threshold is " << best_valid_mse_threshold << endl;
    cout << endl << endl;
}

void Experimentation::train()
{
    PP<ExplicitSplitter> explicit_splitter = new ExplicitSplitter();
    explicit_splitter->splitsets.resize(1,2);
    explicit_splitter->splitsets(0,0) = train_valid_file;
    explicit_splitter->splitsets(0,1) = test_file;
    experiment = ::PLearn::deepCopy(experiment_template);
    experiment->setOption("expdir", header_expdir + "/" + deletion_threshold_str);
    experiment->splitter = new ExplicitSplitter();
    experiment->splitter = explicit_splitter;
    experiment->build();
    Vec results = experiment->perform(true);
}

int Experimentation::outputsize() const {return 0;}
void Experimentation::computeOutput(const Vec&, Vec&) const {}
void Experimentation::computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const {}
TVec<string> Experimentation::getTestCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
}
TVec<string> Experimentation::getTrainCostNames() const
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
