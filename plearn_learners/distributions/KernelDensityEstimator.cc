// -*- C++ -*-

// KernelDensityEstimator.cc
//
// Copyright (C) 2008 Dumitru Erhan
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

// Authors: Dumitru Erhan

/*! \file KernelDensityEstimator.cc */


#include "KernelDensityEstimator.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    KernelDensityEstimator,
    "Performs kernel density estimation ('Parzen Windows') with ANY given kernel",
    "Does not take into account the input weights!"
    );

//////////////////
// KernelDensityEstimator //
//////////////////
KernelDensityEstimator::KernelDensityEstimator()
{
}

////////////////////
// declareOptions //
////////////////////
void KernelDensityEstimator::declareOptions(OptionList& ol)
{
    declareOption(ol, "kernel", &KernelDensityEstimator::kernel,
                   OptionBase::buildoption,
                   "The kernel used at each point in the training set");

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void KernelDensityEstimator::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void KernelDensityEstimator::build_()
{
}

/////////
// cdf //
/////////
real KernelDensityEstimator::cdf(const Vec& y) const
{
    PLERROR("cdf not implemented for KernelDensityEstimator"); return 0;
}

/////////////////
// expectation //
/////////////////
void KernelDensityEstimator::expectation(Vec& mu) const
{
    PLERROR("expectation not implemented for KernelDensityEstimator");
}

// ### Remove this method if your distribution does not implement it.
////////////
// forget //
////////////
void KernelDensityEstimator::forget()
{
    inherited::forget();
}

//////////////
// generate //
//////////////
void KernelDensityEstimator::generate(Vec& y) const
{
    PLERROR("generate not implemented for KernelDensityEstimator");
}

// ### Default version of inputsize returns learner->inputsize()
// ### If this is not appropriate, you should uncomment this and define
// ### it properly here:
// int KernelDensityEstimator::inputsize() const {}

/////////////////
// log_density //
/////////////////
real KernelDensityEstimator::log_density(const Vec& y) const
{
    int numTrain = train_set.length();
    Vec input, target;
    real weight;
    real logprob = -INFINITY;

    for(int i=0; i<numTrain; i++) {
        train_set->getExample(i,input,target,weight);
        logprob = logadd(logprob,kernel->evaluate(input,y)); 
    }
    
    logprob -= pl_log(numTrain);

    return logprob;

}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void KernelDensityEstimator::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

////////////////////
// resetGenerator //
////////////////////
void KernelDensityEstimator::resetGenerator(long g_seed)
{
    inherited::resetGenerator(g_seed);
}

/////////////////
// survival_fn //
/////////////////
real KernelDensityEstimator::survival_fn(const Vec& y) const
{
    PLERROR("survival_fn not implemented for KernelDensityEstimator"); return 0;
}

// ### Remove this method, if your distribution does not implement it.
///////////
// train //
///////////
void KernelDensityEstimator::train()
{
    // PLERROR("train method not implemented for KernelDensityEstimator");
}

//////////////
// variance //
//////////////
void KernelDensityEstimator::variance(Mat& covar) const
{
    PLERROR("variance not implemented for KernelDensityEstimator");
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
