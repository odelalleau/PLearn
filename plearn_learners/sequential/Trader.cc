// -*- C++ -*-

// Trader.cc
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
 * $Id: Trader.cc,v 1.3 2003/09/28 21:40:46 dorionc Exp $ 
 ******************************************************* */

// Authors: Christian Dorion

/*! \file Trader.cc */


#include "Trader.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_ABSTRACT_OBJECT(Trader, "ONE LINE DESCR", "NO HELP");

Trader::Trader():
  build_complete(false),
  very_first_test_t(-1),
  nb_assets(0),
  first_asset_is_cash(true), risk_free_rate("risk_free_rate"),
  price_tag("close:level"), tradable_tag("is_tradable"), //return_type(1),
  additive_cost(0.0), multiplicative_cost(0.0),
  rebalancing_threshold(0.0), 
  stop_loss_active(false), stop_loss_horizon(-1), stop_loss_threshold(-INFINITY),
  sp500(""), assets_names(TVec<string>()), deduce_assets_names(false)
{
  // ...
  
  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

void Trader::build()
{
  inherited::build();
  build_();
}

void Trader::build_()
{
  if(train_set.isNull())
    return;
  
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
    internal_stats.compute_covariance = true;
  }

  if (max_seq_len == -1)
    PLERROR("The field name 'max_seq_len' has not been set");
  else
  {
    portfolios.resize(max_seq_len, nb_assets);
    portfolios.fill(MISSING_VALUE);
  }

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

void Trader::assets_info()
{
  if(internal_data_set.isNull())
    return;
  
  if(assets_names.isNotEmpty())
  {
    if (deduce_assets_names)
      assets_names.clear();
    assets_price_indices.clear();
    assets_tradable_indices.clear();
  }
  
  if(first_asset_is_cash)
    assets_names.append("Cash");

  if(deduce_assets_names)
  {
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
  else if (assets_names.isEmpty())
    PLERROR("The field name 'assets_names' has not been set and 'deduce_assets_names' is false");
  cout << assets_names << endl; //TMP
  
  outputsize_ = nb_assets = assets_names.length();
  if(nb_assets == 0)
    PLERROR("Must provide a non empty TVec<string> assets_names");
  

  // Building a list of the indices needed to access the prices in the VMat
  assets_price_indices.resize(assets_names.length());
  assets_tradable_indices.resize(assets_names.length());
  int asset_index = 0;
  if(first_asset_is_cash){
    assets_price_indices[0] = internal_data_set->fieldIndex(risk_free_rate);    
    assets_tradable_indices[0] = -1;
    asset_index++;
  }
  for(; asset_index < assets_names.length(); asset_index++){
    assets_price_indices[asset_index] = internal_data_set->fieldIndex(assets_names[asset_index]+":"+price_tag);
    assets_tradable_indices[asset_index] = internal_data_set->fieldIndex(assets_names[asset_index]+":"+tradable_tag);
  }
}

void Trader::forget()
{
  inherited::forget();
  advisor->forget();
  
  if(portfolios) 
    portfolios.fill(MISSING_VALUE);

  if(stop_loss_active && !internal_data_set.isNull()){
    stop_loss_values.fill(0.0);
  }
  
  internal_stats.forget();
}

void Trader::declareOptions(OptionList& ol)
{
  declareOption(ol, "advisor", &Trader::advisor,
                OptionBase::buildoption,
                "An embedded learner issuing recommendations on what should the portfolio look like");

  
  declareOption(ol, "first_asset_is_cash", &Trader::first_asset_is_cash,
                OptionBase::buildoption,
                "To be set as true if the undrlying models has a cash position. If it is the case, the \n"
                "cash position will be considered to be the 1^rst in the portfolio, as reflected in the option's\n"
                "name. Default: true.");
  
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
                "The cost of performing a 1$ value trade\n"
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
  
  declareOption(ol, "assets_names", &Trader::assets_names,
                OptionBase::buildoption,
                "The name of all the assets");

  declareOption(ol, "deduce_assets_names", &Trader::deduce_assets_names,
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
  if (!build_complete) build_();
  
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
  if( is_missing( advisor->predictions(last_call_train_t, 0) ) )
    PLERROR("The advisior->train method is assumed to compute and set the advisor->predictions up to the\n\t" 
            "(last_call_train_t)^th row. If (last_train_t < last_call_train_t), the\n\t"
            "advisor should have copied portfolios[last_train_t] to the\n\t"
            "(last_call_train_t - last_train_t) following lines\n\t");
  
  if(last_test_t == -1)
    portfolios.subMatRows(0, last_call_train_t+1) << advisor->predictions.subMatRows(0, last_call_train_t+1);
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
}

void Trader::test(VMat testset, PP<VecStatsCollector> test_stats,
                  VMat testoutputs, VMat testcosts) const
{
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
  
  //  The advisor first starts with last_test_t, takes positions 
  //  w_t (where t=last_test_t) according to the information it is
  //  provided. The advisor MUST store the positions in the row t of
  //  its predictions matrix. Afterwards, the returns on the w_t portfolio are 
  //  computed and managed. 
  //
  //  The clock then moves one time forward and the process is repeated.
  for(int t = last_test_t+1; t < testset.length(); t++)
  {
    // Calling the advisor one row at the time ensures us that the predictions of 
    //  the advisor is updated properly by the delta and stop_loss methods 
    //  before advisor goes forward
    // In the length position of the subMatRows, we use t+1 because we want the
    //  matrix to contain info at times 0, ..., t; these are on t+1 rows
    advisor->test(testset.subMatRows(0, t+1), test_stats, testoutputs, testcosts);
    if( is_missing(advisor->predictions(t, 0)) )
      PLERROR("The advisior->test method is assumed to compute and set the advisor->predictions up to its\n\t"
              "(last_test_t)^th row.");
    portfolios.subMatRows(t, 1) << advisor->predictions.subMatRows(t, 1);
        

    
    //**************************************************************
    //*** Body of the test which is specific to the type of Trader
    real absolute_Rt, relative_Rt;
    trader_test(t, testset, absolute_Rt, relative_Rt);    
    //**************************************************************   

    // The order of the 'append' statements is IMPORTANT! (enum stats_indices) 
    Vec update;
    update.append( absolute_Rt );
    update.append( log(relative_Rt) );
    if(sp500 != "")
      update.append( log(testset(t,sp500_index)/testset(t-horizon,sp500_index)) );
    internal_stats.update(update);
    
//#ifdef VERBOSE 
    cout << "(rt, log_rel_rt, sp)[" << t << "]: " << update << endl;
    
//#endif
  }
  
  real average = internal_stats.stats[rt].mean(); 
  real sigmasquare = internal_stats.stats[rt].variance();
  
  cout << "***** ***** *****" << endl
       << "Test: " << endl
#if defined(VERBOSE) || defined(VERBOSE_TEST)
       << "\t weights:\n" << portfolios << endl
#endif
       << "\t Average Return:\t" << average << endl
       << "\t Empirical Variance:\t" << sigmasquare << endl
       << "\t Sharpe Ratio:\t\t" << average/sqrt(sigmasquare) << endl;
  if(sp500 != "")
    cout << "\t Correlation with S&P500:\t"  << internal_stats.getCorrelation()(log_rel_rt, log_sp) << endl;
  cout <<  "***** ***** *****" << endl;
  
  // Keeping track of the call
  last_test_t = advisor->get_last_test_t();
  if(last_test_t != testset.length()-1)
    PLERROR("The Trader::advisor didn't set its last_test_t properly");
}


//TBAdded: Somewhere after stop_loss, a check_if_roll_over => on vend tout weight(k, t) et 
//           on achete tout weight(k, t+1)!!!
real Trader::delta(int k, int t) const
{
  // Should not be called on the cash
  if(first_asset_is_cash && k==0)
    PLERROR("*** INTERNAL *** call to delta(%d, %d) with first_asset_is_cash set as true", k, t);

  // Here we consider that the initial test portfolio is the last train portfolio. This assumption seems reasonable since, 
  //  if we sequentially train the model, we will always have, in practice, kept the previous portfolios... The only exception could be at the 
  //  very first time that test is called... For now it is neglected... 
  real current_wkt = weight(k, t);
  
  // We are sure that the weight(k, t+1) is available since the advisor did set it predictions field up to last_test_t, if horizon is at least 1.
  if( stop_loss(k, t) )
    return current_wkt;

  real delta_ = weight(k, t+1) - current_wkt;
  if(fabs(delta_) < rebalancing_threshold){
    weight(k, t+1) = current_wkt;             // Rebalancing isn't needed
    if(first_asset_is_cash){
      // Since we didn't follow the recommendation of the advisor (sell or buy), the cash position 
      //  will be affected. Didn't buy (delta_>0) we saved money; didn't sell (delta_<0), did not gain the expected money.
      weight(0, t+1) += delta_ * price(k, t); 
    }
    return 0.0;
  }
  return fabs(delta_);  //else: delta_ >= rebalancing_threshold
}

bool Trader::stop_loss(int k, int t) const
{
  if(!stop_loss_active) return false;
  
  stop_loss_values[k] += absolute_return(k, t) * weight(k, t);
  
  if(t-very_first_test_t+1 > stop_loss_horizon)
    stop_loss_values[k] -= absolute_return(k, t-stop_loss_horizon) * weight(k, t-stop_loss_horizon);    
  
  if( stop_loss_values[k]/stop_loss_horizon < stop_loss_threshold )
  {
    if(first_asset_is_cash){
      // Since we didn't follow the recommendation of the advisor (sell or buy), the cash position 
      //  will be affected. Didn't buy: delta_>0; didn't sell: delta_<0.
      weight(0, t+1) += weight(k,t+1) * price(k, t); 
    }
    weight(k, t+1) = 0.0;             // Selling or covering short sales.
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

void Trader::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  
  // ### Remove this line when you have fully implemented this method.
  PLERROR("Trader::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
} 

/*!
  Since FuturesTrader::train mainly call its advisor train method
   it is only normal that FuturesTrader::getTrainCostNames calls FT's 
   advisor getTrainCostNames method. Make sure it is properly implemented.
*/
TVec<string> Trader::getTrainCostNames() const
{
  return advisor->getTrainCostNames();
}

TVec<string> Trader::getTestCostNames() const
{ return Trader::getTrainCostNames(); }

%> // end of namespace PLearn

