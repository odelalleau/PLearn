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
#include <plearn/base/tostring.h>
#include <plearn/base/ProgressBar.h>
#include <plearn/misc/PLearnService.h>
#include <plearn/misc/RemotePLearnServer.h>
#include <plearn/vmat/MemoryVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    BaggingLearner,
    "Learner that trains several sub-learners on 'bags'",
    "Learner that trains several sub-learners on 'bags'... (TODO: more txt)");

BaggingLearner::BaggingLearner(PP<Splitter> splitter_, 
                               PP<PLearner> template_learner_,
                               TVec<string> stats_,
                               int exclude_extremes_,
                               bool output_sub_outputs_)
    :splitter(splitter_),
     template_learner(template_learner_),
     stats(stats_),
     exclude_extremes(exclude_extremes_),
     output_sub_outputs(output_sub_outputs_)
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

    declareOption(ol, "stats", &BaggingLearner::stats,
                  OptionBase::buildoption,
                  "Functions used to combine outputs from all learners.\n"
                  "\t- 'A' = Average\n",
                  "", OptionBase::basic_level);

    declareOption(ol, "exclude_extremes", &BaggingLearner::exclude_extremes,
                  OptionBase::buildoption,
                  "If >0, sub-learners outputs are sorted and the exclude_extremes "
                  "highest and lowest are excluded");
                  
    declareOption(ol, "output_sub_outputs", &BaggingLearner::output_sub_outputs,
                  OptionBase::buildoption,
                  "Wether computeOutput should append sub-learners outputs to output.");
                  
    declareOption(ol, "learners", &BaggingLearner::learners,
                  OptionBase::learntoption,
                  "Trained sub-learners");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void BaggingLearner::build_()
{}

///////////
// build //
///////////
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
    PLASSERT(template_learner);
    PLASSERT(splitter);
    int sz= template_learner->outputsize() * stats.length(); 
    if(output_sub_outputs)
        sz+= template_learner->outputsize() * splitter->nsplits();
    return sz;
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
            learners[i]->report_progress= false;
        }
    }

    PP<ProgressBar> pb= 0;
    if(report_progress)
        pb= new ProgressBar("BaggingLearner::train",nbags);

    PLearnService& service(PLearnService::instance());
    int nservers= min(nbags, service.availableServers());

    if(nservers > 1 && parallelize_here)//parallel train
    {
        TVec<PP<RemotePLearnServer> > servers= service.reserveServers(nservers);
        nservers= servers.length();

        map<PP<RemotePLearnServer>, int> learners_ids;
        map<PP<RemotePLearnServer>, int> bagnums;
        map<PP<RemotePLearnServer>, int> step;

        for(int i= 0; i < nservers; ++i)
        {
            RemotePLearnServer* s= servers[i];
            int id= s->newObject(*learners[i]);
            VMat sts= splitter->getSplit(i)[0];
            if(master_sends_testset_rows)
                sts= new MemoryVMatrix(sts.toMat());
            s->callMethod(id, "setTrainingSet", sts, true);
            learners_ids[s]= id;
            bagnums[s]= i;
            step[s]= 1;
        }

        int lastbag= nservers-1;
        int ndone= 0;

        while(nservers > 0)
        {
            PP<RemotePLearnServer> s= service.waitForResult();
            switch(step[s])
            {
            case 1: 
                DBG_LOG << "** get setTrainingSet result" << endl;
                s->getResults();//from setTrainingSet
                s->callMethod(learners_ids[s], "train");
                step[s]= 2;
                break;
            case 2:
                DBG_LOG << "** get train result" << endl;
                s->getResults();//from train
                if(pb) pb->update(++ndone);
                s->callMethod(learners_ids[s], "getObject");
                step[s]= 3;
                break;
            case 3:
                DBG_LOG << "** get getObject result" << endl;
                s->getResults(learners[bagnums[s]]);//from getObject
                s->deleteObject(learners_ids[s]);
                if(++lastbag < nbags)
                {
                    int id= s->newObject(*learners[lastbag]);
                    VMat sts= splitter->getSplit(lastbag)[0];
                    if(master_sends_testset_rows)
                        sts= new MemoryVMatrix(sts.toMat());
                    s->callMethod(id, "setTrainingSet", sts, true);
                    learners_ids[s]= id;
                    bagnums[s]= lastbag;
                    step[s]= 1;
                }
                else
                {
                    service.freeServer(s);
                    --nservers;
                }
                break;
            }
        }

        return; // avoid extra indentation
    }

    // sequential train
    for(int i= 0; i < nbags; ++i)
    {
        PP<PLearner> l = learners[i];
        l->setTrainingSet(splitter->getSplit(i)[0]);
        l->train();
        if(pb) pb->update(i);
    }

    stage++;
    PLASSERT( stage == 1 );
}

