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
   * $Id: LearnerCommand.cc,v 1.10 2005/01/04 21:24:06 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file LearnerCommand.cc */


#include "LearnerCommand.h"
#include <plearn_learners/generic/PLearner.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/db/getDataSet.h>
#include <plearn/io/load_and_save.h>

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
                  "learner compute_outputs <trained_learner.psave> <test_inputs.vmat> <outputs.pmat> (there is 'learner co' as a shortcut for compute_outputs)\n"
                  // "learner compute_costs <trained_learner.psave> <testset.vmat> <outputs.pmat> <costs.pmat>\n" 
                  "learner compute_outputs_on_1D_grid <trained_learner.psave> <gridoutputs.pmat> <xmin> <xmax> <nx> (shortcut: learner cg1)\n"
                  "  -> Computes output of learner on nx equally spaced points in range [xmin, xmax] and writes the list of (x,output) in gridoutputs.pmat \n"
                  "learner compute_outputs_on_2D_grid <trained_learner.psave> <gridoutputs.pmat> <xmin> <xmax> <ymin> <ymax> <nx> <ny> (shortcut: learner cg2)\n"
                  "  -> Computes output of learner on the regular 2d grid specified and writes the list of (x,y,output) in gridoutputs.pmat \n"
                  "learner compute_outputs_on_auto_grid <trained_learner.psave> <gridoutputs.pmat> <trainset.vmat> <nx> [<ny>] (shortcut: learner cg)\n"
                  "  -> Automatically determines a bounding-box from the trainset (enlarged by 5%), and computes the output along a regular 1D grid of <nx> points or a regular 2D grid of <nx>*<ny> points. (Note: you can also invoke command vmat bbox to determine the bounding-box by yourself, and then invoke learner cg1 or learner cg2 appropriately)\n"
                  "\nThe datasets do not need to be .vmat they can be any valid vmatrix (.amat .pmat .dmat)"
                  ) 
  {}

