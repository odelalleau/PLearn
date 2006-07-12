// -*- C++ -*-

// VPLProcessor.h
//
// Copyright (C) 2005, 2006 Pascal Vincent 
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

/* *******************************************************      
 * $Id: VPLProcessor.h 5480 2006-05-03 18:57:39Z plearner $ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file VPLProcessor.h */


#ifndef VPLProcessor_INC
#define VPLProcessor_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/vmat/VMatLanguage.h>

namespace PLearn {

class VPLProcessor: public PLearner
{

private:

    typedef PLearner inherited;
  
protected:

    VMatLanguage input_prg_;

    TVec<string> input_prg_fieldnames; 

    mutable Vec processed_input;

    // *********************
    // * protected options *
    // *********************

    TVec<string> orig_fieldnames; // fieldnames of training set
    int orig_inputsize; // inputsize of training set
    int orig_targetsize; // targetsize of training set

    bool use_filtering_prg_for_repeat;
    string repeat_id_field_name; // 0, 1, ..., n-1; "" means no field is added
    string repeat_count_field_name; // n; "" means no field is added
    
public:

    // ************************
    // * public build options *
    // ************************


    string filtering_prg;

    string input_prg;
    string target_prg;
    string weight_prg;
    string extra_prg;
  
    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    // (Make sure the implementation in the .cc
    // initializes all fields to reasonable default values)
    VPLProcessor();


    // ********************
    // * PLearner methods *
    // ********************

private: 

    //! This does the actual building. 
    void build_();

    void initializeInputPrograms();


protected: 
  
    //! Declares this class' options.
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods.
    // If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT.
    PLEARN_DECLARE_OBJECT(VPLProcessor);


    // **************************
    // **** PLearner methods ****
    // **************************

    virtual int outputsize() const;

    
    //! Forwarded to inner learner
    virtual void forget();

    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    virtual void computeOutput(const Vec& input, Vec& output) const;

    virtual void computeOutputAndCosts(const Vec& input, const Vec& target, 
                                       Vec& output, Vec& costs) const;

    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;

    //! If there's an output_prg, it returns output_prg_fieldnames 
    //! If there's no output_prg, the call is forwarded to the inner learner
    virtual TVec<string> getOutputNames() const;


    virtual VMat processDataSet(VMat dataset) const;



    virtual void train();
    virtual TVec<std::string> getTestCostNames() const;
    virtual TVec<string> getTrainCostNames() const;



};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(VPLProcessor);
  
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
