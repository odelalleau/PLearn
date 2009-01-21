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
#include <plearn_learners/regressors/RegressionTreeRegisters.h>
#define PL_LOG_MODULE_NAME "MultiClassAdaBoost"
#include <plearn/io/pl_log.h>
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
    time_sum(0),
    time_sum_rtr(0),
    time_last_stage(0),
    time_last_stage_rtr(0),
    last_stage(0),
    forward_sub_learner_test_costs(false),
    forward_test(0)
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
    declareOption(ol, "forward_test", 
                  &MultiClassAdaBoost::forward_test,
                  OptionBase::buildoption,
                  "if 0, default test. If 1 forward the test fct to the sub"
                  " learner. If 2, determine at each stage what is the faster"
                  " based on past  test time.\n");

    declareOption(ol, "train_time",
                  &MultiClassAdaBoost::train_time, OptionBase::learntoption,
                  "The time spent in the last call to train() in second.");

    declareOption(ol, "total_train_time",
                  &MultiClassAdaBoost::total_train_time, OptionBase::learntoption,
                  "The total time spent in the train() function in second.");

    declareOption(ol, "test_time",
                  &MultiClassAdaBoost::test_time, OptionBase::learntoption,
                  "The time spent in the last call to test() in second.");

    declareOption(ol, "total_test_time",
                  &MultiClassAdaBoost::total_test_time, OptionBase::learntoption,
                  "The total time spent in the test() function in second.");

    declareOption(ol, "time_sum",
                  &MultiClassAdaBoost::time_sum, 
                  OptionBase::learntoption|OptionBase::nosave,
                  "The time spend in test() during the last stage. If test() is called multiple time for the same stage this is the sum of the time.");
    declareOption(ol, "time_sum_rtr",
                  &MultiClassAdaBoost::time_sum_rtr, 
                  OptionBase::learntoption|OptionBase::nosave,
                  "The time spend in test() during the last stage. If test() is called multiple time for the same stage this is the sum of the time.");
    declareOption(ol, "time_last_stage",
                  &MultiClassAdaBoost::time_last_stage, 
                  OptionBase::learntoption|OptionBase::nosave,
                  "");
    declareOption(ol, "time_last_stage_rtr",
                  &MultiClassAdaBoost::time_last_stage_rtr, 
                  OptionBase::learntoption|OptionBase::nosave,
                  "");
    declareOption(ol, "last_stage",
                  &MultiClassAdaBoost::last_stage, 
                  OptionBase::learntoption |OptionBase::nosave,
                  "The stage at witch time_sum was used");
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
    tmp_target.resize(1);
    tmp_output.resize(outputsize());
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
    Profiler::reset("MultiClassAdaBoost::test()");
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

    deepCopyField(tmp_input,             copies);
    deepCopyField(tmp_target,            copies);
    deepCopyField(tmp_output,            copies);
    deepCopyField(tmp_costs,             copies);
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

void MultiClassAdaBoost::finalize()
{
    inherited::finalize();
    learner1->finalize();
    learner2->finalize();
}

void MultiClassAdaBoost::forget()
{
    inherited::forget();

    stage = 0;
    train_stats->forget();
    learner1->forget();
    learner2->forget();
}

void MultiClassAdaBoost::train()
{
    EXTREME_MODULE_LOG<<"train() start"<<endl;
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
      
      PLCHECK(learner1->weak_learner_template->verbosity==0);
      PLCHECK(learner2->weak_learner_template->verbosity==0);
    }
    
    EXTREME_MODULE_LOG<<"train() // start"<<endl;
#pragma omp parallel sections default(none)
{
#pragma omp section 
    learner1->train();
#pragma omp section 
    learner2->train();
}
    EXTREME_MODULE_LOG<<"train() // end"<<endl;
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
    const Profiler::Stats& stats_test = Profiler::getStats("MultiClassAdaBoost::test()");
    tmp=stats_test.wall_duration/Profiler::ticksPerSecond();
    test_time=tmp-total_test_time;
    total_test_time=tmp;
    EXTREME_MODULE_LOG<<"train() end"<<endl;
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
  subcosts1.resize(0);
  subcosts2.resize(0);
  computeCostsFromOutputs_(input, output, target, subcosts1, subcosts2, costs);
}

