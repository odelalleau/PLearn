
// -*- C++ -*-

// MountLucasIndex.h
//
// Copyright (C) 2003  Rejean Ducharme 
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
   * $Id: MountLucasIndex.h,v 1.12 2003/10/16 20:51:37 ducharme Exp $ 
   ******************************************************* */

/*! \file MountLucasIndex.h */
#ifndef MountLucasIndex_INC
#define MountLucasIndex_INC

#include "FinancialAdvisor.h"

namespace PLearn <%
using namespace std;

//! The MountLucasIndex class implement the construction of the MLM Index, as invented
//! by Mount Lucas Management Corp. and presented in their paper "Future Investment
//! Series: The MLM Index (Special Report #3)".  We can find this report on their
//! web page : http://www.mtlucas.com/research.htm

class MountLucasIndex: public FinancialAdvisor
{
  public:
    TVec<string> commodity_price_columns; // the commodity price columns in the input data
    TVec<int> commodity_start_year; // the year we begin to trade the corresponding commodity
    string last_day_of_month_column; // the last_day_of_month column in the input data
    string julian_day_column; // the julian day number column in the input data
    string risk_free_rate_column; // the risk free rate column in the input data
    //string sp500_column;
    real transaction_multiplicative_cost;

  protected:
    TVec<bool> is_long_position; // long or short position (for this month)
    TVec<bool> tradable_commodity; // is this commidity tradable for this year
    Vec twelve_month_moving_average;
    Mat next_to_last_unit_asset_value;
    Vec unit_asset_value;
    Vec index_value; // the monthly MLM Index
    mutable int current_month; // the current month (=0 for the first month of the train_set)
    int nb_commodities; // number of commodities included in the MLM Index
    TVec<int> commodity_price_index; // the index corresponding to commodity_price_columns
    int last_day_of_month_index; // the index corresponding to last_day_of_month_column
    int julian_day_index; // the index corresponding to julian_day_column
    int risk_free_rate_index; // the index corresponding to risk_free_rate_column
    //int sp500_index;
    bool build_complete;

    Vec last_month_last_price;
    Vec last_month_next_to_last_price;
    Vec last_tradable_price;
    Vec next_to_last_tradable_price;
    mutable real last_month_risk_free_rate;

    //Vec tbill_return;
    //mutable real s,s2,sf,sf2,sp,sp2;
    //mutable int ns;
    //mutable real last_sp500, last_month_sp500;
 
  private:
    //! This does the actual building
    void build_();

    //! The core implementation of the train and test methods
    void TrainTestCore(const Vec& input, int t, VMat testoutputs=0, VMat testcosts=0) const;

  protected:
    //! Declare this class' options
    static void declareOptions(OptionList& ol);

  public:

    //! Constructor
    MountLucasIndex();

    //! simply calls inherited::build() then build_()
    virtual void build();

    bool next_position(int pos, const Mat& unit_asset_value_) const;

    //! *** SUBCLASS WRITING: ***
    virtual void train();
 
    virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
        VMat testoutputs=0, VMat testcosts=0) const;

    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
        Vec& output, Vec& costs) const;

    virtual void computeCostsOnly(const Vec& input, const Vec& target,
        Vec& costs) const;

    virtual void computeOutput(const Vec& input, Vec& output) const;

    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
        const Vec& target, Vec& costs) const;

    virtual TVec<string> getTrainCostNames() const;
    virtual TVec<string> getTestCostNames() const;

    virtual void forget();

    //!  Does the necessary operations to transform a shallow copy (this)
    //!  into a deep copy by deep-copying all the members that need to be.
    typedef FinancialAdvisor inherited;
    PLEARN_DECLARE_OBJECT(MountLucasIndex);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(MountLucasIndex);

%> // end of namespace PLearn

#endif

