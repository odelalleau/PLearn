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
 * $Id: LiftStatsCollector.h,v 1.1 2003/11/04 18:13:39 tihocan Exp $
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

  //! Store the n samples with the highest output.
  Mat n_first_samples;

  //! After finalize, this matrix contains the same columns as all_updates
  //! plus an additional column containing the selection_criterion.
  //! Matrix is sorted according to selection_criterion
/*  bool is_finalized; // TODO useful
  Mat sorted_updates; // TODO useful 

  double total_premium;                      //!< Total premium after finalize // TODO useful
  double total_claim;                        //!< Total claim after finalize// TODO useful
  double total_duration;                     //!< Total duration after finalize// TODO useful */
  
public:

  // ************************
  // * public build options *
  // ************************

  int output_column;


  //! What is the learner trying to predict?
/*  static const int PurePremium = 1;          //!< predict one-year premium// TODO useful
  static const int RelPRRProfit = 2;         //!< predict (claim-PRR_LR*premium)/premium// TODO useful
  static const int LossRatio = 3;            //!< predict claim/premium// TODO useful
  static const int LossRatioDiv1000 = 4;     //!< predict (claim/premium)/1000// TODO useful
  static const int ProbabilityCeede = 5;     //!< between 0 and 1, with 1 "should ceede for sure"// TODO useful
  int output_type;                           //!< one of the 4 values above// TODO useful

  //! Preferred sorting method (may not be honoured if the learner
  //! cannot handle it, e.g. for a probabilistic classifier)
  static const int SortNatural = 0;          //!< Most "natural" sort for output_type// TODO useful
  static const int SortLossRatio = 1;        //!< sort by decreasing loss ratio// TODO useful
  static const int SortPRRProfit = 2;        //!< sort by decrasing profit// TODO useful
  int sort_type;// TODO useful
  
  //! What loss ratio are we going to use in the PRR profit formula
  //! (default value is 0.8)
  double prr_loss_ratio;// TODO useful

  //! Column number of model output in cost vector
  int output_column;// TODO useful

  //! Column number of claim in cost vector. We assume UNSCALED CLAIM.
  int claim_column;// TODO useful

  //! Column number of charged premium in cost vector.  We assume the
  //! UNSCALED premium actually charged to driver, irrespective of
  //! duration.
  int premium_column;// TODO useful

  //! Column number of duration (if necessary, depending on target type)
  int duration_column;// TODO useful

  //! Column number of a boolean flag which, if 1, indicates that
  //! the policy cannot be ceeded to PRR regardless of risk level
  int ceede_forbidden_column;// TODO useful
  
  //! True if claim has been normalized by duration
  bool duration_normalized_claim;// TODO useful

  //! Claim multiplier: e.g. =1000 if claim is in thousands of dollars
  double claim_multiplier;// TODO useful

  //! Minimum fraction of premium volume to ceede to pool
  double min_ceede;// TODO useful

  //! Maximum fraction of premium volume to ceede to pool
  double max_ceede;// TODO useful

  //! Name of file in which to save sorted scores
  string score_filename;// TODO useful */
  

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
  virtual void update(const Vec& x, real weight = 1.0); // TODO change ?
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

public:
  //! Core optimizer: returns the maximal profit, the associated threshold,
  //! and the associated premium fraction
/*  void optimizeWorld(double& max_profit, double& opt_threshold,
                     double& opt_fraction); // TODO wtf */
  
  //! Compute the actual profit arising out of ceeding everything beyond
  //! a list of thresholds.  For performance reasons, the thresholds should
  //! be sorted in DECREASING order.
/*  Vec profitFromThreshold(const Vec& thresholds); // TODO wtf */

  //! Actual profit, for a single threshold
/*  double profitFromThreshold(double threshold) {
    Vec t(1);
    t[0] = threshold;
    return profitFromThreshold(t)[0]; // TODO wtf
  } */

  //! Compute the thresholds corresponding to the specified fraction ceeded.
  //! For performance reasons, the fractions should be sorted in
  //! INCREASING ORDER.  The returned matrix contains two rows: the first
  //! row contains the thresholds, and the second one contains the
  //! corresponding the EFFECTIVE fraction ceeded (each one corresponding 
  //! to the NOMINAL fraction ceeded passed as argument).
/*   Mat thresholdFromFraction(const Vec& fractions); // TODO wtf */

  //! Actual threshold, for a single fraction
/*  double thresholdFromFraction(double fraction) {
    Vec t(1);
    t[0] = fraction;
    return (thresholdFromFraction(t))(0,0);
  } // TODO wtf

  //! Return the profit made for the given row, GIVEN that we ceede it to
  //! the pool. This is given by (claim - PRR_LR*premium)
  double prrSingleProfit(int row_number) const; // TODO wtf

  //! Return the REAL claim: undo all normalization by duration and such
  double actualClaim(int row_number) const; // TODO wtf

  //! Return the selection criterion for a given row number)
  double selectionCriterion(int row_number) const {
    return selectionCriterion(all_updates(row_number, output_column),
                              output_type,
                              all_updates(row_number, premium_column),
                              all_updates(row_number, duration_column),
                              prr_loss_ratio, sort_type);
  } // TODO wtf
  
  //! Return the "selection criterion" (which will be thresholded to
  //! make a decision) depending on the target type.  Static with explicit
  //! interface since this is called from outside.
  //! The meaning of criterion varies depending on output_type :
  //! 1. PurePremium:      (possibly scaled) loss ratio
  //! 2. RelPRRProfit:     loss ratio - PRR_LR (e.g. PRR_LR=0.8)
  //! 3. LossRatio:        loss ratio
  //! 4. LossRatioDiv1000: loss ratio divided by 1000
  //! 5. ProbabilityCeede: probability beyond which should ceede
  static double selectionCriterion(real model_output, int output_type,
                                   real nonflexed_premium, real duration,
                                   real prr_loss_ratio = 0.8,
                                   int sort_type = SortLossRatio); // TODO wtf

  //! Return the pure premium given by a model, depending on the actual
  //! model output, for observation row_number.  Static with explicit
  //! interface since this is called from outside.
  static double purePremium(real model_output, int output_type,
                            real nonflexed_premium, real duration,
                            real prr_loss_ratio); // TODO wtf */

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
