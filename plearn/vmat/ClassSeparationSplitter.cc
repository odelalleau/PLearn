// -*- C++ -*-

// ClassSeparationSplitter.cc
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

/*! \file ClassSeparationSplitter.cc */

#include "SelectRowsVMatrix.h"
#include "ClassSeparationSplitter.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ClassSeparationSplitter,
    "Splitter that separates examples of some classes (test) from the examples of other classes (train)",
    "This splitter is intended to measure inductive transfer performance from some tasks to another task"
    );

ClassSeparationSplitter::ClassSeparationSplitter()
    :Splitter(), numsplits(-1), nclasses(-1), nclasses_test_set(1), select_classes_randomly(1), append_train(0), seed(-1)
{
    random_gen = new PRandom();
}

void ClassSeparationSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "numsplits", &ClassSeparationSplitter::numsplits, OptionBase::buildoption,
                  "Number of splits. If <= 0, then it is set to nclasses.");
    declareOption(ol, "nclasses", &ClassSeparationSplitter::nclasses, OptionBase::buildoption,
                  "Number of classes.");
    declareOption(ol, "nclasses_test_set", &ClassSeparationSplitter::nclasses_test_set, OptionBase::buildoption,
                  "Number of classes in the test sets.");
    declareOption(ol, "classes", &ClassSeparationSplitter::classes, OptionBase::buildoption,
                  "Classes to isolate from the others, for each split. When this field is specified,\n"
                  "then nclasses, nclasses_test_set and nsplit are ignored.");
    declareOption(ol, "select_classes_randomly", &ClassSeparationSplitter::select_classes_randomly, OptionBase::buildoption,
                  "Indication that the classes should be chosen at random.\n"
                  "Otherwise, the classes are selected by order of their index.");
    declareOption(ol, "append_train", &ClassSeparationSplitter::append_train, OptionBase::buildoption,
                  "Indication that the training set should be appended to the split sets lists.");

    declareOption(ol, "seed", &ClassSeparationSplitter::seed, OptionBase::buildoption,
                  "Seed of random generator");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ClassSeparationSplitter::build_()
{
    if (seed != 0)
        random_gen->manual_seed(seed);

    if(classes.length() == 0)
    {
        if(nclasses <= 0) PLERROR("In ClassSeparationSplitter::build_(): nclasses should be > 0");
        if(nclasses_test_set <= 0) PLERROR("In ClassSeparationSplitter::build_(): nclasses_test_set should be > 0");
        if(nclasses_test_set >= nclasses) PLERROR("In ClassSeparationSplitter::build_(): nclasses_test_set should be < nclasses");
        if(numsplits <= 0) numsplits = nclasses;

        classes.resize(numsplits);
        int it = 0;
        for(int i=0; i<numsplits; i++)
        {
            if(select_classes_randomly)
            {
                classes[i].resize(nclasses);
                for(int j=0; j<nclasses; j++)
                    classes[i][j] = j;
                random_gen->shuffleElements(classes[i]);
                classes.resize(nclasses_test_set);
            }
            else
            {
                classes[i].resize(nclasses_test_set);
                for(int j=0; j<nclasses_test_set; j++)
                {
                    classes[i][j] = it%nclasses;
                    it++;
                }
            }
        }
    }
}

// ### Nothing to add here, simply calls build_
void ClassSeparationSplitter::build()
{
    inherited::build();
    build_();
}

void ClassSeparationSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(classes, copies);
    deepCopyField(random_gen, copies);

    //PLERROR("ClassSeparationSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

int ClassSeparationSplitter::nsplits() const
{
    return classes.length();
}

int ClassSeparationSplitter::nSetsPerSplit() const
{
    return append_train ? 3 : 2;
}

TVec<VMat> ClassSeparationSplitter::getSplit(int k)
{

    if (k >= nsplits())
        PLERROR("ClassSeparationSplitter::getSplit() - k (%d) cannot be greater than "
                " the number of splits (%d)", k, nsplits());

    TVec<int> classes_k = classes[k];
    Vec row(dataset->width());
    TVec<int> indices(0);
    for(int i=0; i<dataset->length(); i++)
    {
        dataset->getRow(i,row);
        if(classes_k.find((int)row[dataset->inputsize()]) >= 0)
        {
            indices.push_back(i);
        }
    }

    TVec<VMat> split(2);
    split[0] = new SelectRowsVMatrix(dataset,indices, true);
    split[1] = new SelectRowsVMatrix(dataset,indices);
    if(append_train) split.append(split[0]);
    return split;
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
