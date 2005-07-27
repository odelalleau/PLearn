// -*- C++ -*-

// IncrementalNNet.cc
//
// Copyright (C) 2005 Yoshua Bengio, Mantas Lukosevicius 
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
   * $Id$ 
   ******************************************************* */

// Authors: Yoshua Bengio, Mantas Lukosevicius

/*! \file IncrementalNNet.cc */


#include "IncrementalNNet.h"

namespace PLearn {
using namespace std;

IncrementalNNet::IncrementalNNet() 
  : internal_weights(0),
    internal_weight_gradients(0),
    candidate_unit_bias(0),
    n_examples_seen(0),
    current_average_cost(0),
    next_average_cost(0),
    n_examples_training_candidate(0),
    current_example(0),
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
    max_n_epochs_to_fail(1),
    rand_range(1),
    enable_internal_weights(false),
    incremental_connections(false),
    connection_gradient_threshold(0.5),
    residual_correlation_gradient(true)
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
                
  declareOption(ol, "rand_range", &IncrementalNNet::rand_range, OptionBase::buildoption,
                "Interval of random numbers when initializing weights/biases: (-rand_range/2, rand_range/2).\n");

  declareOption(ol, "enable_internal_weights", &IncrementalNNet::enable_internal_weights, OptionBase::buildoption,
                "Network has a cascade topology (each hidden unit has connections to all previous ones) if true,\n" 
                "or a one hidden layer topology if false (default).\n");
  
  declareOption(ol, "incremental_connections", &IncrementalNNet::incremental_connections, OptionBase::buildoption,
                "Add hidden connections incrementally if true, or all at once with a new unit if false (default).\n"
                "This option is only supported with n_outputs == 1." );

  declareOption(ol, "connection_gradient_threshold", &IncrementalNNet::connection_gradient_threshold, OptionBase::buildoption,
                "Threshold of gradient for connection to be added, when incremental_connections == true." );

  declareOption(ol, "residual_correlation_gradient", &IncrementalNNet::residual_correlation_gradient, OptionBase::buildoption,
                "Use residual correlation gradient (ConvexNN) if true (default), or classical error back-propagation if false." );


  //declareOption(ol, "", &IncrementalNNet::, OptionBase::buildoption,

  declareOption(ol, "output_weights", &IncrementalNNet::output_weights, OptionBase::learntoption,
                "matrix of [hidden_unit, output] output weights.\n"
                "** NOTE IT IS TRANSPOSED ** with respect to\n"
                "the 'natural' index order, so as to easily add hidden units.\n");
    
  declareOption(ol, "output_weight_gradients", &IncrementalNNet::output_weight_gradients, OptionBase::learntoption,
                "Moving average gradients on matrix of [hidden_unit, output] output weights\n"
                "(enabled by residual_correlation_gradient && outputsize() > 1).\n"
                "** NOTE IT IS TRANSPOSED ** with respect to\n"
                "the 'natural' index order, so as to easily add hidden units.\n");

  declareOption(ol, "output_biases", &IncrementalNNet::output_biases, OptionBase::learntoption,
                "vector of output biases\n");

  declareOption(ol, "hidden_layer_weights", &IncrementalNNet::hidden_layer_weights, OptionBase::learntoption,
                "matrix of weights from input to hidden units: [hidden_unit, input].\n");
  
  declareOption(ol, "hidden_layer_weight_gradients", &IncrementalNNet::hidden_layer_weight_gradients, OptionBase::learntoption,
                "Moving average gradients on hidden_layer_weights (enabled by incremental_connections).\n");

  declareOption(ol, "internal_weights", &IncrementalNNet::internal_weights, OptionBase::learntoption,
                "weights among hidden units [to, from] in cascade architecture (enabled by enable_internal_weights).\n");

  declareOption(ol, "internal_weight_gradients", &IncrementalNNet::internal_weight_gradients, OptionBase::learntoption,
                "Moving average gradients on internal_weights (enabled by incremental_connections).\n");

  declareOption(ol, "hidden_layer_biases", &IncrementalNNet::hidden_layer_biases, OptionBase::learntoption,
                "vector of biases of the hidden units.\n");

