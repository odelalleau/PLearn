// -*- C++ -*-

// SequentialValidation.cc
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



#include "SequentialValidation.h"
#include <plearn/math/VecStatsCollector.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn_learners/testers/PTester.h>  // for using class StatSpec

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
  SequentialValidation,
  "The SequentialValidation class allows you to describe a typical "
  "sequential validation experiment that you wish to perform.",
  "NO HELP");

SequentialValidation::SequentialValidation()
  : init_train_size(1),
    last_test_time(-1),
    expdir(""),
    report_stats(true),
    save_final_model(true),
    save_initial_model(false),
    save_initial_seqval(true),
    save_data_sets(false),
    save_test_outputs(false),
    save_test_costs(false),
    save_stat_collectors(false),
    provide_learner_expdir(true),
    save_sequence_stats(true)
{}

void SequentialValidation::build_()
{
  if (expdir != "")
  {
    if(pathexists(expdir))
      PLERROR("Directory (or file) %s already exists. First move it out of the way.", expdir.c_str());
    if(!force_mkdir(expdir))
      PLERROR("Could not create experiment directory %s", expdir.c_str());
  }

  if ( dataset && dataset->inputsize() < 0 )
    dataset->defineSizes(dataset->width(), 0, 0);
}

void SequentialValidation::build()
{
  inherited::build();
  build_();
}

void SequentialValidation::declareOptions(OptionList& ol)
{
  declareOption(ol, "report_stats", &SequentialValidation::report_stats,
                OptionBase::buildoption,
                "If true, the computed global statistics specified in statnames will be saved in global_stats.pmat \n"
                "and the corresponding per-split statistics will be saved in split_stats.pmat \n"
                "For reference, all cost names (as given by the learner's getTrainCostNames() and getTestCostNames() ) \n"
                "will be reported in files train_cost_names.txt and test_cost_names.txt");

  declareOption(ol, "statnames", &SequentialValidation::statnames,
                OptionBase::buildoption,
                "A list of global statistics we are interested in.\n"
                "These are strings of the form S1[S2[dataset.cost_name]] where:\n"
                "  - dataset is train or test1 or test2 ... (train being \n"
                "    the first dataset in a split, test1 the second, ...) \n"
                "  - cost_name is one of the training or test cost names (depending on dataset) understood \n"
                "    by the underlying learner (see its getTrainCostNames and getTestCostNames methods) \n"
                "  - S1 and S2 are a statistic, i.e. one of: E (expectation), V(variance), MIN, MAX, STDDEV, ... \n"
                "    S2 is computed over the samples of a given dataset split. S1 is over the splits. \n");

  declareOption(ol, "timewise_statnames", &SequentialValidation::timewise_statnames,
                OptionBase::buildoption,
                "Statistics to be collected into a VecStatsCollector at each timestep.");
  
  declareOption(ol, "expdir", &SequentialValidation::expdir,
                OptionBase::buildoption,
                "Path of this experiment's directory in which to save all experiment results (will be created if it does not already exist). \n");

  declareOption(ol, "learner", &SequentialValidation::learner,
                OptionBase::buildoption,
                "The SequentialLearner to train/test. \n");

  declareOption(ol, "dataset", &SequentialValidation::dataset,
                OptionBase::buildoption,
                "The dataset to use for training/testing. \n");

  declareOption(ol, "init_train_size", &SequentialValidation::init_train_size,
                OptionBase::buildoption,
                "Size of the first training set. \n");

  declareOption(ol, "last_test_time", &SequentialValidation::last_test_time,
                OptionBase::buildoption,
                "The last time-step to use for testing (Default = -1, i.e. use all data)");
  
  declareOption(ol, "save_final_model", &SequentialValidation::save_final_model,
                OptionBase::buildoption,
                "If true, the final model will be saved in model.psave \n");

  declareOption(ol, "save_initial_model", &SequentialValidation::save_initial_model,
                OptionBase::buildoption,
                "If true, the initial model will be saved in initial_model.psave. \n");

  declareOption(ol, "save_initial_seqval", &SequentialValidation::save_initial_seqval,
                OptionBase::buildoption,
                "If true, this SequentialValidation object will be saved in sequential_validation.psave. \n");

  declareOption(ol, "save_data_sets", &SequentialValidation::save_data_sets,
                OptionBase::buildoption,
                "If true, the data sets (train/test) for each split will be saved. \n");

  declareOption(ol, "save_test_outputs", &SequentialValidation::save_test_outputs,
                OptionBase::buildoption,
                "If true, the outputs of the tests will be saved in test_outputs.pmat \n");

  declareOption(ol, "save_test_costs", &SequentialValidation::save_test_costs,
                OptionBase::buildoption,
                "If true, the costs of the tests will be saved in test_costs.pmat \n");

  declareOption(ol, "save_stat_collectors", &SequentialValidation::save_stat_collectors,
                OptionBase::buildoption,
                "If true, stat collectors of each data sets (train/test) will be saved for each split. \n");

  declareOption(ol, "provide_learner_expdir", &SequentialValidation::provide_learner_expdir,
                OptionBase::buildoption,
                "If true, learning results from the learner will be saved. \n");

  declareOption(ol, "save_sequence_stats",
                &SequentialValidation::save_sequence_stats,
                OptionBase::buildoption,
                "Whether the statistics accumulated at each time step should\n"
                "be saved in the file \"sequence_stats.pmat\".  WARNING: this\n"
                "file can get big!  (Default = 1, i.e. true)");
  
  inherited::declareOptions(ol);
}

