// -*- C++ -*-

// LimitedGaussianSmoother.h
// 
// Copyright (C) 2002 Xavier Saint-Mleux
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
   * $Id: LimitedGaussianSmoother.h,v 1.6 2004/02/20 21:11:46 chrish42 Exp $ 
   ******************************************************* */

/*! \file LimitedGaussianSmoother.h */
#ifndef LimitedGaussianSmoother_INC
#define LimitedGaussianSmoother_INC

#include "Smoother.h"
#include <cmath>

namespace PLearn {
using namespace std;


// smoothed_function[k] = sum_{j=max(0,k-window_size)}^{min(l-1,k+window_size)} w_{k,j} source_function[j]
//                        / sum_{j=max(0,k-window_size)}^{min(l-1,k+window_size)} w_{k,j} 
// with w_{k,j} = phi(bin_positions[j+1];mu_k,sigma_k)-phi(bin_positions[j];mu_k,sigma_k)
// where mu_k = 0.5*(bin_positions[k+1]+bin_positions[k]),
//       sigma_k = bin_positions[k+window_size]-bin_positions[k]
// where phi(x;mu,sigma) = cdf of normal(mu,sigma) at x,
// window_size = window_size_wrt_sigma * sigma_bin

class LimitedGaussianSmoother: public Smoother
{
protected:
  // *********************
  // * protected options *
  // *********************

  real window_size_wrt_sigma, sigma_bin;

    
public:

  typedef Smoother inherited;

  // ************************
  // * public build options *
  // ************************

  // ### declare public option fields (such as build options) here
  // ...

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  LimitedGaussianSmoother();


  LimitedGaussianSmoother(real window_size_wrt_sigma_, real sigma_bin_);


  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:
  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Provides a help message describing this class
  static string help();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(LimitedGaussianSmoother);


  /****
   * Smoother methods
   */

 public:
  virtual real smooth(const Vec& source_function, Vec& smoothed_function, 
		      Vec bin_positions = Vec(), Vec dest_bin_positions = Vec()) const;


};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(LimitedGaussianSmoother);
  
} // end of namespace PLearn

#endif
