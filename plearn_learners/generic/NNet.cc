// -*- C++ -*-

// NNet.cc
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
   * $Id: NNet.cc,v 1.54 2004/05/21 02:18:52 yoshua Exp $
   ******************************************************* */

/*! \file PLearnLibrary/PLearnAlgo/NNet.h */

#include "AffineTransformVariable.h"
#include "AffineTransformWeightPenalty.h"
#include "BinaryClassificationLossVariable.h"
#include "ClassificationLossVariable.h"
#include "ConcatColumnsVariable.h"
#include "CrossEntropyVariable.h"
#include "ExpVariable.h"
#include "LiftOutputVariable.h"
#include "LogSoftmaxVariable.h"
#include "MarginPerceptronCostVariable.h"
#include "MulticlassLossVariable.h"
#include "NegCrossEntropySigmoidVariable.h"
#include "OneHotSquaredLoss.h"
// #include "RBFLayerVariable.h" //TODO Put it back when the file exists.
#include "SigmoidVariable.h"
#include "SoftmaxVariable.h"
#include "SoftplusVariable.h"
#include "SumVariable.h"
#include "SumAbsVariable.h"
#include "SumOfVariable.h"
#include "SumSquareVariable.h"
#include "TanhVariable.h"
#include "TransposeProductVariable.h"
#include "UnaryHardSlopeVariable.h"
#include "Var_operators.h"
#include "Var_utils.h"

#include "ConcatColumnsVMatrix.h"
//#include "DisplayUtils.h"
//#include "GradientOptimizer.h"
#include "NNet.h"
#include "random.h"
#include "SubVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(NNet, "Ordinary Feedforward Neural Network with 1 or 2 hidden layers", 
                        "Neural network with many bells and whistles...");

NNet::NNet() // DEFAULT VALUES FOR ALL OPTIONS
  :
   nhidden(0),
   nhidden2(0),
   noutputs(0),
   weight_decay(0),
   bias_decay(0),
   layer1_weight_decay(0),
   layer1_bias_decay(0),
   layer2_weight_decay(0),
   layer2_bias_decay(0),
   output_layer_weight_decay(0),
   output_layer_bias_decay(0),
   direct_in_to_out_weight_decay(0),
   classification_regularizer(0),
   margin(1),
   fixed_output_weights(0),
   rbf_layer_size(0),
   first_class_is_junk(1),
   L1_penalty(false),
   input_reconstruction_penalty(0),
   direct_in_to_out(false),
   output_transfer_func(""),
   hidden_transfer_func("tanh"),
   interval_minval(0), interval_maxval(1),
   batch_size(1),
   initialization_method("normal_linear")
{}

NNet::~NNet()
{
}

