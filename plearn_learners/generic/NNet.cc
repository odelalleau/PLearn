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
   * $Id: NNet.cc,v 1.42 2004/02/29 16:44:06 nova77 Exp $
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
   L1_penalty(false),
   direct_in_to_out(false),
   output_transfer_func(""),
   interval_minval(0), interval_maxval(1),
   batch_size(1)
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

  declareOption(ol, "direct_in_to_out", &NNet::direct_in_to_out, OptionBase::buildoption, 
                "    should we include direct input to output connections?\n");

  declareOption(ol, "output_transfer_func", &NNet::output_transfer_func, OptionBase::buildoption, 
                "    what transfer function to use for ouput layer? \n"
                "    one of: tanh, sigmoid, exp, softplus, softmax \n"
                "    or interval(<minval>,<maxval>), which stands for\n"
                "    <minval>+(<maxval>-<minval>)*sigmoid(.).\n"
                "    An empty string or \"none\" means no output transfer function \n");

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
                "      lift_output (not a real cost function, just the output for lift computation)\n"
                "    The first function of the list will be used as \n"
                "    the objective function to optimize \n"
                "    (possibly with an added weight decay penalty) \n");
  
  declareOption(ol, "classification_regularizer", &NNet::classification_regularizer, OptionBase::buildoption, 
                "    used only in the stable_cross_entropy cost function, to fight overfitting (0<=r<1)\n");

  declareOption(ol, "optimizer", &NNet::optimizer, OptionBase::buildoption, 
                "    specify the optimizer to use\n");

  declareOption(ol, "batch_size", &NNet::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the avergage gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->length() \n");

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

      // first hidden layer
      if(nhidden>0)
        {
          w1 = Var(1+inputsize(), nhidden, "w1");      
          output = tanh(affine_transform(output,w1));
          params.append(w1);
        }

      // second hidden layer
      if(nhidden2>0)
        {
          w2 = Var(1+nhidden, nhidden2, "w2");
          output = tanh(affine_transform(output,w2));
          params.append(w2);
        }

      if (nhidden2>0 && nhidden==0)
        PLERROR("NNet:: can't have nhidden2 (=%d) > 0 while nhidden=0",nhidden2);
      
      // output layer before transfer function
      wout = Var(1+output->size(), outputsize(), "wout");
      output = affine_transform(output,wout);
      params.append(wout);

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
      unsigned int p=0;
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
            unsigned int q = (unsigned int)output_transfer_func.find(",");
            interval_minval = atof(output_transfer_func.substr(p+1,q-(p+1)).c_str());
            unsigned int r = (unsigned int)output_transfer_func.find(")");
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
  real delta = 1./inputsize();
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
      //delta = 1./sqrt(nhidden);
      fill_random_normal(w1->value, 0, delta);
      if(direct_in_to_out)
      {
        //fill_random_uniform(wdirect->value, -delta, +delta);
        fill_random_normal(wdirect->value, 0, 0.01*delta);
        wdirect->matValue(0).clear();
      }
      delta = 1./nhidden;
      w1->matValue(0).clear();
    }
  if(nhidden2>0)
    {
      //fill_random_uniform(w2->value, -delta, +delta);
      //delta = 1./sqrt(nhidden2);
      fill_random_normal(w2->value, 0, delta);
      delta = 1./nhidden2;
      w2->matValue(0).clear();
    }
  //fill_random_uniform(wout->value, -delta, +delta);
  fill_random_normal(wout->value, 0, delta);
  wout->matValue(0).clear();

  // Reset optimizer
  if(optimizer)
    optimizer->reset();
}

void NNet::forget()
{
  if (train_set) initializeParams();
  stage = 0;
}

void NNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(input, copies);
  deepCopyField(target, copies);
  deepCopyField(sampleweight, copies);
  deepCopyField(w1, copies);
  deepCopyField(w2, copies);
  deepCopyField(wout, copies);
  deepCopyField(wdirect, copies);
  deepCopyField(output, copies);
  deepCopyField(costs, copies);
  deepCopyField(penalties, copies);
  deepCopyField(training_cost, copies);
  deepCopyField(test_costs, copies);
  deepCopyField(invars, copies);
  deepCopyField(params, copies);
  deepCopyField(paramsvalues, copies);
  deepCopyField(f, copies);
  deepCopyField(test_costf, copies);
  deepCopyField(output_and_target_to_cost, copies);
  deepCopyField(optimizer, copies);
}

} // end of namespace PLearn
