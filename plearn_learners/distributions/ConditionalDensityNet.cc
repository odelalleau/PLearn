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
   * $Id: ConditionalDensityNet.cc,v 1.16 2003/12/02 14:02:01 yoshua Exp $ 
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file ConditionalDensityNet.cc */


#include "ConditionalDensityNet.h"
#include "random.h"
#include "DisplayUtils.h"
#include "DilogarithmVariable.h"
#include "SoftSlopeVariable.h"
#include "plapack.h"

namespace PLearn <%
using namespace std;

ConditionalDensityNet::ConditionalDensityNet() 
/* ### Initialise all fields to their default value here */
  :
   nhidden(0),
   nhidden2(0),
   weight_decay(0),
   bias_decay(1e-6),
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
   maxY(1), // if Y is normalized to be in interval [0,1], that would be OK
   log_likelihood_vs_squared_error_balance(1),
   n_output_density_terms(0),
   steps_type("sloped_steps"),
   centers_initialization("uniform"),
   curve_positions("uniform"),
   scale(5.0),
   unconditional_p0(0.01),
   mu_is_fixed(true),
   initial_hardness(1)
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
                          "For the output curve options (outputs_def='L',D','C', or 'S'), the lower_bound and upper_bound\n"
                          "options of PDistribution are automatically set to 0 and maxY respectively.\n"
                          );

  void ConditionalDensityNet::declareOptions(OptionList& ol)
  {
  declareOption(ol, "nhidden", &ConditionalDensityNet::nhidden, OptionBase::buildoption, 
                "    number of hidden units in first hidden layer (0 means no hidden layer)\n");

  declareOption(ol, "nhidden2", &ConditionalDensityNet::nhidden2, OptionBase::buildoption, 
                "    number of hidden units in second hidden layer (0 means no hidden layer)\n");

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
                "    should we use L1 penalty instead of the default L2 penalty on the weights? (default=0)\n");

  declareOption(ol, "direct_in_to_out", &ConditionalDensityNet::direct_in_to_out, OptionBase::buildoption, 
                "    should we include direct input to output connections? (default=0)\n");

  declareOption(ol, "optimizer", &ConditionalDensityNet::optimizer, OptionBase::buildoption, 
                "    specify the optimizer to use\n");

  declareOption(ol, "batch_size", &ConditionalDensityNet::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the avergage gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->length(); default=1 (stochastic gradient)\n");

  declareOption(ol, "maxY", &ConditionalDensityNet::maxY, OptionBase::buildoption, 
                "    maximum allowed value for Y. Default = 1.0 (data normalized in [0,1]\n");

  declareOption(ol, "log_likelihood_vs_squared_error_balance", &ConditionalDensityNet::log_likelihood_vs_squared_error_balance, 
                OptionBase::buildoption, 
                "    Relative weight given to negative log-likelihood (1- this weight given squared error). Default=1\n");

  declareOption(ol, "n_output_density_terms", &ConditionalDensityNet::n_output_density_terms, 
                OptionBase::buildoption, 
                "    Number of terms (steps) in the output density function.\n");

  declareOption(ol, "steps_type", &ConditionalDensityNet::steps_type, 
                OptionBase::buildoption, 
                "    The type of steps used to build the cumulative distribution.\n"
                "    Allowed values are:\n"
                "      - sigmoid_steps: g(y,theta,i) = sigmoid(s(c_i)*(y-mu_i))\n"
                "      - sloped_steps: g(y,theta,i) = s(s(c_i)*(mu_i-y))-s(s(c_i)*(mu_i-y))\nDefault=sloped_steps\n");

  declareOption(ol, "centers_initialization", &ConditionalDensityNet::centers_initialization, 
                OptionBase::buildoption, 
                "    How to initialize the step centers (mu_i). Allowed values are:\n"
                "      - uniform: at regular intervals in [0,maxY]\n"
                "      - log-scale: as the exponential of values at regular intervals in log-scale, using formula:\n"
                "          i-th position = (exp(scale*(i+1-n_output_density_terms)/n_output_density_terms)-exp(-scale))/(1-exp(-scale))\n");
  declareOption(ol, "curve_positions", &ConditionalDensityNet::curve_positions,
                OptionBase::buildoption, 
                "    How to choose the y-values for the probability curve (upper case output_def):\n"
                "      - uniform: at regular intervals in [0,maxY]\n"
                "      - log-scale: as the exponential of values at regular intervals in log-scale, using formula:\n"
                "          i-th position = (exp(scale*(i+1-n_output_density_terms)/n_output_density_terms)-exp(-scale))/(1-exp(-scale))\n");
  declareOption(ol, "scale", &ConditionalDensityNet::scale,
                OptionBase::buildoption, 
                "    scale used in the log-scale formula for centers_initialization and curve_positions");

  declareOption(ol, "unconditional_p0", &ConditionalDensityNet::unconditional_p0, OptionBase::buildoption, 
                "    approximate unconditional probability of Y=0 (mass point), used\n"
                "    to initialize the parameters.\n");

  declareOption(ol, "mu_is_fixed", &ConditionalDensityNet::mu_is_fixed, OptionBase::buildoption, 
                "    whether to keep the step centers (mu[i]) fixed or to learn them.\n");

  declareOption(ol, "initial_hardness", &ConditionalDensityNet::initial_hardness, OptionBase::buildoption, 
                "    initial value of softplus(c), only used during parameter initialization\n");

  declareOption(ol, "paramsvalues", &ConditionalDensityNet::paramsvalues, OptionBase::learntoption, 
                "    The learned neural network parameter vector\n");


  declareOption(ol, "unconditional_cdf", &ConditionalDensityNet::unconditional_cdf, OptionBase::learntoption, 
                "    Unconditional cumulative distribution function.\n");

  declareOption(ol, "unconditional_delta_cdf", &ConditionalDensityNet::unconditional_delta_cdf, OptionBase::learntoption, 
                "    Variations of the cdf from one step center to the next.\n");

  declareOption(ol, "mu", &ConditionalDensityNet::mu, OptionBase::learntoption, 
                "    Step centers.\n");

  declareOption(ol, "y_values", &ConditionalDensityNet::y_values, OptionBase::learntoption, 
                "    Values of Y at which the cumulative (or density or survival) curves are computed if required.\n");

    inherited::declareOptions(ol);
  }

  void ConditionalDensityNet::build_()
  {
  // Don't do anything if we don't have a train_set
  // It's the only one who knows the inputsize and targetsize anyway...

  if(train_set)  
    {
      lower_bound = 0;
      upper_bound = maxY;
      int n_output_parameters = mu_is_fixed?(1+n_output_density_terms*2):(1+n_output_density_terms*3);

      if (n_curve_points<0)
        n_curve_points = n_output_density_terms+1;

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
      
      if (nhidden==-1) 
        // special code meaning that the inputs should be ignored, only use biases
      {
        wout = Var(1, n_output_parameters, "wout");
        output = transpose(wout);
      }
      // output layer before transfer function
      else
      {
        wout = Var(1+output->size(), n_output_parameters, "wout");
        output = affine_transform(output,wout);
      }
      params.append(wout);

      // direct in-to-out layer
      if(direct_in_to_out)
        {
          wdirect = Var(inputsize(), n_output_parameters, "wdirect");
          //wdirect = Var(1+inputsize(), n_output_parameters, "wdirect");
          output += transposeProduct(wdirect, input);// affine_transform(input,wdirect);
          params.append(wdirect);
        }

      /*
       * target and weights
       */
      
      target = Var(targetsize(), "target");
      
      if(train_set->hasWeights())
        sampleweight = Var(1, "weight");

      // output = parameters of the Y distribution
      
      int i=0;
      a = output[i++]; a->setName("a");
      //b = new SubMatVariable(output,0,i,1,n_output_density_terms); 
      b = new SubMatVariable(output,i,0,n_output_density_terms,1);
      b->setName("b");
      i+=n_output_density_terms;
      //c = new SubMatVariable(output,0,i,1,n_output_density_terms);
      c = new SubMatVariable(output,i,0,n_output_density_terms,1);
      c->setName("c");
      if (mu_is_fixed)
        //mu = Var(1,n_output_density_terms);
        mu = Var(n_output_density_terms,1);
      else
      {
        i+=n_output_density_terms;
        //mu = new SubMatVariable(output,0,i,1,n_output_density_terms);
        mu = new SubMatVariable(output,i,0,n_output_density_terms,1);
      }
      mu->setName("mu");

      /*
       * output density
       */
      Var max_y = var(maxY); max_y->setName("maxY");
      centers = target-mu; centers->setName("centers");
      centers_M = max_y-mu; centers_M->setName("centers_M");
      unconditional_cdf.resize(n_output_density_terms);
      //unconditional_delta_cdf = Var(1,n_output_density_terms);
      unconditional_delta_cdf = Var(n_output_density_terms,1);
      Var left_side = vconcat(var(0.0) & (new SubMatVariable(mu,0,0,n_output_density_terms-1,1)));
      //Var left_side = hconcat(var(0.0) & (new SubMatVariable(mu,0,0,1,n_output_density_terms-1))); 
      left_side->setName("left_side"); 
      initial_hardnesses = var(initial_hardness) / (mu - left_side);
      pos_a = softplus(a); 
      pos_a->setName("pos_a");
      pos_b = softplus(b)*unconditional_delta_cdf; 
      pos_b->setName("pos_b");
      pos_c = softplus(c)*initial_hardnesses; 
      pos_c->setName("pos_c");
      Var scaled_centers = pos_c*centers;
      // scaled centers evaluated at target = M
      Var scaled_centers_M = pos_c*centers_M;
      // scaled centers evaluated at target = 0
      Var scaled_centers_0 = -pos_c*mu;
      if (steps_type=="sigmoid_steps")
      {
        steps = sigmoid(scaled_centers); 
        // steps evaluated at target = M
        steps_M = sigmoid(scaled_centers_M);
        // derivative of steps wrt target
        steps_gradient = pos_c*steps*(1-steps);
        steps_integral = (softplus(scaled_centers_M) - softplus(scaled_centers_0))/pos_c;
        delta_steps = centers_M*steps_M + mu*sigmoid(scaled_centers_0);
      }
      else if (steps_type=="sloped_steps")
      {
        steps = soft_slope(target, pos_c, left_side, mu);
        steps_M = soft_slope(max_y, pos_c, left_side, mu);
        steps_gradient = d_soft_slope(target, pos_c, left_side, mu);
        steps_integral = soft_slope_integral(pos_c,left_side,mu,0.0,maxY);
        delta_steps = soft_slope_limit(target, pos_c, left_side, mu);
      }
      else PLERROR("ConditionalDensityNet::build, steps_type option value unknown: %s",steps_type.c_str());

      steps->setName("steps");
      steps_M->setName("steps_M");
      steps_integral->setName("steps_integral");
      Var density_numerator = dot(pos_b,steps_gradient);
      density_numerator->setName("density_numerator");
      cum_denominator = pos_a + dot(pos_b,steps_M); cum_denominator->setName("cum_denominator");
      Var inverse_denominator = 1.0/cum_denominator;
      inverse_denominator->setName("inverse_denominator");
      cum_numerator = pos_a + dot(pos_b,steps);
      cum_numerator->setName("cum_numerator");
      cumulative = cum_numerator * inverse_denominator;
      cumulative->setName("cumulative");
      density = density_numerator * inverse_denominator;
      density->setName("density");
      // apply l'hopital rule if pos_c --> 0 to avoid blow-up (N.B. lim_{pos_c->0} pos_b/pos_c*steps_integral = pos_b*delta_steps)
      Var lhopital = ifThenElse(isAboveThreshold(pos_c,1e-20),pos_b*steps_integral,pos_b*delta_steps); lhopital->setName("lhopital");
      expected_value = (1 - pos_a*inverse_denominator)*max_y - sum(lhopital)*inverse_denominator;
      expected_value->setName("expected_value");
      /*
       * cost functions:
       *   training_criterion = log_likelihood_vs_squared_error_balance*neg_log_lik
       *                      +(1-log_likelihood_vs_squared_error_balance)*squared_err
       *                      +penalties
       *   neg_log_lik = -log(1_{target=0} cumulative + 1_{target>0} density)
       *   squared_err = square(target - expected_value)
       */
      costs.resize(3);
      
      costs[1] = -log(ifThenElse(isAboveThreshold(target,0.0),density,cumulative));
      costs[2] = square(target-expected_value);
      if (log_likelihood_vs_squared_error_balance==1)
        costs[0] = costs[1];
      else if (log_likelihood_vs_squared_error_balance==0)
        costs[0] = costs[2];
      else costs[0] = log_likelihood_vs_squared_error_balance*costs[1]+
             (1-log_likelihood_vs_squared_error_balance)*costs[2];

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
      {
        params << paramsvalues;
        initialize_mu(mu->value);
      }
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
      if(expected_value)
      {
        outvars.push_back(expected_value);
      }
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

      VarArray outputs_array;

      for (unsigned int i=0;i<outputs_def.length();i++)
      {
        if (outputs_def[i]=='e')
          outputs_array &= expected_value;
        else if (outputs_def[i]=='S' || outputs_def[i]=='C' ||
                 outputs_def[i]=='L' || outputs_def[i]=='D')
        {
          Func prob_f(target&output,outputs_def[i]=='S'?(var(1.0)-cumulative):
                      (outputs_def[i]=='C'?cumulative:
                       (outputs_def[i]=='D'?density:log(density))));
          y_values.resize(n_curve_points);
          if (curve_positions=="uniform")
          {
            real delta = maxY/(n_curve_points-1);
            for (int i=0;i<n_curve_points;i++)
            {
              y_values[i] = var(i*delta);
              y_values[i]->setName("y"+tostring(i));
              outputs_array &= prob_f(y_values[i] & output);
            }
          } else // log-scale
          {
            real denom = 1.0/(1-exp(-scale));
            for (int i=0;i<n_curve_points;i++)
            {
              y_values[i] = var((exp(scale*(i-n_output_density_terms)/n_output_density_terms)-exp(-scale))*denom);
              y_values[i]->setName("y"+tostring(i));
              outputs_array &= prob_f(y_values[i] & output);
            }    
          }
        } else PLERROR("ConditionalDensityNet::build: can't handle outputs_def with option value = %c",outputs_def[i]);
      }
      outputs = hconcat(outputs_array);
      if (mu_is_fixed)
        f = Func(input, params&mu, outputs);
      else
        f = Func(input, params, outputs);
      f->recomputeParents();

      if (mu_is_fixed)
        test_costf = Func(testinvars, params&mu, outputs&test_costs);
      else
        test_costf = Func(testinvars, params, outputs&test_costs);
      test_costf->recomputeParents();
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

void ConditionalDensityNet::computeOutput(const Vec& inputv, Vec& outputv) const
{
  f->fprop(inputv,outputv);
}

void ConditionalDensityNet::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                 Vec& outputv, Vec& costsv) const
{
  test_costf->fprop(inputv&targetv, outputv&costsv);
}

TVec<string> ConditionalDensityNet::getTrainCostNames() const
{
  if (penalties.size() > 0)
  {
    TVec<string> cost_funcs(4);
    cost_funcs[0]="training_criterion+penalty";
    cost_funcs[1]="training_criterion";
    cost_funcs[2]="NLL";
    cost_funcs[3]="mse";
    return cost_funcs;
  }
  else return getTestCostNames();
}

TVec<string> ConditionalDensityNet::getTestCostNames() const
{ 
  TVec<string> cost_funcs(3);
  cost_funcs[0]="training_criterion";
  cost_funcs[1]="NLL";
  cost_funcs[2]="mse";
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
  deepCopyField(outputs, copies);
  deepCopyField(costs, copies);
  deepCopyField(penalties, copies);
  deepCopyField(training_cost, copies);
  deepCopyField(test_costs, copies);
  deepCopyField(params, copies);
  deepCopyField(paramsvalues, copies);
  deepCopyField(f, copies);
  deepCopyField(test_costf, copies);
  deepCopyField(optimizer, copies);
  deepCopyField(unconditional_delta_cdf, copies);
  deepCopyField(unconditional_cdf, copies);
  deepCopyField(mu, copies);
  deepCopyField(y_values,copies);
  inherited::makeDeepCopyFromShallowCopy(copies);
  }


void ConditionalDensityNet::setInput(const Vec& in) const
{
  input->value << in;
}

double ConditionalDensityNet::log_density(const Vec& y) const
{ 
  test_costf->fprop(input->value & y,target_dependent_outputs);
  return -target_dependent_outputs[1];
}

double ConditionalDensityNet::survival_fn(const Vec& y) const
{ 
  test_costf->fprop(input->value & y,target_dependent_outputs);
  return 1-target_dependent_outputs[target_dependent_outputs.length()-1];
}

double ConditionalDensityNet::cdf(const Vec& y) const
{ 
  test_costf->fprop(input->value & y,target_dependent_outputs);
  return target_dependent_outputs[target_dependent_outputs.length()-1];
}

void ConditionalDensityNet::expectation(Vec& mu) const
{ 
  f->fprop(input->value,mu);
}

void ConditionalDensityNet::variance(Mat& covar) const
{ 
  PLERROR("variance not implemented for ConditionalDensityNet"); 
}

void ConditionalDensityNet::resetGenerator(long g_seed) const
{ 
  PLERROR("resetGenerator not implemented for ConditionalDensityNet"); 
}

void ConditionalDensityNet::generate(Vec& x) const
{ 
  PLERROR("generate not implemented for ConditionalDensityNet"); 
}


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
      delta = 0.1/nhidden;
      w1->matValue(0).clear();
    }
  if(nhidden2>0)
    {
      //fill_random_uniform(w2->value, -delta, +delta);
      //delta = 1./sqrt(nhidden2);
      delta = 0.1/nhidden2;
      fill_random_normal(w2->value, 0, delta);
      w2->matValue(0).clear();
    }
  //fill_random_uniform(wout->value, -delta, +delta);
  fill_random_normal(wout->value, 0, delta);
  Vec output_biases = wout->matValue(0);
  int i=0;
  Vec a_ = output_biases.subVec(i++,1);
  Vec b_ = output_biases.subVec(i,n_output_density_terms); i+=n_output_density_terms;
  Vec c_ = output_biases.subVec(i,n_output_density_terms); i+=n_output_density_terms;
  Vec mu_;
  Vec sc(n_output_density_terms);
  if (mu_is_fixed)
    mu_ = mu->value;
  else
    mu_ = output_biases.subVec(i,n_output_density_terms); i+=n_output_density_terms;
  b_.fill(inverse_softplus(1.0));
  initialize_mu(mu_);
  for (int i=0;i<n_output_density_terms;i++)
  {
    real prev_mu = i==0?0:mu_[i-1];
    real delta = mu_[i]-prev_mu;
    sc[i] = initial_hardness/delta;
    c_[i] = inverse_softplus(1.0);
  }

  real *dcdf = unconditional_delta_cdf->valuedata;
  if (dcdf[0]==0)
    a_[0]=inverse_softplus(unconditional_p0);
  else 
  {
    real s=0;
    if (steps_type=="sigmoid_steps")
      for (int i=0;i<n_output_density_terms;i++)
        s+=dcdf[i]*(unconditional_p0*sigmoid(sc[i]*(maxY-mu_[i]))-sigmoid(-sc[i]*mu_[i]));
    else
      for (int i=0;i<n_output_density_terms;i++)
      {
        real prev_mu = i==0?0:mu_[i-1];
        real ss1 = soft_slope(maxY,sc[i],prev_mu,mu_[i]);
        real ss2 = soft_slope(0,sc[i],prev_mu,mu_[i]);
        s+=dcdf[i]*(unconditional_p0*ss1 - ss2);
      }
    real sa=s/(1-unconditional_p0);
    a_[0]=inverse_softplus(sa);

    /*
    Mat At(n_output_density_terms,n_output_density_terms); // transpose of the linear system matrix
    Mat rhs(1,n_output_density_terms); // right hand side of the linear system
    // solve the system to find b's that make the unconditional fit the observed data
    //  sum_j sb_j dcdf_j (cdf_j step_j(maxY) - step_j(mu_i)) = sa (1 - cdf_i)
    //
    for (int i=0;i<n_output_density_terms;i++)
    {
      real* Ati = At[i];
      real prev_mu = i==0?0:mu_[i-1];
      for (int j=0;j<n_output_density_terms;j++)
      {
        if (steps_type=="sigmoid_steps")
          Ati[j] = dcdf[i]*(unconditional_cdf[j]*sigmoid(initial_hardness*(maxY-mu_[i]))-
                           sigmoid(initial_hardness*(mu_[j]-mu_[i])));
        else
          Ati[j] = dcdf[i]*(unconditional_cdf[j]*soft_slope(maxY,initial_hardness,prev_mu,mu_[i])-
                            soft_slope(mu_[j],initial_hardness,prev_mu,mu_[i]));
      }
      rhs[0][i] = sa*(1-unconditional_cdf[i]);
    }
    TVec<int> pivots(n_output_density_terms);
    int status = lapackSolveLinearSystem(At,rhs,pivots);
    if (status==0)
      for (int i=0;i<n_output_density_terms;i++)
        b_[i] = inverse_softplus(rhs[0][i]);
    else
      PLWARNING("ConditionalDensityNet::initializeParams() Could not invert matrix to obtain exact init. of b");
    */
  }

  // Reset optimizer
  if(optimizer)
    optimizer->reset();
}

