
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
   * $Id: FuturesTrader.h,v 1.1 2003/08/29 15:40:39 dorionc Exp $ 
   ******************************************************* */

/*! \file FuturesTrader.h */
#ifndef FuturesTrader_INC
#define FuturesTrader_INC

#include "SequentialLearner.h"

namespace PLearn <%
using namespace std;

class FuturesTrader: public SequentialLearner
{
  //START OF THINGS I ADDED *************************
protected:
  /*!
    The only purpose of this Map is that, conventionaly, financial papers refer to w_kt 
    but here, with respect to the architecture of the Mat, w_tk is more efficient.
    
    The map also allows an abstraction of the horizon
  */
  class Map
  {
  private:
    int h; //!< The horizon
    Mat mat;
  public:
    Map(): h(0) {}
    Map(const Map& m){ mat = m.mat; h = m.h; }
    Map(int N, int T, int h_=0):h(h_),mat(T-h, N){}
    fill(real r){ mat.fill(0.0); }
    real& operator()(int k, int t){ return mat(t-h, k); }
  };
  friend ostream& operator<<(ostream& out, const PMVMap& m){ return (out << m.mat); }

  // *********************
  // * protected options *
  // *********************

  /*! 
    The names of the assets contained in the train_set
     as parsed by the assets method
  */
  TVec<string> assets_names;
  int nb_assets;               //!< Simply assets_names.length()
  
  /*! 
    Could be usefull to store prices in a map instead of accessing them with 
    the field name multiple times
  */
  Map prices_map;
  VMat test_set;
  int test_length;             //!< Simply test_set.length()
  Map test_weights;            //!< The portfolio weights for the test
  Vec test_Rt;                 //!< The return on each test period
  Vec stop_loss_values;        //!< Will be used to keep track of the losses/gains

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
  
  //! The cost of performing a 1$ value trade
  real multiplicative_cost;

  //! The minimum amplitude for a transaction to be considered worthy
  real rebalancing_threshold;
  
  // Stop loss relative options
  bool stop_loss_active;                //!< Is the PMV using the stop_loss protection
  int  stop_loss_horizon;               //!< The horizon on which the loss/gains are considered
  real stop_loss_threshold;             //!< The value under which the stop loss is triggered


  //**************
  // Methods     *   
  //**************

  //! This parses the train_set VMat to get the assets_names
  static void assets();
  
  //! Returns the price of the given asset at a given time. 
  inline real price(int k, int t)
    { return train_set(t, train_set->fieldIndex(asset_names[k]+":"+price_tag)); }
  
  //! Returns the price of the given asset at a given *TEST* time. 
  inline real test_price(int k, int t)
    { return test_set(t, test_set->fieldIndex(assets_names[k]+":close:level")); }

  //******************
  // These version of return are to be modified
  //! Returns the return on the given asset at a given time. 
  inline real ret(int k, int t)
    { return (price(k, t) - price(k, t-horizon)); }
  
  inline real test_return(int k, int t)
    { return (test_prices(k, t) - (t==0)?price(k, T-1):test_price(k,t)); }
  //******************
  
  //! Returns the price of the given asset at a given time. 
  inline bool is_tradable(int k, int t)
    { return train_set(t, train_set->fieldIndex(assets_names[k]+":"+tradable_tag)); }

  //! Called at the begining of test
  void build_test(const VMat& testset);
  
  //! Ensures a basic stop loss
  bool stop_loss(int k, int t);
  
  /*!  
    Returns |test_weights(k, t) - test_weights(k, t-1)| (if < rebalancing_threshold) 
    or 0, otherwise.
  */
  real delta(int k, int t);

  //END OF THINGS I ADDED *************************
  
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
