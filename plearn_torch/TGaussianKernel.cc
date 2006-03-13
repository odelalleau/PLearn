// -*- C++ -*-

// TGaussianKernel.cc
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

/*! \file TGaussianKernel.cc */


#include "TGaussianKernel.h"

namespace PLearn {
using namespace std;

TGaussianKernel::TGaussianKernel() 
: gaussian_kernel(0),
  g(-1),
  sigma(1)
{
}

PLEARN_IMPLEMENT_OBJECT(TGaussianKernel,
    "Interface between PLearn and a Torch GaussianKernel object",
    "For the ease of use, we allow one to define either the original 'g' options,\n"
    "or alternatively to define the kernel bandwidth 'sigma'.\n"
);

void TGaussianKernel::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "sigma", &TGaussianKernel::sigma, OptionBase::buildoption,
      "The kernel bandwidth.");

  declareOption(ol, "g", &TGaussianKernel::g, OptionBase::buildoption,
      "If >= 0, will override sigma: sigma = 1 / sqrt(g).");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void TGaussianKernel::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
  if (sigma <= 0)
    PLERROR("In TGaussianKernel::build_ - 'sigma' must be > 0");
}

void TGaussianKernel::build()
{
  inherited::build();
  build_();
}

void TGaussianKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("TGaussianKernel::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//////////////////////
// updateFromPLearn //
//////////////////////
void TGaussianKernel::updateFromPLearn(Torch::Object* ptr) {
  if (ptr)
    gaussian_kernel = (Torch::GaussianKernel*) ptr;
  else {
    if (!gaussian_kernel)
      gaussian_kernel = new(allocator) Torch::GaussianKernel(g);
  }
  if (g >= 0)
    gaussian_kernel->g = g;
  else
    gaussian_kernel->g = 1.0 / (sigma * sigma);

  inherited::updateFromPLearn(gaussian_kernel);
}

/////////////////////
// updateFromTorch //
/////////////////////
void TGaussianKernel::updateFromTorch() {
  if (gaussian_kernel->g == 0)
    g = 0;
  else {
    assert( gaussian_kernel->g > 0 );
    sigma = 1.0 / sqrt(gaussian_kernel->g);
    if (g >= 0)
      g = gaussian_kernel->g;
  }

  inherited::updateFromTorch();
}

} // end of namespace PLearn
