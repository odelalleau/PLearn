

// -*- C++ -*-

// SequentialSplitter.cc
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
   * $Id: SequentialSplitter.cc,v 1.2 2003/05/07 05:39:18 plearner Exp $ 
   ******************************************************* */

/*! \file SequentialSplitter.cc */
#include "SequentialSplitter.h"

namespace PLearn <%
using namespace std;

SequentialSplitter::SequentialSplitter(int train_step_, int min_train_)
    : train_step(train_step_), min_train(min_train_)
{};

IMPLEMENT_NAME_AND_DEEPCOPY(SequentialSplitter);

void SequentialSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "train_step", &SequentialSplitter::train_step, OptionBase::buildoption,
                  "TODO: Comments for train_step");
    declareOption(ol, "min_train", &SequentialSplitter::min_train, OptionBase::buildoption,
                  "TODO: Comments for min_train");
    inherited::declareOptions(ol);
}

string SequentialSplitter::help()
{
    // ### Provide some useful description of what the class is ...
    return 
        "SequentialSplitter implements several splits, TODO: Comments"
        + optionHelp();
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

int SequentialSplitter::nsplits() const
{
    // Ugly...
    if (dataset.isNull())
        PLERROR("SequentialSplitter::nsplits() - Must call setDataSet()");
    int seq_length = dataset.length();
    int n_splits = 0;
    for (int t = min_train; t < seq_length; t += train_step, ++n_splits)
        ;
    cout << "SequentialSplitter::nsplits() = " << n_splits << endl;
    return n_splits;
}

Array<VMat> SequentialSplitter::getSplit(int k)
{
    int n_splits = nsplits();
    if (dataset.isNull())
        PLERROR("SequentialSplitter::getSplit() - Must call setDataSet()");
    int seq_length = dataset.length();

    if (k >= n_splits)
        PLERROR("SequentialSplitter::getSplit() - k (%d) cannot be greater than K (%d)", k, n_splits);
    if (min_train >= seq_length)
        PLERROR("SequentialSplitter::getSplit() - min_train (%d) >= dataset.length() (%d)", min_train, seq_length);

    int t = min_train + k * train_step;
    int n_test = train_step;
    if (t + n_test > seq_length)
        n_test = seq_length - t; // truncate so it fits
    
    Array<VMat> split_(2);
    split_[0] = dataset.subMatRows(0, t);
    split_[1] = dataset.subMatRows(t, n_test);
    return split_;
}


%> // end of namespace PLearn
