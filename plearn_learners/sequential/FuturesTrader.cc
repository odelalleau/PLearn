
// -*- C++ -*-

// FuturesTrader.cc
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
   * $Id: FuturesTrader.cc,v 1.23 2003/10/27 05:14:34 dorionc Exp $ 
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
  rollover_tag("rollover"), 
  leverage(1.0), maintenance_margin_ratio(0.75)
{}

void FuturesTrader::build()
{
  inherited::build();
  build_();
}

void FuturesTrader::build_()
{
  if(train_set.isNull())
    return;

  margin_cash.resize(max_seq_len, nb_assets);
  margin_cash.fill(MISSING_VALUE);

  if(maintenance_margin_ratio < 0 || maintenance_margin_ratio > 1)
    PLERROR("The margin_call_ratio was set to %f but must be between 0 and 1", 
            maintenance_margin_ratio);
  
  if(assets_names.isEmpty())
    return;
  assets_info(assets_rollover_indices, rollover_tag);
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
    
    if (w_kt != 0)
    {
      real p_kt = price(k,last_call_train_t);
      // Must not use the margin accessor here!!!
      margin(k, last_call_train_t) = fabs(w_kt)*p_kt/leverage;
    }
    else
      margin(k, last_call_train_t) = 0.0;
  }
}
  
void FuturesTrader::trader_test(int t, VMat testset, PP<VecStatsCollector> test_stats,
    VMat testoutputs, VMat testcosts) const
{
  real transaction_cost = 0;
  portfolio_value[t] = 0;
  real previous_value_t = 0;
  real absolute_return_t = 0;
  real relative_sum = 0;

  real daily_risk_free_return = exp(log(1.0+risk_free(t-horizon))/252.0) - 1.0;
  if (is_missing(daily_risk_free_return))
  {
    PLWARNING("daily_risk_free_return is MISSING VALUE");
    daily_risk_free_return = 0;
  }
  for(int k=0; k < nb_assets; k++)
  { 
    real w_kt = weight(k, t);

    // Marking the market: initializing the current margin to the ancient one
    // PLUS the risk free rate return
    margin(k, t) = margin(k, t-1) * (1.0 + daily_risk_free_return);

    // Relative return computation
    if (w_kt != 0.0)
    {
      // In regards of the relative return...
      real p_price = price(k, t-horizon);
      real p_value = w_kt * p_price;
      previous_value_t += abs(p_value);
      relative_sum += p_value * (relative_return(k, t)-1.0);

      // The abolute return on asset k at time t
      real absolute_return_on_asset_k_at_t = w_kt * absolute_return(k, t);

      /* Marking the market: 
          Instead of waiting until the maturity for trader to realize all gains and losses, 
          the clearinghouse requires all positions to recognize profits as they accrue daily.
          (Investments, p.725)
      */
      margin(k, t) += absolute_return_on_asset_k_at_t;
      
      // Update of the portfolio return, adding the k^th assets return
      absolute_return_t += absolute_return_on_asset_k_at_t;

      // No additive_cost on a null delta since there will be no transaction
      real delta_ = delta(k, t);
      real p_kt = price(k, t);
      portfolio_value[t] += abs(w_kt)*p_kt;
      if( delta_ != 0.0 ) //To change for 0 when will pass to units!!!
        transaction_cost += additive_cost + multiplicative_cost[k]*delta_;

      // Checkout if a margin call is needed
      //check_margin(k, t); PAS D'APPELS DE MARGE POUR L'INSTANT!!!
    }
  }

  transaction_costs[t] = transaction_cost;
  absolute_return_t += previous_value_t*daily_risk_free_return - transaction_cost;
  real relative_return_t = daily_risk_free_return + (relative_sum-transaction_cost)/previous_value_t;

  real monthly_turnover = sum(transaction_costs,true)/(sum(portfolio_value,true)/12.0);
  Vec update = advisor->errors(t).copy();
  real log_return = log(1.0+relative_return_t);
  real tbill_log_return = log(1.0+daily_risk_free_return);
  update.append(absolute_return_t);
  update.append(log_return);
  update.append(monthly_turnover);
  update.append(tbill_log_return);
  update.append(log_return - tbill_log_return);
  if(sp500 != "")
  {
    real sp500_log_return = log(sp500_price(t)/sp500_price(t-horizon));
    update.append(sp500_log_return);
    update.append(log_return - sp500_log_return);
  }
  test_stats->update(update);

  if (testoutputs) testoutputs->appendRow(portfolios(t));
  predictions(t) << portfolios(t);
  if (testcosts) testcosts->appendRow(update);
  errors(t) << update;
}

real FuturesTrader::delta(int k, int t) const
{
  bool rollover = (bool)internal_data_set(t,assets_rollover_indices[k]);
  if(!rollover)
    return inherited::delta(k, t);
  
  // In the case of rollover:
  real abs_curr = abs(weight(k, t));
  if( stop_loss(k, t) )
    return abs_curr;
  return abs_curr + abs(weight(k, t+1));  
}

void FuturesTrader::check_margin(int k, int t) const
{
  real v_kt = fabs(weight(k,t)) * price(k, t);
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
  declareOption(ol, "rollover_tag", &FuturesTrader::rollover_tag,
                OptionBase::buildoption,
                "The string such that asset_name:rollover_tag is the field name\n"
                "of the column containing the rollover information.\n"
                "Default: \"rollover\"");

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

/*!
  The advisor cost names + other returns
*/
TVec<string> FuturesTrader::getTrainCostNames() const
{
  TVec<string> cost_names = advisor->getTrainCostNames();
  cost_names.append("absolute_return");
  cost_names.append("log_return");
  cost_names.append("turnover");
  cost_names.append("tbill_log_return"); // model(log_Rt) - TBILL(log_Rt)
  cost_names.append("relative_tbill_log_return"); // model(log_Rt) - TBILL(log_Rt)
  if (sp500.size() > 0)
  {
    cost_names.append("sp500_log_return");
    cost_names.append("relative_sp500_log_return"); // model(log_Rt) - SP500(log_Rt)
  }

  return cost_names;
}

TVec<string> FuturesTrader::getTestCostNames() const
{ return FuturesTrader::getTrainCostNames(); }

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
  
  deepCopyField(assets_rollover_indices, copies);
  deepCopyField(margin_cash, copies);
} 
%> // end of namespace PLearn

