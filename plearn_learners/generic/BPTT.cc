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
  alpha = 0.01;
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

  declareOption(ol, "alpha", &BPTT::alpha, OptionBase::buildoption, 
                "    Coefficent use when update the weights");

  declareOption(ol, "cost_type", &BPTT::cost_type, OptionBase::buildoption, 
                "    A string to specify the function to minimise. The choices are MSE");

  declareOption(ol, "params", &BPTT::params, OptionBase::learntoption, 
                "    The learned parameter vector");

  inherited::declareOptions(ol);
}

void BPTT::build()
{
  inherited::build();
  build_();
}

void BPTT::build_()
{
  // Don't do anything if we don't have a train_set
  // It's the only one who knows the inputsize and targetsize anyway...
  cout << "build_()" << endl;
  inputsize_ = temp.inputsize();
  targetsize_ = temp.targetsize();
  weightsize_ = temp.weightsize();
  if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
    {
      weights = Vec(links.nrows(), (real)0.5);
      bias = Vec(nneuron_input + nneuron_hidden + nneuron_output, (real)0.01);
      params = VarArray(Var(weights), Var(bias));
      rec_net = BPTTVariable(params, &temp /*dynamic_cast<SequenceVMatrix*>((VMatrix*)train_set)*/, batch_size,
			     links, nneuron_input, nneuron_hidden, nneuron_output, alpha, units_type, cost_type);
    }
}

int BPTT::outputsize() const
{ return targetsize_; }

TVec<string> BPTT::getTrainCostNames() const
{
  return TVec<string>();
}

TVec<string> BPTT::getTestCostNames() const
{ 
  return TVec<string>();
}


void BPTT::train()
{
  cout << "train()" << endl;
  rec_net.fbprop();
  rec_net.printState();
  cout << "Error : " << rec_net.value[0] << endl;
}


void BPTT::computeOutput(const Vec& inputv, Vec& outputv) const
{

}

void BPTT::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                 Vec& outputv, Vec& costsv) const
{
}


void BPTT::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, 
                                   const Vec& targetv, Vec& costsv) const
{

}

void BPTT::run() {
  cout << "run()" << endl;
  rec_net.printOrder();
  rec_net.verifyGradient();
}

void BPTT::initializeParams()
{
  // Reset optimizer
  if(optimizer)
    optimizer->reset();
}

void BPTT::forget()
{
  if (train_set) initializeParams();
  stage = 0;
}

void BPTT::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

} // end of namespace PLearn
