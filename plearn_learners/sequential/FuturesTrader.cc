
// -*- C++ -*-

// FuturesTrader.cc
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
   * $Id: FuturesTrader.cc,v 1.7 2003/09/24 19:22:43 dorionc Exp $ 
   ******************************************************* */

/*! \file FuturesTrader.cc */
#include "FuturesTrader.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_OBJECT(FuturesTrader, "A meta-sequential learner trading futures", 
                        "The FuturesTrader class is meant to replicate as realisticly as possible\n"
                        "the market trading of futures. The learner relies on an *advisor* sequential\n"
                        "learner for the training part. The advisor can be any model that suggests an ideal\n"
                        "portfolio given the historical data. In the test part, the FuturesTrader ask the\n"
                        "advisor for his recommendation and faces it to the market friction, additive and\n"
                        "multiplicative transaction cost, and implements some classical trading mechanism\n"
                        "such as a basic stop loss.");

FuturesTrader::FuturesTrader():
  very_first_test_t(-1)
{}

void FuturesTrader::build()
{
  inherited::build();
  build_();
}

void FuturesTrader::build_()
{}

void FuturesTrader::declareOptions(OptionList& ol)
{
  inherited::declareOptions(ol);
}


/*!
  The advisior->test method is assumed to compute and set the advisor->state up to its 
   (last_test_t)^th row.
*/
void FuturesTrader::test(VMat testset, PP<VecStatsCollector> test_stats,
                         VMat testoutputs/*=0*/, VMat testcosts/*=0*/) const
{
#ifdef MY_PROFILE
  string method_as_str = ("FuturesTrader::test (test_length=" + tostring(test_length) 
                          + "; N=" + tostring(nb_assets) + ")");
  Profiler::start(method_as_str);
#endif

  if( testset.length()-1 <= last_test_t ||
      testset.length()-1 <= last_call_train_t )
    return;
  
  if(very_first_test_t == -1){
    // Will be used in stop_loss
    very_first_test_t = testset.length()-1;
  }

  
  // Since we reached that point, we know we are going to test. Therefore, there is no arm
  //  in modifying directly last_test_t. If we retrained the model after the last test, there is
  //  no need for doing the computation twice and that why:
  if(last_test_t < last_call_train_t)
    last_test_t = last_call_train_t;
  
  // Keeping references for easy access to internal_data_set
  internal_data_set = testset;
  //------------------------------------------------
  
  
  //  The advisor first start with last_test_t, takes positions 
  //  w_t (where t=last_test_t) according to the information it is
  //  provided. The advisor MUST store the positions in the row t of
  //  its state matrix. Afterwards, the returns on the w_t portfolio are 
  //  computed and managed. 
  //
  //  The clock then moves one time forward and the process is repeated.
  
  for(int t = last_test_t+1; t < testset.length(); t++)
  {      
    // Calling the advisor one row at the time ensures us that the state of 
    //  the advisor is updated properly by the delta and stop_loss methods 
    //  before advisor goes forward
    // In the length position of the subMatRows, we use t+1 because we want the
    //  matrix to contain info at times 0, ..., t; these are on t+1 rows
    advisor->test(testset.subMatRows(0, t+1), test_stats, testoutputs, testcosts);
    if( is_missing(advisor->state(t, 0)) )
      PLERROR("The advisior->test method is assumed to compute and set the advisor->state up to its\n\t"
              "(last_test_t)^th row.");

    // Clearing sums
    real value_t = 0;
    real relative_sum = 0;
    real Rt = 0;
    for(int k=0; k < nb_assets; k++)
    { 
      // Relative return computation
      real v_kt = weights(k, t) * price(k, t);
      value_t += v_kt;
      relative_sum += v_kt * relative_return(k, t);
      
      // Update of the portfolio return, adding the k^th assets return
      Rt += weights(k, t) * absolute_return(k, t);
      
      if(first_asset_is_cash && k==0)
        continue; // No call to delta and no trasaction cost on cash
      
      // No additive_cost on a null delta since there will be no transaction
      real delta_ = delta(k, t);
      if( delta_ > 0 )
        Rt -= additive_cost + multiplicative_cost*delta_; // Not 1+mult since that cash pos should take care of the '1'
    }
#ifdef VERBOSE 
    cout << "R" << t << ": " << Rt << endl; 
#endif

    // Should there be a reinvestment procedure alike:
    // weights(k,t+1) += Rt;                                     ???


    // AM I SURE: For now I assume that the first test portfolio is the last train pf. See delta comments.
    // The following four lines should be resumed to the last one as soon as possible! 
    //if(t==0) // 0==> premiere fois que l'on fait test... (le dernier jour de train)
    //  PLWARNING("First test period HACK"); //Is ok: Rt is missing at t==0
    //else
    Rt_stat.update(Rt);

    if(sp500 != "")
    {
      // The relative return of the portfolio can be computed by relative_sum/value_t 
      Vec update(2);
      update[0] = log(relative_sum/value_t);
      update[1] = log(testset(t,sp500_index)/testset(t-horizon,sp500_index)); 
      log_returns.update(update);
    }
  }
  
  real average = Rt_stat.mean(); 
  real sigmasquare = Rt_stat.variance();
  
  cout << "***** ***** *****" << endl
       << "Test: " << endl
#if defined(VERBOSE) || defined(VERBOSE_TEST)
       << "\t weights:\n" << advisor->state << endl
#endif
       << "\t Average Return:\t" << average << endl
       << "\t Empirical Variance:\t" << sigmasquare << endl
       << "\t Sharpe Ratio:\t\t" << average/sqrt(sigmasquare) << endl;
  if(sp500 != "")
    cout << "\t Correlation with S&P500:\t"  << log_returns.getCorrelation()(0,1) << endl;
  cout <<  "***** ***** *****" << endl;
  
  // Keeping track of the call
  last_test_t = advisor->get_last_test_t();
  if(last_test_t != testset.length()-1)
    PLERROR("The FuturesTrader::advisor didn't set its last_test_t properly");
  
#ifdef MY_PROFILE
  Profiler::end(method_as_str);
#endif
}

