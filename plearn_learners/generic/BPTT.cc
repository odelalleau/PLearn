// -*- C++ -*-

// BPTT.cc
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

#include "Optimizer.h"
#include "BPTT.h"
#include "random.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(BPTT, "Backpropagation through time on a recurrent neural network", 
                        "Backpropagation through time on a recurrent neural network");

BPTT::BPTT(): // DEFAULT VALUES FOR ALL OPTIONS
  SequencePLearner()
{
  links = TMat<int>(0,3);
  units_type = TVec<string>(0);
}

BPTT::~BPTT()
{
}

void BPTT::declareOptions(OptionList& ol)
{
  declareOption(ol, "optimizer", &BPTT::optimizer, OptionBase::buildoption, 
                "    specify the optimizer to use\n");

  declareOption(ol, "nneuron_input", &BPTT::nneuron_input, OptionBase::buildoption, 
                "    number of input neuron in the network.");

  declareOption(ol, "nneuron_hidden", &BPTT::nneuron_hidden, OptionBase::buildoption, 
                "    number of hidden neuron in the network.");

  declareOption(ol, "nneuron_output", &BPTT::nneuron_output, OptionBase::buildoption, 
                "    number of output neuron in the network.");

  declareOption(ol, "links", &BPTT::links, OptionBase::buildoption, 
                "    A TMat of 3 columns use to define the links between neurons in the reccurent neural network. The first column correspond the no of the parent neuron. The second colunm correspond to the no of the child neuron. The third column correspond to the delay that the value will stay in the parent node before it will propagate the child neuron.");

  declareOption(ol, "units_type", &BPTT::units_type, OptionBase::buildoption, 
                "    A TVec of string with width equals to nneuron. Use to specified the squashing function of each neuron. The choices are LIN or TANH");

  declareOption(ol, "cost_type", &BPTT::cost_type, OptionBase::buildoption, 
                "    A string to specify the function to minimise. The choices are MSE");

  inherited::declareOptions(ol);
}

void BPTT::makeDeepCopyFromShallowCopy(CopiesMap& copies) {
  inherited::makeDeepCopyFromShallowCopy(copies);
}

void BPTT::build()
{
  inherited::build();
  build_();
}

/*
  build the recurrent network and allocate places for bias and weigths
*/
void BPTT::build_()
{
  if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0) {
    if (links.nrows() == 0)
      build_fully_connected_network();
    if (units_type.size() == 0)
      build_default_units_type();
    weights = Vec(links.nrows(), 0.5);
    bias = Vec(nneuron_input + nneuron_hidden + nneuron_output, 1.0);
    params = VarArray(Var(weights), Var(bias));
    rec_net = new BPTTVariable(params, dynamic_cast<SequenceVMatrix*>((VMatrix*)train_set), batch_size,
			       links, nneuron_input, nneuron_hidden, nneuron_output, units_type, cost_type);
  } else {
    rec_net = 0;
  }    
}

void BPTT::build_default_units_type() {
  units_type = TVec<string>(nneuron_input + nneuron_hidden + nneuron_output);
  for (int i = 0; i < nneuron_input; i++)
    units_type[i] = "ID";

  for (int i = nneuron_input; i < nneuron_input + nneuron_hidden + nneuron_output; i++)
    units_type[i] = "TANH";  
}

void BPTT::build_fully_connected_network() {
  // Input to hidden links
  for (int i = 0; i < nneuron_input; i++) {
    for (int h = nneuron_input; h < nneuron_input + nneuron_hidden; h++) {
      TVec<int> l = TVec<int>(3);
      l[0] = i; // src
      l[1] = h; // dst
      l[2] = 0; // delay
      links.appendRow(l);
    }
  }

  // Hidden to hidden links
  for (int h1 = nneuron_input; h1 < nneuron_input + nneuron_hidden; h1++) {
    for (int h2 = nneuron_input; h2 < nneuron_input + nneuron_hidden; h2++) {
      TVec<int> l = TVec<int>(3);
      l[0] = h1; // src
      l[1] = h2; // dst
      l[2] = 1; // delay
      links.appendRow(l);      
    }
  }

  // Hidden to output links

  for (int o = nneuron_input + nneuron_hidden; o < nneuron_input + nneuron_hidden + nneuron_output; o++) {
    for (int h = nneuron_input; h < nneuron_input + nneuron_hidden; h++) {
      TVec<int> l = TVec<int>(3);
      l[0] = h; // src
      l[1] = o; // dst
      l[2] = 0; // delay
      links.appendRow(l);            
    }
  }
}

