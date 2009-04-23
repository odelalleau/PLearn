// -*- C++ -*-

// RegressionTreeRegisters.cc
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


/* **********************************************************************************    
 * $Id: RegressionTreeRegisters.cc, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout *
 * This file is part of the PLearn library.                                       *
 ********************************************************************************** */

#include "RegressionTreeRegisters.h"
#include "RegressionTreeLeave.h"
#define PL_LOG_MODULE_NAME RegressionTreeRegisters
#include <plearn/io/pl_log.h>
#include <plearn/vmat/TransposeVMatrix.h>
#include <plearn/vmat/MemoryVMatrixNoSave.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/io/fileutils.h>
#include <plearn/io/load_and_save.h>
#include <limits>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(RegressionTreeRegisters,
                        "Object to maintain the various registers of a regression tree", 
                        "It is used first, to sort the learner train set on all dimensions of the input samples.\n"
                        "It keeps matrices of row indices to navigate thru the training set in ascending value order fo each variable.\n"
                        "Missing values are sorted at the beginning of the column.\n"
                        "It also keeps registers of which leave, a row belongs to as the tree is built.\n"
                        "It is also used to maintain the next available leave id.\n"
    );

RegressionTreeRegisters::RegressionTreeRegisters():
    report_progress(0),
    verbosity(0),
    next_id(0),
    do_sort_rows(true),
    mem_tsource(true),
    have_missing(true),
    compact_reg_leave(-1)
{
    build();
}

RegressionTreeRegisters::RegressionTreeRegisters(VMat source_,
                                                 TMat<RTR_type> tsorted_row_,
                                                 VMat tsource_,
                                                 bool report_progress_,
                                                 bool verbosity_,
                                                 bool do_sort_rows_,
                                                 bool mem_tsource_):
    report_progress(report_progress_),
    verbosity(verbosity_),
    next_id(0),
    do_sort_rows(do_sort_rows_),
    mem_tsource(mem_tsource_),
    have_missing(true),
    compact_reg_leave(-1)
{
    source = source_;
    tsource = tsource_;
    if(tsource->classname()=="MemoryVMatrixNoSave")
        tsource_mat = tsource.toMat();
    tsorted_row = tsorted_row_;
    checkMissing();
    build();
}

RegressionTreeRegisters::RegressionTreeRegisters(VMat source_,
                                                 bool report_progress_,
                                                 bool verbosity_,
                                                 bool do_sort_rows_,
                                                 bool mem_tsource_):
    report_progress(report_progress_),
    verbosity(verbosity_),
    next_id(0),
    do_sort_rows(do_sort_rows_),
    mem_tsource(mem_tsource_),
    have_missing(true),
    compact_reg_leave(-1)
{
    source = source_;
    build();
}

RegressionTreeRegisters::~RegressionTreeRegisters()
{
}

void RegressionTreeRegisters::declareOptions(OptionList& ol)
{ 
    declareOption(ol, "report_progress", &RegressionTreeRegisters::report_progress, OptionBase::buildoption,
                  "The indicator to report progress through a progress bar\n");
    declareOption(ol, "verbosity", &RegressionTreeRegisters::verbosity, OptionBase::buildoption,
                  "The desired level of verbosity\n");
    declareOption(ol, "tsource", &RegressionTreeRegisters::tsource,
                  OptionBase::learntoption | OptionBase::nosave,
                  "The source VMatrix transposed");

    declareOption(ol, "source", &RegressionTreeRegisters::source,
                  OptionBase::buildoption,
                  "The source VMatrix");

    declareOption(ol, "next_id", &RegressionTreeRegisters::next_id, OptionBase::learntoption,
                  "The next id for creating a new leave\n");
    declareOption(ol, "leave_register", &RegressionTreeRegisters::leave_register, OptionBase::learntoption,
                  "The vector identifying the leave to which, each row belongs\n");

    declareOption(ol, "do_sort_rows", &RegressionTreeRegisters::do_sort_rows,
                  OptionBase::buildoption,
                  "Do we generate the sorted rows? Not usefull if used only to test.\n");

    declareOption(ol, "mem_tsource", &RegressionTreeRegisters::mem_tsource,
                  OptionBase::buildoption,
                  "Do we put the tsource in memory? default to true as this"
                  " give an great speed up for the trainning of RegressionTree.\n");

    //too big to save
    declareOption(ol, "tsorted_row", &RegressionTreeRegisters::tsorted_row, OptionBase::nosave,
                  "The matrix holding the sequence of samples in ascending value order for each dimension\n");

    inherited::declareOptions(ol);
}

