// -*- C++ -*-

// StackedLearner.h
//
// Copyright (C) 2003 Yoshua Bengio 
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
   * $Id: StackedLearner.h,v 1.7 2004/02/17 14:52:47 tihocan Exp $
   ******************************************************* */

// Authors: Yoshua Bengio, Olivier Delalleau

/*! \file StackedLearner.h */


#ifndef StackedLearner_INC
#define StackedLearner_INC

#include "PLearner.h"
#include "Splitter.h"

namespace PLearn <%
using namespace std;

class StackedLearner: public PLearner
{
protected:
  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...

  // NON-OPTIONs
  Mat base_learners_outputs;

public:

  typedef PLearner inherited;
  

  // ************************
  // * public build options *
  // ************************

  // ### declare public option fields (such as build options) here

  //! A set of 1st level base learners that are independently trained
  //! (here or elsewhere) and whose outputs will serve as inputs to the combiner.
  TVec<PP<PLearner> > base_learners;

  //! A learner that is trained (possibly on a data set different from the
  //! one used to train the base_learners) using the outputs of the base_learners
  //! as inputs.
  PP<PLearner> combiner;

  //! This can be 
  //! and which data subset(s) goes to training the combiner.
  PP<Splitter> splitter;

  //! This can be used to split the training set into different training sets
  //! for each base learner. If it is not set, the same training set will be
  //! applied to the base learners.
  PP<Splitter> base_train_splitter;

  //! whether to train the base learners in the method train (otherwise they should be
  //! initialized at construction / setOption time)
  bool train_base_learners;

  //! optionally put the raw input as additional input of the combiner
  bool put_raw_input;

  //! optionally put the costs of all the learners as additional inputs of the combiner
  bool put_costs;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  StackedLearner();


  // ******************
  // * PLearner methods *
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

  // ************************
  // **** Object methods ****
  // ************************

  void setTrainStatsCollector(PP<VecStatsCollector> statscol);

  //! simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declares other standard object methods
  //  If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
  PLEARN_DECLARE_OBJECT(StackedLearner);


  // **************************
  // **** PLearner methods ****
  // **************************

  //! returns the size of this learner's output, (which typically
  //! may depend on its inputsize(), targetsize() and set options)
  // (PLEASE IMPLEMENT IN .cc)
  virtual int outputsize() const;

  //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
  // (PLEASE IMPLEMENT IN .cc)
  virtual void forget();

    
  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  // (PLEASE IMPLEMENT IN .cc)
  virtual void train();


  //! Computes the output from the input
  // (PLEASE IMPLEMENT IN .cc)
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Computes the costs from already computed output. 
  // (PLEASE IMPLEMENT IN .cc)
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;
                                

  //! Returns the names of the costs computed by computeCostsFromOutpus (and thus the test method)
  // (PLEASE IMPLEMENT IN .cc)
  virtual TVec<string> getTestCostNames() const;

  //! Returns the names of the objective costs that the train method computes and 
  //! for which it updates the VecStatsCollector train_stats
  // (PLEASE IMPLEMENT IN .cc)
  virtual TVec<string> getTrainCostNames() const;


    //! Declares the train_set
    //! Then calls build() and forget() if necessary
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

  // *** SUBCLASS WRITING: ***
  // While in general not necessary, in case of particular needs 
  // (efficiency concerns for ex) you may also want to overload
  // some of the following methods:
  // virtual void computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const;
  // virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;
  // virtual void test(VMat testset, PP<VecStatsCollector> test_stats, VMat testoutputs=0, VMat testcosts=0) const;
  // virtual int nTestCosts() const;
  // virtual int nTrainCosts() const;

};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(StackedLearner);
  
%> // end of namespace PLearn

#endif
