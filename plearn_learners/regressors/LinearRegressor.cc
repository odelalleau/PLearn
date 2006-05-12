
// -*- C++ -*-

// LinearRegressor.cc
//
// Copyright (C) 2003  Yoshua Bengio 
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

/*! \file LinearRegressor.cc */
#include "LinearRegressor.h"
#include <plearn/vmat/VMat_linalg.h>
#include <plearn/vmat/ExtendedVMatrix.h>
#include <plearn/math/pl_erf.h>

namespace PLearn {
using namespace std;

/* ### Initialise all fields to their default value here */
LinearRegressor::LinearRegressor()
    : sum_squared_y(MISSING_VALUE),
      sum_gammas(MISSING_VALUE),
      weights_norm(MISSING_VALUE),
      weights(),
      AIC(MISSING_VALUE),
      BIC(MISSING_VALUE),
      resid_variance(),
      include_bias(true),
      cholesky(true),
      weight_decay(0.0),
      output_learned_weights(false)
{ }

PLEARN_IMPLEMENT_OBJECT(
    LinearRegressor,
    "Ordinary Least Squares and Ridge Regression, optionally weighted", 
    "This class performs OLS (Ordinary Least Squares) and Ridge Regression, optionally on weighted\n"
    "data, by solving the linear equation (X'W X + weight_decay*n_examples*I) theta = X'W Y\n"
    "where X is the (n_examples x (1+inputsize)) matrix of extended inputs \n"
    "(with a 1 in the first column, only if the option 'include_bias' is true),\n"
    "Y is the (n_example x targetsize), W is a diagonal matrix of weights (one per example)\n"
    "{the identity matrix if weightsize()==0 in the training set}, and theta is the resulting\n"
    "set of parameters. W_{ii} is obtained from the weight column of the training set, if any.\n"
    "This column must have width 0 (no weight) or 1.\n"
    "A prediction (computeOutput) is obtained from an input vector as follows:\n"
    "   output = theta * (1,input)\n"
    "The criterion that is minimized by solving the above linear system is the squared loss"
    "plus squared norm penalty (weight_decay*sum_{ij} theta_{ij}^2) PER EXAMPLE. This class also measures"
    "the ordinary squared loss (||output-theta||^2). The two costs are named 'mse+penalty' and 'mse' respectively.\n"
    "Training has two steps: (1) computing X'W X and X' W Y, (2) solving the linear system.\n"
    "The first step takes time O(n_examples*inputsize^2 + n_examples*inputsize*outputsize).\n"
    "The second step takes time O(inputsize^3).\n"
    "If train() is called repeatedly with different values of weight_decay, without intervening\n"
    "calls to forget(), then the first step will be done only once, and only the second step\n"
    "is repeated.\n"
    "\n"
    "The Akaike Information Criterion (AIC) and Bayerian Information Criterion (BIC)\n"
    "are computed on the training set.  They are output as both training and test costs,\n"
    "with respective cost-names \"aic\" and \"bic\".  Their arithmetic mean is also output\n"
    "under costname \"mabic\".  Since these criteria are TRAINING concepts,  the\n"
    "test costs that are output are CONSTANT and equal to the training costs.\n"
    );

void LinearRegressor::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    //#####  Build Options  ####################################################

    declareOption(ol, "include_bias", &LinearRegressor::include_bias,
                  OptionBase::buildoption,
                  "Whether to include a bias term in the regression (true by default)");
  
    declareOption(ol, "cholesky", &LinearRegressor::cholesky,
                  OptionBase::buildoption, 
                  "Whether to use the Cholesky decomposition or not, "
                  "when solving the linear system. Default=1 (true)");

    declareOption(ol, "weight_decay", &LinearRegressor::weight_decay,
                  OptionBase::buildoption, 
                  "The weight decay is the factor that multiplies the "
                  "squared norm of the parameters in the loss function");

    declareOption(ol, "output_learned_weights",
                  &LinearRegressor::output_learned_weights,
                  OptionBase::buildoption,
                  "If true, the result of computeOutput*() functions is not the\n"
                  "result of thre regression, but the learned regression parameters.\n"
                  "(i.e. the matrix 'weights').  The matrix is flattened by rows.\n"
                  "NOTE by Nicolas Chapados: this option is a bit of a hack and might\n"
                  "be removed in the future.  Let me know if you come to rely on it.");
                
  
    //#####  Learnt Options  ###################################################
  
    declareOption(ol, "weights", &LinearRegressor::weights,
                  OptionBase::learntoption, 
                  "The weight matrix, which are the parameters computed by "
                  "training the regressor.\n");

    declareOption(ol, "AIC", &LinearRegressor::AIC,
                  OptionBase::learntoption,
                  "The Akaike Information Criterion computed at training time;\n"
                  "Saved as a learned option to allow outputting AIC as a test cost.");

