// -*- C++ -*-

// UniformDistribution.cc
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
   * $Id: UniformDistribution.cc,v 1.5 2004/07/21 16:30:56 chrish42 Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file UniformDistribution.cc */

#include <plearn/math/random.h>
#include "UniformDistribution.h"

namespace PLearn {
using namespace std;

/////////////////////////
// UniformDistribution //
/////////////////////////
UniformDistribution::UniformDistribution() 
{
  // Default = generate points uniformly in [0,1].
  min.resize(1);
  max.resize(1);
  min[0] = 0;
  max[0] = 1;
}

PLEARN_IMPLEMENT_OBJECT(UniformDistribution,
    "Implements uniform distribution over intervals.",
    "Currently, only very few methods are implemented.\n"
    "For example, to sample points in 2D in [a,b] x [c,d], use\n"
    " min = [a c]\n"
    " max = [b d]\n"
);

////////////////////
// declareOptions //
////////////////////
void UniformDistribution::declareOptions(OptionList& ol)
{
  declareOption(ol, "min", &UniformDistribution::min, OptionBase::buildoption,
      "The inferior bound for all intervals.");

  declareOption(ol, "max", &UniformDistribution::max, OptionBase::buildoption,
      "The superior bound for all intervals.");

  // Now call the parent class' declareOptions().
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void UniformDistribution::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void UniformDistribution::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.

  // Check consistency of intervals.
  if (min.length() != max.length()) {
    PLERROR("In UniformDistribution::build_ - 'min' and 'max' should have the same size");
  }
  int n_dim = min.length();
  for (int i = 0; i < n_dim; i++) {
    if (min[i] > max[i]) {
      PLERROR("In UniformDistribution::build_ - 'min' should be always <= 'max'");
    }
  }
}

/////////
// cdf //
/////////
real UniformDistribution::cdf(const Vec& x) const
{
  PLERROR("cdf not implemented for UniformDistribution"); return 0;
}

/////////////////
// expectation //
/////////////////
void UniformDistribution::expectation(Vec& mu) const
{
  PLERROR("expectation not implemented for UniformDistribution");
}

//////////////
// generate //
//////////////
void UniformDistribution::generate(Vec& x) const
{
  x.resize(min.length());
  for (int i = 0; i < min.length(); i++) {
    x[i] = bounded_uniform(min[i], max[i]);
  }
}

///////////////
// inputsize //
///////////////
int UniformDistribution::inputsize() const {
  return min.length();
}

/////////////////
// log_density //
/////////////////
real UniformDistribution::log_density(const Vec& x) const
{
  PLERROR("density not implemented for UniformDistribution"); return 0; 
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void UniformDistribution::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("UniformDistribution::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// resetGenerator //
////////////////////
void UniformDistribution::resetGenerator(long g_seed) const
{
  manual_seed(g_seed);
}

/////////////////
// survival_fn //
/////////////////
real UniformDistribution::survival_fn(const Vec& x) const
{
  PLERROR("survival_fn not implemented for UniformDistribution"); return 0;
}

//////////////
// variance //
//////////////
void UniformDistribution::variance(Mat& covar) const
{
  PLERROR("variance not implemented for UniformDistribution");
}

} // end of namespace PLearn

