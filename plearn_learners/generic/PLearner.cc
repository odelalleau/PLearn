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
   * $Id: PLearner.cc,v 1.3 2003/05/05 21:18:14 plearner Exp $
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
  :inputsize_(0), targetsize_(0), outputsize_(0), weightsize_(0), 
   seed(0), stage(0), nstages(1)
{}

IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY(PLearner);
void PLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  Object::makeDeepCopyFromShallowCopy(copies);
  //deepCopyField(test_sets, copies);
  //deepCopyField(measurers, copies);
}

void PLearner::declareOptions(OptionList& ol)
{
  declareOption(ol, "inputsize", &PLearner::inputsize_, OptionBase::buildoption, 
                "dimensionality of input vector \n");

  declareOption(ol, "outputsize", &PLearner::outputsize_, OptionBase::buildoption, 
                "dimensionality of output \n");

  declareOption(ol, "targetsize", &PLearner::targetsize_, OptionBase::buildoption, 
                "dimensionality of target \n");

  declareOption(ol, "weightsize", &PLearner::weightsize_, OptionBase::buildoption, 
                "Number of weights within target.  The last 'weightsize' fields of the target vector will be used as cost weights.\n"
                "This is usually 0 (no weight) or 1 (1 weight per sample). Special loss functions may be able to give a meaning\n"
                "to weightsize>1. Not all learners support weights.");

  declareOption(ol, "seed", &PLearner::seed, OptionBase::buildoption, 
                "The initial seed for the random number generator used to initialize this learner's parameters\n"
                "as typically done in the forget() method... \n"
                "With a given seed, forget() should always initialize the parameters to the same values.");

  declareOption(ol, "stage", &PLearner::stage, OptionBase::buildoption, 
                "The current training stage: 0 means untrained, n often means after n epochs or optimization steps, etc...\n"
                "The true meaning is learner-dependant.");

  declareOption(ol, "nstages", &PLearner::nstages, OptionBase::buildoption, 
                "Stage until which train() should train this learner and return.\n");

  inherited::declareOptions(ol);
}


void PLearner::setExperimentDirectory(const string& the_expdir) 
{ 
#if USING_MPI
  if(PLMPI::rank==0) {
#endif
  if(!force_mkdir(the_expdir))
  {
    PLERROR("In PLearner::setExperimentDirectory Could not create experiment directory %s",the_expdir.c_str());}
#if USING_MPI
  }
#endif
  expdir = abspath(the_expdir);
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
                                
void PLearner::computeOutputAndCosts(const VVec& input, VVec& target, const VVec& weight,
                           Vec& output, Vec& costs)
{
  computeOutput(input, output);
  computeCostsFromOutputs(input, output, target, weight, costs);
}

void PLearner::computeCostsOnly(const VVec& input, VVec& target, VVec& weight, 
                  Vec& costs)
{
  static Vec tmp_output;
  tmp_output.resize(outputsize());
  computeOutputAndCosts(input, target, weight, tmp_output, costs);
}

void PLearner::test(VMat testset, VecStatsCollector& test_stats, 
             VMat testoutputs, VMat testcosts)
{
  int l = testset.length();
  VVec input;
  VVec target;
  VVec weight;

  Vec output(testoutputs ?outputsize() :0);
  Vec costs(nTrainCosts());

  testset->defineSizes(inputsize(),targetsize(),weightsize());

  test_stats.forget();

  for(int i=0; i<l; i++)
    {
      testset.getSample(i, input, target, weight);

      if(testoutputs)
        {
          computeOutputAndCosts(input, target, weight, output, costs);
          testoutputs->putOrAppendRow(i,output);
        }
      else // no need to compute outputs
        computeCostsOnly(input, target, weight, costs);

      if(testcosts)
        testcosts->putOrAppendRow(i, costs);

      test_stats.update(costs);
    }

  test_stats.finalize();
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
  else if(dset.substr(0,4)=="test")
    {
      setnum = toint(dset.substr(4));
      if(setnum<=0)
        PLERROR("In parse_statname: parse error for %s",statname.c_str());        
    }
  else
    PLERROR("In parse_statname: parse error for %s",statname.c_str());

  errorname = set_cost.second;
}

Vec trainTestLearner(PP<PLearner> learner, const VMat &dataset, PP<Splitter> splitter, TVec<string> statnames)
{
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
      VecStatsCollector& st = stats[0];
      learner->setTrainingSet(split[0]);
      st.forget();
      learner->train(st);
      st.finalize();

      // tests
      for(int m=1; m<split.size(); m++)
        {
          VMat testset = split[m];
          VecStatsCollector& st = stats[m];
          st.forget();
          learner->test(testset,st);
          st.finalize();
        }

      for(int i=0; i<nst; i++)
        results[i] = stats[setnum[i]].getStats(costindex[i]).getStat(intstat[i]);

      globalstats.update(results);
    }

  globalstats.finalize();

  for(int i=0; i<nst; i++)
    results[i] = globalstats.getStats(i).getStat(extstat[i]);

  return results;
}

%> // end of namespace PLearn

