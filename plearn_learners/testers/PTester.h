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
   * $Id: PTester.h,v 1.1 2003/07/04 18:48:59 plearner Exp $ 
   ******************************************************* */

/*! \file PTester.h */
#ifndef PTester_INC
#define PTester_INC

#include "Object.h"
#include "PLearner.h"
#include "VMat.h"
#include "Splitter.h"

namespace PLearn <%
using namespace std;

class PTester: public Object
{    
public:

  typedef Object inherited;

  // ************************
  // * public build options *
  // ************************
  
  // See declareOptions method in .cc for the role of these options.

  //! Path of this tester's experiment directory in which to save all tester results (will be created if it does not already exist)
  string expdir;  
  PP<PLearner> learner;
  VMat dataset;
  PP<Splitter> splitter;
  TVec<string> statnames;
  bool report_stats;
  bool save_initial_tester;
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

  //! Provides a help message describing this class
  static string help();

  //! Declares name and deepCopy methods
  DECLARE_NAME_AND_DEEPCOPY(PTester);

  //! The experiment directory is the directory in which files 
  //! related to this model are to be saved.     
  //! If it is an empty string, it is understood to mean that the 
  //! user doesn't want any file created by this learner.
  void setExperimentDirectory(const string& the_expdir);
  
  //! This returns the currently set expdir (see setExperimentDirectory)
  string getExperimentDirectory() const { return expdir; }
  


  //! runs the tester
  virtual void run();

  //! performs the test, and returns the global stats specified in statnames
  //! If set_training_set is set to false AND the splitter returns only one split,
  //! then we *don't* call setTrainingSet() and a forget() on the learner prior to training it:
  //! we assume the training set is already set. This is useful for continuation of an incremental  training
  //! (such as after increasing the number of epochs (nstages) ). 
  Vec perform(bool set_training_set=true);

};


//! The specification of a statistic to compute (as can be specified as a string in PTester)

class StatSpec
{
public:
  string extstat;  //! "external" stat, to be computed over splits
  string intstat;  //! "internal" stat to be computed over examples the given a train or test set of a split
  string setname;  //! "train" or "test1" or "test2" ...
  int setnum;      //! data set on which to compute stat: 0 :train, 1: test1, ...
  string costname; //! the name of the cost we are interested in.
  int costindex; // index of cost in vector of train costs (if setnum==0) or test costs (if setnum==1) computed by the learner.

  StatSpec() : setnum(-1), costindex(-1) {}

  void init(const string& statname, PP<PLearner> learner);

  string intStatName()
  { return intstat + "[" + setname + "." + costname + "]"; }

  string statName()
  { return extstat + "[" + intStatName() + "]"; }

private:

  //! will determine extstat, intstat, setnum and costname from statname 
  void parseStatname(const string& statname);
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PTester);
  
%> // end of namespace PLearn

#endif