void MultiClassAdaBoost::computeCostsFromOutputs_(const Vec& input, const Vec& output,
						  const Vec& target, Vec& sub_costs1,
						  Vec& sub_costs2, Vec& costs) const
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
	PLASSERT(sub_costs1.size()==learner1->nTestCosts() || sub_costs1.size()==0);
	PLASSERT(sub_costs2.size()==learner2->nTestCosts() || sub_costs2.size()==0);

        getSubLearnerTarget(target, sub_target_tmp);
	if(sub_costs1.size()==0){
            PLASSERT(input.size()>0);
            sub_costs1.resize(learner1->nTestCosts());
            learner1->computeCostsOnly(input,sub_target_tmp[0],sub_costs1);
	}
	if(sub_costs2.size()==0){
            PLASSERT(input.size()>0);
            sub_costs2.resize(learner2->nTestCosts());
            learner2->computeCostsOnly(input,sub_target_tmp[1],sub_costs2);
	}
        sub_costs1+=sub_costs2;
        costs.append(sub_costs1);
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

void MultiClassAdaBoost::getSubLearnerTarget(const Vec target,
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
    }else{
        PLERROR("In MultiClassAdaBoost::getSubLearnerTarget - "
                  "We only support target 0/1/2. We got %f.", target[0]); 
        sub_target[0][0]=0;
        sub_target[1][0]=0;
    }
}

void MultiClassAdaBoost::setTrainingSet(VMat training_set, bool call_forget)
{ 
    PLCHECK(learner1 && learner2);

    bool training_set_has_changed = !train_set || !(train_set->looksTheSameAs(training_set));

    targetname = training_set->fieldName(training_set->inputsize());
    input_prg  = "[%0:%"+tostring(training_set->inputsize()-1)+"]";
    target_prg1= "@"+targetname+" 1 0 ifelse :"+targetname;
    target_prg2= "@"+targetname+" 2 - 0 1 ifelse :"+targetname;

    if(training_set->weightsize()>0){
        int index = training_set->inputsize()+training_set->targetsize();
        weight_prg = "[%"+tostring(index)+"]";
    }else
        weight_prg = "1 :weights";
    
    //We don't give it if the script give them one explicitly.
    //This can be usefull for optimization
    if(training_set_has_changed || !learner1->getTrainingSet()){
        VMat vmat1 = new ProcessingVMatrix(training_set, input_prg,
                                           target_prg1,  weight_prg);
        learner1->setTrainingSet(vmat1, call_forget);
    }
    if(training_set_has_changed || !learner2->getTrainingSet()){
        VMat vmat2 = new ProcessingVMatrix(training_set, input_prg,
                                           target_prg2,  weight_prg);
        PP<RegressionTreeRegisters> t1 =
            (PP<RegressionTreeRegisters>)learner1->getTrainingSet();
        if(t1->classname()=="RegressionTreeRegisters"){
            vmat2 = new RegressionTreeRegisters(vmat2,
                                                t1->getTSortedRow(),
                                                t1->getTSource(),
                                                learner1->report_progress,
                                                learner1->verbosity,false);
        }
        learner2->setTrainingSet(vmat2, call_forget);
    }

    //we do it here as RegressionTree need a trainingSet to know
    // the number of test.
    subcosts2.resize(learner2->nTestCosts());
    subcosts1.resize(learner1->nTestCosts());

    inherited::setTrainingSet(training_set, call_forget);
}

