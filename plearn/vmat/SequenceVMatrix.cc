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


/* *******************************************************      
   * $Id: SequenceVMatrix.cc,v 1.1 2004/05/05 21:26:01 lapalmej Exp $
   ******************************************************* */

#include "SequenceVMatrix.h"

namespace PLearn {
using namespace std;


/** SequenceVMatrix **/

PLEARN_IMPLEMENT_OBJECT(SequenceVMatrix, "ONE LINE DESCR", "NO HELP");

////////////////
// SequenceVMatrix //
////////////////
SequenceVMatrix::SequenceVMatrix()
{
  SequenceVMatrix(0);
}

SequenceVMatrix::SequenceVMatrix(int nbSequences)
  :nbSeq(nbSequences)
{
  sequences = TVec<Mat>(nbSeq);
  build_();
}

////////////////////
// declareOptions //
////////////////////
void SequenceVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "nbSeq", &SequenceVMatrix::nbSeq, OptionBase::buildoption, "Sequence number");
  declareOption(ol, "sequences", &SequenceVMatrix::sequences, OptionBase::buildoption, "Sequences Matrix");
  inherited::declareOptions(ol);
}

void SequenceVMatrix::build()
{
  inherited::build();
  build_();
}

void SequenceVMatrix::build_()
{
  
}

void SequenceVMatrix::reset_dimensions() 
{ 
}

real SequenceVMatrix::get(int i, int j) const
{
  PLERROR("Cannot access to an (i,j) element in a SequenceVMatrix");
  return 0.0;
}

void SequenceVMatrix::getSubRow(int i, int j, Vec v) const
{
  getRowInSeq(i, j, v);
}

void SequenceVMatrix::getRowInSeq(int i, int j, Vec v) const
{
  v << ((Mat)sequences[i])(j);
}

void SequenceVMatrix::getMat(int i, int j, Mat m) const
{
  PLERROR("Cannot access to an (i,j) Mat in a SequenceVMatrix");
}

void SequenceVMatrix::put(int i, int j, real value)
{
  PLERROR("Cannot put an element in (i,j) in a SequenceVMatrix");
}
void SequenceVMatrix::putSubRow(int i, int j, Vec v)
{
  putRowInSeq(i, j, v);
}

void SequenceVMatrix::putRowInSeq(int i, int j, Vec v)
{
  ((Mat)sequences[i])(j) << v;
}

void SequenceVMatrix::putMat(int i, int j, Mat m)
{
  PLERROR("Cannot put a Mat in (i,j) in a SequenceVMatrix");
}

VMat SequenceVMatrix::subMat(int i, int j, int l, int w)
{
  PLERROR("subMat not implemented");
  return ((VMat)0);
}

real SequenceVMatrix::dot(int i1, int i2, int inputsize) const
{
  PLERROR("dot(int, int, int) not implemented");
  return 0.0;
}

real SequenceVMatrix::dot(int i, const Vec& v) const
{
  PLERROR("dot(iny, Vec) not implemented");
  return 0.0;
}

void SequenceVMatrix::getSeq(int i, Mat m) const
{
  m << sequences[i];
}

void SequenceVMatrix::putSeq(int i, Mat m)
{
  sequences[i] << m;
}

void SequenceVMatrix::run()
{
  for (int i = 0; i < sequences.size(); i++) {
    real* data = sequences[i].data();
    cout << "Seq : " << i << " (size = " << sequences[i].length() << ")" << endl;
    for (int j = 0; j < sequences[i].length(); j++) {
      cout << "[" << *data;
      data++;
      for (int k = 1; k < sequences[i].width(); k++) {
	cout << "," << *data;
	data++;
      }
      cout << "]" << endl;
    }
  }

  Vec v(2);

  getRowInSeq(1,2,v);

  cout << v << endl;
}

} // end of namespcae PLearn
