// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Pascal Vincent
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
   * $Id: GaussianDistribution.cc,v 1.13 2005/03/07 15:40:17 chapados Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnAlgo/GaussianDistribution.cc */

#include "GaussianDistribution.h"
//#include "fileutils.h"
#include <plearn/vmat/VMat_basic_stats.h>
#include <plearn/math/plapack.h>
#include <plearn/math/distr_maths.h>
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;

#define ZEROGAMMA

PLEARN_IMPLEMENT_OBJECT(GaussianDistribution, "ONE LINE DESCR", "NO HELP");

void GaussianDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(mu, copies);
  deepCopyField(eigenvalues, copies);
  deepCopyField(eigenvectors, copies);
}


GaussianDistribution::GaussianDistribution()
  :k(1000), gamma(0), ignore_weights_below(0)
{
}


void GaussianDistribution::declareOptions(OptionList& ol)
{
  // Build options
  declareOption(ol, "k", &GaussianDistribution::k, OptionBase::buildoption, 
                "number of eigenvectors to keep");
  declareOption(ol, "gamma", &GaussianDistribution::gamma, OptionBase::buildoption, 
                "Add this to diagonal of empirical covariance matrix.\n"
                "The actual covariance matrix used will be VDV' + gamma.I \n"
                "where V'=eigenvectors and D=diag(eigenvalues).");
  declareOption(ol, "ignore_weights_below", &GaussianDistribution::ignore_weights_below, OptionBase::buildoption, 
                "When doing a weighted fitting (weightsize==1), points with a weight below this value will be ignored");

  // Learnt options
  declareOption(ol, "mu", &GaussianDistribution::mu, OptionBase::learntoption, "");
  declareOption(ol, "eigenvalues", &GaussianDistribution::eigenvalues, OptionBase::learntoption, "");
  declareOption(ol, "eigenvectors", &GaussianDistribution::eigenvectors, OptionBase::learntoption, "");

  inherited::declareOptions(ol);
}                

void GaussianDistribution::forget()
{ }

void GaussianDistribution::train()
{
  VMat training_set = getTrainingSet();
  int l = training_set.length();
  int d = training_set.width();
  int ws = training_set->weightsize();

  if(d!=inputsize()+ws)
    PLERROR("In GaussianDistribution::train width of training_set should be equal to inputsize()+weightsize()");

  // these are used in SVD
  static Mat trainmat;
  static Mat U;

  // The maximum number of eigenvalues we want.
  int maxneigval = min(k+1, min(l,d));

  // First get mean and covariance
  // (declared static to avoid repeated dynamic memory allocation)
  static Mat covarmat;

  if(ws==0)
    computeMeanAndCovar(training_set, mu, covarmat);
  else if(ws==1)
    computeWeightedMeanAndCovar(training_set, mu, covarmat, ignore_weights_below);
  else
    PLERROR("In GaussianDistribution, weightsize can only be 0 or 1");
      
  // cerr << "maxneigval: " << maxneigval << " ";
  // cerr << eigenvalues.length() << endl;
  // cerr << "eig V: \n" << V << endl;

  // Compute eigendecomposition only if there is a training set...
  // Otherwise, just fill the eigen-* matrices to all NaN...
  if (l > 0)
    eigenVecOfSymmMat(covarmat, maxneigval, eigenvalues, eigenvectors);
  else {
    eigenvalues.resize(maxneigval);
    eigenvectors.resize(maxneigval, mu.size());
    eigenvalues.fill(0);
    eigenvectors.fill(0);
  }
}

real GaussianDistribution::log_density(const Vec& x) const 
{ 
  return logOfCompactGaussian(x, mu, eigenvalues, eigenvectors, gamma, true);
}


void GaussianDistribution::resetGenerator(long g_seed) const
{
  manual_seed(g_seed);
}

void GaussianDistribution::generate(Vec& x) const
{
  static Vec r;
  int n = eigenvectors.length();
  int m = mu.length();
  r.resize(n);
  fill_random_normal(r);
  for(int i=0; i<n; i++)
    r[i] *= sqrt(eigenvalues[i]);
  x.resize(m);
  transposeProduct(x,eigenvectors,r);
  r.resize(m);
  fill_random_normal(r,0,gamma);
  x += r;
  x += mu;
}

///////////////
// inputsize //
///////////////
int GaussianDistribution::inputsize() const {
  if (train_set || mu.length() == 0)
    return inherited::inputsize();
  return mu.length();
}

} // end of namespace PLearn
