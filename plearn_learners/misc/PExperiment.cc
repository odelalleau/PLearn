// -*- C++ -*-

// PExperiment.cc
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
   * $Id: PExperiment.cc,v 1.3 2003/05/26 04:12:43 plearner Exp $ 
   ******************************************************* */

/*! \file PExperiment.cc */
#include "PExperiment.h"
#include "pl_io.h"
#include "VecStatsCollector.h"
#include "AsciiVMatrix.h"
#include "FileVMatrix.h"

namespace PLearn <%
using namespace std;

TVec<string> addprepostfix(const string& prefix, const TVec<string>& names, const string& postfix)
{
  TVec<string> newnames(names.size());
  TVec<string>::const_iterator it = names.begin();
  TVec<string>::iterator newit = newnames.begin();
  while(it!=names.end())
    {
      *newit = prefix + *it + postfix;
      ++it;
      ++newit;
    }
  return newnames;
}

template<class T> TVec<T> operator&(const T& x, const TVec<T>& v)
{
  int l = v.size();
  TVec<T> res(1+l);
  res[0] = x;
  res.subVec(1,l) << v;
  return res;
}

PExperiment::PExperiment() 
  : report_stats(true),
    save_initial_experiment(true),
    save_stat_collectors(true),
    save_learners(true),
    save_initial_learners(false),
    save_data_sets(false),
    save_test_outputs(false),
    save_test_costs(false),
    provide_learner_expdir(false)
{}

  IMPLEMENT_NAME_AND_DEEPCOPY(PExperiment);

  void PExperiment::declareOptions(OptionList& ol)
  {
    declareOption(ol, "expdir", &PExperiment::expdir, OptionBase::buildoption,
                  "Path of this experiment's directory in which to save all experiment results.\n"
                  "The directory will be created if it does not already exist.\n"
                  "If this is an empty string, no directory is created and no output file is generated.\n");
    declareOption(ol, "learner", &PExperiment::learner, OptionBase::buildoption,
                  "The learner to train/test");
    declareOption(ol, "dataset", &PExperiment::dataset, OptionBase::buildoption,
                  "The dataset to use for training/testing (will be split by the specified splitter)\n"
                  "You may omit this only if your splitter is an ExplicitSplitter");
    declareOption(ol, "splitter", &PExperiment::splitter, OptionBase::buildoption,
                  "The splitter to use to generate one or several train/test tuples from the dataset.");
    declareOption(ol, "statnames", &PExperiment::statnames, OptionBase::buildoption,
                  "A list of global statistics we are interested in.\n"
                  "These are strings of the form S1[S2[dataset.cost_name]] where:\n"
                  "  - dataset is train or test1 or test2 ... (train being \n"
                  "    the first dataset in a split, test1 the second, ...) \n"
                  "  - cost_name is one of the training or test cost names (depending on dataset) understood \n"
                  "    by the underlying learner (see its getTrainCostNames and getTestCostNames methods) \n" 
                  "  - S1 and S2 are a statistic, i.e. one of: E (expectation), V(variance), MIN, MAX, STDDEV, ... \n"
                  "    S2 is computed over the samples of a given dataset split. S1 is over the splits. \n");
    declareOption(ol, "report_stats", &PExperiment::report_stats, OptionBase::buildoption,
                  "If true, the computed global statistics specified in statnames will be saved in global_stats.pmat \n"
                  "and the corresponding per-split statistics will be saved in split_stats.pmat \n"
                  "For reference, all cost names (as given by the learner's getTrainCostNames() and getTestCostNames() ) \n"
                  "will be reported in files train_cost_names.txt and test_cost_names.txt");
    declareOption(ol, "save_initial_experiment", &PExperiment::save_initial_experiment, OptionBase::buildoption,
                  "If true, this PExperiment object will be saved in its initial state in experiment.psave \n"
                  "Thus if the initial .plearn file gets lost, or modified, we can always see what this experiment was.\n");
    declareOption(ol, "save_stat_collectors", &PExperiment::save_stat_collectors, OptionBase::buildoption,
                  "If true, stat collectors for split#k will be saved in Split#k/train_stats.psave and Split#k/test#i_stats.psave");
    declareOption(ol, "save_learners", &PExperiment::save_learners, OptionBase::buildoption,
                  "If true, the final trained learner for split#k will be saved in Split#k/final_learner.psave");
    declareOption(ol, "save_initial_learners", &PExperiment::save_initial_learners, OptionBase::buildoption,
                  "If true, the initial untrained learner for split#k (just after forget() has been called) will be saved in Split#k/initial_learner.psave");
    declareOption(ol, "save_data_sets", &PExperiment::save_data_sets, OptionBase::buildoption,
                  "If true, the data set generated for split #k will be saved as Split#k/training_set.psave Split#k/test1_set.psave ...");
    declareOption(ol, "save_test_outputs", &PExperiment::save_test_outputs, OptionBase::buildoption,
                  "If true, the outputs of the test for split #k will be saved in Split#k/test#i_outputs.pmat");
    declareOption(ol, "save_test_costs", &PExperiment::save_test_costs, OptionBase::buildoption,
                  "If true, the costs of the test for split #k will be saved in Split#k/test#i_costs.pmat");
    declareOption(ol, "provide_learner_expdir", &PExperiment::provide_learner_expdir, OptionBase::buildoption,
                  "If true, each learner to be trained will have its experiment directory set to Split#k/LearnerExpdir/");
    inherited::declareOptions(ol);
  }

  string PExperiment::help()
  {
    return 
      "The PExperiment class allows you to describe a typical learning experiment that you wish to perform, \n"
      "as a training/testing of a learning algorithm on a particular dataset.\n"
      "The splitter is used to obtain one or several (such as for k-fold) splits of the dataset \n"
      "and training/testing is performed on each split. \n"
      "Requested statistics are computed, and all requested results are written in an appropriate \n"
      "file inside the specified experiment directory. \n";
  }

