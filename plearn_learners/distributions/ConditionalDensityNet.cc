// -*- C++ -*-

// ConditionalDensityNet.cc
//
// Copyright (C) 2003 Yoshua Bengio 
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
   * $Id: ConditionalDensityNet.cc,v 1.2 2003/11/17 01:53:22 yoshua Exp $ 
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file ConditionalDensityNet.cc */


#include "ConditionalDensityNet.h"

namespace PLearn <%
using namespace std;

ConditionalDensityNet::ConditionalDensityNet() 
/* ### Initialise all fields to their default value here */
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
   L1_penalty(false),
   direct_in_to_out(false),
   batch_size(1),
   maxY(0), // must be provided
   max_likelihood_vs_squared_error_balance(1),
   n_output_density_terms(0),
   steps_type("sloped_steps"),
   centers_initialization("uniform")
  {
  }

  PLEARN_IMPLEMENT_OBJECT(ConditionalDensityNet, "Neural Network that Implements a Positive Random Variable Conditional Density", 
                          "The input vector is used to compute parameters of an output density or output\n"
                          "cumulative distribution as well as output expected value. The ASSUMPTIONS\n"
                          "on the generating distribution P(Y|X) are the following:\n"
                          "  * Y is a single real value\n"
                          "  * 0 <= Y <= maxY, with maxY a known finite value\n"
                          "  * the density has a mass point at Y=0\n"
                          "  * the density is continuous for Y>0\n"
                          "The form of the conditional cumulative of Y is the following:\n"
                          "   P(Y<=y|theta) = (1/Z) (s(a) + sum_i s(b_i) g(y,theta,i))\n"
                          "where s(z)=log(1+exp(z)) is the softplus function, and g is a monotonic function\n"
                          "in y whose first derivative and indefinite integral are known analytically.\n"
                          "The parameters theta of Y's distribution are (a,b_1,b_2,...,c_1,c_2,...,mu_1,mu_2,...),\n"
                          "which are obtained as the unconstrained outputs (no output transfer function) of a neural network.\n"
                          "The normalization constant Z is computed analytically easily:\n"
                          "   Z = s(a) + sum_i s(b_i) g(y,theta,i)\n"
                          "The current implementation considers two choices for g:\n"
                          "  - sigmoid_steps: g(y,theta,i) = sigmoid(s(c_i)*(y-mu_i))\n"
                          "  - sloped_steps: g(y,theta,i) = s(s(c_i)*(mu_i-y))-s(s(c_i)*(mu_i-y))\n"
                          "The density is analytically obtained using the derivative g' of g and\n"
                          "expected value is analytically obtained using the primitive G of g.\n"
                          "For the mass point at the origin,\n"
                          "   P(Y=0|theta) = P(Y<=0|theta).\n"
                          "For positive values of Y:\n"
                          "   p(y|theta) = (1/Z) sum_i s(b_i) s(c_i) g'(y,theta,i).\n"
                          "And the expected value of Y is obtained using the primitive:\n"
                          "   E[Y|theta] = (1-s(a)/Z)*M - sum_i s(b_i)/(Z*s(c_i))*(G(M,theta,i)-G(0,theta,i))\n"
                          "Training the model can be done by maximum likelihood (minimizing the log of the\n"
                          "density) or by minimizing the average of squared error (y-E[Y|theta])^2\n"
                          "or a combination of the two (with the max_likelihood_vs_squared_error_balance option).\n"
                          "The step 'centers' mu_i are initialized according to some rule, in the interval [0,maxY]:\n"
                          " - uniform: at regular intervals in [0,maxY]\n"
                          " - log-scale: as the exponential of values at regular intervals in [0,log(1+maxY)], minus 1.\n"
                          "The c_i are initialized to 2/(mu_{i+1}-mu_{i-1}), and a and b_i to 0.\n"
                          );

  void ConditionalDensityNet::declareOptions(OptionList& ol)
  {
  declareOption(ol, "nhidden", &ConditionalDensityNet::nhidden, OptionBase::buildoption, 
                "    number of hidden units in first hidden layer (0 means no hidden layer)\n");

  declareOption(ol, "nhidden2", &ConditionalDensityNet::nhidden2, OptionBase::buildoption, 
                "    number of hidden units in second hidden layer (0 means no hidden layer)\n");

  declareOption(ol, "noutputs", &ConditionalDensityNet::noutputs, OptionBase::buildoption, 
                "    number of output units. This gives this learner its outputsize.\n"
                "    It is typically of the same dimensionality as the target for regression problems \n"
                "    But for classification problems where target is just the class number, noutputs is \n"
                "    usually of dimensionality number of classes (as we want to output a score or probability \n"
                "    vector, one per class");

  declareOption(ol, "weight_decay", &ConditionalDensityNet::weight_decay, OptionBase::buildoption, 
                "    global weight decay for all layers\n");

  declareOption(ol, "bias_decay", &ConditionalDensityNet::bias_decay, OptionBase::buildoption, 
                "    global bias decay for all layers\n");

  declareOption(ol, "layer1_weight_decay", &ConditionalDensityNet::layer1_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the first hidden layer.  Is added to weight_decay.\n");
  declareOption(ol, "layer1_bias_decay", &ConditionalDensityNet::layer1_bias_decay, OptionBase::buildoption, 
                "    Additional bias decay for the first hidden layer.  Is added to bias_decay.\n");

  declareOption(ol, "layer2_weight_decay", &ConditionalDensityNet::layer2_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the second hidden layer.  Is added to weight_decay.\n");

  declareOption(ol, "layer2_bias_decay", &ConditionalDensityNet::layer2_bias_decay, OptionBase::buildoption, 
                "    Additional bias decay for the second hidden layer.  Is added to bias_decay.\n");

  declareOption(ol, "output_layer_weight_decay", &ConditionalDensityNet::output_layer_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the output layer.  Is added to 'weight_decay'.\n");

  declareOption(ol, "output_layer_bias_decay", &ConditionalDensityNet::output_layer_bias_decay, OptionBase::buildoption, 
                "    Additional bias decay for the output layer.  Is added to 'bias_decay'.\n");

  declareOption(ol, "direct_in_to_out_weight_decay", &ConditionalDensityNet::direct_in_to_out_weight_decay, OptionBase::buildoption, 
                "    Additional weight decay for the direct in-to-out layer.  Is added to 'weight_decay'.\n");

  declareOption(ol, "L1_penalty", &ConditionalDensityNet::L1_penalty, OptionBase::buildoption, 
                "    should we use L1 penalty instead of the default L2 penalty on the weights?\n");

  declareOption(ol, "direct_in_to_out", &ConditionalDensityNet::direct_in_to_out, OptionBase::buildoption, 
                "    should we include direct input to output connections?\n");

  declareOption(ol, "optimizer", &ConditionalDensityNet::optimizer, OptionBase::buildoption, 
                "    specify the optimizer to use\n");

  declareOption(ol, "batch_size", &ConditionalDensityNet::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the avergage gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->length() \n");

  declareOption(ol, "maxY", &ConditionalDensityNet::maxY, OptionBase::buildoption, 
                "    maximum allowed value for Y.\n");

  declareOption(ol, "log_likelihood_vs_squared_error_balance", &ConditionalDensityNet::log_likelihood_vs_squared_error_balance, 
                OptionBase::buildoption, 
                "    Relative weight given to negative log-likelihood (1- this weight given squared error).\n");

  declareOption(ol, "n_output_density_terms", &ConditionalDensityNet::n_output_density_terms, 
                OptionBase::buildoption, 
                "    Number of terms (steps) in the output density function.\n");

  declareOption(ol, "steps_type", &ConditionalDensityNet::steps_type, 
                OptionBase::buildoption, 
                "    The type of steps used to build the cumulative distribution.\n"
                "    Allowed values are:\n"
                "      - sigmoid_steps: g(y,theta,i) = sigmoid(s(c_i)*(y-mu_i))\n"
                "      - sloped_steps: g(y,theta,i) = s(s(c_i)*(mu_i-y))-s(s(c_i)*(mu_i-y))\n.\n");

  declareOption(ol, "centers_initialization", &ConditionalDensityNet::centers_initialization, 
                OptionBase::buildoption, 
                "    How to initialize the step centers (mu_i). Allowed values are:\n"
                "      - uniform: at regular intervals in [0,maxY]\n"
                "      - log-scale: as the exponential of values at regular intervals in [0,log(1+maxY)], minus 1\n");

  declareOption(ol, "paramsvalues", &ConditionalDensityNet::paramsvalues, OptionBase::learntoption, 
                "    The learned neural network parameter vector\n");

    inherited::declareOptions(ol);
  }

  void ConditionalDensityNet::build_()
  {
  // Don't do anything if we don't have a train_set
  // It's the only one who knows the inputsize and targetsize anyway...

  if(train_set)  
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
        PLERROR("ConditionalDensityNet:: can't have nhidden2 (=%d) > 0 while nhidden=0",nhidden2);
      
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
            unsigned int q = output_transfer_func.find(",");
            interval_minval = atof(output_transfer_func.substr(p+1,q-(p+1)).c_str());
            unsigned int r = output_transfer_func.find(")");
            interval_maxval = atof(output_transfer_func.substr(q+1,r-(q+1)).c_str());
            output = interval_minval + (interval_maxval - interval_minval)*sigmoid(output);
          }
          else
            PLERROR("In ConditionalDensityNet::build_()  unknown output_transfer_func option: %s",output_transfer_func.c_str());
        }

      /*
       * target and weights
       */
      
      target = Var(targetsize(), "target");
      
      if(train_set->hasWeights())
        sampleweight = Var(1, "weight");

      /*

       * costfuncs
       */
      int ncosts = cost_funcs.size();  
      if(ncosts<=0)
        PLERROR("In ConditionalDensityNet::build_()  Empty cost_funcs : must at least specify the cost function to optimize!");
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
          else if(cost_funcs[k]=="multiclass_error")
            costs[k] = multiclass_loss(output, target);
          else if(cost_funcs[k]=="cross_entropy")
            costs[k] = cross_entropy(output, target);
          else if (cost_funcs[k]=="stable_cross_entropy") {
            costs[k] = stable_cross_entropy(before_transfer_func, target);
          }
          else  // Assume we got a Variable name and its options
            {
              costs[k]= dynamic_cast<Variable*>(newObject(cost_funcs[k]));
              if(costs[k].isNull())
                PLERROR("In ConditionalDensityNet::build_()  unknown cost_func option: %s",cost_funcs[k].c_str());
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

      // apply penalty to cost
      if(penalties.size() != 0) {
        // only multiply by sampleweight if there are weights
        if (train_set->hasWeights()) {
          training_cost = hconcat(sampleweight*sum(hconcat(costs[0] & penalties))
                                  & (test_costs*sampleweight));
        }
        else {
          training_cost = hconcat(sum(hconcat(costs[0] & penalties)) & test_costs);
        }
      }
      else {
        // only multiply by sampleweight if there are weights
        if(train_set->hasWeights()) {
          training_cost = test_costs*sampleweight;
        } else {
          training_cost = test_costs;
        }
      }
      
      training_cost->setName("training_cost");
      test_costs->setName("test_costs");
      output->setName("output");
      
      // Shared values hack...
      if(paramsvalues && (paramsvalues.size() == params.nelems()))
        params << paramsvalues;
      else
        {
          paramsvalues.resize(params.nelems());
          initializeParams();
        }
      params.makeSharedValue(paramsvalues);

      // Funcs
      VarArray invars;
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

      // The total training cost
      int l = train_set->length();
      int nsamples = batch_size>0 ? batch_size : l;
      Func paramf = Func(invars, training_cost); // parameterized function to optimize
      Var totalcost = meanOf(train_set, paramf, nsamples);

      if(optimizer)
        {
          optimizer->setToOptimize(params, totalcost);  
          optimizer->build();
        }
    }
  }

int ConditionalDensityNet::outputsize() const
{ return 1; }

TVec<string> ConditionalDensityNet::getTrainCostNames() const
{
  if (penalties.size() > 0)
    return (cost_funcs[0]+"+penalty") & cost_funcs;
  else
    return cost_funcs;
}

TVec<string> ConditionalDensityNet::getTestCostNames() const
{ 
  return cost_funcs;
}

  // ### Nothing to add here, simply calls build_
  void ConditionalDensityNet::build()
  {
    inherited::build();
    build_();
  }

  void ConditionalDensityNet::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
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
  deepCopyField(params, copies);
  deepCopyField(paramsvalues, copies);
  deepCopyField(f, copies);
  deepCopyField(test_costf, copies);
  deepCopyField(output_and_target_to_cost, copies);
  deepCopyField(optimizer, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);

  }


double ConditionalDensityNet::log_density(const Vec& x) const
{ PLERROR("density not implemented for ConditionalDensityNet"); return 0; }

double ConditionalDensityNet::survival_fn(const Vec& x) const
{ PLERROR("survival_fn not implemented for ConditionalDensityNet"); return 0; }

double ConditionalDensityNet::cdf(const Vec& x) const
{ PLERROR("cdf not implemented for ConditionalDensityNet"); return 0; }

void ConditionalDensityNet::expectation(Vec& mu) const
{ PLERROR("expectation not implemented for ConditionalDensityNet"); }

void ConditionalDensityNet::variance(Mat& covar) const
{ PLERROR("variance not implemented for ConditionalDensityNet"); }

void ConditionalDensityNet::resetGenerator(long g_seed) const
{ PLERROR("resetGenerator not implemented for ConditionalDensityNet"); }

void ConditionalDensityNet::generate(Vec& x) const
{ PLERROR("generate not implemented for ConditionalDensityNet"); }


// Default version of inputsize returns learner->inputsize()
// If this is not appropriate, you should uncomment this and define
// it properly in the .cc
// int ConditionalDensityNet::inputsize() const {}

void ConditionalDensityNet::initializeParams()
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

//! Remove this method, if your distribution does not implement it
void ConditionalDensityNet::forget()
{
  if (train_set) initializeParams();
  stage = 0;
}
    
//! Remove this method, if your distribution does not implement it
void ConditionalDensityNet::train()
{
  if(!train_set)
    PLERROR("In ConditionalDensityNet::train, you did not setTrainingSet");
    
  if(!train_stats)
    PLERROR("In ConditionalDensityNet::train, you did not setTrainStatsCollector");

  int l = train_set->length();  

  if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
    build();

  // number of samples seen by optimizer before each optimizer update
  int nsamples = batch_size>0 ? batch_size : l;

  // number of optimiser stages corresponding to one learner stage (one epoch)
  int optstage_per_lstage = l/nsamples;

  ProgressBar* pb = 0;
  if(report_progress)
    pb = new ProgressBar("Training ConditionalDensityNet from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

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
        cerr << "Epoch " << stage << " train objective: " << train_stats->getMean() << endl;
      ++stage;
      if(pb)
        pb->update(stage-initial_stage);
    }
  if(verbosity>1)
    cerr << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

  if(pb)
    delete pb;

  output_and_target_to_cost->recomputeParents();
  test_costf->recomputeParents();
}



%> // end of namespace PLearn
