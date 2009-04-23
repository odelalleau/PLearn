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
#define PL_LOG_MODULE_NAME "RegressionTreeNode"
#include <plearn/io/pl_log.h>

#include "RegressionTreeNode.h"
#include "RegressionTreeRegisters.h"
#include "RegressionTreeLeave.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(RegressionTreeNode,
                        "Object to represent the nodes of a regression tree.",
                        "It may be a final node pointing to a leave.\n"
                        "If that is the case, it knows always what would be the best possible split for that leave.\n"
                        "It may be an expanded node pointing to 3 children nodes: a leave for missing values on the splitting attribute,\n"
                        "a left leave for samples with values below the value of the splitting attribute, and a right leave for the others,\n"
    );
int RegressionTreeNode::dummy_int = 0;
Vec RegressionTreeNode::tmp_vec;
RegressionTreeNode::RegressionTreeNode():
    missing_is_valid(0),
    split_col(-1),
    split_balance(INT_MAX),
    split_feature_value(REAL_MAX),
    after_split_error(REAL_MAX)
{
    build();
}
RegressionTreeNode::RegressionTreeNode(int missing_is_valid_):
    missing_is_valid(missing_is_valid_),
    split_col(-1),
    split_balance(INT_MAX),
    split_feature_value(REAL_MAX),
    after_split_error(REAL_MAX)
{
    build();
}

RegressionTreeNode::~RegressionTreeNode()
{
}

void RegressionTreeNode::finalize(){
    //those variable are not needed after training.
    right_leave = 0;
    left_leave = 0;
    leave = 0;
    //missing_leave used in computeOutputsAndNodes
    if(right_node)
        right_node->finalize();
    if(left_node)
        left_node->finalize();
    if(missing_node)
        missing_node->finalize();
}