  declareOption(ol, "candidate_unit_weights", &IncrementalNNet::candidate_unit_weights, OptionBase::learntoption,
                "vector of weights from input to next candidate hidden unit.\n");

  declareOption(ol, "candidate_unit_weight_gradients", &IncrementalNNet::candidate_unit_weight_gradients, OptionBase::learntoption,
                "Moving average gradients on candidate_unit_weights (enabled by incremental_connections).\n");

  declareOption(ol, "candidate_unit_bias", &IncrementalNNet::candidate_unit_bias, OptionBase::learntoption,
                "bias parameter of next candidate hidden unit.\n");

  declareOption(ol, "candidate_unit_output_weights", &IncrementalNNet::candidate_unit_output_weights, OptionBase::learntoption,
                "vector of weights from next candidate hidden unit to outputs.\n");
  
  declareOption(ol, "candidate_unit_output_weight_gradients", &IncrementalNNet::candidate_unit_output_weight_gradients,
                OptionBase::learntoption,
                "Moving average gradients on vector of weights from next candidate hidden unit to outputs.\n"
                "(enabled by residual_correlation_gradient && outputsize() > 1).\n");
      
  declareOption(ol, "candidate_unit_internal_weights", &IncrementalNNet::candidate_unit_internal_weights, OptionBase::learntoption,
                "vector of weights from previous hidden units to the candidate unit (enabled by enable_internal_weights).\n");

  declareOption(ol, "candidate_unit_internal_weight_gradients", &IncrementalNNet::candidate_unit_internal_weight_gradients,
                OptionBase::learntoption,
                "Moving average gradients on candidate_unit_internal_weights (enabled by incremental_connections).\n");

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
  if (output_cost_type=="squared_error")
    cost_type=0;
  else if (output_cost_type=="hinge_loss")
    cost_type=1;
  else if (output_cost_type=="discrete_log_likelihood")
    cost_type=2;
  else PLERROR("IncrementalNNet:build: output_cost_type should either be 'squared_error', 'hinge_loss', or 'discrete_log_likelihood'");
  
  if(!train_set) return;
      
  output_weights.resize(stage,n_outputs);
  output_biases.resize(n_outputs);
  hidden_layer_weights.resize(stage,inputsize_);
  hidden_layer_biases.resize(stage);
 
  linear_output.resize(n_outputs);
  act.resize(stage);
  h.resize(stage);
  
  candidate_unit_output_weights.resize(n_outputs);
  candidate_unit_weights.resize(inputsize_);
  
  if ( enable_internal_weights ) {
    internal_weights.resize(stage); //.clear();
    candidate_unit_internal_weights.resize(stage);
  }
  if ( incremental_connections ) {
    hidden_layer_weight_gradients.resize(stage,inputsize_);
    candidate_unit_weight_gradients.resize(inputsize_);
    if ( enable_internal_weights ) {
      internal_weight_gradients.resize(stage);
      candidate_unit_internal_weight_gradients.resize(stage);
    }
  } 
  if ( residual_correlation_gradient & n_outputs > 1 ) {
    output_weight_gradients.resize(stage,n_outputs);
    candidate_unit_output_weight_gradients.resize(n_outputs);
  }

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
  deepCopyField(output_weight_gradients, copies);
  deepCopyField(output_biases, copies);
  deepCopyField(hidden_layer_weights, copies);
  deepCopyField(hidden_layer_weight_gradients, copies);
  deepCopyField(hidden_layer_biases, copies);
  deepCopyField(internal_weights, copies);
  deepCopyField(internal_weight_gradients, copies);
  deepCopyField(candidate_unit_weights, copies);  
  deepCopyField(candidate_unit_weight_gradients, copies);
  deepCopyField(candidate_unit_output_weights, copies);
  deepCopyField(candidate_unit_output_weight_gradients, copies);
  deepCopyField(candidate_unit_internal_weights, copies);
  deepCopyField(candidate_unit_internal_weight_gradients, copies);
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
  // reset the number of hidden units to 0 = stage
  stage=0;
  n_examples_seen=0;
  current_average_cost=0;
  next_average_cost=0;
  current_example=0;
  // resize all the matrices, vectors with stage=0
  build_();
  
