// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: PricingTransactionPairProfitFunction.cc,v 1.3 2004/04/02 19:56:54 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "PricingTransactionPairProfitFunction.h"

/*#include <cmath>
#include "stringutils.h"
#include "Kernel.h"
#include "TMat_maths.h"
#include "PLMPI.h"*/
//////////////////////////
namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(PricingTransactionPairProfitFunction, "ONE LINE DESCR", "NO HELP");

string PricingTransactionPairProfitFunction::info() const { return "pricing_pair_profit"; }


real PricingTransactionPairProfitFunction::evaluate(const Vec& output, const Vec& target) const
{
  real nb_units_transaction = output[0];
  real cash_earned_at_t1 = output[1];
  real price_t2 = target[0];
  real transaction_loss_t2 = nb_units_transaction>0 ? additive_cost + 
    fabs(nb_units_transaction) * (price_t2 * multiplicative_cost + per_unit_cost) : 0;
  real profit = cash_earned_at_t1 + nb_units_transaction * price_t2 - transaction_loss_t2;
  return profit;
}


void PricingTransactionPairProfitFunction::write(ostream& out) const
{
  writeHeader(out,"PricingTransactionPairProfitFunction");
  writeField(out,"multiplicative_cost",multiplicative_cost);
  writeField(out,"addititive_cost",additive_cost);
  writeField(out,"per_unit_cost",per_unit_cost);
  writeFooter(out,"PricingTransactionPairProfitFunction");
}

void PricingTransactionPairProfitFunction::oldread(istream& in)
{
  readHeader(in,"PricingTransactionPairProfitFunction");
  readField(in,"multiplicative_cost",multiplicative_cost);
  readField(in,"addititive_cost",additive_cost);
  readField(in,"per_unit_cost",per_unit_cost);
  readFooter(in,"PricingTransactionPairProfitFunction");
}


/*
void PricingTransactionPairProfitFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname == "multiplicative_cost")
    PLearn::read(in,multiplicative_cost);
  else if (optionname == "additive_cost")
    PLearn::read(in,additive_cost);
  else if (optionname == "per_unit_cost")
    PLearn::read(in,per_unit_cost);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void PricingTransactionPairProfitFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "multiplicative_cost", &PricingTransactionPairProfitFunction::multiplicative_cost, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "additive_cost", &PricingTransactionPairProfitFunction::additive_cost, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "per_unit_cost", &PricingTransactionPairProfitFunction::per_unit_cost, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}



} // end of namespace PLearn

