// -*- C++ -*-

// Trader.h
//
// Copyright (C) 2003 Christian Dorion, Rejean Ducharme
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
 * $Id: Trader.h,v 1.10 2003/10/20 21:08:19 ducharme Exp $ 
 ******************************************************* */

// Authors: Christian Dorion

/*! \file Trader.h */


#ifndef Trader_INC
#define Trader_INC

#include "SequentialLearner.h"
#include "FinancialAdvisor.h"

namespace PLearn <%
using namespace std;

class FinancialAdvisor;

class Trader: public SequentialLearner
{
public:
  typedef SequentialLearner inherited;

private:  
  bool build_complete;
  
  //! Time at which test was called the very first time
  mutable int very_first_test_t;
  
  //! A reference to the largest of train_set/testset  
  mutable VMat internal_data_set;   
  
  //! The index of the risk_free_rate column in the internal_data_set
  int risk_free_rate_index;
  
  //! List of indices associated with the price fields in the VMat
  TVec<int> assets_price_indices;
  
  //! The k element is the last time at which asset k had a non-missing price
  TMat<int> last_valid_price;
  
  //! The index of the SP500 column in the vmat (if sp500 != ""; see below)
  int sp500_index;
  bool compute_covariance_with_sp500;

  //! Same thing s above, but for the S&P500
  TVec<int> last_valid_sp500_price;
  
  //! List of indices associated with the tradable flag fields in the VMat
  TVec<int> assets_tradable_indices;
  
  //! List of indices associated with the rollover flag fields in the VMat
  TVec<int> assets_rollover_indices;
  
  //! Will be used to keep track of the losses/gains
  Vec stop_loss_values;        

protected:
  
  /*! 
    The portfolios matrix is used to store the positions (weights) in each asset. The portfolio 
     matrix will have a length of max_seq_len. Its r^th row will be initialized with the portfolio 
     predicted by the advisor at time r, therefore corresponding to the ideal portfolio at time
     t=r+horizon given the information at time r. 
     
     The ideal portfolios suggested by the advisor will then by faced to market reality of corrected 
      directly in the portfolios matrix.
  */
  Mat portfolios;

  //! The (absolute) value of the portfolio at each time step
  Vec portfolio_value;
  
  //! Simply assets_names.length()
  int nb_assets;               

  /*! 
    Sum of transaction costs payed index by time
    SUBCLASS WRITING: 
      The subclass trader_test method is expected to fill the transaction_costs vector.
   */
  Vec transaction_costs;   

  
  //! An accessor to the risk_free_rate at time t
  real risk_free(int t) const { return internal_data_set(t,risk_free_rate_index); }  

  //! Access the portfolio weight k at time t (see the portfolios matrix comment) 
  real& weight(int k, int t) const { return portfolios(t-horizon, k); }

  //**************
  // Methods     *   
  //**************
  
  /*! 
    SUBCLASS WRITING:
      Trader::test method SHOULD NOT BE OVERLOADED by ANY subclass!!! It does 
        some pre and postprocessing to the body of the test method.
        
      However, it is well understood that a specific trader may have specific
        field to set entering the first test.
  */
  virtual void build_test() const {}

  //***********************
  // Abstract Methods     *   
  //***********************
  
  /*! 
    SUBCLASS WRITING:
      Trader::test method SHOULD NOT BE OVERLOADED by ANY subclass!!! It does 
        some pre and postprocessing to the body of the test method.

      The method to be overloaded is trader_test and it will be called by
        Trader::test for each of test's time step. The method must set the 
        absolute & relative returns on test period t.
  */
  virtual void trader_test(int t, VMat testset, PP<VecStatsCollector> test_stats,
      VMat testoutputs, VMat testcosts) const =0;

public:
  
  //! Constructor
  Trader();

  //**************
  // Members     *   
  //**************  
  
  //! An embedded learner issuing recommendations on what should the portfolio look like
  PP<FinancialAdvisor> advisor;

  //! The risk free rate column name in the VMat. Default: risk_free_rate.
  string risk_free_rate;
//#error Cash: as an hyper param != 0 au depart. Default: 1.0. Attention Cash doit etre signifiactivement > que le additive cost