void RegressionTreeNode::declareOptions(OptionList& ol)
{ 
    declareOption(ol, "missing_is_valid", &RegressionTreeNode::missing_is_valid, OptionBase::buildoption,
                  "If set to 1, missing values will be treated as valid, and missing nodes will be potential for splits.\n");
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

    declareStaticOption(ol, "left_error", &RegressionTreeNode::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The left leave error vector\n");
    declareStaticOption(ol, "right_error", &RegressionTreeNode::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The right leave error vector\n");
    declareStaticOption(ol, "missing_error", &RegressionTreeNode::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The missing leave error vector\n");
    declareStaticOption(ol, "left_output", &RegressionTreeNode::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The left leave output vector\n");
    declareStaticOption(ol, "right_output", &RegressionTreeNode::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The right leave output vector\n");
    declareStaticOption(ol, "missing_output", &RegressionTreeNode::tmp_vec,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The mising leave output vector\n");

    declareStaticOption(ol, "right_leave_id", &RegressionTreeNode::dummy_int,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The id of the right leave\n");     
    declareStaticOption(ol, "left_leave_id", &RegressionTreeNode::dummy_int,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The id of the left leave\n");
    declareStaticOption(ol, "missing_leave_id", &RegressionTreeNode::dummy_int,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The id of the missing leave\n");
    declareStaticOption(ol, "leave_id", &RegressionTreeNode::dummy_int,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The id of the leave\n");
    declareStaticOption(ol, "length", &RegressionTreeNode::dummy_int,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The length of the train set\n");
    declareStaticOption(ol, "inputsize", &RegressionTreeNode::dummy_int,
                  OptionBase::learntoption | OptionBase::nosave,
                  "DEPRECATED The inputsize of the train set\n");

    inherited::declareOptions(ol);
}

void RegressionTreeNode::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    
    //not done as the template don't change
    deepCopyField(leave, copies);
    deepCopyField(leave_output, copies);
    deepCopyField(leave_error, copies);

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

void RegressionTreeNode::initNode(PP<RegressionTree> the_tree,
                                  PP<RegressionTreeLeave> the_leave)
{
    tree=the_tree;
    leave=the_leave;
    PP<RegressionTreeRegisters> the_train_set = tree->getSortedTrainingSet();
    PP<RegressionTreeLeave> leave_template = tree->leave_template;
    int missing_leave_id = the_train_set->getNextId();
    int left_leave_id =  the_train_set->getNextId();
    int right_leave_id =  the_train_set->getNextId();

    missing_leave = ::PLearn::deepCopy(leave_template);
    missing_leave->initLeave(the_train_set, missing_leave_id, missing_is_valid);

    left_leave = ::PLearn::deepCopy(leave_template);
    left_leave->initLeave(the_train_set, left_leave_id);

    right_leave = ::PLearn::deepCopy(leave_template);
    right_leave->initLeave(the_train_set, right_leave_id);

    leave_output.resize(leave_template->outputsize());
    leave_error.resize(3);

    leave->getOutputAndError(leave_output,leave_error);

    //we do it here as an optimization
    //this don't change the leave_error.
    //If you want the leave_error to include this rounding, 
    // use the RegressionTreeMultiVlassLeave
    Vec multiclass_outputs = tree->multiclass_outputs;
    if (multiclass_outputs.length() <= 0) return;
    real closest_value=multiclass_outputs[0];
    real margin_to_closest_value=abs(leave_output[0] - multiclass_outputs[0]);
    for (int value_ind = 1; value_ind < multiclass_outputs.length(); value_ind++)
    {
        real v=abs(leave_output[0] - multiclass_outputs[value_ind]);
        if (v < margin_to_closest_value)
        {
            closest_value = multiclass_outputs[value_ind];
            margin_to_closest_value = v;
        }
    }
    leave_output[0] = closest_value;
}

//#define RCMP
void RegressionTreeNode::lookForBestSplit()
{
    if(leave->length()<=1)
        return;
    TVec<RTR_type> candidate(0, leave->length());//list of candidate row to split
    TVec<RTR_type> registered_row(leave->length());
    TVec<pair<RTR_target_t,RTR_weight_t> > registered_target_weight(leave->length());
    registered_target_weight.resize(leave->length());
    registered_target_weight.resize(0);
    Vec registered_value(0, leave->length());
    tmp_vec.resize(leave->outputsize());
    Vec left_error(3);
    Vec right_error(3);
    Vec missing_error(3);
    missing_error.clear();
    PP<RegressionTreeRegisters> train_set = tree->getSortedTrainingSet();
    bool one_pass_on_data=!train_set->haveMissing();

    int inputsize = train_set->inputsize();
#ifdef RCMP
    Vec row_split_err(inputsize);
    Vec row_split_value(inputsize);
    Vec row_split_balance(inputsize);
    row_split_err.clear();
    row_split_value.clear();
    row_split_balance.clear();
#endif
    int leave_id = leave->getId();
    
    int l_length = 0;
    real l_weights_sum = 0;
    real l_targets_sum = 0;
    real l_weighted_targets_sum = 0;
    real l_weighted_squared_targets_sum = 0;

    for (int col = 0; col < inputsize; col++)
    {
        missing_leave->initStats();
        left_leave->initStats();
        right_leave->initStats();
        
        PLASSERT(registered_row.size()==leave->length());
        PLASSERT(candidate.size()==0);
        tuple<real,real,int> ret;
#ifdef NPREFETCH
        //The ifdef is in case we don't want to use the optimized version with
        //prefetch of memory. Maybe the optimization is hurtfull for some computer.
        train_set->getAllRegisteredRow(leave_id, col, registered_row,
                                       registered_target_weight,
                                       registered_value);

        PLASSERT(registered_row.size()==leave->length());
        PLASSERT(candidate.size()==0);

        //we do this optimization in case their is many row with the same value
        //at the end as with binary variable.
        int row_idx_end = registered_row.size() - 1;
        int prev_row=registered_row[row_idx_end];
        real prev_val=registered_value[row_idx_end];
        for( ;row_idx_end>0;row_idx_end--)
        {
            int row=prev_row;
            real val=prev_val;
            prev_row = registered_row[row_idx_end - 1];
            prev_val = registered_value[row_idx_end - 1];
            if (RTR_HAVE_MISSING && is_missing(val))
                missing_leave->addRow(row, registered_target_weight[row_idx_end].first,
                                      registered_target_weight[row_idx_end].second);
            else if(val==prev_val)
                right_leave->addRow(row, registered_target_weight[row_idx_end].first,
                                    registered_target_weight[row_idx_end].second);
            else
                break;
        }

        for(int row_idx = 0;row_idx<=row_idx_end;row_idx++)
        {
            int row=registered_row[row_idx];
            if (RTR_HAVE_MISSING && is_missing(registered_value[row_idx]))
                missing_leave->addRow(row, registered_target_weight[row_idx].first,
                                      registered_target_weight[row_idx].second);
            else {
                left_leave->addRow(row, registered_target_weight[row_idx].first,
                                   registered_target_weight[row_idx].second);
                candidate.append(row);
            }
        }

        missing_leave->getOutputAndError(tmp_vec, missing_error);
        ret=bestSplitInRow(col, candidate, left_error,
                           right_error, missing_error,
                           right_leave, left_leave,
                           train_set, registered_value,
                           registered_target_weight);

#else
        if(!one_pass_on_data){
            train_set->getAllRegisteredRowLeave(leave_id, col, registered_row,
                                                registered_target_weight,
                                                registered_value,
                                                missing_leave,
                                                left_leave,
                                                right_leave, candidate);
            PLASSERT(registered_target_weight.size()==candidate.size());
            PLASSERT(registered_value.size()==candidate.size());
            PLASSERT(left_leave->length()+right_leave->length()
                     +missing_leave->length()==leave->length());
            PLASSERT(candidate.size()>0||(left_leave->length()+right_leave->length()==0));
            missing_leave->getOutputAndError(tmp_vec, missing_error);
            ret=bestSplitInRow(col, candidate, left_error,
                               right_error, missing_error,
                               right_leave, left_leave,
                               train_set, registered_value,
                               registered_target_weight);
        }else{
            ret=train_set->bestSplitInRow(leave_id, col, registered_row,
                                          left_leave,
                                          right_leave, left_error,
                                          right_error);
        }
        PLASSERT(registered_row.size()==leave->length());
#endif

        if(col==0){
            l_length=left_leave->length()+right_leave->length()+missing_leave->length();
            l_weights_sum=left_leave->weights_sum+right_leave->weights_sum+missing_leave->weights_sum;
            l_targets_sum=left_leave->targets_sum+right_leave->targets_sum+missing_leave->targets_sum;
            l_weighted_targets_sum=left_leave->weighted_targets_sum
                +right_leave->weighted_targets_sum+missing_leave->weighted_targets_sum;
            l_weighted_squared_targets_sum=left_leave->weighted_squared_targets_sum
                +right_leave->weighted_squared_targets_sum+missing_leave->weighted_squared_targets_sum;
        }else if(!one_pass_on_data){
            PLCHECK(l_length==left_leave->length()+right_leave->length()
                    +missing_leave->length());
            PLCHECK(fast_is_equal(l_weights_sum,
                                  left_leave->weights_sum+right_leave->weights_sum
                                  +missing_leave->weights_sum));
            PLCHECK(fast_is_equal(l_targets_sum,
                                  left_leave->targets_sum+right_leave->targets_sum
                                  +missing_leave->targets_sum));
            PLCHECK(fast_is_equal(l_weighted_targets_sum,
                                  left_leave->weighted_targets_sum
                                  +right_leave->weighted_targets_sum
                                  +missing_leave->weighted_targets_sum));
            PLCHECK(fast_is_equal(l_weighted_squared_targets_sum,
                                  left_leave->weighted_squared_targets_sum
                                  +right_leave->weighted_squared_targets_sum
                                  +missing_leave->weighted_squared_targets_sum));
        }

#ifdef RCMP
        row_split_err[col] = get<0>(ret);
        row_split_value[col] = get<1>(ret);
        row_split_balance[col] = get<2>(ret);
#endif
        if (fast_is_more(get<0>(ret), after_split_error)) continue;
        else if (fast_is_equal(get<0>(ret), after_split_error) &&
                 fast_is_more(get<2>(ret), split_balance)) continue;
        else if (fast_is_equal(get<0>(ret), REAL_MAX)) continue;

        split_col = col;
        after_split_error = get<0>(ret);
        split_feature_value = get<1>(ret);
        split_balance = get<2>(ret);
        PLASSERT(fast_is_less(after_split_error,REAL_MAX)||split_col==-1);
    }
    PLASSERT(fast_is_less(after_split_error,REAL_MAX)||split_col==-1);

    EXTREME_MODULE_LOG<<"error after split: "<<after_split_error<<endl;
    EXTREME_MODULE_LOG<<"split value: "<<split_feature_value<<endl;
    EXTREME_MODULE_LOG<<"split_col: "<<split_col;
    if(split_col>=0)
        EXTREME_MODULE_LOG<<" "<<train_set->fieldName(split_col);
    EXTREME_MODULE_LOG<<endl;
}

tuple<real,real,int>RegressionTreeNode::bestSplitInRow(
    int col,
    TVec<RTR_type>& candidates,
    Vec left_error,
    Vec right_error,
    const Vec missing_error,
    PP<RegressionTreeLeave> right_leave,
    PP<RegressionTreeLeave> left_leave,
    PP<RegressionTreeRegisters> train_set,
    Vec values,TVec<pair<RTR_target_t,RTR_weight_t> > t_w
    )
{
    int best_balance=INT_MAX;
    real best_feature_value = REAL_MAX;
    real best_split_error = REAL_MAX;
    //in case of only missing value
    if(candidates.size()==0)
        return make_tuple(best_feature_value, best_split_error, best_balance);

    int row = candidates.last();
    Vec tmp(3);

    real missing_errors = missing_error[0] + missing_error[1];
    real first_value=values.first();
    real next_feature=values.last();

    //next_feature!=first_value is to check if their is more split point
    // in case of binary variable or variable with few different value,
    // this give a great speed up.
    for(int i=candidates.size()-2;i>=0&&next_feature!=first_value;i--)
    {
        int next_row = candidates[i];
        real row_feature=next_feature;
        PLASSERT(is_equal(row_feature,values[i+1]));
//                 ||(is_missing(row_feature)&&is_missing(values[i+1])));
        next_feature=values[i];

        real target=t_w[i+1].first;
        real weight=t_w[i+1].second;
        PLASSERT(train_set->get(next_row, col)==values[i]);
        PLASSERT(train_set->get(row, col)==values[i+1]);
        PLASSERT(next_feature<=row_feature);


        left_leave->removeRow(row, target, weight);
        right_leave->addRow(row, target, weight);
        row = next_row;
        if (next_feature < row_feature){
            left_leave->getOutputAndError(tmp, left_error);
            right_leave->getOutputAndError(tmp, right_error);
        }else
            continue;
        real work_error = missing_errors + left_error[0]
            + left_error[1] + right_error[0] + right_error[1];
        int work_balance = abs(left_leave->length() -
                               right_leave->length());
        if (fast_is_more(work_error,best_split_error)) continue;
        else if (fast_is_equal(work_error,best_split_error) &&
                 fast_is_more(work_balance,best_balance)) continue;

        best_feature_value = 0.5 * (row_feature + next_feature);
        best_split_error = work_error;
        best_balance = work_balance;

    }
    candidates.resize(0);
    return make_tuple(best_split_error, best_feature_value, best_balance);
}

void RegressionTreeNode::compareSplit(int col, real left_leave_last_feature, real right_leave_first_feature,
                                      Vec left_error, Vec right_error, Vec missing_error)
{
    PLASSERT(left_leave_last_feature<=right_leave_first_feature);
    if (left_leave_last_feature >= right_leave_first_feature) return;
    real work_error = missing_error[0] + missing_error[1] + left_error[0] + left_error[1] + right_error[0] + right_error[1];
    int work_balance = abs(left_leave->length() - right_leave->length());
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
    TVec<RTR_type>registered_row(leave->length());
    PP<RegressionTreeRegisters> train_set = tree->getSortedTrainingSet();
    train_set->getAllRegisteredRow(leave->getId(),split_col,registered_row);

    for (int row_index = 0;row_index<registered_row.size();row_index++)
    {
        int row=registered_row[row_index];
        if (RTR_HAVE_MISSING && is_missing(train_set->get(row, split_col)))
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

    PLASSERT(left_leave->length()>0);
    PLASSERT(right_leave->length()>0);
    PLASSERT(left_leave->length() + right_leave->length() + 
             missing_leave->length() == registered_row.size());
//  leave->printStats();
//  left_leave->printStats();
//  right_leave->printStats();
    if (RTR_HAVE_MISSING && missing_is_valid > 0)
    {
        missing_node = new RegressionTreeNode(missing_is_valid);
        missing_node->initNode(tree, missing_leave);
        missing_node->lookForBestSplit();
    }
    left_node = new RegressionTreeNode(missing_is_valid);
    left_node->initNode(tree, left_leave);
    left_node->lookForBestSplit();
    right_node = new RegressionTreeNode(missing_is_valid);
    right_node->initNode(tree, right_leave);
    right_node->lookForBestSplit();
    return split_col;
}

void RegressionTreeNode::computeOutputAndNodes(const Vec& inputv, Vec& outputv,
                                       TVec<PP<RegressionTreeNode> >* nodes)
{
    if(nodes)
        nodes->append(this);
    if (!left_node)
    {
        outputv << leave_output;
        return;
    }
    if (RTR_HAVE_MISSING && is_missing(inputv[split_col]))
    {
        if (missing_is_valid > 0)
        {
            missing_node->computeOutputAndNodes(inputv, outputv, nodes);
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
        right_node->computeOutputAndNodes(inputv, outputv, nodes);
        return;
    }
    else
    {
        left_node->computeOutputAndNodes(inputv, outputv, nodes);
        return;
    }
}

void RegressionTreeNode::verbose(string the_msg, int the_level)
{
    if (tree->verbosity >= the_level)
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
