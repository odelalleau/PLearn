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
   * $Id: IncrementalNNet.cc,v 1.6 2005/05/31 02:57:17 yoshua Exp $ 
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file IncrementalNNet.cc */


#include "IncrementalNNet.h"

namespace PLearn {
using namespace std;

IncrementalNNet::IncrementalNNet() 
  : n_examples_seen(0),
    current_average_cost(0),
    next_average_cost(0),
    candidate_unit_bias(0),
    n_outputs(1),
    output_weight_decay(0),
    online(true),
    minibatch_size(0),
    output_cost_type("squared_error"),
    boosting(false),
    minimize_local_cost(false),
    hard_activation_function(false),
    use_hinge_loss_for_hard_activation(true),
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
                "use online or batch version? only consider adding a hidden unit after minibatch_size examples\n"
                "Add a hidden unit only if it would reduce the average cost (including the L1 penalty).\n"
                "This current_average_cost is calculated either with a moving average over a moving target (online version)\n"
                "or the algorithm proceeds in two phases (batch version): on even batches one improves the\n"
                "tentative hidden unit, while on odd batches one evaluates its quality.\n");

  declareOption(ol, "minibatch_size", &IncrementalNNet::minibatch_size, OptionBase::buildoption,
                "0 is a special value meaning minibatch_size == training set size.\n"
                "After a hidden unit is added, wait at least that number of examples before considering\n"
                "to add a new one.\n");

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

  declareOption(ol, "hard_activation_function", &IncrementalNNet::hard_activation_function, OptionBase::buildoption,
                "if true then h(x) = sign(w'x + b), else h(x) = tanh(w'x + b).\n");

  declareOption(ol, "use_hinge_loss_for_hard_activation", &IncrementalNNet::use_hinge_loss_for_hard_activation, OptionBase::buildoption,
                "use hinge loss or cross-entropy to train hidden units, when hard_activation_function\n");

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

  declareOption(ol, "candidate_unit_weights", &IncrementalNNet::candidate_unit_weights, OptionBase::learntoption,
                "vector of weights from input to next candidate hidden unit.\n");

  declareOption(ol, "candidate_unit_bias", &IncrementalNNet::candidate_unit_bias, OptionBase::learntoption,
                "bias parameter of next candidate hidden unit.\n");

  declareOption(ol, "n_examples_seen", &IncrementalNNet::n_examples_seen, OptionBase::learntoption,
                "number of training examples seen (= number of updates done) seen beginning of training.\n");

  declareOption(ol, "current_average_cost", &IncrementalNNet::current_average_cost, OptionBase::learntoption,
                "current average cost, including fitting and regularization terms. It is computed\n"
                "differently according to the online and minibatch_size options.\n");

  declareOption(ol, "next_average_cost", &IncrementalNNet::next_average_cost, OptionBase::learntoption,
                "average cost if candidate hidden unit was included. It is computed like current_average_cost.\n");

  declareOption(ol, "n_examples_training_candidate", &IncrementalNNet::n_examples_training_candidate, OptionBase::learntoption,
                "number of examples seen since started to train current candidate hidden unit. Used in\n"
                "stopping criterion: stop when n_examples_training_candidate >= max_n_epochs_to_fail * train_set->length().\n");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void IncrementalNNet::build_()
{
  output_biases.resize(n_outputs);
  linear_output.resize(n_outputs);
  output_with_candidate.resize(n_outputs);
  output_gradient_with_candidate.resize(n_outputs);
  act.resize(stage);
  h.resize(stage);
  costs_with_candidate.resize(3);
  if (output_cost_type=="squared_error")
    cost_type=0;
  else if (output_cost_type=="hinge_loss")
    cost_type=1;
  else if (output_cost_type=="discrete_log_likelihood")
    cost_type=2;
  else PLERROR("IncrementalNNet:build: output_cost_type should either be 'squared_error', 'hinge_loss', or 'discrete_log_likelihood'");
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
  deepCopyField(output_biases, copies);
  deepCopyField(hidden_layer_weights, copies);
  deepCopyField(hidden_layer_biases, copies);
  deepCopyField(candidate_unit_weights, copies);
  deepCopyField(act, copies);
  deepCopyField(h, copies);
  deepCopyField(linear_output, copies);
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
  next_average_cost=0;
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
  static Vec output_gradient;
  static Vec hidden_gradient;
  static Vec output_with_candidate;
  static Vec output_with_signchange;
  static Mat candidate_unit_output_weights_mat;
  static Vec candidate_h_vec;
  static Vec candidate_hidden_gradient;
  input.resize(inputsize());    // the train_set's inputsize()
  output.resize(outputsize());    
  output_gradient.resize(outputsize());    
  hidden_gradient.resize(stage);    
  output_with_candidate.resize(outputsize());    
  output_with_signchangne.resize(outputsize());    
  target.resize(targetsize());  // the train_set's targetsize()
  candidate_unit_output_weights_mat = candidate_unit_output_weights.toMat(n_outputs,1);
  candidate_h_vec.resize(1);
  candidate_hidden_gradient.resize(1);
  real sampleweight; // the train_set's weight on the current example
  real new_cost;

  if(!train_stats)  // make a default stats collector, in case there's none
    train_stats = new VecStatsCollector();

  if(nstages<stage) // asking to revert to a previous stage!
    forget();  // reset the learner to stage=0

  bool stopping_criterion_not_met = true;

  real moving_average_coefficient = 1.0/minibatchsize;

  while(stage<nstages && stopping_criterion_not_met)
  {
    // clear statistics of previous epoch
    train_stats->forget() ;
  
    // iterate through the data for some time...
    do 
    {
      train_set->getSample(input, target, sampleweight);
      computeOutput(input,output);
      computeCostsFromOutputs(input,output,target,train_costs);
      train_costs*=sampleweight;
      train_stats->update(train_costs);
      // compute output and cost IF WE USED THE CANDIDATE HIDDEN UNIT
      real candidate_act = 
        dot(input, candidate_unit_weights) + candidate_unit_bias;
      real candidate_h;
      if (hard_activation_function)
        candidate_h = sign(candidate_act);
      else
        candidate_h = tanh(candidate_act);
      candidate_h_vec[0]=candidate_h;
      // linear_output_with_candidate = linear_output + candidate_unit_output_weight*candidate_h;
      multiplyAdd(linear_output,candidate_unit_output_weight,
                  candidate_h,linear_output_with_candidate);
      if (cost_type == 2) // "discrete_log_likelihood"
        softmax(output_with_candidate,output_with_candidate);
      computeCostsFromOutputs(input,output_with_candidate,target,costs_with_candidate); 

      real learning_rate = initial_learning_rate / ( 1 + n_examples_seen * decay_factor );

      // backprop & update regular network parameters
      if (!boosting) // i.e. continue training the existing hidden units
      {
        // ** compute gradient on linear output
        output_loss_gradient(output, target, output_gradient, sampleweight);

        // ** bprop through the network & update

        // bprop on output layer
        multiplyAcc(output_biases, output_gradient, learning_rate);
        layerL1BpropUpdate(hidden_gradient, output_weights, h, output_gradient, learning_rate, output_weight_decay);

        // bprop through hidden units activation
        if (hard_activation_function) 
          // Should h_i(x) change of sign?
          // Consider the loss that would occur if it did, i.e. with output replaced by output - 2*W[.,i]*h_i(x)
          // Then consider a weighted classification problem
          // with the appropriate sign and weight = gradient on h_i(x).
        {
          real current_fit_error = train_costs[1];
          for (int i=0;i<stage;i++) // loop over hidden units
          {
            Vec Wi = output_weights.column(i).toVec();
            multiplyAdd(output,Wi,-2*h[i],output_with_signchange);
            real fit_error_with_sign_change = output_loss(output_with_signchange,target);
            int target_i = sign(fit_error_with_sign_change-current_fit_error)*h[i];
            real weight_i = fabs(hidden_gradient[i]); // CHECK: when is the sign of hidden_gradient different from (h[i]-target_i)?
            if (use_hinge_loss_for_hard_activation)
              hidden_gradient[i] = weight_i * d_hinge_loss(act[i],target_i);
            else // use cross-entropy
              hidden_gradient[i] = weight_i * (sigmoid(act[i]) - 2(target_i+1));
          }
        }
        else
          bprop_tanh(h,hidden_gradient,hidden_gradient);

        // bprop through hidden layer and update hidden_biases and hidden_weights
        hidden_gradient *= -learning_rate;
        hidden_biases += hidden_gradient;
        externalProductAcc(hidden_weights, hidden_gradient, input);
      }

      // backprop & update candidate hidden unit
      output_loss_gradient(output_with_candidate, target, output_gradient, sampleweight);
      layerBpropUpdate(candidate_hidden_gradient, candidate_unit_output_weights_mat, 
                       candidate_h_vec, output_gradient, learning_rate);
      // bprop through candidate hidden unit activation, heuristic method
      if (hard_activation_function)
      {
        multiplyAdd(output_with_candidate,candidate_unit_output_weights,-2*candidate_h,output_with_signchange);
        real fit_error_with_sign_change = output_loss(output_with_signchange,target);
        int hidden_class = sign(fit_error_with_sign_change-current_fit_error)*candidate_h;
        real weight_on_loss = fabs(candidate_hidden_gradient[0]); // CHECK: when is the sign of hidden_gradient different from (h[i]-target_i)?
        if (use_hinge_loss_for_hard_activation)
          candidate_hidden_gradient[0] = weight_on_loss * d_hinge_loss(candidate_act,hidden_class);
        else // use cross-entropy
          candidate_hidden_gradient[0] = weight_on_loss * (sigmoid(candidate_act) - 2(hidden_class+1));
      }
      else
        bprop_tanh(candidate_h_vec,candidate_hidden_gradient,candidate_hidden_gradient);        
      candidate_hidden_gradient *= -learning_rate;
      candidate_unit_bias += candidate_hidden_gradient[0];
      multiplyAcc(candidate_unit_weights, input, candidate_hidden_gradient[0]);

      // keep track of average performance with and without candidate hidden unit
      n_examples_seen++;
      int n_batches_seen = n_examples_seen / minibatchsize;
      int t_since_beginning_of_batch = n_examples_seen - n_batches_seen*minibatchsize;
      if (!online)
        moving_average_coefficient = 1.0/(1+t_since_beginning_of_batch);
      current_average_cost = moving_average_coefficient*train_costs[0]
        +(1-moving_average_coefficient)*current_average_cost;
      next_average_cost = moving_average_coefficient*costs_with_candidate[0]
        +(1-moving_average_coefficient)*next_average_cost;

      // consider inserting the candidate hidden unit (at every minibatchsize examples)
      if (t_since_beginning_of_batch == 0)
      {
        n_examples_seen_since_training_candidate += minibatchsize;
        if (next_average_cost < current_average_cost) 
        {
          // insert candidate hidden unit
          // ...
          // initialize a new candidate
          n_examples_seen_since_training_candidate=0;
        } else // should we stop?
        {
          if (n_examples_training_candidate >= max_n_epochs_to_fail*train_set->size())
            stopping_criterion_not_met = false;
        }
        if (!online)
          current_average_cost = 0;
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
  product(act,hidden_layer_weights, input);
  act+=hidden_biases;
  if (hard_activation_function)
    compute_tanh(act,h);
  else
    compute_sign(act,h);
  product(linear_output,output_weights,h);
  linear_output+=output_biases;
  if (cost_type==2) // "discrete_log_likelihood"
    softmax(linear_output,output);
  else
    output << linear_output;
}    

real IncrementalNNet::output_loss(const Vec& output,const Vec& target)
{
  real fit_error=0;
  if (cost_type == 0) // "squared_error"
    fit_error = powdistance(output,target);
  else {
    int target_class = target[0];
    if (cost_type == 1) // "hinge_loss", one against all binary classifiers
      fit_error = one_against_all_hinge_loss(output,target);
    else // (output_cost_type == "discrete_log_likelihood")
      fit_error = - safelog(output[target_class]);
  }
  return fit_error;
}

void IncrementalNNet::output_loss_gradient(const Vec& output,const Vec& target,
                                           Vec output_gradient, real sampleweight)
{
  if (cost_type==0) // "squared_error"
  {
    substract(output,target,output_gradient);
    output_gradient *= sampleweight * 2;
    return;
  }
  int target_class = target[0];
  if (cost_type==1) // "hinge_loss"
  {
    one_against_all_hinge_loss_bprop(output,target_class,
                                     output_gradient);
    if (sampleweight!=1)
      output_gradient *= sampleweight;
  }
  else // (output_cost_type=="discrete_log_likelihood")
  {
    int target_class = target[0];
    for (int i=0;i<n_outputs;i++)
    {
      real y_i = (target_class==i)?1:0;
      output_gradient[i] = sampleweight*(output[i] - y_i);
    }
  }  
}

void IncrementalNNet::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                              const Vec& target, Vec& costs) const
{
  // Compute the costs from *already* computed output. 
  real fit_error = output_loss(output,target);
  real regularization_penalty = output_weight_decay * sumabs(output_weights);
  costs[0] = fit_error + regularization_penalty;
  costs[1] = fit_error;
  costs[2] = regularization_penalty;
}                                

TVec<string> IncrementalNNet::getTestCostNames() const
{
  // Return the names of the costs computed by computeCostsFromOutpus
  // (these may or may not be exactly the same as what's returned by getTrainCostNames).
  TVev<string> names(3);
  names[0]=output_cost_type+"+L1_regularization";
  names[1]=output_cost_type;
  names[2]="+L1_regularization";
  return names;
}

TVec<string> IncrementalNNet::getTrainCostNames() const
{
  // Return the names of the objective costs that the train method computes and 
  // for which it updates the VecStatsCollector train_stats
  // (these may or may not be exactly the same as what's returned by getTestCostNames).
  return getTestCostNames();
}


} // end of namespace PLearn
