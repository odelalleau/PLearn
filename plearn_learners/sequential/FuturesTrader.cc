
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
   * $Id: FuturesTrader.cc,v 1.30 2004/02/19 16:39:42 dorionc Exp $ 
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

  monthly_transaction_costs.resize(nb_assets);
  monthly_transaction_costs.fill(0.0);
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

real FuturesTrader::transactionCost(bool monthly_call, const int& k, const int& t) const
{
  if(monthly_call)
  {
    // On a monthly call we use the precomputed transaction costs 
    // monthly_transaction_cost which kept (see the else part) the assetwise 
    // sum of transaction cost throughout the month 
    // (models with in month trading, rollover, etc.)
    // It also avoid a misleading call to the delta method.
    real tc = monthly_transaction_costs[k];
    monthly_transaction_costs[k] = 0; // reinitializing for next month.
    return tc;
  }

  // Daily Transaction Costs: No additive_cost on a null delta since there will be no transaction
  real delta_ = delta(k, t);  
  if( delta_ == 0.0 ) //To change for 0 when will pass to units!!!
    return 0.0;
  
  real daily_tc = additive_cost + multiplicative_cost[k]*delta_;
  monthly_transaction_costs[k] += daily_tc;
  return daily_tc;
}

void FuturesTrader::assetwise_management(const int& k, const int& t, const real& risk_free_return,
                                         Vec& assetwise, real& previous_value, 
                                         real& absolute_return, real& transaction_cost, 
                                         int previous_t /*-1*/) const
{ 
  bool monthly_call = true;
  if(previous_t == -1) // Then it's a daily call
  {
    previous_t = t-horizon;
    monthly_call = false;
  }
  bool margin_management = !monthly_call; // The margin is managed daily.

  if(margin_management)
  {
    // Marking the market: initializing the current margin to the ancient one
    // PLUS the risk free rate return
    margin(k, t) = margin(k, t-1) * (1.0 + risk_free_return);
  }
  
  real w_kt = weight(k, t);
  // Relative return computation
  if (w_kt == 0.0)
    return;
  
  // Previous and current prices: for returns computation
  real previous_price_k = price(k, previous_t);
  real p_kt = price(k, t);

  // Updates
  real absolute_position = abs(w_kt);
  previous_value += absolute_position * previous_price_k;
  portfolio_value[t] += absolute_position * p_kt;
  
  // The abolute return on asset k at time t. The absolute return computation can NOT 
  // be done by the absolute_return method since previous_t can be passed as an arg!
  real absolute_return_on_asset_k_at_t = w_kt * (p_kt - previous_price_k);
  absolute_return += absolute_return_on_asset_k_at_t;

  if(margin_management)
  {
    /* Marking the market: 
       Instead of waiting until the maturity for trader to realize all gains and losses, 
       the clearinghouse requires all positions to recognize profits as they accrue daily.
       (Investments, p.725)
    */
    margin(k, t) += absolute_return_on_asset_k_at_t;
  }

  real t_cost = transactionCost(monthly_call, k, t);
  transaction_cost += t_cost;
  assetwise[k] = absolute_return_on_asset_k_at_t - t_cost;
  
  if(margin_management)
  {
    // Checkout if a margin call is needed
    //check_margin(k, t); PAS D'APPELS DE MARGE POUR L'INSTANT!!!
  }
}

real FuturesTrader::time_step_relative_return(Vec& assetwise, const real& previous_value, const real& risk_free_return) const
{
  // If only nan values are encountered, we know that 
  //  the portfolio was empty. Therefore, we shall not add 
  //  the risk_free_return to relative_return 
  bool traded = false;
  real relative_return = 0.0;
  
  if(previous_value != 0.0)
  {    
    assetwise /= previous_value;

    real* a = assetwise.data();
    for(int k=0; k < assetwise.length(); k++)
    {
      if (!is_missing(a[k])) 
      {
        // There was at least one asset traded.
        traded = true;
        relative_return += a[k];
        
        // Assetwise returns are added the risk free rate on their margin cash counterpart
        a[k] += risk_free_return;
      }
      else 
        a[k] = 0.0; // Overwritting the missing value with zero return
    }
  }
  else
    assetwise.fill(0.0);
  
  // Adding the return on cash position
  if(traded)
    relative_return += risk_free_return;
  
  return relative_return;
}

