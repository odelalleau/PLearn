// -*- C++ -*-

// LLEKernel.h
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
   * $Id: LLEKernel.h,v 1.6 2004/09/14 16:04:36 chrish42 Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file LLEKernel.h */


#ifndef LLEKernel_INC
#define LLEKernel_INC

#include "Kernel.h"
#include "ReconstructionWeightsKernel.h"

namespace PLearn {
using namespace std;

class LLEKernel: public Kernel
{

private:

  typedef Kernel inherited;

  //! True iff build() has been called but build_() has not been called yet.
  bool build_in_progress;
  
  //! Used in 'evaluate_i_x_again' to remember whether x is a training point
  //! or not.
  mutable bool x_is_training_point;

  //! Used in 'evaluate_i_x_again' to remember the index of x when it is a
  //! training point.
  mutable int x_index;
    
protected:

  // *********************
  // * Protected options *
  // *********************

  // Fields below are not options.

  //! The kernel used to compute the reconstruction weights.
  PP<ReconstructionWeightsKernel> reconstruct_ker;

public:

  // ************************
  // * Public build options *
  // ************************

  int knn;
  real reconstruct_coeff;
  real regularizer;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  LLEKernel();

  // ******************
  // * Kernel methods *
  // ******************

private: 

  //! This does the actual building. 
  void build_();

protected: 
  
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(LLEKernel);

  // **************************
  // **** Kernel methods ****
  // **************************

  //! Overridden for efficiency purpose.
  virtual void computeGramMatrix(Mat K) const;

  //! Compute K(x1,x2).
  virtual real evaluate(const Vec& x1, const Vec& x2) const;

  virtual real evaluate_i_j(int i, int j) const;

  virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const;

  virtual real evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x=-1, bool first_time = false) const;

  virtual void setDataForKernelMatrix(VMat the_data);

  // *** SUBCLASS WRITING: ***
  // While in general not necessary, in case of particular needs 
  // (efficiency concerns for ex) you may also want to overload
  // some of the following methods:
  // virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const;
  // virtual real evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x=-1, bool first_time = false) const;
  // virtual void addDataForKernelMatrix(const Vec& newRow);
  // virtual void setParameters(Vec paramvec);
  // virtual Vec getParameters() const;
  

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(LLEKernel);
  
} // end of namespace PLearn

#endif

