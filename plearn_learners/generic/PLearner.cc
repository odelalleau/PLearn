// -*- C++ -*-

// PLearner.cc
//
// Copyright (C) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio, Nicolas Chapados, Charles Dugas, Rejean Ducharme, Universite de Montreal
// Copyright (C) 2001,2002 Francis Pieraut, Jean-Sebastien Senecal
// Copyright (C) 2002 Frederic Morin, Xavier Saint-Mleux, Julien Keable
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
   * $Id: PLearner.cc,v 1.7 2003/06/03 14:52:10 plearner Exp $
   ******************************************************* */

#include "PLearner.h"
#include "TmpFilenames.h"
#include "fileutils.h"
#include "stringutils.h"
#include "MPIStream.h"
#include "FileVMatrix.h"
#include "RemoveRowsVMatrix.h"
#include "PLMPI.h"

namespace PLearn <%
using namespace std;

PLearner::PLearner()
  :
   seed(0), 
   stage(0), nstages(1),
   report_progress(true),
   verbosity(1)
{}

IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY(PLearner);
void PLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  Object::makeDeepCopyFromShallowCopy(copies);
  //deepCopyField(test_sets, copies);
  //deepCopyField(measurers, copies);
}

string PLearner::help()
{
  return "The base class for learning algorithms, which should be the main 'products' of PLearn.\n";
}

void PLearner::declareOptions(OptionList& ol)
{
  declareOption(ol, "expdir", &PLearner::expdir, OptionBase::buildoption, 
                "Path of the directory associated with this learner, in which\n"
                "it should save any file it wishes to create. \n"
                "The directory will be created if it does not already exist.\n"
                "If expdir is the empty string (the default), then the learner \n"
                "should not create *any* file. Note that, anyway, most file creation and \n"
                "reporting are handled at the level of the PTester class rather than \n"
                "at the learner's. \n");

  declareOption(ol, "train_set", &PLearner::train_set, OptionBase::buildoption,
                "The dataset this learner is trained on. \n"
                "You don't have to set this option, if your learner \n"
                "is embedded in an Experiment or similar class, \n"
                "as the Experiment will set the train_set it wants to use.\n"
                "Data-sets are seen as matrices whose columns or fields are layed out as \n"
                "follows: a number of input fields, followed by (optional) target fields, \n"
                "followed by a (optional) weight field (to weigh each example).\n"
                "The sizes of those areas are given by the VMatrix options \n"
                "inputsize targetsize, and weightsize, which are typically used by the \n"
                "learner upon building\n");

  declareOption(ol, "train_stats", &PLearner::train_stats, OptionBase::buildoption,
                "The stats_collector responsible for collecting train cost statistics during training. \n"
                "You don't have to provide this manually. It is typically set by some external \n"
                "training harness that wants to collect some stats. \n");

  declareOption(ol, "seed", &PLearner::seed, OptionBase::buildoption, 
                "The initial seed for the random number generator used to initialize this learner's parameters\n"
                "as typically done in the forget() method... \n"
                "With a given seed, forget() should always initialize the parameters to the same values.");

  declareOption(ol, "stage", &PLearner::stage, OptionBase::learntoption, 
                "The current training stage, since last fresh initialization (forget()): \n"
                "0 means untrained, n often means after n epochs or optimization steps, etc...\n"
                "The true meaning is learner-dependant."
                "You should never modify this option directly!"
                "It is the role of forget() to bring it back to 0,\n"
                "and the role of train() to bring it up to 'nstages'...");

  declareOption(ol, "nstages", &PLearner::nstages, OptionBase::buildoption, 
                "The stage until which train() should train this learner and return.\n"
                "The meaning of 'stage' is learner-dependent, but learner's whose \n"
                "training is incremental (such as involving incremental optimization), \n"
                "it is typically synonym with the number of 'epochs', i.e. the number \n"
                "of passages of the optimization process through the whole training set, \n"
                "since the last fresh initialisation.");

  declareOption(ol, "report_progress", &PLearner::report_progress, OptionBase::buildoption, 
                "should progress in learning and testing be reported in a ProgressBar.\n");

  declareOption(ol, "verbosity", &PLearner::verbosity, OptionBase::buildoption, 
                "Level of verbosity. If 0 should not write anything on cerr. \n"
                "If >0 may write some info on the steps performed along the way.\n"
                "The level of details written should depend on this value.");

  inherited::declareOptions(ol);
}


