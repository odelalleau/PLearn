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
   * $Id: BuyAndHoldAdvisor.cc,v 1.5 2004/02/24 18:48:50 dorionc Exp $ 
   ******************************************************* */

// Authors: Christian Dorion

/*! \file BuyAndHoldAdvisor.cc */


#include "BuyAndHoldAdvisor.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(BuyAndHoldAdvisor, "ONE LINE DESCR", "NO HELP");

BuyAndHoldAdvisor::BuyAndHoldAdvisor():
  bought(-1)
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

void BuyAndHoldAdvisor::build_from_train_set()
{
  int t = train_set.length()-1;
#ifdef DEBUG
  cout << "BuyAndHoldAdvisor::build_from_train_set -- t = " << t << endl;
#endif

  predictions.subMatRows(0, t).fill(0.0);

  // Buy
  if(underlying.isNull())
  {
    predictions(t).fill(1.0);
    bought = t;
    errors(t, 0) = bought;
  }

  last_train_t = t-train_step;
  build_complete = true;
}

void BuyAndHoldAdvisor::train_test_core(const Vec& input, int t, VMat testoutputs, VMat testcosts) const
{
  if(bought >= 0)
  {
    // Hold
    predictions(t) << predictions(t-1);
    return;
  }
    
  // We have an underlying which did not provide any position yet.
  if(trader->monthly_version() && !trader->is_last_day_of_month(t))
  {
    predictions(t).fill(0.0);
    return;
  }
  
  // We have an underlying which is ready to take a position.
  underlying->setTrainingSet( trader->viewInternal(), false );
  underlying->train();
  predictions(t) << underlying->predictions(t);
  errors(t) << underlying->errors(t);
  bought = t;
  
#ifdef DEBUG
  cout << "BuyAndHoldAdvisor::train_test_core -- t = " << t << endl;
  cout << predictions(t) << endl << endl;
#endif
}

TVec<string> BuyAndHoldAdvisor::getTrainCostNames() const
{
  if(underlying.isNull())
    return TVec<string>(1, "Bought");
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

} // end of namespace PLearn

