// -*- C++ -*-

// StatefulLearner.h
//
// Copyright (C) 2004 Réjean Ducharme 
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
   * $Id: StatefulLearner.h,v 1.4 2004/07/23 20:24:20 ducharme Exp $ 
   ******************************************************* */

// Authors: Réjean Ducharme

/*! \file StatefulLearner.h */


#ifndef StatefulLearner_INC
#define StatefulLearner_INC

#include "PLearner.h"

namespace PLearn {
using namespace std;

class StatefulLearner: public PLearner
{
private:

  typedef PLearner inherited;

protected:

  //! Time step at which the current test is perform
  //! This can be useful when entering computeOutputAndCosts(...) method.
  int current_test_t;
  
public:

  //! Default constructor.
  StatefulLearner();

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
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declares other standard object methods.
  PLEARN_DECLARE_ABSTRACT_OBJECT(StatefulLearner);


  // **************************
  // **** PLearner methods ****
  // **************************

  //! Forget still needs to be redefined in derived classes, but the
  //! implementation here performs a no-op.  This allows derived classes
  //! to write inherited::forget() without fear.
  virtual void forget() = 0;
  
  //! Computes the output from the input.
  //! Simply calls computeOutputAndCosts(...)
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Computes the costs from already computed output. 
  //! This has no sense for a StatefulLearner since we suppose that the output
  //! and costs are computed together from the SAME state!
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;

  //! Computes the outputs and costs simultaneously
  virtual void computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const =0;

  //! Computes the costs from the input and target vectors
  //! Simply calls computeOutputAndCosts(...)
  virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;

  //! Declares the train set
  virtual void setTrainingSet(VMat training_set, bool call_forget=true);


  // *******************************
  // **** StafulLearner methods ****
  // *******************************

  //! Does this PLearner has an internal state?
  //! Default: true
  virtual bool isStatefulLearner() const;

  //! Set the dataset of an AssetManager (if any) to this testset
  //! Default: do nothing!
  virtual void setTestSet(VMat testset);

  //! Set the time step at which the current test is perform
  virtual void setCurrentTestTime(int test_t);
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(StatefulLearner);
  
} // end of namespace PLearn

#endif