void NNet::declareOptions(OptionList& ol)
{
  declareOption(ol, "nhidden", &NNet::nhidden, OptionBase::buildoption, 
                "    number of hidden units in first hidden layer (0 means no hidden layer)\n");

  declareOption(ol, "nhidden2", &NNet::nhidden2, OptionBase::buildoption, 
                "    number of hidden units in second hidden layer (0 means no hidden layer)\n");

  declareOption(ol, "noutputs", &NNet::noutputs, OptionBase::buildoption, 
                "    number of output units. This gives this learner its outputsize.\n"
                "    It is typically of the same dimensionality as the target for regression problems \n"
                "    But for classification problems where target is just the class number, noutputs is \n"
                "    usually of dimensionality number of classes (as we want to output a score or probability \n"
                "    vector, one per class)");

  declareOption(ol, "weight_decay", &NNet::weight_decay, OptionBase::buildoption, 
                "    global weight decay for all layers\n");

  declareOption(ol, "bias_decay", &NNet::bias_decay, OptionBase::buildoption, 
                "    global bias decay for all layers\n");

  declareOption(ol, "layer1_weight_decay", &NNet::layer1_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the first hidden layer.  Is added to weight_decay.\n");

  declareOption(ol, "layer1_bias_decay", &NNet::layer1_bias_decay, OptionBase::buildoption, 
                "    Additional bias decay for the first hidden layer.  Is added to bias_decay.\n");

  declareOption(ol, "layer2_weight_decay", &NNet::layer2_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the second hidden layer.  Is added to weight_decay.\n");

  declareOption(ol, "layer2_bias_decay", &NNet::layer2_bias_decay, OptionBase::buildoption, 
                "    Additional bias decay for the second hidden layer.  Is added to bias_decay.\n");

  declareOption(ol, "output_layer_weight_decay", &NNet::output_layer_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the output layer.  Is added to 'weight_decay'.\n");

  declareOption(ol, "output_layer_bias_decay", &NNet::output_layer_bias_decay, OptionBase::buildoption, 
                "    Additional bias decay for the output layer.  Is added to 'bias_decay'.\n");

  declareOption(ol, "direct_in_to_out_weight_decay", &NNet::direct_in_to_out_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the direct in-to-out layer.  Is added to 'weight_decay'.\n");

  declareOption(ol, "L1_penalty", &NNet::L1_penalty, OptionBase::buildoption, 
                "    should we use L1 penalty instead of the default L2 penalty on the weights?\n");

  declareOption(ol, "fixed_output_weights", &NNet::fixed_output_weights, OptionBase::buildoption, 
                "    If true then the output weights are not learned. They are initialized to +1 or -1 randomly.\n");

  declareOption(ol, "input_reconstruction_penalty", &NNet::input_reconstruction_penalty, OptionBase::buildoption,
                "    if >0 then a set of weights will be added from a hidden layer to predict (reconstruct) the inputs\n"
                "    and the total loss will include an extra term that is the squared input reconstruction error,\n"
                "    multiplied by the input_reconstruction_penalty factor.\n");

  declareOption(ol, "direct_in_to_out", &NNet::direct_in_to_out, OptionBase::buildoption, 
                "    should we include direct input to output connections?\n");

  declareOption(ol, "rbf_layer_size", &NNet::rbf_layer_size, OptionBase::buildoption,
                "    If non-zero, add an extra layer which computes N(h(x);mu_i,sigma_i) (Gaussian density) for the\n"
                "    i-th output unit with mu_i a free vector and sigma_i a free scalar, and h(x) the vector of\n"
                "    activations of the 'representation' output, i.e. what would be the output layer otherwise. The\n"
                "    given non-zero value is the number of these 'representation' outputs. Typically this\n"
                "    makes sense for classification problems, with a softmax output_transfer_func. If the\n"
                "    first_class_is_junk option is set then the first output (first class) does not get a\n"
                "    Gaussian density but just a 'pseudo-uniform' density (the single free parameter is the\n"
                "    value of that density) and in a softmax it makes sure that when h(x) is far from the\n"
                "    centers mu_i for all the other classes then the last class gets the strongest posterior probability.\n");

  declareOption(ol, "first_class_is_junk", &NNet::first_class_is_junk, OptionBase::buildoption, 
                "    This option is used only when rbf_layer_size>0. If true then the first class is\n"
                "    treated differently and gets a pre-transfer-function value that is a learned constant, whereas\n"
                "    the others get a normal centered at mu_i.\n");

  declareOption(ol, "output_transfer_func", &NNet::output_transfer_func, OptionBase::buildoption, 
                "    what transfer function to use for ouput layer? \n"
                "    one of: tanh, sigmoid, exp, softplus, softmax, log_softmax \n"
                "    or interval(<minval>,<maxval>), which stands for\n"
                "    <minval>+(<maxval>-<minval>)*sigmoid(.).\n"
                "    An empty string or \"none\" means no output transfer function \n");

  declareOption(ol, "hidden_transfer_func", &NNet::hidden_transfer_func, OptionBase::buildoption, 
                "    what transfer function to use for hidden units? \n"
                "    one of: linear, tanh, sigmoid, exp, softplus, softmax, log_softmax, hard_slope or symm_hard_slope\n");

  declareOption(ol, "cost_funcs", &NNet::cost_funcs, OptionBase::buildoption, 
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
  
  declareOption(ol, "classification_regularizer", &NNet::classification_regularizer, OptionBase::buildoption, 
                "    used only in the stable_cross_entropy cost function, to fight overfitting (0<=r<1)\n");

  declareOption(ol, "margin", &NNet::margin, OptionBase::buildoption, 
                "    margin requirement, used only with the margin_perceptron_cost cost function.\n"
                "    It should be positive, and larger values regularize more.\n");

  declareOption(ol, "optimizer", &NNet::optimizer, OptionBase::buildoption, 
                "    specify the optimizer to use\n");

  declareOption(ol, "batch_size", &NNet::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the avergage gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->length() \n");

  declareOption(ol, "initialization_method", &NNet::initialization_method, OptionBase::buildoption, 
                "    The method used to initialize the weights:\n"
                "     - normal_linear = a normal law with variance 1 / n_inputs\n"
                "     - normal_sqrt   = a normal law with variance 1 / sqrt(n_inputs)\n");

  declareOption(ol, "paramsvalues", &NNet::paramsvalues, OptionBase::learntoption, 
                "    The learned parameter vector\n");

  inherited::declareOptions(ol);

}

void NNet::build()
{
  inherited::build();
  build_();
}

void NNet::build_()
{
  /*
   * Create Topology Var Graph
   */

  // Don't do anything if we don't have a train_set
  // It's the only one who knows the inputsize and targetsize anyway...

  if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
    {

      
      // init. basic vars
      input = Var(inputsize(), "input");
      output = input;
      params.resize(0);
      Var hidden_layer;

      // first hidden layer
      if(nhidden>0)
        {
          w1 = Var(1+inputsize(), nhidden, "w1");      
          hidden_layer = affine_transform(output,w1); 
          params.append(w1);
          if(hidden_transfer_func=="linear")
            output = hidden_layer;
          else if(hidden_transfer_func=="tanh")
            output = tanh(hidden_layer);
          else if(hidden_transfer_func=="sigmoid")
            output = sigmoid(hidden_layer);
          else if(hidden_transfer_func=="softplus")
            output = softplus(hidden_layer);
          else if(hidden_transfer_func=="exp")
            output = exp(hidden_layer);
          else if(hidden_transfer_func=="softmax")
            output = softmax(hidden_layer);
          else if (hidden_transfer_func == "log_softmax")
            output = log_softmax(output);
          else if(hidden_transfer_func=="hard_slope")
            output = unary_hard_slope(hidden_layer,0,1);
          else if(hidden_transfer_func=="symm_hard_slope")
            output = unary_hard_slope(hidden_layer,-1,1);
          else
            PLERROR("In NNet::build_()  unknown hidden_transfer_func option: %s",hidden_transfer_func.c_str());
        }

      // second hidden layer
      if(nhidden2>0)
        {
          w2 = Var(1+nhidden, nhidden2, "w2");
          output = affine_transform(output,w2);
          params.append(w2);
          if(hidden_transfer_func=="linear")
            output = output;
          else if(hidden_transfer_func=="tanh")
            output = tanh(output);
          else if(hidden_transfer_func=="sigmoid")
            output = sigmoid(output);
          else if(hidden_transfer_func=="softplus")
            output = softplus(output);
          else if(hidden_transfer_func=="exp")
            output = exp(output);
          else if(hidden_transfer_func=="softmax")
            output = softmax(output);
          else if (hidden_transfer_func == "log_softmax")
            output = log_softmax(output);
          else if(hidden_transfer_func=="hard_slope")
            output = unary_hard_slope(output,0,1);
          else if(hidden_transfer_func=="symm_hard_slope")
            output = unary_hard_slope(output,-1,1);
          else
            PLERROR("In NNet::build_()  unknown hidden_transfer_func option: %s",hidden_transfer_func.c_str());
        }

      if (nhidden2>0 && nhidden==0)
        PLERROR("NNet:: can't have nhidden2 (=%d) > 0 while nhidden=0",nhidden2);

      if (rbf_layer_size>0)
      {
        if (first_class_is_junk)
        {
          rbf_centers = Var(outputsize()-1, rbf_layer_size, "rbf_centers");
          rbf_sigmas = Var(outputsize()-1, "rbf_sigmas");
          PLERROR("In NNet.cc, the code needs to be completed, rbf_layer isn't declared and thus it doesn't compile with the line below");
          // TODO (Also put back the corresponding include).
//          output = hconcat(rbf_layer(output,rbf_centers,rbf_sigmas)&junk_prob);
          params.append(junk_prob);
        }
        else
        {
          rbf_centers = Var(outputsize(), rbf_layer_size, "rbf_centers");
          rbf_sigmas = Var(outputsize(), "rbf_sigmas");
          PLERROR("In NNet.cc, the code needs to be completed, rbf_layer isn't declared and thus it doesn't compile with the line below");
//          output = rbf_layer(output,rbf_centers,rbf_sigmas);
        }
        params.append(rbf_centers);
        params.append(rbf_sigmas);
      }

      // output layer before transfer function
      wout = Var(1+output->size(), outputsize(), "wout");
      output = affine_transform(output,wout);
      if (!fixed_output_weights)
        params.append(wout);
      else
      {
        outbias = Var(output->size(),"outbias");
        output = output + outbias;
        params.append(outbias);
      }

      // direct in-to-out layer
      if(direct_in_to_out)
        {
          wdirect = Var(inputsize(), outputsize(), "wdirect");// Var(1+inputsize(), outputsize(), "wdirect");
          output += transposeProduct(wdirect, input);// affine_transform(input,wdirect);
          params.append(wdirect);
        }

      Var before_transfer_func = output;
      
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
            PLERROR("In NNet::build_()  unknown output_transfer_func option: %s",output_transfer_func.c_str());
        }

      /*
       * target and weights
       */
      
      target = Var(targetsize(), "target");
      
      if(weightsize_>0)
      {
        if (weightsize_!=1)
          PLERROR("NNet: expected weightsize to be 1 or 0 (or unspecified = -1, meaning 0), got %d",weightsize_);
        sampleweight = Var(1, "weight");
      }
      /*
       * costfuncs
       */
      int ncosts = cost_funcs.size();  
      if(ncosts<=0)
        PLERROR("In NNet::build_()  Empty cost_funcs : must at least specify the cost function to optimize!");
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
            Var c = stable_cross_entropy(before_transfer_func, target);
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
                PLERROR("In NNet::build_()  unknown cost_func option: %s",cost_funcs[k].c_str());
              costs[k]->setParents(output & target);
              costs[k]->build();
            }
          
          // take into account the sampleweight
          //if(sampleweight)
          //  costs[k]= costs[k] * sampleweight; // NO, because this is taken into account (more properly) in stats->update
        }
      

      /*
       * weight and bias decay penalty
       */

      // create penalties
      penalties.resize(0);  // prevents penalties from being added twice by consecutive builds
      if(w1 && ((layer1_weight_decay + weight_decay)!=0 || (layer1_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(w1, (layer1_weight_decay + weight_decay), (layer1_bias_decay + bias_decay), L1_penalty));
      if(w2 && ((layer2_weight_decay + weight_decay)!=0 || (layer2_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(w2, (layer2_weight_decay + weight_decay), (layer2_bias_decay + bias_decay), L1_penalty));
      if(wout && ((output_layer_weight_decay + weight_decay)!=0 || (output_layer_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(wout, (output_layer_weight_decay + weight_decay), 
                                                         (output_layer_bias_decay + bias_decay), L1_penalty));
      if(wdirect && (direct_in_to_out_weight_decay + weight_decay) != 0)
      {
        if (L1_penalty)
          penalties.append(sumabs(wdirect)*(direct_in_to_out_weight_decay + weight_decay));
        else
          penalties.append(sumsquare(wdirect)*(direct_in_to_out_weight_decay + weight_decay));
      }

      if (input_reconstruction_penalty>0)
        {
          wrec = Var(hidden_layer->size(),inputsize(),"wrec");
          predicted_input = transposeProduct(wrec, hidden_layer);
          params.append(wrec);
          penalties.append(input_reconstruction_penalty*sumsquare(predicted_input - input));
        }

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
      
      // Shared values hack...
      if((bool)paramsvalues && (paramsvalues.size() == params.nelems()))
        params << paramsvalues;
      else
        {
          paramsvalues.resize(params.nelems());
          initializeParams();
        }
      params.makeSharedValue(paramsvalues);

      // Funcs
      invars.resize(0);
      VarArray outvars;
      VarArray testinvars;
      if(input)
      {
        invars.push_back(input);
        testinvars.push_back(input);
      }
      if(output)
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
}

int NNet::outputsize() const
{ return noutputs; }

TVec<string> NNet::getTrainCostNames() const
{
  return (cost_funcs[0]+"+penalty") & cost_funcs;
}

TVec<string> NNet::getTestCostNames() const
{ 
  return cost_funcs;
}


void NNet::train()
{
  // NNet nstages is number of epochs (whole passages through the training set)
  // while optimizer nstages is number of weight updates.
  // So relationship between the 2 depends whether we are in stochastic, batch or minibatch mode

  if(!train_set)
    PLERROR("In NNet::train, you did not setTrainingSet");
    
  if(!train_stats)
    PLERROR("In NNet::train, you did not setTrainStatsCollector");

  int l = train_set->length();  

  if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
    build();

  // number of samples seen by optimizer before each optimizer update
  int nsamples = batch_size>0 ? batch_size : l;
  Func paramf = Func(invars, training_cost); // parameterized function to optimize
  Var totalcost = meanOf(train_set, paramf, nsamples);
  if(optimizer)
    {
      optimizer->setToOptimize(params, totalcost);  
      optimizer->build();
    }
  else PLERROR("RecommandationNet::train can't train without setting an optimizer first!");

  // number of optimizer stages corresponding to one learner stage (one epoch)
  int optstage_per_lstage = l/nsamples;

  ProgressBar* pb = 0;
  if(report_progress)
    pb = new ProgressBar("Training NNet from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

  int initial_stage = stage;
  bool early_stop=false;
  while(stage<nstages && !early_stop)
    {
      optimizer->nstages = optstage_per_lstage;
      train_stats->forget();
      optimizer->early_stop = false;
      optimizer->optimizeN(*train_stats);
      train_stats->finalize();
      if(verbosity>2)
        cout << "Epoch " << stage << " train objective: " << train_stats->getMean() << endl;
      ++stage;
      if(pb)
        pb->update(stage-initial_stage);
    }
  if(verbosity>1)
    cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

  if(pb)
    delete pb;

  output_and_target_to_cost->recomputeParents();
  test_costf->recomputeParents();
  // cerr << "totalcost->value = " << totalcost->value << endl;
  // cout << "Result for benchmark is: " << totalcost->value << endl;
}



void NNet::computeOutput(const Vec& inputv, Vec& outputv) const
{
  f->fprop(inputv,outputv);
}

void NNet::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                 Vec& outputv, Vec& costsv) const
{
  test_costf->fprop(inputv&targetv, outputv&costsv);
}


void NNet::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, 
                                   const Vec& targetv, Vec& costsv) const
{
  output_and_target_to_cost->fprop(outputv&targetv, costsv); 
}

void NNet::initializeParams()
{
  if (seed_>=0)
    manual_seed(seed_);
  else
    PLearn::seed();

  //real delta = 1./sqrt(inputsize());
  real delta = 0;
  if (initialization_method == "normal_linear") {
    delta = 1.0 / inputsize();
  } else if (initialization_method == "normal_sqrt") {
    delta = 1.0 / sqrt(real(inputsize()));
  } else {
    PLERROR("In NNet::initializeParams - Unknown value for 'initialization_method'");
  }

  /*
  if(direct_in_to_out)
    {
      //fill_random_uniform(wdirect->value, -delta, +delta);
      fill_random_normal(wdirect->value, 0, delta);
      //wdirect->matValue(0).clear();
    }
  */
  if(nhidden>0)
    {
      //fill_random_uniform(w1->value, -delta, +delta);
      fill_random_normal(w1->value, 0, delta);
      if(direct_in_to_out)
      {
        //fill_random_uniform(wdirect->value, -delta, +delta);
        fill_random_normal(wdirect->value, 0, 0.01*delta);
        wdirect->matValue(0).clear();
      }
      if (initialization_method == "normal_linear") {
        delta = 1.0 / real(nhidden);
      } else if (initialization_method == "normal_sqrt") {
        delta = 1.0 / sqrt(real(nhidden));
      }
      w1->matValue(0).clear();
    }
  if(nhidden2>0)
    {
      //fill_random_uniform(w2->value, -delta, +delta);
      fill_random_normal(w2->value, 0, delta);
      if (initialization_method == "normal_linear") {
        delta = 1.0 / real(nhidden2);
      } else if (initialization_method == "normal_sqrt") {
        delta = 1.0 / sqrt(real(nhidden2));
      }
      w2->matValue(0).clear();
    }
  if (fixed_output_weights)
  {
    static Vec values;
    if (values.size()==0)
      {
        values.resize(2);
        values[0]=-1;
        values[1]=1;
      }
    fill_random_discrete(wout->value, values);
    wout->matValue(0).clear();
  }
  else
  {
    fill_random_uniform(wout->value, -delta, +delta);
    //fill_random_normal(wout->value, 0, delta);
    wout->matValue(0).clear();
  }

  // Reset optimizer
  if(optimizer)
    optimizer->reset();
}

void NNet::forget()
{
  if (train_set) initializeParams();
  stage = 0;
}

//! To use varDeepCopyField.
extern void varDeepCopyField(Var& field, CopiesMap& copies);

void NNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  varDeepCopyField(input, copies);
  varDeepCopyField(target, copies);
  varDeepCopyField(sampleweight, copies);
  varDeepCopyField(w1, copies);
  varDeepCopyField(w2, copies);
  varDeepCopyField(wout, copies);
  varDeepCopyField(outbias, copies);
  varDeepCopyField(wdirect, copies);
  varDeepCopyField(wrec, copies);
  varDeepCopyField(rbf_centers, copies);
  varDeepCopyField(rbf_sigmas, copies);
  varDeepCopyField(junk_prob, copies);
  varDeepCopyField(output, copies);
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
}

} // end of namespace PLearn
