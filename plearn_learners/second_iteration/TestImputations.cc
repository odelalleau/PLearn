// -*- C++ -*-

// TestImputations.cc
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

/*! \file TestImputations.cc */

#define PL_LOG_MODULE_NAME "TestImputations"
#include <plearn/io/pl_log.h>

#include "TestImputations.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    TestImputations,
    "Computes imputations errors using various imputation methods.",
    "name of the discrete variable, of the target and the values to check are options.\n"
);

/////////////////////////
// TestImputations //
/////////////////////////
TestImputations::TestImputations()
{
}
    
////////////////////
// declareOptions //
////////////////////
void TestImputations::declareOptions(OptionList& ol)
{

    declareOption(ol, "min_number_of_samples", &TestImputations::min_number_of_samples,
                  OptionBase::buildoption,
                  "The minimum number of samples required to test imputations for a variable.");
    declareOption(ol, "max_number_of_samples", &TestImputations::max_number_of_samples,
                  OptionBase::buildoption,
                  "The maximum number of samples used to test imputations for a variable.");
    declareOption(ol, "mean_median_mode_file_name", &TestImputations::mean_median_mode_file_name,
                  OptionBase::buildoption,
                  "The Path of the file with those statistics for all the variables.");
    declareOption(ol, "tree_conditional_mean_directory", &TestImputations::tree_conditional_mean_directory,
                  OptionBase::buildoption,
                  "The Path of the dircetory containing the tree conditional means computed for each variable.");
    declareOption(ol, "covariance_preservation_file_name", &TestImputations::covariance_preservation_file_name,
                  OptionBase::buildoption,
                  "The Path of the file with the train_set empirically observed covariances and means.");
    declareOption(ol, "reference_set_with_covpres", &TestImputations::reference_set_with_covpres,
                  OptionBase::buildoption,
                  "The reference set corresponding to the index computed with the ball_tree, with the initial imputations.");
    declareOption(ol, "reference_set_with_missing", &TestImputations::reference_set_with_missing,
                  OptionBase::buildoption,
                  "The reference set corresponding to the index computed with the ball_tree, with missing values.");
    declareOption(ol, "missing_indicators", &TestImputations::missing_indicators,
                  OptionBase::buildoption,
                  "The vector of missing indicator field names to be excluded in the distance computation.");

    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void TestImputations::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(min_number_of_samples, copies);
    deepCopyField(max_number_of_samples, copies);
    deepCopyField(mean_median_mode_file_name, copies);
    deepCopyField(tree_conditional_mean_directory, copies);
    deepCopyField(covariance_preservation_file_name, copies);
    deepCopyField(reference_set_with_covpres, copies);
    deepCopyField(reference_set_with_missing, copies);
    deepCopyField(missing_indicators, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);

}

///////////
// build //
///////////
void TestImputations::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void TestImputations::build_()
{
/*
  for each variable with missing values in the train set(which is in this case the test set)
    - radomly choose up to n samples with a value in the variable
    - build a set with these samples replacing the value with missing
    - perform the various type of imputations and compute the errors
  valider meanmedianmode, treeconditionalmean  covariancepreservation, neighborhood
  create a Mat: width is #of variables with missing values
  row 0: nb_present
  row 1: mean/mode imputation from preprocessing/final_train_input_preprocessed.pmat.metadata/mean_median_mode_file.pmat
  row 2: median/mode imputation from preprocessing/final_train_input_preprocessed.pmat.metadata/mean_median_mode_file.pmat
  row 3: treeconditionalmean imputation from prep/data/targeted_ind_no_imp.vmat.metadata/TreeCondMean/dir/'field_names'/Split0/test1_outputs.pmat
  row 4: covariance preservation imputation from preprocessing/final_train_input_preprocessed.pmat.metadata/covariance_file.pmat
  row 5 to 24: (row - 4) * i neighbors imputation from neighborhood/test_train_imputed_with_covariance_preservation.pmat.metadata/neighborhood_file.pmat
  lire le train_set
*/
    MODULE_LOG << "build_() called" << endl;
    if (train_set)
    {
        build_ball_tree();
        output_file_name = train_metadata + "/TestImputation/output.pmat";
        for (int iteration = 1; iteration <= train_set->width(); iteration++)
        {
            cout << "In TestImputations, Iteration # " << iteration << endl;
            initialize();
            computeMeanMedianModeStats();
            computeTreeCondMeanStats();
            computeCovPresStats();
            computeNeighborhoodStats();
            train();
        }
        endtestimputation("In TestImputations::build_(): we are done here");
    }
}

