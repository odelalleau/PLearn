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
   * $Id: SemiSupervisedProbClassCostVariable.cc,v 1.4 2004/04/27 16:03:35 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SemiSupervisedProbClassCostVariable.h"
#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** SemiSupervisedProbClassVariable **/

PLEARN_IMPLEMENT_OBJECT(SemiSupervisedProbClassCostVariable,
                        "ONE LINE DESCR",
                        "NO HELP");

SemiSupervisedProbClassCostVariable::SemiSupervisedProbClassCostVariable(Var prob_, Var target_, Var prior_, real ff)
  : inherited(prob_ & target_ & (VarArray)prior_,1,1), flatten_factor(ff)
{
    build_();
}

void
SemiSupervisedProbClassCostVariable::build()
{
    inherited::build();
    build_();
}

void
SemiSupervisedProbClassCostVariable::build_()
{
    if (varray.size() >= 3 && varray[0] && varray[1] && varray[2]) {
        // varray[0], varray[1] and varray[2] are (respectively) prob_, target_ and prior_ from constructor
        if (varray[2]->length()>0 && varray[0]->length() != varray[2]->length())
            PLERROR("In SemiSupervisedProbClassCostVariable: If prior.length()>0 then prior and prob must have the same size");
        if (!varray[1]->isScalar())
            PLERROR("In SemiSupervisedProbClassCostVariable: target must be a scalar");
        raised_prob.resize(varray[0]->length());
    }
    if (flatten_factor <= 0)
        PLERROR("In SemiSupervisedProbClassCostVariable: flatten_factor must be positive, and even > 1 for normal use.");
}

void
SemiSupervisedProbClassCostVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "flatten_factor", &SemiSupervisedProbClassCostVariable::flatten_factor, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void SemiSupervisedProbClassCostVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }


void SemiSupervisedProbClassCostVariable::fprop()
{
  //!   If target is not missing:
  //!     cost = - log(prob[target])
  //!   Else
  //!     cost = - (1/flatten_factor) log sum_i (prior[i] * prob[i])^flatten_factor
  real target_value = target()->valuedata[0];
  int n=prob()->size();
  real* p=prob()->valuedata;
  if (finite(target_value)) // supervised case
    {
      int t = int(target_value);
      if (t<0 || t>=n)
        PLERROR("In SemiSupervisedProbClassCostVariable: target must be either missing or between 0 and %d incl.\n",prob()->size()-1);
      valuedata[0] = -safeflog(p[t]);
    }
  else // unsupervised case
    {
      sum_raised_prob=0;
      real* priorv = prior()->valuedata;
      for (int i=0;i<n;i++)
      {
        raised_prob[i] = pow(priorv[i]*p[i],flatten_factor);
        sum_raised_prob += raised_prob[i];
      }
      valuedata[0] = - safeflog(sum_raised_prob)/flatten_factor;
    }
}

void SemiSupervisedProbClassCostVariable::bprop()
{
  real target_value = target()->valuedata[0];
  int n=prob()->size();
  real* dprob=prob()->gradientdata;
  real* p=prob()->valuedata;
  if (finite(target_value)) // supervised case
    {
      int t = int(target_value);
      for (int i=0;i<n;i++)
        if (i==t && p[t]>0)
          dprob[i] += -gradientdata[0]/p[t];
    }
  else // unsupervised case
    {
      for (int i=0;i<n;i++)
        if (p[i]>0)
          {
            real grad = - gradientdata[0]*raised_prob[i]/(p[i]*sum_raised_prob);
            if (finite(grad))
              dprob[i] += grad;
          }
    }
}


void SemiSupervisedProbClassCostVariable::symbolicBprop()
{
  PLERROR("SemiSupervisedProbClassCostVariable::symbolicBprop() not implemented");
}


void SemiSupervisedProbClassCostVariable::rfprop()
{
  PLERROR("SemiSupervisedProbClassCostVariable::rfprop() not implemented");
}



} // end of namespace PLearn