void PLearner::setExperimentDirectory(const string& the_expdir) 
{ 
  if(the_expdir=="")
    expdir = "";
  else
    {
      if(!force_mkdir(the_expdir))
        PLERROR("In PLearner::setExperimentDirectory Could not create experiment directory %s",the_expdir.c_str());
      expdir = abspath(the_expdir);
    }
}

void PLearner::setTrainingSet(VMat training_set)
{ 
  train_set = training_set; 
  build(); forget();
}

//! Returns train_set->inputsize()
int PLearner::inputsize() const
{ 
  if(!train_set) 
    PLERROR("Must specify a training set before calling PLearner::inputsize()"); 
  return train_set->inputsize(); 
}

//! Returns train_set->targetsize()
int PLearner::targetsize() const 
{ 
  if(!train_set) 
    PLERROR("Must specify a training set before calling PLearner::targetsize()"); 
  return train_set->targetsize(); 
}

void PLearner::build_()
{
  if(expdir!="")
    {
      if(!force_mkdir(expdir))
        PLERROR("In PLearner Could not create experiment directory %s",expdir.c_str());
      expdir = abspath(expdir);
    }
}

void PLearner::build()
{
  inherited::build();
  build_();
}

PLearner::~PLearner()
{
}

int PLearner::nTestCosts() const 
{ return getTestCostNames().size(); }

int PLearner::nTrainCosts() const 
{ return getTrainCostNames().size(); }

int PLearner::getTestCostIndex(const string& costname) const
{
  TVec<string> costnames = getTestCostNames();
  for(int i=0; i<costnames.length(); i++)
    if(costnames[i]==costname)
      return i;
  return -1;
}

int PLearner::getTrainCostIndex(const string& costname) const
{
  TVec<string> costnames = getTrainCostNames();
  for(int i=0; i<costnames.length(); i++)
    if(costnames[i]==costname)
      return i;
  return -1;
}
                                
void PLearner::computeOutputAndCosts(const Vec& input, const Vec& target, 
                                     Vec& output, Vec& costs) const
{
  computeOutput(input, output);
  computeCostsFromOutputs(input, output, target, costs);
}

void PLearner::computeCostsOnly(const Vec& input, const Vec& target,  
                                Vec& costs) const
{
  static Vec tmp_output;
  tmp_output.resize(outputsize());
  computeOutputAndCosts(input, target, tmp_output, costs);
}

void PLearner::run()
{
  if(verbosity<1)
    train();
  else  // verbosity >=1    
    {
      cerr << "Training learner of type " << classname() 
           << " from stage " << stage << " to " << nstages << "..." << endl;
      train();
      cerr << "*** Training finished at stage " << stage << " *** " << endl;
      cerr << "Final mean train costs: " << endl;
      cerr << getTrainCostNames() << endl;
      cerr << train_stats->getMean() << endl;
      cerr << "-----------------------------------------------" << endl;
    }
}

void PLearner::test(VMat testset, PP<VecStatsCollector> test_stats, 
             VMat testoutputs, VMat testcosts) const
{
  int l = testset.length();
  Vec input;
  Vec target;
  real weight;

  Vec output(testoutputs ?outputsize() :0);
  Vec costs(nTrainCosts());

  // testset->defineSizes(inputsize(),targetsize(),weightsize());

  if(test_stats)
    test_stats->forget();

  ProgressBar* pb;
  if(report_progress)
    pb = new ProgressBar("Testing learner",l);


  for(int i=0; i<l; i++)
    {
      testset.getExample(i, input, target, weight);

      if(testoutputs)
        {
          computeOutputAndCosts(input, target, output, costs);
          testoutputs->putOrAppendRow(i,output);
        }
      else // no need to compute outputs
        computeCostsOnly(input, target, costs);

      if(testcosts)
        testcosts->putOrAppendRow(i, costs);

      if(test_stats)
        test_stats->update(costs);

      if(pb)
        pb->update(i);
    }

  if(test_stats)
    test_stats->finalize();

  if(pb)
    delete pb;

}


