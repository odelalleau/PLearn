// -*- C++ -*-

// RandomGenerator.cc
//
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, University of Montreal
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file RandomGenerator.cc */

#define __STDC_LIMIT_MACROS     //!< For UINT32_MAX.
#include <stdint.h>

// Constants used for random numbers generation.
#define RAND_EPS 1.2e-7
#define RAND_RNMX (1.0 - RAND_EPS)


#include "RandomGenerator.h"

namespace PLearn {
using namespace std;

/////////////////////
// RandomGenerator //
/////////////////////
RandomGenerator::RandomGenerator() 
: rgen(),
  uniform_01(rgen),
  the_seed(0),
  seed_(-1)
{}

PLEARN_IMPLEMENT_OBJECT(RandomGenerator,
    "Perform a number of random operations, including generating random numbers",
    ""
);

////////////////////
// declareOptions //
////////////////////
void RandomGenerator::declareOptions(OptionList& ol)
{
  declareOption(ol, "seed", &RandomGenerator::seed_, OptionBase::buildoption,
      "Seed for the random number generator, set at build time:\n"
      " - -1      : initialized with the current CPU time\n"
      " -  0      : the current seed is left intact\n"
      " -  x > 0  : the seed is changed to 'x'");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

/////////////////////
// bounded_uniform //
/////////////////////
real RandomGenerator::bounded_uniform(real a, real b) {
  real res = uniform_sample()*(b-a) + a;
  if (res >= b)
    return b*RAND_RNMX;
  else
    return res;
}

///////////
// build //
///////////
void RandomGenerator::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void RandomGenerator::build_()
{
  if (seed_ == -1)
    this->seed();
  else if (seed_ == 0) {}
  else if (seed_ > 0)
    this->manual_seed(seed_);
  else
    PLERROR("In RandomGenerator::build_ - The 'seed' option must be set to "
            "-1, 0 or a positive value");
}

/////////////////
// gaussian_01 //
/////////////////
real RandomGenerator::gaussian_01() {
  return real(normal_distribution(rgen));
}

///////////////////////
// gaussian_mu_sigma //
///////////////////////
real RandomGenerator::gaussian_mu_sigma(real mu, real sigma) {
  return gaussian_01() * sigma + mu;
}

//////////////
// get_seed //
//////////////
long RandomGenerator::get_seed() {
  return long(the_seed);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RandomGenerator::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("RandomGenerator::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////////////
// manual_seed //
/////////////////
void RandomGenerator::manual_seed(long x)
{
  if (x >= long(UINT32_MAX))
    PLERROR("In RandomGenerator::manual_seed - The seed cannot be more than %d", UINT32_MAX);
  the_seed = uint32_t(x);
  rgen.seed(the_seed);
}

////////////////////////
// multinomial_sample //
////////////////////////
int RandomGenerator::multinomial_sample(const Vec& distribution) {
  real  u  = this->uniform_sample();
  real* pi = distribution.data();
  real  s  = *pi;
  int   n  = distribution.length();
  int   i  = 0;
  while ((i<n) && (s<u)) {
    i++;
    pi++;
    s += *pi;
  }
  if (i == n)
    i = n - 1; // Improbable, but...
  return i;
}


//////////
// seed //
//////////
void RandomGenerator::seed()
{
  time_t ltime;
  struct tm *today;
  time(&ltime);
  today = localtime(&ltime);
  manual_seed((long)today->tm_sec+
      60*today->tm_min+
      60*60*today->tm_hour+
      60*60*24*today->tm_mday);
}

////////////////////
// uniform_sample //
////////////////////
real RandomGenerator::uniform_sample() {
  return real(uniform_01());
}

} // end of namespace PLearn
