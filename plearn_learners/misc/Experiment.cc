// -*- C++ -*-

// Experiment.cc
// 
// Copyright (C) 2002 Pascal Vincent, Frederic Morin
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
   * $Id: Experiment.cc,v 1.1 2002/09/10 23:32:22 plearner Exp $ 
   ******************************************************* */

/*! \file Experiment.cc */
#include "Experiment.h"

namespace PLearn <%
using namespace std;

Experiment::Experiment() 
  :Object()
  {}

  IMPLEMENT_NAME_AND_DEEPCOPY(Experiment);

  void Experiment::declareOptions(OptionList& ol)
  {
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &Experiment::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    
    declareOption(ol, "expdir", &Experiment::expdir, OptionBase::buildoption,
                  "Path of this experiment's directory in which to save all experiment results (will be created if it does not already exist)");
    declareOption(ol, "learner", &Experiment::learner, OptionBase::buildoption,
                  "The learner to train/test");
    declareOption(ol, "dataset", &Experiment::dataset, OptionBase::buildoption,
                  "The dataset to use for training/testing (will be split according to what is specified in the testmethod)");
    declareOption(ol, "testmethod", &Experiment::testmethod, OptionBase::buildoption,
                  "The testmethod to use. This will typically an instance of TestMethod, built with a specific Splitter,\n"
                  "such as TrainTestSplitter or KFoldSplitter... Ask for help about TestMethod and Splitter for more info.");
    inherited::declareOptions(ol);
  }

  string Experiment::help() const
  {
    return 
      "The Experiment class allows you to describe a typical learning experiment that you wish to perform, \n"
      "as a training/testing of a learning algorithm on a particular dataset.\n"
      + optionHelp();
  }

  void Experiment::build_()
  {
    // not much to build for now
  }

  // ### Nothing to add here, simply calls build_
  void Experiment::build()
  {
    inherited::build();
    build_();
  }

void Experiment::run()
{
  if(expdir=="")
    PLERROR("In experiment::run, experiment directory not set.");
  if(!force_mkdir(const string& expdir))
    PLERROR("Could not create experiment directory %s", epxdir.c_str());

  
}

%> // end of namespace PLearn
