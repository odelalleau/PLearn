// -*- C++ -*-

// VariableSelectionWithDirectedGradientDescent.cc
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio and University of Montreal
// Copyright (c) 2002 Jean-Sebastien Senecal, Xavier Saint-Mleux, Rejean Ducharme
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


/* **************************************************************************************************************    
   * $Id: VariableSelectionWithDirectedGradientDescent.cc, v 1.0 2005/01/15 10:00:00 Bengio/Kegl/Godbout        *
   * This file is part of the PLearn library.                                                                   *
   ************************************************************************************************************** */

#include "VariableSelectionWithDirectedGradientDescent.h"
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(VariableSelectionWithDirectedGradientDescent,
                       "Variable selection algorithm", 
		       "Variable selection algorithm using a linear density estimator and\n"
		       "directed gradient descent to identify most relevant variables.\n"
		       "\n"
		       "there are 4 options to set:\n"
		       "   learning_rate, the gradient step to be used by the descent algorithm,\n"
		       "   nstages, the number of epoch to be performed by the algorithm,\n"
		       "   verbosity, the level of information you want to get while in progress,\n"
		       "   report_progress, whether a progress bar should inform you of the progress.\n"
		       "\n"
		       "If both verbosity > 1 and report_progress is not zero, it works but it is ugly.\n"
		       "\n"
		       "The selected variables are returned in the selected_variables vector in\n"
		       "the order of their selection. The vector is a learnt option of the algorithm.\n");

VariableSelectionWithDirectedGradientDescent::VariableSelectionWithDirectedGradientDescent()
{
}

VariableSelectionWithDirectedGradientDescent::~VariableSelectionWithDirectedGradientDescent()
{
}

void VariableSelectionWithDirectedGradientDescent::declareOptions(OptionList& ol)
{ 
  declareOption(ol, "learning_rate", &VariableSelectionWithDirectedGradientDescent::learning_rate, OptionBase::buildoption,
      "The learning rate of the gradient descent algorithm\n");
  declareOption(ol, "relevancy_ind", &VariableSelectionWithDirectedGradientDescent::relevancy_ind, OptionBase::learntoption,
      "The number of variables selected\n");
  declareOption(ol, "input_weights", &VariableSelectionWithDirectedGradientDescent::input_weights, OptionBase::learntoption,
      "The lerant weights of the linear probability estimator\n");
  declareOption(ol, "weights_selected", &VariableSelectionWithDirectedGradientDescent::weights_selected, OptionBase::learntoption,
      "The vector of identifying the non-zero weights\n");
  declareOption(ol, "selected_variables", &VariableSelectionWithDirectedGradientDescent::selected_variables, OptionBase::learntoption,
      "The vector with the selected variables in the order of their selection\n");
  inherited::declareOptions(ol);
}

void VariableSelectionWithDirectedGradientDescent::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(learning_rate, copies);
  deepCopyField(relevancy_ind, copies);
  deepCopyField(input_weights, copies);
  deepCopyField(weights_selected, copies);
  deepCopyField(selected_variables, copies);
}

void VariableSelectionWithDirectedGradientDescent::build()
{
  inherited::build();
  build_();
}

void VariableSelectionWithDirectedGradientDescent::build_()
{
  if (train_set)
  {
    length = train_set->length();
    width = train_set->width();
    if (length < 1)
      PLERROR("VariableSelectionWithDirectedGradientDescent: the training set must contain at least one sample, got %d", length);
    inputsize = train_set->inputsize();
    targetsize = train_set->targetsize();
    weightsize = train_set->weightsize();
    if (inputsize < 1)
      PLERROR("VariableSelectionWithDirectedGradientDescent: expected  inputsize greater than 0, got %d", inputsize);
    if (targetsize != 1)
      PLERROR("VariableSelectionWithDirectedGradientDescent: expected targetsize to be 1, got %d", targetsize);
    if (weightsize != 0)
      PLERROR("VariableSelectionWithDirectedGradientDescent: expected weightsize to be 1, got %d", weightsize_);
    input_weights.resize(inputsize + 1);
    weights_selected.resize(inputsize + 1);
    weights_gradient.resize(inputsize + 1);
    selected_variables.resize(inputsize + 1);
    sample_input.resize(inputsize);
    sample_target.resize(1);
    sample_output.resize(1);
    sample_cost.resize(1);
    for (col = 0; col <= inputsize; col++)
    {
      input_weights[col] = 0.0;
      weights_selected[col] = 0.0;
      selected_variables[col] = 0.0;
    }
    relevancy_ind = 0;
    stage = 0;
  }
}

