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
 * $Id: LiftStatsCollector.h,v 1.13 2004/11/23 21:31:16 tihocan Exp $
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file LiftStatsCollector.h */

#ifndef LiftStatsCollector_INC
#define LiftStatsCollector_INC

#include "VecStatsCollector.h"

namespace PLearn {
using namespace std;

class LiftStatsCollector: public VecStatsCollector
{

private:

  typedef VecStatsCollector inherited;

protected:

  // Protected options.
  
  int count_fin;
  Vec roc_values;

  // Fields below are not options.

  //! Matrix storing the output and target of the samples with highest output,
  //! as well as all the other data retrieved since the last call to finalize.
  Mat n_first_updates;

  //! Set to true after each call to finalize().
  bool is_finalized;

  //! Number of examples stored in the n_first_updates matrix.
  int nstored;

  //! Number of samples seen.
  int nsamples;

  //! Number of positive examples that are not retained in the ones with the
  //! highest output (that is to say the ones in n_first_updates).
  int npos;

  //! Number of examples to keep (nsamples * lift_fraction).
  int n_samples_to_keep;

  //! Index of the output column.
  int output_column_index;

public:

  // ************************
  // * public build options *
  // ************************

  string lift_file;
  real lift_fraction;
  bool opposite_lift;
  string output_column;
  string roc_file;
  Vec roc_fractions;
  int sign_trick;
  int target_column;
  int verbosity;

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

  //! This finalize override makes sure only the n_samples_to_keep samples
  //! from the matrix n_first_updates with the highest output are left.
  virtual void finalize();

  //! In addition to the regular VecStatsCollector statistics, we
  //! understand specific lift statistics (see the .cc).
  virtual double getStat(const string& statspec);

  //! Overridden because it is not supported in this VecStatsCollector.
  virtual void remove_observation(const Vec& x, real weight = 1.0);

protected:

  //! Return the AUC statistic.
  real computeAUC();

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
  PLEARN_DECLARE_OBJECT(LiftStatsCollector);

  // simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(LiftStatsCollector);
  
} // end of namespace PLearn

#endif
