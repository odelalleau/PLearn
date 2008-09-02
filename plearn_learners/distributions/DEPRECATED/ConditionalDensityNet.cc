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
 * $Id$ 
 ******************************************************* */

// Authors: Yoshua Bengio

/*! \file PLearn/plearn_learners/distributions/DEPRECATED/ConditionalDensityNet.cc */

#include <plearn/var/AffineTransformVariable.h>
#include <plearn/var/AffineTransformWeightPenalty.h>
#include "ConditionalDensityNet.h"
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/var/ConcatRowsVariable.h>
#include <plearn/var/CutBelowThresholdVariable.h>
#include <plearn/display/DisplayUtils.h>
#include <plearn/var/DotProductVariable.h>
#include <plearn/var/IfThenElseVariable.h>
#include <plearn/var/IsAboveThresholdVariable.h>
#include <plearn/var/LogVariable.h>
//#include "DilogarithmVariable.h"
#include <plearn/var/SoftSlopeVariable.h>
#include <plearn/var/SoftSlopeIntegralVariable.h>
//#include "plapack.h"
#include <plearn/var/SoftplusVariable.h>
#include <plearn/var/SubMatTransposeVariable.h>
#include <plearn/var/SubMatVariable.h>
#include <plearn/var/SumAbsVariable.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/SumSquareVariable.h>
#include <plearn/var/SumVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/TransposeProductVariable.h>
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;

ConditionalDensityNet::ConditionalDensityNet() 
    : nhidden(0),
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
      penalty_type("L2_square"),
      L1_penalty(false),
      direct_in_to_out(false),
      batch_size(1),
      c_penalization(0),
      maxY(1), // if Y is normalized to be in interval [0,1], that would be OK
      thresholdY(0.1),
      log_likelihood_vs_squared_error_balance(1),
      separate_mass_point(1),
      n_output_density_terms(0),
      generate_precision(1e-3),
      steps_type("sloped_steps"),
      centers_initialization("data"),
      curve_positions("uniform"),
      scale(5.0),
      unconditional_p0(0.01),
      mu_is_fixed(true),
      initial_hardness(1)
{}