void ConditionalDensityNet::initialize_mu(Vec& mu_)
{
  if (centers_initialization=="uniform")
  {
    real delta=maxY/n_output_density_terms;
    real center=delta;
    for (int i=0;i<n_output_density_terms;i++,center+=delta)
      mu_[i]=center;
  } else if (centers_initialization=="log-scale")
  {
    real denom = 1.0/(1-exp(-scale));
    for (int i=0;i<n_output_density_terms;i++)
      mu_[i]=(exp(scale*(i+1-n_output_density_terms)/n_output_density_terms)-exp(-scale))*denom;
  } else PLERROR("ConditionalDensityNet::initialize_mu: unknown value %s for centers_initialization option",
                 centers_initialization.c_str());
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

  // estimate the unconditional cdf
  static real weight;
  Vec mu_values = mu->value;
  unconditional_cdf.clear();
  real sum_w=0;
  unconditional_p0 = 0;
  for (int i=0;i<l;i++)
  {
    train_set->getExample(i,input->value,target->value,weight);
    real y = target->valuedata[0];
    if (y<=0)
      unconditional_p0 += weight;
    for (int j=0;j<n_output_density_terms;j++)
      if (y<=mu_values[j]) 
        unconditional_cdf[j]+=weight;
    sum_w += weight;
  }
  unconditional_cdf *= 1.0/sum_w;
  unconditional_p0 *= 1.0/sum_w;
  unconditional_delta_cdf->valuedata[0]=unconditional_cdf[0]-unconditional_p0;
  for (int i=1;i<n_output_density_terms;i++)
    unconditional_delta_cdf->valuedata[i]=unconditional_cdf[i]-unconditional_cdf[i-1];
  
  initializeParams();
  // debugging
  f->fprop(input->value,outputs->value);
      static bool display_graph = false;
      //displayVarGraph(outputs,true);
      if (display_graph)
        displayFunction(f,true);
      if (display_graph)
        displayFunction(test_costf,true);

  int initial_stage = stage;
  bool early_stop=false;
  while(stage<nstages && !early_stop)
    {
      optimizer->nstages = optstage_per_lstage;
      train_stats->forget();
      optimizer->early_stop = false;
      optimizer->optimizeN(*train_stats);

      if (display_graph)
        displayFunction(f,true);
      if (display_graph)
        displayFunction(test_costf,true);

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

  test_costf->recomputeParents();
}



%> // end of namespace PLearn
