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

  splitter->setDataSet(dataset);

  int nsplits = splitter->nsplits();
  if(nsplits>1)
    call_forget = true;

  for(int splitnum=0; splitnum<nsplits; splitnum++) {
    TVec<VMat> dsets = splitter->getSplit(splitnum);
    VMat trainset = dsets[0];
    
    if(dsets.size()>1)
      bptt_learner->setValidationSet(dsets[1]);

    bptt_learner->setTrainStatsCollector(new VecStatsCollector());
    bptt_learner->setTrainingSet(trainset, call_forget && train);
    
    if (train) {
      bptt_learner->train();
    } else
      bptt_learner->build();

    for(int setnum=1; setnum<dsets.length(); setnum++) {
      SequenceVMat testset = dynamic_cast<SequenceVMatrix*>((VMatrix*)dsets[setnum]);

      PP<VecStatsCollector> test_stats = new VecStatsCollector();
      SequenceVMat test_outputs = new SequenceVMatrix();
      SequenceVMat test_costs = new SequenceVMatrix();

      if (testset->length()==0) {
	PLWARNING("PTester:: test set is of length 0, costs will be set to -1");
      }

      bptt_learner->test(testset, test_stats, test_outputs, test_costs);
      save(testset, test_outputs, test_costs, setnum);
    }
  }

  return Vec();
}

void VarLengthPTester::save(SequenceVMat test_set, SequenceVMat test_outputs,
			    SequenceVMat test_costs, int nsetnum) {
  force_mkdir(expdir);

  SequenceVMat all = test_set & test_outputs & test_costs;
  int nligne = all->getNbRowInSeqs(0, all->getNbSeq()) + all->getNbSeq();
  Vec sep = Vec(all->width(), -1.0);
  FileVMatrix file = FileVMatrix(append_slash(expdir) + "out" + tostring(nsetnum) + ".pmat", 0, all->width());
  for (int i = 0; i < all->getNbSeq(); i++) {
    Mat seq = Mat(all->getNbRowInSeq(i), all->width());
    all->getSeq(i, seq);
    for (int j = 0; j < seq.nrows(); j++) {
      Vec row = seq(j);
      file.appendRow(row);
    }
    file.appendRow(sep);
  }
  file.flush();
}

} // end of namespace PLearn