void TestImputations::build_ball_tree()
{ 
   // initialize primary dataset
    cout << "initialize the train set" << endl;
    train_length = train_set->length();
    train_width = train_set->width();
    train_input.resize(train_width);
    train_names.resize(train_width);
    train_names << train_set->fieldNames();
    train_metadata = train_set->getMetaDataDir();
    weights.resize(train_width);
    weights.fill(1.0);
    for (int mi_col = 0; mi_col < missing_indicators.length(); mi_col++)
    {
        int train_col;
        for (train_col = 0; train_col < train_width; train_col++)
        {
            if (missing_indicators[mi_col] != train_names[train_col]) continue;
            weights[train_col] = 0.0;
            break;
        }
        if (train_col >= train_width)
            PLERROR("In TestImputations::build_ball_tree():: no field with this name in input dataset: %s", (missing_indicators[mi_col]).c_str());
    }
    weighted_distance_kernel = new WeightedDistance(weights);
/*
    if (!reference_set_with_covpres) PLERROR("In TestImputations::build_ball_tree() no reference_set_with_covpres provided.");
    if (!reference_set_with_missing) PLERROR("In TestImputations::build_ball_tree() no reference_set_with_missing provided.");
    ball_tree = new BallTreeNearestNeighbors();
    ball_tree->setOption("rmin", "1");
    ball_tree->setOption("train_method", "anchor");
    ball_tree->setOption("num_neighbors", "100");
    ball_tree->setOption("copy_input", "0");
    ball_tree->setOption("copy_target", "0");
    ball_tree->setOption("copy_weight", "0");
    ball_tree->setOption("copy_index", "1");
    ball_tree->setOption("nstages", "-1");
    ball_tree->setOption("report_progress", "1");
    ball_tree->setTrainingSet(reference_set_with_covpres, true);
    ball_tree->train();
    ref_cov = reference_set_with_covpres->toMat();
    ref_mis = reference_set_with_missing->toMat();
*/
    if (!reference_set_with_covpres) PLERROR("In TestImputations::build_ball_tree() no reference_set_with_covpres provided.");
    if (!reference_set_with_missing) PLERROR("In TestImputations::build_ball_tree() no reference_set_with_missing provided.");
    ball_tree = new ExhaustiveNearestNeighbors();
    ball_tree->setOption("num_neighbors", "100");
    ball_tree->setOption("copy_input", "0");
    ball_tree->setOption("copy_target", "0");
    ball_tree->setOption("copy_weight", "0");
    ball_tree->setOption("copy_index", "1");
    ball_tree->setOption("nstages", "-1");
    ball_tree->setOption("report_progress", "1");
    ball_tree->distance_kernel = weighted_distance_kernel;
    ball_tree->setTrainingSet(reference_set_with_covpres, true);
    ball_tree->train();
    ref_cov = reference_set_with_covpres->toMat();
    ref_mis = reference_set_with_missing->toMat();
/*
ExhaustiveNearestNeighbors(
# bool: Whether the kernel defined by the 'distance_kernel' option should be
# interpreted as a (pseudo-)distance measure (true) or a similarity
# measure (false). Default = true.  Note that this interpretation is
# strictly specific to the class ExhaustiveNearestNeighbors.
kernel_is_pseudo_distance = 1  ;

# Ker: Alternate name for 'distance_kernel'.  (Deprecated; use only so that
# existing scripts can run.)
kernel = *1 ->DistanceKernel(
n = 2 ;
pow_distance = 0 ;
optimized = 0 ;
is_symmetric = 1 ;
report_progress = 0 ;
specify_dataset = *0 ;
cache_gram_matrix = 0 ;
data_inputsize = -1 ;
n_examples = -1  )
 ;

# int: Number of nearest-neighbors to compute.  This is usually called "K".
# The output vector is simply the concatenation of all found neighbors.
# (Default = 1)
num_neighbors = 1  ;

# bool: If true, the output contains a copy of the found input vector(s).
# (Default = false)
copy_input = 0  ;

# bool: If true, the output contains a copy of the found target vector(s).
# (Default = true)
copy_target = 1  ;

# bool: If true, the output contains a copy of the found weight.  If no
# weight is present in the training set, a weight of 1.0 is put.
# (Default = true)
copy_weight = 0  ;

# bool: If true, the output contains the index of the found neighbor
# (as the row number, zero-based, in the training set.)
# (Default = false)
copy_index = 0  ;

# Ker: An optional alternative to the Euclidean distance (DistanceKernel with
# n=2 and pow_distance=1).  It should be a 'distance-like' kernel rather
# than a 'dot-product-like' kernel, i.e. small when the arguments are
# similar, and it should always be non-negative, and 0 only if arguments
# are equal.
distance_kernel = *1 ->DistanceKernel(
n = 2 ;
pow_distance = 0 ;
optimized = 0 ;
is_symmetric = 1 ;
report_progress = 0 ;
specify_dataset = *0 ;
cache_gram_matrix = 0 ;
data_inputsize = -1 ;
n_examples = -1  )
 ;
*/

}
void TestImputations::endtestimputation(const char* msg, ...){
    va_list args;
    va_start(args,msg);
    getHeaderRecord();
    for (int train_col = 0; train_col < train_width; train_col++)
    {
        if (header_record[train_col] == 1.0) 
            PLWARNING("No all variable done!!!");
        else if (header_record[train_col] == 2.0){
            getOutputRecord(train_col);
            if(output_record[100]==0.0)
                PLWARNING("Element %d,%d is at zero in the output file. Meaby this variable was not treated",train_col,100);
        }
    }
    PLERROR(msg,args);
}
void TestImputations::initialize()
{
    
    // initialize the header file
    cout << "initialize the header file" << endl;
    train_set->lockMetaDataDir();
    header_record.resize(train_width);
    header_file_name = train_metadata + "/TestImputation/header.pmat";
    cout << "header_file_name: " << header_file_name << endl;
    if (!isfile(header_file_name)) createHeaderFile();
    else getHeaderRecord();
    
    // choose a variable to test imputations for
    cout << "choose a variable to test imputations for" << endl;
    to_deal_with_total = 0;
    to_deal_with_next = -1;

    for (int train_col = 0; train_col < train_width; train_col++)
    {
        if (header_record[train_col] != 1.0) continue;
        to_deal_with_total += 1;
        if (to_deal_with_next < 0) to_deal_with_next = train_col;
    }
    cout << "total number of variable left to deal with: " << to_deal_with_total << endl;
    if (to_deal_with_next < 0)
    {
        train_set->unlockMetaDataDir();
        // reviewGlobalStats();
        endtestimputation("In TestImputations::initialize() we are done here");
    }
    cout << "next variable to deal with: " << train_names[to_deal_with_next] << "("<<to_deal_with_next<<")"<<endl;
    to_deal_with_name = train_names[to_deal_with_next];
    updateHeaderRecord(to_deal_with_next);
    train_set->unlockMetaDataDir();
    
    // find the available samples with non-missing values for this variable
    train_stats = train_set->getStats(to_deal_with_next);
    train_total = train_stats.n();
    train_missing = train_stats.nmissing();
    train_present = train_total - train_missing;
    indices.resize((int) train_present);
    int ind_next = 0;
    ProgressBar* pb = new ProgressBar( "Building the indices for " + to_deal_with_name, train_length);
    for (int train_row = 0; train_row < train_length; train_row++)
    {
        to_deal_with_value = train_set->get(train_row, to_deal_with_next);
        if (is_missing(to_deal_with_value)) continue;
        if (ind_next >= indices.length()) 
            PLERROR("In TestImputations::initialize() There seems to be more present values than indicated by the stats file");
        indices[ind_next] = train_row;
        ind_next += 1;
        pb->update( train_row );
    }
    delete pb;
    
    // shuffle the indices.
    manual_seed(123456);
    shuffleElements(indices);
    
    // load the test samples for this variable
    if (indices.length() > max_number_of_samples) test_length = max_number_of_samples;
    else if (indices.length() < min_number_of_samples)
        PLERROR("TestImputations::initialize() Their is less examples for the variable %s then the min_number_of semples(%d)",
                to_deal_with_name.c_str(),min_number_of_samples);
    else test_length = indices.length();
    test_width = train_width;
    test_samples_set = new MemoryVMatrix(test_length, test_width);
    pb = new ProgressBar( "Loading the test samples for " + to_deal_with_name, test_length);
    for (int test_row = 0; test_row < test_length; test_row++)
    {
        train_set->getRow(indices[test_row], train_input);
        test_samples_set->putRow(test_row, train_input);
        pb->update( test_row );
    }
    delete pb;
}

