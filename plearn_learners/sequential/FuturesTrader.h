
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
   * $Id: FuturesTrader.h,v 1.2 2003/09/08 18:44:30 dorionc Exp $ 
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
public:
  /*!
    The only purpose of this Map is that, conventionaly, financial papers refer to w_kt 
    but here, with respect to the architecture of the Mat, w_tk is more efficient.
    
    The map also allows an abstraction of the horizon
  */
  // class Map
//   {
//     friend ostream& operator<<(ostream& out, const Map& m);
//   private:
//     int h; //!< The horizon
//     Mat mat;
//   public:
//     Map(): h(0) {}
//     Map(const Map& m){ mat = m.mat; h = m.h; }
//     Map(int T, int h_): h(h_),mat(1,T-h){}            //!< One row map (instead of template bla, bla...
//     Map(int N, int T, int h_):h(h_),mat(T-h, N){}
//     void fill(real r){ mat.fill(0.0); }
//     real& operator()(int k, int t){ return mat(t-h, k); }
//     real& operator[](int t){ return mat(0,t-h); }
//   };
//   friend ostream& operator<<(ostream& out, const Map& m){ return (out << m.mat); }

protected:
  // *********************
  // * protected options *
  // *********************

  /*! 
    The names of the assets contained in the train_set
     as parsed by the assets method
  */
  TVec<string> assets_names;
  int nb_assets;               //!< Simply assets_names.length()

  //! List of indices associated with the price fields in the VMat
  TVec<int> assets_price_indices;

  //! List of indices associated with the tradable flag fields in the VMat
  TVec<int> assets_tradable_indices;

  mutable VMat test_set;               //!< A reference to the testset received in test method
  mutable int test_length;             //!< Simply test_set.length()-train_set.length()

  //mutable Mat test_weights_;          //!< The portfolio weights for the test
  real& test_weights(int k, int t) const { return /*test_weights_*/advisor->state(t, k); }
  
  //mutable Vec test_Rt;                 //!< The return on each test period
  mutable Vec stop_loss_values;        //!< Will be used to keep track of the losses/gains

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

  //! Each w_0t will be set to (capital_level - \sum_{k>0} w_kt). Default: 1.0
  real capital_level;

  //! The fix cost of performing a trade. Default: 0
  real additive_cost;

  //! The cost of performing a 1$ value trade. Default: 0
  real multiplicative_cost;

  //! The minimum amplitude for a transaction to be considered worthy. Default: 0
  real rebalancing_threshold;
  
  // Stop loss relative options
  bool stop_loss_active;                //!< Is the PMV using the stop_loss protection. Default: false
  int  stop_loss_horizon;               //!< The horizon on which the loss/gains are considered
  real stop_loss_threshold;             //!< The value under which the stop loss is triggered


  //**************
  // Methods     *   
  //**************

  //! This parses the train_set VMat to get the assets_names
  static void assets(const VMat& vmat, TVec<string>& names);
  
  //! Returns the price of the given asset at a given time. 
  inline real price(int k, int t) const
    { return train_set(t, assets_price_indices[k]); }
  
  //! Returns the price of the given asset at a given *TEST* time. 
  inline real test_price(int k, int t) const
    { return test_set(t+train_set.length(), assets_price_indices[k]); }
  
  //******************
  // These version of return are to be modified
  //! Returns the return on the given asset at a given time. 
  // #error *** should be +horizon... BAD definition of ret!!! *** 
  inline real ret(int k, int t) const 
    { return (price(k, t) - price(k, t-horizon)); }
  
  inline real test_return(int k, int t) const
    { 
      return test_price(k, t) - test_price(k,t-1); 
    }
  //******************
  
  //! Returns the price of the given asset at a given time. 
  inline bool is_tradable(int k, int t)
    { return train_set(t, assets_tradable_indices[k]); }
  
  //! Called at the begining of test
  void build_test(const VMat& testset) const;

  /*!  
    Returns |test_weights(k, t) - test_weights(k, t-1)| (if < rebalancing_threshold) 
    or 0, otherwise.

    Also calls stop_loss
  */
  real delta(int k, int t) const;
  
  //! Ensures a basic stop loss
  bool stop_loss(int k, int t) const;

  //! Calls the PLearner::setTrainingSet and the advisor one
  virtual void setTrainingSet(VMat training_set, bool call_forget=true);

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
