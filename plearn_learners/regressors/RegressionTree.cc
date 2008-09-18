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
#include "RegressionTreeQueue.h"
#include "RegressionTreeLeave.h"
#include "RegressionTreeMulticlassLeave.h"
#include "RegressionTreeRegisters.h"
#include "RegressionTreeNode.h"

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
    : missing_is_valid(false),
      loss_function_weight(1.0),
      maximum_number_of_nodes(400),
      compute_train_stats(1),
      complexity_penalty_factor(0.0),
      output_confidence_target(false)

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
    declareOption(ol, "compute_train_stats", &RegressionTree::compute_train_stats, OptionBase::buildoption,
                  "If set to 1 (the default value) the train statistics are computed.\n"
                  "(When using the tree as a base regressor, we dont need the stats and it goes quicker when computations are suppressed).");
    declareOption(ol, "complexity_penalty_factor", &RegressionTree::complexity_penalty_factor, OptionBase::buildoption,
                  "A factor that is multiplied with the square root of the number of leaves.\n"
                  "If the error inprovement for the next split is less than the result, the algorithm proceed to an early stop."
                  "(When set to 0.0, the default value, it has no impact).");

    declareOption(ol, "output_confidence_target",
                  &RegressionTree::output_confidence_target,
                  OptionBase::buildoption,
                  "If false the output size is 1 and contain only the predicted"
                  " target. Else output size is 2 and contain also the"
                  " confidence\n");


    declareOption(ol, "multiclass_outputs", &RegressionTree::multiclass_outputs, OptionBase::buildoption,
                  "A vector of possible output values when solving a multiclass problem.\n"
                  "When making a prediction, the tree will adjust the output value of each leave to the closest value provided in this vector.");
    declareOption(ol, "leave_template", &RegressionTree::leave_template, OptionBase::buildoption,
                  "The template for the leave objects to create.\n");
    declareOption(ol, "sorted_train_set", &RegressionTree::sorted_train_set,
                  OptionBase::buildoption | OptionBase::nosave, 
                  "The train set sorted on all columns. If it is not provided by a\n"
                  " wrapping algorithm, it is created at stage 0.\n");
      
    declareOption(ol, "root", &RegressionTree::root, OptionBase::learntoption,
                  "The root node of the tree being built\n");
    declareOption(ol, "priority_queue", &RegressionTree::priority_queue, OptionBase::learntoption,
                  "The heap to store potential nodes to expand\n");
    declareOption(ol, "first_leave", &RegressionTree::first_leave, OptionBase::learntoption,
                  "The first leave built with the root containing all train set rows at the beginning\n");
    declareOption(ol, "split_cols", &RegressionTree::split_cols,
                  OptionBase::learntoption,
                  "Contain in order of addition of node the columns used to"
                  " split the tree.\n");
    declareOption(ol, "split_values", &RegressionTree::split_values,
                  OptionBase::learntoption,
                  "Contain in order of addition of node the split value.\n");

    declareOption(ol, "first_leave_output", &RegressionTree::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED\n");
    declareOption(ol, "first_leave_error", &RegressionTree::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED\n");


    inherited::declareOptions(ol);
}

void RegressionTree::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(missing_is_valid, copies);
    deepCopyField(loss_function_weight, copies);
    deepCopyField(maximum_number_of_nodes, copies);
    deepCopyField(compute_train_stats, copies);
    deepCopyField(complexity_penalty_factor, copies);
    deepCopyField(multiclass_outputs, copies);
    deepCopyField(leave_template, copies);
    deepCopyField(sorted_train_set, copies);
    deepCopyField(root, copies);
    deepCopyField(priority_queue, copies);
    deepCopyField(first_leave, copies);
    deepCopyField(split_cols, copies);
    deepCopyField(split_values, copies);
    deepCopyField(tmp_vec, copies);
    
}

void RegressionTree::build()
{
    inherited::build();
    build_();
}

