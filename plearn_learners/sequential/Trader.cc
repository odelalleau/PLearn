// -*- C++ -*-

// Trader.cc
//
// Copyright (C) 2003 Christian Dorion, Rejean Ducharme
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
 * $Id: Trader.cc,v 1.20 2004/02/16 22:26:09 dorionc Exp $ 
 ******************************************************* */

// Authors: Christian Dorion

/*! \file Trader.cc */


#include "Trader.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_ABSTRACT_OBJECT(Trader, "ONE LINE DESCR", "NO HELP");

Trader::Trader():
  build_complete(false), very_first_test_t(-1), 
  assetwise_log_returns(false),
  last_day_of_month_column(""), last_day_of_month_index(-1), last_day_of_previous_month(-1), 
  nb_assets(0),
  risk_free_rate("risk_free_rate"),
  price_tag("close:level"), tradable_tag("is_tradable"),
  additive_cost(0.0), rebalancing_threshold(0.0), 
  stop_loss_active(false), stop_loss_horizon(-1), stop_loss_threshold(-INFINITY),
  sp500(""), assets_names(TVec<string>()), deduce_assets_names_(false)
{
  multiplicative_cost.resize(0);
}

void Trader::build()
{
  inherited::build();
  build_();
}

void Trader::build_()
{
  max_train_len = advisor->max_train_len;

  if(train_set.isNull())
    return;

  if (max_seq_len == -1)
    PLERROR("The field name 'max_seq_len' has not been set");

  // Gestion of the list of names
  assets_info();

  last_valid_price.resize(max_seq_len, assets_names.length());
  last_valid_price.fill(-1);
  
  if( stop_loss_active ){
    if(stop_loss_horizon < 1 || stop_loss_threshold == -INFINITY)
      PLERROR("In Trader::build_()\n\t"
              "stop_loss_active is true but stop_loss_horizon and/or stop_loss_threshold are not properly setted");  
    else
      stop_loss_values = Vec(nb_assets, 0.0);
  }

  if(sp500 != "")
  {
    sp500_index = train_set->fieldIndex(sp500);
    if(sp500_index == -1)
      PLERROR("The sp500 column name provided (%s) corresponds to no column in the train_set.", sp500.c_str());

    last_valid_sp500_price.resize(max_seq_len);
    last_valid_sp500_price.fill(-1);
  }

  if(last_day_of_month_column != "")
    last_day_of_month_index = train_set->fieldIndex(last_day_of_month_column);

  portfolios.resize(max_seq_len, nb_assets);
  portfolios.fill(MISSING_VALUE);
  portfolio_value.resize(max_seq_len);
  portfolio_value.fill(MISSING_VALUE);

  if (multiplicative_cost.size() == 0)
  {
    multiplicative_cost.resize(nb_assets);
    multiplicative_cost.fill(0.0);  // default value
  }

  transaction_costs.resize(max_seq_len);
  transaction_costs.fill(MISSING_VALUE);
  
  // Sync with the advisor
  advisor->trader = this;
  if (horizon!=advisor->horizon && advisor->horizon!=1)
    PLERROR("Problem with the field name 'horizon'");
  advisor->horizon = horizon;
  advisor->max_seq_len = max_seq_len;
  advisor->outputsize_ = nb_assets;
  advisor->build();
  
  forget();
  
  build_complete = true;
}

void Trader::deduce_assets_names()
{
  if(assets_names.isNotEmpty())
    assets_names.clear();
  
  Array<VMField> finfo = internal_data_set->getFieldInfos();  
  int pos=-1;
  string current_name="";
  for(int info=0; info < finfo.size(); info++)
  {
    pos = finfo[info].name.find_first_of(":");
    if( pos == -1 )
      continue;
    
    current_name = finfo[info].name.substr(0, pos);
    if (assets_names.find(current_name) != -1) // already found
      continue;
    
    assets_names.append(current_name);
  }
}

void Trader::assets_info(TVec<int>& info, const string& info_tag) const
{
  info.resize(assets_names.length());
  for(int asset_index=0; asset_index<assets_names.length(); asset_index++)
  {
    string ainfo = assets_names[asset_index]+":"+info_tag;
    info[asset_index] = internal_data_set->fieldIndex(ainfo);
    if(info[asset_index] == -1)
      PLERROR("The asset info %s was not found.", ainfo.c_str());
  }
}

