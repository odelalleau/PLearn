

// -*- C++ -*-

// PConditionalDistribution.cc
// 
// Copyright (C) *YEAR* *AUTHOR(S)* 
// ...
// Copyright (C) *YEAR* *AUTHOR(S)* 
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

#include "PConditionalDistribution.h"

namespace PLearn <%
using namespace std;

PConditionalDistribution::PConditionalDistribution() 
  :PDistribution()
{
   
}

PLEARN_IMPLEMENT_OBJECT(PConditionalDistribution, 
                        "Conditional distribution or conditional density model P(Y|X)",
                        "Abstract superclass for conditional distribution classes.\n"
                        "It is a subclass of PDistribution, with the added method\n"
                        "   setInput(Vec& input)\n"
                        "to set X, that must be called before PDistribution methods such as\n"
                        "log_density,cdf,survival_fn,expectation,variance,generate.\n"
                        "The PDistribution option output_defs must be set to specify\n"
                        "what the PLearner method computeOutput will produce. If it is\n"
                        "set to 'l' (log_density), 'd' (density), 'c' (cdf), or 's' (survival_fn)\n"
                        "then the input part of the data should contain both the input X and\n"
                        "the 'target' Y values (targetsize()==0). Instead, if output_defs is set to\n"
                        " 'e' (expectation) or 'v' (variance), then the input part of the data should\n"
                        "contain only X, while the target part should contain Y\n");

void PConditionalDistribution::declareOptions(OptionList& ol)
{
  declareOption(ol, "input_part_size", &PConditionalDistribution::input_part_size, OptionBase::buildoption,
                "This option should be used only if outputs_def is 'l','d','c' or 's' (or upper case),\n"
                "which is when computeOutput takes as input both the X and Y parts to compute P(Y|X).\n"
                "This option gives the size of X, that is the length of the part of the data input which\n"
                "contains the conditioning part of the distribution. The rest of the data input vector should\n"
                "contain the Y value. If outputs_def is 'e' or 'v' or upper case then this option is ignored.\n");
  inherited::declareOptions(ol);
}

  void PConditionalDistribution::build_()
  {
    if (train_set)
    {
      if (outputs_def=="L" || outputs_def=="D" || outputs_def=="C" || outputs_def=="S" || outputs_def=="e" || outputs_def=="v")
        input_part_size = train_set->inputsize();
    }
  }

  // ### Nothing to add here, simply calls build_
  void PConditionalDistribution::build()
  {
    inherited::build();
    build_();
  }

void PConditionalDistribution::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

void PConditionalDistribution::setInput(const Vec& input) const
{ PLERROR("setInput must be implemented for this PConditionalDistribution"); }


void PConditionalDistribution::computeOutput(const Vec& input, Vec& output) const
{
  Vec x = input.subVec(0,input_part_size);
  int d=input.length()-input_part_size;
  Vec y = input.subVec(input_part_size,d);
  setInput(x);
  if (outputs_def=="l")
    output[0]=log_density(y);
  else if (outputs_def=="d")
    output[0]=density(y);
  else if (outputs_def=="c")
    output[0]=cdf(y);
  else if (outputs_def=="s")
    output[0]=survival_fn(y);
  else if (outputs_def=="e")
    expectation(output);
  else if (outputs_def=="v")
  {
    Mat covmat = output.toMat(d,d);
    variance(covmat);
  }
  else if (outputs_def=="L")
  {
    real lower = lower_bound;
    real upper = upper_bound;
    real delta = (upper - lower)/n_curve_points;
    Vec y(1); y[0]=lower;
    for (int i=0;i<n_curve_points;i++)
    {
      output[i] = log_density(y);
      y[0]+=delta;
    }
  }
  else if (outputs_def=="D")
  {
    real lower = lower_bound;
    real upper = upper_bound;
    real delta = (upper - lower)/n_curve_points;
    Vec y(1); y[0]=lower;
    for (int i=0;i<n_curve_points;i++)
    {
      output[i] = density(y);
      y[0]+=delta;
    }
  }
  else if (outputs_def=="C")
  {
    real lower = lower_bound;
    real upper = upper_bound;
    real delta = (upper - lower)/n_curve_points;
    Vec y(1); y[0]=lower;
    for (int i=0;i<n_curve_points;i++)
    {
      output[i] = cdf(y);
      y[0]+=delta;
    }
  }
  else if (outputs_def=="S")
  {
    real lower = lower_bound;
    real upper = upper_bound;
    real delta = (upper - lower)/n_curve_points;
    Vec y(1); y[0]=lower;
    for (int i=0;i<n_curve_points;i++)
    {
      output[i] = survival_fn(y);
      y[0]+=delta;
    }
  }
  else PLERROR("PConditionalDistribution: unknown setting of outputs_def = %s",outputs_def.c_str());
}

%> // end of namespace PLearn
