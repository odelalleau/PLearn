// -*- C++ -*-

// NeighborhoodConditionalMean.h
//
// Copyright (C) 2006 Dan Popovici
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

/*! \file NeighborhoodConditionalMean.h */


#ifndef NeighborhoodConditionalMean_INC
#define NeighborhoodConditionalMean_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/testers/PTester.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/io/load_and_save.h>          //!<  For save
#include <plearn/io/fileutils.h>              //!<  For isfile()
#include <plearn/math/random.h>               //!<  For the seed stuff.
#include <plearn/vmat/ExplicitSplitter.h>     //!<  For the splitter stuff.
#include <plearn_learners/second_iteration/CovariancePreservationImputationVMatrix.h>
#include <plearn_learners/second_iteration/NeighborhoodImputationVMatrix.h>
#include <plearn_learners/second_iteration/BallTreeNearestNeighbors.h>
#include <plearn_learners/second_iteration/Experimentation.h>

namespace PLearn {

/**
 * Generate samples from a mixture of two gaussians
 *
 */
class NeighborhoodConditionalMean : public PLearner
{
    typedef PLearner inherited;

public:

    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!
    
    //! The concatenated test and train input vectors with missing values.
    VMat test_train_input_set;
    //! The corresponding target vectors.
    VMat test_train_target_set;
    //! The number of test samples at the beginning of the test train concatenated sets.
    int number_of_test_samples;
    //! The number of train samples in the reference set to compute the % of missing.
    int number_of_train_samples;
    //! The vector of names of the field to select from the target_set as target for the built training files.
    TVec<string> target_field_names;
    //! The directory offset where to find and/or create the various files.
    string dir_offset;
    //! The vector of various Ks to experiment with. Values must be between 1 and 100.
    TVec<int> various_ks;
    //! The vector of thresholds to be tested for each of the various Ks.
    Vec deletion_thresholds;
    //! The name of the group of experiments to conduct.
    string experiment_name;
    //!  The field names of the missing indicators to exclude when we experiment without them.
    TVec< string > missing_indicator_field_names;
    //! The template of the script to conduct the experiment.
    PP< PTester > experiment_template;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    NeighborhoodConditionalMean();
    int outputsize() const;
    void train();
    void computeOutput(const Vec&, Vec&) const;
    void computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const;
    TVec<string> getTestCostNames() const;
    TVec<string> getTrainCostNames() const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(NeighborhoodConditionalMean);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);    

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();
    void computeNeighborhood();
    void experimentWithVariousKs();
    void createMasterHeaderFile();
    void getMasterHeaderRecords();
    void updateMasterHeaderRecords(int row, int col);

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
    ProgressBar*                              pb;
    PPath                                     train_covariance_name;
    VMat                                      train_covariance_file;
    CovariancePreservationImputationVMatrix*  train_covariance_vmatrix;
    VMat                                      train_covariance_vmat;
    Vec                                       train_covariance_vector;
    PPath                                     test_train_covariance_file_name;
    VMat                                      test_train_covariance_file;
    CovariancePreservationImputationVMatrix*  test_train_covariance_vmatrix;
    VMat                                      test_train_covariance_vmat;
    Vec                                       test_train_covariance_vector;
    PPath                                     test_train_neighborhood_file_name;
    BallTreeNearestNeighbors*                 test_train_neighborhood_learner;
    VMat                                      test_train_neighborhood_file;
    Vec                                       test_train_neighborhood_vector;
    PPath                                     master_header_file_name;
    VMat                                      master_header_file;
    int                                       master_header_length;
    int                                       master_header_width;
    int                                       master_header_row;
    int                                       master_header_col;
    TVec<string>                              master_header_names;
    Mat                                       master_header_records;
    int                                       to_deal_with_k;
    string                                    to_deal_with_target;
    int                                       to_deal_with_ind;
    NeighborhoodImputationVMatrix*            test_train_neighbor_imputation_vmatrix;
    VMat                                      test_train_neighbor_imputation_vmat;
    VMat                                      test_train_neighbor_imputation_file;
    Vec                                       test_train_neighbor_imputation_vector;
    Experimentation*                          experimentation_learner;
    
 /*   
    int main_row;
    int main_col;
    int main_length;
    int main_width;
    Vec main_input;
    TVec<string> main_names;
    StatsCollector  main_stats;
    PPath main_metadata;
    TVec<int> main_ins;
    real main_total;
    real main_missing;
    real main_present;
    int targeted_length;
    int targeted_width;
    Vec targeted_input;
    TVec<string> targeted_names;
    StatsCollector  targeted_stats;
    PPath targeted_metadata;
    real targeted_missing;
    PPath header_file_name;
    VMat header_file;
    Vec header_record;
    int fields_col;
    int fields_width;
    TVec<int> fields_selected;
    int to_deal_with_total;
    int to_deal_with_next;
    real to_deal_with_value;
    string to_deal_with_name;
    int ind_next;
    int output_length;
    int output_width;
    int output_col;
    string output_path;
    TVec<string> output_names;
    Vec output_vec;
    TVec<int> output_variable_src;
    VMat output_file;
    int train_test_length;
    string train_test_path;
    TVec<int> train_test_variable_src;
    VMat train_test_file;
    PP<PTester> cond_mean;
    PPath results_file_name;
    VMat results_file;
    int results_length;
    real results_nstages;
    real results_mse;
    real results_std_err;
    PPath test_output_file_name;
    VMat test_output_file;
    int test_output_length;
*/    
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(NeighborhoodConditionalMean);

} // end of namespace PLearn

#endif


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
