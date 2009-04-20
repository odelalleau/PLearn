// -*- C++ -*-

// AddCostToLearner.h
//
// Copyright (C) 2004 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file AddCostToLearner.h */

#ifndef AddCostToLearner_INC
#define AddCostToLearner_INC

#include "EmbeddedLearner.h"
#include "PLearner.h"
#include <plearn/var/VarArray.h>
#include <plearn/var/Var.h>

namespace PLearn {
using namespace std;

class AddCostToLearner: public EmbeddedLearner
{

private:

    typedef EmbeddedLearner inherited;

    //! Global storage to save memory allocations.
    mutable Vec combined_output;
  
protected:

    // *********************
    // * protected options *
    // *********************

    // Fields below are not options.

    //! Used to store the outputs of the sub-learner for each sample in a bag.
    mutable Mat bag_outputs;

    //! Used to count the number of instances in a bag.
    mutable int bag_size;
    
    //! Propagation path for the cross entropy cost.
    mutable VarArray cross_entropy_prop;
  
    //! Variable giving the cross entropy cost.
    Var cross_entropy_var;
  
    //! Used to store the target when computing a cost.
    mutable Vec desired_target;

    //! A precomputed factor for faster mapping.
    real fac;

    //! Constraints on the output given the costs being computed.
    real output_max;
    real output_min;

    //! Its value is sub_learner_output[0].
    Var output_var;
  
    //! Used to store the sub-learner output.
    Vec sub_learner_output;

    //! Used to store the input given to the sub-learner, when it needs to be
    //! copied in a separate place.
    mutable Vec sub_input;

    //! Its value is desired_target[0].
    Var target_var;

    //! The threshold between class
    Vec class_threshold;

    //! The time it took for the last execution of the train() function
    real train_time;
    //! The total time passed in training
    real total_train_time;

    //! The time it took for the last execution of the test() function
    real test_time;
    //! The total time passed in test()
    real total_test_time;

    bool train_time_b;
    bool test_time_b;

public:

    // ************************
    // * public build options *
    // ************************

    bool check_output_consistency;
    int combine_bag_outputs_method;
    bool compute_costs_on_bags;
    TVec<string> costs;
    bool force_output_to_target_interval;
    real from_max;
    real from_min;
    bool rescale_output;
    bool rescale_target;
    real to_max;
    real to_min;
    int n_classes;
    int confusion_matrix_target;
    bool find_class_threshold;

    // ****************
    // * Constructors *
    // ****************

    AddCostToLearner();

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

    //! Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(AddCostToLearner);

    // **************************
    // **** PLearner methods ****
    // **************************

    virtual void train();

    virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
                      VMat testoutputs=0, VMat testcosts=0) const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!).
    virtual void forget();

    //! Computes our and from the sub_learner costs from already computed output. 
    //! If 'add_sub_learner_costs' is true, then the underlying learner will be
    //! used to compute its own cost on the given input, output and target.
    //! Otherwise, these costs are not computed (they are assumed to be already
    //! given in the first part of the 'costs' vector).
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs,
                                         bool add_sub_learner_costs) const;

    //! Computes our and from the sub_learner costs from already computed output. 
    void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const{
        computeCostsFromOutputs(input,output,target,costs,true);
    }

    //! Overridden to use default PLearner behavior.
    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                       Vec& output, Vec& costs) const;

    //! Overridden to use the sublearner version and complete it
    virtual void computeOutputsAndCosts(const Mat& input, const Mat& target,
                                       Mat& output, Mat& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and thus the test method).
    virtual TVec<string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes and 
    //! for which it updates the VecStatsCollector train_stats.
    virtual TVec<string> getTrainCostNames() const;

    //! Overridden because of the specific bag behavior.
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(AddCostToLearner);
  
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
