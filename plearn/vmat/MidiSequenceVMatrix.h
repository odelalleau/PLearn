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


#ifndef MidiSequenceVMatrix_INC
#define MidiSequenceVMatrix_INC

#include "SequenceVMatrix.h"
#include "libmidi/MidiFile.h"

namespace PLearn {
using namespace std;
 
 typedef MidiFile* MidiFilePtr;

class MidiSequenceVMatrix: public SequenceVMatrix
{
  typedef SequenceVMatrix inherited;
protected:
  string dirname_;
  string type_;
  int track_;
  bool normalize_value_;

  int* harms;
  int* offset;
  int* lags;

  void read_meta_event(string);

  void fill_test();
  void fill_shepard();
  void fill_noteson();
  void add_seq_noteson(string);
  void add_seq_shepard(string);

  void build_target();

  real normalize(real);

public:
  
  MidiSequenceVMatrix();

  PLEARN_DECLARE_OBJECT(MidiSequenceVMatrix);


  string get_dirname() const;
  string get_type() const;
  int get_track() const;

  static void declareOptions(OptionList &ol);
  virtual void build();
  void build_();
  virtual void run();

};

DECLARE_OBJECT_PTR(MidiSequenceVMatrix);

} // end of namespace PLearn
#endif
