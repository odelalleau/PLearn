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
   * $Id: KFoldSplitter.cc,v 1.7 2003/11/27 14:43:02 tihocan Exp $ 
   ******************************************************* */

/*! \file SequentialSplitter.cc */
#include "KFoldSplitter.h"
#include "VMat_maths.h"

namespace PLearn <%
using namespace std;

KFoldSplitter::KFoldSplitter(int k)
    : K(k),append_train(0)
{};

PLEARN_IMPLEMENT_OBJECT(KFoldSplitter, "ONE LINE DESCR", "NO HELP");

void KFoldSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "K", &KFoldSplitter::K, OptionBase::buildoption,
                  "split dataset in K parts");

    declareOption(ol, "append_train", &KFoldSplitter::append_train, OptionBase::buildoption,
                  "if set to 1, the trainset will be appended after the test set (thus each split"
                  " will contain three sets");

    inherited::declareOptions(ol);
}

string KFoldSplitter::help()
{
    // ### Provide some useful description of what the class is ...
    return 
        "KFoldSplitter implements K splits of the dataset into a training-set and a test-set"
        + optionHelp();
}

void KFoldSplitter::build_()
{
}

// ### Nothing to add here, simply calls build_
void KFoldSplitter::build()
{
    inherited::build();
    build_();
}

int KFoldSplitter::nsplits() const
{
    return K;
}

int KFoldSplitter::nSetsPerSplit() const
{
  if (append_train)
    return 3;
  else
    return 2;
}

TVec<VMat> KFoldSplitter::getSplit(int k)
{
    if (k >= K)
        PLERROR("KFoldSplitter::getSplit() - k (%d) cannot be greater than K (%d)", k, K);

    int n_data = dataset->length();
    real test_fraction = K > 0 ? (n_data/(real)K) : 0;
    if ((int)(test_fraction) < 1)
        test_fraction = 1; // leave-one-out cross-validation

    TVec<VMat> split_(2);
    split(dataset, test_fraction, split_[0], split_[1], k);
    if (append_train) {
      split_.resize(3);
      split_[2] = split_[0];
    }
    return split_;
}

%> // end of namespace PLearn
