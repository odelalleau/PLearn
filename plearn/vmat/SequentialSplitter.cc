// -*- C++ -*-

// SequentialSplitter.cc
//
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2002 Frederic Morin
// Copyright (C) 2004 Rejean Ducharme
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
#include "SequentialSplitter.h"

namespace PLearn {
using namespace std;

SequentialSplitter::SequentialSplitter(int horizon_, int init_train_size_, bool return_entire_vmat_)
    : horizon(horizon_), init_train_size(init_train_size_), return_entire_vmat(return_entire_vmat_)
{}

PLEARN_IMPLEMENT_OBJECT(SequentialSplitter, "ONE LINE DESCR",
                        "SequentialSplitter implements several splits, TODO: Comments");

void SequentialSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "horizon", &SequentialSplitter::horizon, OptionBase::buildoption,
                  "How far in the future is the test set (split[1])");

    declareOption(ol, "init_train_size", &SequentialSplitter::init_train_size, OptionBase::buildoption,
                  "Initial length of the train set (split[0])");

    declareOption(ol, "return_entire_vmat", &SequentialSplitter::return_entire_vmat, OptionBase::buildoption,
                  "If true, the test split (split[1]) will start at t=0.");

    inherited::declareOptions(ol);
}

void SequentialSplitter::build_()
{
}

// ### Nothing to add here, simply calls build_
void SequentialSplitter::build()
{
    inherited::build();
    build_();
}

int SequentialSplitter::nSetsPerSplit() const
{
    return 2;
}

int SequentialSplitter::nsplits() const
{
    if (dataset.isNull())
        PLERROR("SequentialSplitter::nsplits() - Must call setDataSet()");
    if (init_train_size < 1)
        PLERROR("SequentialSplitter::nsplits() - init_train_size must be stricktly positive (%d)", init_train_size);
    if (horizon < 1)
        PLERROR("SequentialSplitter::nsplits() - horizon must be stricktly positive (%d)", horizon);

    return dataset.length() - init_train_size - horizon + 1;
}

TVec<VMat> SequentialSplitter::getSplit(int k)
{
    if (dataset.isNull())
        PLERROR("SequentialSplitter::getSplit() - Must call setDataSet()");

    int n_splits = nsplits();
    if (k >= n_splits)
        PLERROR("SequentialSplitter::getSplit() - k (%d) cannot be greater than K (%d)", k, n_splits);

    int seq_length = dataset.length();
    if (init_train_size >= seq_length)
        PLERROR("SequentialSplitter::getSplit() - init_train_size (%d) >= dataset.length() (%d)", init_train_size, seq_length);

    int t = init_train_size + k;
    int start_test_t = return_entire_vmat ? 0 : t;
    int n_test = t + horizon - start_test_t;

    TVec<VMat> split_(2);
    split_[0] = dataset.subMatRows(0, t);
    split_[1] = dataset.subMatRows(start_test_t, n_test);

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
