// -*- C++ -*-

// VPLCombinedLearner.h
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
 * $Id: VPLCombinedLearner.h 5480 2006-05-03 18:57:39Z plearner $ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file VPLCombinedLearner.h */


#ifndef VPLCombinedLearner_INC
#define VPLCombinedLearner_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/vmat/VMatLanguage.h>

namespace PLearn {

class VPLCombinedLearner: public PLearner
{

private:

    typedef PLearner inherited;
  
protected:

    VMatLanguage output_prg_;
    VMatLanguage costs_prg_;

    TVec<string> outputnames_;
    TVec<string> costnames_;

    mutable Vec invec_for_output_prg;
    mutable Vec invec_for_costs_prg;

    TVec<int> sublearners_outputsizes;
    TVec<int> sublearners_ntestcosts;
    

    // *********************
    // * protected options *
    // *********************

    TVec<string> orig_fieldnames; // fieldnames of training set
    int orig_inputsize; // inputsize of training set
    int orig_targetsize; // targetsize of training set

    
public:

    // ************************
    // * public build options *
    // ************************

    //! Inner learner which is embedded into the current learner
    TVec< PP<PLearner> > sublearners_;

    string output_prg;
    string costs_prg;
  
    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    // (Make sure the implementation in the .cc
    // initializes all fields to reasonable default values)
    VPLCombinedLearner();


    // ********************
    // * PLearner methods *
    // ********************

private: 

    //! This does the actual building. 
    void build_();

    void initializeOutputPrograms();
    void initializeCostNames();

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
    PLEARN_DECLARE_OBJECT(VPLCombinedLearner);


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

    virtual TVec<string> getOutputNames() const;

    virtual TVec<std::string> getTestCostNames() const;

    virtual TVec<string> getTrainCostNames() const;

    virtual void resetInternalState();

    virtual bool isStatefulLearner() const;
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(VPLCombinedLearner);
  
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
