// -*- C++ -*-

// SequentialValidation.cc
//
// Copyright (C) 2003 Rejean Ducharme, Yoshua Bengio
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



#include "SequentialValidation.h"
#include "VecStatsCollector.h"

namespace PLearn <%
using namespace std;


IMPLEMENT_NAME_AND_DEEPCOPY(SequentialValidation);

SequentialValidation::SequentialValidation()
  : init_train_size(1), expdir(""), save_models(true), save_initial_models(false),
    save_test_outputs(false), save_test_costs(false)
{}

void SequentialValidation::build_()
{}

void SequentialValidation::build()
{
  inherited::build();
  build_();
}

void SequentialValidation::declareOptions(OptionList& ol)
{
  declareOption(ol, "expdir", &SequentialValidation::expdir,
    OptionBase::buildoption, "Path of this experiment's directory in which to save all experiment results (will be created if it does not already exist). \n");

  declareOption(ol, "learner", &SequentialValidation::learner,
    OptionBase::buildoption, "The SequentialLearner to train/test. \n");

  declareOption(ol, "dataset", &SequentialValidation::dataset,
    OptionBase::buildoption, "The dataset to use for training/testing. \n");

  declareOption(ol, "init_train_size", &SequentialValidation::init_train_size,
    OptionBase::buildoption, "Size of the first training set. \n");

  declareOption(ol, "save_model", &SequentialValidation::save_model,
    OptionBase::buildoption, "If true, the final model will be saved in model.psave \n");

  declareOption(ol, "save_initial_model", &SequentialValidation::save_initial_model,
    OptionBase::buildoption, "If true, the initial model will be saved in initial_model.psave. \n");

  declareOption(ol, "save_test_outputs", &SequentialValidation::save_test_outputs,
    OptionBase::buildoption, "If true, the outputs of the tests will be saved in test_outputs.pmat \n");

  declareOption(ol, "save_test_costs", &SequentialValidation::save_test_costs,
    OptionBase::buildoption, "If true, the costs of the tests will be saved in test_costs.pmat \n");
}

void SequentialValidation::help()
{
  return
    "The SequentialValidation class allows you to describe a typical sequential \n"
    "validation experiment that you wish to perform........."
    + optionHelp();
}

void SequentialValidation::run()
{
  if (expdir=="")
    PLERROR("No expdir specified for SequentialValidation.");
  if (!learner)
    PLERROR("No learner specified for SequentialValidation.");

  if(pathexists(expdir))
    PLERROR("Directory (or file) %s already exists. First move it out of the way.", expdir.c_str());

  if(!force_mkdir(expdir))
    PLERROR("Could not create experiment directory %s", expdir.c_str());

  // Save this experiment description in the expdir (buildoptions only)
  /*
  PLearn::save(append_slash(expdir)+"sequential_validation.psave", *this, OptionBase::buildoption);

  for (t=init_train_size;t<=train.length();t++)
    sub_train = train_set.subMatRows(0,t); // excludes t, last training pair is (t-1-horizon,t-1)
  sub_test = train_set.subMatRows(t-1,1+horizon)
     model.train(sub_train)
     model.test(sub_test)

     (output summary statistics of test performances in model.errors)
*/
}

%> // end of namespace PLearn