TVec<string> BPTT::getTrainCostNames() const
{
  return TVec<string>(1, "MSE");
}

TVec<string> BPTT::getTestCostNames() const
{ 
  return TVec<string>(1, "MSE");
}

/*
  Train the network.
*/
void BPTT::train()
{
  if(!train_set)
    PLERROR("In BPTT::train, you did not setTrainingSet");
    
  if(!train_stats)
    PLERROR("In BPTT::train, you did not setTrainStatsCollector");

  if(!rec_net)
    build();

  if(!rec_net)
    PLERROR("In BBPT::train, the reccurent network is not set properly");

  Vec min_weights = Vec(weights.size());
  Vec min_bias = Vec(bias.size());
  real min_cost = REAL_MAX;

  int l = train_set->length();

  // number of samples seen by optimizer before each optimizer update
  int nsamples = batch_size>0 ? batch_size : l;
  if(optimizer)
    {
      optimizer->setToOptimize(params, VarArray(rec_net));  
      optimizer->build();
    }
  else PLERROR("BPTT::train can't train without setting an optimizer first!");

  // number of optimizer stages corresponding to one learner stage (one epoch)
  int optstage_per_lstage = l/nsamples;

  ProgressBar* pb = 0;
  if(report_progress)
    pb = new ProgressBar("Training BPTT from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

  int initial_stage = stage;

  real last_value = 0.0;

  while(stage<nstages && !optimizer->early_stop) {
    rec_net->reset();
    optimizer->nstages = optstage_per_lstage;
    train_stats->forget();
    optimizer->early_stop = false;
    optimizer->optimizeN(*train_stats);
    train_stats->finalize();

    if(verbosity>2) {
      cout << "Epoch " << stage << " train objective: " << rec_net->value[0];
      cout << " (" << (rec_net->value[0] - last_value) << ") min : " << min_cost;
      cout << endl;
      cout << "Weights : " << weights << endl;
      cout << "Bias : " << bias << endl;
      last_value = rec_net->value[0];
    } else if (verbosity == 2) {
      if ((stage % 50) == 0) {
	cout << "Epoch " << stage << " train objective: " << rec_net->value[0];
	cout << " (" << (rec_net->value[0] - last_value) << ") min : " << min_cost;
	cout << endl;
	last_value = rec_net->value[0];
      }
    }
    
    if (rec_net->value[0] < min_cost) {
      min_cost = rec_net->value[0];
      min_weights << weights;
      min_bias << bias;
    } else {
      //      weights << min_weights;
      //bias << min_bias;      
    }
    ++stage;
    if(pb)
      pb->update(stage-initial_stage);
  }
  if(verbosity>1)
    cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;
  
  weights << min_weights;
  bias << min_bias;

  cout << "We use the params for the cost " << min_cost << endl;

  if(pb)
    delete pb;
}

void BPTT::computeOutput(const Mat& input, Mat& output) const
{
  rec_net->computeOutputFromInput(input, output);
}

void BPTT::computeCostsFromOutputs(const Mat& input, const Mat& output, 
                                   const Mat& target, Mat& costs) const {
  rec_net->computeCostFromOutput(output, target, costs);  
}

/*
To test the verify gradient
*/
void BPTT::run() {
  //    train();
      rec_net->verifyGradient();
      //  cout << "err(2,4)=" << rec_net->computeErr(2, 4) << " err(1,4)=" << rec_net->computeErr(1,4) << endl;
      //cout << "err'(2,4)=" << rec_net->computeGradErr(2, 4) << " err'(1,4)=" << rec_net->computeGradErr(1,4) << endl;
}

void BPTT::get_next_step(Vec& output) {
  rec_net->next_step(output);
}

void BPTT::init_step(const Mat& input) {
  rec_net->init_step(input);
}

/*
  Initialize the parameter
*/
void BPTT::initializeParams() {
  // Reset optimizer
  if(optimizer)
    optimizer->reset();
}

/*
  Initialize the parameter
*/
void BPTT::forget() {
  if (train_set) initializeParams();
  for (int i=0;i<nneuron_input + nneuron_hidden + nneuron_output;i++) {
    bias[i] = 0.0;
    int fanin = rec_net->get_indexDest(i,0);
    real r = 1.0/sqrt((float)fanin);
    for (int l = 1; l <= rec_net->get_indexDest(i,0); l++) {
      int nlink =  rec_net->get_indexDest(i,l);
      weights[nlink] = bounded_uniform(-r,r);
    }
  }
  stage = 0;
}

} // end of namespace PLearn
