// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2003 Olivier Delalleau
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
 * $Id: LiftStatsCollector.h,v 1.2 2003/11/04 21:24:03 tihocan Exp $
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file LiftStatsCollector.h */

#ifndef LiftStatsCollector_INC
#define LiftStatsCollector_INC

#include "VecStatsCollector.h"

namespace PLearn <%
using namespace std;

class LiftStatsCollector: public VecStatsCollector
{

public:

  typedef VecStatsCollector inherited;

protected:

  Mat n_first_samples;

  //! Set to true after each call to finalize().
  bool is_finalized;

  //! Number of examples stored in the n_first_samples matrix.
  int nstored;

  //! Number of samples seen.
  int nsamples;

  //! Number of positive examples that are not retained in the ones with the
  //! highest output.
  int npos;

  //! Number of examples to keep (nsamples * lift_fraction).
  int n_samples_to_keep;

public:

  // ************************
  // * public build options *
  // ************************

  int output_column;
  int target_column;
  real lift_fraction;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  LiftStatsCollector();


  // ******************
  // * Object methods *
  // ******************

  // A few overrides to properly save the accumulated information
  virtual void forget();
  virtual void update(const Vec& x, real weight = 1.0);
//  using inherited::update; // TODO wtf

  //! This finalize override sorts the matrix by selection criterion
  //! in order to make it easier to compute the  // TODO change comment
  virtual void finalize();

  //! In addition to the regular VecStatsCollector statistics, we
  //! understand:
  //! - PROFIT_PERCENT[<number>]  Profit made when this % is ceeded to pool
  //! - PROFIT_THRESH[<number>]   Profit made when all scores greater than
  //!                             number are ceeded to pool
  //! - THRESH_PERCENT[<number>]  Threshold corresponding to % ceeded
  //! - OPTIMAL_PROFIT            Maximum possible profit at optimal % ceeded
  //! - OPTIMAL_THRESH            Optimal threshold yielding max profit
  //! - OPTIMAL_PERCENT           % ceeded yielding max profit // TODO change comments
  virtual double getStat(const string& statspec); // TODO wtf

protected:

  //! Return the LIFT statistic.
  real computeLift();
  
  //! Return the LIFT_MAX statistic.
  real computeLiftMax();

private: 

  //! This does the actual building. 
  void build_();

protected: 

  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:

  // Declares other standard object methods
  //  If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
  PLEARN_DECLARE_OBJECT(LiftStatsCollector);

  // simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(LiftStatsCollector);
  
%> // end of namespace PLearn

#endif