void RegressionTree::build_()
{
    PP<VMatrix> the_train_set;
    if(sorted_train_set)
    {
        the_train_set = sorted_train_set;
    }
    else if (train_set)
    { 
        the_train_set = train_set;
    }
    if(the_train_set)
    {
        length = the_train_set->length();
        int inputsize = the_train_set->inputsize();
        int targetsize = the_train_set->targetsize();
        int weightsize = the_train_set->weightsize();

        if (length < 1)
            PLERROR("RegressionTree: the training set must contain at least one"
                    " sample, got %d", length);
        if (inputsize < 1)
            PLERROR("RegressionTree: expected  inputsize greater than 0, got %d",
                    inputsize);
        if (targetsize != 1)
            PLERROR("RegressionTree: expected targetsize to be 1,"" got %d",
                    targetsize);
        if (weightsize != 1 && weightsize != 0)
            PLERROR("RegressionTree: expected weightsize to be 1 or 0, got %d",
                    weightsize);
    }

    tmp_vec.resize(2);
    nodes = new TVec<PP<RegressionTreeNode> >();
    tmp_computeCostsFromOutput.resize(outputsize());
    
    if (loss_function_weight != 0.0)
    {
        l2_loss_function_factor = 2.0 / pow(loss_function_weight, 2);
        l1_loss_function_factor = 2.0 / loss_function_weight;
    }
    else
    {
        l2_loss_function_factor = 1.0;
        l1_loss_function_factor = 1.0;
    }
}

void RegressionTree::train()
{
    Profiler::pl_profile_start("RegressionTree::train");

    if (stage == 0) initialiseTree();
    PP<ProgressBar> pb;
    if (report_progress)
    {
        pb = new ProgressBar("RegressionTree : train stages: ", nstages);
    }
    for (; stage < nstages; stage++)
    {    
        if (stage > 0)
        {
            PP<RegressionTreeNode> node= expandTree();
            if (node == NULL) break;
            split_cols.append(node->getSplitCol());
            split_values.append(node->getSplitValue());
        }
        if (report_progress) pb->update(stage);
    }
    pb = NULL;
    verbose("split_cols: "+tostring(split_cols),2);
    verbose("split_values: "+tostring(split_values),2);
    if (compute_train_stats < 1){
        Profiler::pl_profile_end("RegressionTree::train");
        return;
    }
    if (report_progress)
    {
        pb = new ProgressBar("RegressionTree : computing the statistics: ", length);
    } 
    train_stats->forget();

    real sample_weight;
    Vec sample_input(sorted_train_set->inputsize());
    Vec sample_output(outputsize());
    Vec sample_target(sorted_train_set->targetsize());
    Vec sample_costs(nTestCosts());

    for (int train_sample_index = 0; train_sample_index < length;
         train_sample_index++)
    {  
        sorted_train_set->getExample(train_sample_index, sample_input, sample_target, sample_weight);
        computeOutputAndCosts(sample_input,sample_target,sample_output,sample_costs);
        train_stats->update(sample_costs);
        if (report_progress) pb->update(train_sample_index);
    }
    train_stats->finalize();

    Profiler::pl_profile_end("RegressionTree::train");
}

void RegressionTree::verbose(string the_msg, int the_level)
{
    if (verbosity >= the_level)
        pout << the_msg << endl;
}

void RegressionTree::forget()
{
    stage = 0;
}

void RegressionTree::initialiseTree()
{
    if (!sorted_train_set && train_set->classname()=="RegressionTreeRegisters")
    {
        sorted_train_set=(PP<RegressionTreeRegisters>)train_set;
        sorted_train_set->reinitRegisters();
    }
    else if(!sorted_train_set)
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
    //Set value common value of all leave
    // for optimisation, by default they aren't missing leave
    leave_template->missing_leave = 0;
    leave_template->loss_function_weight = loss_function_weight;
    leave_template->verbosity = verbosity;
    leave_template->initStats();

    first_leave = ::PLearn::deepCopy(leave_template);
    first_leave->id=sorted_train_set->getNextId();
    first_leave->initLeave(sorted_train_set);

    for (int train_sample_index = 0; train_sample_index < length;
         train_sample_index++)
    {
        first_leave->addRow(train_sample_index);
        first_leave->registerRow(train_sample_index);
    }
    root = new RegressionTreeNode(missing_is_valid,loss_function_weight,
                                  verbosity);
    root->initNode(sorted_train_set, first_leave, leave_template);
    root->lookForBestSplit();

    if (maximum_number_of_nodes < nstages) maximum_number_of_nodes = nstages;
    priority_queue = new RegressionTreeQueue(verbosity,maximum_number_of_nodes);
    priority_queue->addHeap(root);
}

