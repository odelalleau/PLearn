// -*- C++ -*-

// GaussMixLocalProjections.cc
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
   * $Id: GaussMixLocalProjections.cc,v 1.1 2005/03/03 20:09:19 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file GaussMixLocalProjections.cc */


#include "GaussMixLocalProjections.h"

namespace PLearn {
using namespace std;

//////////////////////////////
// GaussMixLocalProjections //
//////////////////////////////
GaussMixLocalProjections::GaussMixLocalProjections() 
: n_components(-1)
{
}

PLEARN_IMPLEMENT_OBJECT(GaussMixLocalProjections,
    "Train a Gaussian mixture and output the corresponding local projections.",
    "This learner just takes a mixture of Gaussians and gives in output a vector\n"
    "which is the concatenation of the local coordinates computed for each Gaussian\n"
    "in the mixture and weighted by the posterior P(k|x).\n"
    "At the beginning of each of those coordinates a bias (1) is added, then the\n"
    "coordinate is multiplied by the 'responsibility' of the Gaussian, i.e. its\n"
    "posterior P(k|x).\n"
    "The local coordinates are obtained by projecting on the eigenvectors associated\n"
    "to non-zero eigenvalues in the covariance matrix.\n"
);

////////////////////
// declareOptions //
////////////////////
void GaussMixLocalProjections::declareOptions(OptionList& ol)
{
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // Build options.

  // declareOption(ol, "myoption", &GaussMixLocalProjections::myoption, OptionBase::buildoption,
  //               "Help text describing this option");
  // ...

  // Learnt options.

  declareOption(ol, "n_components", &GaussMixLocalProjections::n_components, OptionBase::learntoption,
      "Equal to learner->L, i.e. the number of components in the mixture.");

  declareOption(ol, "outputsizes", &GaussMixLocalProjections::outputsizes, OptionBase::learntoption,
      "The size of the projection for each Gaussian, i.e. learner->n_eigen.");

  // Now call the parent class' declareOptions.
  inherited::declareOptions(ol);

  redeclareOption(ol, "seed", &GaussMixLocalProjections::seed_, OptionBase::nosave,
      "No need for a seed.");

  redeclareOption(ol, "nstages", &GaussMixLocalProjections::nstages, OptionBase::nosave,
      "One only needs to specify mixture->nstages.");

}

///////////
// build //
///////////
void GaussMixLocalProjections::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void GaussMixLocalProjections::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
  if (learner_) {
    if (learner_->classname() != "GaussMix")
      PLERROR("In GaussMixLocalProjections::build_ - A GaussMix learner is needed");
    gauss_mix = (GaussMix*) (PLearner*) learner_;
    if (gauss_mix->type != "general")
      PLERROR("In GaussMixLocalProjections::build_ - The underlying GaussMix "
              "distribution must be of type 'general'");
    n_components = gauss_mix->L;
    outputsizes.resize(n_components);
    outputsizes.fill(gauss_mix->n_eigen >= 0 ? gauss_mix->n_eigen : inputsize());
  }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void GaussMixLocalProjections::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
  // No cost computed.
}                                

///////////////////
// computeOutput //
///////////////////
void GaussMixLocalProjections::computeOutput(const Vec& input, Vec& output) const
{
  Mat eigen_vec;
  int size_k = gauss_mix->n_eigen > 0 ? gauss_mix->n_eigen : input.length();
  output.resize((size_k + 1) * gauss_mix->L);
  int index = 0;
  // Compute densities in order to be able to get posteriors.
  real log_density = gauss_mix->log_density(input);
  Vec log_likelihood_dens = gauss_mix->getLogLikelihoodDens();
  for (int k = 0; k < gauss_mix->L; k++) {
    // Obtain the (right number of) eigenvectors.
    output[index] = 1.0;
    eigen_vec = gauss_mix->getEigenvectors(k).subMatRows(0, size_k);
    // Compute local coordinates.
    product(output.subVec(index+1, size_k), eigen_vec, input);
    // Scale by the responsibility.
    output.subVec(index, size_k + 1) *= exp(log_likelihood_dens[k] - log_density);
    index += size_k + 1;
  }
}    

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> GaussMixLocalProjections::getTestCostNames() const
{
  static TVec<string> noCost;
  return noCost;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> GaussMixLocalProjections::getTrainCostNames() const
{
  static TVec<string> noCost;
  return noCost;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GaussMixLocalProjections::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("GaussMixLocalProjections::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////
// outputsize //
////////////////
int GaussMixLocalProjections::outputsize() const
{
  return sum(outputsizes) + n_components;
}

} // end of namespace PLearn
