// -*- C++ -*-

// SubInputVMatrix.h
//
// Copyright (C) 2004 Olivier Delalleau 
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
   * $Id: SubInputVMatrix.h,v 1.1 2004/02/26 03:31:55 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file SubInputVMatrix.h */


#ifndef SubInputVMatrix_INC
#define SubInputVMatrix_INC

#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;

class SubInputVMatrix: public SourceVMatrix
{

public:

  typedef SourceVMatrix inherited;

protected:

  // *********************
  // * protected options *
  // *********************

public:

  // ************************
  // * public build options *
  // ************************

  int j_start;
  int n_inputs;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  SubInputVMatrix();

  // ******************
  // * Object methods *
  // ******************

private: 

  //! This does the actual building. 
  void build_();

protected: 

  //! Declares this class' options
  static void declareOptions(OptionList& ol);

public:

  //! This is the only method requiring implementation.
  virtual void getRow(int i, Vec v) const;

  // Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(SubInputVMatrix);

};

DECLARE_OBJECT_PTR(SubInputVMatrix);

} // end of namespace PLearn

#endif
