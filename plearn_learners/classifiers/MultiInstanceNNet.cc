// -*- C++ -*-

// MultiInstanceNNet.cc
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
   * $Id: MultiInstanceNNet.cc,v 1.5 2004/02/19 15:25:49 yoshua Exp $
   ******************************************************* */

/*! \file PLearnLibrary/PLearnAlgo/MultiInstanceNNet.h */


#include "AffineTransformVariable.h"
#include "AffineTransformWeightPenalty.h"
#include "BinaryClassificationLossVariable.h"
#include "ClassificationLossVariable.h"
#include "ConcatColumnsVariable.h"
#include "ConcatColumnsVMatrix.h"
#include "CrossEntropyVariable.h"
#include "ExpVariable.h"
#include "LogVariable.h"
#include "LiftOutputVariable.h"
#include "LogSoftmaxVariable.h"
#include "MulticlassLossVariable.h"
#include "MultiInstanceNNet.h"
#include "UnfoldedSumOfVariable.h"
#include "SumOverBagsVariable.h"
#include "SumSquareVariable.h"
#include "random.h"
#include "SigmoidVariable.h"
#include "SumVariable.h"
#include "SumAbsVariable.h"
#include "SumOfVariable.h"
#include "SubVMatrix.h"
#include "TanhVariable.h"
#include "TransposeProductVariable.h"
#include "Var_operators.h"
#include "Var_utils.h"

//#include "DisplayUtils.h"
//#include "GradientOptimizer.h"

namespace PLearn <%
using namespace std;

PLEARN_IMPLEMENT_OBJECT(MultiInstanceNNet, 
                        "Multi-instance feedforward neural network for probabilistic classification", 
                        "The data has the form of a set of input vectors x_i associated with a single\n"
                        "label y. Each x_i is an instance and the overall set of instance is called a bag.\n"
                        "We don't know which of the inputs is responsible for the label, i.e.\n"
                        "there are hidden (not observed) labels y_i associated with each of the inputs x_i.\n"
                        "We also know that y=1 if at least one of the y_i is 1, otherwise y=0, i.e.\n"
                        "   y = y_1 or y_2 or ... y_m\n"
                        "In terms of probabilities, it means that\n"
                        "   P(Y=0|x_1..x_m) = \\prod_{i=1}^m P(y_i=0|x_i)\n"
                        "which determines the likelihood of the observation (x_1...x_m,y).\n"
                        "The neural network implements the computation of P(y_i=1|x_i). The same\n"
                        "model is assumed for all instances in the bag. The number of instances is variable but\n"
                        "bounded a-priori (max_n_instances). The gradient is computed for a whole bag\n"
                        "at a time. The architectural parameters and hyper-parameters of the model\n"
                        "are otherwise the same as for the generic NNet class.\n"
                       );

MultiInstanceNNet::MultiInstanceNNet() // DEFAULT VALUES FOR ALL OPTIONS
  :
  max_n_instances(1),
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
   L1_penalty(false),
   direct_in_to_out(false),
   interval_minval(0), interval_maxval(1),
   batch_size(1)
{}

MultiInstanceNNet::~MultiInstanceNNet()
{
}

