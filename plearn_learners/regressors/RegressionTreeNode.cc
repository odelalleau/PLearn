// -*- C++ -*-

// RegressionTreeNode.cc
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
 * $Id: RegressionTreeNode.cc, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout    *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#include "RegressionTreeNode.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(RegressionTreeNode,
                        "Object to represent the nodes of a regression tree.",
                        "It may be a final node pointing to a leave.\n"
                        "If that is the case, it knows always what would be the best possible split for that leave.\n"
                        "It may be an expanded node pointing to 3 children nodes: a leave for missing values on the splitting attribute,\n"
                        "a left leave for samples with values below the value of the splitting attribute, and a right leave for the others,\n"
    );

RegressionTreeNode::RegressionTreeNode():
    missing_is_valid(0),
    loss_function_weight(1),
    verbosity(0),
    split_col(-1),
    split_balance(INT_MAX),
    split_feature_value(REAL_MAX),
    after_split_error(REAL_MAX),
    dummy_int(0)
{
    build();
}

RegressionTreeNode::~RegressionTreeNode()
{
}

void RegressionTreeNode::declareOptions(OptionList& ol)
{ 
    declareOption(ol, "missing_is_valid", &RegressionTreeNode::missing_is_valid, OptionBase::buildoption,
                  "If set to 1, missing values will be treated as valid, and missing nodes will be potential for splits.\n");
    declareOption(ol, "loss_function_weight", &RegressionTreeNode::loss_function_weight, OptionBase::buildoption,
                  "The hyper parameter to balance the error and the confidence factor\n");
    declareOption(ol, "verbosity", &RegressionTreeNode::verbosity, OptionBase::buildoption,
                  "The desired level of verbosity\n");
    declareOption(ol, "leave_template", &RegressionTreeNode::leave, OptionBase::buildoption,
                  "The template for the leave objects to create.\n");
    declareOption(ol, "train_set", &RegressionTreeNode::train_set, OptionBase::buildoption,
                  "The matrix with the sorted train set\n");
    declareOption(ol, "leave", &RegressionTreeNode::leave, OptionBase::buildoption,
                  "The leave of all the  belonging rows when this node is a leave\n");
      
    declareOption(ol, "leave_output", &RegressionTreeNode::leave_output, OptionBase::learntoption,
                  "The leave output vector\n");
    declareOption(ol, "leave_error", &RegressionTreeNode::leave_error, OptionBase::learntoption,
                  "The leave error vector\n");
    declareOption(ol, "split_col", &RegressionTreeNode::split_col, OptionBase::learntoption,
                  "The dimension of the best split of leave\n");
    declareOption(ol, "split_balance", &RegressionTreeNode::split_balance, OptionBase::learntoption,
                  "The balance between the left and the right leave\n");
    declareOption(ol, "split_feature_value", &RegressionTreeNode::split_feature_value, OptionBase::learntoption,
                  "The feature value of the split\n");
    declareOption(ol, "after_split_error", &RegressionTreeNode::after_split_error, OptionBase::learntoption,
                  "The error after split\n");
    declareOption(ol, "missing_node", &RegressionTreeNode::missing_node, OptionBase::learntoption,
                  "The node for the missing values when missing_is_valid is set to 1\n");
    declareOption(ol, "missing_leave", &RegressionTreeNode::missing_leave, OptionBase::learntoption,
                  "The leave containing rows with missing values after split\n");
    declareOption(ol, "left_node", &RegressionTreeNode::left_node, OptionBase::learntoption,
                  "The node on the left of the split decision\n");
    declareOption(ol, "left_leave", &RegressionTreeNode::left_leave, OptionBase::learntoption,
                  "The leave with the rows lower than the split feature value after split\n");
    declareOption(ol, "right_node", &RegressionTreeNode::right_node, OptionBase::learntoption,
                  "The node on the right of the split decision\n"); 
    declareOption(ol, "right_leave", &RegressionTreeNode::right_leave, OptionBase::learntoption,
                  "The leave with the rows greater thean the split feature value after split\n");

    declareOption(ol, "left_error", &RegressionTreeNode::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The left leave error vector\n");
    declareOption(ol, "right_error", &RegressionTreeNode::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The right leave error vector\n");
    declareOption(ol, "missing_error", &RegressionTreeNode::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The missing leave error vector\n");
    declareOption(ol, "left_output", &RegressionTreeNode::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The left leave output vector\n");
    declareOption(ol, "right_output", &RegressionTreeNode::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The right leave output vector\n");
    declareOption(ol, "missing_output", &RegressionTreeNode::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The mising leave output vector\n");

    declareOption(ol, "right_leave_id", &RegressionTreeNode::dummy_int,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The id of the right leave\n");     
    declareOption(ol, "left_leave_id", &RegressionTreeNode::dummy_int,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The id of the left leave\n");
    declareOption(ol, "missing_leave_id", &RegressionTreeNode::dummy_int,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The id of the missing leave\n");
    declareOption(ol, "leave_id", &RegressionTreeNode::dummy_int,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The id of the leave\n");
    declareOption(ol, "length", &RegressionTreeNode::dummy_int,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The length of the train set\n");
    declareOption(ol, "inputsize", &RegressionTreeNode::dummy_int,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The inputsize of the train set\n");

    inherited::declareOptions(ol);
}

void RegressionTreeNode::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(missing_is_valid, copies);
    deepCopyField(loss_function_weight, copies);
    deepCopyField(verbosity, copies);
    deepCopyField(leave_template, copies);
    deepCopyField(train_set, copies);
    deepCopyField(leave, copies);
    deepCopyField(leave_output, copies);
    deepCopyField(leave_error, copies);
    deepCopyField(split_col, copies);
    deepCopyField(split_balance, copies);
    deepCopyField(split_feature_value, copies);
    deepCopyField(after_split_error, copies);
    deepCopyField(missing_node, copies);
    deepCopyField(missing_leave, copies);
    deepCopyField(left_node, copies);
    deepCopyField(left_leave, copies);
    deepCopyField(right_node, copies);
    deepCopyField(right_leave, copies);
}

void RegressionTreeNode::build()
{
    inherited::build();
    build_();
}

void RegressionTreeNode::build_()
{
}

void RegressionTreeNode::initNode(PP<RegressionTreeRegisters> the_train_set, PP<RegressionTreeLeave> the_leave, PP<RegressionTreeLeave> the_leave_template)
{
    train_set = the_train_set;
    leave = the_leave;
    leave_template = the_leave_template;
    int missing_leave_id = train_set->getNextId();
    int left_leave_id =  train_set->getNextId();
    int right_leave_id =  train_set->getNextId();

    missing_leave = ::PLearn::deepCopy(leave_template);
    missing_leave->id=missing_leave_id;
    missing_leave->missing_leave=missing_is_valid;
    missing_leave->initLeave(train_set);

    left_leave = ::PLearn::deepCopy(leave_template);
    left_leave->id=left_leave_id;
    left_leave->initLeave(train_set);

    right_leave = ::PLearn::deepCopy(leave_template);
    right_leave->id=right_leave_id;
    right_leave->initLeave(train_set);

    leave_output.resize(2);
    leave_error.resize(3);

    leave->getOutputAndError(leave_output,leave_error);
}

void RegressionTreeNode::lookForBestSplit()
{
    if(leave->length<=1)
        return;
    TVec<int> candidate(train_set->length());//list of candidate row to split
    candidate.resize(0);
    TVec<int> registered_row;
    tmp_vec.resize(2);
    Vec left_error(3);
    Vec right_error(3);
    Vec missing_error(3);
    missing_error.clear();

    int leave_id = leave->id;
    for (int col = 0; col < train_set->inputsize(); col++)
    {
        missing_leave->initStats();
        left_leave->initStats();
        right_leave->initStats();
        registered_row.resize(0);
        train_set->getAllRegisteredRow(leave_id,col,registered_row);
        PLASSERT(registered_row.size()>0);
        for(int row_idx = 0;row_idx<registered_row.size();row_idx++)
        {
            int row=registered_row[row_idx];
            if (is_missing(train_set->get(row, col)))
                missing_leave->addRow(row);
            else {
                left_leave->addRow(row);
                candidate.append(row);
            }
        }
        missing_leave->getOutputAndError(tmp_vec, missing_error);

        //in case of missing value
        if(candidate.size()==0)
            continue;
        int row = candidate.pop();
        while (candidate.size()>0)
        {
            int next_row = candidate.pop();
            left_leave->removeRow(row, tmp_vec, left_error);
            right_leave->addRow(row, tmp_vec, right_error);
            compareSplit(col, train_set->get(next_row, col), train_set->get(row, col), left_error, right_error, missing_error);
            row = next_row;
        }
    }
}

void RegressionTreeNode::compareSplit(int col, real left_leave_last_feature, real right_leave_first_feature,
                                      Vec left_error, Vec right_error, Vec missing_error)
{
    PLASSERT(left_leave_last_feature<=right_leave_first_feature);
    if (left_leave_last_feature >= right_leave_first_feature) return;
    real work_error = missing_error[0] + missing_error[1] + left_error[0] + left_error[1] + right_error[0] + right_error[1];
    int work_balance = abs(left_leave->getLength() - right_leave->getLength());
    if (fast_is_more(work_error,after_split_error)) return;
    else if (fast_is_equal(work_error,after_split_error) &&
             fast_is_more(work_balance,split_balance)) return;

    split_col = col;
    split_feature_value = 0.5 * (right_leave_first_feature + left_leave_last_feature);
    after_split_error = work_error;
    split_balance = work_balance;
}

int RegressionTreeNode::expandNode()
{
    if (split_col < 0)
    {
        verbose("RegressionTreeNode: there is no more split candidate", 3);
        return -1;
    }
    missing_leave->initStats();
    left_leave->initStats();
    right_leave->initStats();
    TVec<int>registered_row;
    train_set->getAllRegisteredRow(leave->id,split_col,registered_row);

    for (int row_index = 0;row_index<registered_row.size();row_index++)
    {
        int row=registered_row[row_index];
        if (is_missing(train_set->get(row, split_col)))
        {
            missing_leave->addRow(row);
            missing_leave->registerRow(row);
        }
        else
        {
            if (train_set->get(row, split_col) < split_feature_value)
            {
                left_leave->addRow(row);
                left_leave->registerRow(row);
            }
            else
            {
                right_leave->addRow(row);
                right_leave->registerRow(row);
            }
        }
    }

    PLASSERT(left_leave->length>0);
    PLASSERT(right_leave->length>0);
    PLASSERT(left_leave->length + right_leave->length + missing_leave->length
             == registered_row.size());
//  leave->printStats();
//  left_leave->printStats();
//  right_leave->printStats();
    if (missing_is_valid > 0)
    {
        missing_node = new RegressionTreeNode();
        missing_node->missing_is_valid=missing_is_valid;
        missing_node->loss_function_weight=loss_function_weight;
        missing_node->verbosity=verbosity;
        missing_node->initNode(train_set, missing_leave, leave_template);
        missing_node->lookForBestSplit();
    }
    left_node = new RegressionTreeNode();
    left_node->missing_is_valid=missing_is_valid;
    left_node->loss_function_weight=loss_function_weight;
    left_node->verbosity=verbosity;
    left_node->initNode(train_set, left_leave, leave_template);
    left_node->lookForBestSplit();
    right_node = new RegressionTreeNode();
    right_node->missing_is_valid=missing_is_valid;
    right_node->loss_function_weight=loss_function_weight;
    right_node->verbosity=verbosity;
    right_node->initNode(train_set, right_leave, leave_template);
    right_node->lookForBestSplit();
    return split_col;
}

int RegressionTreeNode::getSplitBalance()const
{
    if (split_col < 0) return train_set->length();
    return split_balance;
}

real RegressionTreeNode::getErrorImprovment()const
{
    if (split_col < 0) return -1.0;
    real err=leave_error[0] + leave_error[1] - after_split_error;
    PLASSERT(is_equal(err,0)||err>0);
    return err;
}

int RegressionTreeNode::getSplitCol()const
{
    return split_col;
}

TVec< PP<RegressionTreeNode> > RegressionTreeNode::getNodes()
{
    TVec< PP<RegressionTreeNode> > return_value;
    if (missing_is_valid > 0)
    {
        return_value.resize(3);
        return_value[2] = missing_node;
    }
    else
    {
        return_value.resize(2);
    }
    return_value[0] = left_node;
    return_value[1] = right_node;
    return return_value;
}

void RegressionTreeNode::computeOutput(const Vec& inputv, Vec& outputv)
{
    if (!left_node)
    {
        outputv[0] = leave_output[0];
        outputv[1] = leave_output[1];
        return;
    }
    if (is_missing(inputv[split_col]))
    {
        if (missing_is_valid > 0)
        {
            missing_node->computeOutput(inputv, outputv);
        }
        else
        {
            tmp_vec.resize(3);
            missing_leave->getOutputAndError(outputv,tmp_vec);
        }
        return;
    }
    if (inputv[split_col] > split_feature_value)
    {
        right_node->computeOutput(inputv, outputv);
        return;
    }
    else
    {
        left_node->computeOutput(inputv, outputv);
        return;
    }
}

void RegressionTreeNode::verbose(string the_msg, int the_level)
{
    if (verbosity >= the_level)
        cout << the_msg << endl;
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
