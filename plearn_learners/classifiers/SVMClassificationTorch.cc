// -*- C++ -*-

// SVMClassificationTorch.cc
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
   * $Id: SVMClassificationTorch.cc,v 1.2 2005/01/24 14:29:35 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file SVMClassificationTorch.cc */

#include "SVMClassificationTorch.h"
#include <plearn_torch/TorchDataSet.h>
#include <plearn_torch/TorchKernel.h>

namespace PLearn {
using namespace std;

////////////////////////////
// SVMClassificationTorch //
////////////////////////////
SVMClassificationTorch::SVMClassificationTorch() 
: C_cst(100),
  cache_size_in_megs(50)
{
  allocator = new Torch::Allocator;
  inputs = 0;
}

PLEARN_IMPLEMENT_OBJECT(SVMClassificationTorch,
    "PLearn interface to the Torch SVMClassification class.",
    "THIS CLASS SHOULD NOT BE USED YET, IT IS STILL IN DEVELOPMENT."
);

////////////////////
// declareOptions //
////////////////////
void SVMClassificationTorch::declareOptions(OptionList& ol)
{
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // Build options.

  declareOption(ol, "C", &SVMClassificationTorch::C_cst, OptionBase::buildoption,
      "Trade off margin/classification error");

  declareOption(ol, "kernel", &SVMClassificationTorch::kernel, OptionBase::buildoption,
      "The kernel to be used.");

  declareOption(ol, "cache_size", &SVMClassificationTorch::cache_size_in_megs, OptionBase::buildoption,
      "Cache size (in Mo)");

  // Learnt options.

  // declareOption(ol, "myoption", &SVMClassificationTorch::myoption, OptionBase::learntoption,
  //               "Help text describing this option");

  // Now call the parent class' declareOptions.
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void SVMClassificationTorch::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void SVMClassificationTorch::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
  if (kernel) {
    Torch::Kernel* torch_kernel = new(allocator) Torch::TorchKernel(kernel);
    svm = new(allocator) Torch::SVMClassification(torch_kernel);
    svm->setROption("C", C_cst);
    svm->setROption("cache size", cache_size_in_megs);
  }
  // TODO Will there be memory leaks if successive calls to build() ?
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void SVMClassificationTorch::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
  // Compute the costs from *already* computed output. 
  // ...
}                                

///////////////////
// computeOutput //
///////////////////
void SVMClassificationTorch::computeOutput(const Vec& input, Vec& output) const
{
  // Compute the output from the input.
  // int nout = outputsize();
  // output.resize(nout);
  output.resize(1);
  inputs->copyFrom(input.data());
  svm->forward(inputs);
  svm->outputs->copyTo(output.data());
}    

////////////
// forget //
////////////
void SVMClassificationTorch::forget()
{
  stage = 0;
}
    
//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> SVMClassificationTorch::getTestCostNames() const
{
  // Return the names of the costs computed by computeCostsFromOutpus
  // (these may or may not be exactly the same as what's returned by getTrainCostNames).
  // ...
  // TODO
  TVec<string> bid;
  return bid;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> SVMClassificationTorch::getTrainCostNames() const
{
  // Return the names of the objective costs that the train method computes and 
  // for which it updates the VecStatsCollector train_stats
  // (these may or may not be exactly the same as what's returned by getTestCostNames).
  // ...
  // TODO
  TVec<string> bid;
  return bid;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SVMClassificationTorch::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  
  // TODO See what this means for a Torch object.

  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("SVMClassificationTorch::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////
// outputsize //
////////////////
int SVMClassificationTorch::outputsize() const
{
  return targetsize();
}

////////////////////
// setTrainingSet //
////////////////////
void SVMClassificationTorch::setTrainingSet(VMat training_set, bool call_forget) {
  inherited::setTrainingSet(training_set, call_forget);
  dataset = new(allocator) Torch::TorchDataSet(training_set);
  allocator->free(inputs); // Get rid of old inputs.
  inputs = new(allocator) Torch::Sequence(1, training_set->inputsize());
}

///////////
// train //
///////////
void SVMClassificationTorch::train()
{
  if (stage >= nstages) {
    PLWARNING("In SVMClassificationTorch::train - Learner has already been trained, skipping training");
    return;
  }
  if (!kernel)
    PLERROR("In SVMClassificationTorch::train - I need a kernel in order to be trained");
  trainer = new(allocator) Torch::QCTrainer(svm);
  trainer->train(dataset, NULL);
  allocator->free(trainer); // We don't need the trainer anymore.
  stage = 1;
}

/////////////////////////////
// ~SVMClassificationTorch //
/////////////////////////////
SVMClassificationTorch::~SVMClassificationTorch() {
  delete allocator;
}

} // end of namespace PLearn