PLEARN_IMPLEMENT_OBJECT(ConditionalDensityNet, "Neural Network that Implements a Positive Random Variable Conditional Density", 
                        "The input vector is used to compute parameters of an output density or output\n"
                        "cumulative distribution as well as output expected value. The ASSUMPTIONS\n"
                        "on the generating distribution P(Y|X) are the following:\n"
                        "  * Y is a single real value\n"
                        "  * 0 <= Y <= maxY, with maxY a known finite value\n"
                        "  * the density has a mass point at Y=0\n"
                        "  * the density is continuous for Y>0\n"
                        "The form of the conditional cumulative of Y is the following (separate_mass_points=false):\n"
                        "   P(Y<=y|theta) = (1/Z) (s(a) + sum_i u_i s(b_i) g(y,theta,i))\n"
                        "or for separate_mass_point=true:\n"
                        "   P(Y<=y|theta) = sigmoid(a) + (1-sigmoid(a))(sum_i u_i s(b_i) (g(y,theta,i)-g(0,theta,i))/Z\n"
                        "where s(z)=log(1+exp(z)) is the softplus function, and g is a monotonic function\n"
                        "in y whose first derivative and indefinite integral are known analytically.\n"
                        "The u_i are fixed from the unconditional distribution, such that s(b_i)=1 gives\n"
                        "approximately the right unconditional cumulative function (for infinite hardness):\n"
                        "  u_i = P(mu_{i-1}<Y<=mu_i) [unconditional].\n"
                        "The parameters theta of Y's distribution are (a,b_1,b_2,...,c_1,c_2,...,mu_1,mu_2,...),\n"
                        "which are obtained as the unconstrained outputs (no output transfer function) of a neural network.\n"
                        "The normalization constant Z is computed analytically easily: (separate_mass_point=false)\n"
                        "   Z = s(a) + sum_i u_i s(b_i) g(y,theta,i)\n"
                        "or for separate_mass_point=true:\n"
                        "   Z = sum_i s(b_i) (g(y,theta,i)-g(0,theta,i))\n"
                        "The current implementation considers two choices for g:\n"
                        "  - sigmoid_steps: g(y,theta,i) = sigmoid(h*s(c_i)*(y-mu_i)/(mu_{i+1}-mu_i))\n"
                        "  - sloped_steps: g(y,theta,i) = 1 + s(s(c_i)*(mu_i-y))-s(s(c_i)*(mu_{i+1}-y))/(s(c_i)*(mu_{i+1}-mu_i))\n"
                        "where h is the 'initial_hardness' option.\n"
                        "The density is analytically obtained using the derivative g' of g and\n"
                        "expected value is analytically obtained using the primitive G of g.\n"
                        "For the mass point at the origin,\n"
                        "   P(Y=0|theta) = P(Y<=0|theta).\n"
                        "(which is simply sigmoid(a) if separate_mass_point).\n"
                        "For positive values of Y: (separate_mass_point=false)\n"
                        "   p(y|theta) = (1/Z) sum_i s(b_i) g'(y,theta,i).\n"
                        "or for separate_mass_point=true:\n"
                        "   p(y|theta) = (1-sigmoid(a)) (1/Z) sum_i s(b_i) g'(y,theta,i).\n"
                        "And the expected value of Y is obtained using the primitive: (separate_mass_point=false)\n"
                        "   E[Y|theta] = (1/Z)*s(a)*M + sum_i u_i s(b_i)(G(M,theta,i)-G(0,theta,i)))\n"
                        "or for separate_mass_point=true:\n"
                        "   E[Y|theta] = M - ((sigmoid(a)-(1-sigmoid(a)*(1/Z)*sum_i u_i s(b_i)g(0,theta,i))*M + (1-sigmoid(a))*(1/Z)*sum_i u_i s(b_i)(G(M,theta,i)-G(0,theta,0)))\n"
                        "Training the model can be done by maximum likelihood (minimizing the log of the\n"
                        "density) or by minimizing the average of squared error (y-E[Y|theta])^2\n"
                        "or a combination of the two (with the max_likelihood_vs_squared_error_balance option).\n"
                        "The step 'centers' mu_i are initialized according to some rule, in the interval [0,maxY]:\n"
                        " - uniform: at regular intervals in [0,maxY]\n"
                        " - log-scale: as the exponential of values at regular intervals in [0,log(1+maxY)], minus 1.\n"
                        "The c_i and b_i are initialized to inverse_softplus(1), and a using the empirical unconditional P(Y=0).\n"
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

    declareOption(ol, "penalty_type", &ConditionalDensityNet::penalty_type,
                  OptionBase::buildoption,
                  "Penalty to use on the weights (for weight and bias decay).\n"
                  "Can be any of:\n"
                  "  - \"L1\": L1 norm,\n"
                  "  - \"L1_square\": square of the L1 norm,\n"
                  "  - \"L2_square\" (default): square of the L2 norm.\n");

    declareOption(ol, "L1_penalty", &ConditionalDensityNet::L1_penalty, OptionBase::buildoption,
                  "Deprecated - You should use \"penalty_type\" instead\n"
                  "should we use L1 penalty instead of the default L2 penalty on the weights?\n");

    declareOption(ol, "direct_in_to_out", &ConditionalDensityNet::direct_in_to_out, OptionBase::buildoption, 
                  "    should we include direct input to output connections? (default=0)\n");

    declareOption(ol, "optimizer", &ConditionalDensityNet::optimizer, OptionBase::buildoption, 
                  "    specify the optimizer to use\n");

    declareOption(ol, "batch_size", &ConditionalDensityNet::batch_size, OptionBase::buildoption, 
                  "    how many samples to use to estimate the avergage gradient before updating the weights\n"
                  "    0 is equivalent to specifying training_set->length(); default=1 (stochastic gradient)\n");

    declareOption(ol, "maxY", &ConditionalDensityNet::maxY, OptionBase::buildoption, 
                  "    maximum allowed value for Y. Default = 1.0 (data normalized in [0,1]\n");

    declareOption(ol, "thresholdY", &ConditionalDensityNet::thresholdY, OptionBase::buildoption, 
                  "    threshold value of Y for which we might want to compute P(Y>thresholdY), with outputs_def='t'\n");

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
                  "      - data: from the data at regular quantiles, with last one at maxY (default)\n"
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

    declareOption(ol, "separate_mass_point", &ConditionalDensityNet::separate_mass_point, OptionBase::buildoption, 
                  "    whether to model separately the mass point at the origin.\n");

    declareOption(ol, "initial_hardness", &ConditionalDensityNet::initial_hardness, OptionBase::buildoption, 
                  "    value that scales softplus(c).\n");

    declareOption(ol, "c_penalization", &ConditionalDensityNet::c_penalization, OptionBase::buildoption, 
                  "    the penalization coefficient for the 'c' output of the neural network");

    declareOption(ol, "generate_precision", &ConditionalDensityNet::generate_precision, OptionBase::buildoption, 
                  "    precision when generating a new sample\n");

    declareOption(ol, "paramsvalues", &ConditionalDensityNet::paramsvalues, OptionBase::learntoption, 
                  "    The learned neural network parameter vector\n");

    declareOption(ol, "unconditional_cdf", &ConditionalDensityNet::unconditional_cdf, OptionBase::learntoption, 
                  "    Unconditional cumulative distribution function.\n");

    declareOption(ol, "unconditional_delta_cdf", &ConditionalDensityNet::unconditional_delta_cdf, OptionBase::learntoption, 
                  "    Variations of the cdf from one step center to the next (this is u_i in above eqns).\n");

    declareOption(ol, "mu", &ConditionalDensityNet::mu, OptionBase::learntoption, 
                  "    Step centers.\n");

    declareOption(ol, "y_values", &ConditionalDensityNet::y_values, OptionBase::learntoption, 
                  "    Values of Y at which the cumulative (or density or survival) curves are computed if required.\n");

    inherited::declareOptions(ol);
}

