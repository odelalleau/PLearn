// -*- C++ -*-

// ClassSubsetVMatrix.h
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
   * $Id: ClassSubsetVMatrix.h 3760 2005-07-11 17:27:18Z tihocan $
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file ClassSubsetVMatrix.h */


#ifndef ClassSubsetVMatrix_INC
#define ClassSubsetVMatrix_INC

#include "SelectRowsVMatrix.h"

namespace PLearn {
using namespace std;

class ClassSubsetVMatrix: public SelectRowsVMatrix
{

private:

  typedef SelectRowsVMatrix inherited;

  Vec input,target;
  real weight;
protected:

  // *********************
  // * protected options *
  // *********************

public:

  // ************************
  // * public build options *
  // ************************

  //! Classes of examples to keep
  TVec<int> classes;

  //! Indication that the class values should be
  //! redistributed between 0 and classes.length()-1,
  //! based on the order in classes
  bool redistribute_classes;

  //! Indication that, if classes contains 2 class indexes,
  //! than they should be mapped to -1 (the first index) and 1
  bool one_vs_minus_one_classification;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  ClassSubsetVMatrix();

  //! Convenience constructor.
  //! Note that the vector 'the_classes' is copied and thus may be modified
  //! afterwards.
  ClassSubsetVMatrix(VMat the_source, const TVec<int>& the_classes,
                     bool call_build_ = true);

  //! Convenience constructor.
  ClassSubsetVMatrix(VMat the_source, int the_class, bool call_build_ = true);

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

  // Simply calls inherited::build() then build_().
  virtual void build();

  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;


  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(ClassSubsetVMatrix);

};

DECLARE_OBJECT_PTR(ClassSubsetVMatrix);

} // end of namespace PLearn

#endif
