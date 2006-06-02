// -*- C++ -*-

// LocallyMagnifiedDistribution.cc
//
// Copyright (C) 2005 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file LocallyMagnifiedDistribution.cc */


#include "LocallyMagnifiedDistribution.h"
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn_learners/nearest_neighbors/ExhaustiveNearestNeighbors.h>
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;

//////////////////
// LocallyMagnifiedDistribution //
//////////////////
LocallyMagnifiedDistribution::LocallyMagnifiedDistribution()
    :mode(0),
     width_neighbor(0),
     width_factor(1.0),
     width_optionname("sigma")
{
}

PLEARN_IMPLEMENT_OBJECT(LocallyMagnifiedDistribution,
                        "ONE LINE DESCR",
                        "NO HELP"
    );

////////////////////
// declareOptions //
////////////////////
void LocallyMagnifiedDistribution::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "mode", &LocallyMagnifiedDistribution::mode, OptionBase::buildoption,
                  "Output computation mode");

    declareOption(ol, "weighting_kernel", &LocallyMagnifiedDistribution::weighting_kernel, OptionBase::buildoption,
                  "The magnifying kernel that will be used to locally weigh the samples");

    declareOption(ol, "width_neighbor", &LocallyMagnifiedDistribution::width_neighbor, OptionBase::buildoption,
                  "If width_neighbor>0, kernel width is set to be the distance to the width_neighbor'th neighbor times the width_factor (Euclidean distance neighbors for now)\n"
                  "If width_neighbor==0, kernel is left as specified upon construction");

    declareOption(ol, "width_factor", &LocallyMagnifiedDistribution::width_factor, OptionBase::buildoption,
                  "Only used if width_neighbor>0 (see width_neighbor)");

    declareOption(ol, "width_optionname", &LocallyMagnifiedDistribution::width_optionname, OptionBase::buildoption,
                  "Only used if width_neighbor>0. The name of the option in the weighting kernel that should be used to set or modifiy its width");

    declareOption(ol, "localdistr", &LocallyMagnifiedDistribution::localdistr, OptionBase::buildoption,
                  "The distribution that will be trained with local weights obtained from the magnifying kernel");

    declareOption(ol, "train_set", &LocallyMagnifiedDistribution::train_set, OptionBase::learntoption,
                  "We need to store the training set, as this learner is memory-based...");

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void LocallyMagnifiedDistribution::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void LocallyMagnifiedDistribution::build_()
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

    if(width_neighbor>0)
    {
        NN = new ExhaustiveNearestNeighbors(); // for now use Exhaustive search and default Euclidean distance
        NN->num_neighbors = width_neighbor;
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
        NN_outputs.resize(width_neighbor);
        NN_costs.resize(width_neighbor);
    }

}

/////////////////
// log_density //
/////////////////
real LocallyMagnifiedDistribution::log_density(const Vec& y) const
{
    int l = train_set.length();
    int w = inputsize();
    int ws = train_set->weightsize();
    weights.resize(l);
    // 'weights' will contain the "localization" weights for the current test point.
    trainsample.resize(w+ws);
    Vec input = trainsample.subVec(0,w);

    int nneighbors = 0;
    if(width_neighbor>0)
    {
        // Find distance d to width_neighbor neighbour
        NN->computeOutputAndCosts(y, emptyvec, NN_outputs, NN_costs);
        nneighbors = NN_outputs.length();
        real d = NN_costs[width_neighbor-1];

        if(weighting_kernel.isNotNull())
        {
            // Now set new kernel width:
            real newwidth = d*width_factor;
            weighting_kernel->setOption(width_optionname,tostring(newwidth));
            weighting_kernel->build(); // rebuild to adapt to width change
        }
    }

    double weightsum = 0;

    VMat local_trainset;
    if(weighting_kernel.isNotNull()) // build a weighted local_trainset
    {
        for(int i=0; i<l; i++)
        {
            train_set->getRow(i,trainsample);
            real weight = weighting_kernel(y,input);
            if(ws==1)
                weight *= trainsample[w];
            weights[i] = weight;
            weightsum += weight;
        }

        VMat weight_column(columnmatrix(weights));

        if(ws==0) // append weight column
            local_trainset = hconcat(train_set, weight_column);
        else // replace last column by weight column
            local_trainset = hconcat(train_set.subMatColumns(0,inputsize()), weight_column);
        local_trainset->defineSizes(w,0,1);        
    }
    else // no weighting kernel, we just use the neighbors
    {
        Mat neighbors_input(nneighbors, inputsize_);
        for(int k=0; k<nneighbors; k++)
            train_set->getSubRow(int(NN_outputs[k]),0,neighbors_input(k));
        local_trainset = new MemoryVMatrix(neighbors_input);
        local_trainset->defineSizes(w,0,0);
    }
    localdistr->forget();
    localdistr->setTrainingSet(local_trainset);
    localdistr->train();
    double log_local_p = localdistr->log_density(y);

    switch(mode)
    {
    case 0:
        return log_local_p + pl_log((double)weightsum) - pl_log((double)l) - pl_log((double)weighting_kernel(input,input));
    case 1:
        return log_local_p;
    case 2:
        return pl_log((double)weightsum) - pl_log((double)l);
    case 3:
        return pl_log((double)weightsum);
    case 4:
        return log_local_p+pl_log((double)nneighbors)-pl_log((double)l);
    default:
        PLERROR("Invalid mode %d", mode);
        return 0; 
    }

}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LocallyMagnifiedDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);
    deepCopyField(weighting_kernel, copies);
    deepCopyField(localdistr, copies);
    deepCopyField(NN, copies);
}


// ### Remove this method, if your distribution does not implement it.
///////////
// train //
///////////
void LocallyMagnifiedDistribution::train()
{
    if(width_neighbor>0) // train nearest neighbor searcher
    {
        NN->setTrainingSet(train_set);
        NN->train();
    }
}


void LocallyMagnifiedDistribution::forget()
{
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
