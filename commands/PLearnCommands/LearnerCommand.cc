// -*- C++ -*-

// LearnerCommand.cc
//
// Copyright (C) 2004 Pascal Vincent 
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
   * $Id: LearnerCommand.cc,v 1.3 2004/02/20 21:11:40 chrish42 Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file LearnerCommand.cc */


#include "LearnerCommand.h"
#include "PLearner.h"
//#include "VecStatsCollector.h"
#include "FileVMatrix.h"
#include "getDataSet.h"
//#include "PStream.h"

namespace PLearn {
using namespace std;

//! This allows to register the 'LearnerCommand' command in the command registry
PLearnCommandRegistry LearnerCommand::reg_(new LearnerCommand);

LearnerCommand::LearnerCommand():
    PLearnCommand("learner",

                  "Allows to train, use and test a learner",

                  "learner train <learner_spec.plearn> <trainset.vmat> <trained_learner.psave>\n"
                  "  -> Will train the specified learner on the specified trainset and save the resulting trained learner as trained_learner.psave\n"
                  "learner test <trained_learner.psave> <testset.vmat> <cost.stats> [<outputs.pmat>] [<costs.pmat>]\n"
                  "  -> Tests the specified learner on the testset. Will produce a cost.stats file (viewable with the plearn stats command) and optionally saves individual outputs and costs\n"
                  "learner compute_outputs <trained_learner.psave> <test_inputs.vmat> <outputs.pmat>\n"
                  // "learner compute_costs <trained_learner.psave> <testset.vmat> <outputs.pmat> <costs.pmat>\n" 
                  "The datasets do not need to be .vmat they can be any valid vmatrix (.amat .pmat .dmat)"
                  ) 
  {}


void LearnerCommand::train(const string& learner_spec_file, const string& trainset_spec, const string& save_learner_file)
{
  PP<PLearner> learner;
  PLearn::load(learner_spec_file,learner);
  VMat trainset = getDataSet(trainset_spec);
  learner->setTrainingSet(trainset);
  learner->train();
  PLearn::save(save_learner_file, learner);  
}

void LearnerCommand::test(const string& trained_learner_file, const string& testset_spec, const string& stats_file, const string& outputs_file, const string& costs_file)
{
  PP<PLearner> learner;
  PLearn::load(trained_learner_file,learner);
  VMat testset = getDataSet(testset_spec);
  int l = testset.length();
  VMat testoutputs;
  if(outputs_file!="")
    testoutputs = new FileVMatrix(outputs_file,l,learner->outputsize());
  VMat testcosts;
  if(costs_file!="")
    testcosts = new FileVMatrix(costs_file,l,learner->nTestCosts());

  PP<VecStatsCollector> test_stats;
  learner->test(testset, test_stats, testoutputs, testcosts);

  PLearn::save(stats_file,test_stats);
}

void LearnerCommand::compute_outputs(const string& trained_learner_file, const string& test_inputs_spec, const string& outputs_file)
{
  PP<PLearner> learner;
  PLearn::load(trained_learner_file,learner);
  VMat testinputs = getDataSet(test_inputs_spec);
  int l = testinputs.length();
  VMat testoutputs = new FileVMatrix(outputs_file,l,learner->outputsize());
  learner->use(testinputs,testoutputs);
}

//! The actual implementation of the 'LearnerCommand' command 
void LearnerCommand::run(const vector<string>& args)
{
  string command = args[0];
  if(command=="train")
    train(args[1],args[2],args[3]);
  else if(command=="test")    
    {
      string trained_learner_file = args[1];
      string testset_spec = args[2];
      string stats_basename = args[3];
      string outputs_file;
      if(args.size()>4)
        outputs_file = args[4];
      string costs_file;
      if(args.size()>5)
        costs_file = args[5];
      test(trained_learner_file, testset_spec, stats_basename, outputs_file, costs_file);
    }
  else if(command=="compute_outputs")
    compute_outputs(args[1],args[2],args[3]);
  else
    PLERROR("Invalid command %s check the help for available commands",command.c_str());
}

} // end of namespace PLearn

