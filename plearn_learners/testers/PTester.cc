// -*- C++ -*-

// PTester.cc
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
   * $Id: PTester.cc,v 1.24 2004/02/17 21:07:18 tihocan Exp $ 
   ******************************************************* */

/*! \file PTester.cc */
#include "PTester.h"
//#include "pl_io.h"
#include "VecStatsCollector.h"
//#include "AsciiVMatrix.h"
#include "FileVMatrix.h"

namespace PLearn <%
using namespace std;

TVec<string> addprepostfix(const string& prefix, const TVec<string>& names, const string& postfix)
{
  TVec<string> newnames(names.size());
  TVec<string>::const_iterator it = names.begin();
  TVec<string>::iterator newit = newnames.begin();
  while(it!=names.end())
    {
      *newit = prefix + *it + postfix;
      ++it;
      ++newit;
    }
  return newnames;
}

template<class T> TVec<T> operator&(const T& x, const TVec<T>& v)
{
  int l = v.size();
  TVec<T> res(1+l);
  res[0] = x;
  res.subVec(1,l) << v;
  return res;
}

PTester::PTester() 
  : report_stats(true),
    save_initial_tester(true),
    save_stat_collectors(true),
    save_learners(true),
    save_initial_learners(false),
    save_data_sets(false),
    save_test_outputs(false),
    save_test_costs(false),
    provide_learner_expdir(false),
    train(true)
{}

  PLEARN_IMPLEMENT_OBJECT(PTester, "Manages a learning experiment, with training and estimation of generalization error.", 
      "The PTester class allows you to describe a typical learning experiment that you wish to perform, \n"
      "as a training/testing of a learning algorithm on a particular dataset.\n"
      "The splitter is used to obtain one or several (such as for k-fold) splits of the dataset \n"
      "and training/testing is performed on each split. \n"
      "Requested statistics are computed, and all requested results are written in an appropriate \n"
      "file inside the specified experiment directory. \n");


  void PTester::declareOptions(OptionList& ol)
  {
    declareOption(ol, "expdir", &PTester::expdir, OptionBase::buildoption,
                  "Path of this tester's directory in which to save all tester results.\n"
                  "The directory will be created if it does not already exist.\n"
                  "If this is an empty string, no directory is created and no output file is generated.\n");
    declareOption(ol, "learner", &PTester::learner, OptionBase::buildoption,
                  "The learner to train/test.\n");
    declareOption(ol, "dataset", &PTester::dataset, OptionBase::buildoption,
                  "The dataset to use to generate splits. \n"
                  "(This is ignored if your splitter is an ExplicitSplitter)\n"
                  "Data-sets are seen as matrices whose columns or fields are layed out as \n"
                  "follows: a number of input fields, followed by (optional) target fields, \n"
                  "followed by a (optional) weight field (to weigh each example).\n"
                  "The sizes of those areas are given by the VMatrix options \n"
                  "inputsize targetsize, and weightsize, which are typically used by the \n"
                  "learner upon building\n");
    declareOption(ol, "splitter", &PTester::splitter, OptionBase::buildoption,
                  "The splitter to use to generate one or several train/test tuples from the dataset.");
    declareOption(ol, "statnames", &PTester::statnames, OptionBase::buildoption,
                  "A list of global statistics we are interested in.\n"
                  "These are strings of the form S1[S2[dataset.cost_name]] where:\n"
                  "  - dataset is train or test1 or test2 ... (train being \n"
                  "    the first dataset in a split, test1 the second, ...) \n"
                  "  - cost_name is one of the training or test cost names (depending on dataset) understood \n"
                  "    by the underlying learner (see its getTrainCostNames and getTestCostNames methods) \n" 
                  "  - S1 and S2 are a statistic, i.e. one of: E (expectation), V(variance), MIN, MAX, STDDEV, ... \n"
                  "    S2 is computed over the samples of a given dataset split. S1 is over the splits. \n");
    declareOption(ol, "report_stats", &PTester::report_stats, OptionBase::buildoption,
                  "If true, the computed global statistics specified in statnames will be saved in global_stats.pmat \n"
                  "and the corresponding per-split statistics will be saved in split_stats.pmat \n"
                  "For reference, all cost names (as given by the learner's getTrainCostNames() and getTestCostNames() ) \n"
                  "will be reported in files train_cost_names.txt and test_cost_names.txt");
    declareOption(ol, "save_initial_tester", &PTester::save_initial_tester, OptionBase::buildoption,
                  "If true, this PTester object will be saved in its initial state in tester.psave \n"
                  "Thus if the initial .plearn file gets lost, or modified, we can always see what this tester was.\n");
    declareOption(ol, "save_stat_collectors", &PTester::save_stat_collectors, OptionBase::buildoption,
                  "If true, stat collectors for split#k will be saved in Split#k/train_stats.psave and Split#k/test#i_stats.psave");
    declareOption(ol, "save_learners", &PTester::save_learners, OptionBase::buildoption,
                  "If true, the final trained learner for split#k will be saved in Split#k/final_learner.psave");
    declareOption(ol, "save_initial_learners", &PTester::save_initial_learners, OptionBase::buildoption,
                  "If true, the initial untrained learner for split#k (just after forget() has been called) will be saved in Split#k/initial_learner.psave");
    declareOption(ol, "save_data_sets", &PTester::save_data_sets, OptionBase::buildoption,
                  "If true, the data set generated for split #k will be saved as Split#k/training_set.psave Split#k/test1_set.psave ...");
    declareOption(ol, "save_test_outputs", &PTester::save_test_outputs, OptionBase::buildoption,
                  "If true, the outputs of the test for split #k will be saved in Split#k/test#i_outputs.pmat");
    declareOption(ol, "save_test_costs", &PTester::save_test_costs, OptionBase::buildoption,
                  "If true, the costs of the test for split #k will be saved in Split#k/test#i_costs.pmat");
    declareOption(ol, "provide_learner_expdir", &PTester::provide_learner_expdir, OptionBase::buildoption,
                  "If true, each learner to be trained will have its experiment directory set to Split#k/LearnerExpdir/");
    declareOption(ol, "train", &PTester::train, OptionBase::buildoption,
                  "If true, the learners are trained, otherwise only tested (in that case it is advised\n"
                  "to load an already trained learner in the 'learner' field");
    declareOption(ol, "template_stats_collector", &PTester::template_stats_collector, OptionBase::buildoption,
                  "If provided, this instance of a subclass of VecStatsCollector will be used as a template\n"
                  "to build all the stats collector used during training and testing of the learner");
    declareOption(ol, "global_template_stats_collector", &PTester::global_template_stats_collector, OptionBase::buildoption,
                  "If provided, this instance of a subclass of VecStatsCollector will be used as a template\n"
                  "to build all the global stats collector that collects statistics over splits");
    declareOption(ol, "final_commands", &PTester::final_commands, OptionBase::buildoption,
                  "If provided, the shell commands given will be executed after training is completed");
    inherited::declareOptions(ol);
  }

