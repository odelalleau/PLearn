// -*- C++ -*-

// IncrementalNNet.cc
//
// Copyright (C) 2005 Yoshua Bengio 
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
   * $Id: IncrementalNNet.cc,v 1.1 2005/05/27 15:44:42 yoshua Exp $ 
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file IncrementalNNet.cc */


#include "IncrementalNNet.h"

namespace PLearn {
using namespace std;

IncrementalNNet::IncrementalNNet() 
  : n_outputs(1),
    output_weight_decay(0),
    online(true),
    minibatch_size(0),
    output_cost_type("squared_error"),
    boosting(false),
    minimize_local_cost(false),
    hard_activation_function(false),
    n_epochs_before_considering_a_new_unit(1),
    initial_learning_rate(0.01),
    decay_factor(1e-6),
    max_n_epochs_to_fail(1)
{
}

PLEARN_IMPLEMENT_OBJECT(IncrementalNNet, 
                        "Incremental one-hidden-layer neural network with L1 regularization of output weights",
                        "Stops either when the number of hidden units (==stage) reaches the user-specified\n"
                        "maximum (nstages) or when it does not appear possible to add a hidden unit without\n"
                        "increasing the penalized cost.");

void IncrementalNNet::declareOptions(OptionList& ol)
{

  declareOption(ol, "n_outputs", &IncrementalNNet::n_outputs, OptionBase::buildoption,
                "Number of output units. Must be coherent with output_cost_type and targetsize:\n"
                "n_outputs==targetsize for 'squared_error', and targetsize==1 && n_outputs==n_classes for\n"
                "hinge_loss and discrete_log_likelihood.\n");

  declareOption(ol, "output_weight_decay", &IncrementalNNet::output_weight_decay, OptionBase::buildoption,
                "L1 regularizer's penalty factor on output weights.");

  declareOption(ol, "online", &IncrementalNNet::online, OptionBase::buildoption,
                "use online or batch version? if batch only consider adding a hidden unit after minibatch_size examples\n"
                "Add a hidden unit only if it would reduce the average cost (including the L1 penalty).\n"
                "This current_average_cost is calculated either with a moving average over a moving target (online version)\n"
                "or the algorithm proceeds in two phases (batch version): on even batches one improves the\n"
                "tentative hidden unit, while on odd batches one evaluates its quality.\n");

  declareOption(ol, "minibatch_size", &IncrementalNNet::minibatch_size, OptionBase::buildoption,
                "0 is a special value meaning minibatch_size == training set size. In batch mode (online==false)\n"
                "it is the length\n");

  declareOption(ol, "output_cost_type", &IncrementalNNet::output_cost_type, OptionBase::buildoption,
                "'squared_error', 'hinge_loss', 'discrete_log_likelihood' (for probabilistic classification).\n");

  declareOption(ol, "boosting", &IncrementalNNet::boosting, OptionBase::buildoption,
                "use a boosting-like approach (only works with online=false) and train the new hidden unit \n"
                "but not the previous ones; also descend not the actual cost but a weighted cost obtained\n"
                "from the gradient of the output cost on the hidden unit function (see minimize_local_cost option).\n");

  declareOption(ol, "minimize_local_cost", &IncrementalNNet::minimize_local_cost, OptionBase::buildoption,
                "if true then instead of minimize global cost sum_t Q(f(x_t),y_t),\n"
                "each hidden unit minimizes sum_t Q'(f(x_t),y_t) h(x_t)\n"
                "or some approximation of it if h is a hard threshold (weighted logistic regression cost\n"
                "with targets sign(Q'(f(x_t),y_t)) and weights |Q'(f(x_t),y_t)|),\n"
                "where Q is the output cost, f(x_t) is the current prediction, y_t the target, h(x_t) the\n"
                "output of the new hidden unit.\n");

  declareOption(ol, "hidden_activation_function", &IncrementalNNet::, OptionBase::buildoption,
                "if true then h(x) = sign(w'x + b), else h(x) = tanh(w'x + b).\n");

  declareOption(ol, "n_epochs_before_considering_a_new_unit", &IncrementalNNet::n_epochs_before_considering_a_new_unit, 
                OptionBase::buildoption, 
                "After a hidden unit is added, wait at least that fraction of the training set before considering\n"
                "to add a new one.\n");

  declareOption(ol, "initial_learning_rate", &IncrementalNNet::initial_learning_rate, OptionBase::buildoption,
                "learning_rate = initial_learning_rate / (1 + n_examples_seen * decay_factor).\n");

  declareOption(ol, "decay_factor", &IncrementalNNet::decay_factor, OptionBase::buildoption,
                "decay factor in learning_rate formula.\n");

  declareOption(ol, "max_n_epochs_to_fail", &IncrementalNNet::max_n_epochs_to_fail, OptionBase::buildoption,
                "Maximum number of epochs (not necessarily an integer) to try improving the new hidden unit\n"
                "before declaring failure to improve the regularized cost (and thus stopping training).\n");

  //declareOption(ol, "", &IncrementalNNet::, OptionBase::buildoption,

  declareOption(ol, "output_weights", &IncrementalNNet::output_weights, OptionBase::learntoption,
                "matrix of [hidden_unit, output] output weights.\n"
                "** NOTE IT IS TRANSPOSED ** with respect to\n"
                "the 'natural' index order, so as to easily add hidden units.\n");

  declareOption(ol, "output_biases", &IncrementalNNet::output_biases, OptionBase::learntoption,
                "vector of output biases\n");

  declareOption(ol, "hidden_layer_weights", &IncrementalNNet::hidden_layer_weights, OptionBase::learntoption,
                "matrix of weights from input to hidden units: [hidden_unit, input].\n");

  declareOption(ol, "hidden_layer_biases", &IncrementalNNet::hidden_layer_biases, OptionBase::learntoption,
                "vector of biases of the hidden units.\n");

  declareOption(ol, "n_examples_seen", &IncrementalNNet::n_examples_seen, OptionBase::learntoption,
                "number of training examples seen (= number of updates done) seen beginning of training.\n");

  declareOption(ol, "current_average_cost", &IncrementalNNet::current_average_cost, OptionBase::learntoption,
                "current average cost, including fitting and regularization terms. It is computed\n"
                "differently according to the online and minibatch_size options.\n");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void IncrementalNNet::build_()
{
  output_biases.resize(n_outputs);
}

// ### Nothing to add here, simply calls build_
void IncrementalNNet::build()
{
  inherited::build();
  build_();
}


void IncrementalNNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(output_weights, copies);
  deepCopyField(hidden_layer_weights, copies);
  deepCopyField(hidden_layer_biases, copies);
}


int IncrementalNNet::outputsize() const
{
  return n_outputs;
}

void IncrementalNNet::forget()
{
  // reset the number of hidden units to 0 = nstages
  output_weights.resize(0,n_outputs);
  hidden_layer_weights.resize(0,inputsize());
  hidden_layer_biases.resize(0);
  output_biases.clear();
  stage=0;
  n_examples_seen=0;
  current_average_cost=0;
}
    
void IncrementalNNet::train()
{
  // The role of the train method is to bring the learner up to stage==nstages,
  // updating train_stats with training costs measured on-line in the process.

  if (!train_set)
    PLERROR("IncrementalNNet::train train_set must be set before calling train\n");
  if (output_cost_type == "squared_error" &&
      train_set->targetsize() != n_outputs)
    PLERROR("IncrementalNNet::train with 'squared_error' output_cost_type, train_set->targetsize(%d) should equal n_outputs(%d)",
            train_set->targetsize(),n_outputs);
  if ((output_cost_type == "hinge_loss" || output_cost_type == "discrete_log_likelihood") &&
      train_set->targetsize()!=1)
    PLERROR("IncrementalNNet::train 'hinge_loss' or 'discrete_log_likelihood' output_cost_type is for classification, train_set->targetsize(%d) should be 1",
            train_set->targetsize());

  int minibatchsize = minibatch_size;
  if (minibatch_size == 0)
    minibatchsize = train_set->length();

  static Vec input;  // static so we don't reallocate/deallocate memory each time...
  static Vec output;
  static Vec target; // (but be careful that static means shared!)
  input.resize(inputsize());    // the train_set's inputsize()
  output.resize(outputsize());    // the train_set's inputsize()
  target.resize(targetsize());  // the train_set's targetsize()
  real sampleweight; // the train_set's weight on the current example

  if(!train_stats)  // make a default stats collector, in case there's none
    train_stats = new VecStatsCollector();

  if(nstages<stage) // asking to revert to a previous stage!
    forget();  // reset the learner to stage=0

  bool stopping_criterion_not_met = true;

  while(stage<nstages && stopping_criterion_not_met)
  {
    // clear statistics of previous epoch
    train_stats->forget() ;
  
    // iterate through the data for some time...
    do 
    {
      train_set->getSample(input, target, sampleweight);
      computeOutput(input,output);
      computeCostsfromOutputs(input,output,target,train_costs);
      train_costs*=sampleweight;
      train_stats->update(train_costs);
      if (online)
        current_average_cost = train_costs[0];
      else
        current_average_cost = train_costs[0];
      n_examples_seen++;
      if (n_examples_seen % minibatchsize == 0) // consider adding a hidden unit
      {
      }
    }
    until (stage!=nstages || !stopping_criterion_not_met);
          
    ++stage;
    train_stats->finalize(); // finalize statistics for this epoch
  }
}


void IncrementalNNet::computeOutput(const Vec& input, Vec& output) const
{
  // Compute the output from the input.
  int nout = outputsize();
  output.resize(nout);
  
}    

void IncrementalNNet::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output. 
// ...
}                                

TVec<string> IncrementalNNet::getTestCostNames() const
{
  // Return the names of the costs computed by computeCostsFromOutpus
  // (these may or may not be exactly the same as what's returned by getTrainCostNames).
  // ...
}

TVec<string> IncrementalNNet::getTrainCostNames() const
{
  // Return the names of the objective costs that the train method computes and 
  // for which it updates the VecStatsCollector train_stats
  // (these may or may not be exactly the same as what's returned by getTestCostNames).
  // ...
}


} // end of namespace PLearn
