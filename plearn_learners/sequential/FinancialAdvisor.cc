// -*- C++ -*-

// FinancialAdvisor.cc
//
// Copyright (C) 2003 Christian Dorion 
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
   * $Id: FinancialAdvisor.cc,v 1.5 2004/02/20 21:14:49 chrish42 Exp $ 
   ******************************************************* */

// Authors: Christian Dorion

/*! \file FinancialAdvisor.cc */


#include "FinancialAdvisor.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_ABSTRACT_OBJECT(FinancialAdvisor, "ONE LINE DESCR", "NO HELP");

FinancialAdvisor::FinancialAdvisor():
  trader(0),
  build_complete(false)
{}

void FinancialAdvisor::build()
{
  inherited::build();
  build_();
}

void FinancialAdvisor::build_()
{      
}

void FinancialAdvisor::build_from_train_set()
{      
}

void FinancialAdvisor::forget()
{
  inherited::forget();
}

void FinancialAdvisor::declareOptions(OptionList& ol)
{
  inherited::declareOptions(ol);
}

void FinancialAdvisor::train()
{
  if (!build_complete)
    build_from_train_set();

  last_call_train_t = train_set.length()-1;
  int start_t = MAX(last_train_t+train_step, last_test_t+train_step);
  if( last_call_train_t < start_t )
    return;

  ProgressBar* pb = NULL;
  if (report_progress)
    pb = new ProgressBar("Training " + classname() + " learner",train_set.length()-start_t);

  Vec input(train_set->inputsize()), target(train_set->targetsize());
  real w=1.0;
  for (int t=start_t; t<train_set.length(); t++)
  {
    train_set->getExample(t, input, target, w);
    train_test_core(input, t);
    last_train_t = t;
    if (pb) pb->update(t-start_t);
  }
//  last_call_train_t = train_set.length()-1;
  if (pb) delete pb;
}

void FinancialAdvisor::test(VMat testset, PP<VecStatsCollector> test_stats,
    VMat testoutputs, VMat testcosts) const
{
  int start_t = MAX(last_train_t+1,last_test_t+1);
  if( testset.length()-1 < start_t )
    return;
 
  ProgressBar* pb = NULL;
  if (report_progress)
    pb = new ProgressBar("Testing " + classname() + " learner",testset.length()-start_t);

  Vec input(testset->inputsize()), target(testset->targetsize());
  real w=1.0;
  for (int t=start_t; t<testset.length(); t++)
  {
    testset->getExample(t, input, target, w);
    train_test_core(input, t, testoutputs, testcosts);
    test_stats->update(errors(t));
    if (pb) pb->update(t-start_t);
  }
  last_test_t = testset.length()-1;
  if (pb) delete pb;
}


TVec<string> FinancialAdvisor::getTestCostNames() const
{
  return getTrainCostNames();
}

void FinancialAdvisor::computeOutputAndCosts(const Vec& input,
    const Vec& target, Vec& output, Vec& costs) const
{ PLERROR("The method computeOutputAndCosts is not defined for this FinancialAdvisor"); }

void FinancialAdvisor::computeCostsOnly(const Vec& input, const Vec& target,
    Vec& costs) const
{ PLERROR("The method computeCostsOnly is not defined for this FinancialAdvisor"); }

void FinancialAdvisor::computeOutput(const Vec& input, Vec& output) const
{ PLERROR("The method computeOutput is not defined for this FinancialAdvisor"); }

void FinancialAdvisor::computeCostsFromOutputs(const Vec& input,
    const Vec& output, const Vec& target, Vec& costs) const
{ PLERROR("The method computeCostsFromOutputs is not defined for this FinancialAdvisor"); }

void FinancialAdvisor::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
} 

} // end of namespace PLearn