/*
  int ConditionalDensityNet::outputsize() const
  {
  int target_size = targetsize_<0?(train_set?train_set->targetsize():1):targetsize_;
  int l=0;
  for (unsigned int i=0;i<outputs_def.length();i++)
  if (outputs_def[i]=='L' || outputs_def[i]=='D' || outputs_def[i]=='C' || outputs_def[i]=='S')
  l+=n_curve_points;
  else if (outputs_def[i]=='e')
  l+=target_size;
  else if (outputs_def[i]=='v') // by default assume variance is full nxn matrix 
  l+=target_size*target_size;
  else l++;
  return l;
  }
*/

////////////
// build_ //
////////////
void ConditionalDensityNet::build_()
{
    if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
    {
        lower_bound = 0;
        upper_bound = maxY;
        int n_output_parameters = mu_is_fixed?(1+n_output_density_terms*2):(1+n_output_density_terms*3);

        if (n_curve_points<0)
            n_curve_points = n_output_density_terms+1;

        // init. basic vars
        input = Var(n_input, "input");
        output = input;
        params.resize(0);

        // first hidden layer
        if(nhidden>0)
        {
            w1 = Var(1+n_input, nhidden, "w1");      
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
            wdirect = Var(n_input, n_output_parameters, "wdirect");
            //wdirect = Var(1+inputsize(), n_output_parameters, "wdirect");
            output += transposeProduct(wdirect, input);// affine_transform(input,wdirect);
            params.append(wdirect);
        }

        /*
         * target and weights
         */

        target = Var(n_target, "target");

        if(weightsize_>0)
        {
            if (weightsize_!=1)
                PLERROR("ConditionalDensityNet: expected weightsize to be 1 or 0 (or unspecified = -1, meaning 0), got %d",weightsize_);
            sampleweight = Var(1, "weight");
        }
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

        // we don't want to clear mu if this build is called
        // just after a load(), because mu is a learnt option
        if (!mu || (mu->length()!=n_output_density_terms && train_set))
        {
            if (mu_is_fixed)
                //mu = Var(1,n_output_density_terms);
                mu = Var(n_output_density_terms,1);
            else
            {
                i+=n_output_density_terms;
                //mu = new SubMatVariable(output,0,i,1,n_output_density_terms);
                mu = new SubMatVariable(output,i,0,n_output_density_terms,1);
            }
        }
        mu->setName("mu");

        /*
         * output density
         */
        Var nll; // negative log likelihood
        Var max_y = var(maxY); 
        Var left_side = vconcat(var(0.0) & (new SubMatVariable(mu,0,0,n_output_density_terms-1,1)));
        centers = target-mu; 
        centers_M = max_y-mu;
        unconditional_cdf.resize(n_output_density_terms);
        if (unconditional_delta_cdf)
        {
            // don't clear it if this build is called just after a load
            if (unconditional_delta_cdf.length()!=n_output_density_terms)
                unconditional_delta_cdf->resize(n_output_density_terms,1);
        }
        else
            unconditional_delta_cdf = Var(n_output_density_terms,1);
        initial_hardnesses = var(initial_hardness) / (mu - left_side);
        pos_b = softplus(b)*unconditional_delta_cdf; 
        pos_c = softplus(c)*initial_hardnesses; 
        Var scaled_centers = pos_c*centers;
        // scaled centers evaluated at target = M
        Var scaled_centers_M = pos_c*centers_M;
        // scaled centers evaluated at target = 0
        Var scaled_centers_0 = -pos_c*mu;
        Var lhopital, inverse_denominator, density_numerator;
        if (separate_mass_point)
        {
            pos_a = sigmoid(a); 
            if (steps_type=="sigmoid_steps")
            {
                steps = sigmoid(scaled_centers); 
                // steps evaluated at target = M
                steps_M = sigmoid(scaled_centers_M);
                steps_0 = sigmoid(scaled_centers_0);
                // derivative of steps wrt target
                steps_gradient = pos_c*steps*(1-steps);
                steps_integral = (softplus(scaled_centers_M) - softplus(scaled_centers_0))/pos_c;
                delta_steps = centers_M*steps_M + mu*sigmoid(scaled_centers_0);
            }
            else if (steps_type=="sloped_steps")
            {
                steps = soft_slope(target, pos_c, left_side, mu);
                steps_M = soft_slope(max_y, pos_c, left_side, mu);
                steps_0 = soft_slope(var(0.0), pos_c, left_side, mu);
                steps_gradient = d_soft_slope(target, pos_c, left_side, mu);
                steps_integral = soft_slope_integral(pos_c,left_side,mu,0.0,maxY);
                delta_steps = soft_slope_limit(target, pos_c, left_side, mu);
            }
            else PLERROR("ConditionalDensityNet::build, steps_type option value unknown: %s",steps_type.c_str());

            density_numerator = dot(pos_b,steps_gradient);
            cum_denominator = dot(pos_b,positive(steps_M-steps_0));
            inverse_denominator = 1.0/cum_denominator;
            cum_numerator = dot(pos_b,(steps-steps_0));
            cumulative = pos_a + (1-pos_a) * cum_numerator * inverse_denominator;
            density = density_numerator * inverse_denominator; // this is the conditional density for Y>0
            // apply l'hopital rule if pos_c --> 0 to avoid blow-up (N.B. lim_{pos_c->0} pos_b/pos_c*steps_integral = pos_b*delta_steps)
            lhopital = ifThenElse(isAboveThreshold(pos_c,1e-20),steps_integral,delta_steps); 
            expected_value = max_y - ((pos_a-(1-pos_a)*inverse_denominator*dot(pos_b,steps_0))*max_y + 
                                      (1-pos_a)*dot(pos_b,lhopital)*inverse_denominator);
            mass_cost = -log(ifThenElse(isAboveThreshold(target,0.0,1,0,true),(1-pos_a),pos_a)); 
            pos_y_cost = ifThenElse(isAboveThreshold(target,0.0,1,0,true),-log(density),var(0.0)); 
            nll = -log(ifThenElse(isAboveThreshold(target,0.0,1,0,true),density*(1-pos_a),pos_a));
        }
        else
        {
            pos_a = var(0.0);
            if (steps_type=="sigmoid_steps")
            {
                steps = sigmoid(scaled_centers); 
                // steps evaluated at target = M
                steps_M = sigmoid(scaled_centers_M);
                steps_0 = sigmoid(scaled_centers_0);
                // derivative of steps wrt target
                steps_gradient = pos_c*steps*(1-steps);
                steps_integral = (softplus(scaled_centers_M) - softplus(scaled_centers_0))/pos_c;
                delta_steps = centers_M*steps_M + mu*sigmoid(scaled_centers_0);
            }
            else if (steps_type=="sloped_steps")
            {
                steps = soft_slope(target, pos_c, left_side, mu);
                steps_M = soft_slope(max_y, pos_c, left_side, mu);
                steps_0 = soft_slope(var(0.0), pos_c, left_side, mu);
                steps_gradient = d_soft_slope(target, pos_c, left_side, mu);
                steps_integral = soft_slope_integral(pos_c,left_side,mu,0.0,maxY);
                delta_steps = soft_slope_limit(target, pos_c, left_side, mu);
            }
            else PLERROR("ConditionalDensityNet::build, steps_type option value unknown: %s",steps_type.c_str());

            density_numerator = dot(pos_b,steps_gradient);
            cum_denominator = dot(pos_b,steps_M - steps_0); 
            inverse_denominator = 1.0/cum_denominator;
            cum_numerator = dot(pos_b,steps - steps_0);
            cumulative = cum_numerator * inverse_denominator;
            density = density_numerator * inverse_denominator;
            // apply l'hopital rule if pos_c --> 0 to avoid blow-up (N.B. lim_{pos_c->0} pos_b/pos_c*steps_integral = pos_b*delta_steps)
            lhopital = ifThenElse(isAboveThreshold(pos_c,1e-20),steps_integral,delta_steps); 
            expected_value = dot(pos_b,lhopital)*inverse_denominator;
            nll = -log(ifThenElse(isAboveThreshold(target,0.0,1,0,true),density,cumulative));
        }
        max_y->setName("maxY");
        left_side->setName("left_side"); 
        pos_a->setName("pos_a");
        pos_b->setName("pos_b");
        pos_c->setName("pos_c");
        steps->setName("steps");
        steps_M->setName("steps_M");
        steps_integral->setName("steps_integral");
        expected_value->setName("expected_value");
        density_numerator->setName("density_numerator");
        cum_denominator->setName("cum_denominator");
        inverse_denominator->setName("inverse_denominator");
        cum_numerator->setName("cum_numerator");
        cumulative->setName("cumulative");
        density->setName("density");
        lhopital->setName("lhopital");
    
        /*
         * cost functions:
         *   training_criterion = log_likelihood_vs_squared_error_balance*neg_log_lik
         *                      +(1-log_likelihood_vs_squared_error_balance)*squared_err
         *                      +penalties
         *   neg_log_lik = -log(1_{target=0} cumulative + 1_{target>0} density)
         *   squared_err = square(target - expected_value)
         */
        costs.resize(3);
      
        costs[1] = nll;
        costs[2] = square(target-expected_value);
        // for debugging gradient computation error
        if (fast_exact_is_equal(log_likelihood_vs_squared_error_balance, 1))
            costs[0] = costs[1];
        else if (fast_exact_is_equal(log_likelihood_vs_squared_error_balance, 0))
            costs[0] = costs[2];
        else costs[0] = log_likelihood_vs_squared_error_balance*costs[1]+
                 (1-log_likelihood_vs_squared_error_balance)*costs[2];
        if (c_penalization > 0) {
            costs[0] = costs[0] + c_penalization * sumsquare(c);
        }
    
        // for debugging
        //costs[0] = mass_cost + pos_y_cost;
        //costs[1] = mass_cost;
        //costs[2] = pos_y_cost;

        /*
         * weight and bias decay penalty
         */
        if( L1_penalty )
        {
            PLDEPRECATED("Option \"L1_penalty\" deprecated. Please use \"penalty_type = L1\" instead.");
            L1_penalty = 0;
            penalty_type = "L1";
        }

        string pt = lowerstring( penalty_type );
        if( pt == "l1" )
            penalty_type = "L1";
        else if( pt == "l1_square" || pt == "l1 square" || pt == "l1square" )
            penalty_type = "L1_square";
        else if( pt == "l2_square" || pt == "l2 square" || pt == "l2square" )
            penalty_type = "L2_square";
        else if( pt == "l2" )
        {
            PLWARNING("L2 penalty not supported, assuming you want L2 square");
            penalty_type = "L2_square";
        }
        else
            PLERROR("penalty_type \"%s\" not supported", penalty_type.c_str());

        // create penalties
        penalties.resize(0);  // prevents penalties from being added twice by consecutive builds
        if(w1 && (!fast_exact_is_equal(layer1_weight_decay + weight_decay, 0) ||
                  !fast_exact_is_equal(layer1_bias_decay   + bias_decay,   0)))
            penalties.append(affine_transform_weight_penalty(w1, (layer1_weight_decay + weight_decay), (layer1_bias_decay + bias_decay), penalty_type));
        if(w2 && (!fast_exact_is_equal(layer2_weight_decay + weight_decay, 0) ||
                  !fast_exact_is_equal(layer2_bias_decay   + bias_decay,   0)))
            penalties.append(affine_transform_weight_penalty(w2, (layer2_weight_decay + weight_decay), (layer2_bias_decay + bias_decay), penalty_type));
        if(wout && (!fast_exact_is_equal(output_layer_weight_decay + weight_decay, 0) ||
                    !fast_exact_is_equal(output_layer_bias_decay   + bias_decay,   0)))
            penalties.append(affine_transform_weight_penalty(wout, (output_layer_weight_decay + weight_decay), 
                                                             (output_layer_bias_decay + bias_decay), penalty_type));
        if(wdirect && !fast_exact_is_equal(direct_in_to_out_weight_decay + weight_decay, 0))
        {
            if (penalty_type == "L1_square")
                penalties.append(square(sumabs(wdirect))*(direct_in_to_out_weight_decay + weight_decay));
            else if (penalty_type == "L1")
                penalties.append(sumabs(wdirect)*(direct_in_to_out_weight_decay + weight_decay));
            else if (penalty_type == "L2_square")
                penalties.append(sumsquare(wdirect)*(direct_in_to_out_weight_decay + weight_decay));
        }

        test_costs = hconcat(costs);

        // apply penalty to cost
        if(penalties.size() != 0) {
            // only multiply by sampleweight if there are weights
            if (weightsize_>0)
                training_cost = hconcat(sampleweight*sum(hconcat(costs[0] & penalties))
                                        & (test_costs*sampleweight));
            else {
                training_cost = hconcat(sum(hconcat(costs[0] & penalties)) & test_costs);
            }
        }
        else {
            // only multiply by sampleweight if there are weights
            if(weightsize_>0) {
                training_cost = test_costs*sampleweight;
            } else {
                training_cost = test_costs;
            }
        }
  
        training_cost->setName("training_cost");
        test_costs->setName("test_costs");
        output->setName("output");
      
        // Shared values hack...
        bool use_paramsvalues=(bool)paramsvalues && (paramsvalues.size() == params.nelems());
        if(use_paramsvalues)
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

        VarArray output_and_target = output & target;
        output_and_target_values.resize(output.length()+target.length());
        output_and_target.makeSharedValue(output_and_target_values);

        cdf_f = Func(output_and_target,cumulative);
        mean_f = Func(output,expected_value);
        density_f = Func(output_and_target,density);
  
        // Funcs
        VarArray outvars;
        VarArray testinvars;
        invars.resize(0);
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

        for (unsigned int k=0;k<outputs_def.length();k++)
        {
            if (outputs_def[k]=='e')
                outputs_array &= expected_value;
            else if (outputs_def[k]=='t')
            {
                Func survival_f(target&output,var(1.0)-cumulative);
                Var threshold_y(1,1);
                threshold_y->valuedata[0]=thresholdY;
                outputs_array &= survival_f(threshold_y & output);
            }
            else if (outputs_def[k]=='S' || outputs_def[k]=='C' ||
                     outputs_def[k]=='L' || outputs_def[k]=='D')
            {
                Func prob_f(target&output,outputs_def[k]=='S'?(var(1.0)-cumulative):
                            (outputs_def[k]=='C'?cumulative:
                             (outputs_def[k]=='D'?density:log(density))));
                y_values.resize(n_curve_points);
                if (curve_positions=="uniform")
                {
                    real delta = maxY/(n_curve_points-1);
                    for (int j=0;j<n_curve_points;j++)
                    {
                        y_values[j] = var(j*delta);
                        y_values[j]->setName("y"+tostring(j));
                        outputs_array &= prob_f(y_values[j] & output);
                    }
                } else // log-scale
                {
                    real denom = 1.0/(1-exp(-scale));
                    for (int j=0;j<n_curve_points;j++)
                    {
                        y_values[j] = var((exp(scale*(j-n_output_density_terms)/n_output_density_terms)-exp(-scale))*denom);
                        y_values[j]->setName("y"+tostring(j));
                        outputs_array &= prob_f(y_values[j] & output);
                    }    
                }
            } else
                outputs_array &= expected_value;
//          PLERROR("ConditionalDensityNet::build: can't handle outputs_def with option value = %c",outputs_def[k]);
        }
        outputs = hconcat(outputs_array);
        if (mu_is_fixed)
            f = Func(input, params&mu, outputs);
        else
            f = Func(input, params, outputs);
        f->recomputeParents();

        in2distr_f = Func(input,pos_a);
        in2distr_f->recomputeParents();

        if (mu_is_fixed)
            test_costf = Func(testinvars, params&mu, outputs&test_costs);
        else
            test_costf = Func(testinvars, params, outputs&test_costs);

        if (use_paramsvalues)
            test_costf->recomputeParents();
    }
    // PDistribution::finishConditionalBuild();
}


