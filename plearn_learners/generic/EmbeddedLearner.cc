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
   * $Id: EmbeddedLearner.cc,v 1.22 2005/02/08 21:54:20 tihocan Exp $ 
   ******************************************************* */

/*! \file EmbeddedLearner.cc */
#include "EmbeddedLearner.h"
#include <assert.h>

#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;

// ###### EmbeddedLearner ######################################################

PLEARN_IMPLEMENT_OBJECT(
  EmbeddedLearner,
  "Wraps an underlying learner", 
  "EmbeddedLearner implements nothing but forwarding \n"
  "calls to an underlying learner. It is typically used as\n"
  "baseclass for learners that are built on top of another learner.\n"
  "Note that only the NECESSARY member functions are forwarded to\n"
  "the embedded learner; for others, we rely on the base class\n"
  "implementation (which themselves call forwarded functions).\n"
  "This makes it easier to override only a few select functions.");

EmbeddedLearner::EmbeddedLearner(string expdir_append_)
  : learner_(0),
    expdir_append(expdir_append_)
{ }

void EmbeddedLearner::declareOptions(OptionList& ol)
{
  declareOption(ol, "learner", &EmbeddedLearner::learner_,
                OptionBase::buildoption,
                "The embedded learner");

  declareOption(ol, "expdir_append", &EmbeddedLearner::expdir_append,
                OptionBase::buildoption,
                "A string which should be appended to the expdir for the inner learner;"
                "default = \"\".");
  
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

void EmbeddedLearner::setTrainingSet(VMat training_set, bool call_forget)
{
  assert( learner_ );
  bool training_set_has_changed = !train_set || !(train_set->looksTheSameAs(training_set));
  // If 'call_forget' is true, learner_->forget() will be called
  // in this->forget() (called by PLearner::setTrainingSet a few lines below),
  // so we don't need to call it here.
  learner_->setTrainingSet(training_set, false);
  if (call_forget && !training_set_has_changed)
    // In this case, learner_->build() will not have been called, which may
    // cause trouble if it updates data from the training set.
    learner_->build();
  inherited::setTrainingSet(training_set, call_forget);
}

void EmbeddedLearner::setValidationSet(VMat validset)
{
  assert( learner_ );
  inherited::setValidationSet(validset);
  learner_->setValidationSet(validset);
}

void EmbeddedLearner::setTrainStatsCollector(PP<VecStatsCollector> statscol)
{
  assert( learner_ );
  inherited::setTrainStatsCollector(statscol);
  learner_->setTrainStatsCollector(statscol);
}

void EmbeddedLearner::setExperimentDirectory(const PPath& the_expdir)
{
  assert( learner_ );
  inherited::setExperimentDirectory(the_expdir);
  if (the_expdir != "") {
    string sub_expdir =
      append_slash(append_slash(the_expdir) + expdir_append);
    learner_->setExperimentDirectory(sub_expdir);
  }
  else
    learner_->setExperimentDirectory("");
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

bool EmbeddedLearner::computeConfidenceFromOutput(
  const Vec& input, const Vec& output,
  real probability, TVec< pair<real,real> >& intervals) const
{
  assert( learner_ );
  return learner_->computeConfidenceFromOutput(input,output,probability,
                                               intervals);
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
