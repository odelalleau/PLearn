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
   * $Id: SequenceVMatrix.cc,v 1.8 2004/06/28 21:19:21 lapalmej Exp $
   ******************************************************* */

#include "SequenceVMatrix.h"
#include <fstream>
#include "libmidi/Shepard.h"

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
  /*
#ifdef BOUNDCHECK
  if (width() == 0)
    PLERROR("In SequenceVMatrix::putOrAppendSequence the VMat has a width=0");
#endif
  */
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

void SequenceVMatrix::clean() {
  for (int i = 0; i < getNbSeq(); i++) {
    int j;
    for (j = 0; j < sequences[i].nrows(); j++) {
      if (is_missing((sequences[i])[j][0]))
	break;
    }
    sequences[i].resize(j, ((Mat)sequences[i]).ncols());
  }
}

int SequenceVMatrix::getNbRowInSeq(int i) const
{
  return ((Mat)sequences[i]).nrows();
}

SequenceVMat SequenceVMatrix::get_level(int n, int rep_length) {

  SequenceVMat in = dynamic_cast<SequenceVMatrix*>( (VMatrix*)subMat(0, n*rep_length, getNbSeq(), rep_length));
  SequenceVMat target = dynamic_cast<SequenceVMatrix*>( (VMatrix*)subMat(0, inputsize()+n*rep_length, getNbSeq(), rep_length));
  SequenceVMat all = in & target;
  all->defineSizes(rep_length, rep_length);
  all->clean();

  return all;

}

void SequenceVMatrix::calc_mean() {
  Vec sum = Vec(width(), 0.0);
  Vec count = Vec(width(), 0.0);
  mean = Vec(width(), 0.0);
  max = Vec(width(), 0.0);
  for (int i = 0; i < getNbSeq(); i++) {
    for (int j = 0; j < sequences[i].nrows(); j++) {
      for (int k = 0; k < sequences[i].ncols(); k++) {
	real val = (sequences[i])[j][k];
	if (!is_missing(val)) {
	  sum[k] += val;
	  if (max[k] < abs(val))
	    max[k] = abs(val);
	  count[k]++;
	}
      }
    }
  }
  cout << "Mean" << endl;
  for (int i = 0; i < width(); i++) {
    mean[i] = sum[i] / count[i];
    cout << mean[i] << " ";
  }
  cout << endl;
}

void SequenceVMatrix::calc_stdev() {
  Vec sum = Vec(width(), 0.0);
  Vec count = Vec(width(), 0.0);
  stdev = Vec(width(), 0.0);
  for (int i = 0; i < getNbSeq(); i++) {
    for (int j = 0; j < sequences[i].nrows(); j++) {
      for (int k = 0; k < sequences[i].ncols(); k++) {
	if (!is_missing((sequences[i])[j][k])) {
	  sum[k] += ((sequences[i])[j][k] - mean[k]) * ((sequences[i])[j][k] - mean[k]);
	  count[k]++;
	}
      }
    }
  }
  cout << "STDEV" << endl;
  for (int i = 0; i < width(); i++) {
    stdev[i] = sqrt(sum[i] / (count[i] - 1.0));
    cout << stdev[i] << " ";
  }
  cout << endl;
}

void SequenceVMatrix::normalize_mean() {
  calc_mean();
  calc_stdev();
  for (int i = 0; i < getNbSeq(); i++) {
    for (int j = 0; j < sequences[i].nrows(); j++) {
      for (int k = 0; k < sequences[i].ncols(); k++) {
	if (!is_missing((sequences[i])[j][k])) {
	  (sequences[i])[j][k] = ((sequences[i])[j][k] - mean[k]) / stdev[k];
	}
      }
    }
  }
}

