// -*- C++ -*-

// EmbeddedSequentialLearner.cc
//
// Copyright (C) 2003 Rejean Ducharme, Yoshua Bengio
// Copyright (C) 2003 Pascal Vincent
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



#include "EmbeddedSequentialLearner.h"
#include "TemporalHorizonVMatrix.h"
#include "TmpFilenames.h"
#include "VMat_maths.h"

namespace PLearn <%
using namespace std;


IMPLEMENT_NAME_AND_DEEPCOPY(EmbeddedSequentialLearner);

EmbeddedSequentialLearner::EmbeddedSequentialLearner()
{}

void EmbeddedSequentialLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(learner, copies);
} 

void EmbeddedSequentialLearner::build_()
{
  if (learner.isNull())
    PLERROR("EmbeddedSequentialLearner::build()_ - learner attribute is NULL");

  learner->build();

  inputsize_ = learner->inputsize();
  outputsize_ = learner->outputsize();
  targetsize_ = learner->targetsize();
  weightsize_ = learner->targetsize();
}

void EmbeddedSequentialLearner::build()
{
  inherited::build();
  build_();
}

void EmbeddedSequentialLearner::declareOptions(OptionList& ol)
{
  declareOption(ol, "learner", &EmbeddedSequentialLearner::learner,
    OptionBase::buildoption, "The underlying learner \n");
}

void EmbeddedSequentialLearner::train(VecStatsCollector& train_stats)
{
  int t = train_set.length();
  if (t >= last_train_t+train_step)
  {
    VMat aligned_set = new TemporalHorizonVMatrix(train_set, horizon, targetsize()); // last training pair is (t-1-horizon,t-1)
    int start = max(0,aligned_set.length()-max_train_len);
    int len = aligned_set.length()-start;
    TmpFilenames tmpfile;
    string index_fname = tmpfile.addFilename();
    VMat aligned_set_non_missing = filter(aligned_set.subMatRows(start,len), index_fname);
    learner->setTrainingSet(aligned_set_non_missing);
    learner->train(train_stats);
    last_train_t = t;
  }
  last_call_train_t = t;
}
 
void EmbeddedSequentialLearner::test(VMat testset, VecStatsCollector& test_stats,
    VMat testoutputs, VMat testcosts)
{
  int l = testset.length();
  VVec input, dummy_input;
  VVec target, dummy_target;
  VVec weight, dummy_weight;
 
  Vec output(testoutputs ?outputsize() :0);
  Vec costs(nTrainCosts());
 
  testset->defineSizes(inputsize(),targetsize(),weightsize());
 
  test_stats.forget();
 
  ProgressBar* pb;
  if(report_progress)
    pb = new ProgressBar("Testing learner",l);

  for (int t=last_call_train_t-1; t<last_call_train_t-1+testset.length(); t++)
  {
    testset.getSample(t-last_call_train_t+1, input, dummy_target, weight);
    testset.getSample(t-last_call_train_t+1+horizon, dummy_input, target, dummy_weight);

/*
    Vec testset_t(testset.width());
    testset->getRow(t-last_call_train_t+1, testset_t);
    Vec input = testset_t.subVec(0,inputsize());
    testset->getRow(t-last_call_train_t+1+horizon, testset_t);
    Vec target = testset_t.subVec(inputsize(),targetsize());
*/
    if (!Vec(input).hasMissing() && !Vec(target).hasMissing())
    {    
      learner->computeOutputAndCosts(input, target, weight, output, costs);

      predictions(t) << output;
      errors(t+horizon) << costs;

      if (testoutputs) testoutputs->putOrAppendRow(t, output);
      if (testcosts) testcosts->putOrAppendRow(t+horizon, costs);

      test_stats.update(costs);

      if (pb)
        pb->update(t-last_call_train_t+1);
    }
  }

  test_stats.finalize();

  if (pb)
    delete pb;
}

void EmbeddedSequentialLearner::forget()
{ learner->forget(); }
 
void EmbeddedSequentialLearner::computeOutput(const VVec& input, Vec& output)
{ learner->computeOutput(input, output); }
 
void EmbeddedSequentialLearner::computeCostsFromOutputs(const VVec& input, const Vec& output,
    const VVec& target, const VVec& weight, Vec& costs)
{ learner->computeCostsFromOutputs(input, output, target, weight, costs); }
 
void EmbeddedSequentialLearner::computeOutputAndCosts(const VVec& input, VVec& target,
    const VVec& weight, Vec& output, Vec& costs)
{ learner->computeOutputAndCosts(input, target, weight, output, costs); }
 
void EmbeddedSequentialLearner::computeCostsOnly(const VVec& input, VVec& target, VVec& weight, Vec& costs)
{ learner->computeCostsOnly(input, target, weight, costs); }

TVec<string> EmbeddedSequentialLearner::getTestCostNames() const
{ return learner->getTestCostNames(); }
 
TVec<string> EmbeddedSequentialLearner::getTrainCostNames() const
{ return learner->getTrainCostNames(); }

/*
void EmbeddedSequentialLearner::computeCost(const Vec& input, const Vec& target, const Vec& output, const Vec& cost)
{ learner->computeCost(input, target, output, cost); }

void EmbeddedSequentialLearner::computeCosts(const VMat& data, VMat costs)
{ learner->computeCosts(data, costs); }
 
void EmbeddedSequentialLearner::use(const Vec& input, Vec& output)
{ learner->use(input, output); }

void EmbeddedSequentialLearner::useAndCost(const Vec& input, const Vec& target, Vec output, Vec cost)
{ learner->useAndCost(input, target, output, cost); }

void EmbeddedSequentialLearner::useAndCostOnTestVec(const VMat& test_set, int i, const Vec& output, const Vec& cost)
{ learner->useAndCostOnTestVec(test_set, i, output, cost); }

void EmbeddedSequentialLearner::apply(const VMat& data, VMat outputs)
{ learner->apply(data, outputs); }

void EmbeddedSequentialLearner::applyAndComputeCosts(const VMat& data, VMat outputs, VMat costs)
{ learner->applyAndComputeCosts(data, outputs, costs); }

void EmbeddedSequentialLearner::applyAndComputeCostsOnTestMat(const VMat& test_set, int i,
    const Mat& output_block, const Mat& cost_block)
{ learner->applyAndComputeCostsOnTestMat(test_set, i, output_block, cost_block); }

void EmbeddedSequentialLearner::computeLeaveOneOutCosts(const VMat& data, VMat costs)
{ learner->computeLeaveOneOutCosts(data, costs); }

void EmbeddedSequentialLearner::computeLeaveOneOutCosts(const VMat& data, VMat costsmat, CostFunc costf)
{ learner->computeLeaveOneOutCosts(data, costsmat, costf); }

Array<string> EmbeddedSequentialLearner::costNames() const
{ return learner->costNames(); }

Array<string> EmbeddedSequentialLearner::testResultsNames() const
{ return learner->testResultsNames(); }

Array<string> EmbeddedSequentialLearner::trainObjectiveNames() const
{ return learner->trainObjectiveNames(); }
*/

%> // end of namespace PLearn

