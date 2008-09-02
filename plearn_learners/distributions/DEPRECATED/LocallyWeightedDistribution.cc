// -*- C++ -*-

// Distribution.cc
// 
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

/* *******************************************************      
 * $Id$ 
 ******************************************************* */

/*! \file PLearn/plearn_learners/distributions/DEPRECATED/LocallyWeightedDistribution.cc */

#include "LocallyWeightedDistribution.h"
#include <plearn/vmat/ConcatColumnsVMatrix.h>

namespace PLearn {
using namespace std;

LocallyWeightedDistribution::LocallyWeightedDistribution() 
{}


PLEARN_IMPLEMENT_OBJECT(LocallyWeightedDistribution, "ONE LINE DESCR", "NO HELP");

void LocallyWeightedDistribution::declareOptions(OptionList& ol)
{
    declareOption(ol, "weighting_kernel", &LocallyWeightedDistribution::weighting_kernel, OptionBase::buildoption,
                  "The kernel that will be used to locally weigh the samples");

    declareOption(ol, "localdistr", &LocallyWeightedDistribution::localdistr, OptionBase::buildoption,
                  "The distribution that will be trianed with local weights");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void LocallyWeightedDistribution::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.

    if(weightsize()!=0 && weightsize()!=1)
        PLERROR("In LocallyWeightedDistribution::build_, weightsize must be 0 or 1");

    localdistr->inputsize_ = inputsize_;
    localdistr->weightsize_ = 1;
    localdistr->build();
}

// ### Nothing to add here, simply calls build_
void LocallyWeightedDistribution::build()
{
    inherited::build();
    build_();
}


void LocallyWeightedDistribution::train(VMat training_set)
{ 
    if(training_set.width() != inputsize()+weightsize())
        PLERROR("In LocallyWeightedDistribution::train width of training set is different from inputsize()+weightsize()");
    setTrainingSet(training_set);
}


void LocallyWeightedDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Distribution::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("LocallyWeightedDistribution::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


double LocallyWeightedDistribution::log_density(const Vec& x) const
{
    int l = train_set.length();
    int w = inputsize();
    weights.resize(l);
    // 'weights' will contain the "localization" weights for the current test point.
    trainsample.resize(w+weightsize());
    Vec input = trainsample.subVec(0,w);

    for(int i=0; i<l; i++)
    {
        train_set->getRow(i,trainsample);
        real weight = weighting_kernel(x,input);
        if(weightsize()==1)
            weight *= trainsample[w];
        weights[i] = weight;
    }
  
    VMat weight_column(columnmatrix(weights));

    VMat weighted_trainset;
    if(weightsize()==0) // append weight column    
        weighted_trainset = hconcat(train_set, weight_column);
    else // replace last column by weight column
        weighted_trainset = hconcat(train_set.subMatColumns(0,inputsize()), weight_column);

    localdistr->forget();
    localdistr->train(weighted_trainset);
    return localdistr->log_density(x);
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
