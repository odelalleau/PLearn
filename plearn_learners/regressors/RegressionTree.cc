// -*- C++ -*-

// RegressionTree.cc
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


/* ********************************************************************************    
   * $Id: RegressionTree.cc, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout        *
   * This file is part of the PLearn library.                                     *
   ******************************************************************************** */

#include "RegressionTree.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(RegressionTree,
                        "Regression tree algorithm", 
                        "Algorithm built to serve as a base regressor for the LocalMedianBoost algorithm.\n"
                        "It can also be used as a stand alone learner.\n"
                        "It can learn from a weighted train set to represent different distribution on the training set.\n"
                        "It can separate a confidence fonction from the output whenmaking a prediction.\n"
                        "At each node expansion, it splits the node to maximize the improvement of an objective function\n"
                        "with the mean square error and a facto of the confidence funtion.\n"
                        "At each node expansion, it creates 3 nodes, one to hold any samples with a missing value on the\n"
                        "splitting attribute, one for the samples with values less than the value of the splitting attribute\n"
                        "and one fr the others.\n"
                        );

RegressionTree::RegressionTree()     
  : missing_is_valid(0),
    loss_function_weight(1.0),
    maximum_number_of_nodes(400)
{
}

RegressionTree::~RegressionTree()
{
}

void RegressionTree::declareOptions(OptionList& ol)
{ 
  declareOption(ol, "missing_is_valid", &RegressionTree::missing_is_valid, OptionBase::buildoption,
      "If set to 1, missing values will be treated as valid, and missing nodes will be potential for splits.\n");
  declareOption(ol, "loss_function_weight", &RegressionTree::loss_function_weight, OptionBase::buildoption,
      "The hyper parameter to balance the error and the confidence factor.\n");
  declareOption(ol, "maximum_number_of_nodes", &RegressionTree::maximum_number_of_nodes, OptionBase::buildoption,
      "The maximum number of nodes for this tree.\n"
      "(If less than nstages, nstages will be used).");
  declareOption(ol, "sorted_train_set", &RegressionTree::sorted_train_set, OptionBase::buildoption, 
      "The train set sorted on all columns\n"
      "If it is not provided by a wrapping algorithm, it is created at stage 0.\n");
      
  declareOption(ol, "root", &RegressionTree::root, OptionBase::learntoption,
      "The root node of the tree being built\n");
  declareOption(ol, "priority_queue", &RegressionTree::priority_queue, OptionBase::learntoption,
      "The heap to store potential nodes to expand\n");
  declareOption(ol, "first_leave", &RegressionTree::first_leave, OptionBase::learntoption,
      "The first leave built with the root containing all train set rows at the beginning\n");
  declareOption(ol, "first_leave_output", &RegressionTree::first_leave_output, OptionBase::learntoption,
      "The vector to compute the ouput and the confidence function of the first leave.\n");
  declareOption(ol, "first_leave_error", &RegressionTree::first_leave_error, OptionBase::learntoption,
      "The vector to compute the errors of the first leave.n");
  inherited::declareOptions(ol);
}

void RegressionTree::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(missing_is_valid, copies);
  deepCopyField(loss_function_weight, copies);
  deepCopyField(maximum_number_of_nodes, copies);
  deepCopyField(sorted_train_set, copies);
  deepCopyField(root, copies);
  deepCopyField(priority_queue, copies);
  deepCopyField(first_leave, copies);
  deepCopyField(first_leave_output, copies);
  deepCopyField(first_leave_error, copies);
}

void RegressionTree::build()
{
  inherited::build();
  build_();
}

void RegressionTree::build_()
{
  if (train_set)
  { 
    length = train_set->length();
    if (length < 1) PLERROR("RegressionTree: the training set must contain at least one sample, got %d", length);
    inputsize = train_set->inputsize();
    targetsize = train_set->targetsize();
    weightsize = train_set->weightsize();
    if (inputsize < 1) PLERROR("RegressionTree: expected  inputsize greater than 0, got %d", inputsize);
    if (targetsize != 1) PLERROR("RegressionTree: expected targetsize to be 1, got %d", targetsize);
    if (weightsize != 1 && weightsize != 0)  PLERROR("RegressionTree: expected weightsize to be 1 or 0, got %d", weightsize_);
    if (loss_function_weight != 0.0) computed_loss_function_weight = 1.0 / pow(loss_function_weight, 2.0);
    else computed_loss_function_weight = 0.0;
    sample_input.resize(inputsize);
    sample_target.resize(targetsize);
    sample_output.resize(2);
    sample_costs.resize(3);
  }
}

