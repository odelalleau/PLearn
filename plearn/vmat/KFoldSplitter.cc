// -*- C++ -*-

// KFoldSplitter.cc
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

/*! \file SequentialSplitter.cc */

#include "KFoldSplitter.h"
#include "VMat_basic_stats.h"
#include <plearn/vmat/ConcatRowsVMatrix.h>
#include <plearn/vmat/SubVMatrix.h>

namespace PLearn {
using namespace std;

KFoldSplitter::KFoldSplitter(int k)
    : append_non_constant_test(false),
      append_train(false),
      include_test_in_train(false),
      K(k)
{
    // Default cross-validation range is the whole dataset.
    cross_range.first   = 0;
    cross_range.second  = 1;
}

PLEARN_IMPLEMENT_OBJECT(KFoldSplitter,
                        "K-fold cross-validation splitter.",
                        "KFoldSplitter implements K splits of the dataset into a training-set and a test-set.\n"
                        "To perform leave-one-out cross-validation, K must be set to -1 (or, obviously, to the\n"
                        "exact number of examples).\n"
                        "The cross-validation may be performed only on a subset of the source data, using the option\n"
                        "'cross_range', that will define a range of samples on which to perform cross-validation.\n"
                        "All samples before this range will systematically be added to the train set, while all samples\n"
                        "after this range will be added to the test set.\n"
    );

void KFoldSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "K", &KFoldSplitter::K, OptionBase::buildoption,
                  "Split dataset in K parts (you can use K = -1 to perform leave-one-out CV).");

    declareOption(ol, "append_train", &KFoldSplitter::append_train, OptionBase::buildoption,
                  "If set to 1, the trainset will be appended after in the returned sets.");

    declareOption(ol, "append_non_constant_test", &KFoldSplitter::append_non_constant_test, OptionBase::buildoption,
                  "If set to 1, the non-constant part of the test set will be appended in the returned sets.");

    declareOption(ol, "include_test_in_train", &KFoldSplitter::include_test_in_train, OptionBase::buildoption,
                  "If set to 1, the test set will be included in the train set.");

    declareOption(ol, "cross_range", &KFoldSplitter::cross_range, OptionBase::buildoption,
                  "The range on which cross-validation is applied (similar to the FractionSplitter ranges).");

    inherited::declareOptions(ol);
}

void KFoldSplitter::build_()
{
    PLASSERT( K > 0 || K == -1 );
}

void KFoldSplitter::build()
{
    inherited::build();
    build_();
}

int KFoldSplitter::nsplits() const
{
    return K > 0 ? K
                 : dataset ? dataset->length()
                           : -1;
}

int KFoldSplitter::nSetsPerSplit() const
{
    int nsets = 2;
    if (append_train)
        nsets++;
    if (append_non_constant_test)
        nsets++;
    return nsets;
}

TVec<VMat> KFoldSplitter::getSplit(int k)
{
    if (k >= nsplits())
        PLERROR("KFoldSplitter::getSplit() - k (%d) cannot be greater than "
                " the number of splits (%d)", k, nsplits());

    real start = cross_range.first;
    real end   = cross_range.second;
    int n_data = dataset->length();
    PLASSERT( start >= 0 && end >= 0 && end > start && start < n_data && end < n_data );
    int i_start = 0;
    if (start > 0)
        i_start = start >= 1 ? int(round(start)) : int(round(n_data * start));
    int i_end = n_data;
    if (!fast_exact_is_equal(end, 1))
        i_end   = end   >= 1 ? int(round(end))   : int(round(n_data * end));
    // The cross validation will be done only on examples i_start, ..., i_end - 1.
    int n_cross_data = i_end - i_start;
    bool do_partial_cross = (n_cross_data != n_data);
    if (K > 0 && K > n_data)
        PLERROR("In KFoldSplitter::getSplit - The number of splits (%d) cannot"
                " be greater than the number of samples in the dataset (%d). "
                "If you want to perform leave-one-out cross-validation, please"
                "set K = -1", K, n_data);
    real test_fraction = K > 0 ? (n_cross_data/(real)K) : 0;
    if ((int)(test_fraction) < 1)
        test_fraction = 1; // leave-one-out cross-validation

    TVec<VMat> split_(2);
    VMat non_constant_test;
    if (do_partial_cross) {
        VMat sub_data = new SubVMatrix(dataset, i_start, 0, n_cross_data, dataset->width());
        split(sub_data, test_fraction, split_[0], split_[1], k, true);
        non_constant_test = split_[1];
        if (i_start > 0) {
            VMat constant_train = new SubVMatrix(dataset, 0, 0, i_start, dataset->width());
            split_[0] = new ConcatRowsVMatrix(constant_train, split_[0]);
        }
        if (i_end < n_data) {
            VMat constant_test = new SubVMatrix(dataset, i_end, 0, n_data - i_end, dataset->width());
            split_[1] = new ConcatRowsVMatrix(split_[1], constant_test);
        }
    } else {
        split(dataset, test_fraction, split_[0], split_[1], k, true);
        non_constant_test = split_[1];
    }
    if (include_test_in_train)
        split_[0] = new ConcatRowsVMatrix(split_[0], split_[1]);
    if (append_train)
        split_.append(split_[0]);
    if (append_non_constant_test)
        split_.append(non_constant_test);
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