void LearnerCommand::train(const string& learner_spec_file, const string& trainset_spec, const string& save_learner_file)
{


    PP<PLearner> learner;
    PLearn::load(learner_spec_file,learner);
    VMat trainset = getDataSet(trainset_spec);
    PP<VecStatsCollector> train_stats = new VecStatsCollector();
    learner->setTrainStatsCollector(train_stats);
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
  test_stats->build();
  test_stats->forget();
  learner->test(testset, test_stats, testoutputs, testcosts);
  test_stats->finalize();

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

void LearnerCommand::compute_outputs_on_2D_grid(const string& trained_learner_file, const string& grid_outputs_file, 
                                                real xmin, real xmax, real ymin, real ymax,
                                                int nx, int ny)
{
  if(nx<2 || ny<2)
    PLERROR("In LearnerCommand::compute_outputs_on_2D_grid invalid nx or ny. Must have at least a 2x2 grid");
  PP<PLearner> learner;
  PLearn::load(trained_learner_file,learner);
  if(learner->inputsize()!=2)
    PLERROR("In LearnerCommand::compute_outputs_on_2D_grid learner must have inputsize==2 (it's %d)",learner->inputsize());
  int outputsize = learner->outputsize();
  VMat gridoutputs = new FileVMatrix(grid_outputs_file,nx*ny,2+outputsize);
  real deltax = (xmax-xmin)/(nx-1);
  real deltay = (ymax-ymin)/(ny-1);

  Vec v(2+outputsize);
  Vec input = v.subVec(0,2);
  Vec output = v.subVec(2,outputsize);

  real x = xmin;
  for(int i=0; i<nx; i++, x+=deltax)
    {
        input[0] = x;
        real y = ymin;
        for(int j=0; j<ny; j++, y+=deltay)
          {
            input[1] = y;
            learner->computeOutput(input,output);
            gridoutputs->appendRow(v);
          }
    }
  
}

void LearnerCommand::compute_outputs_on_1D_grid(const string& trained_learner_file, const string& grid_outputs_file, 
                                                real xmin, real xmax, int nx)
{
  if(nx<2)
    PLERROR("In LearnerCommand::compute_outputs_on_1D_grid invalid nx. Must be at least 2");
  PP<PLearner> learner;
  PLearn::load(trained_learner_file,learner);
  if(learner->inputsize()!=1)
    PLERROR("In LearnerCommand::compute_outputs_on_1D_grid learner must have inputsize==1 (it's %d)",learner->inputsize());
  int outputsize = learner->outputsize();
  VMat gridoutputs = new FileVMatrix(grid_outputs_file,nx,1+outputsize);
  real deltax = (xmax-xmin)/(nx-1);

  Vec v(1+outputsize);
  Vec input = v.subVec(0,1);
  Vec output = v.subVec(1,outputsize);
  
  real x=xmin;
  for(int i=0; i<nx; i++, x+=deltax)
    {
        input[0] = x;
        learner->computeOutput(input,output);
        gridoutputs->appendRow(v);
    }  
}

void LearnerCommand::compute_outputs_on_auto_grid(const string& trained_learner_file, const string& grid_outputs_file, 
                                                  const string& dataset_spec, real extra_percent,
                                                  int nx, int ny)
{
  TVec< pair<real,real> > bbox = getDataSet(dataset_spec)->getBoundingBox(extra_percent);
  if(ny>0)
    compute_outputs_on_2D_grid(trained_learner_file, grid_outputs_file, 
                               bbox[0].first, bbox[0].second, bbox[1].first, bbox[1].second,
                               nx, ny);
  else
    compute_outputs_on_1D_grid(trained_learner_file, grid_outputs_file, 
                               bbox[0].first, bbox[0].second,
                               nx);
}


//! The actual implementation of the 'LearnerCommand' command 
void LearnerCommand::run(const vector<string>& args)
{
  string command = args[0];
  if(command=="train")
    {
        if (args.size()==4)
            train(args[1],args[2],args[3]);
        else 
            PLERROR("LearnerCommand::run you must provide 'plearn learner train learner_spec_file trainset_spec save_learner_file'");
    }
  else if(command=="test")    
    {
      if (args.size()>3)
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
      else
        PLERROR("LearnerCommand::run you must provide at least 'plearn learner test <trained_learner.psave> <testset.vmat> <cost.stats>'");
    }
  else if ((command=="compute_outputs") ||(command=="co"))
    {
      if (args.size()==4)
        compute_outputs(args[1],args[2],args[3]);
      else
        PLERROR("LearnerCommand::run you must provide 'plearn learner compute_outputs learner_spec_file trainset_spec save_learner_file'");
    }
  else if (command=="compute_outputs_on_1D_grid" || command=="cg1")
    {
      if(args.size()!=6)
        PLERROR("Subcommand learner compute_outputs_on_1D_grid requires 5 arguments. Check the help!");
      compute_outputs_on_1D_grid(args[1], args[2], toreal(args[3]), toreal(args[4]), toint(args[5]));
    }
  else if (command=="compute_outputs_on_2D_grid" || command=="cg2")
    {
      if(args.size()!=9)
        PLERROR("Subcommand learner compute_outputs_on_2D_grid requires 8 arguments. Check the help!");
      compute_outputs_on_2D_grid(args[1], args[2], 
                                 toreal(args[3]), toreal(args[4]),
                                 toreal(args[5]), toreal(args[6]),
                                 toint(args[7]), toint(args[8]) );
    }
  else if (command=="compute_outputs_on_auto_grid" || command=="cg")
    {
      if(args.size()<5)
        PLERROR("Subcommand learner compute_outputs_on_auto_grid requires 4 or 5 arguments. Check the help!");
      int nx = toint(args[4]);
      int ny = 0;
      if(args.size()==6)
        ny = toint(args[5]);      
      compute_outputs_on_auto_grid(args[1], args[2],
                                   args[3], 0.05,
                                   nx, ny);
    }
  else
    PLERROR("Invalid command %s check the help for available commands",command.c_str());
}

} // end of namespace PLearn

