

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Pascal Vincent
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



#include "EmpiricalDistribution.h"
#include <plearn/math/random.h>
#include <plearn/vmat/VMat_maths.h>

namespace PLearn {
using namespace std;



PLEARN_IMPLEMENT_OBJECT(EmpiricalDistribution, "ONE LINE DESCR", "NO HELP");

void EmpiricalDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(data, copies);
}


EmpiricalDistribution::EmpiricalDistribution()
  :inherited()
{
  seed();
}


EmpiricalDistribution::EmpiricalDistribution(int inputsize, bool random_sample_)
  :inherited(), random_sample(random_sample_)
{
  inputsize_ = inputsize;
  current_sample_x = 0;
  current_sample_y = 0;
  flip = false;
  seed();
}



void EmpiricalDistribution::declareOptions(OptionList& ol)
{
  inherited::declareOptions(ol);
}                


void EmpiricalDistribution::train(VMat training_set)
{
  if(inputsize_ == 0)
    PLERROR("inputsize_ must be specified before training");
  data = training_set.subMatColumns(0, inputsize_);
  targetsize_ = data.width()-inputsize_;
  length = training_set.length();
}

double EmpiricalDistribution::log_density(const Vec& x) const
{
  PLERROR("Density not implemented for EmpiricalDistribution");
  return 0;
}


double EmpiricalDistribution::survival_fn(const Vec& x) const
{
  double nbHigher = 0;
  bool addOne;
  for(int i = 0; i<data.length(); i++){
    addOne = true;
    for(int j = 0;j<data.width(); j++){
      if(data(i,j) <= x[j])
        addOne = false;
    }
    if(addOne)
      nbHigher++;
  }
  return nbHigher / ((double) data.length());
}

double EmpiricalDistribution::cdf(const Vec& x) const
{
  double nbLower = 0;
  bool addOne;
  for(int i = 0; i<data.length(); i++){
    addOne = true;
    for(int j = 0;j<data.width(); j++){
      if(data(i,j) >= x[j])
        addOne = false;
    }
    if(addOne)
      nbLower++;
  }
  return nbLower / ((double) data.length());
}


Vec EmpiricalDistribution::expectation() const
{
  Vec mean(inputsize_);
  computeMean(data, mean);
  return mean;
}

Mat EmpiricalDistribution::variance() const
{

  Vec mean(inputsize_);
  Mat covar(inputsize_,inputsize_);
  computeMeanAndCovar(data, mean, covar);
  return covar;
}


void EmpiricalDistribution::generate(Vec& x) const
{
  if(random_sample){
    x.resize(data.width());
    x << data(uniform_multinomial_sample(length));
  }
  //Hack for generating all the possible combinations of two
  //examples.
  else{
    if(!flip){
      x.resize(data.width());
      x << data(current_sample_x);
      flip = true;
    }
    else{
      x.resize(data.width());
      x << data(current_sample_y);
      current_sample_y++;
      flip = false;
    }
    if(current_sample_y == length){
      current_sample_y = 0;
      current_sample_x++;
    }
    if(current_sample_x == length)
      current_sample_x = 0;
  }
}

} // end of namespace PLearn