void TestImputations::computeMeanMedianModeStats()
{
    if (!isfile(mean_median_mode_file_name)) PLERROR("In TestImputations::computeMeanMedianModeStats() a valid mean_median_mode_file path must be provided.");
    VMat mmmf_file = new FileVMatrix(mean_median_mode_file_name);
    int mmmf_length = mmmf_file->length();
    int mmmf_width = mmmf_file->width();
    if (mmmf_length != 3) PLERROR("In TestImputations::computeMeanMedianModeStats() there should be exactly 3 records in the mmm file, got %i.", mmmf_length);
    if (mmmf_width != train_width) PLERROR("In TestImputations::computeMeanMedianModeStats() train set and mmm width should be the same, got %i.", mmmf_width);
    real mmmf_mean = mmmf_file->get(0, to_deal_with_next);
    real mmmf_median = mmmf_file->get(1, to_deal_with_next);
    real mmmf_mode = mmmf_file->get(2, to_deal_with_next);
    mmmf_mean_err = 0.0;
    mmmf_median_err = 0.0;
    mmmf_mode_err = 0.0;
    ProgressBar* pb = new ProgressBar( "computing the mean, median and mode imputation errors for " + to_deal_with_name, test_length);
    for (int test_row = 0; test_row < test_length; test_row++)
    {
        to_deal_with_value = test_samples_set->get(test_row, to_deal_with_next);
        mmmf_mean_err += pow(to_deal_with_value - mmmf_mean, 2);
        mmmf_median_err += pow(to_deal_with_value - mmmf_median, 2);
        mmmf_mode_err += pow(to_deal_with_value - mmmf_mode, 2);
        pb->update( test_row );
    }
    delete pb;
    mmmf_mean_err = mmmf_mean_err / (real) test_length;
    mmmf_median_err = mmmf_median_err / (real) test_length;
    mmmf_mode_err = mmmf_mode_err / (real) test_length;
    //TODO check the formul
    //mmmf_mean_stddev = sqrt(mmmf_mean_err);
    //mmmf_median_stddev = sqrt(mmmf_median_err);
    //mmmf_mode_stddev = sqrt(mmmf_mode_err);

}

