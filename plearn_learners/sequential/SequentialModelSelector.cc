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


PLEARN_IMPLEMENT_OBJECT_METHODS(SequentialModelSelector, "SequentialModelSelector", SequentialLearner);

SequentialModelSelector::SequentialModelSelector()
  : init_train_size(1), cost_index(0), cost_type("SumCost")
{}

void SequentialModelSelector::train()
{
  ProgressBar* pb;
  if (report_progress)
    pb = new ProgressBar("Training SequentialModelSelector learner",train_set.length());

  PP<VecStatsCollector> dummy_stats = new VecStatsCollector();
  int start = MAX(init_train_size+horizon, last_train_t+1);
  for (int t=start; t<=train_set.length(); t++)
  {
#ifdef DEBUG
    cout << "SequentialModelSelector::train() -- sub_train.length = " << t-horizon << " et sub_test.length = " << t << endl;
#endif
    //int start = max(t-max_train_len,init_train_size-1);
    VMat sub_train = train_set.subMatRows(0,t-horizon); // last training pair is (t-1-2*horizon,t-1-horizon)
    sub_train->defineSizes(train_set->inputsize(), train_set->targetsize(), train_set->weightsize());
    VMat sub_test  = train_set.subMatRows(0,t); // last test pair is (t-1-horizon,t-1) (input,target)
    sub_test->defineSizes(train_set->inputsize(), train_set->targetsize(), train_set->weightsize());
    for (int i=0; i<models.size(); i++)
    {
      models[i]->setTrainingSet(sub_train, false);
      models[i]->setTrainStatsCollector(dummy_stats);
      models[i]->train();
      models[i]->test(sub_test, dummy_stats); // last cost computed goes at t-1, last prediction at t-1-horizon
      Vec sequence_errors = remove_missing(models[i]->errors.subMat(0,cost_index,t,1).toVecCopy());
      sequence_costs[i] = sequenceCost(sequence_errors);
    }
    // we set the best model for this time step
    best_model[t] = argmin(sequence_costs);
#ifdef DEBUG
    cout << "SequentialModelSelector::train() -- t = " << t << " et best_model = " << best_model[t] << endl;
#endif
    if (predictions(t-1-horizon).hasMissing())
    {
      predictions(t-1-horizon) << models[best_model[t]]->predictions(t-1-horizon);
      errors(t-1) << models[best_model[t]]->errors(t-1);
    }

#ifdef DEBUG
    cout << "SequentialModelSelector::train() -- train_set.length = " << t << endl;
#endif
    // now train with everything that is available
    sub_train = train_set.subMatRows(0,t); // last training pair is (t-1-horizon,t-1)
    for (int i=0; i<models.size(); i++)
    {
      models[i]->setTrainingSet(train_set, false);
      models[i]->setTrainStatsCollector((i==best_model[t]) ? train_stats : dummy_stats);
      models[i]->train();
    }

    if (pb)
      pb->update(t);
  }
  last_train_t = MAX(train_set.length(), last_train_t);
#ifdef DEBUG
  cout << "SequentialModelSelector.last_train_t = " << last_train_t << endl;
#endif

  string s1 = append_slash(expdir) + "predictions_train_t=" + tostring(last_train_t); //"seq_model/predictions_train_t=" + tostring(last_train_t);
  string s2 = append_slash(expdir) + "errors_train_t=" + tostring(last_train_t);
  saveAsciiWithoutSize(s1, predictions);
  saveAsciiWithoutSize(s2, errors);

  if (pb)
   delete pb; 
}

void SequentialModelSelector::test(VMat test_set, PP<VecStatsCollector> test_stats,
    VMat testoutputs, VMat testcosts) const
{
  ProgressBar* pb;
  if (report_progress)
    pb = new ProgressBar("Testing SequentialModelSelector learner",test_set.length());

  PP<VecStatsCollector> dummy_stats = new VecStatsCollector();
  // first test example is the pair (last_call_train_t-1,last_call_train_t-1+horizon)
#ifdef DEBUG
  cout << "SequentialModelSelector::test() -- test_set.length = " << test_set.length() << endl;
#endif
  for (int i=0; i<models.size(); i++)
  {
    if (i == best_model[last_train_t])
      models[i]->test(test_set, test_stats, testoutputs, testcosts);
    else
      models[i]->test(test_set, dummy_stats);
  }
  int start = MAX(init_train_size,last_test_t+1);
  start = MAX(last_train_t+1,start);
  for (int t=start; t<test_set.length(); t++)
  {
    int best_t = MIN(t, last_train_t);
#ifdef DEBUG
    cout << "SequentialModelSelector::test() -- t = " << t << ", best_t = " << best_t << " et best_model = " << best_model[best_t] << endl;
#endif
    predictions(t) << models[best_model[best_t]]->predictions(t);
    errors(t) << models[best_model[best_t]]->errors(t);
  }
  last_test_t = MAX(test_set.length()-1, last_test_t);
#ifdef DEBUG
  cout << "SequentialModelSelector.last_test_t = " << last_test_t << endl;
#endif

  string s1 = append_slash(expdir) + "predictions_test_t=" + tostring(last_test_t);
  string s2 = append_slash(expdir) + "errors_test_t=" + tostring(last_test_t);
  saveAsciiWithoutSize(s1, predictions);
  saveAsciiWithoutSize(s2, errors);
}

void SequentialModelSelector::computeOutput(const Vec& input, Vec& output) const
{
  models[best_model[last_train_t]]->computeOutput(input, output);
}

void SequentialModelSelector::computeCostsFromOutputs(const Vec& input,
    const Vec& output, const Vec& target, Vec& costs) const
{
  models[best_model[last_train_t]]->computeCostsFromOutputs(input, output, target, costs);
}

void SequentialModelSelector::computeOutputAndCosts(const Vec& input,
    const Vec& target, Vec& output, Vec& costs) const
{
  models[best_model[last_train_t]]->computeOutputAndCosts(input, target, output, costs);
}
 
void SequentialModelSelector::computeCostsOnly(const Vec& input,
    const Vec& target, Vec& costs) const
{
  models[best_model[last_train_t]]->computeCostsOnly(input, target, costs);
}

TVec<string> SequentialModelSelector::getTestCostNames() const
{ return models[0]->getTestCostNames(); }

TVec<string> SequentialModelSelector::getTrainCostNames() const
{ return models[0]->getTrainCostNames(); }

void SequentialModelSelector::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  parentclass::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(models, copies);
  //deepCopyField(mean_costs, copies);
} 

void SequentialModelSelector::build_()
{
  best_model.resize(max_seq_len);
  sequence_costs.resize(models.size());
  for (int i=0; i<models.size(); i++)
    models[i]->horizon = horizon;

  forget();
}

void SequentialModelSelector::build()
{
  build_();
  parentclass::build();
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

  parentclass::declareOptions(ol);
}

real SequentialModelSelector::sequenceCost(const Vec& sequence_errors)
{
  real cost_sequence = 0;

  if (cost_type == "SumCost")
    cost_sequence = mean(sequence_errors);
  else if (cost_type == "SharpeRatio")
    PLERROR("SharpeRatio cost_type not defined!");
  else
    PLERROR("Invalid cost_type!");

  return cost_sequence;
}

void SequentialModelSelector::forget()
{
  last_train_t = init_train_size;
  best_model.resize(max_seq_len);
  best_model.fill(0);  // by default
  for (int i=0; i<models.size(); i++)
    models[i]->forget();

  parentclass::forget();
}

%> // end of namespace PLearn

