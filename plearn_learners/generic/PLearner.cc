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
   * $Id: PLearner.cc,v 1.6 2003/05/26 04:12:43 plearner Exp $
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
  :inputsize_(0), 
   targetsize_(0), 
   outputsize_(0), 
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
  return "The base class for learning algorithms, which should be the main 'products' of PLearn.\n"
    "Data-sets are seen as matrices whose columns or fields are layed out as \n"
    "follows: a number of input fields, followed by (optional) target fields, \n"
    "followed by (optional) weight fields (to weigh each example).\n";
}

void PLearner::declareOptions(OptionList& ol)
{
  declareOption(ol, "inputsize", &PLearner::inputsize_, OptionBase::buildoption, 
                "dimensionality of input vector (should be same as in trainset)\n");

  declareOption(ol, "targetsize", &PLearner::targetsize_, OptionBase::buildoption, 
                "dimensionality of target (should be same as in trainset)\n");

  declareOption(ol, "weightsize", &PLearner::weightsize_, OptionBase::buildoption, 
                "Should be the same as in trainset: dimensionality of weights (0 unweighted, 1 weighted) \n");

  declareOption(ol, "outputsize", &PLearner::outputsize_, OptionBase::buildoption, 
                "dimensionality of the output vectors produced by this learner\n");

  declareOption(ol, "seed", &PLearner::seed, OptionBase::buildoption, 
                "The initial seed for the random number generator used to initialize this learner's parameters\n"
                "as typically done in the forget() method... \n"
                "With a given seed, forget() should always initialize the parameters to the same values.");

  declareOption(ol, "stage", &PLearner::stage, OptionBase::learntoption, 
                "The current training stage: 0 means untrained, n often means after n epochs or optimization steps, etc...\n"
                "The true meaning is learner-dependant.");

  declareOption(ol, "nstages", &PLearner::nstages, OptionBase::buildoption, 
                "Stage until which train() should train this learner and return.\n");

  declareOption(ol, "report_progress", &PLearner::report_progress, OptionBase::buildoption, 
                "should progress in learning and testing be reported in a ProgressBar.\n");
  declareOption(ol, "verbosity", &PLearner::verbosity, OptionBase::buildoption, 
                "Level of verbosity. If 0 should not write anything on cerr. If >0 may write some info on the steps performed\n");

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

void PLearner::build_()
{
}

void PLearner::build()
{
  inherited::build();
  build_();
}

void PLearner::forget()
{
  stage = 0;
}

PLearner::~PLearner()
{
}

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
                                     Vec& output, Vec& costs)
{
  computeOutput(input, output);
  computeCostsFromOutputs(input, output, target, costs);
}

void PLearner::computeCostsOnly(const Vec& input, const Vec& target,  
                                Vec& costs)
{
  static Vec tmp_output;
  tmp_output.resize(outputsize());
  computeOutputAndCosts(input, target, tmp_output, costs);
}

void PLearner::test(VMat testset, VecStatsCollector& test_stats, 
             VMat testoutputs, VMat testcosts)
{
  int l = testset.length();
  Vec input;
  Vec target;
  real weight;

  Vec output(testoutputs ?outputsize() :0);
  Vec costs(nTrainCosts());

  // testset->defineSizes(inputsize(),targetsize(),weightsize());

  test_stats.forget();

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

      test_stats.update(costs);

      if(pb)
        pb->update(i);
    }

  test_stats.finalize();

  if(pb)
    delete pb;

}


//! Parses a statname of the form "E[E[test1.class_error]]" or "V[ MIN [train.squared_error]]"
//! If the external stat is omitted in statname, it will be assumed to be "E[...]"
//! It will set extat and intstat to be for ex. "E"
//! setnum will be 0 for "train" and 1 for "test1", 2 for "test2", ...
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

%> // end of namespace PLearn