void Trader::assets_info()
{
  if(internal_data_set.isNull())
    return;

  if(deduce_assets_names_)
    deduce_assets_names();
  else if (assets_names.isEmpty())
    PLERROR("The field name 'assets_names' has not been set and 'deduce_assets_names_' is false");
  
  outputsize_ = nb_assets = assets_names.length();
  if(nb_assets == 0)
    PLERROR("Must provide a non empty TVec<string> assets_names");  

  assets_info(assets_price_indices, price_tag);
  assets_info(assets_tradable_indices, tradable_tag);
  risk_free_rate_index = internal_data_set->fieldIndex(risk_free_rate);    
}

int Trader::price_of_which_asset(int col) const
{
  for(int k=0; k < nb_assets; k++)
    if(assets_price_indices[k] == col) 
      return k;
  return -1;
}


void Trader::forget()
{
  inherited::forget();
  advisor->forget();

  if(portfolios) portfolios.fill(MISSING_VALUE);
  if(portfolio_value) portfolio_value.fill(MISSING_VALUE);

  if(stop_loss_active && internal_data_set.isNotNull())
    stop_loss_values.fill(0.0);
}

void Trader::declareOptions(OptionList& ol)
{
  declareOption(ol, "assetwise_log_returns", &Trader::assetwise_log_returns, OptionBase::buildoption,
                "Enables the assetwise computation of the log returns\n"
                "Default: false");
  
  declareOption(ol, "last_day_of_month_column", &Trader::last_day_of_month_column, OptionBase::buildoption,
                "If not empty, the trader will also compute monthly returns at each last day\n"
                "of month, given that the string indicates the name of the boolean column\n"
                "of the data vmat indicating the if a day is the last one of the month.\n"
                "Default: \"\".");

  declareOption(ol, "advisor", &Trader::advisor,
                OptionBase::buildoption,
                "An embedded learner issuing recommendations on what should the portfolio look like");
  
  declareOption(ol, "risk_free_rate", &Trader::risk_free_rate,
                OptionBase::buildoption,
                "The risk free rate column name in the VMat. Default: risk_free_rate.\n");

  declareOption(ol, "price_tag", &Trader::price_tag,
                OptionBase::buildoption,
                "The string such that asset_name:price_tag is the field name\n" 
                "of the column containing the price.\n"
                "Default: \"close:level\"");
  
  declareOption(ol, "tradable_tag", &Trader::tradable_tag,
                OptionBase::buildoption,
                "The string such that asset_name:tradable_tag is the field name\n"
                "of the column containing the tradable or not boolean.\n"
                "Default: \"is_tradable\"");

  declareOption(ol, "additive_cost", &Trader::additive_cost,
                OptionBase::buildoption,
                "The fix cost of performing a trade.\n"
                "Default: 0");
  
  declareOption(ol, "multiplicative_cost", &Trader::multiplicative_cost,
                OptionBase::buildoption,
                "The cost (for each asset) of performing a unit trade\n"
                "Default: 0");
  
  declareOption(ol, "rebalancing_threshold", &Trader::rebalancing_threshold,
                OptionBase::buildoption,
                "The minimum amplitude for a transaction to be considered worthy\n"
                "Default: 0");
  
  declareOption(ol, "stop_loss_active", &Trader::stop_loss_active,
                OptionBase::buildoption,
                "Is the PMV using the stop_loss protection\n"
                "Default: false");
  
  declareOption(ol, "stop_loss_horizon", &Trader::stop_loss_horizon,
                OptionBase::buildoption,
                "The horizon on which the loss/gains are considered");
  
  declareOption(ol, "stop_loss_threshold", &Trader::stop_loss_threshold,
                OptionBase::buildoption,
                "The value under which the stop loss is triggered");

  declareOption(ol, "sp500", &Trader::sp500,
                OptionBase::buildoption,
                "If the user wants the test method to compute the model log-returns correlation\n"
                "with the S&P500 index, he only needs to provide the sp500 field name in the vmat");
  
  declareOption(ol, "compute_covariance_with_sp500", &Trader::compute_covariance_with_sp500,
                OptionBase::buildoption,
                "Should we compute the covariance between the log-returns and the S&P500.\n"
                "The sp500 flag must be set!");
  
  declareOption(ol, "assets_names", &Trader::assets_names,
                OptionBase::buildoption,
                "The name of all the assets");

  declareOption(ol, "deduce_assets_names", &Trader::deduce_assets_names_,
                OptionBase::buildoption,
                "Whether or not the assets names should be parse by the assets_info method");
  
  inherited::declareOptions(ol);
}

