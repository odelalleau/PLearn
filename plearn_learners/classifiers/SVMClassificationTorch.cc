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
 * $Id$ 
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file SVMClassificationTorch.cc */


#include "SVMClassificationTorch.h"
#include <plearn_torch/TTorchDataSetFromVMat.h>
#include <plearn_torch/TTorchKernelFromKernel.h>
#include <plearn_torch/TMachine.h>
#include <plearn_torch/TSVMClassification.h>
#include <plearn_torch/TQCTrainer.h>
#include <plearn_torch/TTrainer.h>

#include <plearn/vmat/ProcessingVMatrix.h>

namespace PLearn {
using namespace std;

////////////////////////////
// SVMClassificationTorch //
////////////////////////////
SVMClassificationTorch::SVMClassificationTorch() 
    : C(100),
      cache_size(50),
      iter_msg(1000),
      output_the_class(true),
      target_01(false)
{}

PLEARN_IMPLEMENT_OBJECT(SVMClassificationTorch,
    "SVM classification using the Torch library",
    "Do not do anything that needs this object to be deep-copied, because it\n"
    "is not possible yet.\n"
    "Only binary classification is currently supported. By default, the\n"
    "target should be -1 or 1. You can use 0 and 1 by setting the option\n"
    "'target_01' to 1.\n"
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

    declareOption(ol, "kernel", &SVMClassificationTorch::kernel, OptionBase::buildoption,
                  "The kernel we use.");

    declareOption(ol, "C", &SVMClassificationTorch::C, OptionBase::buildoption,
                  "Trade-off margin / error.");

    declareOption(ol, "output_the_class", &SVMClassificationTorch::output_the_class, OptionBase::buildoption,
                  "If set to 1, the output will be the class, otherwise it will be a real value.");

    declareOption(ol, "target_01", &SVMClassificationTorch::target_01,
                                   OptionBase::buildoption,
        "If set to 1, the target in the training set will be assumed to be\n"
        "either 0 or 1 (instead of the default -1 / 1).");

    declareOption(ol, "iter_msg", &SVMClassificationTorch::iter_msg, OptionBase::buildoption,
                  "Number of iterations between each message.");

    declareOption(ol, "cache_size", &SVMClassificationTorch::cache_size, OptionBase::buildoption,
                  "Cache size (in Mb).");

    // Learnt options.

    // declareOption(ol, "myoption", &SVMClassificationTorch::myoption, OptionBase::learntoption,
    //               "Help text describing this option");

    // Now call the parent class' declareOptions.
    inherited::declareOptions(ol);

    // Redeclare some parent's options.
    redeclareOption(ol, "machine", &SVMClassificationTorch::machine, OptionBase::learntoption,
                    "Constructed at build time and saved to store learnt parameters.");

    redeclareOption(ol, "trainer", &SVMClassificationTorch::trainer, OptionBase::nosave,
                    "Constructed at build time (there is no need to save it).");

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
    // Build machine.
    if (!machine)
        machine = new TSVMClassification();
    PP<TSVMClassification> svm_class = (TSVMClassification*) (TMachine*) machine;
    svm_class->C = this->C;
    svm_class->cache_size = this->cache_size;
    svm_class->kernel = new TTorchKernelFromKernel(this->kernel);
    svm_class->build();
    // Build trainer.
    if (!trainer)
        trainer = new TQCTrainer();
    PP<TQCTrainer> qc_trainer = (TQCTrainer*) (TTrainer*) this->trainer;
    qc_trainer->qc_machine = (TQCMachine*) (TMachine*) this->machine;
    qc_trainer->iter_msg = this->iter_msg;
    qc_trainer->build();
    // We can now build the TorchLearner.
    inherited::build();
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void SVMClassificationTorch::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                     const Vec& target, Vec& costs) const
{
    // No cost computed.
    // For safety, we check we are trying to do binary classification with -1
    // and 1, or 0 and 1 when the 'target_01' option is set.
    PLASSERT( target.length() == 1 &&
            ((target_01  && (target[0] == 1 || target[0] == 0)) ||
             (!target_01 && (target[0] == 1 || target[0] == -1))) );
    PLASSERT( output.length() == 1 );
    costs.resize(1);
    real sig_output = sigmoid(output[0]);
    if (fast_exact_is_equal(sig_output, 0))
        sig_output = REAL_EPSILON;
    if (fast_exact_is_equal(target[0], 1))
        costs[0] = sig_output;
    else
        costs[0] = - sig_output;
}                                

///////////////////
// computeOutput //
///////////////////
void SVMClassificationTorch::computeOutput(const Vec& input, Vec& output) const
{
    inherited::computeOutput(input, output);
    if (output_the_class)
        for (int i = 0; i < output.length(); i++)
            output[i] = output[i] > 0 ? 1 : target_01 ? 0 : -1;
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
    static TVec<string> costs;
    if (costs.isEmpty())
        costs.append("lift_output");
    return costs;
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

////////////////////
// setTrainingSet //
////////////////////
void SVMClassificationTorch::setTrainingSet(VMat training_set, bool call_forget) {
    VMat the_train_set = training_set;
    if (target_01) {
        // Create processing program.
        int target_col = training_set->inputsize();
        PLASSERT( target_col > 0 );
        string prog = "[%0:%" + tostring(target_col - 1) + "] %" +
            tostring(target_col) + " 1 == 1 -1 ifelse :target";
        if (training_set->weightsize() > 0) {
            int weight_col = training_set->inputsize() +
                             training_set->targetsize();
            prog += " [%" + tostring(weight_col) + ":%" +
                tostring(weight_col + training_set->weightsize() - 1) + "]";
        }
        the_train_set = new ProcessingVMatrix(the_train_set, prog);
    }
    kernel->setDataForKernelMatrix(the_train_set);
    inherited::setTrainingSet(the_train_set, call_forget);
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