void RegressionTreeRegisters::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(leave_register, copies);
//tsource and tsorted_row should be deep copied, but currently when it is deep copied
// the copy is modified. To save memory we don't do it.
// It is deep copied eavily by HyperLearner and HyperOptimizer
//    deepCopyField(tsorted_row, copies);
//    deepCopyField(tsource,copies);
//no need to deep copy source as we don't reuse it after initialization
//    deepCopyField(source,copies);
}

void RegressionTreeRegisters::build()
{
    inherited::build();
    build_();
}

void RegressionTreeRegisters::build_()
{
    if(!source)
        return;
    //check that we can put all the examples of the train_set
    //with respect to the size of RTR_type who limit the capacity
    PLCHECK(source.length()>0 
            && (unsigned)source.length()
            <= std::numeric_limits<RTR_type>::max());
    PLCHECK(source->targetsize()==1);
    PLCHECK(source->weightsize()<=1);
    PLCHECK(source->inputsize()>0);

    if(!tsource){
        tsource = VMat(new TransposeVMatrix(new SubVMatrix(
                                                source, 0,0,source->length(),
                                                source->inputsize())));
        if(mem_tsource){
            PP<MemoryVMatrixNoSave> tmp = new MemoryVMatrixNoSave(tsource);
            tsource = VMat(tmp);
        }
        if(tsource->classname()=="MemoryVMatrixNoSave")
            tsource_mat = tsource.toMat();
    }
    setMetaInfoFrom(source);
    weightsize_=1;
    targetsize_=1;
    target_weight.resize(source->length());
    if(source->weightsize()<=0){
        width_++;
        for(int i=0;i<source->length();i++){
            target_weight[i].first=source->get(i,inputsize());
            target_weight[i].second=1.0 / length();
        }
    }else
        for(int i=0;i<source->length();i++){
            target_weight[i].first=source->get(i,inputsize());
            target_weight[i].second=source->get(i,inputsize()+targetsize());
        }
#if 0
    //usefull to weight the dataset to have the sum of weight==1 or ==length()
    real weights_sum=0;
    for(int i=0;i<source->length();i++){
        weights_sum+=target_weight[i].second;
    }
    pout<<weights_sum<<endl;
//    real t=length()/weights_sum;
    real t=1/weights_sum;
    for(int i=0;i<source->length();i++){
        target_weight[i].second*=t;
    }
    weights_sum=0;
    for(int i=0;i<source->length();i++){
        weights_sum+=target_weight[i].second;
    }
    pout<<weights_sum<<endl;
#endif

    leave_register.resize(length());
    sortRows();
//    compact_reg.resize(length());
}

void RegressionTreeRegisters::reinitRegisters()
{
    next_id = 0;

    //in case we don't save the sorted data
    sortRows();
}

