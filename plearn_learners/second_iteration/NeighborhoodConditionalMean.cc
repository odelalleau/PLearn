// -*- C++ -*-

// NeighborhoodConditionalMean.cc
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

/*! \file NeighborhoodConditionalMean.cc */

#define PL_LOG_MODULE_NAME "NeighborhoodConditionalMean"
#include <plearn/io/pl_log.h>

#include "NeighborhoodConditionalMean.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NeighborhoodConditionalMean,
    "Computes correlation coefficient between various discrete values and the target.",
    "name of the discrete variable, of the target and the values to check are options.\n"
);

/////////////////////////
// NeighborhoodConditionalMean //
/////////////////////////
NeighborhoodConditionalMean::NeighborhoodConditionalMean()
{
}
    
////////////////////
// declareOptions //
////////////////////
void NeighborhoodConditionalMean::declareOptions(OptionList& ol)
{

    declareOption(ol, "test_train_input_set", &NeighborhoodConditionalMean::test_train_input_set,
                  OptionBase::buildoption,
                  "The concatenated test and train input vectors with missing values.");
    declareOption(ol, "test_train_target_set", &NeighborhoodConditionalMean::test_train_target_set,
                  OptionBase::buildoption,
                  "The corresponding target vectors.");
    declareOption(ol, "number_of_test_samples", &NeighborhoodConditionalMean::number_of_test_samples,
                  OptionBase::buildoption,
                  "The number of test samples at the beginning of the test train concatenated sets.");
    declareOption(ol, "number_of_train_samples", &NeighborhoodConditionalMean::number_of_train_samples,
                  OptionBase::buildoption,
                  "The number of train samples in the reference set to compute the % of missing.");
    declareOption(ol, "target_field_names", &NeighborhoodConditionalMean::target_field_names,
                  OptionBase::buildoption,
                  "The vector of names of the field to select from the target_set as target for the built training files.");
    declareOption(ol, "train_covariance_file_name", &NeighborhoodConditionalMean::train_covariance_file_name,
                  OptionBase::buildoption,
                  "The path to the file train set where missing value are imputed by the covariance preservation algo.");
    declareOption(ol, "test_train_covariance_file_name", &NeighborhoodConditionalMean::test_train_covariance_file_name,
                  OptionBase::buildoption,
                  "The path to the file test_train set where missing value are imputed by the covariance preservation algo.");
    declareOption(ol, "various_ks", &NeighborhoodConditionalMean::various_ks,
                  OptionBase::buildoption,
                  "The vector of various Ks to experiment with. Values must be between 1 and 100.");
    declareOption(ol, "deletion_thresholds", &NeighborhoodConditionalMean::deletion_thresholds,
                  OptionBase::buildoption,
                  "The vector of thresholds to be tested for each of the various Ks.");
    declareOption(ol, "experiment_name", &NeighborhoodConditionalMean::experiment_name,
                  OptionBase::buildoption,
                  "The name of the group of experiments to conduct.");
    declareOption(ol, "missing_indicator_field_names", &NeighborhoodConditionalMean::missing_indicator_field_names,
                  OptionBase::buildoption,
                  "The field names of the missing indicators to exclude when we experiment without them.");
    declareOption(ol, "experiment_template", &NeighborhoodConditionalMean::experiment_template,
                  OptionBase::buildoption,
                  "The template of the script to conduct the experiment.");

    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void NeighborhoodConditionalMean::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(test_train_input_set, copies);
    deepCopyField(test_train_target_set, copies);
    deepCopyField(number_of_test_samples, copies);
    deepCopyField(number_of_train_samples, copies);
    deepCopyField(target_field_names, copies);
    deepCopyField(test_train_covariance_file_name, copies);
    deepCopyField(train_covariance_file_name, copies);
    deepCopyField(various_ks, copies);
    deepCopyField(deletion_thresholds, copies);
    deepCopyField(experiment_name, copies);
    deepCopyField(missing_indicator_field_names, copies);
    deepCopyField(deletion_thresholds, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);

}

