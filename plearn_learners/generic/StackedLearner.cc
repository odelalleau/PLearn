// -*- C++ -*-

// StackedLearner.cc
//
// Copyright (C) 2003 Yoshua Bengio 
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

// Authors: Yoshua Bengio

/*! \file StackedLearner.cc */


#include "StackedLearner.h"
#include <plearn/vmat/PLearnerOutputVMatrix.h>
#include <plearn/vmat/ShiftAndRescaleVMatrix.h>
#include <plearn/base/stringutils.h>
#include <plearn/vmat/SeparateInputVMatrix.h>

namespace PLearn {
using namespace std;

StackedLearner::StackedLearner() 
    : default_operation("mean"),
      base_train_splitter(0),
      train_base_learners(true),
      normalize_base_learners_output(false),
      precompute_base_learners_output(false),
      put_raw_input(false),
      share_learner(false),
      nsep(1)
{ }

PLEARN_IMPLEMENT_OBJECT(
    StackedLearner, 
    "Implements stacking, that combines two levels of learner, the 2nd level using the 1st outputs as inputs",
    "NOTE: If you need a simple mechanism for chaining multiple learners, consider using ChainedLearners instead.\n"
    "Stacking is a generic strategy in which two levels (or more, recursively) of learners\n"
    "are combined. The lower level may have one or more learners, and they may be trained\n"
    "on the same or different data from the upper level single learner. A shared learner can\n"
    "also be trained on different parts of the input. The outputs of the\n"
    "1st level learners are concatenated and serve as inputs to the second level learner.\n"
    "\n"
    "Contrarily to previous versions, it is now PERMITTED for each learner to\n"
    "have a different outputsize() if an explicit combiner is in use.  We assume\n"
    "that the combiner knows what to do."
    "\n"
    "There is also the option to copy the input of the 1st level learner as additional\n"
    "inputs for the second level (put_raw_input).  If requested, the raw_inputs are\n"
    "appended AT THE END of the combiner input vector.\n"
    "\n"
    "A Splitter can optionally be provided to specify how to split the data into\n"
    "the training /validation sets for the lower and upper levels respectively\n"
    );

void StackedLearner::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "base_learners", &StackedLearner::base_learners, OptionBase::buildoption,
                  "A set of 1st level base learners that are independently trained (here or elsewhere)\n"
                  "and whose outputs will serve as inputs to the combiner (2nd level learner)");

    declareOption(ol, "combiner", &StackedLearner::combiner, OptionBase::buildoption,
                  "A learner that is trained (possibly on a data set different from the\n"
                  "one used to train the base_learners) using the outputs of the\n"
                  "base_learners as inputs. If it is not provided, then the StackedLearner\n"
                  "simply performs \"default_operation\" on the outputs of the base_learners\n");

    declareOption(ol, "default_operation", &StackedLearner::default_operation,
                  OptionBase::buildoption,
                  "If no combiner is provided, simple operation to be performed\n"
                  "on the outputs of the base_learners.\n"
                  "Supported: mean (default), min, max, variance, sum, sumofsquares, dmode (majority vote)\n");

    declareOption(ol, "splitter", &StackedLearner::splitter, OptionBase::buildoption,
                  "A Splitter used to select which data subset(s) goes to training the base_learners\n"
                  "and which data subset(s) goes to training the combiner. If not provided then the\n"
                  "same data is used to train and test both levels. If provided, in each split, there should be\n"
                  "two sets: the set on which to train the first level and the set on which to train the second one\n");

    declareOption(ol, "base_train_splitter", &StackedLearner::base_train_splitter, OptionBase::buildoption,
                  "This splitter can be used to split the training set into different training sets for each base learner\n"
                  "If it is not set, the same training set will be applied to the base learners.\n"
                  "If \"splitter\" is also used, it will be applied first to determine the training set used by base_train_splitter.\n"
                  "The splitter should give as many splits as base learners, and each split should contain one set.");

    declareOption(ol, "train_base_learners", &StackedLearner::train_base_learners, OptionBase::buildoption,
                  "whether to train the base learners in the method train (otherwise they should be\n"
                  "initialized properly at construction / setOption time)\n");

    declareOption(ol, "normalize_base_learners_output", &StackedLearner::normalize_base_learners_output, OptionBase::buildoption,
                  "If set to 1, the output of the base learners on the combiner training set\n"
                  "will be normalized (zero mean, unit variance) before training the combiner.");
                
    declareOption(ol, "precompute_base_learners_output", &StackedLearner::precompute_base_learners_output, OptionBase::buildoption,
                  "If set to 1, the output of the base learners on the combiner training set\n"
                  "will be precomputed in memory before training the combiner (this may speed\n"
                  "up significantly the combiner training process).");

  
    declareOption(ol, "put_raw_input", &StackedLearner::put_raw_input, OptionBase::buildoption,
                  "Whether to put the raw inputs in addition of the base learners\n"
                  "outputs, in input of the combiner.  The raw_inputs are\n"
                  "appended AT THE END of the combiner input vector\n");

    declareOption(ol, "share_learner", &StackedLearner::share_learner, OptionBase::buildoption,
                  "If set to 1, the input is divided in nsep equal parts, and a common learner\n"
                  "is trained as if each part constitutes a training example.");

    declareOption(ol, "nsep", &StackedLearner::nsep, OptionBase::buildoption,
                  "Number of input separations. The input size needs to be a "
                  "multiple of that value\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void StackedLearner::setTrainStatsCollector(PP<VecStatsCollector> statscol)
{
    train_stats = statscol;
    if (combiner)
        combiner->setTrainStatsCollector(statscol);
}

void StackedLearner::build_()
{
    /**
     *  NOTE BY NICOLAS: according to Pascal, it's not kosher to call build
     *  inside build for no reason, since the inner objects HAVE ALREADY been
     *  built.  You should uncomment the following code only FOR GOOD REASON.
     *
     *   for (int i=0;i<base_learners.length();i++)
     *   {
     *     if (!base_learners[i])
     *       PLERROR("StackedLearner::build: base learners have not been created!");
     *     base_learners[i]->build();
     *     if (i>0 && base_learners[i]->outputsize()!=base_learners[i-1]->outputsize())
     *       PLERROR("StackedLearner: expecting base learners to have the same number of outputs!");
     *   }
     *   if (combiner)
     *     combiner->build();
     *   if (splitter)
     *     splitter->build();
     */
  
    if (base_learners.size() == 0)
        PLERROR("StackedLearner::build_: no base learners specified!  Use the "
                "'base_learners' option");
  
    if (splitter && splitter->nSetsPerSplit()!=2)
        PLERROR("StackedLearner: the Splitter should produce only two sets per split, got %d",
                splitter->nSetsPerSplit());
    if(share_learner && base_train_splitter)
        PLERROR("StackedLearner::build_: options 'base_train_splitter' and 'share_learner'\n"
                "cannot both be true");

    resizeBaseLearnersOutputs();
    default_operation = lowerstring( default_operation );
}

// ### Nothing to add here, simply calls build_
void StackedLearner::build()
{
    inherited::build();
    build_();
}


void StackedLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(base_learners_outputs,     copies);
    deepCopyField(all_base_learners_outputs, copies);
    deepCopyField(base_learners,             copies);
    deepCopyField(combiner,                  copies);
    deepCopyField(splitter,                  copies);
    deepCopyField(base_train_splitter,       copies);

    inherited::makeDeepCopyFromShallowCopy(copies);
}


