// -*- C++ -*-

// SequentialLearner.cc
//
// Copyright (C) 2003 Rejean Ducharme, Yoshua Bengio
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



#include "SequentialLearner.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_ABSTRACT_OBJECT(SequentialLearner, "ONE LINE DESCR", "NO HELP");

SequentialLearner::SequentialLearner()
  : last_train_t(-1), last_call_train_t(-1), last_test_t(-1),
    init_train_size(1), max_seq_len(-1), max_train_len(-1), train_step(1), horizon(1),
    outputsize_(1)
{}

void SequentialLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(predictions, copies);
  deepCopyField(errors, copies);
} 

void SequentialLearner::build_()
{
  if(max_seq_len != -1)
  {
    if( outputsize() == 0 )
      PLERROR("SequentialLearner::build_\n"
              "outputsize() returns 0 but predictions will later be assumed to have nonzero width.");
    predictions.resize(max_seq_len, outputsize());
    predictions.fill(MISSING_VALUE);
    
    if( nTestCosts() == 0 )
      PLERROR("SequentialLearner::build_\n"
              "nTestCosts() returns 0 but errors will later be assumed to have nonzero width.");
    errors.resize(max_seq_len, nTestCosts());
    errors.fill(MISSING_VALUE);
  }  
}
  
void SequentialLearner::build()
{
  inherited::build();
  build_();
}

void SequentialLearner::declareOptions(OptionList& ol)
{
  declareOption(ol, "init_train_size", &SequentialLearner::init_train_size, OptionBase::buildoption,
                "Before the length of train_set reaches init_train_size, train doesn't do anything.\n"
                "Default = 1.");

  declareOption(ol, "max_seq_len", &SequentialLearner::max_seq_len,
                OptionBase::buildoption,
                "Maximum length that the training matrix will ever reach;\n"
                "this is used as an optimization to preallocate buffers\n"
                "and avoid reallocations as training/testing proceeds.");

  declareOption(ol, "max_train_len", &SequentialLearner::max_train_len,
                OptionBase::buildoption,
                "Maximum number of (input,target) pairs used for training;\n"
                "for longer training sequences, only the last max_train_len\n"
                "pairs are actually used for training");

  declareOption(ol, "train_step", &SequentialLearner::train_step,
                OptionBase::buildoption,
                "How often we have to re-train a model;\n"
                "value of 1 = after every time step");

  declareOption(ol, "horizon", &SequentialLearner::horizon,
                OptionBase::buildoption,
                "How much to offset the target columns with respect to\n"
                "the input columns");

  declareOption(ol, "outputsize", &SequentialLearner::outputsize_,
                OptionBase::buildoption,
                "Size of the output vector (number of outputs)");

  inherited::declareOptions(ol);
}

void SequentialLearner::setTrainingSet(VMat training_set, bool call_forget)
{
  train_set = training_set;
  if (call_forget) forget();
}

void SequentialLearner::forget()
{
  if (predictions.isNotEmpty())
    predictions.fill(MISSING_VALUE);
  if (errors.isNotEmpty())
    errors.fill(MISSING_VALUE);
  last_train_t      = -1;
  last_call_train_t = -1;
  last_test_t       = -1;
}

//! Returns train_set->targetsize()
int SequentialLearner::outputsize() const
{ return outputsize_; }

void SequentialLearner::computeOutputAndCosts(const Vec& input,
    const Vec& target, Vec& output, Vec& costs) const
{ PLERROR("The method computeOutputAndCosts is not defined for this SequentialLearner"); }

void SequentialLearner::computeCostsOnly(const Vec& input, const Vec& target,
    Vec& costs) const
{ PLERROR("The method computeCostsOnly is not defined for this SequentialLearner"); }

void SequentialLearner::computeOutput(const Vec& input, Vec& output) const
{ PLERROR("The method computeOutput is not defined for this SequentialLearner"); }

void SequentialLearner::computeCostsFromOutputs(const Vec& input,
    const Vec& output, const Vec& target, Vec& costs) const
{ PLERROR("The method computeCostsFromOutputs is not defined for this SequentialLearner"); }

void SequentialLearner::matlabSave(const string& matlab_subdir)
{
  string save_dir = append_slash(getExperimentDirectory()) + matlab_subdir;
  Vec dummy, add(1); add[0] = 0;

  TVec<string> cost_names = getTestCostNames();  
  Vec startX(1, real(sequenceStart()));
  for(int g=0; g < cost_names.length(); g++)
    PLearn::matlabSave(save_dir, cost_names[g], 
                       startX,
                       getCostSequence(g), add, dummy);       

  PLearn::matlabSave(save_dir, "Predictions", predictions, dummy, dummy);
}

} // end of namespace PLearn

