
// -*- C++ -*-

// AddTradableColumnVMatrix.h
//
// Copyright (C) 2003  Rejean Ducharme 
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
   * $Id: AddTradableColumnVMatrix.h,v 1.1 2003/08/07 20:21:37 ducharme Exp $ 
   ******************************************************* */

/*! \file AddTradableColumnVMatrix.h */
#ifndef AddTradableColumnVMatrix_INC
#define AddTradableColumnVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn <%
using namespace std;

class AddTradableColumnVMatrix: public RowBufferedVMatrix
{
public:

  int min_volume_threshold; //! tradable = 1 if volume>min_volume_threshold

protected:

  VMat underlying; //! the underlying vmat
  vector< pair<string,int> > name_col; //! all the (asset:is_tradable)/(volume column) pairs
  Vec row_buffer; //! a row buffer for the getRow method

public:

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  AddTradableColumnVMatrix();

  //! Simple constructor: takes as input only the underlying matrix,
  //! the number of assets and the threshold on the volume.
  AddTradableColumnVMatrix(VMat vm, int nb_assets, int threshold=20);

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

  void setVMFields();

public:
  //!  This is the only method requiring implementation
  virtual void getRow(int i, Vec v) const;

  // simply calls parentclass::build() then build_() 
  virtual void build();

  //! Provides a help message describing this class
  static string help();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT_METHODS(AddTradableColumnVMatrix, "AddTradableColumnVMatrix", RowBufferedVMatrix);

};

%> // end of namespace PLearn
#endif
