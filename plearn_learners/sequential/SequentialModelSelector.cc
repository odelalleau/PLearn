// -*- C++ -*-

// SequentialModelSelector.cc
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



#include "SequentialModelSelector.h"
#include "TMat_maths_impl.h"

namespace PLearn <%
using namespace std;


IMPLEMENT_NAME_AND_DEEPCOPY(SequentialModelSelector);

SequentialModelSelector::SequentialModelSelector()
  : init_train_size(1), cost_index(0), cost_type(SumCost)
{}

void SequentialModelSelector::train(VecStatsCollector& train_stats)
{
  ProgressBar* pb;
  if (report_progress)
    pb = new ProgressBar("Training learner",train_set.length());

  VecStatsCollector dummy_stats;
  best_model.resize(train_set.length());
  for (int t=init_train_size+horizon; t<=train_set.length(); t++)
  {
    int start = max(t-max_train_len,init_train_size-1);
    VMat sub_train = train_set.subMatRows(0,t-horizon); // last training pair is (t-1-2*horizon,t-1-horizon)
    VMat sub_test  = train_set.subMatRows(0,t); // last test pair is (t-1-horizon,t-1) (input,target)
    for (int i=0; i<models.size(); i++)
    {
      models[i]->setTrainingSet(sub_train);
      models[i]->train(dummy_stats);
      models[i]->test(sub_test, dummy_stats); // last cost computed goes at t-1, last prediction at t-1-horizon
      Vec sequence_errors = models[i]->errors.subMat(start+horizon,cost_index,t-start-horizon,1).toVecCopy();  // VERIFIER LES INDICES!!!
      sequence_costs[i] = sequenceCost(sequence_errors);
    }
    // we set the best model for this time step
    best_model[t] = argmin(sequence_costs);
    if (predictions(t-1-horizon).hasMissing())
    {
      predictions(t-1-horizon) << models[best_model[t]]->predictions(t-1-horizon);
      errors(t-1) << models[best_model[t]]->errors(t-1);
    }

    // now train with everything that is available
    for (int i=0; i<models.size(); i++)
    {
      models[i]->setTrainingSet(train_set);
      if (i == best_model[t])
        models[i]->train(train_stats);
      else
        models[i]->train(dummy_stats);
    }

    if (pb)
      pb->update(t);
  }
  last_train_t = train_set.length();

  if (pb)
   delete pb; 
}

void SequentialModelSelector::test(VMat test_set, VecStatsCollector& test_stats,
    VMat testoutputs, VMat testcosts)
{
  ProgressBar* pb;
  if (report_progress)
    pb = new ProgressBar("Testing learner",test_set.length());

  VecStatsCollector dummy_stats;
  // first test example is the pair (last_call_train_t-1,last_call_train_t-1+horizon)
  for (int i=0; i<models.size(); i++)
  {
    if (i == best_model[last_train_t])
      models[i]->test(test_set, test_stats, testoutputs, testcosts);
    else
      models[i]->test(test_set, dummy_stats);
  }
  for (int t=last_test_t; t<test_set.length(); t++)
  {
    predictions(t) << models[best_model[last_train_t]]->predictions(t);
    errors(t) << models[best_model[last_train_t]]->errors(t);
  }
/*
  for (int t=last_test_t-1; t<test_set.length()-horizon; t++)
  {
    predictions(t) << models[best_model[last_train_t]]->predictions(t);
    errors(t+horizon) << models[best_model[last_train_t]]->errors(t+horizon);
  }
*/
  last_test_t = test_set.length();
}

void SequentialModelSelector::computeOutput(const Vec& input, Vec& output)
{
  models[best_model[last_train_t]]->computeOutput(input, output);
}

void SequentialModelSelector::computeCostsFromOutputs(const Vec& input, const Vec& output, const Vec& target, Vec& costs)
{
  models[best_model[last_train_t]]->computeCostsFromOutputs(input, output, target, costs);
}

void SequentialModelSelector::computeOutputAndCosts(const Vec& input,
    const Vec& target, Vec& output, Vec& costs)
{
  models[best_model[last_train_t]]->computeOutputAndCosts(input, target, output, costs);
}
 
void SequentialModelSelector::computeCostsOnly(const Vec& input,
    const Vec& target, Vec& costs)
{
  models[best_model[last_train_t]]->computeCostsOnly(input, target, costs);
}

TVec<string> SequentialModelSelector::getTestCostNames() const
{ return models[best_model[last_train_t]]->getTestCostNames(); }

TVec<string> SequentialModelSelector::getTrainCostNames() const
{ return models[best_model[last_train_t]]->getTrainCostNames(); }

void SequentialModelSelector::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(models, copies);
  //deepCopyField(mean_costs, copies);
} 

void SequentialModelSelector::build_()
{
  //mean_costs.resize(max_train_len,models.size());
  sequence_costs.resize(models.size());
  forget();
}

void SequentialModelSelector::build()
{
  inherited::build();
  build_();
}

void SequentialModelSelector::declareOptions(OptionList& ol)
{
  declareOption(ol, "models", &SequentialModelSelector::models,
    OptionBase::buildoption, "list of all the models \n");

  declareOption(ol, "init_train_size", &SequentialModelSelector::init_train_size,
    OptionBase::buildoption, "size of first training set \n");

  declareOption(ol, "cost_index", &SequentialModelSelector::cost_index,
    OptionBase::buildoption, " which element of costs vector is use to select best model \n");

  declareOption(ol, "cost_type", &SequentialModelSelector::cost_type,
    OptionBase::buildoption, " the type of cost to be used to select best model \n");

  inherited::declareOptions(ol);
}

real SequentialModelSelector::sequenceCost(const Vec& sequence_errors)
{
  real cost_sequence = 0;

  switch (cost_type)
  {
    case SumCost:
      cost_sequence = sum(sequence_errors);
      break;

    case SharpeRatio:
      PLERROR("SharpeRatio cost_type not defined!");
      break;

    default:
      PLERROR("Invalid cost_type!");
      break;
  }

  return cost_sequence;
}

void SequentialModelSelector::forget()
{
  last_train_t = init_train_size;
  best_model.resize(last_train_t);
  best_model.fill(0);  // by default
  //mean_costs.fill(MISSING_VALUE);
  for (int i=0; i<models.size(); i++)
    models[i]->forget();
}

%> // end of namespace PLearn

