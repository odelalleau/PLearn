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
   * $Id: PExperiment.cc,v 1.1 2003/05/03 05:02:18 plearner Exp $ 
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
  :save_models(true), save_initial_models(false),
   save_test_outputs(false), save_test_costs(false)
  {}

  IMPLEMENT_NAME_AND_DEEPCOPY(PExperiment);

  void PExperiment::declareOptions(OptionList& ol)
  {
    declareOption(ol, "expdir", &PExperiment::expdir, OptionBase::buildoption,
                  "Path of this experiment's directory in which to save all experiment results (will be created if it does not already exist)");
    declareOption(ol, "learner", &PExperiment::learner, OptionBase::buildoption,
                  "The learner to train/test");
    declareOption(ol, "dataset", &PExperiment::dataset, OptionBase::buildoption,
                  "The dataset to use for training/testing (will be split by the specified splitter)\n"
                  "You may omit this only if your splitter is an ExplicitSplitter");
    declareOption(ol, "splitter", &PExperiment::splitter, OptionBase::buildoption,
                  "The splitter to use to generate one or several train/test pairs from the dataset.");
    declareOption(ol, "save_models", &PExperiment::save_models, OptionBase::buildoption,
                  "If true, the final model#k will be saved in Split#k/final.psave");
    declareOption(ol, "save_initial_models", &PExperiment::save_initial_models, OptionBase::buildoption,
                  "If true, the initial model#k (just after forget() has been called) will be saved in Split#k/initial.psave");
    declareOption(ol, "save_test_outputs", &PExperiment::save_test_outputs, OptionBase::buildoption,
                  "If true, the outputs of the test for split #k will be saved in Split#k/test_outputs.pmat");
    declareOption(ol, "save_test_costs", &PExperiment::save_test_costs, OptionBase::buildoption,
                  "If true, the costs of the test for split #k will be saved in Split#k/test_costs.pmat");
    inherited::declareOptions(ol);
  }

  string PExperiment::help() const
  {
    return 
      "The PExperiment class allows you to describe a typical learning experiment that you wish to perform, \n"
      "as a training/testing of a learning algorithm on a particular dataset.\n"
      "Detailed results for each split #k will be saved in sub-directory Split#k of the experiment directory. \n"
      "Final results for each split and basic statistics across all splits will be saved in the results.summary \n"
      "file in the experiment directory.\n"
      + optionHelp();
  }

  void PExperiment::build_()
  {
    splitter->setDataSet(dataset);
  }

  // ### Nothing to add here, simply calls build_
  void PExperiment::build()
  {
    inherited::build();
    build_();
  }

void PExperiment::run()
{
  if(expdir=="")
    PLERROR("No expdir specified for PExperiment.");
  if(!learner)
    PLERROR("No leaner specified for PExperiment.");
  if(!splitter)
    PLERROR("No splitter specified for PExperiment");

  if(PLMPI::rank==0)
    {
      if(pathexists(expdir))
        PLERROR("Directory (or file) %s already exists. First move it out of the way.",expdir.c_str());

      if(!force_mkdir(expdir))
        PLERROR("Could not create experiment directory %s", expdir.c_str());

      // Save this experiment description in the expdir (buildoptions only)
      PLearn::save(append_slash(expdir)+"experiment.psave", *this, OptionBase::buildoption);
    }

  int nsplits = splitter->nsplits();
  TVec<string> testcostnames = learner->getTestCostNames();
  TVec<string> traincostnames = learner->getTrainCostNames();

  // int traincostsize = traincostnames.size();
  int testcostsize = testcostnames.size();
  int outputsize = learner->outputsize();

  // stats for a train on one split 
  VecStatsCollector train_stats;

  // stats for a test on one split 
  VecStatsCollector test_stats;

  // stats over all splits
  VecStatsCollector split_stats;

  // the vmat in which to save results
  VMat global_results;

  // filename
  string fname = append_slash(expdir)+"results.amat";

  // fieldnames 
  TVec<string> fieldnames(1,string("splitnum")); 
  fieldnames.append(addprepostfix("train.",traincostnames,".mean"));
  fieldnames.append(addprepostfix("train.",traincostnames,".stddev"));
  fieldnames.append(addprepostfix("test.",testcostnames,".mean"));
  fieldnames.append(addprepostfix("test.",testcostnames,".stddev"));

  int nfields = fieldnames.size();

  global_results = new AsciiVMatrix(fname, nfields, fieldnames, "# Special values for splitnum are: -1 -> MEAN; -2 -> STDERROR; -3 -> STDDEV");

  for(int k=0; k<nsplits; k++)
    {
      Array<VMat> train_test = splitter->getSplit(k);
      if(train_test.size()!=2) 
        PLERROR("Splitter returned a split with %d subsets, instead of the expected 2: train&test",train_test.size());
      VMat trainset = train_test[0];
      VMat testset = train_test[1];
      string learner_expdir = append_slash(expdir)+"Split"+tostring(k);
      learner->setExperimentDirectory(learner_expdir);

      learner->forget();
      learner->setTrainingSet(trainset);
      if(save_initial_models)
        PLearn::save(learner_expdir+"/initial.psave",learner);
      
      train_stats.forget();
      learner->train(train_stats);
      if(save_models)
        PLearn::save(learner_expdir+"/final.psave",learner);

      VMat test_outputs;
      VMat test_costs;

      if(save_test_outputs)
        test_outputs = new FileVMatrix(learner_expdir+"/test_outputs.pmat",0,outputsize);
      if(save_test_costs)
        test_costs = new FileVMatrix(learner_expdir+"/test_costs.pmat",0,testcostsize);

      test_stats.forget();
      learner->test(testset, test_stats, test_outputs, test_costs);      
   
      Vec splitres(1,real(k));
      splitres.append(train_stats.getMean());
      splitres.append(train_stats.getStdDev());
      splitres.append(test_stats.getMean());
      splitres.append(test_stats.getStdDev());
      
      split_stats.update(splitres);
      global_results->appendRow(splitres);
    }

  // MEAN
  Vec resultrow = split_stats.getMean();
  resultrow[0] = -1;  
  global_results->appendRow(resultrow);

  // STDERROR
  resultrow << split_stats.getStdError();
  resultrow[0] = -2;
  global_results->appendRow(resultrow);

  // STDDEV
  resultrow << split_stats.getStdDev();
  resultrow[0] = -3;
  global_results->appendRow(resultrow);

}

%> // end of namespace PLearn
