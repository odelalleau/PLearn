

// -*- C++ -*-

// TrainTestBagsSplitter.cc
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
   * $Id: TrainTestBagsSplitter.cc,v 1.1 2004/03/10 13:55:01 yoshua Exp $
   ******************************************************* */

/*! \file TrainTestBagsSplitter.cc */

#include "TrainTestBagsSplitter.h"

namespace PLearn {
using namespace std;

TrainTestBagsSplitter::TrainTestBagsSplitter(real the_test_fraction)
  : append_train(0), test_fraction(the_test_fraction)
{};

PLEARN_IMPLEMENT_OBJECT(TrainTestBagsSplitter, "ONE LINE DESCR", "NO HELP");

void TrainTestBagsSplitter::declareOptions(OptionList& ol)
{
  declareOption(ol, "append_train", &TrainTestBagsSplitter::append_train, OptionBase::buildoption,
      "if set to 1, the trainset will be appended after the test set (thus each split"
      " will contain three sets)");

  declareOption(ol, "test_fraction", &TrainTestBagsSplitter::test_fraction, OptionBase::buildoption,
                "the fraction of the dataset reserved to the test set");

  inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(TrainTestBagsSplitter,"Splits a dataset in two parts",
                        "TrainTestBagsSplitter implements a single split of the dataset into\n"
                        "a training set and a test set (the test part being the last few samples of the dataset)\n"
                        "Optionally a third set is provided which is the training set itself (in order to test on it)\n");

void TrainTestBagsSplitter::build_()
{
}

// ### Nothing to add here, simply calls build_
void TrainTestBagsSplitter::build()
{
  inherited::build();
  build_();
}

int TrainTestBagsSplitter::nsplits() const
{
  return 1; // only one split
}

int TrainTestBagsSplitter::nSetsPerSplit() const
{
  if (append_train)
    return 3;
  else
    return 2;
}

TVec<VMat> TrainTestBagsSplitter::getSplit(int k)
{
  if (k)
    PLERROR("TrainTestBagsSplitter::getSplit() - k cannot be greater than 0");
  
  TVec<VMat> split_(2);
  
  int l = dataset->length();
  int test_length = int(test_fraction*l);
  int train_length = l - test_length;
  
  split_[0] = dataset.subMatRows(0, train_length);
  split_[1] = dataset.subMatRows(train_length, test_length);
  if (append_train) {
    split_.resize(3);
    split_[2] = split_[0];
  }
  return split_;
}

} // end of namespace PLearn