void VariableSelectionWithDirectedGradientDescent::train()
{
  if (!train_set)
    PLERROR("VariableSelectionWithDirectedGradientDescent: the algorithm has not been properly built");
  pb = NULL;
  if (report_progress)
  {
    pb = new ProgressBar("VariableSelectionWithDirectedGradientDescent : train stages: ", nstages);
  }
/*
  We loop through the data for the specified maximum number of stages.
*/
  for (; stage < nstages; stage++)
  {
    for (col = 0; col <= inputsize; col ++)
    {
      weights_gradient[col] = 0.0;
    }
/*
  We compute the train criterion for this stage and compute the weigth gradient.
*/
    train_criterion = 0.0;
    for (row = 0; row < length; row++)
    {
      n7_value = input_weights[inputsize];
      for (col = 0; col < inputsize; col++)
      {
        n7_value += input_weights[col] * train_set(row, col);
      }
      n8_value = train_set(row, inputsize) * n7_value;
      n9_value = 1.0 / (1.0 + exp(-n8_value));
      n10_value = -log(n9_value);
      train_criterion += n10_value;
      n10_gradient = 1.0;
      n9_gradient = n10_gradient * (-1.0 / n9_value);
      n8_gradient = n9_gradient * n9_value * 1.0 / (1.0 + exp(n8_value));
      n7_gradient = n8_gradient * train_set(row, inputsize);
      for (col = 0; col < inputsize; col++)
      {
        weights_gradient[col] += n7_gradient * train_set(row, col);
      }
      weights_gradient[inputsize] += n7_gradient;     
    }
/*
  We perform this stage weight update according to the directed gradient descent algorithm.
*/
    input_weights[inputsize] -= learning_rate * weights_gradient[inputsize];
    weights_gradient_max = 0.0;
    for (col = 0; col < inputsize; col++)
    {
      if (fabs(weights_gradient[col]) > weights_gradient_max)
      {
        weights_gradient_max = fabs(weights_gradient[col]);
        weights_gradient_max_col = col;
      }
    }
    if (weights_selected[weights_gradient_max_col] == 0.0)
    {
      selected_variables[relevancy_ind] = weights_gradient_max_col;
      relevancy_ind += 1;
      verbose("VariableSelectionWithDirectedGradientDescent: variable " + tostring(weights_gradient_max_col)
              + " was added.", 2);
    }
    weights_selected[weights_gradient_max_col] = 1.0;
    for (col = 1; col < inputsize; col++)
    {
      input_weights[col] -= learning_rate * weights_gradient[col] * weights_selected[col];
    }
    verbose("VariableSelectionWithDirectedGradientDescent: After " + tostring(stage) + " stages, the train criterion is: "
            + tostring(train_criterion), 3);
    if (report_progress) pb->update(stage);
  }
  if (report_progress)
  {
    delete pb;
    pb = new ProgressBar("VariableSelectionWithDirectedGradientDescent : computing the training statistics: ", length);
  }
  train_stats->forget();
  for (row = 0; row < length; row++)
  {   
    train_set->getExample(row, sample_input, sample_target, sample_weight);
    computeOutput(sample_input, sample_output);
    computeCostsFromOutputs(sample_input, sample_output, sample_target, sample_cost);
    train_stats->update(sample_cost);
    if (report_progress) pb->update(row);
  }
  train_stats->finalize();
  if (report_progress) delete pb;
  verbose("VariableSelectionWithDirectedGradientDescent: After " + tostring(stage) + " stages, average error is: "
          + tostring(train_stats->getMean()), 1);
}

void VariableSelectionWithDirectedGradientDescent::verbose(string the_msg, int the_level)
{
  if (verbosity >= the_level)
    cout << the_msg << endl;
}

void VariableSelectionWithDirectedGradientDescent::forget()
{
  for (col = 0; col <= inputsize; col++)
  {
    input_weights[col] = 0.0;
    weights_selected[col] = 0.0;
    selected_variables[col] = 0.0;
  }
  relevancy_ind = 0;
  stage = 0;
}

int VariableSelectionWithDirectedGradientDescent::outputsize() const
{
  return 1;
}

TVec<string> VariableSelectionWithDirectedGradientDescent::getTrainCostNames() const
{
  TVec<string> return_msg(1);
  return_msg[0] = "negloglikelihood";
  return return_msg;
}

TVec<string> VariableSelectionWithDirectedGradientDescent::getTestCostNames() const
{ 
  return getTrainCostNames();
}

void VariableSelectionWithDirectedGradientDescent::computeOutput(const Vec& inputv, Vec& outputv) const
{
  outputv[0] = input_weights[inputsize];
  for (int col = 0; col < inputsize; col++)
  {
    outputv[0] += input_weights[col] * inputv[col];
  }
}

void VariableSelectionWithDirectedGradientDescent::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                 Vec& outputv, Vec& costsv) const
{
  computeOutput(inputv, outputv);
  computeCostsFromOutputs(inputv, outputv, targetv, costsv);
}

void VariableSelectionWithDirectedGradientDescent::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, 
                                   const Vec& targetv, Vec& costsv) const
{
      costsv[0] = -log(1.0 / (1.0 + exp(-targetv[0] * outputv[0])));;
}

} // end of namespace PLearn
