// -*- C++ -*-

// CascadeCorrelation.cc
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio and University of Montreal
// Copyright (c) 2002 Jean-Sebastien Senecal, Xavier Saint-Mleux, Rejean Ducharme
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
   * $Id: CascadeCorrelation.cc,v 1.1 2004/11/12 20:10:53 larocheh Exp $
   ******************************************************* */

/*! \file PLearnLibrary/PLearnAlgo/CascadeCorrelation.h */
#include <plearn/display/DisplayUtils.h>
#include <plearn/var/AffineTransformVariable.h>
#include <plearn/var/AffineTransformWeightPenalty.h>
#include <plearn/var/BinaryClassificationLossVariable.h>
#include <plearn/var/ClassificationLossVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/var/ConcatRowsVariable.h>
#include <plearn/var/CrossEntropyVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/var/LiftOutputVariable.h>
#include <plearn/var/LogSoftmaxVariable.h>
#include <plearn/var/MarginPerceptronCostVariable.h>
#include <plearn/var/MulticlassLossVariable.h>
#include <plearn/var/NegCrossEntropySigmoidVariable.h>
#include <plearn/var/OneHotSquaredLoss.h>
#include <plearn/var/IdentityVariable.h>
#include <plearn/var/CCCostVariable.h>
// #include "RBFLayerVariable.h" //TODO Put it back when the file exists.
#include <plearn/var/SigmoidVariable.h>
#include <plearn/var/SoftmaxVariable.h>
#include <plearn/var/SoftplusVariable.h>
#include <plearn/var/SumVariable.h>
#include <plearn/var/SumAbsVariable.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/SumSquareVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/TransposeProductVariable.h>
#include <plearn/var/UnaryHardSlopeVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/var/Var_utils.h>

#include <plearn/vmat/ConcatColumnsVMatrix.h>
//#include "DisplayUtils.h"
//#include "GradientOptimizer.h"
#include "CascadeCorrelation.h"
#include <plearn/math/random.h>
#include <plearn/vmat/SubVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(CascadeCorrelation, "Cascade Correlation learning algorithm", 
                        "Cascade Correlation with many bells and whistles...");

CascadeCorrelation::CascadeCorrelation() // DEFAULT VALUES FOR ALL OPTIONS
  :
   n_opt_iterations(0),
   noutputs(0),
   weight_decay(0),
   bias_decay(0),
   hidden_nodes_weight_decay(0),
   hidden_nodes_bias_decay(0),
   output_layer_weight_decay(0),
   output_layer_bias_decay(0),
   classification_regularizer(0),
   margin(1),
   early_stop_threshold(0.00001),
   correlation_early_stop_threshold(0.001),
   use_correlation_optimization(true),
   L1_penalty_in_to_hid(false),
   L1_penalty_hid_to_out(false),
   output_transfer_func(""),
   hidden_transfer_func("tanh"),
   interval_minval(0), interval_maxval(1),
   batch_size(1),
   initialization_method("uniform_linear")
{}

CascadeCorrelation::~CascadeCorrelation()
{
}

