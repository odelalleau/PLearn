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
#include "VecStatsCollector.h"
#include "AsciiVMatrix.h"
#include "FileVMatrix.h"

namespace PLearn <%
using namespace std;


IMPLEMENT_NAME_AND_DEEPCOPY(SequentialValidation);

SequentialValidation::SequentialValidation()
  : init_train_size(1), expdir(""), save_final_model(true), save_initial_model(false),
    save_data_sets(false), save_test_outputs(false), save_test_costs(false),
    save_stat_collectors(false)
{}

void SequentialValidation::build_()
{}

void SequentialValidation::build()
{
  inherited::build();
  build_();
}

void SequentialValidation::declareOptions(OptionList& ol)
{
  declareOption(ol, "expdir", &SequentialValidation::expdir,
    OptionBase::buildoption, "Path of this experiment's directory in which to save all experiment results (will be created if it does not already exist). \n");

  declareOption(ol, "learner", &SequentialValidation::learner,
    OptionBase::buildoption, "The SequentialLearner to train/test. \n");

  declareOption(ol, "dataset", &SequentialValidation::dataset,
    OptionBase::buildoption, "The dataset to use for training/testing. \n");

  declareOption(ol, "init_train_size", &SequentialValidation::init_train_size,
    OptionBase::buildoption, "Size of the first training set. \n");

  declareOption(ol, "save_final_model", &SequentialValidation::save_final_model,
    OptionBase::buildoption, "If true, the final model will be saved in model.psave \n");

  declareOption(ol, "save_initial_model", &SequentialValidation::save_initial_model,
    OptionBase::buildoption, "If true, the initial model will be saved in initial_model.psave. \n");

  declareOption(ol, "save_data_sets", &SequentialValidation::save_data_sets,
    OptionBase::buildoption, "If true, the data sets (train/test) for each split will be saved. \n");

  declareOption(ol, "save_test_outputs", &SequentialValidation::save_test_outputs,
    OptionBase::buildoption, "If true, the outputs of the tests will be saved in test_outputs.pmat \n");

  declareOption(ol, "save_test_costs", &SequentialValidation::save_test_costs,
    OptionBase::buildoption, "If true, the costs of the tests will be saved in test_costs.pmat \n");

  declareOption(ol, "save_stat_collectors", &SequentialValidation::save_stat_collectors, OptionBase::buildoption, "If true, stat collectors of each data sets (train/test) will be saved for each split. \n");

  inherited::declareOptions(ol);
}

string SequentialValidation::help()
{
  return
    "The SequentialValidation class allows you to describe a typical sequential \n"
    "validation experiment that you wish to perform........."
    + optionHelp();
}

