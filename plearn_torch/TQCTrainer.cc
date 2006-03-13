// -*- C++ -*-

// TQCTrainer.cc
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

/*! \file TQCTrainer.cc */


#include "TQCTrainer.h"
#include <plearn_torch/TQCMachine.h>
#include <plearn_torch/TDataSet.h>

namespace PLearn {
using namespace std;

TQCTrainer::TQCTrainer() 
: qc_trainer(0),
  end_accuracy(0.01),
#ifdef USEDOUBLE
  epsilon_shrink(1e-9),
#else
  epsilon_shrink(1e-4),
#endif
  iter_msg(1000),
  iter_shrink(100),
  max_unshrink(1),
  unshrink(false)
{
}

PLEARN_IMPLEMENT_OBJECT(TQCTrainer,
    "Interface between PLearn and a Torch QCTrainer object",
    ""
);

void TQCTrainer::declareOptions(OptionList& ol)
{
  declareOption(ol, "qc_machine", &TQCTrainer::qc_machine, OptionBase::buildoption,
      "The QCMachine that this trainer trains.");

  declareOption(ol, "epsilon_shrink", &TQCTrainer::epsilon_shrink, OptionBase::buildoption,
      "Shrinking accuracy.");

  declareOption(ol, "end_accuracy", &TQCTrainer::end_accuracy, OptionBase::buildoption,
      "End accuracy.");

  declareOption(ol, "iter_msg", &TQCTrainer::iter_msg, OptionBase::buildoption,
      "Number of iterations between messages.");

  declareOption(ol, "iter_shrink", &TQCTrainer::iter_shrink, OptionBase::buildoption,
      "Minimal number of iterations to shrink.");

  declareOption(ol, "max_unshrink", &TQCTrainer::max_unshrink, OptionBase::buildoption,
      "Maximal number of unshrinking.");

  declareOption(ol, "unshrink", &TQCTrainer::unshrink, OptionBase::buildoption,
      "Whether to unshrink or not.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);

  // Hide unused parent's options.

  redeclareOption(ol, "machine", &TTrainer::machine, OptionBase::nosave,
      "A QCTrainer only uses 'qc_machine'.");

}

void TQCTrainer::build_()
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
void TQCTrainer::build()
{
  inherited::build();
  build_();
}

void TQCTrainer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("TQCTrainer::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

///////////
// train //
///////////
void TQCTrainer::train(PP<TDataSet> data) {
  trainer->train(data->dataset, NULL);
  updateFromTorch();              // Update Trainer parameters after training.
  qc_machine->updateFromTorch();  // Update QCMachine parameters after training.
}

//////////////////////
// updateFromPLearn //
//////////////////////
void TQCTrainer::updateFromPLearn(Torch::Object* ptr) {
  if (ptr)
    qc_trainer = (Torch::QCTrainer*) ptr;
  else {
    if (qc_trainer)
      allocator->free(qc_trainer);
    if (!qc_machine)
      // We cannot create the QCTrainer without its machine.
      return;
    qc_trainer = new(allocator) Torch::QCTrainer(((TQCMachine*) qc_machine)->qc_machine);
  }
  FROM_P_BASIC(end_accuracy,   end_accuracy,   qc_trainer, end_eps             );
  FROM_P_BASIC(epsilon_shrink, epsilon_shrink, qc_trainer, eps_shrink          );
  FROM_P_BASIC(iter_msg,       iter_msg,       qc_trainer, n_iter_message      );
  FROM_P_BASIC(iter_shrink,    iter_shrink,    qc_trainer, n_iter_min_to_shrink);
  FROM_P_BASIC(max_unshrink,   max_unshrink,   qc_trainer, n_max_unshrink      );
  FROM_P_BASIC(unshrink,       unshrink,       qc_trainer, unshrink_mode       );

  FROM_P_OBJ(qc_machine, qc_machine, qc_machine, TQCMachine, qc_trainer, qcmachine);

  inherited::updateFromPLearn(qc_trainer);
  // NB: not updating cache, k_xi, k_xj, old_alpha_xi, old_alpha_xj, current_error,
  //                  active_var_new, n_active_var_new, n_alpha, deja_shrink,
  //                  unshrink_mode, y, alpha, grad, bound_eps, n_active_var,
  //                  active_var, not_at_bound_at_iter, iter, status_alpha,
  //                  Cup, Cdown
}

/////////////////////
// updateFromTorch //
/////////////////////
void TQCTrainer::updateFromTorch() {
  FROM_T_BASIC(end_accuracy,   end_accuracy,   qc_trainer, end_eps             );
  FROM_T_BASIC(epsilon_shrink, epsilon_shrink, qc_trainer, eps_shrink          );
  FROM_T_BASIC(iter_msg,       iter_msg,       qc_trainer, n_iter_message      );
  FROM_T_BASIC(iter_shrink,    iter_shrink,    qc_trainer, n_iter_min_to_shrink);
  FROM_T_BASIC(max_unshrink,   max_unshrink,   qc_trainer, n_max_unshrink      );
  FROM_T_BASIC(unshrink,       unshrink,       qc_trainer, unshrink_mode       );

  FROM_T_OBJ(qc_machine, qc_machine, qc_machine, TQCMachine, qc_trainer, qcmachine);

  inherited::updateFromTorch();
}

} // end of namespace PLearn