void TestImputations::computeTreeCondMeanStats()
{
    tcmf_file_name = tree_conditional_mean_directory + "/" + to_deal_with_name + "/Split0/test1_outputs.pmat";
    if (!isfile(tcmf_file_name)) 
        PLERROR("In TestImputations::computeTreeCondMeanStats(): The '%s' file was not found in the tcf directory.",tcmf_file_name.c_str());
    tcmf_file = new FileVMatrix(tcmf_file_name);
    int tcmf_length = tcmf_file->length();
    int tcmf_width = tcmf_file->width();
    if (tcmf_length < train_length) 
        PLERROR("In TestImputations::computeTreeCondMeanStats(): there are only %d records in the tree conditional output file. We need %d.",tcmf_length,train_length);
    tcmf_mean_err = 0.0;
    ProgressBar* pb = new ProgressBar( "computing the tree conditional mean imputation errors for " + to_deal_with_name, test_length);
    for (int test_row = 0; test_row < test_length; test_row++)
    {
        to_deal_with_value = test_samples_set->get(test_row, to_deal_with_next);
        tcmf_mean_err += pow(to_deal_with_value - tcmf_file->get(indices[test_row], 0), 2);
        pb->update( test_row );
    }
    delete pb;
    tcmf_mean_err = tcmf_mean_err / (real) test_length;
    //TODO check the formul
    //tcmf_mean_stddev = sqrt(tcmf_mean_err);
}

