// -*- C++ -*-

// TTrainer.cc
//
// Copyright (C) 2005 Olivier Delalleau 
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
   * $Id$ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file TTrainer.cc */


#include "TTrainer.h"
#include <plearn_torch/TMachine.h>
#include <plearn_torch/TDataSet.h>

namespace PLearn {
using namespace std;

TTrainer::TTrainer() 
: trainer(0)
{
}

PLEARN_IMPLEMENT_OBJECT(TTrainer,
    "Interface between PLearn and a Torch Trainer object",
    ""
);

void TTrainer::declareOptions(OptionList& ol)
{
  declareOption(ol, "machine", &TTrainer::machine, OptionBase::learntoption,
      "The machine that this trainer trains.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void TTrainer::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
}

// ### Nothing to add here, simply calls build_
void TTrainer::build()
{
  inherited::build();
  build_();
}

void TTrainer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("TTrainer::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

///////////
// train //
///////////
void TTrainer::train(PP<TDataSet> data) {
  trainer->train(data->dataset, NULL);
  updateFromTorch();          // Update Trainer parameters after training.
  machine->updateFromTorch(); // Update Machine parameters after training.
}

//////////////////////
// updateFromPLearn //
//////////////////////
void TTrainer::updateFromPLearn(Torch::Object* ptr) {
  if (ptr)
    trainer = (Torch::Trainer*) ptr;
  else
    PLERROR("In TTrainer::updateFromPLearn - Torch::Trainer is an abstract class "
            "and cannot be instantiated");
  FROM_P_OBJ(machine, machine, machine, TMachine, trainer, machine);
  inherited::updateFromPLearn(trainer);
}

/////////////////////
// updateFromTorch //
/////////////////////
void TTrainer::updateFromTorch() {
  FROM_T_OBJ(machine, machine, machine, TMachine, trainer, machine);
  inherited::updateFromTorch();
}

} // end of namespace PLearn