bool FuturesTrader::stop_loss(int k, int t) const
{
  if(!stop_loss_active) return false;
  
  stop_loss_values[k] += absolute_return(k, t) * weights(k, t);
  
  if(t-very_first_test_t+1 > stop_loss_horizon)
    stop_loss_values[k] -= absolute_return(k, t-stop_loss_horizon) * weights(k, t-stop_loss_horizon);    
  
  if( stop_loss_values[k]/stop_loss_horizon < stop_loss_threshold )
  {
    if(first_asset_is_cash){
      // Since we didn't follow the recommendation of the advisor (sell or buy), the cash position 
      //  will be affected. Didn't buy: delta_>0; didn't sell: delta_<0.
      weights(0, t+1) += weights(k,t+1) * price(k, t); 
    }
    weights(k, t+1) = 0.0;             // Selling or covering short sales.
    // Une autre question... Idealement il faudrait  avoir une facon de determiner pendant combien
    // de temps on n'echange plus sur un asset suite a un stop_loss..
    stop_loss_values[k] = -INFINITY;      // This way: pi >= t ==> weights(k, pi) = 0 
    return true;
  }           
  
  // If we reach that point, stop_loss was not applyed
  return false;
}

void FuturesTrader::computeOutputAndCosts(const Vec& input,
    const Vec& target, Vec& output, Vec& costs) const
{ PLERROR("The method computeOutputAndCosts is not defined for this FuturesTrader"); }

void FuturesTrader::computeCostsOnly(const Vec& input, const Vec& target,
    Vec& costs) const
{ PLERROR("The method computeCostsOnly is not defined for this FuturesTrader"); }

void FuturesTrader::computeOutput(const Vec& input, Vec& output) const
{ PLERROR("The method computeOutput is not defined for this FuturesTrader"); }

void FuturesTrader::computeCostsFromOutputs(const Vec& input,
    const Vec& output, const Vec& target, Vec& costs) const
{ PLERROR("The method computeCostsFromOutputs is not defined for this FuturesTrader"); } 

%> // end of namespace PLearn

