
// -*- C++ -*-

// FuturesTrader.h
//
// Copyright (C) 2003  Christian Dorion 
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
   * $Id: FuturesTrader.h,v 1.5 2003/09/22 20:15:25 dorionc Exp $ 
   ******************************************************* */

/*! \file FuturesTrader.h */
#ifndef FuturesTrader_INC
#define FuturesTrader_INC

#include "SequentialLearner.h"

namespace PLearn <%
using namespace std;

class FuturesTrader: public SequentialLearner
{
protected:
  // *********************
  // * protected options *
  // *********************
  bool build_complete;
  
  int nb_assets;               //!< Simply assets_names.length()
  
  //! List of indices associated with the price fields in the VMat
  TVec<int> assets_price_indices;
  
  //! List of indices associated with the tradable flag fields in the VMat
  TVec<int> assets_tradable_indices;

  //! The index of the SP500 column in the vmat (if sp500 != ""; see below)
  int sp500_index;
  mutable VecStatsCollector log_returns; //!< If sp500 != ""; see below

  //! Time at which test was called the very first time
  mutable int very_first_test_t;
  
  mutable VMat test_set;               //!< A reference to the testset received in test method
//  mutable int test_length;             //!< Simply test_set.length()-train_set.length()
  
  mutable StatsCollector Rt_stat;
  //?? mutable Mat test_Rkt;
  //Dans le stats collector mutable Vec test_Rt;                 //!< The return on each test period
  mutable Vec stop_loss_values;        //!< Will be used to keep track of the losses/gains

  real& test_weights(int k, int t) const { return advisor->state(t-horizon, k); }

public:
  typedef SequentialLearner inherited;

  //**************
  // Constructor *
  //**************  
  FuturesTrader();
  
  //**************
  // Members     *   
  //**************  

  //! An embedded learner issuing recommendations on what should the portfolio look like
  PP<SequentialLearner> advisor;

  /*!
    To be set as true if the undrlying models has a cash position. If it is the case, the 
     cash position will be considered to be the 1^rst in the portfolio, as reflected in the option's
     name. 

    Default: true.
  */
  bool first_asset_is_cash;
  
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
    The return type field is to be setted according to which formula the user 
     wants the trader to use for returns. The DEFAULT is 1

     1.- Absolute returns on period t:
          r_kt = p_kt - p_k(t-horizon)
          
     2.- Relative returns on period t:
                 p_kt - p_k(t-horizon)
          r_kt = ---------------------
                    p_k(t-horizon)
  */
  //int return_type;

  //! The fix cost of performing a trade. Default: 0
  real additive_cost;

  //! The cost of performing a 1$ value trade. Default: 0
  real multiplicative_cost;

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
  static void assets_info(const VMat& vmat, TVec<string>& names,
      bool the_first_asset_is_cash,  const string& risk_free_rate_,
      TVec<int>& price_indices,      const string& price_tag_,
      TVec<int>& tradable_indices,   const string& tradable_tag_,
      bool deduce_assets_names_);

  //! Returns the price of the given asset at a given *TEST* time. 
  inline real price(int k, int t) const
    { return test_set(t, assets_price_indices[k]); }
  
  //******************
  //! Returns the return on the given asset at a given time. 
  inline real absolute_return(int k, int t) const
    { return price(k, t) - price(k,t-horizon); }
  
  inline real relative_return(int k, int t) const
    { return price(k, t)/price(k,t-horizon); }
  //******************
  
  //! Returns the price of the given asset at a given time. 
  inline bool is_tradable(int k, int t)
    { 
      if(first_asset_is_cash && k==0) 
        return true;
      return train_set(t, assets_tradable_indices[k]); 
    }
  
  /*!  
    Returns |test_weights(k, t) - test_weights(k, t+1)| (if < rebalancing_threshold) 
    or 0, otherwise.

    Also calls stop_loss
  */
  real delta(int k, int t) const;
  
  //! Ensures a basic stop loss
  bool stop_loss(int k, int t) const;

  //! Calls the inherited::setTrainingSet and the advisor one
  virtual void setTrainingSet(VMat training_set, bool call_forget=true);
  
private:
  //! This does the actual building
  void build_();
  
protected:
  //! Declare this class' options
  static void declareOptions(OptionList& ol);
  
public:
  
  //! simply calls inherited::build() then build_()
  virtual void build();
  
  //! *** SUBCLASS WRITING: ***
  virtual void train();
  
  //! *** SUBCLASS WRITING: ***
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
  
  virtual TVec<string> getTrainCostNames() const;
  virtual TVec<string> getTestCostNames() const;
  
  //!  Does the necessary operations to transform a shallow copy (this)
  //!  into a deep copy by deep-copying all the members that need to be.
  PLEARN_DECLARE_OBJECT(FuturesTrader);
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(FuturesTrader);

%> // end of namespace PLearn

#endif
