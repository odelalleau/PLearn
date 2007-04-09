// -*- C++ -*-

// MultiTaskSeparationSplitter.cc
//
// Copyright (C) 2006 Hugo Larochelle
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

/* *******************************************************
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file MultiTaskSeparationSplitter.cc */

#include "SelectRowsVMatrix.h"
#include "MultiTaskSeparationSplitter.h" 
#include "AddMissingVMatrix.h"
#include "GetInputVMatrix.h"
#include "MultiTargetOneHotVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    MultiTaskSeparationSplitter,
    "Splitter that  one class (test) from the examples of other classes (train)",
    "This splitter is intended to measure inductive transfer performance from some tasks to another task"
    );

MultiTaskSeparationSplitter::MultiTaskSeparationSplitter()
    :Splitter(), numsplits(-1), select_tasks_randomly(1), append_train(0), seed(-1)
{
    random_gen = new PRandom();
}

void MultiTaskSeparationSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "tasks", &MultiTaskSeparationSplitter::tasks, OptionBase::buildoption,
                  "Tasks to isolate from the others. Determines number of splits.");
    declareOption(ol, "numsplits", &MultiTaskSeparationSplitter::numsplits, OptionBase::buildoption,
                  "Number of splits. Ignored if tasks is not empty.");
    declareOption(ol, "select_tasks_randomly", &MultiTaskSeparationSplitter::select_tasks_randomly, OptionBase::buildoption,
                  "If tasks is not specified, indication that the tasks to isolate from the others should be chosen at random.\n"
                  "Otherwise, the tasks are selected by order of their index.");
    declareOption(ol, "append_train", &MultiTaskSeparationSplitter::append_train, OptionBase::buildoption,
                  "Indication that the training set should be appended to the split sets lists.");

    declareOption(ol, "seed", &MultiTaskSeparationSplitter::seed, OptionBase::buildoption,
        "Seed of random generator");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void MultiTaskSeparationSplitter::build_()
{
    if (seed != 0)
        random_gen->manual_seed(long(seed));

    if(tasks.length() == 0 && dataset)
    {
        if(numsplits <= 0) PLERROR("MultiTaskSeparationSplitter::build_(): numsplits must be > 0");

        tasks.resize(numsplits);
        int it = 0;
        for(int i=0; i<numsplits; i++)
        {
            if(select_tasks_randomly)
                random_gen->uniform_multinomial_sample(dataset->targetsize());
            else
            {
                tasks[i] = it;
                it = (it+1)%dataset->targetsize();
            }
        }
    }
}

// ### Nothing to add here, simply calls build_
void MultiTaskSeparationSplitter::build()
{
    inherited::build();
    build_();
}

void MultiTaskSeparationSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(tasks, copies);
    deepCopyField(random_gen, copies);

    //PLERROR("MultiTaskSeparationSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

int MultiTaskSeparationSplitter::nsplits() const
{
    return tasks.length();
}

int MultiTaskSeparationSplitter::nSetsPerSplit() const
{
    return append_train ? 3 : 2;
}

TVec<VMat> MultiTaskSeparationSplitter::getSplit(int k)
{

    if (k >= nsplits())
        PLERROR("MultiTaskSeparationSplitter::getSplit() - k (%d) cannot be greater than "
                " the number of splits (%d)", k, nsplits());

    int task_k = tasks[k];
    Vec row(dataset->width());
    TVec<int> miss_cols_train(0);
    miss_cols_train.push_back(dataset->inputsize()+task_k);
    TVec<int> miss_cols_test(0);
    for(int i=0; i<dataset->targetsize(); i++)
    {
        if(task_k != i)
            miss_cols_test.push_back(dataset->inputsize()+i);
    }

    TVec<VMat> split(2);
    split[0] = get_input(multi_target_one_hot(add_missing(dataset,miss_cols_train),MISSING_VALUE,MISSING_VALUE),dataset->inputsize(),dataset->targetsize());
    split[1] = get_input(multi_target_one_hot(add_missing(dataset,miss_cols_test),MISSING_VALUE,MISSING_VALUE),dataset->inputsize(),dataset->targetsize());
    if(append_train) split.append(split[0]);
    return split;
}


void MultiTaskSeparationSplitter::setDataSet(VMat the_dataset)
{
    dataset = the_dataset;
    build();
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
