// -*- C++ -*-

// KernelPCA.cc
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
   * $Id: KernelPCA.cc,v 1.9 2005/05/30 20:15:23 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file KernelPCA.cc */

#include <plearn/ker/AdditiveNormalizationKernel.h>
#include "KernelPCA.h"

namespace PLearn {
using namespace std;

//////////////////
// KernelPCA //
//////////////////
KernelPCA::KernelPCA() 
: kernel_is_distance(false),
  remove_bias(false),
  remove_bias_in_evaluate(false)
{
  // Usually, one will want only positive eigenvalues.
  min_eigenvalue = 0;
}

PLEARN_IMPLEMENT_OBJECT(KernelPCA,
    "Kernel Principal Component Analysis",
    "Perform PCA in a feature space phi(x), defined by a kernel K such that\n"
    " K(x,y) = < phi(x), phi(y) >\n"
);

////////////////////
// declareOptions //
////////////////////
void KernelPCA::declareOptions(OptionList& ol)
{
  declareOption(ol, "kernel_is_distance", &KernelPCA::kernel_is_distance, OptionBase::buildoption,
      "If set to 1, then the kernel will be considered as a squared distance instead of\n"
      "a dot product (i.e. the double-centering formula will be applied).");

  declareOption(ol, "remove_bias", &KernelPCA::remove_bias, OptionBase::buildoption,
      "If set to 1, the (additively) normalized kernel will not take into account terms\n"
      "of the form K(x_i,x_i), in order to remove bias induced by those terms.");

  declareOption(ol, "remove_bias_in_evaluate", &KernelPCA::remove_bias_in_evaluate, OptionBase::buildoption,
      "If set to 1, the (additively) normalized kernel will not take into account terms\n"
      "of the form K(x_i,x_i), but only when evaluated on test points.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);

  // Redirect the 'kernel' option toward kpca_kernel.
  redeclareOption(ol, "kernel", &KernelPCA::kpca_kernel, OptionBase::buildoption,
      "The kernel used to (implicitly) project the data in feature space.");
  
  redeclareOption(ol, "ignore_n_first", &KernelPCA::ignore_n_first, OptionBase::nosave,
      "In KernelPCA, no eigenvector is ignored.");

  // And declare the normalized kernel so that it can be saved.
  declareOption(ol, "normalized_kernel", &KernelProjection::kernel, OptionBase::learntoption,
      "The normalized kernel.");

}

///////////
// build //
///////////
void KernelPCA::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void KernelPCA::build_()
{
  // Obtain the "real" kernel by additive normalization of 'kpca_kernel'.
  // We have to do this iff:
  // 1. A 'kpca_kernel' is provided, and
  // 2. either:
  //    2.a. the 'kernel' option is not set, or
  //    2.b. the 'kernel' option is not an AdditiveNormalization acting on 'kpca_kernel'.
  // This is to ensure that a loaded 'kernel' won't be overwritten.
  if (kpca_kernel &&
      (!kernel ||
       (dynamic_cast<AdditiveNormalizationKernel*>((Kernel*) kernel))->source_kernel != kpca_kernel)) {
    this->kernel = new AdditiveNormalizationKernel
      (kpca_kernel, remove_bias, remove_bias_in_evaluate, kernel_is_distance);
  }
}

////////////
// forget //
////////////
void KernelPCA::forget()
{
  inherited::forget();
}
    
/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void KernelPCA::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(kpca_kernel, copies);
}

} // end of namespace PLearn

