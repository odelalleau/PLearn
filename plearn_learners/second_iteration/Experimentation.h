// -*- C++ -*-

// Experimentation.h
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

/*! \file Experimentation.h */


#ifndef Experimentation_INC
#define Experimentation_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/testers/PTester.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>

namespace PLearn {

/**
 * Generate samples from a mixture of two gaussians
 *
 */
class Experimentation : public PLearner
{
    typedef PLearner inherited;

public:

    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!
    
    //! If set to 1, save the built train and test files instead of running the experiment.
    int save_files;
    //! If set to 1, the missing_indicator_field_names will be excluded from the built training files.
    int experiment_without_missing_indicator;
    //! The name of the field to select from the target_set as target for the built training files.
    string target_field_name;
    //! The field names of the missing indicators to exclude when we experiment without them.
    TVec<string> missing_indicator_field_names;
    //! The name of the group of experiments to conduct.
    string experiment_name;
    //! The number of test samples at the beginning of the train set.
    int number_of_test_samples;
    //! The number of train samples in the reference set to compute the % of missing.
    int number_of_train_samples;
    //! The train and valid set with missing values to compute the % of missing.
    VMat reference_train_set;
    //! The data set with the targets corresponding to the train set.
    VMat target_set;
    //! The vector of the various deletion threshold to run this experiment with.
    Vec deletion_thresholds;
    //! The path in which to build the directories for the experiment's results.
    PPath experiment_directory;
    //! The template of the script to conduct the experiment
    PP<PTester> experiment_template;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    Experimentation();
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
    PLEARN_DECLARE_OBJECT(Experimentation);

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
    void experimentSetUp();
    void createHeaderFile();
    void getHeaderRecord();
    void updateHeaderRecord(int var_col);
    void setSourceDataset();
    void reviewGlobalStats();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
    
    int          main_row;
    int          main_col;
    int          main_length;
    int          main_width;
    Vec          main_input;
    TVec<string> main_names;
    PPath        main_metadata;
    int          main_fields_selected_col;
    TVec<string> main_fields_selected;
    
    int          fields_width;
    int          fields_col;
    
    int          target_row;
    int          target_col;
    int          target_length;
    int          target_width;
    Vec          target_input;
    TVec<string> target_names;
    
    int             header_width;
    Vec             header_record;
    TVec<string>    header_names;
    string          header_expdir;
    PPath           header_file_name;
    PP<FileVMatrix> header_file;
    
    int          to_deal_with_total;
    int          to_deal_with_next;
    real         deletion_threshold;
    string       deletion_threshold_str;
    
    int          test_length;
    int          test_width;
    Vec          test_record;
    VMat         test_file;
    
    int          train_valid_length;
    int          train_valid_width;
    Vec          train_valid_record;
    VMat         train_valid_file;
    
    VMat         source_set;
    TVec<string> source_names;
    PP<PTester>  experiment;
    
/*
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
DECLARE_OBJECT_PTR(Experimentation);

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
