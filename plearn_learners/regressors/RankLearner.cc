// -*- C++ -*-

// RankLearner.cc
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

/*! \file RankLearner.cc */


#include "RankLearner.h"

namespace PLearn {
using namespace std;

/////////////////
// RankLearner //
/////////////////
RankLearner::RankLearner() 
{}

PLEARN_IMPLEMENT_OBJECT(RankLearner,
                        "Trains another learner to predict the rank of the target, instead of its value.",
                        "The targets of the training set are sorted by increasing value, and the\n"
                        "underlying learner is trained to predict the ranks.\n"
                        "The output of this learner is an interpolation from the targets of the\n"
                        "training set, given the output (predicted rank) of the sub-learner. A\n"
                        "linear interpolation between the two closest targets is used, and the\n"
                        "output is bounded by the lowest and highest targets in the training set.\n"
                        "\n"
                        "The costs computed are those of the sub-learner, and they are preceded\n"
                        "with the 'learner.' prefix. For instance, if the sub-learner computes the\n"
                        "'mse' cost, this learner will rename it into 'learner.mse'.\n"
    );

////////////////////
// declareOptions //
////////////////////
void RankLearner::declareOptions(OptionList& ol)
{

    // Build options.

    // declareOption(ol, "myoption", &RankLearner::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Learnt options.

    declareOption(ol, "sorted_targets", &RankLearner::sorted_targets, OptionBase::learntoption,
                  "The sorted targets of the training set.");

    // Now call the parent class' declareOptions.
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void RankLearner::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void RankLearner::build_()
{
    if (learner_ && learner_->outputsize() >= 0) {
        learner_output.resize(learner_->outputsize());
    }
    // The sub-learner's target is a rank, thus of dimension 1.
    learner_target.resize(1);
    // Currently, only works with 1-dimensional targets.
    last_output.resize(1);
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void RankLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                          const Vec& target, Vec& costs) const
{
    static real desired_rank, val, frac;
    static int n, left, right, mid;
    // Find the desired rank.
    val = target[0];
    n = sorted_targets.length();
    if (val <= sorted_targets[0])
        // Lowest than all targets.
        desired_rank = 0;
    else if (val >= sorted_targets[n - 1])
        // Highest than all targets.
        desired_rank = n-1;
    else {
        // Looking for the closest targets by binary search.
        left = 0;
        right = n - 1;
        while (right > left + 1) {
            mid = (left + right) / 2;
            if (val < sorted_targets[mid])
                right = mid;
            else
                left = mid;
        }
        if (right == left){
            if (left == n - 1)
                left--;
            else
                right++;
        }
        frac = sorted_targets[right] - sorted_targets[left];
        if (frac < 1e-30)
            // Equal targets, up to numerical precision.
            desired_rank = left;
        else
            desired_rank = left + (val - sorted_targets[left]) / frac;
    }
    learner_target[0] = desired_rank;
    if (!fast_exact_is_equal(last_output[0], output[0]))
        // This case is not handled yet.
        PLERROR("In RankLearner::computeCostsFromOutputs - Currently, one can only use computeCostsFromOutputs() "
                "after calling computeOutput.");
    // In this case, the sub-learner's output is the last one computed in computeOutput().
    learner_->computeCostsFromOutputs(input, learner_output, learner_target, costs);
}                                

///////////////////
// computeOutput //
///////////////////
void RankLearner::computeOutput(const Vec& input, Vec& output) const
{
    static real val;
    static int rank_inf;
    learner_->computeOutput(input, learner_output);
#ifdef BOUNDCHECK
    // Safety check to ensure we are only working with 1-dimensional targets.
    if (learner_output.length() != 1)
        PLERROR("In RankLearner::computeOutput - Ranking can only work with 1-dimensional targets");
#endif
    val = learner_output[0];
    if (val <= 0)
        output[0] = sorted_targets[0];
    else if (val >= sorted_targets.length() - 1)
        output[0] = sorted_targets[sorted_targets.length() - 1];
    else {
        rank_inf = int(val);
        output[0] = sorted_targets[rank_inf] + (val - rank_inf) * (sorted_targets[rank_inf + 1] - sorted_targets[rank_inf]);
    }
    last_output[0] = output[0];
}    

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void RankLearner::computeOutputAndCosts(const Vec& input, const Vec& target,
                                        Vec& output, Vec& costs) const {
    // TODO Optimize to take advantage of the sub-learner's method.
    PLearner::computeOutputAndCosts(input, target, output, costs);
}

////////////
// forget //
////////////
void RankLearner::forget()
{
    inherited::forget();
    sorted_targets.resize(0);
}
    
//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> RankLearner::getTestCostNames() const
{
    // Add 'learner.' in front of the sub-learner's costs.
    TVec<string> learner_costs = learner_->getTestCostNames();
    TVec<string> costs(learner_costs.length());
    for (int i = 0; i < costs.length(); i++)
        costs[i] = "learner." + learner_costs[i];
    return costs;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> RankLearner::getTrainCostNames() const
{
    // Add 'learner.' in front of the sub-learner's costs.
    TVec<string> learner_costs = learner_->getTrainCostNames();
    TVec<string> costs(learner_costs.length());
    for (int i = 0; i < costs.length(); i++)
        costs[i] = "learner." + learner_costs[i];
    return costs;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RankLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(sorted_targets, copies);
    deepCopyField(last_output, copies);
    deepCopyField(learner_output, copies);
    deepCopyField(learner_target, copies);
    deepCopyField(ranked_trainset, copies);
}

////////////////
// outputsize //
////////////////
int RankLearner::outputsize() const
{
    // The outputsize is the usual outputsize (the one from the training set).
    // Currently this can only be one, because we only deal with real targets
    // (they are easier to sort).
    return 1;
}

////////////////////
// setTrainingSet //
////////////////////
void RankLearner::setTrainingSet(VMat training_set, bool call_forget) {
    // Some stuff similar to EmbeddedLearner.
    bool training_set_has_changed = !train_set || !(train_set->looksTheSameAs(training_set));
    ranked_trainset = new RankedVMatrix(training_set);
    learner_->setTrainingSet((RankedVMatrix *) ranked_trainset, false);
    if (call_forget && !training_set_has_changed)
        learner_->build();
    // Resize work variable.
    if (learner_->outputsize() >= 0)
        learner_output.resize(learner_->outputsize());
    PLearner::setTrainingSet(training_set, call_forget);
}

///////////
// train //
///////////
void RankLearner::train() {
    // Remember the sorted targets, because we will need them for prediction.
    Mat mat_sorted_targets = ranked_trainset->getSortedTargets().column(0);
    sorted_targets.resize(mat_sorted_targets.length());
    sorted_targets << mat_sorted_targets;
    inherited::train();
}
 
} // end of namespace PLearn


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
