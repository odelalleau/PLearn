// -*- C++ -*-

// TSVMClassification.cc
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
   * $Id: TSVMClassification.cc,v 1.2 2005/02/23 17:26:36 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file TSVMClassification.cc */


#include "TSVMClassification.h"
#include <plearn_torch/TDataSet.h>
#include <plearn_torch/TKernel.h>
#include <plearn_torch/TSequence.h>
#include <torch/SVMClassification.h>

namespace PLearn {
using namespace std;

TSVMClassification::TSVMClassification() 
: svm_class(0),
  C(100),
  cache_size(50)
{
}

PLEARN_IMPLEMENT_OBJECT(TSVMClassification,
    "Interface between PLearn and a Torch SVMClassification object",
    ""
);

void TSVMClassification::declareOptions(OptionList& ol)
{

  declareOption(ol, "C", &TSVMClassification::C, OptionBase::buildoption,
      "Trade-off margin / error.");

  declareOption(ol, "C_j", &TSVMClassification::C_j, OptionBase::buildoption,
      "Trade-off margin / error for each example (if provided, must have\n"
      "the same size as the training set, and will override 'C').");

  declareOption(ol, "cache_size", &TSVMClassification::cache_size, OptionBase::buildoption,
      "Cache size (in Mb).");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);

  // Hide some parent's options.

  declareOption(ol, "sv_sequences", &TSVMClassification::sv_sequences, OptionBase::nosave,
      "This is just a subset of the dataset. Thus we do not need to save it explicitely,\n"
      "since it can be constructed from 'data' and 'support_vectors'.");

}

void TSVMClassification::build_()
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
void TSVMClassification::build()
{
  inherited::build();
  build_();
}

void TSVMClassification::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("TSVMClassification::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//////////////////////
// updateFromPLearn //
//////////////////////
  void TSVMClassification::updateFromPLearn(Torch::Object* ptr) {
    if (ptr)
      svm_class = (Torch::SVMClassification*) ptr;
    else {
      if (svm_class)
        allocator->free(svm_class);
      svm_class = new(allocator) Torch::SVMClassification(kernel->kernel, C_j ? C_j.data() : 0);
    }
    if (options["cache_size"])  svm_class->cache_size_in_megs = cache_size;
    if (options["C"])           svm_class->C_cst              = C;
    if (options["C_j"])         svm_class->Cuser              = C_j ? C_j.data() : 0;
    if (options["data"] && options["support_vectors"] && data) {
      // Build sv_sequences.
      int n = support_vectors.length();
      // TODO Could have to free memory ?
      svm_class->sv_sequences =
        (Torch::Sequence **)allocator->alloc(sizeof(Torch::Sequence *)*n);
      Torch::DataSet* dat = data->dataset;
      int frame_buf_size = 0;
      for (int i = 0; i < n; i++) {
        dat->setExample(support_vectors[i]);
        frame_buf_size += dat->inputs->getFramesSpace();
      }
      int seq_size = dat->inputs->getSequenceSpace();
      char* seq_buf    = (char*) allocator->alloc(seq_size * n  );
      char* frames_buf = (char*) allocator->alloc(frame_buf_size);
      for (int i = 0; i < n; i++) {
        dat->setExample(support_vectors[i]);
        svm_class->sv_sequences[i] = dat->inputs->clone(allocator, seq_buf, frames_buf);
        seq_buf += seq_size;
        frames_buf += dat->inputs->getFramesSpace();
      }
    }

    inherited::updateFromPLearn(svm_class);
  }

/////////////////////
// updateFromTorch //
/////////////////////
void TSVMClassification::updateFromTorch() {
  cache_size = svm_class->cache_size_in_megs;
  C          = svm_class->C_cst;
  if (svm_class->Cuser) {
    int n_alpha = svm_class->n_alpha;
    C_j.resize(n_alpha);
    C_j.copyFrom(svm_class->Cuser, n_alpha);
  } else
    C_j = Vec();

  inherited::updateFromTorch();
}

} // end of namespace PLearn
