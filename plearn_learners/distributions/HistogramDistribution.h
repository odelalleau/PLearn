// -*- C++ -*-

// HistogramDistribution.h
// 
// Copyright (C) 2002 Yoshua Bengio, Pascal Vincent, Xavier Saint-Mleux
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
   * $Id: HistogramDistribution.h,v 1.3 2002/11/11 20:16:23 zouave Exp $ 
   ******************************************************* */

/*! \file HistogramDistribution.h */
#ifndef HistogramDistribution_INC
#define HistogramDistribution_INC

#include "Distribution.h"
#include "Binner.h"
#include "Smoother.h"

namespace PLearn <%
using namespace std;

class HistogramDistribution: public Distribution
{
protected:
public: //TEMP!!! s/b protected -xsm  
  // *********************
  // * protected options *
  // *********************

  //! there is one more bin position than number of bins, all the bins are supposed adjacent
  Vec bin_positions;

  //! the density is supposed constant within each bin:
  //! p(x) = bin_density[i] if bin_positions[i] < x <= bin_positions[i+1]
  Vec bin_density;

  //! redundant with density is the pre-computed (optional) survival fn
  Vec survival_values;

  //! this Binner is used to do binning at training time.
  PP<Binner> the_binner;

  //! this Smoother is used at training time.
  PP<Smoother> the_smoother;

  bool smooth_density; //if false, smoothing is done on survival

    
public:

  typedef Distribution inherited;

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
  HistogramDistribution();

  //! This constructor calls train as part of the construction process.
  //! The use function can then be used right away.
  HistogramDistribution(VMat data, PP<Binner> the_binner_= 0, PP<Smoother> the_smoother_= 0, bool smooth_density_= true);

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
  virtual string help() const;

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  DECLARE_NAME_AND_DEEPCOPY(HistogramDistribution);

  // *******************
  // * Learner methods *
  // *******************

  //! trains the model
  virtual void train(VMat training_set); 

  //! computes the ouptu of a trained model
  virtual void use(const Vec& input, Vec& output);

  // ************************
  // * Distribution methods *
  // ************************

  //! return log of probability density log(p(x))
  virtual double log_density(const Vec& x) const;

  //! return probability density p(x)
  //! [ default version returns exp(log_density(x)) ]
  //virtual double density(const Vec& x) const;
  
  //! return survival fn = P(X>x)
  virtual double survival_fn(const Vec& x) const;

  //! return survival fn = P(X<x)
  virtual double cdf(const Vec& x) const;

  //! return E[X] 
  virtual double expectation() const;

  //! return Var[X]
  virtual double variance() const;


protected:
  //the following methods are used internally by HistogramDistribution

  //! Find the bin where x belongs; -1 if x is out of range.
  int find_bin(real x) const;

  //! calculate bin_density from survival_values  
  void calc_density_from_survival();
  //! calculate survival_values from bin_density
  void calc_survival_from_density();

  
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(HistogramDistribution);
  
%> // end of namespace PLearn

#endif
