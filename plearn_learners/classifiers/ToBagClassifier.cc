// -*- C++ -*-

// ToBagClassifier.cc
//
// Copyright (C) 2007 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file ToBagClassifier.cc */


#include "ToBagClassifier.h"
#include <plearn/var/SumOverBagsVariable.h>
#include <plearn/vmat/SubVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ToBagClassifier,
    "Makes an existing classifier operate on bags.",
    "Training is performed by simply removing bag information.\n"
    "For testing, a majority vote is performed on each bag: assuming the\n"
    "inner learner's output is made of the probabilities for each class,\n"
    "these probabilities are summed over a full bag, and the class with\n"
    "highest sum is taken as prediction.\n"
    "This learner can also compute the confusion matrix as a test cost, in\n"
    "addition to classification error. Each element of the confusion matrix\n"
    "is named 'cm_ij' with i the index of the true class, and j the index of\n"
    "the predicted class.");

/////////////////////
// ToBagClassifier //
/////////////////////
ToBagClassifier::ToBagClassifier():
    n_classes(-1)
{}

////////////////////
// declareOptions //
////////////////////
void ToBagClassifier::declareOptions(OptionList& ol)
{
    declareOption(ol, "n_classes", &ToBagClassifier::n_classes,
                  OptionBase::buildoption,
        "Number of classes in the dataset. This option is required to\n"
        "compute the confusion matrix, but may be ignored otherwise.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void ToBagClassifier::build_()
{
}

///////////
// build //
///////////
void ToBagClassifier::build()
{
    inherited::build();
    build_();
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void ToBagClassifier::computeCostsFromOutputs(const Vec& input,
                                              const Vec& output,
                                              const Vec& target,
                                              Vec& costs) const
{
    fillSubTarget(target);
    inherited::computeCostsFromOutputs(input, output, sub_target, costs);
    updateCostAndBagOutput(target, output, costs);
}

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void ToBagClassifier::computeOutputAndCosts(const Vec& input,
                                            const Vec& target,
                                            Vec& output, Vec& costs) const
{
    fillSubTarget(target);
    inherited::computeOutputAndCosts(input, sub_target, output, costs);
    updateCostAndBagOutput(target, output, costs);
}

///////////////////
// fillSubTarget //
///////////////////
void ToBagClassifier::fillSubTarget(const Vec& target) const
{
    sub_target.resize(target.length() - 1);
    sub_target << target.subVec(0, sub_target.length());
}


//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> ToBagClassifier::getTestCostNames() const
{
    TVec<string> costs;
    costs.append("class_error");
    if (n_classes > 0)
        for (int i = 0; i < n_classes; i++)
            for (int j = 0; j < n_classes; j++)
                costs.append("cm_" + tostring(i) + tostring(j));
    return costs;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ToBagClassifier::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ToBagClassifier::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// setTrainingSet //
////////////////////
void ToBagClassifier::setTrainingSet(VMat training_set, bool call_forget)
{
    // Remove bag information (last target).
    PLCHECK( training_set->weightsize() == 0 &&
             training_set->extrasize() == 0 ); // Not compatible yet.
    PP<SubVMatrix> sub_train_set = new SubVMatrix(training_set, 0, 0, 
                                                  training_set->length(),
                                                  training_set->width() - 1);
    sub_train_set->defineSizes(training_set->inputsize(),
                               training_set->targetsize() - 1,
                               training_set->weightsize(),
                               training_set->extrasize());
    setInnerLearnerTrainingSet(get_pointer(sub_train_set), call_forget);
    PLearner::setTrainingSet(training_set, call_forget);
}

////////////////
// targetsize //
////////////////
int ToBagClassifier::targetsize() const
{
    return learner_->targetsize() + 1;
}

////////////////////////////
// updateCostAndBagOutput //
////////////////////////////
void ToBagClassifier::updateCostAndBagOutput(const Vec& target,
                                             const Vec& output,
                                             Vec& costs) const
{
    PLASSERT( is_equal(sum(output), 1) );   // Ensure probabilities sum to 1.
    int bag_info = int(round(target.lastElement()));
    if (bag_info & SumOverBagsVariable::TARGET_COLUMN_FIRST)
        bag_output.resize(0, 0);
    bag_output.appendRow(output);
    costs.resize(nTestCosts());
    costs.fill(MISSING_VALUE);
    if (bag_info & SumOverBagsVariable::TARGET_COLUMN_LAST) {
        // Perform majority vote.
        votes.resize(bag_output.width());
        columnSum(bag_output, votes);
        int target_class = int(round(target[0]));
        int prediction = argmax(votes);
        if (prediction == target_class)
            costs[0] = 0;
        else
            costs[0] = 1;
        if (n_classes > 0) {
            int i_start = 1 + target_class * n_classes;
            costs.subVec(i_start, n_classes).fill(0);
            costs[i_start + prediction] = 1;
        }
    }
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
