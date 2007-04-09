// -*- C++ -*-

// TrainValidTestSplitter.cc
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

/*! \file TrainValidTestSplitter.cc */

#include "ConcatRowsVMatrix.h"
#include <plearn/math/random.h>
#include "SelectRowsVMatrix.h"
#include "SubVMatrix.h"
#include "TrainValidTestSplitter.h"

namespace PLearn {
using namespace std;

////////////////////////////
// TrainValidTestSplitter //
////////////////////////////
TrainValidTestSplitter::TrainValidTestSplitter()
    : Splitter(),
      append_train(0),
      append_valid(0),
      n_splits(1),
      n_train(-1),
      n_valid(-1),
      shuffle_valid_and_test(1)
{}

PLEARN_IMPLEMENT_OBJECT(TrainValidTestSplitter,
                        "This splitter will basically return [Train+Valid, Test].",
                        "The train test returned by the splitter is formed from the first n_train+n_valid\n"
                        "samples in the dataset. The other samples are returned in the test set.\n"
                        "The validation and test sets (given by the samples after the n_train-th one) can\n"
                        "be shuffled in order to get a different validation and test sets at each split.\n"
                        "However, the train set (the first n_train samples) remains fixed.");

////////////////////
// declareOptions //
////////////////////
void TrainValidTestSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "append_train", &TrainValidTestSplitter::append_train, OptionBase::buildoption,
                  "If set to 1, the train set will be appended to each split, after the test set\n"
                  "(the train set means the first n_train samples).");

    declareOption(ol, "append_valid", &TrainValidTestSplitter::append_valid, OptionBase::buildoption,
                  "If set to 1, the validation set will be appended to each split, after the test set\n"
                  "(or the train set if append_train is also set to 1).");

    declareOption(ol, "n_splits", &TrainValidTestSplitter::n_splits, OptionBase::buildoption,
                  "The number of splits we want (a value > 1 is useful with shuffle_valid_and_test = 1).");

    declareOption(ol, "n_train", &TrainValidTestSplitter::n_train, OptionBase::buildoption,
                  "The number of samples that define the train set, assumed to be at the beginning\n"
                  "of the dataset.");

    declareOption(ol, "n_valid", &TrainValidTestSplitter::n_valid, OptionBase::buildoption,
                  "The number of samples that define the validation set (they are taken among\n"
                  "the samples after the n_train first ones).");

    declareOption(ol, "shuffle_valid_and_test", &TrainValidTestSplitter::shuffle_valid_and_test, OptionBase::buildoption,
                  "If set to 1, then the part of the dataset after the first n_train ones will\n"
                  "be shuffled before taking the validation and test sets. Note that if you want\n"
                  "to set it to 0, then using a TrainTestSplitter is probably more appropriate.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void TrainValidTestSplitter::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void TrainValidTestSplitter::build_()
{
    if (dataset) {
        if (n_train < 0 || n_valid < 0) {
            PLERROR("In TrainValidTestSplitter::build_ - Please initialize correctly 'n_train' and 'n_valid'");
        }
        int n = dataset->length();
        int n_test = n - n_train - n_valid;
        // Define the train set.
        train_set = new SubVMatrix(dataset, 0, 0, n_train, dataset->width());
        // Precompute all the indices.
        valid_indices.resize(n_splits, n_valid);
        test_indices.resize(n_splits, n_test);
        TVec<int> valid_and_test_indices(n_valid + n_test);
        for (int i = 0; i < n_splits; i++) {
            for (int j = 0; j < n_valid + n_test; j++) {
                valid_and_test_indices[j] = j + n_train;
            }
            if (shuffle_valid_and_test) {
                shuffleElements(valid_and_test_indices);
            }
            valid_indices(i) << valid_and_test_indices.subVec(0, n_valid);
            test_indices(i) << valid_and_test_indices.subVec(n_valid, n_test);
            if (shuffle_valid_and_test) {
                // Now sort the indices for (hopefully) faster access.
                sortElements(valid_indices(i));
                sortElements(test_indices(i));
            }
        }
    }
}

//////////////
// getSplit //
//////////////
TVec<VMat> TrainValidTestSplitter::getSplit(int k)
{
    // ### Build and return the kth split .
    TVec<VMat> result(2);
    VMat valid_set = new SelectRowsVMatrix(dataset, valid_indices(k));
    result[0] = vconcat(train_set, valid_set);
    result[1] = new SelectRowsVMatrix(dataset, test_indices(k));
    if (append_train) {
        result.append(train_set);
    }
    if (append_valid) {
        result.append(valid_set);
    }
    return result;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void TrainValidTestSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("TrainValidTestSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////////
// nsplits //
/////////////
int TrainValidTestSplitter::nsplits() const
{
    // ### Return the number of available splits
    return this->n_splits;
}

///////////////////
// nSetsPerSplit //
///////////////////
int TrainValidTestSplitter::nSetsPerSplit() const
{
    // ### Return the number of sets per split
    int result = 2;
    if (append_train) {
        result++;
    }
    if (append_valid) {
        result++;
    }
    return result;
}

////////////////
// setDataSet //
////////////////
void TrainValidTestSplitter::setDataSet(VMat the_dataset) {
    inherited::setDataSet(the_dataset);
    build_(); // To recompute the indices.
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
