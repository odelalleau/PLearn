

// -*- C++ -*-

// ConditionalGaussianDistribution.cc
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

#include "ConditionalGaussianDistribution.h"
#include "plapack.h"
#include "VMat_maths.h"
#include "TMat.h"

namespace PLearn <%
using namespace std;

ConditionalGaussianDistribution::ConditionalGaussianDistribution() 
  :inherited()
{
}


PLEARN_IMPLEMENT_OBJECT(ConditionalGaussianDistribution, "ONE LINE DESCR", "NO HELP");

void ConditionalGaussianDistribution::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave
  
  declareOption(ol, "mean", &ConditionalGaussianDistribution::mean, OptionBase::buildoption,
                "The mean of the gaussian distribution \n"
                "Could be learned on a training set or specified by calling setInput");

  declareOption(ol, "covariance", &ConditionalGaussianDistribution::covariance, OptionBase::buildoption,
                "The covariance of the gaussian distribution \n"
                "Could be learned on a training set or specified manually");
  
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

  string ConditionalGaussianDistribution::help()
  {
    // ### Provide some useful description of what the class is ...
    return 
      "ConditionalGaussianDistribution is a gaussian distribution in which the \n."
      "parameters could be learned or specified manually."
      + optionHelp();
  }


  void ConditionalGaussianDistribution::build()
  {
    inherited::build();
  }


  void ConditionalGaussianDistribution::train(VMat training_set)
{
  mean.resize(training_set.width());
  covariance.resize(training_set.width(), training_set.width());
  computeMeanAndCovar(training_set, mean, covariance);
}

void ConditionalGaussianDistribution::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Learner::makeDeepCopyFromShallowCopy(copies);
}

double ConditionalGaussianDistribution::log_density(const Vec& x) const
{ PLERROR("density not implemented yet for ConditionalGaussianDistribution"); return 0; }

double ConditionalGaussianDistribution::density(const Vec& x) const
{ return exp(log_density(x)); }
  
double ConditionalGaussianDistribution::survival_fn(const Vec& x) const
{ PLERROR("survival_fn not implemented yet for ConditionalGaussianDistribution"); return 0; }

double ConditionalGaussianDistribution::cdf(const Vec& x) const
{ PLERROR("cdf not implemented yet for ConditionalGaussianDistribution"); return 0; }

Vec ConditionalGaussianDistribution::expectation() const
{
  return mean;
}

Mat ConditionalGaussianDistribution::variance() const
{
  return covariance;
}

void ConditionalGaussianDistribution::generate(Vec& x) const
{
  x = multivariate_normal(mean, covariance);
}

void ConditionalGaussianDistribution::setInput(const Vec& input)
{
  mean.resize(input.size());
  mean << input;
}

%> // end of namespace PLearn
