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
#include "random.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(EMPLearner, "EM Learner with N-gram on sequence", 
                        "EM Learner with N-gram on sequence");

EMPLearner::EMPLearner(): // DEFAULT VALUES FOR ALL OPTIONS
  SequencePLearner()
{
  max_n_bins = 50;
  log_base = 1.5;
  output_gen_size = 50;
  verbosity = 0;
  N = 3;
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

  declareOption(ol, "output_gen_size", &EMPLearner::output_gen_size, OptionBase::buildoption, 
                "    Number of note to generate in the test phase");  

  declareOption(ol, "verbosity", &EMPLearner::verbosity, OptionBase::buildoption, 
                "    Degree of verbosity");  

  inherited::declareOptions(ol);
}

void EMPLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies) {
  inherited::makeDeepCopyFromShallowCopy(copies);
}

int EMPLearner::get_cunigram(int i) const {
  SimpleI::const_iterator it = cunigram.find(i);
  if (it == cunigram.end())
    return 0;
  else
    return it->second;
}

int EMPLearner::get_cbigram(int i,int j) const {
  DoubleI::const_iterator it = cbigram.find(j);
  if (it == cbigram.end())
    return 0;
  SimpleI::const_iterator iit = it->second.find(i);
  if (iit == it->second.end())
    return 0;
  else
    return iit->second;
}

int EMPLearner::get_ctrigram(int i,int j,int k) const {
  TripleI::const_iterator it = ctrigram.find(k);
  if (it == ctrigram.end())
    return 0;
  DoubleI::const_iterator iit = it->second.find(j);
  if (iit == it->second.end())
    return 0;
  SimpleI::const_iterator iiit = iit->second.find(i);
  if (iiit == iit->second.end())
    return 0;
  else
    return iiit->second;
}

real EMPLearner::get_punigram(int i) const {
  SimpleR::const_iterator it = punigram.find(i);
  if (it == punigram.end())
    return 0.0;
  else
    return it->second;
}

real EMPLearner::get_pbigram(int i,int j) const {
  DoubleR::const_iterator it = pbigram.find(j);
  if (it == pbigram.end())
    return 0.0;
  SimpleR::const_iterator iit = it->second.find(i);
  if (iit == it->second.end())
    return 0.0;
  else
    return iit->second;
}

real EMPLearner::get_ptrigram(int i,int j,int k) const {
  TripleR::const_iterator it = ptrigram.find(k);
  if (it == ptrigram.end())
    return 0.0;
  DoubleR::const_iterator iit = it->second.find(j);
  if (iit == it->second.end())
    return 0.0;
  SimpleR::const_iterator iiit = iit->second.find(i);
  if (iiit == iit->second.end())
    return 0.0;
  else
    return iiit->second;
}

int EMPLearner::get_noteno(int n) const {
  int i = 0;
  for (SimpleI::const_iterator it = cunigram.begin(); it != cunigram.end(); ++it) {
    if (i >= n)
      return it->first;
    i++;
  }
#ifdef BOUNDCHECK
  PLERROR("In EMPLearner::get_noteno(int tr, int n) : n >= note_count");
#endif
  return -1;
}

real EMPLearner::get_zerogram() const {
  return zero_gram;
}

int EMPLearner::get_notecount() const {
  return note_count;
}

int EMPLearner::get_trainstreamsize() const {
  return train_stream.size() - (2 * train_stream.nb_seq());
}

int EMPLearner::get_ctrigramsize() const {
  int sum = 0;
  for (TripleI::const_iterator it = ctrigram.begin(); it != ctrigram.end(); ++it)
    sum += it->second.size();
  return sum;
}

void EMPLearner::build()
{
  inherited::build();
  build_();
}

void EMPLearner::build_() {


  if (train_set) {
    train_stream = SequenceVMatrixStream(dynamic_cast<SequenceVMatrix*>((VMatrix*)train_set), 0);
    n_track = (dynamic_cast<SequenceVMatrix*>((VMatrix*)train_set))->inputsize();

    trigram_mixture_posterior = Mat(max_n_bins, 4);
    bigram_mixture_posterior = Mat(max_n_bins, 3);

    init_mixture();
  }

  if (validation_set)
    valid_stream = SequenceVMatrixStream(dynamic_cast<SequenceVMatrix*>((VMatrix*)validation_set), 0);

  if (test_set)
    test_stream = SequenceVMatrixStream(test_set, 0);
  
}

