// -*- C++ -*-

// EMPLearner.cc
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

#include "EMPLearner.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(EMPLearner, "EM Learner with N-gram on sequence", 
                        "EM Learner with N-gram on sequence");

EMPLearner::EMPLearner(): // DEFAULT VALUES FOR ALL OPTIONS
  SequencePLearner()
{
  max_n_bins = 50;
  log_base = 1.5;
}

EMPLearner::~EMPLearner()
{
}

void EMPLearner::declareOptions(OptionList& ol)
{

  declareOption(ol, "max_n_bins", &EMPLearner::max_n_bins, OptionBase::buildoption, 
                "    maximum number of bins");  

  declareOption(ol, "log_base", &EMPLearner::log_base, OptionBase::buildoption, 
                "    log base for the frequency map");  

  inherited::declareOptions(ol);
}

void EMPLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies) {
  inherited::makeDeepCopyFromShallowCopy(copies);
}

int EMPLearner::get_cunigram(int i) {
  return cunigram[i];
}

int EMPLearner::get_cbigram(int i,int j) {
  return cbigram[i][j];
}

int EMPLearner::get_ctrigram(int i,int j,int k) {
  return ctrigram[i][j][k];
}

real EMPLearner::get_punigram(int i) {
  return punigram[i];
}

real EMPLearner::get_pbigram(int i,int j) {
  return pbigram[i][j];
}

real EMPLearner::get_ptrigram(int i,int j,int k) {
  return ptrigram[i][j][k];
}

real EMPLearner::get_zerogram() {
  return zero_gram;
}

int EMPLearner::get_notecount() {
  return note_count;
}

void EMPLearner::build()
{
  inherited::build();
  build_();
}

void EMPLearner::build_()
{
  init_mixture();

  if (train_set)
    train_stream = SequenceVMatrixStream(dynamic_cast<SequenceVMatrix*>((VMatrix*)train_set), 0);

  if (validation_set)
    valid_stream = SequenceVMatrixStream(dynamic_cast<SequenceVMatrix*>((VMatrix*)validation_set), 0);

  if (test_set)
    test_stream = SequenceVMatrixStream(test_set, 0);
}

void EMPLearner::init_mixture() {
  trigram_mixture = Mat(max_n_bins, 4);
  bigram_mixture = Mat(max_n_bins, 3);
  trigram_mixture_posterior = Mat(max_n_bins, 4);
  bigram_mixture_posterior = Mat(max_n_bins, 3);
  for (int i = 0; i < max_n_bins; i++) {
    for (int j = 0; j < 4; j++)
      trigram_mixture[i][j] = 1.0 / 4.0;
    for (int j = 0; j < 3; j++)
      bigram_mixture[i][j] = 1.0 / 3.0;
  }
}

void EMPLearner::init_prob() {
  train_stream.init();
  while (train_stream.hasMoreSequence()) {
#ifdef BOUNDCHECK
    if (!(train_stream.hasMoreInSequence()))
      PLERROR("In EMPLearner::init_prob() : All the seq must have a lenght >= 2");
#endif
    int u = (int)train_stream.next();
#ifdef BOUNDCHECK
    if (!(train_stream.hasMoreInSequence()))
      PLERROR("In EMPLearner::init_prob() : All the seq must have a lenght >= 2");
#endif
    int v = (int)train_stream.next();
    cunigram[u]++;
    cunigram[v]++;
    cbigram[v][u]++;
    while (train_stream.hasMoreInSequence()) {
      int w = (int)train_stream.next();
      cunigram[w]++;
      cbigram[w][v]++;
      ctrigram[u][v][w]++;
      u = v;
      v = w;
    }

    cbigram[0][v]++;
    ctrigram[u][v][0]++;

    train_stream.nextSeq();
  }

  cout << "normalizing parameters...." << endl;

  zero_gram = 1.0 / (real)cunigram.size();
  note_count = cunigram.size();

  for (SimpleI::iterator it = cunigram.begin(); it != cunigram.end(); ++it) {
    int i = it->first;
    punigram[i] = (real)(cunigram[i]) / (real)train_stream.size();
  }

  for (DoubleI::iterator it = cbigram.begin(); it != cbigram.end(); ++it) {
    int i = it->first;
    for (SimpleI::iterator iit = it->second.begin(); iit != it->second.end(); ++iit) {
      int j = iit->first;
      pbigram[i][j] = (real)(cbigram[i][j]) / (real)(cunigram[j]);
    }
  }

  for (TripleI::iterator it = ctrigram.begin(); it != ctrigram.end(); ++it) {
    int i = it->first;
    for (DoubleI::iterator iit = it->second.begin(); iit != it->second.end(); ++iit) {
      int j = iit->first;
      for (SimpleI::iterator iiit = iit->second.begin(); iiit != iit->second.end(); ++iiit){
	int k = iiit->first;
	ptrigram[i][j][k] = (real)(ctrigram[i][j][k]) / (real)(cbigram[j][i]);
      }
    }
  }  
}

