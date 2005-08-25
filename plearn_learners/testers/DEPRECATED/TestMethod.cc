// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Pascal Vincent, Frederic Morin

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
 * $Id$
 * AUTHORS: Pascal Vincent & Frederic Morin
 * This file is part of the PLearn library.
 ******************************************************* */

#include "TestMethod.h"

namespace PLearn {

// ###### TestMethod ##########################################################

PLEARN_IMPLEMENT_OBJECT(TestMethod, "ONE LINE DESCR", "NO HELP");


/*
  Vec TestMethod::test(PP<Learner> learner, const VMat &dataset)
  {
  int nsp = splitter->nsplits(); 
  int nst = statnames.length();
  Vec results(nst);

  // First parse the desired statnames

  TVec<string> extstat(nst);
  TVec<string> intstat(nst);
  TVec<int> costindex(nst);
  TVec<int> setnum(nst);

  for(int i=0; i<nst; i++)
  {          
  string costname;
  parse_statname(statnames[i], extstat[i], intstat[i], setnum[i], costname);
  if(setnum==0) // train cost
  costindex[i] = learner->getTrainCostIndex(costname);
  else // test cost
  costindex[i] = learner->getTestCostIndex(costname);
  }

  // The global stats collector
  VecStatsCollector globalstats;

  // Now train/test and collect stats
  TVec<VecStatsCollector> stats;

  for(int k=0; k<nsp; k++)
  {
  Array<VMat> split = splitter->getSplit(k);
  stats.resize(split.size());

  if(forget_learner)
  learner->forget();
      
  // train
  VecStatsCollector& st = stats[0];
  learner->setTrainingSet(split[0]);
  st.forget();
  learner->newtrain(st);
  st.finalize();

  // tests
  for(int m=1; m<split.size(); m++)
  {
  VMat testset = split[m];
  VecStatsCollector& st = stats[m];
  st.forget();
  learner->newtest(testset,st);
  st.finalize();
  }

  for(int i=0; i<nst; i++)
  results[i] = stats[setnum[i]].getStats(costindex[i]).getStat(intstat[i]);

  globalstats.update(results);
  }

  globalstats.finalize();

  for(int i=0; i<nst; i++)
  results[i] = globalstats.getStats(i).getStat(extstat[i]);

  return results;
  }
*/

void TestMethod::build_()
{
}

void TestMethod::build()
{
    inherited::build();
    build_();
}

void
TestMethod::declareOptions(OptionList &ol)
{
    declareOption(ol, "splitter", &TestMethod::splitter, OptionBase::buildoption,
                  "Splitter object defining the test method");
    declareOption(ol, "statnames", &TestMethod::statnames, OptionBase::buildoption,
                  "A vector of strings containing train/test statistics definitions of the train/test stats whose values should be returned by the test method.\n"
                  "Ex: E[E[train.class_error]]");
    declareOption(ol, "forget_learner", &TestMethod::forget_learner, OptionBase::buildoption,
                  "Should we call forget on the learner prior to every train?");
    inherited::declareOptions(ol);
}



}; // end of namespace PLearn


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