///////////
// build //
///////////
void NeighborhoodConditionalMean::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void NeighborhoodConditionalMean::build_()
{
    MODULE_LOG << "build_() called" << endl;
    if (train_set)
    {
        for (int iteration = 1; iteration <= 1; iteration++)
        {
            cout << "In NeighborhoodConditionalMean, Iteration # " << iteration << endl;
            computeNeighborhood();
            experimentWithVariousKs();
            train();
        }
        PLERROR("In NeighborhoodConditionalMean: we are done here");
    }
}

void NeighborhoodConditionalMean::computeNeighborhood()
{
/*
    prepare correlation based versions of datatset: we have to write a VMatrix for that
    use the ball tree nearest neighbor to build a ball tree using train only, with unknown it would be too long
    find the 100 nearest neighbors of samples in train and test in order from the closest to the furthest
    now we can create a neighborhood imputation for K from 1 up to 100 averaging 
    the observed values of the the k closest input vectors.
    If there is no observed values in the k closest, we have to use something else:
    mean of the covariance preservation imputationof the the k closest input vectors.
*/
    cout << "In NeighborhoodConditionalMean:" << endl;
    cout << endl << "****** STEP 1 ******" << endl;
    cout << "The first thing to do is to impute an initial value the the missing values in order to be able" << endl;
    cout << "to compute distance between samples." << endl;
    cout << "This step uses the CovariancePreservationVMatrix to do that." << endl;
    cout << "The Covariance PreservationVMatrix creates a covariance_file in the metadata of the  source file" << endl;
    cout << "if it is not already there." << endl;
    cout << "The file is kept in train_imputed_with_covariance_preservation.pmat." << endl;
    if( train_covariance_file_name == "" )
        PLERROR("In NeighborhoodConditionalMean::computeNeighborhood() train_covariance_file_name must not be empty",train_covariance_file_name.c_str());
    if (isfile(train_covariance_file_name))
    {
        train_covariance_file = new FileVMatrix(train_covariance_file_name);
        train_covariance_file->defineSizes(train_covariance_file->width(), 0, 0);
        cout << train_covariance_file_name << " already exist, we are skipping this step." << endl;
    }
    else 
    {
        train_covariance_vmatrix = new CovariancePreservationImputationVMatrix();
        train_covariance_vmatrix->source = train_set;
        train_covariance_vmatrix->train_set = train_set;
        train_covariance_vmatrix->build();
        train_covariance_vmat = train_covariance_vmatrix;
        train_covariance_file = new FileVMatrix(train_covariance_file_name, train_covariance_vmat->length(), train_covariance_vmat->fieldNames());
        train_covariance_file->defineSizes(train_covariance_vmat->width(), 0, 0);
        pb = new ProgressBar("Saving the train file imputed with the covariance preservation", train_covariance_vmat->length());
        train_covariance_vector.resize(train_covariance_vmat->width());
        for (int train_covariance_row = 0; train_covariance_row < train_covariance_vmat->length(); train_covariance_row++)
        {
            train_covariance_vmat->getRow(train_covariance_row, train_covariance_vector);
            train_covariance_file->putRow(train_covariance_row, train_covariance_vector);
            pb->update( train_covariance_row );
        }
        delete pb;
    }
    cout << endl << "****** STEP 2 ******" << endl;
    cout << "We do the same thing with the test_train dataset" << endl;
    cout << "using the covariance file created at the previous step." << endl;
    cout << "The file is kept in test_train_imputed_with_covariance_preservation.pmat." << endl;
    if( test_train_covariance_file_name == "" )
        PLERROR("In NeighborhoodConditionalMean::computeNeighborhood() test_train_covariance_file_name must not be empty",test_train_covariance_file_name.c_str());
    if (isfile(test_train_covariance_file_name))
    {
        test_train_covariance_file = new FileVMatrix(test_train_covariance_file_name);
        test_train_covariance_file->defineSizes(test_train_covariance_file->width(), 0, 0);
        cout << test_train_covariance_file_name << " already exist, we are skipping this step." << endl;
    }
    else 
    {
        test_train_covariance_vmatrix = new CovariancePreservationImputationVMatrix();
        test_train_covariance_vmatrix->source = test_train_input_set;
        test_train_covariance_vmatrix->train_set = train_set;
        test_train_covariance_vmatrix->build();
        test_train_covariance_vmat = test_train_covariance_vmatrix;
        test_train_covariance_file = new FileVMatrix(test_train_covariance_file_name, test_train_covariance_vmat->length(), test_train_covariance_vmat->fieldNames());
        test_train_covariance_file->defineSizes(test_train_covariance_vmat->width(), 0, 0);
        pb = new ProgressBar("Saving the test_train file imputed with the covariance preservation", test_train_covariance_vmat->length());
        test_train_covariance_vector.resize(test_train_covariance_vmat->width());
        for (int test_train_covariance_row = 0; test_train_covariance_row < test_train_covariance_vmat->length(); test_train_covariance_row++)
        {
            test_train_covariance_vmat->getRow(test_train_covariance_row, test_train_covariance_vector);
            test_train_covariance_file->putRow(test_train_covariance_row, test_train_covariance_vector);
            pb->update( test_train_covariance_row );
        }
        delete pb;
    }
    cout << endl << "****** STEP 3 ******" << endl;
    cout << "We this initial imputation, we find the 100 nearest neighbors of each sample in the test_train dataset." << endl;
    cout << "Their indexes are kept in the neighborhood_file of the test_train dataset metadata." << endl;
    cout << "The BallTreeNearestNeighbors learner is used to build a tree with the train set" << endl;
    cout << "in order to speed up the identification of the 100 nearest neighbors of the test_train dataset." << endl;
    test_train_neighborhood_file_name = test_train_covariance_file_name + ".metadata/neighborhood_file.pmat";
    if (isfile(test_train_neighborhood_file_name))
    {
        test_train_neighborhood_file = new FileVMatrix(test_train_neighborhood_file_name);
        cout << test_train_neighborhood_file_name << " already exist, we are skipping this step." << endl;
    }
    else 
    {
        test_train_neighborhood_learner = new BallTreeNearestNeighbors();
        test_train_neighborhood_learner->setOption("rmin", "1");
        test_train_neighborhood_learner->setOption("train_method", "anchor");
        test_train_neighborhood_learner->setOption("num_neighbors", "100");
        test_train_neighborhood_learner->setOption("copy_input", "0");
        test_train_neighborhood_learner->setOption("copy_target", "0");
        test_train_neighborhood_learner->setOption("copy_weight", "0");
        test_train_neighborhood_learner->setOption("copy_index", "1");
        test_train_neighborhood_learner->setOption("nstages", "-1");
        test_train_neighborhood_learner->setOption("report_progress", "1");
        test_train_neighborhood_learner->setTrainingSet(train_covariance_file, true);
        test_train_neighborhood_learner->train();
        test_train_neighborhood_file = new FileVMatrix(test_train_neighborhood_file_name, test_train_covariance_file->length(), 100);
        test_train_covariance_vector.resize(test_train_covariance_file->width());
        test_train_neighborhood_vector.resize(100);
        pb = new ProgressBar("Saving the test_train file with the index of the 100 nearest neighbors", test_train_covariance_file->length());
        for (int test_train_neighborhood_row = 0; test_train_neighborhood_row < test_train_covariance_file->length(); test_train_neighborhood_row++)
        {
            test_train_covariance_file->getRow(test_train_neighborhood_row, test_train_covariance_vector);
            test_train_neighborhood_learner->computeOutput(test_train_covariance_vector, test_train_neighborhood_vector);
            test_train_neighborhood_file->putRow(test_train_neighborhood_row, test_train_neighborhood_vector);
            pb->update( test_train_neighborhood_row );
        }
        delete pb;
    }
}

