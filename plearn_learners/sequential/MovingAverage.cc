// -*- C++ -*-

// MovingAverage.cc
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



#include "MovingAverage.h"

namespace PLearn <%
using namespace std;


IMPLEMENT_NAME_AND_DEEPCOPY(MovingAverage);

MovingAverage::MovingAverage()
  : window_length(-1)
{}

void MovingAverage::build_()
{
  if(cost_funcs.size() < 1)
    PLERROR("In MovingAverage::build_()  Empty cost_funcs : must at least specify one cost function!");
  if (window_length < 1)
    PLERROR("In MovingAverage::build_()  window_length has not been set!");
  if (output.length() != target.length())
    PLERROR("The output and target vectors don't have the same length.");

  input.resize(0);
  target.resize(targetsize());
  output.resize(targetsize());
  cost.resize(targetsize());
  all_targets.resize(window_length, targetsize());
}

void MovingAverage::build()
{
  inherited::build();
  build_();
}

void MovingAverage::declareOptions(OptionList& ol)
{
  declareOption(ol, "window_length", &MovingAverage::window_length,
    OptionBase::buildoption, "the length of the moving average window \n");

  declareOption(ol, "cost_funcs", &MovingAverage::cost_funcs,
    OptionBase::buildoption, "a list of cost functions to use \n");

  inherited::declareOptions(ol);
}

void MovingAverage::train(VecStatsCollector& train_stats)
{
  ProgressBar* pb;

  int target_pos = inputsize();
  int start = MAX(window_length-1, last_train_t);
  if (report_progress)
    pb = new ProgressBar("Training MovingAverage learner", train_set.length()-start);
  for (int t=start; t<train_set.length(); t++)
  {
    all_targets = train_set.subMat(t-window_length, target_pos, window_length+1, targetsize());
    columnMean(all_targets,output);
    predictions(t) << output;
    if (t >= horizon)
    {
      output = predictions(t-horizon);
      train_set->getSubRow(t, target_pos, target);
      if (!target.hasMissing() && !output.hasMissing())
      {
        computeCostsFromOutputs(input, output, target, cost);
        errors(t) << cost;
        train_stats.update(cost);
      }
    }
    if (pb) pb->update(t-start);
  }
  last_train_t = train_set.length();

  train_stats.finalize();

  if (pb) delete pb;
}
 
void MovingAverage::test(VMat testset, VecStatsCollector& test_stats,
    VMat testoutputs, VMat testcosts)
{
  ProgressBar* pb;

  int start = MAX(window_length-1, last_test_t);
  start = MAX(last_train_t-1,start);
  int target_pos = inputsize();
  if (report_progress)
    pb = new ProgressBar("Testing MovingAverage learner", testset.length()-start);
  for (int t=start; t<testset.length(); t++)
  {
    all_targets = testset.subMat(t-window_length, target_pos, window_length+1, targetsize());
    columnMean(all_targets,output);
    predictions(t) << output;
    if (testoutputs) testoutputs->putOrAppendRow(t, output);
    if (t >= horizon)
    {
      output = predictions(t-horizon);
      testset->getSubRow(t, target_pos, target);
      if (!target.hasMissing() && !output.hasMissing())
      {
        computeCostsFromOutputs(input, output, target, cost);
        errors(t) << cost;
        if (testcosts) testcosts->putOrAppendRow(t, cost);
        test_stats.update(cost);
      }
    }
    if (pb) pb->update(t-start);
  }
  last_test_t = testset.length();

  test_stats.finalize();

  if (pb) delete pb;
}

void MovingAverage::computeCostsFromOutputs(const Vec& inputs, const Vec& outputs,
    const Vec& targets, Vec& costs)
{
  if (cost_funcs.size() != 1)
    PLERROR("There is only 1 cost_funcs defined yet.");

  for (int i=0; i<cost_funcs.size(); i++)
  {
    if (cost_funcs[i]=="mse" || cost_funcs[i]=="MSE")
      costs << square(outputs-targets);
    else
      PLERROR("This cost_funcs is not implemented.");
  }
}

TVec<string> MovingAverage::getTrainCostNames() const
{ return cost_funcs; }

TVec<string> MovingAverage::getTestCostNames() const
{ return getTrainCostNames(); }

void MovingAverage::forget()
{ inherited::forget(); }

/*
void MovingAverage::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(cost_funcs, copies);
}
*/


%> // end of namespace PLearn

