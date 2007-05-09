// -*- C++ -*-

// VPLPreprocessedLearner.h
//
// Copyright (C) 2005 Pascal Vincent 
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
 * $Id$ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file VPLPreprocessedLearner.h */


#ifndef VPLPreprocessedLearner_INC
#define VPLPreprocessedLearner_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/vmat/VMatLanguage.h>

namespace PLearn {

class VPLPreprocessedLearner: public PLearner
{

private:

    typedef PLearner inherited;
  
protected:

    // *********************
    // * protected options *
    // *********************

    // ### declare protected option fields (such as learnt parameters) here
    PP<VMatLanguage> row_prg;
    PP<VMatLanguage> input_prg;
    PP<VMatLanguage> output_prg;
    PP<VMatLanguage> costs_prg;

    TVec<string> row_prg_fieldnames; 
    TVec<string> input_prg_fieldnames;
    TVec<string> output_prg_fieldnames;
    TVec<string> costs_prg_fieldnames;

    mutable Vec row;
    mutable Vec processed_row;
    mutable Vec processed_input;
    mutable Vec pre_output;
    mutable Vec pre_costs;
    
public:

    // ************************
    // * public build options *
    // ************************

    //! Inner learner which is embedded into the current learner
    PP<PLearner> learner_;

    string trainset_preproc; // program string in VPL language to be applied to each row of the training set
    int newtargetsize;
    int newweightsize;
    int newextrasize;

    string input_preproc; // program string in VPL language to be applied to an input 
    string output_postproc;
    string costs_postproc;
  
    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    // (Make sure the implementation in the .cc
    // initializes all fields to reasonable default values)
    VPLPreprocessedLearner();


    // ********************
    // * PLearner methods *
    // ********************

private: 

    //! This does the actual building. 
    // (Please implement in .cc)
    void build_();

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
    PLEARN_DECLARE_OBJECT(VPLPreprocessedLearner);


    // **************************
    // **** PLearner methods ****
    // **************************

    virtual int outputsize() const;

    
    //! Forwarded to inner learner
    virtual void setValidationSet(VMat validset);

    //! Forwarded to inner learner
    virtual void setTrainStatsCollector(PP<VecStatsCollector> statscol);

    //! Forwarded to inner learner
    virtual void setExperimentDirectory(const PPath& the_expdir);
 
    //! Forwarded to inner learner
    virtual void forget();

    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    virtual void train();

    virtual void computeOutput(const Vec& input, Vec& output) const;

    virtual void computeOutputAndCosts(const Vec& input, const Vec& target, 
                                       Vec& output, Vec& costs) const;

    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;

    virtual
    bool computeConfidenceFromOutput(const Vec& input, const Vec& output,
                                     real probability,
                                     TVec< pair<real,real> >& intervals) const;

    //! If there's an output_prg, it returns output_prg_fieldnames 
    //! If there's no output_prg, the call is forwarded to the inner learner
    virtual TVec<string> getOutputNames() const;

    virtual TVec<std::string> getTestCostNames() const;

    //! Forwarded to inner learner
    virtual TVec<string> getTrainCostNames() const;

    //! Forwarded to inner learner
    virtual void resetInternalState();

    //! Forwarded to inner learner
    virtual bool isStatefulLearner() const;
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(VPLPreprocessedLearner);
  
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
