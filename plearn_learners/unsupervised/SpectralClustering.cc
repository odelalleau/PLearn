// -*- C++ -*-

// SpectralClustering.cc
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
   * $Id: SpectralClustering.cc,v 1.3 2004/07/21 16:30:59 chrish42 Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file SpectralClustering.cc */

#include <plearn/ker/DivisiveNormalizationKernel.h>
#include "SpectralClustering.h"

namespace PLearn {
using namespace std;

//////////////////
// SpectralClustering //
//////////////////
SpectralClustering::SpectralClustering() 
: remove_bias(false)
{
  // Usually, one will want only positive eigenvalues.
  min_eigenvalue = 0;
}

PLEARN_IMPLEMENT_OBJECT(SpectralClustering,
    "Spectral Clustering dimensionality reduction.",
    "The current code only performs dimensionality reduction, and does not do\n"
    "clustering."
);

////////////////////
// declareOptions //
////////////////////
void SpectralClustering::declareOptions(OptionList& ol)
{
  declareOption(ol, "remove_bias", &SpectralClustering::remove_bias, OptionBase::buildoption,
      "If set to 1, the (divisively) normalized kernel will not take into account terms\n"
      "of the form K(x_i,x_i), in order to remove bias induced by those terms.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);

  // Redirect the 'kernel' option toward sc_kernel.
  redeclareOption(ol, "kernel", &SpectralClustering::sc_kernel, OptionBase::buildoption,
      "The kernel used to (implicitly) project the data in feature space.");

  redeclareOption(ol, "ignore_n_first", &SpectralClustering::ignore_n_first, OptionBase::nosave,
      "In Spectral clustering, no eigenvector is ignored.");

  // And declare the normalized kernel so that it can be saved.
  declareOption(ol, "normalized_kernel", &KernelProjection::kernel, OptionBase::learntoption,
      "The normalized kernel.");
}

///////////
// build //
///////////
void SpectralClustering::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void SpectralClustering::build_()
{
  // Obtain the "real" kernel by divisive normalization of 'sc_kernel'.
  // We have to do this iff:
  // 1. A 'sc_kernel' is provided, and
  // 2. either:
  //    2.a. the 'kernel' option is not set, or
  //    2.b. the 'kernel' option is not a DivisiveNormalizationKernel acting on 'sc_kernel'.
  // This is to ensure that a loaded 'kernel' won't be overwritten.
  if (sc_kernel &&
      (!kernel ||
       (dynamic_cast<DivisiveNormalizationKernel*>((Kernel*) kernel))->source_kernel != sc_kernel)) {
    this->kernel = new DivisiveNormalizationKernel
      (sc_kernel, remove_bias);
  }
}

////////////
// forget //
////////////
void SpectralClustering::forget()
{
  inherited::forget();
}
    
/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SpectralClustering::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("SpectralClustering::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn

