
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
   * $Id: LinearRegressor.cc,v 1.1 2003/08/26 23:24:13 chapados Exp $
   ******************************************************* */

/*! \file LinearRegressor.cc */
#include "LinearRegressor.h"
#include "VMat_maths.h"

namespace PLearn <%
using namespace std;

/* ### Initialise all fields to their default value here */
LinearRegressor::LinearRegressor() : weight_decay(0)
{}

PLEARN_IMPLEMENT_OBJECT(LinearRegressor, "Ordinary Least Squares and Ridge Regression, optionally weighted", 
                        "This class performs OLS (Ordinary Least Squares) and Ridge Regression, optionally on weighted\n"
                        "data, by solving the linear equation (X'W X + weight_decay*n_examples*I) theta = X'W Y\n"
                        "where X is the (n_examples x (1+inputsize)) matrix of extended inputs (with a 1 in the first column),\n"
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
                        "is repeated.\n");

  void LinearRegressor::declareOptions(OptionList& ol)
  {
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave
    declareOption(ol, "weight_decay", &LinearRegressor::weight_decay, OptionBase::buildoption, 
                  "    The weight decay is the factor that multiplies the squared norm of the parameters in the loss function\n");

    declareOption(ol, "weights", &LinearRegressor::weights, OptionBase::learntoption, 
                  "    The weight matrix, which are the parameters computed by training the regressor.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  void LinearRegressor::build_()
  {
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
  }

  // ### Nothing to add here, simply calls build_
  void LinearRegressor::build()
  {
    inherited::build();
    build_();
  }


  void LinearRegressor::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    deepCopyField(weights, copies);
    deepCopyField(XtX, copies);
    deepCopyField(XtY, copies);
  }


int LinearRegressor::outputsize() const
{
  // compute and return the size of this learner's output, (which typically
  // may depend on its inputsize(), targetsize() and set options)
  return targetsize();
}

void LinearRegressor::forget()
{
  XtX.resize(0,XtX.width());
  XtY.resize(0,XtY.width());
  sum_squared_y = 0;
}
    
static Vec extendedinput; //!<  length 1+inputsize(), first element is 1.0 (used by the use method)
static Vec input; //!<  extendedinput.subVec(1,inputsize())
static Vec target;
static Vec train_costs;

void LinearRegressor::train()
{
  bool recompute_XXXY = (XtX.length()==0);
  extendedinput.resize(1+inputsize());
  input = extendedinput.subVec(1,inputsize());
  extendedinput[0]=1.0;
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

  real squared_error=0;
  train_costs.resize(2);

  if (train_set->weightsize()<=0)
    {
      squared_error =
        linearRegression(train_set.subMatColumns(0, inputsize()), 
                         train_set.subMatColumns(inputsize(), outputsize()), 
                         weight_decay*train_set.length(), weights, 
                         !recompute_XXXY, XtX, XtY,sum_squared_y,true);
    }
  else if (train_set->weightsize()==1)
    {
      squared_error =
        weightedLinearRegression(train_set.subMatColumns(0, inputsize()), 
                                 train_set.subMatColumns(inputsize(), outputsize()),
                                 train_set.lastColumn(), weight_decay*train_set.length(), weights,
                                 !recompute_XXXY, XtX, XtY,sum_squared_y,true);
    }
  else PLERROR("LinearRegressor: expected dataset's weightsize to be either 1 or 0, got %d\n",train_set->weightsize());

  Mat weights_excluding_biases = weights.subMatRows(1,inputsize());
  weights_norm = dot(weights_excluding_biases,weights_excluding_biases);
  train_costs[0] = (squared_error + weight_decay*train_set.length()*weights_norm)/train_set.length();
  train_costs[1] = squared_error/train_set.length();
  train_stats->update(train_costs);
  train_stats->finalize(); 
}


void LinearRegressor::computeOutput(const Vec& actual_input, Vec& output) const
{
  // Compute the output from the input
  int nout = outputsize();
  output.resize(nout);
  input << actual_input;
  transposeProduct(output,weights,extendedinput);
}    

void LinearRegressor::computeCostsFromOutputs(const Vec& actual_input, const Vec& output, 
                                              const Vec& target, Vec& costs) const
{
  // Compute the costs from *already* computed output. 
  costs.resize(2);
  real squared_loss = powdistance(output,target);
  costs[0] = squared_loss + weight_decay*weights_norm;
  costs[1] = squared_loss;
}                                

TVec<string> LinearRegressor::getTestCostNames() const
{
  return getTrainCostNames();
}

TVec<string> LinearRegressor::getTrainCostNames() const
{
  // Return the names of the objective costs that the train method computes and 
  // for which it updates the VecStatsCollector train_stats
  TVec<string> names(2);
  names[0] = "mse+penalty";
  names[1] = "mse";
  return names;
}



%> // end of namespace PLearn
