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
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    MultiClassAdaBoost,
    "Implementation of a 3 class AdaBoost learning algorithm.",
    "It divide the work in 2 sub learner AdaBoost, class 0 vs other"
    " and 2 vs other.");

MultiClassAdaBoost::MultiClassAdaBoost():
    train_time(0),
    total_train_time(0),
    test_time(0),
    total_test_time(0),
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
                  OptionBase::learntoption,
                  "The sub learner to use.");
    declareOption(ol, "learner2", &MultiClassAdaBoost::learner2,
                  OptionBase::learntoption,
                  "The sub learner to use.");
    declareOption(ol, "forward_sub_learner_test_costs", 
                  &MultiClassAdaBoost::forward_sub_learner_test_costs,
                  OptionBase::buildoption,
                  "Did we add the learner1 and learner2 costs to our costs.\n");
    declareOption(ol, "learner_template", 
                  &MultiClassAdaBoost::learner_template,
                  OptionBase::buildoption,
                  "The template to use for learner1 and learner2.\n");

}

void MultiClassAdaBoost::build_()
{
    sub_target_tmp.resize(2);
    for(int i=0;i<sub_target_tmp.size();i++)
        sub_target_tmp[i].resize(1);
    
    if(learner_template){
        if(!learner1)
            learner1 = ::PLearn::deepCopy(learner_template);
        if(!learner2)
            learner2 = ::PLearn::deepCopy(learner_template);
    }
    if(learner1)
        output1.resize(learner1->outputsize());
    if(learner2)
        output2.resize(learner2->outputsize());
    if(!train_stats)
        train_stats=new VecStatsCollector();

    if(train_set){
        if(learner1 && learner2)
            if(! learner1->getTrainingSet()
               || ! learner2->getTrainingSet()
               || targetname.empty()
                )
                setTrainingSet(train_set);
    }

    Profiler::activate();
    Profiler::reset("MultiClassAdaBoost::test");
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

    deepCopyField(output1,           copies);
    deepCopyField(output2,           copies);
    deepCopyField(subcosts1,         copies);
    deepCopyField(subcosts2,         copies);
    deepCopyField(learner1,          copies);
    deepCopyField(learner2,          copies);
    //not needed as we only read it.
    //deepCopyField(learner_template,  copies);
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
    Profiler::start("MultiClassAdaBoost::train");

    learner1->nstages = nstages;
    learner2->nstages = nstages;

//if you use the parallel version, you must disable all verbose, verbosity and report progress int he learner1 and learner2.
//Otherwise this will cause crash due to the parallel printing to stdout stderr.
#ifdef _OPENMP
    //the AdaBoost and the weak learner should not print anything as this will cause race condition on the printing
    if(omp_get_max_threads()>1){
      PLCHECK(learner1->verbosity==0);
      PLCHECK(learner2->verbosity==0);
      PLCHECK(learner1->report_progress==false);
      PLCHECK(learner2->report_progress==false);
      
      PLCHECK(learner1->weak_learner_template->verbosity==0);
      PLCHECK(learner2->weak_learner_template->verbosity==0);
      PLCHECK(learner1->weak_learner_template->report_progress==false);
      PLCHECK(learner2->weak_learner_template->report_progress==false);
    }
#pragma omp parallel sections default(none)
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

    Profiler::end("MultiClassAdaBoost::train");
    const Profiler::Stats& stats = Profiler::getStats("MultiClassAdaBoost::train");
    real tmp=stats.wall_duration/Profiler::ticksPerSecond();
    train_time=tmp - total_train_time;
    total_train_time=tmp;

    //we get the test_time here as we want the test time for all dataset.
    //if we put it in the test function, we would have it for one dataset.
    const Profiler::Stats& stats_test = Profiler::getStats("MultiClassAdaBoost::test");
    tmp=stats_test.wall_duration/Profiler::ticksPerSecond();
    test_time=tmp-total_test_time;
    total_test_time=tmp;  
}

void MultiClassAdaBoost::computeOutput(const Vec& input, Vec& output) const
{
    PLASSERT(output.size()==outputsize());
    PLASSERT(output1.size()==learner1->outputsize());
    PLASSERT(output2.size()==learner2->outputsize());
#ifdef _OPENMP
#pragma omp parallel sections default(none)
{
#pragma omp section
    learner1->computeOutput(input, output1);
#pragma omp section
    learner2->computeOutput(input, output2);
}

#else
    learner1->computeOutput(input, output1);
    learner2->computeOutput(input, output2);
#endif
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

    getSubLearnerTarget(target, sub_target_tmp);
#ifdef _OPENMP
#pragma omp parallel sections default(none)
{
#pragma omp section
    learner1->computeOutputAndCosts(input, sub_target_tmp[0],
                                    output1, subcosts1);
#pragma omp section
    learner2->computeOutputAndCosts(input, sub_target_tmp[1],
                                    output2, subcosts2);
}

#else
    learner1->computeOutputAndCosts(input, sub_target_tmp[0],
                                    output1, subcosts1);
    learner2->computeOutputAndCosts(input, sub_target_tmp[1],
                                    output2, subcosts2);
#endif

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
    costs[7]=train_time;
    costs[8]=total_train_time;
    costs[9]=test_time;
    costs[10]=total_test_time;
    if(forward_sub_learner_test_costs){
        costs.resize(7+4);
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
    costs[7]=train_time;
    costs[8]=total_train_time;
    costs[9]=test_time;
    costs[10]=total_test_time;

    if(forward_sub_learner_test_costs){
        costs.resize(7+4);
        subcosts1.resize(learner1->nTestCosts());
        subcosts2.resize(learner1->nTestCosts());
        getSubLearnerTarget(target, sub_target_tmp);

//not paralized as this to add more overhead then the time saved.
//meaby not true for all weak_learner.
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
    names.append("train_time");
    names.append("total_train_time");
    names.append("test_time");
    names.append("total_test_time");
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

    targetname = training_set->fieldName(training_set->inputsize());
    input_prg  = "[%0:%"+tostring(training_set->inputsize()-1)+"]";
    target_prg1= "@"+targetname+" 1 0 ifelse :"+targetname;
    target_prg2= "@"+targetname+" 2 - 0 1 ifelse :"+targetname;

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
    subcosts2.resize(learner2->nTestCosts());
    subcosts1.resize(learner1->nTestCosts());

    inherited::setTrainingSet(training_set, call_forget);
}

void MultiClassAdaBoost::test(VMat testset, PP<VecStatsCollector> test_stats,
                              VMat testoutputs, VMat testcosts) const
{
    Profiler::pl_profile_start("MultiClassAdaBoost::test");
    subcosts1.resize(learner1->nTestCosts());
    subcosts2.resize(learner2->nTestCosts());
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
