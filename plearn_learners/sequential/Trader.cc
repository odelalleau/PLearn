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
 * $Id: Trader.cc,v 1.1 2003/09/24 19:41:01 dorionc Exp $ 
 ******************************************************* */

// Authors: Christian Dorion

/*! \file Trader.cc */


#include "Trader.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_ABSTRACT_OBJECT(Trader, "ONE LINE DESCR", "NO HELP");

Trader::Trader():
  build_complete(false),
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
  if(assets_names.isNotEmpty())
  {
    //PLWARNING("Multiple call to Trader::build_");
    if (deduce_assets_names)
      assets_names.clear();
    assets_price_indices.clear();
    assets_tradable_indices.clear();
  }
  assets_info(train_set, assets_names,
              first_asset_is_cash, risk_free_rate,
              assets_price_indices, price_tag,
              assets_tradable_indices, tradable_tag,
              deduce_assets_names);
  cout << assets_names << endl; //TMP

  nb_assets = assets_names.length();
  if(nb_assets == 0)
    PLERROR("Must provide a non empty TVec<string> assets_names");
  
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
    log_returns.compute_covariance = true;
  }
  
  // Sync with the advisor
  advisor->trader = this;
  if (horizon!=advisor->horizon && advisor->horizon!=1)
    PLERROR("Problem with the field name 'horizon'");
  advisor->horizon = horizon;
  if (max_seq_len == -1)
    PLERROR("The field name 'max_seq_len' has not been set");
  advisor->max_seq_len = max_seq_len;
  advisor->outputsize_ = nb_assets;
  advisor->build();
  
  forget();
  
  build_complete = true;
}

void Trader::assets_info(const VMat& vmat, TVec<string>& names,
                                bool the_first_asset_is_cash,  const string& risk_free_rate_,
                                TVec<int>& price_indices,      const string& price_tag_,
                                TVec<int>& tradable_indices,   const string& tradable_tag_,
                                bool deduce_assets_names_)
{
  if(vmat.isNull())
    return;
  
  if(the_first_asset_is_cash)
    names.append("Cash");

  if (deduce_assets_names_)
  {
    Array<VMField> finfo = vmat->getFieldInfos();  
    int pos=-1;
    string current_name="";
    for(int info=0; info < finfo.size(); info++)
    {
      pos = finfo[info].name.find_first_of(":");
      if( pos == -1 )
        continue;

      current_name = finfo[info].name.substr(0, pos);
      if (names.find(current_name) != -1) // already found
        continue;

      names.append(current_name);
    }
  }
  else if (names.isEmpty())
    PLERROR("The field name 'assets_names' has not been set and 'deduce_assets_names' is false");

  // Building a list of the indices needed to access the prices in the VMat
  price_indices.resize(names.length());
  tradable_indices.resize(names.length());
  int asset_index = 0;
  if(the_first_asset_is_cash){
    price_indices[0] = vmat->fieldIndex(risk_free_rate_);
    tradable_indices[0] = -1;
    asset_index++;
  }
  for(; asset_index < names.length(); asset_index++){
    price_indices[asset_index] = vmat->fieldIndex(names[asset_index]+":"+price_tag_);
    tradable_indices[asset_index] = vmat->fieldIndex(names[asset_index]+":"+tradable_tag_);
  }
}

void Trader::forget()
{
  inherited::forget();
  advisor->forget();
  
  if(stop_loss_active && !internal_data_set.isNull()){
    stop_loss_values.fill(0.0);
  }
  
  Rt_stat.forget();
  log_returns.forget();
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

void Trader::setTrainingSet(VMat training_set, bool call_forget/*=true*/)
{
  inherited::setTrainingSet(training_set, call_forget);
  advisor->setTrainingSet(training_set, call_forget);
  
  if(training_set.length()-1 > MAX(last_call_train_t, last_test_t))
    internal_data_set = training_set;
}

/*!
  The advisior->train method is assumed to compute and set the advisor->state up to the 
   (last_call_train_t)^th row. If (last_train_t < last_call_train_t), the 
   advisor should have copy state[last_train_t] to the 
   (last_call_train_t - last_train_t) following lines
*/
void Trader::train()
{
  if (!build_complete) build_();
  
  advisor->train();
  last_call_train_t = advisor->get_last_call_train_t();
  last_train_t = advisor->get_last_train_t();
  if(last_call_train_t != train_set.length()-1)
    PLERROR("The FuturesTrader::advisor did not set its last_call_train_t properly.");
  if( is_missing( advisor->state(last_call_train_t, 0) ) )
    PLERROR("The advisior->train method is assumed to compute and set the advisor->state up to the\n\t" 
            "(last_call_train_t)^th row. If (last_train_t < last_call_train_t), the\n\t"
            "advisor should have copied state[last_train_t] to the\n\t"
            "(last_call_train_t - last_train_t) following lines\n\t");
}

real& Trader::weights(int k, int t) const { return advisor->state(t-horizon, k); }

//TBAdded: Somewhere after stop_loss, a check_if_roll_over => on vend tout weights(k, t) et 
//           on achete tout weights(k, t+1)!!!
real Trader::delta(int k, int t) const
{
  // Should not be called on the cash
  if(first_asset_is_cash && k==0)
    PLERROR("*** INTERNAL *** call to delta(%d, %d) with first_asset_is_cash set as true", k, t);

  // Here we consider that the initial test portfolio is the last train portfolio. This assumption seems reasonable since, 
  //  if we sequentially train the model, we will always have, in practice, kept the previous portfolios... The only exception could be at the 
  //  very first time that test is called... For now it is neglected... 
  real current_wkt = weights(k, t);
  
  // We are sure that the weights(k, t+1) is available since the advisor did set it state field up to last_test_t, if horizon is at least 1.
  if( stop_loss(k, t) )
    return current_wkt;

  real delta_ = weights(k, t+1) - current_wkt;
  if(fabs(delta_) < rebalancing_threshold){
    weights(k, t+1) = current_wkt;             // Rebalancing isn't needed
    if(first_asset_is_cash){
      // Since we didn't follow the recommendation of the advisor (sell or buy), the cash position 
      //  will be affected. Didn't buy (delta_>0) we saved money; didn't sell (delta_<0), did not gain the expected money.
      weights(0, t+1) += delta_ * price(k, t); 
    }
    return 0.0;
  }
  return fabs(delta_);  //else: delta_ >= rebalancing_threshold
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

