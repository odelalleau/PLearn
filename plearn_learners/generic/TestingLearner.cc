// -*- C++ -*-

// TestingLearner.cc
//
// Copyright (C) 2004 Marius Muja 
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
   * $Id: TestingLearner.cc,v 1.4 2005/02/08 21:54:23 tihocan Exp $ 
   ******************************************************* */

// Authors: Marius Muja

/*! \file TestingLearner.cc */


#include "TestingLearner.h"

namespace PLearn {
using namespace std;

TestingLearner::TestingLearner() 
{
  // ...

  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

PLEARN_IMPLEMENT_OBJECT(TestingLearner, "ONE LINE DESCRIPTION", "MULTI-LINE \nHELP");

void TestingLearner::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

 
  declareOption(ol, "tester", &TestingLearner::tester, OptionBase::buildoption,
                 "The tester used by the TestingLearner.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void TestingLearner::build_()
{
}

// ### Nothing to add here, simply calls build_
void TestingLearner::build()
{
  inherited::build();
  build_();
}


void TestingLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("TestingLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int TestingLearner::outputsize() const
{
  return tester->learner->outputsize();
}

void TestingLearner::forget()
{
  stage = 0;
}
    
void TestingLearner::train()
{
  if (stage > 0) {
    PLWARNING("In TestingLearner::train - Learner has already been trained");
    return;
  }
  train_stats->update(tester->perform(true));
  train_stats->setFieldNames(tester->getStatNames());
  stage = 1;
}


void TestingLearner::computeOutput(const Vec& input, Vec& output) const
{
  // Compute the output from the input.
  // int nout = outputsize();
  // output.resize(nout);
  // ...
}    

void TestingLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output. 
// ...
}                                

TVec<string> TestingLearner::getTestCostNames() const
{
  TVec<string> result;

  return result;
}

TVec<string> TestingLearner::getTrainCostNames() const
{
  return tester->getStatNames();
}

void TestingLearner::setTrainingSet(VMat training_set, bool call_forget)
{
  inherited::setTrainingSet(training_set, call_forget);
  tester->dataset = training_set;
}


void TestingLearner::setExperimentDirectory(const PPath& the_expdir)
{
  tester->setExperimentDirectory(the_expdir);
}

} // end of namespace PLearn