void EMPLearner::verify_prob() {
  real sum = 0;
  
  for (SimpleI::iterator it = cunigram.begin(); it != cunigram.end(); ++it) {
    int i = it->first;
    sum += punigram[i];
    cout << "C(" << i << ")=" << cunigram[i] << "  P(" << i << ")=" << punigram[i] << endl;
  }

  cout << "Sum of unigram prob : " << sum << endl;

  sum = 0;
  for (DoubleI::iterator it = cbigram.begin(); it != cbigram.end(); ++it) {
    int i = it->first;
    for (SimpleI::iterator iit = it->second.begin(); iit != it->second.end(); ++iit) {
      int j = iit->first;
      cout << "C(" << i << "," << j << ")=" << cbigram[i][j] << 
	"  P(" << i << "," << j << ")=" << pbigram[i][j] << endl;
      sum += pbigram[i][j];
    }
  }

  cout << "Mean of bigram prob : " << (sum / (real)cbigram.size()) << endl;

  sum = 0;
  for (TripleI::iterator it = ctrigram.begin(); it != ctrigram.end(); ++it) {
    int i = it->first;
    for (DoubleI::iterator iit = it->second.begin(); iit != it->second.end(); ++iit) {
      int j = iit->first;
      for (SimpleI::iterator iiit = iit->second.begin(); iiit != iit->second.end(); ++iiit){
	int k = iiit->first;
	cout << "C(" << i << "," << j << "," << k << ")=" << ctrigram[i][j][k] << 
	  "  P(" << i << "," << j << "," << k << ")=" << ptrigram[i][j][k] << endl;
	sum += ptrigram[i][j][k];
      }
    }
  }  
  cout << "Mean of trigram prob : " << (sum / (real)ctrigram.size()) << endl;
}

void EMPLearner::clear_mixture_posterior() {
  for (int i = 0; i < max_n_bins; i++) {
    for (int j = 0; j < 4; j++)
      trigram_mixture_posterior[i][j] = 0.0;
    for (int j = 0; j < 3; j++)
      bigram_mixture_posterior[i][j] = 0.0;
  }
}

/*
  Entire copy of Christian Jauvin's code
*/
void EMPLearner::update_mixture() {
  for (int i = 0; i < max_n_bins; i++) {
    real sum_j = 0.0;
    for (int j = 0; j < 4; j++)
      sum_j += trigram_mixture_posterior[i][j];
    if (sum_j > 0.0) {
      real norm_i = 1.0 / sum_j;
      for (int j = 0; j < 4; j++)
        trigram_mixture[i][j] = trigram_mixture_posterior[i][j] * norm_i;
    }
    sum_j = 0.0;
    for (int j = 0; j < 3; j++)
      sum_j += bigram_mixture_posterior[i][j];
    if (sum_j > 0.0) {
      real norm_i = 1.0 / sum_j;
      for (int j = 0; j < 3; j++)
        bigram_mixture[i][j] = bigram_mixture_posterior[i][j] * norm_i;
    }
  }    
}

int EMPLearner::mapFrequency(int freq, int T)
{
  int bin_index = (abs((int)ceil(safeflog(log_base, (real)(1 + freq) / (real)T))));
  if (bin_index < 0 || bin_index >= max_n_bins)
    PLERROR("trying to reach invalid bin index: %d", bin_index);
  return bin_index;
}

TVec<string> EMPLearner::getTrainCostNames() const
{
  return TVec<string>(1, "Maximum likelihood");
}

TVec<string> EMPLearner::getTestCostNames() const
{ 
  return TVec<string>(1, "Maximum likelihood");
}

