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


#include "XORSequenceVMatrix.h"
#include "random.h"

namespace PLearn {
using namespace std;


/** XORSequenceVMatrix **/

PLEARN_IMPLEMENT_OBJECT(XORSequenceVMatrix, "ONE LINE DESCR", "NO HELP");

////////////////
// SequenceVMatrix //
////////////////
XORSequenceVMatrix::XORSequenceVMatrix()
  :SequenceVMatrix(200)
{
  seq_length = 2;  
}

////////////////////
// declareOptions //
////////////////////
void XORSequenceVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "seq_length", &XORSequenceVMatrix::seq_length, OptionBase::buildoption, "Length of each sequence");
  inherited::declareOptions(ol);
}

void XORSequenceVMatrix::build()
{
  inherited::build();
  build_();
}

void XORSequenceVMatrix::build_()
{
  inputsize_ = 1;
  targetsize_ = 1;
  weightsize_ = 0;
  length_ = nbSeq;
  width_ = inputsize_ + targetsize_ + weightsize_;
  sequences = TVec<Mat>(nbSeq);
  
  for (int i = 0; i < nbSeq; i++) {
    sequences[i] = Mat(seq_length, 2);
    int last = uniform_sample() < 0.5 ? 0 : 1;
    (sequences[i])[0][0] = last;
    (sequences[i])[0][1] = MISSING_VALUE;
    for (int j = 1; j < seq_length; j++) {
      int cour = uniform_sample() < 0.5 ? 0 : 1;
      int xo = last == cour ? 0 : 1;
      (sequences[i])[j][0] = cour;
      (sequences[i])[j][1] = xo;
      last = cour;
    }
  }
}

void XORSequenceVMatrix::run()
{
  Vec input, target;
  real weight;

  getExample(3 ,input, target, weight);

  cout << input << endl;
  cout << endl;
  cout << target << endl;
  cout << endl;
}

} // end of namespcae PLearn
