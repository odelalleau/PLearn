// -*- C++ -*-

// ExhaustiveNearestNeighbors.cc
//
// Copyright (C) 2004 Nicolas Chapados
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
   * $Id: ExhaustiveNearestNeighbors.cc,v 1.2 2004/12/21 07:13:15 chapados Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file StatefulLearner.cc */


#include <plearn/base/stringutils.h>
#include <plearn/ker/DistanceKernel.h>
#include "ExhaustiveNearestNeighbors.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
  ExhaustiveNearestNeighbors,
  "Classical nearest-neighbors implementation using exhaustive search",
  "This class provides the basic implementation of the classical O(N^2)\n"
  "nearest-neighbors algorithm.  For each test point, it performs an\n"
  "exhaustive search in the training set to find the K (specified by the\n"
  "inherited 'num_neighbors' option) closest examples according to a\n"
  "user-specified Kernel.\n"
  "\n"
  "It is important to specify whether the Kernel denotes a SIMILARITY or a\n"
  "(pseudo-)DISTANCE measure.  A similarity measure is HIGHER for points\n"
  "that are closer.  The GaussianKernel is a similarity measure.  On the\n"
  "other hand, a distance measure is LOWER for points that are closer.  A\n"
  "DistanceKernel is a distance measure.  The option\n"
  "'kernel_is_pseudo_distance' controls this:\n"
  "\n"
  "   - if false: the kernel is a similarity measure\n"
  "   - if true (the default): the kernel is a distance measure\n"
  "\n"
  "The output costs are simply the kernel values for each found training\n"
  "point.  The costs are named 'ker0', 'ker1', ..., 'kerK-1'.\n"
  "\n"
  "The training set is SAVED with this learner, under the option name\n"
  "'training_mat'. Otherwise, one would NOT be able to reload the learner\n"
  "and carry out test operations!\n"
  );

Ker ExhaustiveNearestNeighbors::default_kernel = new DistanceKernel();
  
ExhaustiveNearestNeighbors::ExhaustiveNearestNeighbors(
  Ker kernel_, bool kernel_is_pseudo_distance_)
  : inherited(),
    training_mat(),
    kernel(kernel_),
    kernel_is_pseudo_distance(kernel_is_pseudo_distance_)
{ }

void ExhaustiveNearestNeighbors::declareOptions(OptionList& ol)
{
  declareOption(
    ol, "training_mat", &ExhaustiveNearestNeighbors::training_mat,
    OptionBase::learntoption,
    "Saved training set");

  declareOption(
    ol, "kernel", &ExhaustiveNearestNeighbors::kernel,
    OptionBase::buildoption,
    "Kernel that must be used to evaluate distances.  Default is a\n"
    "DistanceKernel with n=2, which gives an Euclidian distance.");

  declareOption(
    ol, "kernel_is_pseudo_distance",
    &ExhaustiveNearestNeighbors::kernel_is_pseudo_distance,
    OptionBase::buildoption,
    "Whether the kernel should be interpreted as a (pseudo-)distance\n"
    "measure (true) or a similarity measure (false). Default = true.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void ExhaustiveNearestNeighbors::build_()
{
  if (! kernel)
    PLERROR("ExhaustiveNearestNeighbors::build_: the 'kernel' option "
            "must be specified");
}

// ### Nothing to add here, simply calls build_
void ExhaustiveNearestNeighbors::build()
{
  inherited::build();
  build_();
}


void ExhaustiveNearestNeighbors::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  deepCopyField(training_mat, copies);
  deepCopyField(costs,        copies);
  deepCopyField(kernel,       copies);
  deepCopyField(indexes,      copies);
  inherited::makeDeepCopyFromShallowCopy(copies);
}


void ExhaustiveNearestNeighbors::setTrainingSet(VMat training_set,
                                                bool call_forget)
{
  training_mat = training_set.toMat();
  inherited::setTrainingSet(training_set, call_forget);
}


void ExhaustiveNearestNeighbors::forget()
{
  // Nothing to forget!
}


void ExhaustiveNearestNeighbors::train()
{
  // Train is instantaneous. :-)
}


void ExhaustiveNearestNeighbors::computeOutputAndCosts(
  const Vec& input, const Vec& target, Vec& output, Vec& costs) const
{
  assert( costs.size() == num_neighbors );
  priority_queue< pair<real,int> > q;
  findNearestNeighbors(input, q);
  indexes.resize(0, num_neighbors);

  // Dequeue the found nearest-neighbors in order of largest
  int i = 0;
  for ( ; i<num_neighbors && ! q.empty(); ++i) {
    const pair<real,int>& cur_top = q.top();
    costs[i] = (kernel_is_pseudo_distance? -1 : +1) * cur_top.first;
    indexes.push_back(cur_top.second);
    q.pop();
  }

  // Make remaining costs into missing values if the found number of
  // neighbors is smaller than the requested number of neighbors
  for ( ; i < num_neighbors ; ++i )
    costs[i] = MISSING_VALUE;

  constructOutputVector(indexes, output);
}

void ExhaustiveNearestNeighbors::computeOutput(const Vec& input, Vec& output) const
{
  costs.resize(num_neighbors);
  Vec unused_targets;
  computeOutputAndCosts(input, unused_targets, output, costs);
}


void ExhaustiveNearestNeighbors::computeCostsFromOutputs(
  const Vec& input, const Vec& output, const Vec& target, Vec& costs) const
{
  // In general, this cannot be done since the output vector does not
  // necessarily contain the input vector, which would allow computing a
  // kernel value.  Make it an error for now.
  PLERROR("ExhaustiveNearestNeighbors::computeCostsFromOutputs: "
          "this function is not supported; use computeOutputAndCosts instead");
}                                


TVec<string> ExhaustiveNearestNeighbors::getTestCostNames() const
{
  TVec<string> costs(num_neighbors);
  for (int i=0, n=num_neighbors ; i<n ; ++i)
    costs[i] = "ker" + tostring(i);
  return costs;
}


TVec<string> ExhaustiveNearestNeighbors::getTrainCostNames() const
{
  // No training statistics
  return TVec<string>();
}


void ExhaustiveNearestNeighbors::findNearestNeighbors(
  const Vec& input,
  priority_queue< pair<real,int> >& q) const
{
  assert( q.empty() );

  Vec train_input;
  int inputsize = train_set->inputsize();
  for(int i=0, n=training_mat.length() ; i<n ; ++i) {
    train_input = training_mat(i).subVec(0,inputsize);
    real kernel_value = kernel(input, train_input);
    real q_value = (kernel_is_pseudo_distance? -1 : +1) * kernel_value;
    q.push(make_pair(q_value, i));
  }
}

} // end of namespace PLearn
