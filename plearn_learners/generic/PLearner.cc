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
   * $Id: PLearner.cc,v 1.2 2003/05/03 05:02:18 plearner Exp $
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


%> // end of namespace PLearn