void EMPLearner::init_mixture() {
  trigram_mixture = Mat(max_n_bins, 4);
  bigram_mixture = Mat(max_n_bins, 3);
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
    cbigram[u][v]++;

    while (train_stream.hasMoreInSequence()) {
      int w = (int)train_stream.next();
      cunigram[w]++;
      cbigram[v][w]++;
      ctrigram[u][v][w]++;
      u = v;
      v = w;
    }

    cunigram[v]--;
    cunigram[u]--;
    cbigram[u][v]--;
    if (get_cunigram(v) == 0)
      cunigram.erase(v);
    if (get_cunigram(u) == 0)
      cunigram.erase(u);

    if (get_cbigram(v, u) == 0) {
      cbigram[u].erase(v);
    }
    if (cbigram[u].empty())
      cbigram.erase(u);

    train_stream.nextSeq();
  }

  if (verbosity > 0)
    cout << "normalizing parameters...." << endl;

  zero_gram = 1.0 / (real)cunigram.size();
  note_count = cunigram.size();

  for (SimpleI::iterator it = cunigram.begin(); it != cunigram.end(); ++it) {
    int i = it->first;
    punigram[i] = (real)(cunigram[i]) / (real)(get_trainstreamsize());
  }

  for (DoubleI::iterator it = cbigram.begin(); it != cbigram.end(); ++it) {
    int i = it->first;
    for (SimpleI::iterator iit = it->second.begin(); iit != it->second.end(); ++iit) {
      int j = iit->first;
      pbigram[i][j] = (real)(cbigram[i][j]) / (real)(cunigram[i]);
    }
  }

  for (TripleI::iterator it = ctrigram.begin(); it != ctrigram.end(); ++it) {
    int i = it->first;
    for (DoubleI::iterator iit = it->second.begin(); iit != it->second.end(); ++iit) {
      int j = iit->first;
      for (SimpleI::iterator iiit = iit->second.begin(); iiit != iit->second.end(); ++iiit){
	int k = iiit->first;
	ptrigram[i][j][k] = (real)(ctrigram[i][j][k]) / (real)(cbigram[i][j]);
      }
    }
  }  
}

void EMPLearner::verify_prob() {
  real sum = 0;
  
  for (SimpleI::iterator it = cunigram.begin(); it != cunigram.end(); ++it) {
    int i = it->first;
    sum += punigram[i];
    if (verbosity > 1)
      cout << "C(" << i << ")=" << cunigram[i] << 
	"  P(" << i << ")=" << punigram[i] << endl;
  }

  if (verbosity > 1)
    cout << "Sum of unigram prob : " << sum << endl;

  if (!is_little(1.0 - sum)) {
    PLERROR("In EMPLearner::verify_prob() : The sum of unigram prob is %f",sum);
  }

  sum = 0;
  for (DoubleI::iterator it = cbigram.begin(); it != cbigram.end(); ++it) {
    int i = it->first;
    real internal_sum = 0;
    for (SimpleI::iterator iit = it->second.begin(); iit != it->second.end(); ++iit) {
      int j = iit->first;
      if (verbosity > 1)
	cout << "C(" << i << "," << j << ")=" << cbigram[i][j] << 
	  "  P(" << i << "," << j << ")=" << pbigram[i][j] << endl;
      internal_sum += pbigram[i][j];
    }
    if (!is_little(1.0 - internal_sum))
      PLERROR("In EMPLearner::verify_prob() : The internal_sum of bigram prob of %d is %f",
	      i, internal_sum);
    sum += internal_sum;
  }

  if (verbosity > 1)
    cout << "Mean of bigram prob : " << (sum / (real)cbigram.size()) << endl;

  if (!is_little(1.0 - (sum / (real)cbigram.size()))) {
    PLERROR("In EMPLearner::verify_prob() : The mean of sum of bigram prob is %f", 
	    (sum / (real)cbigram.size()));
  }

  sum = 0;
  for (TripleI::iterator it = ctrigram.begin(); it != ctrigram.end(); ++it) {
    int i = it->first;
    for (DoubleI::iterator iit = it->second.begin(); iit != it->second.end(); ++iit) {
      int j = iit->first;
      real internal_sum = 0;
      for (SimpleI::iterator iiit = iit->second.begin(); iiit != iit->second.end(); ++iiit){
	int k = iiit->first;
	if (verbosity > 1)
	  cout << "C(" << i << "," << j << "," << k << ")=" << ctrigram[i][j][k] << 
	    "  P(" << i << "," << j << "," << k << ")=" << ptrigram[i][j][k] << endl;
	internal_sum += ptrigram[i][j][k];
      }
      if (!is_little(1.0 - internal_sum))
	PLERROR("In EMPLearner::verify_prob() : The internal_sum of trigram prob of %d.%d is %f",
		i, j, internal_sum);      
      sum += internal_sum;
    }
  }  
  if (verbosity > 1) {
    cout << "Mean of trigram prob : " << (sum / (real)get_ctrigramsize()) << endl;
  }

  if (!is_little(1.0 - (sum / (real)get_ctrigramsize()))) {
    PLERROR("In EMPLearner::verify_prob() : The mean of sum of trigram prob is %f", 
	    (sum / (real)get_ctrigramsize()));
  }
}

