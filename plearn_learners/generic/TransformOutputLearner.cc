// -*- C++ -*-

// plearn_learners/generic/TransformOutputLearner.cc
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

/*! \file plearn_learners/generic/TransformOutputLearner.cc */


#include "plearn_learners/generic/TransformOutputLearner.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    TransformOutputLearner,
    "Transform a Learner who give the log probality as output to give the probability as output",
    "MULTI-LINE \nHELP");

TransformOutputLearner::TransformOutputLearner()
    :inherited(),
     output_function(-1),
     warning0(true),
     warning1(true)
{
    forward_test=true;
}

void TransformOutputLearner::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &TransformOutputLearner::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...
    declareOption(ol, "output_function", &TransformOutputLearner::output_function,
                  OptionBase::buildoption,
                  "The operation to do on the output\n"
                  "0: We transform the sublearner log probability output to probability output\n"
                  "1: We transform the sublearner probability output of class to class output\n"
                  "2: We transform the sublearner regression output to class output.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void TransformOutputLearner::build_()
{
    tmp_output2.resize(learner_->outputsize());
}

// ### Nothing to add here, simply calls build_
void TransformOutputLearner::build()
{
    inherited::build();
    build_();
}

void TransformOutputLearner::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input.
    // int nout = outputsize();
    // output.resize(nout);
    // ...
    if(output_function==0){//logprob to prob
        learner_->computeOutput(input,output);
        exp(output,output);
    }else if(output_function==1){//logprob or prob to class
        learner_->computeOutput(input,tmp_output2);
        output[0]=argmax(tmp_output2);
    }else if(output_function==2){//Regression to class v1
        learner_->computeOutput(input,tmp_output2);
        output[0]=int(round(tmp_output2[0]));
//    }else if(output_function==2){//Regression to class v2
//         learner_->computeOutput(input, output);
//         if (multiclass_outputs.length() <= 0) return;
//         real closest_value=multiclass_outputs[0];
//         real margin_to_closest_value=abs(output[0] - multiclass_outputs[0]);
//         for (int value_ind = 1; value_ind < multiclass_outputs.length(); value_ind++)
//         {
//             real v=abs(output[0] - multiclass_outputs[value_ind]);
//             if (v < margin_to_closest_value)
//             {
//                 closest_value = multiclass_outputs[value_ind];
//                 margin_to_closest_value = v;
//             }
//         }
//         output[0] = closest_value;
    }else
        PLERROR("In TransformOutputLearner::computeOutput - unknow output_function %d",output_function);
}

void TransformOutputLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    PLASSERT( learner_ );
    if(output_function==0){
        if(warning0){
            PLWARNING("In TransformOutputLearner::computeCostsFromOutputs - you are loosing precision");
            warning0=false;
        }
        compute_log(output,tmp_output2);
        learner_->computeCostsFromOutputs(input,tmp_output2,target,costs);
    }else if(output_function==1 || output_function==2){
        if(warning1){
            PLWARNING("In TransformOutputLearner::computeCostsFromOutputs - we can't compute the costs from\n"
                      "outputs with output_function %d. We use TransformOutputLearner::computeOutputAndCosts.",output_function);
            warning1=false;
        }
        computeOutputAndCosts(input,target,tmp_output2,costs);
    }else
        PLERROR("In TransformOutputLearner::computeCostsFromOutputs - unknow output_function %d",output_function);
}
void TransformOutputLearner::computeOutputAndCosts(const Vec& input,  const Vec& target,
                                          Vec& output, Vec& costs) const
{
    PLASSERT( learner_ );
    if(output_function==0){//logprob to prob
        learner_->computeOutputAndCosts(input,target,output,costs);
        exp(output,output);
    }else if(output_function==1){//logprob or prob to class
        learner_->computeOutputAndCosts(input,target,tmp_output2,costs);
        output[0]=argmax(output);
    }else if(output_function==2){//Regression to class v1
        learner_->computeOutputAndCosts(input,target,tmp_output2,costs);
        output[0]=int(round(tmp_output2[0]));
    }else
        PLERROR("In TransformOutputLearner::computeOutputAndCosts - unknow output_function %d",output_function);
}

void TransformOutputLearner::test(VMat testset, PP<VecStatsCollector> test_stats,
                    VMat testoutputs, VMat testcosts) const
{
    PLASSERT( learner_ );
    if(output_function==0){
        learner_->test(testset, test_stats, testoutputs, testcosts);
        for(int i = 0;i<testoutputs.length();i++){
            Vec v = testoutputs(i);
            exp(v,v);
        }
    }
    else
        PLERROR("In TransformOutputLearner::test");
}
int TransformOutputLearner::outputsize() const
{
    PLASSERT( learner_ );
    if(output_function==0)
        return learner_->outputsize();
    else if (output_function==1 || output_function==2)
        return 1;
    else
        PLERROR("In TransformOutputLearner::outputsize");
    return -1;
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
