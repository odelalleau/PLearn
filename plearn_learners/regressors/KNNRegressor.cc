// -*- C++ -*-

// KNNRegressor.cc
//
// Copyright (C) 2004 Nicolas Chapados 
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

// Authors: Nicolas Chapados

/*! \file KNNRegressor.cc */

#include "KNNRegressor.h"
#include <assert.h>
#include <math.h>

#include <plearn/base/tostring.h>
#include <plearn/math/TMat_maths.h>
#include <plearn_learners/nearest_neighbors/ExhaustiveNearestNeighbors.h>
#include <plearn/ker/EpanechnikovKernel.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    KNNRegressor,
    "Implementation of the Nadaraya-Watson kernel estimator for regression",
    "This class provides a simple multivariate regressor based upon an\n"
    "enclosed K-nearest-neighbors finder (derived from\n"
    "GenericNearestNeighbors; specified with the 'knn' option).\n"
    "\n"
    "The class contains several options to determine the number of neighbors\n"
    "to use (K).  This number always overrides the option 'num_neighbors'\n"
    "that may have been specified in the GenericNearestNeighbors utility\n"
    "object.  Basically, the generic formula for the number of neighbors is\n"
    "\n"
    "    K = max(kmin, kmult*(n^kpow)),\n"
    "\n"
    "where 'kmin', 'kmult', and 'kpow' are options, and 'n' is the number of\n"
    "examples in the training set.\n"
    "\n"
    "The cost output from this class is:\n"
    "\n"
    "- 'mse', the mean-squared error, i.e. given an output o and target t,\n"
    "      mse(o,t) = sum_i (o[i]-t[i])^2,\n"
    "\n"
    "If the option 'use_knn_costs_as_weights' is true (by default), it is\n"
    "assumed that the costs coming from the 'knn' object are kernel\n"
    "evaluations for each nearest neighbor.  These are used as weights to\n"
    "determine the final class probabilities.  (NOTE: it is important to use\n"
    "a kernel that computes a SIMILARITY MEASURE, and not a DISTANCE MEASURE;\n"
    "the default EpanechnikovKernel has the proper behavior.)  If the option\n"
    "is false, an equal weighting is used (equivalent to square window).  In\n"
    "addition, a different weighting kernel may be specified with the\n"
    "'kernel' option.\n"
    "\n"
    "A local weighted regression model may be trained at each test point\n"
    "by specifying a 'local_model'.  For instance, to perform local linear\n"
    "regression, you may use a LinearRegressor for this purpose.\n"
    );

KNNRegressor::KNNRegressor()
    : knn(new ExhaustiveNearestNeighbors(new EpanechnikovKernel(), false)),
      kmin(5),
      kmult(0.0),
      kpow(0.5),
      use_knn_costs_as_weights(true),
      kernel(),
      local_model()
{ }

void KNNRegressor::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "knn", &KNNRegressor::knn, OptionBase::buildoption,
        "The K-nearest-neighbors finder to use (default is an\n"
        "ExhaustiveNearestNeighbors with a EpanechnikovKernel, lambda=1)");

    declareOption(
        ol, "kmin", &KNNRegressor::kmin, OptionBase::buildoption,
        "Minimum number of neighbors to use (default=5)");

    declareOption(
        ol, "kmult", &KNNRegressor::kmult, OptionBase::buildoption,
        "Multiplicative factor on n^kpow to determine number of neighbors to\n"
        "use (default=0)");

    declareOption(
        ol, "kpow", &KNNRegressor::kpow, OptionBase::buildoption,
        "Power of the number of training examples to determine number of\n"
        "neighbors (default=0.5)");

    declareOption(
        ol, "use_knn_costs_as_weights", &KNNRegressor::use_knn_costs_as_weights,
        OptionBase::buildoption,
        "Whether to weigh each of the K neighbors by the kernel evaluations,\n"
        "obtained from the costs coming out of the 'knn' object (default=true)");

    declareOption(
        ol, "kernel", &KNNRegressor::kernel, OptionBase::buildoption,
        "Disregard the 'use_knn_costs_as_weights' option, and use this kernel\n"
        "to weight the observations.  If this object is not specified\n"
        "(default), and the 'use_knn_costs_as_weights' is false, the\n"
        "rectangular kernel is used.");

    declareOption(
        ol, "local_model", &KNNRegressor::local_model, OptionBase::buildoption,
        "Train a local regression model from the K neighbors, weighted by\n"
        "the kernel evaluations.  This is carried out at each test point.");
  
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void KNNRegressor::build_()
{
    if (!knn)
        PLERROR("KNNRegressor::build_: the 'knn' option must be specified");

    if (kmin <= 0)
        PLERROR("KNNRegressor::build_: the 'kmin' option must be strictly positive");
}

// ### Nothing to add here, simply calls build_
void KNNRegressor::build()
{
    inherited::build();
    build_();
}


void KNNRegressor::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(knn_output,           copies);
    deepCopyField(knn_costs,            copies);
    deepCopyField(knn,                  copies);
    deepCopyField(kernel,               copies);
    deepCopyField(local_model,          copies);
    inherited::makeDeepCopyFromShallowCopy(copies);
}