void StackedLearner::setExperimentDirectory(const PPath& the_expdir)
{
    if (the_expdir != "") {
        for (int i=0, n=base_learners.size() ; i<n ; ++i)
            base_learners[i]->setExperimentDirectory(the_expdir /
                                                     "Base"+tostring(i));
        if (combiner)
            combiner->setExperimentDirectory(the_expdir / "Combiner");
    }
}


int StackedLearner::outputsize() const
{
    // compute and return the size of this learner's output, (which typically
    // may depend on its inputsize(), targetsize() and set options)
    if (combiner)
        return combiner->outputsize();
    else
        return base_learners[0]->outputsize();
}

void StackedLearner::forget()
{
    if (train_base_learners)
        for (int i=0;i<base_learners.length();i++)
            base_learners[i]->forget();
    if (combiner)
        combiner->forget();
}

void StackedLearner::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set, call_forget);

    if (splitter) {
        splitter->setDataSet(training_set);
        if (splitter->nsplits() !=1 )
            PLERROR("In StackedLearner::setTrainingSet - "
                    "The splitter provided should only return one split");

        // Split[0] goes to the base learners; Split[1] goes to combiner
        TVec<VMat> sets = splitter->getSplit();
        setBaseLearnersTrainingSet(sets[0], call_forget);
        setCombinerTrainingSet    (sets[1], call_forget);
    }
    else {
        setBaseLearnersTrainingSet(training_set, call_forget);
        setCombinerTrainingSet    (training_set, call_forget);
    }
  
    // Changing the training set may change the outputsize of the base learners.
    resizeBaseLearnersOutputs();
}

