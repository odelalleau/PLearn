// -*- C++ -*-

// EpanechnikovKernel.cc
//
// Copyright (C) 2004 Nicolas Chapados 
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
   * $Id: EpanechnikovKernel.cc,v 1.3 2004/12/25 08:02:03 chapados Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file EpanechnikovKernel.cc */


#include "EpanechnikovKernel.h"

namespace PLearn {
using namespace std;

EpanechnikovKernel::EpanechnikovKernel()
  : gamma(1)
{
}

PLEARN_IMPLEMENT_OBJECT(
  EpanechnikovKernel,
  "Classical Epanechnikov kernel for local regression",
  "The Epanechnikov kernel is very appropriate for locally-weighted regression\n"
  "and nearest-neighbors problems.  It is designed to have finite support, unlike\n"
  "the GaussianKernel, and integrates to 1. \n"
  "(For examples of use, see KNNRegressor, KNNClassifier, and\n"
  "classes derived from GenericNearestNeighbors.)\n"
  "\n"
  "In each dimension, the Epanechnikov kernel is defined as follows:\n"
  "    K_gamma(x0,x) = D(|x-x0|/gamma) \n"
  "where\n"
  "    D(t) = 3/4 (1-t^2),   if |t| <= 1;\n"
  "         = 0          ,   otherwise,\n"
  "with the user-specified gamma a smoothing parameter.\n");


////////////////////
// declareOptions //
////////////////////
void EpanechnikovKernel::declareOptions(OptionList& ol)
{
  declareOption(ol, "gamma", &EpanechnikovKernel::gamma,
                OptionBase::buildoption,
                "Smoothing parameter for the Epanechnikov kernel (default=1.0)");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void EpanechnikovKernel::build()
{
  // ### Nothing to add here, simply calls build_
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void EpanechnikovKernel::build_()
{
  if (gamma <= 0.0)
    PLERROR("EpanechnikovKernel::build_: the 'gamma' option must be strictly positive");
}

//////////////
// evaluate //
//////////////
real EpanechnikovKernel::evaluate(const Vec& x1, const Vec& x2) const
{
  real t = L2distance(x1,x2) / gamma;
  if (t <= 1.0)
    return 3 * (1-t*t) / 4;
  return 0;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void EpanechnikovKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

} // end of namespace PLearn
