// -*- C++ -*-

// TSVM.h
//
// Copyright (C) 2005 Olivier Delalleau 
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
   * $Id$ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file TSVM.h */


#ifndef TSVM_INC
#define TSVM_INC

#include <plearn/vmat/VMat.h>
#include <plearn_torch/TQCMachine.h>
#include <torch/SVM.h>

namespace PLearn {

class TDataSet;
class TKernel;
class TSequence;

class TSVM: public TQCMachine
{

private:
  
  typedef TQCMachine inherited;

protected:

  //! The underlying Torch SVM object.
  Torch::SVM* svm;

  //! Contains sv_sequences_vmat->toMat()
  Mat sv_sequences_mat;

  //! Contains the indices of each sv_sequence:
  //! sv_sequences_index[i] = sv_sequences_mat[i]
  TVec<real*> sv_sequences_index;

  //! Contains pointers to each sequence (= support vector).
  TVec< Torch::Sequence* > sv_sequences_torch;

  // *********************
  // * protected options *
  // *********************

  Vec sv_alpha;
  PP<TDataSet> data;
  int n_support_vectors_bound;
  TVec<int> support_vectors;
  TVec< PP<TSequence> > sv_sequences;
  VMat sv_sequences_vmat;

public:

  // ************************
  // * public build options *
  // ************************

  real b;
  PP<TKernel> kernel;
  string sv_save;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  TSVM();

  // ******************
  // * Object methods *
  // ******************

private: 

  //! This does the actual building. 
  void build_();

protected: 

  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:

  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(TSVM);

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  //! Update the underlying Torch object from this object's options.
  virtual void updateFromPLearn(Torch::Object* ptr);

  //! Update this object's options from the underlying Torch object.
  virtual void updateFromTorch();

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(TSVM);
  
} // end of namespace PLearn

#endif
