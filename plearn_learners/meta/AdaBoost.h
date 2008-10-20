// -*- C++ -*-

// AdaBoost.h
//
// Copyright (C) 2003  Pascal Vincent 
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

// Authors: Yoshua Bengio

/*! \file AdaBoost.h */

#ifndef AdaBoost_INC
#define AdaBoost_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {
using namespace std;

class AdaBoost: public PLearner
{
    typedef PLearner inherited;

    //! Global storage to save memory allocations.
    mutable Vec tmp_output2;
    mutable Vec weighted_costs;
    mutable Vec sum_weighted_costs;
protected:
    // average weighted error of each learner
    Vec learners_error;
    // weighing scheme over examples
    Vec example_weights;

    //! Used to store outputs from the weak learners.
    mutable Vec weak_learner_output;

    // *********************
    // * protected options *
    // *********************

    // saved options:
    // (unnormalized) weight associated to each weak learner
    Vec voting_weights;
    real sum_voting_weights; // = sum(voting_weights);
    real initial_sum_weights;

    //! Vector of weak learners learned from boosting
    TVec< PP<PLearner> > weak_learners;

    //! Indication that a weak learner with 0 training error has been found
    bool found_zero_error_weak_learner;

public:

    // ************************
    // * public build options *
    // ************************

    //! Weak learner to use as a template for each boosting round.
    //! AdaBoost requires classification weak-learners that provide an
    //! essential non-linearity (e.g. linear regression does not work)
    //! NOTE: this weak learner must support deepCopy().
    PP<PLearner> weak_learner_template;

    // normally 0.5
    real target_error;

    // whether to give an expdir to the underlying weak learners
    bool provide_learner_expdir;
  
    // threshold on output of weak learner to decide if class 0 or class 1
    real output_threshold;

    // whether to compute training error during training
    bool compute_training_error;

    // use Pseudo-loss Adaboost
    bool pseudo_loss_adaboost;

    // use Confidence-rated adaboost
    bool conf_rated_adaboost;

    // use resampling (vs weighting) to train the underlying classifier
    bool weight_by_resampling;

    // stop if weak learner does not seem to help
    bool early_stopping;

    // save model after each stage into <expdir>/model.psave
    bool save_often;

    // Did we add the sub_learner_costs to our costs
    bool forward_sub_learner_test_costs;

    // Did we modif directly the train_set weights?
    bool modif_train_set_weights;
    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    AdaBoost();


    // ******************
    // * PLearner methods *
    // ******************

private: 
    //! This does the actual building. 
    void build_();

    void computeTrainingError(Vec input, Vec target);

protected: 
    //! Declares this class' options
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods
    //  If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
    PLEARN_DECLARE_OBJECT(AdaBoost);


    // **************************
    // **** PLearner methods ****
    // **************************

    //! returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options)
    //! This implementation of AdaBoost always performs two-class
    //! classification, hence returns 1
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
    virtual void         setTrainingSet(VMat training_set, bool call_forget=true);


};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(AdaBoost);
  
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
