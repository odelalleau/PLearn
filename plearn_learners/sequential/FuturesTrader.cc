
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
   * $Id: FuturesTrader.cc,v 1.9 2003/09/29 21:02:46 ducharme Exp $ 
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

FuturesTrader::FuturesTrader()
{}

void FuturesTrader::build()
{
  inherited::build();
  build_();
}

void FuturesTrader::build_()
{}

void FuturesTrader::trader_test( int t, VMat testset, 
                                 real& absolute_return_t, real& relative_return_t) const
{
  absolute_return_t = relative_return_t = 0;
  
  real value_t = 0;
  real relative_sum = 0;
  for(int k=0; k < nb_assets; k++)
  { 
    // Relative return computation
    real w_kt = weight(k, t);
    if (w_kt != 0.0)
    {
      real p_kt = price(k, t);
      real v_kt = w_kt * p_kt;
      value_t += v_kt;
      relative_sum += v_kt * relative_return(k, t);

      // Update of the portfolio return, adding the k^th assets return
      absolute_return_t += w_kt * absolute_return(k, t);

      if(first_asset_is_cash && k==0)
        continue; // No call to delta and no transaction cost on cash

      // No additive_cost on a null delta since there will be no transaction
      real delta_ = delta(k, t);
      if( delta_ > 0 )
        absolute_return_t -= additive_cost + multiplicative_cost*delta_; // Not 1+mult since that cash pos should take care of the '1'
    }
    // Should there be a reinvestment procedure alike:
    // weight(k,t+1) += absolute_return_t;   ???
  }
  
  relative_return_t = relative_sum/value_t;
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

