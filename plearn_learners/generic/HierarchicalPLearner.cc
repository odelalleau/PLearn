// -*- C++ -*-

// HierarchicalPLearner.cc
// Copyright (c) 2004 Jasmin Lapalme
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

#include "HierarchicalPLearner.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(HierarchicalPLearner, "A Learner that train separatly different hierarchical level and then output a sequence that comes from a choices of value comming from each hierarchical level", 
                        "");

HierarchicalPLearner::HierarchicalPLearner(): // DEFAULT VALUES FOR ALL OPTIONS
  SequencePLearner()
{

}

HierarchicalPLearner::~HierarchicalPLearner()
{
}

void HierarchicalPLearner::declareOptions(OptionList& ol)
{

  declareOption(ol, "n_level", &HierarchicalPLearner::n_level_, OptionBase::buildoption, 
                "   number of hierarchical level \n");
  declareOption(ol, "rep_length", &HierarchicalPLearner::rep_length_, OptionBase::buildoption, 
                "   the representation length of one step\n");
  declareOption(ol, "harms", &HierarchicalPLearner::harms_, OptionBase::buildoption, 
                "   the harmonic for each level (TVec<int>)\n");
  declareOption(ol, "level_learner", &HierarchicalPLearner::level_learner, OptionBase::buildoption, 
                "   a Vec of learner to apply to each level\n");
  declareOption(ol, "gen_size", &HierarchicalPLearner::gen_size_, OptionBase::buildoption, 
                "   number of step to generate when the computeOutput is called\n");
  declareOption(ol, "start_size", &HierarchicalPLearner::start_size_, OptionBase::buildoption, 
                "   number of note of the highest level, use for the start of output\n");
  
  inherited::declareOptions(ol);
}

void HierarchicalPLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies) {
  inherited::makeDeepCopyFromShallowCopy(copies);
}

void HierarchicalPLearner::build()
{
  inherited::build();
  build_();
}

/*
  build the recurrent network and allocate places for bias and weigths
*/
void HierarchicalPLearner::build_()
{

  if (train_set) {
    SequenceVMat ts = dynamic_cast<SequenceVMatrix*>( (VMatrix*)train_set );
    for (int i = 0; i < n_level_; i++) {
      level_learner[i]->setTrainingSet((VMatrix*)(ts->get_level(i, rep_length_)));
    }
  }

  if (validation_set) {
    SequenceVMat vs = dynamic_cast<SequenceVMatrix*>( (VMatrix*)validation_set );
    for (int i = 0; i < n_level_; i++)
      level_learner[i]->setValidationSet((VMatrix*)(vs->get_level(i, rep_length_)));
  }

  if (test_set) {
    for (int i = 0; i < n_level_; i++) {
      SequenceVMat ts = test_set->get_level(i, rep_length_);
      level_learner[i]->setTestSet( ts );
      level_learner[i]->setTrainStatsCollector(train_stats);
    }
  }

  for (int i = 0; i < n_level_; i++) {
    level_learner[i]->setTrainStatsCollector(train_stats);
  }
}

TVec<string> HierarchicalPLearner::getTrainCostNames() const
{
  return TVec<string>(1, "MSE");
}

TVec<string> HierarchicalPLearner::getTestCostNames() const
{ 
  return TVec<string>(1, "MSE");
}

/*
  Train the network.
*/
void HierarchicalPLearner::train()
{
  if(!train_set)
    PLERROR("In HierarchicalPLearner::train, you did not setTrainingSet");
    
  if(!train_stats)
    PLERROR("In HierarchicalPLearner::train, you did not setTrainStatsCollector");

  build();

  for (int i = 0; i < n_level_; i++) {
    cout << "HierarchicalPLearner train the level " << i << endl;
    level_learner[i]->train();
  }
}

void HierarchicalPLearner::computeOutput(const Mat& input, Mat& output) const {
  output.resize(gen_size_, n_level_ * rep_length_);
  output.fill(MISSING_VALUE);
  int n_note_start = start_size_ * harms_[n_level_-1];
  for (int i = 0; i < n_note_start; i++) {
    for (int j = n_level_ - 1; j >= 0; j--) {
      if ((i % harms_[j]) == 0) {
	for (int k = 0; k < rep_length_; k++) {
	  output[i][j*rep_length_+k] = input[i/harms_[j]][j*rep_length_+k];
	}
      }
    }
  }

  for (int i = 0; i < n_level_; i++) {
    int l = n_note_start / harms_[i];
    Mat init = Mat(l, rep_length_);
    for (int pos = 0; pos < l; pos++) {
      for (int j = 0; j < rep_length_; j++)
	init[pos][j] = input[pos][i * rep_length_ + j];
    }
    level_learner[i]->init_step(init);
  }

  Vec out = Vec(rep_length_);
  // loop on each step
  for (int i = n_note_start; i < gen_size_; i++) {
    // loop on each level
    for (int h = n_level_-1; h >= 0; h--) {
      if ((i % harms_[h]) == 0) {
	// good level. we generate a new note on this level
	level_learner[h]->get_next_step(out);
	// loop to copy the output on the levels below
	for (int k = h; k >= 0; k--) {
	  for (int j = 0; j < rep_length_; j++) {
	    output[i][rep_length_*k+j] = out[j];
	  }
	}
	break;
      }
    }
  }
}

void HierarchicalPLearner::computeCostsFromOutputs(const Mat& input, const Mat& output, 
                                   const Mat& target, Mat& costs) const {
  
}

void HierarchicalPLearner::run() {

}

void HierarchicalPLearner::get_next_step(Vec& output) {
  PLERROR("In HierarchicalPLearner::get_next_step(Vec&) invalid call");
}

void HierarchicalPLearner::init_step(const Mat& input) {
  PLERROR("In HierarchicalPLearner::init_step(const Mat&) invalid call");
}

/*
  Initialize the parameter
*/
void HierarchicalPLearner::initializeParams() {
}

/*
  Initialize the parameter
*/
void HierarchicalPLearner::forget() {

}

} // end of namespace PLearn