void NeighborhoodConditionalMean::experimentWithVariousKs()
{
/*
    We control the experiments using a  master header file giving the status for each ks.
    If the file is not there, we create it.
    An experiment directory is created for each ks to eexperiment with various level 
    of variable deletion.
*/
    cout << endl << "****** STEP 4 ******" << endl;
    cout << "We now prepare experimentation at various levels of Ks, the number of neighbors between 1 and 100." << endl;
    cout << "The first thing is to load the master header file from the test_train_imputed_with_covariance_preservation.pmat metadata." << endl;
    cout << "If it is not there, the file is created." << endl;
    train_set->lockMetaDataDir();
    master_header_file_name = test_train_covariance_file_name + ".metadata";
    master_header_file_name += "/Experiment/" + experiment_name + "/";
    master_header_file_name += "neighborhood_header.pmat";
    if (!isfile(master_header_file_name)) createMasterHeaderFile();
    else getMasterHeaderRecords();
    cout << "With the master header data, we can choose which K to experiment with." << endl;
    for (master_header_row = 0; master_header_row < master_header_length; master_header_row++)
    {
        for (master_header_col = 0; master_header_col < master_header_width; master_header_col++)
            if (master_header_records(master_header_row, master_header_col) <= 0.0) break;
        if (master_header_col < master_header_width) break;
    }
    if (master_header_row >= master_header_length)
    {
        train_set->unlockMetaDataDir();
        //reviewGlobalStats();
        PLERROR("In NeighborhoodConditionalMean: we are done here");
    }
    to_deal_with_k = various_ks[master_header_col];
    to_deal_with_target = target_field_names[master_header_row / 2];
    to_deal_with_ind = master_header_row % 2;
    cout << "Next target to deal with: " << to_deal_with_target << endl;
    cout << "Next experiment missing indicator: " << to_deal_with_ind << endl;
    cout << "Next k (number of neighbors) to experiment with: " << to_deal_with_k << endl;
    updateMasterHeaderRecords(master_header_row, master_header_col);
    train_set->unlockMetaDataDir();
    cout << endl << "****** STEP 5 ******" << endl;
    cout << "We perform the imputaton with the selected number of neighbors." << endl;
    cout << "The resulting file is loaded in memory to be passed to the experimentation script." << endl;
    test_train_neighbor_imputation_vmatrix = new NeighborhoodImputationVMatrix();
    test_train_neighbor_imputation_vmatrix->source = test_train_input_set;
    test_train_neighbor_imputation_vmatrix->reference_index = test_train_neighborhood_file;
    test_train_neighbor_imputation_vmatrix->reference_with_missing = train_set;
    test_train_neighbor_imputation_vmatrix->reference_with_covariance_preserved = train_covariance_file;
    test_train_neighbor_imputation_vmatrix->number_of_neighbors = to_deal_with_k;
    test_train_neighbor_imputation_vmatrix->build();
    test_train_neighbor_imputation_vmat = test_train_neighbor_imputation_vmatrix;
    test_train_neighbor_imputation_file = new MemoryVMatrix(test_train_neighbor_imputation_vmat->length(), test_train_neighbor_imputation_vmat->width());
    test_train_neighbor_imputation_file->defineSizes(test_train_neighbor_imputation_vmat->width(), 0, 0);
    test_train_neighbor_imputation_file->declareFieldNames(test_train_neighbor_imputation_vmat->fieldNames());
    test_train_neighbor_imputation_vector.resize(test_train_neighbor_imputation_vmat->width());
    pb = new ProgressBar("Loading the test_train file imputed with the selected # of neighbors", test_train_neighbor_imputation_vmat->length());
    for (int  test_train_neighbor_imputation_row = 0;
              test_train_neighbor_imputation_row < test_train_neighbor_imputation_vmat->length();
              test_train_neighbor_imputation_row++)
    {
        test_train_neighbor_imputation_vmat->getRow(test_train_neighbor_imputation_row, test_train_neighbor_imputation_vector);
        test_train_neighbor_imputation_file->putRow(test_train_neighbor_imputation_row, test_train_neighbor_imputation_vector);
        pb->update( test_train_neighbor_imputation_row );
    }
     //       ::PLearn::save(header_expdir + "/" + deletion_threshold_str + "/source_names.psave", source_names);
    delete pb;
    cout << endl << "****** STEP 6 ******" << endl;
    cout << "We are now ready to launch the experimentation for this k." << endl;
    cout << "The Experimentation program will build learners for the specified deletion thresholds." << endl;
    experimentation_learner = new Experimentation();
    experimentation_learner->save_files = 0;
    experimentation_learner->experiment_without_missing_indicator = to_deal_with_ind;
    experimentation_learner->target_field_name = to_deal_with_target;
    experimentation_learner->missing_indicator_field_names = missing_indicator_field_names;
    experimentation_learner->experiment_name = experiment_name;
    experimentation_learner->number_of_test_samples = number_of_test_samples;
    experimentation_learner->number_of_train_samples = number_of_train_samples;
    experimentation_learner->reference_train_set = train_set;
    experimentation_learner->target_set = test_train_target_set;
    experimentation_learner->experiment_template = experiment_template;
    experimentation_learner->deletion_thresholds = deletion_thresholds;
    experimentation_learner->experiment_directory = test_train_covariance_file_name + ".metadata";
    experimentation_learner->experiment_directory += "/Experiment/" + experiment_name + "/";
    experimentation_learner->experiment_directory += "K_" + tostring(to_deal_with_k);
    experimentation_learner->setTrainingSet(test_train_neighbor_imputation_file);
}

