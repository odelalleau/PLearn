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
   * $Id: SelectInputSubsetLearner.cc,v 1.1 2004/04/13 00:44:18 yoshua Exp $
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file SelectInputSubsetLearner.cc */


#include "SelectInputSubsetLearner.h"

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
                "If greater than 0 then the selected_inputs will be OVERRIDEN (with randomly chosen indices)\n");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void SelectInputSubsetLearner::build_()
{
  if (random_fraction>0 && learner_ && learner_->inputsize()>0)
  {
    int n_selected = rint(random_fraction*learner_->inputsize());
    selected_inputs.resize(learner_->inputsize());
    for (int i=0;i<n_selected;i++) 
      selected_input[i]=i;
    shuffleElements(selected_inputs);
    selected_inputs.resize(n_selected);
  }
}

// ### Nothing to add here, simply calls build_
void SelectInputSubsetLearner::build()
{
  inherited::build();
  build_();
}


void SelectInputSubsetLearner::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  deepCopyField(selected_inputs, copies);
}


void SelectInputSubsetLearner::computeOutput(const Vec& input, Vec& output) const
{
  // Compute the output from the input
  // int nout = outputsize();
  // output.resize(nout);
  // ...
}    

void SelectInputSubsetLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output. 
// ...
}                                

void SelectInputSubsetLearner::setTrainingSet(VMat training_set, bool call_forget=true)
{
}

} // end of namespace PLearn
