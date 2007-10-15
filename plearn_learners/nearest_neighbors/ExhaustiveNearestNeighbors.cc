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
 * $Id$ 
 ******************************************************* */

// Authors: Nicolas Chapados

/*! \file StatefulLearner.cc */


#include "ExhaustiveNearestNeighbors.h"
#include <assert.h>
#include <plearn/base/stringutils.h>
#include <plearn/ker/DistanceKernel.h>

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
    "   - if false: the distance_kernel is a similarity measure\n"
    "   - if true (the default): the distance_kernel is a distance measure\n"
    "\n"
    "The output costs are simply the kernel values for each found training\n"
    "point.  The costs are named 'ker0', 'ker1', ..., 'kerK-1'.\n"
    "\n"
//    "The training set is SAVED with this learner, under the option name\n"
//    "'train_set'. Otherwise, one would NOT be able to reload the learner\n"
//    "and carry out test operations!\n"
    );

Ker ExhaustiveNearestNeighbors::default_kernel = new DistanceKernel();
  
ExhaustiveNearestNeighbors::ExhaustiveNearestNeighbors(
    Ker distance_kernel_, bool kernel_is_pseudo_distance_)
    : inherited(),
      kernel_is_pseudo_distance(kernel_is_pseudo_distance_)
{
    distance_kernel = distance_kernel_;
}

void ExhaustiveNearestNeighbors::declareOptions(OptionList& ol)
{
    /* // No longer needed: train_set is saved as part of the options of 
       // parent class GenericNearestNeighbors. See comment there.
    declareOption(
        ol, "training_mat", &ExhaustiveNearestNeighbors::training_mat,
        OptionBase::learntoption,
        "Saved training set");
    */

    declareOption(
        ol, "kernel_is_pseudo_distance",
        &ExhaustiveNearestNeighbors::kernel_is_pseudo_distance,
        OptionBase::buildoption,
        "Whether the kernel defined by the 'distance_kernel' option should be\n"
        "interpreted as a (pseudo-)distance measure (true) or a similarity\n"
        "measure (false). Default = true.  Note that this interpretation is\n"
        "strictly specific to the class ExhaustiveNearestNeighbors.\n");

    declareOption(
        ol, "kernel", &GenericNearestNeighbors::distance_kernel,
        OptionBase::buildoption | OptionBase::nosave,
        "Alternate name for 'distance_kernel'.  (Deprecated; use only so that\n"
        "existing scripts can run.)");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ExhaustiveNearestNeighbors::build_()
{
    if (! distance_kernel)
        PLERROR("ExhaustiveNearestNeighbors::build_: the 'distance_kernel' option "
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
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(dummy_vec,        copies);
    deepCopyField(tmp_indices,      copies);
    deepCopyField(tmp_distances,    copies);
    deepCopyField(cached_inputs,    copies);
}

void ExhaustiveNearestNeighbors::setTrainingSet(VMat training_set,
                                                bool call_forget)
{
    inherited::setTrainingSet(training_set, call_forget);
    cached_inputs.resize(0,0);
}


void ExhaustiveNearestNeighbors::forget()
{
    cached_inputs.resize(0,0);
}

void ExhaustiveNearestNeighbors::train()
{
    // Train is nearly instantaneous. :-)
    // Note: this conversion is performed on train() rather than
    // setTrainingSet since the training VMat may depend upon some
    // PLearners which may not have been trained when setTrainingSet is
    // called.  It's safer to delay the conversion until necessary.
    cached_inputs.resize(0,0);
    preloadInputCache();
}

// New implementation (more efficient)
void ExhaustiveNearestNeighbors::findNearestNeighbors(const Vec& input, int K, TVec<int>& indices, Vec& distances) const
{
    PLASSERT(pq.empty());
    if(cached_inputs.size()==0)
        preloadInputCache();    

    int l = train_set->length();
    for(int i=0; i<l; ++i) 
    {
        real d = distance_kernel(input, cached_inputs(i));
        if(!kernel_is_pseudo_distance) // make it distance-like (smaller means closer)
            d = -d;
        if(int(pq.size())<K)
            pq.push(pair<double,int>(d,i));
        else if(d<pq.top().first)
        {
            pq.pop();
            pq.push(pair<double,int>(d,i));
        }
    }

    int pqsize = (int)pq.size();
    indices.resize(pqsize);
    distances.resize(pqsize);

    for(int j=pqsize-1; j>=0; j--)
    {
        const pair<real,int>& cur_top = pq.top();
        real d = cur_top.first;
        if(!kernel_is_pseudo_distance) // restore actual kernel value (larger means closer)
            d = -d;
        distances[j] = d;
        indices[j] = cur_top.second;
        pq.pop();
    }
}


void ExhaustiveNearestNeighbors::computeOutputAndCosts(
    const Vec& input, const Vec& target, Vec& output, Vec& costs) const
{
    findNearestNeighbors(input, num_neighbors, tmp_indices, tmp_distances);
    int effective_num_neighbors = tmp_indices.size();
    costs.resize(num_neighbors);
    for(int j=0; j<effective_num_neighbors; j++)
        costs[j] = tmp_distances[j];
    // Make remaining costs into missing values if the found number of
    // neighbors is smaller than the requested number of neighbors
    for(int j=effective_num_neighbors; j<num_neighbors; j++)
        costs[j] = MISSING_VALUE;

    constructOutputVector(tmp_indices, output);
}


void ExhaustiveNearestNeighbors::computeOutput(const Vec& input, Vec& output) const
{
    findNearestNeighbors(input, num_neighbors, tmp_indices, tmp_distances);
    constructOutputVector(tmp_indices, output);
}


void ExhaustiveNearestNeighbors::computeCostsFromOutputs(
    const Vec& input, const Vec& output, const Vec& target, Vec& costs) const
{
    // Not really efficient (the output has probably already been computed).
    dummy_vec.resize(outputsize());
    computeOutputAndCosts(input, target, dummy_vec, costs);
}


TVec<string> ExhaustiveNearestNeighbors::getTestCostNames() const
{
    TVec<string> costs(num_neighbors);
    for (int i=0, n=num_neighbors ; i<n ; ++i)
        costs[i] = "ker" + tostring(i);
    return costs;
}


int ExhaustiveNearestNeighbors::nTestCosts() const
{
    return num_neighbors;
}


TVec<string> ExhaustiveNearestNeighbors::getTrainCostNames() const
{
    // No training statistics
    return TVec<string>();
}

void ExhaustiveNearestNeighbors::preloadInputCache() const
{
    int l = train_set->length();
    int ninputs = train_set->inputsize();
    cached_inputs.resize(l,ninputs);
    for(int i=0; i<l; i++)
        train_set->getSubRow(i,0,cached_inputs(i));
}

} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
