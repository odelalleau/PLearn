

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
   * $Id: TrainTestSplitter.cc,v 1.4 2003/05/20 15:42:12 plearner Exp $ 
   ******************************************************* */

/*! \file TrainTestSplitter.cc */
#include "TrainTestSplitter.h"

namespace PLearn <%
using namespace std;

TrainTestSplitter::TrainTestSplitter(real the_test_fraction)
  : test_fraction(the_test_fraction)
{};

IMPLEMENT_NAME_AND_DEEPCOPY(TrainTestSplitter);

void TrainTestSplitter::declareOptions(OptionList& ol)
{
  declareOption(ol, "test_fraction", &TrainTestSplitter::test_fraction, OptionBase::buildoption,
                "Defined the fraction of the dataset reserved to the test set");
  inherited::declareOptions(ol);
}

string TrainTestSplitter::help()
{
  // ### Provide some useful description of what the class is ...
  return 
    "TrainTestSplitter implements a single split of the dataset into a training-set and a test-set (the test part being the last few samples of the dataset)"
    + optionHelp();
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

Array<VMat> TrainTestSplitter::getSplit(int k)
{
  if (k)
    PLERROR("TrainTestSplitter::getSplit() - k cannot be greater than 0");
  
  Array<VMat> split_(2);
  
  int l = dataset->length();
  int test_length = int(test_fraction*l);
  int train_length = l - test_length;
  
  split_[0] = dataset.subMatRows(0, train_length);
  split_[1] = dataset.subMatRows(train_length, test_length);
  return split_;
}


%> // end of namespace PLearn