real Trader::price(int k, int t) const
{ 
  real price_ = internal_data_set(t, assets_price_indices[k]); 
  if(is_missing(price_))
  {
    // This test also ensures an end the recursive call that may follow...
    if(t==0)
      PLERROR("price(%d, %d): The model tries to trade on an asset on which we had no valid price yet", k,t);

    // The method price was not called at t-1 for assset k
    // The call to 'price(k, t-1)' ensures that the last_valid_price matrix is properly filled
    if( last_valid_price(t-1, k) == -1 )
      price_ = price(k, t-1); 
    else
      price_ = internal_data_set(last_valid_price(t-1, k), assets_price_indices[k]);  

    // Update the field last_valid_price with the last one for asset k
    last_valid_price(t, k) = last_valid_price(t-1, k);
  }
  else
  {
    // The price is valid and we must update
    last_valid_price(t, k) = t;
  }
  return price_;
}

real Trader::sp500_price(int t) const
{ 
  real price_ = internal_data_set(t, sp500_index);
  if(is_missing(price_))
  {
    // This test also ensures an end the recursive call that may follow...
    if(t==0)
      PLERROR("sp500_price(%d): The model tries to trade on S&P500 for which we had no valid price yet", t);

    // The method price was not called at t-1
    // The call to 'sp500_price(t-1)' ensures that the last_valid_sp500_price vector is properly filled
    if( last_valid_sp500_price[t-1] == -1 )
      price_ = sp500_price(t-1);
    else
      price_ = internal_data_set(last_valid_sp500_price[t-1], sp500_index);

    // Update the field last_valid_sp500_price with the last one
    last_valid_sp500_price[t] = last_valid_sp500_price[t-1];
  }
  else
  {
    // The price is valid and we must update
    last_valid_sp500_price[t] = t;
  }
  return price_;
}

void Trader::setTrainingSet(VMat training_set, bool call_forget/*=true*/)
{
  // This affectation must come before the call to inherited::setTrainingSet 
  //  since build will need internal_data_set to be setted if call_forget is true.
  if(training_set.length()-1 > MAX(last_call_train_t, last_test_t))
    internal_data_set = training_set;

  inherited::setTrainingSet(training_set, call_forget);
  advisor->setTrainingSet(training_set, call_forget);
}

/*!
  The advisior->train method is assumed to compute and set the advisor->predictions up to the 
   (last_call_train_t)^th row. If (last_train_t < last_call_train_t), the 
   advisor should have copy predictions[last_train_t] to the 
   (last_call_train_t - last_train_t) following lines
*/
void Trader::train()
{
  if (!build_complete) build();
  
  // It could be possible but would require a complicated management of the portfolios matrix
  //   which I'll avoid unless really necessary.
  if( train_set.length()-1 < last_test_t-horizon )
    PLERROR("The Trader::train method should not be called at a time t (%d) smaller\n"
            "than last_test_t-horizon (%d-%d)", 
            train_set.length()-1, last_test_t, horizon);

  advisor->train();
  last_call_train_t = advisor->get_last_call_train_t();
  last_train_t = advisor->get_last_train_t();
  if(last_call_train_t != train_set.length()-1)
    PLERROR("The FuturesTrader::advisor did not set its last_call_train_t properly.");
  
  // Update of portfolios
  if(last_test_t == -1)
  {
    portfolios.subMatRows(0, last_call_train_t+1) << advisor->predictions.subMatRows(0, last_call_train_t+1);
  }
/*
  else
  {
    // The portfolios matrix reflects the real positions took on the market by the trader
    //   from time 0 to (last_test_t-horizon): these are no to be changed. 
    // Given the new train information however, the advisor will have change/made predictions
    //   on times (last_test_t-horizon)+1 to last_call_train_t. Since those time steps were
    //   never tested, the portfolios positions will be initialized by the ideal portfolio
    //   suggested by the advisor. 
    // Length: last_call_train_t - ((last_test_t-horizon)+1) + 1, i.e.:
    int copy_length = last_call_train_t - (last_test_t-horizon);
    portfolios.subMatRows((last_test_t-horizon)+1, copy_length) << advisor->predictions.subMatRows((last_test_t-horizon)+1, copy_length);
  }
*/
}