void TestImputations::computeCovPresStats()
{
    if (!isfile(covariance_preservation_file_name)) PLERROR("In TestImputations::computeCovPresStats() a valid covariance_preservation_file path must be provided.");
    VMat cvpf_file = new FileVMatrix(covariance_preservation_file_name);
    int cvpf_length = cvpf_file->length();
    int cvpf_width = cvpf_file->width();
    if (cvpf_length != train_width + 1)
        PLERROR("In TestImputations::computeCovPresStats() there should be %i records in the cvp file, got %i.", train_width + 1, cvpf_length);
    if (cvpf_width != train_width)
        PLERROR("In TestImputations::computeCovPresStats() train set and cvp width should be the same, got %i.", cvpf_width);
    //cvpf_file = new FileVMatrix(covariance_preservation_file_name);
    cvpf_cov.resize(train_width, train_width);
    cvpf_mu.resize(train_width);
    for (int cvpf_row = 0; cvpf_row < train_width; cvpf_row++)
    {
        for (int cvpf_col = 0; cvpf_col < train_width; cvpf_col++)
        {
            cvpf_cov(cvpf_row, cvpf_col) = cvpf_file->get(cvpf_row, cvpf_col);
        }
    }
    for (int cvpf_col = 0; cvpf_col < train_width; cvpf_col++)
    {
        cvpf_mu[cvpf_col] = cvpf_file->get(train_width, cvpf_col);
    }
    cvpf_mean_err = 0.0;
    ProgressBar* pb = new ProgressBar( "computing the covariance preservation imputation errors for " + to_deal_with_name, test_length);
    for (int test_row = 0; test_row < test_length; test_row++)
    {
        test_samples_set->getRow(test_row, train_input);
        cvpf_mean_err += pow(to_deal_with_value - covariancePreservationValue(to_deal_with_next), 2);
        pb->update( test_row );
    }
    delete pb;
    cvpf_mean_err = cvpf_mean_err / (real) test_length;
    //TODO check the formul
    //cvpf_mean_stddev = sqrt(cvpf_mean_err);

}

real TestImputations::covariancePreservationValue(int col)
{
    real cvpf_sum_cov_xl = 0;
    real cvpf_sum_xl_square = 0;
    for (int cvpf_col = 0; cvpf_col < train_width; cvpf_col++)
    {
        if (cvpf_col == col) continue;
        if (is_missing(train_input[cvpf_col])) continue;
        cvpf_sum_cov_xl += cvpf_cov(cvpf_col, col) * (train_input[cvpf_col] - cvpf_mu[cvpf_col]);
        cvpf_sum_xl_square += (train_input[cvpf_col] - cvpf_mu[cvpf_col]) * (train_input[cvpf_col] - cvpf_mu[cvpf_col]);
    }
    real cvpf_value;
    if (cvpf_sum_xl_square == 0.0) cvpf_value = cvpf_mu[col];
    else cvpf_value = cvpf_mu[col] + cvpf_sum_cov_xl / cvpf_sum_xl_square;
    return cvpf_value;
}

void TestImputations::computeNeighborhoodStats()
{
    knnf_input.resize(train_width);
    knnf_neighbors.resize(100);
    knnf_mean_err.resize(100);
    knnf_mean_err.clear();
    ProgressBar* pb = new ProgressBar( "computing the neighborhood imputation errors for " + to_deal_with_name, test_length);
    for (int test_row = 0; test_row < test_length; test_row++)
    {
        test_samples_set->getRow(test_row, train_input);
        for (int test_col = 0; test_col < train_width; test_col++)
        {
            if (test_col == to_deal_with_next) knnf_input[test_col] = covariancePreservationValue(test_col);
            else if (is_missing(train_input[test_col])) knnf_input[test_col] = covariancePreservationValue(test_col);
            else knnf_input[test_col] = train_input[test_col];
        }
        ball_tree->computeOutput(knnf_input, knnf_neighbors);
        real knnf_sum_value = 0.0;
        real knnf_sum_cov = 0.0;
        real knnv_value_count = 0.0;
        for (int knnf_row = 0; knnf_row < knnf_neighbors.size(); knnf_row++)
        {
            real knnf_value = ref_mis((int) knnf_neighbors[knnf_row], to_deal_with_next);
            if (!is_missing(knnf_value))
            {
                knnf_sum_value += knnf_value;
                knnv_value_count += 1.0;
            }
            if (knnv_value_count > 0.0)
            {
                knnf_mean_err[knnf_row] += pow(to_deal_with_value - (knnf_sum_value / knnv_value_count), 2);
                continue;
            }
            knnf_value = ref_cov((int) knnf_neighbors[knnf_row], to_deal_with_next);
            if (is_missing(knnf_value))
                PLERROR("In TestImputations::computeNeighborhoodStats(): missing value found in the reference with covariance preserved at: %i , %i",
                         (int) knnf_neighbors[knnf_row], to_deal_with_next);
            knnf_sum_cov += knnf_value;
            knnf_mean_err[knnf_row] += pow(to_deal_with_value - (knnf_sum_cov / (knnf_row + 1)), 2);
        }
        pb->update( test_row );
    }
    delete pb;
    for (int knnf_row = 0; knnf_row < knnf_mean_err.size(); knnf_row++) knnf_mean_err[knnf_row] = knnf_mean_err[knnf_row] /  (real) test_length;
}