void NeighborhoodConditionalMean::createMasterHeaderFile()
{
    master_header_length = target_field_names.length() * 2;
    master_header_width = various_ks.length();
    master_header_names.resize(master_header_width);
    master_header_records.resize(master_header_length, master_header_width);
    master_header_records.clear();
    for (master_header_col = 0; master_header_col < master_header_width; master_header_col++)
        master_header_names[master_header_col] = "K_" + tostring(master_header_col);
    master_header_file = new FileVMatrix(master_header_file_name, master_header_length, master_header_names);
    for (master_header_row = 0; master_header_row < master_header_length; master_header_row++)
        for (master_header_col = 0; master_header_col < master_header_width; master_header_col++)
            master_header_file->put(master_header_row, master_header_col, 0.0);
}
void NeighborhoodConditionalMean::getMasterHeaderRecords()
{ 
    master_header_file = new FileVMatrix(master_header_file_name, true);
    master_header_length = master_header_file->length();
    master_header_width = master_header_file->width();
    if (master_header_length != target_field_names.length() * 2)
        PLERROR("In NeighborhoodConditionalMean: master header file length and target_field_names do not agree");
    if (master_header_width != various_ks.length())
        PLERROR("In NeighborhoodConditionalMean: master header file width and various_ks do not agree");
    master_header_records.resize(master_header_length, master_header_width);
    for (master_header_row = 0; master_header_row < master_header_length; master_header_row++)
        for (master_header_col = 0; master_header_col < master_header_width; master_header_col++)
            master_header_records(master_header_row, master_header_col) = master_header_file->get(master_header_row, master_header_col);
}