//! Parses a statname of the form "E[E[test1.class_error]]" or "V[ MIN [train.squared_error]]"
//! If the external stat is omitted in statname, it will be assumed to be "E[...]"
//! It will set extat and intstat to be for ex. "E"
//! setnum will be 0 for "train" and 1 for "test1", 2 for "test2", ...

/*
void parse_statname(const string& statname, string& extstat, string& intstat, int& setnum, string& errorname)
{
  vector<string> tokens = split(removeallblanks(statname), "[]");
  string set_and_cost;
  
  if(tokens.size()==2)
    {
      extstat = "E";
      intstat = tokens[0];
      set_and_cost = tokens[1];
    }
  else if(tokens.size()==3)
    {
      extstat = tokens[0];
      intstat = tokens[1];
      set_and_cost = tokens[2];
    }
  else
    PLERROR("In parse_statname: parse error for %s",statname.c_str());

  if(set_and_cost.length()<5)
    PLERROR("In parse_statname: parse error for %s",statname.c_str());

  pair<string,string> set_cost = split_on_first(set_and_cost,".");
  
  string dset = set_cost.first;
  if(dset=="train")
    setnum = 0;
  else if(dset=="test")
    setnum = 1;
  else if(dset.substr(0,4)=="test")
    {
      setnum = toint(dset.substr(4));
      if(setnum==0)
        PLERROR("In parse_statname: use the name train instead of test0.\n"
                "The first set of a split is the training set. The following are test sets named test1 test2 ..."); 
      if(setnum<=0)
        PLERROR("In parse_statname: parse error for %s",statname.c_str());        
    }
  else
    PLERROR("In parse_statname: parse error for %s",statname.c_str());

  errorname = set_cost.second;
}
*/

/*
Vec trainTestLearner(PP<PLearner> learner, const VMat &dataset, PP<Splitter> splitter, TVec<string> statnames)
{
  splitter->setDataSet(dataset);
  int nsp = splitter->nsplits(); 
  int nst = statnames.length();
  Vec results(nst);

  // First parse the desired statnames

  TVec<string> extstat(nst);
  TVec<string> intstat(nst);
  TVec<int> costindex(nst);
  TVec<int> setnum(nst);

  for(int i=0; i<nst; i++)
    {          
      string costname;
      parse_statname(statnames[i], extstat[i], intstat[i], setnum[i], costname);
      if(setnum==0) // train cost
        costindex[i] = learner->getTrainCostIndex(costname);
      else // test cost
        costindex[i] = learner->getTestCostIndex(costname);
    }

  // The global stats collector
  VecStatsCollector globalstats;

  // Now train/test and collect stats
  TVec<VecStatsCollector> stats;

  for(int k=0; k<nsp; k++)
    {
      Array<VMat> split = splitter->getSplit(k);
      stats.resize(split.size());

      // train
      VMat trainset = split[0];
      VecStatsCollector& st = stats[0];
      st.forget();
      learner->setTrainingSet(trainset);
      learner->setTrainStatsCollector();
      if(trainset.length()>0)
        learner->train(st);
      st.finalize();

      // tests
      for(int m=1; m<split.size(); m++)
        {
          VMat testset = split[m];
          VecStatsCollector& st = stats[m];
          st.forget();
          if(testset.length()>0)
            learner->test(testset,st);
          st.finalize();
        }

      for(int i=0; i<nst; i++)
        {
          int setn = setnum[i];
          if(setn>=stats.size() || stats[setn].size()==0)
            results[i] = MISSING_VALUE;
          else
            results[i] = stats[setn].getStats(costindex[i]).getStat(intstat[i]);
        }

      globalstats.update(results);
    }

  globalstats.finalize();

  for(int i=0; i<nst; i++)
    results[i] = globalstats.getStats(i).getStat(extstat[i]);

  return results;
}
*/


%> // end of namespace PLearn