void RegressionTreeRegisters::getAllRegisteredRowLeave(
    RTR_type_id leave_id, int col,
    TVec<RTR_type> &reg,
    TVec<pair<RTR_target_t,RTR_weight_t> > &t_w,
    Vec &value,
    PP<RegressionTreeLeave> missing_leave,
    PP<RegressionTreeLeave> left_leave,
    PP<RegressionTreeLeave> right_leave,
    TVec<RTR_type> &candidate) const
{
    PLASSERT(tsource_mat.length()==tsource.length());

    getAllRegisteredRow(leave_id,col,reg);
    t_w.resize(reg.length());
    value.resize(reg.length());
    real * p = tsource_mat[col];
    pair<RTR_target_t,RTR_weight_t> * ptw = target_weight.data();
    pair<RTR_target_t,RTR_weight_t>* ptwd = t_w.data();
    real * pv = value.data();
    RTR_type * preg = reg.data();

    //It is better to do multiple pass for memory access.

    //we do this optimization in case their is many row with the same value
    //at the end as with binary variable.
    //we do it here to overlap computation and memory access
    int row_idx_end = reg.size() - 1;
    int prev_row=preg[row_idx_end];
    real prev_val=p[prev_row];
    PLASSERT(reg.size()>row_idx_end && row_idx_end>=0);
    PLASSERT(is_equal(p[prev_row],tsource(col,prev_row)));

    for( ;row_idx_end>0;row_idx_end--)
    {
        int futur_row = preg[row_idx_end-8];
        __builtin_prefetch(&ptw[futur_row],1,2);
        __builtin_prefetch(&p[futur_row],1,2);

        int row=prev_row;
        real val=prev_val;
        prev_row = preg[row_idx_end-1];
        prev_val = p[prev_row];

        PLASSERT(reg.size()>row_idx_end && row_idx_end>0);
        PLASSERT(target_weight.size()>row && row>=0);
        PLASSERT(is_equal(p[row],tsource(col,row)));
        RTR_target_t target = ptw[row].first;
        RTR_weight_t weight = ptw[row].second;

        if (RTR_HAVE_MISSING && is_missing(val))
            missing_leave->addRow(row, target, weight);
        else if(val==prev_val)
            right_leave->addRow(row, target, weight);
        else
            break;
    }

    //We need the last data for an optimization in RTN
    {
        int idx=reg.size()-1;
        PLASSERT(reg.size()>idx && idx>=0);
        int row=int(preg[idx]);
        PLASSERT(target_weight.size()>row && row>=0);
        PLASSERT(is_equal(p[row],tsource(col,row)));
        pv[idx]=p[row];
    }
    for(int row_idx = 0;row_idx<=row_idx_end;row_idx++)
    {
        int futur_row = preg[row_idx+8];
        __builtin_prefetch(&ptw[futur_row],1,2);
        __builtin_prefetch(&p[futur_row],1,2);
            
        PLASSERT(reg.size()>row_idx && row_idx>=0);
        int row=int(preg[row_idx]);
        real val=p[row];
        PLASSERT(target_weight.size()>row && row>=0);
        PLASSERT(is_equal(p[row],tsource(col,row)));
            
        RTR_target_t target = ptw[row].first;
        RTR_weight_t weight = ptw[row].second;
        if (RTR_HAVE_MISSING && is_missing(val)){
            missing_leave->addRow(row, target, weight);
        }else {
            left_leave->addRow(row, target, weight);
            candidate.append(row);
            ptwd[row_idx].first=ptw[row].first;
            ptwd[row_idx].second=ptw[row].second;
            pv[row_idx]=val;
        }
    }
    t_w.resize(candidate.size());
    value.resize(candidate.size());
}

void RegressionTreeRegisters::getAllRegisteredRow(RTR_type_id leave_id, int col,
                                                  TVec<RTR_type> &reg,
                                                  TVec<pair<RTR_target_t,RTR_weight_t> > &t_w,
                                                  Vec &value) const
{
    PLASSERT(tsource_mat.length()==tsource.length());

    getAllRegisteredRow(leave_id,col,reg);
    t_w.resize(reg.length());
    value.resize(reg.length());
    real * p = tsource_mat[col];
    pair<RTR_target_t,RTR_weight_t> * ptw = target_weight.data();
    pair<RTR_target_t,RTR_weight_t>* ptwd = t_w.data();
    real * pv = value.data();
    RTR_type * preg = reg.data();

    if(weightsize() <= 0){
        RTR_weight_t w = 1.0 / length();
        for(int i=0;i<reg.length();i++){
            PLASSERT(tsource->get(col, reg[i])==p[reg[i]]);
            int idx = int(preg[i]);
            ptwd[i].first = ptw[idx].first;
            ptwd[i].second = w;
            pv[i] = p[idx];
        }
    } else {
        //It is better to do multiple pass for memory access.
        for(int i=0;i<reg.length();i++){
            int idx = int(preg[i]);
            ptwd[i].first = ptw[idx].first;
            ptwd[i].second = ptw[idx].second;

        }
        for(int i=0;i<reg.length();i++){
            PLASSERT(tsource->get(col, reg[i])==p[reg[i]]);
            int idx = int(preg[i]);
            pv[i] = p[idx];
        }
    }
}