void TestImputations::createHeaderFile()
{ 
    cout << "in createHeaderFile()" << endl;
    for (int train_col = 0; train_col < train_width; train_col++)
    {
        train_stats = train_set->getStats(train_col);
        train_total = train_stats.n();
        train_missing = train_stats.nmissing();
        train_present = train_total - train_missing;
        if (train_missing <= 0.0) header_record[train_col] = 0.0;                       // no missing, noting to do.
        else if (train_present < min_number_of_samples){
            header_record[train_col] = -1.0; // should not happen
            PLERROR("In TestImputations::createHeaderFile: train_present(%d) < min_number_of_samples (%d)",
                    train_present,min_number_of_samples);
        }
        else header_record[train_col] = 1.0;                                            // test imputations
    }
    header_file = new FileVMatrix(header_file_name, 1, train_names);
    header_file->putRow(0, header_record);
}

void TestImputations::getHeaderRecord()
{ 
    header_file = new FileVMatrix(header_file_name, true);
    header_file->getRow(0, header_record);
}

void TestImputations::updateHeaderRecord(int var_col)
{ 
    header_file->put(0, var_col, 2.0);
    header_file->flush();
}

void TestImputations::train()
{
    // initialize the output file
    cout << "initialize the output file: " << output_file_name << endl;
    train_set->lockMetaDataDir();
    output_record.resize(knnf_mean_err.size()+5);
    if (!isfile(output_file_name)) createOutputFile();
    else getOutputRecord(to_deal_with_next);
    output_record[0] = mmmf_mean_err;
    output_record[1] = mmmf_median_err;
    output_record[2] = mmmf_mode_err;
    output_record[3] = tcmf_mean_err;
    output_record[4] = cvpf_mean_err;
    for (int knnf_row = 0; knnf_row < knnf_mean_err.size(); knnf_row++)
    {
       output_record[knnf_row + 5] = knnf_mean_err[knnf_row];
    }
    updateOutputRecord(to_deal_with_next);
    train_set->unlockMetaDataDir();
}

void TestImputations::createOutputFile()
{
    output_names.resize(knnf_mean_err.size()+5);
    output_names[0] = "mean";
    output_names[1] = "median";
    output_names[2] = "mode";
    output_names[3] = "tree_cond";
    output_names[4] = "cov_pres";
    for (int knnf_row = 0; knnf_row < knnf_mean_err.size(); knnf_row++)
    {
       output_names[knnf_row + 5] = "KNN_" + tostring(knnf_row);
    }
    output_record.clear();
    output_file = new FileVMatrix(output_file_name, train_width, output_names);
    for (int train_col = 0; train_col < train_width; train_col++)
        output_file->putRow(train_col, output_record);
}

void TestImputations::getOutputRecord(int var_col)
{ 
    output_file = new FileVMatrix(output_file_name, true);
    output_record.resize(output_file->width());
    output_file->getRow(var_col, output_record);
}

void TestImputations::updateOutputRecord(int var_col)
{ 
    output_file->putRow(var_col, output_record);
    output_file->flush();
}

int TestImputations::outputsize() const {return 0;}
void TestImputations::computeOutput(const Vec&, Vec&) const {}
void TestImputations::computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const {}
TVec<string> TestImputations::getTestCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
}
TVec<string> TestImputations::getTrainCostNames() const
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