PP<RegressionTreeNode> RegressionTree::expandTree()
{
    if (priority_queue->isEmpty() <= 0)
    {
        verbose("RegressionTree: priority queue empty, stage: " + tostring(stage), 3);
        return NULL;
    }
    PP<RegressionTreeNode> node = priority_queue->popHeap();
    if (node->getErrorImprovment() < complexity_penalty_factor * sqrt((real)stage))
    {
        verbose("RegressionTree: early stopping at stage: " + tostring(stage)
                + ", error improvement: " + tostring(node->getErrorImprovment())
                + ", penalty: " + tostring(complexity_penalty_factor * sqrt((real)stage)), 3);
        return NULL;
    }
    int split_col = node->expandNode();
    if (split_col < 0)
    {
        verbose("RegressionTree: expand is negative?", 3);
        return NULL;
    }
    TVec< PP<RegressionTreeNode> > subnode = node->getNodes();
    priority_queue->addHeap(subnode[0]); 
    priority_queue->addHeap(subnode[1]);
    if (missing_is_valid) priority_queue->addHeap(subnode[2]);
    return node; 
}

int RegressionTree::outputsize() const
{
    if(output_confidence_target)
        return 2;
    else
        return 1;
}

TVec<string> RegressionTree::getTrainCostNames() const
{
    TVec<string> return_msg(5);
    return_msg[0] = "mse";
    return_msg[1] = "base_confidence";
    return_msg[2] = "base_reward_l2";
    return_msg[3] = "base_reward_l1";
    return_msg[4] = "class_error";
    return return_msg;
}

TVec<string> RegressionTree::getTestCostNames() const
{ 
    TVec<string> costs=getTrainCostNames();
    PP<VMatrix> the_train_set=train_set;
    if(sorted_train_set)
        the_train_set = sorted_train_set;

    PLCHECK_MSG(the_train_set,"In RegressionTree::getTestCostNames() - "
                "a train set is needed!");
    for(int i=0;i<the_train_set->width();i++)
    {
        costs.append("SPLIT_VAR_"+the_train_set->fieldName(i));
    }
    return costs;
}

void RegressionTree::computeOutput(const Vec& inputv, Vec& outputv) const
{
    if(!output_confidence_target){
        computeOutputAndNodes(inputv, tmp_vec);
        outputv[0]=tmp_vec[0];
    }
    else
        computeOutputAndNodes(inputv, outputv);
        
}

void RegressionTree::computeOutputAndNodes(const Vec& inputv, Vec& outputv,
                                           TVec<PP<RegressionTreeNode> >* nodes) const
{
    if(!output_confidence_target){
        root->computeOutputAndNodes(inputv, tmp_vec, nodes);
        outputv[0]=tmp_vec[0];
    }
    else
        root->computeOutputAndNodes(inputv, outputv, nodes);

    if (multiclass_outputs.length() <= 0) return;
    real closest_value=multiclass_outputs[0];
    real margin_to_closest_value=abs(outputv[0] - multiclass_outputs[0]);
    for (int value_ind = 1; value_ind < multiclass_outputs.length(); value_ind++)
    {
        real v=abs(outputv[0] - multiclass_outputs[value_ind]);
        if (v < margin_to_closest_value)
        {
            closest_value = multiclass_outputs[value_ind];
            margin_to_closest_value = v;
        }
    }
    outputv[0] = closest_value;
}

void RegressionTree::computeOutputAndCosts(const Vec& input,
                                           const Vec& target,
                                           Vec& output, Vec& costs) const
{
    PLASSERT(costs.size()==nTestCosts());
    PLASSERT(nodes);
    nodes->resize(0);

    computeOutputAndNodes(input, tmp_vec, nodes);
    if(!output_confidence_target)
        output[0]=tmp_vec[0];
    else
        output<<tmp_vec;

    computeCostsFromOutputsAndNodes(input, tmp_vec, target, *nodes, costs);
}

void RegressionTree::computeCostsFromOutputsAndNodes(const Vec& input,
                                                     const Vec& output, 
                                                     const Vec& target,
                                                     const TVec<PP<RegressionTreeNode> >& nodes,
                                                     Vec& costs) const
{
    costs.clear();
    costs[0] = pow((output[0] - target[0]), 2);
    if(output.length()>1) costs[1] = output[1];
    else costs[1] = MISSING_VALUE;
    costs[2] = 1.0 - (l2_loss_function_factor * costs[0]);
    costs[3] = 1.0 - (l1_loss_function_factor * abs(output[0] - target[0]));
    costs[4] = !fast_is_equal(target[0],output[0]);

    for(int i=0;i<nodes.length();i++)
        costs[5+nodes[i]->getSplitCol()]++;
}

void RegressionTree::computeCostsFromOutputs(const Vec& input,
                                             const Vec& output, 
                                             const Vec& target,
                                             Vec& costs) const
{
    computeOutputAndCosts(input, target, tmp_computeCostsFromOutput, costs); 
    PLASSERT(output==tmp_computeCostsFromOutput);
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