real FuturesTrader::last_month_relative_return(const int& t) const
{
#ifdef TCOST_DEBUG
  cout << "FuturesTrader::last_month_relative_return[[[" << endl;
#endif

  if(last_day_of_previous_month == -1)
    for(int last=t-1; last > 0; last--)
      if(is_last_day_of_month(last))
      { last_day_of_previous_month = last; break; }
  
  real last_month_risk_free_return = exp(log(risk_free(last_day_of_previous_month) + 1.0)/12.0) - 1.0;
//    cout << "monthly_risk_free_return: " << monthly_risk_free_return << endl;

  /*!
    The missing value will be overwritten in assetwise_management if and only if 
    the asset was traded. Therefore, time_step_relative_return can make the difference 
    between a zero return traded asset and an asset which was not traded at all.
  */
  Vec assetwise_ret(nb_assets, MISSING_VALUE);

  real last_month_value=0.0; 
  real last_month_absolute_return=0.0;
  real last_month_transaction_cost=0.0;
  
  for(int k=0; k < nb_assets; k++)
  {
//       cout << "k: " << k << "  " << "t: " << t << endl
//            << "monthly_risk_free_return: " << monthly_risk_free_return << endl 
//            << "monthly_previous_value_t : " << monthly_previous_value_t  << endl
//            << "monthly_absolute_return_t : " << monthly_absolute_return_t  << endl
//            << "monthly_relative_sum : " << monthly_relative_sum  << endl
//            << "no_assetwise_ret : " << no_assetwise_ret  << endl
//            << "monthly_transaction_cost : " << monthly_transaction_cost  << endl
//            << "last_day_of_previous_month : " << last_day_of_previous_month  << endl
//            << endl;
    
    assetwise_management(k, t, last_month_risk_free_return, 
                         assetwise_ret, last_month_value, 
                         last_month_absolute_return, last_month_transaction_cost,
                         last_day_of_previous_month);
  }
#ifdef TCOST_DEBUG
  cout << "last_month_transaction_cost: " << last_month_transaction_cost << endl;
  cout << "]]]FuturesTrader::last_month_relative_return" << endl;
#endif

  last_day_of_previous_month = t;
  return time_step_relative_return(assetwise_ret, last_month_value, last_month_risk_free_return);  
}

void FuturesTrader::trader_test(int t, VMat testset, PP<VecStatsCollector> test_stats,
    VMat testoutputs, VMat testcosts) const
{
  real transaction_cost = 0;
  portfolio_value[t] = 0;
  real previous_value_t = 0;
  real absolute_return_t = 0;

  /*!
    The missing value will be overwritten in assetwise_management if and only if 
    the asset was traded. Therefore, time_step_relative_return can make the difference 
    between a zero return traded asset and an asset which was not traded at all.
  */
  Vec assetwise_lret(nb_assets, MISSING_VALUE); 
  
  real daily_risk_free_return = exp(log(1.0+risk_free(t-horizon))/252.0) - 1.0;
  if (is_missing(daily_risk_free_return))
  {
    PLWARNING("daily_risk_free_return is MISSING VALUE");
    daily_risk_free_return = 0;
  }
  for(int k=0; k < nb_assets; k++)
    assetwise_management(k, t, daily_risk_free_return, 
                         assetwise_lret, previous_value_t, 
                         absolute_return_t, transaction_cost);
  
  transaction_costs[t] = transaction_cost;
#ifdef TCOST_DEBUG
  cout << "transaction_costs["<<t<<"]: " << transaction_costs[t] << endl;
#endif

  real relative_return_t = time_step_relative_return(assetwise_lret, previous_value_t, daily_risk_free_return);
  
  // See trainCostNames
  //real monthly_turnover = sum(transaction_costs,true)/(sum(portfolio_value,true)/12.0);
  
  // Computing the errors
  real tbill_log_return = log(1.0+daily_risk_free_return);
  absolute_return_t += previous_value_t*daily_risk_free_return - transaction_cost;
  real log_return = log(1.0+relative_return_t);
  if(assetwise_log_returns)
    assetwise_lret = log(1.0+assetwise_lret);

  real monthly_log_return = MISSING_VALUE;
  if(last_day_of_month_index != -1 && is_last_day_of_month(t))
    monthly_log_return = log(1.0+last_month_relative_return(t));
  
  real sp500_log_return = MISSING_VALUE;
  if(sp500 != "")
    sp500_log_return = log(sp500_price(t)/sp500_price(t-horizon));
  
  Vec update = updateCosts(t, tbill_log_return, absolute_return_t, log_return, assetwise_lret, monthly_log_return, sp500_log_return);
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

