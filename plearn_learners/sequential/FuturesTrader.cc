
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
   * $Id: FuturesTrader.cc,v 1.1 2003/08/29 15:40:39 dorionc Exp $ 
   ******************************************************* */

/*! \file FuturesTrader.cc */
#include "FuturesTrader.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_OBJECT(FuturesTrader, "ONE LINE DESCR", "NO HELP");

FuturesTrader::FuturesTrader()
{}

void FuturesTrader::assets(const VMat& vmat, TVec<string>& names)
{
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
  forget();
  
  assets(train_set, assets_names);
  cout << names << endl; //TMP
  nb_assets = assets_names.length();
  if(nb_assets == 0)
    PLERROR("Must provide a non empty TVec<string> assets_names");

#error  Gestion du price_map ICI (MPMV::build_)  
  
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
void FuturesTrader::build_test(const VMat& testset)
{
  test_set = testset;
  test_length = testset.length();
  
  test_weights = Map(test_length, nb_assets, horizon);
#error Rt = Map(1, test_length, horizon);  
}

void FuturesTrader::forget()
{
  inherited::forget();
  advisor->forget();

  if(!test_set.isNull()){
    test_weights.fill(0.0);        
    test_Rt.fill(0.0);         
    stop_loss_values.fill(0.0);
  }
}

void FuturesTrader::declareOptions(OptionList& ol)
{
  PLERROR("Not Done Yet!!!");
  
  inherited::declareOptions(ol);
}

void FuturesTrader::train()
{
  advisor->train();
}
 
void FuturesTrader::test(VMat testset, PP<VecStatsCollector> test_stats,
                         VMat testoutputs=0, VMat testcosts=0) const
{
#error multiplicative_cost ici ou dans computeOutputAndCosts
  build_test(testset_);
  
#ifdef MY_PROFILE
  string method_as_str = ("FuturesTrader::test (test_length=" + tostring(test_length) 
                          + "; N=" + tostring(nb_assets) + ")");  
  Profiler::start(method_as_str);
#endif

  //dorionc: args from BTMPMV::test
  real average = 0.0; 
  real sigmasquare = 0.0; 
  real sum_of_squared_w = 0.0;
  //---

  real Rt_sum = 0;
  real squaredRt_sum = 0;
  real value_sum = 0;
    
  PLWARNING("Revoir la formule de w_k_t (vs getPf)!!! Aussi faire une map pour ne pas appeler kernel->ev_i_x plrs fois sur les memes arg");
  PLWARNING("Erreur dans le calcul des profits: voir la formule dans stop_loss!");
  for(int t = 0; t < test_length; t++)
  {
    value_sum = 0;
    for(int k=1; k < nb_assets; k++)
    {
      if( !stop_loss(k, t) ){
        test_weights(k, t) = 0.0;      
        for(int j=0; j < data.length(); j++)
          test_weights(k, t) += alpha->matValue(j, k) * kernel->evaluate_i_x(j, testset_as_mat.row(t).toVecCopy());
      }
      
      trading_controls(k, t);
      
      value_sum += test_weights(k, t) * test_prices(k, t);       

      // Erreur dans le calcul des profits: voir la formule dans stop_loss!
      //  Ca ne devrait pas etre ici!!!
      Rtmap[t] += test_weights(k, t) * test_returns(k, t);
      
      sum_of_squared_w += square( test_weights(k,t) );
    }
    
    // Penalty on the value of w_{0t}
    test_weights(0,t) = capital_level - value_sum;
    sum_of_squared_w += square( test_weights(0,t) );
    
    Rt_sum += Rtmap[t];
    squaredRt_sum += square(Rtmap[t]);
  }
  
  test_weights_ = test_weights.mat;
  test_Rt = Rtmap.mat;
  average = (1.0/test_length)*Rt_sum;  
  sigmasquare = ( (1.0/(test_length-1)) * 
                  (squaredRt_sum - test_length*square(average)) );
  
#ifdef MY_PROFILE
  Profiler::end(method_as_str);
#endif
}

bool FuturesTrader::stop_loss(int k, int t)
{
  // The Stop Loss part has - for now - no sens for (t==0) since we suppose (test_weights(k, 0-1) == 0)
  if(!stop_loss_active || t==0) return false;
  
  // t >= 1
  stop_loss_values[k] += test_returns(k, t) * test_weights(k, t-1);  
  
  if(t > stop_loss_horizon)
    stop_loss_values[k] -= test_returns(k, t-stop_loss_horizon) * test_weights(k, t-stop_loss_horizon-1);    
  
  // Here, if |test_weights(k, t)| < rebalancing_threshold , the effect will be conterbalanced
  //  by the delta method: Is it a problem???
  if( stop_loss_values[k] < stop_loss_threshold )
  { 
    test_weights(k, t) = 0.0;             // Selling or covering short sales.
    stop_loss_values[k] = -INFINITY;      // This way: pi >= t ==> test_weights(k, pi) = 0 
    return true;
  }           
  
  // If we reach that point, stop_loss was not applyed
  return false;
}

real FuturesTrader::delta(int k, int t)
{
  // For now, the initial testing portfolio is considered to be of weights 0
  real prev_wkt = 0.0;
  if(t > 0)
    prev_wkt = test_weights(k, t-1);
  
  // The threshold testing
  real delta_ = fabs(test_weights(k, t) - prev_wkt);
  if(delta_ < rebalancing_threshold){
    test_weights(k, t) = prev_wkt;             // Rebalancing isn't needed
    return 0.0;
  }
  return delta_;  //else: delta_ >= rebalancing_threshold
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
  return ...;
}

TVec<string> FuturesTrader::getTestCostNames() const
{ return FuturesTrader::getTrainCostNames(); }

%> // end of namespace PLearn

