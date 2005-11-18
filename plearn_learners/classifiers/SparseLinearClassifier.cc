// -*- C++ -*-

// SparseLinearClassifier.cc
//
// Copyright (C) 2004 Yoshua Bengio & Hugo Larochelle 
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
   * $Id: SparseLinearClassifier.cc,v 1.10 2005/06/17 16:45:39 larocheh Exp $
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file SparseLinearClassifier.cc */


#include "SparseLinearClassifier.h"
#include <plearn/var/Var_operators.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/Var_utils.h>
#include <plearn/var/SigmoidVariable.h>
#include <plearn/var/SoftplusVariable.h>
#include <plearn/var/SoftmaxVariable.h>
#include <plearn/var/LogSoftmaxVariable.h>
#include <plearn/var/InterValuesVariable.h>
#include <plearn/var/SquareVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/var/DotProductVariable.h>
#include <plearn/var/SumSquareVariable.h>
#include <plearn/var/OneHotSquaredLoss.h>
#include <plearn/var/CrossEntropyVariable.h>
#include <plearn/var/BinaryClassificationLossVariable.h>
#include <plearn/var/ClassificationLossVariable.h>
#include <plearn/var/MulticlassLossVariable.h>
#include <plearn/var/ConfRatedAdaboostCostVariable.h>
#include <plearn/var/GradientAdaboostCostVariable.h>
#include <plearn/var/NegCrossEntropySigmoidVariable.h>
#include <plearn/var/MarginPerceptronCostVariable.h>
#include <plearn/var/LiftOutputVariable.h>
#include <plearn/var/SumVariable.h>
#include <plearn/var/SparseIncrementalAffineTransformVariable.h>
#include <plearn/opt/GradientOptimizer.h>
#include <plearn/var/AffineTransformWeightPenalty.h>