/* TODO Remove (?)
   void ConditionalDensityNet::computeOutput(const Vec& inputv, Vec& outputv) const
   {
   f->fprop(inputv,outputv);
   }
*/

/* TODO Remove (?)
   void ConditionalDensityNet::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
   Vec& outputv, Vec& costsv) const
   {
   test_costf->fprop(inputv&targetv, outputv&costsv);
   }
*/

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

/*
  TVec<string> ConditionalDensityNet::getTestCostNames() const
  { 
  TVec<string> cost_funcs(3);
  cost_funcs[0]="training_criterion";
  cost_funcs[1]="NLL";
  cost_funcs[2]="mse";
  return cost_funcs;
  }
*/

// ### Nothing to add here, simply calls build_
void ConditionalDensityNet::build()
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


void ConditionalDensityNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    varDeepCopyField(input, copies);
    varDeepCopyField(target, copies);
    varDeepCopyField(sampleweight, copies);
    varDeepCopyField(w1, copies);
    varDeepCopyField(w2, copies);
    varDeepCopyField(wout, copies);
    varDeepCopyField(wdirect, copies);
    varDeepCopyField(output, copies);
    varDeepCopyField(outputs, copies);
    varDeepCopyField(a, copies);
    varDeepCopyField(pos_a, copies);
    varDeepCopyField(b, copies);
    varDeepCopyField(pos_b, copies);
    varDeepCopyField(c, copies);
    varDeepCopyField(pos_c, copies);
    varDeepCopyField(density, copies);
    varDeepCopyField(cumulative, copies);
    varDeepCopyField(expected_value, copies);
    deepCopyField(costs, copies);
    deepCopyField(penalties, copies);
    varDeepCopyField(training_cost, copies);
    varDeepCopyField(test_costs, copies);
    deepCopyField(invars, copies);
    deepCopyField(params, copies);
    deepCopyField(paramsvalues, copies);
    varDeepCopyField(centers, copies);
    varDeepCopyField(centers_M, copies);
    varDeepCopyField(steps, copies);
    varDeepCopyField(steps_M, copies);
    varDeepCopyField(steps_0, copies);
    varDeepCopyField(steps_gradient, copies);
    varDeepCopyField(steps_integral, copies);
    varDeepCopyField(delta_steps, copies);
    varDeepCopyField(cum_numerator, copies);
    varDeepCopyField(cum_denominator, copies);
    deepCopyField(unconditional_cdf, copies);
    varDeepCopyField(unconditional_delta_cdf, copies);
    varDeepCopyField(initial_hardnesses, copies);
    varDeepCopyField(prev_centers, copies);
    varDeepCopyField(prev_centers_M, copies);
    varDeepCopyField(scaled_prev_centers, copies);
    varDeepCopyField(scaled_prev_centers_M, copies);
    varDeepCopyField(minus_prev_centers_0, copies);
    varDeepCopyField(minus_scaled_prev_centers_0, copies);
    deepCopyField(y_values, copies);
    varDeepCopyField(mu, copies);
    deepCopyField(f, copies);
    deepCopyField(test_costf, copies);
    deepCopyField(output_and_target_to_cost, copies);
    deepCopyField(cdf_f, copies);
    deepCopyField(mean_f, copies);
    deepCopyField(density_f, copies);
    deepCopyField(in2distr_f, copies);
    deepCopyField(output_and_target, copies);
    deepCopyField(output_and_target_values, copies);
    varDeepCopyField(totalcost, copies);
    varDeepCopyField(mass_cost, copies);
    varDeepCopyField(pos_y_cost, copies);
    deepCopyField(optimizer, copies);
}

