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
#include <plearn_learners/distributions/GaussianDistribution.h>

namespace PLearn {
using namespace std;

//////////////////
// LocallyMagnifiedDistribution //
//////////////////
LocallyMagnifiedDistribution::LocallyMagnifiedDistribution()
    :display_adapted_width(true),
     mode(0),
     computation_neighbors(-1),
     kernel_adapt_width_mode(' '),
     fix_localdistr_center(true),
     width_neighbors(1.0),
     width_factor(1.0),
     width_optionname("sigma")
{
}

PLEARN_IMPLEMENT_OBJECT(LocallyMagnifiedDistribution,
                        "Density estimation by fitting a local model (specified by localdistr) to a view of the training samples, magnified locally around the test point.",
                        ""
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

    declareOption(ol, "computation_neighbors", &LocallyMagnifiedDistribution::computation_neighbors, OptionBase::buildoption,
                  "This indicates to how many neighbors we should restrict ourselves for the computations.\n"
                  "(it's equivalent to giving all other data points a weight of 0)\n"
                  "If <=0 we use all training points (with an appropriate weight).\n"
                  "If >1 we consider only that many neighbors of the test point;\n"
                  "If between 0 and 1, it's considered a coefficient by which to multiply\n"
                  "the square root of the numbder of training points, to yield the actual \n"
                  "number of computation neighbors used");

    declareOption(ol, "weighting_kernel", &LocallyMagnifiedDistribution::weighting_kernel, OptionBase::buildoption,
                  "The magnifying kernel that will be used to locally weigh the samples.\n"
                  "If it is left null then all computation_neighbors will receive a weight of 1\n");

    declareOption(ol, "kernel_adapt_width_mode", &LocallyMagnifiedDistribution::kernel_adapt_width_mode, OptionBase::buildoption,
                  "This controls how we adapt the width of the kernel to the local neighborhood of the test point.\n"
                  "' ' means leave width unchanged\n"
                  "'A' means set the width to width_factor times the average distance to the neighbors determined by width_neighborss.\n" 
                  "'M' means set the width to width_faactor times the maximum distance to the neighbors determined by width_neighborss.\n");

    declareOption(ol, "width_neighbors", &LocallyMagnifiedDistribution::width_neighbors, OptionBase::buildoption,
                  "width_neighbors tells how many neighbors to consider to determine the kernel width.\n"
                  "(see kernel_adapt_width_mode) \n"
                  "If width_neighbors>1 we consider that many neighbors.\n"
                  "If width_neighbors>=0 and <=1 it's considered a coefficient by which to multiply\n"
                  "the square root of the numbder of training points, to yield the actual \n"
                  "number of neighbors used");

    declareOption(ol, "width_factor", &LocallyMagnifiedDistribution::width_factor, OptionBase::buildoption,
                  "Only used if width_neighbors>0 (see width_neighbors)");

    declareOption(ol, "width_optionname", &LocallyMagnifiedDistribution::width_optionname, OptionBase::buildoption,
                  "Only used if kernel_adapt_width_mode!=' '. The name of the option in the weighting kernel that should be used to set or modifiy its width");

    declareOption(ol, "localdistr", &LocallyMagnifiedDistribution::localdistr, OptionBase::buildoption,
                  "The kind of distribution that will be trained with local weights obtained from the magnifying kernel.\n"
                  "If left unspecified (null), it will be set to GaussianDistribution by default.");

    declareOption(ol, "fix_localdistr_center", &LocallyMagnifiedDistribution::fix_localdistr_center, OptionBase::buildoption,
                  "If true, and localdistr is GaussianDistribution, then the mu of the localdistr will be forced to be the given test point.");

    declareOption(ol, "train_set", &LocallyMagnifiedDistribution::train_set, OptionBase::learntoption,
                  "We need to store the training set, as this learner is memory-based...");

    declareOption(ol, "NN", &LocallyMagnifiedDistribution::NN, OptionBase::learntoption,
                  "The nearest neighbor algorithm used to find nearest neighbors");

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

    if(localdistr.isNull())
    {
        GaussianDistribution* distr = new GaussianDistribution();
        distr->ignore_weights_below = 1e-6;
        distr->build();
        localdistr = distr;
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
    trainsample.resize(w+ws);
    Vec input = trainsample.subVec(0,w);

    PLASSERT(targetsize()==0);

    int comp_n = getActualNComputationNeighbors();
    int width_n = getActualNWidthNeighbors();

    if(comp_n>0 || width_n>0)
        NN->computeOutputAndCosts(y, emptyvec, NN_outputs, NN_costs);

    if(kernel_adapt_width_mode!=' ')
    {
        real new_width = 0;
        if(kernel_adapt_width_mode=='M')
        {
            new_width = width_factor*NN_costs[width_n-1];
            // if(display_adapted_width)
            //   perr << "new_width=" << width_factor << " * NN_costs["<<width_n-1<<"] = "<< new_width << endl;
        }
        else if(kernel_adapt_width_mode=='Z')
        {
            new_width = width_factor*sqrt(square(NN_costs[width_n-1])/w);
         }
        else if(kernel_adapt_width_mode=='A')
        {
            for(int k=0; k<width_n; k++)
                new_width += NN_costs[k];
            new_width *= width_factor/width_n;
        }
        else
            PLERROR("Invalid kernel_adapt_width_mode: %c",kernel_adapt_width_mode);

        // hack to display only first adapted width
        if(display_adapted_width)
        {
            /*
            perr << "NN_outputs = " << NN_outputs << endl;
            perr << "NN_costs = " << NN_costs << endl;
            perr << "inutsize = " << w << endl;
            perr << "length = " << l << endl;
            */
            perr << "Adapted kernel width = " << new_width << endl;
            display_adapted_width = false;
        }

        weighting_kernel->setOption(width_optionname,tostring(new_width));
        weighting_kernel->build(); // rebuild to adapt to width change
    }

    double weightsum = 0;

    VMat local_trainset;
    if(comp_n>0) // we'll use only the neighbors
    {
        int n = NN_outputs.length();
        Mat neighbors(n, w+1);
        neighbors.lastColumn().fill(1.0); // default weight 1.0
        for(int k=0; k<n; k++)
        {
            Vec neighbors_k = neighbors(k);
            Vec neighbors_row = neighbors_k.subVec(0,w+ws);
            Vec neighbors_input = neighbors_row.subVec(0,w);
            train_set->getRow(int(NN_outputs[k]),neighbors_row);
            real weight = 1.;
            if(weighting_kernel.isNotNull())
                weight = weighting_kernel(y,neighbors_input);
            weightsum += weight;
            neighbors_k[w] *= weight;
        }
        local_trainset = new MemoryVMatrix(neighbors);
        local_trainset->defineSizes(w,0,1); 
    }
    else // we'll use all the points
    {
        // 'weights' will contain the "localization" weights for the current test point.
        weights.resize(l);
        for(int i=0; i<l; i++)
        {
            train_set->getRow(i,trainsample);
            real weight = 1.;
            if(weighting_kernel.isNotNull())
                weight = weighting_kernel(y,input);
            if(ws==1)
                weight *= trainsample[w];
            weightsum += weight;
            weights[i] = weight;
        }

        VMat weight_column(columnmatrix(weights));
        if(ws==0) // append weight column
            local_trainset = hconcat(train_set, weight_column);
        else // replace last column by weight column
            local_trainset = hconcat(train_set.subMatColumns(0,w), weight_column);
        local_trainset->defineSizes(w,0,1);        
    }


    // perr << "local_trainset =" << endl << local_trainset->toMat() << endl;
    double log_local_p = 0;

    switch(mode)
    {
    case 0:
        log_local_p = trainLocalDistrAndEvaluateLogDensity(local_trainset, y);
        return log_local_p + pl_log((double)weightsum) - pl_log((double)l) - pl_log((double)weighting_kernel(input,input));
    case 1:
        log_local_p = trainLocalDistrAndEvaluateLogDensity(local_trainset, y);
        return log_local_p;
    case 2:
        return pl_log((double)weightsum) - pl_log((double)l);
    case 3:
        return pl_log((double)weightsum);
    case 4:
        log_local_p = trainLocalDistrAndEvaluateLogDensity(local_trainset, y);
        return log_local_p+pl_log((double)width_n)-pl_log((double)l);
    default:
        PLERROR("Invalid mode %d", mode);
        return 0; 
    }
}

double LocallyMagnifiedDistribution::trainLocalDistrAndEvaluateLogDensity(VMat local_trainset, Vec y) const
{
    if(fix_localdistr_center)
    {
        GaussianDistribution* distr = dynamic_cast<GaussianDistribution*>((PDistribution*)localdistr);
        if(distr!=0)
            distr->given_mu = y;
    }
    localdistr->forget();
    localdistr->setTrainingSet(local_trainset);
    localdistr->train();
    double log_local_p = localdistr->log_density(y);
    return log_local_p;
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

int LocallyMagnifiedDistribution::getActualNComputationNeighbors() const
{
    if(computation_neighbors<=0)
        return 0;
    else if(computation_neighbors>1)
        return int(computation_neighbors);
    else
        return int(computation_neighbors*sqrt(train_set->length()));
}

int LocallyMagnifiedDistribution::getActualNWidthNeighbors() const
{
    if(width_neighbors<0)
        return 0;
    else if(width_neighbors>1)
        return int(width_neighbors);
    return int(width_neighbors*sqrt(train_set->length()));
}


// ### Remove this method, if your distribution does not implement it.
///////////
// train //
///////////
void LocallyMagnifiedDistribution::train()
{
    int comp_n = getActualNComputationNeighbors();
    int width_n = getActualNWidthNeighbors();
    int actual_nneighbors = max(comp_n, width_n);

    if(train_set.isNotNull())
        actual_nneighbors = min(actual_nneighbors, train_set.length());

    if(actual_nneighbors>0)
    {
        NN = new ExhaustiveNearestNeighbors(); // for now use Exhaustive search and default Euclidean distance
        NN->num_neighbors = actual_nneighbors;
        NN->copy_input = false;
        NN->copy_target = false;
        NN->copy_weight = false;
        NN->copy_index = true;
        NN->build();
        if(train_set.isNotNull())
        {
            NN->setTrainingSet(train_set);
            NN->train();
        }
        NN_outputs.resize(actual_nneighbors);
        NN_costs.resize(actual_nneighbors);
    }
}


void LocallyMagnifiedDistribution::forget()
{
    if(NN.isNotNull())
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
