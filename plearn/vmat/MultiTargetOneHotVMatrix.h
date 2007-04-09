// -*- C++ -*-

// MultiTargetOneHotVMatrix.h
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
   * $Id: MultiTargetOneHotVMatrix.h,v 1.5 2005/05/13 14:55:46 delallea Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file MultiTargetOneHotVMatrix.h */


#ifndef MultiTargetOneHotVMatrix_INC
#define MultiTargetOneHotVMatrix_INC

#include <plearn/vmat/SourceVMatrix.h>

namespace PLearn {
using namespace std;

class MultiTargetOneHotVMatrix: public SourceVMatrix
{

private:

  typedef SourceVMatrix inherited;

protected:

  //! Equal to source->width() - source->weightsize().
  int source_inputsize;

  // *********************
  // * protected options *
  // *********************

  // Fields below are not options.

  //! A precomputed view of the data. The i-th row contains 3 (or 4) numbers:
  //! the index of the source row for the i-row, the index of the target
  //! considered (the one-hot cell to fill), and the value of the target we
  //! output in this VMat. Additionally, if 'reweight_targets' is set to 1,
  //! a 4-th number is added: the weight of the sample.
  Mat data;

public:

  // ************************
  // * public build options *
  // ************************

  real missing_target;
  bool reweight_targets;
  VMat source_target;
  VMat target_descriptor;
  string target_val;
  int verbosity;
  real hot_value;
  real cold_value;
  VMat source_and_target;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  MultiTargetOneHotVMatrix();

  // ******************
  // * Object methods *
  // ******************

private: 

  //! This does the actual building. 
  void build_();

protected: 

  //! Declares this class' options
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
  PLEARN_DECLARE_OBJECT(MultiTargetOneHotVMatrix);

  //! Return 'source_inputsize'.
  int getSourceInputSize() { return source_inputsize; }

};

DECLARE_OBJECT_PTR(MultiTargetOneHotVMatrix);

  inline VMat multi_target_one_hot(VMat source_and_target,real cold_value, real hot_value)
  {
    MultiTargetOneHotVMatrix* ret = new MultiTargetOneHotVMatrix();
    ret->source_and_target = source_and_target;
    ret->cold_value = cold_value;
    ret->hot_value = hot_value;
    ret->build();
    return ret;
  }

} // end of namespace PLearn

#endif