/*
  Train the network.
*/
void EMPLearner::train()
{
  if (!train_set)
    PLERROR("In EMPLearner::train, you did not setTrainingSet");
    
  if (!validation_set)
    PLERROR("In EMPLearner::train, you did not setValidationSet");

  if (!test_set)
    PLERROR("In EMPLearner::train, you did not setTestSet");
  (dynamic_cast<SequenceVMatrix*>((VMatrix*)train_set))->print();

  (dynamic_cast<SequenceVMatrix*>((VMatrix*)validation_set))->print();

  (dynamic_cast<SequenceVMatrix*>((VMatrix*)test_set))->print();

  build_();
  init_prob();

  verify_prob();

  ProgressBar* pb = 0;
  if(report_progress)
    pb = new ProgressBar("Training EM from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);
  for (int e = 0; e < batch_size; e++) {
    cout << "epoch " << tostring(e) << endl;
    one_step_train();
    one_step_valid();
    cout << "updating mixture..." << endl;
    update_mixture();
    one_step_test();
  }

}

void EMPLearner::one_step_train() {
  real bigram_train_loglikelihood = 0.0;
  real trigram_train_loglikelihood = 0.0;

  Vec bm = Vec(3);
  Vec tm = Vec(4);

  train_stream.init();

  while (train_stream.hasMoreSequence()) {
#ifdef BOUNDCHECK
    if (!train_stream.hasMoreInSequence())
      PLERROR("In EMPLearner::one_step_train() : All the seq must have a lenght >= 2");
#endif
    int u = (int)train_stream.next();
#ifdef BOUNDCHECK
    if (!train_stream.hasMoreInSequence())
      PLERROR("In EMPLearner::one_step_train() : All the seq must have a lenght >= 2");
#endif
    int v = (int)train_stream.next();
    int context_freq = get_cunigram(u);
    int mapped_context_freq = mapFrequency(context_freq, train_stream.size());
    bm << bigram_mixture(mapped_context_freq);
    bigram_train_loglikelihood += safeflog(bm[0] * get_pbigram(v, u) + 
					   bm[1] * get_punigram(v) + 
					   bm[2] * get_zerogram());
    while (train_stream.hasMoreInSequence()) {
      int w = (int)train_stream.next();
      context_freq = get_cbigram(v, u);
      mapped_context_freq = mapFrequency(context_freq, train_stream.size());
      tm << trigram_mixture(mapped_context_freq);
      trigram_train_loglikelihood += safeflog(tm[0] * get_ptrigram(w, v, u) + 
					      tm[1] * get_pbigram(w, v) + 
					      tm[2] * get_punigram(w) + 
					      tm[3] * get_zerogram());

      context_freq = get_cunigram(v);
      mapped_context_freq = mapFrequency(context_freq, train_stream.size());
      bm << bigram_mixture(mapped_context_freq);
      bigram_train_loglikelihood += safeflog(bm[0] * get_pbigram(w, v) + 
					     bm[1] * get_punigram(w) + 
					     bm[2] * get_zerogram());
      
      u = v;
      v = w;
    }
    train_stream.nextSeq();
  }

  cout << "trigram training perplexity = " << 
    safeexp(-trigram_train_loglikelihood / train_stream.size()) << endl;
  cout << "bigram training perplexity = " << 
    safeexp(-bigram_train_loglikelihood / train_stream.size()) << endl;
}

void EMPLearner::one_step_valid() {
  real bigram_valid_loglikelihood = 0.0;
  real trigram_valid_loglikelihood = 0.0;

  Vec bm = Vec(3);
  Vec tm = Vec(4);

  clear_mixture_posterior();
  valid_stream.init();
  
  while (valid_stream.hasMoreSequence()) {
#ifdef BOUNDCHECK
    if (!valid_stream.hasMoreInSequence())
      PLERROR("In EMPLearner::one_step_valid() : All the seq must have a lenght >= 2");
#endif
    int u = (int)valid_stream.next();
#ifdef BOUNDCHECK
    if (!valid_stream.hasMoreInSequence())
      PLERROR("In EMPLearner::one_step_valid() : All the seq must have a lenght >= 2");
#endif
    int v = (int)valid_stream.next();
    int context_freq = get_cunigram(u);
    int mapped_context_freq = mapFrequency(context_freq, train_stream.size());
    bm << bigram_mixture(mapped_context_freq);
    
    bigram_valid_loglikelihood += safeflog(bm[0] * get_pbigram(v, u) + 
					   bm[1] * get_punigram(v) + 
					   bm[2] * get_zerogram());
    
    while (valid_stream.hasMoreInSequence()) {
      int w = (int)valid_stream.next();
      context_freq = get_cbigram(v, u);
      mapped_context_freq = mapFrequency(context_freq, train_stream.size());
      tm << trigram_mixture(mapped_context_freq);
      real p_trigram = tm[0] * get_ptrigram(w, v, u);
      real p_bigram = tm[1] * get_pbigram(w, v);
      real p_unigram = tm[2] * get_punigram(w);
      real p_zerogram = tm[3] * get_zerogram();
      real trigram_weighted = p_trigram + p_bigram + p_unigram + p_zerogram;
      trigram_valid_loglikelihood += safeflog(trigram_weighted);
      trigram_mixture_posterior[mapped_context_freq][0] += (p_trigram / trigram_weighted);
      trigram_mixture_posterior[mapped_context_freq][1] += (p_bigram / trigram_weighted);
      trigram_mixture_posterior[mapped_context_freq][2] += (p_unigram / trigram_weighted);
      trigram_mixture_posterior[mapped_context_freq][3] += (p_zerogram / trigram_weighted);
      
      context_freq = get_cunigram(v);
      mapped_context_freq = mapFrequency(context_freq, train_stream.size());
      bm << bigram_mixture(mapped_context_freq);
      p_bigram = bm[0] * get_pbigram(w, v);
      p_unigram = bm[1] * get_punigram(w);
      p_zerogram = bm[2] * get_zerogram();
      real bigram_weighted = p_bigram + p_unigram + p_zerogram;
      bigram_valid_loglikelihood += safeflog(bigram_weighted); 
      bigram_mixture_posterior[mapped_context_freq][0] += (p_bigram / bigram_weighted);
      bigram_mixture_posterior[mapped_context_freq][1] += (p_unigram / bigram_weighted);
      bigram_mixture_posterior[mapped_context_freq][2] += (p_zerogram / bigram_weighted);
      
      u = v;
      v = w;
    }
    valid_stream.nextSeq();
  }
  cout << "trigram validation perplexity = " << 
    safeexp(-trigram_valid_loglikelihood / valid_stream.size()) << endl;
  cout << "bigram validation perplexity = " << 
    safeexp(-bigram_valid_loglikelihood / valid_stream.size()) << endl;  
}

void EMPLearner::one_step_test() {
  real bigram_test_loglikelihood = 0.0;
  real trigram_test_loglikelihood = 0.0;

  test_stream.init();
  Vec bm = Vec(3);
  Vec tm = Vec(4);

  while (test_stream.hasMoreSequence()) {
    int u = (int)test_stream.next();
    int v = (int)test_stream.next();
    int context_freq = get_cunigram(u);
    int mapped_context_freq = mapFrequency(context_freq, train_stream.size());
    bm << bigram_mixture(mapped_context_freq);

    bigram_test_loglikelihood += safeflog(bm[0] * get_pbigram(v, u) +
					  bm[1] * get_punigram(v) +
					  bm[2] * get_zerogram());

    while (test_stream.hasMoreInSequence()) {
      int w = (int)test_stream.next();

      context_freq = get_cbigram(u,v);
      mapped_context_freq = mapFrequency(context_freq, train_stream.size());
      tm << trigram_mixture(mapped_context_freq);
      trigram_test_loglikelihood += safeflog((tm[0] * get_ptrigram(w, v, u)) + 
					     (tm[1] * get_pbigram(w, v)) + 
					     (tm[2] * get_punigram(w)) +
					     (tm[3] * get_zerogram()));
      
      context_freq = get_cunigram(v);
      mapped_context_freq = mapFrequency(context_freq, train_stream.size());
      bm << bigram_mixture(mapped_context_freq);
      bigram_test_loglikelihood += safeflog((bm[0] * get_pbigram(w, v)) +
					    (bm[1] * get_punigram(w)) +
					    (bm[2] * get_zerogram()));
      u = v;
      v = w;
    }
    test_stream.nextSeq();
  }

  cout << "trigram test perplexity = " << 
    safeexp(-trigram_test_loglikelihood / test_stream.size()) << endl;
  cout << "bigram test perplexity = " << 
    safeexp(-bigram_test_loglikelihood / test_stream.size()) << endl;  
}

void EMPLearner::computeOutput(const Mat& input, Mat& output) const
{

}

void EMPLearner::computeCostsFromOutputs(const Mat& input, const Mat& output, 
                                   const Mat& target, Mat& costs) const {

}

void EMPLearner::run() {

}

/*
  Initialize the parameter
*/
void EMPLearner::initializeParams() {

}

/*
  Initialize the parameter
*/
void EMPLearner::forget() {
  if (train_set) initializeParams();
  stage = 0;
}

} // end of namespace PLearn