  candidate_unit_output_weights.fill(0.1);
  candidate_unit_bias = ((real)rand()/RAND_MAX - 0.5)*rand_range;
  if (!incremental_connections) {
    for( int i=0; i < inputsize_; i++ )
      candidate_unit_weights[i] = ((real)rand()/RAND_MAX - 0.5)*rand_range; 
  } else {
    candidate_unit_weights.fill(0.0);
    candidate_unit_weight_gradients.fill(0.0);
  }
  if ( residual_correlation_gradient && n_outputs > 1 ){
    candidate_unit_output_weight_gradients.fill(0.0);
  }
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
//  if ( incremental_connections && n_outputs != 1 )
//    PLERROR("IncrementalNNet::train incremental_connections is only supported with n_outputs == 1\n");

  int minibatchsize = minibatch_size;
  if (minibatch_size == 0)
    minibatchsize = train_set->length();

  real current_average_class_error=0;
  real next_average_class_error=0;

  static Vec input;  // static so we don't reallocate/deallocate memory each time...
  static Vec output;
  static Vec target; // (but be careful that static means shared!)
  static Vec train_costs;
  static Vec costs_with_candidate;
  static Vec output_gradient;
  static Vec hidden_gradient;
  static Vec output_with_candidate;
  static Vec output_gradient_with_candidate;
  static Vec output_with_signchange;
  static Mat candidate_unit_output_weights_mat;
  static Vec candidate_h_vec;
  static Vec candidate_hidden_gradient;
  static Vec linear_output_with_candidate;
  int nc=nTrainCosts();
  train_costs.resize(nc);
  costs_with_candidate.resize(nc);
  input.resize(inputsize());    // the train_set's inputsize()
  output.resize(n_outputs);    
  output_gradient.resize(n_outputs);    
  hidden_gradient.resize(stage);    
  output_with_candidate.resize(n_outputs);
  output_gradient_with_candidate.resize(n_outputs);
  output_with_signchange.resize(n_outputs);    
  target.resize(targetsize());  // the train_set's targetsize()
  candidate_unit_output_weights_mat = candidate_unit_output_weights.toMat(n_outputs,1);
  candidate_h_vec.resize(1);
  candidate_hidden_gradient.resize(1);
  linear_output_with_candidate.resize(n_outputs);
  real sampleweight; // the train_set's weight on the current example

  if(!train_stats)  // make a default stats collector, in case there's none
    train_stats = new VecStatsCollector();

  if(nstages<stage) // asking to revert to a previous stage!
    forget();  // reset the learner to stage=0

  bool stopping_criterion_not_met = true;

  moving_average_coefficient = 1.0/minibatchsize;

