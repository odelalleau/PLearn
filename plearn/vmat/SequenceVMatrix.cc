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
   * $Id: SequenceVMatrix.cc,v 1.6 2004/05/26 21:21:57 lapalmej Exp $
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
  init(0,0);
}

SequenceVMatrix::SequenceVMatrix(int nbSequences, int the_width):
  VMatrix(nbSequences, the_width)
{
  init(nbSequences, the_width);
}

void SequenceVMatrix::init(int nbSequences, int the_width) {
  sequences = TVec<Mat>(nbSequences);
  inputsize_ = 0;
  targetsize_ = 0;
  weightsize_ = 0;
  width_ = the_width;
  length_ = getNbSeq();
}

////////////////////
// declareOptions //
////////////////////
void SequenceVMatrix::declareOptions(OptionList &ol)
{
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
  length_ = getNbSeq();
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

void SequenceVMatrix::getExample(int i, Mat& input, Mat& target) {
  int nrowseq = getNbRowInSeq(i);
  Mat seq = Mat(nrowseq, inputsize_ + targetsize_ + weightsize_);
  getSeq(i, seq);

  if(inputsize_<0)
    PLERROR("In SequenceVMatrix::getExample, inputsize_ not defined for this vmat");
  input.resize(nrowseq, inputsize_);    
  input << seq.subMat(0, 0, nrowseq, inputsize_);
  
  if(targetsize_<0)
    PLERROR("In SequenceVMatrix::getExample, targetsize_ not defined for this vmat");
  target.resize(nrowseq, targetsize_);
  if (targetsize_ > 0)
    target << seq.subMat(0, inputsize_, nrowseq, targetsize_);
}

void SequenceVMatrix::putOrAppendSequence(int i, Mat m) {
  int w = width();
#ifdef BOUNDCHECK
  if (getNbSeq() != 0 && w != m.width())
    PLERROR("In SequenceVMatrix::putOrAppendSequence the Mat must have the same width has the other sequences");
#endif
  if (i >= getNbSeq()) {
    sequences.resize(i+1);
    length_ = i + 1;
    width_ = m.width();
  }
#ifdef BOUNDCHECK
  if (width() == 0)
    PLERROR("In SequenceVMatrix::putOrAppendSequence the VMat has a width=0");
#endif
  sequences[i] = Mat(m.length(), m.width());
  putSeq(i, m);
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
  SequenceVMatrix* subseq = new SequenceVMatrix(l, w);
  //  subseq->width_ = w;
  for (int s = 0; s < l; s++) {
    subseq->sequences[s] = Mat(sequences[i+s].nrows(), w);
    subseq->sequences[s] << sequences[i+s].subMat(0, j, sequences[i+s].nrows(), w);
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
  PLERROR("dot(int, Vec) not implemented");
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
  return sequences.size();
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

void SequenceVMatrix::print() {
  cout << "[ ";
  for (int i = 0; i < 1; i++) {
    cout << "[ ";
    for (int j = 0; j < sequences[i].nrows(); j++) {
      for (int k = 0; k < sequences[i].width(); k++) {
	cout << (sequences[i])[j][k] << " ";
      }
      cout << endl;
    }
    cout << "]" << endl;
  }
  cout << "]" << endl;
}

void SequenceVMatrix::run()
{
  Mat input, target;

  getExample(3 ,input, target);

  cout << input << endl;
  cout << endl;
  cout << target << endl;
  cout << endl;
}

SequenceVMat operator&(const SequenceVMat& s1, const SequenceVMat& s2) {
#ifdef BOUNDCHECK
  if (s1->getNbSeq() != s2->getNbSeq())
    PLERROR("In operator& : the two SequenceVMat must have the same number of sequence");
#endif
  int w1 = s1->width();
  int w2 = s2->width();
  SequenceVMat new_seq = new SequenceVMatrix(s1->getNbSeq(), w1 + w2);

  for (int i = 0; i < s1->getNbSeq(); i++) {
#ifdef BOUNDCHECK
    if (s1->getNbRowInSeq(i) != s2->getNbRowInSeq(i))
      PLERROR("In operator& : each sequence must have the same number of row");
#endif
    int nrow = s1->getNbRowInSeq(i);
    Mat m1 = Mat(nrow, w1);
    Mat m2 = Mat(nrow, w2);
    Mat m3 = Mat(nrow, w1 + w2);
    s1->getSeq(i, m1);
    s2->getSeq(i, m2);

    for (int j = 0; j < nrow; j++) {
      for (int k = 0; k < w1; k++) {
	m3[j][k] = m1[j][k];
      }
      for (int k = 0; k < w2; k++) {
	m3[j][w1+k] = m2[j][k];
      }
    }
    new_seq->putOrAppendSequence(i, m3);
  }


  return new_seq;
}

} // end of namespcae PLearn
