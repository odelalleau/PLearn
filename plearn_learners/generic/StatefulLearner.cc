// -*- C++ -*-

// StatefulLearner.cc
//
// Copyright (C) 2004 Réjean Ducharme 
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
   * $Id: StatefulLearner.cc,v 1.1 2004/07/19 13:46:39 ducharme Exp $ 
   ******************************************************* */

// Authors: Réjean Ducharme

/*! \file StatefulLearner.cc */


#include "StatefulLearner.h"

namespace PLearn {
using namespace std;

StatefulLearner::StatefulLearner() : current_test_t(-1)
{}

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(StatefulLearner, "PLearner with an internal state", "PLearner with an internal state.\n"
                                                  "It replaces, for efficacity and compatibility reasons, SequentialLearner.");

void StatefulLearner::declareOptions(OptionList& ol)
{
  inherited::declareOptions(ol);
}

void StatefulLearner::build_()
{}

// ### Nothing to add here, simply calls build_
void StatefulLearner::build()
{
  inherited::build();
  build_();
}

void StatefulLearner::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

void StatefulLearner::computeOutput(const Vec& input, Vec& output) const
{
  PLWARNING("You called StatefulLearner::computeOutput(...), are you sure you don't want to use computeOutputAndCosts(...) instead???");

  static Vec tmp_target;
  static Vec tmp_costs;
  tmp_target.resize(targetsize());
  tmp_costs.resize(nTestCosts());
  computeOutputAndCosts(input, tmp_target, output, tmp_costs);
}

void StatefulLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
    const Vec& target, Vec& costs) const
{
  PLERROR("The method computeCostsFromOutputs is not defined and has no meaning for a StatefulLearner");
}                                

void StatefulLearner::computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const
{
  PLWARNING("You called StatefulLearner::computeCostsOnly(...), are you sure you don't want to use computeOutputAndCosts(...) instead???");

  static Vec tmp_output;
  tmp_output.resize(outputsize());
  computeOutputAndCosts(input, target, tmp_output, costs);
}

void StatefulLearner::setTrainingSet(VMat training_set, bool call_forget)
{
  train_set = training_set;
  if (call_forget) forget();
}

} // end of namespace PLearn
