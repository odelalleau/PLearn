// -*- C++ -*-

// Smoother.h
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
   * $Id: Smoother.h,v 1.8 2004/09/14 16:04:37 chrish42 Exp $ 
   ******************************************************* */

/*! \file Smoother.h */
#ifndef Smoother_INC
#define Smoother_INC

#include <plearn/base/Object.h>
#include "TVec.h"

namespace PLearn {
using namespace std;

class Smoother: public Object
{
protected:
  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...
    
public:

  typedef Object inherited;

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
  Smoother();


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

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_ABSTRACT_OBJECT(Smoother);


  /****
   * Smoother methods
   */

 public:
  // The source function is either f(i) = source_function[i] as a function of i
  // or if bin_positions is provided (non-zero length), 
  //    f(x) = source_function[i]
  //      where i is s.t. bin_positions[i]>x>=bin_positions[i+1]
  // the optional bin_positions vector has length 0, or 1 more than source_function.
  // By default (if not provided) the dest_bin_positions are assumed the same as the source bin_positions.
  // Returns integral(smoothed_function).
  virtual real smooth(const Vec& source_function, Vec& smoothed_function, 
		      Vec bin_positions = Vec(), Vec dest_bin_positions = Vec()) const = 0;

  //   real smooth(const HistogramCDF& source_cdf, HistogramCDF& dest_cdf);

};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(Smoother);
  
} // end of namespace PLearn

#endif
