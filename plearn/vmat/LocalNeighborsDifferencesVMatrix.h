// -*- C++ -*-

// LocalNeighborsDifferencesVMatrix.h
//
// Copyright (C) 2004 Martin Monperrus 
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
   * $Id: LocalNeighborsDifferencesVMatrix.h,v 1.1 2004/05/28 21:55:02 monperrm Exp $ 
   ******************************************************* */

// Authors: Martin Monperrus & Yoshua Bengio

/*! \file LocalNeighborsDifferencesVMatrix.h */


#ifndef LocalNeighborsDifferencesVMatrix_INC
#define LocalNeighborsDifferencesVMatrix_INC

#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;

class LocalNeighborsDifferencesVMatrix: public SourceVMatrix
{

private:

  typedef SourceVMatrix inherited;

protected:
  // NON-OPTION FIELDS

  TMat<int> neighbors; // length x n_neighbors matrix of nearest neighbor indices

  // *********************
  // * protected options *
  // *********************

public:

  // ************************
  // * public build options *
  // ************************

  // ### declare public option fields (such as build options) here
  // ...
  int n_neighbors;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  LocalNeighborsDifferencesVMatrix();

  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:
  //!  This is the only method requiring implementation
  virtual void getRow(int i, Vec v) const;

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(LocalNeighborsDifferencesVMatrix);

};
DECLARE_OBJECT_PTR(LocalNeighborsDifferencesVMatrix);

inline VMat local_neighbors_differences(VMat source, int n_neighbors)
{
  LocalNeighborsDifferencesVMatrix* vmat = new LocalNeighborsDifferencesVMatrix();
  vmat->source=source;
  vmat->n_neighbors=n_neighbors;
  vmat->build();
  return vmat;
}

} // end of namespace PLearn
#endif
