// -*- C++ -*-

// PBPTTTester.cc
// 
// Copyright (C) 2004 Jasmin Lapalme
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

#include "VarLengthPTester.h"
#include "FileVMatrix.h"
#include "BPTT.h"

namespace PLearn {
using namespace std;

VarLengthPTester::VarLengthPTester() 
  : PTester()
{}

PLEARN_IMPLEMENT_OBJECT(VarLengthPTester, "Manages a learning experiment, with training and estimation of generalization error.", 
  "The VarLengthPTester has the same goal of PTester but it is specific to the learner BPTT which allows to"
  " have sequences of variable length. This cause to hava some outputsize variable depending on inputsize."
  "The tester must deal a little differently which the learner.");


void VarLengthPTester::declareOptions(OptionList& ol)
{
  inherited::declareOptions(ol);
}

void VarLengthPTester::build_()
{
}

void VarLengthPTester::build()
{
  inherited::build();
  build_();
}

void VarLengthPTester::run()
{
  /*
    To apply the verify gradient to the var
  *
    BPTT *bptt_learner = dynamic_cast<BPTT*>((PLearner*)learner);
   PP<VecStatsCollector> stcol = new VecStatsCollector();
  stcol->setFieldNames(bptt_learner->getTrainCostNames());
  PP<VecStatsCollector> train_stats = stcol;
  bptt_learner->setTrainStatsCollector(train_stats);
  bptt_learner->setTrainingSet(dataset);
  bptt_learner->build();
  bptt_learner->forget();
  bptt_learner->run();
  */  
  
  perform(true);
}

Vec VarLengthPTester::perform(bool call_forget)
{
  BPTT *bptt_learner = dynamic_cast<BPTT*>((PLearner*)learner);
  if(!bptt_learner)
    PLERROR("The learner specified is not a BPTTLearner or no learner specified for PTester.");
  if(!splitter)
    PLERROR("No splitter specified for PTester");


  int nstats = statnames.length();
  Vec global_result(nstats);

  {

  if(expdir!="")
    {
      // Save this tester description in the expdir
      if(save_initial_tester)
        PLearn::save(append_slash(expdir)+"tester.psave", *this);
    }

  splitter->setDataSet(dataset);

  int nsplits = splitter->nsplits();
  if(nsplits>1)
    call_forget = true;

  TVec<string> testcostnames = bptt_learner->getTestCostNames();
  TVec<string> traincostnames = bptt_learner->getTrainCostNames();

  int nsets = splitter->nSetsPerSplit();

  // Stats collectors for individual sets of a split:
  TVec< PP<VecStatsCollector> > stcol(nsets);
  for(int setnum=0; setnum<nsets; setnum++)
    {
      if (template_stats_collector)
        {
          CopiesMap copies;
          stcol[setnum] = template_stats_collector->deepCopy(copies);
        }
      else
        stcol[setnum] = new VecStatsCollector();

      if(setnum==0)
        stcol[setnum]->setFieldNames(traincostnames);
      else
        stcol[setnum]->setFieldNames(testcostnames);

      stcol[setnum]->build();
      stcol[setnum]->forget();      
    }

  PP<VecStatsCollector> train_stats = stcol[0];
  bptt_learner->setTrainStatsCollector(train_stats);

  // Global stats collector
  PP<VecStatsCollector> global_statscol;
  if (global_template_stats_collector)
  {
    CopiesMap copies;
    global_statscol = global_template_stats_collector->deepCopy(copies);
    global_statscol->build();
    global_statscol->forget();
  }
  else
    global_statscol = new VecStatsCollector();

  // Stat specs
  TVec<StatSpec> statspecs(nstats);
  for(int k=0; k<nstats; k++)
    statspecs[k].init(statnames[k]);
  
  // int traincostsize = traincostnames.size();
  int testcostsize = testcostnames.size();

  VMat global_stats_vm;    // the vmat in which to save global result stats specified in statnames
  VMat split_stats_vm;   // the vmat in which to save per split result stats
  if(expdir!="" && report_stats)
    {
      saveStringInFile(expdir+"train_cost_names.txt", join(traincostnames,"\n")+"\n"); 
      saveStringInFile(expdir+"test_cost_names.txt", join(testcostnames,"\n")+"\n"); 

      global_stats_vm = new FileVMatrix(expdir+"global_stats.pmat", 1, nstats);
      for(int k=0; k<nstats; k++)
        global_stats_vm->declareField(k,statspecs[k].statName());
      global_stats_vm->saveFieldInfos();

      split_stats_vm = new FileVMatrix(expdir+"split_stats.pmat", 0, 1+nstats);
      split_stats_vm->declareField(0,"splitnum");
      for(int k=0; k<nstats; k++)
        split_stats_vm->declareField(k+1,statspecs[k].setname + "." + statspecs[k].intstatname);
      split_stats_vm->saveFieldInfos();
    }

  for(int splitnum=0; splitnum<nsplits; splitnum++)
    {
      string splitdir;
      if(expdir!="")
        splitdir = append_slash(append_slash(expdir)+"Split"+tostring(splitnum));

      TVec<VMat> dsets = splitter->getSplit(splitnum);
      VMat trainset = dsets[0];
      if(splitdir!="" && save_data_sets)
        PLearn::save(splitdir+"training_set.psave",trainset);

      if(splitdir!="" && train && provide_learner_expdir)
        bptt_learner->setExperimentDirectory(append_slash(splitdir+"LearnerExpdir"));

      bptt_learner->setTrainingSet(trainset, call_forget && train);
      if(dsets.size()>1)
        bptt_learner->setValidationSet(dsets[1]);



      if (train)
        {
          if(splitdir!="" && save_initial_learners)
            PLearn::save(splitdir+"initial_learner.psave",bptt_learner);
      
          train_stats->forget();
          bptt_learner->train();
          train_stats->finalize();
          if(splitdir != "" && save_stat_collectors)
            PLearn::save(splitdir+"train_stats.psave",train_stats);
          if(splitdir != "" && save_learners)
            PLearn::save(splitdir+"final_learner.psave",bptt_learner);
        }
      else
        bptt_learner->build();
      for(int setnum=1; setnum<dsets.length(); setnum++)
        {
          VMat testset = dsets[setnum];
	  int outputsize = bptt_learner->outputsize(testset);
          PP<VecStatsCollector> test_stats = stcol[setnum];
          string setname = "test"+tostring(setnum);
          if(splitdir!="" && save_data_sets)
            PLearn::save(splitdir+setname+"_set.psave",testset);
          VMat test_outputs;
          VMat test_costs;
          force_mkdir(splitdir);
          if(splitdir != "" && save_test_outputs)
            test_outputs = new FileVMatrix(splitdir+setname+"_outputs.pmat",0,outputsize);
          if(splitdir != "" && save_test_costs)
            test_costs = new FileVMatrix(splitdir+setname+"_costs.pmat",0,testcostsize);
      
          test_stats->forget();
          if (testset->length()==0) {
            PLWARNING("PTester:: test set % is of length 0, costs will be set to -1",setname.c_str());
          }
          bptt_learner->test(testset, test_stats, test_outputs, test_costs);      
          test_stats->finalize();
          if(splitdir != "" && save_stat_collectors)
            PLearn::save(splitdir+setname+"_stats.psave",test_stats);
        }
   
      Vec splitres(1+nstats);
      splitres[0] = splitnum;

      for(int k=0; k<nstats; k++)
        {
          StatSpec& sp = statspecs[k];
          if (sp.setnum>=stcol.length())
            splitres[k+1] = MISSING_VALUE;
//            PLERROR("PTester::perform, trying to access a test set (test%d) beyond the last one (test%d)",
//                    sp.setnum, stcol.length()-1);
          else
            splitres[k+1] = stcol[sp.setnum]->getStat(sp.intstatname);
        }

      if(split_stats_vm) {
        split_stats_vm->appendRow(splitres);
        split_stats_vm->flush();
      }

      global_statscol->update(splitres.subVec(1,nstats));
    }


  global_statscol->finalize();
  for(int k=0; k<nstats; k++)
    global_result[k] = global_statscol->getStats(k).getStat(statspecs[k].extstat);
  

  if(global_stats_vm)
    global_stats_vm->appendRow(global_result);

  }

  // Perform the final commands provided in final_commands.
  for (int i = 0; i < final_commands.length(); i++) {
    system(final_commands[i].c_str());
  }

  return global_result;
}

} // end of namespace PLearn
