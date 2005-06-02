// -*- C++ -*-

// LLE.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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
   * $Id: LLE.cc,v 1.6 2005/06/02 14:01:11 crompb Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file LLE.cc */


#include "LLE.h"

namespace PLearn {
using namespace std;

/////////
// LLE //
/////////
LLE::LLE() 
: classical_induction(true),
  knn(5),
  reconstruct_coeff(-1),
  regularizer(1e-6)
{
  lle_kernel = new LLEKernel();
  this->normalize = "unit_var";  // In LLE, we normalize.
  this->ignore_n_first = 1; // In LLE, we ignore the first eigenvector.
}

PLEARN_IMPLEMENT_OBJECT(LLE,
    "Performs Locally Linear Embedding.",
    ""
);

////////////////////
// declareOptions //
////////////////////
void LLE::declareOptions(OptionList& ol)
{
  // Build options.

  declareOption(ol, "knn", &LLE::knn, OptionBase::buildoption,
      "The number of nearest neighbors considered.");

  declareOption(ol, "classical_induction", &LLE::classical_induction, OptionBase::buildoption,
      "If set to 1, then the out-of-sample extension of LLE will be the classical\n"
      "one, corresponding to an infinite 'reconstruct_coeff' (whose value is ignored).");

  declareOption(ol, "reconstruct_coeff", &LLE::reconstruct_coeff, OptionBase::buildoption,
      "The weight of K' in the weighted sum of K' and K'' (see LLEKernel).");

  declareOption(ol, "regularizer", &LLE::regularizer, OptionBase::buildoption,
      "The regularization factor used to make the linear systems stable.");

  // Learnt options.

  declareOption(ol, "lle_kernel", &LLE::lle_kernel, OptionBase::learntoption,
      "The kernel used in LLE.");

  // Now call the parent class' declareOptions.
  inherited::declareOptions(ol);

  // Hide unused options from KernelProjection.
  redeclareOption(ol, "kernel", &LLE::kernel, OptionBase::nosave,
      "Will be set at build time.");

  redeclareOption(ol, "normalize", &LLE::normalize, OptionBase::nosave,
      "Will be set at construction and build time.");

  redeclareOption(ol, "ignore_n_first", &LLE::ignore_n_first, OptionBase::nosave,
      "Will be set to 1 at construction time.");

}

///////////
// build //
///////////
void LLE::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void LLE::build_()
{
  if (classical_induction) {
    lle_kernel->reconstruct_coeff = -1;
    normalize = "unit_eigen";
  } else {
    lle_kernel->reconstruct_coeff = this->reconstruct_coeff;
    normalize = "unit_var";
  }
  lle_kernel->knn = this->knn;
  lle_kernel->regularizer = this->regularizer;
  lle_kernel->report_progress = this->report_progress;
  lle_kernel->build();
  this->kernel = (Kernel*) lle_kernel;
}

////////////
// forget //
////////////
void LLE::forget()
{
  inherited::forget();
}
    
/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LLE::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("LLE::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn

