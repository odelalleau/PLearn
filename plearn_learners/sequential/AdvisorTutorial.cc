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
   * $Id: AdvisorTutorial.cc,v 1.5 2004/02/20 21:14:49 chrish42 Exp $ 
   ******************************************************* */

// Authors: Christian Dorion

/*! \file AdvisorTutorial.cc */


#include "AdvisorTutorial.h"

namespace PLearn {
using namespace std;


// *****************************************************************************
// ***** Beggining of tutorial *************************************************

PLEARN_IMPLEMENT_OBJECT(AdvisorTutorial, "Tutorial to write a FuturesTrader advisor (SequentialLearner)", 
                        "A little SequentialLearner that implements the basis of what a FuturesTrader advisor should do.");

/*!
  CLASS TO BE UPDATED
 */
void AdvisorTutorial::train_test_core(const Vec& input, int t, VMat testoutputs, VMat testcosts) const
{
  PLERROR("CLASS TO BE UPDATED AdvisorTutorial");
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

} // end of namespace PLearn