void SequentialValidation::run()
{
  if (expdir=="")
    PLERROR("No expdir specified for SequentialValidation.");
  if (!learner)
    PLERROR("No learner specified for SequentialValidation.");

  if(pathexists(expdir))
    PLERROR("Directory (or file) %s already exists. First move it out of the way.", expdir.c_str());

  if(!force_mkdir(expdir))
    PLERROR("Could not create experiment directory %s", expdir.c_str());

  // This is to set inputsize() and targetsize()
  learner->setOnlyTrainingSet(dataset);

  // Save this experiment description in the expdir (buildoptions only)
  PLearn::save(append_slash(expdir)+"sequential_validation.psave", *this, OptionBase::buildoption);

  TVec<string> testcostnames = learner->getTestCostNames();
  TVec<string> traincostnames = learner->getTrainCostNames();
 
  // int traincostsize = traincostnames.size();
  int testcostsize = testcostnames.size();
  int outputsize = learner->outputsize();

  // stats for a test on one split
  PP<VecStatsCollector> train_stats = new VecStatsCollector();
  learner->setTrainStatsCollector(train_stats);

  // stats for a test on one split
  PP<VecStatsCollector> test_stats = new VecStatsCollector();

  // stats over all sequence
  //VecStatsCollector sequence_stats;

  // the vmat in which to save results
  //VMat results;

  saveStringInFile(append_slash(expdir)+"train_cost_names.txt", join(traincostnames,"\n")+"\n");
  saveStringInFile(append_slash(expdir)+"test_cost_names.txt", join(testcostnames,"\n")+"\n");

  // filename
  //string fname = append_slash(expdir)+"results.amat";

  // fieldnames
/*
  TVec<string> fieldnames(1,string("sequence_num"));
  fieldnames.append(addprepostfix("train.",traincostnames,".mean"));
  fieldnames.append(addprepostfix("train.",traincostnames,".stddev"));
  fieldnames.append(addprepostfix("test.",testcostnames,".mean"));
  fieldnames.append(addprepostfix("test.",testcostnames,".stddev"));
*/

  //int nfields = fieldnames.size();

  // the learner horizon
  int horizon = learner->horizon;
 
  //results = new AsciiVMatrix(fname, nfields, fieldnames, "# Special values for sequence_num are: -1 -> MEAN; -2 -> STDERROR; -3 -> STDDEV");

  //string learner_expdir = append_slash(expdir)+"subtrain";
  //learner->setExperimentDirectory(learner_expdir);
  if (save_initial_model)
    PLearn::save(append_slash(expdir)+"initial_learner.psave",learner);

  for (int t=init_train_size; t<=dataset.length()-horizon; t++)
  {
    VMat sub_train = dataset.subMatRows(0,t); // excludes t, last training pair is (t-1-horizon,t-1)
    sub_train->defineSizes(dataset->inputsize(), dataset->targetsize(), dataset->weightsize());
    VMat sub_test = dataset.subMatRows(0, t+horizon);
    sub_test->defineSizes(dataset->inputsize(), dataset->targetsize(), dataset->weightsize());

    string splitdir = append_slash(expdir)+"train_t="+tostring(t)+"/";
    if (save_data_sets)
      PLearn::save(splitdir+"training_set.psave", sub_train);

    // Train
    //learner->forget(); // PAS CERTAIN!  Doit-on faire un forget a chaque t?
    learner->setOnlyTrainingSet(sub_train);
    learner->train();
    //train_stats.finalize();
    if (save_stat_collectors)
      PLearn::save(splitdir+"train_stats.psave",train_stats);
    if (save_final_model)
      PLearn::save(splitdir+"final_learner.psave",learner);

    // Test
    VMat test_outputs;
    VMat test_costs;
    if (save_test_outputs)
      test_outputs = new FileVMatrix(splitdir+"test_outputs.pmat",0,outputsize);
    if (save_test_costs)
      test_costs = new FileVMatrix(splitdir+"test_costs.pmat",0,testcostsize);

    if (save_data_sets)
      PLearn::save(splitdir+"test_set.psave", sub_test);

    learner->test(sub_test, test_stats, test_outputs, test_costs);
    //test_stats.finalize();
    if (save_stat_collectors)
      PLearn::save(splitdir+"test_stats.psave",test_stats);

/*
    Vec sequence_res(1,real(t));
    sequence_res.append(train_stats.getMean());
    sequence_res.append(train_stats.getStdDev());
    sequence_res.append(test_stats.getMean());
    sequence_res.append(test_stats.getStdDev());

    sequence_stats.update(sequence_res);
    results->appendRow(sequence_res);
*/
  }

/*
  // MEAN
  Vec resultrow = sequence_stats.getMean();
  resultrow[0] = -1;
  results->appendRow(resultrow);

  // STDERROR
  resultrow << sequence_stats.getStdError();
  resultrow[0] = -2;
  results->appendRow(resultrow);

  // STDDEV
  resultrow << sequence_stats.getStdDev();
  resultrow[0] = -3;
  results->appendRow(resultrow);
*/

  //(output summary statistics of test performances in model.errors)
}

%> // end of namespace PLearn

