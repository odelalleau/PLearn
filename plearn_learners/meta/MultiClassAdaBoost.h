// -*- C++ -*-

// plearn_learners/meta/MultiClassAdaBoost.h
//
// Copyright (C) 2007 Frederic Bastien
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

// Authors: Frederic Bastien

/*! \file plearn_learners/meta/MultiClassAdaBoost.h */


#ifndef MultiClassAdaBoost_INC
#define MultiClassAdaBoost_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/meta/AdaBoost.h>
#include <plearn/misc/PTimer.h>

namespace PLearn {

/**
 * The first sentence should be a BRIEF DESCRIPTION of what the class does.
 * Place the rest of the class programmer documentation here.  Doxygen supports
 * Javadoc-style comments.  See http://www.doxygen.org/manual.html
 *
 * @todo Write class to-do's here if there are any.
 *
 * @deprecated Write deprecated stuff here if there is any.  Indicate what else
 * should be used instead.
 */
class MultiClassAdaBoost : public PLearner
{
    typedef PLearner inherited;

    mutable Vec tmp_input;
    mutable Vec tmp_target;
    mutable Vec tmp_output;
    mutable Vec tmp_costs;

    mutable Vec output1;
    mutable Vec output2;
    mutable Vec subcosts1;
    mutable Vec subcosts2;

    mutable TVec<VMat> saved_testset;
    mutable TVec<VMat> saved_testset1;
    mutable TVec<VMat> saved_testset2;

    //! The time it took for the last execution of the train() function
    real train_time;
    //! The total time passed in training
    real total_train_time;

    //! The time it took for the last execution of the test() function
    real test_time;
    //! The total time passed in test()
    real total_test_time;

    bool time_costs;
    bool warn_once_target_gt_2;
    mutable bool done_warn_once_target_gt_2;

    PP<PTimer> timer;

    //Variable used when forward_test==2.
    //They are used to determine if inherited::test is faster then forwarding the test fct.
    mutable real time_sum;
    mutable real time_sum_ft;
    mutable real time_last_stage;
    mutable real time_last_stage_ft;
    mutable int last_stage;
    mutable int nb_sequential_ft;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    //! Did we add the learner1 and learner2 costs to our costs
    bool forward_sub_learner_test_costs;

    //! Did we forward the test function to the sub learner?
    uint16_t forward_test;

    //! The learner1 and learner2 must be trained!
    PP<AdaBoost> learner1;
    PP<AdaBoost> learner2;
    PP<AdaBoost> learner_template;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    MultiClassAdaBoost();


    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    virtual void finalize();

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    virtual void train();

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    void computeCostsFromOutputs_(const Vec& input, const Vec& output,
                                  const Vec& target, Vec& sub_costs1,
                                  Vec& sub_costs2, Vec& costs) const;
    virtual TVec<string> getOutputNames() const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;


    // *** SUBCLASS WRITING: ***
    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                       Vec& output, Vec& costs) const;
    // virtual void computeCostsOnly(const Vec& input, const Vec& target,
    //                               Vec& costs) const;
    virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
                      VMat testoutputs=0, VMat testcosts=0) const;
    // virtual int nTestCosts() const;
    // virtual int nTrainCosts() const;
    // virtual void resetInternalState();
    // virtual bool isStatefulLearner() const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(MultiClassAdaBoost);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

    void getSubLearnerTarget(const Vec target, TVec<Vec> sub_target)const;
private:
    //#####  Private Data Members  ############################################
    mutable TVec<Vec> sub_target_tmp;

    string targetname;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(MultiClassAdaBoost);

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
