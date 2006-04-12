// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 2005 Yoshua Bengio

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
 * This file is part of the PLearn library.
 ******************************************************* */

//#include "ProductTransposeVariable.h"
//#include "ProductVariable.h"
//#include "TransposeProductVariable.h"
#include "FNetLayerVariable.h"
#include <plearn/math/random.h>
#include <plearn/math/TMat_maths.h>
#include <plearn/math/TMat_maths_specialisation.h>

namespace PLearn {
using namespace std;


/** FNetLayerVariable **/

// Single layer of a neural network, with acceleration tricks

PLEARN_IMPLEMENT_OBJECT(FNetLayerVariable,
                        "Single layer of a neural network, with acceleration tricks",
                        "This variable takes four inputs:\n"
                        "(1) the input of the layer (minibatch_size x n_inputs) matrix, and\n"
                        "(2) the weights matrix (n_hidden x n_inputs)\n"
                        "(3) the bias vector (n_hidden) b\n"
                        "(4) a 2-element parameter vector c = (c1,c2) which is used when inhibition is active.\n"
                        "For each row vector x[k] of the input matrix, it computes the following\n"
                        "output row vector y[k] in the fprop function:\n"
                        "  y[k,i] = sigmoid(a[k,i])\n"
                        "where\n"
                        "  a[k,i] = dot(W[i],u[k,i]) + b[i] - 1_{inhibit_next_units} c1*sigmoid(c2* avg_{j<i} y[k,j])\n"
                        "where u[k,i]= col. vector from optionally normalizing the x[k] vector:\n"
                        "  u[k,i] = (x[k] - mu[i])*invs[i]\n"
                        "and the free parameters and the W's, the b's, and c1 and c2.\n"
                        "The negative sum over j<i is optional and should help the units of the layer\n"
                        "to differentiate, since when one is active (y[k,j] close to 1), it inhibits\n"
                        "the units that follow it (y[k,i], with i>j). The normalization parameters\n"
                        "mu[i] and invs[i] are estimated by an exponential moving average of the\n"
                        "inputs x[k] for which |dC/da[k,i]| was above a threshold, described below.\n"
                        "The exponential moving average is with the option value\n"
                        "exp_moving_average_coefficient, also used to compute the threshold.\n"
                        "The moving averages are only updated during the bprop phase, i.e. only\n"
                        "during training.\n"
                        "\n"
                        "In the bprop phase, unlike with other Variable classes, this class can\n"
                        "optionally compute a pseudo-gradient which is not the actual gradient.\n"
                        "The pseudo-gradient is obtained by zeroing the gradient on some of the\n"
                        "a[k,i] terms before continuing the gradient propagation to the input and\n"
                        "weight matrices. The gradient on a[k,i] is zeroed if its absolute value is\n"
                        "below a threshold gradient_threshold, that is adapted to represent\n"
                        "approximately the fraction average_error_fraction_to_threshold\n"
                        "of the exponential moving average of the |dC/da[k,i]| over k,i, and\n"
                        "past examples, with the exponential moving average being done with\n"
                        "the value of the exp_moving_average_coefficient option.\n"
    );

FNetLayerVariable::FNetLayerVariable()
    : c1_(0),
      c2_(0),
      n_inputs(-1), // MUST BE SPECIFIED BY THE USER
      n_hidden(-1), // MUST BE SPECIFIED BY THE USER
      minibatch_size(1),
      inhibit_next_units(true),
      inhibit_by_sum(false),
      squashed_inhibition(true),
      normalize_inputs(true),
      backprop_to_inputs(false),
      exp_moving_average_coefficient(0.001),
      average_error_fraction_to_threshold(0.5),
      min_stddev(1e-2)
{
    avg_act_gradient = -1;
}

FNetLayerVariable::FNetLayerVariable(Var inputs,  // x
                                     Var weights,  // W
                                     Var biases, // b
                                     Var inhibition_weights, // c
                                     bool _inhibit_next_units,
                                     bool _normalize_inputs,
                                     bool _backprop_to_inputs,
                                     real _exp_moving_average_coefficient,
                                     real _average_error_fraction_to_threshold)
    : inherited(inputs & weights &
                biases & inhibition_weights,
                inputs->length(), weights->length()),
      c1_(0),
      c2_(0),
      n_inputs(inputs->matValue.width()),
      n_hidden(weights->matValue.length()),
      minibatch_size(inputs->matValue.length()),
      inhibit_next_units(_inhibit_next_units),
      inhibit_by_sum(false),
      squashed_inhibition(true),
      normalize_inputs(_normalize_inputs),
      backprop_to_inputs(_backprop_to_inputs),
      exp_moving_average_coefficient(_exp_moving_average_coefficient),
      average_error_fraction_to_threshold(_average_error_fraction_to_threshold),
      min_stddev(1e-2)
{
    avg_act_gradient = -1;
    build_();
}

void
FNetLayerVariable::build()
{
    inherited::build();
    build_();
}

void
FNetLayerVariable::build_()
{
    if (varray.length() == 0 && n_inputs == -1)
        // Cannot do anything yet.
        return;
    if (   varray.size() != 4
           || n_hidden      != varray[1].length()
           || n_inputs      != varray[1].width()  )
    {
        varray.resize(4);
        if (varray[0])
            n_inputs = varray[0]->width();    // Get n_inputs from first var if present.
        varray[1] = Var(n_hidden,n_inputs);
        varray[2] = Var(n_hidden);
        varray[3] = Var(2);
    }
    if (varray[0]) {
        if (n_inputs != varray[0]->width())
            PLERROR("In FNetLayerVariable: input var 0 should have width = %d = n_inputs, but is %d\n",n_inputs, varray[0]->width());
        if (n_hidden != varray[1]->length())
            PLERROR("In FNetLayerVariable: input var 1 should have length = %d = n_hidden, but is %d\n",n_hidden, varray[1]->length());
        if (minibatch_size != varray[0]->length())
            PLERROR("In FNetLayerVariable: input var 0 should have length = %d = minibatch_size, but is %d\n",minibatch_size, varray[0]->length());
        if (n_inputs != varray[1]->width())
            PLERROR("In FNetLayerVariable: the size of inputs and weights are not compatible for an affine application of weights on inputs");
        if (varray[2]->size() != n_hidden)
            PLERROR("In FNetLayerVariable: the biases vector should have the same length as the weights matrix number of rows.");
        if (normalize_inputs && (mu.length() != n_hidden || mu.width() != n_inputs)) {
            mu.resize(n_hidden, n_inputs);
            mu.clear();
            invs.resize(n_hidden, n_inputs);
            invs.fill(1.0);
            mu2.resize(n_hidden, n_inputs);
            mu2.fill(0);
        } else
            // TODO Remove later, this is just a safety check.
            PLWARNING("In FNetLayerVariable::build_ - Using previously saved normalization parameters");
        inh.resize(minibatch_size, n_hidden);
        cum_inh.resize(minibatch_size, n_hidden);
        u.resize(minibatch_size);
        if (normalize_inputs)
            for (int i=0;i<minibatch_size;i++)
                u[i].resize(n_hidden,n_inputs);
        no_bprop_has_been_done = true;
        gradient_threshold = 0;
        if (avg_act_gradient < 0)
            avg_act_gradient = 0.0;
        // Initialize parameters.
        real delta = real(1.0 / n_inputs);
        fill_random_uniform(varray[1]->matValue, -delta, delta);
        varray[2]->matValue.fill(0.0);
        varray[3]->matValue.fill(1.0);
        if (!fast_exact_is_equal(c1_, 0))
            varray[3]->value[0] = c1_;
        if (!fast_exact_is_equal(c2_, 0))
            varray[3]->value[1] = c2_;
        // Set correct sizes.
        resize(minibatch_size, n_hidden);
    }
}

////////////////////
// declareOptions //
////////////////////
void FNetLayerVariable::declareOptions(OptionList& ol)
{
    declareOption(ol, "n_inputs", &FNetLayerVariable::n_inputs, OptionBase::buildoption, 
                  "    Number of inputs of the layer, for each element of the mini-batch.\n");

    declareOption(ol, "n_hidden", &FNetLayerVariable::n_hidden, OptionBase::buildoption, 
                  "    Number of outputs of the layer (hidden units), for each element of the mini-batch.\n");

    declareOption(ol, "minibatch_size", &FNetLayerVariable::minibatch_size, OptionBase::buildoption, 
                  "    Number of elements of each mini-batch.\n");

    declareOption(ol, "inhibit_next_units", &FNetLayerVariable::inhibit_next_units, OptionBase::buildoption, 
                  "    If true then activation of unit i contains minus the sum of the outputs of\n"
                  "    all units j for j<i, i.e. y[k,i] = sigmoid(W (u[k,i] 1) - 1_{inhibit_next_units} sum_{j<i} y[k,j]).\n");

    declareOption(ol, "inhibit_by_sum", &FNetLayerVariable::inhibit_by_sum, OptionBase::buildoption, 
                  "    If true, then the inhibition will be based on the sum of the previous units'\n"
                  "    activations, instead of their average.");

    declareOption(ol, "squashed_inhibition", &FNetLayerVariable::squashed_inhibition, OptionBase::buildoption, 
                  "    If true, then the inhibition will be squashed by a sigmoid (if false, c2 is not used).");
      
    declareOption(ol, "normalize_inputs", &FNetLayerVariable::normalize_inputs, OptionBase::buildoption, 
                  "    If true, then normalized input u[k,i]=(x[k] - mu[i])*invs[i], otherwise u[k,i]=x[k].\n"
                  "    mu[i,j] is a moving average of the x[k,j]'s when |dC/da[k,i]| is above gradient_threshold.\n"
                  "    Similarly, mu2[i,j] is a moving average of x[k,j]*x[k,j] when |dC/da[k,i]| is above gradient_threshold\n"
                  "    and invs[i,j] = 1/sqrt(mu2[i,j] - mu[i,j]*mu[i,j]). The moving averages are exponential moving\n"
                  "    averages with coefficient exp_moving_average_coefficient.\n");

    declareOption(ol, "min_stddev", &FNetLayerVariable::min_stddev, OptionBase::buildoption, 
                  "Used only when 'normalize_inputs' is true, any input whose standard deviation is less than this value\n"
                  "will be considered as having this standard deviation (prevents numerical problems with constant inputs).");

    declareOption(ol, "backprop_to_inputs", &FNetLayerVariable::backprop_to_inputs, OptionBase::buildoption, 
                  "    If true then gradient is propagated to the inputs. When this object is the first layer\n"
                  "    of a neural network, it is more efficient to set this option to false (which is its default).\n");

    declareOption(ol, "exp_moving_average_coefficient", &FNetLayerVariable::exp_moving_average_coefficient, OptionBase::buildoption, 
                  "    The moving average coefficient used in updating mu, var and gradient_threshold, with\n"
                  "    updates of the form\n"
                  "       newvalue = (1 - exp_moving_average_coefficient)*oldvalue + exp_moving_average_coefficient*summand\n"
                  "    in order to obtain a moving average of the summands.\n");

    declareOption(ol, "average_error_fraction_to_threshold", &FNetLayerVariable::average_error_fraction_to_threshold, 
                  OptionBase::buildoption, 
                  "    The fraction of the average of |dC/da[k,i]| that determines the gradient_threshold.\n");

    declareOption(ol, "c1", &FNetLayerVariable::c1_, OptionBase::buildoption, 
                  "    Fixed coefficient c1. '0' means it will be optimized, starting from 1.\n");

    declareOption(ol, "c2", &FNetLayerVariable::c2_, OptionBase::buildoption, 
                  "    Fixed coefficient c2. '0' means it will be optimized, starting from 1.\n");

    // Learnt options.

    declareOption(ol, "avg_act_gradient", &FNetLayerVariable::avg_act_gradient, OptionBase::learntoption, 
                  "The exponential moving average of the absolute value of the gradient.");

    declareOption(ol, "mu", &FNetLayerVariable::mu, OptionBase::learntoption, 
                  "The centers for normalization.");

    declareOption(ol, "mu2", &FNetLayerVariable::mu, OptionBase::learntoption, 
                  "The squared centers for computation of the variance.");

    declareOption(ol, "mu2", &FNetLayerVariable::mu, OptionBase::learntoption, 
                  "The normalization factors.");

    inherited::declareOptions(ol);
}


void FNetLayerVariable::recomputeSize(int& l, int& w) const
{
    if (varray.length() >= 2 && varray[0] && varray[1]) {
        l = varray[0]->length();
        w = varray[1]->length();
    } else
        l = w = 0;
}

void FNetLayerVariable::fprop()
{
    real* x = varray[0]->valuedata;
    real* y = valuedata;
    real* b = varray[2]->valuedata;
    real c1 = varray[3]->valuedata[0];
    real c2 = varray[3]->valuedata[1];
    int mx=varray[0]->matValue.mod();
    int my=matValue.mod();
    for (int k=0;k<minibatch_size;k++, x+=mx, y+=my)
    {
        real cum_s = 0;
        Mat u_k = u[k];
        real* inh_k = inh[k];
        real* cum_inh_k = cum_inh[k];
        for (int i=0;i<n_hidden;i++)
        {
            real* Wi = varray[1]->matValue[i];
            real bi = b[i];
            if (inhibit_next_units && i>0)
            {
                if (inhibit_by_sum)
                    cum_inh_k[i] = cum_s;
                else
                    cum_inh_k[i] = cum_s / real(i);
                if (squashed_inhibition)
                    inh_k[i] = sigmoid(c2 * cum_inh_k[i]);
                else
                    inh_k[i] = cum_inh_k[i];
                bi -= c1*inh_k[i];
            }
            if (normalize_inputs)
            {
                real* mu_i = mu[i];
                real* invs_i = invs[i];
                real* u_ki = u_k[i];
                for (int j=0;j<n_inputs;j++)
                    u_ki[j] = (x[j] - mu_i[j])*invs_i[j];
                y[i] = sigmoid(dot_product(bi,u_ki,Wi,n_inputs));
            }
            else
                y[i] = sigmoid(dot_product(bi,x,Wi,n_inputs));
            cum_s += y[i];
        }
    }
}


void FNetLayerVariable::bprop()
{
    real* x = varray[0]->valuedata;
    real* dx = varray[0]->gradientdata;
    real* y = valuedata;
    real* dy = gradientdata;
    real c1 = varray[3]->valuedata[0];
    real c2 = varray[3]->valuedata[1];
    real* db = varray[2]->gradientdata;
    real& dc1 = varray[3]->gradientdata[0];
    real& dc2 = varray[3]->gradientdata[1];
    int mx=varray[0]->matValue.mod();
    int mdx = varray[0]->matGradient.mod();
    int my=matValue.mod();
    int mdy = matGradient.mod();
    for (int k=0;k<minibatch_size;k++, x+=mx, y+=my, dx+=mdx, dy+=mdy)
    {
        Mat u_k = u[k];
        real* inh_k = inh[k];
        real* cum_inh_k = cum_inh[k];
        real dcum_s = 0;
        Vec xk = varray[0]->matValue(k);
        Vec dxk = varray[0]->matGradient(k);
        for (int i=n_hidden-1;i>=0;i--)
        {
            real dai = (dy[i]+dcum_s)*y[i]*(1-y[i]);
            real erri = fabs(dai);
            avg_act_gradient = (1 - exp_moving_average_coefficient)*avg_act_gradient +
                exp_moving_average_coefficient * erri;
            if (erri > gradient_threshold)
            {
                real* dWi = varray[1]->matGradient[i];
                if (normalize_inputs)
                {
                    real* u_ki = u_k[i];
                    for (int j=0;j<n_inputs;j++)
                        dWi[j] += dai * u_ki[j];
                    Vec mu_i = mu(i);
                    Vec mu2_i = mu2(i);
                    exponentialMovingAverageUpdate(mu_i, xk, exp_moving_average_coefficient);
                    exponentialMovingSquareUpdate(mu2_i, xk, exp_moving_average_coefficient);
                } else
                    for (int j=0;j<n_inputs;j++)
                        dWi[j] += dai * x[j];
                db[i] += dai;
                if (inhibit_next_units && i>0)
                {
                    real inh_ki = inh_k[i]; 
                    if (!fast_exact_is_equal(c1_, 0)) // c1 is optimized.
                        dc1 -= dai * inh_ki;
                    if (squashed_inhibition) {
                        real dinh_ki = - dai * c1 * inh_ki * (1 - inh_ki);
                        if (!fast_exact_is_equal(c2_, 0)) // c2 is optimized.
                            dc2 += dinh_ki * cum_inh_k[i];
                        if (inhibit_by_sum)
                            dcum_s += dinh_ki * c2;
                        else
                            dcum_s += dinh_ki * c2 / i;
                    } else {
                        real dinh_ki = - dai * c1;
                        if (inhibit_by_sum)
                            dcum_s += dinh_ki;
                        else
                            dcum_s += dinh_ki / i;
                    }
                }
                if (backprop_to_inputs)
                {
                    Vec Wi = varray[1]->matValue(i);
                    multiplyAcc(dxk,Wi,dai);
                }
            }
        }
    }
    if (normalize_inputs)
        // invs = 1/ sqrt(mu2 - mu*mu)
        computeInverseStandardDeviationFromMeanAndSquareMean(invs,mu,mu2, min_stddev, min_stddev);
    gradient_threshold = average_error_fraction_to_threshold * avg_act_gradient;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void FNetLayerVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies) {
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(mu, copies);
    deepCopyField(invs, copies);
    deepCopyField(mu2, copies);
    deepCopyField(u, copies);
    deepCopyField(inh, copies);
    deepCopyField(cum_inh, copies);
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