void PTester::build_()
{
  if(expdir!="")
    {
      if(pathexists(expdir))
        PLERROR("Directory (or file) %s already exists. First move it out of the way.",expdir.c_str());
      if(!force_mkdir(expdir))
        PLERROR("In PTester Could not create experiment directory %s",expdir.c_str());
      expdir = abspath(expdir);
    }
}

  // ### Nothing to add here, simply calls build_
  void PTester::build()
  {
    inherited::build();
    build_();
  }

void PTester::run()
{
  perform(true);
}

void PTester::setExperimentDirectory(const string& the_expdir) 
{ 
  if(the_expdir=="")
    expdir = "";
  else
    {
      if(!force_mkdir(the_expdir))
        PLERROR("In PTester::setExperimentDirectory Could not create experiment directory %s",the_expdir.c_str());
      expdir = abspath(the_expdir);
    }
}

Vec PTester::perform(bool call_forget)
{
  if(!learner)
    PLERROR("No learner specified for PTester.");
  if(!splitter)
    PLERROR("No splitter specified for PTester");

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

  TVec<string> testcostnames = learner->getTestCostNames();
  TVec<string> traincostnames = learner->getTrainCostNames();

  int nsets = splitter->nSetsPerSplit();
  int nstats = statnames.length();

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
  learner->setTrainStatsCollector(train_stats);

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
        learner->setExperimentDirectory(append_slash(splitdir+"LearnerExpdir"));

      learner->setTrainingSet(trainset, call_forget && train);
      if(dsets.size()>1)
        learner->setValidationSet(dsets[1]);

      int outputsize = learner->outputsize();


      if (train)
        {
          if(splitdir!="" && save_initial_learners)
            PLearn::save(splitdir+"initial_learner.psave",learner);
      
          train_stats->forget();
          learner->train();
          train_stats->finalize();
          if(splitdir != "" && save_stat_collectors)
            PLearn::save(splitdir+"train_stats.psave",train_stats);
          if(splitdir != "" && save_learners)
            PLearn::save(splitdir+"final_learner.psave",learner);
        }
      else
        learner->build();
      for(int setnum=1; setnum<dsets.length(); setnum++)
        {
          VMat testset = dsets[setnum];
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
          learner->test(testset, test_stats, test_outputs, test_costs);      
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


  Vec global_result(nstats);

  global_statscol->finalize();
  for(int k=0; k<nstats; k++)
    global_result[k] = global_statscol->getStats(k).getStat(statspecs[k].extstat);
  

  if(global_stats_vm)
    global_stats_vm->appendRow(global_result);

  // Perform the final commands provided in final_commands.
  for (int i = 0; i < final_commands.length(); i++) {
    system(final_commands[i].c_str());
  }

  return global_result;
}

void StatSpec::init(const string& statname)
  {
    parseStatname(statname);
  }

/* Old parseStatname
void StatSpec::parseStatname(const string& statname)
{
  string name1=removeallblanks(statname);
  unsigned int p = name1.find("[");
  if (p==string::npos)
    PLERROR("StatSpec::parseStatname(%s): can't find left bracket!",statname.c_str());
  // get first statistic name, which is anything before the [
  extstat = name1.substr(0,p);
  string n1 = name1.substr(p+1);
  PIStringStream ss1(n1);
  string name2="";
  int closing_symbol = ss1.smartReadUntilNext("]", name2);
  if (closing_symbol!=']')
    PLERROR("StatSpec::parseStatname(%s): can't find right bracket corresponding to the first [!",statname.c_str());
  p = name2.find("[");
  if (p==string::npos)
    PLERROR("StatSpec::parseStatname(%s): can't find 2nd left bracket!",statname.c_str());
  intstat = name2.substr(0,p);
  string n2 = name2.substr(p+1);
  PIStringStream ss2(n2);
  string set_and_cost="";
  closing_symbol = ss2.smartReadUntilNext("]", set_and_cost);
  if (closing_symbol!=']')
    PLERROR("StatSpec::parseStatname(%s): can't find right bracket corresponding to the 2nd [!",statname.c_str());

  if(set_and_cost.length()<5)
    PLERROR("In parse_statname: parse error for %s",statname.c_str());

  split_on_first(set_and_cost,".", setname, costname);
  
  if(setname=="train")
    setnum = 0;
  else if(setname=="test")
    setnum = 1;
  else if(setname.substr(0,4)=="test")
    {
      setnum = toint(setname.substr(4));
      if(setnum==0)
        PLERROR("In parse_statname: use the name train instead of test0.\n"
                "The first set of a split is the training set. The following are test sets named test1 test2 ..."); 
      if(setnum<=0)
        PLERROR("In parse_statname: parse error for %s",statname.c_str());        
    }
  else
    PLERROR("In parse_statname: parse error for %s",statname.c_str());
}
*/

void StatSpec::parseStatname(const string& statname)
{
  PIStringStream in(removeblanks(statname));
  if(in.smartReadUntilNext("[", extstat)==EOF)
    PLERROR("No opening bracket found in statname %s", statname.c_str());
  string token;
  int nextsep = in.smartReadUntilNext(".[",token);
  if(nextsep==EOF)
    PLERROR("Expected dataset.xxxSTATxxx after the opening bracket. Got %s", token.c_str());
  else if(nextsep=='[') // Old format (for backward compatibility) ex: E[E[train.mse]]
    {
      intstatname = token;
      if(in.smartReadUntilNext(".",setname)==EOF)
        PLERROR("Error while parsing statname: expected a dot");
      string costname;
      if(in.smartReadUntilNext("]",costname)==EOF)
        PLERROR("Error while parsing statname: expected a closing bracket");
      intstatname = intstatname+"["+costname+"]";
    }
  else // We've read an opening bracket. That's the new format E[train.E[mse]]
    {
      setname = token;
      if(in.smartReadUntilNext("]",intstatname)==EOF)
        PLERROR("Error while parsing statname: expected a closing bracket");
    }
    
  if(setname=="train")
    setnum = 0;
  else if(setname=="test")
    setnum = 1;
  else if(setname.substr(0,4)=="test")
    {
      setnum = toint(setname.substr(4));
      if(setnum==0)
        PLERROR("In parseStatname: use the name train instead of test0.\n"
                "The first set of a split is the training set. The following are test sets named test1 test2 ..."); 
      if(setnum<=0)
        PLERROR("In parseStatname: parse error for %s",statname.c_str());        
    }
  else
    PLERROR("In parseStatname: parse error for %s",statname.c_str());
}


%> // end of namespace PLearn
