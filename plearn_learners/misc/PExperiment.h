// -*- C++ -*-

// PTester.h
// 
// Copyright (C) 2002 Pascal Vincent, Frederic Morin
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
   * $Id: PExperiment.h,v 1.10 2004/07/21 16:30:57 chrish42 Exp $ 
   ******************************************************* */

/*! \file PTester.h */
#ifndef PExperiment_INC
#define PExperiment_INC

#include <plearn/base/Object.h>
#include <plearn_learners/generic/PLearner.h>
#include <plearn/vmat/VMat.h>
#include <plearn/vmat/Splitter.h>

namespace PLearn {
using namespace std;

//! This code is deprecated, use PTester.h and PTester.cc instead.
class PTester: public Object
{    
public:
  bool save_initial_experiment;
  typedef Object inherited;

  // ************************
  // * public build options *
  // ************************
  
  // See declareOptions method in .cc for the role of these options.

  //! Path of this experiment's directory in which to save all experiment results (will be created if it does not already exist)
  string expdir;  
  PP<PLearner> learner;
  PP<Splitter> splitter;
  TVec<string> statnames;
  bool report_stats;

  bool save_stat_collectors;
  bool save_learners;
  bool save_initial_learners;
  bool save_data_sets;
  bool save_test_outputs;
  bool save_test_costs;
  bool provide_learner_expdir;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor
  PTester();


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

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(PTester);

  //! The experiment directory is the directory in which files 
  //! related to this model are to be saved.     
  //! If it is an empty string, it is understood to mean that the 
  //! user doesn't want any file created by this learner.
  void setExperimentDirectory(const string& the_expdir);
  
  //! This returns the currently set expdir (see setExperimentDirectory)
  string getExperimentDirectory() const { return expdir; }
  


  //! runs the experiment
  virtual void run();

  //! performs the experiment, and returns the global stats specified in statnames
  //! If dont_set_training_set is set to true AND the splitter returns only one split,
  //! then we *don't* call setTrainingSet() and a forget() on the learner prior to training it:
  //! we assume the training set is already set. This is useful for continuation of an incremental  training
  //! (such as after increasing the number of epochs (nstages) ). 
  Vec perform(bool dont_set_training_set=false);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PTester);
  
} // end of namespace PLearn

#endif