namespace PLearn {
using namespace std;


SparseLinearClassifier::SparseLinearClassifier() 
/* ### Initialize all fields to their default value here */
  :  n(0),  n_weights(0),max_n_weights(10),  add_weight_every_n_epochs(5), add_n_weights(1), batch_size(1), noutputs(0), output_transfer_func("sigmoid"), classification_regularizer(0),weight_decay(0),penalty_type("L2_square"),margin(1), weights_are_in_input(false)
{
}

PLEARN_IMPLEMENT_OBJECT(SparseLinearClassifier, "Sparse Linear Classifier",
                        "NO HELP\n"
                        );


void SparseLinearClassifier::declareOptions(OptionList& ol)
{
  declareOption(ol, "optimizer", &SparseLinearClassifier::optimizer, OptionBase::buildoption,
		"Optimizer that optimizes the cost function.\n"
		);
  
  declareOption(ol, "max_n_weights", &SparseLinearClassifier::max_n_weights, OptionBase::buildoption,
		"Maximum number of weights to add.\n"
		);

  declareOption(ol, "add_weight_every_n_epochs", &SparseLinearClassifier::add_weight_every_n_epochs, OptionBase::buildoption,
		"Number of epochs between each addition of a weight.\n"
		);

  declareOption(ol, "add_n_weights", &SparseLinearClassifier::add_n_weights, OptionBase::buildoption,
		"Number of weights to add at a time.\n"
		);

  declareOption(ol, "batch_size", &SparseLinearClassifier::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the average gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->length() \n");

  declareOption(ol, "noutputs", &SparseLinearClassifier::noutputs, OptionBase::buildoption, 
                "Number of output units. This gives this learner its outputsize.\n"
                "For binary classification problems, the noutputs should be 1.\n"
                "For more classes, where target is just the class number, noutputs is \n"
                "usually of number of classes (as we want to output a score or probability \n"
                "vector, one per class)");

  declareOption(ol, "output_transfer_func", &SparseLinearClassifier::output_transfer_func, OptionBase::buildoption, 
                "what transfer function to use for ouput layer? One of: \n"
                "  - \"tanh\" \n"
                "  - \"sigmoid\" \n"
                "  - \"exp\" \n"
                "  - \"softplus\" \n"
                "  - \"softmax\" \n"
                "  - \"log_softmax\" \n"
                "  - \"interval(<minval>,<maxval>)\", which stands for\n"
                "          <minval>+(<maxval>-<minval>)*sigmoid(.).\n"
                "An empty string or \"none\" means no output transfer function \n");


  declareOption(ol, "cost_funcs", &SparseLinearClassifier::cost_funcs, OptionBase::buildoption, 
                "A list of cost functions to use\n"
                "in the form \"[ cf1; cf2; cf3; ... ]\" where each function is one of: \n"
                //"  - \"mse\" (for regression)\n"
                //"  - \"mse_onehot\" (for classification)\n"
                //"  - \"NLL\" (negative log likelihood -log(p[c]) for classification) \n"
                //"  - \"class_error\" (classification error) \n"
                //"  - \"binary_class_error\" (classification error for a 0-1 binary classifier)\n"
                //"  - \"multiclass_error\" \n"
                //"  - \"cross_entropy\" (for binary classification)\n"
                //"  - \"stable_cross_entropy\" (more accurate backprop and possible regularization, for binary classification)\n"
                //"  - \"margin_perceptron_cost\" (a hard version of the cross_entropy, uses the 'margin' option)\n"
                //"  - \"lift_output\" (not a real cost function, just the output for lift computation)\n"
                "  - \"conf_rated_adaboost_cost\" (for confidence rated Adaboost)\n"
                "  - \"gradient_adaboost_cost\" (also for confidence rated Adaboost)\n"
                "The FIRST function of the list will be used as \n"
                "the objective function to optimize. \n"
                );

  declareOption(ol, "classification_regularizer", &SparseLinearClassifier::classification_regularizer, OptionBase::buildoption, 
                "Used only in the stable_cross_entropy cost function, to fight overfitting (0<=r<1)\n");

  declareOption(ol, "margin", &SparseLinearClassifier::margin, OptionBase::buildoption, 
                "Margin requirement, used only with the margin_perceptron_cost cost function.\n"
                "It should be positive, and larger values regularize more.\n");


  declareOption(ol, "parameters", &SparseLinearClassifier::parameters, OptionBase::learntoption,
		"Parameters of the linear classifier function.\n"
		);

  declareOption(ol, "afft", &SparseLinearClassifier::afft, OptionBase::learntoption,
		"Variable that controls the incremental learning of the weights.\n"
		);

  declareOption(ol, "weights_are_in_input", &SparseLinearClassifier::weights_are_in_input, OptionBase::buildoption,
		"Variable that controls the incremental learning of the weights.\n"
		);

  declareOption(ol, "weight_decay", &SparseLinearClassifier::weight_decay, OptionBase::buildoption, 
                "Global weight decay for all layers. This value is multiplied by the\n",
                "number of parameters to get the actual weight decay\n");

  declareOption(ol, "penalty_type", &SparseLinearClassifier::penalty_type,
                OptionBase::buildoption,
                "Penalty to use on the weights (for weight and bias decay).\n"
                "Can be any of:\n"
                "  - \"L1\": L1 norm,\n"
                "  - \"L1_square\": square of the L1 norm,\n"
                "  - \"L2_square\" (default): square of the L2 norm.\n");


  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void SparseLinearClassifier::build_()
{

  n = PLearner::inputsize_-(weights_are_in_input?noutputs:0);

  if (n>0)
  {
    
    VarArray params;     
    x = Var(n);

    if(max_n_weights > n*noutputs) max_n_weights = n*noutputs;

    if(noutputs < 1) PLERROR("SparseLinearClassifier::build_(): noutputs is < 1");
    W = Var(n+1,noutputs,"V ");               
    params.append(W);
      
    afft_var =  sparse_incremental_affine_transform(x,W);
    afft = dynamic_cast<SparseIncrementalAffineTransformVariable*>((Variable*)afft_var);
    output = afft_var;
    invars.clear();
    invars.resize(0);
        
    // Set input vars of cost function
    if (inputsize_>0)
      invars.push_back(x);
    if(weights_are_in_input)
    {
      sampleweight = Var(noutputs);
      invars.push_back(sampleweight);
    }

    if(targetsize_>0)
    {
      target = Var(targetsize_);
      invars.push_back(target);
    }
    else PLERROR("In SparseLinearClassifier:: build_(): learner should have targetsize_ > 0");
    if(weightsize_ && !weights_are_in_input)
    {
      sampleweight = Var(1);
      invars.push_back(sampleweight);
    }
    

    // Set output transfer function
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
        real interval_minval = atof(output_transfer_func.substr(p+1,q-(p+1)).c_str());
        size_t r = output_transfer_func.find(")");
        real interval_maxval = atof(output_transfer_func.substr(q+1,r-(q+1)).c_str());
        output = interval_minval + (interval_maxval - interval_minval)*sigmoid(output);
      }
      else
        PLERROR("In SparseLinearClassifier::build_()  unknown output_transfer_func option: %s",output_transfer_func.c_str());
    }
    
    // Set cost functions
    int ncosts = cost_funcs.size();  
    if(ncosts<=0)
      PLERROR("In SparseLinearClassifier::buildCosts - Empty cost_funcs : must at least specify the cost function to optimize!");
    costs.resize(ncosts);
    
    for(int k=0; k<ncosts; k++)
    {
      /*
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
      else 
      
      if(cost_funcs[k]=="class_error")
        costs[k] = classification_loss(output, target);
      else 
        
          if(cost_funcs[k]=="binary_class_error")
        costs[k] = binary_classification_loss(output, target);
      else if(cost_funcs[k]=="multiclass_error")
      costs[k] = multiclass_loss(output, target);
      else 
        */
      if(cost_funcs[k]=="cross_entropy")
      {
        if(weights_are_in_input)
          costs[k] = dot(sampleweight,cross_entropy(output, target));
        else
          costs[k] = sum(cross_entropy(output, target));
      }
      else 
      if(cost_funcs[k]=="conf_rated_adaboost_cost")
      {
        if(output_transfer_func != "sigmoid")
          PLWARNING("In SparseLinearClassifier:buildCosts(): conf_rated_adaboost_cost expects an output in (0,1)");
        Var alpha_adaboost = Var(1,1); alpha_adaboost->value[0] = 1.0;
        params.append(alpha_adaboost);
        if(weights_are_in_input)
          costs[k] = dot(sampleweight,conf_rated_adaboost_cost(output, target, square(alpha_adaboost)));
        else
          costs[k] = sum(conf_rated_adaboost_cost(output, target, alpha_adaboost));
      }
      else if(cost_funcs[k]=="gradient_adaboost_cost")
      {
        if(output_transfer_func != "sigmoid")
          PLWARNING("In SparseLinearClassifier:buildCosts(): gradient_adaboost_cost expects an output in (0,1)");
        if(weights_are_in_input)
          costs[k] = dot(sampleweight,gradient_adaboost_cost(output, target));
        else
          costs[k] = sum(gradient_adaboost_cost(output, target));
      }
      else
        PLERROR("In SparseLinearClassifier::build_(): unknown cost %s",cost_funcs[k].c_str());

      /*
      else if (cost_funcs[k]=="stable_cross_entropy") {
        Var c = stable_cross_entropy((Variable *)afft, target);
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
          PLERROR("In SparseLinearClassifier::build_()  unknown cost_func option: %s",cost_funcs[k].c_str());
        costs[k]->setParents(output & target);
        costs[k]->build();
      }
      */
    }
      
    test_costs = hconcat(costs);

    // Constuct penalties
    if(weight_decay != 0)
    {
      penalties.clear();
      penalties.resize(0);
      penalties.append(affine_transform_weight_penalty(W, weight_decay*n*noutputs,0,penalty_type));
    }

    if(penalties.size() != 0)
    {
      if(weightsize_>0 && !weights_are_in_input) {
        // only multiply by sampleweight if there are weights
        training_cost = hconcat(sampleweight*sum(hconcat(costs[0] & penalties)) & test_costs*sampleweight);
      } else {
        training_cost = hconcat(sum(hconcat(costs[0] & penalties)) & test_costs);
      }
    }
    else
    {
      if(weightsize_>0 && !weights_are_in_input) {
        // only multiply by sampleweight if there are weights
        training_cost = hconcat(costs[0]*sampleweight & test_costs*sampleweight);
      } else {
        training_cost = hconcat(costs[0] & test_costs);
      }
    }
      
    // Building functions
    classify = Func(x, params , output );
    if(weights_are_in_input)
      output_and_target_to_cost = Func(sampleweight & output & target,test_costs);
    else
      output_and_target_to_cost = Func(output & target,test_costs);
    initializeParams();
    if (parameters.size()>0 && parameters.nelems() == classify->parameters.nelems())
      classify->parameters.copyValuesFrom(parameters);
    parameters.resize(classify->parameters.size());
    for (int i=0;i<parameters.size();i++)
      parameters[i] = classify->parameters[i];  
  }

}

// ### Nothing to add here, simply calls build_
void SparseLinearClassifier::build()
{
  inherited::build();
  build_();
}

#ifdef __INTEL_COMPILER
#pragma warning(disable:1419)  // Get rid of compiler warning.
#endif
extern void varDeepCopyField(Var& field, CopiesMap& copies);
#ifdef __INTEL_COMPILER
#pragma warning(default:1419)
#endif

void SparseLinearClassifier::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{  inherited::makeDeepCopyFromShallowCopy(copies);

  varDeepCopyField(x, copies);
  varDeepCopyField(output, copies);
  varDeepCopyField(afft_var,copies);
  varDeepCopyField(W, copies);
  varDeepCopyField(test_costs, copies);
  varDeepCopyField(sampleweight, copies);
  varDeepCopyField(training_cost, copies);
  varDeepCopyField(target, copies);

  deepCopyField(classify, copies);
  deepCopyField(costs, copies);
  deepCopyField(cost_funcs, copies);
  deepCopyField(parameters, copies);
  deepCopyField(penalties, copies);
  deepCopyField(optimizer, copies);
  deepCopyField(invars, copies);
  deepCopyField(output_and_target_to_cost, copies);

  afft = dynamic_cast<SparseIncrementalAffineTransformVariable*>((Variable*)afft_var);  
}


void SparseLinearClassifier::forget()
{
  if (train_set) initializeParams();
  stage = 0;
}
    
void SparseLinearClassifier::train()
{
  
  if(!train_set)
    PLERROR("In SparseLinearClassifier::train, you did not setTrainingSet");
    
  if(!train_stats)
    PLERROR("In SparseLinearClassifier::train, you did not setTrainStatsCollector");

  int l = train_set->length();  

  // number of samples seen by optimizer before each optimizer update
  int nsamples = batch_size>0 ? batch_size : l;
  Func paramf = Func(invars, training_cost); // parameterized function to optimize
  Var totalcost = meanOf(train_set, paramf, nsamples);
  if(optimizer)
    {
      optimizer->setToOptimize(parameters, totalcost);  
      optimizer->build();
    }
  else PLERROR("SparseLinearClassifier::train can't train without setting an optimizer first!");

  // number of optimizer stages corresponding to one learner stage (one epoch)
  int optstage_per_lstage = l/nsamples;

  ProgressBar* pb = 0;
  if(report_progress)
    pb = new ProgressBar("Training " + classname() + " from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

  int initial_stage = stage;
  bool early_stop=false;
  while(stage<nstages && !early_stop)
    {
      optimizer->nstages = optstage_per_lstage;
      train_stats->forget();
      optimizer->early_stop = false;
      optimizer->optimizeN(*train_stats);
      // optimizer->verifyGradient(1e-4); // Uncomment if you want to check your new Var.
      train_stats->finalize();
      if(verbosity>2)
        cout << "Epoch " << stage << " train objective: " << train_stats->getMean() << endl;
      ++stage;

      if(pb)
        pb->update(stage-initial_stage);
      if((stage)%add_weight_every_n_epochs == 0 && n_weights < max_n_weights)
      {
        int to_add = (max_n_weights <= n_weights + add_n_weights ? max_n_weights - n_weights : add_n_weights);
        afft->add_n_weights += to_add;
        n_weights += to_add;
      }
    }
  if(verbosity>1)
    cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

  if(pb)
    delete pb;

  //output_and_target_to_cost->recomputeParents();
}

//////////////////////
// initializeParams //
//////////////////////
void SparseLinearClassifier::initializeParams()
{
  W->value.clear();
  afft->reset();
  n_weights =0;
  if(optimizer)
    optimizer->reset();
}

///////////////////
// computeOutput //
///////////////////
void SparseLinearClassifier::computeOutput(const Vec& input, Vec& output) const
{
  int nout = outputsize();
  output.resize(nout);
  output << classify(input.subVec(0,n));

}

////////////////
// outputsize //
////////////////
int SparseLinearClassifier::outputsize() const
{
  return noutputs;
}

void SparseLinearClassifier::computeCostsFromOutputs(const Vec& input, const Vec& output, 
					     const Vec& target, Vec& costs) const
{
  if(weights_are_in_input)
    output_and_target_to_cost->fprop(input.subVec(n,noutputs)&output&target, costs.subVec(0,cost_funcs.length())); 
  else
    output_and_target_to_cost->fprop(output&target, costs.subVec(0,cost_funcs.length())); 

  costs.subVec(cost_funcs.length(),noutputs).fill(0.0);
  if(penalties.length() != 0)
  { 
    for(int i=0; i<noutputs; i++)
    {
      for(int j=0; j<W.length()-1; j++)
      {
        if (penalty_type == "L1_square" || penalty_type == "L1" )
          costs[i+cost_funcs.length()] += abs(W->matValue(j+1,i))*weight_decay*n*noutputs;
        else if (penalty_type == "L2_square")
          costs[i+cost_funcs.length()] += square(W->matValue(j+1,i))*weight_decay*n*noutputs;
      }
      if (penalty_type == "L1_square" )
        costs[i+cost_funcs.length()] *= costs[i+1+cost_funcs.length()];
    }
  }
}                                

TVec<string> SparseLinearClassifier::getTestCostNames() const
{
  TVec<string> test_costs = getTrainCostNames();
  return test_costs.subVec(1,test_costs.length()-1);
}

TVec<string> SparseLinearClassifier::getTrainCostNames() const
{
  assert( !cost_funcs.isEmpty() );
  TVec<string> costnames(1+cost_funcs.length()+noutputs);
  costnames[0] = cost_funcs[0]+"+penalty";
  costnames.subVec(1,cost_funcs.length()) << cost_funcs;
  for(int i=0; i<noutputs; i++)
    costnames[i+1+cost_funcs.length()] = "penalty_cost_target_" + tostring(i);
  return costnames;
}


} // end of namespace PLearn