void SequenceVMatrix::print() {
  cout << "[ ";
  for (int i = 0; i < getNbSeq(); i++) {
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
  print();
  cout << "run" << endl;
  normalize_mean();
  print();
  calc_mean();
  calc_stdev();
}

int SequenceVMatrix::countNonNAN(int c) const {
  int total = 0;
  for (int i = 0; i < getNbSeq(); i++)
    for (int j = 0; j < getNbRowInSeq(i); j++)
      if (!is_missing((sequences[i])[j][c]))
	total++;
  return total;
}

void SequenceVMatrix::save_ascii(const string& file_name) const {
  ofstream file;
  file.open(file_name.c_str());
  
  if (file.fail()) {
    PLWARNING("In SequenceVMatrix::save_ascii(string) : Unable to open file %s", file_name.c_str());
    return;
  }

  for (int i = 0; i < getNbSeq(); i++) {
    Mat m = Mat(getNbRowInSeq(i), width());
    getSeq(i, m);
    for (int j = 0; j < m.nrows(); j++) {
      for (int k = 0; k < m.ncols() - 1; k++) {
	file << m[j][k] << "\t";
      }
      if (m.ncols() > 0)
	file << m[j][m.ncols()-1];
      file << endl;
    }
    file << endl;
  }
  file.close();
}

void SequenceVMatrix::save_ascii_abc(const string& file_name) const {
  static const char * ABCName[] = {
  "z", "^C,,,,," , "D,,,,," , "^D,,,,," , "E,,,,," , "F,,,,," , "^F,,,,," , "G,,,,," , "^G,,,,," , "A,,,,," , "^A,,,,," , "B,,,,," ,
  "C,,,,", "^C,,,," , "D,,,," , "^D,,,," , "E,,,," , "F,,,," , "^F,,,," , "G,,,," , "^G,,,," , "A,,,," , "^A,,,," , "B,,,," ,
  "C,,," , "^C,,," , "D,,," , "^D,,," , "E,,," , "F,,," , "^F,,," , "G,,," , "^G,,," , "A,,," , "^A,,," , "B,,," ,
  "C,," , "^C,," , "D,," , "^D,," , "E,," , "F,," , "^F,," , "G,," , "^G,," , "A,," , "^A,," , "B,," ,
  "C," ,  "^C," ,  "D," ,  "^D," ,  "E," ,  "F," ,  "^F," ,  "G," ,  "^G," ,  "A," ,  "^A," ,  "B," ,
  "C" ,  "^C" ,  "D" ,  "^D" ,  "E" ,  "F" ,  "^F" ,  "G" ,  "^G" ,  "A" ,  "^A" ,  "B" ,
  "c" ,   "^c" ,   "d" ,   "^d" ,   "e" ,   "f" ,   "^f" ,   "g" ,   "^g" ,   "a" ,   "^a" ,   "b" ,
  "c'" ,  "^c'" ,  "d'" ,  "^d'" ,  "e'" ,  "f'" ,  "^f'" ,  "g'" ,  "^g'" ,  "a'" ,  "^a'" ,  "b'" ,
  "c''" , "^c''" , "d''" , "^d''" , "e''" , "f''" , "^f''" , "g''" , "^g''" , "a''" , "^a''" , "b''"};

  ofstream file;
  file.open(file_name.c_str());
  
  Shepard* shp = new Shepard(-1.0, 1.0);
  real arr[SHP_LEN];

  if (file.fail()) {
    PLWARNING("In SequenceVMatrix::save_ascii_abc(string) : Unable to open file %s", file_name.c_str());
    return;
  }

  for (int i = 0; i < getNbSeq(); i++) {
    Mat m = Mat(getNbRowInSeq(i), width());
    getSeq(i, m);
    for (int j = 0; j < m.nrows(); j++) {
      for (int k = 0; k < (m.ncols() / SHP_LEN) - 1; k++) {
	copy_mat_to_arr(m, j, k * SHP_LEN, SHP_LEN, arr);
	int midi = nearest(arr); // (int)m[j][k]
	if (midi == -12)
	  midi = 0;
	file << ABCName[midi] << "\t";
      }
      if (m.ncols() > 0) {
	copy_mat_to_arr(m, j, m.ncols() - SHP_LEN, SHP_LEN, arr);
	int midi = nearest(arr); // (int)m[j][(m.ncols() / SHP_LEN )-1]
	if (midi == -12)
	  midi = 0;
	file << ABCName[midi];
      }
      file << endl;
    }
    file << endl;
  }
  delete shp;
  file.close();
}

int SequenceVMatrix::nearest(real* arr) {
  
  int midi_min = -1;
  real min = REAL_MAX;
  
  Shepard* shp = new Shepard(-1.0, 1.0);

  if (is_missing(arr[0]))
    return 0;

  for (int i = shp->shpLowest; i < shp->shpHighest; i++) {
    real dist = shp->distance(arr, shp->noteAsPtr(i));
    if (dist < min) {
      min = dist;
      midi_min = i;
    }
  }
  delete shp;

  if (midi_min == -1)
    PLERROR("In SequenceVMatrix::nearest(real*) : No note was nearest than infinity");

  return midi_min;
}

void SequenceVMatrix::copy_mat_to_arr(Mat m, int nr, int nc, int length, real* arr) {
  for (int i = 0; i < length; i++) {
    arr[i] = m[nr][nc+i];
  }
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
    int min_nrow = min(s1->getNbRowInSeq(i), s2->getNbRowInSeq(i));
    int max_nrow = max(s1->getNbRowInSeq(i), s2->getNbRowInSeq(i));
    Mat m1 = Mat(s1->getNbRowInSeq(i), w1);
    Mat m2 = Mat(s2->getNbRowInSeq(i), w2);
    Mat m3 = Mat(max_nrow, w1 + w2);

    s1->getSeq(i, m1);
    s2->getSeq(i, m2);

    for (int j = 0; j < min_nrow; j++) {
      for (int k = 0; k < w1; k++) {
	m3[j][k] = m1[j][k];
      }
      for (int k = 0; k < w2; k++) {
	m3[j][w1+k] = m2[j][k];
      }
    }

    if (max_nrow == s1->getNbRowInSeq(i)) {
      for (int j = min_nrow; j < max_nrow; j++) {
	for (int k = 0; k < w1; k++) {
	  m3[j][k] = m1[j][k];
	}
	for (int k = 0; k < w2; k++) {
	  m3[j][w1+k] = MISSING_VALUE;
	}
      }
    } else {
      for (int j = min_nrow; j < max_nrow; j++) {
	for (int k = 0; k < w1; k++) {
	  m3[j][k] = MISSING_VALUE;
	}
	for (int k = 0; k < w2; k++) {
	  m3[j][w1+k] = m2[j][k];
	}
      }
    }
    new_seq->putOrAppendSequence(i, m3);
  }

  return new_seq;
}

