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
   * $Id: ExhaustiveNearestNeighbors.cc,v 1.1 2004/12/20 15:46:50 chapados Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file StatefulLearner.cc */


#include <plearn/base/stringutils.h>
#include <plearn/kernel/DistanceKernel.h>
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
  "The output costs are simply the kernel values for each found training\n"
  "point.  The costs are named 'ker0', 'ker1', ..., 'kerK-1'.\n"
  "\n"
  "The training set is SAVED with this learner, under the option name\n"
  "'training_mat'. Otherwise, one would NOT be able to reload the learner\n"
  "and carry out test operations!\n"
  );

ExhaustiveNearestNeighbors::ExhaustiveNearestNeighbors()
  : inherited(),
    training_mat(),
    kernel(DistanceKernel())
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
  deepCopyField(kernel,       copies);
  inherited::makeDeepCopyFromShallowCopy(copies);
}


void ExhaustiveNearestNeighbors::forget()
{
  // Nothing to forget!
}
    
void ExhaustiveNearestNeighbors::train()
{
  // Train is instantaneous. :-)
}


void ExhaustiveNearestNeighbors::computeOutput(const Vec& input, Vec& output) const
{
  // Compute the output from the input.
  // int nout = outputsize();
  // output.resize(nout);
  // ...
}    

void ExhaustiveNearestNeighbors::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output. 
// ...
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


} // end of namespace PLearn
