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

#include "AffineTransformVariable.h"
#include "AffineTransformWeightPenalty.h"
#include "BinaryClassificationLossVariable.h"
#include "ClassificationLossVariable.h"
#include "ConcatColumnsVariable.h"
#include "CrossEntropyVariable.h"
#include "ExpVariable.h"
#include "LiftOutputVariable.h"
#include "LogSoftmaxVariable.h"
#include "MulticlassLossVariable.h"
#include "NegCrossEntropySigmoidVariable.h"
#include "OneHotSquaredLoss.h"
#include "SigmoidVariable.h"
#include "SoftmaxVariable.h"
#include "SoftplusVariable.h"
#include "SumVariable.h"
#include "SumAbsVariable.h"
#include "SumOfVariable.h"
#include "SumSquareVariable.h"
#include "TanhVariable.h"
#include "TransposeProductVariable.h"
#include "Var_operators.h"
#include "Var_utils.h"

#include "ConcatColumnsVMatrix.h"
//#include "DisplayUtils.h"
//#include "GradientOptimizer.h"
#include "BPTT.h"
#include "random.h"
#include "SubVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(BPTT, "Backpropagation through time on a recurrent neural network", 
                        "Backpropagation through time on a recurrent neural network");

BPTT::BPTT() // DEFAULT VALUES FOR ALL OPTIONS
{
  batch_size = 1;
}

BPTT::~BPTT()
{
}

void BPTT::declareOptions(OptionList& ol)
{
  declareOption(ol, "optimizer", &BPTT::optimizer, OptionBase::buildoption, 
                "    specify the optimizer to use\n");

  declareOption(ol, "batch_size", &BPTT::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the average gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->length() \n");

  declareOption(ol, "nneuron_input", &BPTT::nneuron_input, OptionBase::buildoption, 
                "    number of input neuron in the network.");

  declareOption(ol, "nneuron_hidden", &BPTT::nneuron_hidden, OptionBase::buildoption, 
                "    number of hidden neuron in the network.");

  declareOption(ol, "nneuron_output", &BPTT::nneuron_output, OptionBase::buildoption, 
                "    number of output neuron in the network.");

  declareOption(ol, "seqs", &BPTT::temp, OptionBase::buildoption, 
                "    A sequence for the train set. (Temp. Testing and debug)");

  declareOption(ol, "links", &BPTT::links, OptionBase::buildoption, 
                "    A TMat of 3 columns use to define the links between neurons in the reccurent neural network. The first column correspond the no of the parent neuron. The second colunm correspond to the no of the child neuron. The third column correspond to the delay that the value will stay in the parent node before it will propagate the child neuron.");

  declareOption(ol, "units_type", &BPTT::units_type, OptionBase::buildoption, 
                "    A TVec of string with width equals to nneuron. Use to specified the squashing function of each neuron. The choices are LIN or TANH");

  declareOption(ol, "cost_type", &BPTT::cost_type, OptionBase::buildoption, 
                "    A string to specify the function to minimise. The choices are MSE");

  declareOption(ol, "params", &BPTT::params, OptionBase::learntoption, 
                "    The learned parameter vector");

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

void BPTT::build_()
{
  if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0) {
    weights = Vec(links.nrows(), 0.5);
    bias = Vec(nneuron_input + nneuron_hidden + nneuron_output, 1.0);
    params = VarArray(Var(weights), Var(bias));
    rec_net = new BPTTVariable(params, dynamic_cast<SequenceVMatrix*>((VMatrix*)train_set), batch_size,
			       links, nneuron_input, nneuron_hidden, nneuron_output, units_type, cost_type);
  } else {
    rec_net = 0;
  }
    
}

int BPTT::outputsize() const
{
  PLERROR("Unable to evaluate the output size without knowing the input set");
  return 0;
}

int BPTT::outputsize(VMat set) const
{ 
  SequenceVMatrix *seq = dynamic_cast<SequenceVMatrix*>((VMatrix*)set);
  if (!seq)
    PLERROR("The VMat given to BPTT::outputsize is not a SequenceVMatrix");
  return seq->getNbRowInSeqs(0, seq->getNbSeq());
}

TVec<string> BPTT::getTrainCostNames() const
{
  return TVec<string>(1, "MSE");
}

TVec<string> BPTT::getTestCostNames() const
{ 
  return TVec<string>(1, "MSE");
}


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
  bool early_stop=false;
  while(stage<nstages && !early_stop) {
    optimizer->nstages = optstage_per_lstage;
    train_stats->forget();
    optimizer->early_stop = false;
    optimizer->optimizeN(*train_stats);
    train_stats->finalize();
    if(verbosity>2) {
      cout << "Epoch " << stage << " train objective: " << train_stats->getMean() << endl;
      cout << weights;
      cout << endl;
    }
    ++stage;
    if(pb)
      pb->update(stage-initial_stage);
  }
  if(verbosity>1)
    cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;
  
  if(pb)
    delete pb;
}

void BPTT::test(VMat testset, PP<VecStatsCollector> test_stats, 
		VMat testoutputs, VMat testcosts) const
{
  int l = testset.length();
  Vec input;
  Vec target;
  real weight;

  Vec output(testoutputs ?outputsize(testset) :0);

  Vec costs(nTestCosts());

  // testset->defineSizes(inputsize(),targetsize(),weightsize());

  if(test_stats)
    test_stats->forget();

  ProgressBar* pb = NULL;
  if(report_progress) 
    pb = new ProgressBar("Testing learner",l);

  if (l == 0) {
    // Empty test set: we give -1 cost arbitrarily.
    costs.fill(-1);
    test_stats->update(costs);
  }

  for(int i=0; i<l; i++)
    {
      testset.getExample(i, input, target, weight);

      if(testoutputs)
        {
          computeOutputAndCosts(input, target, output, costs);
	  testoutputs->putOrAppendRow(i,output);
        }
      else // no need to compute outputs
        computeCostsOnly(input, target, costs);

      if(testcosts)
        testcosts->putOrAppendRow(i, costs);

      if(test_stats)
        test_stats->update(costs,weight);

      if(pb)
        pb->update(i);
    }

  if(test_stats)
    test_stats->finalize();

  if(pb)
    delete pb;

}


void BPTT::computeOutput(const Vec& inputv, Vec& outputv) const
{
  int ninputrow = inputv.length() / inputsize_;
  int ntargetrow = outputv.length() / targetsize_;
  Mat inputm = inputv.toMat(ninputrow, inputsize_);
  Mat outputm = Mat(ntargetrow, targetsize_);
  rec_net->computeOutputFromInput(inputm, outputm);
  outputv = outputm.toVecCopy();
}

void BPTT::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, 
                                   const Vec& targetv, Vec& costsv) const
{
  int nrow = outputv.length() / targetsize_;
  Mat outputm = outputv.toMat(nrow, targetsize_);
  Mat targetm = targetv.toMat(nrow, targetsize_);
  Mat costsm = Mat(nrow, targetsize_);
  rec_net->computeCostFromOutput(outputm, targetm, costsm);
  costsv = costsm.toVecCopy();
}

void BPTT::run() {
  rec_net->gradient[0] = 1.0;
  cout << rec_net->gradient[0] << endl;
  rec_net->verifyGradient();
}

void BPTT::initializeParams() {
  // Reset optimizer
  if(optimizer)
    optimizer->reset();
}

void BPTT::forget() {
  if (train_set) initializeParams();
  for (int i=0;i<nneuron_input + nneuron_hidden + nneuron_output;i++) {
    bias[i] = 0.01;
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
