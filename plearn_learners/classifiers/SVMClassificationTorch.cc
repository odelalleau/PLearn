// -*- C++ -*-

// SVMClassificationTorch.cc
//
// Copyright (C) 2005 Olivier Delalleau 
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
   * $Id: SVMClassificationTorch.cc,v 1.5 2005/02/24 14:27:30 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file SVMClassificationTorch.cc */


#include "SVMClassificationTorch.h"

namespace PLearn {
using namespace std;

////////////////////////////
// SVMClassificationTorch //
////////////////////////////
SVMClassificationTorch::SVMClassificationTorch() 
: output_the_class(true)
{}

PLEARN_IMPLEMENT_OBJECT(SVMClassificationTorch,
    "SVM classification using the Torch library",
    "Do not do anything that needs this object to be deep-copied, because it\n"
    "is not possible yet."
);

////////////////////
// declareOptions //
////////////////////
void SVMClassificationTorch::declareOptions(OptionList& ol)
{
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // Build options.

  declareOption(ol, "output_the_class", &SVMClassificationTorch::output_the_class, OptionBase::buildoption,
      "If set to 1, the output will be the class, otherwise it will be a real value.");

  // Learnt options.

  // declareOption(ol, "myoption", &SVMClassificationTorch::myoption, OptionBase::learntoption,
  //               "Help text describing this option");

  // Now call the parent class' declareOptions.
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void SVMClassificationTorch::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void SVMClassificationTorch::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void SVMClassificationTorch::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
  // No cost computed.
}                                

///////////////////
// computeOutput //
///////////////////
void SVMClassificationTorch::computeOutput(const Vec& input, Vec& output) const
{
  inherited::computeOutput(input, output);
  if (output_the_class)
    for (int i = 0; i < output.length(); i++)
      output[i] = output[i] > 0 ? 1 : -1;
}    

#if 0
////////////
// forget //
////////////
void SVMClassificationTorch::forget()
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
#endif
    
//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> SVMClassificationTorch::getTestCostNames() const
{
  return inherited::getTestCostNames();
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> SVMClassificationTorch::getTrainCostNames() const
{
  return inherited::getTrainCostNames();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SVMClassificationTorch::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("SVMClassificationTorch::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

#if 0
////////////////
// outputsize //
////////////////
int SVMClassificationTorch::outputsize() const
{
  // Compute and return the size of this learner's output, (which typically
  // may depend on its inputsize(), targetsize() and set options).
}

///////////
// train //
///////////
void SVMClassificationTorch::train()
{
    // The role of the train method is to bring the learner up to stage==nstages,
    // updating train_stats with training costs measured on-line in the process.

    /* TYPICAL CODE:

      static Vec input  // static so we don't reallocate/deallocate memory each time...
      static Vec target // (but be careful that static means shared!)
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

  if (stage >= nstages) {
    PLWARNING("In SVMClassificationTorch::train - Learner has already been trained, skipping training");
    return;
  }
}
#endif

} // end of namespace PLearn
