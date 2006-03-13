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
   * $Id$ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file TSVMClassification.cc */


#include <plearn/vmat/GetInputVMatrix.h>
#include <plearn/vmat/SelectRowsVMatrix.h>
#include "TSVMClassification.h"
#include <plearn_torch/TDataSet.h>
#include <plearn_torch/TKernel.h>
#include <plearn_torch/TorchDataSetFromVMat.h>
#include <plearn_torch/TSequence.h>
#include <torch/SVMClassification.h>

namespace PLearn {
using namespace std;

TSVMClassification::TSVMClassification() 
: svm_class(0),
  C(100),
  cache_size(50)
{
  sv_save = "vmat";
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

  redeclareOption(ol, "sv_sequences", &TSVMClassification::sv_sequences, OptionBase::nosave,
      "No need for this because sv_save == 'vmat'.");

  redeclareOption(ol, "sv_save", &TSVMClassification::sv_save, OptionBase::nosave,
      "It is set automatically to 'vmat' because we use a subset of the training set.");

}

void TSVMClassification::build_()
{
}

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
  FROM_P_BASIC(cache_size, cache_size, svm_class, cache_size_in_megs);
  FROM_P_BASIC(C,          C,          svm_class, C_cst             );
  FROM_P_TVEC (C_j,        C_j,        svm_class, Cuser, n_alpha    );

  if (options["data"] && options["support_vectors"] && data) {
    sv_sequences_vmat =
      new SelectRowsVMatrix(
          new GetInputVMatrix(
            ((Torch::TorchDataSetFromVMat*) data->dataset)->vmat),
          support_vectors                                         );
  } else
    sv_sequences_vmat = 0;
  /*
  if (options["data"] && options["support_vectors"] && data && false) {
    // Build sv_sequences from the dataset and the support vectors.
    // TODO Do something cleaner !
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
  */

  inherited::updateFromPLearn(svm_class);
  // NB: not updating sequences_buffer, frames_buffer.
}

/////////////////////
// updateFromTorch //
/////////////////////
void TSVMClassification::updateFromTorch() {
  FROM_T_BASIC(cache_size, cache_size, svm_class, cache_size_in_megs);
  FROM_T_BASIC(C,          C,          svm_class, C_cst             );
  FROM_T_TVEC (C_j,        C_j,        svm_class, Cuser, n_alpha    );

  // There is actually no need to update the VMat 'sv_sequences_vmat' since
  // it can be obtained automatically from the dataset and the indices.
  string sv_save_backup = sv_save;
  sv_save = "no";
  inherited::updateFromTorch();
  sv_save = sv_save_backup;
}

} // end of namespace PLearn
