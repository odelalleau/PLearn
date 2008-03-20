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

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    MultiClassAdaBoost,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP");

MultiClassAdaBoost::MultiClassAdaBoost():
    nb_stage_to_use(-1),
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
    declareOption(ol, "nb_stage_to_use",
                  &MultiClassAdaBoost::nb_stage_to_use,
                  OptionBase::buildoption,
                  "The number of stage to use when testing."
                  " Can be lower then the number of trained stage,"
                  " but can't be higher!");
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
    
    PLWARNING("In MultiClassAdaBoost::forget() - not implemented, training not implemented");
}

void MultiClassAdaBoost::train()
{
    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    /* TYPICAL CODE:

    static Vec input;  // static so we don't reallocate memory each time...
    static Vec target; // (but be careful that static means shared!)
    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    real weight;

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    while(stage<nstages)
    {
        // clear statistics of previous epoch
        train_stats->forget();

        //... train for 1 stage, and update train_stats,
        // using train_set->getExample(input, target, weight)
        // and train_stats->update(train_costs)

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
    */
    PLWARNING("In MultiClassAdaBoost::train() - not implemented, should be already trained");
}

void MultiClassAdaBoost::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(outputsize());

    Vec tmp1(learner1.outputsize());
    Vec tmp2(learner2.outputsize());
    if(nb_stage_to_use!=-1){
        learner1.computeOutput(input,tmp1,nb_stage_to_use);
        learner2.computeOutput(input,tmp2,nb_stage_to_use);
    }else{
        learner1.computeOutput(input,tmp1);
        learner2.computeOutput(input,tmp2);
    }
    int ind1=int(round(tmp1[0]));
    int ind2=int(round(tmp2[0]));
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
    output[1]=tmp1[0];
    output[2]=tmp2[0];
}
void MultiClassAdaBoost::computeOutputAndCosts(const Vec& input,
                                               const Vec& target,
                                               Vec& output, Vec& costs) const
{
    PLASSERT(costs.size()==nTestCosts());

    output.resize(outputsize());

    Vec output1(learner1.outputsize());
    Vec output2(learner2.outputsize());
    Vec subcosts1(learner1.nTestCosts());
    Vec subcosts2(learner1.nTestCosts());

    learner1.computeOutputAndCosts(input, target, output1, subcosts1);
    learner2.computeOutputAndCosts(input, target, output2, subcosts2);

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
        Vec subcosts1(learner1.nTestCosts());
        Vec subcosts2(learner1.nTestCosts());
        Vec target1(0,1), target2(0,1);
        if(fast_is_equal(target[0],0.)){
            target1.append(0);
            target2.append(0);
        }else if(fast_is_equal(target[0],1.)){
            target1.append(1);
            target2.append(0);
        }else if(fast_is_equal(target[0],2.)){
            target1.append(1);
            target2.append(1);
        }
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
    PLASSERT(nTestCosts()==costs.size());
    if(forward_sub_learner_test_costs){
        costs.resize(7);
        Vec subcosts1(learner1.nTestCosts());
        Vec subcosts2(learner1.nTestCosts());
        Vec target1(0,1), target2(0,1);
        if(fast_is_equal(target[0],0.)){
            target1.append(0);
            target2.append(0);
        }else if(fast_is_equal(target[0],1.)){
            target1.append(1);
            target2.append(0);
        }else if(fast_is_equal(target[0],2.)){
            target1.append(1);
            target2.append(1);
        }
        learner1.computeCostsOnly(input,target1,subcosts1);
        learner2.computeCostsOnly(input,target2,subcosts2);
        subcosts1+=subcosts2;
        costs.append(subcosts1);
    }

    PLASSERT(costs.size()==nTestCosts());
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
        TVec<string> subcosts=learner1.getTestCostNames();
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
