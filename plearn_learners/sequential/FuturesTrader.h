
// -*- C++ -*-

// FuturesTrader.h
//
// Copyright (C) 2003  Christian Dorion, Rejean Ducharme
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
   * $Id: FuturesTrader.h,v 1.16 2004/02/20 21:14:49 chrish42 Exp $ 
   ******************************************************* */

/*! \file FuturesTrader.h */
#ifndef FuturesTrader_INC
#define FuturesTrader_INC

#include "Trader.h"

namespace PLearn {
using namespace std;

class FuturesTrader: public Trader
{
private:
  mutable Vec monthly_transaction_costs;
  
protected:

  // *********************
  // * protected options *
  // *********************

  //! List of indices associated with the rollover flag fields in the VMat
  TVec<int> assets_rollover_indices;
  
  /*! 
    The string such that asset_name:rollover_tag is the field name
    of the column containing the rollover information.
    Default: "rollover"
  */
  string rollover_tag;

  // *********************
  // * protected members *
  // *********************
    
  /*!
    margin_cash(t) contains the cash position of each asset.
    It has a length of max_seq_len and is initialize after the first
    init_train_size train steps.
  */
  Mat margin_cash;

  //! Access the margin value on asset k at time t (see the margin_cash matrix) 
  real& margin(int k, int t) const { return margin_cash(t-horizon, k); }


  // ***********************
  // * protected functions *
  // ***********************
    
  /*!
    Manages rollover and, as Trader::delta, returns 
    |weight(k, t+1) - weight(k, t)|, if < rebalancing_threshold, or 0 otherwise.
    
    Also calls stop_loss
  */
  virtual real delta(int k, int t) const;

  
  /*! 
    SUBCLASS WRITING:
      Trader::test method SHOULD NOT BE OVERLOADED by ANY subclass!!! It does 
        some pre and postprocessing to the body of the test method.
        
      However, it is well understood that a specific trader may have specific
        field to set entering the first test.
  */
  virtual void build_test() const;
  
  /*! 
    SUBCLASS WRITING:
      Trader::test method SHOULD NOT BE OVERLOADED by ANY subclass!!! It does 
        some pre and postprocessing to the body of the test method.

      The method to be overloaded is trader_test and it will be called by
        Trader::test for each of test's time step. The method must set the 
        absolute & relative returns on test period t.
  */
  virtual void trader_test(int t, VMat testset, PP<VecStatsCollector> test_stats,
      VMat testoutputs, VMat testcosts) const;
//   void assetwise_management(const int& k, const int& t, const real& daily_risk_free_return,
//                             real& previous_value_t, real& absolute_return_t,
//                             real& relative_sum, Vec& assetwise_lret, 
//                             real& transaction_cost, 
//                             int previous_t=-1, bool margin_management=true) const; // subpart of trader_test 
  void assetwise_management(const int& k, const int& t, const real& risk_free_return,
                            Vec& assetwise, real& previous_value, 
                            real& absolute_return, real& transaction_cost, 
                            int previous_t = -1) const;

  /*!
    Subpart of trader_test that computes the time step relative return given...
    The assetwise arg may be empty (no computation to be done) or filled with the assetwise relative sums.
    Comment: Should maybe be changed for
      static real time_step_relative_return(Vec& assetwise, const Vec& transaction_cost, const real& risk_free_return) const;
    returning the relative return from forced assetwise data...
   */
//   void time_step_relative_return(real& relative_return, Vec& assetwise, 
//                                  const real& relative_sum, const real& previous_value, 
//                                  const real& transaction_cost, const real& risk_free_return) const;
  real time_step_relative_return(Vec& assetwise, const real& previous_value, const real& risk_free_return) const;
  
  real last_month_relative_return(const int& t) const;

  real transactionCost(bool monthly_call, const int& k, const int& t) const;
  
  //! Checkout if a margin call is needed
  void check_margin(int k, int t) const;

public:
  typedef Trader inherited;

  //**************
  // Constructor *
  //**************  
  FuturesTrader();

  //**************
  // Members     *   
  //**************
  
  //! The ratio between the value of the contracts we are in and the margin we have. Default: 1
  real leverage;
  
  //! The ratio of the initial margin at which the clearinghouse will proceed to a margin call
  real maintenance_margin_ratio;

  //**************
  // Methods     *   
  //**************
      
private:
  //! This does the actual building
  void build_();
  
protected:
  //! Declare this class' options
  static void declareOptions(OptionList& ol);
  
public:
  
  //! simply calls inherited::build() then build_()
  virtual void build();

  virtual void forget();
  
  virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                     Vec& output, Vec& costs) const;
  
  virtual void computeCostsOnly(const Vec& input, const Vec& target,
                                Vec& costs) const;
  
  virtual void computeOutput(const Vec& input, Vec& output) const;
  
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                       const Vec& target, Vec& costs) const;
    
  //!  Does the necessary operations to transform a shallow copy (this)
  //!  into a deep copy by deep-copying all the members that need to be.
  PLEARN_DECLARE_OBJECT(FuturesTrader);
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(FuturesTrader);

} // end of namespace PLearn

#endif
