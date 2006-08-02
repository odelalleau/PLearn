// -*- C++ -*-

// ChainedLearners.cc
//
// Copyright (C) 2006 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file ChainedLearners.cc */


#include "ChainedLearners.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ChainedLearners,
    "A learner that allows to chain several learners to perform various preprocessing steps.",
    "Learners in the list will be trained in sequence. After a learner_k has been trained,\n"
    "the training set for learner_{k+1} is obtained with learner_k->processDataSet(...) \n"
    "By default, this is a view of the learner_k's training set but with the inputs\n"
    "replaced by learner_k's computed outputs. This allows for ex. to chain a \n"
    "NormalizationLearner, followed by a PCA, followed by a NNet\n"
    "Note: StackedLearner offers a similar functionality and much more, but it is\n"
    "more cumbersome to use for multiple chaining. Consider using StackedLEarner if\n"
    "you need extra functionality, such as using a concatenation of the outputs of \n"
    "several learners at the same level.\n");

ChainedLearners::ChainedLearners()
{}

void ChainedLearners::declareOptions(OptionList& ol)
{
    declareOption(ol, "learners", &ChainedLearners::learners, 
                  OptionBase::buildoption,
                  "This is a list of learners to train in sequence.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ChainedLearners::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.
}

// ### Nothing to add here, simply calls build_
void ChainedLearners::build()
{
    inherited::build();
    build_();
}


void ChainedLearners::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(learners, copies);
    deepCopyField(tmp_input, copies);
    deepCopyField(tmp_output, copies);
}


int ChainedLearners::outputsize() const
{
    return learners.lastElement()->outputsize();
}

void ChainedLearners::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!)
    /*!
      A typical forget() method should do the following:
      - call inherited::forget() to initialize its random number generator
        with the 'seed' option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */

    for(int k=0; k<learners.length(); k++)
        learners[k]->forget();
    inherited::forget();
    stage = 0;
}

void ChainedLearners::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set, call_forget);
    VMat dataset = getTrainingSet();
    int nlearners = learners.length();
    for(int k=0; k<nlearners; k++)
    {
        learners[k]->setTrainingSet(dataset, call_forget);
        if(k<nlearners-1)
            dataset = learners[k]->processDataSet(dataset);            
    }
}

void ChainedLearners::train()
{
    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    VMat dataset = getTrainingSet();
    int nlearners = learners.length();
    if(stage>nstages)
        forget();

    if(stage==0)
    {
        for(int k=0; k<nlearners; k++)
        {
            learners[k]->setTrainingSet(dataset);
            if(k<nlearners-1)
            {
                learners[k]->train();
                dataset = learners[k]->processDataSet(dataset);            
            }
            else // last learner
            {
                learners[k]->setTrainStatsCollector(train_stats);
                train_stats->forget();
                learners[k]->train();                
                train_stats->finalize(); 
            }
        }
        ++stage;
    }
    else // stage already==1
    { // only call train on last learner, in case its own nstages has changed or something similar
        learners[nlearners-1]->train();
    }
}

void ChainedLearners::computeOutput(const Vec& input, Vec& output) const
{
    int nlearners = learners.length();
    if(nlearners==1)
        learners[0]->computeOutput(input, output);
    else
    {
        tmp_output.resize(learners[0]->outputsize());
        learners[0]->computeOutput(input, tmp_output);
        for(int k=1; k<nlearners-1; k++)
        {
            int n = tmp_output.length();
            tmp_input.resize(n);
            tmp_input << tmp_output;
            tmp_output->resize(learners[k]->outputsize());
            learners[k]->computeOutput(tmp_input, tmp_output);
        }
        learners[nlearners-1]->computeOutput(tmp_output, output);
    }
}

void ChainedLearners::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    return learners.lastElement()->computeCostsFromOutputs(input, output, target, costs);
}

TVec<string> ChainedLearners::getTestCostNames() const
{
    return learners.lastElement()->getTestCostNames();
}

TVec<string> ChainedLearners::getTrainCostNames() const
{
    return learners.lastElement()->getTrainCostNames();
}


TVec<string> ChainedLearners::getOutputNames() const
{
    return learners.lastElement()->getOutputNames();
}


void ChainedLearners::setExperimentDirectory(const PPath& the_expdir)
{
    inherited::setExperimentDirectory(the_expdir);
    if (! the_expdir.isEmpty())
        for(int k= 0; k < learners.length(); ++k)
            learners[k]->setExperimentDirectory(the_expdir /
                                                ("SubLearner_"+tostring(k)));
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
