// -*- C++ -*-

// KernelProjection.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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
   * $Id: KernelProjection.cc,v 1.1 2004/04/05 14:30:34 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file KernelProjection.cc */

#include "GaussianKernel.h"   //!< For the default kernel.
#include "KernelProjection.h"

namespace PLearn {
using namespace std;

//////////////////////
// KernelProjection //
//////////////////////
KernelProjection::KernelProjection() 
: n_comp(1)
{
  kernel = new GaussianKernel();
}

PLEARN_IMPLEMENT_OBJECT(KernelProjection,
    "Implements dimensionality reduction by using eigenfunctions of a kernel.", 
    ""
);

////////////////////
// declareOptions //
////////////////////
void KernelProjection::declareOptions(OptionList& ol)
{
  declareOption(ol, "kernel", &KernelProjection::kernel, OptionBase::buildoption,
      "The kernel used to compute the Gram matrix.");

  declareOption(ol, "n_comp", &KernelProjection::n_comp, OptionBase::buildoption,
      "Number of components computed.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void KernelProjection::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void KernelProjection::build_()
{
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void KernelProjection::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("KernelProjection::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


////////////////
// outputsize //
////////////////
int KernelProjection::outputsize() const
{
  return n_comp;
}

////////////
// forget //
////////////
void KernelProjection::forget()
{
  //! (Re-)initialize the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    /*!
      A typical forget() method should do the following:
         - initialize a random number generator with the seed option
         - initialize the learner's parameters, using this random generator
         - stage = 0
    */
  stage = 0;
}
    
///////////
// train //
///////////
void KernelProjection::train()
{
    // The role of the train method is to bring the learner up to stage==nstages,
    // updating train_stats with training costs measured on-line in the process.

    /* TYPICAL CODE:

      static Vec input  // static so we don't reallocate/deallocate memory each time...
      static Vec target
      input.resize(inputsize())    // the train_set's inputsize()
      target.resize(targetsize())  // the train_set's targetsize()
      real weight

      if(!train_stats)  // make a default stats collector, in case there's none
         train_stats = new VecStatsCollector()

      if(nstages<stage) // asking to revert to a previous stage!
         forget()  // reset the learner to stage=0

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
  if (stage == 0) {
    PLWARNING("In KernelProjection::train - Learner has already been trained");
    return;
  }
  stage = 1;
}


///////////////////
// computeOutput //
///////////////////
void KernelProjection::computeOutput(const Vec& input, Vec& output) const
{
  // Compute the output from the input
  // int nout = outputsize();
  // output.resize(nout);
  // ...
}    

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void KernelProjection::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output. 
// ...
}                                

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> KernelProjection::getTestCostNames() const
{
  // Return the names of the costs computed by computeCostsFromOutpus
  // (these may or may not be exactly the same as what's returned by getTrainCostNames)
  // ...
  TVec<string> t;
  return t;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> KernelProjection::getTrainCostNames() const
{
  // Return the names of the objective costs that the train method computes and 
  // for which it updates the VecStatsCollector train_stats
  // (these may or may not be exactly the same as what's returned by getTestCostNames)
  // ...
  TVec<string> t;
  return t;
}

} // end of namespace PLearn