    declareOption(ol, "BIC", &LinearRegressor::BIC,
                  OptionBase::learntoption,
                  "The Bayesian Information Criterion computed at training time;\n"
                  "Saved as a learned option to allow outputting BIC as a test cost.");

    declareOption(ol, "resid_variance", &LinearRegressor::resid_variance,
                  OptionBase::learntoption,
                  "Estimate of the residual variance for each output variable\n"
                  "Saved as a learned option to allow outputting confidence intervals\n"
                  "when model is reloaded and used in test mode.\n");
  
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Unused options.

    redeclareOption(ol, "seed", &LinearRegressor::seed_, OptionBase::nosave,
                    "The random seed is not used in a linear regressor.");
}

void LinearRegressor::build_()
{
    // This resets various accumulators to speed up successive iterations of
    // training in the case the training set has not changed.
    resetAccumulators();
}

// ### Nothing to add here, simply calls build_
void LinearRegressor::build()
{
    inherited::build();
    build_();
}


void LinearRegressor::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    deepCopyField(extendedinput, copies);
    deepCopyField(input, copies);
    deepCopyField(target, copies);
    deepCopyField(train_costs, copies);
    deepCopyField(XtX, copies);
    deepCopyField(XtY, copies);
    deepCopyField(weights, copies);
    deepCopyField(resid_variance, copies);
}


int LinearRegressor::outputsize() const
{
    // If we output the learned parameters, the outputsize is the number of
    // parameters
    if (output_learned_weights)
        return max(effective_inputsize() * targetsize(), -1);

    int ts = targetsize();
    if (ts >= 0) {
        return ts;
    } else {
        // This learner's training set probably hasn't been set yet, so
        // we don't know the targetsize.
        return 0;
    }
}

void LinearRegressor::resetAccumulators()
{
    XtX.resize(0,XtX.width());
    XtY.resize(0,XtY.width());
    sum_squared_y = 0;
    sum_gammas = 0;
}

void LinearRegressor::forget()
{
    resetAccumulators();
    resid_variance.resize(0);
}
  

void LinearRegressor::train()
{
    // Preparatory buffer allocation
    bool recompute_XXXY = (XtX.length()==0);
    extendedinput.resize(effective_inputsize());
    input = extendedinput;
    if (include_bias) {
        input = extendedinput.subVec(1,inputsize());
        extendedinput[0]=1.0;
    }
    target.resize(targetsize());  // the train_set's targetsize()
    weights.resize(extendedinput.length(),target.length());
    if (recompute_XXXY)
    {
        XtX.resize(extendedinput.length(),extendedinput.length());
        XtY.resize(extendedinput.length(),target.length());
    }
    if(!train_stats)  // make a default stats collector, in case there's none
        train_stats = new VecStatsCollector();

    train_stats->forget(); 

    // Compute training inputs and targets; take into account optional bias
    real squared_error=0;
    Vec outputwise_sum_squared_Y;
    VMat trainset_inputs  = train_set.subMatColumns(0,inputsize());
    VMat trainset_targets = train_set.subMatColumns(inputsize(), targetsize());
    if (include_bias)                          // prepend a first column of ones
        trainset_inputs = new ExtendedVMatrix(trainset_inputs,0,0,1,0,1.0);

    // Choose proper function depending on whether the dataset is weighted
    if (train_set->weightsize()<=0)
    {
        squared_error =
            linearRegression(trainset_inputs, trainset_targets,
                             weight_decay, weights, 
                             !recompute_XXXY, XtX, XtY,
                             sum_squared_y, outputwise_sum_squared_Y,
                             true, 0, cholesky, include_bias?1:0);
    }
    else if (train_set->weightsize()==1)
    {
        squared_error =
            weightedLinearRegression(trainset_inputs, trainset_targets,
                                     train_set.subMatColumns(inputsize()+targetsize(),1),
                                     weight_decay, weights,
                                     !recompute_XXXY, XtX, XtY, sum_squared_y, outputwise_sum_squared_Y,
                                     sum_gammas, true, 0, cholesky, include_bias?1:0);
    }
    else
        PLERROR("LinearRegressor: expected dataset's weightsize to be either 1 or 0, got %d\n",
                train_set->weightsize());

    // Update the AIC and BIC criteria
    computeInformationCriteria(squared_error, train_set.length());

    // Update the sigmas for confidence intervals (the current formula does
    // not account for the weights in the case of weighted linear regression)
    computeResidualsVariance(outputwise_sum_squared_Y);

    // Update the training costs
    Mat weights_excluding_biases = weights.subMatRows(include_bias? 1 : 0, inputsize());
    weights_norm = dot(weights_excluding_biases,weights_excluding_biases);
    train_costs.resize(5);
    train_costs[0] = squared_error + weight_decay*weights_norm;
    train_costs[1] = squared_error;
    train_costs[2] = AIC;
    train_costs[3] = BIC;
    train_costs[4] = (AIC+BIC)/2;
    train_stats->update(train_costs);
    train_stats->finalize(); 
}


