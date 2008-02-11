// -*- C++ -*-

// MergeDond2Files.h
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

/*! \file MergeDond2Files.h */


#ifndef MergeDond2Files_INC
#define MergeDond2Files_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/vmat/FileVMatrix.h>

namespace PLearn {

/**
 * Generate samples from a mixture of two gaussians
 *
 */
class MergeDond2Files : public PLearner
{
    typedef PLearner inherited;

public:

    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!
    
    //! The secondary dataset to merge with the main one.
    //! The main one is provided as the train_set.
    VMat external_dataset;

    //! The variable missing regeneration instructions in the form of pairs field : instruction.
    //! Supported instructions are skip, as_is, zero_is_missing, 2436935_is_missing.
    TVec< pair<string, string> >  missing_instructions;
    
    //! The variable merge instructions in the form of pairs field : instruction.
    //! Supported instructions are skip, mean, mode, present.
    TVec< pair<string, string> >  merge_instructions;
    
    //! The file name of the merge file to be created.
    string merge_path;
    
    //! The column of the merge key in the secondary dataset.
    int sec_key;
    
    //! The column of the merge key in the main dataset.
    int main_key;
    
    //! The column of the indicator of a train record in the main dataset.
    int train_ind;
    
    //! The column of the indicator of a test record in the main dataset.
    int test_ind;
    
    //! The train file created.
    VMat train_file;
    
    //! The test file created.
    VMat test_file;
    
    //! The unknown target file created.
    VMat unknown_file;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    MergeDond2Files();
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
    PLEARN_DECLARE_OBJECT(MergeDond2Files);

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
    void mergeFiles();
    void accumulateVec();
    void combineAndPut();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
    // secondary dataset variables
    int sec_length;
    int sec_width;
    int sec_row;
    Vec sec_input;
    TVec<string> sec_names;
    TVec<string> sec_ins;
    int extension_width;
    int ext_col;
    TVec<int> extension_pos;
    TVec<string> extension_names;
    Mat sec_values;
    Mat sec_value_cnt;
    TVec<int> sec_value_ind;
    bool sec_value_found;
    real sec_value_count_max;
    
    // primary dataset variables
    int main_length;
    int main_width;
    int main_row;
    Vec main_input;
    TVec<string> main_names;
    TVec<string> main_ins;
    int primary_width;
    TVec<string> primary_names;
    
    // merge dataset variables
    int train_length;
    int test_length;
    int unknown_length;
    int merge_width;
    int train_row;
    int test_row;
    int unknown_row;
    int merge_col;
    Vec merge_output;
    TVec<string> merge_names;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(MergeDond2Files);

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
