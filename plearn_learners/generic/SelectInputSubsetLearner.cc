// -*- C++ -*-

// SelectInputSubsetLearner.cc
//
// Copyright (C) 2004 Yoshua Bengio 
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
   * $Id: SelectInputSubsetLearner.cc,v 1.4 2004/09/14 16:04:56 chrish42 Exp $
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file SelectInputSubsetLearner.cc */

#include "SelectInputSubsetLearner.h"
#include <plearn/vmat/SelectColumnsVMatrix.h>
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;

  SelectInputSubsetLearner::SelectInputSubsetLearner() : random_fraction(0)
/* ### Initialize all fields to their default value here */
{
}

PLEARN_IMPLEMENT_OBJECT(SelectInputSubsetLearner, "PLearner which selects a subset of the inputs for an embedded learner.", 
                        "This learner class contains an embedded learner for which it selects a subset of the inputs.\n"
                        "The subset can be either selected explicitly or chosen randomly (the user chooses what fraction\n"
                        "of the original inputs will be selected).");

void SelectInputSubsetLearner::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "selected_inputs", &SelectInputSubsetLearner::selected_inputs, OptionBase::buildoption,
                "List of selected inputs. If this option is set then random_fraction should not be set (or set to 0).\n");

  declareOption(ol, "random_fraction", &SelectInputSubsetLearner::random_fraction, OptionBase::buildoption,
                "Fraction of the original inputs that is randomly selected.\n"
                "If 0 then the selected_inputs option should be set.\n"
                "If selected_inputs is provided (length>0) then this option is ignored.\n");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void SelectInputSubsetLearner::build_()
{
  if (random_fraction>0 && learner_ && inputsize_>0 && selected_inputs.length()==0)
  {
    int n_selected = int(rint(random_fraction*inputsize_));
    selected_inputs.resize(inputsize_);
    for (int i=0;i<n_selected;i++) 
      selected_inputs[i]=i;
    shuffleElements(selected_inputs);
    selected_inputs.resize(n_selected);
  }
  learner_inputs.resize(selected_inputs.length());
}

// ### Nothing to add here, simply calls build_
void SelectInputSubsetLearner::build()
{
  inherited::build();
  build_();
}


void SelectInputSubsetLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  deepCopyField(selected_inputs, copies);
  deepCopyField(all_indices, copies);
  deepCopyField(learner_inputs, copies);
}

int SelectInputSubsetLearner::inputsize() const
{ return inputsize_; }


void SelectInputSubsetLearner::computeOutput(const Vec& input, Vec& output) const
{
  for (int i=0;i<learner_inputs.length();i++)
    learner_inputs[i] = input[selected_inputs[i]];
  learner_->computeOutput(learner_inputs,output);
}    

void SelectInputSubsetLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
  // Compute the costs from *already* computed output. 
  for (int i=0;i<learner_inputs.length();i++)
    learner_inputs[i] = input[selected_inputs[i]];
  learner_->computeCostsFromOutputs(learner_inputs,output,target,costs);
}                                

void SelectInputSubsetLearner::computeOutputAndCosts(const Vec& input, const Vec& target,
                                                     Vec& output, Vec& costs) const
{ 
  for (int i=0;i<learner_inputs.length();i++)
    learner_inputs[i] = input[selected_inputs[i]];
  learner_->computeOutputAndCosts(learner_inputs, target, output, costs); 
}

void SelectInputSubsetLearner::setTrainingSet(VMat training_set, bool call_forget)
{
  inherited::setTrainingSet(training_set,call_forget);
  int n_other_columns = training_set->width()-inputsize();
  all_indices.resize(selected_inputs.length()+n_other_columns);
  for (int i=0;i<selected_inputs.length();i++)
    all_indices[i]=selected_inputs[i];
  for (int j=0;j<n_other_columns;j++)
    all_indices[selected_inputs.length()+j]=inputsize()+j;
  VMat vm = new SelectColumnsVMatrix(training_set,all_indices);
  vm->defineSizes(selected_inputs.length(),training_set->targetsize(),training_set->weightsize());
  learner_->setTrainingSet(vm,call_forget);
}

} // end of namespace PLearn