void SequentialValidation::run()
{
  if (expdir=="")
    PLERROR("No expdir specified for SequentialValidation.");
  if (!learner)
    PLERROR("No learner specified for SequentialValidation.");

  // This is to set inputsize() and targetsize()
  learner->setTrainingSet(dataset, false);

  setExperimentDirectory( append_slash(expdir) );

  // Save this experiment description in the expdir (buildoptions only)
  if (save_initial_seqval)
    PLearn::save(expdir+"sequential_validation.psave", *this);

  TVec<string> testcostnames = learner->getTestCostNames();
  TVec<string> traincostnames = learner->getTrainCostNames();
 
  int outputsize = learner->outputsize();
  int nstats = statnames.length();
  int timewise_nstats = timewise_statnames.length();

  TVec< PP<VecStatsCollector> > stcol(2);  // one for train and one for test

  // stats for a train on one split
  PP<VecStatsCollector> train_stats = new VecStatsCollector();
  train_stats->setFieldNames(traincostnames);
  learner->setTrainStatsCollector(train_stats);
  stcol[0] = train_stats;

  // stats for a test on one split
  PP<VecStatsCollector> test_stats = new VecStatsCollector();
  test_stats->setFieldNames(testcostnames);
  stcol[1] = test_stats;

  // stats over all sequence
  PP<VecStatsCollector> sequence_stats = new VecStatsCollector();

  // Stat specs (overall)
  TVec<StatSpec> statspecs(nstats);
  for (int k=0; k<nstats; k++)
    statspecs[k].init(statnames[k]);

  // timewise stats (may not be used)
  PP<VecStatsCollector> timewise_stats = new VecStatsCollector();

  // Stat specs (timewise)
  TVec<StatSpec> timewise_statspecs(timewise_nstats);
  for (int k=0; k<timewise_nstats; ++k)
    timewise_statspecs[k].init(timewise_statnames[k]);

  VMat global_stats_vm;   // vmat where to save global result stats specified in statnames
  VMat split_stats_vm;    // vmat where to save per split result stats
  VMat timewise_stats_vm; // vmat where to save timewise statistics

  // Create all VMatrix related to saving statistics
  if (report_stats)
  {
    saveStringInFile(expdir+"train_cost_names.txt", join(traincostnames,"\n")+"\n");
    saveStringInFile(expdir+"test_cost_names.txt", join(testcostnames,"\n")+"\n");

    global_stats_vm = new FileVMatrix(expdir+"global_stats.pmat", 0, nstats);
    for(int k=0; k<nstats; k++)
      global_stats_vm->declareField(k,statspecs[k].statName());
    global_stats_vm->saveFieldInfos();

    if (save_sequence_stats) {
      split_stats_vm = new FileVMatrix(expdir+"sequence_stats.pmat", 0,
                                       1+nstats);
      split_stats_vm->declareField(0,"splitnum");
      for(int k=0; k<nstats; k++)
        split_stats_vm->declareField(k+1,statspecs[k].setname + "." + statspecs[k].intstatname);
      split_stats_vm->saveFieldInfos();
    }

    if (timewise_nstats > 0) {
      timewise_stats_vm = new FileVMatrix(expdir+"timewise_stats.pmat", 0,
                                          timewise_nstats);
      for (int k=0; k<timewise_nstats; ++k)
        timewise_stats_vm->declareField(k, timewise_statspecs[k].statName());
      timewise_stats_vm->saveFieldInfos();
    }
  }

  // Ensure correct build and reset internal state
  learner->build();
  learner->resetInternalState();
  
  VMat test_outputs;
  VMat test_costs;
  if (save_test_outputs)
    test_outputs = new FileVMatrix(expdir+"/test_outputs.pmat",0,outputsize);
  if (save_test_costs)
    test_costs = new FileVMatrix(expdir+"/test_costs.pmat",0,testcostnames);

  // Some further initializations
  int maxt = (last_test_time >= 0? last_test_time : dataset.length() - 1);
  int splitnum = 0;
  double weight;
  Vec input, target;
  Vec output(learner->outputsize());
  Vec costs(learner->nTestCosts());
  
  for (int t=init_train_size; t <= maxt; t++, splitnum++)
  {
#ifdef DEBUG
    cout << "SequentialValidation::run() -- sub_train.length = " << t << " et sub_test.length = " << t+horizon << endl;
#endif
    VMat sub_train = dataset.subMatRows(0,t); // excludes t, last training pair is (t-2,t-1)
    VMat sub_test = dataset.subMatRows(0, t+1);
    VMat only_test = dataset.subMatRows(t, 1);

    string splitdir = expdir+"train_t="+tostring(t)+"/";
    if (save_data_sets || save_initial_model || save_stat_collectors || save_final_model)
      force_mkdir(splitdir);
    if (save_data_sets)
      PLearn::save(splitdir+"training_set.psave", sub_train);
    if (save_initial_model)
      PLearn::save(splitdir+"initial_learner.psave",learner);

    // TRAIN
    train_stats->forget();
    learner->setTrainingSet(sub_train, false);
    learner->train();
    train_stats->finalize();

    if (save_stat_collectors)
      PLearn::save(splitdir+"train_stats.psave",train_stats);
    if (save_final_model)
      PLearn::save(splitdir+"final_learner.psave",learner);

    // TEST: simply use computeOutputAndCosts for 1 observation in this
    // implementation
    dataset.getExample(t, input, target, weight);
    test_stats->forget();
    learner->setTestSet(sub_test);           // temporary hack
//    learner->setCurrentTestTime(t);          // temporary hack
    learner->computeOutputAndCosts(input, target, output, costs);
    test_stats->update(costs);
    test_stats->finalize();

    // Save what is required from the test run
    if (save_data_sets)
      PLearn::save(splitdir+"test_set.psave", sub_test);
    if (test_outputs)
      test_outputs->appendRow(output);
    if (test_costs)
      test_costs->appendRow(costs);
    if (save_stat_collectors)
      PLearn::save(splitdir+"test_stats.psave",test_stats);

    Vec splitres(1+nstats);
    splitres[0] = splitnum;

    // Compute statnames for this split only
    for(int k=0; k<nstats; k++)
    {
      StatSpec& sp = statspecs[k];
      if (sp.setnum>=stcol.length())
        PLERROR("SequentialValidation::run, trying to access a test set (test%d) beyond the last one (test%d)",
            sp.setnum, stcol.length()-1);
      splitres[k+1] = stcol[sp.setnum]->getStat(sp.intstatname);
    }

    if (split_stats_vm)
      split_stats_vm->appendRow(splitres);

    // Add to overall stats collector
    sequence_stats->update(splitres.subVec(1,nstats));

    // Now compute timewise statnames.  First loop is on the inner
    // statistics; then update the stats collector; then loop on the outer
    // statistics
    if (timewise_stats_vm) {
      Vec timewise_res(timewise_nstats);
      for (int k=0; k<timewise_nstats; ++k) {
        StatSpec& sp = timewise_statspecs[k];
        if (sp.setnum>=stcol.length())
          PLERROR("SequentialValidation::run, trying to access a test set "
                  "(test%d) beyond the last one (test%d)",
                  sp.setnum, stcol.length()-1);
        timewise_res[k] = stcol[sp.setnum]->getStat(sp.intstatname);
      }
      timewise_stats->update(timewise_res);
      for (int k=0; k<timewise_nstats; ++k)
        timewise_res[k] =
          timewise_stats->getStats(k).getStat(timewise_statspecs[k].extstat);
      timewise_stats_vm->appendRow(timewise_res);
    }
  }

  sequence_stats->finalize();

  Vec global_result(nstats);
  for (int k=0; k<nstats; k++)
    global_result[k] = sequence_stats->getStats(k).getStat(statspecs[k].extstat);

  if (global_stats_vm)
    global_stats_vm->appendRow(global_result);
  
  reportStats(global_result);
}

void SequentialValidation::setExperimentDirectory(const string& _expdir)
{
  expdir = _expdir;
  if(provide_learner_expdir)
    learner->setExperimentDirectory(append_slash(expdir)+"Model");
}

void SequentialValidation::reportStats(const Vec& global_result)
{
  if (!report_stats)
    return;
  
  saveAscii(expdir+"global_result.avec", global_result);
//  saveAscii(expdir+"predictions.amat", learner->predictions);
//  saveAscii(expdir+"errors.amat", learner->errors, learner->getTestCostNames());
}

} // end of namespace PLearn