void BaggingLearner::computeOutput(const Vec& input, Vec& output) const
{
    int nout = outputsize();
    output.resize(nout);
    int nlearners= learners.size();
    PLASSERT(template_learner);
    int sub_nout = template_learner->outputsize();
    learners_outputs.resize(nlearners, sub_nout);

    last_test_input.resize(input.size());
    last_test_input << input;//save it, to test in computeCostsFromOutputs

    for(int i= 0; i < nlearners; ++i)
    {
        Vec outp= learners_outputs(i);
        learners[i]->computeOutput(input, outp);
    }

    if(exclude_extremes > 0)
    {
        outputs.resize(nlearners, sub_nout);
        outputs << learners_outputs;
        //exclude highest and lowest n predictions for each output
        int nexcl= 2*exclude_extremes;
        if(nlearners <= nexcl)
            PLERROR("BaggingLearner::computeOutput : Cannot exclude all outputs! "
                    "nlearners=%d, exclude_extremes=%d",nlearners,exclude_extremes);
        // sort all in place, one output at a time
        for(int j= 0; j < sub_nout; ++j)
            sortElements(outputs.column(j).toVec());
        // exclude from both ends
        outputs= outputs.subMatRows(exclude_extremes, outputs.length()-nexcl);
        nlearners-= nexcl;
    }
    else 
        outputs= learners_outputs;

    stcol.forget();
    for(int i= 0; i < outputs.length(); ++i)
        stcol.update(outputs(i));
    
    int i= 0;
    for(int j= 0; j < stcol.size(); ++j)
        for(TVec<string>::iterator it= stats.begin();
            it != stats.end(); ++it)
            output[i++]= stcol.getStats(j).getStat(*it);

    if(output_sub_outputs)
        for(int j= 0; j < nlearners; ++j)
            for(int k= 0; k < sub_nout; ++k)
                output[i++]= learners_outputs(j,k);
}

void BaggingLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                             const Vec& target, Vec& costs) const
{
    if(input != last_test_input)
        PLERROR("BaggingLearner::computeCostsFromOutputs has to be called "
                "right after computeOutput, with the same input.");
    
    int nlearners= learners.size();
    costs.resize(nTestCosts());
    int k= 0;
    for(int i= 0; i < nlearners; ++i)
    {
        Vec subcosts;
        learners[i]->computeCostsFromOutputs(input, learners_outputs(i),
                                             target, subcosts);
        for(int j= 0; j < subcosts.length(); ++j)
            costs[k++]= subcosts[j];
    }

}

TVec<string> BaggingLearner::getTestCostNames() const
{
    PLASSERT(splitter);
    PLASSERT(template_learner);
    int nbags= splitter->nsplits();
    TVec<string> subcosts= template_learner->getTestCostNames();
    TVec<string> costnames(nTestCosts());
    int nsubcosts= subcosts.length();
    int k= 0;
    for(int i= 0; i < nbags; ++i)
        for(int j= 0; j < nsubcosts; ++j)
            costnames[k++]= string("learner")+tostring(i)+"."+subcosts[j];
    return costnames;
}

TVec<string> BaggingLearner::getTrainCostNames() const
{
    return TVec<string>(); // for now
}

////////////////
// nTestCosts //
////////////////
int BaggingLearner::nTestCosts() const
{
    PLASSERT(splitter);
    PLASSERT(template_learner);
    return splitter->nsplits()*template_learner->nTestCosts();
}

/////////////////
// nTrainCosts //
/////////////////
int BaggingLearner::nTrainCosts() const
{
    return 0;
}

////////////////////////
// resetInternalState //
////////////////////////
void BaggingLearner::resetInternalState()
{
    for(int i= 0; i < learners.length(); ++i)
        learners[i]->resetInternalState();
}

///////////////////////
// isStatefulLearner //
///////////////////////
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

TVec<string> BaggingLearner::getOutputNames() const
{
    PLASSERT(template_learner);
    PLASSERT(splitter);
    TVec<string> suboutputnames= template_learner->getOutputNames();
    TVec<string> outputnames= addStatNames(suboutputnames);
    if(output_sub_outputs)
    {
        int nbags= splitter->nsplits();
        int nsout= suboutputnames.length();
        for(int i= 0; i < nbags; ++i)
            for(int j= 0; j < nsout; ++j)
                outputnames.append(string("learner")+tostring(i)+"."+suboutputnames[j]);
    }
    return outputnames;
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