  /*! 
    The string such that asset_name:price_tag is the field name
     of the column containing the price. 
    Default: "close:level"
  */
  string price_tag;

  /*! 
    The string such that asset_name:tradable_tag is the field name
     of the column containing the tradable or not boolean. 
    Default: "is_tradable"
  */
  string tradable_tag;

  /*! 
    The string such that asset_name:rollover_tag is the field name
     of the column containing the rollover information.
    Default: "rollover"
  */
  string rollover_tag;

  //! The fix cost of performing a trade. Default: 0
  real additive_cost;

  //! The cost (for each asset) of performing a unit trade. Default: 0
  Vec multiplicative_cost;

  //! The minimum amplitude for a transaction to be considered worthy. Default: 0
  // It could be interesting to rebal only after a given period of time...
  real rebalancing_threshold;
  
  // Stop loss relative options
  bool stop_loss_active;                //!< Is the PMV using the stop_loss protection. Default: false
  int  stop_loss_horizon;               //!< The horizon on which the loss/gains are considered
  real stop_loss_threshold;             //!< The value under which the stop loss is triggered

  /*!
    The following string is the SP500 field name. If given, the test will compute the 
     covariance of the log returns model/SP500 
     
    Default: ""  //No computation 
  */
  string sp500;

  /*! 
    The names of the assets contained in the train_set
    as parsed by the assets(...) method. 
    Default: TVec<string>()
  */
  TVec<string> assets_names;

  /*!
    If true, the assets_names will be deduce by parsing by the
    assets_info method.
    Default: false
  */
  bool deduce_assets_names;

  //**************
  // Methods     *  
  //**************
  
  //! This parses the train_set VMat to get the infos on the assets
  void assets_info();
  
  //! Returns the price of the given asset at a given time. 
  real price(int k, int t) const;
  
  //! Returns the price of the S&P500 at a given time. 
  real sp500_price(int t) const;
  
  //******************
  //! Returns the return on the given asset at a given time. 
  inline real absolute_return(int k, int t) const
    { return price(k, t) - price(k,t-horizon); }
  
  inline real relative_return(int k, int t) const
    { return price(k, t)/price(k,t-horizon); }
  //******************
  
  //! Returns the price of the given asset at a given time. 
  inline bool is_tradable(int k, int t)
    { return train_set(t, assets_tradable_indices[k]); }
  
  /*!  
    Returns [weight(k, t+1) - weight(k, t)] (if < rebalancing_threshold) 
    or 0, otherwise.

    Also calls stop_loss
  */
  real delta(int k, int t) const;

  //! Ensures a basic stop loss
  virtual bool stop_loss(int k, int t) const;
  
  //! Calls the inherited::setTrainingSet and the advisor one
  virtual void setTrainingSet(VMat training_set, bool call_forget=true);

  int N(){ return nb_assets; }
  
  //**********************
  // Learner Methods     *   
  //**********************
private:
  //! This does the actual building
  void build_();
  
protected:
  //! Declare this class' options
  static void declareOptions(OptionList& ol);

public:
  //! simply calls inherited::build() then build_()
  virtual void build();
  
  /*! 
    This method should be a generic way to do things within a Trader
      and will be overloaded only by models have special particularities
   */
  virtual void train();
  
  /*! 
    SUBCLASS WRITING:
      This method SHOULD NOT BE OVERLOADED by ANY subclass!!! It does 
        some pre and postprocessing to the body of the test method.
        (see the trader_test method comment above)
  */
  virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
                    VMat testoutputs=0, VMat testcosts=0) const;
  
  virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                     Vec& output, Vec& costs) const;
  
  virtual void computeCostsOnly(const Vec& input, const Vec& target,
                                Vec& costs) const;
  
  virtual void computeOutput(const Vec& input, Vec& output) const;
  
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                       const Vec& target, Vec& costs) const;
  
  virtual void forget();
  
  //!  Does the necessary operations to transform a shallow copy (this)
  //!  into a deep copy by deep-copying all the members that need to be.
  PLEARN_DECLARE_ABSTRACT_OBJECT(Trader);
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(Trader);

%> // end of namespace PLearn

#endif
