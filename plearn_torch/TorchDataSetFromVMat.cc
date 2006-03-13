// -*- C++ -*-

// TorchDataSetFromVMat.cc
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

/*! \file TorchDataSetFromVMat.cc */

#include "TorchDataSetFromVMat.h"

namespace Torch {

//////////////////////////
// TorchDataSetFromVMat //
//////////////////////////
TorchDataSetFromVMat::TorchDataSetFromVMat(PLearn::VMat vm) {
  this->vmat = vm;
  n_inputs = vm->inputsize();
  n_targets = vm->targetsize();
  input.resize(n_inputs);
  target.resize(n_targets);
  if (vm->weightsize() > 0)
    warning("In TorchDataSetFromVMat::TorchDataSetFromVMat - The source VMat's weights will be ignored");
  if (n_inputs > 0)
    inputs = new(allocator) Sequence(1, n_inputs);
  if(n_targets > 0)
    targets = new(allocator) Sequence(1, n_targets);
  real_current_example_index = -1;
  pre_processes = new(allocator) PreProcessingList;
  DataSet::init(vmat->length(), n_inputs, n_targets);
}

///////////////////////
// getNumberOfFrames //
///////////////////////
void TorchDataSetFromVMat::getNumberOfFrames(int t, int *n_input_frames, int *n_target_frames) {
  // A VMat's example has only one frame.
  if (n_input_frames)
    *n_input_frames = 1;
  if (n_target_frames)
    *n_target_frames = 1;
}

////////////////
// preProcess //
////////////////
void TorchDataSetFromVMat::preProcess(PreProcessing *pre_processing) {
  pre_processes->addNode(pre_processing);
}

////////////////////
// setRealExample //
////////////////////
void TorchDataSetFromVMat::setRealExample(int t, bool set_inputs, bool set_targets) {
  if (t == real_current_example_index)
    return;
  vmat->getExample(t, input, target, dummy_weight);
  if (n_inputs > 0 && set_inputs) {
    inputs->copyFrom(input->data());
    if (pre_processes) {
      for (int i = 0; i < pre_processes->n_nodes; i++)
        pre_processes->nodes[i]->preProcessInputs(inputs);
    }
  }
  if (n_targets > 0 && set_targets) {
    targets->copyFrom(target->data());
    if (pre_processes)
    {
      for (int i = 0; i < pre_processes->n_nodes; i++)
        pre_processes->nodes[i]->preProcessTargets(targets);
    }
  }
  real_current_example_index = t;
}


////////////////
// popExample //
////////////////
void TorchDataSetFromVMat::popExample() {
  allocator->free(inputs);
  allocator->free(targets);
  pushed_examples->pop();
  pushed_examples->pop();
  pushed_examples->pop();
}

/////////////////
// pushExample //
/////////////////
void TorchDataSetFromVMat::pushExample() {
  pushed_examples->push(&inputs, sizeof(Sequence *));
  pushed_examples->push(&targets, sizeof(Sequence *));
  pushed_examples->push(&real_current_example_index, sizeof(int));
  if (n_inputs > 0)
    inputs = new(allocator) Sequence(1, n_inputs);
  if (n_targets > 0)
    targets = new(allocator) Sequence(1, n_targets);
  real_current_example_index = -1;
}

///////
// ~ //
///////
TorchDataSetFromVMat::~TorchDataSetFromVMat() {
}

} // end of namespace Torch