//! reg.size() == the number of row that we will put in it.
//! the register are not sorted. They are in increasing order.
void RegressionTreeRegisters::getAllRegisteredRow(RTR_type_id leave_id,
                                                  TVec<RTR_type> &reg) const
{
    PLASSERT(tsource_mat.length()==tsource.length());

    int idx=0;
    int n=reg.length();
    RTR_type* preg = reg.data();
    RTR_type_id* pleave_register = leave_register.data();
    for(int i=0;i<length() && n> idx;i++){
        if (pleave_register[i] == leave_id){
            preg[idx++]=i;
            PLASSERT(reg[idx-1]==i);
        }
    }
    PLASSERT(idx==reg->size());
}

//! reg.size() == the number of row that we will put in it.
//! the register are sorted by col.
void RegressionTreeRegisters::getAllRegisteredRow(RTR_type_id leave_id, int col,
                                                  TVec<RTR_type> &reg) const
{
    PLASSERT(tsource_mat.length()==tsource.length());

    int idx=0;
    int n=reg.length();
    RTR_type* preg = reg.data();
    RTR_type* ptsorted_row = tsorted_row[col];
    RTR_type_id* pleave_register = leave_register.data();
    if(compact_reg.size()==0){
        for(int i=0;i<length() && n> idx;i++){
            PLASSERT(ptsorted_row[i]==tsorted_row(col, i));
            RTR_type srow = ptsorted_row[i];
            if ( pleave_register[srow] == leave_id){
                PLASSERT(leave_register[srow] == leave_id);
                PLASSERT(preg[idx]==reg[idx]);
                preg[idx++]=srow;
            }
        }
    }else if(compact_reg_leave==leave_id){
        //compact_reg is used as an optimization.
        //as it is more compact in memory then leave_register
        //we are more memory friendly.
        for(int i=0;i<length() && n> idx;i++){
            PLASSERT(ptsorted_row[i]==tsorted_row(col, i));
            RTR_type srow = ptsorted_row[i];
            if ( compact_reg[srow] ){
                PLASSERT(leave_register[srow] == leave_id);
                PLASSERT(preg[idx]==reg[idx]);
                preg[idx++]=srow;
            }
        }
    }else{
        compact_reg.resize(0);
        compact_reg.resize(length(),false);
//        for(uint i=0;i<compact_reg.size();i++)
//            compact_reg[i]=false;
        for(int i=0;i<length() && n> idx;i++){
            PLASSERT(ptsorted_row[i]==tsorted_row(col, i));
            RTR_type srow = ptsorted_row[i];
            if ( pleave_register[srow] == leave_id){
                PLASSERT(leave_register[srow] == leave_id);
                PLASSERT(preg[idx]==reg[idx]);
                preg[idx++]=srow;
                compact_reg[srow]=true;
            }
        }
        compact_reg_leave = leave_id;
    }
    PLASSERT(idx==reg->size());

}

