// -*- C++ -*-

// TSVM.cc
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
   * $Id: TSVM.cc,v 1.3 2005/02/23 18:01:50 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file TSVM.cc */


#include "TSVM.h"
#include <plearn_torch/TDataSet.h>
#include <plearn_torch/TKernel.h>
#include <plearn_torch/TSequence.h>

namespace PLearn {
using namespace std;

TSVM::TSVM() 
: svm(0),
  b(0)
{
}

PLEARN_IMPLEMENT_OBJECT(TSVM,
    "Interface between PLearn and a Torch SVM object",
    "This is NOT a transductive SVM."
);

void TSVM::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // Build options.

  declareOption(ol, "kernel", &TSVM::kernel, OptionBase::buildoption,
      "The kernel associated to the SVM.");

  // Learnt options.

  declareOption(ol, "b", &TSVM::b, OptionBase::learntoption,
      "The bias.");

  declareOption(ol, "data", &TSVM::data, OptionBase::learntoption,
      "The train dataset associated to the SVM.");

  declareOption(ol, "n_support_vectors_bound", &TSVM::n_support_vectors_bound, OptionBase::learntoption,
      "The number of support vectors whose coefficient reached the bound.");

  declareOption(ol, "support_vectors", &TSVM::support_vectors, OptionBase::learntoption,
      "The indices of the support vectors in the training set.");

  declareOption(ol, "sv_alpha", &TSVM::sv_alpha, OptionBase::learntoption,
      "The coefficients (weights) of the support vectors.");

  declareOption(ol, "sv_sequences", &TSVM::sv_sequences, OptionBase::learntoption,
      "The support vectors themselves.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void TSVM::build_()
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
void TSVM::build()
{
  inherited::build();
  build_();
}

void TSVM::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("TSVM::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//////////////////////
// updateFromPLearn //
//////////////////////
void TSVM::updateFromPLearn(Torch::Object* ptr) {
  if (ptr)
    svm = (Torch::SVM*) ptr;
  else {
    PLERROR("In TSVM::updateFromPLearn - Torch::SVM is an abstract class "
            "and cannot be instantiated");
  }
  if (options["kernel"])                  svm->kernel                   = kernel->kernel;
  if (options["data"])                    svm->data                     = data ? data->dataset : 0;
  if (options["b"])                       svm->b                        = b;
  if (options["support_vectors"])         svm->support_vectors          = support_vectors ? support_vectors.data()
                                                                                          : 0;
  if (options["support_vectors"])         svm->n_support_vectors        = support_vectors.length();
  if (options["sv_alpha"])                svm->sv_alpha                 = sv_alpha ? sv_alpha.data()
                                                                                   : 0;
  // TODO Could free some memory if possible.
  if (options["sv_sequences"]) {
    svm->sv_sequences = (Torch::Sequence **)allocator->alloc(sizeof(Torch::Sequence *)*sv_sequences.length());
    for (int i = 0; i < sv_sequences.length(); i++) {
      svm->sv_sequences[i] = sv_sequences[i]->sequence;
    }
  }
  if (options["n_support_vectors_bound"]) svm->n_support_vectors_bound  = n_support_vectors_bound;
  PLWARNING("In TSVM::updateFromPLearn - Implementation not completed");
  inherited::updateFromPLearn(svm);
}

/////////////////////
// updateFromTorch //
/////////////////////
void TSVM::updateFromTorch() {
  assert( (kernel ? kernel->kernel : 0) == svm->kernel ); // TODO Necessary ?
  if (kernel)
    kernel->updateFromTorch(); // TODO Not really necessary ?
  if (svm->data != (data ? data->dataset : 0)) {
    // The SVM dataset has changed.
    TObjectMap::const_iterator it = torch_objects.find(svm->data);
    if (it == torch_objects.end())
      PLERROR("In TSVM::updateFromTorch - Could not find a TObject associated with the new dataset");
    data = (TDataSet*) it->second;
  }
  if (options["b"])
    b = svm->b;
  if (data)
    data->updateFromTorch(); // TODO Necessary ?
  support_vectors.resize(svm->n_support_vectors);
  sv_alpha.resize       (svm->n_support_vectors);
  support_vectors.copyFrom(svm->support_vectors, svm->n_support_vectors);
  sv_alpha.copyFrom       (svm->sv_alpha       , svm->n_support_vectors);
  n_support_vectors_bound = svm->n_support_vectors_bound;
  if (options["sv_sequences"]) {
    // TODO May have to be done in subclasses ?
    int n = svm->n_support_vectors;
    sv_sequences.resize(n);
    for (int i = 0; i < n; i++) {
      Torch::Sequence* seq = svm->sv_sequences[i];
      TObjectMap::const_iterator it = torch_objects.find(seq);
      if (it == torch_objects.end()) {
        // Need to create a new sequence.
        // TODO Would be cool to be able to create subclasses of TSequence.
        PP<TSequence> new_seq = new TSequence();
        new_seq->sequence = seq;
        new_seq->updateFromTorch();
        sv_sequences[i] = new_seq;
      } else
        sv_sequences[i] = (TSequence*) it->second;
    }
  }
  PLWARNING("In TSVM::updateFromTorch - Implementation non completed");
  inherited::updateFromTorch();
}

} // end of namespace PLearn
