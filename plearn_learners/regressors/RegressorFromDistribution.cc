// -*- C++ -*-

// RegressorFromDistribution.cc
//
// Copyright (C) 2006 Olivier Delalleau
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

/*! \file RegressorFromDistribution.cc */


#include "RegressorFromDistribution.h"
#include <plearn/vmat/ForwardVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RegressorFromDistribution,
    "Regression from a distribution trained on both the input and target data",
    "This regressor outputs E[target|input], where this expectation is\n"
    "computed by an underlying conditional distribution, trained with a"
    "predictor part corresponding to the input part, and the predicted part\n"
    "corresponding to the target part.\n"
);

///////////////////////////////
// RegressorFromDistribution //
///////////////////////////////
RegressorFromDistribution::RegressorFromDistribution()
{}

void RegressorFromDistribution::declareOptions(OptionList& ol)
{
    declareOption(ol, "distribution", &RegressorFromDistribution::distribution,
                                      OptionBase::buildoption,
        "The underlying (conditional) distribution. Its predictor and\n"
        "predicted sizes will be set automatically.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void RegressorFromDistribution::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void RegressorFromDistribution::build_()
{
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RegressorFromDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("RegressorFromDistribution::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////
// outputsize //
////////////////
int RegressorFromDistribution::outputsize() const
{
    if (!distribution)
        return -1;
    else
        return distribution->getNPredicted();
}

////////////
// forget //
////////////
void RegressorFromDistribution::forget()
{
    inherited::forget();
    if (distribution)
        distribution->forget();
}

///////////
// train //
///////////
void RegressorFromDistribution::train()
{
    if (!distribution)
        PLERROR("In RegressorFromDistribution::train - You need to specify a "
                "distribution before calling the train() method");
    distribution->train();
}

///////////////////
// computeOutput //
///////////////////
void RegressorFromDistribution::computeOutput(const Vec& input, Vec& output) const
{
    assert( distribution );
    distribution->setPredictor(input);
    distribution->expectation(output);
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void RegressorFromDistribution::computeCostsFromOutputs(
        const Vec& input, const Vec& output,
        const Vec& target, Vec& costs) const
{
    costs.resize(1);
    costs[0] = powdistance(target, output);
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> RegressorFromDistribution::getTestCostNames() const
{
    static TVec<string> test_costs;
    if (test_costs.isEmpty())
        test_costs.append("mse");
    return test_costs;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> RegressorFromDistribution::getTrainCostNames() const
{
    static TVec<string> no_train_cost;
    return no_train_cost;
}

////////////////////
// setTrainingSet //
////////////////////
void RegressorFromDistribution::setTrainingSet(VMat training_set,
                                               bool call_forget)
{
    inherited::setTrainingSet(training_set, call_forget);
    assert( training_set->inputsize()  >= 0 &&
            training_set->targetsize() >= 1    );
    // Create a dataset whose input part is the concatenation of the input and
    // target parts of the training set.
    VMat all_input_trainset = new ForwardVMatrix(training_set);
    all_input_trainset->build();
    all_input_trainset->defineSizes(training_set->inputsize() +
            training_set->targetsize(), 0, training_set->weightsize(),
            training_set->extrasize());
    assert( distribution );
    // Note that 'call_forget' is set to false in the following call, because
    // if it was true, then distribution->forget() would have already been
    // called in this->forget().
    distribution->setTrainingSet(all_input_trainset, false);
    // Set sizes for the underlying distribution.
    distribution->setPredictorPredictedSizes(training_set->inputsize(),
                                             training_set->targetsize());
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
