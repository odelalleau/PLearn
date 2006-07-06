// -*- C++ -*-

// AutoLinearRegressor.cc
//
// Copyright (C) 2006 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file AutoLinearRegressor.cc */


#include "AutoLinearRegressor.h"
#include "plearn/math/plapack.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    AutoLinearRegressor,
    "This class performs ridge regression but automatically choosing the weight_decay.",
    "The selection of weight decay is done in order to minimize the Generalized \n"
    "Cross Validation (GCV) criterion(Craven & Wahba 1979).\n"
    "Note: when mature enough, this class might get folded back into LinearRegressor,\n"
    "which will require proper handling of the extra bells and whistles of LinearRegressor.\n");

AutoLinearRegressor::AutoLinearRegressor()
    : include_bias(false),
      weight_decay(0.0)
{
}

void AutoLinearRegressor::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &AutoLinearRegressor::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    declareOption(ol, "include_bias", &AutoLinearRegressor::include_bias,
                  OptionBase::buildoption,
                  "Whether to include a bias term in the regression \n"
                  "Note: this is currently ignored.\n");

    declareOption(ol, "weight_decay", &AutoLinearRegressor::weight_decay,
                  OptionBase::learntoption, 
                  "The weight decay is the factor that multiplies the \n"
                  "squared norm of the parameters in the loss function.\n"
                  "It is automatically tuned by the algorithm \n");

    declareOption(ol, "weights", &AutoLinearRegressor::weights,
                  OptionBase::learntoption, 
                  "The weight matrix, which are the parameters computed by "
                  "training the regressor.\n");

    declareOption(ol, "mean_target", &AutoLinearRegressor::mean_target,
                  OptionBase::learntoption,
                  "The mean of the target. (used as a default bias)");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void AutoLinearRegressor::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.
}

// ### Nothing to add here, simply calls build_
void AutoLinearRegressor::build()
{
    inherited::build();
    build_();
}


void AutoLinearRegressor::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("AutoLinearRegressor::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int AutoLinearRegressor::outputsize() const
{
    return targetsize();
}

void AutoLinearRegressor::forget()
{
    weights.resize(0,0);
    stage = 0;
    inherited::forget();
}

void AutoLinearRegressor::train()
{
    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    if(stage<1)
    {
        // clear statistics of previous epoch
        train_stats->forget();
        
        int ninputs = train_set->inputsize();
        int ntargets = train_set->targetsize();
        int nweights = train_set->weightsize();
        
        // extended inputs
        int insize = ninputs; 

        if(nweights!=0)
            PLERROR("In AutoLinearRegressor, sample weights not yet supported");

        Mat tset = train_set->toMat();
        Mat X = tset.subMatColumns(0,ninputs);
        Mat Y = tset.subMatColumns(ninputs, ntargets);

        mean_target.resize(ntargets);
        columnMean(Y, mean_target);
        Y -= mean_target;

        //weights.resize(insize, ntargets);
        weights.resize(ntargets, insize);
        real best_GCV;
      
        weight_decay = ridgeRegressionByGCV(X, Y, weights, best_GCV);

        //Mat weights_excluding_biases = weights.subMatRows(include_bias? 1 : 0, ninputs);
        Mat weights_excluding_biases = weights.subMatColumns(include_bias? 1 : 0, ninputs);
        weights_norm = dot(weights_excluding_biases,weights_excluding_biases);

        //Vec trcosts(1);
        Vec trcosts(2);
        trcosts[0] = best_GCV;
        trcosts[1] = best_GCV;
        train_stats->update(trcosts);

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
}


void AutoLinearRegressor::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input
    int nout = outputsize();
    output.resize(nout);
    if(!include_bias)        
        product(output,weights,input);
    else
    {   
        int nin = inputsize();
        extendedinput.resize(1+nin);
        extendedinput.subVec(1,nin) << input;
        extendedinput[0] = 1.0;
        product(output,weights,extendedinput);
    }
    output += mean_target;
}

void AutoLinearRegressor::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    // Compute the costs from *already* computed output. 
    costs.resize(2);
    real squared_loss = powdistance(output,target);
    costs[0] = squared_loss + weight_decay*weights_norm;
    costs[1] = squared_loss;
}

TVec<string> AutoLinearRegressor::getTestCostNames() const
{
    TVec<string> names;
    names.push_back("mse+penalty");
    names.push_back("mse");
    return names;
}

TVec<string> AutoLinearRegressor::getTrainCostNames() const
{
    TVec<string> names;
    names.push_back("GCV_mse");
    names.push_back("mse");
    return names;
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
