
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
   * $Id: FuturesTrader.cc,v 1.2 2003/09/08 18:44:30 dorionc Exp $ 
   ******************************************************* */

/*! \file FuturesTrader.cc */
#include "FuturesTrader.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_OBJECT(FuturesTrader, "ONE LINE DESCR", "NO HELP");

FuturesTrader::FuturesTrader():
  price_tag("close:level"), tradable_tag("is_tradable"),
  capital_level(1.0), additive_cost(0.0), multiplicative_cost(0.0),
  rebalancing_threshold(0.0), 
  stop_loss_active(false), stop_loss_horizon(-1), stop_loss_threshold(-INFINITY)
{}

void FuturesTrader::setTrainingSet(VMat training_set, bool call_forget/*=true*/)
{
  inherited::setTrainingSet(training_set, call_forget);
  advisor->setTrainingSet(training_set, call_forget);
}

void FuturesTrader::assets(const VMat& vmat, TVec<string>& names)
{
  names = TVec<string>();
  if(vmat.isNull())
    return;

  Array<VMField> finfo = vmat->getFieldInfos();  
  int pos=-1;
  string current="";
  string last=""; 
  for(int info=0; info < finfo.size(); info++)
  {
    pos = finfo[info].name.find_first_of(":");
    if( pos == -1 )
      continue;
    
    current = finfo[info].name.substr(0, pos);
    if(current == last)
      continue;
    
    names.append(current);
    last = current;
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
  
  forget();

  if( stop_loss_active && 
      (stop_loss_horizon == -1 || stop_loss_threshold == -INFINITY) )
    PLERROR("In FuturesTrader::build_()\n\t"
            "stop_loss_active is true but stop_loss_horizon and/or stop_loss_threshold are not properly setted");

  // Gestion of the list of names
  assets(train_set, assets_names);
  cout << assets_names << endl; //TMP
  nb_assets = assets_names.length();
  if(nb_assets == 0)
    PLERROR("Must provide a non empty TVec<string> assets_names");

  // Building a list of the indices needed to access the prices in the VMat
  assets_price_indices    = TVec<int>(nb_assets);
  assets_tradable_indices = TVec<int>(nb_assets);
  for(int k=0; k < nb_assets; k++){
    assets_price_indices[k] = train_set->fieldIndex(assets_names[k]+":"+price_tag);
    assets_tradable_indices[k] = train_set->fieldIndex(assets_names[k]+":"+tradable_tag);
  }

//#error  Gestion du price_map ICI (MPMV::build_)  
  
  if(stop_loss_active){
    if(stop_loss_horizon == -1 || stop_loss_threshold == -1)
      PLERROR("stop_loss_horizon && stop_loss_threshold must be setted when stop_loss_active is true");
    else
      stop_loss_values = Vec(nb_assets, 0.0);
  }
}

/*!
  Initializes test_set, test_length, test_weights, test_Rt members
*/
//dorionc: BTMPMV::
void FuturesTrader::build_test(const VMat& testset) const
{
  test_set = testset;
  test_length = testset.length()-train_set.length();
  
  //test_weights_ = Mat(test_length-horizon, nb_assets);  //Map(nb_assets, test_length, horizon);
  advisor->state = Mat(0, nb_assets);
//#error Rt = Map(1, test_length, horizon);  
}

void FuturesTrader::forget()
{
  inherited::forget();
  advisor->forget();

  if(!test_set.isNull()){
    advisor->state.fill(0.0); //test_weights_.fill(0.0);        
//    test_Rt.fill(0.0);         
    stop_loss_values.fill(0.0);
  }
}

void FuturesTrader::declareOptions(OptionList& ol)
{
  declareOption(ol, "advisor", &FuturesTrader::advisor,
                OptionBase::buildoption,
                "An embedded learner issuing recommendations on what should the portfolio look like");
    
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
  
  declareOption(ol, "capital_level", &FuturesTrader::capital_level,
                OptionBase::buildoption,
                "Each w_0t will be set to (capital_level - \\sum_{k>0} w_kt)\n"
                "Default: 1.0");
  
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

  inherited::declareOptions(ol);
}

void FuturesTrader::train()
{
  last_call_train_t = train_set.length();
  if(last_call_train_t < last_train_t+train_step)
  { PLWARNING("last_call_train_t < last_train_t+train_step"); return; }
  advisor->train();
}
 
void FuturesTrader::test(VMat testset, PP<VecStatsCollector> test_stats,
                         VMat testoutputs/*=0*/, VMat testcosts/*=0*/) const
{
  build_test(testset);
  Mat testset_as_mat = test_set.toMat(); // To avoid multiple call to toMat() (see the for loops)
  
#ifdef MY_PROFILE
  string method_as_str = ("FuturesTrader::test (test_length=" + tostring(test_length) 
                          + "; N=" + tostring(nb_assets) + ")");  
  Profiler::start(method_as_str);
#endif

  // Appending a new StatsCollector at the end of test_stats. The 
  //  new StatsCollector will be used to keep track of portfolio test returns stats.
  int Rt_stat = test_stats->length();
  test_stats->stats.append(StatsCollector());
  
  real delta_;
  real Rt = 0;
  for(int t = 0; t < test_length; t++) 
  {  
    advisor->test(test_set.subMat(0, 0, train_set.length()+t+1, test_set.width()), test_stats);
    
//#error Erreur dans le calcul des profits: voir la formule dans stop_loss
    Rt = 0;
    for(int k=0; k < nb_assets; k++)
    {
      // Must be called before computing anything with test_weights(k, t) 
      //  since the method may update the weight
      delta_ = delta(k, t);
      
      // Update of the portfolio return, adding the k^th assets return
      Rt += test_weights(k, t) * test_return(k, t);
      
      // No additive_cost on a null delta since there will be no transaction
      if( delta_ > 0 )
        Rt -= additive_cost + multiplicative_cost*delta(k, t);
    }
#ifdef VERBOSE 
    cout << "R" << t << ": " << Rt << endl; 
#endif

    // The following four lines should be resumed to the last one as soon as possible! 
    if(t==0)
      PLWARNING("First test period HACK");
    else
      test_stats->stats[Rt_stat].update(Rt);
  }
  
  real average = test_stats->getStats(Rt_stat).mean(); //(1.0/test_length)*Rt_sum; 
  real sigmasquare = test_stats->getStats(Rt_stat).variance();
  //( (1.0/(test_length-1)) * (squaredRt_sum - test_length*square(average)) );
  
  cout << "***** ***** *****" << endl
       << "Test: " << endl
#if defined(VERBOSE) || defined(VERBOSE_TEST)
       << "\t weights:\n" << advisor->state << endl
#endif
       << "\t Average Return:\t" << average << endl
       << "\t Empirical Variance:\t" << sigmasquare << endl
       << "\t Sharpe Ratio:\t" << average/sqrt(sigmasquare) << endl
       <<  "***** ***** *****" << endl;
  
#ifdef MY_PROFILE
  Profiler::end(method_as_str);
#endif
}

real FuturesTrader::delta(int k, int t) const
{
  // For now, the initial testing portfolio is considered to be of weights 0
  real prev_wkt = 0.0;
  if(t > 0)
    prev_wkt = test_weights(k, t-1);
  
  if( stop_loss(k, t) )
    return prev_wkt;
  
  // The threshold testing
  real delta_ = fabs(test_weights(k, t) - prev_wkt);
  if(delta_ < rebalancing_threshold){
    test_weights(k, t) = prev_wkt;             // Rebalancing isn't needed
    return 0.0;
  }
  return delta_;  //else: delta_ >= rebalancing_threshold
}

bool FuturesTrader::stop_loss(int k, int t) const
{
  // The Stop Loss part has - for now - no sens for (t==0) since we suppose (test_weights(k, 0-1) == 0)
  if(!stop_loss_active || t==0) return false;
  
  // t >= 1
  stop_loss_values[k] += test_return(k, t) * test_weights(k, t-1);
  
  if(t > stop_loss_horizon)
    stop_loss_values[k] -= test_return(k, t-stop_loss_horizon) * test_weights(k, t-stop_loss_horizon-1);    
  
  // Here, if |test_weights(k, t)| < rebalancing_threshold , the effect will be conterbalanced
  //  by the delta method: Is it a problem???
  if( stop_loss_values[k]/stop_loss_horizon < stop_loss_threshold )
  { 
    test_weights(k, t) = 0.0;             // Selling or covering short sales.
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

  // ### Remove this line when you have fully implemented this method.
  PLERROR("FuturesTrader::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
} 

TVec<string> FuturesTrader::getTrainCostNames() const
{
  return advisor->getTrainCostNames();
}

TVec<string> FuturesTrader::getTestCostNames() const
{ return FuturesTrader::getTrainCostNames(); }

%> // end of namespace PLearn

