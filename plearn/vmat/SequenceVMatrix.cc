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
   * $Id: SequenceVMatrix.cc,v 1.4 2004/05/18 14:18:13 lapalmej Exp $
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
  length_ = nbSeq;
  width_ = inputsize_ + targetsize_ + weightsize_;
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
  PLERROR("SequenceVMatrix doesn't handle getSubRow function");
}

void SequenceVMatrix::getExample(int i, Vec& input, Vec& target, real& weight) {
  int nrowseq = getNbRowInSeq(i);
  Mat seq = Mat(nrowseq, inputsize_ + targetsize_ + weightsize_);
  getSeq(i, seq);

  if(inputsize_<0)
    PLERROR("In SequenceVMatrix::getExample, inputsize_ not defined for this vmat");
  input.resize(inputsize_*nrowseq);             
  Mat in = seq.subMat(0, 0, nrowseq, inputsize_);
  input = in.toVecCopy();
  
  if(targetsize_<0)
    PLERROR("In SequenceVMatrix::getExample, targetsize_ not defined for this vmat");
  target.resize(targetsize_*nrowseq);
  if (targetsize_ > 0) {
    Mat t = seq.subMat(0, inputsize_, nrowseq, targetsize_);
    target = t.toVecCopy();    
  }

  if(weightsize_==0)
    weight = 1;
  else if(weightsize_<0)
    PLERROR("In SequenceVMatrix::getExample, weightsize_ not defined for this vmat");
  else if(weightsize_>1)
    PLERROR("In SequenceVMatrix::getExample, weightsize_ >1 not supported by this call");
  else
    PLERROR("In SequenceVMatrix::getExample, weightsize==1 not supported by this call");
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
  PLERROR("SequenceVMatrix doesn't handle putSubRow(i, j, v)");
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
  SequenceVMatrix *subseq = new SequenceVMatrix(l);
  for (int s = 0; s < l; s++) {
    subseq->sequences[s] = sequences[i+s].subMat(0, j, sequences[i+s].nrows(), w);
  }
  return VMat(subseq);
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

int SequenceVMatrix::getNbSeq() const
{
  return nbSeq;
}

/*
 return the number of rows of the start sequence and the length next.
*/
int SequenceVMatrix::getNbRowInSeqs(int start, int length) const
{
  int total = 0;
  for (int i = start; i < start + length; i++)
    total += ((Mat)sequences[i]).nrows();
  return total;
}

int SequenceVMatrix::getNbRowInSeq(int i) const
{
  return ((Mat)sequences[i]).nrows();
}

void SequenceVMatrix::run()
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
