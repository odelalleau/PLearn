
// -*- C++ -*-

// ConstantRegressor.cc
//
// Copyright (C) 2003  *AUTHOR(S)* 
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
   * $Id: ConstantRegressor.cc,v 1.1 2003/08/04 21:28:48 yoshua Exp $ 
   ******************************************************* */

/*! \file ConstantRegressor.cc */
#include "ConstantRegressor.h"

namespace PLearn <%
using namespace std;

ConstantRegressor::ConstantRegressor() 
/* ### Initialise all fields to their default value here */
  {
    // ### You may or may not want to call build_() to finish building the object
    // build_();
  }

  PLEARN_IMPLEMENT_OBJECT_METHODS(ConstantRegressor, "ConstantRegressor", PLearner);

  void ConstantRegressor::declareOptions(OptionList& ol)
  {
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    declareOption(ol, "constant_output", &ConstantRegressor::constant_output, 
                  OptionBase::learntoption,
                  "This is the learnt parameter, the constant output. During training\n"
                  "It is set to the (possibly weighted) average of the targets.\n"
      );

    // Now call the parent class' declareOptions
    parentclass::declareOptions(ol);
  }

  string ConstantRegressor::help()
  {
    // ### Provide some useful description of what the class is ...
    return 
      "ConstantRegressor is a PLearner that outputs a constant (input-independent\n"
      "but training-data-dependent) vector. It is a regressor (i.e. during training\n"
      "the constant vector is chosen to minimize the (possibly weighted) average\n"
      "of the training set targets. Let\n"
      "  N = number of training examples,\n"
      "  M = target size (= output size),\n"
      "  y_{ij} = the jth target value of the ith training example,\n"
      "  w_i  = weight associated to the ith training example,\n"
      "then the j-th component of the learned vector is\n"
      "  (sum_{i=1}^N w_i * y_ij) / (sum_{i=1}^N w_i)\n";
  }

  void ConstantRegressor::build_()
  {
    constant_output.resize(outputsize());
  }

  // ### Nothing to add here, simply calls build_
  void ConstantRegressor::build()
  {
    parentclass::build();
    build_();
  }


  void ConstantRegressor::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    parentclass::makeDeepCopyFromShallowCopy(copies);
  }


int ConstantRegressor::outputsize() const
{
  return targetsize();
}

void ConstantRegressor::forget()
{
  // Since this is a one-shot learner, there is nothing to forget.
}
    
void ConstantRegressor::train()
{
  // The role of the train method is to bring the learner up to stage==nstages,
  // updating train_stats with training costs measured on-line in the process.

  static Vec input;  // static so we don't reallocate/deallocate memory each time...
  static Vec target;
  static Vec train_costs;
  static Vec sum_of_weighted_targets;
  real weight;
  train_costs.resize(1);
  input.resize(inputsize());    // the train_set's inputsize()
  target.resize(targetsize());  // the train_set's targetsize()
  sum_of_weighted_targets.resize(targetsize());  // the running sum of weighted targets

  if(!train_stats)  // make a default stats collector, in case there's none
    train_stats = new VecStatsCollector();

  real sum_of_weights = 0;
  sum_of_weighted_targets.clear();
      
  int n_examples = train_set->length();
  for (int i=0;i<n_examples;i++)
  {
    train_set->getExample(i, input, target, weight);
    multiplyAdd(sum_of_weighted_targets,target,weight,sum_of_weighted_targets);
    sum_of_weights += weight;
    multiply(sum_of_weighted_targets,1.0/sum_of_weights,constant_output);
    train_costs[0] = weight*powdistance(constant_output,target);
    train_stats->update(train_costs);
  }
  train_stats->finalize(); // finalize statistics for this one and only epoch
}


void ConstantRegressor::computeOutput(const Vec& input, Vec& output) const
{
  // Compute the output from the input
  output.resize(outputsize());
  output << constant_output;
}    

void ConstantRegressor::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
  // Compute the costs from *already* computed output. 
  costs[0] = powdistance(output,target);
}                                

TVec<string> ConstantRegressor::getTestCostNames() const
{
  // Return the names of the costs computed by computeCostsFromOutpus
  return getTrainCostNames();
}

TVec<string> ConstantRegressor::getTrainCostNames() const
{
  // Return the names of the objective costs that the train method computes and 
  // for which it updates the VecStatsCollector train_stats
  return TVec<string>(1,"mse");
}



%> // end of namespace PLearn
