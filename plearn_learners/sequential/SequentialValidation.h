// -*- C++ -*-

// SequentialValidation.h
//
// Copyright (C) 2003 Rejean Ducharme, Yoshua Bengio
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



#ifndef SEQUENTIAL_VALIDATION
#define SEQUENTIAL_VALIDATION

#include <plearn/base/PP.h>
#include <plearn/base/Object.h>
#include <plearn/vmat/VMat.h>
#include <plearn/math/VecStatsCollector.h>
#include <plearn/io/PPath.h>
#include <plearn_learners/testers/PTester.h>  // for using class StatSpec
#include <plearn_learners/generic/StatefulLearner.h>

namespace PLearn {
using namespace std;


class SequentialValidation: public Object
{
  typedef Object inherited;

protected:
  //#####  Data members required for simulation  ############################

  //! Training stat collector for main learner
  PP<VecStatsCollector> train_stats;

  //! Training stat collector for accessory learners
  PP<VecStatsCollector> accessory_train_stats;
  
  //! Test stat collector
  PP<VecStatsCollector> test_stats;

  //! Sequence stat collector
  PP<VecStatsCollector> sequence_stats;

  //! Timewise stat collector
  PP<VecStatsCollector> timewise_stats;
  
  // Contains pointers to train_stats and test_stats
  TVec< PP<VecStatsCollector> > stcol;

  //! Statspec corresponding to statnames
  TVec<StatSpec> statspecs;

  //! Statspec corresponding to timewise_statnames
  TVec<StatSpec> timewise_statspecs;
  
  //! vmat where to save global result stats specified in statnames
  VMat global_stats_vm;

  //! vmat where to save per split result stats
  VMat split_stats_vm;

  //! vmat where to save timewise statistics
  VMat timewise_stats_vm; 

  //! Buffers
  Vec input, target;
  Vec dummy_output;                          //!< for accessory_learners
  Vec dummy_costs;                           //!< for accessory_learners  
  Vec output;
  Vec costs;
  
public:
  
  // *********************
  // * public options    *
  // *********************
    
  //! Size of first training set (default: 1)
  int init_train_size;

  /**
   * If specified, this is a number of time-steps that are taken FROM THE
   * END of init_train_size to start "testing" (i.e. alternating between
   * train and test), but WITHOUT ACCUMULATING ANY TEST STATISTICS.  In
   * other words, this is a "warmup" period just before the true test.
   * Before starting the real test period, the setTestStartTime() method is
   * called on the learner, followed by resetInternalState().  Note that
   * the very first "init_train_size" is REDUCED by the warmup_size.
   */
  int warmup_size;
    
  //! At how many timesteps must we retrain?  (default: 1)
  //! If this is zero, train() is never called.
  int train_step;
  
  //! The last time-step to use for testing (Default = -1, i.e. use all data)
  int last_test_time;
  
  //! The training/test set
  VMat dataset;

  //! The StatefulLearner that will be tested
  PP<StatefulLearner> learner;

  /*!
    Accessory learners that must be managed in parallel with the main one.
  */
  TVec< PP<StatefulLearner> > accessory_learners;
  
  //! Global statistics or split statistics to be computed
  TVec<string> statnames;

  //! Timewise statistics to be computed
  TVec<string> timewise_statnames;

  //! the directory where everything will be saved
  PPath expdir;
  
  bool report_stats;
  bool save_final_model;
  bool save_initial_model;
  bool save_initial_seqval;
  bool save_data_sets;
  bool save_test_outputs;
  bool save_test_costs;
  bool save_stat_collectors;
  bool provide_learner_expdir; /* =true */

  //! Whether the statistics accumulated at each time step should
  //! be saved in the file "sequence_stats.pmat".  WARNING: this
  //! file can get big!  (Default = 1, i.e. true)
  bool save_sequence_stats;

  //! Whether to report memory usage in a directory expdir/MemoryUsage.
  //! Memory usage is reported AT THE BEGINNING OF EACH time-step, using
  //! both the /proc/PID/status method, and the 'mem_usage PID' method
  //! (if available).  This is only supported on Linux at the moment.
  //! (Default = false)
  bool report_memory_usage;

  /**
   * List of options to "measure" AFTER training at each timestep, but
   * BEFORE testing.  The options are specified as a list of pairs
   * 'option':'filename', where the option is measured with respect to the
   * sequential validation object itself.  Hence, if the learner contains
   * an option 'advisor' that you want to save at each time step, you would
   * write ["learner.advisor":"advisor.psave"].  The files are saved in the
   * splitdir directory, which is unique for each timestep.
   */
  TVec< pair<string,string> > measure_after_train;

  //! List of options to "measure" AFTER test, in the same format as
  //! 'measure_after_test'.
  TVec< pair<string,string> > measure_after_test;
  
private:

  //! This does the actual building
  void build_();

protected:

  //! Declare this class' options
  static void declareOptions(OptionList& ol);

  //! Utility method to report statistics
  virtual void reportStats(const Vec& global_result);

  //! Utility method to report the amount of memory used at timestep t
  virtual void reportMemoryUsage(int t);

  //! Utility method that returns true if train() should be called
  //! at timestep t.  By default, this is determined from the
  //! 'train_step' option, but can be redefined in derived classes
  virtual bool shouldTrain(int t);
  
  //! Utility method to return the training VMatrix at timestep t
  //! (i.e. all timesteps 0..t-1, t excluded)
  virtual VMat trainVMat(int t);

  //! Utility method to return the test VMatrix at timestep t
  //! (i.e. all timesteps 0..t, t included)
  virtual VMat testVMat(int t);

  //! Utility method to return the highest possible timestep plus 1
  virtual int maxTimeStep() const;

  //! Utility function to measure a list of options and save them
  //! in the specified directory
  virtual void measureOptions(const TVec< pair<string,string> >& options,
                              PPath where_to_save);
  
public:

  //! Default constructor
  SequentialValidation();
  
  //! Simply calls inherited::build() then build_()
  virtual void build();
  
  virtual void setExperimentDirectory(const PPath& _expdir);

  //! The main method;  runs the experiment
  virtual void run();

  //! If warmup_size > 0, warmup the learner before running the experiment
  virtual void warmupModel(int warmup_size);

  //! Set the test-start time of learner and accessory learners;
  //! call resetInternalState() and optionally build()
  virtual void setTestStartTime(int test_start_time, bool call_build = true);

  //! Create the stat collectors
  virtual void createStatCollectors();

  //! Create the stat specs
  virtual void createStatSpecs();
  
  //! Create the vmatrix required for saving the statistics
  virtual void createStatVMats();

  //! Train the main learner (and accessory learners)
  virtual void trainLearners(VMat training_set);

  //! Test learner on LAST OBSERVATION of test_set; also call
  //! computeOutputAndCosts on accessory learners
  virtual void testLearners(VMat test_set);
  
  //!  Does the necessary operations to transform a shallow copy (this)
  //!  into a deep copy by deep-copying all the members that need to be.
  void makeDeepCopyFromShallowCopy(CopiesMap& copies);
  
  PLEARN_DECLARE_OBJECT(SequentialValidation);
};
  
//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(SequentialValidation);

} // end of namespace PLearn

#endif