void CascadeCorrelation::declareOptions(OptionList& ol)
{
  declareOption(ol, "n_opt_iterations", &CascadeCorrelation::n_opt_iterations, OptionBase::buildoption, 
                "    number of iterations for the optimization algorithm of the network's weights)\n");

  declareOption(ol, "noutputs", &CascadeCorrelation::noutputs, OptionBase::buildoption, 
                "    number of output units. This gives this learner its outputsize.\n"
                "    It is typically of the same dimensionality as the target for regression problems \n"
                "    But for classification problems where target is just the class number, noutputs is \n"
                "    usually of dimensionality number of classes (as we want to output a score or probability \n"
                "    vector, one per class)");

  declareOption(ol, "weight_decay", &CascadeCorrelation::weight_decay, OptionBase::buildoption, 
                "    global weight decay for all layers\n");

  declareOption(ol, "bias_decay", &CascadeCorrelation::bias_decay, OptionBase::buildoption, 
                "    global bias decay for all layers\n");

  declareOption(ol, "hidden_nodes_weight_decay", &CascadeCorrelation::hidden_nodes_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the hidden layer.  Is added to weight_decay.\n");

  declareOption(ol, "hidden_nodes_bias_decay", &CascadeCorrelation::hidden_nodes_bias_decay, OptionBase::buildoption, 
                "    Additional bias decay for the hidden layer.  Is added to bias_decay.\n");

  declareOption(ol, "output_layer_weight_decay", &CascadeCorrelation::output_layer_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the output layer.  Is added to 'weight_decay'.\n");

  declareOption(ol, "output_layer_bias_decay", &CascadeCorrelation::output_layer_bias_decay, OptionBase::buildoption, 
                "    Additional bias decay for the output layer.  Is added to 'bias_decay'.\n");

  declareOption(ol, "L1_penalty_in_to_hid", &CascadeCorrelation::L1_penalty_in_to_hid, OptionBase::buildoption, 
                "    should we use L1 penalty instead of the default L2 penalty on the weights between input and hidden nodes?\n");

  declareOption(ol, "L1_penalty_hid_to_out", &CascadeCorrelation::L1_penalty_hid_to_out, OptionBase::buildoption, 
                "    should we use L1 penalty instead of the default L2 penalty on the weights between hidden and output nodes?\n");

  declareOption(ol, "output_transfer_func", &CascadeCorrelation::output_transfer_func, OptionBase::buildoption, 
                "    what transfer function to use for ouput layer? \n"
                "    one of: tanh, sigmoid, exp, softplus, softmax, log_softmax \n"
                "    or interval(<minval>,<maxval>), which stands for\n"
                "    <minval>+(<maxval>-<minval>)*sigmoid(.).\n"
                "    An empty string or \"none\" means no output transfer function \n");

  declareOption(ol, "hidden_transfer_func", &CascadeCorrelation::hidden_transfer_func, OptionBase::buildoption, 
                "    what transfer function to use for hidden units? \n"
                "    one of: linear, tanh, sigmoid, exp, softplus, softmax, log_softmax, hard_slope or symm_hard_slope\n");

  declareOption(ol, "cost_funcs", &CascadeCorrelation::cost_funcs, OptionBase::buildoption, 
                "    a list of cost functions to use\n"
                "    in the form \"[ cf1; cf2; cf3; ... ]\" where each function is one of: \n"
                "      mse (for regression)\n"
                "      mse_onehot (for classification)\n"
                "      NLL (negative log likelihood -log(p[c]) for classification) \n"
                "      class_error (classification error) \n"
                "      binary_class_error (classification error for a 0-1 binary classifier)\n"
                "      multiclass_error\n"
                "      cross_entropy (for binary classification)\n"
                "      stable_cross_entropy (more accurate backprop and possible regularization, for binary classification)\n"
                "      margin_perceptron_cost (a hard version of the cross_entropy, uses the 'margin' option)\n"
                "      lift_output (not a real cost function, just the output for lift computation)\n"
                "    The first function of the list will be used as \n"
                "    the objective function to optimize \n"
                "    (possibly with an added weight decay penalty) \n");
  
  declareOption(ol, "classification_regularizer", &CascadeCorrelation::classification_regularizer, OptionBase::buildoption, 
                "    used only in the stable_cross_entropy cost function, to fight overfitting (0<=r<1)\n");

  declareOption(ol, "margin", &CascadeCorrelation::margin, OptionBase::buildoption, 
                "    margin requirement, used only with the margin_perceptron_cost cost function.\n"
                "    It should be positive, and larger values regularize more.\n");
  declareOption(ol, "early_stop_threshold", &CascadeCorrelation::early_stop_threshold, OptionBase::buildoption, 
                "    value of the relative difference of the optimized cost under which.\n"
                "    optimization will be stopped.\n");

  declareOption(ol, "correlation_early_stop_threshold", &CascadeCorrelation::correlation_early_stop_threshold, OptionBase::buildoption, 
                "    value of the relative difference of the correlation cost under which.\n"
                "    optimization will be stopped.\n");

  declareOption(ol, "use_correlation_optimization", &CascadeCorrelation::use_correlation_optimization, OptionBase::buildoption, 
                "    indication that the hidden nodes weights are optimized using the correlation cost.\n"
                "    If not, the gradient from the global cost is used.\n");

  declareOption(ol, "optimizer", &CascadeCorrelation::optimizer, OptionBase::buildoption, 
                "    specify the optimizer to use\n");

  declareOption(ol, "batch_size", &CascadeCorrelation::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the avergage gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->length() \n");

  declareOption(ol, "initialization_method", &CascadeCorrelation::initialization_method, OptionBase::buildoption, 
                "    The method used to initialize the weights:\n"
                "     - normal_linear  = a normal law with variance 1/n_inputs\n"
                "     - normal_sqrt    = a normal law with variance 1/sqrt(n_inputs)\n"
                "     - uniform_linear = a uniform law in [-1/n_inputs, 1/n_inputs]\n"
                "     - uniform_sqrt   = a uniform law in [-1/sqrt(n_inputs), 1/sqrt(n_inputs)]\n"
                "     - zero           = all weights are set to 0\n");

  declareOption(ol, "paramsvalues", &CascadeCorrelation::paramsvalues, OptionBase::learntoption, 
                "    The learned parameter vector\n");

  inherited::declareOptions(ol);

}

///////////
// build //
///////////
void CascadeCorrelation::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void CascadeCorrelation::build_()
{
  /*
   * Create Topology Var Graph
   */

  // Don't do anything if we don't have a train_set
  // It's the only one who knows the inputsize and targetsize anyway...

  if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
  {

    hidden_nodes.resize(nstages);
    w.resize(nstages);
    wout.resize(nstages);

    // Initialize the input.
    input = Var(inputsize(), "input");

    params.resize(0);

    // Build main network graph.
    buildOutputFromInput();

    // Build target and weight variables.
    buildTargetAndWeight();

    // Build costs.
    buildCosts();

    // Shared values hack...
    if((bool)paramsvalues)
    {
      int fake_stage=0;
      while(paramsvalues.size() > params.nelems())
      {
        addCandidateNode(fake_stage);
        updateOutputFromInput(fake_stage);
        fake_stage++;
      }
      if(paramsvalues.size() != params.nelems())
        PLERROR("In CascadeCorrelation::build_(): params");
      params << paramsvalues;
      updateCosts(fake_stage);
      updateFuncs();
    }
    else
    {
      paramsvalues.resize(params.nelems());
      initializeParams();
      if(optimizer)
        optimizer->reset();
    }
    params.makeSharedValue(paramsvalues);
    

    // Build functions.
    buildFuncs();

  }
}

////////////////
// buildCosts //
////////////////
void CascadeCorrelation::buildCosts() {
  int ncosts = cost_funcs.size();  
  if(ncosts<=0)
    PLERROR("In CascadeCorrelation::buildCosts - Empty cost_funcs : must at least specify the cost function to optimize!");
  costs.resize(ncosts);

  for(int k=0; k<ncosts; k++)
  {
    // create costfuncs and apply individual weights if weightpart > 1
    if(cost_funcs[k]=="mse")
      costs[k]= sumsquare(output-target);
    else if(cost_funcs[k]=="mse_onehot")
      costs[k] = onehot_squared_loss(output, target);
    else if(cost_funcs[k]=="NLL") 
    {
      if (output->size() == 1) {
        // Assume sigmoid output here!
        costs[k] = cross_entropy(output, target);
      } else {
        if (output_transfer_func == "log_softmax")
          costs[k] = -output[target];
        else
          costs[k] = neg_log_pi(output, target);
      }
    } 
    else if(cost_funcs[k]=="class_error")
      costs[k] = classification_loss(output, target);
    else if(cost_funcs[k]=="binary_class_error")
      costs[k] = binary_classification_loss(output, target);
    else if(cost_funcs[k]=="multiclass_error")
      costs[k] = multiclass_loss(output, target);
    else if(cost_funcs[k]=="cross_entropy")
      costs[k] = cross_entropy(output, target);
    else if (cost_funcs[k]=="stable_cross_entropy") {
      Var c = stable_cross_entropy(identity, target);
      costs[k] = c;
      if (classification_regularizer) {
        // There is a regularizer to add to the cost function.
        dynamic_cast<NegCrossEntropySigmoidVariable*>((Variable*) c)->
          setRegularizer(classification_regularizer);
      }
    }
    else if (cost_funcs[k]=="margin_perceptron_cost")
      costs[k] = margin_perceptron_cost(output,target,margin);
    else if (cost_funcs[k]=="lift_output")
      costs[k] = lift_output(output, target);
    else  // Assume we got a Variable name and its options
    {
      costs[k]= dynamic_cast<Variable*>(newObject(cost_funcs[k]));
      if(costs[k].isNull())
        PLERROR("In CascadeCorrelation::build_()  unknown cost_func option: %s",cost_funcs[k].c_str());
      costs[k]->setParents(output & target);
      costs[k]->build();
    }

  }

  // Build residual error
  if(use_correlation_optimization)
  {
    residual_error = target - output;
  }

  /*
   * weight and bias decay penalty
   */

  // create penalties
  buildPenalties();
  test_costs = hconcat(costs);

  // Apply penalty to cost.
  // If there is no penalty, we still add costs[0] as the first cost, in
  // order to keep the same number of costs as if there was a penalty.
  if(penalties.size() != 0) {
    if (weightsize_>0)
      // only multiply by sampleweight if there are weights
      training_cost = hconcat(sampleweight*sum(hconcat(costs[0] & penalties))
          & (test_costs*sampleweight));
    else {
      training_cost = hconcat(sum(hconcat(costs[0] & penalties)) & test_costs);
    }
  } 
  else {
    if(weightsize_>0) {
      // only multiply by sampleweight if there are weights
      training_cost = hconcat(costs[0]*sampleweight & test_costs*sampleweight);
    } else {
      training_cost = hconcat(costs[0] & test_costs);
    }
  }

  training_cost->setName("training_cost");
  test_costs->setName("test_costs");
  output->setName("output");
}

void CascadeCorrelation::updateCosts(int i) {

  /*
   * weight decay penalty
   */

  // create penalties
  updatePenalties(i);

  // Apply penalty to cost.
  // If there is no penalty, we still add costs[0] as the first cost, in
  // order to keep the same number of costs as if there was a penalty.
  if(penalties.size() != 0) {
    if (weightsize_>0)
      // only multiply by sampleweight if there are weights
      training_cost = hconcat(sampleweight*sum(hconcat(costs[0] & penalties))
          & (test_costs*sampleweight));
    else {
      training_cost = hconcat(sum(hconcat(costs[0] & penalties)) & test_costs);
    }
  } 
  else {
    if(weightsize_>0) {
      // only multiply by sampleweight if there are weights
      training_cost = hconcat(costs[0]*sampleweight & test_costs*sampleweight);
    } else {
      training_cost = hconcat(costs[0] & test_costs);
    }
  }
}


////////////////
// buildFuncs //
////////////////
void CascadeCorrelation::buildFuncs() {
  invars.resize(0);
  VarArray outvars;
  VarArray testinvars;
  if (input)
  {
    invars.push_back(input);
    testinvars.push_back(input);
  }
  if (output)
    outvars.push_back(output);
  if(target)
  {
    invars.push_back(target);
    testinvars.push_back(target);
    outvars.push_back(target);
  }
  if(sampleweight)
  {
    invars.push_back(sampleweight);
  }
  f = Func(input, output);
  test_costf = Func(testinvars, output&test_costs);
  test_costf->recomputeParents();
  output_and_target_to_cost = Func(outvars, test_costs); 
  output_and_target_to_cost->recomputeParents();
}

void CascadeCorrelation::updateFuncs() {
  buildFuncs();
}

//////////////////////////
// buildOutputFromInput //
//////////////////////////
void CascadeCorrelation::buildOutputFromInput() {

  // input to output
  w_in_to_out = Var(inputsize_+1,targetsize_,"w_in_to_out");
  before_transfer_func = affine_transform(input,w_in_to_out);
 
  identity = iden(before_transfer_func);
  params.append(w_in_to_out);
  output = identity;

  /*
   * output_transfer_func
   */
  size_t p=0;
  if(output_transfer_func!="" && output_transfer_func!="none")
  {
    if(output_transfer_func=="tanh")
      output = tanh(output);
    else if(output_transfer_func=="sigmoid")
      output = sigmoid(output);
    else if(output_transfer_func=="softplus")
      output = softplus(output);
    else if(output_transfer_func=="exp")
      output = exp(output);
    else if(output_transfer_func=="softmax")
      output = softmax(output);
    else if (output_transfer_func == "log_softmax")
      output = log_softmax(output);
    else if ((p=output_transfer_func.find("interval"))!=string::npos)
    {
      size_t q = output_transfer_func.find(",");
      interval_minval = atof(output_transfer_func.substr(p+1,q-(p+1)).c_str());
      size_t r = output_transfer_func.find(")");
      interval_maxval = atof(output_transfer_func.substr(q+1,r-(q+1)).c_str());
      output = interval_minval + (interval_maxval - interval_minval)*sigmoid(output);
    }
    else
      PLERROR("In CascadeCorrelation::build_()  unknown output_transfer_func option: %s",output_transfer_func.c_str());
  }
}

void CascadeCorrelation::updateOutputFromInput(int i) {

  // adding candidate node
  string var_name = "wout["+tostring(i)+"]";
  wout[i] = Var(1,targetsize_,var_name.c_str());
  params.append(wout[i]);
  fillWeights(wout[i],false,inputsize_+i+1);
  hidden_nodes[i] = candidate_node;
  before_transfer_func += new TransposeProductVariable(wout[i],candidate_node);
  dynamic_cast<IdentityVariable*> ((Variable*) identity)->setInput(before_transfer_func);
}


////////////////////
// buildPenalties //
////////////////////
void CascadeCorrelation::buildPenalties() {
  penalties.resize(0);  // prevents penalties from being added twice by consecutive builds
  
  if(((output_layer_weight_decay + weight_decay)!=0 || (output_layer_bias_decay + bias_decay)!=0))
    penalties.append(affine_transform_weight_penalty(w_in_to_out, (output_layer_weight_decay*(inputsize_) + weight_decay), 
          (output_layer_bias_decay + bias_decay), L1_penalty_hid_to_out));
}

void CascadeCorrelation::updatePenalties(int i) {

  penalties.resize(0);  // prevents penalties from being added twice by consecutive builds
  
  if(((output_layer_weight_decay + weight_decay)!=0 || (output_layer_bias_decay + bias_decay)!=0))
    penalties.append(affine_transform_weight_penalty(vconcat( ((VarArray)w_in_to_out) & wout), (output_layer_weight_decay*(inputsize_+i+1) + weight_decay), 
          (output_layer_bias_decay + bias_decay), L1_penalty_hid_to_out));
}

//////////////////////////
// buildTargetAndWeight //
//////////////////////////
void CascadeCorrelation::buildTargetAndWeight() {
  target = Var(targetsize(), "target");
  if(weightsize_>0)
  {
    if (weightsize_!=1)
      PLERROR("In CascadeCorrelation::buildTargetAndWeight - Expected weightsize to be 1 or 0 (or unspecified = -1, meaning 0), got %d",weightsize_);
    sampleweight = Var(1, "weight");
  }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void CascadeCorrelation::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, 
                                   const Vec& targetv, Vec& costsv) const
{
  output_and_target_to_cost->fprop(outputv&targetv, costsv); 
}

///////////////////
// computeOutput //
///////////////////
void CascadeCorrelation::computeOutput(const Vec& inputv, Vec& outputv) const
{
  f->fprop(inputv,outputv);
}

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void CascadeCorrelation::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                 Vec& outputv, Vec& costsv) const
{
  test_costf->fprop(inputv&targetv, outputv&costsv);
}

/////////////////
// fillWeights //
/////////////////
void CascadeCorrelation::fillWeights(const Var& weights, bool clear_first_row, int the_length) {
  if (initialization_method == "zero") {
    weights->value->clear();
    return;
  }
  real delta;
  int is = the_length;
  if(is == -1)
    is = weights.length();
    
  if (clear_first_row)
    is--; // -1 to get the same result as before.
  if (initialization_method.find("linear") != string::npos)
    delta = 1.0 / real(is);
  else
    delta = 1.0 / sqrt(real(is));
  if (initialization_method.find("normal") != string::npos)
    fill_random_normal(weights->value, 0, delta);
  else
    fill_random_uniform(weights->value, -delta, delta);
  if (clear_first_row)
    weights->matValue(0).clear();
}

////////////
// forget //
////////////
void CascadeCorrelation::forget()
{
  if (train_set) initializeParams();
  if(optimizer)
    optimizer->reset();
  stage = 0;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> CascadeCorrelation::getTrainCostNames() const
{
  return (cost_funcs[0]+"+penalty") & cost_funcs;
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> CascadeCorrelation::getTestCostNames() const
{ 
  return cost_funcs;
}

//////////////////////
// addCandidateNode //
//////////////////////
void CascadeCorrelation::addCandidateNode(int i)
{
  string var_name = "w["+tostring(i)+"]";
  w[i] = Var(inputsize_+1+i,var_name.c_str());
  fillWeights(w[i],true);
  params.append(w[i]);
  candidate_node = hiddenLayer(vconcat(((VarArray) input) & hidden_nodes.subVarArray(0,i)),w[i],hidden_transfer_func);
}

/////////////////
// hiddenLayer //
/////////////////
Var CascadeCorrelation::hiddenLayer(const Var& input, const Var& weights, string transfer_func) {
  Var hidden = affine_transform(input, weights); 
  Var result;
  if (transfer_func == "default")
    transfer_func = hidden_transfer_func;
  if(transfer_func=="linear")
    result = hidden;
  else if(transfer_func=="tanh")
    result = tanh(hidden);
  else if(transfer_func=="sigmoid")
    result = sigmoid(hidden);
  else if(transfer_func=="softplus")
    result = softplus(hidden);
  else if(transfer_func=="exp")
    result = exp(hidden);
  else if(transfer_func=="softmax")
    result = softmax(hidden);
  else if (transfer_func == "log_softmax")
    result = log_softmax(hidden);
  else if(transfer_func=="hard_slope")
    result = unary_hard_slope(hidden,0,1);
  else if(transfer_func=="symm_hard_slope")
    result = unary_hard_slope(hidden,-1,1);
  else
    PLERROR("In CascadeCorrelation::hiddenLayer - Unknown value for transfer_func: %s",transfer_func.c_str());
  return result;
}

//////////////////////
// initializeParams //
//////////////////////
void CascadeCorrelation::initializeParams(bool set_seed)
{
  if (set_seed) {
    if (seed_>=0)
      manual_seed(seed_);
    else
      PLearn::seed();
  }

  fillWeights(w_in_to_out, true);
  paramsvalues.resize(w_in_to_out->size());
}

//! To use varDeepCopyField.
extern void varDeepCopyField(Var& field, CopiesMap& copies);

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void CascadeCorrelation::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  varDeepCopyField(input, copies);
  varDeepCopyField(target, copies);
  varDeepCopyField(sampleweight, copies);
  varDeepCopyField(w_in_to_out, copies);
  deepCopyField(hidden_nodes, copies);
  deepCopyField(w, copies);
  deepCopyField(wout, copies);
  varDeepCopyField(wrec, copies);
  varDeepCopyField(candidate_node, copies);
  varDeepCopyField(output, copies);
  varDeepCopyField(identity, copies);
  varDeepCopyField(before_transfer_func, copies);
  varDeepCopyField(predicted_input, copies);
  deepCopyField(costs, copies);
  deepCopyField(penalties, copies);
  varDeepCopyField(training_cost, copies);
  varDeepCopyField(test_costs, copies);
  deepCopyField(invars, copies);
  deepCopyField(params, copies);
  deepCopyField(paramsvalues, copies);
  deepCopyField(f, copies);
  deepCopyField(test_costf, copies);
  deepCopyField(output_and_target_to_cost, copies);
  deepCopyField(optimizer, copies);
  deepCopyField(cost_funcs, copies); // Not sure ...
}

////////////////
// outputsize //
////////////////
int CascadeCorrelation::outputsize() const {
  return noutputs;
}

///////////
// train //
///////////
void CascadeCorrelation::train()
{
  // CascadeCorrelation nstages is number of epochs (whole passages through the training set)
  // while optimizer nstages is number of weight updates.
  // So relationship between the 2 depends whether we are in stochastic, batch or minibatch mode

  if(!train_set)
    PLERROR("In CascadeCorrelation::train, you did not setTrainingSet");
    
  if(!train_stats)
    PLERROR("In CascadeCorrelation::train, you did not setTrainStatsCollector");

  PP<VecStatsCollector> ccstats = new VecStatsCollector();
  ccstats->fieldnames.resize(1);
  ccstats->fieldnames[0] = "CC cost";

  int l = train_set->length();  

  if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
    build();

  // number of samples seen by optimizer before each optimizer update
  int nsamples = batch_size>0 ? batch_size : l;
  Func paramf = Func(invars, training_cost); // parameterized function to optimize
  Var totalcost = meanOf(train_set, paramf, nsamples);
  if(optimizer)
    {
      optimizer->setToOptimize(((VarArray) w_in_to_out), totalcost);  
      optimizer->build();
    }
  else PLERROR("CascadeCorrelation::train can't train without setting an optimizer first!");

  // number of optimizer stages corresponding to one learner stage (one epoch)
  int optstage_per_lstage = l/nsamples;
  int opt_iter=0;
  
  int initial_stage = stage;
  bool early_stop=false;
  real last_cost_value = REAL_MAX;
  VarArray candidate_penalty(0);
  Var cascade_correlation_cost;

  if(verbosity>3)
    cout << "Initial input to output weigths optimization" << endl;
  while(opt_iter < n_opt_iterations)
  {
    optimizer->nstages = optstage_per_lstage;
    train_stats->forget();
    optimizer->early_stop = false;
    optimizer->optimizeN(*train_stats);
    train_stats->finalize();
    if(verbosity>3)
      cout << "Epoch " << opt_iter << " train objective: " << train_stats->getMean() << endl;
    if(train_stats->getMean()[0] == 0 || (last_cost_value - train_stats->getMean()[0])/abs(train_stats->getMean()[0]) < early_stop_threshold)
    {
      cout << "Early stop " << opt_iter << " with train objective: " << train_stats->getMean() << endl;
      last_cost_value = REAL_MAX;
      break;
    }
    else
    {
      last_cost_value = train_stats->getMean()[0];
    }
    opt_iter++;
  }
  optimizer->reset();

  ProgressBar* pb = 0;
  if(report_progress)
    pb = new ProgressBar("Training " + classname() + " from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

  while(stage<nstages && !early_stop)
  {    
    
    // Making new candidate hidden node
    addCandidateNode(stage);

    if(use_correlation_optimization)
    {
      // Optimizing the input to hidden weights of the candidate node
    
      if(((hidden_nodes_weight_decay + weight_decay)!=0 || (hidden_nodes_bias_decay + bias_decay)!=0))
        candidate_penalty.append(affine_transform_weight_penalty(w[stage], (hidden_nodes_weight_decay*(inputsize_+1+stage) + weight_decay), 
                                                                 (hidden_nodes_bias_decay + bias_decay), L1_penalty_in_to_hid));
      Func temp1 = new Function(invars,residual_error);
      Func temp2 = new Function(input,w[stage],candidate_node);
      Var cccost_var = cccost(train_set,temp1,temp2);
      //Var cccost_var = cccost(train_set,new Function(invars,residual_error),new Function(input,w[stage],candidate_node));

      if(candidate_penalty.size() != 0) 
        cascade_correlation_cost = hconcat(sum( hconcat(cccost_var & candidate_penalty)) & cccost_var);
      else 
        cascade_correlation_cost = cccost_var;

      optimizer->setToOptimize(w[stage],cascade_correlation_cost);
      optimizer->build();
      if(verbosity > 3) cout << "Optimizing input to hidden weights" << endl;
      opt_iter=0;
      while(opt_iter<n_opt_iterations)
      {
        optimizer->nstages = 1;
        ccstats->forget();
        optimizer->early_stop = false;
        optimizer->optimizeN(*ccstats);
        ccstats->finalize();
        if(verbosity>3)
          cout << ">> Epoch " << opt_iter << " cascade correlation objective: " << ccstats->getMean() << endl;
        if(((real)last_cost_value - ccstats->getMean()[0])/abs(ccstats->getMean()[0]) < correlation_early_stop_threshold)
        {
          if(verbosity>3)
            cout << "Early stop " << opt_iter << " with cascade correlation objective: " << ccstats->getMean() << endl;
          last_cost_value = REAL_MAX;
          break;
        }
        else
        {
          last_cost_value = ccstats->getMean()[0];
        }
        opt_iter++;
      }
      optimizer->reset();
    }
    /*
    temp1->fproppath.clearGradient();
    temp2->fproppath.clearGradient();
    displayVarGraph(temp1->fproppath, true, 333);
    displayVarGraph(temp2->fproppath, true, 333);
    temp1->fproppath[temp1->fproppath.size()-1]->gradient[0] = -1;
    temp2->fproppath[temp2->fproppath.size()-1]->gradient[0] = -1;
    temp1->fproppath.fbprop();
    temp2->fproppath.fbprop();
    displayVarGraph(temp1->fproppath, true, 333);
    displayVarGraph(temp2->fproppath, true, 333);
    */
    // Reconstructing proppath and optimizer
    if(verbosity > 3) cout << "Optimizing hidden to output weights" << endl;

    updateOutputFromInput(stage);
    updateCosts(stage);
    updateFuncs();

    paramf = Func(invars, training_cost); // Maybe could just rebuild func ???
    totalcost = meanOf(train_set, paramf, nsamples);
    if(!use_correlation_optimization) optimizer->setToOptimize(((VarArray) w_in_to_out) & wout.subVarArray(0,stage+1) & w.subVarArray(0,stage+1), totalcost);  
    else optimizer->setToOptimize(((VarArray) w_in_to_out) & wout.subVarArray(0,stage+1), totalcost);  
    optimizer->build();

    // Optimizing hidden to output weights
    opt_iter=0;
    while(opt_iter<n_opt_iterations)
    {
      optimizer->nstages = optstage_per_lstage;
      train_stats->forget();
      optimizer->early_stop = false;
      optimizer->optimizeN(*train_stats);
      train_stats->finalize();
      if(verbosity>3 || (verbosity>2 && opt_iter == n_opt_iterations-1))
        cout << " Epoch " << opt_iter << " train objective: " << train_stats->getMean() << endl;
      if(train_stats->getMean()[0] == 0 || (last_cost_value - train_stats->getMean()[0])/abs(train_stats->getMean()[0]) < early_stop_threshold)
      {
        if(verbosity>2) cout << "train objective: " << train_stats->getMean() << endl;
        last_cost_value = REAL_MAX;
        break;
      }
      else
      {
        last_cost_value = train_stats->getMean()[0];
      }
      opt_iter++;
    }
    optimizer->reset();

    ++stage;
    if(pb)
      pb->update(stage-initial_stage);
  }
  if(verbosity>1)
    cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

  if(pb)
    delete pb;

  // Updating the paramsvalues size
  int new_params_size = 0;
  for(int i=0; i<params.length();i++)
    new_params_size += params[i]->size();
  paramsvalues.resize(new_params_size);
  params.makeSharedValue(paramsvalues);

  output_and_target_to_cost->recomputeParents();
  test_costf->recomputeParents();
}

} // end of namespace PLearn
