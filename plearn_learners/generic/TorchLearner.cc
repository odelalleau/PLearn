// -*- C++ -*-

// TorchLearner.cc
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
 * $Id$ 
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file TorchLearner.cc */


#include "TorchLearner.h"
#include <plearn_torch/TMachine.h>
#include <plearn_torch/TTorchDataSetFromVMat.h>
#include <plearn_torch/TTrainer.h>

namespace PLearn {
using namespace std;

//////////////////
// TorchLearner //
//////////////////
TorchLearner::TorchLearner() 
    : outputsize_(-1)
{
    allocator = new Torch::Allocator;
    inputs = 0;
}

PLEARN_IMPLEMENT_OBJECT(TorchLearner,
                        "A generic learner that can use Torch learning algorithms.",
                        ""
    );

////////////////////
// declareOptions //
////////////////////
void TorchLearner::declareOptions(OptionList& ol)
{
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // Build options.

    declareOption(ol, "machine", &TorchLearner::machine, OptionBase::buildoption,
                  "The Torch learning machine.");

    declareOption(ol, "trainer", &TorchLearner::trainer, OptionBase::buildoption,
                  "The Torch trainer, responsible for training the machine.");

    // Learnt options.

    declareOption(ol, "outputsize", &TorchLearner::outputsize_, OptionBase::learntoption,
                  "Saves the output size of this learner for faster access.");

    // Now call the parent class' declareOptions.
    inherited::declareOptions(ol);

    // Hide unused parent's options.

    redeclareOption(ol, "seed", &TorchLearner::seed_, OptionBase::nosave,
                    "Torch learners in general will not use the PLearn seed.");

    redeclareOption(ol, "nstages", &TorchLearner::nstages, OptionBase::nosave,
                    "A Torch learner is usually only trained on one stage.");

}

///////////
// build //
///////////
void TorchLearner::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void TorchLearner::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
    if (machine && machine->machine->outputs)
        outputsize_ = machine->machine->outputs->frame_size;
    // Initialize the inputs sequence.
    if (inputsize_ >= 0 && (!inputs || inputs->frame_size != inputsize_)) {
        allocator->free(inputs); // Free old input sequence.
        inputs = new(allocator) Torch::Sequence(1, inputsize_);
    }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void TorchLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
    // No cost computed for now.
}                                

///////////////////
// computeOutput //
///////////////////
void TorchLearner::computeOutput(const Vec& input, Vec& output) const
{
    PLASSERT( outputsize_ >= 0);
    output.resize(outputsize_);
    inputs->copyFrom(input.data());
    machine->forward(inputs);
    machine->machine->outputs->copyTo(output.data());
}    

////////////
// forget //
////////////
void TorchLearner::forget()
{
    stage = 0;
    outputsize_ = -1;
    if (machine)
        machine->reset();
}
    
//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> TorchLearner::getTestCostNames() const
{
    // No cost computed for now.
    return TVec<string>();
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> TorchLearner::getTrainCostNames() const
{
    // No cost computed for now.
    return TVec<string>();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void TorchLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("TorchLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////
// outputsize //
////////////////
int TorchLearner::outputsize() const
{
    // Compute and return the size of this learner's output, (which typically
    // may depend on its inputsize(), targetsize() and set options).
    PLASSERT( machine );
    PLASSERT( outputsize_ >= 0 || machine->machine->outputs );
    if (outputsize_ >=0)
        return outputsize_;
    return machine->machine->outputs->frame_size;
}

////////////////////
// setTrainingSet //
////////////////////
void TorchLearner::setTrainingSet(VMat training_set, bool call_forget) {
    inherited::setTrainingSet(training_set, call_forget);
    torch_train_set = new TTorchDataSetFromVMat(training_set);
    allocator->free(inputs); // Free old input sequence.
    inputs = new(allocator) Torch::Sequence(1, training_set->inputsize());
}

///////////
// train //
///////////
void TorchLearner::train()
{
    if (stage >= nstages) {
        PLWARNING("In TorchLearner::train - Learner has already been trained, skipping training");
        return;
    }
    if (!trainer || !machine)
        PLERROR("In TorchLearner::train - You must set both the 'trainer' and 'machine' options "
                "before calling train()");
    trainer->train((TTorchDataSetFromVMat*) torch_train_set);
    if (machine->machine->outputs)
        // Update outputsize_
        outputsize_ = machine->machine->outputs->frame_size;
    stage = 1;
}

///////////////////
// ~TorchLearner //
///////////////////
TorchLearner::~TorchLearner() {
    delete allocator;
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
