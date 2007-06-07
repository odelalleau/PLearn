// -*- C++ -*-

// AnalyzeFieldStats.h
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

/*! \file AnalyzeFieldStats.h */


#ifndef AnalyzeFieldStats_INC
#define AnalyzeFieldStats_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/testers/PTester.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>

namespace PLearn {

/**
 * Generate samples from a mixture of two gaussians
 *
 */
class AnalyzeFieldStats : public PLearner
{
    typedef PLearner inherited;

public:

    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!
    
    //! The minimum number of samples required to train the learner.
    int min_number_of_samples;
    //! The maximum number of samples used to train the learner.
    int max_number_of_samples;
    //! The train and test data sets with the target field.
    VMat targeted_set;
    //! The template of the script to learn the conditional mean
    PP<PTester> cond_mean_template;
    //! The field name of the variable to be analyzed.
    TVec<string> fields;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    AnalyzeFieldStats();
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
    PLEARN_DECLARE_OBJECT(AnalyzeFieldStats);

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
    void analyzeVariableStats();
    void createHeaderFile();
    void getHeaderRecord();
    void updateHeaderRecord(int var_col);
    void reviewGlobalStats();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
    
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
    
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(AnalyzeFieldStats);

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
