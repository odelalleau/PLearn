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
   * $Id: KernelPCA.cc,v 1.1 2004/05/07 19:01:58 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file KernelPCA.cc */

#include "AdditiveNormalizationKernel.h"
#include "GaussianKernel.h"
#include "KernelPCA.h"

namespace PLearn {
using namespace std;

//////////////////
// KernelPCA //
//////////////////
KernelPCA::KernelPCA() 
/* ### Initialize all fields to their default value here */
{
  // Usually, one will want only positive eigenvalues.
  min_eigenvalue = 0;
  kernel = 0; // When created, there exists no normalized kernel.
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
  // ### ex:
  // declareOption(ol, "myoption", &KernelPCA::myoption, OptionBase::buildoption,
  //               "Help text describing this option");
  // ...

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);

  // Redirect the 'kernel' option toward kpca_kernel.
  redeclareOption(ol, "kernel", &KernelPCA::kpca_kernel, OptionBase::buildoption,
      "The kernel used to (implicitly) project the data in feature space.");
  
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
    this->kernel = new AdditiveNormalizationKernel(kpca_kernel);
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
void KernelPCA::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("KernelPCA::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn

