// -*- C++ -*-

// EmbeddedLearner.h
// 
// Copyright (C) 2002 Frederic Morin
// Copyright (C) 2003 Pascal Vincent
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
   * $Id: EmbeddedLearner.h,v 1.12 2004/10/06 05:43:30 chapados Exp $ 
   ******************************************************* */

/*! \file EmbeddedLearner.h */
#ifndef EmbeddedLearner_INC
#define EmbeddedLearner_INC

#include "PLearner.h"

namespace PLearn {
using namespace std;

// ###### EmbeddedLearner ######################################################

class EmbeddedLearner: public PLearner
{
  typedef PLearner inherited;

public:

  // build option
  PP<PLearner> learner_;


  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  EmbeddedLearner();

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
  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(EmbeddedLearner);

  // *******************
  // * PLearner methods *
  // *******************

  //! Forwarded to inner learner
  virtual int inputsize() const;
  
  //! Forwarded to inner learner
  virtual int targetsize() const; 
  
  //! Forwarded to inner learner
  virtual int outputsize() const;

  //! Forwarded to inner learner
  virtual void forget();

  //! Forwarded to inner learner
  virtual void train();

  //! Forwarded to inner learner
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Forwarded to inner learner
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;
                              
  //! Forwarded to inner learner
  virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                     Vec& output, Vec& costs) const;

  //! Forwarded to inner learner
  virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;

  //! Forwarded to inner learner
  virtual void use(VMat testset, VMat outputs) const;

  //! Forwarded to inner learner
  virtual void useOnTrain(Mat& outputs) const;  
  
  //! Forwarded to inner learner
  virtual void test(VMat testset, PP<VecStatsCollector> test_stats, 
                    VMat testoutputs=0, VMat testcosts=0) const;

  //! Forwarded to inner learner
  virtual TVec<string> getTestCostNames() const;

  //! Forwarded to inner learner
  virtual TVec<string> getTrainCostNames() const;

  //! Forwarded to inner learner
  virtual int nTestCosts() const;

  //! Forwarded to inner learner
  virtual int nTrainCosts() const;

  //! Forwarded to inner learner
  virtual void resetInternalState();

  //! Forwarded to inner learner
  virtual bool isStatefulLearner() const;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(EmbeddedLearner);
  
} // end of namespace PLearn

#endif
