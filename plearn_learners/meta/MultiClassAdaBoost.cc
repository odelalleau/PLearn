// -*- C++ -*-

// plearn_learners/meta/MultiClassAdaBoost.cc
//
// Copyright (C) 2007 Frederic Bastien
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

// Authors: Frederic Bastien

/*! \file MultiClassAdaBoost.cc */


#include "MultiClassAdaBoost.h"
#include <plearn/vmat/ProcessingVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    MultiClassAdaBoost,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP");

MultiClassAdaBoost::MultiClassAdaBoost():
    forward_sub_learner_test_costs(false)
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)

    // ### If this learner needs to generate random numbers, uncomment the
    // ### line below to enable the use of the inherited PRandom object.
    // random_gen = new PRandom();
}

void MultiClassAdaBoost::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &MultiClassAdaBoost::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
    declareOption(ol, "learner1", &MultiClassAdaBoost::learner1,
                  OptionBase::buildoption,
                  "The sub learner to use.");
    declareOption(ol, "learner2", &MultiClassAdaBoost::learner2,
                  OptionBase::buildoption,
                  "The sub learner to use.");
    declareOption(ol, "forward_sub_learner_test_costs", 
                  &MultiClassAdaBoost::forward_sub_learner_test_costs,
                  OptionBase::buildoption,
                  "Did we add the learner1 and learner2 costs to our costs.\n");

}

void MultiClassAdaBoost::build_()
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
    sub_target_tmp.resize(2);
    for(int i=0;i<sub_target_tmp.size();i++)
        sub_target_tmp[i].resize(1);
    if(learner1)
        output1.resize(learner1->outputsize());
    if(learner2)
        output2.resize(learner2->outputsize());
    if(!train_stats)
        train_stats=new VecStatsCollector();
}

// ### Nothing to add here, simply calls build_
void MultiClassAdaBoost::build()
{
    inherited::build();
    build_();
}


void MultiClassAdaBoost::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("MultiClassAdaBoost::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int MultiClassAdaBoost::outputsize() const
{
    // Compute and return the size of this learner's output (which typically
    // may depend on its inputsize(), targetsize() and set options).

    return 3;
}

void MultiClassAdaBoost::forget()
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
    inherited::forget();

    stage = 0;
    train_stats->forget();
    learner1->forget();
    learner2->forget();
}

void MultiClassAdaBoost::train()
{
    learner1->nstages = nstages;
    learner2->nstages = nstages;

//if you use the parallel version, you must disable all verbose, verbosity and report progress int he learner1 and learner2.
//Otherwise this will cause crash due to the parallel printing to stdout stderr.
#ifdef _OPENMP
    //the AdaBoost and the weak learner should not print anything as this will cause race condition on the printing
    PLCHECK(learner1->verbosity==0);
    PLCHECK(learner2->verbosity==0);
    PLCHECK(learner1->report_progress==false);
    PLCHECK(learner2->report_progress==false);

    PLCHECK(learner1->weak_learner_template->verbosity==0);
    PLCHECK(learner2->weak_learner_template->verbosity==0);
    PLCHECK(learner1->weak_learner_template->report_progress==false);
    PLCHECK(learner2->weak_learner_template->report_progress==false);

#pragma omp parallel sections
{
#pragma omp section 
    learner1->train();
#pragma omp section 
    learner2->train();
}
#else
    learner1->train();
    learner2->train();
#endif

    stage=max(learner1->stage,learner2->stage);

    train_stats->stats.resize(0);
    PP<VecStatsCollector> v;

    //we do it this way in case the learner don't have train_stats
    if(v=learner1->getTrainStatsCollector())
        train_stats->append(*(v),"sublearner1.");
    if(v=learner2->getTrainStatsCollector())
        train_stats->append(*(v),"sublearner2.");
}

void MultiClassAdaBoost::computeOutput(const Vec& input, Vec& output) const
{
    PLASSERT(output.size()==outputsize());
    PLASSERT(output1.size()==learner1->outputsize());
    PLASSERT(output2.size()==learner2->outputsize());

    learner1->computeOutput(input, output1);
    learner2->computeOutput(input, output2);
    int ind1=int(round(output1[0]));
    int ind2=int(round(output2[0]));

    int ind=-1;
    if(ind1==0 && ind2==0)
        ind=0;
    else if(ind1==1 && ind2==0)
        ind=1;
    else if(ind1==1 && ind2==1)
        ind=2;
    else
        ind=1;//TODOself.confusion_target;
    output[0]=ind;
    output[1]=output1[0];
    output[2]=output2[0];
}
void MultiClassAdaBoost::computeOutputAndCosts(const Vec& input,
                                               const Vec& target,
                                               Vec& output, Vec& costs) const
{
    PLASSERT(costs.size()==nTestCosts());
    PLASSERT_MSG(output.length()==outputsize(),
                 "In MultiClassAdaBoost::computeOutputAndCosts -"
                 " output don't have the good length!");
    output.resize(outputsize());

    subcosts1.resize(learner1->nTestCosts());
    subcosts2.resize(learner1->nTestCosts());

    getSubLearnerTarget(target, sub_target_tmp);

    learner1->computeOutputAndCosts(input, sub_target_tmp[0],
                                   output1, subcosts1);
    learner2->computeOutputAndCosts(input, sub_target_tmp[1],
                                   output2, subcosts2);

    int ind1=int(round(output1[0]));
    int ind2=int(round(output2[0]));
    int ind=-1;
    if(ind1==0 && ind2==0)
        ind=0;
    else if(ind1==1 && ind2==0)
        ind=1;
    else if(ind1==1 && ind2==1)
        ind=2;
    else
        ind=1;//TODOself.confusion_target;
    output[0]=ind;
    output[1]=output1[0];
    output[2]=output2[0];

    int out = ind;
    int pred = int(round(target[0]));
    costs[0]=int(out != pred);//class_error
    costs[1]=abs(out-pred);//linear_class_error
    costs[2]=pow(real(abs(out-pred)),2);//square_class_error
    
    //append conflict cost
    if(fast_is_equal(round(output[1]),0) 
       && fast_is_equal(round(output[2]),1))
        costs[3]=1;
    else
        costs[3]=0;

    costs[4]=costs[5]=costs[6]=0;
    costs[out+4]=1;

    if(forward_sub_learner_test_costs){
        costs.resize(7);
        subcosts1+=subcosts2;
        costs.append(subcosts1);
    }

    PLASSERT(costs.size()==nTestCosts());

}

