// -*- C++ -*-

// EmbeddedLearner.cc
// 
// Copyright (C) 2002 Frederic Morin
// Copyright (C) 2003 Pascal Vincent
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
   * $Id: EmbeddedLearner.cc,v 1.14 2004/10/06 05:43:30 chapados Exp $ 
   ******************************************************* */

/*! \file EmbeddedLearner.cc */
#include "EmbeddedLearner.h"

namespace PLearn {
using namespace std;

// ###### EmbeddedLearner ######################################################

PLEARN_IMPLEMENT_OBJECT(
  EmbeddedLearner,
  "Wraps an underlying learner", 
  "EmbeddedLearner implements nothing but forwarding \n"
  "calls to an underlying learner. It is typically used as\n"
  "baseclass for learners that are built on top of another learner");

EmbeddedLearner::EmbeddedLearner()
{ }

void EmbeddedLearner::declareOptions(OptionList& ol)
{
  declareOption(ol, "learner", &EmbeddedLearner::learner_, OptionBase::buildoption,
                "The embedded learner");
  inherited::declareOptions(ol);
}

void EmbeddedLearner::build_()
{
  if (!learner_)
    PLERROR("EmbeddedLearner::_build() - learner_ attribute is NULL");

  learner_->build();
}

void EmbeddedLearner::build()
{
  inherited::build();
  build_();
}
   
int EmbeddedLearner::inputsize() const
{
  assert( learner_ );
  return learner_->inputsize();
}

int EmbeddedLearner::targetsize() const
{
  assert( learner_ );
  return learner_->targetsize();
}

int EmbeddedLearner::outputsize() const
{
  assert( learner_ );
  return learner_->outputsize();
}

void EmbeddedLearner::forget()
{
  assert( learner_ );
  learner_->forget();
}

void EmbeddedLearner::train()
{
  assert( learner_ );
  learner_->train();
}

void EmbeddedLearner::computeOutput(const Vec& input, Vec& output) const
{ 
  assert( learner_ );
  learner_->computeOutput(input, output); 
}

void EmbeddedLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                              const Vec& target, Vec& costs) const
{ 
  assert( learner_ );
  learner_->computeCostsFromOutputs(input, output, target, costs); 
}
                                                      
void EmbeddedLearner::computeOutputAndCosts(const Vec& input, const Vec& target,
                                            Vec& output, Vec& costs) const
{ 
  assert( learner_ );
  learner_->computeOutputAndCosts(input, target, output, costs); 
}

void EmbeddedLearner::computeCostsOnly(const Vec& input, const Vec& target,
                                       Vec& costs) const
{
  assert( learner_ );
  learner_->computeCostsOnly(input, target, costs);
}

void EmbeddedLearner::use(VMat testset, VMat outputs) const
{
  assert( learner_ );
  learner_->use(testset, outputs);
}

void EmbeddedLearner::useOnTrain(Mat& outputs) const
{
  assert( learner_ );
  learner_->useOnTrain(outputs);
}

void EmbeddedLearner::test(VMat testset, PP<VecStatsCollector> test_stats, 
                           VMat testoutputs, VMat testcosts) const
{
  assert( learner_ );
  learner_->test(testset, test_stats, testoutputs, testcosts);
}

TVec<string> EmbeddedLearner::getTestCostNames() const
{
  assert( learner_ );
  return learner_->getTestCostNames();
}

TVec<string> EmbeddedLearner::getTrainCostNames() const
{
  assert( learner_ );
  return learner_->getTrainCostNames();
}

int EmbeddedLearner::nTestCosts() const
{
  assert( learner_ );
  return learner_->nTestCosts();
}

int EmbeddedLearner::nTrainCosts() const
{
  assert( learner_ );
  return learner_->nTrainCosts();
}

void EmbeddedLearner::resetInternalState()
{
  assert( learner_ );
  learner_->resetInternalState();
}

bool EmbeddedLearner::isStatefulLearner() const
{
  assert( learner_ );
  return learner_->isStatefulLearner();
}

void EmbeddedLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  deepCopyField(learner_, copies);    
}

} // end of namespace PLearn