void MultiInstanceNNet::declareOptions(OptionList& ol)
{
  declareOption(ol, "max_n_instances", &MultiInstanceNNet::max_n_instances, OptionBase::buildoption, 
                "    maximum number of instances (input vectors x_i) allowed\n");

  declareOption(ol, "nhidden", &MultiInstanceNNet::nhidden, OptionBase::buildoption, 
                "    number of hidden units in first hidden layer (0 means no hidden layer)\n");

  declareOption(ol, "nhidden2", &MultiInstanceNNet::nhidden2, OptionBase::buildoption, 
                "    number of hidden units in second hidden layer (0 means no hidden layer)\n");

  declareOption(ol, "noutputs", &MultiInstanceNNet::noutputs, OptionBase::buildoption, 
                "    number of output units. This gives this learner its outputsize.\n"
                "    It is typically of the same dimensionality as the target for regression problems \n"
                "    But for classification problems where target is just the class number, noutputs is \n"
                "    usually of dimensionality number of classes (as we want to output a score or probability \n"
                "    vector, one per class");

  declareOption(ol, "weight_decay", &MultiInstanceNNet::weight_decay, OptionBase::buildoption, 
                "    global weight decay for all layers\n");

  declareOption(ol, "bias_decay", &MultiInstanceNNet::bias_decay, OptionBase::buildoption, 
                "    global bias decay for all layers\n");

  declareOption(ol, "layer1_weight_decay", &MultiInstanceNNet::layer1_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the first hidden layer.  Is added to weight_decay.\n");
  declareOption(ol, "layer1_bias_decay", &MultiInstanceNNet::layer1_bias_decay, OptionBase::buildoption, 
                "    Additional bias decay for the first hidden layer.  Is added to bias_decay.\n");

  declareOption(ol, "layer2_weight_decay", &MultiInstanceNNet::layer2_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the second hidden layer.  Is added to weight_decay.\n");

  declareOption(ol, "layer2_bias_decay", &MultiInstanceNNet::layer2_bias_decay, OptionBase::buildoption, 
                "    Additional bias decay for the second hidden layer.  Is added to bias_decay.\n");

  declareOption(ol, "output_layer_weight_decay", &MultiInstanceNNet::output_layer_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the output layer.  Is added to 'weight_decay'.\n");

  declareOption(ol, "output_layer_bias_decay", &MultiInstanceNNet::output_layer_bias_decay, OptionBase::buildoption, 
                "    Additional bias decay for the output layer.  Is added to 'bias_decay'.\n");

  declareOption(ol, "direct_in_to_out_weight_decay", &MultiInstanceNNet::direct_in_to_out_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the direct in-to-out layer.  Is added to 'weight_decay'.\n");

  declareOption(ol, "L1_penalty", &MultiInstanceNNet::L1_penalty, OptionBase::buildoption, 
                "    should we use L1 penalty instead of the default L2 penalty on the weights?\n");

  declareOption(ol, "direct_in_to_out", &MultiInstanceNNet::direct_in_to_out, OptionBase::buildoption, 
                "    should we include direct input to output connections?\n");

  declareOption(ol, "optimizer", &MultiInstanceNNet::optimizer, OptionBase::buildoption, 
                "    specify the optimizer to use\n");

  declareOption(ol, "batch_size", &MultiInstanceNNet::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the avergage gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->n_non_missing_rows() \n");

  declareOption(ol, "paramsvalues", &MultiInstanceNNet::paramsvalues, OptionBase::learntoption, 
                "    The learned parameter vector\n");

  inherited::declareOptions(ol);

}

void MultiInstanceNNet::build()
{
  inherited::build();
  build_();
}

void MultiInstanceNNet::setTrainingSet(VMat training_set, bool call_forget)
{ 
  bool training_set_has_changed =
    !train_set || train_set->width()!=training_set->width() ||
    train_set->length()!=training_set->length() || train_set->inputsize()!=training_set->inputsize()
    || train_set->weightsize()!= training_set->weightsize();

  train_set = training_set;
  if (training_set_has_changed)
  {
    inputsize_ = train_set->inputsize();
    targetsize_ = train_set->targetsize();
    weightsize_ = train_set->weightsize();
  }

  if (training_set_has_changed || call_forget)
  {
    build(); // MODIF FAITE PAR YOSHUA: sinon apres un setTrainingSet le build n'est pas complete dans un MultiInstanceNNet train_set = training_set;
    if (call_forget) forget();
  }
}

void MultiInstanceNNet::build_()
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
        PLERROR("MultiInstanceNNet:: can't have nhidden2 (=%d) > 0 while nhidden=0",nhidden2);
      
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
   
      // the output transfer function is FIXED: it must be a sigmoid (0/1 probabilistic classification)

      output = sigmoid(output);

      /*
       * target and weights
       */
      
      target = Var(targetsize(), "target");
      
      if(weightsize_>0)
      {
        if (weightsize_!=1)
          PLERROR("MultiInstanceNNet: expected weightsize to be 1 or 0 (or unspecified = -1, meaning 0), got %d",weightsize_);
        sampleweight = Var(1, "weight");
      }

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

      // Shared values hack...
      if(paramsvalues && (paramsvalues.size() == params.nelems()))
        params << paramsvalues;
      else
        {
          paramsvalues.resize(params.nelems());
          initializeParams();
        }
      params.makeSharedValue(paramsvalues);

      output->setName("element output");

      f = Func(input, output);

      input_to_logP0 = Func(input, log(1 - output));

      bag_size = Var(1,1);
      bag_inputs = Var(max_n_instances,inputsize());
      bag_output = 1-exp(unfoldedSumOf(bag_inputs,bag_size,input_to_logP0,max_n_instances));

      costs.resize(2); // (negative log-likelihood, classification error) for the bag

      costs[0] = cross_entropy(bag_output, target);
      costs[1] = binary_classification_loss(bag_output,target);
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

      invars = bag_inputs & bag_size & target & sampleweight;
      inputs_and_targets_to_costs = Func(invars,costs);

      inputs_and_targets_to_costs->recomputeParents();

      /*  VarArray outvars;
  VarArray testinvars;
  testinvars.push_back(input);
  outvars.push_back(output);
  testinvars.push_back(target);
  outvars.push_back(target);
  
  test_costf = Func(testinvars, output&test_costs);
  test_costf->recomputeParents();
  output_and_target_to_cost = Func(outvars, test_costs); 
  output_and_target_to_cost->recomputeParents();
      */
    }
}

int MultiInstanceNNet::outputsize() const
{ return noutputs; }

TVec<string> MultiInstanceNNet::getTrainCostNames() const
{
  TVec<string> names(3);
  names[0] = "NLL+penalty";
  names[1] = "NLL";
  names[2] = "class_error";
  return names;
}

TVec<string> MultiInstanceNNet::getTestCostNames() const
{ 
  return getTrainCostNames();
}


void MultiInstanceNNet::train()
{
  // MultiInstanceNNet nstages is number of epochs (whole passages through the training set)
  // while optimizer nstages is number of weight updates.
  // So relationship between the 2 depends whether we are in stochastic, batch or minibatch mode

  if(!train_set)
    PLERROR("In MultiInstanceNNet::train, you did not setTrainingSet");
    
  if(!train_stats)
    PLERROR("In MultiInstanceNNet::train, you did not setTrainStatsCollector");

  if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
    build();

  Var totalcost = sumOverBags(train_set, inputs_and_targets_to_costs, max_n_instances, batch_size);
  if(optimizer)
    {
      optimizer->setToOptimize(params, totalcost);  
      optimizer->build();
    }

  // number of optimiser stages corresponding to one learner stage (one epoch)
  int optstage_per_lstage = 0;
  int n_bags = -1;
  if (batch_size<=0)
    optstage_per_lstage = 1;
  else // must count the nb of bags in the training set
  {
    n_bags=0;
    int l = train_set->length();
    ProgressBar* pb = 0;
    if(report_progress)
      pb = new ProgressBar("Counting nb bags in train_set for MultiInstanceNNet ", l);
    Vec row(train_set->width());
    Vec row_target = row.subVec(train_set->inputsize(),train_set->targetsize());
    for (int i=0;i<l;i++)
    {
      train_set->getRow(i,row);
      if (!row_target.hasMissing())
        n_bags++;
      if(pb)
        pb->update(i);
    }
    if(pb)
      delete pb;
    optstage_per_lstage = n_bags/batch_size;
  }

  ProgressBar* pb = 0;
  if(report_progress)
    pb = new ProgressBar("Training MultiInstanceNNet from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

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

  //output_and_target_to_cost->recomputeParents();
  //test_costf->recomputeParents();

  // cerr << "totalcost->value = " << totalcost->value << endl;
  // cout << "Result for benchmark is: " << totalcost->value << endl;
}


void MultiInstanceNNet::computeOutput(const Vec& inputv, Vec& outputv) const
{
  f->fprop(inputv,outputv);
}

void MultiInstanceNNet::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                 Vec& outputv, Vec& costsv) const
{
  //test_costf->fprop(inputv&targetv, outputv&costsv);
  //TO BE DONE
}


void MultiInstanceNNet::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, 
                                   const Vec& targetv, Vec& costsv) const
{
  //output_and_target_to_cost->fprop(outputv&targetv, costsv); 
}

void MultiInstanceNNet::initializeParams()
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

void MultiInstanceNNet::forget()
{
  if (train_set) initializeParams();
  stage = 0;
}

void MultiInstanceNNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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

%> // end of namespace PLearn
