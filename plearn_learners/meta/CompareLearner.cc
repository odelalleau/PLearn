// -*- C++ -*-

// CompareLearner.cc
//
// Copyright (C) 2004 Hugo Larochelle 
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
   * $Id: CompareLearner.cc,v 1.1 2005/01/06 18:45:11 larocheh Exp $ 
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file CompareLearner.cc */


#include "CompareLearner.h"

namespace PLearn {
using namespace std;

CompareLearner::CompareLearner() 
  : learners(0)
{
  build_();
}

PLEARN_IMPLEMENT_OBJECT(CompareLearner, "Learner that trains and compares different learners ", 
                        "This learner takes different learners, trains them and computes their\n"
                        "common costs in order to compare them. The learners and the names of the \n"
                        "learners are given by the user, and the common costs shared by all the \n" 
                        "learners are also specified by the user. The cost name [LEARNER]::[COST] \n"
                        "is the name of the cost [COST] for the learner named [LEARNER]. If a specifed learner \n"
                        "doesn't use this cost, its cost value is always MISSING_VALUE. Also, \n"
                        "the cost names difference_[COST]_[LEARNER1]_VS_[LEARNER2] and \n"
                        "abs_difference_[COST]_[LEARNER1]_VS_[LEARNER2] are the difference and \n"
                        "absolute difference of [COST]::[LEARNER1] and [COST]::[LEARNER2]. Note that \n"
                        "[LEARNER1] must appear before [LEARNER2] in the learner_names vector."
  );
void CompareLearner::declareOptions(OptionList& ol)
{
  declareOption(ol, "learners", &CompareLearner::learners, OptionBase::buildoption,
                "learners to compare");
  declareOption(ol, "common_costs", &CompareLearner::common_costs, OptionBase::buildoption,
                "common costs of the learners to compare");
  declareOption(ol, "learner_names", &CompareLearner::learner_names, OptionBase::buildoption,
                "names of the learners");

  inherited::declareOptions(ol);
}

void CompareLearner::build_()
{
  n_learners = learners.length();
  if(n_learners > 0)
  {
    learners_outputsize = learners[0]->outputsize();
    if(learner_names.length() != n_learners) PLERROR("Number of learner names is different from number of learners");
    costs_indexes.resize(n_learners,common_costs.length());
    TVec<string> test_costs;
    for(int i=0; i<n_learners; i++)
    {
      learners[i]->build();
      test_costs = learners[i]->getTestCostNames();
      for(int c=0; c<common_costs.length(); c++)
      {
        costs_indexes(i,c) = test_costs.find(common_costs[c]);
      }
    }
  }
}

void CompareLearner::build()
{
  inherited::build();
  build_();
}


void CompareLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(learners, copies);
  deepCopyField(learner_names, copies);
  deepCopyField(common_costs, copies);
  deepCopyField(costs_indexes, copies);

  //PLERROR("CompareLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int CompareLearner::outputsize() const
{
  if(n_learners == 0) return 0;
  else return n_learners * learners_outputsize;
}

void CompareLearner::forget()
{
  for(int i=0; i<learners.length(); i++)
    learners[i]->forget();
}
    
void CompareLearner::train()
{
  for(int i=0; i<n_learners; i++)
    learners[i]->train();
}


void CompareLearner::computeOutput(const Vec& input, Vec& output) const
{
  output.resize(outputsize());
  Vec output_i;
  for(int i=0; i<n_learners; i++)
  {
    output_i = output.subVec(i*learners_outputsize,learners_outputsize);
    learners[i]->computeOutput(input,output_i);
  }
}    

void CompareLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
  Vec one_learner_costs;   
  costs.resize(0);
  Vec output_i;
  for(int i=0; i<n_learners; i++)
  {
    one_learner_costs.resize(learners[i]->getTestCostNames().length());
    output_i = output.subVec(i*learners_outputsize,learners_outputsize);
    learners[i]->computeCostsFromOutputs(input, output_i,target,one_learner_costs);
    for(int c=0; c<common_costs.length(); c++)
    {
      if(costs_indexes(i,c) == -1)
        costs.append(MISSING_VALUE);
      else
        costs.append(one_learner_costs[costs_indexes(i,c)]);
      
    }
  }

  for ( int m1=0; m1 < n_learners; m1++ )
    for ( int m2=(m1+1); m2 < n_learners; m2++ )
      for ( int cc=0; cc < common_costs.length(); cc++ )
      {
        real diff = costs[m1*common_costs.length()+cc]-costs[m2*common_costs.length()+cc];
        costs.append(diff);
        costs.append(fabs(diff));
      }
}                                

TVec<string> CompareLearner::getTestCostNames() const
{
  TVec<string> cost_names(0);

  for(int i=0; i<n_learners; i++)
  {
    for(int c=0; c<common_costs.length(); c++)
    {
      cost_names.append(learner_names[i] + "::" + common_costs[c]);
    }
  }

  for ( int m1=0; m1 < n_learners; m1++ )
    for ( int m2=(m1+1); m2 < n_learners; m2++ )
      for ( int cc=0; cc < common_costs.length(); cc++ )
      {
        string postfix = common_costs[cc] + "_" + learner_names[m1] + "_VS_" + learner_names[m2];        
        cost_names.append( "difference_" + postfix );
        cost_names.append( "abs_difference_" + postfix );
      }

  return cost_names;
}

TVec<string> CompareLearner::getTrainCostNames() const
{
  TVec<string> cost_names(0);
  TVec<string> cost_names_i;
  for(int i=0; i<n_learners; i++)
  {
    cost_names_i = learners[i]->getTrainCostNames();
    for(int c=0; c<cost_names_i.length(); c++)
      cost_names.append(learner_names[i] + "::" + cost_names_i[c]);
  }

  return cost_names;
}

void CompareLearner::setTrainStatsCollector(PP<VecStatsCollector> statscol)
{ 
  PP<VecStatsCollector> stat_i;
  for(int i=0; i<n_learners; i++)
  {
    stat_i = new VecStatsCollector();
    learners[i]->setTrainStatsCollector(stat_i); 
  }
}

void CompareLearner::setTrainingSet(VMat training_set, bool call_forget)
{ 
  for(int i=0; i<n_learners; i++)
  {
    learners[i]->setTrainingSet(training_set,call_forget);
  }
  if (call_forget)
    forget();
}

void CompareLearner::setValidationSet(VMat validset)
{ 
  for(int i=0; i<n_learners; i++)
  {
    learners[i]->setValidationSet(validset);
  }
}

void CompareLearner::setExperimentDirectory(const string& the_expdir) 
{ 
  if(the_expdir=="")
  {
    expdir = "";
    for(int i=0; i<n_learners; i++)
    {
      learners[i]->setExperimentDirectory(the_expdir);
    }
  }
  else
    {
      if(!force_mkdir(the_expdir))
        PLERROR("In PLearner::setExperimentDirectory Could not create experiment directory %s",the_expdir.c_str());
      expdir = abspath(the_expdir);
      string learner_expdir;
      for(int i=0; i<n_learners; i++)
      {
        learner_expdir = abspath(the_expdir + "_" + learner_names[i]);
        learners[i]->setExperimentDirectory(learner_expdir); 
      }
    }
}

} // end of namespace PLearn
