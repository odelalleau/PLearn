// -*- C++ -*-

// TestInTrainSplitter.cc
//
// Copyright (C) 2004 Olivier Delalleau
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
 * $Id$
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file TestInTrainSplitter.cc */


#include "ConcatRowsVMatrix.h"      //!< For vconcat.
#include "SubVMatrix.h"
#include "TestInTrainSplitter.h"

namespace PLearn {
using namespace std;

TestInTrainSplitter::TestInTrainSplitter()
    : percentage_added(0.1)
    /* ### Initialize all fields to their default value */
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(TestInTrainSplitter,
                        "A splitter that adds the test points given by another splitter into the training set.",
                        "The underlying splitter should return train / test sets of constant size.\n"
                        "For instance, if the underlying splitter returns 3 splits of (train,test)\n"
                        "pairs with size 2000 and 500, this splitter will return:\n"
                        " - for 'percentage_added' == 5%, 15 splits of size 2100 and 100, with each\n"
                        "   test point appearing once and only once in a train set and a test set\n"
                        " - for 'percentage_added' == 20%, 6 splits of size 2400,400 and 2400,100, with\n"
                        "   each test point appearing once or more in a train set, and only once in a\n"
                        "   test set (note that the test points appearing more than once in a train set\n"
                        "   will be those at the beginning of the test sets returned by the underlying\n"
                        "   splitter)\n"
    );

void TestInTrainSplitter::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "percentage_added", &TestInTrainSplitter::percentage_added, OptionBase::buildoption,
                  "The ratio between the number of examples in the test set added to the train set and the\n"
                  "number of examples in the train set.");

    declareOption(ol, "source_splitter", &TestInTrainSplitter::source_splitter, OptionBase::buildoption,
                  "The underlying splitter.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void TestInTrainSplitter::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

void TestInTrainSplitter::build()
{
    inherited::build();
    build_();
}

void TestInTrainSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("TestInTrainSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////////
// nsplits //
/////////////
int TestInTrainSplitter::nsplits() const
{
    if (first_source_split.isEmpty()) {
        getFirstSplit(); // This is just to initialize n_train and n_test.
    }
    n_to_add = int(n_train * percentage_added + 0.5);
    if (n_to_add == 0) {
        // Do NOT add points in the train set.
        n_splits_per_source_split = 1;
        n_left = 0;
    } else {
        n_splits_per_source_split = n_test / n_to_add;
        n_left = n_test % n_to_add;
    }
    if (n_splits_per_source_split == 0) {
        PLERROR("In TestInTrainSplitter::nsplits - Asked to add more test samples than available");
    }
    if (n_left > 0)
        n_splits_per_source_split++;
    int n_total = n_splits_per_source_split * source_splitter->nsplits();
    return n_total;
}

///////////////////
// nSetsPerSplit //
///////////////////
int TestInTrainSplitter::nSetsPerSplit() const
{
    return source_splitter->nSetsPerSplit();
}

///////////////////
// getFirstSplit //
///////////////////
void TestInTrainSplitter::getFirstSplit() const {
    first_source_split = source_splitter->getSplit(0);
    n_train = first_source_split[0]->length();
    n_test = first_source_split[1]->length();
}

//////////////
// getSplit //
//////////////
TVec<VMat> TestInTrainSplitter::getSplit(int k)
{
    TVec<VMat> source_split;
    if (first_source_split.isEmpty()) {
        getFirstSplit();
    }
    if (k == 0) {
        source_split = first_source_split;
    } else {
        source_split = source_splitter->getSplit(k / n_splits_per_source_split);
    }
    int n_test_part = k % n_splits_per_source_split;
    int i_test_start = n_test_part * n_to_add;
    VMat train_set = source_split[0];
    VMat test_set = source_split[1];
    if (train_set->length() != n_train || test_set->length() != n_test) {
        PLERROR("In TestInTrainSplitter::getSplit - The train / test sizes have changed!");
    }
    TVec<VMat> result(source_split.length());
    if (n_to_add == 0) {
        // Do not change the split.
        result[0] = train_set;
        result[1] = test_set;
    } else if (n_left == 0 || n_test_part != n_splits_per_source_split - 1) {
        // Easy case: we add the same subset in train that the one for test.
        VMat added_to_train = new SubVMatrix(test_set, i_test_start, 0, n_to_add, test_set->width());
        result[0] = vconcat(train_set, added_to_train);
        result[1] = added_to_train;
    } else {
        // We also take the beginning of the test to fill added_to_train,
        // so that we add the correct number of points in the training set.
        VMat new_test = new SubVMatrix(test_set, i_test_start, 0, n_left, test_set->width());
        result[1] = new_test;
        VMat compl_for_train = new SubVMatrix(test_set, 0, 0, n_to_add - n_left, test_set->width());
        VMat added_to_train = vconcat(new_test, compl_for_train);
        result[0] = vconcat(train_set, added_to_train);
    }
    for (int i = 2; i < result.length(); i++) {
        result[i] = source_split[i];
    }
    return result;
}

////////////////
// setDataSet //
////////////////
void TestInTrainSplitter::setDataSet(VMat the_dataset) {
    first_source_split.resize(0);
    inherited::setDataSet(the_dataset);
    source_splitter->setDataSet(the_dataset);
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
