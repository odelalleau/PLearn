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
   * $Id$
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file StackedLearner.h */

#ifndef StackedLearner_INC
#define StackedLearner_INC

#include "PLearner.h"
#include <plearn/vmat/Splitter.h>

namespace PLearn {
using namespace std;

class StackedLearner: public PLearner
{
  typedef PLearner inherited;
  
protected:
  //! Temporary buffers for the output of base learners.  This is a
  //! TVec of Vec since we now allow each learner's outputsize to be
  //! different
  TVec< Vec > base_learners_outputs;

  //! Buffer for concatenated output of base learners
  mutable Vec all_base_learners_outputs;


public:

  // ************************
  // * public build options *
  // ************************

  //! A set of 1st level base learners that are independently trained
  //! (here or elsewhere) and whose outputs will serve as inputs to the combiner.
  TVec<PP<PLearner> > base_learners;

  //! A learner that is trained (possibly on a data set different from the
  //! one used to train the base_learners) using the outputs of the base_learners
  //! as inputs.
  PP<PLearner> combiner;

  //! if no combiner is provided, simple operation to be performed 
  //! on the outputs of the base_learners.
  //! Supported: mean (default), min, max, variance, sum, sumofsquares
  string default_operation;

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

  //! If set to 1, the output of the base learners on the combiner training set
  //! will be normalized (zero mean, unit variance).
  bool normalize_base_learners_output;

  //! If set to 1, the output of the base learners on the combiner training set
  //! will be precomputed in memory before training the combiner.
  bool precompute_base_learners_output;
  
  //! Optionally put the raw input as additional input of the combiner.
  bool put_raw_input;

  //! If set to 1, the input is divided in nsep equal parts, and a common learner
  //! is trained as if each part constitutes a training example.
  bool share_learner;

  //! Number of input separations.
  int nsep;

  //! optionally put the costs of all the learners as additional inputs of the combiner
  //! (NB: this option actually isn't implemented yet)
  bool put_costs;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  StackedLearner();


  // ******************
  // * PLearner methods *
  // ******************

private: 

  //! This does the actual building. 
  void build_();

  //! Resize 'base_learners_outputs' to fit with current base learners.
  void resizeBaseLearnersOutputs();

protected: 

  //! Declares this class' options
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Forwarded to combiner
  void setTrainStatsCollector(PP<VecStatsCollector> statscol);

  //! Forwarded to inner learners
  virtual void setExperimentDirectory(const PPath& the_expdir);

  //! simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  // Declares other standard object methods
  PLEARN_DECLARE_OBJECT(StackedLearner);


  // **************************
  // **** PLearner methods ****
  // **************************

  //! returns the size of this learner's output, (which typically
  //! may depend on its inputsize(), targetsize() and set options)
  virtual int outputsize() const;

  //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
  virtual void forget();

    
  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  virtual void train();


  //! Computes the output from the input
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Computes the costs from already computed output. 
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;
                                

  //! Forwarded to combiner after passing input vector through baselearners
  virtual
  bool computeConfidenceFromOutput(const Vec& input, const Vec& output,
                                   real probability,
                                   TVec< pair<real,real> >& intervals) const;
  
  //! Returns the names of the costs computed by computeCostsFromOutpus (and thus the test method)
  virtual TVec<string> getTestCostNames() const;

  //! Returns the names of the objective costs that the train method computes and 
  //! for which it updates the VecStatsCollector train_stats
  virtual TVec<string> getTrainCostNames() const;


  //! Declares the train_set
  //! Then calls build() and forget() if necessary
  virtual void setTrainingSet(VMat training_set, bool call_forget=true);

protected:
  //! Utility to set training set of base learners
  void setBaseLearnersTrainingSet(VMat base_trainset, bool call_forget);

  //! Utility to set training set of combiner
  void setCombinerTrainingSet(VMat comb_trainset, bool call_forget);
};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(StackedLearner);
  
} // end of namespace PLearn

#endif