  while(stage<nstages && stopping_criterion_not_met)
  {
    // clear statistics of previous epoch
    train_stats->forget() ;
  
    // iterate through the data for some time...
    do 
    {
      // compute output and cost
      train_set->getExample(current_example, input, target, sampleweight);
      current_example++; 
      if (current_example==train_set->length()) current_example=0;
      computeOutput(input,output);
      computeCostsFromOutputs(input,output,target,train_costs);
      real current_total_cost = train_costs[0];
      real current_fit_error = train_costs[1];
      real current_class_error = (cost_type!=0)?train_costs[3]:0;
      train_costs*=sampleweight;
      train_stats->update(train_costs);
      // compute output and cost IF WE USED THE CANDIDATE HIDDEN UNIT
      real candidate_act = 
        dot(input, candidate_unit_weights) + candidate_unit_bias;
      if ( enable_internal_weights && stage > 0 ) 
        candidate_act += dot( h, candidate_unit_internal_weights );
      real candidate_h;
      if (hard_activation_function)
        candidate_h = sign(candidate_act);
      else
        candidate_h = tanh(candidate_act);
      candidate_h_vec[0]=candidate_h;
      // linear_output_with_candidate = linear_output + candidate_unit_output_weight*candidate_h;
      multiplyAdd(linear_output,candidate_unit_output_weights,
                  candidate_h,linear_output_with_candidate);
      if (cost_type == 2) // "discrete_log_likelihood"
        softmax(linear_output_with_candidate,output_with_candidate);
      else
        output_with_candidate << linear_output_with_candidate;
      computeCostsFromOutputs(input,output_with_candidate,target,costs_with_candidate); 
      // computeCostsFromOutputs does not count the cost of the candidate's output weights, so add it:
      costs_with_candidate[0] += output_weight_decay * sumabs(candidate_unit_output_weights);
      real candidate_class_error = (cost_type!=0)?costs_with_candidate[3]:0;

      learning_rate = initial_learning_rate / ( 1 + n_examples_seen * decay_factor );
      
      // TRAINING OF THE NETWORK
      // backprop & update regular network parameters // TRAINING OF THE NETWORK
      if (!boosting) // i.e. continue training the existing hidden units
      {
        // ** compute gradient on linear output
        output_loss_gradient(output, target, output_gradient, sampleweight);

        // ** bprop through the network & update

        // bprop on output layer
        multiplyAcc(output_biases, output_gradient, -learning_rate);
        if (stage>0)
        {
          // hidden_gradient[j] = sum_i output_weights[j,i]*output_gradient[i]
          // output_weights[i,j] -= learning_rate * (output_gradient[i] * h[j] + output_weight_decay * sign(output_weights[i,j]))
          transposedLayerL1BpropUpdate(hidden_gradient, output_weights, h, output_gradient, learning_rate, output_weight_decay);
          
          if ( residual_correlation_gradient ) {
            if ( n_outputs > 1 ){
              for ( int i = 0; i < stage; i++ ) { // calculate output_weight_gradients
                int max_gradient_index = 0;
                real max_gradient_value = -1.0;
                bool initial = ( h[i] == 0.0 ); // maybe not needed
                for ( int j = 0; j < n_outputs; j++ ) {
                  output_weight_gradients[i][j] = output_gradient[j] * h[i] 
                      * moving_average_coefficient +(1-moving_average_coefficient)*output_weight_gradients[i][j];
                  real gradient_abs = fabs( initial ? output_gradient[j] : output_weight_gradients[i][j] );
                  if ( gradient_abs > max_gradient_value ){ 
                    max_gradient_value = gradient_abs;
                    max_gradient_index = j;
                  }
                }
                hidden_gradient[i] = output_gradient[max_gradient_index] * sign( output_weights[i][max_gradient_index] );
              }             
            } else hidden_gradient.fill(output_gradient[0]);           
          }
          
          if ( !enable_internal_weights ){  // simple one-hidden-layer topology 
            // bprop through hidden units activation
            if (hard_activation_function) 
              // Should h_i(x) change of sign?
              // Consider the loss that would occur if it did, i.e. with output replaced by output - 2*W[.,i]*h_i(x)
              // Then consider a weighted classification problem
              // with the appropriate sign and weight = gradient on h_i(x).
            {
              for (int i=0;i<int(stage);i++) // loop over hidden units
              {
                Vec Wi = output_weights(i);
                multiplyAdd(output,Wi,-2*h[i],output_with_signchange);
                real fit_error_with_sign_change = output_loss(output_with_signchange,target);
                int target_i = int(sign(fit_error_with_sign_change-current_fit_error)*h[i]);
                real weight_i = fabs(hidden_gradient[i]); // CHECK: when is the sign of hidden_gradient different from (h[i]-target_i)?
                if (use_hinge_loss_for_hard_activation)
                  hidden_gradient[i] = weight_i * d_hinge_loss(act[i],target_i);
                else // use cross-entropy
                  hidden_gradient[i] = weight_i * (sigmoid(act[i]) - 2*(target_i+1));
              }
            }
            else
              bprop_tanh(h,hidden_gradient,hidden_gradient);  //  hidden_gradient *= ( 1 - h^2 )
          } else { // cascade topology
            if ( !incremental_connections ){
            //if (hard_activation_function) { /*not implemented*/ } else
              for ( int i = stage-1; i >= 0; i-- ) { // bprop_tanh equivalent, also modifies internal_weights
                hidden_gradient[i] *= (1 - h[i]*h[i]);
                for ( int j = 0; j < i; j++ ) {
                  if ( !residual_correlation_gradient ) // back-propagate gradients through internal weights
                    hidden_gradient[j] += internal_weights[i][j] * hidden_gradient[i]; 
                  internal_weights[i][j] -= learning_rate * ( hidden_gradient[i] * h[j] );
                                              //+ output_weight_decay * sign(internal_weights[i][j]) );
                }
              }            
            } else { // incremental_connections
              for ( int i = stage-1; i >= 0; i-- ) { 
                hidden_gradient[i] *= (1 - h[i]*h[i]);
                for ( int j = 0; j < i; j++ ) {
                  if ( internal_weights[i][j] == 0.0 ){
                    internal_weight_gradients[i][j] = hidden_gradient[i] * h[j] 
                      * moving_average_coefficient +(1-moving_average_coefficient)*internal_weight_gradients[i][j];
                    if ( fabs(internal_weight_gradients[i][j]) > connection_gradient_threshold ){ // add connetion
                      internal_weights[i][j] = - 0.1 * /*sign*/(internal_weight_gradients[i][j]);
                    }      
                  } else {
                    if ( !residual_correlation_gradient )
                      hidden_gradient[j] += internal_weights[i][j] * hidden_gradient[i];
                    internal_weights[i][j] -= learning_rate * ( hidden_gradient[i] * h[j] );
                                                //+ output_weight_decay * sign(internal_weights[i][j]) );
                  }
                }
              } 
            }           
          }
          
          //hidden_gradient *= -learning_rate;
          hidden_layer_biases -= hidden_gradient * learning_rate;
          if ( !incremental_connections ) {
            // bprop through hidden layer and update hidden_weights
            externalProductAcc(hidden_layer_weights, hidden_gradient * (-learning_rate), input);
          } else { // incremental_connections
            for ( int i = 0; i < stage; i++ ){
              update_incremental_connections( hidden_layer_weights(i), hidden_layer_weight_gradients(i), input, 
                hidden_gradient[i] );
/*              for ( int j = 0; j < inputsize_; j++ ){
                if ( hidden_layer_weights[i][j] == 0.0 ){
                  hidden_layer_weight_gradients[i][j] = hidden_gradient[i] * input[j] 
                    * moving_average_coefficient + (1-moving_average_coefficient)*hidden_layer_weight_gradients[i][j];
                  if ( fabs(hidden_layer_weight_gradients[i][j]) > connection_gradient_threshold ){ // add connetion
                    hidden_layer_weights[i][j] = - 0.1 * (hidden_layer_weight_gradients[i][j]);
                  }      
                } else {
                  hidden_layer_weights[i][j] -= hidden_gradient[i] * input[j] * learning_rate;
                }
              }*/
            }
          }
        }
      }

            //MNT
      if ( verbosity > 3 ) {
        cout << "STAGE: " << stage << endl
             << "input: " << input << endl 
             << "output: " << output << endl  
             << "target: " << target << endl                          
             << "train_costs: " << train_costs << endl
             << "output_gradient: " << output_gradient << endl
             << "candidate_h: " << candidate_h << endl
             << "current_average_cost: " << current_average_cost << endl
        ;   
        if ( stage > 0 ) {
          cout << "hidden_layer_weights: " << hidden_layer_weights //<< endl             
               << "hidden_layer_biases: " << hidden_layer_biases << endl
               ;  
        }
        if ( verbosity > 4 ) {     
          cout << "  output_with_candidate: " << output_with_candidate << endl;
          cout << "  target: " << target << endl;
          cout << "  candidate_unit_output_weights_mat(before): " << candidate_unit_output_weights_mat;
          cout << "  candidate_unit_weights (before): " << candidate_unit_weights << endl;
          cout << "  candidate_unit_bias (before): " << candidate_unit_bias << endl;
        }
      }
    
      // TRAINING OF THE CANDIDATE UNIT
      // backprop & update candidate hidden unit
      output_loss_gradient(output_with_candidate, target, output_gradient_with_candidate, sampleweight);     
      // computes candidate_hidden_gradient, and updates candidate_unit_output_weights_mat
      layerBpropUpdate(candidate_hidden_gradient, candidate_unit_output_weights_mat, 
                       candidate_h_vec, output_gradient_with_candidate, learning_rate);
      
      if ( residual_correlation_gradient ) {
        if ( n_outputs > 1 ){ // calculate candidate_unit_output_weight_gradients
            int max_gradient_index = 0;
            real max_gradient_value = -1.0;
            bool initial = ( candidate_h == 0.0 );
            for ( int j = 0; j < n_outputs; j++ ) {
              candidate_unit_output_weight_gradients[j] = output_gradient_with_candidate[j] * candidate_h 
                  * moving_average_coefficient +(1-moving_average_coefficient)*candidate_unit_output_weight_gradients[j];
              real gradient_abs = 
                  fabs( initial ? output_gradient_with_candidate[j] : candidate_unit_output_weight_gradients[j] );
              if ( gradient_abs > max_gradient_value ){ 
                max_gradient_value = gradient_abs;
                max_gradient_index = j;
              }
            }
            candidate_hidden_gradient[0] = output_gradient_with_candidate[max_gradient_index] 
              * sign( candidate_unit_output_weights[max_gradient_index] );              
        } else candidate_hidden_gradient.fill(output_gradient_with_candidate[0]);
      }
      
      // bprop through candidate hidden unit activation, heuristic method
      if (hard_activation_function)
      {
        multiplyAdd(output_with_candidate,candidate_unit_output_weights,-2*candidate_h,output_with_signchange);
        real fit_error_with_sign_change = output_loss(output_with_signchange,target);
        int hidden_class = int(sign(fit_error_with_sign_change-current_fit_error)*candidate_h);
        real weight_on_loss = fabs(candidate_hidden_gradient[0]); // CHECK: when is the sign of hidden_gradient different from (h[i]-target_i)?
        if (use_hinge_loss_for_hard_activation)
          candidate_hidden_gradient[0] = weight_on_loss * d_hinge_loss(candidate_act,hidden_class);
        else // use cross-entropy
          candidate_hidden_gradient[0] = weight_on_loss * (sigmoid(candidate_act) - 2*(hidden_class+1));
      } else {
        bprop_tanh(candidate_h_vec,candidate_hidden_gradient,candidate_hidden_gradient);        
      }

      //candidate_hidden_gradient *= -learning_rate;
      candidate_unit_bias -= candidate_hidden_gradient[0] * learning_rate;
            
      if ( incremental_connections ) {
        update_incremental_connections( candidate_unit_weights, candidate_unit_weight_gradients, input,
          candidate_hidden_gradient[0]);
        
        if ( enable_internal_weights && stage > 0 ) { // consider weights from older hidden units
          update_incremental_connections( candidate_unit_internal_weights, candidate_unit_weight_gradients, h,
            candidate_hidden_gradient[0]);
        }                
      } else {  // train all connections at once    
        multiplyAcc( candidate_unit_weights, input, candidate_hidden_gradient[0] * (-learning_rate) );
        if ( enable_internal_weights && stage > 0 ) // consider weights from older hidden units
          multiplyAcc( candidate_unit_internal_weights, h, candidate_hidden_gradient[0] * (-learning_rate) );
      }

      //MNT
      if ( verbosity > 4 ) {
        cout << "  candidate_hidden_gradient: " << candidate_hidden_gradient << endl;
        cout << "  candidate_unit_output_weights_mat(after): " << candidate_unit_output_weights_mat;
        cout << "  candidate_unit_weights (after): " << candidate_unit_weights << endl;
        cout << "  candidate_unit_bias (after): " << candidate_unit_bias << endl;
      }
           
      // keep track of average performance with and without candidate hidden unit
      n_examples_seen++;
      int n_batches_seen = n_examples_seen / minibatchsize;
      int t_since_beginning_of_batch = n_examples_seen - n_batches_seen*minibatchsize;
      if (!online)
        moving_average_coefficient = 1.0/(1+t_since_beginning_of_batch);
      if (n_examples_seen==1)
        current_average_cost = current_total_cost;
      else  
        current_average_cost = moving_average_coefficient*current_total_cost
          +(1-moving_average_coefficient)*current_average_cost;
      next_average_cost = moving_average_coefficient*costs_with_candidate[0]
        +(1-moving_average_coefficient)*next_average_cost;
      if (verbosity>1 && cost_type!=0)
      {
        current_average_class_error = moving_average_coefficient*current_class_error
        +(1-moving_average_coefficient)*current_average_class_error;
        next_average_class_error = moving_average_coefficient*candidate_class_error
        +(1-moving_average_coefficient)*next_average_class_error;
      }
      
      
      // consider inserting the candidate hidden unit (at every minibatchsize examples)
      if (t_since_beginning_of_batch == 0) 
      {
        n_examples_training_candidate += minibatchsize;
        if (verbosity>1) 
        {
          cout << "At t=" << real(n_examples_seen)/train_set->length() 
               << " epochs, estimated current average cost = " << current_average_cost << endl
               << "Estimated average cost with candidate unit = " << next_average_cost << endl;
          if (verbosity>2)
            cout << "(current cost = " << current_total_cost << "; and with candidate = " 
            << costs_with_candidate[0] << ")" << endl;
          if (cost_type!=0)
            cout << "Estimated current classification error = " << current_average_class_error << endl
                 << "Estimated classification error with candidate unit = " << next_average_class_error << endl;
        }
        
        if ( next_average_cost < current_average_cost ) 
        //if ( next_average_class_error < current_average_class_error ) MNT
        {
          // insert candidate hidden unit
          stage++;
          output_weights.resize(stage,n_outputs);
          hidden_layer_weights.resize(stage,inputsize());
          hidden_layer_biases.resize(stage);
          hidden_gradient.resize(stage);
          output_weights(stage-1) << candidate_unit_output_weights;
          hidden_layer_weights(stage-1) << candidate_unit_weights;
          hidden_layer_biases[stage-1] = candidate_unit_bias;
          if ( incremental_connections ){
            hidden_layer_weight_gradients.resize(stage,inputsize());  
            hidden_layer_weight_gradients(stage-1) << candidate_unit_weight_gradients;
          }
          if ( residual_correlation_gradient && n_outputs > 1 ) {
            output_weight_gradients.resize(stage,n_outputs);
            output_weight_gradients(stage-1) << candidate_unit_output_weight_gradients;
            candidate_unit_output_weight_gradients.fill(0.0);
          }
          if ( enable_internal_weights ) {
            internal_weights.resize(stage);
            internal_weights[stage-1].resize(stage-1);
            internal_weights[stage-1] << candidate_unit_internal_weights;
            //if ( stage > 1 ) 
            //cout << "internal_weights.size(): " << internal_weights.size() << endl;
            candidate_unit_internal_weights.resize(stage);
            if  ( incremental_connections ){
              internal_weight_gradients.resize(stage);
              internal_weight_gradients[stage-1].resize(stage-1);
              internal_weight_gradients[stage-1] << candidate_unit_internal_weight_gradients;
              candidate_unit_internal_weight_gradients.resize(stage);
              candidate_unit_internal_weights.fill(.0);
              candidate_unit_internal_weight_gradients.fill(.0);
            } else candidate_unit_internal_weights.fill(0.01/stage);
          }
          act.resize(stage);
          h.resize(stage);
          // initialize a new candidate
          candidate_unit_output_weights.fill(0.01/stage);
          //candidate_unit_weights.clear();
          //MNT
          if (!incremental_connections) {
            for( int i=0; i < candidate_unit_weights.length(); i++ )
              candidate_unit_weights[i] = ((real)rand()/RAND_MAX - 0.5)*rand_range; 
          } else candidate_unit_weights.fill(.0);
          candidate_unit_bias = ((real)rand()/RAND_MAX - 0.5)*rand_range;
          
          if (verbosity>1)
            cout << "Adding hidden unit number " << stage << " after training it for "
                 << n_examples_training_candidate << " examples.\n The average cost is "
                 << "expected to decrease from " << current_average_cost << " to " 
                 << next_average_cost << "." << endl;
          n_examples_training_candidate=0;
        } else // should we stop?
        {
          if (n_examples_training_candidate >= max_n_epochs_to_fail*train_set->length())
          {
            stopping_criterion_not_met = false; // STOP
            if (verbosity>0)
              cout << "Stopping at " << stage << " units, after seeing " << n_examples_seen 
                   << " examples in " << n_examples_seen/train_set->length() << " epochs." << endl
                   << "The next candidate unit yields an apparent average cost of " 
                   << next_average_cost << " instead of the current one of " << current_average_cost << endl;
          }
        }
        if (!online)
          current_average_cost = 0;
      }
    }
    while (stage<nstages && stopping_criterion_not_met);
          
    //++stage;
    train_stats->finalize(); // finalize statistics for this epoch
  }
}


