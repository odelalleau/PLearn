// -*- C++ -*-

// EmbeddedLearner.cc
// 
// Copyright (C) 2002 Frederic Morin
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

/* *******************************************************      
   * $Id: EmbeddedLearner.cc,v 1.2 2003/04/29 21:33:46 plearner Exp $ 
   ******************************************************* */

/*! \file EmbeddedLearner.cc */
#include "EmbeddedLearner.h"

namespace PLearn <%
using namespace std;


// ###### EmbeddedLearner ######################################################

IMPLEMENT_NAME_AND_DEEPCOPY(EmbeddedLearner);

void EmbeddedLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "learner", &EmbeddedLearner::learner, OptionBase::buildoption,
                  "The underlying learner");
    inherited::declareOptions(ol);
}

string EmbeddedLearner::help() const
{
    return 
        "EmbeddedLearner implements nothing but forwarding \n"
        "calls to an underlying learner. It is typically used as\n"
      "baseclass for learners that are built on top of another learner"
      + optionHelp();
}

void EmbeddedLearner::build_()
{
    if (learner.isNull())
        PLERROR("EmbeddedLearner::_build() - learner attribute is NUL");

    learner->build();

    inputsize_ = learner->inputsize();
    outputsize_ = learner->outputsize();
    targetsize_ = learner->targetsize();
    weightsize_ = learner->targetsize();\
}

void EmbeddedLearner::build()
{
    inherited::build();
    build_();
}

   
 void EmbeddedLearner::forget()
{ learner->forget(); }

 void EmbeddedLearner::newtrain(VecStatsCollector& train_stats)
{ learner->newtrain(VecStatsCollector& train_stats); }
    
 void EmbeddedLearner::newtest(VMat testset, VecStatsCollector& test_stats, 
                         VMat testoutputs=0, VMat testcosts=0)
{ learner->newtest(testset, test_stats, testoutputs, testcosts); }

 void EmbeddedLearner::computeOutput(const VVec& input, Vec& output)
{ learner->computeOutput(input, output); }

 void EmbeddedLearner::computeCostsFromOutputs(const VVec& input, const Vec& output, 
                                         const VVec& target, const VVec& weight,
                                         Vec& costs)
{ learner->computeCostsFromOutputs(input, output, target, weight, costs); }

                                
 void EmbeddedLearner::computeOutputAndCosts(const VVec& input, VVec& target, const VVec& weight,
                                       Vec& output, Vec& costs)
{ learner->computeOutputAndCosts(input, target, weight, output, costs); }

 void EmbeddedLearner::computeCostsOnly(const VVec& input, VVec& target, VVec& weight, 
                              Vec& costs)
{ learner->computeCostsOnly(input, target, weight, costs); }
    
Array<string> EmbeddedLearner::costNames() const
{ return learner->costNames(); } 

Array<string> EmbeddedLearner::trainObjectiveNames() const
{ return learner->trainObjectiveNames(); }


void EmbeddedLearner::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
    PLearner::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    deepCopyField(learner, copies);    
}

%> // end of namespace PLearn
