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


#ifndef SequenceVMatrix_INC
#define SequenceVMatrix_INC

#include "VMat.h"

namespace PLearn {
using namespace std;

class SequenceVMatrix;

typedef PP<SequenceVMatrix> SequenceVMat;

class SequenceVMatrix: public VMatrix
{
  typedef VMatrix inherited;

protected:
  //! Build options.
  TVec<Mat> sequences;

  Vec stdev;  // Standard deviation of each column
  Vec mean;  // Mean of each column
  Vec max;  // Max of each column

  void init(int nbSequences, int the_width);

public:
  
  SequenceVMatrix();
  SequenceVMatrix(int nbSequences, int the_width);

  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual void getMat(int i, int j, Mat m) const;
  virtual void put(int i, int j, real value);
  virtual void putSubRow(int i, int j, Vec v);
  virtual void putMat(int i, int j, Mat m);
  virtual VMat subMat(int i, int j, int l, int w);
  virtual void getExample(int i, Mat&, Mat&);

  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;

  void getRowInSeq(int i, int j, Vec v) const;
  void putRowInSeq(int i, int j, Vec v);
 
  void getSeq(int i, Mat m) const;
  void putSeq(int i, Mat m);

  int getNbSeq() const;
  int getNbRowInSeq(int i) const;
  int getNbRowInSeqs(int, int) const;

  SequenceVMat get_level(int, int);
  void clean();

  int countNonNAN(int) const;

  void putOrAppendSequence(int, Mat);

  void calc_mean();
  void calc_stdev();
  virtual void normalize_mean();

  void print();

  virtual void run();

  virtual void reset_dimensions();

  PLEARN_DECLARE_OBJECT(SequenceVMatrix);
  static void declareOptions(OptionList &ol);
  virtual void build();
  void build_();

  virtual void save_ascii(const string&) const;
  virtual void save_ascii_abc(const string&) const;

  static void copy_mat_to_arr(Mat, int, int, int, real*);
  static int nearest(real*);
};

DECLARE_OBJECT_PTR(SequenceVMatrix);


  SequenceVMat operator&(const SequenceVMat&, const SequenceVMat&);

  // === STREAM ===

class SequenceVMatrixStream: public Object
{
  typedef Object inherited;

protected:
  int col;
  int pos_seq;
  int pos_in_seq;
  TVec<int> size_;
  int nbseq_;
  SequenceVMat mat;

  Vec row;

  void internal_init();
  void move_pos();

public:
  SequenceVMatrixStream();
  
  SequenceVMatrixStream(SequenceVMat, int=0);

  void setSequence(SequenceVMat);

  bool hasMoreSequence();
  bool hasMoreInSequence();

  void init(int=0);

  void nextSeq();
  real next();

  int size(int=0) const;
  int nb_seq() const;

  PLEARN_DECLARE_OBJECT(SequenceVMatrixStream);
};

DECLARE_OBJECT_PTR(SequenceVMatrixStream);

} // end of namespcae PLearn
#endif
