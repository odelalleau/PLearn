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
   * $Id: RankLearner.cc,v 1.2 2004/10/29 17:46:44 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file RankLearner.cc */


#include "RankLearner.h"
#include <plearn/vmat/RankedVMatrix.h>

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
    "For the moment, no costs are computed. Access to sub-learner costs\n"
    "should be implemented in the future.\n"
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
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void RankLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
  // No cost computed.
  costs.resize(0);
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
}    

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void RankLearner::computeOutputAndCosts(const Vec& input, const Vec& target,
                                   Vec& output, Vec& costs) const {
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
  // No cost computed.
  return TVec<string>();
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> RankLearner::getTrainCostNames() const
{
  // No cost computed.
  return TVec<string>();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RankLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("RankLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
  PLearner::setTrainingSet(training_set, call_forget);
  PP<RankedVMatrix> ranked_trainset = new RankedVMatrix(training_set);
  // Avoid calling learner_->forget() twice.
  learner_->setTrainingSet((RankedVMatrix *) ranked_trainset, false);
  // Remember the sorted targets, because we will need them for prediction.
  Mat mat_sorted_targets = ranked_trainset->getSortedTargets().column(0);
  sorted_targets.resize(mat_sorted_targets.length());
  sorted_targets << mat_sorted_targets;
  // Resize work variable.
  if (learner_->outputsize() >= 0)
    learner_output.resize(learner_->outputsize());
}

} // end of namespace PLearn
