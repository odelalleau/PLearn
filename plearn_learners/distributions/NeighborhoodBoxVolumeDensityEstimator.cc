// -*- C++ -*-

// NeighborhoodBoxVolumeDensityEstimator.cc
//
// Copyright (C) 2006 Pascal Vincent
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
 * $Id: NeighborhoodBoxVolumeDensityEstimator.cc 5668 2006-05-25 17:50:24Z plearner $
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file NeighborhoodBoxVolumeDensityEstimator.cc */


#include "NeighborhoodBoxVolumeDensityEstimator.h"
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn_learners/nearest_neighbors/ExhaustiveNearestNeighbors.h>
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;

//////////////////
// NeighborhoodBoxVolumeDensityEstimator //
//////////////////
NeighborhoodBoxVolumeDensityEstimator::NeighborhoodBoxVolumeDensityEstimator()
    :nneighbors(1),
     min_radius(1e-6)
{
    // for default use Exhaustive search and default Euclidean distance
    NN = new ExhaustiveNearestNeighbors(); 
}

PLEARN_IMPLEMENT_OBJECT(NeighborhoodBoxVolumeDensityEstimator,
                        "Simple density estimation based on the volume of the smallest symmetic box, "
                        "centered on the test point, and containing its k neighbors.",
                        "This estimator is not guaranteed to sum to 1."
    );

////////////////////
// declareOptions //
////////////////////
void NeighborhoodBoxVolumeDensityEstimator::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "nneighbors", &NeighborhoodBoxVolumeDensityEstimator::nneighbors, OptionBase::buildoption,
                  "Number of neighbors to consider to determine the boxed region centered on the test point");

    declareOption(ol, "min_radius", &NeighborhoodBoxVolumeDensityEstimator::min_radius, OptionBase::buildoption,
                  "This is added to each dimension of the found box (as some dimensions may be initially 0)");

    declareOption(ol, "train_set", &NeighborhoodBoxVolumeDensityEstimator::train_set, OptionBase::learntoption,
                  "We need to store the training set, as this learner is memory-based...");

    declareOption(ol, "NN", &NeighborhoodBoxVolumeDensityEstimator::NN, OptionBase::buildoption,
                  "The nearest neighbor search method to use (default uses ehaustive search with Euclidean distance)");

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void NeighborhoodBoxVolumeDensityEstimator::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void NeighborhoodBoxVolumeDensityEstimator::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.

    // ### If the distribution is conditional, you should finish build_() by:
    // PDistribution::finishConditionalBuild();

    NN->num_neighbors = nneighbors;
    NN->copy_input = false;
    NN->copy_target = false;
    NN->copy_weight = false;
    NN->copy_index = true;
    NN->build();
    if(!train_set.isNull())
    {
        NN->setTrainingSet(train_set);
        NN->train();
    }
    NN_outputs.resize(nneighbors);
}

/////////////////
// log_density //
/////////////////
bool NeighborhoodBoxVolumeDensityEstimator::box_contains(const Vec& center, const Vec& radius, const Vec& input) const
{
    int w = center.length();
    for(int j=0; j<w; j++)
        if(fabs(input[j]-center[j])>radius[j])
            return false;
    return true;
}


real NeighborhoodBoxVolumeDensityEstimator::log_density(const Vec& y) const
{
    int l = train_set.length();
    int w = inputsize();
    int ws = train_set->weightsize();
    trainsample.resize(w+ws);
    Vec input = trainsample.subVec(0,w);

    // Find distance d to width_neighbor neighbour
    NN->computeOutput(y, NN_outputs);

    Vec radius(w);
    for(int k=0; k<NN_outputs.length(); k++)
    {
        train_set->getRow((int)NN_outputs[k],trainsample);
        for(int j=0; j<w; j++)
        {
            real d = fabs(y[j]-input[j]);
            radius[j] = max(radius[j],d);
        }
    }
    radius += min_radius;
    
    int region_count = 0;
    for(int i=0; i<l; i++)
    {
        train_set->getRow(i,trainsample);
        if(box_contains(y,radius,input))
            region_count++;
    }
    
    double log_region_volume = 0;
    for(int j=0; j<w; j++)
        log_region_volume += pl_log(radius[j]);

    return pl_log(double(region_count))-log_region_volume-pl_log(double(l));
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void NeighborhoodBoxVolumeDensityEstimator::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);
    deepCopyField(NN, copies);
}


// ### Remove this method, if your distribution does not implement it.
///////////
// train //
///////////
void NeighborhoodBoxVolumeDensityEstimator::train()
{
    NN->setTrainingSet(train_set);
    NN->train();
}


void NeighborhoodBoxVolumeDensityEstimator::forget()
{
    NN->forget();
}


}


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