/** SequenceVMatrixStream **/

PLEARN_IMPLEMENT_OBJECT(SequenceVMatrixStream, "ONE LINE DESCR", "NO HELP");

SequenceVMatrixStream::SequenceVMatrixStream() {
  mat = 0;
  col = -1;
  pos_seq = -1;
  pos_in_seq = -1;
}

SequenceVMatrixStream::SequenceVMatrixStream(SequenceVMat m, int c) {
  col = c;
  setSequence(m);
  size_ = TVec<int>(mat->width());
  for (int i = 0; i < mat->width(); i++)
    size_[i] = mat->countNonNAN(i);
}

void SequenceVMatrixStream::internal_init() {
  pos_seq = 0;
  row = Vec(mat->width());  
  nbseq_ = mat->getNbSeq();
  pos_in_seq = -1;
  move_pos();
}

void SequenceVMatrixStream::init(int c) {
  col = c;
  internal_init();
}

void SequenceVMatrixStream::setSequence(SequenceVMat m) {
  mat = m;
  internal_init();
}

bool SequenceVMatrixStream::hasMoreSequence() {
  if (mat == 0)
    PLERROR("In SequenceVMatrixStream::hasMoreSequence(), no SequenceVMatrix specified");

  //  cout << "SequenceVMatrixStream::hasMoreSequence() : " << pos_seq << "<" << mat->getNbSeq() << endl;
  return pos_seq < mat->getNbSeq();
}

bool SequenceVMatrixStream::hasMoreInSequence() {
  if (mat == 0)
    PLERROR("In SequenceVMatrixStream::hasMoreInSeq(), no SequenceVMatrix specified");

  //  cout << "SequenceVMatrixStream::hasMoreInSequence()" << pos_in_seq << "<" <<  mat->getNbRowInSeq(pos_seq) << endl;
  if (hasMoreSequence())
    return pos_in_seq < mat->getNbRowInSeq(pos_seq);
  return false;
}

void SequenceVMatrixStream::nextSeq() {
#ifdef BOUNDCHECK
  if (!hasMoreSequence())
    PLERROR("In SequenceVMatrixStream::nextSeq(), no more sequence available");
#endif
  pos_seq++;
  if (hasMoreSequence()) {
    pos_in_seq = -1;
    move_pos();
  }
}

real SequenceVMatrixStream::next() {
#ifdef BOUNDCHECK
  if (!hasMoreSequence())
    PLERROR("In SequenceVMatrixStream::next(), no more sequence available");
  if (!hasMoreInSequence())
    PLERROR("In SequenceVMatrixStream::next(), no more data in the sequence available");  
#endif

  mat->getRowInSeq(pos_seq, pos_in_seq, row);
  real val = row[col];
  move_pos();

  return val;
}

void SequenceVMatrixStream::move_pos() {
  do {
    pos_in_seq++;
    if (pos_in_seq < mat->getNbRowInSeq(pos_seq))
      mat->getRowInSeq(pos_seq, pos_in_seq, row);
    else
      break;
  } while (is_missing(row[col]));  
}

int SequenceVMatrixStream::size(int n) const {
  return size_[n];
}

int SequenceVMatrixStream::nb_seq() const {
  return nbseq_;
}

} // end of namespace PLearn