void Trader::test(VMat testset, PP<VecStatsCollector> test_stats,
                  VMat testoutputs, VMat testcosts) const
{
  if( testset.length()-1 <= last_test_t ||
      testset.length()-1 <= last_call_train_t )
    return;
  
  if(very_first_test_t == -1)
  {
    build_test();
    test_stats->compute_covariance = compute_covariance_with_sp500;

    // Since we reached that point, we know we are going to test. Therefore, there is no arm
    //  in modifying directly last_test_t.
    last_test_t = last_call_train_t;
    
    // Will be used in stop_loss
    very_first_test_t = testset.length()-1;
  }
  
  // E.g. the margin_cash field in FuturesTrader would not be available at time 
  //  t = (testset.length()-1) - 1 
  if(last_test_t < last_call_train_t)
    PLERROR("The last_test_t (%d) is smaller than the last_call_train_t (%d): management not supported yet", 
            last_test_t, last_call_train_t);

  // Keeping references for easy access to internal_data_set
  internal_data_set = testset;
  //------------------------------------------------
  
  //  The advisor first starts with last_test_t, takes positions 
  //  w_t (where t=last_test_t) according to the information it is
  //  provided. The advisor MUST store the positions in the row t of
  //  its predictions matrix. Afterwards, the returns on the w_t portfolio are 
  //  computed and managed. 
  //
  //  The clock then moves one time forward and the process is repeated.

  PP<VecStatsCollector> dummy_stats = new VecStatsCollector();
  for(int t = last_test_t+1; t < testset.length(); t++)
  {
    // Calling the advisor one row at the time ensures us that the predictions of 
    //  the advisor is updated properly by the delta and stop_loss methods 
    //  before advisor goes forward
    // In the length position of the subMatRows, we use t+1 because we want the
    //  matrix to contain info at times 0, ..., t; these are on t+1 rows
    advisor->test(testset.subMatRows(0, t+1), dummy_stats);
    if( is_missing(advisor->predictions(t, 0)) )
      PLERROR("The advisor->test method is assumed to compute and set the advisor->predictions up to its\n\t"
              "(last_test_t)^th row.");
    //portfolios.subMatRows(t, 1) << advisor->predictions.subMatRows(t, 1);
    portfolios(t) << advisor->predictions(t);
        
    //**************************************************************
    //*** Body of the test which is specific to the type of Trader
    //real absolute_Rt, relative_Rt;
    //trader_test(t, testset, absolute_Rt, relative_Rt);    
    trader_test(t, testset, test_stats, testoutputs, testcosts);
    //**************************************************************   

    if( is_missing(transaction_costs[t]) )
      PLERROR("SUBCLASS WRITING: the subclass trader_test method is expected to fill the transaction_costs vector");
  }

  // Keeping track of the call
  last_test_t = advisor->get_last_test_t();
  if(last_test_t != testset.length()-1)
    PLERROR("The Trader::advisor didn't set its last_test_t properly");
}


real Trader::delta(int k, int t) const
{
  // Here we consider that the initial test portfolio is the last train portfolio. This assumption seems reasonable since, 
  //  if we sequentially train the model, we will always have, in practice, kept the previous portfolios... The only exception could be at the 
  //  very first time that test is called... For now it is neglected... 
  real current_wkt = weight(k, t);
  real next_wkt = weight(k, t+1);
  
  // We are sure that the weight(k, t+1) is available since the advisor did set it predictions field up to last_test_t, if horizon is at least 1.
  if( stop_loss(k, t) )
    return fabs(current_wkt);

  real delta_ = fabs(next_wkt - current_wkt);
  if(delta_ < rebalancing_threshold){
    weight(k, t+1) = current_wkt;             // Rebalancing isn't needed
    return 0.0;
  }
  return delta_;
}