void MultiClassAdaBoost::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    PLASSERT(costs.size()==nTestCosts());

    int out = int(round(output[0]));
    int pred = int(round(target[0]));
    costs[0]=int(out != pred);//class_error
    costs[1]=abs(out-pred);//linear_class_error
    costs[2]=pow(real(abs(out-pred)),2);//square_class_error
    
    //append conflict cost
    if(fast_is_equal(round(output[1]),0) 
       && fast_is_equal(round(output[2]),1))
        costs[3]=1;
    else
        costs[3]=0;

    costs[4]=costs[5]=costs[6]=0;
    costs[out+4]=1;

    if(forward_sub_learner_test_costs){
        costs.resize(7);
        subcosts1.resize(learner1->nTestCosts());
        subcosts2.resize(learner1->nTestCosts());
        getSubLearnerTarget(target, sub_target_tmp);

        learner1->computeCostsOnly(input,sub_target_tmp[0],subcosts1);
        learner2->computeCostsOnly(input,sub_target_tmp[1],subcosts2);
        subcosts1+=subcosts2;
        costs.append(subcosts1);
    }

    PLASSERT(costs.size()==nTestCosts());
}

TVec<string> MultiClassAdaBoost::getOutputNames() const
{
    TVec<string> names(3);
    names[0]="prediction";
    names[1]="prediction_learner_1";
    names[2]="prediction_learner_2";
    return names;
}

TVec<string> MultiClassAdaBoost::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).
    // ...
    TVec<string> names;
    names.append("class_error");
    names.append("linear_class_error");
    names.append("square_class_error");
    names.append("conflict");
    names.append("class0");
    names.append("class1");
    names.append("class2");
    if(forward_sub_learner_test_costs){
        TVec<string> subcosts=learner1->getTestCostNames();
        for(int i=0;i<subcosts.length();i++){
            subcosts[i]="sum_sublearner."+subcosts[i];
        }
        names.append(subcosts);
    }
    return names;
}

TVec<string> MultiClassAdaBoost::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes
    // and for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by
    // getTestCostNames).
    // ...

    TVec<string> names;
    return names;
}

void MultiClassAdaBoost::getSubLearnerTarget(Vec target,
                                             TVec<Vec> sub_target) 
{
    if(fast_is_equal(target[0],0.)){
        sub_target[0][0]=0;
        sub_target[1][0]=0;
    }else if(fast_is_equal(target[0],1.)){
        sub_target[0][0]=1;
        sub_target[1][0]=0;
    }else if(fast_is_equal(target[0],2.)){
        sub_target[0][0]=1;
        sub_target[1][0]=1;
    }else if(target[0]>2){
        PLWARNING("In MultiClassAdaBoost::getSubLearnerTarget - "
                  "We only support target 0/1/2. We got %f. We transform "
                  "it to a target of 2.", target[0]);
        sub_target[0][0]=1;
        sub_target[1][0]=1;
    }else
        PLERROR("In MultiClassAdaBoost::getSubLearnerTarget - "
                  "We only support target 0/1/2. We got %f.", target[0]); 
}

void MultiClassAdaBoost::setTrainingSet(VMat training_set, bool call_forget)
{ 
    PLCHECK(learner1 && learner2);
    inherited::setTrainingSet(training_set, call_forget);
    string targetname = training_set->fieldName(training_set->inputsize());
    string input_prg  = "[%0:%"+tostring(training_set->inputsize()-1)+"]";
    string target_prg1= "@"+targetname+" 1 0 ifelse :"+targetname;
    string target_prg2= "@"+targetname+" 2 - 0 1 ifelse :"+targetname;
    string weight_prg = "1 :weight";

    VMat vmat1 = new ProcessingVMatrix(training_set, input_prg,
                                       target_prg1,  weight_prg);
    VMat vmat2 = new ProcessingVMatrix(training_set, input_prg,
                                       target_prg2,  weight_prg);

    //We don't give it if the script give them one explicitly.
    //This can be usefull for optimization
    if(!learner1->getTrainingSet())
        learner1->setTrainingSet(vmat1, call_forget);
    if(!learner2->getTrainingSet())
        learner2->setTrainingSet(vmat2, call_forget);
}

void MultiClassAdaBoost::test(VMat testset, PP<VecStatsCollector> test_stats,
                              VMat testoutputs, VMat testcosts) const
{
    Profiler::pl_profile_start("MultiClassAdaBoost::test");
    inherited::test(testset,test_stats,testoutputs,testcosts);

    Profiler::pl_profile_end("MultiClassAdaBoost::test");
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
