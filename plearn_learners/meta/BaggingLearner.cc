// -*- C++ -*-

// BaggingLearner.cc
//
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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

// Authors: Xavier Saint-Mleux

/*! \file BaggingLearner.cc */


#include "BaggingLearner.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    BaggingLearner,
    "Learner that trains several sub-learners on 'bags'",
    "Learner that trains several sub-learners on 'bags'... (TODO: more txt)");

BaggingLearner::BaggingLearner(PP<Splitter> splitter_, 
                               PP<PLearner> template_learner_,
                               char reduce_func_)
    :splitter(splitter_),
     template_learner(template_learner_),
     reduce_func(reduce_func_)
{
}

void BaggingLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "splitter", &BaggingLearner::splitter,
                  OptionBase::buildoption,
                  "Splitter used to get bags(=splits)",
                  "", OptionBase::basic_level);

    declareOption(ol, "template_learner", &BaggingLearner::template_learner,
                  OptionBase::buildoption,
                  "Template for all sub-learners; deep-copied once for each bag",
                  "", OptionBase::basic_level);

    declareOption(ol, "reduce_func", &BaggingLearner::reduce_func,
                  OptionBase::buildoption,
                  "Function used to combine outputs from all learners.\n"
                  "\t- 'A' = Average\n",
                  "", OptionBase::basic_level);

    declareOption(ol, "learners", &BaggingLearner::learners,
                  OptionBase::learntoption,
                  "Trained sub-learners",
                  "", OptionBase::basic_level);

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void BaggingLearner::build_()
{
    if(splitter) splitter->build();
    if(template_learner) template_learner->build();
}

void BaggingLearner::build()
{
    inherited::build();
    build_();
}

void BaggingLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(splitter, copies);
    deepCopyField(template_learner, copies);
    deepCopyField(learners, copies);
}

int BaggingLearner::outputsize() const
{
    return template_learner->outputsize();
}

void BaggingLearner::forget()
{
    for(int i= 0; i < learners.length(); ++i)
        learners[i]->forget();
    inherited::forget();
}

void BaggingLearner::train()
{
    PLASSERT(train_set);
    if(!splitter)
        PLERROR("BaggingLearner::train() needs a splitter.");
    if(!template_learner)
        PLERROR("BaggingLearner::train() needs a template learner.");
    if(nstages != 1)
        PLERROR("BaggingLearner.nstages should be 1 (not %d).", nstages);
    if(splitter->nSetsPerSplit() != 1)
        PLERROR("BaggingLearner.splitter->nSetsPerSplit() should be 1 (not %d).", 
                splitter->nSetsPerSplit());

    splitter->setDataSet(train_set);

    if (!initTrain())
        return;

    // init learners
    int nbags= splitter->nsplits();
    if(learners.size() != nbags)
    {
        learners.resize(nbags);
        for(int i= 0; i < nbags; ++i)
        {
            CopiesMap c;
            learners[i]= template_learner->deepCopy(c);
        }
    }

    // sequential train
    for(int i= 0; i < nbags; ++i)
    {
        PLearner& l= *learners[i];
        l.setTrainingSet(splitter->getSplit(i)[0]);
        l.train();
    }

}

void BaggingLearner::computeOutput(const Vec& input, Vec& output) const
{
    int nout = outputsize();
    output.resize(nout);
    output.fill(0.);
    int nlearners= learners.size();
    static TVec<Vec> learners_outputs(nlearners);//don't realloc every time

    for(int i= 0; i < nlearners; ++i)
        learners[i]->computeOutput(input, learners_outputs[i]);

    switch(reduce_func)
    {
    case 'A':
        for(int i= 0; i < nlearners; ++i)
            for(int j= 0; j < nout; ++j)
                output[j]+= learners_outputs[i][j];
        for(int j= 0; j < nout; ++j)
            output[j]/= nlearners;
        break;
    default:
        PLERROR("BaggingLearner::computeOutput : reduce_func '%c' unknown.",
                reduce_func);
    }
}

void BaggingLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    // FIXME: for now, costs are the average of underlying learners' costs
    //        BUT the name of those costs is the same... (misleading)
    int nlearners= learners.size();
    TVec<Vec> learners_costs(nlearners);
    int ncosts= nTestCosts();
    costs.resize(ncosts);
    costs.fill(0.);
    for(int i= 0; i < nlearners; ++i)
        learners[i]->computeCostsFromOutputs(input, output, target, 
                                             learners_costs[i]);
    for(int i= 0; i < nlearners; ++i)
        for(int j= 0; j < ncosts; ++j)
            costs[j]+= learners_costs[i][j];
    for(int i= 0; i < ncosts; ++i)
        costs[i]/= nlearners;
}

TVec<string> BaggingLearner::getTestCostNames() const
{
    PLASSERT(template_learner);
    PLWARNING("BaggingLearner::getTestCostNames() : the test costs are actually the mean of test costs for all learners (bags).");
    return template_learner->getTestCostNames();
}

TVec<string> BaggingLearner::getTrainCostNames() const
{
    PLASSERT(template_learner);
    PLWARNING("BaggingLearner::getTrainCostNames() : the train costs are actually the mean of train costs for all learners (bags).");
    return template_learner->getTrainCostNames();
}

int BaggingLearner::nTestCosts() const
{
    PLASSERT(template_learner);
    return template_learner->nTestCosts();
}
int BaggingLearner::nTrainCosts() const
{
    PLASSERT(template_learner);
    return template_learner->nTrainCosts();
}
void BaggingLearner::resetInternalState()
{
    for(int i= 0; i < learners.length(); ++i)
        learners[i]->forget();
}
bool BaggingLearner::isStatefulLearner() const
{
    PLASSERT(template_learner);
    return template_learner->isStatefulLearner();
}

void BaggingLearner::setTrainingSet(VMat training_set, bool call_forget)
{
    PLASSERT(template_learner);
    //set template learner's train set so that we can get 
    //output size and names (among others)
    template_learner->setTrainingSet(training_set, call_forget);
    inherited::setTrainingSet(training_set, call_forget);
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
