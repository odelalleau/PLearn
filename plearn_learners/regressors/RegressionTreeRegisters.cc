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

RegressionTreeRegisters::RegressionTreeRegisters()
{
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
    declareOption(ol, "source", &RegressionTreeRegisters::source, OptionBase::buildoption,
                  "The source VMatrix");

    declareOption(ol, "next_id", &RegressionTreeRegisters::next_id, OptionBase::learntoption,
                  "The next id for creating a new leave\n");
    declareOption(ol, "length", &RegressionTreeRegisters::length_, OptionBase::learntoption,
                  "The length of the train set\n");
    declareOption(ol, "width", &RegressionTreeRegisters::width_, OptionBase::learntoption,
                  "The width of the train set\n");
    declareOption(ol, "inputsize", &RegressionTreeRegisters::inputsize_, OptionBase::learntoption,
                  "The input size of the train set\n");
    declareOption(ol, "targetsize", &RegressionTreeRegisters::targetsize_, OptionBase::learntoption,
                  "The target size of the train set\n");
    declareOption(ol, "weightsize", &RegressionTreeRegisters::weightsize_, OptionBase::learntoption,
                  "The weight of each sample in the train set\n");
    declareOption(ol, "leave_register", &RegressionTreeRegisters::leave_register, OptionBase::learntoption,
                  "The vector identifying the leave to which, each row belongs\n");
    declareOption(ol, "leave_candidate", &RegressionTreeRegisters::leave_candidate, OptionBase::learntoption,
                  "The vector identifying the candidate leave to which, each row could belong after split\n");

    //too big to save
    declareOption(ol, "sorted_row", &RegressionTreeRegisters::sorted_row, OptionBase::nosave,
                  "The matrix holding the sequence of samples in ascending value order for each dimension\n");
    //too big to save
    declareOption(ol, "inverted_sorted_row", &RegressionTreeRegisters::inverted_sorted_row, OptionBase::nosave,
                  "The matrix holding the position of a given entry in the sorted row matrix\n");

    inherited::declareOptions(ol);
}

void RegressionTreeRegisters::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(sorted_row, copies);
    deepCopyField(inverted_sorted_row, copies);
    deepCopyField(leave_register, copies);
    deepCopyField(leave_candidate, copies);
}

void RegressionTreeRegisters::build()
{
    inherited::build();
    build_();
}

void RegressionTreeRegisters::build_()
{
}

void RegressionTreeRegisters::initRegisters(VMat the_train_set)
{
    source = the_train_set;
    length_ = source->length();
    width_ = source->width();
    inputsize_ = source->inputsize();
    targetsize_ = source->targetsize();
    weightsize_ = source->weightsize();
    leave_register.resize(length());
    leave_candidate.resize(length());
    sortRows();
}

void RegressionTreeRegisters::reinitRegisters()
{
    next_id = 0;

    //in case we don't save the sorted data
    sortRows();
}

void RegressionTreeRegisters::applyForRow(int leave_id, int row)
{
    leave_candidate[row] = leave_id;
}

void RegressionTreeRegisters::registerLeave(int leave_id, int row)
{
    leave_register[row] = leave_id;
}

real RegressionTreeRegisters::get(int i, int j) const
{
    return source->get(i,j);
}

real RegressionTreeRegisters::getTarget(int row)
{
    return source->get(row, inputsize());
}

real RegressionTreeRegisters::getWeight(int row)
{
    if (weightsize() <= 0) return 1.0 / length();
    else return source->get(row, inputsize() + targetsize() );
}

void RegressionTreeRegisters::setWeight(int row, real val)
{
    PLASSERT(weightsize() > 0);
    source->put(row, inputsize() + targetsize(), val );
}

int RegressionTreeRegisters::getNextId()
{
    next_id += 1;
    return next_id;
}

int RegressionTreeRegisters::getNextRegisteredRow(int leave_id, int col, int previous_row)
{
    if (previous_row >= length()) return length();
    int each_train_sample_index;
    if (previous_row < 0) each_train_sample_index = 0;
    else each_train_sample_index = inverted_sorted_row(previous_row, col) + 1;
    while (true)
    {
        if (each_train_sample_index >= length()) return length();
        if (leave_register[sorted_row(each_train_sample_index, col)] == leave_id) break;
        each_train_sample_index += 1;
    }
    return sorted_row(each_train_sample_index, col);
}

int RegressionTreeRegisters::getNextCandidateRow(int leave_id, int col, int previous_row)
{
    if (previous_row >= length()) return length();
    int each_train_sample_index;
    if (previous_row < 0) each_train_sample_index = 0;
    else each_train_sample_index = inverted_sorted_row(previous_row, col) + 1;
    while (true)
    {
        if (each_train_sample_index >= length()) return length();
        if (leave_candidate[sorted_row(each_train_sample_index, col)] == leave_id) break;
        each_train_sample_index += 1;
    }
    return sorted_row(each_train_sample_index, col);
}

void RegressionTreeRegisters::sortRows()
{
    next_id = 0;
    if (sorted_row.length() == length() && sorted_row.width() == inputsize())
    {
        verbose("RegressionTreeRegisters: Sorted train set indices are present, no sort required", 3);
        return;
    }
    verbose("RegressionTreeRegisters: The train set is being sorted", 3);
    sorted_row.resize(length(), inputsize());
    PP<ProgressBar> pb;
    if (report_progress)
    {
        pb = new ProgressBar("RegressionTreeRegisters : sorting the train set on input dimensions: ", inputsize());
    }
    for (int each_train_sample_index = 0; each_train_sample_index < length(); each_train_sample_index++)
    {
        sorted_row(each_train_sample_index).fill(each_train_sample_index);
    }
    for (int sample_dim = 0; sample_dim < inputsize(); sample_dim++)
    {
        sortEachDim(sample_dim);
        if (report_progress) pb->update(sample_dim);
    }
    inverted_sorted_row.resize(length(), inputsize());
    for (int each_train_sample_index = 0; each_train_sample_index < length(); each_train_sample_index++)
    {
        for (int sample_dim = 0; sample_dim < inputsize(); sample_dim++)
        {
            inverted_sorted_row(sorted_row(each_train_sample_index, sample_dim), sample_dim) = each_train_sample_index;
        }
    }  
}
  
