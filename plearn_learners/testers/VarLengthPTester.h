// -*- C++ -*-

// VarLengthPTester.h
// 
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

#ifndef VarLengthPTester_INC
#define VarLengthPTester_INC

#include "PTester.h"
#include "SequenceVMatrix.h"

namespace PLearn {
using namespace std;

/*
This class is the Tester for neural network with sequences.
We cannot do it with a simple PTester because the output of each sequence
is variable. 
*/

class VarLengthPTester: public PTester
{    
public:

  typedef PTester inherited;

  // ************************
  // * public build options *
  // ************************

  // Default constructor
  VarLengthPTester();

  // ******************
  // * Object methods *
  // ******************

private: 
  void build_();

protected: 
  static void declareOptions(OptionList& ol);

public:
  virtual void build();

  PLEARN_DECLARE_OBJECT(VarLengthPTester);

  virtual void run();

  Vec perform(bool call_forget=true);

  void save(SequenceVMat, SequenceVMat, SequenceVMat, int);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(VarLengthPTester);
  
} // end of namespace PLearn

#endif