bool Trader::stop_loss(int k, int t) const
{
  if(!stop_loss_active) return false;
  
  stop_loss_values[k] += absolute_return(k, t) * weight(k, t);
  
  if(t-very_first_test_t+1 > stop_loss_horizon)
    stop_loss_values[k] -= absolute_return(k, t-stop_loss_horizon) * weight(k, t-stop_loss_horizon);    
  
  if( stop_loss_values[k]/stop_loss_horizon < stop_loss_threshold )
  {
    // Une autre question... Idealement il faudrait  avoir une facon de determiner pendant combien
    // de temps on n'echange plus sur un asset suite a un stop_loss..
    stop_loss_values[k] = -INFINITY;      // This way: pi >= t ==> weight(k, pi) = 0 
    return true;
  }           
  
  // If we reach that point, stop_loss was not applyed
  return false;
}

void Trader::computeOutputAndCosts(const Vec& input,
                                   const Vec& target, Vec& output, Vec& costs) const
{ PLERROR("The method computeOutputAndCosts is not defined for this Trader"); }

void Trader::computeCostsOnly(const Vec& input, const Vec& target,
                              Vec& costs) const
{ PLERROR("The method computeCostsOnly is not defined for this Trader"); }

void Trader::computeOutput(const Vec& input, Vec& output) const
{ PLERROR("The method computeOutput is not defined for this Trader"); }

void Trader::computeCostsFromOutputs(const Vec& input,
                                     const Vec& output, const Vec& target, Vec& costs) const
{ PLERROR("The method computeCostsFromOutputs is not defined for this Trader"); }


Vec Trader::updateCosts(const int& t, const real& tbill_log_return, const real& absolute_return, const real& log_return, 
                        const Vec& assetwise_log_return /*=Vec()*/, const real& monthly_log_return /*=MISSING_VALUE*/,
                        const real& sp500_log_return /*=MISSING_VALUE*/) const
{
  Vec update = advisor->errors(t).copy();
  
  update.append(tbill_log_return);
  
  update.append(absolute_return);  
  update.append(log_return);
  
  update.append(log_return - tbill_log_return);  

  if(assetwise_log_returns)
  {
    if(assetwise_log_return.length() != nb_assets)
      PLERROR("%d = assetwise_log_return.length() != nb_assets = %d", assetwise_log_return.length(), nb_assets);
    update.append(assetwise_log_return);
  }
    
  if(last_day_of_month_index != -1)
  {
    if(monthly_log_return == MISSING_VALUE)
      PLWARNING("monthly_log_return is MISSING_VALUE even if user requested computation\n"
                "according to last_day_of_month_column (%s).", last_day_of_month_column.c_str());
    update.append(monthly_log_return);
  }
  
  if(sp500.size() > 0)
  {
    if(sp500_log_return == MISSING_VALUE)
      PLWARNING("sp500_log_return is MISSING_VALUE even if user requested computation\n"
                "according to sp500 (%s).", sp500.c_str());
    update.append(sp500_log_return);
    update.append(log_return - sp500_log_return);
  }

  return update;
}

/*!
  The advisor cost names + other returns
*/
TVec<string> Trader::getTrainCostNames() const
{
  TVec<string> cost_names = advisor->getTrainCostNames();
  
  cost_names.append("tbill_log_return"); // TBILL(log_Rt)
  cost_names.append("absolute_return");
  cost_names.append("log_return");
  cost_names.append("relative_tbill_log_return"); // model(log_Rt) - TBILL(log_Rt)

  if(assetwise_log_returns)
    for(int k=0; k < assets_names.length(); k++)
      cost_names.append( getAssetCost(k) );
  

  if(last_day_of_month_column.size() > 0)
    cost_names.append("monthly_log_return");
  
  if (sp500.size() > 0)
  {
    cost_names.append("sp500_log_return"); // SP500(log_Rt)
    cost_names.append("relative_sp500_log_return"); // model(log_Rt) - SP500(log_Rt)
  }

  // Is the computation reflecting what a user wants to get??? (See FuturesTrader::trader_test)
  //cost_names.append("turnover");
  
  return cost_names;
}

TVec<string> Trader::getTestCostNames() const
{ return Trader::getTrainCostNames(); }

void Trader::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  
  //deepCopyField(internal_stats, copies);
  deepCopyField(assets_price_indices, copies);
  deepCopyField(last_valid_price, copies);
  deepCopyField(last_valid_sp500_price, copies);
  deepCopyField(assets_tradable_indices, copies);
  deepCopyField(stop_loss_values, copies);
  deepCopyField(portfolios, copies);
  deepCopyField(advisor, copies);
  deepCopyField(assets_names, copies);
  deepCopyField(transaction_costs, copies);
} 

%> // end of namespace PLearn

