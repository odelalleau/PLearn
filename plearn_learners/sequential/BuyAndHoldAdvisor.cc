// -*- C++ -*-

// BuyAndHoldAdvisor.cc
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
   * $Id: BuyAndHoldAdvisor.cc,v 1.1 2003/12/02 15:40:53 dorionc Exp $ 
   ******************************************************* */

// Authors: Christian Dorion

/*! \file BuyAndHoldAdvisor.cc */


#include "BuyAndHoldAdvisor.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_OBJECT(BuyAndHoldAdvisor, "ONE LINE DESCR", "NO HELP");

BuyAndHoldAdvisor::BuyAndHoldAdvisor()
{}

void BuyAndHoldAdvisor::build()
{
  inherited::build();
  build_();
}

void BuyAndHoldAdvisor::build_()
{      
  if(underlying.isNull())
    return;
  
  underlying->trader = trader;
  underlying->horizon = horizon;
  underlying->max_seq_len = max_seq_len;
  underlying->outputsize_ = outputsize_;
  underlying->build();
}

void BuyAndHoldAdvisor::forget()
{
  inherited::forget();
}

void BuyAndHoldAdvisor::declareOptions(OptionList& ol)
{
  declareOption(ol, "underlying", &BuyAndHoldAdvisor::underlying, OptionBase::buildoption,
                " If the underlying FinancialAdvisor is null, the positions will be only ones.\n"
                "If not, the positions will be setted to the first test positions of the\n"
                "underlying advisors. ");

  inherited::declareOptions(ol);
}

void BuyAndHoldAdvisor::train()
{
  last_call_train_t = train_set.length() - 1;
  
  if(last_train_t != -1)
    return;

  // Buy
  last_train_t = last_call_train_t;
  if(underlying.isNull())
    predictions(last_train_t).fill(1.0);
  else
  {
    underlying->setTrainingSet(train_set, false);
    underlying->train();
    predictions.subMatRows(0, last_train_t) << underlying->predictions.subMatRows(0, last_train_t);
    predictions(last_train_t) << underlying->predictions(last_train_t);
    errors(last_train_t) << underlying->errors(last_train_t);
  }

  // Hold
  for(int t = last_train_t+1; t < predictions.length(); t++)
    predictions(t) << predictions(last_train_t);
}

void BuyAndHoldAdvisor::test(VMat testset, PP<VecStatsCollector> test_stats,
                        VMat testoutputs, VMat testcosts) const
{
  last_test_t = testset.length()-1;
}
 
TVec<string> BuyAndHoldAdvisor::getTrainCostNames() const
{
  if(underlying.isNull())
    return TVec<string>(0);
  return underlying->getTrainCostNames();
}

void BuyAndHoldAdvisor::computeOutputAndCosts(const Vec& input,
                                         const Vec& target, Vec& output, Vec& costs) const
{
  if(underlying.isNull())
    PLERROR("BuyAndHoldAdvisor::computeOutputAndCosts -- Nothing yet.");
  underlying->computeOutputAndCosts(input, target, output, costs);
}

void BuyAndHoldAdvisor::computeCostsOnly(const Vec& input, const Vec& target,
                                    Vec& costs) const
{ PLERROR("The method computeCostsOnly is not defined for this BuyAndHoldAdvisor"); }

void BuyAndHoldAdvisor::computeOutput(const Vec& input, Vec& output) const
{ PLERROR("The method computeOutput is not defined for this BuyAndHoldAdvisor"); }

void BuyAndHoldAdvisor::computeCostsFromOutputs(const Vec& input,
                                           const Vec& output, const Vec& target, Vec& costs) const
{ PLERROR("The method computeCostsFromOutputs is not defined for this BuyAndHoldAdvisor"); }

void BuyAndHoldAdvisor::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("BuyAndHoldAdvisor::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
} 

%> // end of namespace PLearn

