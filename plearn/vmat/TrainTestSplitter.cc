

// -*- C++ -*-

// TrainTestSplitter.cc
//
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2002 Frederic Morin
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

/*! \file TrainTestSplitter.cc */

#include "TrainTestSplitter.h"

namespace PLearn {
using namespace std;

TrainTestSplitter::TrainTestSplitter(real the_test_fraction)
    : append_train(false), test_fraction(the_test_fraction), calc_with_pct(true), test_fraction_abs(0)
{}

TrainTestSplitter::TrainTestSplitter(int the_test_fraction_abs)
    : append_train(false), test_fraction(0.0), calc_with_pct(false), test_fraction_abs(the_test_fraction_abs)
{}

PLEARN_IMPLEMENT_OBJECT(
    TrainTestSplitter,
    "Simple splitter to split between train and test sets.",
    "TrainTestSplitter implements a single split of the dataset into a\n"
    "training set and a test set (the test part being the last few samples\n"
    "of the dataset)\n"
);

void TrainTestSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "append_train", &TrainTestSplitter::append_train, OptionBase::buildoption,
                  "if set to 1, the trainset will be appended after the test set (thus each split"
                  " will contain three sets)");

    declareOption(ol, "calc_with_pct", &TrainTestSplitter::calc_with_pct, OptionBase::buildoption,
                  "Boolean value : if it's true it will compute the examples in the test set with the test_fraction value");
    declareOption(ol, "test_fraction", &TrainTestSplitter::test_fraction, OptionBase::buildoption,
                  "the fraction of the dataset reserved to the test set");
    declareOption(ol, "test_fraction_abs", &TrainTestSplitter::test_fraction_abs, OptionBase::buildoption,
                  "the number of example of the dataset reserved to the test set");

    inherited::declareOptions(ol);
}

void TrainTestSplitter::build_()
{
}

// ### Nothing to add here, simply calls build_
void TrainTestSplitter::build()
{
    inherited::build();
    build_();
}

int TrainTestSplitter::nsplits() const
{
    return 1; // only one split
}

int TrainTestSplitter::nSetsPerSplit() const
{
    if (append_train)
        return 3;
    else
        return 2;
}

TVec<VMat> TrainTestSplitter::getSplit(int k)
{
    if (k)
        PLERROR("TrainTestSplitter::getSplit() - k cannot be greater than 0");

    TVec<VMat> split_(2);

    int l = dataset->length();
    int test_length = calc_with_pct ? int(test_fraction*l) : test_fraction_abs;
    int train_length = l - test_length;

    if(train_length == l)
        split_[0] = dataset;//to get the right metadatadir when its the same matrix
    else
        split_[0] = dataset.subMatRows(0, train_length);
    if(test_length == l)
        split_[1] = dataset;//to get the right metadatadir when its the same matrix
    else
        split_[1] = dataset.subMatRows(train_length, test_length);

    if (append_train) {
        split_.resize(3);
        split_[2] = split_[0];
    }
    return split_;
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