void LinearRegressor::computeOutput(const Vec& actual_input, Vec& output) const
{
    // If 'output_learned_weights', don't compute the linear regression at
    // all, but instead flatten the weights vector and output it
    if (output_learned_weights) {
        output << weights.toVec();
        return;
    }
  
    // Compute the output from the input
    int nout = outputsize();
    output.resize(nout);
    if (input.length()==0) 
    {
        extendedinput.resize(effective_inputsize());
        input = extendedinput;
        if (include_bias) {
            input = extendedinput.subVec(1,inputsize());
            extendedinput[0] = 1.0;
        }
    }
    input << actual_input;
    transposeProduct(output,weights,extendedinput);
}

void LinearRegressor::computeCostsFromOutputs(
    const Vec& /*input*/, const Vec& output, const Vec& target, Vec& costs) const
{
    // If 'output_learned_weights', there is no test cost
    if (output_learned_weights)
        return;
  
    // Compute the costs from *already* computed output. 
    costs.resize(5);
    real squared_loss = powdistance(output,target);
    costs[0] = squared_loss + weight_decay*weights_norm;
    costs[1] = squared_loss;

    // The AIC/BIC/MABIC costs are computed at TRAINING-TIME and remain
    // constant thereafter.  Simply append the already-computed costs.
    costs[2] = AIC;
    costs[3] = BIC;
    costs[4] = (AIC+BIC)/2;
}

bool LinearRegressor::computeConfidenceFromOutput(
    const Vec&, const Vec& output, real probability,
    TVec< pair<real,real> >& intervals) const
{
    // The option 'output_learned_weights' is incompatible with confidence...
    if (output_learned_weights)
        PLERROR("LinearRegressor::computeConfidenceFromOutput: the option "
                "'output_learned_weights' is incompatible with confidence.");
  
    const int n = output.size();
    if (n != resid_variance.size())
        PLERROR("LinearRegressor::computeConfidenceFromOutput: output vector "
                "size (=%d) is incorrect or residuals variance (=%d) not yet computed",n,resid_variance.size());
  
    // two-tailed
    const real multiplier = gauss_01_quantile((1+probability)/2);
    intervals.resize(n);
    for (int i=0; i<n; ++i) {
        real half_width = multiplier * sqrt(resid_variance[i]);
        intervals[i] = std::make_pair(output[i] - half_width,
                                      output[i] + half_width);
    }
    return true;
}

TVec<string> LinearRegressor::getTestCostNames() const
{
    // If 'output_learned_weights', there is no test cost
    if (output_learned_weights)
        return TVec<string>();
    else
        return getTrainCostNames();
}

TVec<string> LinearRegressor::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes
    // and for which it updates the VecStatsCollector train_stats
    TVec<string> names;
    names.push_back("mse+penalty");
    names.push_back("mse");
    names.push_back("aic");
    names.push_back("bic");
    names.push_back("mabic");
    return names;
}

void LinearRegressor::computeInformationCriteria(real squared_error, int n)
{
    // AIC = ln(squared_error/n) + 2*M/n
    // BIC = ln(squared_error/n) + M*ln(n)/n,
    // where M is the number of parameters
    // NOTE the change in semantics: squared_error is now a MEAN squared error

    real M = weights.length() * weights.width();
    real lnsqerr = pl_log(squared_error);
    AIC = lnsqerr + 2*M/n;
    BIC = lnsqerr + M*pl_log(real(n))/n;
}

void LinearRegressor::computeResidualsVariance(const Vec&
                                               outputwise_sum_squared_Y)
{
    // The following formula (for the unweighted case) is used:
    //
    //    e'e = y'y - b'X'Xb
    //
    // where e is the residuals of the regression (for a single output), y
    // is a column of targets (for a single output), b is the weigths
    // vector, and X is the matrix of regressors.  From this point, use the
    // fact that an estimator of sigma is given by
    //
    //   sigma = e'e / (N-K),
    //
    // where N is the size of the training set and K is the extended input
    // size.
    const int ninputs  = weights.length();
    const int ntargets = weights.width();
    const int N = train_set.length();
    Vec b(ninputs);
    Vec XtXb(ninputs);
    resid_variance.resize(ntargets);
    for (int i=0; i<ntargets; ++i) {
        b << weights.column(i);
        product(XtX, b, XtXb);
        resid_variance[i] =
            (outputwise_sum_squared_Y[i] - dot(b,XtXb)) / (N-ninputs);
    }
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