void IncrementalNNet::computeOutput(const Vec& input, Vec& output) const
{
  // Compute the output from the input.
  int nout = outputsize();
  output.resize(nout);
  if (stage>0)
  {
    product( act, hidden_layer_weights, input );
    act += hidden_layer_biases;

    if ( enable_internal_weights ) { // cascade topology     
      for( int i = 0; i < stage; i++ ) {
        h[i] = hard_activation_function ? sign( act[i] ) : tanh( act[i] );
        for( int j = i+1; j < stage; j++ ) {
          act[j] += h[i] * internal_weights[j][i];
        }
      }
    } else {                         // simple one-hidden-layer topology
      if (hard_activation_function) 
        compute_sign(act,h);
      else 
        compute_tanh(act,h);
    } 
    transposeProduct(linear_output,output_weights,h);      
  } 
  else  linear_output.clear();
  linear_output+=output_biases;
  if (cost_type==2) // "discrete_log_likelihood"
    softmax(linear_output,output);
  else
    output << linear_output;
}    

real IncrementalNNet::output_loss(const Vec& output,const Vec& target) const
{
  real fit_error=0;
  if (cost_type == 0) // "squared_error"
    fit_error = powdistance(output,target);
  else {
    int target_class = int(target[0]);
    if (cost_type == 1) // "hinge_loss", one against all binary classifiers
      fit_error = one_against_all_hinge_loss(output,target_class);
    else // (output_cost_type == "discrete_log_likelihood")
      fit_error = - safelog(output[target_class]);
  }
  return fit_error;
}