void ConditionalDensityNet::setInput(const Vec& in) const
{
#ifdef BOUNDCHECK
    if (!f)
        PLERROR("ConditionalDensityNet:setInput: build was not completed (maybe because training set was not provided)!");
#endif
    in2distr_f->fprop(in,pos_a->value);
}

real ConditionalDensityNet::log_density(const Vec& y) const
{ 
    static Vec d;
    d.resize(1);
    target->value << y;
    density_f->fprop(output_and_target_values, d);
    return pl_log(d[0]);
}

real ConditionalDensityNet::survival_fn(const Vec& y) const
{ 
    return 1 - cdf(y);
}

// must be called after setInput
real ConditionalDensityNet::cdf(const Vec& y) const
{ 
    Vec cum(1);
    target->value << y;
    cdf_f->fprop(output_and_target_values,cum);
#ifdef BOUNDCHECK
    if (cum[0] < -1e-3)
        PLERROR("In ConditionalDensityNet::cdf - The cdf is < 0");
#endif
    return cum[0];
}

void ConditionalDensityNet::expectation(Vec& mu) const
{ 
    mu.resize(n_target);
    mean_f->fprop(output->value,mu);
}

void ConditionalDensityNet::variance(Mat& covar) const
{ 
    PLERROR("variance not implemented for ConditionalDensityNet"); 
}

