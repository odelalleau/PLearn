
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
   * $Id: FuturesTrader.h,v 1.9 2003/10/07 15:45:17 dorionc Exp $ 
   ******************************************************* */

/*! \file FuturesTrader.h */
#ifndef FuturesTrader_INC
#define FuturesTrader_INC

#include "Trader.h"

namespace PLearn <%
using namespace std;

class FuturesTrader: public Trader
{
public:
  typedef Trader inherited;
  
protected:
  // *********************
  // * protected options *
  // *********************

  /*!
    margin_cash(t) contains the cash position of each asset.
    It has a length of max_seq_len and is initialize after the first
    init_train_size train steps.
  */
  mutable Mat margin_cash;

  //! Access the margin value on asset k at time t (see the margin_cash matrix) 
  real& margin(int k, int t) const { return margin_cash(t-horizon, k); }


  // *********************
  // * protected members *
  // *********************

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
  virtual void trader_test(int t, VMat testset, 
                           real& absolute_return_t, real& relative_return_t) const;
  
  //! Checkout if a margin call is needed
  void check_margin(int k, int t) const;

public:
  
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

%> // end of namespace PLearn

#endif
