// -*- C++ -*-

// RepeatVMatrix.h
//
// Copyright (C) 2005 Hugo Larochelle
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file RepeatVMatrix.h */


#ifndef RepeatVMatrix_INC
#define RepeatVMatrix_INC

#include <plearn/vmat/SourceVMatrix.h>

namespace PLearn {

class RepeatVMatrix: public SourceVMatrix
{

private:

  typedef SourceVMatrix inherited;

protected:

  // *********************
  // * protected options *
  // *********************

public:

  // ************************
  // * public build options *
  // ************************

  //! Number of times to repeat the source VMatrix
  int repeat_n_times;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  // ### Make sure the implementation in the .cc
  // ### initializes all fields to reasonable default values.
  RepeatVMatrix();

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

  //! Fill the vector 'v' with the content of the i-th row.
  //! v is assumed to be the right size.
  virtual void getNewRow(int i, const Vec& v) const;

public:

  // Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(RepeatVMatrix);

};

DECLARE_OBJECT_PTR(RepeatVMatrix);

inline VMat repeat_vmatrix(VMat source, int repeat_n_times)
{
  RepeatVMatrix* vmat = new RepeatVMatrix();
  vmat->source = source;
  vmat->repeat_n_times = repeat_n_times;
  vmat->build();
  return vmat;
}

} // end of namespace PLearn

#endif
