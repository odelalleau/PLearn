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
 * $Id: LiftStatsCollector.cc,v 1.1 2003/11/04 18:13:39 tihocan Exp $
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file LiftStatsCollector.cc */

#include "LiftStatsCollector.h"
//#include "VMat.h"
//#include "MemoryVMatrix.h"
//#include "stringutils.h"
//#include "TMat_maths.h"                      //!< for sortRows

namespace PLearn <%
using namespace std;

////////////////////////
// LiftStatsCollector //
////////////////////////
LiftStatsCollector::LiftStatsCollector() 
  : inherited() /*, all_updates(), is_finalized(false), sorted_updates(),
    output_type(-1), sort_type(SortNatural), prr_loss_ratio(0.8),
    output_column(0), claim_column(1), premium_column(2),
    duration_column(3), ceede_forbidden_column(4),
    duration_normalized_claim(false),
    claim_multiplier(1.0), min_ceede(0.0), max_ceede(1.0),
    score_filename("") */
{
}

//////////////////
// Object stuff //
//////////////////
PLEARN_IMPLEMENT_OBJECT(
  LiftStatsCollector,
  "Computes the performance of a binary classifier",
  "The following statistics can be requested out of getStat():\n"
  "- LIFT = % of positive examples in the first n samples, divided by the % of positive examples in the whole database\n"
  "- LIFT_MAX = best performance that could be achieved, if all positive examples where selected in the first n samples\n"
  );