void ConditionalDensityNet::resetGenerator(long g_seed)
{ 
    manual_seed(g_seed);
}

void ConditionalDensityNet::generate(Vec& y) const
{ 
    real u = uniform_sample();
    y.resize(1);
    if (u<pos_a->value[0]) // mass point
    {
        y[0]=0;
        return;
    }
    // then find y s.t. P(Y<y|x) = u by binary search
    real y0=0;
    real y2=maxY;
    real delta;
    real p;
    do 
    {
        delta = y2 - y0;
        y[0] = y0 + delta*0.5;
        p = cdf(y);
        if (p<u)
            // increase y
            y0 = y[0];
        else
            // decrease y
            y2 = y[0];
    }
    while (delta > generate_precision * maxY);
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
    real delta = 1.0 / n_input;
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
        delta = 1.0/nhidden;
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
    // Mat a_weights = wout->matValue.column(0); // Does not seem to be used anymore.
    // a_weights *= 3.0; // to get more dynamic range

    if (centers_initialization!="data")
    {
        Vec output_biases = wout->matValue(0);
        Vec mu_;
        int i=0;
        Vec a_ = output_biases.subVec(i++,1);
        Vec b_ = output_biases.subVec(i,n_output_density_terms); i+=n_output_density_terms;
        Vec c_ = output_biases.subVec(i,n_output_density_terms); i+=n_output_density_terms;
        if (mu_is_fixed)
            mu_ = mu->value;
        else
            mu_ = output_biases.subVec(i,n_output_density_terms); i+=n_output_density_terms;
        initialize_mu(mu_);
        b_.fill(inverse_softplus(1.0));
        c_.fill(inverse_softplus(1.0));
        if (separate_mass_point)
            a_[0] = unconditional_p0>0?inverse_sigmoid(unconditional_p0):-50;
        else a_[0] = -50;
        unconditional_delta_cdf->value.fill((1.0-unconditional_p0)/n_output_density_terms);
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
    } else if (centers_initialization!="data")
        PLERROR("ConditionalDensityNet::initialize_mu: unknown value %s for centers_initialization option",
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
    int i=0, j=0;
    if(!train_set)
        PLERROR("In ConditionalDensityNet::train, you did not setTrainingSet");
    
    if(!train_stats)
        PLERROR("In ConditionalDensityNet::train, you did not setTrainStatsCollector");

    /*
    if (!already_sorted || n_margin > 0)
        PLERROR("In ConditionalDensityNet::train - Currently, can only be trained if the data is given as input, target");
        */

    if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
        build();

    int l = train_set->length();
    int nsamples = batch_size>0 ? batch_size : l;
    Func paramf = Func(invars, training_cost); // parameterized function to optimize
    Var totalcost = meanOf(train_set, paramf, nsamples);
    if(optimizer)
    {
        optimizer->setToOptimize(params, totalcost);  
        optimizer->build();
    }

    // number of optimiser stages corresponding to one learner stage (one epoch)
    int optstage_per_lstage = l/nsamples;

    ProgressBar* pb = 0;
    if(report_progress)
        pb = new ProgressBar("Training ConditionalDensityNet from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

    // estimate the unconditional cdf
    static real weight;

    if (stage==0)
    {
        Vec mu_values = mu->value;
        unconditional_cdf.clear();
        real sum_w=0;
        unconditional_p0 = 0;
        static StatsCollector sc;
        bool init_mu_from_data=centers_initialization=="data";
        if (init_mu_from_data)
        {
            sc.maxnvalues = min(l,100*n_output_density_terms);
            sc.build();
            sc.forget();
        }
        Vec tmp1(inputsize());
        Vec tmp2(targetsize());
        for (i=0;i<l;i++)
        {
            train_set->getExample(i, tmp1, tmp2, weight);
            input->value << tmp1.subVec(0, n_input);
            target->value << tmp1.subVec(n_input, n_target);
            real y = target->valuedata[0];
            if (y < 0)
                PLERROR("In ConditionalDensityNet::train - Found a negative target");
            if (y > maxY)
                PLERROR("In ConditionalDensityNet::train - Found a target > maxY");
            if (fast_exact_is_equal(y, 0))
                unconditional_p0 += weight;
            if (init_mu_from_data)
                sc.update(y,weight);
            else
                for (int k=0;k<n_output_density_terms;k++)
                    if (y<=mu_values[k]) 
                        unconditional_cdf[k] += weight;
            sum_w += weight;
        }
        static Mat cdf;
        unconditional_p0 *= 1.0/sum_w;
        if (init_mu_from_data)
        {
            cdf = sc.cdf();
            int k=3;
            real mean_y = sc.mean();

            real current_mean_fraction = 0;
            real prev_cdf = unconditional_p0;
            real prev_y = 0;
            for (int q=0;q<n_output_density_terms;q++)
            {
                real target_fraction = mean_y*(q+1.0)/n_output_density_terms;
                for (;k<cdf.length() && current_mean_fraction < target_fraction;k++)
                {
                    current_mean_fraction += (cdf(k,0)+prev_y)*0.5*(cdf(k,1)-prev_cdf);
                    prev_cdf = cdf(k,1);
                    prev_y = cdf(k,0);
                }
                if (q==n_output_density_terms-1)
                {
                    mu_values[q]=maxY;
                    unconditional_cdf[q]=1.0;
                }
                else
                {
                    mu_values[q]=cdf(k,0);
                    unconditional_cdf[q]=cdf(k,1);
                }
            }
        }
        else
            for (j=0;j<n_output_density_terms;j++)
                unconditional_cdf[j] *= 1.0/sum_w;

        unconditional_delta_cdf->valuedata[0]=unconditional_cdf[0]-unconditional_p0;
        for (i=1;i<n_output_density_terms;i++)
            unconditional_delta_cdf->valuedata[i]=unconditional_cdf[i]-unconditional_cdf[i-1];

        // initialize biases based on unconditional distribution
        Vec output_biases = wout->matValue(0);
        i=0;
        Vec a_ = output_biases.subVec(i++,1);
        Vec b_ = output_biases.subVec(i,n_output_density_terms); i+=n_output_density_terms;
        Vec c_ = output_biases.subVec(i,n_output_density_terms); i+=n_output_density_terms;
        Vec mu_;
        Vec s_c(n_output_density_terms);
        if (mu_is_fixed)
            mu_ = mu->value;
        else
            mu_ = output_biases.subVec(i,n_output_density_terms); i+=n_output_density_terms;
        b_.fill(inverse_softplus(1.0));
        initialize_mu(mu_);
        for (i=0;i<n_output_density_terms;i++)
        {
            real prev_mu = i==0?0:mu_[i-1];
            real delta = mu_[i]-prev_mu;
            s_c[i] = delta>0?initial_hardness/delta:-50;
            c_[i] = inverse_softplus(1.0);
        }

        if (centers_initialization!="data")
            unconditional_delta_cdf->value.fill(1.0/n_output_density_terms);
        real *dcdf = unconditional_delta_cdf->valuedata;
        if (separate_mass_point)
            a_[0] = unconditional_p0>0?inverse_sigmoid(unconditional_p0):-50;
        else if (fast_exact_is_equal(dcdf[0], 0))
            a_[0]=unconditional_p0>0?inverse_softplus(unconditional_p0):-50;
        else
        {
            real s=0;
            if (steps_type=="sigmoid_steps")
                for (i=0;i<n_output_density_terms;i++)
                    s+=dcdf[i]*(unconditional_p0*sigmoid(s_c[i]*(maxY-mu_[i]))-sigmoid(-s_c[i]*mu_[i]));
            else
                for (i=0;i<n_output_density_terms;i++)
                {
                    real prev_mu = i==0?0:mu_[i-1];
                    real ss1 = soft_slope(maxY,s_c[i],prev_mu,mu_[i]);
                    real ss2 = soft_slope(0,s_c[i],prev_mu,mu_[i]);
                    s+=dcdf[i]*(unconditional_p0*ss1 - ss2);
                }
            real sa=s/(1-unconditional_p0);
            a_[0]=sa>0?inverse_softplus(sa):-50;

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
        test_costf->recomputeParents();

        // debugging
        static bool display_graph = false;
        if (display_graph) f->fprop(input->value,outputs->value);
        //displayVarGraph(outputs,true);
        if (display_graph)
            displayFunction(f,true);
        if (display_graph)
            displayFunction(test_costf,true);
    }
    int initial_stage = stage;
    bool early_stop=false;
    while(stage<nstages && !early_stop)
    {
        optimizer->nstages = optstage_per_lstage;
        train_stats->forget();
        optimizer->early_stop = false;
        early_stop = optimizer->optimizeN(*train_stats);

        //if (verify_gradient)
        //  training_cost->verifyGradient(verify_gradient);
        //if (stage==nstages-1 && verify_gradient)
        static bool verify_gradient = false;
        if (verify_gradient)
        {
            if (batch_size == 0)
            {
                cout << "OPTIMIZER" << endl;
                optimizer->verifyGradient(0.001);
            }
        }
        static bool display_graph = false;
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

} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
