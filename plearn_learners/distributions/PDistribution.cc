
// -*- C++ -*-

// PDistribution.cc
//
// Copyright (C) 2003  Pascal Vincent 
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
   * $Id: PDistribution.cc,v 1.10 2004/02/20 21:14:46 chrish42 Exp $ 
   ******************************************************* */

/*! \file PDistribution.cc */
#include "PDistribution.h"

namespace PLearn {
using namespace std;

PDistribution::PDistribution() 
  :outputs_def("l"), n_curve_points(-1)
{}

  PLEARN_IMPLEMENT_OBJECT(PDistribution, 
                          "Distribution is the base class for distributions.\n",
                          "Distributions derive from PLearner (as some of them may be fitted to data with train() )\n"
                          "but they have additional methods allowing for ex. to compute density or generate data points.\n"
                          "The default implementations of the learner-type methods for computing outputs and costs work as follows:\n"
                          "  - the outputs_def option allows to choose what outputs are produced. \n"
                          "  - cost is by a vector of size 1 containing only the negative log-likelihood (NLL) i.e. -log_density).\n");

  void PDistribution::declareOptions(OptionList& ol)
  {
    declareOption(ol, "outputs_def", &PDistribution::outputs_def, OptionBase::buildoption,
                  "A string where the characters have the following meaning: \n"
                  "'l'-> log_density, 'd' -> density, 'c' -> cdf, 's' -> survival_fn,\n"
                  "and for conditional distributions: 'e' -> expectation, 'v' -> variance\n"
                  "In lower case 'l', 'd', 'c', 's' give the probability associated with\n"
                  "a given value of the observation. In upper case, the whole curve is\n"
                  "evaluated at regular intervals and produced in output (as a histogram). The\n"
                  "number of curve points is determined by the 'n_curve_points' option\n"
                  "Note that these options (upper case letters) only work for SCALAR Y variables."
                  "N.B. If the distribution is unconditional, dataset->targetsize() should be 0.\n"
                  "If the distribution is conditional P(Y|X) and 'l','d','c','s' (NOT upper case)\n"
                  "are selected, then the dataset->inputsize() should include both the X and Y\n"
                  "while dataset->targetsize() should be 0. If the upper case outputs_def are selected\n"
                  "then dataset->inputsize() should be the length of X (if conditional, or 0 otherwise), and\n"
                  "dataset->targetsize() should be the length of Y\n"
      );

    declareOption(ol, "n_curve_points", &PDistribution::n_curve_points, OptionBase::buildoption,
                  "The number of points for which the distribution is evaluated when outputs_defs\n"
                  "equals 'L', 'D', 'C' or 'S' (produce a histogram of the distribution or density).\n"
                  "The lower_bound and upper_bound options specify where the curve begins and ends.\n"
                  "Note that these options (upper case letters) only work for SCALAR Y variables."
                  );

    declareOption(ol, "lower_bound",  &PDistribution::lower_bound, OptionBase::buildoption,
                  "The lower bound of scalar Y values to compute a histogram of the distribution\n"
                  "when upper case outputs_def are specified.\n");
  
    declareOption(ol, "upper_bound",  &PDistribution::lower_bound, OptionBase::buildoption,
                  "The upper bound of scalar Y values to compute a histogram of the distribution\n"
                  "when upper case outputs_def are specified.\n");
  
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  void PDistribution::build_()
  {
  }

  // ### Nothing to add here, simply calls build_
  void PDistribution::build()
  {
    inherited::build();
    build_();
  }


void PDistribution::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}


int PDistribution::outputsize() const
{
  int l=0;
  int targetsize_ = train_set->targetsize();
  for (unsigned int i=0;i<outputs_def.length();i++)
    if (outputs_def[i]=='L' || outputs_def[i]=='D' || outputs_def[i]=='C' || outputs_def[i]=='S')
      l+=n_curve_points;
    else if (outputs_def[i]=='e')
      l+=targetsize_;
    else if (outputs_def[i]=='v') // by default assume variance is full nxn matrix 
      l+=targetsize_*targetsize_;
    else l++;
  return l;
}

void PDistribution::forget()
{ PLERROR("forget not implemented for this PDistribution"); }
    
double PDistribution::log_density(const Vec& x) const
{ PLERROR("density not implemented for this PDistribution"); return 0; }

double PDistribution::density(const Vec& x) const
{ return exp(log_density(x)); }
  
double PDistribution::survival_fn(const Vec& x) const
{ PLERROR("survival_fn not implemented for this PDistribution"); return 0; }

double PDistribution::cdf(const Vec& x) const
{ PLERROR("cdf not implemented for this PDistribution"); return 0; }

void PDistribution::expectation(Vec& mu) const
{ PLERROR("expectation not implemented for this PDistribution"); }

void PDistribution::variance(Mat& covar) const
{ PLERROR("variance not implemented for this PDistribution"); }

void PDistribution::resetGenerator(long g_seed) const
{ PLERROR("resetGenerator not implemented for this PDistribution"); }

void PDistribution::generate(Vec& x) const
{ PLERROR("generate not implemented for this PDistribution"); }

void PDistribution::train()
{ PLERROR("train not implemented for this PDistribution"); }


void PDistribution::computeOutput(const Vec& input, Vec& output) const
{
  int l = outputs_def.length();
  for(int i=0; i<l; i++)
    {
      switch(outputs_def[i])
        {
        case 'l':
          output[i] = (real) log_density(input);
          break;
        case 'd':
          output[i] = (real) density(input);
          break;
        case 'c':
          output[i] = (real) cdf(input);
          break;
        case 's':
          output[i] = (real) survival_fn(input);
          break;
        default:
          PLERROR("In Distribution::use unknown outputs_def character");
        }
    }
}    

void PDistribution::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
  if(outputs_def[0]!='l')
    PLERROR("In PDistribution::computeCostsFromOutputs currently can only 'compute' \n"
            "negative log likelihood from log likelihood returned as first output \n");

  costs.resize(1);
  costs[0] = -output[0];
}                                

TVec<string> PDistribution::getTestCostNames() const
{
  return TVec<string>(1,"NLL");
}

TVec<string> PDistribution::getTrainCostNames() const
{
  return TVec<string>(1,"NLL");
}

void PDistribution::generateN(const Mat& X) const
{
  if(X.width()!=inputsize())
    PLERROR("In PDistribution::generateN  matrix width (%d) differs from inputsize() (%d)", X.width(), inputsize());
  int N = X.length();  
  for(int i=0; i<N; i++)
    {
      Vec v = X(i);
      generate(v);
    }
}



} // end of namespace PLearn
