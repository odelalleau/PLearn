// -*- C++ -*-

// BuyAndHoldAdvisor.h
//
// Copyright (C) 2003 Christian Dorion 
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
   * $Id: BuyAndHoldAdvisor.h,v 1.2 2004/01/26 21:07:24 dorionc Exp $ 
   ******************************************************* */

// Authors: Christian Dorion

/*! \file BuyAndHoldAdvisor.h */


#ifndef BuyAndHoldAdvisor_INC
#define BuyAndHoldAdvisor_INC

#include "FinancialAdvisor.h"

namespace PLearn <%
using namespace std;

class BuyAndHoldAdvisor: public FinancialAdvisor
{
public:
  typedef FinancialAdvisor inherited;

protected:
  // *********************
  // * protected options *
  // *********************


  // *********************
  // * protected members *
  // *********************
  mutable bool did_buy;

public:

  // ************************
  // * public build options *
  // ************************

  /*!
    If the underlying FinancialAdvisor is null, the positions will be only ones. 
    If not, the positions will be setted to the first test positions of the 
    underlying advisors. 
  */
  PP<FinancialAdvisor> underlying;

  /*!
    If the BuyAndHoldAdvisor is based on an underlying advisor, this boolean specifies 
    if the BuyAndHoldAdvisor follows the first non-null position (true) or simply the
    first position.
    Default: true.
  */
  bool first_non_null_position;
  
  // *****************************
  // * public method to overload *
  // *****************************

  virtual void train_test_core(VMat dataset, int t) const;
  
  // This function must be overloaded!
  virtual void train();

  // This function must be overloaded!  
  virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
                    VMat testoutputs=0, VMat testcosts=0) const;

  // This function must be overloaded!  
  virtual TVec<string> getTrainCostNames() const;

  // This function must be overloaded!
  virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                     Vec& output, Vec& costs) const;
  
private:
  //! This does the actual building
  void build_();
  
protected:
  //! Declare this class' options
  static void declareOptions(OptionList& ol);

public:  
  //! Constructor
  BuyAndHoldAdvisor();
    
  virtual void computeCostsOnly(const Vec& input, const Vec& target,
                                Vec& costs) const;
  
  virtual void computeOutput(const Vec& input, Vec& output) const;
  
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                       const Vec& target, Vec& costs) const;
  
  //! simply calls inherited::build() then build_()
  virtual void build();

  virtual void forget();
    
  //!  Does the necessary operations to transform a shallow copy (this)
  //!  into a deep copy by deep-copying all the members that need to be.
  PLEARN_DECLARE_OBJECT(BuyAndHoldAdvisor);
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(BuyAndHoldAdvisor);

%> // end of namespace PLearn

#endif
