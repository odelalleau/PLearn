// -*- C++ -*-

// RankingFromKernel.cc
//
// Copyright (C) 2006 Pierre-Jean L Heureux 
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Pierre-Jean L Heureux

/*! \file RankingFromKernel.cc */


#include "RankingFromKernel.h"
#include <plearn/io/PPath.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RankingFromKernel,
    "This learner will compute \\frac{\\sum_{actives} K(i,j)}{\\sum_{inactives} K(i,j)} for a given kernel K. ",
    "A lift_output is available to compute ranking based costs. The target must be 1 or 0. \n");

RankingFromKernel::RankingFromKernel() 
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

void RankingFromKernel::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    declareOption(ol, "logKernel", &RankingFromKernel::logKernel, OptionBase::buildoption,
                   "A kernel taking an input and returning the log of its result.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RankingFromKernel::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

// ### Nothing to add here, simply calls build_
void RankingFromKernel::build()
{
    inherited::build();
    build_();
}


void RankingFromKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("RankingFromKernel::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int RankingFromKernel::outputsize() const
{
    // Compute and return the size of this learner's output (which typically
    // may depend on its inputsize(), targetsize() and set options).
    return 1;
}

void RankingFromKernel::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    /*!
      A typical forget() method should do the following:
      - initialize a random number generator with the seed option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
}
    
void RankingFromKernel::train()
{
    // The role of the train method is to bring the learner up to stage==nstages,
    // updating train_stats with training costs measured on-line in the process.

    /* TYPICAL CODE:

    static Vec input  // static so we don't reallocate/deallocate memory each time...
    static Vec target // (but be careful that static means shared!)
    input.resize(inputsize())    // the train_set's inputsize()
    target.resize(targetsize())  // the train_set's targetsize()
    real weight

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    while(stage<nstages)
    {
    // clear statistics of previous epoch
    train_stats->forget() 
          
    //... train for 1 stage, and update train_stats,
    // using train_set->getSample(input, target, weight)
    // and train_stats->update(train_costs)
          
    ++stage
    train_stats->finalize() // finalize statistics for this epoch
    }
    */
    if (train_set->targetsize() != 1) PLERROR("This PLearner is not built for multi-target problems");
    PLWARNING("Train not implemented");
}


void RankingFromKernel::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input.
    // int nout = outputsize();
    // output.resize(nout);
    // ...
    int i;
    real log_k,log_result, weight;

    log_act.resize(0);
    log_inact.resize(0);
    for (i=0; i < train_set->length();i++){
        train_set->getExample(i,x, target, weight);
        log_k = logKernel->evaluate(input,x);
        if ( fast_exact_is_equal(target[0],1)) {
            log_act.append(log_k);
        }else{
            log_inact.append(log_k);
        }
    }
    log_result = logadd(log_act) - logadd(log_inact);
    output.resize(1);
    output[0] = exp(log_result);
}

void RankingFromKernel::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
    costs.resize(nTestCosts());
    if(fast_exact_is_equal(target[0],1))
        costs[0] = output[0];
    else
        costs[0] = -output[0];
}                                

TVec<string> RankingFromKernel::getTestCostNames() const
{
    static TVec<string> cost;
    if (cost.isEmpty())
        cost.append("lift_output");
    return cost;
}

TVec<string> RankingFromKernel::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes and 
    // for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by getTestCostNames).
    // ...

    static TVec<string> costs;
    costs.resize(0);
    return costs;
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
