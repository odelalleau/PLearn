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
   * $Id: PricingTransactionPairProfitFunction.h,v 1.4 2004/04/07 23:16:58 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef PricingTransactionPairProfitFunction_INC
#define PricingTransactionPairProfitFunction_INC

#include "Kernel.h"

namespace PLearn {
using namespace std;


/*!   profit function that takes (output,target) as arguments
  and computes a profit. The output has at least 2 elements
  which represent the number of units of the security bought
  (or sold, if <0), and the cash change due to the transaction at time t1. The target vector
  contains at least one element which represents the price of
  the security at a later time t2.
  Profit is computed as follows:
    transaction_loss_t2 =  nb_units_transaction>0 ? 
                           additive_cost + multiplicative_cost * |nb_units_transaction| * price_t2 : 0
    profit = cash_earned_at_t1 + nb_units_transaction * price_t2 - transaction_loss_t2;
  
  where additive_cost, multiplicative_cost, per_unit_cost
  are user-specified parameters that control transaction losses
  
*/
class PricingTransactionPairProfitFunction : public Kernel
{
    typedef Kernel inherited;

protected:
    real multiplicative_cost; //!<  transaction loss
    real additive_cost; //!<  transaction loss
    real per_unit_cost; //!<  transaction loss
public:
    PricingTransactionPairProfitFunction(){}
    PricingTransactionPairProfitFunction(real the_multiplicative_cost, 
                                         real the_additive_cost=0,
                                         real the_per_unit_cost=0) :
      multiplicative_cost(the_multiplicative_cost), additive_cost(the_additive_cost),
      per_unit_cost(the_per_unit_cost) {}
    
    PLEARN_DECLARE_OBJECT(PricingTransactionPairProfitFunction);
    
    virtual string info() const
        { return "pricing_pair_profit"; }

    virtual real evaluate(const Vec& output, const Vec& target) const; 

protected:
    //!  Recognized options: all 3 fields.
    static void declareOptions(OptionList &ol);    
};

DECLARE_OBJECT_PTR(PricingTransactionPairProfitFunction);

} // end of namespace PLearn

#endif

