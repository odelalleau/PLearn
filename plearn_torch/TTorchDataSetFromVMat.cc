// -*- C++ -*-

// TTorchDataSetFromVMat.cc
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

/*! \file TTorchDataSetFromVMat.cc */


#include "TTorchDataSetFromVMat.h"

namespace PLearn {
using namespace std;

///////////////////////////
// TTorchDataSetFromVMat //
///////////////////////////
TTorchDataSetFromVMat::TTorchDataSetFromVMat() 
: torch_dataset_from_vmat(0)
{}

TTorchDataSetFromVMat::TTorchDataSetFromVMat(PLearn::VMat the_vmat)
: torch_dataset_from_vmat(0),
  vmat(the_vmat)
{
  build();
}

PLEARN_IMPLEMENT_OBJECT(TTorchDataSetFromVMat,
    "Interface between PLearn and a Torch TorchDataSetFromVMat object",
    ""
);

void TTorchDataSetFromVMat::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "vmat", &TTorchDataSetFromVMat::vmat, OptionBase::buildoption,
      "The underlying PLearn VMat.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);

  // Hide parent's options that are set by this DataSet.

  redeclareOption(ol, "n_inputs", &TTorchDataSetFromVMat::n_inputs, OptionBase::nosave,
      "Automatically set.");

  redeclareOption(ol, "n_targets", &TTorchDataSetFromVMat::n_targets, OptionBase::nosave,
      "Automatically set.");

  redeclareOption(ol, "n_real_examples", &TTorchDataSetFromVMat::n_real_examples, OptionBase::nosave,
      "Automatically set.");

  redeclareOption(ol, "real_current_example_index", &TTorchDataSetFromVMat::real_current_example_index, OptionBase::nosave,
      "Automatically set.");

  redeclareOption(ol, "select_examples", &TTorchDataSetFromVMat::select_examples, OptionBase::nosave,
      "Automatically set.");

  redeclareOption(ol, "selected_examples", &TTorchDataSetFromVMat::selected_examples, OptionBase::nosave,
      "Automatically set.");

}

void TTorchDataSetFromVMat::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
}

void TTorchDataSetFromVMat::build()
{
  inherited::build();
  build_();
}

void TTorchDataSetFromVMat::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("TTorchDataSetFromVMat::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//////////////////////
// updateFromPLearn //
//////////////////////
void TTorchDataSetFromVMat::updateFromPLearn(Torch::Object* ptr) {
  if (ptr) {
    torch_dataset_from_vmat = (Torch::TorchDataSetFromVMat*) ptr;
    FROM_P_BASIC(vmat, vmat, torch_dataset_from_vmat, vmat);
  } else {
    if (!vmat)
      return;
    if (torch_dataset_from_vmat)
      allocator->free(torch_dataset_from_vmat);
    torch_dataset_from_vmat = new(allocator) Torch::TorchDataSetFromVMat(vmat);
  }
  inherited::updateFromPLearn(torch_dataset_from_vmat);
  // NB: not updating input, target, dummy_weight, pre_processes.
}

/////////////////////
// updateFromTorch //
/////////////////////
void TTorchDataSetFromVMat::updateFromTorch() {
  FROM_T_BASIC(vmat, vmat, torch_dataset_from_vmat, vmat);
  inherited::updateFromTorch();
}

} // end of namespace PLearn