void RegressionTree::train()
{
  if (stage == 0) initialiseTree();
  ProgressBar* pb = NULL;
  if (report_progress)
  {
    pb = new ProgressBar("RegressionTree : train stages: ", nstages);
  }
  for (; stage < nstages; stage++)
  {    
    if (stage > 0) expandTree();
    if (report_progress) pb->update(stage);
  }
  if (report_progress) delete pb;
  if (report_progress)
  {
    pb = new ProgressBar("RegressionTree : computing the statistics: ", length);
  } 
  train_stats->forget();
  for (each_train_sample_index = 0; each_train_sample_index < length; each_train_sample_index++)
  {  
    train_set->getExample(each_train_sample_index, sample_input, sample_target, sample_weight);
    computeOutput(sample_input, sample_output);
    computeCostsFromOutputs(sample_input, sample_output, sample_target, sample_costs); 
    train_stats->update(sample_costs);
    if (report_progress) pb->update(each_train_sample_index);
  }
  train_stats->finalize();
  if (report_progress) delete pb; 
}

void RegressionTree::verbose(string the_msg, int the_level)
{
  if (verbosity >= the_level)
    cout << the_msg << endl;
}

void RegressionTree::forget()
{
  stage = 0;
}

void RegressionTree::initialiseTree()
{
  if (!sorted_train_set)
  {
    sorted_train_set = new RegressionTreeRegisters();
    sorted_train_set->setOption("report_progress", tostring(report_progress));
    sorted_train_set->setOption("verbosity", tostring(verbosity));
    sorted_train_set->initRegisters(train_set);
  }
  else
  {
    sorted_train_set->reinitRegisters();
  }
  first_leave_output.resize(2);
  first_leave_error.resize(2);
  first_leave = new RegressionTreeLeave();
  first_leave->setOption("id", tostring(sorted_train_set->getNextId()));
  first_leave->setOption("missing_leave", "0");
  first_leave->setOption("loss_function_weight", tostring(computed_loss_function_weight));
  first_leave->setOption("verbosity", tostring(verbosity));
  first_leave->initLeave(sorted_train_set);
  first_leave->initStats();
  for (each_train_sample_index = 0; each_train_sample_index < length; each_train_sample_index++)
  {
    first_leave->addRow(each_train_sample_index, first_leave_output, first_leave_error);
    first_leave->registerRow(each_train_sample_index);
  }
  root = new RegressionTreeNode();
  root->setOption("missing_is_valid", tostring(missing_is_valid));
  root->setOption("loss_function_weight", tostring(computed_loss_function_weight));
  root->setOption("verbosity", tostring(verbosity));
  root->initNode(sorted_train_set, first_leave);
  root->lookForBestSplit();
  if (maximum_number_of_nodes < nstages) maximum_number_of_nodes = nstages;
  priority_queue = new RegressionTreeQueue();
  priority_queue->setOption("verbosity", tostring(verbosity));
  priority_queue->setOption("maximum_number_of_nodes", tostring(maximum_number_of_nodes));
  priority_queue->initHeap();
  priority_queue->addHeap(root);
}

void RegressionTree::expandTree()
{
  if (priority_queue->isEmpty() <= 0)
  {
    verbose("RegressionTree: priority queue empty", 3);
    return;
  }
  node = priority_queue->popHeap();
  if (node->expandNode() < 0)
  {
    verbose("RegressionTree: expand is negative?", 3);
    return;
  }
  priority_queue->addHeap(node->getNodes()[0]); 
  priority_queue->addHeap(node->getNodes()[1]);
  if (missing_is_valid > 0) priority_queue->addHeap(node->getNodes()[2]); 
}

void RegressionTree::setSortedTrainSet(PP<RegressionTreeRegisters> the_sorted_train_set)
{
  sorted_train_set = the_sorted_train_set;
}

int RegressionTree::outputsize() const
{
  return 2;
}

TVec<string> RegressionTree::getTrainCostNames() const
{
  TVec<string> return_msg(3);
  return_msg[0] = "mse";
  return_msg[1] = "base confidence";
  return_msg[2] = "base reward";
  return return_msg;
}

TVec<string> RegressionTree::getTestCostNames() const
{ 
  return getTrainCostNames();
}

void RegressionTree::computeOutput(const Vec& inputv, Vec& outputv) const
{
  root->computeOutput(inputv, outputv);
}

void RegressionTree::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, Vec& outputv, Vec& costsv) const
{
  computeOutput(inputv, outputv);
  computeCostsFromOutputs(inputv, outputv, targetv, costsv);
}

void RegressionTree::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, const Vec& targetv, Vec& costsv) const
{
  costsv[0] = pow((outputv[0] - targetv[0]), 2.0);
  costsv[1] = outputv[1];
  costsv[2] = 1.0 - (2.0 * computed_loss_function_weight * costsv[0]);
}

} // end of namespace PLearn
