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
   * $Id: EmbeddedLearner.cc,v 1.1 2003/02/28 00:12:37 plearner Exp $ 
   ******************************************************* */

/*! \file EmbeddedLearner.cc */
#include "EmbeddedLearner.h"

namespace PLearn <%
using namespace std;


// ###### EmbeddedLearner ######################################################

IMPLEMENT_NAME_AND_DEEPCOPY(EmbeddedLearner);

EmbeddedLearner::EmbeddedLearner(PP<Learner> the_learner)
    : Learner(the_learner.isNull() ? 0 : the_learner->inputsize(),
              the_learner.isNull() ? 0 : the_learner->targetsize(),
              the_learner.isNull() ? 0 : the_learner->outputsize()),
      learner(the_learner)
{
}


void EmbeddedLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "learner", &EmbeddedLearner::learner, OptionBase::buildoption,
                  "The associated learner");
    inherited::declareOptions(ol);
}

string EmbeddedLearner::help() const
{
    return 
        "EmbeddedLearner implements nothing but forward"
        "calls to an underlying learner."
        + optionHelp();
}

void EmbeddedLearner::build_()
{
    if (learner.isNull())
        PLERROR("EmbeddedLearner::_build() - learner attribut is NUL");

    learner->build();

    inputsize_ = learner->inputsize();
    outputsize_ = learner->outputsize();
    targetsize_ = learner->targetsize();

    setTestCostFunctions(learner->test_costfuncs);
    setTestStatistics(learner->test_statistics);
}

void EmbeddedLearner::build()
{
    inherited::build();
    build_();
}

void EmbeddedLearner::train(VMat training_set)
{ 
    if(training_set->width() != inputsize()+targetsize())
        PLERROR("In EmbeddedLearner::train(VMat training_set) training_set->width() != inputsize()+targetsize()");

    learner->train(training_set);
}

void EmbeddedLearner::use(const Vec& input, Vec& output)
{
    learner->use(input, output);
}

void EmbeddedLearner::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
    Learner::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);
    
    // ### Remove this line when you have fully implemented this method.
    PLERROR("EmbeddedLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void
EmbeddedLearner::setLearner(PP<Learner> the_learner)
{
    learner = the_learner;
    build();
}

%> // end of namespace PLearn