void LiftStatsCollector::declareOptions(OptionList& ol)
{
/*  declareOption(ol, "all_updates", &LiftStatsCollector::all_updates,
      OptionBase::learntoption,
      "Matrix accumulating all updates made to the stats "
      "collector since the last update"); */

  declareOption(ol, "output_column", &LiftStatsCollector::output_column, OptionBase::buildoption,
      "    the column in which is the output value\n");

/*  declareOption(ol, "target_column", &LiftStatsCollector::target_column), OptionBase::buildoption,
      "    the column in which is the target value\n");*/ // TODO see if we use this

/*  declareOption(ol, "output_type", &LiftStatsCollector::output_type,
                OptionBase::buildoption,
                "Type of function learned by the model. Can be one of:\n"
                "1 : (LiftStatsCollector::PurePremium),      predict one-year premium\n"
                "2 : (LiftStatsCollector::RelPRRProfit),     predict (claim-PRR_LR*premium)/premium\n"
                "3 : (LiftStatsCollector::LossRatio),        predict claim/premium\n"
                "4 : (LiftStatsCollector::LossRatioDiv1000), predict (claim/premium)/1000\n"
                "5 : (LiftStatsCollector::ProbabilityCeede), from 0 (don't ceede) to 1 (ceede)");

  declareOption(ol, "sort_type", &LiftStatsCollector::sort_type,
                OptionBase::buildoption,
                "Type of sorting to perform. Can be one of:\n"
                "0 : (LiftStatsCollector::SortNatural),   use most \"natural\" sort for output_type\n"
                "1 : (LiftStatsCollector::SortLossRatio), sort by decreasing predicted loss ratio\n"
                "2 : (LiftStatsCollector::SortPRRProfit), sort by decrasing (pure_premium - PRR_LR*premium)\n"
                "NOTE: not all sort types are supported by all output types");

  declareOption(ol, "prr_loss_ratio", &LiftStatsCollector::prr_loss_ratio,
                OptionBase::buildoption,
                "Loss ratio above which ceeding to PRR is profitable (default=0.8)");

  declareOption(ol, "output_column", &LiftStatsCollector::output_column,
                OptionBase::buildoption,
                "Column number (zero-based) in the cost vector containing the model output");
  
  declareOption(ol, "claim_column", &LiftStatsCollector::claim_column,
                OptionBase::buildoption,
                "Column number (zero-based) in the cost vector containing the claim amount during the contract");
  
  declareOption(ol, "premium_column", &LiftStatsCollector::premium_column,
                OptionBase::buildoption,
                "Column number (zero-based) in the cost vector containing the unflexed contract premium");
  
  declareOption(ol, "duration_column", &LiftStatsCollector::duration_column,
                OptionBase::buildoption,
                "Column number (zero-based) in the cost vector containing the contract duration");

  declareOption(ol, "ceede_forbidden_column",
                &LiftStatsCollector::ceede_forbidden_column,
                OptionBase::buildoption,
                "Column number of a boolean flag which, if 1, indicates that "
                "the policy cannot be ceeded to PRR regardless of risk level");
  
  declareOption(ol, "duration_normalized_claim", &LiftStatsCollector::duration_normalized_claim,
                OptionBase::buildoption,
                "True (=1) if the claim has been normalized by the duration");

  declareOption(ol, "claim_multiplier", &LiftStatsCollector::claim_multiplier,
                OptionBase::buildoption,
                "e.g. =1000 if the claim is in thousands of dollars (default=1.0)");

  declareOption(ol, "min_ceede", &LiftStatsCollector::min_ceede,
                OptionBase::buildoption,
                "Minimum fraction of premium volume to ceede to pool");

  declareOption(ol, "max_ceede", &LiftStatsCollector::max_ceede,
                OptionBase::buildoption,
                "Maximum fraction of premium volume to ceede to pool");

  declareOption(ol, "score_filename", &LiftStatsCollector::score_filename,
                OptionBase::buildoption,
                "Name of file in which to save sorted scores, if desired");
*/ // TODO see if useful
  
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void LiftStatsCollector::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void LiftStatsCollector::build_()
{
/*  // We do not use resize on purpose, so
  // that the previous result Vec does not get overwritten
  result = Vec(2);
  const int initial_length = 1000;
  output_and_pos.resize(initial_length, 2);  // 1 output + 1 pos
  targets.resize(initial_length);
  nsamples = 0; */ // TODO adapt to new framework
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LiftStatsCollector::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

//  deepCopyField(all_updates, copies);   // TODO see if we use those fields
//  deepCopyField(sorted_updates, copies);
}

////////////
// forget //
////////////
void LiftStatsCollector::forget()
{
/*  is_finalized = false;
  all_updates.resize(0, all_updates.width()); */ // TODO useful ?
  inherited::forget();
}

////////////
// update //
////////////
void LiftStatsCollector::update(const Vec& x, real w)
{
/*  if (all_updates.length() == 0 && all_updates.width() != x.length())
    all_updates.resize(0, x.length());
  else if (all_updates.width() != x.length())
    PLERROR("LiftStatsCollector::update: trying to update with a row of "
            "inconsistent size");
  is_finalized = false;
  all_updates.push_back(x); */ // TODO see what we do
  inherited::update(x,w);
}

//////////////
// finalize //
//////////////
void LiftStatsCollector::finalize()
{
/*  if (!is_finalized) {
    // Create a new sorted_updates matrix
    const int m = all_updates.width() + 1;
    sorted_updates.resize(all_updates.length(), m);

    const int n = all_updates.length();
    total_premium = total_claim = total_duration = 0.0;
    for (int i=0; i < n; ++i) {
      Vec subrow = sorted_updates(i).subVec(0, m-1);
      subrow << all_updates(i);

      // If we cannot ceede the policy, use -FLT_MAX as the selection criterion
      if (subrow[ceede_forbidden_column] || subrow[premium_column] < 0)
        sorted_updates(i,m-1) = -FLT_MAX;
      else
        sorted_updates(i,m-1) = selectionCriterion(i);

      // Accumulate some statistics
      total_premium += subrow[premium_column];
      total_claim   += subrow[claim_column];
      total_duration+= subrow[duration_column];
    }

    // Finally perform the sorting according to the selectionCriterion;
    // sort in DECREASING order: worst risks 1st
    sortRows(sorted_updates, m-1, false);
    is_finalized = true;

    // Save scores if requested
    if (score_filename != "") {
      VMat mat = new MemoryVMatrix(sorted_updates);
      mat->declareField(output_column,"model-output");
      mat->declareField(claim_column,"claim");
      mat->declareField(premium_column,"unflexed-premium");
      mat->declareField(duration_column,"duration");
      mat->declareField(ceede_forbidden_column,"do-not-ceede");
      mat->declareField(m-1,"score");
      mat->savePMAT(score_filename);
    }
  } */ // TODO see what we do here
  inherited::finalize();
}

/////////////
// getStat //
/////////////
double LiftStatsCollector::getStat(const string& statspec)
{
  PIStringStream str(statspec);
  string parsed;
  str.smartReadUntilNext("(",parsed);
/*  if (parsed == "THRESH_PERCENT") {
    str.smartReadUntilNext(")", parsed);
    return thresholdFromFraction(todouble(parsed) / 100);
  }
  else if (parsed == "PROFIT_THRESH") {
    str.smartReadUntilNext(")", parsed);
    return profitFromThreshold(todouble(parsed));
  }
  else if (parsed == "PROFIT_PERCENT") {
    str.smartReadUntilNext(")", parsed);
    double thresh = thresholdFromFraction(todouble(parsed) / 100);
    return profitFromThreshold(thresh);
  }
  else if (parsed == "OPTIMAL_PERCENT") {
    double mp, ot, of;
    optimizeWorld(mp, ot, of);
    return of*100;
  }
  else if (parsed == "OPTIMAL_THRESH") {
    double mp, ot, of;
    optimizeWorld(mp, ot, of);
    return ot;
  }
  else if (parsed == "OPTIMAL_PROFIT") {
    double mp, ot, of;
    optimizeWorld(mp, ot, of);
    return mp;
  }
  else */ // TODO see what we do here
    return inherited::getStat(statspec);
}

///////////////////
// optimizeWorld //
///////////////////
/*void LiftStatsCollector::optimizeWorld(double& max_profit,
                                      double& opt_threshold,
                                      double& opt_fraction)
{
  if (! is_finalized)
    finalize();
  const int n = sorted_updates.length();
  const int m = sorted_updates.width();
  double cum_premium = 0.0;
  double cum_profit = 0.0;
  max_profit = -FLT_MAX;
  opt_threshold = opt_fraction = 0.0;
  
  for (int i=0 ; i<n &&
         cum_premium/total_premium <= max_ceede ; ++i) {
    cum_profit  += prrSingleProfit(i);
    cum_premium += sorted_updates(i,premium_column);

    // As long as we have not reached at least min_ceede, do
    // not record new maxima reached
    double cur_fraction = cum_premium / total_premium;
    if (cur_fraction >= min_ceede && cum_profit > max_profit) {
      max_profit    = cum_profit;
      opt_fraction  = cur_fraction;
      opt_threshold = sorted_updates(i,m-1);
    }
  }  // TODO wtf is that ?
}
/////////////////////////
// profitFromThreshold //
/////////////////////////
Vec LiftStatsCollector::profitFromThreshold(const Vec& thresholds)
{
  if (! is_finalized)
    finalize();
  const int n = sorted_updates.length();
  const int m = sorted_updates.width(); 
  const int max_thresh = thresholds.length();
  int cur_thresh = 0;
  double cum_profit = 0.0;
  Vec results(max_thresh);

  for (int i=0; i < n && cur_thresh < max_thresh; ++i) {
    // As soon as we change bracket, record the cumulative profit until we
    // were ABOVE OR EQUAL to the current threshold
    for( ; cur_thresh < max_thresh &&
           sorted_updates(i,m-1) < thresholds[cur_thresh] ; ++cur_thresh)
      results[cur_thresh] = cum_profit;

    // Accumulate profit for next record
    cum_profit += prrSingleProfit(i);
  }  // TODO wtf
  return results;
}

///////////////////////////
// thresholdFromFraction //
///////////////////////////
Mat LiftStatsCollector::thresholdFromFraction(const Vec& fractions)
{
//  if (! is_finalized)
//    finalize();
  Mat result(2, fractions.length());
  double cum_premium = 0.0;
  const int n = sorted_updates.length();
  const int m = sorted_updates.width();
  const int max_fraction = fractions.length();
  int idx_fraction = 0;

  // Simultaneously go through records and percents vector
  for (int i=0; i<n && idx_fraction<max_fraction; ++i) {
    cum_premium += sorted_updates(i,premium_column);
    double cur_fraction = cum_premium / total_premium;
    for ( ; idx_fraction < max_fraction &&
            cur_fraction > fractions[idx_fraction] ; ++idx_fraction) {
      result(0, idx_fraction) = sorted_updates(i, m-1);
      result(1, idx_fraction) = cur_fraction;
    }
  }  // TODO wtf
  return result;
}

/////////////////////
// prrSingleProfit //
/////////////////////
double LiftStatsCollector::prrSingleProfit(int row_number) const
{
  if (! is_finalized)
    PLERROR("LiftStatsCollector::prrSingleProfit: internal error, "
            "finalize() not called");
  
  // This is the profit GIVEN that policy row_number is ceeded to pool
  const double premium = sorted_updates(row_number, premium_column);
  const double claim = actualClaim(row_number);

  return claim - prr_loss_ratio * premium;  // TODO wtf
  return 0;
}

/////////////////
// actualClaim //
/////////////////
double LiftStatsCollector::actualClaim(int row_number) const
{
  if (! is_finalized)
    PLERROR("LiftStatsCollector::actualClaim: internal error, "
            "finalize() not called");

  double claim = sorted_updates(row_number, claim_column);
  if (duration_normalized_claim) {
    if (duration_column < 0)
      PLERROR("LiftStatsCollector::actualClaim: Claims are normalized by "
              "duration but duration column not specified");
    claim *= sorted_updates(row_number, duration_column);
  }
  return claim * claim_multiplier;  // TODO wtf
  return 0;
}

////////////////////////
// selectionCriterion //
////////////////////////
double LiftStatsCollector::selectionCriterion(real model_output,
                                             int output_type,
                                             real nonflexed_premium,
                                             real duration,
                                             real prr_loss_ratio,
                                             int sort_type)
{

  switch(sort_type)
  {
  case SortNatural:
  case SortLossRatio:
    switch(output_type)
    {
    case PurePremium:
      return model_output * duration / nonflexed_premium;

    case RelPRRProfit:
    case LossRatio:
    case LossRatioDiv1000:
    case ProbabilityCeede:
      return model_output;

    default:
      PLERROR("LiftStatsCollector::selectionCriterion: Unknown target type %d",
              output_type);
    }

  case SortPRRProfit:
    switch(output_type)
    {
    case PurePremium:
    case RelPRRProfit:
    case LossRatio:
    case LossRatioDiv1000:
      return purePremium(model_output, output_type, nonflexed_premium,
                         duration, prr_loss_ratio)
        - prr_loss_ratio * nonflexed_premium;
      
    case ProbabilityCeede:
      return model_output;
      
    default:
      PLERROR("LiftStatsCollector::selectionCriterion: Unknown target type %d",
              output_type);
    }
    
  default:
    PLERROR("LiftStatsCollector::selectionCriterion: Unknown sort type %d",
            sort_type);
  } // TODO wtf
  return 0;
}

/////////////////
// purePremium //
/////////////////
double LiftStatsCollector::purePremium(real model_output, int output_type,
                                      real premium, real duration,
                                      real prr_loss_ratio)
{
  switch(output_type) {
  case PurePremium:
    // Here, the pure premium is for one year: bring it back to true
    // premium
    return model_output / duration;

  case RelPRRProfit:
    // We have model_output = (claim - PRR_LR*premium)/premium.
    // The pure premium is premium*model_output + PRR_LR*premium
    return premium*model_output + prr_loss_ratio*premium;

  case LossRatio:
    // We have model_output = claim/premium
    return model_output * premium;

  case LossRatioDiv1000:
    // We have model_output = (claim/premium)/1000
    return 1000 * model_output * premium;

  case ProbabilityCeede:
    return MISSING_VALUE;                //!< no way to derive pure premium
                                         //!< from a probability assessment
    
  default:
    PLERROR("LiftStatsCollector::purePremium: unknown output_type %d\n",
            output_type);
  }  // TODO wtf
  return 0;
}
*/ // TODO make sure we deleted all the useless stuff
%> // end of namespace PLearn
