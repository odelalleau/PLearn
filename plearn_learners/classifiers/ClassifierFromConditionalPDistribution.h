
// -*- C++ -*-

// ClassifierFromConditionalPDistribution.h
//
// Copyright (C) 20032-2005  Pascal Vincent & Olivier Delalleau
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
 * $Id: ClassifierFromConditionalPDistribution.h 3994 2005-08-25 13:35:03Z chapados $ 
 ******************************************************* */

/*! \file ClassifierFromConditionalPDistribution.h */
#ifndef ClassifierFromConditionalPDistribution_INC
#define ClassifierFromConditionalPDistribution_INC

#include <plearn_learners/distributions/PDistribution.h>
#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

//! Classifier that takes a ConditionalPDistribution and classifies with it,
//! by finding the target class y that maximizes p(y|x), where x is
//! the input.
class ClassifierFromConditionalPDistribution: public PLearner
{

private:

    typedef PLearner inherited;

protected:

    mutable Vec target_values;
    mutable Vec pcd_input;
    mutable Vec pcd_output;

public:

    // ************************
    // * public build options *
    // ************************

    //! Number of classes (doesn't need to be specified if train_set has
    //! a Dictionary for the target column.
    int nclasses;
    //! ConditionalPDistribution, that will be trained if this learner is trained
    PP<PDistribution> pcd;
    //! Output type (choose among: "predicted_class", "class_probabilities" or "class_log-probabilities")
    string output_type;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    ClassifierFromConditionalPDistribution();

    // ******************
    // * PLearner methods *
    // ******************

private: 

    //! This does the actual building. 
    void build_();

protected: 

    //! Declares this class' options
    static void declareOptions(OptionList& ol);

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! Simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods
    //  If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
    PLEARN_DECLARE_OBJECT(ClassifierFromConditionalPDistribution);


    // **************************
    // **** PLearner methods ****
    // **************************

    //! returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options)
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    virtual void forget();

    
    //! The role of the train method is to bring the learner up to stage==nstages,
    //! updating the train_stats collector with training costs measured on-line in the process.
    virtual void train();


    //! Computes the output from the input
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output. 
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;
                                

    //! Returns the names of the costs computed by computeCostsFromOutpus (and thus the test method)
    virtual TVec<string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes and 
    //! for which it updates the VecStatsCollector train_stats
    virtual TVec<string> getTrainCostNames() const;


};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ClassifierFromConditionalPDistribution);
  
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