int KNNRegressor::outputsize() const
{
    return train_set->targetsize();
}


void KNNRegressor::setTrainingSet(VMat training_set, bool call_forget)
{
    PLASSERT( knn );
    inherited::setTrainingSet(training_set,call_forget);

    // Now we carry out a little bit of tweaking on the embedded knn:
    // - ask to report input+target+weight
    // - set number of neighbors
    // - set training set (which performs a build if necessary)
    int n = training_set.length();
    int num_neighbors = max(kmin, int(kmult*pow(double(n), double(kpow))));
    knn->num_neighbors = num_neighbors;
    knn->copy_input  = true;
    knn->copy_target = true;
    knn->copy_weight = true;
    knn->copy_index  = false;
    knn->setTrainingSet(training_set,call_forget);
    knn_costs.resize(knn->nTestCosts());
    knn_output.resize(knn->outputsize());
}

void KNNRegressor::forget()
{
    PLASSERT( knn );
    knn->forget();
}
    
void KNNRegressor::train()
{
    PLASSERT( knn );
    knn->train();
}

void KNNRegressor::computeOutput(const Vec& input, Vec& output) const
{
    PLASSERT( output.size() == outputsize() );

    // Start by computing the nearest neighbors
    Vec knn_targets;                           //!< not used by knn
    knn->computeOutputAndCosts(input, knn_targets, knn_output, knn_costs);

    // A little sanity checking on the knn costs: make sure that they not all
    // zero as this certainly indicates a wrong kernel
    bool has_non_zero_costs = false;
    for (int i=0, n=knn_costs.size() ; i<n && !has_non_zero_costs ; ++i)
        has_non_zero_costs = !is_missing(knn_costs[i]) && !is_equal(knn_costs[i], 0.0);
    if (! has_non_zero_costs) {
        string input_str = tostring(input, PStream::pretty_ascii);
        PLWARNING("KNNRegressor::computeOutput: all %d neighbors have zero similarity with\n"
                  "input vector %s;\n"
                  "check the similarity kernel bandwidth.  Replacing them by uniform weights.",
                  knn_costs.size(), input_str.c_str());
        knn_costs.fill(1.0);
    }
  
    // For each neighbor, the KNN object outputs the following:
    //     1) input vector
    //     2) output vector
    //     3) the weight (in all cases)
    // We shall patch the weight of each neighbor (observation) to reflect
    // the effect of the kernel weighting
    const int inputsize    = input.size();
    const int outputsize   = output.size();
    const int weightoffset = inputsize+outputsize;
    const int rowwidth     = weightoffset+1;
    real* knn_output_data  = knn_output.data();
    real total_weight      = 0.0;
    for (int i=0, n=knn->num_neighbors; i<n; ++i, knn_output_data += rowwidth) {
        real w;
        if (kernel) {
            Vec cur_input(inputsize, knn_output_data);
            w = kernel(cur_input, input);
        }
        else if (use_knn_costs_as_weights)
            w = knn_costs[i];
        else
            w = 1.0;

        if (is_missing(w))
            w = 0.0;
    
        // Patch the existing weight
        knn_output_data[weightoffset] *= w;
        total_weight += knn_output_data[weightoffset];
    }

    // If total weight is too small, make the output all zeros
    if (total_weight < 1e-6) {
        output.fill(0.0);
        return;
    }
  
    // Now compute the output per se
    if (! local_model) {
        // If no local model was requested, simply perform a weighted
        // average of the nearest-neighbors
        output.fill(0.0);
        knn_output_data = knn_output.data();
        for (int i=0, n=knn->num_neighbors; i<n; ++i, knn_output_data+=rowwidth) {
            Vec cur_output(outputsize, knn_output_data+inputsize);
            multiplyAcc(output, cur_output,
                        knn_output_data[weightoffset] / total_weight);
        }
    }
    else {
        // Reinterpret knn_output as a training set and use local model
        Mat training_data = knn_output.toMat(knn->num_neighbors, rowwidth);
        VMat training_set(training_data);
        training_set->defineSizes(inputsize, outputsize, 1 /* weightsize */);
        local_model->setTrainingSet(training_set, true /* forget */);
        local_model->setTrainStatsCollector(new VecStatsCollector());
        local_model->train();
        local_model->computeOutput(input,output);
    }
}

void KNNRegressor::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
    PLASSERT( costs.size() == 1 );
    costs[0] = powdistance(output,target,2);
}

bool KNNRegressor::computeConfidenceFromOutput(const Vec& input, const Vec& output,
                                               real probability,
                                               TVec< pair<real,real> >& intervals) const
{
    if (! local_model)
        return false;                            //!< for now -- to be fixed

    // Assume that the local model has been trained; don't re-train it
    return local_model->computeConfidenceFromOutput(input, output, probability, intervals);
}


TVec<string> KNNRegressor::getTestCostNames() const
{
    static TVec<string> costs(1);
    costs[0] = "mse";
    return costs;
}

TVec<string> KNNRegressor::getTrainCostNames() const
{
    return TVec<string>();
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