void PExperiment::build_()
{
}

  // ### Nothing to add here, simply calls build_
  void PExperiment::build()
  {
    inherited::build();
    build_();
  }

void PExperiment::run()
{
  perform(false);
}

Vec PExperiment::perform(bool dont_set_training_set)
{
  if(!learner)
    PLERROR("No leaner specified for PExperiment.");
  if(!splitter)
    PLERROR("No splitter specified for PExperiment");

  if(expdir!="")
    {
      expdir = abspath(expdir);

      if(pathexists(expdir))
        PLERROR("Directory (or file) %s already exists. First move it out of the way.",expdir.c_str());

      // Save this experiment description in the expdir (buildoptions only)
      if(save_initial_experiment)
        PLearn::save(append_slash(expdir)+"experiment.psave", *this, OptionBase::buildoption);
    }

  splitter->setDataSet(dataset);

  int nsplits = splitter->nsplits();
  int nstats = statnames.length();
  TVec<string> testcostnames = learner->getTestCostNames();
  TVec<string> traincostnames = learner->getTrainCostNames();

  // int traincostsize = traincostnames.size();
  int testcostsize = testcostnames.size();
  int outputsize = learner->outputsize();

  // stats for a train on one split 
  VecStatsCollector train_stats;

  // stats for a test on one split 
  VecStatsCollector test_stats;

  VMat global_stats_vm;    // the vmat in which to save global result stats specified in statnames
  VMat split_stats_vm;   // the vmat in which to save per split result stats
  if(expdir!="" && report_stats)
    {
      saveStringInFile(expdir+"train_cost_names.txt", join(traincostnames,"\n")+"\n"); 
      saveStringInFile(expdir+"test_cost_names.txt", join(testcostnames,"\n")+"\n"); 

      global_stats_vm = new FileVMatrix(expdir+"global_stats.pmat", 1, nstats);
      for(int k=0; k<nstats; k++)
        global_stats_vm->declareField(k,statnames[k]);

      split_stats_vm = new FileVMatrix(expdir+"split_stats.pmat", nsplits, 1+nstats);
      split_stats_vm->declareField(0,"splitnum");
      for(int k=0; k<nstats; k++)
        split_stats_vm->declareField(k+1,statnames[k]);
    }

  for(int k=0; k<nsplits; k++)
    {
      string splitdir;
      if(expdir!="")
        splitdir = append_slash(expdir)+"Split"+tostring(k)+"/";

      Array<VMat> dsets = splitter->getSplit(k);
      VMat trainset = dsets[0];
      if(splitdir!="" && save_data_sets)
        PLearn::save(splitdir+"training_set.psave",trainset);

      if(splitdir!="" && provide_learner_expdir)
        learner->setExperimentDirectory(splitdir+"LearnerExpdir/");

      if(!dont_set_training_set || nsplits>1)
        {
          learner->setTrainingSet(trainset);
          learner->forget();
        }

      if(splitdir!="" && save_initial_learners)
        PLearn::save(splitdir+"initial_learner.psave",learner);
      
      train_stats.forget();
      learner->train(train_stats);
      train_stats.finalize();
      if(save_stat_collectors)
        PLearn::save(splitdir+"train_stats.psave",train_stats);
      if(save_learners)
        PLearn::save(splitdir+"final_learner.psave",learner);

      for(int setnum=1; setnum<dsets.length(); setnum++)
        {
          VMat testset = dsets[setnum];
          string setname = "test"+tostring(setnum);
          if(splitdir!="" && save_data_sets)
            PLearn::save(splitdir+setname+"_set.psave",testset);
          VMat test_outputs;
          VMat test_costs;
          if(save_test_outputs)
            test_outputs = new FileVMatrix(splitdir+setname+"_outputs.pmat",0,outputsize);
          if(save_test_costs)
            test_costs = new FileVMatrix(splitdir+setname+"_costs.pmat",0,testcostsize);

          test_stats.forget();
          learner->test(testset, test_stats, test_outputs, test_costs);      
          test_stats.finalize();
          if(save_stat_collectors)
            PLearn::save(splitdir+setname+"_stats.psave",test_stats);
        }
   
      Vec splitres(1,real(k));
      splitres.append(train_stats.getMean());
      splitres.append(train_stats.getStdDev());
      splitres.append(test_stats.getMean());
      splitres.append(test_stats.getStdDev());
      
      // split_stats.update(splitres);
      //      global_results->appendRow(splitres);
    }

  Vec result;

  return result;
}

%> // end of namespace PLearn