void IncrementalNNet::output_loss_gradient(const Vec& output,const Vec& target,
                                           Vec output_gradient, real sampleweight) const
{
  if (cost_type==0) // "squared_error"
  {
    substract(output,target,output_gradient);
    output_gradient *= sampleweight * 2;
    return;
  }
  int target_class = int(target[0]);
  if (cost_type==1) // "hinge_loss"
  {
    one_against_all_hinge_loss_bprop(output,target_class,
                                     output_gradient);
    if (sampleweight!=1)
      output_gradient *= sampleweight;
  }
  else // (output_cost_type=="discrete_log_likelihood")
  {
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
  if (cost_type!=0) // classification type
  {
    int topscoring_class = argmax(output);
    int target_class = int(target[0]);
    costs[3] = (target_class!=topscoring_class); // 1 or 0
  }
}                                

TVec<string> IncrementalNNet::getTestCostNames() const
{
  // Return the names of the costs computed by computeCostsFromOutpus
  // (these may or may not be exactly the same as what's returned by getTrainCostNames).
  if (output_cost_type=="squared_error") // regression-type
  {
    TVec<string> names(3);
    names[0]=output_cost_type+"+L1_regularization";
    names[1]=output_cost_type;
    names[2]="+L1_regularization";
    return names;
  }
  // else classification-type
  TVec<string> names(4);
  names[0]=output_cost_type+"+L1_regularization";
  names[1]=output_cost_type;
  names[2]="+L1_regularization";
  names[3]="class_error";
  return names;
}

TVec<string> IncrementalNNet::getTrainCostNames() const
{
  // Return the names of the objective costs that the train method computes and 
  // for which it updates the VecStatsCollector train_stats
  // (these may or may not be exactly the same as what's returned by getTestCostNames).
  return getTestCostNames();
}

void IncrementalNNet::update_incremental_connections( Vec weights, Vec MAgradients, const Vec& input, real gradient ) const{
  int n = input.size();
  for ( int i = 0; i < n; i++ ) {
    if ( weights[i] == 0.0 ) {
      MAgradients[i] = gradient * input[i]
        * moving_average_coefficient +(1-moving_average_coefficient)*MAgradients[i];
      if ( fabs(MAgradients[i]) > connection_gradient_threshold ){ // add connetion
        weights[i] = - 0.1 * /*sign*/(MAgradients[i]);
      }
    } else weights[i] -= gradient * input[i] * learning_rate;          
  }
}

} // end of namespace PLearn