tuple<real,real,int> RegressionTreeRegisters::bestSplitInRow(
    RTR_type_id leave_id, int col, TVec<RTR_type> &reg,
    PP<RegressionTreeLeave> left_leave,
    PP<RegressionTreeLeave> right_leave,
    Vec left_error, Vec right_error) const
{
    PLCHECK(!haveMissing());

    if(!tmp_leave){
        tmp_leave = ::PLearn::deepCopy(left_leave);
        tmp_vec.resize(left_leave->outputsize());
    }

    PLASSERT(tsource_mat.length()==tsource.length());
    getAllRegisteredRow(leave_id,col,reg);
    real * p = tsource_mat[col];
    pair<RTR_target_t,RTR_weight_t>* ptw = target_weight.data();
    RTR_type * preg = reg.data();

    int row_idx_end = reg.size() - 1;
    int prev_row=preg[row_idx_end];
    real prev_val=p[prev_row];
    PLASSERT(reg.size()>row_idx_end && row_idx_end>=0);
    PLASSERT(p[prev_row]==tsource(col,prev_row));
    //fill right_leave
    for( ;row_idx_end>0;row_idx_end--)
    {
        int futur_row = preg[row_idx_end-8];
        __builtin_prefetch(&ptw[futur_row],1,2);
        __builtin_prefetch(&p[futur_row],1,2);

        int row=prev_row;
        real val=prev_val;
        prev_row = preg[row_idx_end-1];
        prev_val = p[prev_row];

        PLASSERT(reg.size()>row_idx_end && row_idx_end>0);
        PLASSERT(target_weight.size()>row && row>=0);
        PLASSERT(p[row]==tsource(col,row));
        RTR_target_t target = ptw[row].first;
        RTR_weight_t weight = ptw[row].second;

        if(val==prev_val)
            right_leave->addRow(row, target, weight);
        else
            break;
    }

    if(col==0){//do 2 pass finding of the best split.
        //fill left_leave
        for(int row_idx = 0;row_idx<=row_idx_end;row_idx++)
        {
            int futur_row = preg[row_idx+8];
            __builtin_prefetch(&ptw[futur_row],1,2);
            
            PLASSERT(reg.size()>row_idx && row_idx>=0);
            int row=int(preg[row_idx]);
            PLASSERT(target_weight.size()>row && row>=0);
            
            RTR_target_t target = ptw[row].first;
            RTR_weight_t weight = ptw[row].second;
            left_leave->addRow(row, target, weight);
        }
        tmp_leave->initStats();
        tmp_leave->addLeave(left_leave);
        tmp_leave->addLeave(right_leave);

    }else{//do 1 pass finding of the best split.

        left_leave->initStats();
        left_leave->addLeave(tmp_leave);
        left_leave->removeLeave(right_leave);

        PLASSERT(tmp_leave->length()==left_leave->length()+right_leave->length());
        PLASSERT(fast_is_equal(tmp_leave->weights_sum,left_leave->weights_sum+right_leave->weights_sum));
        PLASSERT(fast_is_equal(tmp_leave->targets_sum,left_leave->targets_sum+right_leave->targets_sum));
        PLASSERT(fast_is_equal(tmp_leave->weighted_targets_sum,left_leave->weighted_targets_sum+right_leave->weighted_targets_sum));
        PLASSERT(fast_is_equal(tmp_leave->weighted_squared_targets_sum,
                              left_leave->weighted_squared_targets_sum+right_leave->weighted_squared_targets_sum));
    }

    //find best_split
    int best_balance=INT_MAX;
    real best_feature_value = REAL_MAX;
    real best_split_error = REAL_MAX;
    if(left_leave->length()==0)
        return make_tuple(best_feature_value, best_split_error, best_balance);

    int iter=reg.size()-right_leave->length()-1;
    RTR_type row=reg[iter];
    real first_value=p[preg[0]];
    real next_feature=p[row];


    //next_feature!=first_value is to check if their is more split point
    // in case of binary variable or variable with few different value,
    // this give a great speed up.
    for(int i=iter-1;i>=0&&next_feature!=first_value;i--)
    {
        RTR_type next_row = preg[i];
        real row_feature=next_feature;
        next_feature=p[next_row];
        
        PLASSERT(next_row!=row);

        PLASSERT((i+1)<reg.size() || row==reg[i+1]);
        PLASSERT(next_row==reg[i]);
        PLASSERT(get(next_row, col)==next_feature);
        PLASSERT(get(row, col)==row_feature);
        PLASSERT(next_feature<=row_feature);

        int futur_row = preg[i-9];
        __builtin_prefetch(&ptw[futur_row],1,2);
        __builtin_prefetch(&p[futur_row],1,2);


        real target=ptw[row].first;
        real weight=ptw[row].second;

        left_leave->removeRow(row, target, weight);
        right_leave->addRow(row, target, weight);

        row = next_row;
        if (next_feature < row_feature){
            left_leave->getOutputAndError(tmp_vec, left_error);
            right_leave->getOutputAndError(tmp_vec, right_error);
        }else
            continue;
        real work_error = left_error[0]
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
    return make_tuple(best_split_error, best_feature_value, best_balance);
}

void RegressionTreeRegisters::sortRows()
{
    next_id = 0;
    if(!do_sort_rows)
        return;
    if (tsorted_row.length() == inputsize() && tsorted_row.width() == length())
    {
        verbose("RegressionTreeRegisters: Sorted train set indices are present, no sort required", 3);
        return;
    }
    string f=source->getMetaDataDir()+"RTR_tsorted_row.psave";

    if(isUpToDate(f)){
        DBG_LOG<<"RegressionTreeRegisters:: Reloading the sorted source VMatrix: "<<f<<endl;
        PLearn::load(f,tsorted_row);
        checkMissing();
        return;
    }

    verbose("RegressionTreeRegisters: The train set is being sorted", 3);
    tsorted_row.resize(inputsize(), length());
    PP<ProgressBar> pb;
    if (report_progress)
    {
        pb = new ProgressBar("RegressionTreeRegisters : sorting the train set on input dimensions: ", inputsize());
    }
    for(int row=0;row<tsorted_row.length();row++)
        for(int col=0;col<tsorted_row.width(); col++)
            tsorted_row(row,col)=col;
            
//     for (int each_train_sample_index = 0; each_train_sample_index < length(); each_train_sample_index++)
//     {
//         sorted_row(each_train_sample_index).fill(each_train_sample_index);
//     }
#ifdef _OPENMP
#pragma omp parallel for default(none) shared(pb)
#endif
    for (int sample_dim = 0; sample_dim < inputsize(); sample_dim++)
    {
        sortEachDim(sample_dim);
        if (report_progress) pb->update(sample_dim+1);
    }
    checkMissing();
    if (report_progress) pb->close();//in case of parallel sort.
    if(source->hasMetaDataDir()){
        DBG_LOG<<"RegressionTreeRegisters:: Saving the sorted source VMatrix: "<<f<<endl;
        PLearn::save(f,tsorted_row);
    }else{
    }
}

//!check if their is missing in the input value.
void RegressionTreeRegisters::checkMissing()
{
    if(have_missing==false)
        return;
    bool found_missing=false;
    for(int j=0;j<inputsize()&&!found_missing;j++)
        for(int i=0;i<length()&&!found_missing;i++)
            if(is_missing(tsource(j,i)))
                found_missing=true;
    if(!found_missing)
        have_missing=false;
}

void RegressionTreeRegisters::sortEachDim(int dim)
{
    PLCHECK_MSG(tsource->classname()=="MemoryVMatrixNoSave",tsource->classname().c_str());
    Mat m = tsource.toMat();
    Vec v = m(dim);
    TVec<int> order = v.sortingPermutation(true, true);
    tsorted_row(dim)<<order;

#ifndef NDEBUG
    for(int i=0;i<length()-1;i++){
        int reg1 = tsorted_row(dim,i);
        int reg2 = tsorted_row(dim,i+1);
        real v1 = tsource(dim,reg1);
        real v2 = tsource(dim,reg2);
//check that the sort is valid.
        PLASSERT(v1<=v2 || is_missing(v2));
//check that the sort is stable
        if(v1==v2 && reg1>reg2)
            PLWARNING("In RegressionTreeRegisters::sortEachDim(%d) - "
                      "sort is not stable. make it stable to be more optimized:"
                      " reg1=%d, reg2=%d, v1=%f, v2=%f", 
                      dim, reg1, reg2, v1, v2);
    }
#endif
    return;

}

void RegressionTreeRegisters::printRegisters()
{
    cout << " register:  ";
    for (int ii = 0; ii < leave_register.length(); ii++) 
        cout << " " << tostring(leave_register[ii]);
    cout << endl;
}

void RegressionTreeRegisters::verbose(string the_msg, int the_level)
{
    if (verbosity >= the_level)
        cout << the_msg << endl;
}

void RegressionTreeRegisters::getExample(int i, Vec& input, Vec& target, real& weight)
{
#ifdef BOUNDCHECK
    if(inputsize_<0)
        PLERROR("In RegressionTreeRegisters::getExample, inputsize_ not defined for this vmat");
    if(targetsize_<0)
        PLERROR("In RegressionTreeRegisters::getExample, targetsize_ not defined for this vmat");
    if(weightsize()<0)
        PLERROR("In RegressionTreeRegisters::getExample, weightsize_ not defined for this vmat");
#endif
    //going by tsource is not thread safe as PP are not thread safe.
    //so we use tsource_mat.copyColumnTo that is thread safe.
    tsource_mat.copyColumnTo(i,input.data());

    target[0]=target_weight[i].first;
    weight = target_weight[i].second;
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
