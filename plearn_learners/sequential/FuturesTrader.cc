
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
   * $Id: FuturesTrader.cc,v 1.6 2003/09/24 15:23:42 ducharme Exp $ 
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
  build_complete(false),
  nb_assets(0),
  very_first_test_t(-1),
  first_asset_is_cash(true), risk_free_rate("risk_free_rate"),
  price_tag("close:level"), tradable_tag("is_tradable"), //return_type(1),
  additive_cost(0.0), multiplicative_cost(0.0),
  rebalancing_threshold(0.0), 
  stop_loss_active(false), stop_loss_horizon(-1), stop_loss_threshold(-INFINITY),
  sp500(""), assets_names(TVec<string>()), deduce_assets_names(false)
{}

void FuturesTrader::setTrainingSet(VMat training_set, bool call_forget/*=true*/)
{
  inherited::setTrainingSet(training_set, call_forget);
  advisor->setTrainingSet(training_set, call_forget);
}

void FuturesTrader::assets_info(const VMat& vmat, TVec<string>& names,
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

void FuturesTrader::build()
{
  inherited::build();
  build_();
}

void FuturesTrader::build_()
{
  if(train_set.isNull())
    return;
  
  // Gestion of the list of names
  if(assets_names.isNotEmpty())
  {
    //PLWARNING("Multiple call to FuturesTrader::build_");
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
      PLERROR("In FuturesTrader::build_()\n\t"
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

void FuturesTrader::forget()
{
  inherited::forget();
  advisor->forget();

  if(stop_loss_active && !test_set.isNull()){
    stop_loss_values.fill(0.0);
  }
  Rt_stat.forget();
  log_returns.forget();
}

void FuturesTrader::declareOptions(OptionList& ol)
{
  declareOption(ol, "advisor", &FuturesTrader::advisor,
                OptionBase::buildoption,
                "An embedded learner issuing recommendations on what should the portfolio look like");

  
  declareOption(ol, "first_asset_is_cash", &FuturesTrader::first_asset_is_cash,
                OptionBase::buildoption,
                "To be set as true if the undrlying models has a cash position. If it is the case, the \n"
                "cash position will be considered to be the 1^rst in the portfolio, as reflected in the option's\n"
                "name. Default: true.");
  
  declareOption(ol, "risk_free_rate", &FuturesTrader::risk_free_rate,
                OptionBase::buildoption,
                "The risk free rate column name in the VMat. Default: risk_free_rate.\n");

  declareOption(ol, "price_tag", &FuturesTrader::price_tag,
                OptionBase::buildoption,
                "The string such that asset_name:price_tag is the field name\n" 
                "of the column containing the price.\n"
                "Default: \"close:level\"");
  
  declareOption(ol, "tradable_tag", &FuturesTrader::tradable_tag,
                OptionBase::buildoption,
                "The string such that asset_name:tradable_tag is the field name\n"
                "of the column containing the tradable or not boolean.\n"
                "Default: \"is_tradable\"");

//   declareOption(ol, "return_type", &FuturesTrader::return_type,
//                 OptionBase::buildoption,
//                 "The return type field is to be setted according to which formula the user\n\t"
//                 "wants the trader to use for returns. The DEFAULT is 1.\n\n"
//                 "1.- Absolute returns on period t:\n\t"
//                 "r_kt = p_kt - p_k(t-horizon)\n\n"
//                 "2.- Relative returns on period t:\n\t"
//                 "       p_kt - p_k(t-horizon)\n\t"
//                 "r_kt = ---------------------\n\t"
//                 "          p_k(t-horizon)\n\n"
//                 "The current version of 2 is partly wrong because, even the formula is\n"
//                 "correctly implemented, in the test code, these returns are still multiplied\n"
//                 "with absolute weights (instead of relative ones)");

  declareOption(ol, "additive_cost", &FuturesTrader::additive_cost,
                OptionBase::buildoption,
                "The fix cost of performing a trade.\n"
                "Default: 0");
  
  declareOption(ol, "multiplicative_cost", &FuturesTrader::multiplicative_cost,
                OptionBase::buildoption,
                "The cost of performing a 1$ value trade\n"
                "Default: 0");
  
  declareOption(ol, "rebalancing_threshold", &FuturesTrader::rebalancing_threshold,
                OptionBase::buildoption,
                "The minimum amplitude for a transaction to be considered worthy\n"
                "Default: 0");
  
  declareOption(ol, "stop_loss_active", &FuturesTrader::stop_loss_active,
                OptionBase::buildoption,
                "Is the PMV using the stop_loss protection\n"
                "Default: false");
  
  declareOption(ol, "stop_loss_horizon", &FuturesTrader::stop_loss_horizon,
                OptionBase::buildoption,
                "The horizon on which the loss/gains are considered");
  
  declareOption(ol, "stop_loss_threshold", &FuturesTrader::stop_loss_threshold,
                OptionBase::buildoption,
                "The value under which the stop loss is triggered");

  declareOption(ol, "sp500", &FuturesTrader::sp500,
                OptionBase::buildoption,
                "If the user wants the test method to compute the model log-returns correlation\n"
                "with the S&P500 index, he only needs to provide the sp500 field name in the vmat");
  
  declareOption(ol, "assets_names", &FuturesTrader::assets_names,
                OptionBase::buildoption,
                "The name of all the assets");

  declareOption(ol, "deduce_assets_names", &FuturesTrader::deduce_assets_names,
                OptionBase::buildoption,
                "Whether or not the assets names should be parse by the assets_info method");

  inherited::declareOptions(ol);
}

/*!
  The advisior->train method is assumed to compute and set the advisor->state up to the 
   (last_call_train_t)^th row. If (last_train_t < last_call_train_t), the 
   advisor should have copy state[last_train_t] to the 
   (last_call_train_t - last_train_t) following lines
*/
void FuturesTrader::train()
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

  
  // If the testset provided is not larger than the previous one, the test method 
  //  won't do anything.
  if(testset.length()-1 <= last_test_t){
    PLWARNING("test was called with testset.length()-1 <= last_test_t");
    return;
  }
  // Since we reached that point, we know we are going to test. Therefore, there is no arm
  //  in modifying directly last_test_t. If we retrained the model after the last test, there is
  //  no need for doing the computation twice and that why:
  if(last_test_t < last_call_train_t)
    last_test_t = last_call_train_t;
  
  // Keeping references for easy access to test_set
  test_set = testset;
  //test_length = testset.length()-train_set.length();  
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
      real v_kt = test_weights(k, t) * price(k, t);
      value_t += v_kt;
      relative_sum += v_kt * relative_return(k, t);
      
      // Update of the portfolio return, adding the k^th assets return
      Rt += test_weights(k, t) * absolute_return(k, t);
      
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
    // test_weights(k,t+1) += Rt;                                     ???


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

//TBAdded: Somewhere after stop_loss, a check_if_roll_over => on vend tout test_weights(k, t) et 
//           on achete tout test_weights(k, t+1)!!!
real FuturesTrader::delta(int k, int t) const
{
  // Should not be called on the cash
  if(first_asset_is_cash && k==0)
    PLERROR("*** INTERNAL *** call to delta(%d, %d) with first_asset_is_cash set as true", k, t);

  // Here we consider that the initial test portfolio is the last train portfolio. This assumption seems reasonable since, 
  //  if we sequentially train the model, we will always have, in practice, kept the previous portfolios... The only exception could be at the 
  //  very first time that test is called... For now it is neglected... 
  real current_wkt = test_weights(k, t);
  
  // We are sure that the test_weights(k, t+1) is available since the advisor did set it state field up to last_test_t, if horizon is at least 1.
  if( stop_loss(k, t) )
    return current_wkt;

  real delta_ = test_weights(k, t+1) - current_wkt;
  if(fabs(delta_) < rebalancing_threshold){
    test_weights(k, t+1) = current_wkt;             // Rebalancing isn't needed
    if(first_asset_is_cash){
      // Since we didn't follow the recommendation of the advisor (sell or buy), the cash position 
      //  will be affected. Didn't buy (delta_>0) we saved money; didn't sell (delta_<0), did not gain the expected money.
      test_weights(0, t+1) += delta_ * price(k, t); 
    }
    return 0.0;
  }
  return fabs(delta_);  //else: delta_ >= rebalancing_threshold
}

bool FuturesTrader::stop_loss(int k, int t) const
{
  if(!stop_loss_active) return false;
  
  stop_loss_values[k] += absolute_return(k, t) * test_weights(k, t);
  
  if(t-very_first_test_t+1 > stop_loss_horizon)
    stop_loss_values[k] -= absolute_return(k, t-stop_loss_horizon) * test_weights(k, t-stop_loss_horizon);    
  
  if( stop_loss_values[k]/stop_loss_horizon < stop_loss_threshold )
  {
    if(first_asset_is_cash){
      // Since we didn't follow the recommendation of the advisor (sell or buy), the cash position 
      //  will be affected. Didn't buy: delta_>0; didn't sell: delta_<0.
      test_weights(0, t+1) += test_weights(k,t+1) * price(k, t); 
    }
    test_weights(k, t+1) = 0.0;             // Selling or covering short sales.
    // Une autre question... Idealement il faudrait  avoir une facon de determiner pendant combien
    // de temps on n'echange plus sur un asset suite a un stop_loss..
    stop_loss_values[k] = -INFINITY;      // This way: pi >= t ==> test_weights(k, pi) = 0 
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

void FuturesTrader::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  PLERROR("FuturesTrader::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
} 


/*!
  Since FuturesTrader::train mainly call its advisor train method
   it is only normal that FuturesTrader::getTrainCostNames calls FT's 
   advisor getTrainCostNames method. Make sure it is properly implemented.
*/
TVec<string> FuturesTrader::getTrainCostNames() const
{
  return advisor->getTrainCostNames();
}

TVec<string> FuturesTrader::getTestCostNames() const
{ return FuturesTrader::getTrainCostNames(); }

%> // end of namespace PLearn

