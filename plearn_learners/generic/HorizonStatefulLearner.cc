// -*- C++ -*-

// HorizonStatefulLearner.cc
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
   * $Id: HorizonStatefulLearner.cc,v 1.1 2004/10/18 14:05:35 chapados Exp $ 
   ******************************************************* */

// Authors: Réjean Ducharme

/*! \file HorizonStatefulLearner.cc */


#include "HorizonStatefulLearner.h"

namespace PLearn {
using namespace std;

HorizonStatefulLearner::HorizonStatefulLearner()
  : inherited(), horizon(0)
{}

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
  HorizonStatefulLearner,
  "Kind of StatefulLearner designed for forecasting at horizon h",
  "A HorizonStatefulLearner is a StatefulLearner designed for forecasting\n"
  "at horizon h.  It serves as a base class for more specialized\n"
  "forecasters.  Contrarily to StatefulLearner, the HorizonStatefulLearner\n"
  "introduces a few additional assumptions on the structure of the training\n"
  "set and the interpretation of the various test methods:\n"
  "\n"
  "- In the training set, it assumes that the inputs and targets are spaced\n"
  "  apart by the forecasting horizon.  Hence, for the inputs at row t, the\n"
  "  targets are located at row t+horizon.\n"
  "\n"
  "- For computeOutputAndCosts (and friends), the inputs and outputs vector\n"
  "  are FOR THE CURRENT TIME STEP, but the targets and costs correspond to\n"
  "  the inputs/outputs that were horizon time steps IN THE PAST.  This\n"
  "  means that derived classes will usually have to keep a memory of the\n"
  "  past \"horizon\" inputs and/or outputs in order to compute meaningful\n"
  "  costs.");

void HorizonStatefulLearner::declareOptions(OptionList& ol)
{
  declareOption(
    ol, "horizon", &HorizonStatefulLearner::horizon,
    OptionBase::buildoption,
    "Forecasting horizon for the learner; see detailed class help for\n"
    "interpretation.  (Default value = 0)");

  inherited::declareOptions(ol);
}

void HorizonStatefulLearner::build_()
{}

// ### Nothing to add here, simply calls build_
void HorizonStatefulLearner::build()
{
  inherited::build();
  build_();
}

void HorizonStatefulLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

} // end of namespace PLearn
