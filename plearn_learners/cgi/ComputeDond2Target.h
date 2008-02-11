// -*- C++ -*-

// ComputeDond2Target.h
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

/*! \file ComputeDond2Target.h */


#ifndef ComputeDond2Target_INC
#define ComputeDond2Target_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/vmat/FileVMatrix.h>

namespace PLearn {

/**
 * Generate samples from a mixture of two gaussians
 *
 */
class ComputeDond2Target : public PLearner
{
    typedef PLearner inherited;

public:

    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    //! The variables to assemble in the input vector by names.
    //! To ease the following steps they are grouped with the binary variables first,
    //! the discrete variables, the continuous variables and finally some variables unused in the training.
    TVec<string> input_vector;
    
    //! If set to 1 and annual sales attribute is missing, the class will be set to missing.
    int unknown_sales;
    
    //! The column of the real annual sales used to compute the real class target.
    int target_sales;
    
    //! The column of the predicted annual sales used to compute the predicted class target.
    int predicted_sales;
    
    //! The column of the total authorized margins including SLA.
    int margin;
    
    //! The column of the total loan balances excluding mortgages.
    int loan;
    
    //! The file path for the targeted output file.
    string output_path;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    ComputeDond2Target();
    int outputsize() const;
    void train();
    void computeOutput(const Vec&, Vec&) const;
    void computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const;
    TVec<string> getTestCostNames() const;
    TVec<string> getTrainCostNames() const;
    VMat getOutputFile();


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(ComputeDond2Target);

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
    void computeTarget();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
    
    // input instructions variables
    int ins_width;
    int ins_col;
    
    // primary dataset variables
    int main_length;
    int main_width;
    int main_row;
    int main_col;
    Vec main_input;
    TVec<string> main_names;
    
    // output dataset variables
    int target_class;
    int predicted_class;
    int output_width;
    real commitment;
    Vec output_vec;
    TVec<string> output_names;
    TVec<int> output_variable_src;
    VMat output_file;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ComputeDond2Target);

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