void EMPLearner::verify_mixture() {
  for (int i = 0; i < max_n_bins; i++) {
    real sum_j = 0.0;
    for (int j = 0; j < 4; j++)
      sum_j += trigram_mixture[i][j];
    if (!is_little(sum_j - 1.0))
      PLERROR("trigram mixture is not summing to 1 at bin %d", i);
    sum_j = 0.0;
    for (int j = 0; j < 3; j++)
      sum_j += bigram_mixture[i][j];
    if (!is_little(sum_j - 1.0))
      PLERROR("bigram mixture is not summing to 1 at bin %d", i);
  }  
}

void EMPLearner::print_mixture() {
  if (verbosity > 1) {
    for (int i = 0; i < max_n_bins; i++) {
      cout << "T : ";
      for (int j = 0; j < 4; j++)
	cout << trigram_mixture[i][j] << ", ";
      cout << endl;
      
      cout << "B : ";
      for (int j = 0; j < 3; j++)
	cout << bigram_mixture[i][j] << ", ";
      cout << endl;
    }  
  }
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

int EMPLearner::mapFrequency(int freq, int T) const {
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
void EMPLearner::train() {
  if (!train_set)
    PLERROR("In EMPLearner::train, you did not setTrainingSet");
    
  if (!validation_set)
    PLERROR("In EMPLearner::train, you did not setValidationSet");

  if (!test_set)
    PLERROR("In EMPLearner::train, you did not setTestSet");

  build_();

  init_prob();
  verify_prob();

  ProgressBar* pb = 0;
  if(report_progress)
    pb = new ProgressBar("Training EM from stage " + 
			 tostring(stage) + " to " + tostring(nstages), nstages-stage);
  for (int e = 0; e < batch_size; e++) {
    if (verbosity > 0)
      cout << "epoch " << tostring(e) << endl;
    verify_mixture();
    one_step_train();
    one_step_valid();
    if (verbosity > 0)
      cout << "updating mixture..." << endl;
    update_mixture();
    one_step_test();
  }
  print_mixture();
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
    int mapped_context_freq = mapFrequency(context_freq, get_trainstreamsize());
    bm << bigram_mixture(mapped_context_freq);
    bigram_train_loglikelihood += safeflog(bm[0] * get_pbigram(v, u) + 
					   bm[1] * get_punigram(v) + 
					   bm[2] * get_zerogram());
    while (train_stream.hasMoreInSequence()) {
      int w = (int)train_stream.next();
      context_freq = get_cbigram(v, u);
      mapped_context_freq = mapFrequency(context_freq, get_trainstreamsize());
      tm << trigram_mixture(mapped_context_freq);
      trigram_train_loglikelihood += safeflog(tm[0] * get_ptrigram(w, v, u) + 
					      tm[1] * get_pbigram(w, v) + 
					      tm[2] * get_punigram(w) + 
					      tm[3] * get_zerogram());

      context_freq = get_cunigram(v);
      mapped_context_freq = mapFrequency(context_freq, get_trainstreamsize());
      bm << bigram_mixture(mapped_context_freq);
      bigram_train_loglikelihood += safeflog(bm[0] * get_pbigram(w, v) + 
					     bm[1] * get_punigram(w) + 
					     bm[2] * get_zerogram());
      
      u = v;
      v = w;
    }
    train_stream.nextSeq();
  }

  if (verbosity > 1) {
    cout << "trigram training perplexity = " << 
      safeexp(-trigram_train_loglikelihood / get_trainstreamsize()) << endl;
    cout << "bigram training perplexity = " << 
      safeexp(-bigram_train_loglikelihood / get_trainstreamsize()) << endl;
  }
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
    int context_freq = get_cunigram(v);
    int mapped_context_freq = mapFrequency(context_freq, get_trainstreamsize());
    bm << bigram_mixture(mapped_context_freq);
    
    bigram_valid_loglikelihood += safeflog(bm[0] * get_pbigram(v, u) + 
					   bm[1] * get_punigram(v) + 
					   bm[2] * get_zerogram());
    
    while (valid_stream.hasMoreInSequence()) {
      int w = (int)valid_stream.next();
      context_freq = get_cbigram(v, u);
      mapped_context_freq = mapFrequency(context_freq, get_trainstreamsize());
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
      mapped_context_freq = mapFrequency(context_freq, get_trainstreamsize());
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
  if (verbosity > 1) {
    cout << "trigram validation perplexity = " << 
      safeexp(-trigram_valid_loglikelihood / valid_stream.size()) << endl;
    cout << "bigram validation perplexity = " << 
      safeexp(-bigram_valid_loglikelihood / valid_stream.size()) << endl;  
  }
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
    int mapped_context_freq = mapFrequency(context_freq, get_trainstreamsize());
    bm << bigram_mixture(mapped_context_freq);

    bigram_test_loglikelihood += safeflog(bm[0] * get_pbigram(v, u) +
					  bm[1] * get_punigram(v) +
					  bm[2] * get_zerogram());

    while (test_stream.hasMoreInSequence()) {
      int w = (int)test_stream.next();

      context_freq = get_cbigram(v, u);
      mapped_context_freq = mapFrequency(context_freq, get_trainstreamsize());
      tm << trigram_mixture(mapped_context_freq);
      trigram_test_loglikelihood += safeflog((tm[0] * get_ptrigram(w, v, u)) + 
					     (tm[1] * get_pbigram(w, v)) + 
					     (tm[2] * get_punigram(w)) +
					     (tm[3] * get_zerogram()));
      
      context_freq = get_cunigram(v);
      mapped_context_freq = mapFrequency(context_freq, get_trainstreamsize());
      bm << bigram_mixture(mapped_context_freq);
      bigram_test_loglikelihood += safeflog((bm[0] * get_pbigram(w, v)) +
					    (bm[1] * get_punigram(w)) +
					    (bm[2] * get_zerogram()));
      u = v;
      v = w;
    }
    test_stream.nextSeq();
  }

  if (verbosity > 1) {
    cout << "trigram test perplexity = " << 
      safeexp(-trigram_test_loglikelihood / test_stream.size()) << endl;
    cout << "bigram test perplexity = " << 
      safeexp(-bigram_test_loglikelihood / test_stream.size()) << endl;  
  }
}

int EMPLearner::get_next_note() const {
  real p = uniform_sample();
  real total = 0.0;
  for (SimpleI::const_iterator it = cunigram.begin(); it != cunigram.end(); ++it) {
    int i = it->first;
    total += get_punigram(i);
    if (total >= p) {
      return i;
    }
  }
  PLERROR("in EMPLearner::get_next_note(int): no note was generated");
  return -1;
}

int EMPLearner::get_next_note(int u) const {
  Vec bm = Vec(3);
  int context_freq = get_cunigram(u);
  int mapped_context_freq = mapFrequency(context_freq, get_trainstreamsize());
  bm << bigram_mixture(mapped_context_freq);
  real p = uniform_sample();
  real total = 0.0;

  real total_b = 0.0, total_u = 0.0, total_z = 0.0;

  for (SimpleI::const_iterator it = cunigram.begin(); it != cunigram.end(); ++it) {
    int i = it->first;
    total += (bm[0] * get_pbigram(i, u)) + (bm[1] * get_punigram(i)) + 
      (bm[2] * get_zerogram());
    total_b += get_pbigram(i, u);
    total_u += get_punigram(i);
    total_z += get_zerogram();
    if (total >= p) {
      return i;
    }
  }

  if (!is_little(1.0 - total_b) || !is_little(1.0 - total_u) || !is_little(1.0 - total_z))
    PLERROR("in EMPLearner::get_next_note(int, int): Incorrect probality\n total_b = %f  total_u = %f  total_z = %f", total_b, total_u, total_z);

  if ((bm[0] == bm[1]) && (bm[1] == bm[2]))
    if (total_b == 0.0)
      PLWARNING("In get_next_note(int n, int u) : the mixture have not been updated but ok");
    else
      PLERROR("In get_next_note(int n, int u) : the mixture have not been updated");

  if (total_b == 0.0)
    return get_next_note();

  PLERROR("in EMPLearner::get_next_note(int, int): no note was generated");
  return -1;
}

bool EMPLearner::is_little(real r) const {
  return (abs(r) < 1e-6);
}

int EMPLearner::get_next_note(int u, int v) const {
  Vec tm = Vec(4);
  int context_freq = get_cbigram(v, u);
  int mapped_context_freq = mapFrequency(context_freq, get_trainstreamsize());
  tm << trigram_mixture(mapped_context_freq);
  real p = uniform_sample();
  real total = 0.0;

  real total_t = 0.0, total_b = 0.0, total_u = 0.0, total_z = 0.0;

  for (SimpleI::const_iterator it = cunigram.begin(); it != cunigram.end(); ++it) {
    int i = it->first;
    total += (tm[0] * get_ptrigram(i, v, u)) + (tm[1] * get_pbigram(i, v)) + 
      (tm[2] * get_punigram(i)) + (tm[3] * get_zerogram());
    total_t += get_ptrigram(i, v, u);
    total_b += get_pbigram(i, v);
    total_u += get_punigram(i);
    total_z += get_zerogram();
    if (total >= p) {
      return i;
    }
  }

  if ((!is_little(1.0 - total_t) && total_t != 0.0) || !is_little(1.0 - total_b) || 
      !is_little(1.0 - total_u) || !is_little(1.0 - total_z))
    PLERROR("in EMPLearner::get_next_note(int, int, int) : Incorrect probability\n total_t = %f   total_b = %f  total_u = %f  total_z = %f", total_t, total_b, total_u, total_z);
  if ((tm[0] == tm[1]) && (tm[1] == tm[2]) && (tm[2] == tm[3]))
    if (total_t == 0.0)
      PLWARNING("In get_next_note(int n, int u, int v) : the mixture have not been updated but ok");
    else
      PLERROR("In get_next_note(int n, int u, int v) : the mixture have not been updated");
  
  if (total_t == 0.0)
    return get_next_note(v);

  PLERROR("in EMPLearner::get_next_note(int, int, int): no note was generated");
  return -1;
}

void EMPLearner::computeOutput(const Mat& input, Mat& output) const {
  
  output.resize(output_gen_size, 1);
  output.fill(MISSING_VALUE);
 
  for (int t = 0; t < output_gen_size; t++) {
    if (t < N-1)
      output[t][0] = input[t][0];
    else
      output[t][0] = get_next_note((int)(output[t-2][0]), (int)(output[t-1][0]));
  }
}

void EMPLearner::get_next_step(Vec& output) {
  output[0] = get_next_note(last_outputs[0], last_outputs[1]);
  last_outputs[0] = last_outputs[1];
  last_outputs[1] = (int)output[0];
}

void EMPLearner::init_step(const Mat& input) {
#ifdef BOUNDCHECK
  if (input.ncols() > 1)
    PLWARNING("In EMPLearner::init_step : input.ncols() > 1, just the first colunm is needed");
  if (input.ncols() < 1 || input.nrows() < N-1)
    PLERROR("In EMPLearner::init_step : not enought data in input");
#endif
  last_outputs = TVec<int>(N-1);
  // We just need the last n-1 outputs to generate the next steps;
  for (int i = 0; i < N-1; i++) {
    last_outputs[i] = (int)input[input.nrows()-N+1+i][0];
  }
}


void EMPLearner::computeCostsFromOutputs(const Mat& input, const Mat& output, 
					 const Mat& target, Mat& costs) const {
  for (int i = 0; i < costs.nrows(); i++) {
    costs[i][0] = 0.0;
  }
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
