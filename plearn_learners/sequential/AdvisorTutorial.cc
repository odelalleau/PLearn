// -*- C++ -*-

// AdvisorTutorial.cc
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
   * $Id: AdvisorTutorial.cc,v 1.2 2003/09/22 20:15:24 dorionc Exp $ 
   ******************************************************* */

// Authors: Christian Dorion

/*! \file AdvisorTutorial.cc */


#include "AdvisorTutorial.h"

namespace PLearn <%
using namespace std;


// *****************************************************************************
// ***** Beggining of tutorial *************************************************

PLEARN_IMPLEMENT_OBJECT(AdvisorTutorial, "Tutorial to write a FuturesTrader advisor (SequentialLearner)", 
                        "A little SequentialLearner that implements the basis of what a FuturesTrader advisor should do.");

/*!
  FuturesTrader assumes advisior's train method computes and sets the advisor state up to the 
   (last_call_train_t)^th row. If (last_train_t < last_call_train_t), the 
   advisor should copy state[last_train_t] to the 
   (last_call_train_t - last_train_t) following lines
*/
void AdvisorTutorial::train()
{ 
  last_call_train_t = train_set.length()-1;
  if( last_train_t != -1 && 
      last_train_t+train_step > last_call_train_t)
  {
    for(int t = last_train_t+1; t <= last_call_train_t; t++)
      for(int k=0; k < state.width(); t++)
        state(t, k) = state(last_train_t, k);
    return;
  }
  
  real initial_cash = 1e06;
  
  // The very first portfolio  
  if(last_train_t == -1)
  {
    state(0, 0) = initial_cash;
    for(int k=1; k < state.width(); k++)
      state(0, k) = 0;
    last_train_t++;
  }

  // Following ones
  real dummy_asset_price = 1.0;
  for(int t=last_train_t+1; t < train_set.length(); t++)
  {
    state(t, 0) = state(t-1, 0) - (state.width()-1)*dummy_asset_price;
    for(int k=1; k < state.width(); k++)
      state(t, k) = state(t-1,k) + 1.0;
  }
  
  last_train_t = last_call_train_t; // Updating because we have trained
}

/*!
  FuturesTrader assumes the advisior test method computes and sets the advisor state up to its 
  (last_test_t)^th row.
*/
void AdvisorTutorial::test(VMat testset, PP<VecStatsCollector> test_stats,
                           VMat testoutputs, VMat testcosts) const
{
  if( testset.length()-1 <= last_test_t || 
      testset.length()-1 <= last_call_train_t )
    return;

  // Could have been done before the first if to avoid having an or (||), 
  //  but it would set last_test_t misleadingly if testset.length()-1 <= last_call_train_t
  //  was true.
  if(last_test_t == -1)
    last_test_t = last_call_train_t;

  real dummy_asset_price = 1.0;
  for(int t=last_test_t+1; t < testset.length(); t++)
  {
    state(t, 0) = state(t-1, 0) - (state.width()-1)*dummy_asset_price;
    for(int k=1; k < state.width(); k++)
      state(t, k) = state(t-1,k) + 1.0;
  }
  
  last_test_t = testset.length()-1;
}

/*!
  Since FuturesTrader::train mainly call its advisor train method
   it is only normal that FuturesTrader::getTrainCostNames calls FT's 
   advisor getTrainCostNames method. Make sure it is properly implemented.
*/
TVec<string> AdvisorTutorial::getTrainCostNames() const
{
  return TVec<string>(1, "AdvisorTutorialCost");
}
// ***** End of tutorial *******************************************************
// *****************************************************************************

/*
  REST OF THE CODE
*/

AdvisorTutorial::AdvisorTutorial()
{
  // ...

  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

void AdvisorTutorial::build()
{
  inherited::build();
  build_();
}

void AdvisorTutorial::build_()
{
  forget();
}

void AdvisorTutorial::forget()
{
  inherited::forget();
}

void AdvisorTutorial::declareOptions(OptionList& ol)
{
  inherited::declareOptions(ol);
}

void AdvisorTutorial::computeOutputAndCosts(const Vec& input,
    const Vec& target, Vec& output, Vec& costs) const
{ PLERROR("The method computeOutputAndCosts is not defined for this AdvisorTutorial"); }

void AdvisorTutorial::computeCostsOnly(const Vec& input, const Vec& target,
    Vec& costs) const
{ PLERROR("The method computeCostsOnly is not defined for this AdvisorTutorial"); }

void AdvisorTutorial::computeOutput(const Vec& input, Vec& output) const
{ PLERROR("The method computeOutput is not defined for this AdvisorTutorial"); }

void AdvisorTutorial::computeCostsFromOutputs(const Vec& input,
    const Vec& output, const Vec& target, Vec& costs) const
{ PLERROR("The method computeCostsFromOutputs is not defined for this AdvisorTutorial"); }

void AdvisorTutorial::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("AdvisorTutorial::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
} 

TVec<string> AdvisorTutorial::getTestCostNames() const
{ return AdvisorTutorial::getTrainCostNames(); }

%> // end of namespace PLearn

