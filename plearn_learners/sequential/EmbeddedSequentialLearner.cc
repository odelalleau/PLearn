// -*- C++ -*-

// EmbeddedSequentialLearner.cc
//
// Copyright (C) 2003 Rejean Ducharme, Yoshua Bengio
// Copyright (C) 2003 Pascal Vincent
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



#include "EmbeddedSequentialLearner.h"
#include <plearn/vmat/TemporalHorizonVMatrix.h>
#include <plearn/io/TmpFilenames.h>
#include <plearn/vmat/VMat_basic_stats.h>

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(EmbeddedSequentialLearner, "ONE LINE DESCR", "NO HELP");

EmbeddedSequentialLearner::EmbeddedSequentialLearner()
{}

void EmbeddedSequentialLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(learner, copies);
} 

void EmbeddedSequentialLearner::build_()
{
    if (learner.isNull())
        PLERROR("EmbeddedSequentialLearner::build()_ - learner attribute is NULL");

    learner->build();

    forget();
}

void EmbeddedSequentialLearner::build()
{
    inherited::build();
    build_();
}

void EmbeddedSequentialLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "learner", &EmbeddedSequentialLearner::learner,
                  OptionBase::buildoption, "The underlying learner \n");

    inherited::declareOptions(ol);
}

void EmbeddedSequentialLearner::train()
{
    // TODO: this code should be moved to overrided setTrainingSet and setTrainStatsCollector (Pascal&Nicolas)

    int t = train_set.length();
    if (t >= last_train_t+train_step)
    {
        VMat aligned_set = new TemporalHorizonVMatrix(train_set, horizon, targetsize()); // last training pair is (t-1-horizon,t-1)
        int start = (max_train_len<0) ? 0 : max(0,aligned_set.length()-max_train_len);
        int len = aligned_set.length()-start;
        TmpFilenames tmpfile;
        // TODO: Remove the ugly, grotesque, brittle and unnecessay use of an "indexfile" (Nicolas&Pascal)
        string index_fname = tmpfile.addFilename();
        VMat aligned_set_non_missing = filter(aligned_set.subMatRows(start,len), index_fname);
        learner->setTrainingSet(aligned_set_non_missing);
        learner->setTrainStatsCollector(train_stats);
        learner->train();
        last_train_t = t;
    }

    // BUG? what about setting last_call_train_t ???
}
 
void EmbeddedSequentialLearner::test(VMat testset, PP<VecStatsCollector> test_stats,
                                     VMat testoutputs, VMat testcosts) const
{
    int l = testset.length();
    Vec input, target;
    static Vec dummy_input;
    real weight;
 
    Vec output(testoutputs ?outputsize() :0);
    Vec costs(nTestCosts());
 
    //testset->defineSizes(inputsize(),targetsize(),weightsize());
 
    //test_stats.forget();
 
    // We DON'T allow in-sample testing; hence, we test either from the end of the
    // last test, or the end of the training set.  The last_train_t MINUS 1 is because
    // we allow the last training day to be part of the test set. Example: using
    // today's price, we can train a model and then use it to make a prediction that
    // has today's price as input (all that WITHOUT CHEATING or breaking the Criminal
    // Code.)
    int start = MAX(last_train_t-1,last_test_t);
    PP<ProgressBar> pb;
    if(report_progress)
        pb = new ProgressBar("Testing learner",l-start);
    for (int t=start; t<testset.length(); t++)
    {
        testset.getExample(t, input, target, weight);
        //testset.getSample(t-last_call_train_t+1, input, dummy_target, weight);
        //testset.getSample(t-last_call_train_t+1+horizon, dummy_input, target, dummy_weight);

        if (!input.hasMissing())
        {
            Vec output = predictions(t);
            learner->computeOutput(input, output);
            if (testoutputs) testoutputs->appendRow(output);
        }
        if (t>=horizon)
        {
            Vec output = predictions(t-horizon);
            if (!target.hasMissing() && !output.hasMissing())
            {
                Vec error_t = errors(t);
                learner->computeCostsFromOutputs(dummy_input, output, target, error_t);
                if (testcosts) testcosts->appendRow(error_t);
                test_stats->update(error_t);
            }
            //learner->computeOutputAndCosts(input, target, weight, output, costs);
            //predictions(t) << output;
            //errors(t+horizon) << costs;

            if (pb)
                pb->update(t-start);
        }
    }
    last_test_t = testset.length();
}

void EmbeddedSequentialLearner::forget()
{
    // BUG? call inherited::forget(); ???
    learner->forget();
}
 
void EmbeddedSequentialLearner::computeOutput(const Vec& input, Vec& output)
{ learner->computeOutput(input, output); }
 
void EmbeddedSequentialLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                                        const Vec& target, Vec& costs)
{ learner->computeCostsFromOutputs(input, output, target, costs); }
 
void EmbeddedSequentialLearner::computeOutputAndCosts(const Vec& input, const Vec& target,
                                                      Vec& output, Vec& costs)
{ learner->computeOutputAndCosts(input, target, output, costs); }
 
void EmbeddedSequentialLearner::computeCostsOnly(const Vec& input, const Vec& target, Vec& costs)
{ learner->computeCostsOnly(input, target, costs); }

TVec<string> EmbeddedSequentialLearner::getTestCostNames() const
{ return learner->getTestCostNames(); }
 
TVec<string> EmbeddedSequentialLearner::getTrainCostNames() const
{ return learner->getTrainCostNames(); }


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
