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
   * $Id$ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file TSVM.cc */


#include <plearn/base/tostring.h>
#include "TSVM.h"
#include <plearn_torch/TDataSet.h>
#include <plearn_torch/TKernel.h>
#include <plearn_torch/TSequence.h>

namespace PLearn {
using namespace std;

TSVM::TSVM() 
: svm(0),
  n_support_vectors_bound(0),
  b(0),
  sv_save("default")
{
}

PLEARN_IMPLEMENT_OBJECT(TSVM,
    "Interface between PLearn and a Torch SVM object",
    "The 'sv_save' option determines how support vectors will be saved. The most\n"
    "flexible way is 'default', but it will waste a lot of space if there are\n"
    "many support vectors, in which case it is advised to use 'vmat' in order to\n"
    "save them as a VMatrix, which could be efficiently serialized (e.g. if it is\n"
    "defined as a subset of the dataset, as in TSVMClassification).\n"
    "NB: this is NOT a transductive SVM.\n"
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

  declareOption(ol, "sv_save", &TSVM::sv_save, OptionBase::buildoption,
      "How the support vectors ('sv_sequences') will be saved:\n"
      "- 'default' : in a vector of sequences ('sv_sequences')\n"
      "- 'no'      : it is not saved\n"
      "- 'vmat'    : in a VMatrix ('sv_sequences_vmat')");

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
      "The support vectors (when sv_save == 'default').");

  declareOption(ol, "sv_sequences_vmat", &TSVM::sv_sequences_vmat, OptionBase::learntoption,
      "The support vectors (when sv_save == 'vmat').");

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
  FROM_P_BASIC(b,                       b,                       svm, b                      );
  FROM_P_BASIC(n_support_vectors_bound, n_support_vectors_bound, svm, n_support_vectors_bound);

  FROM_P_TVEC (support_vectors, support_vectors, svm, support_vectors, n_support_vectors);
  FROM_P_TVEC (sv_alpha,        sv_alpha,        svm, sv_alpha,        n_support_vectors);

  FROM_P_OBJ  (kernel, kernel, kernel,  TKernel,  svm, kernel);
  FROM_P_OBJ  (data,   data,   dataset, TDataSet, svm, data  );

  if (sv_save == "default") {
    int n = sv_sequences.length();
    sv_sequences_torch.resize(n);
    for (int i = 0; i < n; i++)
      sv_sequences_torch[i] = sv_sequences[i]->sequence;
    FROM_P_TVEC(sv_sequences, sv_sequences_torch, svm, sv_sequences, n_support_vectors);
  } else if (sv_save == "no") {
  } else if (sv_save == "vmat") {
    if (sv_sequences_vmat && sv_sequences_vmat->length() > 0)
      sv_sequences_mat = sv_sequences_vmat->toMat();
    else
      sv_sequences_mat = Mat();
    int n = sv_sequences_mat.length();
    int w = sv_sequences_mat.width();
    sv_sequences_index.resize(n);
    for (int i = 0; i < n; i++)
      sv_sequences_index[i] = sv_sequences_mat[i];
    sv_sequences_torch.resize(n);
    for (int i = 0; i < n; i++)
      sv_sequences_torch[i] =
        new(allocator) Torch::Sequence(sv_sequences_index.data() + i, 1, w);
    FROM_P_TVEC(sv_sequences_vmat, sv_sequences_torch, svm, sv_sequences, n_support_vectors);
  } else
    PLERROR("In TSVM::updateFromPLearn - Unknown value for 'sv_save': %s",
            sv_save.c_str());
  /* Not as easy to do with a macro, TODO later !
  // TODO Could free some memory if possible.
  if (options["sv_sequences"]) {
    svm->sv_sequences = (Torch::Sequence **)allocator->alloc(sizeof(Torch::Sequence *)*sv_sequences.length());
    for (int i = 0; i < sv_sequences.length(); i++) {
      svm->sv_sequences[i] = sv_sequences[i]->sequence;
    }
  }
  */
  inherited::updateFromPLearn(svm);
  // NB: not updating io_sequence_array, sv_sequences.
}

/////////////////////
// updateFromTorch //
/////////////////////
void TSVM::updateFromTorch() {
  FROM_T_BASIC(b,                       b,                       svm, b                      );
  FROM_T_BASIC(n_support_vectors_bound, n_support_vectors_bound, svm, n_support_vectors_bound);

  FROM_T_TVEC (support_vectors, support_vectors, svm, support_vectors, n_support_vectors);
  FROM_T_TVEC (sv_alpha,        sv_alpha,        svm, sv_alpha,        n_support_vectors);

  FROM_T_OBJ  (kernel, kernel, kernel,  TKernel,  svm, kernel);
  FROM_T_OBJ  (data,   data,   dataset, TDataSet, svm, data  );

  if (sv_save == "default") {
    FROM_T_TVEC(sv_sequences, sv_sequences_torch, svm, sv_sequences, n_support_vectors);
    int n = sv_sequences_torch.length();
    sv_sequences.resize(n);
    for (int i = 0; i < n; i++) {
      Torch::Sequence* seq = sv_sequences_torch[i];
      TObjectMap::const_iterator it = torch_objects.find(seq);
      if (it == torch_objects.end()) {
        // Need to create a new sequence.
        // TODO Would be cool to be able to create subclasses of TSequence.
        sv_sequences[i] = new TSequence(seq);
      } else
        sv_sequences[i] = (TSequence*) it->second;
    }
  } else if (sv_save == "vmat") {
    FROM_T_TVEC(sv_sequences_vmat, sv_sequences_torch, svm, sv_sequences, n_support_vectors);
    int n = sv_sequences_torch.length();
    int w = n > 0 ? sv_sequences_torch[0]->frame_size : 0;
    // TODO Use a VMatrix->resize(x,y) method ?
    sv_sequences_vmat->setOption("length", tostring(n));
    sv_sequences_vmat->setOption("width", tostring(w));
    sv_sequences_vmat->build();
    sv_sequences_mat.resize(n, w);
    for (int i = 0; i < n; i++) {
      real* frame = sv_sequences_torch[i]->frames[0];
      sv_sequences_mat(i).copyFrom(frame, w);
    }
    if (n > 0) sv_sequences_vmat->putMat(0, 0, sv_sequences_mat);
  } else if (sv_save == "no") {
  } else
    PLERROR("In TSVM::updateFromTorch - Unknown value for 'sv_save': %s",
            sv_save.c_str());

  /* Not as easy to do with a macro, TODO later !
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
  */
  inherited::updateFromTorch();
}

} // end of namespace PLearn