void NeighborhoodConditionalMean::updateMasterHeaderRecords(int row, int col)
{
    master_header_records(row, col) += 1.0;
    master_header_file->put(row, col, master_header_records(row, col));
    master_header_file->flush();
}

/*
void NeighborhoodConditionalMean::createHeaderFile()
{ 
    for (main_col = 0; main_col < main_width; main_col++)
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

void NeighborhoodConditionalMean::getHeaderRecord()
{ 
    header_file = new FileVMatrix(header_file_name, true);
    header_file->getRow(0, header_record);
    for (main_col = 0; main_col < main_width; main_col++)
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

void NeighborhoodConditionalMean::updateHeaderRecord(int var_col)
{ 
    header_file->put(0, var_col, 3.0);
}

void NeighborhoodConditionalMean::reviewGlobalStats()
{ 
    cout << "There is no more variable to deal with." << endl;
    for (main_col = 0; main_col < main_width; main_col++)
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
*/

void NeighborhoodConditionalMean::train()
{
/*
    PP<ExplicitSplitter> explicit_splitter = new ExplicitSplitter();
    explicit_splitter->splitsets.resize(1,2);
    explicit_splitter->splitsets(0,0) = output_file;
    explicit_splitter->splitsets(0,1) = train_test_file;
    cond_mean = ::PLearn::deepCopy(cond_mean_template);
    cond_mean->setOption("expdir", targeted_metadata + "/TreeCondMean/dir/" + to_deal_with_name);
    cond_mean->splitter = new ExplicitSplitter();
    cond_mean->splitter = explicit_splitter;
    cond_mean->build();
    Vec results = cond_mean->perform(true);
*/
}

int NeighborhoodConditionalMean::outputsize() const {return 0;}
void NeighborhoodConditionalMean::computeOutput(const Vec&, Vec&) const {}
void NeighborhoodConditionalMean::computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const {}
TVec<string> NeighborhoodConditionalMean::getTestCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
}
TVec<string> NeighborhoodConditionalMean::getTrainCostNames() const
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
