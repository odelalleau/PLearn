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
   * $Id: BuyAndHoldAdvisor.cc,v 1.2 2004/01/26 21:07:14 dorionc Exp $ 
   ******************************************************* */

// Authors: Christian Dorion

/*! \file BuyAndHoldAdvisor.cc */


#include "BuyAndHoldAdvisor.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_OBJECT(BuyAndHoldAdvisor, "ONE LINE DESCR", "NO HELP");

BuyAndHoldAdvisor::BuyAndHoldAdvisor():
  did_buy(false),
  first_non_null_position(true)
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

  declareOption(ol, "first_non_null_position", &BuyAndHoldAdvisor::first_non_null_position, OptionBase::buildoption,
                "If the BuyAndHoldAdvisor is based on an underlying advisor, this boolean specifies\n"
                "if the BuyAndHoldAdvisor follows the first non-null position (true) or simply the\n"
                "first position.\n"
                "Default: true.");

  inherited::declareOptions(ol);
}

void BuyAndHoldAdvisor::train_test_core(VMat dataset, int t) const
{  
  if(did_buy)
    return;
  
  // Buy
  if(underlying.isNull())
  {
    predictions.subMatRows(0, t).fill(0.0);
    predictions(t).fill(1.0);
  }
  else
  {
    underlying->setTrainingSet(dataset, false);
    underlying->train();
    predictions.subMatRows(0, t) << underlying->predictions.subMatRows(0, t);
    predictions(t) << underlying->predictions(t);
    errors(t) << underlying->errors(t);
    
    if(first_non_null_position)
    {
      int c=0;
      while(c < predictions.width())
      {
        if( predictions(t, c) != 0 )
          break;
        c++;
      }  
      if(c == predictions.width()) return; // The position is the null vector
    }  
  }

  // The model did buy it's position and will now only hold it
  did_buy = true;

  // Hold
  for(int tt = t+1; tt < predictions.length(); tt++)
    predictions(tt) << predictions(t);
}

void BuyAndHoldAdvisor::train()
{
  last_call_train_t = train_set.length() - 1;
  train_test_core(train_set, last_call_train_t);
  last_train_t = last_call_train_t;  
}

void BuyAndHoldAdvisor::test(VMat testset, PP<VecStatsCollector> test_stats,
                        VMat testoutputs, VMat testcosts) const
{
  last_test_t = testset.length()-1;
  train_test_core(testset, last_test_t);
}
 
TVec<string> BuyAndHoldAdvisor::getTrainCostNames() const
{
  if(underlying.isNull())
    return TVec<string>(1, "BidCost");
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

