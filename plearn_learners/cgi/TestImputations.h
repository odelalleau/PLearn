// -*- C++ -*-

// TestImputations.h
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

/*! \file TestImputations.h */


#ifndef TestImputations_INC
#define TestImputations_INC

#include <cstdarg>

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/testers/PTester.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/io/load_and_save.h>          //!<  For save
#include <plearn/io/fileutils.h>              //!<  For isfile()
#include <plearn/math/random.h>               //!<  For the seed stuff.
#include <plearn/vmat/ExplicitSplitter.h>     //!<  For the splitter stuff.
#include <plearn_learners/nearest_neighbors/BallTreeNearestNeighbors.h>
#include <plearn_learners/nearest_neighbors/ExhaustiveNearestNeighbors.h>
#include <plearn_learners/cgi/CovariancePreservationImputationVMatrix.h>
#include <plearn_learners/cgi/NeighborhoodImputationVMatrix.h>
#include <plearn_learners/cgi/WeightedDistance.h>

namespace PLearn {

/**
 * Generate samples from a mixture of two gaussians
 *
 */
class TestImputations : public PLearner
{
    typedef PLearner inherited;

public:

    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!
    
    //! The minimum number of samples required to test imputations for a variable.
    int min_number_of_samples;
    //! The maximum number of samples used to test imputations for a variable.
    int max_number_of_samples;
    //! The Path of the file with those statistics for all the variables.
    PPath mean_median_mode_file_name;
    //! The Path of the dircetory containing the tree conditional means computed for each variable.
    PPath tree_conditional_mean_directory;
    //! The Path of the file with the train_set empirically observed covariances and means.
    PPath covariance_preservation_file_name;
    //! The reference set corresponding to the index computed with the ball_tree, with the initial imputations.
    VMat reference_set_with_covpres;
    //! The reference set corresponding to the index computed with the ball_tree, with missing values.
    VMat reference_set_with_missing;
    //! The vector of missing indicator field names to be excluded in the distance computation.
    TVec<string> missing_indicators;
    
public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    TestImputations();
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
    PLEARN_DECLARE_OBJECT(TestImputations);

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
    void build_ball_tree(int nb_neighbors);
    void initialize();
    void computeMeanMedianModeStats();
    void computeTreeCondMeanStats();
    void computeCovPresStats();
    real covariancePreservationValue(int col);
    void computeNeighborhoodStats(int nb_neighbors,int max_miss_neigbors);
    void createHeaderFile();
    void getHeaderRecord();
    void updateHeaderRecord(int var_col);
    void createOutputFile();
    void getOutputRecord(int var_col);
    void updateOutputRecord(int var_col);
    void endtestimputation(const char* msg, ...);
private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
    ExhaustiveNearestNeighbors* ball_tree;
    Mat ref_cov;
    Mat ref_mis;
    int train_length;
    int train_width;
    Vec train_input;
    TVec<string> train_names;
    PPath train_metadata;
    StatsCollector train_stats;
    int train_total;
    int train_missing;
    int train_present;
    PPath header_file_name;
    VMat header_file;
    Vec header_record;
    int to_deal_with_total;
    int to_deal_with_next;
    string to_deal_with_name;
    real to_deal_with_value;
    VMat test_samples_set;
    TVec<int> indices;
    int test_length;
    int test_width;
    real mmmf_mean_err;
    real mmmf_median_err;
    real mmmf_mode_err;
    PPath tcmf_file_name;
    VMat tcmf_file;
    real tcmf_mean_err;
    Mat cvpf_cov;
    Vec cvpf_mu;
    real cvpf_mean_err;
    Vec knnf_input;
    Vec knnf_neighbors;
    Vec knnf_mean_cov_err;
    Vec knnf_mean_miss_err;
    Vec knnf_nmiss_value_count;
    Vec weights;
    WeightedDistance* weighted_distance_kernel;
    PPath output_file_name;
    VMat output_file;
    Vec output_record;
    TVec<string> output_names;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(TestImputations);

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
