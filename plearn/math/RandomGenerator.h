// -*- C++ -*-

// PRandom.h
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

/*! \file PRandom.h */


#ifndef PRandom_INC
#define PRandom_INC

#include <plearn/base/Object.h>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/uniform_01.hpp>

namespace PLearn {

class PRandom: public Object
{

private:
  
  typedef Object inherited;

protected:

  //! The underlying Boost random number generator used.
  boost::mt19937 rgen;

  //! The underlying Boost distribution for uniform sampling.
  boost::uniform_01<boost::mt19937>* uniform_01;

  //! The underlying Boost distribution for normal sampling.
  boost::normal_distribution<> normal_distribution;

  //! The actual seed used by the random number generator.
  uint32_t the_seed;
    
  // *********************
  // * protected options *
  // *********************

public:

  // ************************
  // * public build options *
  // ************************

  long seed_;

  // ****************
  // * Constructors *
  // ****************

  //! Constructor from a given seed.
  PRandom(long seed = -1);

  //! Destructor to free memory.
  virtual ~PRandom();

  // ******************
  // * Object methods *
  // ******************

private: 

  //! This does the actual building. 
  void build_();

protected: 

  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:

  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(PRandom);

  //! Build object.
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  //! Initialize the random number generator with the CPU time.
  void seed();
  //! Initialize the random number generator with the given long 'x'.
  void manual_seed(long x);
  //! Return the current seed used by the random number generator.
  long get_seed();

  //! Return a random number uniformly distributed between 0 and 1.
  real uniform_sample();
  //! Return a random number uniformly distributed between a and b.
  real bounded_uniform(real a, real b);

  //! Return a random number generated from a Gaussian with mean 0 and stddev 1.
  real gaussian_01();
  inline real normal_sample() { return this->gaussian_01(); }
  //! Return a random number generated from a Gaussian with mean mu and stddev sigma.
  real gaussian_mu_sigma(real mu, real sigma);

  /* TODO Implement.
  //! Return a random number generated from an exponential distribution.
  real exp_sample();
  //! Return a random number generated from a gamma distribution.
  real gamma_sample(int ia);
  //! Return a random number generated from a Poisson distribution.
  real poisson_sample(real xm);
  //! Return a random number generated from a binomial distribution with
  //! probability 'pp' and 'n' trials.
  real binom_sample(real pp, int n = 1);
  */

  //! Return a random deviate from a discrete distribution given explicitely in the
  //! 'distribution' vector. The returned value is an index in 'distribution'.
  //! Elements of 'distribution' must sum to 1.
  int multinomial_sample(const Vec& distribution);
  //! Return an integer between 0 and n-1 with equal probabilities.
  inline int uniform_multinomial_sample(int n)
    { return int(n * this->uniform_sample()); }

  //! Randomly shuffle the entries of a vector.
  template<class T>
  void shuffleElements(const TVec<T>& vec) {
    T* v = vec.data();
    T tmp;
    int n = vec.length();
    for (int i = 0; i < vec.length(); i++) {
      int j = i + this->uniform_multinomial_sample(n - i);
      tmp = v[i];
      v[i] = v[j];
      v[j] = tmp;
    }
  }

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PRandom);
  
} // end of namespace PLearn

#endif
