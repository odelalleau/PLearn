// -*- C++ -*-

// ConditionalGaussMix.h
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
   * $Id: ConditionalGaussMix.h,v 1.1 2004/05/19 17:21:05 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file ConditionalGaussMix.h */


#ifndef ConditionalGaussMix_INC
#define ConditionalGaussMix_INC

#include "GaussMix.h"
#include "PConditionalDistribution.h"

namespace PLearn {
using namespace std;

class ConditionalGaussMix: public PConditionalDistribution
{

private:

  typedef PConditionalDistribution inherited;  

protected:

  // *********************
  // * protected options *
  // *********************

  Mat eigenvalues_x;
  TVec<Mat> eigenvectors_x;
  Vec sigma;

public:

  // ************************
  // * public build options *
  // ************************

  PP<GaussMix> gauss_mix;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  ConditionalGaussMix();

  // ************************************
  // * PConditionalDistribution methods *
  // ************************************

private: 

  //! This does the actual building. 
  void build_();

protected: 

  //! Declare this class' options.
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply call inherited::build() then build_().
  virtual void build();

  //! Transform a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declare other standard object methods.
  PLEARN_DECLARE_OBJECT(ConditionalGaussMix);

  // ******************************************
  // **** PConditionalDistribution methods ****
  // ******************************************

  //! Set the input part (x in p(y | x)).
  virtual void setInput(const Vec& input) const;

  //! Return log of probability density log(p(y | x)).
  virtual double log_density(const Vec& y) const;

  //! Return survival fn = P(Y>y | x).
  virtual double survival_fn(const Vec& y) const;

  //! Return survival fn = P(Y<y | x).
  virtual double cdf(const Vec& y) const;

  //! Return E[Y | x].
  virtual void expectation(Vec& mu) const;

  //! Return Var[Y | x].
  virtual void variance(Mat& cov) const;

  //! Reset the random number generator used by generate() using the given seed.
  virtual void resetGenerator(long g_seed) const;

  //! Return a pseudo-random sample generated from the distribution.
  virtual void generate(Vec& x) const;

  // **************************
  // **** Learner methods ****
  // **************************

  // ### Default version of inputsize returns learner->inputsize()
  // ### If this is not appropriate, you should uncomment this and define
  // ### it properly in the .cc
  // virtual int inputsize() const;

  //! (Re-)initializes the PConditionalDistribution in its fresh state (that state may depend on the 'seed' option).
  //! And sets 'stage' back to 0 (this is the stage of a fresh learner!).
  // ### You may remove this method if your distribution does not implement it.
  virtual void forget();
    
  //! The role of the train method is to bring the learner up to stage == nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  // ### You may remove this method if your distribution does not implement it.
  virtual void train();

};

// Declare a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(ConditionalGaussMix);
  
} // end of namespace PLearn

#endif
