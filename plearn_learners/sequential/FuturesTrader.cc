
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
   * $Id: FuturesTrader.cc,v 1.13 2003/10/07 19:51:32 dorionc Exp $ 
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
  leverage(1.0), maintenance_margin_ratio(0.75)
{}

void FuturesTrader::build()
{
  inherited::build();
  build_();
}

void FuturesTrader::build_()
{
  margin_cash.resize(max_seq_len, nb_assets);
  margin_cash.fill(MISSING_VALUE);

  if(maintenance_margin_ratio < 0 || maintenance_margin_ratio > 1)
    PLERROR("The margin_call_ratio was set to %f but must be between 0 and 1", 
            maintenance_margin_ratio);
}

void FuturesTrader::forget()
{
  inherited::forget();
  if(margin_cash) 
    margin_cash.fill(MISSING_VALUE); 
}

// Entering the first test
void FuturesTrader::build_test() const
{
  // Initialization of the margin
  for (int k=0; k<nb_assets; k++)
  {
    real w_kt = weight(k, last_call_train_t);
    if( is_missing( w_kt ) )
      PLERROR("w_%d%d is missing:\n"
              "The advisior->train method is assumed to compute and set the advisor->predictions up to the\n\t" 
              "(last_call_train_t)^th row. If (last_train_t < last_call_train_t), the\n\t"
              "advisor should have copied portfolios[last_train_t] to the\n\t"
              "(last_call_train_t - last_train_t) following lines\n\t", k, last_call_train_t);
    
    real p_kt = price(k,last_call_train_t);

    margin(k, last_call_train_t) = w_kt*p_kt/leverage; // Which is 0 if w_kt is 0
  }
}
  
void FuturesTrader::trader_test( int t, VMat testset, 
                                 real& absolute_return_t, real& relative_return_t) const
{
  absolute_return_t = 0;
  transaction_costs[t] = 0;
  
  real previous_value_t = 0;
  real relative_sum = 0;
  for(int k=0; k < nb_assets; k++)
  { 
    real w_kt = weight(k, t);
    real p_kt = price(k, t);

    // Marking the market: initializing the current margin to the ancient one
    margin(k, t) = margin(k, t-1);

    // Relative return computation
    if (w_kt != 0.0)
    {
      // In regards of the relative return...
      real p_price = price(k, t-horizon);
      real p_value = w_kt * p_price;
      previous_value_t += abs(p_value);
      relative_sum += p_value * (relative_return(k, t)-1.0);

      // The abolute return on asset k at time t
      real abolute_return_on_asset_k_at_t = w_kt * absolute_return(k, t);

      /* Marking the market: 
          Instead of waiting until the maturity for trader to realize all gains and losses, 
          the clearinghouse requires all positions to recognize profits as they accrue daily.
          (Investments, p.725)
      */
      margin(k, t) += abolute_return_on_asset_k_at_t;
      
      // Update of the portfolio return, adding the k^th assets return
      absolute_return_t += abolute_return_on_asset_k_at_t;

      // No additive_cost on a null delta since there will be no transaction
      real delta_ = delta(k, t);
      if( delta_ != 0.0 ) //To change for 0 when will pass to units!!!
      {
        real transaction_cost = additive_cost + multiplicative_cost*fabs(delta_);
        transaction_costs[t] += transaction_cost;
          
        absolute_return_t -= transaction_cost;
        relative_sum -= transaction_cost;
        
        // Updating the margin, given the leverage
        margin(k, t) += delta_*p_kt/leverage;
      }
    }
    
    // Checkout if a margin call is needed
    check_margin(k, t);
  }
  
  real risk_free_return = exp(risk_free(t-horizon));
  absolute_return_t += previous_value_t * (risk_free_return-1);
  relative_return_t =  risk_free_return + relative_sum/previous_value_t;
  if (relative_return_t < 0)
    PLWARNING("Rendement negatif -> %g", relative_return_t);
}

void FuturesTrader::check_margin(int k, int t) const
{
  real v_kt = weight(k,t) * price(k, t);
  real original_margin = v_kt/leverage;
  real maintenance_margin = original_margin * maintenance_margin_ratio;
  
  if(margin(k, t) > maintenance_margin)
    return;
  
  //*********************************
  // Margin Call:
  real cash_from_nowhere_land = 1e12;
  PLWARNING("Margin call on asset %d at time %d: currently gets the money from\n"
            "nowhere land and doesn't keep track of anything", k, t);
  cout << "original_margin: " << original_margin << endl
       << "maintenance_margin: " << maintenance_margin << endl
       << "margin(k, t): " << margin(k, t) << endl;
  
  // Gets cash from 
  cash_from_nowhere_land -= (original_margin - margin(k, t));  
  
  // And puts it in the margin
  margin(k, t) = original_margin;  
}

void FuturesTrader::declareOptions(OptionList& ol)
{
  declareOption(ol, "leverage", &FuturesTrader::leverage,
                OptionBase::buildoption,
                "The ratio between the value of the contracts we are in and the margin we have.\n"
                "Default: 1");

  declareOption(ol, "maintenance_margin_ratio", &FuturesTrader::maintenance_margin_ratio,
                OptionBase::buildoption,
                "The ratio of the initial margin at which the clearinghouse will proceed to a margin call.\n"
                "Between 0 and 1. Default: 0.75");
  
  inherited::declareOptions(ol);
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

void FuturesTrader::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  
  deepCopyField(margin_cash, copies);
} 
%> // end of namespace PLearn

