// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2004 Jasmin Lapalme
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


#include "DelaySequenceVMatrix.h"
#include "random.h"

namespace PLearn {
using namespace std;


/** DelaySequenceVMatrix **/

PLEARN_IMPLEMENT_OBJECT(DelaySequenceVMatrix, "ONE LINE DESCR", "NO HELP");

////////////////
// SequenceVMatrix //
////////////////
DelaySequenceVMatrix::DelaySequenceVMatrix()
  :SequenceVMatrix(200, 2)
{
  delay = 1;
}

////////////////////
// declareOptions //
////////////////////
void DelaySequenceVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "seq_length", &DelaySequenceVMatrix::seq_length, OptionBase::buildoption, "Length of each sequence");
  declareOption(ol, "delay", &DelaySequenceVMatrix::delay, OptionBase::buildoption, "The delay before the input comes to the output");
  inherited::declareOptions(ol);
}

void DelaySequenceVMatrix::build()
{
  inherited::build();
  build_();
}

void DelaySequenceVMatrix::build_()
{
  inputsize_ = 1;
  targetsize_ = 1;
  weightsize_ = 0;
  width_ = inputsize_ + targetsize_ + weightsize_;
  sequences = TVec<Mat>(length_);
  
  for (int i = 0; i < getNbSeq(); i++) {
    sequences[i] = Mat(seq_length, 2);
    for (int j = 0; j < delay; j++) {
      (sequences[i])[j][0] = uniform_sample() < 0.5 ? 0 : 1;
      (sequences[i])[j][1] = MISSING_VALUE;
    }
    for (int j = delay; j < seq_length; j++) {
      (sequences[i])[j][0] = uniform_sample() < 0.5 ? 0 : 1;
      (sequences[i])[j][1] = (sequences[i])[j-delay][0];
    }
  }
}

void DelaySequenceVMatrix::run() {}

} // end of namespcae PLearn
  