void MultiClassAdaBoost::test(VMat testset, PP<VecStatsCollector> test_stats,
                              VMat testoutputs, VMat testcosts) const
{
    Profiler::start("MultiClassAdaBoost::test()");
    if(!forward_test){
         inherited::test(testset,test_stats,testoutputs,testcosts);
         Profiler::end("MultiClassAdaBoost::test()");
         return;
    }

    if(last_stage<stage && time_sum>0){
        time_last_stage=time_sum;
        time_sum=0;
    }
    if(last_stage<stage && time_sum_rtr>0){
        time_last_stage_rtr=time_sum_rtr;
        time_sum_rtr=0;
    }

    if(forward_test==2 && time_last_stage<time_last_stage_rtr){
        EXTREME_MODULE_LOG<<"inherited start time_sum="<<time_sum<<" time_sum_rtr="<<time_sum_rtr<<" last_stage="<<last_stage <<" stage=" <<stage <<" time_last_stage=" <<time_last_stage<<" time_last_stage_rtr=" <<time_last_stage_rtr<<endl;
        Profiler::reset("MultiClassAdaBoost::test() current");
        Profiler::start("MultiClassAdaBoost::test() current");
        PLCHECK(last_stage<=stage);
        inherited::test(testset,test_stats,testoutputs,testcosts);
        Profiler::end("MultiClassAdaBoost::test() current");
        Profiler::end("MultiClassAdaBoost::test()");
        time_sum += Profiler::getStats("MultiClassAdaBoost::test() current").wall_duration;
        last_stage=stage;
        EXTREME_MODULE_LOG<<"inherited end time_sum="<<time_sum<<" time_sum_rtr="<<time_sum_rtr<<" last_stage="<<last_stage <<" stage=" <<stage <<" time_last_stage=" <<time_last_stage<<" time_last_stage_rtr=" <<time_last_stage_rtr<<endl;
        return;
    }
    EXTREME_MODULE_LOG<<"start time_sum="<<time_sum<<" time_sum_rtr="<<time_sum_rtr<<" last_stage="<<last_stage <<" stage=" <<stage <<" time_last_stage=" <<time_last_stage<<" time_last_stage_rtr=" <<time_last_stage_rtr<<endl;
    Profiler::reset("MultiClassAdaBoost::test() current");
    Profiler::start("MultiClassAdaBoost::test() current");
    //Profiler::pl_profile_start("MultiClassAdaBoost::test() part1");//cheap
    int index=-1;
    for(int i=0;i<saved_testset.length();i++){
        if(saved_testset[i]==testset){
            index=i;break;
        }
    }
    PP<VecStatsCollector> test_stats1 = 0;
    PP<VecStatsCollector> test_stats2 = 0;
    VMat testoutputs1 = VMat(new MemoryVMatrix(testset->length(),
                                               learner1->outputsize()));
    VMat testoutputs2 = VMat(new MemoryVMatrix(testset->length(),
                                               learner2->outputsize()));
    VMat testcosts1 = 0;
    VMat testcosts2 = 0;
    VMat testset1 = 0;
    VMat testset2 = 0;
    if ((testcosts || test_stats )&& forward_sub_learner_test_costs){
        //comment
        testcosts1 = VMat(new MemoryVMatrix(testset->length(),
                                            learner1->nTestCosts()));
        testcosts2 = VMat(new MemoryVMatrix(testset->length(),
                                            learner2->nTestCosts()));
    }
    if(index<0){
        testset1 = new ProcessingVMatrix(testset, input_prg,
                                         target_prg1,  weight_prg);
        testset2 = new ProcessingVMatrix(testset, input_prg,
                                         target_prg2,  weight_prg);
        saved_testset.append(testset);
        saved_testset1.append(testset1);
        saved_testset2.append(testset2);
    }else{
        //we need to do that as AdaBoost need 
        //the same dataset to reuse their test results
        testset1=saved_testset1[index];
        testset2=saved_testset2[index];
        PLCHECK(((PP<ProcessingVMatrix>)testset1)->source==testset);
        PLCHECK(((PP<ProcessingVMatrix>)testset2)->source==testset);
    }

    if (test_stats){
        
    }
    //Profiler::pl_profile_end("MultiClassAdaBoost::test() part1");//cheap
    Profiler::start("MultiClassAdaBoost::test() subtest");
    learner1->test(testset1,test_stats1,testoutputs1,testcosts1);
    learner2->test(testset2,test_stats2,testoutputs2,testcosts2);
    Profiler::end("MultiClassAdaBoost::test() subtest");

    VMat my_outputs = 0;
    VMat my_costs = 0;
    if(testoutputs){
        my_outputs=testoutputs;
    }else if(bool(testcosts) | bool(test_stats)){
        my_outputs=VMat(new MemoryVMatrix(testset->length(),
                                          outputsize()));
    }
    if(testcosts){
        my_costs=testcosts;
    }else if(test_stats){
        my_costs=VMat(new MemoryVMatrix(testset->length(),
					nTestCosts()));
    }
    Profiler::pl_profile_start("MultiClassAdaBoost::test() my_outputs");
    if(my_outputs){
        for(int row=0;row<testset.length();row++){
            real out1=testoutputs1->get(row,0);
            real out2=testoutputs2->get(row,0);
            int ind1=int(round(out1));
            int ind2=int(round(out2));
            int ind=-1;
            if(ind1==0 && ind2==0)
                ind=0;
            else if(ind1==1 && ind2==0)
                ind=1;
            else if(ind1==1 && ind2==1)
                ind=2;
            else
                ind=1;//TODOself.confusion_target;
            tmp_output[0]=ind;
            tmp_output[1]=out1;
            tmp_output[2]=out2;
	    my_outputs->putOrAppendRow(row,tmp_output);
	}
    }
    Profiler::pl_profile_end("MultiClassAdaBoost::test() my_outputs");
    Profiler::pl_profile_start("MultiClassAdaBoost::test() my_costs");

    if (my_costs){
        tmp_costs.resize(nTestCosts());
//        if (forward_sub_learner_test_costs)
	    //TODO optimize by reusing testoutputs1 and testoutputs2
            //            PLWARNING("will be long");
	int target_index = testset->inputsize();
	PLASSERT(testset->targetsize()==1);
        Vec costs1,costs2;
        if(forward_sub_learner_test_costs){
            costs1.resize(learner1->nTestCosts());
            costs2.resize(learner2->nTestCosts());
        }
        for(int row=0;row<testset.length();row++){
            //default version
            //testset.getExample(row, input, target, weight);
	    //computeCostsFromOutputs(input,my_outputs(row),target,costs);
            
            //the input is not needed for the cost of this class if the subcost are know.
            testset->getSubRow(row,target_index,tmp_target);
//	    Vec costs1=testcosts1(row);
//	    Vec costs2=testcosts2(row);
            if(forward_sub_learner_test_costs){
                testcosts1->getRow(row,costs1);
                testcosts2->getRow(row,costs2);
            }
            //TODO??? tmp_input is empty!!!
	    computeCostsFromOutputs_(tmp_input, my_outputs(row), tmp_target, costs1,
                                     costs2, tmp_costs);
	    my_costs->putOrAppendRow(row,tmp_costs);
        }
    }
    Profiler::pl_profile_end("MultiClassAdaBoost::test() my_costs");
    Profiler::pl_profile_start("MultiClassAdaBoost::test() test_stats");

    if (test_stats){
	if(testset->weightsize()==0){
            for(int row=0;row<testset.length();row++){
                Vec costs = my_costs(row);
                test_stats->update(costs, 1);
            }
	}else{
            int weight_index=inputsize()+targetsize();
            Vec costs(my_costs.width());
            for(int row=0;row<testset.length();row++){
//                Vec costs = my_costs(row);
                my_costs->getRow(row, costs);
                test_stats->update(costs, testset->get(row, weight_index));
            }
	}
    }
    Profiler::pl_profile_end("MultiClassAdaBoost::test() test_stats");
    Profiler::end("MultiClassAdaBoost::test() current");
    Profiler::end("MultiClassAdaBoost::test()");
    
    time_sum_rtr += Profiler::getStats("MultiClassAdaBoost::test() current").wall_duration;

    last_stage=stage;
    EXTREME_MODULE_LOG<<"end time_sum="<<time_sum<<" time_sum_rtr="<<time_sum_rtr<<" last_stage="<<last_stage <<" stage=" <<stage <<" time_last_stage=" <<time_last_stage<<" time_last_stage_rtr=" <<time_last_stage_rtr<<endl;

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