void StackedLearner::train()
{
    if (!train_stats)
        PLERROR("StackedLearner::train: train_stats has not been set!");

    if (splitter && splitter->nsplits() != 1)
        PLERROR("StackedLearner: multi-splits case not implemented yet");

    // --- PART 1: TRAIN THE BASE LEARNERS ---
    if (train_base_learners) {
        if(stage == 0) {
            for (int i=0;i<base_learners.length();i++)
            {
                PP<VecStatsCollector> stats = new VecStatsCollector();
                base_learners[i]->setTrainStatsCollector(stats);
                base_learners[i]->nstages = nstages;
                base_learners[i]->train();
                stats->finalize(); // WE COULD OPTIONALLY SAVE THEM AS WELL!
            }
            stage++;
        }
        else
            for (int i=0;i<base_learners.length();i++)
            {
                base_learners[i]->nstages = nstages;
                base_learners[i]->train();
            }
    }

    // --- PART 2: TRAIN THE COMBINER ---
    if (combiner)
    {
        if (normalize_base_learners_output) {
            // Normalize the combiner training set.
            VMat normalized_trainset = 
                new ShiftAndRescaleVMatrix(combiner->getTrainingSet(), -1);
            combiner->setTrainingSet(normalized_trainset);
        }
        if (precompute_base_learners_output) {
            // First precompute the train set of the combiner in memory.
            VMat precomputed_trainset = combiner->getTrainingSet();
            precomputed_trainset.precompute();
            combiner->setTrainingSet(precomputed_trainset, false);
        }
        combiner->setTrainStatsCollector(train_stats);
        combiner->train();
    }
}


void StackedLearner::computeOutput(const Vec& input, Vec& output) const
{
    all_base_learners_outputs.resize(0);
    if(share_learner) {
        for (int i=0;i<nsep;i++)  {
            if (!base_learners[0])
                PLERROR("StackedLearner::computeOutput: base learners have not been created!");
            base_learners_outputs[i].resize(base_learners[0]->outputsize());
            base_learners[0]->computeOutput(input.subVec(i*input.length() / nsep,
                                                         input.length() / nsep),
                                            base_learners_outputs[i]);

            // append() will be costly only the first time computeOutputAndCosts
            // is called; afterwards storage will NOT be reallocated
            all_base_learners_outputs.append(base_learners_outputs[i]);
        }
    }
    else {
        for (int i=0;i<base_learners.length();i++) {
            if (!base_learners[i])
                PLERROR("StackedLearner::computeOutput: base learners have not been created!");
            base_learners_outputs[i].resize(base_learners[i]->outputsize());
            base_learners[i]->computeOutput(input, base_learners_outputs[i]);

            // append() will be costly only the first time computeOutputAndCosts
            // is called; afterwards storage will NOT be reallocated
            all_base_learners_outputs.append(base_learners_outputs[i]);
        }
    }

    if (put_raw_input)
        all_base_learners_outputs.append(input);
  
    if (combiner)
        combiner->computeOutput(all_base_learners_outputs, output);
  
    else // just performs default_operation on the outputs
    {
        // This is a bit inconvenient... Make it a temporary matrix
        // If it's often needed, i'll optimize it further  --Nicolas
        PLASSERT( base_learners_outputs.size() > 0 );
        Mat base_outputs_mat(base_learners_outputs.size(),
                             base_learners[0]->outputsize());
        for (int i=0, n=base_learners_outputs.size() ; i<n ; ++i)
            base_outputs_mat(i) << base_learners_outputs[i];
    
        if( default_operation == "mean" )
            columnMean(base_outputs_mat, output);
        else if( default_operation == "min" )
            columnMin(base_outputs_mat, output);
        else if( default_operation == "max" )
            columnMax(base_outputs_mat, output);
        else if( default_operation == "sum" )
            columnSum(base_outputs_mat, output);
        else if( default_operation == "sumofsquares" )
            columnSumOfSquares(base_outputs_mat, output);
        else if( default_operation == "variance" )
        {
            Vec mean;
            columnMean(base_outputs_mat, mean);
            columnVariance(base_outputs_mat, output, mean);
        }
        else if( default_operation == "dmode")
        {
            // NC: should this vvvvvvvvvvvv be base_learners_outputs.length() for sharing?
            StatsCollector sc(base_learners.length());
            for(int o=0; o<output.length(); o++)
            {
                sc.forget();
                for(int j=0; j<base_outputs_mat.length(); j++)         
                    sc.update(base_outputs_mat(o,j),1);
                output[o] = sc.dmode();
            }
        }
        else
            PLERROR("StackedLearner::computeOutput: unsupported default_operation");
    }
}

void StackedLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                             const Vec& target, Vec& costs) const
{
    if (combiner)
        combiner->computeCostsFromOutputs(all_base_learners_outputs,
                                          output,target,costs);
    else // cheat
    {
        if(share_learner)
            base_learners[0]->computeCostsFromOutputs(input.subVec(0,input.length() / nsep),
                                                      output, target, costs);
        else
            base_learners[0]->computeCostsFromOutputs(input,output,target,costs);
    }
}

bool StackedLearner::computeConfidenceFromOutput(const Vec& input, const Vec& output,
                                                 real probability,
                                                 TVec< pair<real,real> >& intervals) const
{
    if (! combiner)
        PLERROR("StackedLearner::computeConfidenceFromOutput: a 'combiner' must be specified "
                "in order to compute confidence intervals.");
 
    all_base_learners_outputs.resize(0);
    if(share_learner)
    {
        for (int i=0;i<nsep;i++)
        {
            if (!base_learners[0])
                PLERROR("StackedLearner::computeOutput: base learners have not been created!");
            base_learners_outputs[0].resize(base_learners[0]->outputsize());
            base_learners[0]->computeOutput(input.subVec(i*input.length()/nsep,
                                                         input.length()/nsep),
                                            base_learners_outputs[i]);

            all_base_learners_outputs.append(base_learners_outputs[i]);
        }
    }
    else
    {
        for (int i=0;i<base_learners.length();i++)
        {
            if (!base_learners[i])
                PLERROR("StackedLearner::computeOutput: base learners have not been created!");
            base_learners_outputs[i].resize(base_learners[i]->outputsize());
            base_learners[i]->computeOutput(input, base_learners_outputs[i]);

            all_base_learners_outputs.append(base_learners_outputs[i]);
        }
    }
  
    if (put_raw_input)
        all_base_learners_outputs.append(input);
  
    return combiner->computeConfidenceFromOutput(all_base_learners_outputs,
                                                 output, probability, intervals);
}

TVec<string> StackedLearner::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutpus
    // (these may or may not be exactly the same as what's returned by getTrainCostNames)
    if (combiner)
        return combiner->getTestCostNames();
    else
        return base_learners[0]->getTestCostNames();
}

TVec<string> StackedLearner::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes and 
    // for which it updates the VecStatsCollector train_stats
    if (combiner)
        return combiner->getTrainCostNames();
    else
        return base_learners[0]->getTrainCostNames();
}


///////////////////////////////
// resizeBaseLearnersOutputs //
///////////////////////////////
void StackedLearner::resizeBaseLearnersOutputs() {
    // Ensure that all base learners have the same outputsize if we don't use
    // a combiner
    PLASSERT( base_learners.size() > 0 && base_learners[0] );
    if (! combiner && ! share_learner) {
        int outputsize = base_learners[0]->outputsize();
        if (outputsize > 0) {
            for (int i=1, n=base_learners.size() ; i<n ; ++i)
                if (base_learners[i]->outputsize() != outputsize)
                    PLERROR("StackedLearner::build_: base learner #%d does not have the same "
                            "outputsize (=%d) as base learner #0 (=%d); all outputsizes for "
                            "base learners must be identical",
                            i, base_learners[i]->outputsize(), outputsize);
        }
    }
  
    if(share_learner)
        base_learners_outputs.resize(nsep);
    else
        base_learners_outputs.resize(base_learners.size());
}


void StackedLearner::setBaseLearnersTrainingSet(VMat base_trainset, bool call_forget)
{
    PLASSERT( base_learners.size() > 0 );
  
    // Handle parameter sharing
    if(share_learner) {
        base_learners[0]->setTrainingSet(
            new SeparateInputVMatrix(base_trainset, nsep),
            call_forget && train_base_learners);
    }
    else {
        if (base_train_splitter) {
            // Handle base splitter
            base_train_splitter->setDataSet(base_trainset);
            for (int i=0;i<base_learners.length();i++) {
                base_learners[i]->setTrainingSet(base_train_splitter->getSplit(i)[0],
                                                 call_forget && train_base_learners);
            }
        }
        else {
            // Default situation: set the same training set into each base learner
            for (int i=0;i<base_learners.length();i++)
                base_learners[i]->setTrainingSet(base_trainset,
                                                 call_forget && train_base_learners);
        }
    }
}

void StackedLearner::setCombinerTrainingSet(VMat comb_trainset, bool call_forget)
{
    // Handle combiner
    if (combiner) {
        VMat effective_trainset = comb_trainset;
        if (share_learner)
            effective_trainset = new SeparateInputVMatrix(comb_trainset, nsep);
    
        combiner->setTrainingSet(
            new PLearnerOutputVMatrix(effective_trainset, base_learners, put_raw_input),
            call_forget);
    }
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