void RegressionTreeRegisters::sortEachDim(int dim)
{
    int start_index = 0;
    int end_index = length() - 1;
    int forward_index;
    int backward_index;
    int stack_index = -1;
    TVec<int> stack(50);
    for (;;)
    {
        if ((end_index - start_index) < 7)
        {
            if (end_index > start_index)
            {
                sortSmallSubArray(start_index, end_index, dim);
            }
            if (stack_index < 0)
            {
                break;
            }
            end_index = stack[stack_index--];
            start_index = stack[stack_index--];
        }
        else
        {
            swapIndex(start_index + 1, (start_index + end_index) / 2, dim);
            if (compare(source->get(sorted_row(start_index, dim), dim),
                        source->get(sorted_row(end_index, dim), dim)) > 0.0)
                swapIndex(start_index, end_index, dim);
            if (compare(source->get(sorted_row(start_index + 1, dim), dim),
                        source->get(sorted_row(end_index, dim), dim)) > 0.0)
                swapIndex(start_index + 1, end_index, dim);
            if (compare(source->get(sorted_row(start_index, dim), dim),
                        source->get(sorted_row(start_index + 1, dim), dim)) > 0.0)
                swapIndex(start_index, start_index + 1, dim);
            forward_index = start_index + 1;
            backward_index = end_index;
            real sample_feature = source->get(sorted_row(start_index + 1, dim), dim);
            for (;;)
            {
                do forward_index++; while (compare(source->get(sorted_row(forward_index, dim), dim), sample_feature) < 0.0);
                do backward_index--; while (compare(source->get(sorted_row(backward_index, dim), dim), sample_feature) > 0.0);
                if (backward_index < forward_index)
                {
                    break;
                }
                swapIndex(forward_index, backward_index, dim);
            }
            swapIndex(start_index + 1, backward_index, dim);
            stack_index += 2;
            if (stack_index > 50)
                PLERROR("RegressionTreeRegistersVMatrix: the stack for sorting the rows is too small");
            if ((end_index - forward_index + 1) >= (backward_index - start_index))
            {
                stack[stack_index] = end_index;
                stack[stack_index - 1] = forward_index;
                end_index = backward_index - 1;
            }
            else
            {
                stack[stack_index] = backward_index - 1;
                stack[stack_index - 1] = start_index;
                start_index = forward_index;
            }
        }
    }
}
  
void RegressionTreeRegisters::sortSmallSubArray(int the_start_index, int the_end_index, int dim)
{
    for (int next_train_sample_index = the_start_index + 1;
         next_train_sample_index <= the_end_index;
         next_train_sample_index++)
    {
        int saved_index = sorted_row(next_train_sample_index, dim);
        real sample_feature = source->get(saved_index, dim);
        int each_train_sample_index;
        for (each_train_sample_index = next_train_sample_index - 1;
             each_train_sample_index >= the_start_index;
             each_train_sample_index--)
        {
            if (compare(source->get(sorted_row(each_train_sample_index, dim), dim), sample_feature) <= 0.0)
            {
                break;
            }
            sorted_row(each_train_sample_index + 1, dim) = sorted_row(each_train_sample_index, dim);
        }
        sorted_row(each_train_sample_index + 1, dim) = saved_index;
    }  
}

void RegressionTreeRegisters::swapIndex(int index_i, int index_j, int dim)
{
    int saved_index = sorted_row(index_i, dim);
    sorted_row(index_i, dim) = sorted_row(index_j, dim);
    sorted_row(index_j, dim) = saved_index;
}

real RegressionTreeRegisters::compare(real field1, real field2)
{
    if (is_missing(field1) && is_missing(field2)) return 0.0;
    if (is_missing(field1)) return -1.0;
    if (is_missing(field2)) return +1.0;
    return field1 - field2;
}

void RegressionTreeRegisters::printRegisters()
{
    cout << "candidate: ";
    for (int ii = 0; ii < length(); ii++) cout << " " << tostring(leave_candidate[ii]);
    cout << " register:  ";
    for (int ii = 0; ii < length(); ii++) cout << " " << tostring(leave_register[ii]);
    cout << endl;
}

void RegressionTreeRegisters::verbose(string the_msg, int the_level)
{
    if (verbosity >= the_level)
        cout << the_msg << endl;
}

void RegressionTreeRegisters::getExample(int i, Vec& input, Vec& target, real& weight)
{
    if(inputsize_<0)
        PLERROR("In VMatrix::getExample, inputsize_ not defined for this vmat");
    input.resize(inputsize_);
    source->getSubRow(i,0,input);
    if(targetsize_<0)
        PLERROR("In VMatrix::getExample, targetsize_ not defined for this vmat");
    target.resize(targetsize_);
    if (targetsize_ > 0) {
        source->getSubRow(i,inputsize_,target);
    }

    if(weightsize()==0)
        weight = 1;
    else if(weightsize()<0)
        PLERROR("In VMatrix::getExample, weightsize_ not defined for this vmat");
    else if(weightsize()>1)
        PLERROR("In VMatrix::getExample, weightsize_ >1 not supported by this call");
    else
        weight = source->get(i,inputsize()+targetsize());
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
